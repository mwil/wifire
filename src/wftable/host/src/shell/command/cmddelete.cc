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

#include "cmddelete.h"
#include <parser/builder.h>
#include <wfrule.h>
#include <wfmatch.h>
#include <wftarget.h>
#include "../wfshell.h"

#include <iostream>

namespace WiFire {

using Parser::Builder;

DeleteCmd::DeleteCmd() : Command("del")
{
	Builder chain = (Builder("chain") + (chainParser = Parser::StringTerminal::New()));
	Builder rule = (Builder("rule") + (ruleParser = Parser::NumberTerminal<std::size_t>::New()));

	chainOpt = chain;
	ruleOpt = rule;

	sequenceAdd(chain | rule);
}

Parser::Entity::ptr DeleteCmd::_clone() const {
	return Entity::ptr(new DeleteCmd);
}

void DeleteCmd::action()
{
	Entity::ptr c = Parser::as<Parser::Choice>(at(1))->get();
	Shell::ptr s = Shell::Instance();

	if (c == chainOpt) {
		const std::string& name = chainParser->get();
		Chain::ptr chain = s->getChain(name);
		if (!chain) {
			std::cout << "Chain \"" << name << "\" does not exist" << std::endl;
		} else if (chain->isTopLevel()) {
			std::cout << "Chain \"" << name << "\" is a toplevel chain" << std::endl;
		} else if (chain->inUse()) {
			std::cout << "Chain \"" << name << "\" is in use" << std::endl;
		} else {
			if (s->getCurrentChain() == chain)
				s->setCurrentMainChain();
			s->deleteChain(chain);
		}
	} else if (c == ruleOpt) {
		std::size_t r = ruleParser->get();
		Chain::ptr chain = s->getCurrentChain();

		if (r == 0) {
			std::cout << "Rule index too small, begin counting at 1" << std::endl;
			return;
		}

		Rule::ptr rule = chain->getRule(r - 1);
		if (!rule)
			std::cout << "Rule index too large, cannot find rule" << std::endl;
		else
			chain->deleteRule(rule);
	}
}

}
