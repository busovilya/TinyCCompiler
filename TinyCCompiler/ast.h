#ifndef AST_H
#define AST_H

#include <memory>
#include <string>
#include <vector>

#include "token.h"

struct StatementAST;
struct BlockItemAST;
struct BlockAST;

enum class ExpressionType {
	EXPR_INT,
	EXPR_UNARY,
	EXPR_BINARY,
	EXPR_ASSIGNMENT,
	EXPR_VARIABLE
};

enum class StatementType {
	RETURN_STATEMENT,
	EXPRESSION_STATEMENT,
	BLOCK,
	CONDITION
};

enum class BlockItemType {
	DECLARATION,
	STATEMENT
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

	ExprAST(TokenType op, std::unique_ptr<ExprAST> _expr) : type(ExpressionType::EXPR_UNARY), unary{ op, std::move(_expr) } {};
	ExprAST(std::unique_ptr<ExprAST> _left, TokenType op, std::unique_ptr<ExprAST> _right) : type(ExpressionType::EXPR_BINARY), binary{ std::move(_left), op, std::move(_right) } {};
	ExprAST(std::string _name, std::unique_ptr<ExprAST> _expr) : type(ExpressionType::EXPR_ASSIGNMENT), varAssignment{ _name, std::move(_expr) } {};
	ExprAST(int32_t _val) : type(ExpressionType::EXPR_INT), intVal(_val) {};
	ExprAST(std::string name) : type(ExpressionType::EXPR_VARIABLE), varName(name) {};
	~ExprAST() {};
};

struct ConditionAST {
	std::unique_ptr<ExprAST> expr;
	std::unique_ptr<BlockAST> ifClause, elseClause;

	ConditionAST(std::unique_ptr<ExprAST> _expr, std::unique_ptr<BlockAST> _if, std::unique_ptr<BlockAST> _else) : expr(std::move(_expr)), ifClause(std::move(_if)), elseClause(std::move(_else)) {};
};

struct DeclarationAST {
	std::string varName;
	std::unique_ptr<ExprAST> expr;

	DeclarationAST(std::string _varName) : varName(_varName), expr(nullptr) {};
	DeclarationAST(std::string _varName, std::unique_ptr<ExprAST> _expr) : varName(_varName), expr(std::move(_expr)) {};
};

struct BlockItemAST {
	BlockItemType type;

	std::unique_ptr<DeclarationAST> declaration;
	std::unique_ptr<StatementAST> statement;

	BlockItemAST(std::unique_ptr<DeclarationAST> decl) : type(BlockItemType::DECLARATION), declaration(std::move(decl)) {};
	BlockItemAST(std::unique_ptr<StatementAST> _statement) : type(BlockItemType::STATEMENT), statement(std::move(_statement)) {};
	~BlockItemAST() {
		if (type == BlockItemType::DECLARATION) {
			declaration.reset();
		}
		if (type == BlockItemType::STATEMENT) {
			statement.reset();
		}
	};
};

struct StatementAST {
	StatementType type;

	union {
		std::unique_ptr<ExprAST> expr;
		std::unique_ptr<BlockAST> block;
		std::unique_ptr<ConditionAST> condition;
	};

	StatementAST(StatementType _type, std::unique_ptr<ExprAST> _expr) : type(_type), expr(std::move(_expr)) {};
	StatementAST(std::unique_ptr<BlockAST> _block) : type(StatementType::BLOCK), block(std::move(_block)) {};
	StatementAST(std::unique_ptr<ConditionAST> _cond) : type(StatementType::CONDITION), condition(std::move(_cond)) {};

	~StatementAST() {
		if (type == StatementType::BLOCK) {
			block.reset();
		}
		else if (type == StatementType::EXPRESSION_STATEMENT || type == StatementType::RETURN_STATEMENT) {
			expr.reset();
		}
	}
};

struct BlockAST {
	std::vector<std::unique_ptr<BlockItemAST>> items;

	BlockAST(std::vector<std::unique_ptr<BlockItemAST>>& _items) {
		for (int i = 0; i < _items.size(); i++) {
			items.push_back(std::move(_items[i]));
		}
	}
};

struct FunctionAST  {
	std::string name;
	std::unique_ptr<BlockAST> block;

	FunctionAST(std::string _name, std::unique_ptr<BlockAST> _block) : name(_name), block(std::move(_block)) {};
};

struct ProgramAST
{
	std::unique_ptr<FunctionAST> function;
	ProgramAST(std::unique_ptr<FunctionAST> _func) : function(std::move(_func)) {};
};

#endif