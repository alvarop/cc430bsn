# CC430 Makefile
# Build and progam device
# Author: Alvaro Prieto (axp8664@rit.edu)
# (Derived from Jeff Robble's Makefile jjr0192@rit.edu )

BUILD_DIR = build

CC = /opt/mspgcc-gcc-3.2.3/bin/msp430-gcc
CPU = cc430x6137

# Identify library/helper source files one directory deep.
LIB_SOURCE = $(wildcard lib/*.c)

LIB_OBJS = $(patsubst %.c, %.o, $(LIB_SOURCE))

LEDTEST_OBJS += \
	$(LIB_OBJS) \
	ledtest/ledtest.o

SERIALTEST_OBJS += \
	$(LIB_OBJS) \
	serialtest/serialtest.o

RADIOTEST_OBJS += \
	$(LIB_OBJS) \
	radiotest/radiotest.o

ADDRESS = 0x00

CFLAGS += \
	-mmcu=$(CPU) -O1 -mno-stack-init -mendup-at=main -Wall -g \
	-D"__CC430F6137__" \
	-DMHZ_915 \
	-DDEVICE_ADDRESS=$(ADDRESS) \
	-I"." \
	-I"lib" \
	
# Include additional header file directories based on build target.
ledtest: CFLAGS += -I"accessPoint"

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

ledtest: $(addprefix $(BUILD_DIR)/, $(LEDTEST_OBJS))
	@echo
	@echo Invoking linker
	$(CC) $(CFLAGS) $(addprefix $(BUILD_DIR)/, $(LEDTEST_OBJS)) -o \
		$(addprefix $(BUILD_DIR)/, program.elf) $(LFLAGS)
	@echo
	@echo ledtest build complete

serialtest: $(addprefix $(BUILD_DIR)/, $(SERIALTEST_OBJS))
	@echo
	@echo Invoking linker
	$(CC) $(CFLAGS) $(addprefix $(BUILD_DIR)/, $(SERIALTEST_OBJS)) -o \
		$(addprefix $(BUILD_DIR)/, program.elf) $(LFLAGS)
	@echo
	@echo serialtest build complete

radiotest: $(addprefix $(BUILD_DIR)/, $(RADIOTEST_OBJS))
	@echo
	@echo Invoking linker
	$(CC) $(CFLAGS) $(addprefix $(BUILD_DIR)/, $(RADIOTEST_OBJS)) -o \
		$(addprefix $(BUILD_DIR)/, program.elf) $(LFLAGS)
	@echo
	@echo radiotest build complete

program: 
	sudo mspdebug rf2500 "prog $(addprefix $(BUILD_DIR)/, program.elf)"
