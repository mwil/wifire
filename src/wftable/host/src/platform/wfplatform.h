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

#include <stdint.h>
#include <cstdlib>

namespace WiFire {

namespace Platform {

/** remote pointer type (32 bit) */
typedef uint32_t ptr;
typedef uint32_t data_t;

namespace {
template<typename T>
struct _align
{
	static inline std::size_t alignment(std::size_t pos) { return pos % sizeof(T) ? sizeof(T) - pos % sizeof(T) : 0; };
	static inline std::size_t aligned(std::size_t pos) { return pos + alignment(pos); }
};

template<>
struct _align<uint64_t>
{
	static inline std::size_t alignment(std::size_t pos) { return _align<uint32_t>::alignment(pos); }
	static inline std::size_t aligned(std::size_t pos) { return _align<uint32_t>::aligned(pos); }
};

template<typename T>
struct _align<T[]>
{
	static inline std::size_t alignment(std::size_t pos) { return _align<T>::alignment(pos); };
	static inline std::size_t aligned(std::size_t pos) { return _align<T>::aligned(pos); }
};

template<typename T,int N>
struct _align<T[N]>
{
	static inline std::size_t alignment(std::size_t pos) { return _align<T>::alignment(pos); };
	static inline std::size_t aligned(std::size_t pos) { return _align<T>::aligned(pos); }
};

}

/** align address
 * \tparam T type (used for alignment)
 * \param pos current address
 * \return aligned address
 */
template<typename T>
inline std::size_t align(std::size_t pos)
{
	return _align<T>::aligned(pos);
}

/** \brief helper class for aligned field width */
template<typename T>
class SizeOf
{
public:
	/** construct */
	SizeOf();
};

template<typename T>
SizeOf<T>::SizeOf() {}

/** \brief class for keeping track of aligned fields */
class Size {
private:
	/// current size
	std::size_t sz;

	/** construct from size
	 * \param sz size
	 */
	Size(std::size_t sz);
public:
	/** construct new size */
	Size();

	/** add another type size
	 * \tparam X type of other type size
	 * \param ps other type
	 * \return new type size
	 */
	template<typename X>
	Size operator+(const SizeOf<X>&) const;

	/** align to data width */
	void align();

	/** get type size
	 * \param type size
	 */
	operator std::size_t() const;
};

template<typename X>
Size Size::operator+(const SizeOf<X>&) const
{
	return Size(Platform::align<X>(sz) + sizeof(X));
}

}

}
