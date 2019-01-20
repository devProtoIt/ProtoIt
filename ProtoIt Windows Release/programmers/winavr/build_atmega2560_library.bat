rem COMPILE CORE LIBRARIES
winavr\bin\avr-gcc -c -g -Os -w -ffunction-sections -fdata-sections -MMD -mmcu=atmega2560 -DF_CPU=16000000L -DARDUINO=152 -Icores/arduino -Ivariants/mega cores/arduino/hooks.c -o build/hooks.c.o 
winavr\bin\avr-gcc -c -g -Os -w -ffunction-sections -fdata-sections -MMD -mmcu=atmega2560 -DF_CPU=16000000L -DARDUINO=152 -Icores/arduino -Ivariants/mega cores/arduino/malloc.c -o build/malloc.c.o 
winavr\bin\avr-gcc -c -g -Os -w -ffunction-sections -fdata-sections -MMD -mmcu=atmega2560 -DF_CPU=16000000L -DARDUINO=152 -Icores/arduino -Ivariants/mega cores/arduino/WInterrupts.c -o build/WInterrupts.c.o 
winavr\bin\avr-gcc -c -g -Os -w -ffunction-sections -fdata-sections -MMD -mmcu=atmega2560 -DF_CPU=16000000L -DARDUINO=152 -Icores/arduino -Ivariants/mega cores/arduino/wiring.c -o build/wiring.c.o 
winavr\bin\avr-gcc -c -g -Os -w -ffunction-sections -fdata-sections -MMD -mmcu=atmega2560 -DF_CPU=16000000L -DARDUINO=152 -Icores/arduino -Ivariants/mega cores/arduino/wiring_analog.c -o build/wiring_analog.c.o 
winavr\bin\avr-gcc -c -g -Os -w -ffunction-sections -fdata-sections -MMD -mmcu=atmega2560 -DF_CPU=16000000L -DARDUINO=152 -Icores/arduino -Ivariants/mega cores/arduino/wiring_digital.c -o build/wiring_digital.c.o 
winavr\bin\avr-gcc -c -g -Os -w -ffunction-sections -fdata-sections -MMD -mmcu=atmega2560 -DF_CPU=16000000L -DARDUINO=152 -Icores/arduino -Ivariants/mega cores/arduino/wiring_pulse.c -o build/wiring_pulse.c.o 
winavr\bin\avr-gcc -c -g -Os -w -ffunction-sections -fdata-sections -MMD -mmcu=atmega2560 -DF_CPU=16000000L -DARDUINO=152 -Icores/arduino -Ivariants/mega cores/arduino/wiring_shift.c -o build/wiring_shift.c.o 
winavr\bin\avr-g++ -c -g -Os -w -fno-exceptions -ffunction-sections -fdata-sections -MMD -mmcu=atmega2560 -DF_CPU=16000000L -DARDUINO=152 -Icores/arduino -Ivariants/mega cores/arduino/CDC.cpp -o build/CDC.cpp.o 
winavr\bin\avr-g++ -c -g -Os -w -fno-exceptions -ffunction-sections -fdata-sections -MMD -mmcu=atmega2560 -DF_CPU=16000000L -DARDUINO=152 -Icores/arduino -Ivariants/mega cores/arduino/HardwareSerial.cpp -o build/HardwareSerial.cpp.o 
winavr\bin\avr-g++ -c -g -Os -w -fno-exceptions -ffunction-sections -fdata-sections -MMD -mmcu=atmega2560 -DF_CPU=16000000L -DARDUINO=152 -Icores/arduino -Ivariants/mega cores/arduino/HID.cpp -o build/HID.cpp.o 
winavr\bin\avr-g++ -c -g -Os -w -fno-exceptions -ffunction-sections -fdata-sections -MMD -mmcu=atmega2560 -DF_CPU=16000000L -DARDUINO=152 -Icores/arduino -Ivariants/mega cores/arduino/IPAddress.cpp -o build/IPAddress.cpp.o 
winavr\bin\avr-g++ -c -g -Os -w -fno-exceptions -ffunction-sections -fdata-sections -MMD -mmcu=atmega2560 -DF_CPU=16000000L -DARDUINO=152 -Icores/arduino -Ivariants/mega cores/arduino/main.cpp -o build/main.cpp.o 
winavr\bin\avr-g++ -c -g -Os -w -fno-exceptions -ffunction-sections -fdata-sections -MMD -mmcu=atmega2560 -DF_CPU=16000000L -DARDUINO=152 -Icores/arduino -Ivariants/mega cores/arduino/new.cpp -o build/new.cpp.o 
winavr\bin\avr-g++ -c -g -Os -w -fno-exceptions -ffunction-sections -fdata-sections -MMD -mmcu=atmega2560 -DF_CPU=16000000L -DARDUINO=152 -Icores/arduino -Ivariants/mega cores/arduino/Print.cpp -o build/Print.cpp.o 
winavr\bin\avr-g++ -c -g -Os -w -fno-exceptions -ffunction-sections -fdata-sections -MMD -mmcu=atmega2560 -DF_CPU=16000000L -DARDUINO=152 -Icores/arduino -Ivariants/mega cores/arduino/Stream.cpp -o build/Stream.cpp.o 
winavr\bin\avr-g++ -c -g -Os -w -fno-exceptions -ffunction-sections -fdata-sections -MMD -mmcu=atmega2560 -DF_CPU=16000000L -DARDUINO=152 -Icores/arduino -Ivariants/mega cores/arduino/Tone.cpp -o build/Tone.cpp.o 
winavr\bin\avr-g++ -c -g -Os -w -fno-exceptions -ffunction-sections -fdata-sections -MMD -mmcu=atmega2560 -DF_CPU=16000000L -DARDUINO=152 -Icores/arduino -Ivariants/mega cores/arduino/USBCore.cpp -o build/USBCore.cpp.o 
winavr\bin\avr-g++ -c -g -Os -w -fno-exceptions -ffunction-sections -fdata-sections -MMD -mmcu=atmega2560 -DF_CPU=16000000L -DARDUINO=152 -Icores/arduino -Ivariants/mega cores/arduino/WMath.cpp -o build/WMath.cpp.o 
winavr\bin\avr-g++ -c -g -Os -w -fno-exceptions -ffunction-sections -fdata-sections -MMD -mmcu=atmega2560 -DF_CPU=16000000L -DARDUINO=152 -Icores/arduino -Ivariants/mega cores/arduino/WString.cpp -o build/WString.cpp.o 
rem COMPILE STANDARD LIBRARIES
winavr\bin\avr-g++ -c -g -Os -w -fno-exceptions -ffunction-sections -fdata-sections -MMD -mmcu=atmega2560 -DF_CPU=16000000 -DARDUINO=152 -Icores/arduino -Ivariants/mega libraries/Variant/Variant.cpp -o build/Variant.cpp.o
winavr\bin\avr-g++ -c -g -Os -w -fno-exceptions -ffunction-sections -fdata-sections -MMD -mmcu=atmega2560 -DF_CPU=16000000 -DARDUINO=152 -Icores/arduino -Ivariants/mega -Ilibraries libraries/Infento/Infento.cpp -o build/Infento.cpp.o
winavr\bin\avr-g++ -c -g -Os -w -fno-exceptions -ffunction-sections -fdata-sections -MMD -mmcu=atmega2560 -DF_CPU=16000000 -DARDUINO=152 -Icores/arduino -Ivariants/mega libraries/Wire/Wire.cpp -o build/Wire.cpp.o
winavr\bin\avr-gcc -c -g -Os -w -ffunction-sections -fdata-sections -MMD -mmcu=atmega2560 -DF_CPU=16000000L -DARDUINO=152 -Icores/arduino -Ivariants/mega libraries/Wire/utility/twi.c -o build/twi.c.o 
winavr\bin\avr-g++ -c -g -Os -w -fno-exceptions -ffunction-sections -fdata-sections -MMD -mmcu=atmega2560 -DF_CPU=16000000 -DARDUINO=152 -Icores/arduino -Ivariants/mega libraries/OneWire/OneWire.cpp -o build/OneWire.cpp.o

