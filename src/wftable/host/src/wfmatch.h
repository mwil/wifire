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
#include <string>
#include <vector>
#include <ostream>
#include <platform/wfplatform.h>
#include <platform/wfmemory.h>
#include <parser/entity.h>
#include <parser/sequence.h>
#include <parser/choice.h>

namespace WiFire {

class Match;

/** \brief Description of a match */
class MatchDesc {
public:
	/// pointer type
	typedef boost::shared_ptr<MatchDesc> ptr;
	/// const pointer type
	typedef boost::shared_ptr<const MatchDesc> cptr;

private:
	/// match name (will also be name on shell)
	std::string name;
	/// match description (must be equal on target side)
	std::string desc;
	/// match pointer on target side
	Platform::ptr match;
	/// function pointer on target side
	Platform::ptr fun;
	/// whether match was found on target side
	bool found;

protected:
	/** construct a new match description
	 * \param name match name (will also be name on shell)
	 * \param description match description (must match on target side)
	 */
	MatchDesc(const std::string& name, const std::string& desc);

public:
	/** get match name
	 * \return name
	 */
	const std::string& getName() const;

	/** get match description
	 * \return description
	 */
	const std::string& getDescription() const;

	/** get match pointer of target side
	 * \return match pointer of target side
	 */
	Platform::ptr getMatch() const;

	/** get match function of target side
	 * \return match function of target side
	 */
	Platform::ptr getFun() const;

	/** create a new parser for that match description
	 * \return new parser for that match description
	 */
	virtual Parser::Entity::ptr parser() const = 0;

	/** connect a match description with its target counterpart
	 * \param match match description of target side
	 * \param fun match function of target side
	 */
	void connect(Platform::ptr match, Platform::ptr fun);

	/** disconnect a match description from its target counterpart */
	void disconnect();

	/** whether match is connected
	 * \return true iff match is connected
	 */
	bool isConnected() const;
};

/** \brief Match */
class Match {
public:
	/// pointer type
	typedef boost::shared_ptr<Match> ptr;
	/// const pointer type
	typedef boost::shared_ptr<const Match> cptr;

private:
	/// match description
	MatchDesc::cptr desc;
	bool negate;

protected:
	/** construct a new match
	 * \param desc match description
	 * \param negate negate match
	 */
	Match(MatchDesc::cptr desc, bool negate);

	/** pretty print the match data
	 * \param stream stream to be written to
	 */
	virtual void _print(std::ostream& stream) const = 0;

	/** size of the match data
	 * \param config size of match data
	 */
	virtual void _size(Platform::Size& config) const = 0;

	/** write match data
	 * \param stream stream to be written to
	 */
	virtual void _write(Stream& config) const = 0;

public:
	/** destruct match */
	virtual ~Match();

	/** get match description
	 * \return match description
	 */
	MatchDesc::cptr getDescription() const;

	/** pretty print match
	 * \param stream stream to be written to
	 */
	void print(std::ostream& ostream) const;

	/** get size of translated match
	 * \param size of match in chain memory
	 * \param size of match in config memory
	 */
	void size(Platform::Size& chain, Platform::Size& config) const;

	/** translate match
	 * \param chain chain memory
	 * \param config config memory
	 */
	void write(Stream& chain, Stream& config) const;
};

/** \brief Match Parser */
class MatchParser : public Parser::Sequence {
public:
	/// pointer type
	typedef boost::shared_ptr<MatchParser> ptr;
	/// const pointer type
	typedef boost::shared_ptr<MatchParser> cptr;

private:
	/// match description
	MatchDesc::cptr desc;

protected:
	/** construct a new match parser
	 * \param desc match description
	 */
	MatchParser(MatchDesc::cptr desc);

	/** get match description
	 * \return match description
	 */
	MatchDesc::cptr getDescription() const;

public:
	/** compile a parsed match into its corresponding match
	 * \param negate whether this match should be negated
	 * \return match for that parser
	 */
	virtual Match::ptr compile(bool negate) const = 0;
};

/** \brief All Matches for the Shell */
class MatchShell : public Parser::Choice {
private:
	/** construct new match shell */
	MatchShell();

public:
	/// pointer type
	typedef boost::shared_ptr<MatchShell> ptr;
	/// const pointer type
	typedef boost::shared_ptr<const MatchShell> cptr;

protected:
	Entity::ptr _clone() const;

public:
	/** create new match shell
	 * \return new match shell
	 */
	static ptr New();
};

}
