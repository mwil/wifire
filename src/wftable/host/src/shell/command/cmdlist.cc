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

#include "cmdlist.h"
#include "../wfshell.h"
#include <parser/builder.h>
#include <iostream>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>

namespace WiFire {

using Parser::Builder;

ListCmd::ListCmd() : Command("list")
{
	nameParser = Parser::StringTerminal::New();

	optNameParser = Parser::as<Parser::Option>(-Builder(nameParser));

	Builder chains = Builder("chains");
	Builder chain = Builder("chain") + optNameParser;

	optChains = chains;
	optChain = chain;

	sequenceAdd(chains | chain);
}

Parser::Entity::ptr ListCmd::_clone() const {
	return Entity::ptr(new ListCmd);
}

void ListCmd::action()
{
	const std::string& name = nameParser->get();

	Entity::ptr c = Parser::as<Parser::Choice>(at(1))->get();

	if (c == optChains) {
		std::vector<Chain::ptr> chains = Shell::Instance()->getChains();
		std::cout << "Chains: ";
		std::for_each(chains.begin(), chains.end() - 1,
			boost::lambda::var(std::cout) << boost::lambda::bind(&Chain::getName, *boost::lambda::_1) << ", ");
		if (!chains.empty())
			std::cout << (*(chains.end() - 1))->getName() << std::endl;
	} else if (c == optChain) {
		Chain::ptr chain;
		if (optNameParser->hasMatched())
			chain = Shell::Instance()->getChain(name);
		else
			chain = Shell::Instance()->getCurrentChain();
		if (chain)
			chain->print(std::cout);
		else
			std::cout << "Error: Chain \"" << name << "\" does not exist" << std::endl;
	}
}

}
