
#include "token.h"

namespace Parser {

Token::Token() : begin(std::string::npos), end(std::string::npos) {}

Token::Token(std::size_t begin, std::size_t end, const std::string& token) :
	begin(begin), end(end), token(token) {}

bool Token::empty() const
{
	return token.empty();
}

}
