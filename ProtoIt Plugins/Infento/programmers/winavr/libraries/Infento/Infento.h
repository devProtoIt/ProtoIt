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

#ifndef __INFENTO_CONTROLLER__
#define __INFENTO_CONTROLLER__

#include <Arduino.h>
#include "InfentoCommands.h"


// DEBUG SUPPORT - OUTPUT TO USB MONITOR

// Debug at i2c bus level
//#define INF_DEBUG_SWITCH

// Debug io board eeprom
//#define INF_DEBUG_EEPROM

// Debug emergency and attention lines
//#define INF_DEBUG_EMERGENCY
//#define INF_DEBUG_ATTENTION

// Debug reading from and writing to InfentoPort
//#define INF_DEBUG_PORTIO

// Debug the complete InfentoPort operations
//#define INF_DEBUG_PORT

// Debug reading from and writing to InfentoDevice
//#define INF_DEBUG_DEVICEIO



// Debug at device routine level (input parameters and return values)
// Meant to be used in *.ino files
extern bool INF_DEBUG_DEVICE;
#define INF_DEBUG_ON	INF_DEBUG_DEVICE = true;
#define INF_DEBUG_OFF	INF_DEBUG_DEVICE = false;

extern bool INF_WAITATSTART;
#define INF_WAIT_OFF	INF_WAITATSTART = false;

// Port lines

#define IFP_LINES	4		// number of lines per port
#define LINE1		0
#define LINE2		1
#define LINE3		2
#define LINE4		3

// Port counts

#define IFP_PORTS 		9	// total number of ports: IFP_DIGIPORTS + IFP_ANAPORTS + IFP_NXTPORTS
#define IFP_DIGIPORTS	4	// number of digital ports
#define IFP_ANAPORTS	4	// number of analogue ports
#define IFP_NXTPORTS	1	// number of nxt ports

#define MAXBOARDS		5	// maximum number of boards connected to a port

// Port Modes

#define IFM_I2C			0	// an I2C device is connected
#define IFM_UART		1	// the device communicates via Serial1
#define IFM_REF			2	// a device using the voltage divider is connected
#define IFM_DATA		3	// the device is routed to the arduino pins directly


// Ports

#define DP1			IFP[0]		// digital ports
#define DP2			IFP[1]
#define DP3			IFP[2]
#define DP4			IFP[3]

#define AP1			IFP[4]		// analogue ports
#define AP2			IFP[5]
#define AP3			IFP[6]
#define AP4			IFP[7]

#define NP1			IFP[8]		// lego mindstorms port


// Port lines

#define IRQ1		PIN[1]		// interrupt 5 of the Arduino
#define IRQ2		PIN[5]		// interrupt 4 of the Arduino

#define DP1L1		PIN[0]		// digital lines on port 0
#define DP1L2		PIN[1]
#define DP1L3		PIN[2]
#define DP1L4		PIN[3]

#define DP2L1		PIN[4]		// digital lines on port 1
#define DP2L2		PIN[5]
#define DP2L3		PIN[6]
#define DP2L4		PIN[7]

#define DP3L1		PIN[8]		// digital lines on port 2
#define DP3L2		PIN[9]
#define DP3L3		PIN[10]
#define DP3L4		PIN[11]

#define DP4L1		PIN[12]		// digital lines on port 3
#define DP4L2		PIN[13]
#define DP4L3		PIN[14]
#define DP4L4		PIN[15]

#define AP1L1		PIN[16]		// analogue lines on port 4
#define AP1L2		PIN[17]
#define AP1L3		PIN[18]
#define AP1L4		PIN[19]

#define AP2L1		PIN[20]		// analogue lines on port 5
#define AP2L2		PIN[21]
#define AP2L3		PIN[22]
#define AP2L4		PIN[23]

#define AP3L1		PIN[24]		// analogue lines on port 6
#define AP3L2		PIN[25]
#define AP3L3		PIN[26]
#define AP3L4		PIN[27]

#define AP4L1		PIN[28]		// analogue lines on port 7
#define AP4L2		PIN[29]
#define AP4L3		PIN[30]
#define AP4L4		PIN[31]

#define NP1L1		PIN[32]		// analogue lines on port 8
#define NP1L2		PIN[33]
#define NP1L3		PIN[34]
#define NP1L4		PIN[35]
 
// Vernier Sensor Parameters

struct VernierParam {
	String	devname;	// sensor name
	float	intercept;	// calibration intercept
	float	slope;		// calibration slope
};


// Infento Port Class

class InfentoPort
{
public:
	
	InfentoPort();

	static void begin( int baud = 9600);

	static void haltOnStop();

	void setPortMode( int portmode);
	static InfentoPort* findDevicePort( int i2c_address);
	void resetBoard( int i2c_address);
	static void haltOnIncomplete();

	// attention and emergency handling

	static bool attention();
	static void waitAttention();
	bool attention( int i2c_address, uint16_t devicemask);
	static void emergency();

