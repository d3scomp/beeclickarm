# Binaries will be generated with this name (.elf, .bin, .hex, etc)
PROJ_NAME=beeclickarm

BUILD_DIR=build
SRC_DIR=src
CMSIS_DIR=stm32f40xx/CMSIS
PERIPH_DIR=stm32f40xx/STM32F4xx_StdPeriph_Driver
DISCOVERY_DIR=stm32f4discovery

OPENOCD_DIR=C:/Apps/openocd-0.7.0

SRCS +=	${SRC_DIR}/main.c 
SRCS +=	${SRC_DIR}/stm32f4xx_it.c 
SRCS +=	${SRC_DIR}/uart_io.c
SRCS +=	${SRC_DIR}/syscalls.c
SRCS +=	${DISCOVERY_DIR}/system_stm32f4xx.c
SRCS +=	${DISCOVERY_DIR}/startup_stm32f40xx.S
SRCS +=	${DISCOVERY_DIR}/stm32f4_discovery.c
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

CFLAGS  = -mcpu=cortex-m4 -g3 -gdwarf-2 -O2 -Wall 
CFLAGS += -mlittle-endian -mthumb -mcpu=cortex-m4 -mthumb-interwork -mfloat-abi=hard -mfpu=fpv4-sp-d16
CFLAGS += -I$(SRC_DIR)
CFLAGS += -I$(DISCOVERY_DIR)
CFLAGS += -I$(CMSIS_DIR)/Include
CFLAGS += -I$(PERIPH_DIR)/inc

#LDFLAGS = -nostartfiles -Tstm32f407vg.ld -Wl,-Map,beeclickarm.map $(CFLAGS)  
LDFLAGS = -Tstm32_flash.ld -Wl,-Map,$(BUILD_DIR)/beeclickarm.map $(CFLAGS)  

# add startup file to build
OBJS = $(patsubst %.c,build/%.o,$(SRCS))

build/%.o: %.c
	arm-none-eabi-gcc $(CFLAGS) -c -o "$@" "$<"

build/%.o: %.S
	arm-none-eabi-gcc $(CFLAGS) -c -o "$@" "$<"


.PHONY: proj

all: init $(BUILD_DIR)/$(PROJ_NAME).elf

init:
	mkdir -p $(BUILD_DIR)/$(SRC_DIR) $(BUILD_DIR)/$(DISCOVERY_DIR) $(BUILD_DIR)/$(PERIPH_DIR)/src

$(BUILD_DIR)/$(PROJ_NAME).elf: $(OBJS)
	arm-none-eabi-gcc $(LDFLAGS) -o "$@" $(OBJS)
	arm-none-eabi-objcopy -O ihex "$@" $(build_dir)/$(PROJ_NAME).hex 
	arm-none-eabi-objcopy -h -S "$<" > $(build_dir)/$(PROJ_NAME).lst 
	arm-none-eabi-size --format=berkeley "$@"

clean:
	rm -rf build
	
openocd:
	$(OPENOCD_DIR)/bin-x64/openocd-x64-0.7.0 -f interface/stlink-v2.cfg -f target/stm32f4x_stlink.cfg

flash: all
	$(OPENOCD_DIR)/bin-x64/openocd-x64-0.7.0 -f board/stm32f4discovery.cfg -c "program $(BUILD_DIR)/$(PROJ_NAME).elf verify reset"
