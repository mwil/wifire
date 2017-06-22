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

#include "platform/wfusrp.h"
#include "matches/address.h"
#include "matches/frametype.h"
#include "matches/probability.h"
#include "matches/ctp.h"
#include "matches/lqi.h"
#include "shell/wfshell.h"
#include "shell/command/cmdadd.h"
#include "shell/command/cmddelete.h"
#include "shell/command/cmdlist.h"
#include "shell/command/cmdchain.h"
#include "shell/command/cmdexit.h"
#include "shell/command/cmdconnect.h"
#include "shell/command/cmdcommit.h"
#include "shell/command/cmdpolicy.h"
#include "shell/command/cmdinsert.h"
#include "shell/command/cmdflush.h"

using namespace WiFire;

/** \brief entry point
 * \param argc argument count
 * \param argv arguments
 *
 * \return 0
 */
int main(int argc, char * argv[]) {
	/* get shell and usrp */
	Shell::ptr shell(Shell::Instance());
	USRP::ptr usrp(USRP::Instance());

	/* register matches */
	usrp->registerHostMatch(Matches::SourceDesc::Instance());
	usrp->registerHostMatch(Matches::DestinationDesc::Instance());
	usrp->registerHostMatch(Matches::FrameTypeDesc::Instance());
	usrp->registerHostMatch(Matches::ProbabilityDesc::Instance());
	usrp->registerHostMatch(Matches::CtpDesc::Instance());
	usrp->registerHostMatch(Matches::LqiDesc::Instance());

	/* register commands */
	shell->addCommand(Command::ptr(new AddCmd));
	shell->addCommand(Command::ptr(new DeleteCmd));
	shell->addCommand(Command::ptr(new ListCmd));
	shell->addCommand(Command::ptr(new ChainCmd));
	shell->addCommand(Command::ptr(new ExitCmd));
	shell->addCommand(Command::ptr(new ConnectCmd));
	shell->addCommand(Command::ptr(new CommitCmd));
	shell->addCommand(Command::ptr(new PolicyCmd));
	shell->addCommand(Command::ptr(new InsertCmd));
	shell->addCommand(Command::ptr(new FlushCmd));

	/* main loop */
	shell->start();

	return 0;
}
