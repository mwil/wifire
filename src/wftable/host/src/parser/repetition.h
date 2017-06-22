
#pragma once

#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>
#include <vector>
#include "scanner.h"
#include "entity.h"

namespace Parser {

/** \brief repetition (zero or more) parser */
class Repetition : public Entity {
private:
	/// parsed child entities
	std::vector<Entity::ptr> members;

public:
	/// pointer type
	typedef boost::shared_ptr<Repetition> ptr;
	/// const pointer
	typedef boost::shared_ptr<const Repetition> cptr;

	/** construct a repetition */
	Repetition();

protected:
	PARSE_STATE _parse(Scanner& s, std::vector<Expect>& e);
	COMPLETE_STATE complete(Scanner& s, std::vector<std::string>& f) const;
	FIRST_STATE first(std::vector<std::string>& f) const;

public:
	void action();

	/** get begin iterator for children
	 * \return begin iterator for children
	 */
	std::vector<Entity::ptr>::const_iterator begin() const;

	/** get end iterator for children
	 * \return end iterator for children
	 */
	std::vector<Entity::ptr>::const_iterator end() const;

	/** get a list of entities with given name on each child
	 * \tparam T type of entity
	 * \param name name of entity
	 * \return list of entities
	 */
	template<typename T>
	std::vector<typename T::ptr> getNamedEntities(const std::string& name) const;

	Entity::ptr getNamedEntity(const std::string& name) const;

protected:
	/** clone helper: clone all children into r
	 * \param r repetition to clone into
	 */
	void repetitionClone(ptr r) const;

	/** remove all parsed children */
	void repetitionReset();

	/** create a new child for that repetition
	 * \return new child
	 */
	virtual Entity::ptr repetitionNew() const = 0;
};

template<typename T>
std::vector<typename T::ptr> Repetition::getNamedEntities(const std::string& name) const
{
	std::vector<typename T::ptr> ret;
	for (std::vector<Entity::ptr>::const_iterator it = members.begin(); it != members.end(); ++it) {
		Entity::ptr t((*it)->getNamedEntity(name));
		if (t) {
			typename T::ptr x(boost::dynamic_pointer_cast<T>(t));
			if (x)
				ret.push_back(x);
		}
	}
	return ret;
}

}
