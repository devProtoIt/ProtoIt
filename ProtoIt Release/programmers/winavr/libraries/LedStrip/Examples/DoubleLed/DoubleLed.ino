#include "Arduino.h"
#include "LedStrip.h"

// the data line of the strip is connected to arduino pin 13
// the led strip contains two leds
LedStrip leds( 13, 2);


void setup()
{
  // prepare the rgb colors of led 0 and 1
  leds.setColor( 0, RGB( 0, 0, 128));
  leds.setColor( 1, RGB( 0, 128, 0));

  // send the colors to the led strip
  leds.updateColors();
}

void loop() {
}



