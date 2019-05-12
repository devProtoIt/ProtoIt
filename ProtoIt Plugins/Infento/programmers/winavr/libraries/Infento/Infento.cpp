// Copyright (C) 2018 Infento rides
//
// GNU General Public License Usage
// --------------------------------
// This file may be used under the terms of the GNU General Public
// License version 3.0 as published by the Free Software Foundation
// and appearing in the file 'GNU-GPL.txt' included in the packaging
// of this file.  Please review the following information to ensure
// the GNU General Public License version 3.0 requirements will be
// met: http://www.gnu.org/copyleft/gpl.html.

/*
 SINCE THE PCF8574xx DIRECTOR CHIP OCCUPIES THE I2C ADDRESS DEFINED BY 'ADDR_PCF_BUS' (0x38),
 I2C DEVICES HAVING THIS ADDRESS CANNOT BE CONNECTED TO THE INFENTO CONTROLLER
*/          

#include "Infento.h"
#include "Wire.h"

bool INF_WAITATSTART = true;
bool INF_DEBUG_DEVICE = false;
String IfpStr = "InfentoPort";
String IfdStr = "InfentoDevice";

/////////////////
// definitions //
/////////////////

typedef union {
  uint16_t	i;
  uint8_t	b[2];
} BTOI;

typedef union {
  uint32_t	l;
  uint8_t	b[4];
} BTOL;

#define ADDR_DIG_MODE	0x21	// i2c address of the digital port mode switcher chip
#define ADDR_DIG_LED	0x39	// i2c address of the digital led switcher chip
#define ADDR_ANA_MODE	0x22	// i2c address of the analogue port mode switcher chip
#define ADDR_ANA_LED	0x3A	// i2c address of the analogue led switcher chip

#define ADDR_I2C_SWITCH	0x70	// i2c address of the i2c bus switcher chip

#define I2C_BUS_CMD		0x01	// switch i2c bus to mode switcher chip in order to set port mode
#define I2C_BUS_IB1		0x02	// switch i2c bus to digital ports
#define I2C_BUS_IB2		0x04	// switch i2c bus to analogue ports
#define I2C_BUS_IB3		0x08	// switch i2c bus to nxt port

#define IFT_DIGIPORT	0		// port type: the port has a BTD connector (digital port)
#define IFT_ANAPORT		1		// port type: the port has a BTA connector (analogue port)
#define IFT_NXTPORT		2		// port type: the port has a NXT connector (digital port)

#define IFB_ACTUATOR	1		// id of 'ActuatorIO' board
#define IFB_SENSOR		2		// id of 'SensorIO' board

#define LED_HEARTBEAT	40		// Arduino pin that has the heart beat led
#define SWT_STARTSTOP	24		// Arduino pin that has the start/stop switch
#define SWT_EMERGENCY	25		// Arduino pin that has the emergency switch

//////////////////////
// global variables //
//////////////////////

InfentoPort IFP[IFP_PORTS];

int	PIN[IFP_PORTS*IFP_LINES] = { 	2, 5, 8, 11,		3, 6, 9, 12,		4, 7, 10, 13,		50, 51, 52, 53,
// DIO busses on pcb:				P0, P1, P2, P3,		P4, P5, P6, P7,		P8, P9, P10,P11,	P12,P13,P14,P15
									A0, A4,A8, A12,		A1, A5, A9, A13,	A2, A6, A10, A14,	A3, A7, A11, A15,
// AIO busses on pcb: 				AD0,AD1,AD2,nc,		AD3,AD4,AD5,nc,		AD6,AD7,AD8,nc,		AD9,AD10,AD11,nc
									45, 46, 45, 46 };
// NXT busses on pcb: 				NX1, NX2, nc, nc

byte ifg_mode[IFT_NXTPORT];	// ifg_mode[IFT_DIGIPORT] contains the current port mode of all digital ports
							// ifg_mode[IFT_ANAPORT] contains the current port mode of all analogue ports
							// per set of ports the bit occupation is configured as '33221100'

byte ifg_led[IFT_NXTPORT];	// ifg_led[IFT_DIGIPORT] contains the current status of the digital port leds
							// ifg_led[IFT_ANAPORT] contains the current status of the diganalogue port leds
							// per set of ports the bit occupation is configured as 'XXXX3210'

bool ifg_incomplete = false; // is set by findDevice

long g_hbtime = 0;			 // time of the heart beat led
bool g_hbled = false;		 // heart beat led is on or off

String NOPORT = "no port";


/////////////////////
// global routines //
/////////////////////

// DEBUGROUTINE
// Formats debug information and sends it over serial 1 to a debug monitor.
// - name: routine name.
// - comment1: extra information.
// - comment2: extra information.
// - comment3: extra information.
void debugroutine( String name, String comment1 = "", String comment2 = "", String comment3 = "")
{
	String str;
	Serial.println( "\n[" + name + "]");
	for ( int i =0; i < name.length() + 2; i++ )
		str += '=';
	Serial.println( str);
	
	if ( comment1 != "" )
		Serial.println( "      - " + comment1);
	if ( comment2 != "" )
		Serial.println( "      - " + comment2);
	if ( comment3 != "" )
		Serial.println( "      - " + comment3);
}

// RESTART
// On an arduino a call to a NULL pointer will restart the 'setup' and program 'loop'.
// IMPORTANT NOTE: This is NOT a reset of the arduino and globally declarations will
// not be initialized to their original value. Be sure that all initializing is done
// in the arduino's 'setup' routine.
void (* restart)(void) = 0;

// SWITCHI2C
// Switches the Infento controller to the required i2c bus.
// - bus: I2C_BUS_CMD = i2c bus needed to switch a port mode.
//        I2C_BUS_IB1 = i2c bus needed to communicate to an i2c device connected to an BTD port.
//        I2C_BUS_IB2 = i2c bus needed to communicate to an i2c device connected to an BTA port.
//        I2C_BUS_IB3 = i2c bus needed to communicate to an i2c device connected to the NXT port.
void switchI2C( int bus)
{
	#ifdef INF_DEBUG_SWITCH
		debugroutine( "switchI2C");
	#endif

	// set i2c switch via the PCF with address ADDR_I2C_SWITCH
	// switches between I2C_BUS_CMD, I2C_BUS_IB1, I2C_BUS_IB2 and I2C_BUS_IB3
	Wire.beginTransmission( ADDR_I2C_SWITCH); // 0x70
	Wire.write( bus);
	Wire.endTransmission();

	#ifdef INF_DEBUG_SWITCH
		Serial.println( "  PCF-i2c-switcher changes i2c bus");
		Serial.print( "      - to: ");
		switch ( bus ) {
			case I2C_BUS_CMD :  Serial.println( "CMD (PCF-mode-switcher)"); break;
			case I2C_BUS_IB1 :  Serial.println( "IB1 (BTD bus)"); break;
			case I2C_BUS_IB2 :  Serial.println( "IB2 (BTA bus)"); break;
			case I2C_BUS_IB3 :  Serial.println( "IB3 (NXT bus)"); break;
		}
	#endif
}

// SWITCHMODE
// Switches an Infento controllers port to the required mode.
// Call 'switchI2C(I2C_BUS_CMD)' first. Be sure to call 'switchI2C' afterwards once more
// enabling the communication to the ports.
// - port: the controllers port (0 through IFP_PORTS).
// - portmode: IFM_I2C, IFM_UART, IFM_DATA or IFM_REF (see 'setPortMode' of 'InfentoPort' class).
// Note that IFM_UART mode communicates over the serial 2 interface of the arduino.
void switchMode( int port, int portmode)
{
	#ifdef INF_DEBUG_SWITCH
		debugroutine( "switchMode");
	#endif

	if ( port >= IFP_DIGIPORTS + IFP_ANAPORTS ) return;

	int type = port / IFP_DIGIPORTS;
	int connector = port - type * IFP_DIGIPORTS;
	int address;
	
	#ifdef INF_DEBUG_SWITCH
		Serial.println( "  PCF-mode-switcher changes mode");
		Serial.print( "      - of port ");
		Serial.println( port);
		Serial.print( "      - ifg_mode before: ");
		Serial.println( ifg_mode[type], BIN);
	#endif
	
	ifg_mode[ type] = (ifg_mode[ type] & (0xFF ^ (0x03 << (connector * 2)))) | (portmode << (connector * 2));
	
	#ifdef INF_DEBUG_SWITCH
		Serial.print( "      - ifg_mode after:  ");
		Serial.println( ifg_mode[type], BIN);
	#endif
	
	address = ( port < IFP_DIGIPORTS ? ADDR_DIG_MODE : ADDR_ANA_MODE);
	Wire.beginTransmission( address);
	Wire.write( ifg_mode[ type]);
	Wire.endTransmission();
}


/////////////////////////
// InfentoPort members //
/////////////////////////

// INFENTOPORT
// Class constructor.
InfentoPort::InfentoPort()
{
	init();
}

// PORTTYPE
// Gives back if it is a digital, analogue or nxt port.
// - returns: CONST_DIGITAL, CONST_ANALOGUE or CONST_NXT.
InfentoDefinition InfentoPort::portType()
{
	if ( m_port < IFP_DIGIPORTS )
		return CONST_DIGITAL;
	else
	if ( m_port < IFP_DIGIPORTS + IFP_ANAPORTS )
		return CONST_ANALOGUE;
	return CONST_NXT;
}

// PORT
// Gives the associated port with the port label.
// - label: port label, like "DP1" for the port with index 0.
// - returns: pointer to the associated infento port class.
InfentoPort* InfentoPort::port( String label)
{
	if ( label == "DP1" ) return &IFP[0];
	if ( label == "DP2" ) return &IFP[1];
	if ( label == "DP3" ) return &IFP[2];
	if ( label == "DP4" ) return &IFP[3];
	if ( label == "AP1" ) return &IFP[4];
	if ( label == "AP2" ) return &IFP[5];
	if ( label == "AP3" ) return &IFP[6];
	if ( label == "AP4" ) return &IFP[7];
	if ( label == "NP1" ) return &IFP[8];
	return NULL;
}

// PORT
// Gives the port number.
// - returns: the port number as a value from 0 through 8.
int InfentoPort::port()
{
	return m_port;
}

// PORTLABEL
// Gives the port label.
// - returns: the port label like "DP1" for digital port 0.
String InfentoPort::portLabel( int ix)
{
	if ( ix < 0 || ix > 8 ) return "";
	
	String str;
	if ( ix < 4 ) { str = "DP"; str += char( ix + 49); }
	else
	if ( ix < 8 ) { str = "AP"; str += char( ix + 45); }
	else
		str = "NP1";
	return str;
}

// INIT (protected)
// Puts a port in idle state by deactivating its counters and indexes. 
void InfentoPort::init()
{
	m_port = -1;
	m_portmode = -1;
	m_bus = -1;
	m_pinix = -1;
    m_addrcnt = 0;
	for ( int i = 0; i < MAXBOARDS; i++ ) {
		m_address[i] = 0;
		m_attention[i] = 0;
		m_reset[i] = false;
	}
}

// RESETBOARD
// Sends a reset command to an Infento board.
// Records which board has got a reset command in order to avoid repeated resets.
// - i2c_address: the i2c address of the board.
void InfentoPort::resetBoard( int i2c_address)
{
	for ( int i = 0; i < MAXBOARDS; i++ )
		if ( (m_address[i] == i2c_address) && !m_reset[i] ) {
			byte comm = CMD_RESET;
			write( &comm, 1, i2c_address);
			m_reset[i] = true;
			delay( 20);
			break;
		}
}

// BEGIN (static)
// This must be the first routine called in the arduino's setup routine.
// Initializes the ports and begins the communication over wire, serial port 1 and 2.
// - baud: baud rate for communication over serial port 2 of the arduino.
// (Communicates over serial port 1 at a baud rate of 9600.)
void InfentoPort::begin( int baud)
{
	int i;

	#ifdef INF_DEBUG_PORT
		debugroutine( IfpStr + "::begin");
	#endif

	Wire.begin();				// start i2c for switching commands and device communication
	Serial.begin( 9600);		// start UART 0 (usb) for debugging
	Serial2.begin( baud);		// start UART 2 for device communication

	pinMode( LED_HEARTBEAT, OUTPUT);
	pinMode( SWT_EMERGENCY, INPUT);

	// setup all ports
	for ( i = 0; i < IFT_NXTPORT; i++ ) {
		ifg_mode[i] = 0x00;		// set all ports to i2c mode
		ifg_led[i] = 0xFF;		// set all leds to turned off
	}

	for ( i = 0; i < IFP_PORTS; i++ )
		IFP[i].portSetup( i);	// setup the ports of the global port array

	// actually switch all ports to i2c mode
	#ifdef INF_DEBUG_PORT
		Serial.println( "  Change all port modes to i2c.\n");
	#endif

	switchI2C( I2C_BUS_CMD);
	Wire.beginTransmission( ADDR_DIG_MODE);
	Wire.write( 0);
	Wire.endTransmission();
	Wire.beginTransmission( ADDR_ANA_MODE);
	Wire.write( 0);
	Wire.endTransmission();
	switchI2C( I2C_BUS_IB1);

	if ( INF_WAITATSTART ) {
		// start executing after the emergency button has been pressed and released
		blinkHB( false);
		delay( 100); // needed to pass through the button jitter
		while ( !digitalRead( SWT_STARTSTOP) );
		digitalWrite( LED_HEARTBEAT, HIGH);
		delay( 100); // needed to pass through the button jitter
	}
}

// BLINKHB (static, protected)
// Blinks the heart beat led (long on/short off) to indicate that the program halted.
// Waits to (re)start the program.
// - stop: after pressing the start/stop button the program is(TRUE) restarted or (FALSE) continued.
void InfentoPort::blinkHB( bool stop)
{
	long tm = millis();
	digitalWrite( LED_HEARTBEAT, LOW);
	// loop will continue if 'stop' is true until the start/stop button is pressed
	while ( stop || digitalRead( SWT_STARTSTOP) ) {
		if ( tm + 1500 < millis() )
			digitalWrite( LED_HEARTBEAT, HIGH);
		if ( tm + 2000 < millis() ) {
			digitalWrite( LED_HEARTBEAT, LOW);
			tm = millis();
		}
		if ( stop && !digitalRead( SWT_STARTSTOP) )
			// software restart (DOES NOT RESET GLOBALS)
			restart();
	}
}

// HALTONSTOP (static)
// Halts the program if the start/stop button was pressed.
// The program may not continue but can be restarted (see 'blinkHB( true)').
void InfentoPort::haltOnStop()
{
	if ( !digitalRead( SWT_STARTSTOP) ) {
		// call for a fake emergency on all i2c ports
		InfentoPort *ifp;
		for ( int port = 0; port < IFP_PORTS; port++ ) {
			ifp = &IFP[port];
			if ( ifp->m_addrcnt ) {
				pinMode( PIN[ifp->m_pinix+1], OUTPUT);
				ifp->setLed( false);
			}
		}

		// wait until start/stop button is released
		while ( !digitalRead( SWT_STARTSTOP));

		// release emergency lines
		for ( int port = 0; port < IFP_PORTS; port++ ) {
			ifp = &IFP[port];
			if ( ifp->m_addrcnt )
				pinMode( PIN[ifp->m_pinix+1], INPUT);
		}

		// blink heartbeat led infinitely
		blinkHB( true);
	}
}

// PORTSETUP (protected)
// Initializes the port to idle state and then prepares it for reuse
// The port led is turned off.
// - port: the port number of the port (0 through IFP_PORTS)
void InfentoPort::portSetup( int port)
{
	init();
	
	if ( port < 0 || port > IFP_PORTS ) return;
	
	m_port = port;
	m_portmode = IFM_I2C;
	
	if ( port < IFP_DIGIPORTS )
		m_bus = I2C_BUS_IB1;
	else
	if ( port < IFP_DIGIPORTS + IFP_ANAPORTS )
		m_bus = I2C_BUS_IB2;
	else
		m_bus = I2C_BUS_IB3;

	m_pinix = port * IFP_LINES;
	if ( port < IFP_DIGIPORTS || port >= IFP_DIGIPORTS + IFP_ANAPORTS ) {
		pinMode( PIN[m_pinix], INPUT);
		pinMode( PIN[m_pinix+1], INPUT);
		pinMode( PIN[m_pinix+2], INPUT);
		pinMode( PIN[m_pinix+3], INPUT);
	}

	setLed( false);
}

// AELINESSETUP (protected)
// Setup for the attention line and emergency line as used by Infento boards.
// This routine may not be called in 'portSetup', since it affects the port lines.
// Instead it is called by 'findDevicePort' when an i2c device has been detected.
// (For none Infento i2c devices the ae-lines will stay unused.)
void InfentoPort::aeLinesSetup()
{
	// ATTENTION
	// ---------
	pinMode( PIN[m_pinix], INPUT); // line 1 is attention line
	// HIGH = no attention, LO = attention

	// EMERGENCY
	// ---------
	pinMode( PIN[m_pinix+1], INPUT); // line 2 is emergency line
	digitalWrite( PIN[m_pinix+1], 0);
	// SET emergency:    pinMode( PIN[ifp->m_pinix+1], OUTPUT)
	// RESET emergency:  pinMode( PIN[ifp->m_pinix+1], INPUT)
}

