#ifndef LEXER_H
#define LEXER_H

#include "token.h"

#include <string>
#include <vector>

class Lexer {
public:
	Lexer(std::string inputString);
	Token getNextToken();
private:
	std::string inputString;
	std::string::const_iterator itLexemeBegin;
	std::string::const_iterator itCurrent;
	int line;

	void next();
	void moveLexemeBegin();
	bool skipChar();
	Token identifier();
	Token getIntDecimalNumber();
	Token getIntBinaryNumber();
	Token getFloatNumber();
	Token getBracket();
	Token getOperator();
	Token getSemicolon();
	Token classifyKeyword(std::string::const_iterator, std::string::const_iterator);
	Token FAILED();
	bool END();
};

#endif