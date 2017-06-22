
#pragma once

#include <stdexcept>
#include <string>
#include "scanner.h"

namespace Parser {

/** \brief Semantic Error, can be thrown by all action() methods */
class SemanticError : std::exception {
private:
	/// Scanner (state)
	Scanner scan;
	/// Description
	std::string desc;

public:
	/** construct a new semantic error
	 * \param s scanner
	 * \param desc description
	 */
	SemanticError(const Scanner& s, const std::string& desc) throw();

	virtual ~SemanticError() throw();

	virtual const char * what() const throw();

	/** get Scanner
	 * \return scanner
	 */
	const Scanner& getScanner() const throw();

	/** get description
	 * \return description
	 */
	const std::string& getDesc() const throw();
};

}
