#ifndef AST_H
#define AST_H

#include <memory>
#include <string>
#include <vector>

#include "token.h"

enum ExpressionType {
	EXPR_INT,
	EXPR_UNARY,
	EXPR_BINARY,
	EXPR_ASSIGNMENT,
	EXPR_VARIABLE
};

enum StatementType {
	RETURN_STATEMENT,
	EXPRESSION_STATEMENT,
	VARIABLE_DECLARATION,
	VARIABLE_DECLARATION_WITH_ASSIGNMENT,
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

		struct {
			std::string varName;
			std::unique_ptr<ExprAST> expr;
		} varAssignment;

		int32_t intVal;
		std::string varName;
	};

	ExprAST(TokenType op, std::unique_ptr<ExprAST> _expr) : type(EXPR_UNARY), unary{ op, std::move(_expr) } {};
	ExprAST(std::unique_ptr<ExprAST> _left, TokenType op, std::unique_ptr<ExprAST> _right) : type(EXPR_BINARY), binary{ std::move(_left), op, std::move(_right) } {};
	ExprAST(std::string _name, std::unique_ptr<ExprAST> _expr) : type(EXPR_ASSIGNMENT), varAssignment{ _name, std::move(_expr) } {};
	ExprAST(int32_t _val) : type(EXPR_INT), intVal(_val) {};
	ExprAST(std::string name) : type(EXPR_VARIABLE), varName(name) {};
	~ExprAST() {};
};

struct StatementAST {
	std::unique_ptr<ExprAST> expr;
	std::string varName;
	StatementType type;
	StatementAST(StatementType _type, std::unique_ptr<ExprAST> _expr) : type(_type), expr(std::move(_expr)) {};
	StatementAST(StatementType _type, std::string _varName) : type(_type), varName(_varName) {};
	StatementAST(StatementType _type, std::string _varName, std::unique_ptr<ExprAST> _expr) : type(_type), varName(_varName), expr(std::move(_expr)) {};
};

struct FunctionAST  {
	std::string name;
	std::vector<std::unique_ptr<StatementAST>> statements;
	FunctionAST(std::string _name, std::vector<std::unique_ptr<StatementAST>>& _statements) : name(_name) {
		for (int i = 0; i < _statements.size(); i++) {
			statements.push_back(std::move(_statements[i]));
		}
	}
};

struct ProgramAST
{
	std::unique_ptr<FunctionAST> function;
	ProgramAST(std::unique_ptr<FunctionAST> _func) : function(std::move(_func)) {};
};

#endif