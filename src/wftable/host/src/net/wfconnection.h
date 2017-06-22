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

#include <string>
#include <cstddef>
#include <boost/asio.hpp>
#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>
#include <platform/wfmemory.h>

namespace WiFire {

/** \brief Connection (ok it's UDP) */
class Connection : boost::noncopyable {
private:
	/// io service for asio
	boost::asio::io_service io_service;
	/// udp socket
	boost::asio::ip::udp::socket sock;

public:
	/// pointer type
	typedef boost::shared_ptr<Connection> ptr;
	/// const pointer type
	typedef boost::shared_ptr<const Connection> cptr;

public:
	/** Construct a new connection */
	Connection();
	/** Disconnect */
	~Connection();

	/** connect to host
	 * \param target hostname of target
	 */
	void connect(const std::string& target);

	/** disconnect */
	void disconnect();

	/** send a stream over the connection
	 * \param stream stream to be sent
	 */
	void send(const Stream& stream);

	/** send a part of a stream over the connection
	 * \param stream stream to be sent
	 * \param num number of bytes to be sent
	 */
	void send(const Stream& stream, std::size_t num);

	/** receive into a stream
	 * \param stream stream to be received into
	 * \return number of bytes received
	 */
	std::size_t recv(Stream& stream);

	/** receive into a stream with timeout
	 * \param stream stream to be received into
	 * \param timeout timeout in seconds
	 * \return true if data was received, false on timeout
	 */
	bool recv(Stream& stream, std::size_t timeout);

	/** whether socket is connected
	 * \return true iff socket is connected
	 */
	bool isConnected() const;
};


}
