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

#include <iostream>
#include "wfshell.h"
#include <readline/readline.h>
#include <readline/history.h>
#include <cstdlib>
#include <algorithm>
#include <boost/bind.hpp>
#include "wflistcompletor.h"
#include <parser/syntaxerror.h>
#include <parser/semanticerror.h>
#include <signal.h>

namespace WiFire {

Shell::ptr Shell::self;

Shell::ptr Shell::Instance()
{
	if (!self)
		self = ptr(new Shell);
	return self;
}

Shell::Shell() : _line(NULL), line(), mainChain(Chain::NewTop("main", WF_ACCEPT)),
	currentChain(mainChain)
{
	chains.push_back(mainChain);
	setCurrentChain(mainChain);
	::rl_attempted_completion_function = &_complete;
}

Shell::~Shell()
{
	std::free(_line);
}

Parser::Entity::ptr Shell::_clone() const {
	return Parser::Entity::ptr(self);
}

Chain::ptr Shell::getCurrentChain()
{
	return currentChain;
}

Chain::ptr Shell::getChain(const std::string& name)
{
	std::vector<Chain::ptr>::iterator it =
		std::find_if(chains.begin(), chains.end(), boost::bind(&Chain::getName, _1) == boost::cref(name));
	if (it == chains.end())
		return Chain::ptr();
	else
		return *it;
}

void Shell::deleteChain(Chain::ptr c)
{
	chains.erase(std::remove(chains.begin(), chains.end(), c));
}

std::vector<Chain::ptr> Shell::getChains()
{
	return chains;
}

void Shell::setCurrentMainChain()
{
	setCurrentChain(mainChain);
}

void Shell::setCurrentChain(Chain::ptr chain)
{
	currentChain = chain;
	prompt = currentChain->getName() + "> ";
}

void Shell::setCurrentChain(const std::string& name)
{
	Chain::ptr chain = getChain(name);
	if (!chain) {
		std::cout << "Could not find chain " << name << ", creating new" << std::endl;
		chain = Chain::New(name);
		chains.push_back(chain);
		setCurrentChain(chain);
	} else {
		setCurrentChain(chain);
	}
}

void Shell::prepareReadlinePos(std::size_t pos)
{
	preparedReadlinePos = pos;
	::rl_pre_input_hook = &_prepareReadline;
}

int Shell::_prepareReadline()
{
	return self->prepareReadline();
}

int Shell::prepareReadline() const
{
	::rl_insert_text(line.c_str());
	::rl_point = preparedReadlinePos;
	::rl_redisplay();
	::rl_pre_input_hook = NULL;
	return 0;
}

void Shell::start()
{
	::clear_history();
	::read_history(".history");
	while (read()) {
		std::string line = ::rl_line_buffer;
		try {
			parse(line);
			action();
		} catch (Parser::SyntaxError& se) {
			const Parser::Scanner& s = se.getScanner();
			const std::vector<std::string>& expected = se.getExpected();
			std::cout << "Syntax Error:" << std::endl;
			if (se.isJunk()) {
				s.prettyPrint(std::cout, "  Could not parse junk beginning at");
			} else {
				s.prettyPrint(std::cout, "  Could not parse");
				if (!expected.empty()) {
					std::cout << "  Expected: ";
					std::copy(expected.begin(), expected.end() - 1, std::ostream_iterator<std::string>(std::cout, ", "));
					std::cout << *(expected.end() - 1) << std::endl;
				}
			}
			//prepareReadlinePos(s.getPos() + 1); TODO: experimental
		} catch (Parser::SemanticError& se) {
			const Parser::Scanner& s = se.getScanner();
			const std::string& desc = se.getDesc();
			std::cout << "Semantic Error: " << desc << " at ";
			s.prettyPrint(std::cout, "");
		} catch(Exit&) {
			break;
		}
	}
	::write_history(".history");
	::history_truncate_file(".history", 200);
}

void Shell::addCommand(Command::ptr cmd)
{
	choiceAdd(cmd);
}

class SigInt {};

static void sigint(int sig)
{
	::signal(SIGINT, SIG_DFL);
	throw SigInt();
}

bool Shell::read()
{
	try {
		::signal(SIGINT, &sigint);
		do {
			std::free(_line);
			_line = NULL;
			_line = ::readline(prompt.c_str());
			if (_line && *_line) {
				line = _line;
				if (oldline != line)
					::add_history(_line);
				oldline = line;
				::signal(SIGINT, SIG_DFL);
				return true;
			}
		} while (_line);
		::signal(SIGINT, SIG_DFL);
	} catch (SigInt&) {
		std::cout << std::endl;
	}
	return false;
}

char ** Shell::_complete(const char * text, int start, int end) {
	std::string line = ::rl_line_buffer;

	Parser::Scanner s(line, 0, end);
	std::vector<std::string> f;
	/*Parser::COMPLETE_STATE cs = */self->complete(s, f);

	Completor::ptr c(mk_listcompletor(f));
	c->prepare();
	return ::rl_completion_matches(text, &Completor::complete);
}

}
