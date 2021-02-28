#ifndef PARSER_H
#define PARSER_H

#include "ast.h"
#include "token.h"
#include "error.h"

#include <vector>

class Parser {
public:
	Parser(const std::vector<Token>& _tokens) : tokens(_tokens) {};
	std::unique_ptr<ProgramAST> Parse();
	std::vector<Error*> GetErrors();
	~Parser();
private:
	int tokenNum;
	Token curToken;
	std::vector<Token> tokens;
	void getNextToken();
	std::unique_ptr<ProgramAST> parseProgram();
	std::unique_ptr<FunctionAST> parseFunction();
	std::unique_ptr<StatementAST> parseStatement();
	std::unique_ptr<ExprAST> parseExpression();
	std::vector<Error*> errors;
};

#endif