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

#pragma once

#include <wfmatch.h>
#include <parser/numberterminal.h>
#include <inttypes.h>
#include <ostream>

namespace WiFire {
namespace Matches {

class SourceDesc : public MatchDesc {
public:
	typedef boost::shared_ptr<SourceDesc> ptr;
	typedef boost::shared_ptr<SourceDesc> cptr;

private:
	SourceDesc();

	static ptr instance;

public:
	static ptr Instance();

public:
	Parser::Entity::ptr parser() const;
};

class DestinationDesc : public MatchDesc {
public:
	typedef boost::shared_ptr<DestinationDesc> ptr;
	typedef boost::shared_ptr<DestinationDesc> cptr;

private:
	DestinationDesc();

	static ptr instance;

public:
	static ptr Instance();

public:
	Parser::Entity::ptr parser() const;
};

class AddressParser : public MatchParser {
public:
	typedef boost::shared_ptr<AddressParser> ptr;
	typedef boost::shared_ptr<AddressParser> cptr;

private:
	Parser::NumberTerminal<uint16_t>::ptr modeParser;
	Parser::NumberTerminal<uint16_t>::ptr panParser;
	Parser::NumberTerminal<uint64_t>::ptr addrParser;

	AddressParser(MatchDesc::cptr m);

public:
	static ptr New(MatchDesc::cptr m);

protected:
	Entity::ptr _clone() const;

public:
	Match::ptr compile(bool negate) const;
};

class Address : public Match {
public:
	typedef boost::shared_ptr<Address> ptr;
	typedef boost::shared_ptr<const Address> cptr;

private:
	uint8_t mode;
	uint16_t pan;
	uint64_t addr;

private:
	Address(MatchDesc::cptr m, bool negate, uint8_t mode, uint16_t pan, uint64_t addr);

public:
	static ptr New(MatchDesc::cptr m, bool negate, uint8_t mode, uint16_t pan, uint64_t addr);

protected:
	void _print(std::ostream& stream) const;
	void _write(Stream& config) const;
	void _size(Platform::Size& config) const;
};

}
}
