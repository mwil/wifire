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

#include "wftable.h"
#include <stddef.h>

static WF_bool frameTypeMatch(const struct WF_Header * header, const struct WF_Match * match, const struct WF_Payload* payload) {
	uint8_t type = *(const uint8_t*)match->data;
	return (type == (header->frame_ctrl & 7));
}

struct WF_MatchDesc WF_frameTypeMatch = {
	.name = "Frame Type",
	.fun = &frameTypeMatch
};
