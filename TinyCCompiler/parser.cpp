#include "parser.h"

std::unique_ptr<ProgramAST> Parser::Parse()
{
	return parseProgram();
}

std::vector<Error*> Parser:: GetErrors() {
	return errors;
}

Parser::~Parser()
{
	for (auto item : errors) {
		delete item;
	}
	errors.clear();
}

void Parser::getNextToken()
{
	curToken = tokens[++tokenNum];
}

void Parser::getPrevToken()
{
	curToken = tokens[--tokenNum];
}

std::unique_ptr<ProgramAST> Parser::parseProgram() {
	getNextToken();
	auto func = parseFunction();
	if (!func) { return nullptr; }
	return std::make_unique<ProgramAST>(std::move(func));
}

std::unique_ptr<FunctionAST> Parser::parseFunction() {
	if (curToken.type != TokenType::IntType) { 
		errors.push_back(CompilerError::errorAtLine("Expected 'int'!", curToken));
		return nullptr; 
	}

	getNextToken();
	if (curToken.type != TokenType::Identifier) { 
		errors.push_back(CompilerError::errorAtLine("Function definition must have identifier.", curToken));
		return nullptr; 
	}
	auto name = curToken.lexeme;

	getNextToken();
	if (curToken.type != TokenType::OpenParenthese) { 
		errors.push_back(CompilerError::errorAtLine("Expected '('!", curToken));
		return nullptr; 
	}

	getNextToken();
	if (curToken.type != TokenType::CloseParenthese) { 
		errors.push_back(CompilerError::errorAtLine("Expected ')'!", curToken));
		return nullptr; 
	}

	getNextToken();
	if (curToken.type != TokenType::OpenBrace) {
		errors.push_back(CompilerError::errorAtLine("Expected '{'!", curToken));
		return nullptr;
	}

	std::vector<std::unique_ptr<StatementAST>> statements;
	while(true) {
		getNextToken();
		if (STATEMENT_FIRST.find(curToken.type) == STATEMENT_FIRST.end()) {
			break;
		}

		auto statement = parseStatement();
		if (statement == nullptr) { return nullptr; };
		statements.push_back(std::move(statement));
	}

	if (curToken.type != TokenType::CloseBrace) { 
		errors.push_back(CompilerError::errorAtLine("Expected '}'!", curToken));
		return nullptr; 
	}

	return std::make_unique<FunctionAST>(name, statements);
}

std::unique_ptr<StatementAST> Parser::parseStatement() {
	// <statement> := "return" <expr> ";"
	if (curToken.type == TokenType::ReturnKeyword)
	{
		getNextToken();
		auto expr = parseExpression();
		if (!expr) {
			errors.push_back(CompilerError::errorAtLine("Function must return a value!", curToken));
			return nullptr;
		}

		getNextToken();
		if (curToken.type != TokenType::Semicolon) {
			errors.push_back(CompilerError::errorAtLine("Expected ';'!", curToken));
			return nullptr;
		}

		return std::make_unique<StatementAST>(StatementType::RETURN_STATEMENT, std::move(expr));
	}
	// <statement> := <expr> ";"
	else if (EXPRESSION_FIRST.find(curToken.type) != EXPRESSION_FIRST.end()) {
		auto expr = parseExpression();
		if (!expr) {
			errors.push_back(CompilerError::errorAtLine("Invalid expression!", curToken));
			return nullptr;
		}

		getNextToken();
		if (curToken.type != TokenType::Semicolon) {
			errors.push_back(CompilerError::errorAtLine("Expected ';'!", curToken));
			return nullptr;
		}

		return std::make_unique<StatementAST>(StatementType::EXPRESSION_STATEMENT, std::move(expr));
	}
	else if (curToken.type == TokenType::IntType) {
		getNextToken();
		if(curToken.type != TokenType::Identifier) {
			errors.push_back(CompilerError::errorAtLine("Expected identifier!", curToken));
			return nullptr;
		}
		std::string name = curToken.lexeme;

		getNextToken();
		// <statement> := "int" <id> ";"
		if (curToken.type == TokenType::Semicolon) {
			return std::make_unique<StatementAST>(StatementType::VARIABLE_DECLARATION, name);
		}

		std::unique_ptr<ExprAST> expr;
		// <statement> := "int" <id> "=" <exp> ";"
		if (curToken.type == TokenType::Assignment) {
			getNextToken();

			expr = parseExpression();
			if (!expr) {
				errors.push_back(CompilerError::errorAtLine("Invalid expression!", curToken));
				return nullptr;
			}
		}

		getNextToken();
		if (curToken.type != TokenType::Semicolon) {
			errors.push_back(CompilerError::errorAtLine("Expected ';'!", curToken));
			return nullptr;
		}

		return std::make_unique<StatementAST>(StatementType::VARIABLE_DECLARATION_WITH_ASSIGNMENT, name, std::move(expr));
	}

	return nullptr;
}

