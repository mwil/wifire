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

#include "cmdconnect.h"
#include "../wfshell.h"
#include <platform/wfusrp.h>
#include <iostream>

namespace WiFire {

ConnectCmd::ConnectCmd() : Command("connect")
{
	sequenceAdd(targetParser = Parser::StringTerminal::New());
}

Parser::Entity::ptr ConnectCmd::_clone() const {
	return Entity::ptr(new ConnectCmd);
}

void ConnectCmd::action()
{
	const std::string& target = targetParser->get();

	std::cout << "Connecting to " << target << std::endl;

	try {
		USRP::ptr usrp(USRP::Instance());
		usrp->connect(target);

	} catch(std::exception& e) {
		std::cout << "Could not connect: " << e.what() << std::endl;
	}

}

}
