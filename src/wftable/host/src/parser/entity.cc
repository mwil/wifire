
#include "entity.h"

namespace Parser {

Entity::Entity() {}
Entity::~Entity() {}

PARSE_STATE Entity::parse(Scanner& s, std::vector<Expect>& e)
{
	scan = s;
	return _parse(s, e);
}

const Scanner& Entity::getScanner() const
{
	return scan;
}

SemanticError Entity::semanticError(const std::string& desc) const
{
	return SemanticError(getScanner(), desc);
}

const std::string& Entity::getName() const
{
	return name;
}

void Entity::setName(const std::string& name)
{
	this->name = name;
}

Entity::ptr Entity::clone() const
{
	ptr ret = _clone();
	ret->setName(getName());
	return ret;
}

}
