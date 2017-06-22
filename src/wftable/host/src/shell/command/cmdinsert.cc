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

#include "cmdinsert.h"
#include <boost/bind.hpp>
#include <parser/builder.h>
#include <parser/repetition.h>
#include <wfrule.h>
#include <wfmatch.h>
#include <wftarget.h>
#include "../wfshell.h"

#include <iostream>

namespace WiFire {

using Parser::Builder;

InsertCmd::InsertCmd() : Command("insert")
{
	sequenceAdd(ruleParser = Parser::NumberTerminal<std::size_t>::New());
	sequenceAdd(*(Builder("-m") + Builder(-Builder("!") + MatchShell::New())["shell"]));
	sequenceAdd(Builder("-j") + Builder(targetParser = TargetShell::New())["target"]);
}

Parser::Entity::ptr InsertCmd::_clone() const {
	return Entity::ptr(new InsertCmd);
}

void InsertCmd::action()
{
	/* chain */
	Chain::ptr chain = Shell::Instance()->getCurrentChain();

	/* rule number */
	std::size_t ruleNum = ruleParser->get();

	if (ruleNum == 0) {
		std::cout << "Rule index too small, begin counting at 1" << std::endl;
		return;
	}

	ruleNum--;
	if (ruleNum > chain->getRuleCount())
		ruleNum = chain->getRuleCount();

	/* get target */
	Target::ptr target = targetParser->compile();

	/* sanity check: we won't jump into a top level chain */
	if (target->getVerdict() == WF_JMP && target->getTargetChain()->isTopLevel()) {
		std::cout << "Cannot add rule, because jumping into a top level chain is not allowed" << std::endl;
		return;
	}

	if (chain->isTopLevel()) {
		switch (target->getVerdict()) {
		case WF_ACCEPT:
		case WF_JAM:
		case WF_JMP:
			break;
		default:
			std::cout << "Target not allowed for top level chains" << std::endl;
			return;
		}
	}

	/* sanity check: would that rule end in a loop? */
	if (chain->wouldLoop(target)) {
		std::cout << "Cannot add rule, because this would introduce a jump loop" << std::endl;
		return;
	}

	/* collect matches */
	std::vector<Match::ptr> matches;
	std::vector<Sequence::ptr> x =
		Parser::as<Parser::Repetition>(at(2))->getNamedEntities<Sequence>("shell");

	/* following code breaks clang ;)
	 * std::transform(x.begin(), x.end(), std::back_inserter(matches),
		boost::bind(&MatchParser::compile,
		boost::bind(&Parser::as<MatchParser>, boost::bind(&MatchShell::get, _1))));*/
	for (std::vector<Sequence::ptr>::iterator it = x.begin(); it != x.end(); ++it) {
		MatchShell::ptr ms = Parser::as<MatchShell>((*it)->at(1));
		bool negate = Parser::as<Parser::Option>((*it)->at(0))->hasMatched();
		MatchParser::ptr mp = Parser::as<MatchParser>(ms->get());
		matches.push_back(mp->compile(negate));
	}

	/* compile into rule */
	Rule::ptr rule = Rule::New(matches, target);

	/* insert rule */
	chain->insertRule(ruleNum, rule);
}

}
