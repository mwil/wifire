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

#include "probability.h"

#include <parser/builder.h>
#include <parser/numberterminal.h>
#include <iomanip>

namespace WiFire {
namespace Matches {

using Parser::Builder;
using Platform::SizeOf;


ProbabilityDesc::ProbabilityDesc() : MatchDesc("probability", "Probability") {}

ProbabilityDesc::ptr ProbabilityDesc::instance;

ProbabilityDesc::ptr ProbabilityDesc::Instance()
{
        if (!instance)
                instance = ptr(new ProbabilityDesc);
        return instance;
}

Parser::Entity::ptr ProbabilityDesc::parser() const
{
        return ProbabilityParser::New(instance);
}

ProbabilityParser::ProbabilityParser(MatchDesc::cptr m) : MatchParser(m)
{
	Builder mode = Builder("--probmode") + (modeParser = Parser::NumberTerminal<uint32_t>::New());
	Builder prob = Builder("--prob") + (probParser = Parser::NumberTerminal<uint32_t>::New());

	sequenceAdd(mode * prob);
}

ProbabilityParser::ptr ProbabilityParser::New(MatchDesc::cptr m)
{
	return ptr(new ProbabilityParser(m));
}

Parser::Entity::ptr ProbabilityParser::_clone() const
{
	return Entity::ptr(new ProbabilityParser(getDescription()));
}

Match::ptr ProbabilityParser::compile(bool negate) const
{
        uint32_t mode = modeParser->get();
	uint32_t prob = probParser->get();

	return Probability::New(getDescription(), negate, mode, prob);
}

Probability::Probability(MatchDesc::cptr m, bool negate, uint32_t mode, uint32_t prob) :
	Match(m, negate), mode(mode), prob(prob) {}

Probability::ptr Probability::New(MatchDesc::cptr m, bool negate, uint32_t mode, uint32_t prob)
{
	return ptr(new Probability(m, negate, mode, prob));
}

void Probability::_print(std::ostream& stream) const
{
	stream << "--probmode " << std::dec << uint32_t(mode) << " --prob " << std::dec << uint32_t(prob);
}

void Probability::_size(Platform::Size& config) const
{
	config = config + SizeOf<uint32_t>() + SizeOf<uint32_t>();
}

void Probability::_write(Stream& config) const
{
	config.write<uint32_t>(mode);
	config.write<uint32_t>(prob);
}

}
}
