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
#include <vector>
#include <string>
#include <ostream>
#include <cstddef>
#include "wfverdict.h"
#include "wfrule.h"
#include "platform/wfmemory.h"

namespace WiFire {

/** \brief Chain (essentially: set of rules and a default policy) */
class Chain {
public:
	/// pointer type
	typedef boost::shared_ptr<Chain> ptr;
	/// const pointer type
	typedef boost::shared_ptr<const Chain> cptr;

private:
	/// chain name
	std::string name;
	/// chain policy
	Verdict policy;
	/// chain location on target (used for translation to target memory)
	Platform::ptr location;
	/// whether that chain is a top level chain
	bool topLevel;
	/// usage count (by targets)
	std::size_t usageCount;

	/// rules of the chain
	std::vector<Rule::ptr> rules;

	/** construct a chain
	 * \param name chain name
	 * \param policy default policy
	 * \param topLevel whether that chain is a toplevel chain
	 */
	Chain(const std::string& name, Verdict policy, bool topLevel);

public:
	/** create a new chain
	 * \param name chain name
	 * \return new chain
	 */
	static ptr New(const std::string& name);

	/** create a new toplevel chain
	 * \param name chain name
	 * \param policy default policy
	 * \return new chain
	 */
	static ptr NewTop(const std::string& name, Verdict policy);

	/** add a new rule to chain
	 * \param rule rule rule to be added
	 */
	void addRule(Rule::ptr rule);

	/** delete a rule from a chain
	 * \param rule rule to be deleted
	 */
	void deleteRule(Rule::ptr rule);

	/** get a rule by its index
	 * \param i index
	 * \return rule or null pointer if it does not exist
	 */
	Rule::ptr getRule(std::size_t i);

	/** get number of rules in chain
	 * \return number of rules
	 */
	std::size_t getRuleCount() const;

	/** delete all rules from chain */
	void flush();

	/** insert a rule
	 * \param i location
	 * \param rule rule to be inserted
	 */
	void insertRule(std::size_t i, Rule::ptr rule);

	/** increase reference count */
	void ref();

	/** decrease reference count */
	void unref();

	/** whether chain is in use
	 * \return true iff chain is in use
	 */
	bool inUse() const;

	/** whether chain is a toplevel chain
	 * \return true iff chain is toplevel chain
	 */
	bool isTopLevel() const;

	/** set policy
	 * \param v policy
	 */
	void setPolicy(Verdict v);

	/** get chain name
	 * \return chain name
	 */
	const std::string& getName() const;

	/** pretty print chain definition
	 * \param stream stream to be written to
	 */
	void print(std::ostream& stream) const;

	/** get size of translated chain
	 * \param size of chain in chain memory
	 * \param size of chain in config memory
	 */
	void size(Platform::Size& chain, Platform::Size& config) const;

	/** translate chain
	 * \param chain chain memory
	 * \param config config memory
	 */
	void write(Stream& chain, Stream& config) const;

	/** get chains location in target memory
	 * \return location of chain in target memory
	 */
	Platform::ptr getLocation() const;

	/** set chains location in target memory
	 * \param stream pointer to chains location
	 */
	void setLocation(const Stream& stream);

	/** whether adding a rule with a jump target would result in a loop
	 * \param t target of rule
	 * \return true iff there was a loop
	 */
	bool wouldLoop(Target::ptr t) const;
};

}
