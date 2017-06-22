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

#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>

namespace WiFire {

class Completor : boost::noncopyable {
	friend class Shell;

private:
	static Completor * self;
	void prepare();
	static char * complete(const char * text, int state);

protected:
	virtual void init(const char * text);
	virtual char * operator()(const char * text);

public:
	typedef boost::shared_ptr<Completor> ptr;
	typedef boost::shared_ptr<const Completor> cptr;

	Completor();
	virtual ~Completor();
};

class FileCompletor : public Completor {
	/* this one is special */
public:
	FileCompletor();
};

}
