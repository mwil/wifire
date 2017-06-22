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

#include "wfcompletor.h"
#include "wfshell.h"
#include "wfcommand.h"
#include <readline/readline.h>

namespace WiFire {

Completor::Completor() {}

Completor::~Completor() {}

void Completor::prepare()
{
	self = this;
}

Completor * Completor::self(NULL);

char * Completor::complete(const char * text, int state)
{
	if (state == 0) {
		::rl_attempted_completion_over = true;
		self->init(text);
	}
	return self->operator()(text);
}

void Completor::init(const char * text) {}

char * Completor::operator ()(const char * text)
{
	return NULL;
}

FileCompletor::FileCompletor() {}

}
