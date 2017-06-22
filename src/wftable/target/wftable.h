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

#include <stdlib.h>
#include <stdint.h>

typedef uint8_t WF_bool;
#define WF_true 1
#define WF_false 0

struct WF_Header;
struct WF_Payload;
struct WF_Match;
struct WF_Target;

typedef WF_bool (*WF_MatchPtr)(const struct WF_Header *, const struct WF_Match *, const struct WF_Payload*);
typedef enum WF_Verdict (*WF_TargetPtr)(const struct WF_Header *, const struct WF_Target *, const struct WF_Payload*);

struct WF_MatchDesc {
	char name[20];
	struct WF_MatchDesc * self;
	WF_MatchPtr fun;
};

struct WF_TargetDesc {
	char name[20];
	struct WF_TargetDesc * self;
	WF_TargetPtr fun;
};

struct WF_Header {
	uint32_t len;
	uint32_t frame_ctrl;
	uint32_t seqno;
	uint32_t dst_pan;
	uint32_t dst_addr_h;
	uint32_t dst_addr_l;
	uint32_t src_pan;
	uint32_t src_addr_h;
	uint32_t src_addr_l;
};

struct WF_Payload {
    uint8_t len;
    uint8_t payload[128];
};

enum WF_Verdict {
	WF_ACCEPT,
	WF_JAM,
	WF_JMP,
	WF_RET,
	WF_USR,
	WF_CONTINUE,
	//XX
};

struct WF_Chain;

struct WF_Target {
	enum WF_Verdict v;
	union {
		struct WF_Chain * jmpTarget;
		struct {
			WF_TargetPtr fun;
			//struct WF_TargetDesc * desc;
			void * data;
		} trg;
	};
};

struct WF_Match {
	WF_MatchPtr fun;
	WF_bool negate;
	//struct WF_MatchDesc * desc;
	void * data;
};

struct WF_Rule {
	struct WF_Target target;
	size_t size;
	const void * endmatch;
	struct WF_Match matches[0];
};

struct WF_Chain {
	enum WF_Verdict policy;
	char name[20];
	struct WF_Chain * prev;
	struct WF_Rule * currule;
	const void * endrule;
	struct WF_Rule rules[0];
};

static inline struct WF_Rule * WF_nextRule(struct WF_Rule * r) {
	return (struct WF_Rule *)((char*)r + r->size);
}

static inline size_t WF_countMatches(const struct WF_Rule * r) {
	return ((unsigned long)r->endmatch - (unsigned long)r->matches) /
		sizeof(struct WF_Match);
}

/*extern struct MatchDesc matches[];
extern struct TargetDesc targets[];*/

/*struct TargetDesc * getTarget(const char * name);
struct MatchDesc * getMatch(const char * name);*/

WF_bool WF_registerMatch(struct WF_MatchDesc * match);

uint32_t WF_reportMatchesSize();
void WF_reportMatches(void * report);

void WF_initChain(struct WF_Chain * chain);

enum WF_Verdict WF_getVerdict(struct WF_Chain * chain, const struct WF_Header * packet, const struct WF_Payload* payload);


