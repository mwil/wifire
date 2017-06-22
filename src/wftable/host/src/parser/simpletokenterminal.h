
#pragma once

#include "tokenterminal.h"

namespace Parser {

/** \brief simple string token terminal */
class SimpleTokenTerminal : public TokenTerminal {
private:
	/// expected text
	std::string name;

	/** construct a new string token terminal
	 * \param name token text
	 */
	SimpleTokenTerminal(const std::string& name);

public:
	/// pointer type
	typedef boost::shared_ptr<SimpleTokenTerminal> ptr;
	/// const pointer type
	typedef boost::shared_ptr<const SimpleTokenTerminal> cptr;

protected:
	Entity::ptr _clone() const;

	std::string get() const;

public:
	/** create a new string token terminal
	 * \param text token text
	 * \return new token terminal
	 */
	static ptr New(const std::string& name);
};

}
