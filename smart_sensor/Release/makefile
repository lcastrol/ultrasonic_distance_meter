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
smart_sensor.lss \

FLASH_IMAGE += \
smart_sensor.hex \

EEPROM_IMAGE += \
smart_sensor.eep \

SIZEDUMMY += \
sizedummy \


# All Target
all: smart_sensor.elf secondary-outputs

# Tool invocations
smart_sensor.elf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: AVR C Linker'
	avr-gcc -Wl,-Map,smart_sensor.map -mmcu=atmega328p -o "smart_sensor.elf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

smart_sensor.lss: smart_sensor.elf
	@echo 'Invoking: AVR Create Extended Listing'
	-avr-objdump -h -S smart_sensor.elf  >"smart_sensor.lss"
	@echo 'Finished building: $@'
	@echo ' '

smart_sensor.hex: smart_sensor.elf
	@echo 'Create Flash image (ihex format)'
	-avr-objcopy -R .eeprom -R .fuse -R .lock -R .signature -O ihex smart_sensor.elf  "smart_sensor.hex"
	@echo 'Finished building: $@'
	@echo ' '

smart_sensor.eep: smart_sensor.elf
	@echo 'Create eeprom image (ihex format)'
	-avr-objcopy -j .eeprom --no-change-warnings --change-section-lma .eeprom=0 -O ihex smart_sensor.elf  "smart_sensor.eep"
	@echo 'Finished building: $@'
	@echo ' '

sizedummy: smart_sensor.elf
	@echo 'Invoking: Print Size'
	-avr-size --format=avr --mcu=atmega328p smart_sensor.elf
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(FLASH_IMAGE)$(ELFS)$(OBJS)$(ASM_DEPS)$(EEPROM_IMAGE)$(S_DEPS)$(SIZEDUMMY)$(S_UPPER_DEPS)$(LSS)$(C_DEPS) smart_sensor.elf
	-@echo ' '

secondary-outputs: $(LSS) $(FLASH_IMAGE) $(EEPROM_IMAGE) $(SIZEDUMMY)

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
