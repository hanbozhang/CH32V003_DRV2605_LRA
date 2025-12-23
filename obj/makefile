################################################################################
# MRS Version: 2.3.0
# Automatically-generated file. Do not edit!
################################################################################
-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include User/subdir.mk
-include Startup/subdir.mk
-include Peripheral/src/subdir.mk
-include Debug/subdir.mk
-include Core/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(ASM_UPPER_DEPS)),)
-include $(ASM_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
SECONDARY_FLASH += \
I2C_7bit_Mode.hex \

SECONDARY_LIST += \
I2C_7bit_Mode.lst \

SECONDARY_SIZE += \
I2C_7bit_Mode.siz \


# All Target
all: 
	$(MAKE) --no-print-directory main-build 

main-build: I2C_7bit_Mode.elf secondary-outputs

# Tool invocations
I2C_7bit_Mode.elf: $(OBJS) $(USER_OBJS_ESCAPE)
	@	riscv-none-embed-gcc -march=rv32ecxw -mabi=ilp32e -msmall-data-limit=0 -msave-restore -fmax-errors=20 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized -g -T "f:/CHW/EVT/EXAM/I2C/I2C_DRV2605 - ¸±±¾/Ld/Link.ld" -nostartfiles -Xlinker --gc-sections -Wl,-Map,"I2C_7bit_Mode.map" --specs=nano.specs --specs=nosys.specs -o "I2C_7bit_Mode.elf" $(OBJS) $(LIBS) -lprintf
I2C_7bit_Mode.hex: I2C_7bit_Mode.elf
	@	riscv-none-embed-objcopy -O ihex "I2C_7bit_Mode.elf" "I2C_7bit_Mode.hex"
I2C_7bit_Mode.lst: I2C_7bit_Mode.elf
	@	riscv-none-embed-objdump --all-headers --demangle --disassemble -M xw "I2C_7bit_Mode.elf" > "I2C_7bit_Mode.lst"
I2C_7bit_Mode.siz: I2C_7bit_Mode.elf
	riscv-none-embed-size --format=berkeley "I2C_7bit_Mode.elf"

# Other Targets
clean:
	-$(RM) $(DIR_OBJS) $(SECONDARY_FLASH)$(SECONDARY_LIST)$(SECONDARY_SIZE) I2C_7bit_Mode.elf
	-$(RM) $(DIR_EXPANDS) $(CALLGRAPH_DOT)
	-$(RM) $(DIR_DEPS)
secondary-outputs: $(SECONDARY_FLASH) $(SECONDARY_LIST) $(SECONDARY_SIZE)

.PHONY: all clean dependents

-include ../makefile.targets