	// Extra device information

	int infentoBoard( int i2c_address);
	VernierParam vernierParameters( int i2c_address);
	float toVernier( int analoguereading, VernierParam& vp);

	// Convenience calls

	InfentoDefinition portType();				// returns CONST_DIGITAL, CONST_ANALOGUE or CONST_NXT
	static String portLabel( int ix);			// transforms a port index into a portlabel
	static InfentoPort* port( String label);	// gives the instance of the class corresponding to label
												// gives NULL if a wrong label was specified
	int port();									// gives portnumber

	// Port access

	void setLed( bool on);
	int pin( int line);
	void write( byte data[], int size, int i2c_address = 0);
	int read( byte data[], int size, int i2c_address = 0);

protected:

	void init();
	static void blinkHB( bool stop);
	void portSetup( int port);
	void aeLinesSetup();

	// next members are related to the Infento controllers hardware
	// do not modify the code that sets or reads these members

	int			m_port;					// port number
	int			m_portmode;				// port mode: one of the IFM modes
	int			m_bus;					// i2c bus of the port
	int			m_pinix;				// arduino pin number occupation, index to PIN[]
										//   PIN[m_pinix] is line 1, PIN[m_pinix+1] is line 2, etc.
	int			m_pinmode[IFP_LINES];	// arduino pin modes (default to INPUT)
	int			m_address[MAXBOARDS];	// i2c addresses of the connected Infento boards
	bool		m_reset[MAXBOARDS];		// records if a board has been reset
    int     	m_addrcnt;              // number of addresses in the array
	uint16_t	m_attention[MAXBOARDS];	// bit masks if the devices on the board that requested attention
};

extern InfentoPort IFP[IFP_PORTS];
extern int PIN[IFP_PORTS*IFP_LINES];

class InfentoDevice
{
public:

	InfentoDevice();

	InfentoPort* connect( int i2c_address);
	bool connect( InfentoPort& ifp, int portmode);

	bool signal();
	void waitSignal();

	void line( int linenumber, int set, bool pwm = false);
	int  line( int linenumber);

	void write();
	void read();

	void setValue( bool value);
	void setValue( int value);
	void setValue( long value);
	void setValue( float value, int decimalpos);
	void setValue( String value);

	void getValue( bool& value);
	void getValue( int& value);
	void getValue( long& value);
	void getValue( float& value, int decimalpos);
	void getValue( String& value);

	String version();
	void debugEeprom( int i2c_address, byte position);

protected:

	void clearSignal();

	InfentoPort*	m_ifp;
	int				m_address;	// i2c address of the devices board, -1 means no i2c connected
	actuator_dev_t	m_actuator; // one of the bits defines which actuator it is on the board
	sensor_dev_t	m_sensor;	// one of the bits defines which sensor it is on the board
	String			m_devname;	// device name (debug facility)
	String			m_child;	// child class name (debug facility)
	byte			m_data[11];	// value buffer
	byte			m_size;		// buffer size
};

// INFENTO ACTUATORS

class InfentoMelody : public InfentoDevice
{
public:
	InfentoMelody();
/*
	void calibrate( int maxwatt);
	void setVolume( int watt);
*/
	void playOnce( int tracknumber, int volume = 100);
	void playRepeat( int tracknumber, int volume = 100);
	void off();

protected:
	void play( int track, bool repeat, int volume);
};

class InfentoLamp : public InfentoDevice
{
public:
	InfentoLamp( InfentoDefinition lampnumber);

	InfentoDefinition lampNumber();

	void calibrate( int maxlumen);

	void setLuminosity( int luminosity);

	void on( int brightness = 100);
	void blink( int time_a = 10, int brightness_a = 100, int time_b = 10, int brightness_b = 0);
	void fade( int time_a = 10, int brightness_a = 100, int time_b = 10, int brightness_b = 0);
	void off( bool immediate = true);

protected:
	void set( byte func, int time_a, int brightness_a, int time_b, int brightness_b);
	int m_cmdoffs;
};

class InfentoDrive : public InfentoDevice
{
public:
	InfentoDrive();

	InfentoDefinition motorNumber();

	void setStartupPower( int power);

	long ticks();

protected:
	int		m_cmdoffs;

	void setUnit( byte func, InfentoUnit unit);
	void setValue( byte func, int value);
	void setOn( int speed);
	void setMove( byte func, int value, int speed);
};

class InfentoMotor : public InfentoDrive
{
public:
	InfentoMotor( InfentoDefinition motornumber);

	void calibrate( int maxmps, int maxrpm = 0);

	void setVelocityUnit( InfentoUnit unit);

	void setTimeUnit( InfentoUnit unit);
	void setDistanceUnit( InfentoUnit unit);
	void setArcUnit( InfentoUnit unit);

	void setDirection( InfentoConstant direction);
	void setVelocity( int velocity = 0);
	void setAcceleration( int acceleration = 0);
	void invertDirection( bool invert = true);

