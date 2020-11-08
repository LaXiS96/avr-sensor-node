PREFIX=/home/laxis/avr/bin

PROJECT=avr-sensor-node

CFLAGS=-mmcu=attiny85 -Wall -DF_CPU=1000000

SOURCES=$(wildcard *.c)
INCLUDES=-I.

all: $(PROJECT).hex

$(PROJECT).out: $(SOURCES)
	$(PREFIX)/avr-gcc $(CFLAGS) $(INCLUDES) -o $@ $?

$(PROJECT).hex: $(PROJECT).out
	$(PREFIX)/avr-objcopy -O ihex $(PROJECT).out $(PROJECT).hex
	$(PREFIX)/avr-size $(PROJECT).out

flash: $(PROJECT).hex
	sudo $(PREFIX)/avrdude -p t85 -c usbasp -U flash:w:$(PROJECT).hex:i

# Default fuses for ATtiny85
fuses:
	sudo $(PREFIX)/avrdude -p t85 -c usbasp -U lfuse:w:0x62:m -U hfuse:w:0xdf:m -U efuse:w:0xff:m

clean:
	rm $(PROJECT).out $(PROJECT).hex