// ATTENTION (static)
// Checks for attention on all ports that have an Infento board connected.
// - returns: if there is an attention on one of the ports.
bool InfentoPort::attention()
{
	bool ret = false;

	#ifdef INF_DEBUG_ATTENTION
		debugroutine( IfpStr + "::attention (static)");
	#endif

	for ( int port = 0; port < IFP_PORTS; port++ ) {
		#ifdef INF_DEBUG_ATTENTION
			Serial.println( "  attention check");
			Serial.print( "      - on port ");
			Serial.println( port);
		#endif
		if ( !IFP[port].m_addrcnt ) {
			#ifdef INF_DEBUG_ATTENTION
				Serial.println( "      - not an Infento device ");
			#endif
			continue;
		}
		if ( !digitalRead( PIN[IFP[port].m_pinix]) ) {
			#ifdef INF_DEBUG_ATTENTION
				Serial.println( "  attention found");
				Serial.print( "      - on port ");
				Serial.println( port);
			#endif
			byte command = CMD_RD_ATTNSTS;
			BTOI devmask;
			for ( int i = 0; i < IFP[port].m_addrcnt; i++ ) {
				if ( !IFP[port].m_address[i] ) break;
				IFP[port].write( &command, 1, IFP[port].m_address[i]);
				IFP[port].read( devmask.b, 2, IFP[port].m_address[i]);
				IFP[port].m_attention[i] |= devmask.i;
				#ifdef INF_DEBUG_ATTENTION
					Serial.print( "      - i2c-address ");
					Serial.print( IFP[port].m_address[i]);
					Serial.print( " (0x");
					Serial.print( IFP[port].m_address[i], HEX);
					Serial.println( ")");
					Serial.print( "      - device mask ");
					Serial.print( devmask.i, BIN);
					Serial.print( " (0x");
					Serial.print( devmask.i, HEX);
					Serial.println( ") - refresh");
					Serial.print( "      - attention mask ");
					Serial.print( IFP[port].m_attention[i], BIN);
					Serial.print( " (0x");
					Serial.print( IFP[port].m_attention[i], HEX);
					Serial.println( ")");
				#endif
			}
			ret = true;
		}
		#ifdef INF_DEBUG_ATTENTION
		else
			Serial.println( "      - no attention");
		#endif
	}

	return ret;
}

// WAITATTENTION (static)
// Waits for attention on one of the ports.
// Meanwhile checking for emergency and blinking the heart beat led (normal on/normal off).
void InfentoPort::waitAttention()
{
	while ( !attention() ) emergency();
}

// ATTENTION
// Checks this port if an Infento device calls for attention.
// - i2c_address: the i2c address of the Infento board.
// - devicemask: the device mask bit of the device that needs to be checked.
// - returns: if there is an attention for the device.
// The device mask bit of a device is set by the constructor of the device in question.
// (See the 'm_actuator' and 'm_sensor' member of the 'InfentoDevice' class.)
bool InfentoPort::attention( int i2c_address, uint16_t devicemask)
{
	#ifdef INF_DEBUG_ATTENTION
		debugroutine( IfpStr + "::attention (instance)");
	#endif

	int ix;
	for ( ix = 0; ix < m_addrcnt; ix++ )
		if ( m_address[ix] == i2c_address ) break;
	if ( ix >= m_addrcnt ) {
		return false;
	}

	#ifdef INF_DEBUG_ATTENTION
		Serial.println( "  attention check");
		Serial.print( "      - on port ");
		Serial.println( m_port);
		Serial.print( "      - i2c-address ");
		Serial.print( i2c_address);
		Serial.print( " (0x");
		Serial.print( i2c_address, HEX);
		Serial.println( ")");
		Serial.print( "      - device mask ");
		Serial.print( devicemask, BIN);
		Serial.print( " (0x");
		Serial.print( devicemask, HEX);
		Serial.println( ")");
	#endif

	if ( !(m_attention[ix] & devicemask) ) {
		byte command = CMD_RD_ATTNSTS;
		BTOI mask;
		if ( !digitalRead( PIN[m_pinix]) ) {
			write( &command, 1, i2c_address);
			read( mask.b, 2, i2c_address);
			m_attention[ix] |= mask.i;
			#ifdef INF_DEBUG_ATTENTION
				Serial.print( "      - attention mask ");
				Serial.print( mask.i, BIN);
				Serial.print( " (0x");
				Serial.print( mask.i, HEX);
				Serial.println( ") - refreshed");
			#endif
		}
	}

	if ( m_attention[ix] & devicemask ) {
		m_attention[ix] &= (0xFFFF - devicemask);
		#ifdef INF_DEBUG_ATTENTION
			Serial.print( "      - attention mask ");
			Serial.print( m_attention[ix], BIN);
			Serial.print( " (0x");
			Serial.print( m_attention[ix], HEX);
			Serial.println( ") - cleared");
			Serial.println( "  attention for this device");
        #endif
		return true;
	}
	#ifdef INF_DEBUG_ATTENTION
	else
		Serial.println( "  not for this device");
	#endif

	return false;
}

// EMERGENCY (static)
// Checks all Infento boards and Infento controller for emergency.
// If so, all Infento boards are send an emergency call to stop them
// and program execution is blocked. The port led of the faulty device will blink.
// Additionally checks if the start/stop button was pressed ('haltOnStop').
void InfentoPort::emergency()
{
	int port;
	long time = millis();
	InfentoPort* ifp, *fault = NULL;
//!
  int ret;
  BTOI btoi;
  uint8_t data[3];
 //!
 
	haltOnStop();

	#ifdef INF_DEBUG_EMERGENCY
		debugroutine( IfpStr + "::emergency");
	#endif

	// heart beat led
	if ( time - g_hbtime > 500 ) {
		g_hbled = !g_hbled;
		digitalWrite( LED_HEARTBEAT, (g_hbled ? HIGH : LOW));
		g_hbtime = time;
	}

	// check on user emergency
	if ( !digitalRead( SWT_EMERGENCY) )
		goto emergency_now;

	// check on device emergency
	for ( port = 0; port < IFP_PORTS; port++ ) {
		fault = &IFP[port];
		if ( !fault->m_addrcnt ) continue; // not an Infento device
		if ( !digitalRead( PIN[fault->m_pinix+1]) ){
//!      
      for ( int i = 0; i < fault->m_addrcnt; i++ ){
        data[0] = CMD_RD_EMGSTS;
        data[1] = 0;
        Wire.beginTransmission( fault->m_address[i]);
				ret = Wire.write( data, 1);
				Wire.endTransmission();
        delay(10);
				Wire.requestFrom( fault->m_address[i], 2);
				ret = 0;
				while ( Wire.available() && (ret < 2) ) {
					data[ret] = (byte) Wire.read();
					ret++;
				}
        btoi.b[0] = data[0];
		    btoi.b[1] = data[1];
        Serial.print("Emg status: 0b");
        Serial.println(String(btoi.i,BIN));
      }
//!
			goto emergency_now;
    }
	}

	#ifdef INF_DEBUG_EMERGENCY
		Serial.println( "      - no emergency");
	#endif

	return;

emergency_now:

	// call for emergency on all i2c ports
	for ( port = 0; port < IFP_PORTS; port++ ) {
		ifp = &IFP[port];
		if ( ifp->m_addrcnt )
			pinMode( PIN[ifp->m_pinix+1], OUTPUT);
	}

	// blink back light of faulty port
	#ifdef INF_DEBUG_EMERGENCY
		if ( fault ) {
			Serial.print( "      - emergency on port ");
			Serial.println( fault->m_port);
		}
		else
			Serial.println( "      - user emergency");
		Serial.println( "\nPROGRAM HALTED");
	#endif

	while ( true ) {
		if ( fault ) {
			fault->setLed( true);
			delay( 500);
			fault->setLed( false);
			delay( 500);
		}
	}

}

// SETPORTMODE
// Switches a port to the requested mode and prepares it.
// - portmode: IFM_I2C = mode to support i2c communication.
//             IFM_UART = mode to support uart communication *).
//             IFM_DATA = mode to support arduino pin access.
//             IFM_REF = mode to enable a voltage divider and arduino pin access.
// *) The Infento controller communicates over the arduino serial 2 interface.
void InfentoPort::setPortMode( int portmode)
{
	#ifdef INF_DEBUG_PORT
		debugroutine( IfpStr + "::setPortMode");
	#endif

	// port mode must be IFM_DATA, IFM_I2C IFM_UART or IFM_REF

	if ( m_port < 0 ) return;					// portSetup must have been called first
	if ( portmode < 0 || portmode > 3 ) return;	// port mode must be an IFM mode

	switchI2C( I2C_BUS_CMD);
	switchMode( m_port, portmode);
	m_portmode = portmode;

	#ifdef INF_DEBUG_PORT
		Serial.print( "      - on port ");
		Serial.println( m_port);
		Serial.print( "      - mode ");
		switch ( m_portmode ) {
			case IFM_DATA :		Serial.println( "DATA"); break;
			case IFM_I2C :		Serial.println( "I2C"); break;
			case IFM_UART :		Serial.println( "UART"); break;
			case IFM_REF :		Serial.println( "REFERENCE"); break;
		}
	#endif

	setLed( true);
}

// FINDDEVCEPORT (static)
// Searches the controller ports for an i2c device.
// - i2c_address: the i2c_address of the device.
// - returns: a pointer to the port that has the device connected or NULL if the device was not found.
// IMPORTANT NOTE! Be sure all ports that have a none i2c device connected are
// put into the required mode ('setPortMode') BEFORE using this call. If not, a call
// to 'findDevicePort' will wait for the i2c bus infinitely and hang the program.
InfentoPort* InfentoPort::findDevicePort( int i2c_address)
{
	#ifdef INF_DEBUG_PORT
		debugroutine( IfpStr + "::findDevicePort");
	#endif

	InfentoPort* ifp = NULL;
	int port, i;
	byte g_mode[IFT_NXTPORT];

	if ( i2c_address == ADDR_I2C_SWITCH )
		return NULL; // don't try the i2c switcher chip itself

	#ifdef INF_DEBUG_PORT
		Serial.print( "      - i2c address 0x");
		Serial.print( i2c_address, HEX);
		Serial.print( " (");
		Serial.print( i2c_address);
		Serial.println( ")");
	#endif

	// put all ports in data mode
	#ifdef INF_DEBUG_PORT
		Serial.println( "  Change all ports to data mode\n");
	#endif

	for ( i = 0; i < IFT_NXTPORT; i++ ) {
		g_mode[i] = ifg_mode[i];
		ifg_mode[i] = 0xFF;
	}
	switchI2C( I2C_BUS_CMD);
	Wire.beginTransmission( ADDR_DIG_MODE);
	Wire.write( 0xFF);
	Wire.endTransmission();
	Wire.beginTransmission( ADDR_ANA_MODE);
	Wire.write( 0xFF);
	Wire.endTransmission();

	// find device
	for ( port = 0; port < IFP_PORTS; port++ ) {

		ifp = &IFP[port];

		if ( ifp->m_portmode != IFM_I2C ) {
			#ifdef INF_DEBUG_PORT
				Serial.print( "  non i2c device");
				Serial.print( "      - on port ");
				Serial.println( ifp->m_port);
			#endif
			continue; // a non i2c device is connected
		}

		switchI2C( I2C_BUS_CMD);
		switchMode( port, IFM_I2C);
		switchI2C( ifp->m_bus);

		#ifdef INF_DEBUG_PORT
			Serial.println( "  searching ...");
			Serial.print( "      - on port ");
			Serial.println( ifp->m_port);
			Serial.print( "      - i2c address ");
			Serial.println( i2c_address);
		#endif

		Wire.beginTransmission( i2c_address);
		if ( !Wire.endTransmission() ) { // found the device
			#ifdef INF_DEBUG_PORT
				Serial.println( "  DEVICE FOUND");
			#endif
			break;
		}
	}

	// restore all port modes
	for ( i = 0; i < IFT_NXTPORT; i++ )
		ifg_mode[i] = g_mode[i];
	switchI2C( I2C_BUS_CMD);
	for ( i = 0; i < IFP_PORTS; i++ )
		switchMode( i, IFP[i].m_portmode);

	if ( port >= IFP_PORTS ) {
		ifg_incomplete = true;
		#ifdef INF_DEBUG_PORT
			Serial.println( "  INCOMPLETE DEVICE CONFIGURATION");
		#endif
		return NULL;
	}

	// do not register a board twice
	for ( i = 0; i < ifp->m_addrcnt; i++ )
		if ( ifp->m_address[i] == i2c_address )
			break;
	if ( i < ifp->m_addrcnt ) return ifp;

	// register a newly found board
    if ( ifp->m_addrcnt >= MAXBOARDS - 1 ) {
		#ifdef INF_DEBUG_PORT
			Serial.println( "  TOO MANY BOARDS CONNECTED");
		#endif
        return NULL;
    }
    ifp->m_address[ ifp->m_addrcnt] = i2c_address;
    ifp->m_addrcnt++;

	ifp->setPortMode( IFM_I2C);
	ifp->aeLinesSetup();
	// Attention and emergency lines are set up regardless if it is
	// an Infento device. I2c does not use line 1 and 2 and thus the
	// set up does not affect the proper working of other i2c devices.

	return ifp;
}

// HALTONINCOMPLETE
// Checks if there are Infento devices declared but not found on one of the ports.
// If so the program is halted and the leds of all empty ports will start blinking.
void InfentoPort::haltOnIncomplete()
{
	#ifdef INF_DEBUG_PORT
		debugroutine( IfpStr + "::haltOnIncomplete");
	#endif

	if ( ifg_incomplete )
		#ifdef INF_DEBUG_PORT
			Serial.println( "\nPROGRAM HALTED");
		#endif
		while ( true ) {
			for ( int i = 0; i < 8; i++ )
				if ( !IFP[i].m_portmode )
					IFP[i].setLed( true);
			delay( 500);
			for ( int i = 0; i < 8; i++ )
				if ( !IFP[i].m_portmode )
					IFP[i].setLed( false);
			delay( 500);
		}
}

// INFENTOBOARD
// Checks if the i2c device is an Infento board.
// - returns: 0 = not an Infento board.
//            IFB_ACTUATOR = Infento actuator board.
//            IFB_SENSOR = Infento sensor board.
int InfentoPort::infentoBoard( int i2c_address)
{
	#ifdef INF_DEBUG_PORT
		debugroutine( IfpStr + "::infentoBoard");
	#endif

	byte data[8]; // eeprom header size
	int i, id = 0;
	int ret;

	// switch to correct bus
	switchI2C( m_bus);

	data[0] = CMD_RD_EEPROM;
	data[1] = 0; // read from start of eeprom
	write( data, 2, i2c_address);
	read( data, 8, i2c_address);
	if ( strncmp( (char*) data, "Infento", 7) || !data[7] ) {
		#ifdef INF_DEBUG_PORT
			Serial.println( "      - not an Infento board");
		#endif
		return 0;
	}

	id = data[7];

	#ifndef INF_DEBUG_PORT
	if ( INF_DEBUG_DEVICE ) {
	#endif
		Serial.print( "      - board ");
		switch ( id ) {
			case IFB_ACTUATOR :	Serial.print( "ActuatorIO"); break;
			case IFB_SENSOR :	Serial.print( "SensorIO"); break;
			default :			Serial.println( "unknown");
		}
		if ( id ) {
			BTOI btoi;
			data[0] = CMD_RD_VERSION;
			write( data, 1, i2c_address);
			read( data, 6, i2c_address);
			btoi.b[0] = data[2];
			btoi.b[1] = data[3];
			Serial.println( " Version " + String( (int) data[0]) + "." + String( (int) data[1]) + " (" +
							String( (int) data[5]) + "-" + String( (int) data[4]) + "-" + String( btoi.i) + ")");
		}
	#ifndef INF_DEBUG_PORT
	}
	#endif

	return id;
}

// VERNIERPARAMETERS
// Reads the sensor parameters of an intelligent (i2c) Vernier sensor.
// These parameters should be supplied to the routine 'toVernier' of the 'InfentoDevice' class
// in order to convert an analogue reading into a calibrated sensor value.
// - i2c_address: the i2c address of the Vernier sensor.
// - returns: the Vernier parameters.
VernierParam InfentoPort::vernierParameters( int i2c_address)
{
	#ifdef INF_DEBUG_PORT
		debugroutine( IfpStr + "::vernierParameters");
	#endif

	byte info[134];
	int i;
	byte intercept[4];
	byte slope[4];
	int page = info[70];
	VernierParam vp;

	vp.devname = "";
	vp.intercept = 0;
	vp.slope = 0;

	// switch to correct bus
	switchI2C( m_bus);

	// read i2c device info in parts of 32 byte each
	for ( int block = 0; block < 4; block++ ) {
		Wire.beginTransmission( i2c_address);
		Wire.write( block * 32); // set register pointer
		Wire.endTransmission();
		Wire.requestFrom( i2c_address, 32);
		for ( i = 0; i < 32; i++ ) {
			if ( !Wire.available() ) return vp;
			info[block*32+i] = (byte)Wire.read();
		}
	}

	vp.devname = "";
	for ( i = 0; i < 20; i++ ) {
		if ( !info[i + 8] ) break;
		vp.devname += (char) info[i + 8];
	}

	for ( i = 0; i < 4; i++ )
		intercept[i] = info[i + 71 + page * 19];
	for ( i = 0; i < 4; i++ )
		slope[i] = info[ i + 75 + page * 19];
	vp.intercept = *(float*) &intercept;
	vp.slope = *(float*) &slope;
	
	return vp;
}

