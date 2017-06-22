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

#include "ctp.h"

#include <parser/builder.h>
#include <parser/numberterminal.h>
#include <iomanip>

namespace WiFire {
namespace Matches {

using Parser::Builder;
using Platform::SizeOf;


CtpDesc::CtpDesc() : MatchDesc("ctp", "CTP") {}

CtpDesc::ptr CtpDesc::instance;

CtpDesc::ptr CtpDesc::Instance()
{
        if (!instance)
                instance = ptr(new CtpDesc);
        return instance;
}

Parser::Entity::ptr CtpDesc::parser() const
{
        return CtpParser::New(instance);
}

CtpParser::CtpParser(MatchDesc::cptr m) : MatchParser(m)
{
	Builder reqlen = Builder("--minlen") + (reqlenParser = Parser::NumberTerminal<uint32_t>::New());
	Builder type = Builder("--type") + (typeParser = Parser::NumberTerminal<uint32_t>::New());
	Builder pull = Builder("--pull") + (pullParser = Parser::NumberTerminal<uint32_t>::New());
	Builder congestion = Builder("--congestion") + (congestionParser = Parser::NumberTerminal<uint32_t>::New());

	sequenceAdd(reqlen * type * pull * congestion);
}

CtpParser::ptr CtpParser::New(MatchDesc::cptr m)
{
	return ptr(new CtpParser(m));
}

Parser::Entity::ptr CtpParser::_clone() const
{
	return Entity::ptr(new CtpParser(getDescription()));
}

Match::ptr CtpParser::compile(bool negate) const
{
        uint32_t reqlen = reqlenParser->get();
	uint32_t type = typeParser->get();
	uint32_t pull = pullParser->get();
	uint32_t congestion = congestionParser->get();

	return Ctp::New(getDescription(), negate, reqlen, type, pull, congestion);
}

Ctp::Ctp(MatchDesc::cptr m, bool negate, uint32_t reqlen, uint32_t type, uint32_t pull, uint32_t congestion) :
	Match(m, negate), reqlen(reqlen), type(type), pull(pull), congestion(congestion) {}

Ctp::ptr Ctp::New(MatchDesc::cptr m, bool negate, uint32_t reqlen, uint32_t type, uint32_t pull, uint32_t congestion)
{
	return ptr(new Ctp(m, negate, reqlen, type, pull, congestion));
}

void Ctp::_print(std::ostream& stream) const
{
	stream << "--minlen " << std::dec << uint32_t(reqlen) << " --type " << std::dec << uint32_t(type) << " --pull " << std::dec << uint32_t(pull) << " --congestion " << std::dec << uint32_t(congestion);
}

void Ctp::_size(Platform::Size& config) const
{
	config = config + SizeOf<uint32_t>() + SizeOf<uint32_t>() + SizeOf<uint32_t>() + SizeOf<uint32_t>();
}

void Ctp::_write(Stream& config) const
{
	config.write<uint32_t>(reqlen);
	config.write<uint32_t>(type);
	config.write<uint32_t>(pull);
	config.write<uint32_t>(congestion);
}

}
}
