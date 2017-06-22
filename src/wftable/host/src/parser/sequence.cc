
#include "sequence.h"
#include <algorithm>
#include <boost/bind.hpp>

namespace Parser {

Sequence::Sequence() {}

PARSE_STATE Sequence::_parse(Scanner& s, std::vector<Expect>& e)
{
	for (std::vector<Entity::ptr>::iterator it = members.begin(); it != members.end(); ++it) {
		PARSE_STATE ps = (*it)->parse(s, e);
		if (ps == PS_FAIL)
			return PS_FAIL;
	}
	return PS_OK;
}

COMPLETE_STATE Sequence::complete(Scanner& s, std::vector<std::string>& f) const
{
	for (std::vector<Entity::ptr>::const_iterator it = members.begin(); it != members.end(); ++it) {
		if (s.atEnd()) {
			first_from(it, f);
			return CS_EXIT;
		}

		COMPLETE_STATE cs = (*it)->complete(s, f);
		if (cs == CS_EXIT)
			return CS_EXIT;

		if (cs == CS_FAIL)
			return CS_FAIL;
	}

	return CS_DONE;
}

FIRST_STATE Sequence::first_from(std::vector<Entity::ptr>::const_iterator it, std::vector<std::string>& f) const
{
	for (; it != members.end(); ++it)
		if ((*it)->first(f) != FS_EPSILON)
			return FS_DONE; // first set is first set of first non-epsilon child
	return FS_EPSILON;
}

FIRST_STATE Sequence::first(std::vector<std::string>& f) const
{
	return first_from(members.begin(), f);
}

void Sequence::action()
{
	/* call action on each member */
	std::for_each(members.begin(), members.end(), boost::mem_fn(&Entity::action));
}

Entity::ptr Sequence::at(std::size_t n) const
{
	return members.at(n);
}

Entity::ptr Sequence::getNamedEntity(const std::string& name) const
{
	for (std::vector<Entity::ptr>::const_iterator it = members.begin(); it != members.end(); ++it) {
		if ((*it)->getName() == name)
			return *it;
		Entity::ptr x = (*it)->getNamedEntity(name);
		if (x)
			return x;
	}
	return Entity::ptr();
}

void Sequence::sequenceClone(ptr s) const
{
	s->members.clear();
	/* call clone on each child and push the clones in s' members */
	std::transform(members.begin(), members.end(), std::back_inserter(s->members),
		boost::mem_fn(&Entity::clone));
}

void Sequence::sequenceAdd(Entity::ptr entity)
{
	members.push_back(entity);
}

}