// TOVERNIER
// Converts an analogue reading into a calibrated Vernier sensor value.
// - analoguereading: value that was read from a data line or i2c device.
// - vp: the parameters from the Vernier sensors data sheet.
float InfentoPort::toVernier( int analoguereading, VernierParam& vp)
{
	float volt = ((float) analoguereading) / 1023 * 5.0;
	float val = vp.intercept + vp.slope * volt + vp.slope * vp.slope * volt * volt;

	#ifdef INF_DEBUG_DEVICEIO
		debugroutine( IfdStr + "::toVernier", "raw: " + String( analoguereading), "returns (x1000): " + String( (long)val*1000));
	#endif

	return val;
}

// SETLED
// Turns a port led on or off.
// - on: to turn the led (TRUE) on or (FALSE) off.
void InfentoPort::setLed( bool on)
{
	#ifdef INF_DEBUG_PORT
		debugroutine( IfpStr + "::setLed");
	#endif

	if ( m_bus == I2C_BUS_IB3 ) return; // no led with the NXT connector

	int type = m_port / IFP_DIGIPORTS;
	int connector = m_port - type * IFP_DIGIPORTS;
	int address;
	int led = ifg_led[type];
	
	#ifdef INF_DEBUG_PORT
		Serial.println( "  led status");
		Serial.print( "      - of port ");
		Serial.println( m_port);
		Serial.print( "      - status before: ");
		Serial.println( ifg_led[type], BIN);
	#endif

	led = led | (1 << connector);
	if ( on )
		led = led ^ (1 << connector);
	ifg_led[type] = led;

	#ifdef INF_DEBUG_PORT
		Serial.print( "      - status after:  ");
		Serial.println( ifg_led[type], BIN);
	#endif

	switchI2C( I2C_BUS_CMD);
	address = ( m_bus == I2C_BUS_IB1 ? ADDR_DIG_LED : ADDR_ANA_LED);
	Wire.beginTransmission( address);
	Wire.write( led);
	Wire.endTransmission();
}

// PIN
// Gives the corresponding arduino pin of a port line.
// - line: the required port line (LINE1, LINE2, LINE3 or LINE4).
// - returns: the corresponding arduino pin.
int InfentoPort::pin( int line)
{
	#if defined(INF_DEBUG_PORTIO) || defined(INF_DEBUG_PORT)
		debugroutine( IfpStr + "::pin", "on port " + m_port, "line " + line, " pin " + PIN[m_port*IFP_LINES + line]);
	#endif

	return PIN[m_port*IFP_LINES + line];
}

// WRITE
// Writes data to the port. The port must be in IFM_I2C or IFM_UART mode.
// - data: buffer to send the data.
// - size: number of bytes in the buffer to send.
// - i2c_address: the i2c_address of the device (IFM_I2C mode only).
void InfentoPort::write( byte data[], int size, int i2c_address)
{
	#if defined(INF_DEBUG_PORTIO) || defined(INF_DEBUG_PORT)
		debugroutine( IfpStr + "::write");
	#endif

	int ret;
	int i;
	String s;

	switch ( m_portmode ) {
		case IFM_I2C :		switchI2C( m_bus);
							Wire.beginTransmission( i2c_address);
							ret = Wire.write( data, size);
							Wire.endTransmission();
							break;

		case IFM_UART :		ret = Serial2.write( data, size);
							break;
	}

	#if defined(INF_DEBUG_PORTIO) || defined(INF_DEBUG_PORT)
		Serial.print( "      - on port ");
		Serial.println( m_port);
		if ( m_portmode == IFM_I2C ) {
			Serial.print( "      - i2c bus IB");
			switch ( m_bus ) {
				case I2C_BUS_IB1 :  Serial.println( "1"); break;
				case I2C_BUS_IB2 :  Serial.println( "2"); break;
				case I2C_BUS_IB3 :  Serial.println( "3"); break;
			}
			Serial.print( " - i2c address 0x");
			Serial.print( i2c_address, HEX);
			Serial.print( " (");
			Serial.print( i2c_address);
			Serial.println( ")");
		}
		else
			Serial.println( "      - uart");
		Serial.print( "      - data '");
		for ( i = 0; i < size; i++ ) {
			Serial.print( data[i], HEX);
			Serial.print( " ");
		}
		Serial.println( "'");
		if ( ret < size ) {
			Serial.println( "  ERROR: not all bytes written");
			Serial.print( "      - sent ");
			Serial.print( ret);
			Serial.print( " out of ");
			Serial.println( size);
		}
	#endif
}

// READ
// Reads data from the port. The port must be in IFM_I2C or IFM_UART mode.
// - data:  buffer to receive the data.
// - size: number of bytes to receive.
// - i2c_address: the i2c_address of the device (IFM_I2C mode only).
int InfentoPort::read( byte data[], int size, int i2c_address)
{
	#if defined(INF_DEBUG_PORTIO) || defined(INF_DEBUG_PORT)
		debugroutine( IfpStr + "::read");
	#endif

	int ret = 0;
	int i;

	switch ( m_portmode ) {
	case IFM_I2C :   	switchI2C( m_bus);
						Wire.requestFrom( i2c_address, size);
						ret = 0;
						while ( Wire.available() && (ret < size) ) {
							data[ret] = (byte) Wire.read();
							ret++;
						}
						break;

	case IFM_UART :		ret = 0;
						while ( Serial2.available() && (ret < size) ) {
							data[ret] = (byte) Serial2.read();
							ret++;
						}
						break;
	}

	#if defined(INF_DEBUG_PORTIO) || defined(INF_DEBUG_PORT)
		Serial.print( "      - on port ");
		Serial.println( m_port);
		if ( m_portmode == IFM_I2C ) {
			Serial.print( "      - i2c bus IB");
			switch ( m_bus ) {
				case I2C_BUS_IB1 :  Serial.println( "1"); break;
				case I2C_BUS_IB2 :  Serial.println( "2"); break;
				case I2C_BUS_IB3 :  Serial.println( "3"); break;
			}
			Serial.print( "      - i2c address 0x");
			Serial.print( i2c_address, HEX);
			Serial.print( " (");
			Serial.print( i2c_address);
			Serial.println( ")");
		}
		Serial.print( "      - data '");
		for ( i = 0; i < ret; i++ ) {
			Serial.print( data[i], HEX);
			Serial.print( " ");
		}
		Serial.println( "'");
		if ( ret < size ) {
			Serial.println( "  ERROR: not all bytes read");
			Serial.print( "      - received ");
			Serial.print( ret);
			Serial.print( " out of ");
			Serial.println( size);
		}
	#endif

	return ret;
}


///////////////////////////
// InfentoDevice members //
///////////////////////////

// INFENTODEVICE
// Class constructor.
// Base class for all Infento actuator and sensor classes.
InfentoDevice::InfentoDevice()
{
	m_ifp = NULL;
	m_address = -1;
	m_sensor.all = 0;
	m_actuator.all = 0;
}

#ifdef INF_DEBUG_EEPROM
// DEBUGEEPROM
// Reports eeprom values via a serial monitor.
// - i2c_address: the i2c address of the device board.
// - index: index to the eeprom.
void InfentoDevice::debugEeprom( int i2c_address, byte index)
{
	BTOI btoi;
	byte data[3];
	String val;
	
	data[0] = CMD_RD_EEPROM;
	data[1] = index;
	data[2] = 2;
	m_ifp->write( data, 3, i2c_address);
	m_ifp->read( btoi.b, 2, i2c_address);

	switch ( index ) {
		case 0x50 :		val = "EEPROM_MD1_VELVAL"; break;
		case 0x52 :		val = "EEPROM_MD1_TURNVAL"; break;
		case 0x54 :		val = "EEPROM_MD1_DISTVAL"; break;
		case 0x56 :		val = "EEPROM_MD1_ARCVAL"; break;
		case 0x70 :		val = "EEPROM_MD2_VELVAL"; break;
		case 0x72 :		val = "EEPROM_MD2_TURNVAL"; break;
		case 0x74 :		val = "EEPROM_MD2_DISTVAL"; break;
		case 0x76 :		val = "EEPROM_MD2_ARCVAL"; break;
		case 0x90 :		val = "EEPROM_MD3_VELVAL"; break;
		case 0x92 :		val = "EEPROM_MD3_TURNVAL"; break;
		case 0x94 :		val = "EEPROM_MD3_DISTVAL"; break;
		case 0x96 :		val = "EEPROM_MD3_ARCVAL"; break;
	}

	Serial.print( "      - in eeprom: " + val + " ");
	Serial.print( "0x");
	Serial.print( btoi.b[0], HEX);
	Serial.print( " 0x");
	Serial.println( btoi.b[1], HEX);
}
#endif

// CONNECT
// Connects an i2c device to an Infento port.
// Checks if it is an Infento board or Vernier sensor.
// Resets an Infento board and reads which actuators or sensors are available.
// - i2c_address: the i2c address of the device board.
// - returns: pointer to the connected port or NULL if the device was not found.
InfentoPort* InfentoDevice::connect( int i2c_address)
{
	if ( INF_DEBUG_DEVICE ) {
		debugroutine( IfdStr + "::connect (i2c)");
		Serial.print( "      - to i2c address 0x");
		Serial.print( i2c_address, HEX);
		Serial.print( " (");
		Serial.print( i2c_address);
		Serial.println( ")");
	}

	m_address = i2c_address;
	m_ifp = InfentoPort::findDevicePort( m_address); // if found sets portmode too
	if ( !m_ifp ) {
		if ( INF_DEBUG_DEVICE )
			Serial.println( "      - address not found");
		return NULL;
	}
	
	if ( m_ifp->infentoBoard( m_address) ) {
		m_ifp->resetBoard( m_address);
		delay( 10);
		if ( m_actuator.all >= 0 || m_sensor.all >= 0 ) {
			m_data[0] = CMD_RD_DEVMASK;
			m_ifp->write( m_data, 1, m_address);
			m_ifp->read( m_data, 2, m_address);
			BTOI btoi;
			btoi.b[0] = m_data[0];
			btoi.b[1] = m_data[1];
			if ( (btoi.i & m_actuator.all) || (btoi.i & m_sensor.all) ) {
				if ( INF_DEBUG_DEVICE ) {
					INF_DEBUG_DEVICE = false;
					Serial.println( "      - device " + version());
					INF_DEBUG_DEVICE = true;
				}
				delay( 20);
				return m_ifp;
			}

			if ( INF_DEBUG_DEVICE )
				Serial.println( "      - not this board");
			return NULL;
		}
	}
	else {
		if ( INF_DEBUG_DEVICE )
			Serial.println( "      - not an Infento board");
	}

	return m_ifp;
}

// CONNECT
// Connects a none i2c device to a port and sets the required portmode.
// - ifp: class of one of the infento ports.
// - portmode: IFM_I2C = communication via i2c protocol.
//             IFM_UART = communication via uart protocol.
//             IFM_DATA = routing the port lines to the data pins of the arduino.
//             IFM_REF = a voltage divider is activated for the connected device.
// - returns: if the device is connected successfully.
bool InfentoDevice::connect( InfentoPort& ifp, int portmode)
{
	if ( INF_DEBUG_DEVICE ) {
		debugroutine( IfdStr + "::connect (port)");
		Serial.print( "      - to ");
		switch ( portmode ) {
			case IFM_UART :	Serial.println( "uart (rx/tx) device"); break;
			case IFM_REF :	Serial.println( "reference data line device"); break;
			case IFM_DATA :	Serial.println( "data line device"); break;
		}
		Serial.print( "      - on port ");
		Serial.print( ifp.port());
	}

	m_ifp = &ifp;
	ifp.setPortMode( portmode);
	return true;
}

// SIGNAL
// Checks if the device raised a signal
// Signals are raised in reponse to a signal request (see the derived classes).
// - returns: if a signal was raised.
bool InfentoDevice::signal()
{
	#ifndef INF_DEBUG_DEVICEIO
	if ( INF_DEBUG_DEVICE )
	#endif
		debugroutine( IfdStr + "::signal");

	if ( !m_ifp ) {
		#ifdef INF_DEBUG_DEVICEIO
			Serial.println( "      - not connected");
		#endif
		return false;
	}

	uint16_t devmask = 0;
	if ( m_sensor.all ) devmask = m_sensor.all;
	if ( m_actuator.all ) devmask = m_actuator.all;
	if ( !devmask ) {
		#ifdef INF_DEBUG_DEVICEIO
			Serial.println( "      - not an Infento device");
		#endif
		return false;
	}

	bool att = m_ifp->attention( m_address, devmask);

	#ifndef INF_DEBUG_DEVICEIO
	if ( INF_DEBUG_DEVICE )
	#endif
	{
		Serial.print( "      - on port ");
		Serial.println( m_ifp->port());
		Serial.println( "      - for device " + m_devname);
		Serial.println( att ? "      - signal raised" : "      - no signal");
	}

	m_ifp->emergency();

	return att;
}

// WAITSIGNAL
// Waits until the device raises a signal.
// Signals are raised in reponse to a signal request (see the derived classes).
void InfentoDevice::waitSignal()
{
	#ifndef INF_DEBUG_DEVICEIO
	if ( INF_DEBUG_DEVICE )
	#endif
		debugroutine( IfdStr + "::waitSignal");

	if ( !m_ifp ) {
		#ifdef INF_DEBUG_DEVICEIO
			Serial.println( "      - not connected");
		#endif
		return;
	}

	uint16_t devmask = 0;
	if ( m_sensor.all ) devmask = m_sensor.all;
	if ( m_actuator.all ) devmask = m_actuator.all;
	if ( !devmask ) {
		#ifdef INF_DEBUG_DEVICEIO
			Serial.println( "      - not an Infento device");
		#endif
		return;
	}

	#ifndef INF_DEBUG_DEVICEIO
	if ( INF_DEBUG_DEVICE )
	#endif
	{
		Serial.print( "      - on port ");
		Serial.println( m_ifp->port());
		Serial.println( "      - for device " + m_devname);
	}

	while ( !m_ifp->attention( m_address, devmask) ) m_ifp->emergency();
}

// CLEARSIGNAL (protected)
// Helper routine to clear a signal that was raised by the device.
// Signalling is recorded at both sides of the port: by the controller (this code)
// and by the firmware of the device board. When a possibly raised signal by the board
// is no longer needed, a call to 'clearSignal' will assure that the system at both sides
// will be kept in line.
void InfentoDevice::clearSignal()
{
	#ifdef INF_DEBUG_DEVICEIO
		debugroutine( IfdStr + "::clearSignal");
	#endif

	uint16_t devmask = 0;
	if ( m_sensor.all ) devmask = m_sensor.all;
	if ( m_actuator.all ) devmask = m_actuator.all;
	if ( !devmask ) return;
	m_ifp->attention( m_address, devmask);
}

// LINE
// Sets a port line HIGH, LOW or to a pwm value.
// Supports port mode IFM_DATA and IFM_REF.
// - linenumber: number of the port line (LINE1, LINE2, LINE3 or LINE4).
// - set: value to set the port line to.
// - pwm: if the value should be treated as a pwm value.
void InfentoDevice::line( int linenumber, int set, bool pwm)
{
	#ifdef INF_DEBUG_DEVICEIO
		debugroutine( IfdStr + "::line (set)");
	#endif

	pinMode( m_ifp->pin( linenumber), OUTPUT);
	if ( pwm ) {
		if ( set < 0 ) set = 0;
		if ( set > 255 ) set = 255;
		analogWrite( m_ifp->pin( linenumber), set);
		#ifdef INF_DEBUG_DEVICEIO
			Serial.println( "      - analog write");
		#endif
	}
	else {
		digitalWrite( m_ifp->pin( linenumber), (set ? HIGH : LOW));
		#ifdef INF_DEBUG_DEVICEIO
			Serial.println( "      - digital write");
		#endif
	}

	#ifdef INF_DEBUG_DEVICEIO
		Serial.print( "      - on line ");
		Serial.print( linenumber);
		Serial.print( " (pin ");
		Serial.print( m_ifp->pin( linenumber));
		Serial.print( "): ");
		Serial.println( set);
	#endif
}

// LINE
// Reads the value of a port line.
// Supports port mode IFM_DATA and IFM_REF.
// - linenumber: number of the port line (LINE1, LINE2, LINE3 or LINE4).
// - returns: HIGH or LOW for digital ports.
//            an value of 0 through 1023 for analogue ports.
int  InfentoDevice::line( int linenumber)
{
	#ifdef INF_DEBUG_DEVICEIO
		debugroutine( IfdStr + "::line (get)");
	#endif

	int val;

	pinMode( m_ifp->pin( linenumber), INPUT);
	if ( m_ifp->portType() == CONST_DIGITAL ) {
		val = digitalRead( m_ifp->pin( linenumber));
		#ifdef INF_DEBUG_DEVICEIO
			Serial.println( "      - digital read");
		#endif
	}
	else
	if ( m_ifp->portType() == CONST_ANALOGUE ) {
		val = analogRead( m_ifp->pin( linenumber));
		#ifdef INF_DEBUG_DEVICEIO
			Serial.println( "      - analog write");
		#endif
	}
	else
		val = 0; // NXT reading not implemented yet

	#ifdef INF_DEBUG_DEVICEIO
		Serial.print( "      - on line ");
		Serial.print( linenumber);
		Serial.print( " (pin ");
		Serial.print( m_ifp->pin( linenumber));
		Serial.print( "): ");
		Serial.println( val);
	#endif

	return val;
}

