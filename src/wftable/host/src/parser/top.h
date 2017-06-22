
#pragma once

#include "entity.h"
#include "scanner.h"
#include "syntaxerror.h"
#include <algorithm>
#include <boost/iterator/filter_iterator.hpp>
#include <boost/bind.hpp>

namespace Parser {

/** Top Parser
 * \tparam T top entity type
 */
template<typename T>
class Top : public T {
protected:
	/** start parsing
	 * \param s scanner
	 */
	void parse(Scanner s);
};

template<typename T>
void Top<T>::parse(Scanner s)
{
	std::vector<Expect> e;
	/* actually parse stuff and collect expected tokens */
	PARSE_STATE ps = T::parse(s, e);
	if (ps == PS_OK) {
		/* parse went ok */
		if (!s.atTokenEnd()) // but we are not at end of input
			throw SyntaxError(s);
		return; // all done
	}
	/* find the tokens with most advanced scanner states */
	std::vector<Expect>::iterator max = std::max_element(e.begin(), e.end(),
		boost::bind(&Expect::scan, _1) < boost::bind(&Expect::scan, _2));
	Scanner maxScan = max->scan;
	std::vector<std::string> f;
	std::transform(
		boost::make_filter_iterator(boost::bind(&Expect::scan, _1) == boost::cref(maxScan), e.begin(), e.end()),
		boost::make_filter_iterator(boost::bind(&Expect::scan, _1) == boost::cref(maxScan), e.end(), e.end()),
		std::back_inserter(f),
		boost::mem_fn(&Expect::expect));
	throw SyntaxError(maxScan, f);
}

}
