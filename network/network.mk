NETWORKAP_OBJS += \
	lib/leds.o \
	lib/uart.o \
	lib/cc2500.o \
	lib/radio_cc2500.o \
	lib/timers.o \
	lib/intrinsics.o \
	network/access_point.o

NETWORKED_OBJS += \
	lib/leds.o \
	lib/uart.o \
	lib/cc2500.o \
	lib/radio_cc2500.o \
	lib/timers.o \
	lib/intrinsics.o \
	network/end_device.o

NETWORKSNIFF_OBJS += \
	lib/leds.o \
	lib/uart.o \
	lib/cc2500.o \
	lib/radio_cc2500.o \
	lib/timers.o \
	lib/intrinsics.o \
	network/sniffer.o

networkap: $(addprefix $(BUILD_DIR)/, $(NETWORKAP_OBJS))
	$(CC) $(CFLAGS) $(addprefix $(BUILD_DIR)/, $(NETWORKAP_OBJS)) -o \
		$(addprefix $(BUILD_DIR)/, program.elf) $(LFLAGS)
	@echo
	@echo Access Point build complete

networked: $(addprefix $(BUILD_DIR)/, $(NETWORKED_OBJS))
	$(CC) $(CFLAGS) $(addprefix $(BUILD_DIR)/, $(NETWORKED_OBJS)) -o \
		$(addprefix $(BUILD_DIR)/, program.elf) $(LFLAGS)
	@echo
	@echo End Device build complete

networksniff: $(addprefix $(BUILD_DIR)/, $(NETWORKSNIFF_OBJS))
	$(CC) $(CFLAGS) $(addprefix $(BUILD_DIR)/, $(NETWORKSNIFF_OBJS)) -o \
		$(addprefix $(BUILD_DIR)/, program.elf) $(LFLAGS)
	@echo
	@echo End Device build complete