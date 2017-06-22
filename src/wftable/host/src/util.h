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
#include <netinet/in.h>

namespace Util {

namespace {
/** \brief instanceOf typetrait
 * \tparam B base type
 * \tparam T instance type
 */
template<typename B, typename T>
struct _iof {
};

/** \brief partial specialization for references
 * \tparam B base type
 * \tparam T instance type
 */
template<typename B, typename T>
struct _iof<B,T&> {
	/** \brief specialized function for pointers
	 * \param t instance
	 * \return true iff t is castable to base type B
	 */
	static bool iof(const T& t) {
		try {
			dynamic_cast<const B&>(t);
			return true;
		} catch(...) {
			return false;
		}
	}
};

/** \brief partial specialization for pointers
 * \tparam B base type
 * \tparam T instance type
 */
template<typename B, typename T>
struct _iof<B,T*> {
	/** \brief specialized function for pointers
	 * \param t instance
	 * \return true iff t is castable to base type B
	 */
	static bool iof(const T* t) {
		return dynamic_cast<const B*>(t) != NULL;
	}
};

}

/** \brief instanceOf operator emulation
 * \tparam B base type
 * \tparam T (inferred) instance type
 * \param t instance
 * \return true iff t is castable to base type B
 */
template<typename B, typename T>
bool instanceOf(T t) {
	return _iof<B,T>::iof(t);
}


namespace Converter {
/** \brief Endian Converter Type Trait
 * \tparam T instance type
 */
template<typename T>
struct EndianHN {
	/** \brief convert from host to network byte order
	 * \param t whatever to be converted
	 * \return t itself
	 */
	static inline T to(const T& t) { return t; }

	/** \brief convert from network to host byte order
	 * \param t whatever to be converted
	 * \return t itself
	 */
	static inline T from(const T& t) { return t; }
};

/** \brief Endian Converter Type Trait - Partial Specialization for 32 bit unsigned type*/
template<>
struct EndianHN<uint32_t> {
	static inline uint32_t to(const uint32_t& t) { return htonl(t); }
	static inline uint32_t from(const uint32_t& t) { return ntohl(t); }
};

/** \brief Endian Converter Type Trait - Partial Specialization for 16 bit unsigned type*/
template<>
struct EndianHN<uint16_t> {
	static inline uint16_t to(const uint16_t& t) { return htons(t); }
	static inline uint16_t from(const uint16_t& t) { return ntohs(t); }
};

/** \brief Endian Converter Type Trait - Partial Specialization for 64 bit unsigned type*/
template<>
struct EndianHN<uint64_t> {
	static inline uint64_t to(const uint64_t& t) { return (uint64_t(htonl(t & 0xfffffffful)) << 32) | htonl(t >> 32); }
	static inline uint64_t from(const uint64_t& t) { return (uint64_t(ntohl(t & 0xfffffffful)) << 32) | ntohl(t >> 32); }
};

/** \brief Reverting Converer adapter
 * \tparam C converter
 * \tparam T type
 */
template<template <typename> class C,typename T>
struct Revert {
	/** \brief reverse operation of C
	 * \param t data to convert
	 * \return results of C<T>::from(t)
	 */
	inline T to(const T& t) { return C<T>::from(t); }

	/** \brief reverse operation of C
	 * \param t data to convert
	 * \return results of C<T>::to(t)
	 */
	inline T from(const T& t) { return C<T>::to(t); }
};

}

namespace {

/** \brief Array Handling Type Trait - POD case
 * \tparam C converter
 * \tparam T type
 */
template<template <typename> class C,typename T>
struct _arrayTraits
{
	/** \brief convert and write data
	 * \param mem destination memory
	 * \param pos offset in memory
	 * \param t source data
	 */
	static inline void write(void * mem, std::size_t pos, const T& t)
	{
		*reinterpret_cast<T*>(reinterpret_cast<char*>(mem) + pos) = C<T>::to(t);
	}

	/** \brief read and convert data
	 * \param mem source memory
	 * \param pos offset in memory
	 * \param t destination data
	 */
	static inline void read(const void * mem, std::size_t pos, T& t)
	{
		t = C<T>::from(*reinterpret_cast<const T*>(reinterpret_cast<const char*>(mem) + pos));
	}
};

/** \brief Array Handling Type Trait - Array case
 * \tparam C converter
 * \tparam T type
 * \tparam N array length
 */
template<template <typename> class C,typename T,int N>
struct _arrayTraits<C,T[N]>
{
	/** \brief convert and write array data
	 * \param mem destination memory
	 * \param pos offset in memory
	 * \param t source array
	 */
	static inline void write(void * mem, std::size_t pos, const T t[N])
	{
		T * ptr = reinterpret_cast<T*>(reinterpret_cast<char*>(mem) + pos);
		for (std::size_t i = 0; i < N; ++i)
			*(ptr++) = C<T>::to(*(t++));
	}

	/** \brief read and convert array data
	 * \param mem source memory
	 * \param pos offset in memory
	 * \param t target array
	 */
	static inline void read(const void * mem, std::size_t pos, T t[N])
	{
		const T * ptr = reinterpret_cast<T*>(reinterpret_cast<const char*>(mem) + pos);
		for (std::size_t i = 0; i < N; ++i)
			*(t++) = C<T>::from(*(ptr++));
	}
};

}

/** \brief convert and write data
 * \tparam C converter
 * \tparam T type. if it is an array type, conversion is done element-wise
 * \param mem target memory
 * \param pos offset in target memory
 * \param t source data
 */
template<template <typename> class C,typename T>
void write(void * mem, std::size_t pos, const T& t)
{
	_arrayTraits<C,T>::write(mem, pos, t);
}

/** \brief convert and write data regions
 * \tparam C converter
 * \tparam T type
 * \param mem target memory
 * \param pos offset in target memory
 * \param t source data begin
 * \param num number of elements
 */
template<template <typename> class C,typename T>
void write(void * mem, std::size_t pos, const T * t, std::size_t num)
{
	T * ptr = reinterpret_cast<T*>(reinterpret_cast<char*>(mem) + pos);
	for (std::size_t i = 0; i < num; ++i)
		*(ptr++) = C<T>::to(*(t++));
}

/** \brief read and convert data
 * \tparam C converter
 * \tparam T type. if it is an array type, conversion is done element-wise
 * \param mem source memory
 * \param pos offset in source memory
 * \param t target data
 */
template<template <typename> class C,typename T>
void read(const void * mem, std::size_t pos, T& t)
{
	_arrayTraits<C,T>::read(mem, pos, t);
}

/** \brief convert and write data
 * \tparam C converter
 * \tparam T type. if it is an array type, conversion is done element-wise
 * \param mem source memory
 * \param pos offset in source memory
 * \param t target data begin
 */
template<template <typename> class C,typename T>
void read(const void * mem, std::size_t pos, T * t, std::size_t num)
{
	const T * ptr = reinterpret_cast<T*>(reinterpret_cast<const char*>(mem) + pos);
	for (std::size_t i = 0; i < num; ++i)
		*(t++) = C<T>::from(*(ptr++));
}

}
