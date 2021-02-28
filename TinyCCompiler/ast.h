#ifndef AST_H
#define AST_H

#include <memory>
#include <string>

struct ExprAST {
	virtual ~ExprAST() = default;
};

struct IntValAST : ExprAST {
	int val;
	IntValAST(int _val) : val(_val) {};
	~IntValAST() {};
};

struct FloatValAST : ExprAST {
	float val;
	FloatValAST(float _val) : val(_val) {};
	~FloatValAST() {};
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