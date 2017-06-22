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

enum WF_Verdict WF_getVerdict(struct WF_Chain * chain, const struct WF_Header * header, const struct WF_Payload* payload) {
	struct WF_Rule * r = chain->rules;
	const struct WF_Rule * er = chain->endrule;
	const struct WF_Match * m, * em;
	
	while (r < er) {
		/* get matches of the rule */
		m = r->matches;
		em = r->endmatch;
		for (;m < em; ++m) {
			if (m->fun(header, m, payload) == m->negate) {
				/* no match: early evaluation exit, goto next rule */
				goto next;
			}
		}
		/* rule matches, evaluate target */
		switch (r->target.v) {
		case WF_JMP:
			/* remember current rule and chain for returning */
			chain->currule = r;
			r->target.jmpTarget->prev = chain;
			/* load next chain and their rules */
			chain = r->target.jmpTarget;
			r = chain->rules;
			er = chain->endrule;
			continue;
		case WF_RET:
ret:
			/* get previous chain and rules from backlink of current chain */
			chain = chain->prev;
			r = chain->currule;
			er = chain->endrule;
			break;
		case WF_USR:
			/* deferred verdict: call the targets function */
			{
				enum WF_Verdict v = r->target.trg.fun(header, &r->target, payload);
				/* assertion: only final verdicts (ACCEPT, JAM) or
				 * CONTINUE is returned */
				if (v != WF_CONTINUE)
					return v;
			}
			break;
		default:
			/* final verdict: return */
			return r->target.v;
		}
next:
		r = WF_nextRule(r);
	}
	if (chain->prev)
		goto ret;
	return chain->policy;
}


