winavr\bin\avr-g++ -c -g -Os -w -fno-exceptions -ffunction-sections -fdata-sections -MMD -mmcu=atmega2560 -DF_CPU=16000000 -DARDUINO=152 -Icores/arduino -Ivariants/mega -Ilibraries/Variant -Ilibraries/Infento -Ilibraries/Wire build/RoboTiles.cpp -o build/RoboTiles.cpp.o

winavr\bin\avr-gcc -Os -Wl,--gc-sections -mmcu=atmega2560 -o build/RoboTiles.cpp.elf build/RoboTiles.cpp.o core_atmega2560.a -Lbuild -lm 
winavr\bin\avr-objcopy -O ihex -j .eeprom --set-section-flags=.eeprom=alloc,load --no-change-warnings --change-section-lma .eeprom=0 build/RoboTiles.cpp.elf build/RoboTiles.cpp.eep 
winavr\bin\avr-objcopy -O ihex -R .eeprom build/RoboTiles.cpp.elf build/RoboTiles.cpp.hex
winavr\bin\avrdude -Cwinavr/bin/avrdude.conf -q -q -patmega2560 -cwiring -PCOM4 -b115200 -D -Uflash:w:build/RoboTiles.cpp.hex:i 
