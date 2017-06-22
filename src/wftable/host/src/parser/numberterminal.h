
#pragma once

#include <string>
#include "terminal.h"
#include <boost/lexical_cast.hpp>
#include <istream>

namespace Parser {

/** \brief Parser for numeric terminal
 * \tparam T numberical type
 */
template<typename T>
class NumberTerminal : public Terminal {
private:
	/** construct a new number parser */
	NumberTerminal();

	//// parsed number
	T num;

public:
	/// pointer type
	typedef boost::shared_ptr<NumberTerminal> ptr;
	/// const pointer type
	typedef boost::shared_ptr<const NumberTerminal> cptr;

protected:
	PARSE_STATE _parse(Scanner& s, std::vector<Expect>& e);
	COMPLETE_STATE complete(Scanner& s, std::vector<std::string>& f) const;
	FIRST_STATE first(std::vector<std::string>& f) const;
	Entity::ptr _clone() const;

public:
	/** get parsed number
	 * \return number
	 */
	const T& get() const;

	/** create a new number parser
	 * \return new number parser
	 */
	static ptr New();
};

template<typename T>
NumberTerminal<T>::NumberTerminal() {}

template<typename T>
typename NumberTerminal<T>::ptr NumberTerminal<T>::New()
{
	return ptr(new NumberTerminal);
}

template<typename T>
PARSE_STATE NumberTerminal<T>::_parse(Scanner& s, std::vector<Expect>& e)
{
	Token tk = s.pop();

	if (tk.token.empty()) {
		e.push_back(Expect("<numeric value>", getScanner()));
		return PS_FAIL;
	}

	if (tk.token.substr(0, 2) == "0x") {
		std::istringstream in(tk.token.substr(2));
		bool f = (in >> std::hex >> num).fail();
		if (f) {
			e.push_back(Expect("<numeric value>", getScanner()));
			return PS_FAIL;
		}
		return PS_OK;
	}

	try {
		num = boost::lexical_cast<T>(tk.token);
		return PS_OK;
	} catch(boost::bad_lexical_cast&) {
		e.push_back(Expect("<numeric value>", getScanner()));
		return PS_FAIL;
	}
}

template<typename T>
COMPLETE_STATE NumberTerminal<T>::complete(Scanner& s, std::vector<std::string>& f) const
{
	Token tk = s.pop();

	if (s.atEnd())
		return CS_EXIT;

	if (tk.token.empty())
		return CS_FAIL;

	if (tk.token.substr(0, 2) == "0x") {
		std::istringstream in(tk.token.substr(2));
		T val;
		bool f = (in >> std::hex >> val).fail();
		return f ? CS_FAIL : CS_DONE;
	}

	try {
		boost::lexical_cast<T>(tk.token);
		return CS_DONE;
	} catch(boost::bad_lexical_cast&) {
		return CS_FAIL;
	}
}

template<typename T>
const T& NumberTerminal<T>::get() const
{
	return num;
}

template<typename T>
FIRST_STATE NumberTerminal<T>::first(std::vector<std::string>& f) const
{
	/* no first set */
	return FS_DONE;
}

template<typename T>
Entity::ptr NumberTerminal<T>::_clone() const
{
	return Entity::ptr(new NumberTerminal);
}

}
