
#pragma once

#include <boost/shared_ptr.hpp>
#include <vector>
#include "scanner.h"
#include "entity.h"

namespace Parser {

/** \brief choice parser */
class Choice : public Entity {
private:
	/// children entities
	std::vector<Entity::ptr> members;
	/// pointer to matching entity
	Entity::ptr match;

public:
	/// pointer type
	typedef boost::shared_ptr<Choice> ptr;
	/// const pointer type
	typedef boost::shared_ptr<const Choice> cptr;

	/** create a new choice
	 * \return new choice
	 */
	static ptr New();

protected:
	/** construct a new choice */
	Choice();

protected:
	PARSE_STATE _parse(Scanner& s, std::vector<Expect>& e);
	COMPLETE_STATE complete(Scanner& s, std::vector<std::string>& f) const;
	FIRST_STATE first(std::vector<std::string>& f) const;

public:
	void action();

public:
	/** get matched child entity
	 * \return matched child entity (valid after parsing succeeded)
	 */
	Entity::ptr get() const;
	Entity::ptr getNamedEntity(const std::string& name) const;

protected:
	/** clone helper: clone all children into c
	 * \param c choice to clone into
	 */
	void choiceClone(ptr c) const;

	/** add child entity to choice
	 * \param entity child entity to be added
	 */
	void choiceAdd(Entity::ptr entity);
};

}
