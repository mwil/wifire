-include ../.codebase
-include .codebase

CODEBASE ?= /home/mrks/uhd/uhd/fpga/usrp2

WAVES ?= $(addsuffix .vcd, $(TB))
SIMS ?= $(addsuffix .sim, $(TB))

PATHS := vrt fifo control_lib timing sdr_lib models

FLAGS := $(addprefix -y $(CODEBASE)/, $(PATHS)) -y . $(addprefix -y , $(INC))

all: $(WAVES)

%.sim: %.v
	iverilog -o $@ $(FLAGS) $<

%.vcd: %.sim
	vvp $<

.PHONY: clean

clean:
	rm -f $(WAVES) $(SIMS)
