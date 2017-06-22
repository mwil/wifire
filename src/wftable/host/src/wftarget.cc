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

#include "wftarget.h"
#include "shell/wfshell.h"
#include "platform/wfplatform.h"
#include <parser/builder.h>

namespace WiFire {

using Parser::Builder;
using Platform::SizeOf;

Target::Target(Verdict v, Chain::ptr jmpTarget) : v(v), jmpTarget(jmpTarget)
{
	if (jmpTarget)
		jmpTarget->ref();
}

Target::~Target()
{
	if (jmpTarget)
		jmpTarget->unref();
}

Target::ptr Target::New(Verdict v)
{
	return ptr(new Target(v, Chain::ptr()));
}

Target::ptr Target::NewJmp(Chain::ptr jmpTarget)
{
	return ptr(new Target(WF_JMP, jmpTarget));
}

Verdict Target::getVerdict() const
{
	return v;
}

Chain::ptr Target::getTargetChain() const
{
	return jmpTarget;
}

void Target::print(std::ostream& stream) const
{
	stream << "-j " << VerdictToName(v);
	if (v == WF_JMP)
		stream << " " << jmpTarget->getName();
}

void Target::size(Platform::Size& chain, Platform::Size& config) const
{
	chain.align();
	/* verdict (4) + jump target (4) + target data (4) */
	chain = chain + SizeOf<uint32_t>() + SizeOf<Platform::ptr>() + SizeOf<Platform::ptr>();
}

void Target::write(Stream& chain, Stream& config) const
{
	chain.align();
	/* verdict */
	chain.write<uint32_t>(v);
	/* jump target (if applicable) */
	if (v == WF_JMP) {
		chain.write<Platform::ptr>(jmpTarget->getLocation());
	} else {
		chain.write<Platform::ptr>(0);
	}
	/* target data */
	chain.write<Platform::ptr>(0);
}


TargetShell::TargetShell()
{
	choiceAdd(acceptTarget = Builder("ACCEPT"));
	choiceAdd(jamTarget = Builder("JAM"));
	choiceAdd(jmpTarget = (Builder("JMP") + (trgParser = Parser::StringTerminal::New())));
	choiceAdd(returnTarget = Builder("RETURN"));
}

Parser::Entity::ptr TargetShell::_clone() const
{
	return New();
}

TargetShell::ptr TargetShell::New()
{
	return ptr(new TargetShell);
}

Target::ptr TargetShell::compile() const
{
	Entity::ptr ch = get();
	if (ch == acceptTarget) {
		return Target::New(WF_ACCEPT);
	} else if (ch == jamTarget) {
		return Target::New(WF_JAM);
	} else if (ch == jmpTarget) {
		Chain::ptr c = Shell::Instance()->getChain(trgParser->get());
		if (!c)
			throw trgParser->semanticError("Target Chain Not Found");
		return Target::NewJmp(c);
	} else if (ch == returnTarget) {
		return Target::New(WF_RET);
	} else {
		throw ch->semanticError("Internal Parser Error: unknown target");
	}
}

}
