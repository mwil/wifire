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
#include <stddef.h>

#define NO_CHECK 0xffffffff

#define BIT(x,n) (((x) >> (7-n)) & 1)

typedef enum { LQI_ROUTING=0, LQI_DATA=1 } lqi_type_t;

struct Config {
    uint32_t required_len;
    uint32_t lqi_type;
};

struct lqi_routing_t {
    uint16_t originaddr;
    int16_t seqno;
    int16_t originseqno;
    uint16_t parent;
    uint16_t cost;
    uint16_t hopcount;
};

struct lqi_data_t {
    uint16_t originaddr;
    int16_t seqno;
    int16_t originseqno;
    uint16_t hopcount;
    uint8_t collection_id;
    uint8_t data[];
};


static WF_bool lqiMatch(const struct WF_Header* header, const struct WF_Match* match, const struct WF_Payload* payload) {
    const struct Config* cnf = (const struct Config*)match->data;
    const struct lqi_data_t* data_frame;
    const struct lqi_routing_t* routing_frame;

    if(payload == NULL)
	return WF_false;

    // we need a minimum amount of bytes to check..
    if(payload->len < cnf->required_len)
	return WF_false;

    if(payload->payload[0] != '\x3f')
	return WF_false;

    switch(payload->payload[1])
    {
	case '\x73':	// routing frame
	    routing_frame = (const struct lqi_routing_t*) &(payload->payload[2]);

	    if(cnf->lqi_type != NO_CHECK && cnf->lqi_type != LQI_ROUTING)
		return WF_false;

	    return WF_true;
	    //break;
	case '\x74':	// data frame
	    data_frame = (const struct lqi_data_t*) &(payload->payload[2]);

	    if(cnf->lqi_type != NO_CHECK && cnf->lqi_type != LQI_DATA)
		return WF_false;

	    return WF_true;
	    //break;
	default:
	    return WF_false;
    }

    return WF_false;
}

struct WF_MatchDesc WF_lqiMatch = {
    .name = "LQI",
    .fun = &lqiMatch
};

