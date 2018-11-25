#ifndef __LedStrip__
#define __LedStrip__

#include <stdint.h>
#include <stdbool.h>
#include <Arduino.h>

#define LED_EFFECTS

class RGB
{
public:

	RGB();
	RGB( const RGB& rgb);
	RGB( const uint8_t red, const uint8_t green, const uint8_t blue);

	uint8_t red();
	uint8_t green();
	uint8_t blue();

	const bool operator== ( const RGB& rgb) const;
	const bool operator!= ( const RGB& rgb) const;

private:

	uint8_t r;
	uint8_t g;
	uint8_t b;
};

class LedStrip
{
public:

  LedStrip( uint8_t pin, uint8_t ledcount);
  ~LedStrip();

  RGB  getColor( uint8_t ixled);
  void setColor( uint8_t ixled, RGB color1, RGB color2 = RGB());
  void setColorAll( RGB color1, RGB color2 = RGB());
  void updateColors();

#ifdef LED_EFFECTS
  enum EffectType {etOff, etBlink, etFade, etLeft, etRight};

  void setEffect( EffectType et, uint8_t steps = 0);
  void updateEffect();
#endif

//  void debugColors( String header);

private:

  void calcSteps();
  void prepColor( uint8_t ixled, RGB color1, RGB color2);

  uint16_t  ledCount;
  uint8_t   *clr1Matrix;

#ifdef LED_EFFECTS
  uint8_t   *clr2Matrix;
  uint8_t   *currMatrix;
  int8_t    *stepMatrix;

  EffectType effect;
  uint8_t	maxStep;
  int8_t	curStep;
  int8_t	updStep;
#endif

  uint8_t   pinGpio;
  uint8_t   pinMask;
  volatile uint8_t *pinReg;
};

#endif
