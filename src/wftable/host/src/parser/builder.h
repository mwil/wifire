
#pragma once

#include "entity.h"
#include "sequence.h"
#include "choice.h"
#include "repetition.h"
#include "permutation.h"
#include "option.h"

namespace Parser {

class Builder {
private:
	class SequenceHelper : public Sequence {
	friend class Builder;
	public:
		typedef boost::shared_ptr<SequenceHelper> ptr;
		typedef boost::shared_ptr<const SequenceHelper> cptr;

		Entity::ptr _clone() const;

	private:
		SequenceHelper();
		void add(Entity::ptr p);
	};

	class ChoiceHelper : public Choice {
	friend class Builder;
	public:
		typedef boost::shared_ptr<ChoiceHelper> ptr;
		typedef boost::shared_ptr<const ChoiceHelper> cptr;

		Entity::ptr _clone() const;

	private:
		ChoiceHelper();
		void add(Entity::ptr p);
	};

	class RepetitionHelper : public Repetition {
	friend class Builder;
	public:
		typedef boost::shared_ptr<RepetitionHelper> ptr;
		typedef boost::shared_ptr<const RepetitionHelper> cptr;

	private:
		Entity::ptr p;

	public:
		Entity::ptr _clone() const;

	protected:
		Entity::ptr repetitionNew() const;

	private:
		RepetitionHelper(Entity::ptr p);
	};

	class PermutationHelper : public Permutation {
	friend class Builder;
	public:
		typedef boost::shared_ptr<PermutationHelper> ptr;
		typedef boost::shared_ptr<const PermutationHelper> cptr;

		Entity::ptr _clone() const;

	private:
		PermutationHelper();
		void add(Entity::ptr p);
	};

	class OptionHelper : public Option {
	friend class Builder;
	public:
		typedef boost::shared_ptr<OptionHelper> ptr;
		typedef boost::shared_ptr<const OptionHelper> cptr;

		Entity::ptr _clone() const;

	private:
		OptionHelper(Entity::ptr opt);
	};

	Entity::ptr entity;

public:
	Builder(Entity::ptr p);
	Builder(const std::string& name);
	Builder(const Builder& b);

private:
	//Builder(const Builder& b);
	Builder& operator=(const Builder& b);

public:
	Builder& operator+(const Builder& b);
	template<typename T>
	Builder& operator+(T p);

	Builder& operator|(const Builder& b);
	template<typename T>
	Builder& operator|(T p);

	Builder& operator*();

	Builder& operator*(const Builder& b);
	template<typename T>
	Builder& operator*(T p);

	Builder& operator-();

	Builder& operator[](const std::string& name);

	operator Entity::ptr() const;

private:
	template<typename E>
	Builder& operatorBuilder(const Builder& b);
};

template<typename T>
Builder& Builder::operator+(T p)
{
	return operator+(Builder(p));
}

template<typename T>
Builder& Builder::operator|(T p)
{
	return operator|(Builder(p));
}

template<typename T>
Builder& Builder::operator*(T p)
{
	return operator*(Builder(p));
}

}