// WRITE
// Transfers the device data buffer to the device.
// The data buffer is filled by a call to 'setValue'.
// Supports port mode IFM_I2C and IFM_UART.
void InfentoDevice::write()
{
	#ifdef INF_DEBUG_DEVICEIO
		debugroutine( IfdStr + "::write");
		Serial.print( "      - data size ");
		Serial.println( m_size);
		if ( m_size < 0 )
			Serial.println( "      - DATA SIZE TOO SMALL");
		else
		if ( m_size > 10 )
			Serial.println( "      - DATA SIZE TOO LARGE");
		else {
			Serial.print( "      - data buffer ");
			for ( int i = 0; i < m_size; i++ ) {
				Serial.print( "0x");
				Serial.print( m_data[i], HEX);
				Serial.print( " (");
				Serial.print( m_data[i]);
				Serial.println( ")");
			}
		}
	#endif

	if ( m_size < 0 || m_size > 10 ) return ;
	m_ifp->write( m_data, m_size, m_address);
}

// READ
// Transfers data from the device into the device data buffer.
// The data buffer is processed by a call to 'getValue'.
// Supports port mode IFM_I2C and IFM_UART.
void InfentoDevice::read()
{
	#ifdef INF_DEBUG_DEVICEIO
		debugroutine( IfdStr + "::read");
		Serial.print( "      - data size ");
		Serial.println( m_size);
		if ( m_size < 0 )
			Serial.println( "      - DATA SIZE TOO SMALL");
		else
		if ( m_size > 10 )
			Serial.println( "      - DATA SIZE TOO LARGE");
	#endif

	if ( m_size < 0 || m_size > 10 ) return;
	m_ifp->read( m_data, m_size, m_address);
	m_data[m_size] = 0; // string null terminator

	#ifdef INF_DEBUG_DEVICEIO
		Serial.print( "      - data buffer ");
		for ( int i = 0; i < m_size; i++ ) {
			Serial.print( "0x");
			Serial.print( m_data[i], HEX);
			Serial.print( " (");
			Serial.print( m_data[i]);
			Serial.println( ")");
		}
	#endif
}

// SETVALUE
// Places a bool value in the device data buffer.
// Call 'write' afterwards to transfer the data to the device.
void InfentoDevice::setValue( bool value)
{
	m_size = 1;
	m_data[0] = (value ? 1 : 0);

	#ifdef INF_DEBUG_DEVICEIO
		debugroutine( IfdStr + "::setValue (bool)", "value " + String( value));
	#endif
}

// SETVALUE
// Places an integer value in the device data buffer.
// Call 'write' afterwards to transfer the data to the device.
void InfentoDevice::setValue( int value)
{
	BTOI btoi;
	m_size = 2;
	btoi.i = value;
	m_data[0] = btoi.b[0];
	m_data[1] = btoi.b[1];

	#ifdef INF_DEBUG_DEVICEIO
		debugroutine( IfdStr + "::setValue (int)", "value " + String( value));
	#endif
}

// SETVALUE
// Places a long integer value in the device data buffer.
// Call 'write' afterwards to transfer the data to the device.
void InfentoDevice::setValue( long value)
{
	BTOL btol;
	m_size = 4;
	btol.l = value;
	memcpy( m_data, btol.b, 4);

	#ifdef INF_DEBUG_DEVICEIO
		debugroutine( IfdStr + "::setValue (long)", "value " + String( value));
	#endif
}

// SETVALUE
// Places a floating point value in the device data buffer.
// Call 'write' afterwards to transfer the data to the device.
void InfentoDevice::setValue( float value, int decimalpos)
{
	BTOL btol;
	int e = 1;
	m_size = 4;
	while ( decimalpos > 0 ) { e *= 10; decimalpos--; }
	btol.l = value * e;
	memcpy( m_data, btol.b, 4);

	#ifdef INF_DEBUG_DEVICEIO
		debugroutine( IfdStr + "::setValue (float)", "value " + String( btol.l), String( decimalpos) + " decimalen");
	#endif
}

// SETVALUE
// Places a string value in the device data buffer.
// Call 'write' afterwards to transfer the data to the device.
void InfentoDevice::setValue( String value)
{
	m_size = value.length();
	if ( m_size > 10 ) m_size = 10;
	value.getBytes( m_data, m_size);
	m_data[m_size] = 0; // string null terminator

	#ifdef INF_DEBUG_DEVICEIO
		debugroutine( IfdStr + "::setValue (long)", "value " + value);
	#endif
}

// GETVALUE
// Reads a bool value from the device data buffer.
// Call 'read' beforehand in order to transfer the data from the device.
void InfentoDevice::getValue( bool& value)
{
	switch ( m_size ) {
		case 1 :	value = (m_data[0] != 0);
					break;
		case 2 :	{
					BTOI* btoi = (BTOI*) &m_data[0];
					value = (btoi->i != 0);
					break;
					}
		case 4 :	{
					BTOL* btol = (BTOL*) &m_data[0];
					value = (btol->l != 0);
					break;
					}
		default :	value = false;
	}

	#ifdef INF_DEBUG_DEVICEIO
		debugroutine( IfdStr + "::getValue (bool)", "returns: " + String( value));
	#endif
}

// GETVALUE
// Reads an integer value from the device data buffer.
// Call 'read' beforehand in order to transfer the data from the device.
void InfentoDevice::getValue( int& value)
{
	switch ( m_size ) {
		case 1 :	value = m_data[0];
					break;
		case 2 :	{
					BTOI* btoi = (BTOI*) &m_data[0];
					value = btoi->i;
					break;
					}
		case 4 :	{
					BTOL* btol = (BTOL*) &m_data[0];
					value = btol->l;
					break;
					}
		default :	value = 0;
	}

	#ifdef INF_DEBUG_DEVICEIO
		debugroutine( IfdStr + "::getValue (int)", "returns: " + String( value));
	#endif
}

// GETVALUE
// Reads a long integer value from the device data buffer.
// Call 'read' beforehand in order to transfer the data from the device.
void InfentoDevice::getValue( long& value)
{
	switch ( m_size ) {
		case 1 :	value = m_data[0];
					break;
		case 2 :	{
					BTOI* btoi = (BTOI*) &m_data[0];
					value = btoi->i;
					break;
					}
		case 4 :	{
					BTOL* btol = (BTOL*) &m_data[0];
					value = btol->l;
					break;
					}
		default :	value = 0;
	}

	#ifdef INF_DEBUG_DEVICEIO
		debugroutine( IfdStr + "::getValue (long)", "returns: " + String( value));
	#endif
}

// GETVALUE
// Reads a floating point value from the device data buffer.
// Call 'read' beforehand in order to transfer the data from the device.
void InfentoDevice::getValue( float& value, int decimalpos)
{
	int e = 1;
	while ( decimalpos > 0 ) { e *= 10; decimalpos--; }
	switch ( m_size ) {
		case 1 :	value = m_data[0] / e;
					break;
		case 2 :	{
					BTOI* btoi = (BTOI*) &m_data[0];
					value = btoi->i / e;
					break;
					}
		case 4 :	{
					BTOL* btol = (BTOL*) &m_data[0];
					value = btol->l / e;
					break;
					}
		default :	value = false;
	}

	#ifdef INF_DEBUG_DEVICEIO
		debugroutine( IfdStr + "::getValue (float)", "returns: " + String( value));
	#endif
}

// GETVALUE
// Reads a string value from the device data buffer.
// Call 'read' beforehand in order to transfer the data from the device.
void InfentoDevice::getValue( String& value)
{
	value = (char*) &m_data[0];

	#ifdef INF_DEBUG_DEVICEIO
		debugroutine( IfdStr + "::getValue (bool)", "returns: " + value);
	#endif
}

// VERSION
// Gives the version of the current device.
String InfentoDevice::version()
{
	String ret;
	BTOI btoi;
	if ( m_actuator.all ) btoi.i = m_actuator.all;
	else btoi.i = m_sensor.all;

	if ( m_ifp ) {
		m_data[0] = CMD_RD_DEVVERSION;
		m_data[1] = btoi.b[0];
		m_data[2] = btoi.b[1];
		m_ifp->write( m_data, 3, m_address);
		m_ifp->read( m_data, 1, m_address);
		int v = m_data[0] / 10;
		m_data[0] -= (m_data[0] / 10) * 10;
		int s = m_data[0] / 10;
		m_data[0] -= (m_data[0] / 10) * 10;
		ret = m_devname + " Version " + String( v) + "." + String(s) + "." + String( m_data[0]);
	}

	if ( INF_DEBUG_DEVICE )
		debugroutine( IfdStr + "::version", m_ifp ? "returns: " + ret : NOPORT);

	return ret;
}

///////////////////////////
// InfentoMelody members //
///////////////////////////

// INFENTOMELODY
// Class constructor.
InfentoMelody::InfentoMelody()
{
	m_sensor.mel = 1;
	m_devname = "Melody";
	m_child = "InfentoMelody";
}

/*
// CALIBRATE
// Declares the speaker watt with full loudness
// - maxwatt: the watt value given on the datasheet.
void InfentoMelody::calibrate( int maxwatt)
{
	BTOI btoi;
	btoi.i = maxlumen;

	if ( INF_DEBUG_DEVICE ) {
		String slum = "maxdb: " + String( maxdb/10) + " watt";
		debugroutine( m_child + "::calibrate", m_ifp ? slum : NOPORT);
	}

	if ( m_ifp ) {
		m_data[0] = CMD_WR_EEPROM;
		InfentoDefinition lampnum = lampNumber();
		switch ( lampnum ) {
			case CONST_LAMP1 :	m_data[1] = 0xC0; break; // EEPROM_???;
			case CONST_LAMP2 :	m_data[1] = 0xC8; break; // EEPROM_???;
			default: return;
		}
		m_data[2] = btoi.b[0];
		m_data[3] = btoi.b[1];
		m_ifp->write( m_data, 4, m_address);
		delay( 10);

		#ifdef INF_DEBUG_EEPROM
		debugEeprom( m_address, m_data[1]);
		#endif
	}
}

// SETVOLUME
// Sets the working watt for the current program session.
// - dB: maximum loudness in watt during the program. May not exceed the calibrated watt.
void InfentoMelody::setVolume( int watt)
{
	if ( INF_DEBUG_DEVICE )
		debugroutine( m_child + "::setVolume", m_ifp ? "watt: " + String( watt/10) : NOPORT);

	if ( !m_ifp ) return;
	BTOI btoi;
	btoi.i = watt;
	m_data[0] = CMD_???;
	m_data[1] = btoi.b[0];
	m_data[2] = btoi.b[1];
	m_ifp->write( m_data, 3, m_address);
}
*/

// PLAYONCE
// Plays the given tracknumber once. Will not signal when a play back has finished.
// - tracknumber: 1 to 4.
// - volume: percentage of the working volume.
void InfentoMelody::playOnce( int tracknumber, int volume)
{
	if ( INF_DEBUG_DEVICE )
		debugroutine( m_child + "::playOnce", "track: " + String( tracknumber), m_ifp ? "volume: " + String( volume) : NOPORT);

	play( tracknumber, false, volume);
}

// PLAYREPEAT
// Repeats playing the given tracknumber infinite.
// - tracknumber: 1 to 4.
// - volume: percentage of the working volume.
void InfentoMelody::playRepeat( int tracknumber, int volume)
{
	if ( INF_DEBUG_DEVICE )
		debugroutine( m_child + "::playRepeat", "track: " + String( tracknumber), m_ifp ? "volume: " + String( volume) : NOPORT);

	play( tracknumber, true, volume);
}

// PLAY (protected)
// Helper routine for 'playOnce' and 'playRepeat'.
void InfentoMelody::play( int tracknumber, bool repeat, int volume)
{
	if ( !m_ifp ) return;
	if ( tracknumber < 1 || tracknumber > 5 ) return;
	if ( volume < 0 ) volume = 0;
	if ( volume >100 ) volume = 100;

	clearSignal();
/*
	m_data[0] = CMD_MEL_SETVAL;
	m_data[1] = volume;
	m_ifp->write( m_data, 2, m_address);
*/
	m_data[0] = CMD_MEL_PLAY;
	m_data[1] = tracknumber;
	m_data[2] = (repeat ? 2 : 1);
	m_data[3] = volume;
	m_ifp->write( m_data, 4, m_address);
}

// STOP
// Stops the play back of a track.
void InfentoMelody::off()
{
	if ( INF_DEBUG_DEVICE )
		debugroutine( m_child + "::off", m_ifp ? "" : NOPORT);

	if ( !m_ifp ) return;

	clearSignal();

	m_data[0] = CMD_MEL_STOP;
	m_ifp->write( m_data, 1, m_address);
}


/////////////////////////
// InfentoLamp members //
/////////////////////////

// INFENTOLAMP
// Class constructor.
// - lampnumber: CONST_LAMP1 or CONST_LAMP2.
// The lamp number corresponds to its output connector on the actuator board (LWPM1 or LWPM2).
// Note that when motor MD2 is available on the board, lamp LWPM2 is not.
InfentoLamp::InfentoLamp( InfentoDefinition lampnumber)
{
	if ( lampnumber == CONST_LAMP1 ) {
		m_actuator.lpwm1 = 1;
		m_devname = "Lamp 1";
		m_cmdoffs = 0;
	}
	else {
		m_actuator.lpwm2 = 1;
		m_devname = "Lamp 2";
		m_cmdoffs = CMD_LPWM2_SETFUNC - CMD_LPWM1_SETFUNC;
	}
	m_child = "InfentoLamp";
}

// LAMPNUMBER
// Gives the lamp number of the device.
// - returns: CONST_LAMP1 or CONST_LAMP2.
InfentoDefinition InfentoLamp::lampNumber()
{
	if ( m_cmdoffs == (CMD_LPWM2_SETFUNC - CMD_LPWM1_SETFUNC) )
		return CONST_LAMP2;
	if ( !m_cmdoffs )
		return CONST_LAMP1;
	return CONST_UNDEFINED;
}

// CALIBRATE
// Declares the lamp lumen with 100% duty cycle pwm.
// - maxlumen: the lumen value given on the datasheet.
void InfentoLamp::calibrate( int maxlumen)
{
	BTOI btoi;
	btoi.i = maxlumen;

	if ( INF_DEBUG_DEVICE ) {
		String slum = "maxlumen: " + String( maxlumen) + " lumen";
		debugroutine( m_child + "::calibrate", m_ifp ? slum : NOPORT);
	}

	if ( m_ifp ) {
		m_data[0] = CMD_WR_EEPROM;
		InfentoDefinition lampnum = lampNumber();
		switch ( lampnum ) {
			case CONST_LAMP1 :	m_data[1] = 0xC0; break; // EEPROM_LPWM1CALLUM;
			case CONST_LAMP2 :	m_data[1] = 0xC8; break; // EEPROM_LPWM2CALLUM;
			default: return;
		}
		m_data[2] = btoi.b[0];
		m_data[3] = btoi.b[1];
		m_ifp->write( m_data, 4, m_address);
		delay( 10);

		#ifdef INF_DEBUG_EEPROM
		debugEeprom( m_address, m_data[1]);
		#endif
	}
}

// SETLUMINOSITY
// Sets the working lumen for the current program session.
// - luminosity: maximum brightness in lumen during the program. May not exceed the calibrated lumen.
void InfentoLamp::setLuminosity( int luminosity)
{
	if ( INF_DEBUG_DEVICE )
		debugroutine( m_child + "::setLuminosity", m_ifp ? "lumen: " + String( luminosity) : NOPORT);

	if ( !m_ifp ) return;
	BTOI btoi;
	btoi.i = luminosity;
	m_data[0] = CMD_LPWM1_SETVAL + m_cmdoffs;
	m_data[1] = btoi.b[0];
	m_data[2] = btoi.b[1];
	m_ifp->write( m_data, 3, m_address);
}

// ON
// Turns the lamp on with the given brightness.
// - brightness: percentage of the working luminosity.
void InfentoLamp::on( int brightness)
{
	if ( INF_DEBUG_DEVICE )
		debugroutine( m_child + "::on", m_ifp ? "brightness: " + String( brightness) : NOPORT);

	if ( !m_ifp ) return;

	clearSignal();

	m_data[0] = CMD_LPWM1_SETFUNC + m_cmdoffs;
	m_data[1] = 1; // FNC_ON
	m_data[2] = brightness;
	m_ifp->write( m_data, 3, m_address);
}

// SET (protected)
// Helper routine for 'blink' and 'fade'.
void InfentoLamp::set( byte func, int time_a, int brightness_a, int time_b, int brightness_b)
{
	if ( !m_ifp ) return;

	clearSignal();

	BTOI btoi;
	m_data[0] = CMD_LPWM1_SETFUNC + m_cmdoffs;
	m_data[1] = func;
	m_data[2] = brightness_a;
	btoi.i = time_a * 100; // millisec
	m_data[3] = btoi.b[0];
	m_data[4] = btoi.b[1];
	m_data[5] = brightness_b;
	btoi.i = time_b * 100; // millisec
	m_data[6] = btoi.b[0];
	m_data[7] = btoi.b[1];
	m_ifp->write( m_data, 8, m_address);

	if ( INF_DEBUG_DEVICE ) {
		Serial.println( "      - in: brightness " + String( brightness_a) + ", time " + String( time_a/10) + " sec");
		Serial.println( "      - out: brightness " + String( brightness_b) + ", time " + String( time_b/10) + " sec");
	}
}