rem LINK CORE LIBRARIES
winavr\bin\avr-ar rcs core_atmega2560.a build/hooks.c.o 
winavr\bin\avr-ar rcs core_atmega2560.a build/malloc.c.o 
winavr\bin\avr-ar rcs core_atmega2560.a build/WInterrupts.c.o 
winavr\bin\avr-ar rcs core_atmega2560.a build/wiring.c.o 
winavr\bin\avr-ar rcs core_atmega2560.a build/wiring_analog.c.o 
winavr\bin\avr-ar rcs core_atmega2560.a build/wiring_digital.c.o 
winavr\bin\avr-ar rcs core_atmega2560.a build/wiring_pulse.c.o 
winavr\bin\avr-ar rcs core_atmega2560.a build/wiring_shift.c.o 
winavr\bin\avr-ar rcs core_atmega2560.a build/CDC.cpp.o 
winavr\bin\avr-ar rcs core_atmega2560.a build/HardwareSerial.cpp.o 
winavr\bin\avr-ar rcs core_atmega2560.a build/HID.cpp.o 
winavr\bin\avr-ar rcs core_atmega2560.a build/IPAddress.cpp.o 
winavr\bin\avr-ar rcs core_atmega2560.a build/main.cpp.o 
winavr\bin\avr-ar rcs core_atmega2560.a build/new.cpp.o 
winavr\bin\avr-ar rcs core_atmega2560.a build/Print.cpp.o 
winavr\bin\avr-ar rcs core_atmega2560.a build/Stream.cpp.o 
winavr\bin\avr-ar rcs core_atmega2560.a build/Tone.cpp.o 
winavr\bin\avr-ar rcs core_atmega2560.a build/USBCore.cpp.o 
winavr\bin\avr-ar rcs core_atmega2560.a build/WMath.cpp.o 
winavr\bin\avr-ar rcs core_atmega2560.a build/WString.cpp.o 
rem LINK STANDARD LIBRARIES
winavr\bin\avr-ar rcs core_atmega2560.a build/Variant.cpp.o 
winavr\bin\avr-ar rcs core_atmega2560.a build/Infento.cpp.o 
winavr\bin\avr-ar rcs core_atmega2560.a build/Wire.cpp.o 
winavr\bin\avr-ar rcs core_atmega2560.a build/twi.c.o 
winavr\bin\avr-ar rcs core_atmega2560.a build/OneWire.cpp.o 
