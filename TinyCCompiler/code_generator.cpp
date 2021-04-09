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
		"add     ecx, ebx\n\n"
		"test eax, eax\n"
		"js NEGATIVE_NUM\n\n"
		"@@:\n"
		"xor edx, edx\n"
		"div     ebx\n"
		"add     edx, 48\n"
		"mov     BYTE PTR[ecx], dl\n"
		"dec     ecx\n"
		"test    eax, eax\n"
		"jnz     @b\n"
		"jmp LBL\n\n"
		"NEGATIVE_NUM :\n"
		"neg eax\n"
		"@@:\n"
		"xor edx, edx\n"
		"div     ebx\n"
		"add     edx, 48\n"
		"mov     BYTE PTR[ecx], dl\n"
		"dec     ecx\n"
		"test    eax, eax\n"
		"jnz     @b\n"
		"mov BYTE PTR[ecx], '-'\n"
		"dec ecx\n\n"
		"LBL :\n"
		"inc     ecx\n"
		"mov     eax, ecx\n"
		"ret\n"
		"NumbToStr ENDP\n"
		"end start\n";

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
	std::string code = generateCode(*item.expr);
	code += "ret\n";

	return code;
}

std::string CodeGenerator::generateCode(ExprAST& item) {
	if (item.type == ExpressionType::EXPR_INT) {
		return "mov ebx, " + std::to_string(item.intVal) + "\n";
	}
	else if (item.type == ExpressionType::EXPR_UNARY) {
		if (item.unary.unOp == TokenType::Negation) {
			std::string code = generateCode(*item.unary.expr);
			code += "neg ebx\n";
			return code;
		}
		else if (item.unary.unOp == TokenType::BitwiseComplement) {
			std::string code = generateCode(*item.unary.expr);
			code += "not ebx\n";
			return code;
		}
		else if (item.unary.unOp == TokenType::LogicalNegation) {
			std::string code = generateCode(*item.unary.expr);
			code += "cmp ebx, 0\n"
				"mov ebx, 0\n"
				"sete bl\n";
			return code;
		}
	}
	else if (item.type == ExpressionType::EXPR_BINARY) {
		if (item.binary.binOp == TokenType::Addition) {
			std::string code = generateCode(*item.binary.left);
			code += "push ebx\n";
			code += generateCode(*item.binary.right);
			code += "pop ecx\n";
			code += "add ebx, ecx\n";

			return code;
		}
		else if (item.binary.binOp == TokenType::Multiplication) {
			std::string code = generateCode(*item.binary.left);
			code += "push ebx\n";
			code += generateCode(*item.binary.right);
			code += "pop eax\n";
			code += "mul ebx\n";
			code += "mov ebx, eax\n";

			return code;
		}
		else if (item.binary.binOp == TokenType::Negation) {
			std::string code = generateCode(*item.binary.left);
			code += "push ebx\n";
			code += generateCode(*item.binary.right);
			code += "pop ecx\n";
			code += "sub ecx, ebx\n";
			code += "mov ebx, ecx\n";

			return code;
		}
		else if (item.binary.binOp == TokenType::Division) {
			std::string code = generateCode(*item.binary.left);
			code += "push ebx\n";
			code += generateCode(*item.binary.right);
			code += "pop eax\n";
			code += "mov dx, 0\n";
			code += "div bx\n";
			code += "mov ebx, eax\n";

			return code;
		}
		else if (item.binary.binOp == TokenType::LogicalAnd) {
			std::string code = generateCode(*item.binary.left);
			code += "push ebx\n";
			code += generateCode(*item.binary.right);
			code += "pop ecx\n";
			code += "and ebx, ecx\n";

			return code;
		}
		else if (item.binary.binOp == TokenType::LogicalOr) {
			std::string code = generateCode(*item.binary.left);
			code += "push ebx\n";
			code += generateCode(*item.binary.right);
			code += "pop ecx\n";
			code += "or ebx, ecx\n";

			return code;
		}
	}
}
