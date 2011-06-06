LEDTEST_OBJS += \
	$(LIB_OBJS) \
	ledtest/ledtest.o

ledtest: $(addprefix $(BUILD_DIR)/, $(LEDTEST_OBJS))
	$(CC) $(CFLAGS) $(addprefix $(BUILD_DIR)/, $(LEDTEST_OBJS)) -o \
		$(addprefix $(BUILD_DIR)/, program.elf) $(LFLAGS)