// BLINK
// Lets the lamp blink on a give interval and brightness.
// - time_a: time in 0.1 seconds to keep the lamp on
// - brightness_a: 'on' brightness value as a percentage of the working luminosity
// - time_b: time in 0.1 seconds to keep the lamp off
// - brightness_b: 'off' brightness value as a percentage of the working luminosity
void InfentoLamp::blink( int time_a, int brightness_a, int time_b, int brightness_b)
{
	if ( INF_DEBUG_DEVICE )
		debugroutine( m_child + "::blink", m_ifp ? "" : NOPORT);

	set( 2, time_a, brightness_a, time_b, brightness_b); // 2 = FNC_BLINK
}

// FADE
// Lets the lamp fade in and out on a give interval and brightness.
// - time_a: time in 0.1 seconds to turn the lamp on
// - brightness_a: 'on' brightness value as a percentage of the working luminosity
// - time_b: time in 0.1 seconds to turn the lamp off
// - brightness_b: 'off' brightness value as a percentage of the working luminosity
void InfentoLamp::fade( int time_a, int brightness_a, int time_b, int brightness_b)
{
	if ( INF_DEBUG_DEVICE )
		debugroutine( m_child + "::fade", m_ifp ? "" : NOPORT);

	set( 3, time_a, brightness_a, time_b, brightness_b); // 3 = FNC_SLIDE
}

// OFF
// Turns the lamp off
// - immediate: acts upon blink and fade)
//              if not immediate the blink/fade cycle is finished first
void InfentoLamp::off( bool immediate)
{
	if ( INF_DEBUG_DEVICE ) {
		String msg = (immediate ? "immediate" : "when finished");
		debugroutine( m_child + "::off", m_ifp ? msg : NOPORT);
	}

	if ( !m_ifp ) return;

	clearSignal();

	m_data[0] = CMD_LPWM1_SETFUNC + m_cmdoffs;
	m_data[1] = 4; // FNC_OFF
	m_data[2] = (immediate ? 0 : 1);
	m_ifp->write( m_data, 3, m_address);
}


//////////////////////////
// InfentoDrive members //
//////////////////////////

// INFENTODRIVE
// Class constructor.
// Base class for 'InfentoMotor' and 'InfentoLinAct'.
InfentoDrive::InfentoDrive()
{
}

// MOTORNUMBER
// Gives the motor number of the device.
// - returns: CONST_MOTOR1, CONST_MOTOR2, CONST_EDRIVE, CONST_LINACT1 or CONST_LINACT2.
// (See the InfentoMotor and InfentoLinAct constructors.)
InfentoDefinition InfentoDrive::motorNumber()
{
	if ( m_cmdoffs == (CMD_MD3_MOVE - CMD_MD1_MOVE) )
		return CONST_EDRIVE;
	if ( m_cmdoffs == (CMD_MD2_MOVE - CMD_MD1_MOVE) )
		return CONST_MOTOR2;
	if ( !m_cmdoffs )
		return CONST_MOTOR1;
	return CONST_UNDEFINED;
}

// SETSTARTUPPOWER
// Determines the power with which a motor or linear actuator starts up actually.
// The startup power reflects the motor off situation.
// - power: percentage of the highest possible power for the motor/linear actuator.
// In normal functionality the acceleration will slow down a motor's response time.
// A higher startup power and acceleration will give a better reponse but a less natural look and feel.
// Additionally dc-motors will not start up at low power (PWM) values.
// The startup power will adjust this to normal functionality for the motor commands, so that
// for example 'on(3)' will turn on the motor visibly instead of burning its wiring after some time. 
// Use 'setStartupPower(0)' and 'setVelocity(0)' followed by 'on(value)' with a decreasing value from 100%
// to the moment the motor stops spinning in order to determine the actual startup percentage.
void InfentoDrive::setStartupPower( int power)
{
	if ( INF_DEBUG_DEVICE )
		debugroutine( m_child + "::setStartupPower", m_ifp ? "power: " + String( power) : NOPORT);

	setValue( 4, power);
}

// TICKS
// Gives the raw rotary value in passed ticks since the latest motor command.
// (Motor commands are 'on', 'run', 'move' and 'turn'.)
// This function is available only when there is a rotary sensor attached (see 'attachRotary').
// - returns: the number of rotary ticks since the latest command.
long InfentoDrive::ticks()
{
	BTOL btol;
	btol.l = 0;

	if ( m_ifp ) {
		m_data[0] = CMD_MD1_READ + m_cmdoffs;
		m_data[1] = 3; // read ticks
		m_ifp->write( m_data, 2, m_address);
		m_ifp->read( m_data, 4, m_address);
		btol.b[0] = m_data[0];
		btol.b[1] = m_data[1];
		btol.b[2] = m_data[2];
		btol.b[3] = m_data[3];
	}

	if ( INF_DEBUG_DEVICE )
		debugroutine( m_child + "::ticks", m_ifp ? "returns: " + String( btol.l) : NOPORT);

	return btol.l;
}

// SETUNIT (protected)
// Helper routine for 'setVelocityUnit', 'setDistanceUnit', 'setArcUnit', 'setTimeUnit' and 'setPositionUnit'.
// - func: 1 = velocity unit, 2 = distance/position unit, 3 = arc unit, 4 = time unit
// - unit: (velocity) UNIT_MPS, UNIT_KPH, UNIT_MPH, UNIT_RPM, (distance/position*) UNIT_MTR, UNIT_CM*, UNIT_MM,
//         UNIT_INCH*, (arc) UNIT_DEG, UNIT_GRAD, UNIT_TICK, (time) UNIT_MSEC, UNIT_SEC, UNIT_MIN or UNIT_HOUR.
void InfentoDrive::setUnit( byte func, InfentoUnit unit)
{
	if ( !m_ifp ) return;
	m_data[0] = CMD_MD1_SETUNIT + m_cmdoffs;
	m_data[1] = func;
	m_data[2] = unit;
	m_ifp->write( m_data, 3, m_address);
}

// SETVALUE (protected)
// Helper routine for 'setDirection', 'setVelocity', 'setAcceleration' and 'setStartupSpeed'.
// - func: 1 = velocity, 2 = direction, 3 = acceleration, 4 = startup speed
// - value: (direction) CONST_FORWARD, CONST_REVERSE, (velocity/acceleration) 0.1 unit, 0 for full speed/power,
//          (startup speed) percentage of full speed
void InfentoDrive::setValue( byte func, int value)
{
	if ( !m_ifp ) return;
	BTOI btoi;
	btoi.i = value;
	m_data[0] = CMD_MD1_SETVAL + m_cmdoffs;
	m_data[1] = func;
	m_data[2] = btoi.b[0];
	m_data[3] = btoi.b[1];
	m_ifp->write( m_data, 4, m_address);
}

// SETON (protected)
// Helper routine for 'on' and 'off'
// - speed: percentage of the working velocity
void InfentoDrive::setOn( int speed)
{
	if ( !m_ifp ) return;

	clearSignal();

	m_data[0] = CMD_MD1_ONOFF + m_cmdoffs;
	m_data[1] = speed;
	m_ifp->write( m_data, 2, m_address);
}

// SETMOVE (protected)
// Helper routine for 'run', 'move', 'turn', 'moveTo', 'moveIn' and 'moveOut'.
// - func: 1 = move in, 2 = move out, 3 = move to, 4 = move, 5 = turn, 6 = run
// - value: (move in/move out/move to) 0.1 position unit, (move) 0.1 distance unit, (turn) 0.1 arc unit,
//          (run) 0.1 time unit.
// - speed: percentage of the working velocity.
void InfentoDrive::setMove( byte func, int value, int speed)
{
	if ( !m_ifp ) return;

	clearSignal();

	BTOI btoi;
	m_data[0] = CMD_MD1_MOVE + m_cmdoffs;
	m_data[1] = func;
	btoi.i = value;
	m_data[2] = btoi.b[0];
	m_data[3] = btoi.b[1];
	m_data[4] = speed;
	m_ifp->write( m_data, 5, m_address);
}


//////////////////////////
// InfentoMotor members //
//////////////////////////

// INFENTOMOTOR
// Class constructor.
// - motornumber: CONST_MOTOR1, CONST_MOTOR2 or CONST_EDRIVE
// Motor numbers 1 and 2 correspond to their output connector on the actuator board (MD1 or MD2).
// The e-drive motor has a special purpose output connector.
// Note that when motor MD2 is available on the board, lamp LWPM2 is not.
InfentoMotor::InfentoMotor( InfentoDefinition motornumber)
{
	if ( motornumber == CONST_MOTOR1 ) {
		m_actuator.md1 = 1;
		m_devname = "Motor 1";
		m_cmdoffs = 0;
	}
	else
	if ( motornumber == CONST_MOTOR2 ) {
		m_actuator.md2 = 1;
		m_devname = "Motor 2";
		m_cmdoffs = CMD_MD2_MOVE - CMD_MD1_MOVE;
	}
	else {
		m_actuator.md3 = 1;
		m_devname = "Motor 3";
		m_cmdoffs = CMD_MD3_MOVE - CMD_MD1_MOVE;
	}
	m_child = "InfentoMotor";
}

// CALIBRATE
// Declares the maximum velocity of the robot and rotation speed of the motor.
// - maxmps: the maximum velocity in 0.1 m/s (as produced by the wheel: motor plus gearing).
// - maxrpm: the maximum rotation speed in 0.1 rpm (measured where a rotary counter is/would be placed).
void InfentoMotor::calibrate( int maxmps, int maxrpm)
{
	BTOI mps, rpm;

	// mps to mm per second
	// rpm to mdeg per second
	mps.i = maxmps * 100;
	rpm.i = maxrpm * 600; // = 100 * maxrmp * 360 / 60

	if ( INF_DEBUG_DEVICE ) {
		String smps = "velocity: " + String( maxmps/10) + " m/s";
		String srpm = "turnspeed: " + String( maxrpm/10) + " rpm";
		if ( m_ifp )
			debugroutine( m_child + "::calibrate", smps, srpm);
		else
			debugroutine( m_child + "::calibrate", NOPORT);
	}

	if ( m_ifp ) {
		m_data[0] = CMD_WR_EEPROM;
		InfentoDefinition motnum = motorNumber();

		switch ( motnum ) {
			case CONST_MOTOR1 :		m_data[1] = 0x50; break; // EEPROM_MD1_VELVAL;
			case CONST_MOTOR2 :		m_data[1] = 0x70; break; // EEPROM_MD2_VELVAL;
			case CONST_EDRIVE :		m_data[1] = 0x90; break; // EEPROM_MD3_VELVAL;
			default: return;
		}
		m_data[2] = mps.b[0];
		m_data[3] = mps.b[1];
		m_ifp->write( m_data, 4, m_address);
		delay( 10);

		#ifdef INF_DEBUG_EEPROM
		debugEeprom( m_address, m_data[1]);
		#endif

		switch ( motnum ) {
			case CONST_MOTOR1 :		m_data[1] = 0x52; break; // EEPROM_MD1_TURNVAL;
			case CONST_MOTOR2 :		m_data[1] = 0x72; break; // EEPROM_MD2_TURNVAL;
			case CONST_EDRIVE :		m_data[1] = 0x92; break; // EEPROM_MD3_TURNVAL;
			default: return;
		}
		m_data[2] = rpm.b[0];
		m_data[3] = rpm.b[1];
		m_ifp->write( m_data, 4, m_address);
		delay( 10);

		#ifdef INF_DEBUG_EEPROM
		debugEeprom( m_address, m_data[1]);
		#endif
	}
}

// SETVELOCITYUNIT
// Determines the unit for 'setVelocity'.
// - unit: UNIT_MPS, UNIT_KPH, UNIT_MPH or UNIT_RPM.
void InfentoMotor::setVelocityUnit( InfentoUnit unit)
{
	if ( INF_DEBUG_DEVICE )
		debugroutine( m_child + "::setVelocityUnit", m_ifp ? "unit: " + String( unit) : NOPORT);

	setUnit( 1, unit);
}

// SETDISTANCEUNIT
// Determines the unit for 'move' and 'distance'.
// - unit: UNIT_MTR, UNIT_CM, UNIT_MM or UNIT_INCH.
void InfentoMotor::setDistanceUnit( InfentoUnit unit)
{
	if ( INF_DEBUG_DEVICE )
		debugroutine( m_child + "::setDistanceUnit", m_ifp ? "unit: " + String( unit) : NOPORT);

	setUnit( 2, unit);
}

// SETARCUNIT
// Determines the unit for 'turn' and 'arc'.
// - unit: UNIT_DEG, UNIT_GRAD or UNIT_TICK.
void InfentoMotor::setArcUnit( InfentoUnit unit)
{
	if ( INF_DEBUG_DEVICE )
		debugroutine( m_child + "::setArcUnit", m_ifp ? "unit: " + String( unit) : NOPORT);

	setUnit( 3, unit);
}

// SETTIMEUNIT
// Determines the unit for 'run'.
// - unit: UNIT_MSEC, UNIT_SEC, UNIT_MIN or UNIT_HOUR.
void InfentoMotor::setTimeUnit( InfentoUnit unit)
{
	if ( INF_DEBUG_DEVICE )
		debugroutine( m_child + "::setTimeUnit", m_ifp ? "unit: " + String( unit) : NOPORT);

	setUnit( 4, unit);
}

// SETDIRECTION
// Sets the moving direction of the model. (See the note with 'invertDirection'.)
// - direction: CONST_FORWARD or CONST_REVERSE.
void InfentoMotor::setDirection( InfentoConstant direction)
{
	if ( INF_DEBUG_DEVICE )
		debugroutine( m_child + "::setDirection", m_ifp ? "direction: " + String( direction) : NOPORT);

	if ( m_inverted )
		direction = (direction == CONST_FORWARD ? CONST_REVERSE : CONST_FORWARD);

	setValue( 2, direction);
	delay( 50); // the actuator board needs time to initialize the motor characteristics for the opposite direction
}

// SETVELOCITY
// Sets the working velocity for the current program session.
// - velocity: maximum velocity in 0.1 units during the program. May not exceed the calibrated velocity.
//             0 = full speed.
void InfentoMotor::setVelocity( int velocity)
{
	if ( INF_DEBUG_DEVICE )
		debugroutine( m_child + "::setVelocity", m_ifp ? "velocity: " + String( velocity) : NOPORT);

	setValue( 1, velocity);
}

// SETACCELERATION
// Sets the working acceleration for the current program session.
// The acceleration is applied to all changes in velocity.
// - acceleration: velocity (in 0.1 units) per second.
//                 0 = full power.
void InfentoMotor::setAcceleration( int acceleration)
{
	if ( INF_DEBUG_DEVICE )
		debugroutine( m_child + "::setAcceleration", m_ifp ? "velocity per sec: " + String( acceleration) : NOPORT);

	setValue( 3, acceleration);
}

// INVERTDIRECTION
// Depending on the motor construction the meaning of CONST_FORWARD and CONST_REVERSE needs to be inverted.
// If a model moves backward with 'setDirection( CONST_FORWARD)' the direction should be inverted.
// - invert: TRUE or FALSE
void InfentoMotor::invertDirection( bool invert)
{
	m_inverted = invert;
}

// ON
// Turns the motor on in the current direction ('setDirection') with a certain speed.
// - speed: percentage of the working velocity.
// This routine will signal when the motor comes to full speed because of the acceleration.
void InfentoMotor::on( int speed)
{
	if ( INF_DEBUG_DEVICE )
		debugroutine( m_child + "::on", m_ifp ? "speed: " + String( speed) : NOPORT);

	setOn( speed);
}

// OFF
// Turns the motor off.
// - immediate: FALSE to apply the acceleration when turning the motor off.
//              TRUE to turn the motor off abruptly.
// This routine will signal when the motor comes to stop because of the deceleration.
void InfentoMotor::off( bool immediate)
{
	if ( INF_DEBUG_DEVICE )
		debugroutine( m_child + "::off", m_ifp ? "" : NOPORT);

	setOn( immediate ? 255 : 0);
}

// RUN
// Lets the motor run for a certain time with a certain speed.
// - time: running time in 0.1 time units.
// - speed: percentage of the working velocity.
// Note that the running time cannot be modified as long as the motor runs.
// If needed so, the motor should be turned off first.
// This routine will signal when the motor stopped after the required time.
void InfentoMotor::run( int time, int speed)
{
	if ( INF_DEBUG_DEVICE ) {
		String name = m_child + "::run";
		if ( m_ifp )
			debugroutine( name, "time: " + String( time), "speed: " + String( speed));
		else
			debugroutine( name, NOPORT);
	}

	setMove( 6, time, speed);
}

// MOVE
// Lets the model move for a certain distance with a certain speed.
// This function is available only when there is a rotary sensor attached (see 'attachRotary').
// - distance: moving distance in 0.1 distance units.
// - speed: percentage of the working velocity.
// Note that the moving distance cannot be modified as long as the motor runs.
// If needed so, the motor should be turned off first.
// This routine will signal when the motor stopped after the required distance.
void InfentoMotor::move( int distance, int speed)
{
	if ( INF_DEBUG_DEVICE ) {
		String name = m_child + "::move";
		if ( m_ifp )
			debugroutine( name, "distance: " + String( distance), "speed: " + String( speed));
		else
			debugroutine( name, NOPORT);
	}

	setMove( 4, distance, speed);
}

