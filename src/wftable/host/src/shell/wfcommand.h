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

#include <string>
#include <boost/utility.hpp>
#include <parser/sequence.h>

namespace WiFire {

class Command : public Parser::Sequence {
private:
	std::string name;

public:
	Command(const std::string& name);

	const std::string& getName() const;

	typedef boost::shared_ptr<Command> ptr;
	typedef boost::shared_ptr<const Command> cptr;
};

}
