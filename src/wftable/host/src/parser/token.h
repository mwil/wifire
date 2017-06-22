
#pragma once

#include <string>
#include <cstdlib>

namespace Parser {

/** \brief Token */
class Token {
public:
	/// begin in line
	std::size_t begin;
	/// end in line
	std::size_t end;
	/// token itself
	std::string token;

	/** construct an empty token */
	Token();

	/** construct a new token
	 * \param begin beginning in line
	 * \param end ending in line
	 * \param token content
	 */
	Token(std::size_t begin, std::size_t end, const std::string& token);

	/** whether token is empty
	 * \return true iff token is empty
	 */
	bool empty() const;
};

}
