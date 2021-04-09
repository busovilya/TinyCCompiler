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
	curToken = tokens[tokenNum++];
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

	getNextToken();
	auto statement = parseStatement();
	if (statement == nullptr) { return nullptr; };

	getNextToken();
	if (curToken.type != TokenType::CloseBrace) { 
		errors.push_back(CompilerError::errorAtLine("Expected '}'!", curToken));
		return nullptr; 
	}

	return std::make_unique<FunctionAST>(name, std::move(statement));
}

std::unique_ptr<StatementAST> Parser::parseStatement() {
	if (curToken.type != TokenType::ReturnKeyword) { 
		errors.push_back(CompilerError::errorAtLine("Expected 'return'!", curToken));
		return nullptr; 
	}

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

	return std::make_unique<StatementAST>(std::move(expr));
}

std::unique_ptr<ExprAST> Parser::parseExpression() {
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
