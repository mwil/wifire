
#pragma once

#include "entity.h"

namespace Parser {

/** \brief terminal parser */
class Terminal : public Entity {
protected:
	/** construct a new terminal parser */
	Terminal();

public:
	void action();

	Entity::ptr getNamedEntity(const std::string& name) const;
};

}