std::unique_ptr<ExprAST> Parser::parseExpression()
{
	// <expr> := <id> "=" <expr>
	if (curToken.type == TokenType::Identifier) {
		auto name = curToken.lexeme;

		getNextToken();
		if (curToken.type != TokenType::Assignment) {
			getPrevToken();
			return parseLogicalOrExpression();
		}
		
		getNextToken();
		auto expr = parseExpression();
		if (!expr) {
			errors.push_back(CompilerError::errorAtLine("Invalid expression!", curToken));
			return nullptr;
		}

		return std::make_unique<ExprAST>(name, std::move(expr));
	}
	// <expr> := <logical-expr> 
	else {
		return parseLogicalOrExpression();
	}
}

std::unique_ptr<ExprAST> Parser::parseAdditiveExpression() {
	auto left = parseTerm();

	getNextToken();
	while (curToken.type == TokenType::Addition || curToken.type == TokenType::Negation) {
		auto opType = curToken.type;
		getNextToken();
		auto right = parseTerm();
		if (!right) {
			errors.push_back(CompilerError::errorAtLine("Wrong right operand!", curToken));
			return nullptr;
		}
		left = std::make_unique<ExprAST>(std::move(left), opType, std::move(right));
		getNextToken();
	}
	getPrevToken();

	return std::move(left);
}

std::unique_ptr<ExprAST> Parser::parseLogicalOrExpression() {
	auto left = parseLogicalAndExpression();

	getNextToken();
	while (curToken.type == TokenType::LogicalOr) {
		auto opType = curToken.type;
		getNextToken();
		auto right = parseLogicalAndExpression();
		if (!right) {
			errors.push_back(CompilerError::errorAtLine("Wrong right operand!", curToken));
			return nullptr;
		}
		left = std::make_unique<ExprAST>(std::move(left), opType, std::move(right));
		getNextToken();
	}
	getPrevToken();

	return std::move(left);
}

std::unique_ptr<ExprAST> Parser::parseLogicalAndExpression() {
	auto left = parseAdditiveExpression();

	getNextToken();
	while (curToken.type == TokenType::LogicalAnd) {
		auto opType = curToken.type;
		getNextToken();
		auto right = parseAdditiveExpression();
		if (!right) {
			errors.push_back(CompilerError::errorAtLine("Wrong right operand!", curToken));
			return nullptr;
		}
		left = std::make_unique<ExprAST>(std::move(left), opType, std::move(right));
		getNextToken();
	}
	getPrevToken();

	return std::move(left);
}

std::unique_ptr<ExprAST> Parser::parseFactor()
{
	if (curToken.type == TokenType::OpenParenthese) {
		getNextToken();
		auto expr = parseExpression();
		if (!expr) {
 			return nullptr;
		}

		getNextToken();
		if (curToken.type != TokenType::CloseParenthese) {
			errors.push_back(CompilerError::errorAtLine("Expected ')'!", curToken));
			return nullptr;
		}

		return std::move(expr);
	}
	else if (curToken.isUnaryOperator()) {
		auto unOp = curToken;
		getNextToken();
		auto factor = parseFactor();
		if (!factor) {
			errors.push_back(CompilerError::errorAtLine("Wrong operand!", curToken));
			return nullptr;
		}
		return std::make_unique<ExprAST>(unOp.type, std::move(factor));

	}
	else if (curToken.type == TokenType::IntValue) {
		return std::make_unique<ExprAST>(curToken.intVal);
	}
	else if (curToken.type == TokenType::Identifier) {
		return std::make_unique<ExprAST>(curToken.lexeme);
	}

	return nullptr;
}

std::unique_ptr<ExprAST> Parser::parseTerm()
{
	auto left = parseFactor();

	getNextToken();
	while (curToken.type == TokenType::Multiplication || curToken.type == TokenType::Division) {
		auto opType = curToken.type;
		getNextToken();
		auto right = parseFactor();
		left = std::make_unique<ExprAST>(std::move(left), opType, std::move(right));
		getNextToken();
	}
	getPrevToken();

	return std::move(left);
}
