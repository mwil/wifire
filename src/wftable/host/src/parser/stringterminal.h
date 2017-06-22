
#pragma once

#include <string>
#include "terminal.h"

namespace Parser {

/** \brief String (Freetext) Terminal */
class StringTerminal : public Terminal {
public:
	/// pointer type
	typedef boost::shared_ptr<StringTerminal> ptr;
	/// const pointer type
	typedef boost::shared_ptr<const StringTerminal> cptr;

private:
	/// parsed text
	std::string string;

	/** construct a new string terminal */
	StringTerminal();

public:
	/** get text
	 * \return parsed text
	 */
	const std::string& get() const;

protected:
	PARSE_STATE _parse(Scanner& s, std::vector<Expect>& e);
	COMPLETE_STATE complete(Scanner& s, std::vector<std::string>& f) const;
	FIRST_STATE first(std::vector<std::string>& f) const;
	Entity::ptr _clone() const;

public:
	/** create a new string terminal
	 * \return new string terminal
	 */
	static ptr New();
};

}