// TURN
// Lets the model turn for a certain arc with a certain speed.
// This function is available only when there is a rotary sensor attached (see 'attachRotary').
// - distance: turning arc in 0.1 arc units.
// - speed: percentage of the working velocity.
// Note that the turning arc cannot be modified as long as the motor runs.
// If needed so, the motor should be turned off first.
// This routine will signal when the motor stopped after the required rotation.
void InfentoMotor::turn( int arc, int speed)
{
	if ( INF_DEBUG_DEVICE ) {
		String name = m_child + "::turn";
		if ( m_ifp )
			debugroutine( name, "arc: " + String( arc), "speed: " + String( speed));
		else
			debugroutine( name, NOPORT);
	}

	setMove( 5, arc, speed);
}

// DISTANCE
// Gives the distance a model has moved since the latest 'move' command.
// This function is available only when there is a rotary sensor attached (see 'attachRotary').
// - returns: the distance in 0.1 distance unit.
int  InfentoMotor::distance()
{
	BTOI btoi;
	btoi.i = 0;

	if ( m_ifp ) {
		m_data[0] = CMD_MD1_READ + m_cmdoffs;
		m_data[1] = 1;
		m_ifp->write( m_data, 2, m_address);
		m_ifp->read( m_data, 2, m_address);
		btoi.b[0] = m_data[0];
		btoi.b[1] = m_data[1];
	}

	if ( INF_DEBUG_DEVICE )
		debugroutine( m_child + "::distance", "returns: " + String( btoi.i));

	return btoi.i;
}

// ARC
// Gives the arc the motor has turned since the latest 'turn' command.
// This function is available only when there is a rotary sensor attached (see 'attachRotary').
// - returns: the arc in 0.1 arc unit.
int  InfentoMotor::arc()
{
	BTOI btoi;
	btoi.i = 0;

	if ( m_ifp ) {
		m_data[0] = CMD_MD1_READ + m_cmdoffs;
		m_data[1] = 2;
		m_ifp->write( m_data, 2, m_address);
		m_ifp->read( m_data, 2, m_address);
		btoi.b[0] = m_data[0];
		btoi.b[1] = m_data[1];
	}

	if ( INF_DEBUG_DEVICE )
		debugroutine( m_child + "::arc", "returns: " + String( btoi.i));

	return btoi.i;
}

// ATTACHROTARY
// Assigns the rotary sensor of the actuator board to the current motor.
// There is only one rotary sensor available, so one should choose to assign it to MD1 or MD2.
// The routines 'move', 'run', 'distance' and 'arc' are available only when the rotary is attached.
// - steps: number of ticks per full (360 degree) rotation of the wheel (motor plus gearing).
// - diameter: wheel diameter in millimeter (0.1 UNIT_CM).
// IMPORTANT NOTE: the ratio of diameter and steps may not exeed the 50mm per step.
void InfentoMotor::attachRotary( int steps, int diameter)
{
	if ( !m_ifp || (steps < 1) ) {
		if ( INF_DEBUG_DEVICE )
			debugroutine( m_child + "::attachRotary", m_ifp ? "no rotary steps" : NOPORT);
		return;
	}

	m_data[0] = CMD_MD1_USEROT + m_cmdoffs;
	m_ifp->write( m_data, 1, m_address);

	BTOI dist, arc;

	// arc in millidegree per step
	// dist in micrometer per step
	arc.i = 360000 / steps; // = 1000 * 360 / steps
	if ( diameter )
		dist.i = 3142 * (long) diameter / steps; // 1000 * (diameter * 3.142 / steps)
	else
		dist.i = 10;

	if ( INF_DEBUG_DEVICE ) {
		String sarc = "steps per rotation: " + String( steps);
		String sdiam = "diameter: " + String( diameter/10) + " mm";
		if ( m_ifp )
			debugroutine( m_child + "::calibrate", sarc, sdiam);
		else
			debugroutine( m_child + "::calibrate", NOPORT);
	}

	m_data[0] = CMD_WR_EEPROM;
	InfentoDefinition motnum = motorNumber();

	switch ( motnum ) {
		case CONST_MOTOR1 :		m_data[1] = 0x54; break; // EEPROM_MD1_DISTVAL;
		case CONST_MOTOR2 :		m_data[1] = 0x74; break; // EEPROM_MD2_DISTVAL;
		case CONST_EDRIVE :		m_data[1] = 0x94; break; // EEPROM_MD3_DISTVAL;
		default: return;
	}
	m_data[2] = dist.b[0];
	m_data[3] = dist.b[1];
	m_ifp->write( m_data, 4, m_address);
	delay( 10);

	#ifdef INF_DEBUG_EEPROM
	debugEeprom( m_address, m_data[1]);
	#endif

	switch ( motnum ) {
		case CONST_MOTOR1 :		m_data[1] = 0x56; break; // EEPROM_MD1_ARCVAL;
		case CONST_MOTOR2 :		m_data[1] = 0x76; break; // EEPROM_MD2_ARCVAL;
		case CONST_EDRIVE :		m_data[1] = 0x96; break; // EEPROM_MD3_ARCVAL;
		default: return;
	}
	m_data[2] = arc.b[0];
	m_data[3] = arc.b[1];
	m_ifp->write( m_data, 4, m_address);
	delay( 10);

	#ifdef INF_DEBUG_EEPROM
	debugEeprom( m_address, m_data[1]);
	#endif
}


///////////////////////////
// InfentoLinAct members //
///////////////////////////

// INFENTOLINACT
// Class constructor.
// - motornumber: CONST_LINACT1 or CONST_LINACT2.
// Linear actuator numbers 1 and 2 correspond to their output connector on the actuator board (MD1 or MD2).
// Note that when motor MD2 is available on the board, lamp LWPM2 is not.
InfentoLinAct::InfentoLinAct( InfentoDefinition motornumber)
{
	if ( motornumber == CONST_LINACT1 ) {
		m_actuator.md1 = 1;
		m_devname = "Linear Actuator 1";
		m_cmdoffs = 0;
	}
	else {
		m_actuator.md2 = 1;
		m_devname = "Linear Actuator 2";
		m_cmdoffs = CMD_MD2_MOVE - CMD_MD1_MOVE;
	}
	m_child = "InfentoLinAct";
}

// CALIBRATE
// Declares the shaft length of the linear actuar.
// - shaftlength: shaft length in millimeter (0.1 UNIT_CM).
// - endin: end switch position in millimeter (0.1 UNIT_CM) from full in.
// - endout: end switch position in millimeter (0.1 UNIT_CM) from full in.
// - returns: actual shaft length used by the driver.
// The calibrated shaft length should be the distance between the extreme positions.
// These are the positions where the shaft is stopped by the end switches.
// Note that the actual distance can diverge from the value in the datasheet.
// To find the extreme positions, use 'moveIn()' and 'moveOut()'.
// When uncalibrated still these routines will work fine, but possibly do not raise signals.
// In order to raise signals the linear actuator must be stopped by the driver before it hits an end switch.
// The stop distances are calibrated via 'endin' and 'endout'.
void InfentoLinAct::calibrate( int shaftlength, int endin, int endout)
{
	if ( INF_DEBUG_DEVICE ) {
		String slen = "length: " + String( shaftlength/10) + " cm";
		debugroutine( m_child + "::calibrate", m_ifp ? slen : NOPORT);
	}

	if ( m_ifp ) {
		BTOI btoi;
		InfentoDefinition motnum = motorNumber();

		// the AD-converter produces values 0..4096 (4095 steps)
		long microperstep = 1000 * (long) shaftlength / 4095;

		switch ( motnum ) {
			case CONST_LINACT1 :	m_data[1] = 0x54; break; // EEPROM_MD1_DISTVAL;
			case CONST_LINACT2 :	m_data[1] = 0x74; break; // EEPROM_MD2_DISTVAL;
			default: return;
		}
		m_data[0] = CMD_WR_EEPROM;
		btoi.i = microperstep;
		m_data[2] = btoi.b[0];
		m_data[3] = btoi.b[1];
		m_ifp->write( m_data, 4, m_address);
		delay( 10);

		#ifdef INF_DEBUG_EEPROM
		debugEeprom( m_address, m_data[1]);
		#endif

		if ( endin >= 0 ) {
			switch ( motnum ) {
				case CONST_LINACT1 :	m_data[1] = 0x48; break; // EEPROM_MD1_MINSENS;
				case CONST_LINACT2 :	m_data[1] = 0x68; break; // EEPROM_MD2_MINSENS;
				default: return;
			}
			m_data[0] = CMD_WR_EEPROM;
			btoi.i = 1000 * (long) endin / microperstep;
			m_data[2] = btoi.b[0];
			m_data[3] = btoi.b[1];
			m_ifp->write( m_data, 4, m_address);
			delay( 10);

			#ifdef INF_DEBUG_EEPROM
			debugEeprom( m_address, m_data[1]);
			#endif
		}

		if ( endout >= 0 ) {
			switch ( motnum ) {
				case CONST_LINACT1 :	m_data[1] = 0x4A; break; // EEPROM_MD1_MAXSENS;
				case CONST_LINACT2 :	m_data[1] = 0x6A; break; // EEPROM_MD2_MAXSENS;
				default: return;
			}
			m_data[0] = CMD_WR_EEPROM;
			btoi.i = 1000 * (long) endout / microperstep;
			m_data[2] = btoi.b[0];
			m_data[3] = btoi.b[1];
			m_ifp->write( m_data, 4, m_address);
			delay( 10);

			#ifdef INF_DEBUG_EEPROM
			debugEeprom( m_address, m_data[1]);
			#endif
		}
	}
}

// SETPOSITIONUNIT
// Determines the unit for 'moveTo', 'moveIn', 'moveOut' and 'position'.
// - unit: UNIT_CM, UNIT_MM or UNIT_INCH.
void InfentoLinAct::setPositionUnit( InfentoUnit unit)
{
	if ( INF_DEBUG_DEVICE )
		debugroutine( m_child + "::setPositionUnit", m_ifp ? "unit: " + String( unit) : NOPORT);

	setUnit( 2, unit);
}

// MOVETO
// Lets the linear actuator shift to a certain position with a certain speed.
// - position: aimed position in 0.1 position units.
// - speed: percentage of the linear actuator's maximum speed.
// Note that the moving position cannot be modified as long as the linear actuator runs.
// If needed so, the linear actuator should be turned off first.
// This routine will signal when the linear actuator stopped at the required position.
void InfentoLinAct::moveTo( int position, int speed)
{
	if ( INF_DEBUG_DEVICE ) {
		String name = m_child + "::moveTo";
		if ( m_ifp )
			debugroutine( name, "position: " + String( position), "speed: " + String( speed));
		else
			debugroutine( name, NOPORT);
	}

	setMove( 3, position, speed);
}

// MOVEIN
// Lets the linear actuator shift in for to a certain distance with a certain speed.
// - distance: distance in 0.1 position units.
// - speed: percentage of the linear actuator's maximum speed.
// This routine will signal when the linear actuator stopped after the required shift.
void InfentoLinAct::moveIn( int distance, int speed)
{
	if ( INF_DEBUG_DEVICE ) {
		String name = m_child + "::moveIn";
		if ( m_ifp )
			debugroutine( name, "distance: " + String( distance), "speed: " + String( speed));
		else
			debugroutine( name, NOPORT);
	}

	if ( !distance ) distance = 10000;

	setMove( 1, distance, speed);
}

// MOVEOUT
// Lets the linear actuator shift out for to a certain distance with a certain speed.
// - distance: distance in 0.1 position units.
// - speed: percentage of the linear actuator's maximum speed.
// This routine will signal when the linear actuator stopped after the required shift.
void InfentoLinAct::moveOut( int distance, int speed)
{
	if ( INF_DEBUG_DEVICE ) {
		String name = m_child + "::moveOut";
		if ( m_ifp )
			debugroutine( name, "distance: " + String( distance), "speed: " + String( speed));
		else
			debugroutine( name, NOPORT);
	}

	if ( !distance ) distance = 10000;

	setMove( 2, distance, speed);
}

// OFF
// Turns the linear actuator off.
// - immediate: FALSE to apply the acceleration when turning the linear actuator off.
//              TRUE to turn the linear actuator off abruptly.
// This routine will signal when the motor comes to stop because of the deceleration.
void InfentoLinAct::off( bool immediate)
{
	if ( INF_DEBUG_DEVICE )
		debugroutine( m_child + "::off", m_ifp ? "" : NOPORT);

	setOn( immediate ? 255 : 0);
}

// POSITION
// Gives the current shaft position of the linear actuator.
// - returns: the position in 0.1 position unit.
int InfentoLinAct::position()
{
	BTOI btoi;
	btoi.i = 0;

	if ( m_ifp ) {
		m_data[0] = CMD_MD1_READ + m_cmdoffs;
		m_data[1] = 1; // read position
		m_ifp->write( m_data, 2, m_address);
		m_ifp->read( m_data, 2, m_address);
		btoi.b[0] = m_data[0];
		btoi.b[1] = m_data[1];
	}

	if ( INF_DEBUG_DEVICE )
		debugroutine( m_child + "::position", m_ifp ? "returns: " + String( btoi.i) : NOPORT);

	return btoi.i;
}

// SETHOME
// The routine 'setHome' records the current shaft position as home position and thereby supports
// programming a model. It is impossible to push or pull a linear actuator's shaft manually to its
// home position when testing a program. Instead 'setHome' and 'goHome' will do the job.
void InfentoLinAct::setHome()
{
	if ( INF_DEBUG_DEVICE )
		debugroutine( m_child + "::setHome", m_ifp ? "" : NOPORT);

	home( 1);
}

// GOHOME
// Shifts a linear actuator to its home position (see 'setHome').
// This routine will signal when the linear actuator stopped after the required shift.
void InfentoLinAct::goHome()
{
	if ( INF_DEBUG_DEVICE )
		debugroutine( m_child + "::goHome", m_ifp ? "" : NOPORT);

	clearSignal();

	home( 0);
}

// HOME (protected)
// Helper routine for 'setHome' and 'goHome'.
// - func: 0 = go home, 1 = set home
void InfentoLinAct::home( byte func)
{
	if ( !m_ifp ) return;
	m_data[0] = CMD_MD1_ACTHOME + m_cmdoffs;
	m_data[1] = func;
	m_ifp->write( m_data, 2, m_address);
}


/////////////////////////
// InfentoGate members //
/////////////////////////

// INFENTOGATE
// Class constructor.
// - gatenumber: CONST_GATE1 or CONST_GATE2
// Gate numbers 1 and 2 correspond to their output connector on the actuator board (GT1 or GT2).
InfentoGate::InfentoGate( InfentoDefinition gatenumber)
{
	if ( gatenumber == CONST_GATE1 ) {
		m_sensor.gt1 = 1;
		m_devname = "Gate 1";
		m_cmdoffs = 0;
	}
	else {
		m_sensor.gt2 = 1;
		m_devname = "Gate 2";
		m_cmdoffs = CMD_GT2_SIGNAL - CMD_GT1_SIGNAL;
	}
	m_child = "InfentoGate";
}

// GATENUMBER
// Gives the gate number of the device.
// - returns: CONST_GATE1 or CONST_GATE2.
// (See the InfentoGate constructor.)
InfentoDefinition InfentoGate::gateNumber()
{
	if ( m_sensor.gt1 ) return CONST_GATE1;
	if ( m_sensor.gt2 ) return CONST_GATE2;
	return CONST_UNDEFINED;
}

// OPEN
// Gives if the status of the gate is open.
// - returns: TRUE (= open) or FALSE (= closed).
bool InfentoGate::open()
{
	bool gateopen = false;

	if ( m_ifp ) {
		m_data[0] = CMD_GT1_READ + m_cmdoffs;
		m_ifp->write( m_data, 1, m_address);
		m_ifp->read( m_data, 1, m_address);
		gateopen = (m_data[0] == 0);
	}

	if ( INF_DEBUG_DEVICE )
		debugroutine( m_child + "::open", m_ifp ? "returns: " + String( gateopen ? "yes" : "no") : NOPORT);

	return gateopen;
}

// CLOSED
// Gives if the status of the gate is closed.
// - returns: TRUE (= closed) or FALSE (= open).
bool InfentoGate::closed()
{
	bool gateclosed = false;

	if ( m_ifp ) {
		m_data[0] = CMD_GT1_READ + m_cmdoffs;
		m_ifp->write( m_data, 1, m_address);
		m_ifp->read( m_data, 1, m_address);
		gateclosed = (m_data[0] == 1);
	}

	if ( INF_DEBUG_DEVICE )
		debugroutine( m_child + "::closed", m_ifp ? "returns: " + String( gateclosed ? "yes" : "no") : NOPORT);

	return gateclosed;
}

// SIGNALOPENING
// Requests the sensor board to signal when the gate will go open.
void InfentoGate::signalOpening()
{
	if ( INF_DEBUG_DEVICE )
		debugroutine( m_child + "::signalOpening");

	if ( !m_ifp ) return;

	clearSignal();

	m_data[0] = CMD_GT1_SIGNAL + m_cmdoffs;
	m_data[1] = 1;
	m_ifp->write( m_data, 2, m_address);
}

// SIGNALCLOSING
// Requests the sensor board to signal when the gate will be closed.
void InfentoGate::signalClosing()
{
	if ( INF_DEBUG_DEVICE )
		debugroutine( m_child + "::signalClosing");

	if ( !m_ifp ) return;

	clearSignal();

	m_data[0] = CMD_GT1_SIGNAL + m_cmdoffs;
	m_data[1] = 2;
	m_ifp->write( m_data, 2, m_address);
}


