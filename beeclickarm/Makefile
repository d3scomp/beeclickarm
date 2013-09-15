# Binaries will be generated with this name (.elf, .bin, .hex, etc)
PROJ_NAME=beeclickarm

BUILD_DIR=build
SRC_DIR=src
CMSIS_DIR=stm32f40xx/CMSIS
PERIPH_DIR=stm32f40xx/STM32F4xx_StdPeriph_Driver
DISCOVERY_DIR=stm32f4discovery

OPENOCD_DIR=C:/Apps/openocd-0.7.0

SRCS +=	${SRC_DIR}/main.cpp 
SRCS +=	${SRC_DIR}/UART.cpp 
SRCS +=	${SRC_DIR}/LED.cpp 
#SRCS +=	${SRC_DIR}/machine.c 
SRCS +=	${SRC_DIR}/stm32f4xx_it.cpp 
#SRCS +=	${SRC_DIR}/uart_io.c
SRCS +=	${SRC_DIR}/syscalls.c
SRCS +=	${DISCOVERY_DIR}/system_stm32f4xx.c
SRCS +=	${DISCOVERY_DIR}/startup_stm32f40xx.s
#SRCS +=	${DISCOVERY_DIR}/stm32f4_discovery.c
SRCS +=	${PERIPH_DIR}/src/stm32f4xx_rcc.c
SRCS +=	${PERIPH_DIR}/src/stm32f4xx_exti.c
SRCS +=	${PERIPH_DIR}/src/stm32f4xx_gpio.c
SRCS +=	${PERIPH_DIR}/src/stm32f4xx_syscfg.c
SRCS +=	${PERIPH_DIR}/src/stm32f4xx_usart.c
SRCS +=	${PERIPH_DIR}/src/misc.c

#DEFS = -DUSE_STDPERIPH_DRIVER -DSTM32F4XX -DMANGUSTA_DISCOVERY -DUSE_USB_OTG_FS -DHSE_VALUE=8000000


# Normally you shouldn't need to change anything below this line!
#######################################################################################

CC=arm-none-eabi-gcc
OBJCOPY=arm-none-eabi-objcopy

# For some reason -O2 generates wrong code for interrupts. At least the USER_BUTTON interrupt handler get called twice
CFLAGS  = -mcpu=cortex-m4 -g3 -gdwarf-2 -O0 -Wall 
CFLAGS += -mlittle-endian -mthumb -mcpu=cortex-m4 -mthumb-interwork -mfloat-abi=hard -mfpu=fpv4-sp-d16 -MMD -MP
CFLAGS += -I$(SRC_DIR)
CFLAGS += -I$(DISCOVERY_DIR)
CFLAGS += -I$(CMSIS_DIR)/Include
CFLAGS += -I$(PERIPH_DIR)/inc

CPPFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti -std=c++11
#	arm-none-eabi-g++ -O0 -Wall -Wa,-adhlns="$@.lst" -fno-exceptions -fno-rtti -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -mcpu=cortex-m4 -mthumb -g3 -gdwarf-2 -o "$@" "$<"

#LDFLAGS = -nostartfiles -Tstm32f407vg.ld -Wl,-Map,beeclickarm.map $(CFLAGS)  
LDFLAGS = -Tstm32_flash.ld -Wl,-Map,$(BUILD_DIR)/beeclickarm.map $(CFLAGS)  
#	arm-none-eabi-g++ -Wl,-Map,test.map -mcpu=cortex-m4 -mthumb -g3 -gdwarf-2 -o "test.elf" $(OBJS) $(USER_OBJS) $(LIBS)

# add startup file to build
OBJS = $(patsubst %.c,build/%.o,$(SRCS))
OBJS := $(patsubst %.cpp,build/%.o,$(OBJS))
OBJS := $(patsubst %.s,build/%.o,$(OBJS))

DEPS = $(patsubst %.c,build/%.d,$(SRCS))
DEPS := $(patsubst %.cpp,build/%.d,$(DEPS))
DEPS := $(patsubst %.s,,$(DEPS))


build/%.o: %.c
	arm-none-eabi-gcc $(CFLAGS) -c -o "$@" "$<"

build/%.o: %.cpp
	arm-none-eabi-g++ $(CPPFLAGS) -c -o "$@" "$<"

build/%.o: %.s
	arm-none-eabi-gcc $(CFLAGS) -c -o "$@" "$<"

build/%.dep: %.cpp
	arm-none-eabi-g++ -M $(CPPFLAGS) "$<" > "$@"

.PHONY: proj

all: init $(BUILD_DIR)/$(PROJ_NAME).elf

init:
	mkdir -p $(BUILD_DIR)/$(SRC_DIR) $(BUILD_DIR)/$(DISCOVERY_DIR) $(BUILD_DIR)/$(PERIPH_DIR)/src

$(BUILD_DIR)/$(PROJ_NAME).elf: $(OBJS)
	arm-none-eabi-g++ $(LDFLAGS) -o "$@" $(OBJS)
	arm-none-eabi-objcopy -O ihex "$@" $(BUILD_DIR)/$(PROJ_NAME).hex 
#	arm-none-eabi-objdump -h -S "$@" > $(BUILD_DIR)/$(PROJ_NAME).lst 
	arm-none-eabi-size --format=berkeley "$@"

clean:
	rm -rf build
	
openocd:
	$(OPENOCD_DIR)/bin-x64/openocd-x64-0.7.0 -f interface/stlink-v2.cfg -f target/stm32f4x_stlink.cfg

flash: all
	$(OPENOCD_DIR)/bin-x64/openocd-x64-0.7.0 -f board/stm32f4discovery.cfg -c "program $(BUILD_DIR)/$(PROJ_NAME).elf verify reset"

-include $(DEPS)