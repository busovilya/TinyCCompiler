#include "token.h"
#include "lexer.h"
#include "parser.h"
#include "code_generator.h"

#include <fstream>
#include <stdio.h>
#include <string>
#include <exception>

int main(int argc, char* argv[]) {
	std::string filename = "code.c";
	std::string outputFile = "code.asm";

	if (argc > 1) {
		filename = argv[1];

		if (argc == 4) {
			if (std::strcmp(argv[2], "-o") == 0) {
				outputFile = argv[3];
			}
		}
	}

	std::ifstream input(filename, std::ifstream::binary);
	std::string inputString((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());

	Lexer lexer(inputString);
	Token token;
	std::vector<Token> tokens;
	while ((token = lexer.getNextToken()).type != TokenType::End) {
		std::cout << token << std::endl;
		tokens.push_back(token);
	};

	std::cout << std::endl;

	Parser parser(tokens);
	auto ast = parser.Parse();
	if (ast) {
		CodeGenerator codeGen;
		std::ofstream out(outputFile);
		if (!out.is_open()) {
			std::cout << "Wrong output filename!" << std::endl;
			return -1;
		}

		try {
			out << codeGen.generateCode(*ast);
		}
		catch (std::runtime_error err) {
			std::cout << err.what() << std::endl;
		}
		out.close();
	}
	else {
		auto errors = parser.GetErrors();
		for (auto error : errors) {
			std::cout << error->getMessage() << std::endl;
		}
	}

	input.close();

	return 0;
}