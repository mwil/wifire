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

#include "cmdchain.h"
#include "../wfshell.h"

#include <iostream>

namespace WiFire {

ChainCmd::ChainCmd() : Command("chain")
{
	sequenceAdd(nameParser = Parser::StringTerminal::New());
}

Parser::Entity::ptr ChainCmd::_clone() const {
	return Entity::ptr(new ChainCmd);
}

void ChainCmd::action()
{
	const std::string& name = nameParser->get();

	Shell::Instance()->setCurrentChain(name);
}

}
