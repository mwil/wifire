
#include "tokenterminal.h"

namespace Parser {

TokenTerminal::TokenTerminal() {}

PARSE_STATE TokenTerminal::_parse(Scanner& s, std::vector<Expect>& e)
{
	if (s.pop().token != get()) {
		e.push_back(Expect(get(), getScanner()));
		return PS_FAIL;
	}
	return PS_OK;
}

COMPLETE_STATE TokenTerminal::complete(Scanner& s, std::vector<std::string>& f) const
{
	Token tk = s.pop();
	const std::string& name = get();

	if (s.atEnd()) {
		f.push_back(name);
		return CS_EXIT;
	}

	return tk.token == name ? CS_DONE : CS_FAIL;
}

FIRST_STATE TokenTerminal::first(std::vector<std::string>& f) const
{
	f.push_back(get());
	return FS_DONE;
}

}