	void on( int speed = 100);
	void off( bool immediate = false);

	void run( int time, int speed = 100);
	void move( int distance, int speed = 100);
	void turn( int arc, int speed = 100);

	int  distance();
	int  arc();

	void attachRotary( int steps, int diameter = 0);
	
protected:
	bool m_inverted;
};

class InfentoLinAct : public InfentoDrive
{
public:
	InfentoLinAct( InfentoDefinition motornumber);

	void calibrate( int shaftlength, int endin = -1, int endout = -1);

	void setPositionUnit( InfentoUnit unit);

	void moveTo( int position, int speed = 100);
	void moveIn( int distance = CONST_FULL, int speed = 100);
	void moveOut( int distance = CONST_FULL, int speed = 100);
	void off( bool immediate = false);

	int position();

	void setHome();
	void goHome();

protected:
	void home( byte func);
};

// INFENTO SENSORS

class InfentoGate : public InfentoDevice
{
public:
	InfentoGate( InfentoDefinition gatenumber);

	InfentoDefinition gateNumber();

	bool open();
	bool closed();
	void signalOpening();
	void signalClosing();
	
protected:
	int m_cmdoffs;
};

#define InfentoSwitch InfentoGate

class InfentoSwitchPack : public InfentoDevice
{
public:
	InfentoSwitchPack();

	void setResponse( InfentoConstant response);

	bool isOn();
	bool isOff();

	InfentoConstant position();

	void signalPosition( InfentoConstant mask = CONST_ALL,
						 InfentoConstant func = CONST_REACHED);
};

class InfentoLevel : public InfentoDevice
{
public:
	InfentoLevel();

	void setTimer( int time);
	void setSignalRange( int range);
	int  level();
	void signalLevel( int level, InfentoConstant func = CONST_REACHED);

protected:
	int		m_cmdoffs;
};

class InfentoMicrophone : public InfentoLevel
{
public:
	InfentoMicrophone();
};

/*
class InfentoLuminance : public InfentoLevel
{
public:
	InfentoLuminance();
};
*/

class InfentoResistance : public InfentoLevel
{
public:
	InfentoResistance();
	void setReference( long resistance);
};

class InfentoRotary : public InfentoDevice
{
public:
	InfentoRotary();

	void calibrate( int steps, int diameter = 0);
	void setDistanceUnit( InfentoUnit unit);
	void setArcUnit( InfentoUnit unit);

	void clear();
	int  distance();
	int  arc();

	void signalDistance( int distance);
	void signalArc( int arc);
};

class InfentoDistance : public InfentoDevice
{
public:
	InfentoDistance();

	void setDistanceUnit( InfentoUnit unit);
	void setTimer( int time);

	int distance();
	int distance( InfentoConstant& direction);
	
	void setSignalRange( int range);
	void signalDistance( int distance, InfentoConstant func = CONST_REACHED);

protected:
	int m_distance;
	int m_count;
	InfentoConstant	m_direction;
};

class InfentoMotion : public InfentoDevice
{
public:
	InfentoMotion();

	bool objectAhead();
	void signalObjectAhead( InfentoConstant func = CONST_REACHED);
};

class InfentoLineFollow : public InfentoDevice
{
public:
	InfentoLineFollow();

	void setLuminance( int line, int floor);

	InfentoConstant position();

	void signalPosition( InfentoConstant mask = CONST_ALL,
						 InfentoConstant func = CONST_REACHED);
};

class InfentoTwi : public InfentoDevice
{
		// this class fits those i2c devices only, that return a single value
public:
	InfentoTwi( int address);
	
	void setTimer( int time);

	void write( byte command[], int size);
	void read( byte command[], int size, byte value[], int count);

	void signalValue( byte command[], int size, byte value[], int count = 2, InfentoConstant datatype = CONST_LHINT, InfentoConstant func = CONST_REACHED);	

protected:
	int m_address;
};

/*
class InfentoCompass : public InfentoDevice
{
public:
	InfentoCompass();

	void setPrecision( int precision);

	int  azimuth();													// the azimuth given in the required precision
	InfentoConstant orientation();								    // CONST_N, CONST_NE, CONST_E, CONST_SE, CONST_S, CONST_SW, CONST_W, CONST_NW
	void signalAzimuth( int azimuth, InfentoConstant func = CONST_REACHED);	// activates attention when azimuth in +/- 0.5 precision range
	void signalOrientation( InfentoConstant orientation,
							InfentoConstant func = CONST_REACHED);	// activates attention when azimuth in range: orientation +/- 12 deg
};

class InfentoData : public InfentoDevice
{
		// this class fits data line devices only
		// each data line represents a bit in the mask and the value
public:
	InfentoData();

	void write( byte value, byte mask = 0xFF);			// sets the bits of value for the masked bits
	byte read( byte mask = 0xFF);						// returns the value bits for the masked bits

	void signalValue( byte value, byte mask = 0xFF);	// signals if the value bits are equal for the masked bits
};
*/

#endif
