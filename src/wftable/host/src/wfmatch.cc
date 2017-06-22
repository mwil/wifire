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

#include "wfmatch.h"
#include "platform/wfusrp.h"
#include <vector>
#include <parser/simpletokenterminal.h>
#include <algorithm>
#include <boost/bind.hpp>
#include <cstring>
#include "wfnotconnected.h"

namespace WiFire {

using Platform::SizeOf;

MatchDesc::MatchDesc(const std::string& name, const std::string& desc) :
	name(name), desc(desc), match(), fun(), found(false) {}

const std::string& MatchDesc::getName() const
{
	return name;
}

const std::string& MatchDesc::getDescription() const
{
	return desc;
}

Platform::ptr MatchDesc::getMatch() const
{
	return match;
}

Platform::ptr MatchDesc::getFun() const
{
	return fun;
}

void MatchDesc::connect(Platform::ptr match, Platform::ptr fun)
{
	found = true;
	this->match = match;
	this->fun = fun;
}

void MatchDesc::disconnect()
{
	found = false;
}

bool MatchDesc::isConnected() const
{
	return found;
}

Match::Match(MatchDesc::cptr desc, bool negate) : desc(desc), negate(negate) {}

Match::~Match() {}

MatchDesc::cptr Match::getDescription() const
{
	return desc;
}

void Match::print(std::ostream& stream) const
{
	stream << "-m " << (negate ? "! " : "") << desc->getName() << " ";
	_print(stream);
}

void Match::size(Platform::Size& chain, Platform::Size& config) const
{
	chain.align();
	config.align();
	/* function ptr (4), negate bool (1), data ptr (4) */
	chain = chain + SizeOf<Platform::ptr>() + SizeOf<bool>() + SizeOf<Platform::ptr>();
	/* inner data */
	_size(config);
}

void Match::write(Stream& chain, Stream& config) const
{
	if (!desc->isConnected())
		throw NotConnected("Match", desc->getDescription());

	chain.align();
	config.align();
	/* function ptr */
	chain.write(desc->getFun());
	/* negation */
	chain.write<uint8_t>(negate);
	/* pointer to configuration */
	chain.write(&config);
	/* inner configuration */
	_write(config);
}

MatchParser::MatchParser(MatchDesc::cptr desc) : desc(desc)
{
	sequenceAdd(Parser::SimpleTokenTerminal::New(desc->getName()));
}

MatchDesc::cptr MatchParser::getDescription() const
{
	return desc;
}

MatchShell::MatchShell()
{
	const std::vector<MatchDesc::ptr>& matches = USRP::Instance()->getHostMatches();
	/* create a parser for each match and add these */
	std::for_each(matches.begin(), matches.end(),
		boost::bind(&MatchShell::choiceAdd, this, boost::bind(&MatchDesc::parser, _1)));
}

MatchShell::ptr MatchShell::New()
{
	return ptr(new MatchShell);
}

Parser::Entity::ptr MatchShell::_clone() const
{
	return Entity::ptr(new MatchShell);
}

}
