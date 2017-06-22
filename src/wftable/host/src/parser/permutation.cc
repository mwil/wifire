
#include "permutation.h"
#include <boost/bind.hpp>

namespace Parser {

Permutation::Permutation() {}

PARSE_STATE Permutation::_parse(Scanner& s, std::vector<Expect>& e)
{
	/* list of unparsed children */
	std::vector<Entity::ptr> work(members);

	bool all;
	do {
		all = true;
		std::vector<Entity::ptr>::iterator matched;
		Scanner matchedScan;
		/* find a matching child */
		for (std::vector<Entity::ptr>::iterator it = work.begin(); it != work.end(); ++it) {
			Scanner scan(s);
			PARSE_STATE st = (*it)->parse(scan, e);
			if (st == PS_OK && all) {
				/* found first (will continue on others to get expect set */
				all = false;
				matched = it;
				matchedScan = scan;
			}
		}
		if (!all) {
			/* found a matching child */
			s = matchedScan;
			work.erase(matched);
		}
	} while (!all);

	/* if all were found (work set empty) then all is done */
	return work.empty() ? PS_OK : PS_FAIL;
}

COMPLETE_STATE Permutation::complete(Scanner& s, std::vector<std::string>& f) const
{
	std::vector<Entity::ptr> matched(members);
	COMPLETE_STATE ret = CS_DONE;

	bool all;
	do {
		all = true;
		if (s.atEnd()) {
			first_from(matched.begin(), matched.end(), f);
			return CS_EXIT;
		}
		for (std::vector<Entity::ptr>::iterator it = matched.begin(); it != matched.end(); ++it) {
			Scanner scan(s);
			COMPLETE_STATE cs = (*it)->complete(scan, f);
			if (cs == CS_DONE) {
				s = scan;
				matched.erase(it);
				all = false;
				break;
			} else if (cs == CS_EXIT) {
				matched.erase(it);
				all = false;
				ret = CS_EXIT;
				break;
			}
		}
	} while (!all);

	return matched.empty() ? ret : CS_FAIL;
}

FIRST_STATE Permutation::first_from(std::vector<Entity::ptr>::const_iterator begin,
	std::vector<Entity::ptr>::const_iterator end, std::vector<std::string>& f) const
{
	FIRST_STATE ret = FS_EPSILON;
	for (std::vector<Entity::ptr>::const_iterator it = begin; it != end; ++it)
		if ((*it)->first(f) == FS_DONE)
			ret = FS_DONE;
	return ret;
}

FIRST_STATE Permutation::first(std::vector<std::string>& f) const
{
	return first_from(members.begin(), members.end(), f);
}

void Permutation::action()
{
	/* call action on each member */
	std::for_each(members.begin(), members.end(), boost::mem_fn(&Entity::action));
}

Entity::ptr Permutation::at(std::size_t n) const
{
	return members.at(n);
}

Entity::ptr Permutation::getNamedEntity(const std::string& name) const
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

void Permutation::permutationClone(ptr p) const
{
	p->members.clear();
	/* call clone on each child and push the clones in p's members */
	std::transform(members.begin(), members.end(), std::back_inserter(p->members),
		boost::mem_fn(&Entity::clone));
}

void Permutation::permutationAdd(Entity::ptr entity)
{
	members.push_back(entity);
}

}
