#include "lexer.h"

#include <stdexcept>

Lexer::Lexer(std::string _inputString) : inputString(_inputString) {
	itCurrent = inputString.cbegin();
	itLexemeBegin = itCurrent;
	line = 0;
};

void Lexer::next() {
	if (*itCurrent == '\n') {
		line++;
	}
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
		while (!END() && skipChar()) { next(); moveLexemeBegin();  }
		if (END()) { break; }

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

			itCurrent = itLexemeBegin;
			Token intValToken = getIntBinaryNumber();
			if (intValToken != FAILED()) {
				itLexemeBegin = itCurrent;
				return intValToken;
			}

			itCurrent = itLexemeBegin;
			intValToken = getIntDecimalNumber();
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

			itCurrent = itLexemeBegin;
			Token intValToken = getIntDecimalNumber();
			if (intValToken != FAILED()) {
				itLexemeBegin = itCurrent;
				return intValToken;
			}
		}

		itCurrent = itLexemeBegin;
		Token bracket = getBracket();
		if (bracket != FAILED()) { next(); itLexemeBegin = itCurrent; return bracket; }

		itCurrent = itLexemeBegin;
		Token semicolon = getSemicolon();
		if (semicolon != FAILED()) { next(); itLexemeBegin = itCurrent; return semicolon; }

		return FAILED();
	}

	return Token(TokenType::End, inputString.cend(), inputString.cend(), "\0", inputString.cend() - inputString.begin(), line);
}

Token Lexer::identifier() {
	next();
	while (!END() && *itCurrent >= 'a' && *itCurrent <= 'z' || *itCurrent >= 'A' && *itCurrent <= 'Z' || *itCurrent == '_' || *itCurrent >= '0' && *itCurrent <= '9') { next(); }

	std::string lexeme = std::string(itLexemeBegin, itCurrent);
	return Token(TokenType::Identifier, itLexemeBegin, itCurrent, lexeme, itLexemeBegin - inputString.begin(), line);
}

Token Lexer::getIntDecimalNumber() {
	next();
	while (!END() && *itCurrent >= '0' && *itCurrent <= '9') { next(); }

	std::string lexeme = std::string(itLexemeBegin, itCurrent);
	return Token(TokenType::IntValue, itLexemeBegin, itCurrent, lexeme, std::stoi(lexeme), itLexemeBegin - inputString.begin(), line);
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

	return Token(TokenType::IntValue, itLexemeBegin, itCurrent, lexeme, intVal, itLexemeBegin - inputString.begin(), line);
}

Token Lexer::getFloatNumber()
{
	bool dot = false;
	bool leadingZero = (*itCurrent == '0');
	next();

	if (!END() && *itCurrent == '0' && leadingZero) { return FAILED(); }
	while (!END() && (*itCurrent >= '0' && *itCurrent <= '9' || *itCurrent == '.')) { 
		if (*itCurrent == '.') { dot = true; }
		next(); 
	}

	if (!dot) { return FAILED(); }
	std::string lexeme = std::string(itLexemeBegin, itCurrent);
	float floatVal = std::stof(lexeme);
	return Token(TokenType::FloatValue, itLexemeBegin, itCurrent, lexeme, floatVal, itLexemeBegin - inputString.begin(), line);
}

Token Lexer::classifyKeyword(std::string::const_iterator begin, std::string::const_iterator end) {
	std::string str(begin, end);

	if (str == "int") { return Token(TokenType::IntType, begin, end, str, begin - inputString.begin(), line); }
	if (str == "float") { return Token(TokenType::FloatType, begin, end, str, begin - inputString.begin(), line); }
	if (str == "return") { return Token(TokenType::ReturnKeyword, begin, end, str, begin - inputString.begin(), line); }

	return FAILED();
}

Token Lexer::getBracket() {
	switch (*itCurrent) {
	case '(':
		return Token(TokenType::OpenParenthese, itLexemeBegin + 1, itCurrent, "(", itLexemeBegin + 1 - inputString.begin(), line);
	case ')':
		return Token(TokenType::CloseParenthese , itLexemeBegin + 1, itCurrent, ")", itLexemeBegin + 1 - inputString.begin(), line);
	case '{':
		return Token(TokenType::OpenBrace, itLexemeBegin, itCurrent + 1, "{", itLexemeBegin + 1 - inputString.begin(), line);
	case '}':
		return Token(TokenType::CloseBrace, itLexemeBegin, itCurrent + 1, "}", itLexemeBegin + 1 - inputString.begin(), line);
	default:
		return FAILED();
	}
}

Token Lexer::getOperator() {
	return FAILED();
}

Token Lexer::getSemicolon()
{
	if (*itCurrent == ';') { return Token(TokenType::Semicolon, itLexemeBegin, itCurrent + 1, ";", itLexemeBegin - inputString.begin(), line); }

	return FAILED();
}

Token Lexer::FAILED() {
	return Token(TokenType::FAILED, itLexemeBegin, itCurrent, std::string(itLexemeBegin, itCurrent), itLexemeBegin - inputString.begin(), line);
}

bool Lexer::END() {
	return itCurrent == inputString.cend();
}