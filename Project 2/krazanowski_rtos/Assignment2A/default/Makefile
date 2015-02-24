###############################################################################
# Makefile for the project Assignment2
###############################################################################

## General Flags
PROJECT = Assignment2
MCU = atmega2560
TARGET = Assignment2.elf
CC = avr-gcc

CPP = avr-g++

## Options common to compile, link and assembly rules
COMMON = -mmcu=$(MCU)

## Compile options common for all C compilation units.
CFLAGS = $(COMMON)
CFLAGS += -Wall -gdwarf-2 -std=gnu99 -Os -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
CFLAGS += -MD -MP -MT $(*F).o -MF dep/$(@F).d 

## Assembly specific flags
ASMFLAGS = $(COMMON)
ASMFLAGS += $(CFLAGS)
ASMFLAGS += -x assembler-with-cpp -Wa,-gdwarf2

## Linker flags
LDFLAGS = $(COMMON)
LDFLAGS += -nostartfiles -Wl,-Map=Assignment2.map


## Intel Hex file production flags
HEX_FLASH_FLAGS = -R .eeprom -R .fuse -R .lock -R .signature

HEX_EEPROM_FLAGS = -j .eeprom
HEX_EEPROM_FLAGS += --set-section-flags=.eeprom="alloc,load"
HEX_EEPROM_FLAGS += --change-section-lma .eeprom=0 --no-change-warnings


## Objects that must be built in order to link
OBJECTS = Assignment2.o Ass2_Communicate.o Ass2_Servo.o Ass2_Statistics.o ServoController.o radio.o spi.o sonar.o os.o crt0.o uart.o 

## Objects explicitly added by the user
LINKONLYOBJECTS = 

## Build
all: $(TARGET) Assignment2.hex Assignment2.eep Assignment2.lss size

## Compile
Assignment2.o: ../Assignment2.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

Ass2_Communicate.o: ../Ass2_Communicate.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

Ass2_Servo.o: ../Ass2_Servo.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

Ass2_Statistics.o: ../Ass2_Statistics.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

ServoController.o: ../ServoController.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

radio.o: ../radio/radio.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

spi.o: ../radio/spi.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

sonar.o: ../sonar/sonar.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

os.o: ../OS/os.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

crt0.o: ../OS/crt0.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

uart.o: ../uart.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

##Link
$(TARGET): $(OBJECTS)
	 $(CC) $(LDFLAGS) $(OBJECTS) $(LINKONLYOBJECTS) $(LIBDIRS) $(LIBS) -o $(TARGET)

%.hex: $(TARGET)
	avr-objcopy -O ihex $(HEX_FLASH_FLAGS)  $< $@

%.eep: $(TARGET)
	-avr-objcopy $(HEX_EEPROM_FLAGS) -O ihex $< $@ || exit 0

%.lss: $(TARGET)
	avr-objdump -h -S $< > $@

size: ${TARGET}
	@echo
	@avr-size -C --mcu=${MCU} ${TARGET}

## Clean target
.PHONY: clean
clean:
	-rm -rf $(OBJECTS) Assignment2.elf dep/* Assignment2.hex Assignment2.eep Assignment2.lss Assignment2.map


## Other dependencies
-include $(shell mkdir dep 2>/dev/null) $(wildcard dep/*)

