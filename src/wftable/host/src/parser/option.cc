
#include "option.h"

namespace Parser {

Option::Option(Entity::ptr entity) : opt(entity) {}

PARSE_STATE Option::_parse(Scanner& s, std::vector<Expect>& e)
{
	Scanner scan(s);

	if (opt->parse(scan, e) == PS_OK) {
		matched = true;
		s = scan;
	} else {
		matched = false;
	}

	return PS_OK;
}

COMPLETE_STATE Option::complete(Scanner& s, std::vector<std::string>& f) const
{
	Scanner scan(s);

	COMPLETE_STATE cs = opt->complete(scan, f);
	if (cs == CS_DONE)
		s = scan;

	return CS_DONE;
}

FIRST_STATE Option::first(std::vector<std::string>& f) const
{
	if (opt)
		opt->first(f);
	return FS_EPSILON; // epsilon always included in first set
}

void Option::action()
{
	if (matched)
		opt->action();
}

Entity::ptr Option::get() const
{
	return opt;
}

Entity::ptr Option::getNamedEntity(const std::string& name) const
{
	if (opt->getName() == name)
		return opt;
	else
		return opt->getNamedEntity(name);
}

bool Option::hasMatched() const
{
	return matched;
}

void Option::optionClone(ptr o) const
{
	o->opt = opt->clone();
}

void Option::optionSet(Entity::ptr entity)
{
	opt = entity;
}

}
