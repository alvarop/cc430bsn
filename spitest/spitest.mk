SPITEST_OBJS += \
	lib/leds.o \
	lib/uart.o \
	lib/cc2500.o \
	lib/intrinsics.o \
	spitest/spitest.o

spitest: $(addprefix $(BUILD_DIR)/, $(SPITEST_OBJS))
	$(CC) $(CFLAGS) $(addprefix $(BUILD_DIR)/, $(SPITEST_OBJS)) -o \
		$(addprefix $(BUILD_DIR)/, program.elf) $(LFLAGS)
