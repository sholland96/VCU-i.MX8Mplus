# VCU-i.MX8Mplus -- bare-metal CAN bring-up skeleton for the i.MX 8M Plus's
# Cortex-M7 core (Variscite Symphony v1.4a + VAR-SOM-MX8M-PLUS).
#
# Verified 2026-08-22 with Arm GNU Toolchain 14.2.Rel1 (arm-none-eabi-gcc
# 14.2.1) -- builds cleanly, never flashed to hardware. Flags below are
# transcribed from Variscite's own reference example build
# (boards/som_mx8mp/driver_examples/flexcan/interrupt_transfer/armgcc/
# {flags,config}.cmake in github.com/varigit/freertos-variscite, branch
# mcuxpresso_sdk_2.15.x-var02). See README.md.

# GNU Make for Windows defaults to cmd.exe for recipes, which chokes on the
# `rm -rf`/`mkdir -p` below (they're not cmd builtins). Pin SHELL to Git for
# Windows' bash (a safe assumption -- this repo needs git anyway) so recipes
# behave the same whether invoked from a git-bash terminal, a plain
# PowerShell/cmd prompt, or a VS Code task.
ifeq ($(OS),Windows_NT)
SHELL := C:/PROGRA~1/Git/bin/bash.exe
.SHELLFLAGS := -c
endif

TOOLCHAIN_PREFIX ?= arm-none-eabi-
CC      := $(TOOLCHAIN_PREFIX)gcc
AS      := $(TOOLCHAIN_PREFIX)gcc
LD      := $(TOOLCHAIN_PREFIX)gcc
OBJCOPY := $(TOOLCHAIN_PREFIX)objcopy
SIZE    := $(TOOLCHAIN_PREFIX)size

BUILD_DIR := build
TARGET    := vcu-imx8mp

LDSCRIPT := linker/MIMX8ML8xxxxx_cm7_ram.ld

INCLUDES := \
    -ICore \
    -IDrivers/CMSIS/Include \
    -IDrivers/MIMX8ML8 \
    -IDrivers/MIMX8ML8/drivers

C_SRCS := \
    Core/main.c \
    Core/board.c \
    Core/pin_mux.c \
    Core/clock_config.c \
    Drivers/MIMX8ML8/system_MIMX8ML8_cm7.c \
    Drivers/MIMX8ML8/drivers/fsl_common.c \
    Drivers/MIMX8ML8/drivers/fsl_common_arm.c \
    Drivers/MIMX8ML8/drivers/fsl_clock.c \
    Drivers/MIMX8ML8/drivers/fsl_flexcan.c \
    Drivers/MIMX8ML8/drivers/fsl_gpio.c \
    Drivers/MIMX8ML8/drivers/fsl_rdc.c

ASM_SRCS := startup/startup_MIMX8ML8_cm7.S

OBJS := $(C_SRCS:%.c=$(BUILD_DIR)/%.o) $(ASM_SRCS:%.S=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

# Device part is MIMX8ML8DVNLZ (per Variscite's pin_mux.c generator comment:
# package_id MIMX8ML8DVNLZ). CPU_MIMX8ML8DVNLZ_cm7 selects the M7-core
# variant of the device header; FLEXCAN_WAIT_TIMEOUT matches the SDK default.
DEFINES := \
    -DCPU_MIMX8ML8DVNLZ \
    -DCPU_MIMX8ML8DVNLZ_cm7 \
    -DFLEXCAN_WAIT_TIMEOUT=1000

MCU_FLAGS := -mcpu=cortex-m7 -mthumb -mfloat-abi=hard -mfpu=fpv5-sp-d16

CFLAGS := $(MCU_FLAGS) $(DEFINES) $(INCLUDES) \
    -O0 -g -Wall -Wno-address-of-packed-member \
    -MMD -MP -fno-common -ffunction-sections -fdata-sections \
    -ffreestanding -fno-builtin -mapcs -std=gnu99

ASFLAGS := $(MCU_FLAGS) -D__STARTUP_CLEAR_BSS -D__STARTUP_INITIALIZE_NONCACHEDATA

LDFLAGS := $(MCU_FLAGS) -mapcs \
    --specs=nano.specs --specs=nosys.specs \
    -Wl,--gc-sections -Wl,-static -Wl,-z,muldefs \
    -Wl,--print-memory-usage \
    -Wl,-Map=$(BUILD_DIR)/$(TARGET).map \
    -T$(LDSCRIPT) -static

.PHONY: all clean

all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).bin

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: %.S
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) -c $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJS)
	$(LD) $(OBJS) $(LDFLAGS) -o $@
	$(SIZE) $@

$(BUILD_DIR)/$(TARGET).bin: $(BUILD_DIR)/$(TARGET).elf
	$(OBJCOPY) -O binary $< $@

clean:
	rm -rf $(BUILD_DIR)

-include $(DEPS)
