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


// DEVICE BOARD ATTENTION MASKS

typedef union 
{
	uint16_t all;
	uint8_t  b[2];
	struct {
		unsigned dist : 1;
		unsigned pir : 1;
		unsigned ir : 1;
		unsigned comp : 1;
		unsigned mel : 1;
		unsigned mic : 1;
		unsigned lum : 1;
		unsigned _b7 : 1;
		unsigned swp : 1;
		unsigned gt1 : 1;
		unsigned gt2 : 1;
		unsigned res : 1;
		unsigned _b12 : 1;
		unsigned _b13 : 1;
		unsigned twi : 1;
		unsigned dat : 1;
	};
} sensor_dev_t;

typedef union
{
	uint8_t dev[16];
	struct {
		uint8_t dist;
		uint8_t pir;
		uint8_t ir;
		uint8_t comp;
		uint8_t mel;
		uint8_t mic;
		uint8_t lum;
		uint8_t _b7;
		uint8_t swp;
		uint8_t gt1;
		uint8_t gt2;
		uint8_t res;
		uint8_t _b12;
		uint8_t _b13;
		uint8_t twi;
		uint8_t dat;
	};
} sensor_dev_vers_t;

typedef union 
{
	uint16_t all;
	uint8_t  b[2];
	struct {
		unsigned lpwm1 : 1;
		unsigned lpwm2 : 1;
		unsigned md1 : 1;
		unsigned md2 : 1;
		unsigned md3 : 1;
		unsigned rot : 1;
		unsigned step : 1;
		unsigned _b7 : 1;
		unsigned overTemp : 1; // motor driver heat sink
	};
} actuator_dev_t;

typedef union 
{
	uint8_t  dev[16];
	struct {
		uint8_t lpwm1;
		uint8_t lpwm2;
		uint8_t md1;
		uint8_t md2;
		uint8_t md3;
		uint8_t rot;
    uint8_t step;
	};
} actuator_dev_vers_t;

