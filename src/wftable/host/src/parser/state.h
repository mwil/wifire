
#pragma once

namespace Parser {

enum PARSE_STATE {
	PS_OK,
	PS_FAIL,
};

enum COMPLETE_STATE {
	CS_DONE,
	CS_FAIL,
	CS_EXIT,
};

enum FIRST_STATE {
	FS_DONE,
	FS_EPSILON,
};

}
