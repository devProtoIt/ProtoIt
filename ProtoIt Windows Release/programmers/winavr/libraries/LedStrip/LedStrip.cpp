#include "LedStrip.h"

/////////
// RGB //
/////////

RGB::RGB()
{
	r = g = b = 0;
}

RGB::RGB( const RGB& rgb)
{
	r = rgb.r;
	g = rgb.g;
	b = rgb.b;
}

RGB::RGB( const uint8_t red, const uint8_t green, const uint8_t blue)
{
	r = red;
	g = green;
	b = blue;
	if ( r > 255 ) r = 255;
	if ( g > 255 ) g = 255;
	if ( b > 255 ) b = 255;
}

uint8_t RGB::red()
{
	return r;
}

uint8_t RGB::green()
{
	return g;
}

uint8_t RGB::blue()
{
	return b;
}

const bool RGB::operator== ( const RGB& rgb) const
{
  return (r == rgb.r) && (g == rgb.g) && (b == rgb.b);
}

const bool RGB::operator!= ( const RGB& rgb) const
{
  return (r != rgb.r) || (g != rgb.g) || (b != rgb.b);
}

//////////////
// LEDSTRIP //
//////////////

LedStrip::LedStrip( uint8_t pin, uint8_t ledcount)
{
  pinMask = digitalPinToBitMask( pin);
  pinReg = portOutputRegister( digitalPinToPort( pin));
  pinMode( pin, OUTPUT);
  pinGpio = pin;
  ledCount = 0;
  clr1Matrix = (uint8_t*) malloc( ledcount * 3);
  if( clr1Matrix ) memset( clr1Matrix, 0, ledcount * 3);
#ifdef LED_EFFECTS
  clr2Matrix = (uint8_t*) malloc( ledcount * 3);
  if( clr2Matrix ) memset( clr2Matrix, 0, ledcount * 3);
  stepMatrix = (int8_t*) malloc( ledcount * 3);
  if( stepMatrix ) memset( stepMatrix, 0, ledcount * 3);
  currMatrix = (uint8_t*) malloc( ledcount * 3);
  if( currMatrix ) memset( currMatrix, 0, ledcount * 3);
  if ( clr1Matrix && clr2Matrix && stepMatrix && currMatrix )
	  ledCount = ledcount;
#else
  if ( clr1Matrix )
	  ledCount = ledcount;
#endif
}

LedStrip::~LedStrip()
{
  if ( clr1Matrix ) free( clr1Matrix);
#ifdef LED_EFFECTS
  if ( clr2Matrix ) free( clr2Matrix);
  if ( currMatrix ) free( currMatrix);
  if ( stepMatrix ) free( stepMatrix);
#endif
}

RGB LedStrip::getColor( uint8_t ixled)
{
  if( ixled >= ledCount ) return RGB();

  uint8_t ix = ixled * 3;

#ifdef LED_EFFECTS
  if ( effect )
	return RGB( currMatrix[ix + 1], currMatrix[ix], currMatrix[ix + 2]);
  else
#endif
	return RGB( clr1Matrix[ix + 1], clr1Matrix[ix], clr1Matrix[ix + 2]);
}

void LedStrip::setColor( uint8_t ixled, RGB color1, RGB color2)
{
  if ( ixled >= ledCount ) return;

  prepColor( ixled, color1, color2);

#ifdef LED_EFFECTS
  if ( effect == etFade )
	calcSteps();
#endif
}

void LedStrip::setColorAll( RGB color1, RGB color2)
{
  for( int ix = ledCount - 1; ix >= 0; ix--)
    setColor( ix, color1, color2);

#ifdef LED_EFFECTS
  if ( effect == etFade )
	calcSteps();
#endif
}

#ifdef LED_EFFECTS
void LedStrip::setEffect( EffectType et, uint8_t steps)
{
  if ( !ledCount ) return;

  effect = et;
  maxStep = steps;
  curStep = 0;
  if ( effect == etFade ) {
	updStep = 1;
	calcSteps();
  }
  else {
	updStep = 0;
	memset( stepMatrix, 0, 3 * ledCount);
  }
}

