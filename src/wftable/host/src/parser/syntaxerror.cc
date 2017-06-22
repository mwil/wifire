
#include "syntaxerror.h"

namespace Parser {

SyntaxError::SyntaxError(const Scanner& s) throw() : scan(s), junk(true) {}

SyntaxError::SyntaxError(const Scanner& s, const std::vector<std::string>& expected) throw()
	: scan(s), expected(expected), junk(false) {}

SyntaxError::~SyntaxError() throw() {}

const char * SyntaxError::what() const throw()
{
	return "Syntax Error";
}

const Scanner& SyntaxError::getScanner() const throw()
{
	return scan;
}

const std::vector<std::string>& SyntaxError::getExpected() const throw()
{
	return expected;
}

bool SyntaxError::isJunk() const throw()
{
	return junk;
}

}
