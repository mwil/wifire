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

struct Config {
	uint8_t mode;
	uint16_t pan;
	uint32_t addr_h;
	uint32_t addr_l;
};

static WF_bool srcMatch(const struct WF_Header * header, const struct WF_Match * match, const struct WF_Payload* payload) {
	const struct Config * cnf = (const struct Config *)match->data;
	/*printf("Source Match: Mode: %d %d, Pan %x %x, Addr H %x %x, Addr L %x %x\n",
		cnf->mode, header->frame_ctrl >> 14, cnf->pan, header->src_pan,
		cnf->addr_h, header->src_addr_h, cnf->addr_l, header->src_addr_l);*/
	return (cnf->mode == header->frame_ctrl >> 14) &&
		(cnf->mode == 0 || header->src_pan == cnf->pan) &&
		(cnf->mode != 2 || (header->src_addr_l & 0xffff) == cnf->addr_l) &&
		(cnf->mode != 3 || (header->src_addr_l == cnf->addr_l && header->src_addr_h == cnf->addr_h));
}

static WF_bool dstMatch(const struct WF_Header * header, const struct WF_Match * match, const struct WF_Payload* payload) {
	const struct Config * cnf = (const struct Config *)match->data;
	return (cnf->mode == ((header->frame_ctrl >> 10) & 3)) &&
		(cnf->mode == 0 || header->dst_pan == cnf->pan) &&
		(cnf->mode != 2 || (header->dst_addr_l & 0xffff) == cnf->addr_l) &&
		(cnf->mode != 3 || (header->dst_addr_l == cnf->addr_l && header->dst_addr_h == cnf->addr_h));
}

struct WF_MatchDesc WF_sourceMatch = {
	.name = "Source Address",
	.fun = &srcMatch
};

struct WF_MatchDesc WF_destMatch = {
	.name = "Destination Address",
	.fun = &dstMatch
};