void LedStrip::updateEffect()
{
  if ( !ledCount ) return;

  switch ( effect ) {
	case etBlink :	memcpy( currMatrix, updStep ? clr2Matrix : clr1Matrix, 3 * ledCount);
					updStep = !updStep;
					break;
	case etLeft :	memcpy( currMatrix, &clr1Matrix[3 * curStep], 3 * (ledCount - curStep));
					memcpy( &currMatrix[3 * (ledCount - curStep)], clr1Matrix, 3 * curStep);
					curStep++;
					if ( curStep >= ledCount)
						curStep = 0;
					break;
	case etRight :	memcpy( currMatrix, &clr1Matrix[3 * curStep], 3 * (ledCount - curStep));
					memcpy( &currMatrix[3 * (ledCount - curStep)], clr1Matrix, 3 * curStep);
					curStep--;
					if ( curStep < 0)
						curStep = ledCount - 1;
					break;
	case etFade :   curStep += updStep;
					if ( curStep <= 0 )
					  updStep = 1;
					if ( curStep >= maxStep ) {
					  updStep = -1;
					  curStep -= 1;
					}
					for ( int ix = 0; ix < 3 * ledCount; ix++ )
					  currMatrix[ix] = clr1Matrix[ix] - curStep * stepMatrix[ix];
				    break;
	default :		memcpy( currMatrix, clr1Matrix, 3 * ledCount);
  }
  updateColors();
}

void LedStrip::calcSteps()
{
  int stp;
  for ( int ix = 0; ix < 3 * ledCount; ix++ ) {
	stp = (clr1Matrix[ix] - clr2Matrix[ix]) / maxStep;
	if ( stp < -127 ) stp = -127; // stepMatrix is array int8_t
	if ( stp > 127 ) stp = 127;
	stepMatrix[ix] = stp;
  }
}
#endif

/*
void LedStrip::debugColors( String header)
{
  Serial.print( header);
  for ( int l = 0; l < ledCount; l++ ) {
	int ix = l * 3;
	Serial.print( "\nIndex ");
	Serial.print( ix);
	Serial.print( ":");

	Serial.print( " C1 (g,r,b) ");
	Serial.print( clr1Matrix[ix]);
	Serial.print( " - ");
	Serial.print( clr1Matrix[ix+1]);
	Serial.print( " - ");
	Serial.print( clr1Matrix[ix+2]);
#ifdef LED_EFFECTS
	Serial.print( " C2 (g,r,b) ");
	Serial.print( clr2Matrix[ix]);
	Serial.print( " - ");
	Serial.print( clr2Matrix[ix+1]);
	Serial.print( " - ");
	Serial.print( clr2Matrix[ix+2]);

	Serial.print( " CU (g,r,b) ");
	Serial.print( currMatrix[ix]);
	Serial.print( " - ");
	Serial.print( currMatrix[ix+1]);
	Serial.print( " - ");
	Serial.print( currMatrix[ix+2]);

	Serial.print( " CS (g,r,b) ");
	Serial.print( stepMatrix[ix]);
	Serial.print( " - ");
	Serial.print( stepMatrix[ix+1]);
	Serial.print( " - ");
	Serial.print( stepMatrix[ix+2]);
#endif
  }
  Serial.println( "\n");
}
*/

void LedStrip::prepColor( uint8_t ixled, RGB color1, RGB color2)
{
  uint8_t ix = ixled * 3;
  clr1Matrix[ix] = color1.green();
  clr1Matrix[ix + 1] = color1.red();
  clr1Matrix[ix + 2] = color1.blue();
#ifdef LED_EFFECTS
  clr2Matrix[ix] = color2.green();
  clr2Matrix[ix + 1] = color2.red();
  clr2Matrix[ix + 2] = color2.blue();
#endif
}

/*
  The next routine writes an array of bytes with RGB values to the Dataout pin
  using the fast 800kHz clockless WS2811/2812 protocol.
 */
/* Timing in ns */
#define w_zeropulse (350)
#define w_onepulse  (900)
#define w_totalperiod (1250)

/* Fixed cycles used by the inner loop */
#define w_fixedlow  (3)
#define w_fixedhigh (6)
#define w_fixedtotal (10)

