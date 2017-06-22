
#pragma once

#include <boost/shared_ptr.hpp>
#include "scanner.h"
#include "entity.h"

namespace Parser {

/** \brief Optional Parser */
class Option : public Entity {
private:
	/// option to be parsed
	Entity::ptr opt;
	/// whether child could be parsed
	bool matched;

public:
	/// pointer type
	typedef boost::shared_ptr<Option> ptr;
	/// const pointer type
	typedef boost::shared_ptr<const Option> cptr;

public:
	/** construct an option
	 * \param entity option entity
	 */
	explicit Option(Entity::ptr entity);

protected:
	PARSE_STATE _parse(Scanner& s, std::vector<Expect>& e);
	COMPLETE_STATE complete(Scanner& s, std::vector<std::string>& f) const;
	FIRST_STATE first(std::vector<std::string>& f) const;

public:
	void action();

	/** get child entity
	 * \return child entity. valid after parsing succeeded and hasMatched() == true
	 */
	Entity::ptr get() const;

	Entity::ptr getNamedEntity(const std::string& name) const;

	/** whether option has matched
	 * \return true iff option matched
	 */
	bool hasMatched() const;

protected:
	/** clone helper: clone child into o
	 * \param o option to clone into
	 */
	void optionClone(ptr o) const;

	/** set child entity
	 * \param entity child entity
	 */
	void optionSet(Entity::ptr entity);
};

}
