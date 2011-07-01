# CC430 Makefile
# Build and progam device
# Author: Alvaro Prieto (axp8664@rit.edu)
# (Derived from Jeff Robble's Makefile jjr0192@rit.edu )

BUILD_DIR = build

CC = /opt/mspgcc-gcc-3.2.3/bin/msp430-gcc
#CPU = cc430x6137
CPU = msp430x2274

# Identify library/helper source files one directory deep.
LIB_SOURCE = $(wildcard lib/*.c)

LIB_OBJS = $(patsubst %.c, %.o, $(LIB_SOURCE))

# Device radio address defaults to zero here
# Can be changed by adding 'ADDRESS=0xXX' to the make command
ADDRESS = 0x00

CFLAGS += \
	-mmcu=$(CPU) -O1 -mno-stack-init -mendup-at=main -Wall -g \
	-D"__MSP430F2274__" \
	-DMHZ_915_CUSTOM \
	-DDEVICE_ADDRESS=$(ADDRESS) \
	-I"." \
	-I"lib" \

# Include makefile definitions from each subfolder
include */*.mk

# Get rid of any build files
clean:
	@echo
	@echo Cleaning target
	cd $(BUILD_DIR) && rm -rf *

$(addprefix $(BUILD_DIR)/, %.o): %.c
	@echo
	@echo [$<]
	@$(CC) $(CFLAGS) -c $<
	@mkdir -p $(dir $@)
	@mv $(notdir $@) $(dir $@)
#	$(CC) -c -g -Wa,-a,-ad $(CFLAGS) $< > $(BUILD_DIR)/$<.lst

# Program the device with last compiled program using the rf2500 dongle

program: 
	sudo mspdebug rf2500 "prog $(addprefix $(BUILD_DIR)/, program.elf)"
