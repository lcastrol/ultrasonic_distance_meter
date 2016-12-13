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
analog_comp.lss \

FLASH_IMAGE += \
analog_comp.hex \

EEPROM_IMAGE += \
analog_comp.eep \

SIZEDUMMY += \
sizedummy \


# All Target
all: analog_comp.elf secondary-outputs

# Tool invocations
analog_comp.elf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: AVR C Linker'
	avr-gcc -Wl,-Map,analog_comp.map -mmcu=atmega328p -o "analog_comp.elf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

analog_comp.lss: analog_comp.elf
	@echo 'Invoking: AVR Create Extended Listing'
	-avr-objdump -h -S analog_comp.elf  >"analog_comp.lss"
	@echo 'Finished building: $@'
	@echo ' '

analog_comp.hex: analog_comp.elf
	@echo 'Create Flash image (ihex format)'
	-avr-objcopy -R .eeprom -R .fuse -R .lock -R .signature -O ihex analog_comp.elf  "analog_comp.hex"
	@echo 'Finished building: $@'
	@echo ' '

analog_comp.eep: analog_comp.elf
	@echo 'Create eeprom image (ihex format)'
	-avr-objcopy -j .eeprom --no-change-warnings --change-section-lma .eeprom=0 -O ihex analog_comp.elf  "analog_comp.eep"
	@echo 'Finished building: $@'
	@echo ' '

sizedummy: analog_comp.elf
	@echo 'Invoking: Print Size'
	-avr-size --format=avr --mcu=atmega328p analog_comp.elf
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(FLASH_IMAGE)$(ELFS)$(OBJS)$(ASM_DEPS)$(EEPROM_IMAGE)$(S_DEPS)$(SIZEDUMMY)$(S_UPPER_DEPS)$(LSS)$(C_DEPS) analog_comp.elf
	-@echo ' '

secondary-outputs: $(LSS) $(FLASH_IMAGE) $(EEPROM_IMAGE) $(SIZEDUMMY)

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
