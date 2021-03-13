#ifndef AST_H
#define AST_H

#include <memory>
#include <string>

#include "token.h"

enum ExpressionType {
	EXPR_INT,
	EXPR_UNARY,
	EXPR_BINARY
};

struct ExprAST {
	ExpressionType type;

	union {
		struct {
			TokenType unOp;
			std::unique_ptr<ExprAST> expr;
		} unary;

		struct {
			std::unique_ptr<ExprAST> left;
			TokenType binOp;
			std::unique_ptr<ExprAST> right;
		} binary;

		int32_t intVal;
	};

	ExprAST(TokenType op, std::unique_ptr<ExprAST> _expr) : type(EXPR_UNARY), unary{ op, std::move(_expr) } {};
	ExprAST(std::unique_ptr<ExprAST> _left, TokenType op, std::unique_ptr<ExprAST> _right) : type(EXPR_BINARY), binary{ std::move(_left), op, std::move(_right) } {};
	ExprAST(int32_t _val) : type(EXPR_INT), intVal(_val) {};
	~ExprAST() {};
};

struct StatementAST {
	std::unique_ptr<ExprAST> expr;
	StatementAST(std::unique_ptr<ExprAST> _expr) : expr(std::move(_expr)) {};
};

struct FunctionAST  {
	std::string name;
	std::unique_ptr<StatementAST> statement;
	FunctionAST(std::string _name, std::unique_ptr<StatementAST> _statement) : name(_name), statement(std::move(_statement)) {};
};

struct ProgramAST
{
	std::unique_ptr<FunctionAST> function;
	ProgramAST(std::unique_ptr<FunctionAST> _func) : function(std::move(_func)) {};
};

#endif