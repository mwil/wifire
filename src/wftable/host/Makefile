
VPATH := src:src/matches:src/shell:src/shell/command:src/parser:src/net:src/platform

SRC:=$(wildcard src/parser/*.cc src/shell/*.cc src/shell/command/*.cc src/*.cc src/net/*.cc src/matches/*.cc src/platform/*.cc)
SRC_ND:=$(notdir $(SRC))
OBJ:=$(addprefix obj/,$(SRC_ND:.cc=.o))
SRCDEPS:=$(addprefix .deps/,$(SRC_ND:.cc=.d))
CC:=gcc
CXX:=g++
CXXFLAGS:=-O2 -g

BOOST=/opt/local/include

all: wftables

wftables: $(OBJ)
	$(CXX) -I$(BOOST) -lreadline -L/opt/local/lib -lboost_system -pthread -o $@ $^

obj/%.o: %.cc
	@-[ -e obj ] || mkdir obj
	$(CXX) -Wall -I$(BOOST) -Isrc $(CXXFLAGS) -c -o $@ $<

clean:
	rm -rf $(OBJ) wftables .dep .deps obj

deps: .dep

.deps/%.d: %.cc
	@[ -e .deps ] || mkdir .deps
	@$(CXX) -Wall -I$(BOOST) -Isrc -M $< -MT obj/$(notdir $(@:.d=.o)) -MF $@
	
.dep: $(SRCDEPS)
	@echo "Updating Dependencies"
	@cat $^ > $@

-include .dep


