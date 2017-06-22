
#include "terminal.h"

namespace Parser {

Terminal::Terminal() {}

void Terminal::action() {}

Entity::ptr Terminal::getNamedEntity(const std::string& name) const
{
	return Entity::ptr();
}

}
