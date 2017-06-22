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

#include <util.h>
#include <cstddef>
#include <boost/asio/buffer.hpp>
#include "wfplatform.h"

namespace WiFire {

/** \brief Remote Memory */
class Memory {
friend class Pointer;

private:
	/// backup memory
	char * memory;
	/// size of memory
	std::size_t size;
	/// base of memory on remote size
	Platform::ptr remoteBase;

public:
	/** construct empty memory */
	Memory();

	/** construct new memory
	 * \param size size
	 * \param base base on remote size
	 */
	Memory(std::size_t size, Platform::ptr base = 0);

	/** copy memory
	 * \param mem memory to copy from
	 */
	Memory(const Memory& mem);

	/** destruct memory */
	~Memory();

	/** assign memory
	 * \param mem memory to copy from
	 */
	Memory& operator=(const Memory& mem);

	/** copy memory contents
	 * \param mem memory to copy from
	 * \param off offset in source memory
	 */
	void copyFrom(const Memory& mem, std::size_t off = 0);

	/** write into memory
	 * \tparam T type
	 * \param pos position to write
	 * \param t data
	 */
	template<typename T>
	void write(std::size_t pos, const T& t);

	/** read from memory
	 * \tparam T type
	 * \param pos position to read
	 * \return read data
	 */
	template<typename T>
	T read(std::size_t pos) const;

	/** read from memory
	 * \tparam T type
	 * \param pos position to read
	 * \param t [out] data to read into
	 */
	template<typename T>
	void read(std::size_t pos, T& t) const;

	/** get memory size
	 * \return memory size
	 */
	std::size_t getSize() const;

	/** get remote base address for memory
	 * \return remote base address
	 */
	Platform::ptr getRemoteBase() const;

	/** convert content to asio buffer
	 * \return memory content as asio buffer
	 */
	boost::asio::mutable_buffers_1 buffer();

	/** convert content to asio buffer
	 * \return memory content as asio buffer
	 */
	boost::asio::const_buffers_1 buffer() const;

	/** convert content to asio buffer
	 * \param num restrict buffer length to num bytes
	 * \return memory content as asio buffer
	 */
	boost::asio::const_buffers_1 buffer(std::size_t num) const;
};

template<typename T>
void Memory::write(std::size_t pos, const T& t)
{
	Util::write<Util::Converter::EndianHN,T>(memory, pos, t);
}

template<typename T>
T Memory::read(std::size_t pos) const
{
	T ret;
	Util::read<Util::Converter::EndianHN,T>(memory, pos, ret);
	return ret;
}

template<typename T>
void Memory::read(std::size_t pos, T& t) const
{
	Util::read<Util::Converter::EndianHN,T>(memory, pos, t);
}

/** \brief Pointer into remote memory */
class Pointer {
protected:
	/// memory
	Memory * mem;
	/// position
	std::size_t pos;

public:
	/** construct a new pointer
	 * \param mem memory
	 * \param pos position
	 */
	Pointer(Memory& mem, std::size_t pos);

	/** destruct a pointer */
	virtual ~Pointer();

	/** get raw pointer to host backed memory
	 * \return raw pointer to host backed memory
	 */
	void * getRaw();

	/** get raw pointer to host backed memory
	 * \return raw pointer to host backed memory
	 */
	const void * getRaw() const;

	/** get current position
	 * \return current position
	 */
	std::size_t getPos() const;

	/** get remote address
	 * \return remote address
	 */
	Platform::ptr getRemoteAddress() const;
};

/** \brief Stream (essentially a self-advancing pointer) */
class Stream : public Pointer {
private:
	/// automatic padding
	bool autoAlign;

private:
	/** align pointer
	 * \tparam T type to align to
	 */
	template<typename T>
	void _align();

public:
	/** construct a new stream from a pointer
	 * \param ptr pointer
	 */
	explicit Stream(const Pointer& ptr);

	/** construct a new stream
	 * \param mem memory
	 * \param pos position
	 * \param autoAlign turn on automatic alignments
	 */
	Stream(Memory& mem, std::size_t pos, bool autoAlign = false);

	/** set auto alignment feature */
	void setAutoAlign(bool autoPad);

	/** align to data width */
	void align();

	/** write data to current position and advance
	 * \tparam T data type
	 * \param t data
	 */
	template<typename T>
	void write(const T& t);

	/** read data at current position and advance
	 * \tparam T data type
	 * \param t [out] data
	 */
	template<typename T>
	void read(T& t);

	/** read data at current position and advance
	 * \tparam T data type
	 * \return data
	 */
	template<typename T>
	T read();

	/** get memory
	 * \return memory
	 */
	Memory& getMemory();

	/** get memory
	 * \return memory
	 */
	const Memory& getMemory() const;

	/** create an advanced stream
	 * \param off offset to advance
	 * \return new stream
	 */
	Stream operator+(std::size_t off) const;

	/** advance stream
	 * \param off offset to advance
	 * \return self
	 */
	Stream& operator+=(std::size_t off);

	/** create a decreased stream
	 * \param off offset to decrease
	 * \return new stream
	 */
	Stream operator-(std::size_t off) const;

	/** decrease stream
	 * \param off offset to decrease
	 * \return self
	 */
	Stream& operator-=(std::size_t off);

	/** get remote address
	 * \return remote address
	 */
	Platform::ptr operator&() const;
};

template<typename T>
void Stream::_align()
{
	pos = Platform::align<T>(pos);
}

template<typename T>
void Stream::write(const T& t)
{
	if (autoAlign)
		_align<T>();
	mem->write<T>(pos, t);
	pos += sizeof(T);
}

template<typename T>
void Stream::read(T& t)
{
	if (autoAlign)
		_align<T>();
	mem->read<T>(pos, t);
	pos += sizeof(T);
}

template<typename T>
T Stream::read()
{
	if (autoAlign)
		_align<T>();
	T ret(mem->read<T>(pos));
	pos += sizeof(T);
	return ret;
}

}