typedef enum {
  CMD_NONE              = 0x00,
  CMD_RESET             = 0x02,

//-- ActuatorIO commands  

// WARNING: do not renumber!!

  CMD_MD1_MOVE          = 0x10, // 1 = move in, 2 = move out, 3 = move to, 4 = distance, 5 = arc, 6 = time
  CMD_MD1_READ          = 0x11, // 1 = read distance, 2 = read rotation, 3 = rotary ticks
  CMD_MD1_ACTHOME       = 0x12, // 0 = go home, 1 = set home
  CMD_MD1_ONOFF         = 0x16, // 0 = off, 1..255 = speed (on)
  CMD_MD1_USEROT        = 0x19, // none
  CMD_MD1_SETSTAT       = 0x1A, // none
  CMD_MD1_SETUNIT       = 0x1B, // 1 = velocity, 2 = distance, 3 = arc, 4 = time
  CMD_MD1_SETVAL        = 0x1C, // 1 = velocity, 2 = direction, 3 = acceleration, 4 = min. lin. act speed (%)

  CMD_MD2_MOVE          = 0x20, // 1 = move in, 2 = move out, 3 = move to, 4 = distance, 5 = arc, 6 = time
  CMD_MD2_READ          = 0x21, // 1 = read distance, 2 = read rotation, 3 = rotary ticks
  CMD_MD2_ACTHOME       = 0x22, // 0 = go home, 1 = set home
  CMD_MD2_ONOFF         = 0x26, // 0 = off, 1..255 = speed (on)
  CMD_MD2_USEROT        = 0x29, // none
  CMD_MD2_SETSTAT       = 0x2A, // none
  CMD_MD2_SETUNIT       = 0x2B, // 1 = velocity, 2 = distance, 3 = arc, 4 = time
  CMD_MD2_SETVAL        = 0x2C, // 1 = velocity, 2 = direction, 3 = acceleration

  CMD_MD3_MOVE          = 0x30, // 4 = distance, 5 = arc, 6 = time
  CMD_MD3_READ          = 0x31, // 1 = read distance, 2 = read rotation, 3 = rotary ticks
  CMD_MD3_DIR           = 0x33, // none
  CMD_MD3_ONOFF         = 0x36, // 0 = off, 1..255 = speed (on)
  CMD_MD3_USEROT        = 0x39, // none
  CMD_MD3_SETSTAT       = 0x3A, // none
  CMD_MD3_SETUNIT       = 0x3B, // 1 = velocity, 2 = distance, 3 = arc, 4 = time
  CMD_MD3_SETVAL        = 0x3C, // 1 = velocity, 2 = direction, 3 = acceleration
  CMD_MD3_GEAR          = 0x3E, // none

  CMD_ROT_SIGNAL        = 0x40, // 1 = distance, 2 = arc
  CMD_ROT_READ          = 0x41, // 1 = distance, 2 = arc
  CMD_ROT_CLEAR         = 0x44, // none
  CMD_ROT_SETUNIT       = 0x4B, // 2 = distance, 3 = arc

  CMD_LPWM1_SETFUNC     = 0x50, // 1 = on, 2 = blink, 3 = slide, 4 = off
  CMD_LPWM1_SETVAL      = 0x5C, // none
  
  CMD_LPWM2_SETFUNC     = 0x60, // 1 = on, 2 = blink, 3 = slide, 4 = off
  CMD_LPWM2_SETVAL      = 0x6C, // none

  CMD_MDS_MOVE          = 0x70, // 1 = move in, 2 = move out, 4 = distance, 5 = arc, 6 = time
  CMD_MDS_READ          = 0x71, // 1 = read distance, 2 = read rotation, 3 = steps
  CMD_MDS_ACTHOME       = 0x72, // 0 = go home, 1 = set home
  CMD_MDS_ONOFF         = 0x76, // 0 = halt, 1 = off, >2= on
  CMD_MDS_SETUNIT       = 0x7B, // 1 = velocity, 2 = distance, 3 = arc, 4 = time
  CMD_MDS_SETVAL        = 0x7C, // 2 = direction

  CMD_SERVO1_MOVE       = 0x80, // 1 = move in, 2 = move out, 3 = move to
  CMD_SERVO1_ACTHOME    = 0x82, // 0 = go home, 1 = set home

  CMD_SERVO2_MOVE       = 0x90, // 1 = move in, 2 = move out, 3 = move to
  CMD_SERVO2_ACTHOME    = 0x92, // 0 = go home, 1 = set home

  //-- SensorIO commands  

// WARNING: do not renumber!!

  CMD_DIST_SIGNAL       = 0x10, // none
  CMD_DIST_READ         = 0x11, // none
  CMD_DIST_SETUNIT      = 0x1B, // none
  CMD_DIST_SETVAL       = 0x1C, // 1 = signal range, 2 = timer

  CMD_PIR_SIGNAL        = 0x20, // none
  CMD_PIR_READ          = 0x21, // none

  CMD_IR_SIGNAL         = 0x30, // none
  CMD_IR_READ           = 0x31, // none
  CMD_IR_SETVAL         = 0x3C, // first byte = line, second byte = floor (in % from max.)

  CMD_COMP_SIGNAL       = 0x40, // 1 = azimuth, 2 = orientation
  CMD_COMP_READ         = 0x41, // 1 = azimuth, 2 = orientation
  CMD_COMP_SETVAL       = 0x4C, // none

  CMD_MEL_PLAY          = 0x50, // none
  CMD_MEL_STOP          = 0x56, // none
  CMD_MEL_SETVAL        = 0x5C, // none

  CMD_MIC_SIGNAL        = 0x60, // none
  CMD_MIC_READ          = 0x61, // none
  CMD_MIC_SETVAL        = 0x6C, // 1 = signal range, 2 = timer

  CMD_LUM_SIGNAL        = 0x70, // none
  CMD_LUM_READ          = 0x71, // none
  CMD_LUM_SETUNIT       = 0x7B, // none
  CMD_LUM_SETVAL        = 0x7C, // 1 = signal range, 2 = timer

  CMD_SWP_SIGNAL        = 0x80, // none
  CMD_SWP_READ          = 0x81, // none
  CMD_SWP_SETVAL		= 0x8C, // 1 = normal open, 2 = normal closed

  CMD_GT1_SIGNAL        = 0x90, // none
  CMD_GT1_READ          = 0x91, // none

  CMD_GT2_SIGNAL        = 0xA0, // none
  CMD_GT2_READ          = 0xA1, // none

  CMD_RES_SIGNAL        = 0xB0, // none
  CMD_RES_READ          = 0xB1, // none
  CMD_RES_SETREF        = 0xBB, // none
  CMD_RES_SETVAL        = 0xBC, // 1 = signal range, 2 = timer

  CMD_TWI_SIGNAL        = 0xC0, // none
  CMD_TWI_READ          = 0xC1, // none
  CMD_TWI_WRITE         = 0xC2, // none
  CMD_TWI_SETVAL        = 0xCC, // none

  CMD_DAT_SIGNAL        = 0xD0, // none
  CMD_DAT_READ          = 0xD1, // none
  CMD_DAT_WRITE         = 0xD2, // none

//-- System commands

  CMD_SET_SLAVE_ADDR    = 0xE0,
  CMD_WR_EEPROM         = 0xE1,
  CMD_RD_STATUS         = 0xE2,
  CMD_RD_EEPROM         = 0xE3,

  CMD_RD_ATTNSTS        = 0xF1,
  CMD_RD_EMGSTS         = 0xF2,
  CMD_RD_TEMPSTS        = 0xF3,

  CMD_RD_DEVVERSION     = 0xFC, // read device version
  CMD_RD_DEVMASK        = 0xFD, // read board devices
  CMD_RD_VERSION        = 0xFE,
  CMD_GENCALL           = 0xFF,
} InfentoCommand;

 typedef enum {
  CONST_UNDEFINED       = 0,

  CONST_DIGITAL         = 1,        // controller port types
  CONST_ANALOGUE        = 2,
  CONST_NXT             = 3,

  CONST_LAMP1           = 0x01,     // actuator ports
  CONST_LAMP2           = 0x02,
  CONST_MOTOR1          = 0x01,
  CONST_MOTOR2          = 0x02,
  CONST_EDRIVE          = 0x03,
  CONST_LINACT1         = 0x01,
  CONST_LINACT2         = 0x02,
  CONST_GATE1           = 0x01,
  CONST_GATE2           = 0x02,
} InfentoDefinition;

