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

#include "frametype.h"

#include <parser/builder.h>

namespace WiFire {
namespace Matches {

using Parser::Builder;
using Platform::SizeOf;

FrameTypeDesc::FrameTypeDesc() : MatchDesc("type", "Frame Type") {}

FrameTypeDesc::ptr FrameTypeDesc::instance;

FrameTypeDesc::ptr FrameTypeDesc::Instance()
{
	if (!instance)
		instance = ptr(new FrameTypeDesc);
	return instance;
}

Parser::Entity::ptr FrameTypeDesc::parser() const
{
	return FrameTypeParser::New();
}

FrameTypeParser::FrameTypeParser() : MatchParser(FrameTypeDesc::Instance())
{
	Builder beacon = Builder("--beacon");
	Builder data = Builder("--data");
	Builder ack = Builder("--ack");
	Builder control = Builder("--control");

	optBeacon = beacon;
	optData = data;
	optAck = ack;
	optControl = control;

	sequenceAdd(beacon | data | ack | control);
}

FrameTypeParser::ptr FrameTypeParser::New()
{
	return ptr(new FrameTypeParser);
}

Parser::Entity::ptr FrameTypeParser::_clone() const
{
	return Entity::ptr(new FrameTypeParser);
}

Match::ptr FrameTypeParser::compile(bool negate) const
{
	Entity::ptr c = Parser::as<Parser::Choice>(at(1))->get();

	if (c == optAck)
		return FrameType::NewAck(negate);
	else if (c == optBeacon)
		return FrameType::NewBeacon(negate);
	else if (c == optData)
		return FrameType::NewData(negate);
	return Match::ptr();
}

FrameType::FrameType(bool negate, uint8_t frameType) : Match(FrameTypeDesc::Instance(), negate),
	frameType(frameType) {}

FrameType::ptr FrameType::NewBeacon(bool negate)
{
	return ptr(new FrameType(negate, 0));
}

FrameType::ptr FrameType::NewData(bool negate)
{
	return ptr(new FrameType(negate, 1));
}

FrameType::ptr FrameType::NewAck(bool negate)
{
	return ptr(new FrameType(negate, 2));
}

FrameType::ptr FrameType::NewControl(bool negate)
{
	return ptr(new FrameType(negate, 3));
}


void FrameType::_print(std::ostream& stream) const
{
	switch (frameType) {
	case 0:
		stream << "--beacon";
		break;
	case 1:
		stream << "--data";
		break;
	case 2:
		stream << "--ack";
		break;
	case 3:
		stream << "--control";
	}
}

void FrameType::_size(Platform::Size& config) const
{
	config = config + SizeOf<uint8_t>();
}

void FrameType::_write(Stream& config) const
{
	config.write<uint8_t>(frameType);
}

}
}
