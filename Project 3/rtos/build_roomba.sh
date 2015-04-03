echo "Clean..."
rm -f *.o
rm -f *.elf
rm -f *.hex

echo "Prepare: roomba"
cp roomba/main.c main.c
cp roomba/sensor_struct.h sensor_struct.h
cp roomba/uart.h uart.h
cp roomba/blocking_uart.h blocking_uart.h
cp roomba/uart.c uart.c
cp roomba/roomba.h roomba.h
cp roomba/roomba_sci.h roomba_sci.h
cp roomba/roomba.c roomba.c
cp roomba/ir.h ir.h
cp roomba/ir.c ir.c

echo "Compile: roomba"
avr-gcc -Wall -O2 -DF_CPU=16000000UL -mmcu=atmega2560 -c main.c -o main.o
avr-gcc -Wall -O2 -DF_CPU=16000000UL -mmcu=atmega2560 -c cops_and_robbers.c -o cops_and_robbers.o
avr-gcc -Wall -O2 -DF_CPU=16000000UL -mmcu=atmega2560 -c spi.c -o spi.o
avr-gcc -Wall -O2 -DF_CPU=16000000UL -mmcu=atmega2560 -c radio.c -o radio.o
avr-gcc -Wall -O2 -DF_CPU=16000000UL -mmcu=atmega2560 -c os.c -o os.o
avr-gcc -Wall -O2 -DF_CPU=16000000UL -mmcu=atmega2560 -c uart.c -o uart.o
avr-gcc -Wall -O2 -DF_CPU=16000000UL -mmcu=atmega2560 -c roomba.c -o roomba.o
avr-gcc -Wall -O2 -DF_CPU=16000000UL -mmcu=atmega2560 -c ir.c -o ir.o

echo "Link..."
avr-gcc -Wall -O2 -DF_CPU=16000000UL -mmcu=atmega2560 -o main.elf os.o cops_and_robbers.o spi.o radio.o main.o uart.o roomba.o ir.o

echo "Make HEX..."
avr-objcopy -j .text -j .data -O ihex main.elf main.hex

echo "Clean: roomba"
rm -f main.c
rm -f sensor_struct.h
rm -f uart.h
rm -f blocking_uart.h
rm -f uart.c
rm -f roomba.h
rm -f roomba_sci.h
rm -f roomba.c
rm -f ir.h
rm -f ir.c

echo "Uploading..."
sudo avrdude -p m2560 -c wiring -P /dev/tty.usbmodem1411 -U flash:w:main.hex:i

echo "Post Compile Clean..."
rm -f *.o
rm -f *.elf

