
#include "builder.h"
#include "simpletokenterminal.h"
#include <util.h>

namespace Parser {

Builder::SequenceHelper::SequenceHelper() {}

Entity::ptr Builder::SequenceHelper::_clone() const
{
	SequenceHelper::ptr s(new SequenceHelper);
	sequenceClone(s);
	return s;
}

void Builder::SequenceHelper::add(Entity::ptr p)
{
	sequenceAdd(p);
}

Builder::ChoiceHelper::ChoiceHelper() {}

Entity::ptr Builder::ChoiceHelper::_clone() const
{
	ChoiceHelper::ptr c(new ChoiceHelper);
	choiceClone(c);
	return c;
}

void Builder::ChoiceHelper::add(Entity::ptr p)
{
	choiceAdd(p);
}

Builder::RepetitionHelper::RepetitionHelper(Entity::ptr p) : p(p) {}

Entity::ptr Builder::RepetitionHelper::_clone() const
{
	RepetitionHelper::ptr r(new RepetitionHelper(p));
	repetitionClone(r);
	return r;
}

Entity::ptr Builder::RepetitionHelper::repetitionNew() const
{
	return p->clone();
}

Builder::PermutationHelper::PermutationHelper() {}

Entity::ptr Builder::PermutationHelper::_clone() const
{
	PermutationHelper::ptr s(new PermutationHelper);
	permutationClone(s);
	return s;
}

void Builder::PermutationHelper::add(Entity::ptr p)
{
	permutationAdd(p);
}

Builder::OptionHelper::OptionHelper(Entity::ptr p) : Option(p) {}

Entity::ptr Builder::OptionHelper::_clone() const
{
	OptionHelper::ptr o(new OptionHelper(Entity::ptr()));
	optionClone(o);
	return o;
}


Builder::Builder(Entity::ptr p) : entity(p) {}

Builder::Builder(const std::string& name) : entity(SimpleTokenTerminal::New(name)) {}

Builder::Builder(const Builder& b) : entity(b.entity) {}

Builder& Builder::operator=(const Builder& b)
{
	if (this != &b)
		entity = b.entity;
	return *this;
}

template<typename E>
Builder& Builder::operatorBuilder(const Builder& b)
{
	if (Util::instanceOf<E>(entity.get())) {
		Parser::as<E>(entity)->add(b.entity);
	} else {
		typename E::ptr e(new E);
		e->add(entity);
		e->add(b.entity);
		entity = e;
	}
	return *this;
}

Builder& Builder::operator+(const Builder& b)
{
	return operatorBuilder<SequenceHelper>(b);
}

Builder& Builder::operator|(const Builder& b)
{
	return operatorBuilder<ChoiceHelper>(b);
}

Builder& Builder::operator*()
{
	if (!Util::instanceOf<RepetitionHelper>(entity.get()))
		entity = Entity::ptr(new RepetitionHelper(entity));
	return *this;
}

Builder& Builder::operator*(const Builder& b)
{
	return operatorBuilder<PermutationHelper>(b);
}

Builder& Builder::operator-()
{
	if (!Util::instanceOf<OptionHelper>(entity.get()))
		entity = Entity::ptr(new OptionHelper(entity));
	return *this;
}

Builder& Builder::operator[](const std::string& name)
{
	entity->setName(name);
	return *this;
}

Builder::operator Entity::ptr() const
{
	return entity;
}

}