typedef enum {
  UNIT_PERC             = 0x01,
  UNIT_MSEC             = 0x02,
  UNIT_SEC              = 0x03,
  UNIT_MIN              = 0x04,
  UNIT_HOUR             = 0x05,
  UNIT_MTR              = 0x06,
  UNIT_CM               = 0x07,
  UNIT_MM               = 0x08,
  UNIT_INCH             = 0x09,
  UNIT_MPS              = 0x0A,
  UNIT_KPH              = 0x0B,
  UNIT_MPH              = 0x0C,
  UNIT_RPM              = 0x0D,
  UNIT_DEG              = 0x0E,
  UNIT_GRAD             = 0x0F,
  UNIT_TICK             = 0x10,
  UNIT_DB               = 0x11,
  UNIT_LUX              = 0x12,
} InfentoUnit;

typedef enum {
  CONST_FORWARD         = 0x01,     // motor
  CONST_REVERSE         = 0x02,

  CONST_DARK            = 0x01,     // linefollower
  CONST_LIGHT           = 0x02,

  CONST_FARLEFT         = 0x01,     // linefollower
  CONST_LEFT            = 0x02,
  CONST_CENTRE          = 0x04,
  CONST_RIGHT           = 0x08,
  CONST_FARRIGHT        = 0x10,

  CONST_SWITCH_1        = 0x01,     // switchpack
  CONST_SWITCH_2        = 0x02,
  CONST_SWITCH_3        = 0x04,
  CONST_SWITCH_4        = 0x08,
  CONST_SWITCH_5        = 0x10,
  CONST_SWITCH_6        = 0x20,
  CONST_SWITCH_7        = 0x40,
  CONST_SWITCH_8        = 0x80,

  CONST_ONOPEN          = 0x01,
  CONST_ONCLOSE         = 0x02,

  CONST_NONE            = 0x00,
  CONST_ALL             = 0xFF,
  CONST_FULL			= 0x00,

  CONST_REACHED         = 0x01,     // attention type
  CONST_RELEASED        = 0x02,
  CONST_CLOSER          = 0x04,
  CONST_FURTHER         = 0x08,
  CONST_LOWER           = 0x04,
  CONST_HIGHER          = 0x08,
  CONST_CHANGED         = 0x10,

  CONST_STEADY          = 0x00,     // direction
  CONST_APPROACH        = 0x01,
  CONST_LEAVE           = 0x02,

  CONST_N               = 1,        // compass
  CONST_NE              = 2,
  CONST_E               = 3,
  CONST_SE              = 4,
  CONST_S               = 5,
  CONST_SW              = 6,
  CONST_W               = 7,
  CONST_NW              = 8,
  
  CONST_EQUAL           = 0,        // twi
  CONST_NOTEQUAL        = 1,
  CONST_LESS            = 2,
  CONST_GREATER         = 3,
  
  CONST_BYTE            = 0,
  CONST_HLINT           = 1,
  CONST_LHINT           = 2,
  CONST_HLLONG          = 3,
  CONST_LHLONG          = 4,
  CONST_HLARRAY         = 5,
  CONST_LHARRAY         = 6,
} InfentoConstant;
