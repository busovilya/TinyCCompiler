#ifndef AST_H
#define AST_H

#include <memory>
#include <string>

class ExprAST {
public:
	virtual ~ExprAST() = default;
};

class IntValAST : public ExprAST {
private:
	int val;
public:
	IntValAST(int _val) : val(_val) {};
	~IntValAST() {};
};

class FloatValAST : public ExprAST {
private:
	float val;
public:
	FloatValAST(float _val) : val(_val) {};
	~FloatValAST() {};
};

class StatementAST {
private:
	std::unique_ptr<ExprAST> expr;
public:
	StatementAST(std::unique_ptr<ExprAST> _expr) : expr(std::move(_expr)) {};
};

class FunctionAST  {
private:
	std::string name;
	std::unique_ptr<StatementAST> statement;
public:
	FunctionAST(std::string _name, std::unique_ptr<StatementAST> _statement) : name(_name), statement(std::move(_statement)) {};
};

class ProgramAST
{
private:
	std::unique_ptr<FunctionAST> function;
public:
	ProgramAST(std::unique_ptr<FunctionAST> _func) : function(std::move(_func)) {};
};

#endif