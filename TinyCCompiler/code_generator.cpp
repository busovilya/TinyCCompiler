#include "code_generator.h"

#include <exception>
#include <limits>

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
		"call main\n"
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
	stackIndex = -4;
	functionProtos += item.name + " PROTO\n";
	std::string functionCode = item.name + std::string(" PROC\n");

	// Prologue
	functionCode += "push ebp\n"
					"mov ebp, esp\n";

	functionCode += generateCode(*item.block);

	//int offset = 0;
	//for (auto item : varMaps[varMaps.size() - 1]) {
	//	offset = std::max(offset, -item.second);
	//}
	//functionCode += "add esp, " + std::to_string(offset) + "\n";

	// Epilogue 
	functionCode += "mov esp, ebp\n"
					"pop ebp\n";
	functionCode += "ret\n";
	functionCode += item.name + std::string(" ENDP\n");
	return functionCode;
}

std::string CodeGenerator::generateCode(BlockAST& item)
{
	varMaps.push_back(std::unordered_map<std::string, int>());
	std::string code;

	for (int i = 0; i < item.items.size(); i++) {
		code += generateCode(*(item.items[i]));
	}

	for (int i = 0; i < varMaps.size(); i++) {
		code += "pop ecx\n";
	}
	varMaps.pop_back();
	return code;
}

std::string CodeGenerator::generateCode(StatementAST& item)
{
	std::string code;
	if (item.type == StatementType::EXPRESSION_STATEMENT) {
		code = generateCode(*item.expr);
	}
	else if (item.type == StatementType::RETURN_STATEMENT) {
		code = generateCode(*item.expr);
	}
	else if (item.type == StatementType::BLOCK) {
		code += generateCode(*item.block);
	}
	else if(item.type == StatementType::CONDITION) {
		code += generateCode(*item.condition);
	}
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
	else if (item.type == ExpressionType::EXPR_VARIABLE) {
		int offset = findVariableOffset(item.varName);
		if (offset == INT_MAX) {
			throw std::runtime_error("Undeclared variable!");
		}
		std::string code = "mov ebx, [ebp" + std::to_string(offset) + "]\n";
		return code;
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
		else if (item.binary.binOp == TokenType::Equal) {
			std::string code = generateCode(*item.binary.left);
			code += "push ebx\n";
			code += generateCode(*item.binary.right);
			code += "pop eax\n";
			code += "cmp eax, ebx\n";
			code += "mov ebx, 0\n";
			code += "sete bl\n";

			return code;
		}
		else if (item.binary.binOp == TokenType::NotEqual) {
			std::string code = generateCode(*item.binary.left);
			code += "push ebx\n";
			code += generateCode(*item.binary.right);
			code += "pop eax\n";
			code += "cmp eax, ebx\n";
			code += "mov ebx, 0\n";
			code += "setne bl\n";

			return code;
		}
		else if (item.binary.binOp == TokenType::Less) {
			std::string code = generateCode(*item.binary.left);
			code += "push ebx\n";
			code += generateCode(*item.binary.right);
			code += "pop eax\n";
			code += "cmp eax, ebx\n";
			code += "mov ebx, 0\n";
			code += "setl bl\n";

			return code;
		}
		else if (item.binary.binOp == TokenType::Greater) {
			std::string code = generateCode(*item.binary.left);
			code += "push ebx\n";
			code += generateCode(*item.binary.right);
			code += "pop eax\n";
			code += "cmp eax, ebx\n";
			code += "mov ebx, 0\n";
			code += "setg bl\n";

			return code;
		}
	}
	else if (item.type == ExpressionType::EXPR_ASSIGNMENT) {
		std::string code = generateCode(*item.varAssignment.expr);
		int offset = findVariableOffset(item.varName);
		
		if (offset == INT_MAX) {
			throw std::runtime_error("Undeclared variable!");
		}
		code += "mov [ebp" + std::to_string(offset) + "], ebx\n";
		
		return code;
	}
}

std::string CodeGenerator::generateCode(DeclarationAST& item)
{
	std::string code;
	if (!item.expr) {
		if (varMaps[varMaps.size() - 1].find(item.varName) != varMaps[varMaps.size() - 1].end()) {
			throw std::runtime_error("Multiple variable declaration is prohibited!");
		}

		varMaps[varMaps.size() - 1].insert(std::make_pair(item.varName, stackIndex));
		code += "push 0\n";
		stackIndex -= 4;
	}
	else {
		if (varMaps[varMaps.size() - 1].find(item.varName) != varMaps[varMaps.size() - 1].end()) {
			throw std::runtime_error("Multiple variable declaration is prohibited!");
		}

		varMaps[varMaps.size() - 1].insert(std::make_pair(item.varName, stackIndex));
		code += generateCode(*item.expr);
		code += "push ebx\n";
		stackIndex -= 4;
	}
	return code;
}

int CodeGenerator::findVariableOffset(std::string varName)
{
	for (int i = varMaps.size() - 1; i >= 0; i--) {
		if (varMaps[i].find(varName) != varMaps[i].end()) {
			return varMaps[i][varName];
		}
	}

	return INT_MAX;
}

std::string CodeGenerator::generateCode(BlockItemAST& item)
{
	std::string code;
	if (item.type == BlockItemType::DECLARATION) {
		code += generateCode(*item.declaration);
	}
	else if (item.type == BlockItemType::STATEMENT) {
		code += generateCode(*item.statement);
	}

	return code;
}

std::string CodeGenerator::generateCode(ConditionAST& item)
{
	std::string code = generateCode(*item.expr);
	code += "cmp ebx, 0\n";
	code += item.elseClause ? "je LBL_ELSE\n" : "je LBL_POST_COND\n";
	code += generateCode(*item.ifClause);
	code += "jmp LBL_POST_COND\n";
	if (item.elseClause) {
		code += "LBL_ELSE:\n";
		code += generateCode(*item.elseClause);
	}
	code += "LBL_POST_COND:\n";

	return code;
}
