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

typedef enum { CTP_ROUTING=0, CTP_DATA=1 } ctp_type_t;

struct Config {
    uint32_t required_len;
    uint32_t ctp_type;
    uint32_t pull_bit;
    uint32_t congestion_bit;
};

struct ctp_routing_t {
    uint8_t options;
    uint16_t parent;
    uint16_t etx;
};

struct ctp_data_t {
    uint8_t options;
    uint8_t thl;
    uint16_t etx;
    uint16_t origin;
    uint8_t seqno;
    uint8_t collect_id;
    uint8_t data[];
};


static WF_bool ctpMatch(const struct WF_Header* header, const struct WF_Match* match, const struct WF_Payload* payload) {
    const struct Config* cnf = (const struct Config*)match->data;
    const struct ctp_data_t* data_frame;
    const struct ctp_routing_t* routing_frame;

    if(payload == NULL)
	return WF_false;

    // we need a minimum amount of bytes to check..
    if(payload->len < cnf->required_len)
	return WF_false;

    if(payload->payload[0] != '\x3f')
	return WF_false;

    switch(payload->payload[1])
    {
	case '\x70':	// routing frame
	    routing_frame = (const struct ctp_routing_t*) &(payload->payload[4]);

	    if(cnf->ctp_type != NO_CHECK && cnf->ctp_type != CTP_ROUTING)
		return WF_false;

	    if(cnf->pull_bit != NO_CHECK && cnf->pull_bit != BIT(routing_frame->options, 0))
		return WF_false;

	    if(cnf->congestion_bit != NO_CHECK && cnf->congestion_bit != BIT(routing_frame->options, 1))
		return WF_false;

	    return WF_true;
	    //break;
	case '\x71':	// data frame
	    data_frame = (const struct ctp_data_t*) &(payload->payload[2]);

	    if(cnf->ctp_type != NO_CHECK && cnf->ctp_type != CTP_DATA)
		return WF_false;

	    return WF_true;
	    //break;
	default:
	    return WF_false;
    }

    return WF_false;
}

struct WF_MatchDesc WF_ctpMatch = {
    .name = "CTP",
    .fun = &ctpMatch
};

