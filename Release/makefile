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
Buffalo_3SE_OB_FW.lss \

FLASH_IMAGE += \
Buffalo_3SE_OB_FW.hex \

EEPROM_IMAGE += \
Buffalo_3SE_OB_FW.eep \

SIZEDUMMY += \
sizedummy \


# All Target
all: Buffalo_3SE_OB_FW.elf secondary-outputs

# Tool invocations
Buffalo_3SE_OB_FW.elf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: AVR C Linker'
	avr-gcc -Wl,-Map,Buffalo_3SE_OB_FW.map -mmcu=attiny85 -o "Buffalo_3SE_OB_FW.elf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

Buffalo_3SE_OB_FW.lss: Buffalo_3SE_OB_FW.elf
	@echo 'Invoking: AVR Create Extended Listing'
	-avr-objdump -h -S Buffalo_3SE_OB_FW.elf  >"Buffalo_3SE_OB_FW.lss"
	@echo 'Finished building: $@'
	@echo ' '

Buffalo_3SE_OB_FW.hex: Buffalo_3SE_OB_FW.elf
	@echo 'Create Flash image (ihex format)'
	-avr-objcopy -R .eeprom -R .fuse -R .lock -R .signature -O ihex Buffalo_3SE_OB_FW.elf  "Buffalo_3SE_OB_FW.hex"
	@echo 'Finished building: $@'
	@echo ' '

Buffalo_3SE_OB_FW.eep: Buffalo_3SE_OB_FW.elf
	@echo 'Create eeprom image (ihex format)'
	-avr-objcopy -j .eeprom --no-change-warnings --change-section-lma .eeprom=0 -O ihex Buffalo_3SE_OB_FW.elf  "Buffalo_3SE_OB_FW.eep"
	@echo 'Finished building: $@'
	@echo ' '

sizedummy: Buffalo_3SE_OB_FW.elf
	@echo 'Invoking: Print Size'
	-avr-size --format=avr --mcu=attiny85 Buffalo_3SE_OB_FW.elf
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(FLASH_IMAGE)$(ELFS)$(OBJS)$(ASM_DEPS)$(EEPROM_IMAGE)$(S_DEPS)$(SIZEDUMMY)$(S_UPPER_DEPS)$(LSS)$(C_DEPS) Buffalo_3SE_OB_FW.elf
	-@echo ' '

secondary-outputs: $(LSS) $(FLASH_IMAGE) $(EEPROM_IMAGE) $(SIZEDUMMY)

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
