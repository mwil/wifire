
#pragma once

#include <string>
#include <cstdlib>
#include <ostream>
#include "token.h"
#include <boost/shared_ptr.hpp>

namespace Parser {

/** \brief Tokenizer */
class Scanner {
private:
	/// line to be parsed
	boost::shared_ptr<std::string> line;
	/// current position
	std::size_t pos;

public:
	/** construct empty scanner */
	Scanner();

	/** construct scanner from a substring
	 * \param line line to be processed
	 * \param begin beginning in line
	 * \param end ending in line
	 */
	Scanner(const std::string& line, std::size_t begin, std::size_t end);

	/** construct scanner from line
	 * \param line line to be processed
	 */
	Scanner(const std::string& line);

	/** reset scanner state */
	void reset();

	/** pop a token */
	Token pop();

	/** revert to previous token */
	void prev();

	/** whether we are at end of input
	 * \return true iff end of input is reached (maybe there are no more tokens but whitespaces)
	 */
	bool atEnd() const;

	/** whether there are tokens left
	 * \return true iff there are no more tokens
	 */
	bool atTokenEnd() const;

	/** get current position
	 * \return position
	 */
	std::size_t getPos() const;

	/** rest of input
	 * \return untokenized input
	 */
	std::string rest() const;

	/** pretty print scanner state e.g. for syntactic errors
	 * \param stream stream to write to
	 * \param text some introductory free text
	 */
	void prettyPrint(std::ostream& stream, const std::string& text) const;

	/** whether two scanners are the same
	 * \param s scanner to compare
	 * \return true iff scanners are the same (same line, same state)
	 */
	bool operator==(const Scanner& s) const;

	/** whether this scanner state is before anothers one
	 * \param s scanner to compare to
	 * \return true iff this scanner state is before s' state
	 */
	bool operator<(const Scanner& s) const;
};

}
