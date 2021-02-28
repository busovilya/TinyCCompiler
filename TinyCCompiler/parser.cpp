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

std::unique_ptr<ProgramAST> Parser::parseProgram() {
	getNextToken();
	auto func = parseFunction();
	if (!func) { return nullptr; }
	return std::make_unique<ProgramAST>(std::move(func));
}

std::unique_ptr<FunctionAST> Parser::parseFunction() {
	if (curToken.type != TokenType::IntType) { 
		errors.push_back(CompilerError::errorAtLine("Function must have INT type.", curToken.line));
		return nullptr; 
	}

	getNextToken();
	if (curToken.type != TokenType::Identifier) { 
		errors.push_back(CompilerError::errorAtLine("Function definition must have identifier.", curToken.line));
		return nullptr; 
	}
	auto name = curToken.lexeme;

	getNextToken();
	if (curToken.type != TokenType::OpenParenthese) { 
		errors.push_back(CompilerError::errorAtLine("Wrong function definition!", curToken.line));
		return nullptr; 
	}

	getNextToken();
	if (curToken.type != TokenType::CloseParenthese) { 
		errors.push_back(CompilerError::errorAtLine("Wrong function definition!", curToken.line));
		return nullptr; 
	}

	getNextToken();
	if (curToken.type != TokenType::OpenBrace) {
		errors.push_back(CompilerError::errorAtLine("Statement must be surrounded by braces", curToken.line));
		return nullptr;
	}

	getNextToken();
	auto statement = parseStatement();
	if (statement == nullptr) { return nullptr; };

	getNextToken();
	if (curToken.type != TokenType::CloseBrace) { 
		errors.push_back(CompilerError::errorAtLine("Statement must be surrounded by braces", curToken.line));
		return nullptr; 
	}

	return std::make_unique<FunctionAST>(name, std::move(statement));
}

std::unique_ptr<StatementAST> Parser::parseStatement() {
	if (curToken.type != TokenType::ReturnKeyword) { 
		errors.push_back(CompilerError::errorAtLine("Missed return keyword!", curToken.line));
		return nullptr; 
	}

	getNextToken();
	auto expr = parseExpression();
	if (!expr) { return nullptr; }

	getNextToken();
	if (curToken.type != TokenType::Semicolon) { 
		errors.push_back(CompilerError::errorAtLine("Semicolon is missed!", curToken.line));
		return nullptr; 
	}

	return std::make_unique<StatementAST>(std::move(expr));
}

std::unique_ptr<ExprAST> Parser::parseExpression() {
	if (curToken.type == TokenType::IntValue) {
		return std::make_unique<IntValAST>(curToken.intVal);
	}

	if (curToken.type == TokenType::FloatValue) {
		return std::make_unique<IntValAST>(curToken.floatVal);
	}

	errors.push_back(CompilerError::errorAtLine("Compiler supports only integers and floats!", curToken.line));

	return nullptr;
}