echo "Clean..."
rm -f *.o
rm -f *.elf
rm -f *.hex

echo "Compile: tests/" $1
cp tests/$1.c test.c
avr-gcc -Wall -O2 -DF_CPU=16000000UL -mmcu=atmega2560 -c test.c -o main.o
avr-gcc -Wall -O2 -DF_CPU=16000000UL -mmcu=atmega2560 -c os.c -o os.o
rm -f test.c

echo "Link..."
avr-gcc -Wall -O2 -DF_CPU=16000000UL -mmcu=atmega2560 -o main.elf os.o main.o

echo "Make HEX..."
avr-objcopy -j .text -j .data -O ihex main.elf main.hex

echo "Uploading..."
sudo avrdude -p m2560 -c wiring -P /dev/tty.usbmodem1411 -U flash:w:main.hex:i
