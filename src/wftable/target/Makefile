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

VPATH := .:test

SRC:=$(wildcard *.c matches/*.c)
OBJ:=$(SRC:.c=.o)
CC:=mb-gcc
AR:=mb-ar

all: libwftable_uhd.a

libwftable_uhd.a: $(OBJ)
	rm -f $@
	$(AR) r $@ $^

%.o: %.c
	$(CC) -Wall -I. -O2 -c -o $@ $<

.PHONY: clean
clean:
	rm -f $(OBJ) libwftable_uhd.a .deps

.deps:
	$(CC) -I. -M $(SRC) > .deps

-include .deps


