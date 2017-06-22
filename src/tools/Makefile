#
# Copyright 2009-2010 Ettus Research LLC
# Copyright 2009-2011 Disco Labs, TU Kaiserslautern
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

SRC=setup_tool.cpp
CFLAGS+=-DTX -DRX
GR_UHD:=/home/reiner/masterarbeit/uhd
UHD=$(GR_UHD)
UHD_HOST_INCLUDE=$(UHD)/host/include
UHD_HOST_LIB=$(UHD)/host/build/lib

ver = -mt

BOOST=/opt/local/include

all: setup_tool

$(SRC).o: $(SRC)
	c++ -O3 $(CFLAGS) -I$(BOOST) -I$(UHD_HOST_INCLUDE) -Wall -Wextra -ansi -fvisibility=hidden -o $@ -c $<

setup_tool: $(SRC).o
	c++ $^ -o $@ -L/opt/local/lib -I$(BOOST) -I$(UHD_HOST_INCLUDE) -L$(UHD_HOST_LIB) -luhd -dynamic -lboost_date_time$(ver) -lboost_filesystem$(ver) -lboost_program_options$(ver) -lboost_regex$(ver) -lboost_system$(ver) -lboost_thread$(ver) -lboost_unit_test_framework$(ver) -ldl -Wl,-rpath,$(UHD_HOST_LIB)
	rm $^

clean:
	rm -f *.o setup_tool
