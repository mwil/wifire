
#pragma once

#include <stdexcept>
#include <vector>
#include <string>
#include "scanner.h"

namespace Parser {

/** \brief Syntactic Error, can be thrown while parsing */
class SyntaxError : std::exception {
private:
	/// Scanner (state)
	Scanner scan;
	/// list of expected tokens
	std::vector<std::string> expected;
	/// whether the corresponding text was after the parse tree (junk at EOF)
	bool junk;

public:
	/** construct a new syntax error
	 * \param s scanner
	 */
	SyntaxError(const Scanner& s) throw();

	/** consturct a new syntax error with expected tokens list
	 * \param s scanner
	 * \param expected tokens list
	 */
	SyntaxError(const Scanner& s, const std::vector<std::string>& expected) throw();

	virtual ~SyntaxError() throw();

	virtual const char * what() const throw();

	/** get scanner
	 * \return scanner
	 */
	const Scanner& getScanner() const throw();

	/** get expected list
	 * \return expected list
	 */
	const std::vector<std::string>& getExpected() const throw();

	/** whether corresponding text was junk at EOF
	 * \return true iff text was junk
	 */
	bool isJunk() const throw();
};

}
