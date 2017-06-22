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

#include "wfconnection.h"
#include <boost/array.hpp>
#include <boost/optional.hpp>
#include <boost/bind.hpp>
#include <iostream>

namespace WiFire {

using boost::asio::ip::udp;

Connection::Connection() : sock(io_service) {}

Connection::~Connection()
{
	if (sock.is_open())
		sock.close();
}

void Connection::connect(const std::string& target)
{
	udp::resolver resolver(io_service);
	udp::resolver::query query(udp::v4(), target, "49154");
	udp::endpoint receiver_endpoint = *resolver.resolve(query);

	if (sock.is_open())
		sock.close();

	sock.open(udp::v4());
	sock.connect(receiver_endpoint);
}

void Connection::disconnect()
{
	if (sock.is_open())
		sock.close();
}

void Connection::send(const Stream& stream)
{
	sock.send(stream.getMemory().buffer());
}

void Connection::send(const Stream& stream, std::size_t num)
{
	sock.send(stream.getMemory().buffer(num));
}

std::size_t Connection::recv(Stream& stream)
{
	std::size_t ret = sock.receive(stream.getMemory().buffer());
	return ret;
}

static void set_result(boost::optional<boost::system::error_code>* a, boost::system::error_code b)
{
	a->reset(b);
}

bool Connection::recv(Stream& stream, std::size_t seconds)
{
	boost::optional<boost::system::error_code> timer_result;
	boost::asio::deadline_timer timer(io_service);
	timer.expires_from_now(boost::posix_time::seconds(seconds));
	timer.async_wait(boost::bind(set_result, &timer_result, _1));

	boost::optional<boost::system::error_code> read_result;
	sock.async_receive(stream.getMemory().buffer(), boost::bind(set_result, &read_result, _1));
	io_service.reset();
	while (io_service.run_one()) {
		if (read_result)
			timer.cancel();
		else if (timer_result) {
			sock.cancel();
			return false;
		}

		if (*read_result)
			throw boost::system::system_error(*read_result);
	}
	return true;
}

bool Connection::isConnected() const
{
	return sock.is_open();
}

}
