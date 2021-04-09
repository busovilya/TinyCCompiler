#include "lexer.h"

#include <stdexcept>

Lexer::Lexer(std::string _inputString) : inputString(_inputString) {
	itCurrent = inputString.cbegin();
	itLexemeBegin = itCurrent;
	itStartOfLine = itLexemeBegin;
	line = 0;
};

void Lexer::next() {
	if (*itCurrent == '\n') {
		line++;
		itStartOfLine = itCurrent + 1;
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

		itCurrent = itLexemeBegin;
		Token unaryOperator = getUnaryOperator();
		if (unaryOperator != FAILED()) { next(); itLexemeBegin = itCurrent; return unaryOperator; }

		itCurrent = itLexemeBegin;
		Token binaryOperator = getBinaryOperator();
		if (binaryOperator != FAILED()) { next(); itLexemeBegin = itCurrent; return binaryOperator; }

		itCurrent = itLexemeBegin;
		Token assignment = getAssignment();
		if (assignment != FAILED()) { next(); itLexemeBegin = itCurrent; return assignment; }

		return FAILED();
	}

	return Token(TokenType::End, inputString.cend(), inputString.cend(), "\0", inputString.cend() - itStartOfLine, line);
}

Token Lexer::identifier() {
	next();
	while (!END() && *itCurrent >= 'a' && *itCurrent <= 'z' || *itCurrent >= 'A' && *itCurrent <= 'Z' || *itCurrent == '_' || *itCurrent >= '0' && *itCurrent <= '9') { next(); }

	std::string lexeme = std::string(itLexemeBegin, itCurrent);
	return Token(TokenType::Identifier, itLexemeBegin, itCurrent, lexeme, itLexemeBegin - itStartOfLine, line);
}

Token Lexer::getIntDecimalNumber() {
	next();
	while (!END() && *itCurrent >= '0' && *itCurrent <= '9') { next(); }

	std::string lexeme = std::string(itLexemeBegin, itCurrent);
	return Token(TokenType::IntValue, itLexemeBegin, itCurrent, lexeme, std::stoi(lexeme), itLexemeBegin - itStartOfLine, line);
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

	return Token(TokenType::IntValue, itLexemeBegin, itCurrent, lexeme, intVal, itLexemeBegin - itStartOfLine, line);
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
	return Token(TokenType::FloatValue, itLexemeBegin, itCurrent, lexeme, floatVal, itLexemeBegin - itStartOfLine, line);
}

Token Lexer::classifyKeyword(std::string::const_iterator begin, std::string::const_iterator end) {
	std::string str(begin, end);

	if (str == "int") { return Token(TokenType::IntType, begin, end, str, begin - itStartOfLine, line); }
	if (str == "float") { return Token(TokenType::FloatType, begin, end, str, begin - itStartOfLine, line); }
	if (str == "return") { return Token(TokenType::ReturnKeyword, begin, end, str, begin - itStartOfLine, line); }

	return FAILED();
}

Token Lexer::getBracket() {
	switch (*itCurrent) {
	case '(':
		return Token(TokenType::OpenParenthese, itLexemeBegin + 1, itCurrent, "(", itLexemeBegin + 1 - itStartOfLine, line);
	case ')':
		return Token(TokenType::CloseParenthese , itLexemeBegin + 1, itCurrent, ")", itLexemeBegin + 1 - itStartOfLine, line);
	case '{':
		return Token(TokenType::OpenBrace, itLexemeBegin, itCurrent + 1, "{", itLexemeBegin + 1 - itStartOfLine, line);
	case '}':
		return Token(TokenType::CloseBrace, itLexemeBegin, itCurrent + 1, "}", itLexemeBegin + 1 - itStartOfLine, line);
	default:
		return FAILED();
	}
}

Token Lexer::getOperator() {
	return FAILED();
}

Token Lexer::getSemicolon()
{
	if (*itCurrent == ';') { return Token(TokenType::Semicolon, itLexemeBegin, itCurrent + 1, ";", itLexemeBegin - itStartOfLine, line); }

	return FAILED();
}

Token Lexer::getUnaryOperator()
{
	switch (*itCurrent) {
	case '~':
		return Token(TokenType::BitwiseComplement, itLexemeBegin, itCurrent + 1, "~", itLexemeBegin - itStartOfLine, line);
	case '-':
		return Token(TokenType::Negation, itLexemeBegin, itCurrent + 1, "-", itLexemeBegin - itStartOfLine, line);
	case '!':
		return Token(TokenType::LogicalNegation, itLexemeBegin, itCurrent + 1, "!", itLexemeBegin - itStartOfLine, line);
	}

	return FAILED();
}

Token Lexer::getBinaryOperator()
{
	switch (*itCurrent) {
	case '+':
		return Token(TokenType::Addition, itLexemeBegin, itCurrent + 1, "+", itLexemeBegin - itStartOfLine, line);
	case '*':
		return Token(TokenType::Multiplication, itLexemeBegin, itCurrent + 1, "*", itLexemeBegin - itStartOfLine, line);
	case '/':
		return Token(TokenType::Division, itLexemeBegin, itCurrent + 1, "/", itLexemeBegin - itStartOfLine, line);
	}

	if (std::string(itCurrent, itCurrent + 2).compare("&&") == 0) {
		itCurrent += 2;
		return Token(TokenType::LogicalAnd, itLexemeBegin, itCurrent + 2, "&&", itLexemeBegin - itStartOfLine, line);
	}

	if (std::string(itCurrent, itCurrent + 2).compare("||") == 0) {
		itCurrent += 2;
		return Token(TokenType::LogicalOr, itLexemeBegin, itCurrent + 2, "||", itLexemeBegin - itStartOfLine, line);
	}

	return FAILED();
}

Token Lexer::getAssignment()
{
	if (*itCurrent == '=') {
		return Token(TokenType::Assignment, itLexemeBegin, itCurrent + 1, "=", itLexemeBegin - itStartOfLine, line);
	}

	return FAILED();
}

Token Lexer::FAILED() {
	return Token(TokenType::FAILED, itLexemeBegin, itCurrent, std::string(itLexemeBegin, itCurrent), itLexemeBegin - itStartOfLine, line);
}

bool Lexer::END() {
	return itCurrent == inputString.cend();
}