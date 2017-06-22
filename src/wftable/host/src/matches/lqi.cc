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

#include "lqi.h"

#include <parser/builder.h>
#include <parser/numberterminal.h>
#include <iomanip>

namespace WiFire {
namespace Matches {

using Parser::Builder;
using Platform::SizeOf;


LqiDesc::LqiDesc() : MatchDesc("lqi", "LQI") {}

LqiDesc::ptr LqiDesc::instance;

LqiDesc::ptr LqiDesc::Instance()
{
        if (!instance)
                instance = ptr(new LqiDesc);
        return instance;
}

Parser::Entity::ptr LqiDesc::parser() const
{
        return LqiParser::New(instance);
}

LqiParser::LqiParser(MatchDesc::cptr m) : MatchParser(m)
{
	Builder reqlen = Builder("--minlen") + (reqlenParser = Parser::NumberTerminal<uint32_t>::New());
	Builder type = Builder("--type") + (typeParser = Parser::NumberTerminal<uint32_t>::New());

	sequenceAdd(reqlen * type);
}

LqiParser::ptr LqiParser::New(MatchDesc::cptr m)
{
	return ptr(new LqiParser(m));
}

Parser::Entity::ptr LqiParser::_clone() const
{
	return Entity::ptr(new LqiParser(getDescription()));
}

Match::ptr LqiParser::compile(bool negate) const
{
        uint32_t reqlen = reqlenParser->get();
	uint32_t type = typeParser->get();

	return Lqi::New(getDescription(), negate, reqlen, type);
}

Lqi::Lqi(MatchDesc::cptr m, bool negate, uint32_t reqlen, uint32_t type) :
	Match(m, negate), reqlen(reqlen), type(type) {}

Lqi::ptr Lqi::New(MatchDesc::cptr m, bool negate, uint32_t reqlen, uint32_t type)
{
	return ptr(new Lqi(m, negate, reqlen, type));
}

void Lqi::_print(std::ostream& stream) const
{
	stream << "--minlen " << std::dec << uint32_t(reqlen) << " --type " << std::dec << uint32_t(type);
}

void Lqi::_size(Platform::Size& config) const
{
	config = config + SizeOf<uint32_t>() + SizeOf<uint32_t>();
}

void Lqi::_write(Stream& config) const
{
	config.write<uint32_t>(reqlen);
	config.write<uint32_t>(type);
}

}
}
