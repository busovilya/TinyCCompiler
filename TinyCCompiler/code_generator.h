#ifndef CODE_GENERATOR_H
#define CODE_GENERATOR_H

#include <string>
#include <sstream>

#include "ast.h"

class CodeGenerator {
public:
	CodeGenerator();
	std::string generateCode(ProgramAST& item);
	std::string generateCode(FunctionAST& item);
	std::string generateCode(StatementAST& item);
	std::string generateCode(ExprAST& item);
private:
	std::string header;
	std::string functionProtos;
	std::string dataSection;
	std::string codeSection;
};

#endif // !CODE_GENERATOR_H
