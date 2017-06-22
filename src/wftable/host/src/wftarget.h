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

#include <ostream>
#include <boost/shared_ptr.hpp>
#include <cstddef>
#include "wfverdict.h"
#include <parser/choice.h>
#include <parser/stringterminal.h>
#include "platform/wfmemory.h"
#include <boost/shared_ptr.hpp>

namespace WiFire {

class Chain;

/** \brief Target (essentially a verdict and maybe a jump target) */
class Target {
public:
	/// pointer type
	typedef boost::shared_ptr<Target> ptr;
	/// const pointer type
	typedef boost::shared_ptr<const Target> cptr;

private:
	/// Verdict
	Verdict v;
	/// jump target
	boost::shared_ptr<Chain> jmpTarget;

private:
	/** construct a new target
	 * \param v verdict
	 * \param jmpTarget jump target
	 */
	explicit Target(Verdict v, boost::shared_ptr<Chain> jmpTarget);

public:
	/** destruct target */
	~Target();

	/** \create a new jumping target
	 * \param jmpTarget jump target
	 * \return new target
	 */
	static ptr NewJmp(boost::shared_ptr<Chain> jmpTarget);

	/** \create a new non-jumping target
	 * \param v verdict
	 * \return new target
	 */
	static ptr New(Verdict v);

	/** get verdict
	 * \return verdict
	 */
	Verdict getVerdict() const;

	/** get target chain
	 * \return target chain, only valid if getVerdict() == WF_JMP
	 */
	boost::shared_ptr<Chain> getTargetChain() const;

	/** pretty print target
	 * \param stream stream to be written to
	 */
	void print(std::ostream& stream) const;

	/** get size of translated target
	 * \param size of target in chain memory
	 * \param size of target in config memory
	 */
	void size(Platform::Size& chain, Platform::Size& config) const;

	/** translate target
	 * \param chain chain memory
	 * \param config config memory
	 */
	void write(Stream& chain, Stream& config) const;
};

/** \brief All Targets for the Shell */
class TargetShell : public Parser::Choice {
private:
	/// accept target parser
	Entity::ptr acceptTarget;
	/// jam target parser
	Entity::ptr jamTarget;
	/// jump target parser
	Entity::ptr jmpTarget;
	/// return target parser
	Entity::ptr returnTarget;
	/// jump destination parser
	Parser::StringTerminal::ptr trgParser;

	/** construct a new target shell */
	TargetShell();

public:
	/// pointer type
	typedef boost::shared_ptr<TargetShell> ptr;
	/// const pointer type
	typedef boost::shared_ptr<const TargetShell> cptr;

protected:
	Entity::ptr _clone() const;

public:
	/** create a new target shell
	 * \return new target shell
	 */
	static ptr New();

	/** compile parsed target to target
	 * \return target
	 */
	Target::ptr compile() const;
};

}
