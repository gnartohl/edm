//  edm - extensible display manager

//  Copyright (C) 1999 John W. Sinclair

//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.

//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.

//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#ifndef __pvColor_h
#define __pvColor_h 1

#include "color_pkg.h"

#ifdef __epics__
#include "alarm.h"
#endif

#ifndef __epics__
#define NO_ALARM                0x0
#define MINOR_ALARM             0x1
#define MAJOR_ALARM             0x2
#define INVALID_ALARM           0x3
#endif

class pvColorClass {

private:

int effectivePixel;
int pixel;
int staticPixel;
int disconnectedPixel;
int noalarmPixel;
int invalidPixel;
int minorPixel;
int majorPixel;
int saveValPixel; // if value is not equal to saved value

int null, alarmed;

int alarmSensitive;
int connectSensitive;

int connected;
short status;
short severity;

public:

pvColorClass::pvColorClass( void );

void pvColorClass::copy( const pvColorClass &source );

void pvColorClass::setStatus (
  short stat,
  short sev );

void pvColorClass::setConnected ( void );

void pvColorClass::setDisconnected ( void );

unsigned int pvColorClass::getColor( void ) const;

unsigned int pvColorClass::pixelColor( void ) const;

unsigned int pvColorClass::disconnectColor( void );

void pvColorClass::changeColor (
  unsigned int color,
  colorInfoClass *ci );

void pvColorClass::setColor (
  unsigned int color,
  colorInfoClass *ci );

void pvColorClass::setAlarmSensitive ( void );

void pvColorClass::setAlarmInsensitive ( void );

void pvColorClass::setConnectSensitive ( void );

void pvColorClass::setConnectInsensitive ( void );

void pvColorClass::setNull ( void );

void pvColorClass::setNotNull ( void );

void pvColorClass::setNullColor (
  unsigned int color );

unsigned int pvColorClass::nullColor ( void );

};

#endif




