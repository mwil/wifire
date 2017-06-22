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

FW=wifire.bin
#FW=zb_wifire.bin
GR_UHD=../uhd
FPGA=/home/reiner/masterarbeit/wifire_react/images/u2_rev3.bin
#FPGA=$(GR_UHD)/fpga/usrp2/top/u2_rev3/build/u2_rev3.bin

# 1: XCVR
# 2: RFX
DBOARD=DBOARD_XCVR

SD_DEV=/dev/sdX

FIRMWARE_TOP=$(GR_UHD)/firmware/microblaze
FIRMWARE_INCLUDE=$(FIRMWARE_TOP)/include
FIRMWARE_LIB=$(FIRMWARE_TOP)/lib
WFTABLE=../wftable/target

FLASH_TOOL=$(FIRMWARE_TOP)/u2_flash_tool
LIBTOOL=$(FIRMWARE_TOP)/libtool

FW_NAME=$(FW:.bin=)
FW_SRC=$(FW_NAME).c

GCC_INCLUDES+=-I$(FIRMWARE_TOP) 
GCC_INCLUDES+=-I$(FIRMWARE_INCLUDE) 
GCC_INCLUDES+=-I$(FIRMWARE_LIB)
GCC_INCLUDES+=-I$(FIRMWARE_TOP)/lwip
GCC_INCLUDES+=-I$(FIRMWARE_TOP)/lwip_port
GCC_INCLUDES+=-I$(FIRMWARE_TOP)/lwip/lwip-1.3.1/src/include
GCC_INCLUDES+=-I$(FIRMWARE_TOP)/lwip/lwip-1.3.1/src/include/ipv4
GCC_INCLUDES+=-I$(FIRMWARE_TOP)/../../host/lib/usrp
GCC_INCLUDES+=-I$(FIRMWARE_TOP)/usrp2
GCC_INCLUDES+=-I$(WFTABLE)

firmware: $(FW)

%.o: %.c
	mkdir -p .deps
	mb-gcc -DCTIME="\"$(shell date)\"" -DPACKAGE_NAME=\"\" -DPACKAGE_TARNAME=\"\" -DPACKAGE_VERSION=\"\" -DPACKAGE_STRING=\"\" -DPACKAGE_BUGREPORT=\"\" -DPACKAGE_URL=\"\" -DPACKAGE=\"uhd-mb\" -DVERSION=\"0\" -DSTDC_HEADERS=1 -DHAVE_SYS_TYPES_H=1 -DHAVE_SYS_STAT_H=1 -DHAVE_STDLIB_H=1 -DHAVE_STRING_H=1 -DHAVE_INTTYPES_H=1 -DHAVE_STDINT_H=1 -DHAVE_UNISTD_H=1 -DLT_OBJDIR=\".libs/\" --std=gnu99 -Wall -Werror-implicit-function-declaration -mxl-soft-div -msoft-float -mxl-soft-mul -mxl-barrel-shift $(GCC_INCLUDES) -DHAL_IO_USES_UART -g -O2 -MT $@ -MD -MP -MF .deps/$(@:.o=).Tpo -c -o $@ $< -DDBOARD=$(DBOARD)
	mv -f .deps/$(@:.o=).Tpo .deps/$(@:.o=).Po

wifire.elf: uhd.o wifire.o $(WFTABLE)/libwftable_uhd.a rand2.o
	$(LIBTOOL) --tag=CC   --mode=link mb-gcc --std=gnu99 -Wall -Werror-implicit-function-declaration -mxl-soft-div -msoft-float -mxl-soft-mul -mxl-barrel-shift $(GCC_INCLUDES) -DHAL_IO_USES_UART -g -O2 -Wl,-defsym -Wl,_STACK_SIZE=3072 -Wl,-Map -Wl,$(@:.o=).map -o $@ $^ $(FIRMWARE_TOP)/usrp2/libusrp2.a $(WFTABLE)/libwftable_uhd.a >/dev/null
	rm $(@:.o=).map

zb_wifire.elf: zb_uhd.o zb_wifire.o
	$(LIBTOOL) --tag=CC   --mode=link mb-gcc --std=gnu99 -Wall -Werror-implicit-function-declaration -mxl-soft-div -msoft-float -mxl-soft-mul -mxl-barrel-shift $(GCC_INCLUDES) -DHAL_IO_USES_UART -g -O2 -Wl,-defsym -Wl,_STACK_SIZE=3072 -Wl,-Map -Wl,$(@:.o=).map -o $@ $^ $(FIRMWARE_TOP)/usrp2/libusrp2.a >/dev/null
	rm $(@:.o=).map


%.bin: %.elf
	mb-objcopy -O binary $< $@

%.dump: %.elf
	mb-objdump -DSC $< > $@
	
%.rom: %.bin
	hexdump -v -e'1/1 "%.2X\n"' $< > $@

all: firmware flash_fw verify_fw

fpga: flash_fpga verify_fpga


flash_fpga:
	@echo
	@echo Attention: Check whether $(SD_DEV) is your SD card device and press enter.
	@echo If not sure, press CTRL+C.
	@bash -c read
	$(FLASH_TOOL) -t fpga -w --dev=$(SD_DEV) $(FPGA)
	sync

verify_fpga:
	$(FLASH_TOOL) -t fpga -v --dev=$(SD_DEV) $(FPGA)
	sync

flash_fw: $(FW)
	@echo
	@echo Attention: Check whether $(SD_DEV) is your SD card device and press enter.
	@echo If not sure, press CTRL+C.
	@bash -c read
	$(FLASH_TOOL) -t s/w -w --dev=$(SD_DEV) $(FW)
	sync

verify_fw: $(FW)
	$(FLASH_TOOL) -t s/w -v --dev=$(SD_DEV) $(FW)
	sync

.PHONY: clean
clean:
	rm -f wifire wifire.bin wifire.elf wifire.map wifire.o wifire.rom wifire.dump uhd.o
	rm -rf .deps .libs

help:
	@echo Available targets:
	@echo 	firmware: compiles firmware
	@echo	flash_fw: flashes firmware to sd card
	@echo	verify_fw: verifies firmware from sd card
	@echo	all: firmware flash_fw verify_fw
	@echo	flash_fpga: flashes fpga image
	@echo	verify_fpga: verifies fpga code on sd card
	@echo	fpga: flash_fpga verify_fpga
