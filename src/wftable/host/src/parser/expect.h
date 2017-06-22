
#pragma once
#include <string>
#include "scanner.h"

namespace Parser {

/** \brief expected token
 * useful for reporting syntactic / semantic errors
 */
class Expect {
public:
	/// expected token
	std::string expect;
	/// at scanner position
	Scanner scan;

	/** construct a next expected information
	 * \param expect expected token
	 * \param scan scanner position
	 */
	Expect(const std::string& expect, const Scanner& scan);
};

}