///////////////////////////////
// InfentoSwitchPack members //
///////////////////////////////

// INFENTOSWITCHPACK
// Class constructor.
// A switch pack may consist of eight switches.
InfentoSwitchPack::InfentoSwitchPack()
{
	m_child = "InfentoSwitchPack";
	m_sensor.swp = 1;
	m_devname = "Switch pack";
}

// SETRESPONSE
// Determines the type of response used for the switch pack.
// - response: CONST_ONCLOSE = the switches are treated as normal open
//             CONST_ONOPEN = the switches are treated as normal closed
void InfentoSwitchPack::setResponse( InfentoConstant response)
{
	if ( INF_DEBUG_DEVICE )
		debugroutine( m_child + "::setResponse", m_ifp ? "response: " + String( response) : NOPORT);

	if ( !m_ifp ) return;
	m_data[0] = CMD_SWP_SETVAL;
	m_data[1] = response;
	m_ifp->write( m_data, 2, m_address);
}

// ISON
// Gives back if one of the switches got a response.
bool InfentoSwitchPack::isOn()
{
	bool swon = false;

	if ( m_ifp ) {
		m_data[0] = CMD_SWP_READ;
		m_ifp->write( m_data, 1, m_address);
		m_ifp->read( m_data, 1, m_address);
//		m_data[0] ^= 0xFF;
//		m_data[0] &= 0x1F;
		swon = (m_data[0] != 0);
	}

	if ( INF_DEBUG_DEVICE )
		debugroutine( m_child + "::on", "returns: " + String(swon ? "yes" : "no"));

	return swon;
}

// ISOFF
// Gives back if none of the switches got a response.
bool InfentoSwitchPack::isOff()
{
	bool swoff = false;

	if ( m_ifp ) {
		m_data[0] = CMD_SWP_READ;
		m_ifp->write( m_data, 1, m_address);
		m_ifp->read( m_data, 1, m_address);
//		m_data[0] ^= 0xFF;
//		m_data[0] &= 0x1F;
		swoff = (m_data[0] == 0);
	}

	if ( INF_DEBUG_DEVICE )
		debugroutine( m_child + "::off", "returns: " + String(swoff ? "yes" : "no"));

	return swoff;
}

// POSITION
// Gives the bit position of the switches that got a reponse.
// Each switch represents a bit position.
// The bit position is masked by CONST_SWITCH_n where n is the switch number 1 through 8.
InfentoConstant InfentoSwitchPack::position()
{
	InfentoConstant ret = CONST_NONE;

	if ( m_ifp ) {
		m_data[0] = CMD_SWP_READ;
		m_ifp->write( m_data, 1, m_address);
		m_ifp->read( m_data, 1, m_address);
//		m_data[0] ^= 0xFF;
//		m_data[0] &= 0x1F;
		ret = (InfentoConstant) m_data[0];
	}

	if ( INF_DEBUG_DEVICE )
		debugroutine( m_child + "::position", "returns: " + String( ret, BIN));

	return ret;
}

// SIGNALPOSITION
// Requests the sensor board to signal a change of reponse of the switches.
// Each switch represents a bit position.
// The bit position is masked by CONST_SWITCH_n where n is the switch number 1 through 8.
// - mask: masks the bit position of the switches involved.
// - func: CONST_REACHED = signal when one of the masked switches got a response
//         CONST_RELEASED = signal when one of the masked switches no longer has a response
//         CONST_CHANGED = signal when one of the masked switches got or no longer has a response
void InfentoSwitchPack::signalPosition( InfentoConstant mask, InfentoConstant func)
{
	if ( INF_DEBUG_DEVICE ) {
		String name = m_child + "::signalPosition";
		if ( m_ifp )
			debugroutine( name, "mask: " + String( mask, BIN), "function: " + String( func));
		else
			debugroutine( name, NOPORT);
	}

	if ( !m_ifp ) return;

	clearSignal();

	m_data[0] = CMD_SWP_SIGNAL;
	m_data[1] = mask;
	m_data[2] = (byte) func;
	m_ifp->write( m_data, 3, m_address);
}


//////////////////////////
// InfentoLevel members //
//////////////////////////

// INFENTOLEVEL
// Class constructor.
// Base class for 'InfentoMicrophone', 'InfentoLuminance' and 'InfentoResistance'.
InfentoLevel::InfentoLevel()
{
}

// SETTIMER
// Determines the time between each distance measurement.
// The timer affects the measured direction via 'distance( InfentoConstant& direction)'.
// Use high timer values for low speeds and visa versa.
// - time: time in 0.1 sec between the distance measurements.
void InfentoLevel::setTimer( int time)
{
	if ( INF_DEBUG_DEVICE )
		debugroutine( m_child + "::setTimer", m_ifp ? "time: " + String( time) : NOPORT);

	if ( !m_ifp ) return;

	m_data[0] = CMD_MIC_SETVAL + m_cmdoffs;
	m_data[1] = 2;
	m_data[2] = time;
	m_ifp->write( m_data, 3, m_address);
}

// LEVEL
// Gives the sound, luminance or resitance level.
// - returns: the level as percentage.
int InfentoLevel::level()
{
	int ret = 0;
	if ( m_ifp ) {
		m_data[0] = CMD_MIC_READ + m_cmdoffs;
		m_ifp->write( m_data, 1, m_address);
		m_ifp->read( m_data, 1, m_address);
		ret = m_data[0];
	}

	if ( INF_DEBUG_DEVICE )
		debugroutine( m_child + "::level", m_ifp ? "returns: " + String( ret) : NOPORT);

	return ret;
}

// SETSIGNALRANGE
// Determines the precision range for signalling a level.
// Signalling takes place when the level plus or minus the range occurs.
// - range: level as percentage.
void InfentoLevel::setSignalRange( int range)
{
	if ( INF_DEBUG_DEVICE )
		debugroutine( m_child + "::setSignalRange", m_ifp ? "range: " + String( range) : NOPORT);

	if ( !m_ifp ) return;

	BTOI btoi;
	btoi.i = range;
	m_data[0] = CMD_MIC_SETVAL + m_cmdoffs;
	m_data[1] = 1;
	m_data[2] = btoi.b[0];
	m_data[3] = btoi.b[1];
	m_ifp->write( m_data, 4, m_address);
}

// SIGNALLEVEL
// Requests the sensor board to signal a certain sound, luminance or resistance level.
// - level: the level as percentage.
// - func: CONST_REACHED = signal the given level.
//         CONST_RELEASED = signal another level than the given one.
//         CONST_LOWER = signal when the level is lower than a certain level.
//         CONST_HIGHER = signal when the level is higher than a certain level.
void InfentoLevel::signalLevel( int level, InfentoConstant func)
{
	if ( INF_DEBUG_DEVICE ) {
		String name = m_child + "::signalLevel";
		if ( m_ifp )
			debugroutine( name, "level: " + String( level), "function: " + String( func));
		else
			debugroutine( name, NOPORT);
	}

	if ( !m_ifp ) return;

	clearSignal();

	m_data[0] = CMD_MIC_SIGNAL + m_cmdoffs;
	m_data[1] = level;
	m_data[2] = func;
	m_ifp->write( m_data, 3, m_address);
}


///////////////////////////////
// InfentoMicrophone members //
///////////////////////////////

// INFENTOMICROPHONE
// Class constructor.
InfentoMicrophone::InfentoMicrophone()
{
	m_sensor.mic = 1;
	m_devname = "Microphone";
	m_cmdoffs = 0;
	m_child = "InfentoMicrophone";
}

/*
//////////////////////////////
// InfentoLuminance members //
//////////////////////////////

// INFENTOLUMINANCE
// Class constructor.
InfentoLuminance::InfentoLuminance()
{
	m_sensor.lum = 1;
	m_devname = "Luminance";
	m_cmdoffs = CMD_LUM_SIGNAL - CMD_MIC_SIGNAL;
	m_child = "InfentoLuminance";
}

// SETLEVELUNIT
// Determines the unit for 'level' and 'signalLevel'.
// - unit: UNIT_LUX or UNIT_PERC.
void InfentoLuminance::setLevelUnit( InfentoUnit unit)
{
	if ( INF_DEBUG_DEVICE )
		debugroutine( m_child + "::setLevelUnit", m_ifp ? "unit: " + String( unit) : NOPORT);

	if ( !m_ifp ) return;
	m_data[0] = CMD_LUM_SETUNIT;
	m_data[1] = unit;
	m_ifp->write( m_data, 2, m_address);
}
*/

///////////////////////////////
// InfentoResistance members //
///////////////////////////////

// INFENTORESISTANCE
// Class constructor.
InfentoResistance::InfentoResistance()
{
	m_sensor.res = 1;
	m_devname = "Resistance";
	m_cmdoffs = CMD_RES_SIGNAL - CMD_MIC_SIGNAL;
	m_child = "InfentoResistance";
}

// SETREFERENCE
// Changes the reference resistance on the sensor board to a certain value.
// - resistance: the reference resistance in ohm.
// The sensor board contains a programmable resistor that acts as voltage devider for the connected sensor.
void InfentoResistance::setReference( long resistance)
{
	if ( INF_DEBUG_DEVICE )
		debugroutine( m_child + "::setReference", m_ifp ? "resistance: " + String( resistance) : NOPORT);

	if ( !m_ifp ) return;
	BTOL btol;
	btol.l = resistance;
	m_data[0] = CMD_RES_SETREF;
	m_data[1] = btol.b[0];
	m_data[2] = btol.b[1];
	m_data[3] = btol.b[2];
	m_data[4] = btol.b[3];
	m_ifp->write( m_data, 5, m_address);
}


///////////////////////////
// InfentoRotary members //
///////////////////////////

// INFENTOROTARY
// Class constructor
// This class supports the stand allone usage of the rotary sensor.
// Note that this class cannot be used when a motor has attached the rotary.
InfentoRotary::InfentoRotary()
{
	m_actuator.rot = 1;
	m_devname = "Rotary";
	m_child = "InfentoRotary";
}

// CALIBRATE
// Declares the number of steps per rotation and the diameter of the wheel.
// - steps: number of steps per rotation (motor plus gearing).
// - diameter: wheel diameter in millimeter (0.1 UNIT_CM).
void InfentoRotary::calibrate( int steps, int diameter)
{
	if ( !m_ifp || (steps < 1) ) {
		if ( INF_DEBUG_DEVICE )
			debugroutine( m_child + "::calibrate", m_ifp ? "no rotary steps" : NOPORT);
		return;
	}

	BTOI dist, arc;

	// arc in millidegree per step
	// dist in micrometer per step
	arc.i = 360000 / steps; // = 1000 * 360 / steps
	if ( diameter )
		dist.i = 3142 * (long) diameter / steps; // 1000 * (diameter * 3.142 / steps)
	else
		dist.i = 10;

	if ( INF_DEBUG_DEVICE ) {
		String sarc = "steps per rotation: " + String( steps);
		String sdiam = "diameter: " + String( diameter/10) + " mm";
		if ( m_ifp )
			debugroutine( m_child + "::calibrate", sarc, sdiam);
		else
			debugroutine( m_child + "::calibrate", NOPORT);
	}

	m_data[0] = CMD_WR_EEPROM;

	m_data[1] = 0xA8; // EEPROM_ROT_DISTVAL
	m_data[2] = dist.b[0];
	m_data[3] = dist.b[1];
	m_ifp->write( m_data, 4, m_address);
	delay( 10);

	m_data[1] = 0xAA; // EEPROM_ROT_ARCVAL
	m_data[2] = arc.b[0];
	m_data[3] = arc.b[1];
	m_ifp->write( m_data, 4, m_address);
	delay( 10);
}

// SETDISTANCEUNIT
// Determines the unit for 'distance' and 'signalDistance'.
// - unit: UNIT_MTR, UNIT_CM, UNIT_MM or UNIT_INCH.
void InfentoRotary::setDistanceUnit( InfentoUnit unit)
{
	if ( INF_DEBUG_DEVICE )
		debugroutine( m_child + "::setDistanceUnit", m_ifp ? "unit: " + String( unit) : NOPORT);

	if ( !m_ifp ) return;
	m_data[0] = CMD_ROT_SETUNIT;
	m_data[1] = 2;
	m_data[2] = unit;
	m_ifp->write( m_data, 3, m_address);
}

// SETARCUNIT
// Determines the unit for 'arc' and 'signalArc'.
// - unit: UNIT_DEG, UNIT_GRAD or UNIT_TICK.
void InfentoRotary::setArcUnit( InfentoUnit unit)
{
	if ( INF_DEBUG_DEVICE )
		debugroutine( m_child + "::setArcUnit", m_ifp ? "unit: " + String( unit) : NOPORT);

	if ( !m_ifp ) return;
	m_data[0] = CMD_ROT_SETUNIT;
	m_data[1] = 3;
	m_data[2] = unit;
	m_ifp->write( m_data, 3, m_address);
}

// CLEAR
// Sets the rotary counter back to null.
void InfentoRotary::clear()
{
	if ( INF_DEBUG_DEVICE )
		debugroutine( m_child + "::clear", m_ifp ? "" : NOPORT);

	if ( !m_ifp ) return;
	m_data[0] = CMD_ROT_CLEAR;
	m_ifp->write( m_data, 1, m_address);
}

// DISTANCE
// Gives the distance that a model has moved since the latest 'clear', 'signalDistance' or 'signalArc' command.
// - returns: the distance in 0.1 distance unit.
int  InfentoRotary::distance()
{
	BTOI btoi;
	btoi.i = 0;

	if ( m_ifp ) {
		m_data[0] = CMD_ROT_READ;
		m_data[1] = 1;
		m_ifp->write( m_data, 2, m_address);
		m_ifp->read( m_data, 2, m_address);
		btoi.b[0] = m_data[0];
		btoi.b[1] = m_data[1];
	}

	if ( INF_DEBUG_DEVICE )
		debugroutine( m_child + "::distance", "returns: " + String( btoi.i));

	return btoi.i;
}

// ARC
// Gives the arc that the rotary has turned since the latest 'clear', 'signalDistance' or 'signalArc' command.
// - returns: the arc in 0.1 arc unit.
int  InfentoRotary::arc()
{
	BTOI btoi;
	btoi.i = 0;

	if ( m_ifp ) {
		m_data[0] = CMD_ROT_READ;
		m_data[1] = 2;
		m_ifp->write( m_data, 2, m_address);
		m_ifp->read( m_data, 2, m_address);
		btoi.b[0] = m_data[0];
		btoi.b[1] = m_data[1];
	}

	if ( INF_DEBUG_DEVICE )
		debugroutine( m_child + "::arc", "returns: " + String( btoi.i));

	return btoi.i;
}

// SIGNALDISTANCE
// Requests the actuator board to signal a certain change of distance.
// - distance: the distance in 0.1 distance unit.
void InfentoRotary::signalDistance( int distance)
{
	if ( INF_DEBUG_DEVICE )
		debugroutine( m_child + "::signalDistance", m_ifp ? "distance: " + String( distance) : NOPORT);

	if ( !m_ifp ) return;

	clearSignal();

	BTOI btoi;
	btoi.i = distance;
	m_data[0] = CMD_ROT_SIGNAL;
	m_data[1] = 1;
	m_data[2] = btoi.b[0];
	m_data[3] = btoi.b[1];
	m_ifp->write( m_data, 4, m_address);
}

// SIGNALARC
// Requests the actuator board to signal a certain change of arc.
// - arc: the arc in 0.1 arc unit.
void InfentoRotary::signalArc( int arc)
{
	if ( INF_DEBUG_DEVICE )
		debugroutine( m_child + "::signalArc", m_ifp ? "arc: " + String( arc) : NOPORT);

	if ( !m_ifp ) return;

	clearSignal();

	BTOI btoi;
	btoi.i = arc;
	m_data[0] = CMD_ROT_SIGNAL;
	m_data[1] = 2;
	m_data[2] = btoi.b[0];
	m_data[3] = btoi.b[1];
	m_ifp->write( m_data, 4, m_address);
}


/////////////////////////////
// InfentoDistance members //
/////////////////////////////

// INFENTODISTANCE
// Class constructor
InfentoDistance::InfentoDistance()
{
	m_sensor.dist = 1;
	m_devname = "Distance";
	m_child = "InfentoDistance";
	m_distance = 0;
	m_direction = CONST_STEADY;
	m_count = 0;
}

// SETDISTANCEUNIT
// Determines the unit for 'setRange', 'distance'.
// - unit: UNIT_MTR, UNIT_CM, UNIT_MM or UNIT_INCH.
void InfentoDistance::setDistanceUnit( InfentoUnit unit)
{
	if ( INF_DEBUG_DEVICE )
		debugroutine( m_child + "::setDistanceUnit", m_ifp ? "unit: " + String( unit) : NOPORT);

	if ( !m_ifp ) return;
	m_data[0] = CMD_DIST_SETUNIT;
	m_data[1] = unit;
	m_ifp->write( m_data, 2, m_address);
	m_distance = 0;
	m_direction = CONST_STEADY;
	m_count = 0;
}

