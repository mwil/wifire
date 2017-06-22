
#include "choice.h"
#include <iterator>
#include <algorithm>
#include <boost/bind.hpp>

namespace Parser {

Choice::Choice() {}

PARSE_STATE Choice::_parse(Scanner& s, std::vector<Expect>& e)
{
	/* rationale: try to parse each child until first succeedes
	 * but: don't stop then, but parse all children for completing the expectation set
	 */
	Scanner first;
	bool hasMatch = false;
	for (std::vector<Entity::ptr>::iterator it = members.begin(); it != members.end(); ++it) {
		Scanner scan(s);
		PARSE_STATE ps = (*it)->parse(scan, e);
		if (ps == PS_OK && !hasMatch) {
			/* first matching child entity -> store that one */
			first = scan;
			hasMatch = true;
			match = *it;
		}
	}
	if (hasMatch) {
		s = first;
		return PS_OK;
	} else {
		return PS_FAIL;
	}
}

COMPLETE_STATE Choice::complete(Scanner& s,std::vector<std::string>& f) const
{
	/* try to complete each child until first is done */
	COMPLETE_STATE ret = CS_FAIL; // return CS_FAIL if each child fails
	for (std::vector<Entity::ptr>::const_iterator it = members.begin(); it != members.end(); ++it) {
		Scanner scan(s);
		COMPLETE_STATE cs = (*it)->complete(scan, f);
		if (cs == CS_DONE) {
			/* child matched, so choice is done */
			s = scan;
			return CS_DONE;
		} else if (cs == CS_EXIT) {
			/* child could be done partially, so choice is done partially (at least) */
			ret = CS_EXIT;
		}
	}

	return ret;
}

FIRST_STATE Choice::first(std::vector<std::string>& f) const
{
	FIRST_STATE ret = FS_DONE; // done if all child report done
	for (std::vector<Entity::ptr>::const_iterator it = members.begin(); it != members.end(); ++it)
		if ((*it)->first(f) == FS_EPSILON)
			ret = FS_EPSILON; // epsilon if one of the children is
	return ret;
}

void Choice::action()
{
	match->action();
}

Entity::ptr Choice::getNamedEntity(const std::string& name) const
{
	if (match->getName() == name)
		return match;
	else
		return match->getNamedEntity(name);
}

Entity::ptr Choice::get() const
{
	return match;
}

void Choice::choiceClone(ptr c) const
{
	c->members.clear();

	/* call clone on each child and push the clones in c's members */
	std::transform(members.begin(), members.end(), std::back_inserter(c->members),
		boost::mem_fn(&Entity::clone));
}

void Choice::choiceAdd(Entity::ptr entity)
{
	members.push_back(entity);
}

}
