//
// Copyright 2009-2010 Ettus Research LLC
// Copyright 2012      Disco Labs, TU Kaiserslautern
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
#include "../../../firmware/rand2.h"
#include <stddef.h>

struct Config {
    uint32_t mode;
    uint32_t probability;
};

static uint32_t frame_count = 0;

// header will match with specified probability
static WF_bool probabilityMatch(const struct WF_Header* header, const struct WF_Match* match, const struct WF_Payload* payload) {
    const struct Config* cnf = (const struct Config*)match->data;

    switch(cnf->mode)
    {
	case 0: // match every n frames
	    frame_count = (frame_count + 1) % cnf->probability;
	    if(frame_count == 0)
		return WF_true;
	    else
		return WF_false;

	    break;
	case 1: // use rng for decision; probability parameter interpreted as per cent
	    if(rand2() % 100 < cnf->probability)
		return WF_true;
	    else
		return WF_false;

	    break;
	default: // unsupported mode
	    break;
    }

    return WF_false;
}

struct WF_MatchDesc WF_probabilityMatch = {
    .name = "Probability",
    .fun = &probabilityMatch
};

