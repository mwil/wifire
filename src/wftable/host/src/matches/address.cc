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

#include "address.h"

#include <parser/builder.h>
#include <parser/numberterminal.h>
#include <iomanip>

namespace WiFire {
namespace Matches {

using Parser::Builder;
using Platform::SizeOf;

SourceDesc::SourceDesc() : MatchDesc("src", "Source Address") {}

SourceDesc::ptr SourceDesc::instance;

SourceDesc::ptr SourceDesc::Instance()
{
	if (!instance)
		instance = ptr(new SourceDesc);
	return instance;
}

Parser::Entity::ptr SourceDesc::parser() const
{
	return AddressParser::New(instance);
}

DestinationDesc::DestinationDesc() : MatchDesc("dst", "Destination Address") {}

DestinationDesc::ptr DestinationDesc::instance;

DestinationDesc::ptr DestinationDesc::Instance()
{
	if (!instance)
		instance = ptr(new DestinationDesc);
	return instance;
}

Parser::Entity::ptr DestinationDesc::parser() const
{
	return AddressParser::New(instance);
}

AddressParser::AddressParser(MatchDesc::cptr m) : MatchParser(m)
{
	Builder mode = Builder("--mode") + (modeParser = Parser::NumberTerminal<uint16_t>::New());
	Builder pan = Builder("--pan") + (panParser = Parser::NumberTerminal<uint16_t>::New());
	Builder addr = Builder("--addr") + (addrParser = Parser::NumberTerminal<uint64_t>::New());

	sequenceAdd(mode * pan * addr);
}

AddressParser::ptr AddressParser::New(MatchDesc::cptr m)
{
	return ptr(new AddressParser(m));
}

Parser::Entity::ptr AddressParser::_clone() const
{
	return Entity::ptr(new AddressParser(getDescription()));
}

Match::ptr AddressParser::compile(bool negate) const
{
	uint16_t mode = modeParser->get();
	uint16_t pan = panParser->get();
	uint64_t addr = addrParser->get();

	switch(mode) {
	case 0:
		break;
	case 1:
		throw modeParser->semanticError("Mode 1 is unknown");
	case 2:
		if (addr > 0xffff)
			throw addrParser->semanticError("Address is too long for mode 2");
		break;
	case 3:
		break;
	default:
		throw modeParser->semanticError("Mode must be between 0 and 3");
	}

	return Address::New(getDescription(), negate, mode, pan, addr);
}

Address::Address(MatchDesc::cptr m, bool negate, uint8_t mode, uint16_t pan, uint64_t addr) :
	Match(m, negate), mode(mode), pan(pan), addr(addr) {}

Address::ptr Address::New(MatchDesc::cptr m, bool negate, uint8_t mode, uint16_t pan, uint64_t addr)
{
	return ptr(new Address(m, negate, mode, pan, addr));
}

void Address::_print(std::ostream& stream) const
{
	stream << "--mode " << std::dec << uint16_t(mode) <<
			" --pan 0x" << std::hex << std::setw(4) << std::right << std::setfill('0') << pan <<
			" --addr 0x" << std::hex << std::setw(mode == 3 ? 16 : 4) << std::right << std::setfill('0') << addr << std::dec;
}

void Address::_size(Platform::Size& config) const
{
	config = config + SizeOf<uint8_t>() + SizeOf<uint16_t>() + SizeOf<uint64_t>();
}

void Address::_write(Stream& config) const
{
	config.write<uint8_t>(mode);
	config.write<uint16_t>(pan);
	config.write<uint64_t>(addr);
}

}
}
