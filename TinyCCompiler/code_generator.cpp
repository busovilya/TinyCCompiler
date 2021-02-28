#include "code_generator.h"

CodeGenerator::CodeGenerator() {
	header = std::string(".386\n"
		".model flat, stdcall\n"
		"option casemap : none\n"
		"include masm32\\include\\windows.inc\n"
		"include masm32\\include\\kernel32.inc\n"
		"include masm32\\include\\masm32.inc\n"
		"includelib masm32\\lib\\kernel32.lib\n"
		"includelib masm32\\lib\\masm32.lib\n\n");
	functionProtos = "";
	dataSection = "";
	codeSection = "";
}


std::string CodeGenerator::generateCode(ProgramAST& item)
{
	std::string code;
	code += header;
	
	std::string progCode = std::string(
		"start:\n"
		"invoke main\n"
		"invoke ExitProcess, 0\n"
	);

	std::string funcCode = generateCode(*item.function);

	code += functionProtos;
	code += ".code\n";
	code += progCode + '\n';
	code += funcCode + '\n';
	code += "end start";

	return code;
}

std::string CodeGenerator::generateCode(FunctionAST& item)
{
	functionProtos += item.name + " PROTO\n";
	std::string functionCode = item.name + std::string(" PROC\n");
	functionCode += generateCode(*item.statement);
	functionCode += item.name + std::string(" ENDP\n");

	return functionCode;
}

std::string CodeGenerator::generateCode(StatementAST& item)
{
	std::string code = std::string("mov eax, " + generateCode(*item.expr) + '\n');
	code += "ret\n";

	return code;
}

std::string CodeGenerator::generateCode(ExprAST& item) {
	return std::to_string(((IntValAST*)&item)->val);
}
