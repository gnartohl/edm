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
#include "pv_factory.h" // alarm definitions

class pvColorClass {

private:

int effectiveIndex;
int index;
int staticIndex;
int disconnectedIndex;
int noalarmIndex;
int invalidIndex;
int minorIndex;
int majorIndex;
int saveValIndex; // if value is not equal to saved value
int ruleIndex;

int effectivePixel;
int pixel;
int staticPixel;
int disconnectedPixel;
int noalarmPixel;
int invalidPixel;
int minorPixel;
int majorPixel;
int saveValPixel; // if value is not equal to saved value
int rulePixel;

int null, alarmed, ruleMode;

int alarmSensitive;
int connectSensitive;

int connected;
short status;
short severity;

public:

pvColorClass( void );

void copy( const pvColorClass &source );

void setStatus (
  short stat,
  short sev );

void setConnected ( void );

void setDisconnected ( void );

unsigned int getColor( void ) const;

int getDisconnectedIndex( void ) const;

unsigned int getDisconnected( void ) const;

int getIndex ( void ) const;

unsigned int pixelColor( void ) const;

int pixelIndex ( void ) const;

void _changeColor (
  unsigned int color,
  colorInfoClass *ci );

void changeColor (
  unsigned int color,
  colorInfoClass *ci );

void changeIndex (
  int color,
  colorInfoClass *ci );

void setColor (
  unsigned int color,
  colorInfoClass *ci );

void setColorIndex (
  int color,
  colorInfoClass *ci );

void setAlarmSensitive ( void );

void setAlarmInsensitive ( void );

void setConnectSensitive ( void );

void setConnectInsensitive ( void );

void setNull ( void );

void setNotNull ( void );

void setNullColor (
  unsigned int color );

void setNullIndex (
  int color,
  colorInfoClass *ci );

unsigned int nullColor ( void );

int nullIndex ( void );

void setRuleMode ( void );

void setNotRuleMode ( void );

void setRuleColor (
  unsigned int color,
  colorInfoClass *ci );

void setRuleIndex (
  int color,
  colorInfoClass *ci );

int getStatus ( void );

int getSeverity ( void );

};

#endif
