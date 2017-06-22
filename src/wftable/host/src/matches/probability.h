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

#include <wfmatch.h>
#include <parser/numberterminal.h>
#include <inttypes.h>
#include <ostream>

namespace WiFire {
namespace Matches {

class ProbabilityDesc : public MatchDesc {
public:
	typedef boost::shared_ptr<ProbabilityDesc> ptr;
	typedef boost::shared_ptr<ProbabilityDesc> cptr;

private:
	ProbabilityDesc();

	static ptr instance;

public:
	static ptr Instance();

public:
	Parser::Entity::ptr parser() const;
};


class ProbabilityParser : public MatchParser {
public:
	typedef boost::shared_ptr<ProbabilityParser> ptr;
	typedef boost::shared_ptr<ProbabilityParser> cptr;

private:
	Parser::NumberTerminal<uint32_t>::ptr modeParser;
	Parser::NumberTerminal<uint32_t>::ptr probParser;

	ProbabilityParser(MatchDesc::cptr m);

public:
	static ptr New(MatchDesc::cptr m);

protected:
	Entity::ptr _clone() const;

public:
	Match::ptr compile(bool negate) const;
};

class Probability : public Match {
public:
	typedef boost::shared_ptr<Probability> ptr;
	typedef boost::shared_ptr<const Probability> cptr;

private:
	uint32_t mode;
	uint32_t prob;

private:
	Probability(MatchDesc::cptr m, bool negate, uint32_t mode, uint32_t prob);

public:
	static ptr New(MatchDesc::cptr m, bool negate, uint32_t mode, uint32_t prob);

protected:
	void _print(std::ostream& stream) const;
	void _write(Stream& config) const;
	void _size(Platform::Size& config) const;
};

}
}
