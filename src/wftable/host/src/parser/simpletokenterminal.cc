
#include "simpletokenterminal.h"

namespace Parser {

SimpleTokenTerminal::ptr SimpleTokenTerminal::New(const std::string& name)
{
	return ptr(new SimpleTokenTerminal(name));
}

SimpleTokenTerminal::SimpleTokenTerminal(const std::string& name)
	: name(name) {}

Entity::ptr SimpleTokenTerminal::_clone() const
{
	return New(name);
}

std::string SimpleTokenTerminal::get() const
{
	return name;
}

}
