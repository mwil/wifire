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

namespace WiFire {

/** \brief verdicts */
typedef enum {
	/// ACCEPT
	WF_ACCEPT,
	/// JAM
	WF_JAM,
	/// JUMP
	WF_JMP,
	/// RETURN
	WF_RET,
	/// USR FUNCTION
	WF_USR,
	/// CONTINUE
	WF_CONTINUE
} Verdict;

/** \brief get a printable name of a verdict
 * \param v verdict
 * \return name
 */
std::string VerdictToName(Verdict v);

}
