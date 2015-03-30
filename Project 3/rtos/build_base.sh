echo "Clean..."
rm -f *.o
rm -f *.elf
rm -f *.hex

echo "Compile: base_station"
cp base_station/main.c main.c
avr-gcc -Wall -O2 -DF_CPU=16000000UL -mmcu=atmega2560 -c main.c -o main.o
avr-gcc -Wall -O2 -DF_CPU=16000000UL -mmcu=atmega2560 -c cops_and_robbers.c -o cops_and_robbers.o
avr-gcc -Wall -O2 -DF_CPU=16000000UL -mmcu=atmega2560 -c spi.c -o spi.o
avr-gcc -Wall -O2 -DF_CPU=16000000UL -mmcu=atmega2560 -c radio.c -o radio.o
avr-gcc -Wall -O2 -DF_CPU=16000000UL -mmcu=atmega2560 -c os.c -o os.o
rm -f main.c

echo "Link..."
avr-gcc -Wall -O2 -DF_CPU=16000000UL -mmcu=atmega2560 -o main.elf os.o cops_and_robbers.o spi.o radio.o main.o

echo "Make HEX..."
avr-objcopy -j .text -j .data -O ihex main.elf main.hex

echo "Uploading..."
sudo avrdude -p m2560 -c wiring -P /dev/tty.usbmodem1411 -U flash:w:main.hex:i

echo "Post Compile Clean..."
rm -f *.o
rm -f *.elf
rm -f main.c
