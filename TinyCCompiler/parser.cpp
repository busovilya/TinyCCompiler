#include "parser.h"

std::unique_ptr<ProgramAST> Parser::Parse()
{
	return parseProgram();
}

void Parser::getNextToken()
{
	curToken = tokens[tokenNum++];
}

std::unique_ptr<ProgramAST> Parser::parseProgram() {
	getNextToken();
	return std::make_unique<ProgramAST>(parseFunction());
}

std::unique_ptr<FunctionAST> Parser::parseFunction() {
	if (curToken.type != TokenType::IntType) { return nullptr; }

	getNextToken();
	if (curToken.type != TokenType::Identifier) { return nullptr; }
	auto name = curToken.lexeme;

	getNextToken();
	if (curToken.type != TokenType::OpenParenthese) { return nullptr; }

	getNextToken();
	if (curToken.type != TokenType::CloseParenthese) { return nullptr; }

	getNextToken();
	if (curToken.type != TokenType::OpenBrace) { return nullptr; }

	getNextToken();
	auto statement = parseStatement();

	getNextToken();
	if (curToken.type != TokenType::CloseBrace) { return nullptr; }

	return std::make_unique<FunctionAST>(name, std::move(statement));
}

std::unique_ptr<StatementAST> Parser::parseStatement() {
	if (curToken.type != TokenType::ReturnKeyword) { return nullptr; }

	getNextToken();
	auto expr = parseExpression();

	getNextToken();
	if (curToken.type != TokenType::Semicolon) { return nullptr; }

	return std::make_unique<StatementAST>(std::move(expr));
}

std::unique_ptr<ExprAST> Parser::parseExpression() {
	if (curToken.type == TokenType::IntValue) {
		return std::make_unique<IntValAST>(curToken.intVal);
	}
	if (curToken.type == TokenType::FloatValue) {
		return std::make_unique<FloatValAST>(curToken.floatVal);
	}

	return nullptr;
}