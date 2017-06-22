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

#include "wfmemory.h"
#include <cstring>

namespace WiFire {

Memory::Memory() : memory(NULL), size(0), remoteBase(0) {}

Memory::Memory(std::size_t size, Platform::ptr base) : memory(), size(size), remoteBase(base)
{
	this->memory = new char[size];
}

Memory::Memory(const Memory& mem) : memory(), size(size), remoteBase(mem.remoteBase)
{
	this->memory = new char[mem.size];
	std::memcpy(this->memory, mem.memory, size);
}

Memory::~Memory()
{
	delete [] memory;
}

Memory& Memory::operator=(const Memory& mem)
{
	if (this != &mem) {
		size = mem.size;
		remoteBase = mem.remoteBase;
		delete [] memory;
		memory = new char[size];
		std::memcpy(memory, mem.memory, size);
	}
	return *this;
}

void Memory::copyFrom(const Memory& mem, std::size_t off)
{
	std::memcpy(memory + off, mem.memory, mem.size);
}

std::size_t Memory::getSize() const
{
	return size;
}

Platform::ptr Memory::getRemoteBase() const
{
	return remoteBase;
}

boost::asio::mutable_buffers_1 Memory::buffer()
{
	return boost::asio::buffer(boost::asio::mutable_buffer(memory, size));
}

boost::asio::const_buffers_1 Memory::buffer() const
{
	return boost::asio::buffer(boost::asio::const_buffer(memory, size));
}

boost::asio::const_buffers_1 Memory::buffer(std::size_t num) const
{
	return boost::asio::buffer(boost::asio::const_buffer(memory, num));
}

Pointer::Pointer(Memory& mem, std::size_t pos) : mem(&mem), pos(pos) {}

Pointer::~Pointer() {}

std::size_t Pointer::getPos() const
{
	return pos;
}

void * Pointer::getRaw()
{
	return mem->memory + pos;
}

const void * Pointer::getRaw() const
{
	return mem->memory + pos;
}

Platform::ptr Pointer::getRemoteAddress() const
{
	return mem->getRemoteBase() + pos;
}

Stream::Stream(const Pointer& ptr) : Pointer(ptr), autoAlign(false) {}

Stream::Stream(Memory& mem, std::size_t pos, bool autoAlign) : Pointer(mem, pos), autoAlign(autoAlign) {}

Memory& Stream::getMemory()
{
	return *mem;
}

const Memory& Stream::getMemory() const
{
	return *mem;
}

void Stream::align()
{
	_align<Platform::data_t>();
}

Stream Stream::operator+(std::size_t off) const
{
	return Stream(*mem, pos + off, autoAlign);
}

Stream& Stream::operator+=(std::size_t off)
{
	pos += off;
	return *this;
}

Stream Stream::operator-(std::size_t off) const
{
	return Stream(*mem, pos - off, autoAlign);
}

Stream& Stream::operator-=(std::size_t off)
{
	pos -= off;
	return *this;
}

Platform::ptr Stream::operator&() const
{
	return getRemoteAddress();
}

}
