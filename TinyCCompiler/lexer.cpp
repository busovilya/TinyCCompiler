#include "lexer.h"

#include <stdexcept>

Lexer::Lexer(std::string _inputString) : inputString(_inputString) {
	itCurrent = inputString.cbegin();
	itLexemeBegin = itCurrent;
};

void Lexer::next() {
	++itCurrent;
}

void Lexer::moveLexemeBegin() {
	++itLexemeBegin;
}

bool Lexer::skipChar() {
	return *itCurrent == ' ' || *itCurrent == '\n' || *itCurrent == '\r' || *itCurrent == '\t';
}

Token Lexer::getNextToken() {
	while (itCurrent != inputString.cend()) {
		while (skipChar()) { next(); moveLexemeBegin();  }

		if (*itCurrent >= 'a' && *itCurrent <= 'z' || *itCurrent >= 'A' && *itCurrent <= 'Z') {
			 Token token = identifier();
			 Token keyword = classifyKeyword(token.begin, token.end);
			 if (keyword != FAILED()) { token = keyword; }
			 itLexemeBegin = itCurrent;
			 return token;
		}
		if (*itCurrent == '0') {
			Token floatValToken = getFloatNumber();
			if (floatValToken != FAILED()) {
				itLexemeBegin = itCurrent;
				return floatValToken;
			}

			Token intValToken = getIntBinaryNumber();
			if (intValToken != FAILED()) {
				itLexemeBegin = itCurrent;
				return intValToken;
			}
		}
		if (*itCurrent >= '1' && *itCurrent <= '9') {
			Token floatValToken = getFloatNumber();
			if (floatValToken != FAILED()) {
				itLexemeBegin = itCurrent;
				return floatValToken;
			}

			Token intValToken = getIntDecimalNumber();
			if (intValToken != FAILED()) {
				itLexemeBegin = itCurrent;
				return intValToken;
			}
		}

		Token bracket = getBracket();
		if (bracket != FAILED()) { next(); itLexemeBegin = itCurrent; return bracket; }

		Token semicolon = getSemicolon();
		if (semicolon != FAILED()) { next(); itLexemeBegin = itCurrent; return semicolon; }

		return FAILED();
	}

	return Token(TokenType::End, inputString.cend(), inputString.cend(), "\0");
}

Token Lexer::identifier() {
	next();
	while (!END() && *itCurrent >= 'a' && *itCurrent <= 'z' || *itCurrent >= 'A' && *itCurrent <= 'Z' || *itCurrent == '_' || *itCurrent >= '0' && *itCurrent <= '9') { next(); }

	std::string lexeme = std::string(itLexemeBegin, itCurrent);
	return Token(TokenType::Identifier, itLexemeBegin, itCurrent, lexeme);
}

Token Lexer::getIntDecimalNumber() {
	next();
	while (!END() && *itCurrent >= '0' && *itCurrent <= '9') { next(); }

	std::string lexeme = std::string(itLexemeBegin, itCurrent);
	return Token(TokenType::IntValue, itLexemeBegin, itCurrent, lexeme, std::stoi(lexeme));
}

Token Lexer::getIntBinaryNumber() {
	next();
	if (!END() && *itCurrent == 'b') { next(); }
	else { return FAILED(); }

	if (!END() && (*itCurrent == '0' || *itCurrent == '1')) { next(); }
	else { return FAILED(); }

	while (!END() && (*itCurrent == '0' || *itCurrent == '1')) { next(); }

	std::string lexeme = std::string(itLexemeBegin, itCurrent);
	int intVal = std::stoi(std::string(itLexemeBegin + 2, itCurrent), nullptr, 2);

	return Token(TokenType::IntValue, itLexemeBegin, itCurrent, lexeme, intVal);
}

Token Lexer::getFloatNumber()
{
	bool leadingZero = (*itCurrent == '0');
	next();

	if (!END() && *itCurrent == '0' && leadingZero) { return FAILED(); }
	while (!END() && (*itCurrent >= '0' && *itCurrent <= '9' || *itCurrent == '.')) { next(); }

	std::string lexeme = std::string(itLexemeBegin, itCurrent);
	float floatVal = std::stof(lexeme);
	return Token(TokenType::FloatValue, itLexemeBegin, itCurrent, lexeme, floatVal);
}

Token Lexer::classifyKeyword(std::string::const_iterator begin, std::string::const_iterator end) {
	std::string str(begin, end);

	if (str == "int") { return Token(TokenType::IntType, begin, end, str); }
	if (str == "float") { return Token(TokenType::FloatType, begin, end, str); }
	if (str == "return") { return Token(TokenType::ReturnKeyword, begin, end, str); }

	return FAILED();
}

Token Lexer::getBracket() {
	switch (*itCurrent) {
	case '(':
		return Token(TokenType::OpenParenthese, itLexemeBegin + 1, itCurrent, "(");
	case ')':
		return Token(TokenType::CloseParenthese , itLexemeBegin + 1, itCurrent, ")");
	case '{':
		return Token(TokenType::OpenBrace, itLexemeBegin, itCurrent + 1, "{");
	case '}':
		return Token(TokenType::CloseBrace, itLexemeBegin, itCurrent + 1, "}");
	default:
		return FAILED();
	}
}

Token Lexer::getOperator() {
	return FAILED();
}

Token Lexer::getSemicolon()
{
	if (*itCurrent == ';') { return Token(TokenType::Semicolon, itLexemeBegin, itCurrent + 1, ";"); }

	return FAILED();
}

Token Lexer::FAILED() {
	return Token(TokenType::FAILED, itLexemeBegin, itCurrent, std::string(itLexemeBegin, itCurrent));
}

bool Lexer::END() {
	return itCurrent == inputString.cend();
}