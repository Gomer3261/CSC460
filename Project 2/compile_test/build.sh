echo "Clean..."
rm -f *.o
rm -f *.elf
rm -f *.hex

echo "Compile..."
avr-gcc -Wall -O2 -DF_CPU=16000000UL -mmcu=atmega2560 -c main.c -o main.o

echo "Link..."
avr-gcc -Wall -O2 -DF_CPU=16000000UL -mmcu=atmega2560 -o main.elf main.o

echo "Make HEX..."
avr-objcopy -j .text -j .data -O ihex main.elf main.hex

echo "Uploading..."
sudo avrdude -p m2560 -c wiring -P /dev/tty.usbmodem1411 -U flash:w:main.hex:i
