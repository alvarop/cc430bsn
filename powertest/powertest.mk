POWERTESTAP_OBJS += \
	lib/leds.o \
	lib/uart.o \
	lib/cc2500.o \
	lib/radio_cc2500.o \
	lib/timers.o \
	lib/intrinsics.o \
	powertest/access_point.o

POWERTESTED_OBJS += \
	lib/leds.o \
	lib/uart.o \
	lib/cc2500.o \
	lib/radio_cc2500.o \
	lib/timers.o \
	lib/intrinsics.o \
	powertest/end_device.o

POWERTESTSNIFF_OBJS += \
	lib/leds.o \
	lib/uart.o \
	lib/cc2500.o \
	lib/radio_cc2500.o \
	lib/timers.o \
	lib/intrinsics.o \
	powertest/sniffer.o

powertestap: $(addprefix $(BUILD_DIR)/, $(POWERTESTAP_OBJS))
	$(CC) $(CFLAGS) $(addprefix $(BUILD_DIR)/, $(POWERTESTAP_OBJS)) -o \
		$(addprefix $(BUILD_DIR)/, program.elf) $(LFLAGS)
	@echo
	@echo Access Point build complete

powertested: $(addprefix $(BUILD_DIR)/, $(POWERTESTED_OBJS))
	$(CC) $(CFLAGS) $(addprefix $(BUILD_DIR)/, $(POWERTESTED_OBJS)) -o \
		$(addprefix $(BUILD_DIR)/, program.elf) $(LFLAGS)
	@echo
	@echo End Device build complete
