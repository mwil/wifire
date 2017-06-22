
#include "repetition.h"
#include <boost/bind.hpp>
#include <algorithm>

namespace Parser {

Repetition::Repetition() {}

PARSE_STATE Repetition::_parse(Scanner& s, std::vector<Expect>& e)
{
	Scanner scan(s);

	/* new parsing: clear all members */
	members.clear();

	while (true) {
		Scanner locscan(scan);
		/* create a new children and try to parse */
		Entity::ptr n = repetitionNew();

		if (n->parse(locscan, e) == PS_FAIL)
			break;

		/* parsing went ok, push child */
		members.push_back(n);
		scan = locscan;
	}

	s = scan;
	return PS_OK;
}

COMPLETE_STATE Repetition::complete(Scanner& s, std::vector<std::string>& f) const
{
	Scanner scan(s);
	Entity::ptr n = repetitionNew();

	while (true) {
		Scanner locscan(scan);

		COMPLETE_STATE cs = n->complete(locscan, f);
		if (cs != CS_DONE)
			break;

		scan = locscan;
	}

	s = scan;
	return CS_DONE;
}

FIRST_STATE Repetition::first(std::vector<std::string>& f) const
{
	repetitionNew()->first(f);
	return FS_EPSILON; // always epsilon
}

void Repetition::action()
{
	/* call action on each member */
	std::for_each(members.begin(), members.end(), boost::mem_fn(&Entity::action));
}

std::vector<Entity::ptr>::const_iterator Repetition::begin() const
{
	return members.begin();
}

std::vector<Entity::ptr>::const_iterator Repetition::end() const
{
	return members.end();
}

Entity::ptr Repetition::getNamedEntity(const std::string& name) const
{
	return Entity::ptr();
}

void Repetition::repetitionClone(ptr r) const
{
	r->members.clear();
	/* call clone on each child and push the clones in r's members */
	std::transform(members.begin(), members.end(), std::back_inserter(r->members),
		boost::mem_fn(&Entity::clone));
}

void Repetition::repetitionReset()
{
	members.clear();
}

}
