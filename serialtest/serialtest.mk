SERIALTEST_OBJS += \
	lib/leds.o \
	lib/uart.o \
	lib/intrinsics.o \
	serialtest/serialtest.o

serialtest: $(addprefix $(BUILD_DIR)/, $(SERIALTEST_OBJS))
	$(CC) $(CFLAGS) $(addprefix $(BUILD_DIR)/, $(SERIALTEST_OBJS)) -o \
		$(addprefix $(BUILD_DIR)/, program.elf) $(LFLAGS)