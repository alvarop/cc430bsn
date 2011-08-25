DEMOAP_OBJS += \
	lib/leds.o \
	lib/oscillator.o \
	lib/uart.o \
	lib/intrinsics.o \
	lib/hal_pmm.o \
	lib/radio.o \
	lib/timers.o \
	lib/RF1A.o \
	lib/RfRegSettings.o \
	demo/access_point.o

DEMOED_OBJS += \
	lib/leds.o \
	lib/oscillator.o \
	lib/uart.o \
	lib/intrinsics.o \
	lib/hal_pmm.o \
	lib/radio.o \
	lib/timers.o \
	lib/RF1A.o \
	lib/RfRegSettings.o \
	demo/end_device.o

DEMORE_OBJS += \
	lib/leds.o \
	lib/oscillator.o \
	lib/uart.o \
	lib/intrinsics.o \
	lib/hal_pmm.o \
	lib/radio.o \
	lib/timers.o \
	lib/RF1A.o \
	lib/RfRegSettings.o \
	demo/relay.o

demoap: $(addprefix $(BUILD_DIR)/, $(DEMOAP_OBJS))
	$(CC) $(CFLAGS) $(addprefix $(BUILD_DIR)/, $(DEMOAP_OBJS)) -o \
		$(addprefix $(BUILD_DIR)/, program.elf) $(LFLAGS)
	@echo
	@echo Access Point build complete

demoed: $(addprefix $(BUILD_DIR)/, $(DEMOED_OBJS))
	$(CC) $(CFLAGS) $(addprefix $(BUILD_DIR)/, $(DEMOED_OBJS)) -o \
		$(addprefix $(BUILD_DIR)/, program.elf) $(LFLAGS)
	@echo
	@echo End Device build complete

demore: $(addprefix $(BUILD_DIR)/, $(DEMORE_OBJS))
	$(CC) $(CFLAGS) $(addprefix $(BUILD_DIR)/, $(DEMORE_OBJS)) -o \
		$(addprefix $(BUILD_DIR)/, program.elf) $(LFLAGS)
	@echo
	@echo End Device build complete