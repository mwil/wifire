
#pragma once

#include <boost/shared_ptr.hpp>
#include <vector>
#include "scanner.h"
#include "entity.h"

namespace Parser {

/** \brief permutation (sequence of arbitrary order) parser */
class Permutation : public Entity {
private:
	/// children entities
	std::vector<Entity::ptr> members;

public:
	/// pointer type
	typedef boost::shared_ptr<Permutation> ptr;
	/// const pointer type
	typedef boost::shared_ptr<const Permutation> cptr;

public:
	/** construct a new permutation */
	Permutation();

protected:
	PARSE_STATE _parse(Scanner& s, std::vector<Expect>& e);
	COMPLETE_STATE complete(Scanner& s, std::vector<std::string>& f) const;
	FIRST_STATE first(std::vector<std::string>& f) const;

public:
	void action();

	/** get n'th child entity
	 * \param n index of requested child
	 * \return n'th child
	 */
	Entity::ptr at(std::size_t n) const;

	Entity::ptr getNamedEntity(const std::string& name) const;

protected:
	/** clone helper: clone all children into p
	 * \param p permutation to clone into
	 */
	void permutationClone(ptr s) const;

	/** add a child entity to permutation
	 * \param entity entity to add
	 */
	void permutationAdd(Entity::ptr entity);

private:
	/** construct first set of a subset of the permutation
	 * \param begin beginning iterator
	 * \param end end interator
	 * \param f [out] first set
	 */
	FIRST_STATE first_from(std::vector<Entity::ptr>::const_iterator begin,
		std::vector<Entity::ptr>::const_iterator end,
		std::vector<std::string>& f) const;
};

}
