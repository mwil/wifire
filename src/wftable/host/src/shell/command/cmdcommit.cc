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

#include "cmdcommit.h"
#include "../wfshell.h"
#include <platform/wfusrp.h>
#include <shell/wfshell.h>
#include <iostream>
#include <wfnotconnected.h>

namespace WiFire {

CommitCmd::CommitCmd() : Command("commit") {}

Parser::Entity::ptr CommitCmd::_clone() const
{
	return Entity::ptr(new CommitCmd);
}

void CommitCmd::action()
{
	USRP::ptr usrp = USRP::Instance();

	if (!usrp->isConnected()) {
		std::cout << "Error: You have to connect first" << std::endl;
		return;
	}

	Memory& mem = usrp->getMemory();
	std::vector<Chain::ptr> chains = Shell::Instance()->getChains();

	Platform::Size chainSize, configSize;
	for (std::vector<Chain::ptr>::iterator it = chains.begin(); it != chains.end(); ++it) {
		(*it)->setLocation(Stream(mem, chainSize, false));
		(*it)->size(chainSize, configSize);
	}

	if (chainSize + configSize > mem.getSize()) {
		std::cout << "Cannot compile all Chains/Rules/Matches into memory" << std::endl;
		std::cout << " Requested Size: " << (chainSize + configSize) << ", Available Space: " << mem.getSize() << std::endl;
		return;
	}

	Stream chainStream(mem, 0, true);
	Stream configStream(chainStream + chainSize);
	try {
		std::for_each(chains.begin(), chains.end(), boost::bind(&Chain::write, _1, boost::ref(chainStream), boost::ref(configStream)));
	} catch(NotConnected& nc) {
		std::cout << "Could not write remote memory: " << nc.what() << std::endl;
	}

	usrp->sendMemory();
}

}
