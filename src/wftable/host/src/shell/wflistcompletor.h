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

#include <vector>
#include "wfcompletor.h"
#include <cstring>

namespace WiFire {

template<typename T>
struct ID {
	const T& operator()(const T& t) { return t; }
};

template<typename LT, typename P>
class ListCompletor : public Completor {
private:
	const LT& lst;
	P p;
	typename LT::const_iterator it;
	std::size_t len;

public:
	ListCompletor(const LT& lst, const P& p) : lst(lst), p(p) {}

	void init(const char * text)
	{
		it = lst.begin();
		len = std::strlen(text);
	}

	char * operator()(const char * text)
	{
		while (it != lst.end()) {
			const std::string& name = p(*it);
			++it;
			if (name.compare(0, len, text) == 0)
				return ::strdup(name.c_str());
		}

		return NULL;
	}
};

template<typename LT, typename P>
Completor::ptr mk_listcompletor(LT& lt, const P& p)
{
	return Completor::ptr(new ListCompletor<LT,P>(lt, p));
}

template<typename LT>
Completor::ptr mk_listcompletor(LT & lt)
{
	return Completor::ptr(new ListCompletor<LT,ID<std::string>  >(lt, ID<std::string>()));
}

}
