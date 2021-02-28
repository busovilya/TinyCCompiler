#ifndef ERROR_H
#define ERRORE_H

#include <string>
#include <sstream>

struct Error {
	Error(const Error& error) : message(error.getMessage()) {};
	Error(std::string msg) : message(msg) {};
	std::string getMessage() const { return message; }
private:
	std::string message;
};

struct CompilerError : Error {
	CompilerError(std::string msg) : Error(msg) {};
	static CompilerError* errorAtLine(std::string msg, int line) {
		std::stringstream ss;
		ss << msg << " Line " << line + 1;
		return new CompilerError(ss.str());
	}
};

#endif