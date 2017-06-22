
#include "stringterminal.h"

namespace Parser {

StringTerminal::StringTerminal() {}

const std::string& StringTerminal::get() const
{
	return string;
}

PARSE_STATE StringTerminal::_parse(Scanner& s, std::vector<Expect>& e)
{
	Token tk = s.pop();

	if (tk.empty())
		return PS_FAIL;

	string = tk.token;

	return PS_OK;
}

COMPLETE_STATE StringTerminal::complete(Scanner& s, std::vector<std::string>& f) const
{
	s.pop();
	return CS_DONE;
}

FIRST_STATE StringTerminal::first(std::vector<std::string>& f) const
{
	return FS_DONE;
}

Entity::ptr StringTerminal::_clone() const
{
	return New();
}

StringTerminal::ptr StringTerminal::New()
{
	return ptr(new StringTerminal);
}

}