/* Insert NOPs to match the timing, if possible */
#define w_zerocycles ( ( (F_CPU / 1000) * w_zeropulse) / 1000000)
#define w_onecycles ( ( (F_CPU / 1000) * w_onepulse + 500000) / 1000000)
#define w_totalcycles ( ( (F_CPU / 1000) * w_totalperiod + 500000) / 1000000)

/* w1 - nops between rising edge and falling edge - low */
#define w1 (w_zerocycles - w_fixedlow)
/* w2   nops between fe low and fe high */
#define w2 (w_onecycles - w_fixedhigh - w1)
/* w3   nops to complete loop */
#define w3 (w_totalcycles - w_fixedtotal - w1 - w2)

#if w1 > 0
#define w1_nops w1
#else
#define w1_nops 0
#endif

/*
  The only critical timing parameter is the minimum pulse length of the "0"
  Warn or throw error if this timing can not be met with current F_CPU settings.
 */
#define w_lowtime ( (w1_nops + w_fixedlow) * 1000000) / (F_CPU / 1000)
#if w_lowtime > 550
#error "Light_ws2812: Sorry, the clock speed is too low. Did you set F_CPU correctly?"
#elif w_lowtime > 450
#warning "Light_ws2812: The timing is critical and may only work on WS2812B, not on WS2812(S)."
#warning "Please consider a higher clockspeed, if possible"
#endif

#if w2 > 0
#define w2_nops w2
#else
#define w2_nops 0
#endif

#if w3 > 0
#define w3_nops w3
#else
#define w3_nops 0
#endif

#define w_nop1  "nop      \n\t"
#define w_nop2  "rjmp .+0 \n\t"
#define w_nop4  w_nop2 w_nop2
#define w_nop8  w_nop4 w_nop4
#define w_nop16 w_nop8 w_nop8

void LedStrip::updateColors()
{
#ifdef LED_EFFECTS
  uint8_t *data = (effect ? currMatrix : clr1Matrix);
#else
  uint8_t *data = clr1Matrix;
#endif
  uint16_t datlen = 3 * ledCount;
  uint8_t curbyte, ctr;
  uint8_t masklo = *pinReg & ~pinMask;
  uint8_t maskhi = *pinReg | pinMask;

  uint8_t oldSREG = SREG;
  cli();

  while( datlen-- )
  {
    curbyte = *data++;

    asm volatile (
            "       ldi   %0,8  \n\t"
            "loop%=:            \n\t"
            "       st    X,%3 \n\t"        //  '1' [02] '0' [02] - re
#if (w1_nops & 1)
            w_nop1
#endif
#if (w1_nops & 2)
            w_nop2
#endif
#if (w1_nops & 4)
            w_nop4
#endif
#if (w1_nops & 8)
            w_nop8
#endif
#if (w1_nops & 16)
            w_nop16
#endif
            "       sbrs  %1,7  \n\t"       //  '1' [04] '0' [03]
            "       st    X,%4 \n\t"        //  '1' [--] '0' [05] - fe-low
            "       lsl   %1    \n\t"       //  '1' [05] '0' [06]
#if (w2_nops & 1)
            w_nop1
#endif
#if (w2_nops & 2)
            w_nop2
#endif
#if (w2_nops & 4)
            w_nop4
#endif
#if (w2_nops & 8)
            w_nop8
#endif
#if (w2_nops & 16)
            w_nop16
#endif
            "       brcc skipone%= \n\t"    /*  '1' [+1] '0' [+2] - */
            "       st   X,%4      \n\t"    /*  '1' [+3] '0' [--] - fe-high */
            "skipone%=:               "     /*  '1' [+3] '0' [+2] - */

#if (w3_nops & 1)
            w_nop1
#endif
#if (w3_nops & 2)
            w_nop2
#endif
#if (w3_nops & 4)
            w_nop4
#endif
#if (w3_nops & 8)
            w_nop8
#endif
#if (w3_nops & 16)
            w_nop16
#endif

            "       dec   %0    \n\t"       //  '1' [+4] '0' [+3]
            "       brne  loop%=\n\t"       //  '1' [+5] '0' [+4]
            : "=&d" (ctr)
            : "r" (curbyte), "x" (pinReg), "r" (maskhi), "r" (masklo)
    );
  }

  SREG = oldSREG;
  delayMicroseconds( 500);
}
