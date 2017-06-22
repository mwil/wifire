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

#include "wfrule.h"
#include <boost/bind.hpp>
#include <algorithm>

namespace WiFire {

using Platform::SizeOf;

Rule::Rule(const std::vector<Match::ptr>& matches, Target::ptr target)
	: matches(matches), target(target) {}

Rule::ptr Rule::New(const std::vector<Match::ptr>& matches, Target::ptr target)
{
	return ptr(new Rule(matches, target));
}

void Rule::print(std::ostream& stream) const
{
	/* call print(stream) on each match */
	for (std::vector<Match::ptr>::const_iterator it = matches.begin(); it != matches.end(); ++it) {
		stream << " ";
		(*it)->print(stream);
	}

	/*std::for_each(matches.begin(), matches.end(),
		(boost::lambda::var(stream) << " ",
		boost::lambda::bind(&Match::print, *boost::lambda::_1, boost::lambda::var(stream))));*/

	stream << " ";
	/* print target */
	target->print(stream);
}

Target::ptr Rule::getTarget() const
{
	return target;
}

void Rule::size(Platform::Size& chain, Platform::Size& config) const
{
	chain.align();
	/* target size */
	target->size(chain, config);
	/* rule size (4) + endmatch (4) */
	chain = chain + SizeOf<uint32_t>() + SizeOf<Platform::ptr>();
	/* call size(chain, config) on each match */
	std::for_each(matches.begin(), matches.end(),
		boost::bind(&Match::size, _1, boost::ref(chain), boost::ref(config)));
}

void Rule::write(Stream& chain, Stream& config) const
{
	/* target */
	target->write(chain, config);

	chain.align();

	/* rule size */
	Platform::Size cfg, dummy;
	size(cfg, dummy);
	chain.write<uint32_t>(cfg);

	/* save pointer to current position for endrule */
	Stream s(chain);
	chain.write<Platform::ptr>(0); // dummy

	/* call write(chain, config) on each match */
	std::for_each(matches.begin(), matches.end(),
		boost::bind(&Match::write, _1, boost::ref(chain), boost::ref(config)));

	/* endrule */
	s.write(&chain);
}

}
