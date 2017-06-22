
#pragma once

#include <string>
#include "terminal.h"

namespace Parser {

/** \brief token terminal */
class TokenTerminal : public Terminal {
protected:
	/** construct a new token terminal parser */
	TokenTerminal();

public:
	/** get the token text to be parsed
	 * \return token text
	 */
	virtual std::string get() const = 0;

protected:
	PARSE_STATE _parse(Scanner& s, std::vector<Expect>& e);
	COMPLETE_STATE complete(Scanner& s, std::vector<std::string>& f) const;
	FIRST_STATE first(std::vector<std::string>& f) const;
};

}
