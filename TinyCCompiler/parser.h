#ifndef PARSER_H
#define PARSER_H

#include "ast.h"
#include "token.h"
#include "error.h"
#include "algorithm.h"

#include <vector>
#include <set>

const std::set<TokenType> EXPRESSION_FIRST = { Identifier, OpenParenthese, IntValue, Negation, BitwiseComplement, LogicalNegation };
const std::set<TokenType> STATEMENT_FIRST = setUnion(EXPRESSION_FIRST, { OpenBrace, ReturnKeyword, IfOperator });
const std::set<TokenType> BLOCK_ITEM_FIRST = setUnion(STATEMENT_FIRST, { IntType });

class Parser {
public:
	Parser(const std::vector<Token>& _tokens) : tokens(_tokens), tokenNum(-1) {};
	std::unique_ptr<ProgramAST> Parse();
	std::vector<Error*> GetErrors();
	~Parser();
private:
	int tokenNum;
	Token curToken;
	std::vector<Token> tokens;
	void getNextToken();
	void getPrevToken();
	std::unique_ptr<ProgramAST> parseProgram();
	std::unique_ptr<FunctionAST> parseFunction();
	std::unique_ptr<BlockAST> parseBlock();
	std::unique_ptr<BlockItemAST> parseBlockItem();
	std::unique_ptr<DeclarationAST> parseDeclaration();
	std::unique_ptr<ConditionAST> parseCondition();
	std::unique_ptr<StatementAST> parseStatement();
	std::unique_ptr<ExprAST> parseExpression();
	std::unique_ptr<ExprAST> parseLogicalOrExpression();
	std::unique_ptr<ExprAST> parseLogicalAndExpression();
	std::unique_ptr<ExprAST> parseEqualityExpression();
	std::unique_ptr<ExprAST> parseComparasionExpression();
	std::unique_ptr<ExprAST> parseAdditiveExpression();
	std::unique_ptr<ExprAST> parseFactor();
	std::unique_ptr<ExprAST> parseTerm();
	std::vector<Error*> errors;
};

#endif