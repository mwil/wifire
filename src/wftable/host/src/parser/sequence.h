
#pragma once

#include <boost/shared_ptr.hpp>
#include <vector>
#include "scanner.h"
#include "entity.h"

namespace Parser {

/** \brief sequence parser */
class Sequence : public Entity {
private:
	/// children entities
	std::vector<Entity::ptr> members;

public:
	/// pointer type
	typedef boost::shared_ptr<Sequence> ptr;
	/// const pointer type
	typedef boost::shared_ptr<const Sequence> cptr;

public:
	/** construct a new sequence */
	Sequence();

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
	/** clone helper: clone all children into s
	 * \param s sequence to clone into
	 */
	void sequenceClone(ptr s) const;

	/** add a child entity to sequence
	 * \param entity entity to add
	 */
	void sequenceAdd(Entity::ptr entity);

private:
	/** construct first set of a subset of the sequence
	 * \param it beginning iterator
	 * \param f [out] first set
	 */
	FIRST_STATE first_from(std::vector<Entity::ptr>::const_iterator it, std::vector<std::string>& f) const;
};

}
