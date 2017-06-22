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

#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>
#include <net/wfconnection.h>
#include "wfplatform.h"
#include "wfmemory.h"
#include <stdint.h>
#include <wfmatch.h>

namespace WiFire {

/** USRP Remote Side */
class USRP : boost::noncopyable {
public:
	/// pointer type
	typedef boost::shared_ptr<USRP> ptr;
	/// const pointer type
	typedef boost::shared_ptr<const USRP> cptr;

private:
	/// singleton instance
	static ptr instance;

	/// network connection
	Connection::ptr usrp;
	/// configuration size
	uint32_t configSize;
	/// configuration memory
	Memory config;

	/// list of matches (on the host)
	std::vector<MatchDesc::ptr> hostMatches;

	/** construct a new usrp representation */
	USRP();
public:
	/** get singleton instance
	 * \return singleton instance
	 */
	static ptr Instance();

	/** register a new match
	 * \param match match description to be registered
	 */
	void registerHostMatch(MatchDesc::ptr match);

	/** get list of all host matches
	 * \return list of host matches
	 */
	const std::vector<MatchDesc::ptr>& getHostMatches() const;

	/** connect and negotiate to an USRP
	 * \param target usrp address
	 */
	void connect(const std::string& target);

	/** get memory
	 * \return memory
	 */
	Memory& getMemory();

	/** send memory (=configuration) over network to usrp */
	void sendMemory();

	/** whether usrp is connected
	 * \return true iff usrp is connected
	 */
	bool isConnected() const;

private:
	/** receive from socket
	 * \param stream stream to receive into
	 */
	void recv(Stream& stream);

	/** negotiation: create configuration memory */
	void createConfigMemory();

	/** negotiation: connect host matches with remote matches */
	void connectMatches();
};

}
