#ifndef TOKEN_H
#define TOKEN_H

#include <string>
#include <iostream>

enum TokenType {
	IntValue,
	FloatValue,
	Identifier,
	IntType,
	FloatType,
	OpenBrace,
	CloseBrace,
	OpenParenthese,
	CloseParenthese,
	ReturnKeyword,
	Semicolon,
	End,
	FAILED // Lexer failed to determine token
};

inline std::string tokenTypeToString(const TokenType& tokenType) {
	switch (tokenType)
	{
	case IntValue:
		return "Int value";
	case FloatValue:
		return "Float value";
	case IntType:
		return "Int type";
	case Identifier:
		return "Identifier";
	case OpenBrace:
		return "Open brace";
	case CloseBrace:
		return "Close brace";
	case OpenParenthese:
		return "Open parenthese";
	case CloseParenthese:
		return "Close parenthese";
	case Semicolon:
		return "Semicolon";
	default:
		return "undefined";
	}
};

struct Token {
	TokenType type;
	std::string::const_iterator begin;
	std::string::const_iterator end;
	std::string lexeme;

	union {
		int32_t intVal;
		float floatVal;
	};

	Token() {};
	Token(TokenType _type, std::string::const_iterator _begin, std::string::const_iterator _end, std::string _lexeme) : type(_type), begin(_begin), end(_end), lexeme(_lexeme) { };
	Token(TokenType _type, std::string::const_iterator _begin, std::string::const_iterator _end, std::string _lexeme, int _val) : type(_type), begin(_begin), end(_end), lexeme(_lexeme), intVal(_val) { };
	Token(TokenType _type, std::string::const_iterator _begin, std::string::const_iterator _end, std::string _lexeme, float _val) : type(_type), begin(_begin), end(_end), lexeme(_lexeme), floatVal(_val) { };

	bool operator==(const Token& token) { return token.type == type && token.lexeme == lexeme; }
	bool operator!=(const Token& token) { return token.type != type || token.lexeme != lexeme; }
};

inline std::ostream& operator<<(std::ostream& out, const Token& token) { return out << tokenTypeToString(token.type) << " | " << token.lexeme; };

#endif