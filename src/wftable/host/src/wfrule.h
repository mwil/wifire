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

#include <vector>
#include <ostream>
#include <boost/shared_ptr.hpp>
#include <cstdlib>
#include "wftarget.h"
#include "wfmatch.h"
#include "platform/wfmemory.h"

namespace WiFire {

/** \brief Rule (essentially some matches and a target) */
class Rule {
public:
	/// pointer type
	typedef boost::shared_ptr<Rule> ptr;
	/// const pointer type
	typedef boost::shared_ptr<const Rule> cptr;

private:
	/// matches
	std::vector<Match::ptr> matches;
	/// target
	Target::ptr target;

private:
	/** construct a new rule
	 * \param matches matches
	 * \param target target
	 */
	Rule(const std::vector<Match::ptr>& matches, Target::ptr target);

public:
	/** create a new rule
	 * \param matches matches
	 * \param target target
	 * \return new rule
	 */
	static ptr New(const std::vector<Match::ptr>& matches, Target::ptr target);

	/** pretty print rule
	 * \param stream stream to be written to
	 */
	void print(std::ostream& stream) const;

	/** get target
	 * \return target
	 */
	Target::ptr getTarget() const;

	/** get size of translated rule
	 * \param size of rule in chain memory
	 * \param size of rule in config memory
	 */
	void size(Platform::Size& chain, Platform::Size& config) const;

	/** translate rule
	 * \param chain chain memory
	 * \param config config memory
	 */
	void write(Stream& chain, Stream& config) const;
};

}
