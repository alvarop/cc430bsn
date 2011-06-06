SERIALTEST_OBJS += \
	$(LIB_OBJS) \
	serialtest/serialtest.o

serialtest: $(addprefix $(BUILD_DIR)/, $(SERIALTEST_OBJS))
	$(CC) $(CFLAGS) $(addprefix $(BUILD_DIR)/, $(SERIALTEST_OBJS)) -o \
		$(addprefix $(BUILD_DIR)/, program.elf) $(LFLAGS)