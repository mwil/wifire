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
#include <inttypes.h>
#include <ostream>

namespace WiFire {
namespace Matches {

class FrameTypeDesc : public MatchDesc {
public:
	typedef boost::shared_ptr<FrameTypeDesc> ptr;
	typedef boost::shared_ptr<FrameTypeDesc> cptr;

private:
	FrameTypeDesc();

	static ptr instance;

public:
	static ptr Instance();

public:
	Parser::Entity::ptr parser() const;
};

class FrameTypeParser : public MatchParser {
public:
	typedef boost::shared_ptr<FrameTypeParser> ptr;
	typedef boost::shared_ptr<FrameTypeParser> cptr;

private:
	Entity::ptr optBeacon;
	Entity::ptr optData;
	Entity::ptr optAck;
	Entity::ptr optControl;

	FrameTypeParser();

public:
	static ptr New();

protected:
	Entity::ptr _clone() const;

public:
	Match::ptr compile(bool negate) const;
};

class FrameType : public Match {
public:
	typedef boost::shared_ptr<FrameType> ptr;
	typedef boost::shared_ptr<const FrameType> cptr;

private:
	uint8_t frameType;

private:
	FrameType(bool negate, uint8_t frameType);

public:
	static ptr NewBeacon(bool negate);
	static ptr NewData(bool negate);
	static ptr NewAck(bool negate);
	static ptr NewControl(bool negate);

protected:
	void _print(std::ostream& stream) const;
	void _write(Stream& config) const;
	void _size(Platform::Size& config) const;
};

}
}
