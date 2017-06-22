//
// Copyright 2009-2010 Ettus Research LLC
// Copyright 2009-2011 Disco Labs, TU Kaiserslautern
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#include "wfchain.h"
#include <algorithm>
#include <cstring>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/bind.hpp>
#include <iostream>

namespace WiFire {

using Platform::SizeOf;

Chain::Chain(const std::string& name, Verdict policy, bool topLevel)
	: name(name), policy(policy), topLevel(topLevel), usageCount(0) {}

Chain::ptr Chain::New(const std::string& name)
{
	return ptr(new Chain(name, WF_RET, false));
}

Chain::ptr Chain::NewTop(const std::string& name, Verdict policy)
{
	return ptr(new Chain(name, policy, true));
}

void Chain::addRule(Rule::ptr rule)
{
	rules.push_back(rule);
}

void Chain::deleteRule(Rule::ptr rule)
{
	rules.erase(std::remove(rules.begin(), rules.end(), rule));
}

Rule::ptr Chain::getRule(std::size_t i)
{
	if (i >= rules.size())
		return Rule::ptr();
	else
		return rules.at(i);
}

void Chain::insertRule(std::size_t i, Rule::ptr rule)
{
	std::vector<Rule::ptr>::iterator it = rules.begin() + i;
	rules.insert(it, rule);
}

std::size_t Chain::getRuleCount() const
{
	return rules.size();
}

void Chain::flush()
{
	rules.clear();
}

void Chain::ref()
{
	++usageCount;
}

void Chain::unref()
{
	--usageCount;
}

bool Chain::inUse() const
{
	return topLevel || usageCount;
}

bool Chain::isTopLevel() const
{
	return topLevel;
}

void Chain::setPolicy(Verdict v)
{
	policy = v;
}

const std::string& Chain::getName() const
{
	return name;
}

void Chain::print(std::ostream& stream) const
{
	stream << "Chain: " << name << "   Policy: " << VerdictToName(policy) << std::endl;
	/* call print(stream) on each rule */
	std::size_t i = 1;
	std::for_each(rules.begin(), rules.end(),
		(boost::lambda::var(stream) << " (#" << boost::lambda::var(i)++ << ") ",
		boost::lambda::bind(&Rule::print, *boost::lambda::_1, boost::lambda::var(stream)),
		boost::lambda::var(stream) << "\n"));
}

void Chain::size(Platform::Size& chain, Platform::Size& config) const
{
	chain.align();
	/* policy (4) + name (20) + prev (4) + currule (4) + endrule (4) */
	chain = chain + SizeOf<uint32_t>() + SizeOf<char[20]>() + SizeOf<Platform::ptr>() + SizeOf<Platform::ptr>() + SizeOf<Platform::ptr>();
	/* call size(chain, config) on each rule */
	std::for_each(rules.begin(), rules.end(),
		boost::bind(&Rule::size, _1, boost::ref(chain), boost::ref(config)));
}

void Chain::write(Stream& chain, Stream& config) const
{
	chain.align();
	/* policy*/
	chain.write<uint32_t>(policy);
	/* name */
	char name[20];
	std::strncpy(name, this->name.c_str(), sizeof name);
	name[19] = '\0';
	chain.write(name);
	chain.write<Platform::ptr>(0);
	chain.write<Platform::ptr>(0);
	/* save pointer to current position (this will hold the end pointer) */
	Stream s(chain);
	chain.write<Platform::ptr>(0); // dummy
	/* call write(chain, config) on each rule */
	std::for_each(rules.begin(), rules.end(),
		boost::bind(&Rule::write, _1, boost::ref(chain), boost::ref(config)));
	/* write end pointer */
	s.write(&chain);
}

Platform::ptr Chain::getLocation() const
{
	return location;
}

void Chain::setLocation(const Stream& stream)
{
	location = &stream;
}

bool Chain::wouldLoop(Target::ptr trg) const
{
	if (trg->getVerdict() != WF_JMP) return false;
	Chain::ptr trgChain = trg->getTargetChain();
	if (trgChain.get() == this) return true;
	for (std::vector<Rule::ptr>::iterator it = trgChain->rules.begin(); it != trgChain->rules.end(); ++it)
		if (wouldLoop((*it)->getTarget())) return true;
	return false;
}

}
