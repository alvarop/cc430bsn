CC2500TEST_OBJS += \
	lib/leds.o \
	lib/uart_ez430.o \
	lib/cc2500.o \
	lib/radio_cc2500.o \
	lib/intrinsics.o \
	cc2500test/cc2500test.o

cc2500test: $(addprefix $(BUILD_DIR)/, $(CC2500TEST_OBJS))
	$(CC) $(CFLAGS) $(addprefix $(BUILD_DIR)/, $(CC2500TEST_OBJS)) -o \
		$(addprefix $(BUILD_DIR)/, program.elf) $(LFLAGS)
