################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
LSS += \
on-board.lss \

FLASH_IMAGE += \
on-board.hex \

EEPROM_IMAGE += \
on-board.eep \

SIZEDUMMY += \
sizedummy \


# All Target
all: on-board.elf secondary-outputs

# Tool invocations
on-board.elf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: AVR C Linker'
	avr-gcc -Wl,-Map,on-board.map -mmcu=attiny85 -o "on-board.elf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

on-board.lss: on-board.elf
	@echo 'Invoking: AVR Create Extended Listing'
	-avr-objdump -h -S on-board.elf  >"on-board.lss"
	@echo 'Finished building: $@'
	@echo ' '

on-board.hex: on-board.elf
	@echo 'Create Flash image (ihex format)'
	-avr-objcopy -R .eeprom -R .fuse -R .lock -R .signature -O ihex on-board.elf  "on-board.hex"
	@echo 'Finished building: $@'
	@echo ' '

on-board.eep: on-board.elf
	@echo 'Create eeprom image (ihex format)'
	-avr-objcopy -j .eeprom --no-change-warnings --change-section-lma .eeprom=0 -O ihex on-board.elf  "on-board.eep"
	@echo 'Finished building: $@'
	@echo ' '

sizedummy: on-board.elf
	@echo 'Invoking: Print Size'
	-avr-size --format=avr --mcu=attiny85 on-board.elf
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(FLASH_IMAGE)$(ELFS)$(OBJS)$(ASM_DEPS)$(EEPROM_IMAGE)$(S_DEPS)$(SIZEDUMMY)$(S_UPPER_DEPS)$(LSS)$(C_DEPS) on-board.elf
	-@echo ' '

secondary-outputs: $(LSS) $(FLASH_IMAGE) $(EEPROM_IMAGE) $(SIZEDUMMY)

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
