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

#pragma once

namespace WiFire {

/** \brief WiFire Commands */
enum WF_COMMANDS {
	/** Get Config Size */
	WF_CMD_WIFIRE_CONFIG_SIZE = 11,
	/** Get Match Descriptionts */
	WF_CMD_WIFIRE_MATCHES = 12,
	/** Configure WiFire */
	WF_CMD_WIFIRE_CONFIGURE = 13,
};

/** \brief WiFire Replies */
enum WF_REPLIES {
	/** Reply to WF_CMD_WIFIRE_CONFIG_SIZE */
	WF_REP_WIFIRE_CONFIG_SIZE = 5,
	/** Reply to WF_CMD_WIFIRE_MATCHES */
	WF_REP_WIFIRE_MATCHES = 6,
};

}
