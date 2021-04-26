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
	if (curToken.type != TokenType::End) {
		return nullptr;
	}
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
	auto block = parseBlock();
	if (!block) { return nullptr; }

	return std::make_unique<FunctionAST>(name, std::move(block));
}

std::unique_ptr<BlockAST> Parser::parseBlock()
{
	if (curToken.type != TokenType::OpenBrace) {
		errors.push_back(CompilerError::errorAtLine("Expected '{'!", curToken));
		return nullptr;
	}

	std::vector<std::unique_ptr<BlockItemAST>> items;
	getNextToken();
	while (true) {
		if (BLOCK_ITEM_FIRST.find(curToken.type) == BLOCK_ITEM_FIRST.end()) {
			break;
		}

		auto item = parseBlockItem();
		if (item == nullptr) { return nullptr; };
		items.push_back(std::move(item));
	}

	if (curToken.type != TokenType::CloseBrace) {
		errors.push_back(CompilerError::errorAtLine("Expected '}'!", curToken));
		return nullptr;
	}

	if (tokenNum + 1 < tokens.size()) { getNextToken(); }
	return std::make_unique<BlockAST>(items);
}

std::unique_ptr<BlockItemAST> Parser::parseBlockItem() {
	if (curToken.type == TokenType::IntType) {
		auto declaration = parseDeclaration();
		
		if (!declaration) {
			return nullptr;
		}
		return std::make_unique<BlockItemAST>(std::move(declaration));
	}
	else if (STATEMENT_FIRST.find(curToken.type) != STATEMENT_FIRST.end()) {
		auto statement = parseStatement();

		if (!statement) {
			return nullptr;
		}
		return std::make_unique<BlockItemAST>(std::move(statement));
	}

	return nullptr;
}

std::unique_ptr<DeclarationAST> Parser::parseDeclaration()
{
	getNextToken();
	if (curToken.type != TokenType::Identifier) {
		errors.push_back(CompilerError::errorAtLine("Expected identifier!", curToken));
		return nullptr;
	}
	std::string name = curToken.lexeme;

	getNextToken();
	// <declaration> := "int" <id> ";"
	if (curToken.type == TokenType::Semicolon) {
		getNextToken();

		return std::make_unique<DeclarationAST>(name);
	}

	std::unique_ptr<ExprAST> expr;
	// <declaration> := "int" <id> "=" <exp> ";"
	if (curToken.type == TokenType::Assignment) {
		getNextToken();

		expr = parseExpression();
		if (!expr) {
			errors.push_back(CompilerError::errorAtLine("Invalid expression!", curToken));
			return nullptr;
		}
	}

	if (curToken.type != TokenType::Semicolon) {
		errors.push_back(CompilerError::errorAtLine("Expected ';'!", curToken));
		return nullptr;
	}
	getNextToken();

	return std::make_unique<DeclarationAST>(name, std::move(expr));
}

