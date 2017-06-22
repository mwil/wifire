
#pragma once

#include <boost/shared_ptr.hpp>
#include <string>
#include <vector>
#include "scanner.h"
#include "state.h"
#include "expect.h"
#include "semanticerror.h"

namespace Parser {

/** abstract parser entity (any terminal or non-terminal) */
class Entity {
public:
	/// pointer type
	typedef boost::shared_ptr<Entity> ptr;
	/// const pointer type
	typedef boost::shared_ptr<const Entity> cptr;

protected:
	/** construct a new entity */
	Entity();

public:
	/** destruct an entity */
	virtual ~Entity();

protected:
	/** parse entity
	 * \param s scanner
	 * \param e [inout] expected set
	 * \return parse state
	 */
	virtual PARSE_STATE _parse(Scanner& s, std::vector<Expect>& e) = 0;

	/** clone entity
	 * \return clone (at least of parser structure)
	 */
	virtual ptr _clone() const = 0;

public:
	/** parse entity
	 * \param s scanner
	 * \param e expected set
	 * \return parse state
	 */
	PARSE_STATE parse(Scanner& s, std::vector<Expect>& e);

	/** complete entity
	 * \param s scanner
	 * \param f completion list
	 * \return completion state
	 */
	virtual COMPLETE_STATE complete(Scanner& s, std::vector<std::string>& f) const = 0;

	/** calculate first set
	 * \param [inout] f first set
	 * \return first state
	 */
	virtual FIRST_STATE first(std::vector<std::string>& f) const = 0;

	/** after whole tree is parsed, do some fancy actions */
	virtual void action() = 0;

	/** clone entity
	 * \return clone (at least of parser structure)
	 */
	ptr clone() const;

	/** get entity name
	 * \return entity name
	 */
	const std::string& getName() const;

	/** set entity name
	 * \param name entity name
	 */
	void setName(const std::string& name);

	/** get first entity in tree with a specific name
	 * \param name name of entity to be searched
	 * \return entity or null pointer if not found
	 */
	virtual ptr getNamedEntity(const std::string& name) const = 0;

	/** create a semantic error at this entity. can be thrown then
	 * \param desc error description
	 * \return semantic error
	 */
	SemanticError semanticError(const std::string& desc) const;

protected:
	/** get scanner
	 * \param scanner
	 */
	const Scanner& getScanner() const;

private:
	/// entity name
	std::string name;
	/// scanner
	Scanner scan;
};

/** upcast generic entity to specialized one
 * \tparam T entity type to cast to
 * \param e entity to cast
 * \return e as T
 */
template<typename T>
typename T::ptr as(Entity::ptr e)
{
	/* TODO: maybe we should do a checked cast */
	return boost::dynamic_pointer_cast<T>(e);
}

/** upcast generic entity to specialized one (const version)
 * \tparam T const entity type to cast to
 * \param e entity to cast
 * \return e as const T
 */
template<typename T>
typename T::cptr as(Entity::cptr e)
{
	/* TODO: maybe we should do a checked cast */
	return boost::dynamic_pointer_cast<T>(e);
}


}