// SETTIMER
// Determines the time between each distance measurement.
// The timer affects the measured direction via 'distance( InfentoConstant& direction)'.
// Use high timer values for low speeds and visa versa.
// - time: time in 0.1 sec between the distance measurements.
void InfentoDistance::setTimer( int time)
{
	if ( INF_DEBUG_DEVICE )
		debugroutine( m_child + "::setTimer", m_ifp ? "time: " + String( time) : NOPORT);

	if ( !m_ifp ) return;

	m_data[0] = CMD_DIST_SETVAL;
	m_data[1] = 2;
	m_data[2] = time;
	m_ifp->write( m_data, 3, m_address);
}

// DISTANCE
// Gives the distance to an object.
// - returns: the distance in 0.1 distance unit.
int InfentoDistance::distance()
{
	InfentoConstant dir;
	return distance( dir);
}

// DISTANCE
// Gives the distance to an object and the direction the object is moving.
// - returns: the distance in 0.1 distance unit.
//            parameter direction: CONST_STEADY, CONST_APPROACH or CONST_LEAVE
int InfentoDistance::distance( InfentoConstant& direction)
{
	String str;
	BTOI btoi;
	btoi.i = 0;
	int dist;

	if ( m_ifp ) {
		m_data[0] = CMD_DIST_READ;
		m_ifp->write( m_data, 1, m_address);
		m_ifp->read( m_data, 2, m_address);
		btoi.b[0] = m_data[0];
		btoi.b[1] = m_data[1];
	}
	dist = btoi.i;

	if ( dist > 0 ) {
		direction = CONST_APPROACH;
		str = "approaching";
	}
	else {
		dist = -dist;
		if ( dist == m_distance ) {
			direction = CONST_STEADY;
			str = "steady";
		}
		else {
			direction = CONST_LEAVE;
			str = "leaving";
		}
	}
	
	m_distance = dist;

	if ( INF_DEBUG_DEVICE )
		debugroutine( m_child + "::distance", "returns: " + String( dist) + " " + str);

	return dist;
}

// SETRANGE
// Determines the precision range for signalling a distance.
// Signalling takes place when the distance plus or minus the range occurs.
// - range: distance in 0.1 distance unit.
void InfentoDistance::setSignalRange( int range)
{
	if ( INF_DEBUG_DEVICE )
		debugroutine( m_child + "::setSignalRange", m_ifp ? "range: " + String( range) : NOPORT);

	if ( !m_ifp ) return;

	BTOI btoi;
	btoi.i = range;
	m_data[0] = CMD_DIST_SETVAL;
	m_data[1] = 1;
	m_data[2] = btoi.b[0];
	m_data[3] = btoi.b[1];
	m_ifp->write( m_data, 4, m_address);
}

// SIGNALDISTANCE
// Requests the sensor board to signal a certain distance plus or minus a range (see 'setRange').
// - distance: the distance in 0.1 distance unit.
// - func: CONST_REACHED = signal the given distance to an object.
//         CONST_RELEASED = signal another distance to an object than the given one.
//         CONST_CLOSER = signal when an object is closer than a certain distance.
//         CONST_FURTHER = signal when an object is further than a certain distance.
void InfentoDistance::signalDistance( int distance, InfentoConstant func)
{
	if ( INF_DEBUG_DEVICE ) {
		String name = m_child + "::signalDistance";
		if ( m_ifp )
			debugroutine( name, "distance: " + String( distance), "function: " + String( func));
		else
			debugroutine( name, NOPORT);
	}

	if ( !m_ifp ) return;

	clearSignal();

	BTOI btoi;
	btoi.i = distance;
	m_data[0] = CMD_DIST_SIGNAL;
	m_data[1] = btoi.b[0];
	m_data[2] = btoi.b[1];
	m_data[3] = func;
	m_ifp->write( m_data, 4, m_address);
}


///////////////////////////
// InfentoMotion members //
///////////////////////////

// INFENTOMOTION
// Class constructor
InfentoMotion::InfentoMotion()
{
	m_sensor.pir = 1;
	m_devname = "Motion";
	m_child = "InfentoMotion";
}

// OBJECTAHEAD
// Gives back if there is an moving object ahead.
// - returns: (TRUE) if there is an object ahead or (FALSE) not.
bool InfentoMotion::objectAhead()
{
	bool ret = false;

	if ( m_ifp ) {
		m_data[0] = CMD_PIR_READ;
		m_ifp->write( m_data, 1, m_address);
		m_ifp->read( m_data, 1, m_address);
		ret = (m_data[0] ? true : false);
	}

	if ( INF_DEBUG_DEVICE )
		debugroutine( m_child + "::objectAhead", "returns: " + String( ret));

	return ret;
}

// SIGNALOBJECTAHEAD
// Requests the sensor board to signal if there is an object ahead.
// - func: CONST_REACHED = signal when an object starts moving.
//         CONST_RELEASED = signal when an object stops moving.
// Be aware that a PIR sensor needs a few seconds to notice when an object stops moving.
void InfentoMotion::signalObjectAhead( InfentoConstant func)
{
	if ( INF_DEBUG_DEVICE )
		debugroutine( m_child + "::signalObjectAhead", m_ifp ? "function: " + String( func) : NOPORT);

	if ( !m_ifp ) return;

	clearSignal();

	m_data[0] = CMD_PIR_SIGNAL;
    m_data[1] = func;
	m_ifp->write( m_data, 2, m_address);
}


///////////////////////////////
// InfentoLineFollow members //
///////////////////////////////

// INFENTOLINEFOLLOW
// Class constructor.
// The line follower consists of five ir-sensors.
InfentoLineFollow::InfentoLineFollow()
{
	m_sensor.ir = 1;
	m_devname = "Line follower";
	m_child = "InfentoLineFollow";
}

// SETLUMINANCE
// Determines the line and floor luminance in order to get the best reponse.
// The response of the line follower depends on the light conditions in a place.
// - line: luminance percentage of the line (0% is dark, 100% is bright).
// - floor: luminance percentage of the floor (0% is dark, 100% is bright).
void InfentoLineFollow::setLuminance( int line, int floor)
{
	if ( INF_DEBUG_DEVICE ) {
		String name = m_child + "::setLuminance";
		if ( m_ifp )
			debugroutine( name, "line: " + String(line), "floor: " + String( floor));
		else
			debugroutine( name, NOPORT);
	}

	if ( !m_ifp ) return;
	m_data[0] = CMD_IR_SETVAL;
	m_data[1] = line;
	m_data[2] = floor;
	m_ifp->write( m_data, 3, m_address);
}

// POSITION
// Gives the bit position of the ir-sensors that got a reponse.
// Each ir-sensor represents a bit position. The bit position is masked by CONST_FARLEFT,
// CONST_LEFT, CONST_CENTRE, CONST_RIGHT and CONST_FARRIGHT.
InfentoConstant InfentoLineFollow::position()
{
	InfentoConstant ret = CONST_NONE;

	if ( m_ifp ) {
		m_data[0] = CMD_IR_READ;
		m_ifp->write( m_data, 1, m_address);
		m_ifp->read( m_data, 1, m_address);
		ret = (InfentoConstant) m_data[0];
	}

	if ( INF_DEBUG_DEVICE )
		debugroutine( m_child + "::position", "returns: " + String( ret, BIN));

	return ret;
}

// SIGNALPOSITION
// Requests the sensor board to signal a change of reponse of the ir-sensors.
// Each ir-sensor represents a bit position. The bit position is masked by CONST_FARLEFT,
// CONST_LEFT, CONST_CENTRE, CONST_RIGHT and CONST_FARRIGHT.
// - mask: masks the bit position of the ir-sensors involved.
// - func: CONST_REACHED = signal when one of the masked ir-sensors got a response
//         CONST_RELEASED = signal when one of the masked ir-sensors no longer has a response
void InfentoLineFollow::signalPosition( InfentoConstant mask, InfentoConstant func)
{
	if ( INF_DEBUG_DEVICE ) {
		String name = m_child + "::signalPosition";
		if ( m_ifp )
			debugroutine( name, "mask: " + String( mask, BIN), "function: " + String( func));
		else
			debugroutine( name, NOPORT);
	}

	if ( !m_ifp ) return;

	clearSignal();

	m_data[0] = CMD_IR_SIGNAL;
	m_data[1] = mask;
	m_data[2] = (byte) func;
	m_ifp->write( m_data, 3, m_address);
}


////////////////////////
// InfentoTwi members //
////////////////////////

InfentoTwi::InfentoTwi( int address)
{
	m_child = "InfentoTwi";
	m_sensor.twi = 1;
	m_devname = "I2c device";
	m_address = address;
}

// SETTIMER
// Determines the time between each measurement.
// - time: time in 0.1 sec between the measurements.
void InfentoTwi::setTimer( int time)
{
	if ( INF_DEBUG_DEVICE )
		debugroutine( m_child + "::setTimer", m_ifp ? "time: " + String( time) : NOPORT);

	if ( !m_ifp ) return;

	m_data[0] = CMD_TWI_SETVAL;
	m_data[1] = m_address;
	m_data[2] = time;
	m_ifp->write( m_data, 3, m_address);
}

// WRITE
// Writes a value from to twi sensor
// - command: command bytes to send to the twi sensor
// - size: size of the command array
void InfentoTwi::write( byte command[], int size)
{
	String cmd;
	if ( m_ifp ) {
		m_data[0] = CMD_TWI_WRITE;
		m_data[1] = m_address;
		m_data[2] = size;
		for ( int i = 0; i < size; i++ ) {
			m_data[3+i] = command[i];
			cmd = cmd + " " + String( command[i], HEX);
		}
		m_ifp->write( m_data, 3+size, m_address);
	}

	if ( INF_DEBUG_DEVICE )
		debugroutine( m_child + "::write", m_ifp ? "sends to address 0x" + String( m_address, HEX) + ": " + cmd : NOPORT);
}

// READ
// Reads a value from the twi sensor
// - command: command bytes to send to the twi sensor
// - size: size of the command array
// - value: value array to receive from the twi sensor
// - count: number of bytes to receive from the twi sensor
void InfentoTwi::read( byte command[], int size, byte value[], int count)
{
	String cmd, val;
	int i;
	if ( m_ifp ) {
		m_data[0] = CMD_TWI_READ;
		m_data[1] = m_address;
		m_data[2] = count;
		m_data[3] = size;
		for ( i = 0; i < size; i++ ) {
			m_data[4+i] = command[i];
			cmd = cmd + " " + String( command[i], HEX);
		}
		m_ifp->write( m_data, 4+size, m_address);
		m_ifp->read( m_data, count, m_address);
		for ( i = 0; i < count; i++ ) {
			value[i] = m_data[i];
			val = val + " " + String( value[i], HEX);
		}
	}

	if ( INF_DEBUG_DEVICE ) {
		String name = m_child + "::read";
		if ( m_ifp )
			debugroutine( name, "request on address 0x" + String( m_address, HEX) + ": " + cmd, "returns: " + val);
		else
			debugroutine( name, NOPORT);
	}
}

// SIGNALVALUE
// Signals a value from the twi sensor
// - command: command bytes to send to the twi sensor to request the value
// - size: size of the command array
// - value: value array to check against the received value from the twi sensor
// - count: number of bytes in the value array and also to receive from the twi sensor
// - datatype: CONST_BYTE = single byte
//             CONST_HLINT = integer in hi-/lo-byte order
//             CONST_LHINT = integer in lo-/hi-byte order
//             CONST_HLLONG = long in hi-/lo-byte order
//             CONST_LHLONG = long in lo-/hi-byte order
//             CONST_HLARRAY = array of bytes in hi-/lo-byte order
//             CONST_LHARRAY = array of bytes in lo-/hi-byte order
// - func: CONST_EQUAL = signal when 'value' equals the received value of the twi sensor
// -       CONST_NOTEQUAL = signal when 'value' is not equal to the received value of the twi sensor
// -       CONST_LESS = signal when 'value' is less then the received value of the twi sensor
// -       CONST_GREATER = signal when 'value' is greater then the received value of the twi sensor
void InfentoTwi::signalValue( byte command[], int size, byte value[], int count, InfentoConstant datatype, InfentoConstant func)
{
	String cmd, val;
	int i;

	if ( m_ifp ) {
		clearSignal();

		m_data[0] = CMD_TWI_SIGNAL;
		m_data[1] = m_address;
		m_data[2] = func;
		m_data[3] = datatype;
		m_data[4] = count;
		for ( i = 0; i < count; i++ ) {
			m_data[5+i] = value[i];
			val = val + " " + String( value[i], HEX);
		}
		m_data[4+count] = size;
		for ( i = 0; i < size; i++ ) {
			m_data[6+count+i] = value[i];
			cmd = cmd + " " + String( command[i], HEX);
		}
		m_ifp->write( m_data, 6+size+count, m_address);
	}

	if ( INF_DEBUG_DEVICE ) {
		String name = m_child + "::signalValue";
		if ( m_ifp )
			debugroutine( name, "request on address 0x" + String( m_address, HEX) + ": " + cmd, "value: " + val, "function: " + String( func) + ", type: " + String( datatype));
		else
			debugroutine( name, NOPORT);
	}
}


/*
////////////////////////////
// InfentoCompass members //
////////////////////////////

InfentoCompass::InfentoCompass()
{
	m_sensor.comp = 1;
	m_devname = "Compass";
	m_child = "InfentoCompass";
}

void InfentoCompass::setPrecision( int precision)
{
	if ( INF_DEBUG_DEVICE )
		debugroutine( m_child + "::setPrecision", m_ifp ? "precision: " + String( precision) : NOPORT);

}

int InfentoCompass::azimuth()
{
	BTOI btoi;
	btoi.i = 0;

	if ( m_ifp ) {
		m_data[0] = CMD_COMP_READ;
		m_data[1] = 1;
		m_ifp->write( m_data, 2, m_address);
		m_ifp->read( m_data, 2, m_address);
		btoi.b[0] = m_data[0];
		btoi.b[1] = m_data[1];
	}

	if ( INF_DEBUG_DEVICE )
		debugroutine( m_child + "::azimuth", "returns: " + String( btoi.i));

	return btoi.i;
}

InfentoConstant InfentoCompass::orientation()
{
	InfentoConstant ic = CONST_NONE;
	BTOI btoi;
	btoi.i = 0;

	if ( m_ifp ) {
		m_data[0] = CMD_COMP_READ;
		m_data[1] = 2;
		m_ifp->write( m_data, 1, m_address);
		m_ifp->read( m_data, 2, m_address);
		btoi.b[0] = m_data[0];
		btoi.b[1] = m_data[1];
	}

	if ( INF_DEBUG_DEVICE )
		debugroutine( m_child + "::orientation", "returns: " + String( btoi.i));

	return (InfentoConstant) btoi.i;
}

void InfentoCompass::signalAzimuth( int azimuth, InfentoConstant func)
{
	if ( INF_DEBUG_DEVICE ) {
		String name = m_child + "::signalAzimuth";
		if ( m_ifp )
			debugroutine( name, "azimuth: " + String( azimuth), "function: " + String( func));
		else
			debugroutine( name, NOPORT);
	}

	if ( !m_ifp ) return;

	clearSignal();

	m_data[0] = CMD_COMP_SIGNAL;
	m_data[1] = 1;
    m_data[2] = func;
	m_ifp->write( m_data, 3, m_address);
}

void InfentoCompass::signalOrientation( InfentoConstant orientation, InfentoConstant func)
{
	if ( INF_DEBUG_DEVICE ) {
		String name = m_child + "::signalOrientation";
		if ( m_ifp )
			debugroutine( name, "orientation: " + String( orientation), "function: " + String( func));
		else
			debugroutine( name, NOPORT);
	}

	if ( !m_ifp ) return;

	clearSignal();

	m_data[0] = CMD_COMP_SIGNAL;
	m_data[1] = 2;
    m_data[2] = func;
	m_ifp->write( m_data, 3, m_address);
}


/////////////////////////
// InfentoData members //
/////////////////////////

InfentoData::InfentoData()
{
	m_child = "InfentoData";
	m_sensor.dat = 1;
	m_devname = "Data device";
}

void InfentoData::write( byte value, byte mask)
{
	if ( m_ifp ) {
		m_data[0] = CMD_DAT_WRITE;
		m_data[1] = mask;
		m_data[2] = value;
		m_ifp->write( m_data, 3, m_address);
	}

	if ( INF_DEBUG_DEVICE )
		debugroutine( m_child + "::write", m_ifp ? "mask: " + String( mask, BIN) : NOPORT, 
										   m_ifp ? "value: " + String( value, BIN) : "");
}

byte InfentoData::read( byte mask)
{
	byte ret = 0;
	if ( m_ifp ) {
		m_data[0] = CMD_DAT_READ;
		m_data[1] = mask;
		m_ifp->write( m_data, 2, m_address);
		m_ifp->read( m_data, 1, m_address);
		ret = m_data[0];
	}

	if ( INF_DEBUG_DEVICE )
		debugroutine( m_child + "::read", m_ifp ? "mask: " + String( mask, BIN) : NOPORT, 
										  m_ifp ? "returns: " + String( ret, BIN) : "");

	return ret;
}

void InfentoData::signalValue( byte value, byte mask)
{
	if ( INF_DEBUG_DEVICE )
		debugroutine( m_child + "::signalValue", m_ifp ? "mask: " + String( mask, BIN) : NOPORT, 
												 m_ifp ? "value: " + String( value, BIN) : "");

	if ( !m_ifp ) return;

	clearSignal();

	m_data[0] = CMD_DAT_SIGNAL;
	m_data[1] = mask;
	m_data[2] = value;
	m_ifp->write( m_data, 3, m_address);
}
*/