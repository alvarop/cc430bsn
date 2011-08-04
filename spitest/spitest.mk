SPITEST_OBJS += \
	lib/leds.o \
	lib/oscillator.o \
	lib/uart.o \
	lib/cc2500.o \
	lib/intrinsics.o \
	lib/hal_pmm.o \
	lib/radio.o \
	lib/RF1A.o \
	lib/RfRegSettings.o \
	spitest/spitest.o

spitest: $(addprefix $(BUILD_DIR)/, $(SPITEST_OBJS))
	$(CC) $(CFLAGS) $(addprefix $(BUILD_DIR)/, $(SPITEST_OBJS)) -o \
		$(addprefix $(BUILD_DIR)/, program.elf) $(LFLAGS)
