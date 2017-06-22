
CODEBASE ?= /home/mrks/uhd/uhd/fpga/usrp2
ISE ?= /opt/Xilinx/12.2
INC = ../coregen $(ISE)/ISE_DS/ISE/verilog/src/XilinxCoreLib

TB := wifire_tb wifire_frame_decoder_tb

include ../simulation/Makefile

wifire_tb.sim: wifire.v wifire_frame_decoder.v receiver.v common.v

wifire_frame_decoder_tb.sim: wifire_frame_decoder.v receiver.v common.v

