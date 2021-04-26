#ifndef CODE_GENERATOR_H
#define CODE_GENERATOR_H

#include <string>
#include <sstream>
#include <unordered_map>
#include <vector>

#include "ast.h"

class CodeGenerator {
public:
	CodeGenerator();
	std::string generateCode(ProgramAST& item);
	std::string generateCode(FunctionAST& item);
	std::string generateCode(BlockAST& item);
	std::string generateCode(BlockItemAST& item);
	std::string generateCode(ConditionAST& item);
	std::string generateCode(StatementAST& item);
	std::string generateCode(ExprAST& item);
	std::string generateCode(DeclarationAST& item);
private:
	std::string header;
	std::string functionProtos;
	std::string dataSection;
	std::string codeSection;

	int stackIndex;
	std::vector<std::unordered_map<std::string, int>> varMaps;

	int findVariableOffset(std::string varName);
};

#endif // !CODE_GENERATOR_H
