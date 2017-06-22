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
#include <string.h>

#define WF_MATCHES_MAX 10

struct WF_MatchDesc * WF_matches[WF_MATCHES_MAX];
uint32_t WF_matchesCount = 0;

WF_bool WF_registerMatch(struct WF_MatchDesc * match) {
	if (WF_matchesCount == WF_MATCHES_MAX) return WF_false;
	match->self = match;
	WF_matches[WF_matchesCount++] = match;
	return WF_true;
}

#define WF_TARGETS_MAX 10

struct WF_TargetDesc * WF_targets[WF_TARGETS_MAX];
uint32_t WF_targetsCount = 0;

WF_bool WF_registerTarget(struct WF_TargetDesc * target) {
	if (WF_targetsCount == WF_TARGETS_MAX) return WF_false;
	target->self = target;
	WF_targets[WF_targetsCount++] = target;
	return WF_true;
}

void WF_reportMatches(void * report) {
	uint32_t i;

	struct WF_MatchDesc * r = (struct WF_MatchDesc *)((uint32_t*)report + 1);

	*(uint32_t*)report = WF_matchesCount;

	for (i = 0; i < WF_matchesCount; ++i) {
		const struct WF_MatchDesc * match = WF_matches[i];
		memcpy(r++, match, sizeof * match);
	}
}

uint32_t WF_reportMatchesSize() {
	return 1 + WF_matchesCount * sizeof(struct WF_MatchDesc);
}

void WF_reportTargets(void * report) {
	uint32_t i;

	struct WF_TargetDesc * r = (struct WF_TargetDesc *)((uint32_t*)report + 1);

	*(uint32_t*)report = WF_targetsCount;

	for (i = 0; i < WF_targetsCount; ++i) {
		const struct WF_TargetDesc * target = WF_targets[i];
		memcpy(r++, target, sizeof * target);
	}
}

void WF_initChain(struct WF_Chain * chain) {
	chain->policy = WF_ACCEPT;
	strcpy(chain->name, "main");
	chain->prev = NULL;
	chain->currule = NULL;
	chain->endrule = chain->rules;
}

