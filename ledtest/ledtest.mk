LEDTEST_OBJS += \
	lib/leds.o \
	lib/timers.o \
	ledtest/ledtest.o

ledtest: $(addprefix $(BUILD_DIR)/, $(LEDTEST_OBJS))
	$(CC) $(CFLAGS) $(addprefix $(BUILD_DIR)/, $(LEDTEST_OBJS)) -o \
		$(addprefix $(BUILD_DIR)/, program.elf) $(LFLAGS)
