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

#include "wfusrp.h"
#include <net/wfcommands.h>
#include "wfusrperr.h"
#include <netinet/in.h>
#include <shell/wfshell.h>
#include <iostream>
#include <iomanip>
#include <boost/bind.hpp>
#include <boost/mem_fn.hpp>
#include <algorithm>

namespace WiFire {

USRP::ptr USRP::instance;

USRP::ptr USRP::Instance()
{
	if (!instance)
		instance = ptr(new USRP);
	return instance;
}

USRP::USRP() : usrp(new Connection) {}

void USRP::registerHostMatch(MatchDesc::ptr match)
{
	hostMatches.push_back(match);
}

const std::vector<MatchDesc::ptr>& USRP::getHostMatches() const
{
	return hostMatches;
}

void USRP::connect(const std::string& target)
{
	usrp->connect(target);
	createConfigMemory();
	connectMatches();
	//config = Memory(configSize = 1024, 0xdead);
}

bool USRP::isConnected() const
{
	//return true;
	return usrp->isConnected();
}

void USRP::recv(Stream& stream)
{
	if (usrp->recv(stream, 1) == false) {
		usrp->disconnect();
		throw USRPException("Timeout when reading");
	}
}

void USRP::createConfigMemory()
{
	Memory mem(4);
	Stream cmd(mem, 0);
	cmd.write<uint32_t>(WF_CMD_WIFIRE_CONFIG_SIZE);
	usrp->send(cmd);

	mem = Memory(12);
	Stream rep(mem, 0);
	recv(rep);
	if (rep.read<uint32_t>() != WF_REP_WIFIRE_CONFIG_SIZE)
		throw USRPException("Got wrong reply from USRP");
	rep.read(configSize);
	Platform::ptr configBase = rep.read<Platform::ptr>();
	config = Memory(configSize, configBase);
}

Memory& USRP::getMemory()
{
	return config;
}

void USRP::sendMemory()
{
	Memory mem(config.getSize() + 8);
	mem.copyFrom(config, 8);
	Stream cmd(mem, 0);
	cmd.write<uint32_t>(WF_CMD_WIFIRE_CONFIGURE);
	cmd.write<uint32_t>(config.getSize());
	usrp->send(cmd);
}

struct PlatformMatchDesc {
	char name[20];
	Platform::ptr self;
	Platform::ptr fun;
} __attribute__((aligned(4)));

void USRP::connectMatches()
{
	using boost::bind;

	Memory mem(4);
	Stream cmd(mem, 0);
	cmd.write<uint32_t>(WF_CMD_WIFIRE_MATCHES);
	usrp->send(cmd);

	mem = Memory(4 * 2048);
	Stream rep(mem, 0);
	recv(rep);
	if (rep.read<uint32_t>() != WF_REP_WIFIRE_MATCHES)
		throw USRPException("Got wrong reply from USRP");
	uint32_t count = rep.read<uint32_t>();
	PlatformMatchDesc * platformMatches = reinterpret_cast<PlatformMatchDesc*>(rep.getRaw());
	std::for_each(hostMatches.begin(), hostMatches.end(), boost::mem_fn(&MatchDesc::disconnect));
	for (uint32_t i = 0; i < count; ++i) {
		PlatformMatchDesc * match = platformMatches + i;
		std::vector<MatchDesc::ptr>::iterator desc =
			std::find_if(hostMatches.begin(), hostMatches.end(), bind(&MatchDesc::getDescription, _1) == match->name);
		if (desc == hostMatches.end()) {
			std::cout << "  Warning: Could not find host match for description \"" << match->name << "\"" << std::endl;
		} else {
			(*desc)->connect(htonl(match->self), htonl(match->fun));
		}
	}
}

}
