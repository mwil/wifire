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
#include <vector>
#include <boost/shared_ptr.hpp>
#include <parser/choice.h>
#include <parser/top.h>
#include "wfcommand.h"
#include <wfchain.h>

namespace WiFire {

class Shell : public Parser::Top<Parser::Choice> {
public:
	typedef boost::shared_ptr<Shell> ptr;
	typedef boost::shared_ptr<const Shell> cptr;

	class Exit {};

private:
	static ptr self;

	char * _line;
	std::string line;
	std::string oldline;
	std::size_t preparedReadlinePos;

	Chain::ptr mainChain;
	Chain::ptr currentChain;
	std::vector<Chain::ptr> chains;

	std::string prompt;

	Shell();
public:
	virtual ~Shell();

	void start();

	void addCommand(Command::ptr cmd);

	Entity::ptr _clone() const;

	Chain::ptr getCurrentChain();
	Chain::ptr getChain(const std::string& name);
	void deleteChain(Chain::ptr chain);
	std::vector<Chain::ptr> getChains();
	void setCurrentMainChain();
	void setCurrentChain(Chain::ptr chain);
	void setCurrentChain(const std::string& name);

	static ptr Instance();

private:
	bool read();
	static char ** _complete(const char * text, int start, int end);

	static int _prepareReadline();
	int prepareReadline() const;
	void prepareReadlinePos(std::size_t pos);
};

}
