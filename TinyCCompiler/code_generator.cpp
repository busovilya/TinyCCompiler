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
	functionProtos = "NumbToStr   PROTO :DWORD,:DWORD\n";
	dataSection = "buff        db 11 dup(?)\n";
	codeSection = "";
}


std::string CodeGenerator::generateCode(ProgramAST& item)
{
	std::string code;
	code += header;
	
	std::string progCode = std::string(
		"start:\n"
		"invoke main\n"
		"invoke  NumbToStr, ebx, ADDR buff\n"
		"invoke  StdOut, eax\n"
		"invoke ExitProcess, 0\n"
	);

	std::string funcCode = generateCode(*item.function);

	code += functionProtos;
	code += ".data\n";
	code += dataSection;
	code += ".code\n";
	code += progCode + '\n';
	code += funcCode + '\n';
	code += "NumbToStr PROC uses ebx x:DWORD,buffer:DWORD\n"
		"mov     ecx, buffer\n"
		"mov     eax, x\n"
		"mov     ebx, 10\n"
		"add     ecx, ebx\n"
		"@@:\n"
		"xor edx, edx\n"
		"div     ebx\n"
		"add     edx, 48\n"
		"mov     BYTE PTR[ecx], dl\n"
		"dec     ecx\n"
		"test    eax, eax\n"
		"jnz     @b\n"
		"inc     ecx\n"
		"mov     eax, ecx\n"
		"ret\n"
		"NumbToStr ENDP\n";
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
	std::string code = std::string("mov ebx, " + generateCode(*item.expr) + '\n');
	code += "ret\n";

	return code;
}

std::string CodeGenerator::generateCode(ExprAST& item) {
	return std::to_string(((IntValAST*)&item)->val);
}
