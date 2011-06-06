RADIOTEST_OBJS += \
	$(LIB_OBJS) \
	radiotest/radiotest.o

radiotest: $(addprefix $(BUILD_DIR)/, $(RADIOTEST_OBJS))
	$(CC) $(CFLAGS) $(addprefix $(BUILD_DIR)/, $(RADIOTEST_OBJS)) -o \
		$(addprefix $(BUILD_DIR)/, program.elf) $(LFLAGS)