std::unique_ptr<ConditionAST> Parser::parseCondition()
{
	getNextToken();

	if (curToken.type != TokenType::OpenParenthese) {
		errors.push_back(CompilerError::errorAtLine("Expected '('!", curToken));
		return nullptr;
	}

	getNextToken();
	auto expr = parseExpression();

	if (curToken.type != TokenType::CloseParenthese) {
		errors.push_back(CompilerError::errorAtLine("Expected ')'!", curToken));
		return nullptr;
	}

	getNextToken();
	auto ifClause = parseBlock();
	if (!ifClause) { return nullptr; }
	
	if (curToken.type == TokenType::ElseOperator) {
		getNextToken();
		auto elseClause = parseBlock();
		if (!elseClause) { return nullptr; }

		return std::make_unique<ConditionAST>(std::move(expr), std::move(ifClause), std::move(elseClause));
	}

	return std::make_unique<ConditionAST>(std::move(expr), std::move(ifClause), nullptr);
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

		if (curToken.type != TokenType::Semicolon) {
			errors.push_back(CompilerError::errorAtLine("Expected ';'!", curToken));
			return nullptr;
		}

		getNextToken();
		return std::make_unique<StatementAST>(StatementType::RETURN_STATEMENT, std::move(expr));
	}
	// <statement> := <expr> ";"
	else if (EXPRESSION_FIRST.find(curToken.type) != EXPRESSION_FIRST.end()) {
		auto expr = parseExpression();
		if (!expr) {
			errors.push_back(CompilerError::errorAtLine("Invalid expression!", curToken));
			return nullptr;
		}

		if (curToken.type != TokenType::Semicolon) {
			errors.push_back(CompilerError::errorAtLine("Expected ';'!", curToken));
			return nullptr;
		}

		getNextToken();
		return std::make_unique<StatementAST>(StatementType::EXPRESSION_STATEMENT, std::move(expr));
	}
	else if (curToken.type == TokenType::OpenBrace) {
		auto block = parseBlock();

		if (!block) {
			return nullptr;
		}

		return std::make_unique<StatementAST>(std::move(block));
	}
	// <statement> := if(<expr>) <block> [ else <block> ]
	else if (curToken.type == TokenType::IfOperator) {
		auto condition = parseCondition();
		if (!condition) {
			return nullptr;
		}
		return std::make_unique<StatementAST>(std::move(condition));
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

std::unique_ptr<ExprAST> Parser::parseLogicalOrExpression() {
	auto left = parseLogicalAndExpression();

	while (curToken.type == TokenType::LogicalOr) {
		auto opType = curToken.type;
		getNextToken();
		auto right = parseLogicalAndExpression();
		if (!right) {
			errors.push_back(CompilerError::errorAtLine("Wrong right operand!", curToken));
			return nullptr;
		}
		left = std::make_unique<ExprAST>(std::move(left), opType, std::move(right));
	}

	return std::move(left);
}

std::unique_ptr<ExprAST> Parser::parseLogicalAndExpression() {
	auto left = parseEqualityExpression();

	while (curToken.type == TokenType::LogicalAnd) {
		auto opType = curToken.type;
		getNextToken();
		auto right = parseEqualityExpression();
		if (!right) {
			errors.push_back(CompilerError::errorAtLine("Wrong right operand!", curToken));
			return nullptr;
		}
		left = std::make_unique<ExprAST>(std::move(left), opType, std::move(right));
	}

	return std::move(left);
}

std::unique_ptr<ExprAST> Parser::parseEqualityExpression()
{
	auto left = parseComparasionExpression();

	while (curToken.type == TokenType::Equal || curToken.type == TokenType::NotEqual) {
		auto opType = curToken.type;
		getNextToken();
		auto right = parseComparasionExpression();
		if (!right) {
			errors.push_back(CompilerError::errorAtLine("Wrong right operand!", curToken));
			return nullptr;
		}
		left = std::make_unique<ExprAST>(std::move(left), opType, std::move(right));
	}

	return std::move(left);
}

std::unique_ptr<ExprAST> Parser::parseComparasionExpression()
{
	auto left = parseAdditiveExpression();

	while (curToken.type == TokenType::Less || curToken.type == TokenType::Greater) {
		auto opType = curToken.type;
		getNextToken();
		auto right = parseAdditiveExpression();
		if (!right) {
			errors.push_back(CompilerError::errorAtLine("Wrong right operand!", curToken));
			return nullptr;
		}
		left = std::make_unique<ExprAST>(std::move(left), opType, std::move(right));
	}

	return std::move(left);
}

std::unique_ptr<ExprAST> Parser::parseAdditiveExpression() {
	auto left = parseTerm();

	while (curToken.type == TokenType::Addition || curToken.type == TokenType::Negation) {
		auto opType = curToken.type;
		getNextToken();
		auto right = parseTerm();
		if (!right) {
			errors.push_back(CompilerError::errorAtLine("Wrong right operand!", curToken));
			return nullptr;
		}
		left = std::make_unique<ExprAST>(std::move(left), opType, std::move(right));
	}

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

		if (curToken.type != TokenType::CloseParenthese) {
			errors.push_back(CompilerError::errorAtLine("Expected ')'!", curToken));
			return nullptr;
		}
		getNextToken();

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
		getNextToken();

		return std::make_unique<ExprAST>(unOp.type, std::move(factor));

	}
	else if (curToken.type == TokenType::IntValue) {
		int value = curToken.intVal;
		getNextToken();

		return std::make_unique<ExprAST>(value);
	}
	else if (curToken.type == TokenType::Identifier) {
		std::string lexeme = curToken.lexeme;
		getNextToken();

		return std::make_unique<ExprAST>(lexeme);
	}

	return nullptr;
}

std::unique_ptr<ExprAST> Parser::parseTerm()
{
	auto left = parseFactor();

	while (curToken.type == TokenType::Multiplication || curToken.type == TokenType::Division) {
		auto opType = curToken.type;
		getNextToken();
		auto right = parseFactor();
		left = std::make_unique<ExprAST>(std::move(left), opType, std::move(right));
	}

	return std::move(left);
}
