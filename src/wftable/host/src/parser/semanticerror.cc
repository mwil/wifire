
#include "semanticerror.h"

namespace Parser {

SemanticError::SemanticError(const Scanner& s, const std::string& desc) throw()
	: scan(s), desc(desc) {}

SemanticError::~SemanticError() throw() {}

const char * SemanticError::what() const throw()
{
	return desc.c_str();
}

const Scanner& SemanticError::getScanner() const throw()
{
	return scan;
}

const std::string& SemanticError::getDesc() const throw()
{
	return desc;
}

}
