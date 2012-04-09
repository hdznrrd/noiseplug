
TARGETS=np.hex sound1

all: $(TARGETS)

clean:
	rm -f $(TARGETS) np.elf sound1.o 

.PHONY: flash clean

flash: np.hex
	-sudo dfu-programmer atmega32u4 erase --suppress-validation
	-sudo dfu-programmer atmega32u4 flash np.hex
	-sudo dfu-programmer atmega32u4 start

np.hex: np.elf
	avr-objcopy -R .eeprom -O ihex np.elf np.hex

np.elf: noiseplug.c sound1.c
	avr-gcc -o np.elf noiseplug.c -Os -mmcu=atmega32u4

sound1: sound1.o

sound1.o: sound1.c
