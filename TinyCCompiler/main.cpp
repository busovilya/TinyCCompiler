#include "token.h"
#include "lexer.h"
#include "parser.h"

#include <fstream>
#include <stdio.h>
#include <string>

int main() {
	std::ifstream input("code.c", std::ifstream::binary);
	std::string inputString((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());

	Lexer lexer(inputString);
	Token token;
	std::vector<Token> tokens;
	while ((token = lexer.getNextToken()).type != TokenType::End) {
		std::cout << token << std::endl;
		tokens.push_back(token);
	};

	Parser parser(tokens);
	auto ast = parser.Parse();
	return 0;
}