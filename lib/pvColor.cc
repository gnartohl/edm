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

#include "pvColor.h"

pvColorClass::pvColorClass( void )
{

  alarmSensitive = 0;
  connectSensitive = 0;
  pixel = 0;
  staticPixel = 0;
  effectivePixel = 0;
  disconnectedPixel = 0;
  noalarmPixel = 0;
  invalidPixel = 0;
  minorPixel = 0;
  majorPixel = 0;
  saveValPixel = 0;
  status = 0;
  severity = 0;
  null = 0;
  alarmed = 0;
  ruleMode = 0;

}

void pvColorClass::copy( const pvColorClass &source )
{

  pixel = source.pixel;
  staticPixel = source.staticPixel;
  effectivePixel = source.effectivePixel;
  disconnectedPixel = source.disconnectedPixel;
  noalarmPixel = source.noalarmPixel;
  invalidPixel = source.invalidPixel;
  minorPixel = source.minorPixel;
  majorPixel = source.majorPixel;
  saveValPixel = source.saveValPixel;
  alarmSensitive = source.alarmSensitive;
  connectSensitive = source.connectSensitive;
  connected = source.connected;
  status = source.status;
  severity = source.severity;
  null = 0;
  alarmed = 0;
  ruleMode = 0;

}

void pvColorClass::setStatus (
  short stat,
  short sev )
{

  status = stat;
  severity = sev;

  if ( !connected && connectSensitive ) {

    effectivePixel = disconnectedPixel;
    alarmed = 1;

  }
  else if ( alarmSensitive ) {

    switch ( severity ) {

    case INVALID_ALARM:
      effectivePixel = invalidPixel;
      alarmed = 1;
      break;

    case MINOR_ALARM:
      effectivePixel = minorPixel;
      alarmed = 1;
      break;

    case MAJOR_ALARM:
      effectivePixel = majorPixel;
      alarmed = 1;
      break;

    default:
      alarmed = 0;
      if ( noalarmPixel == -1 ) {
	if ( null ) {
          effectivePixel = saveValPixel;
	}
	else {
          effectivePixel = pixel;
	}
      }
      else {
	if ( null ) {
          effectivePixel = saveValPixel;
	}
	else {
          effectivePixel = noalarmPixel;
	}
      }
      break;

    }

  }
  else {

    alarmed = 0;
    if ( null ) {
      effectivePixel = saveValPixel;
    }
    else {
      effectivePixel = pixel;
    }

  }

}

unsigned int pvColorClass::pixelColor( void ) const
{

  return staticPixel;

}

unsigned int pvColorClass::getColor( void ) const
{

  return effectivePixel;

}

void pvColorClass::setNullColor (
  unsigned int color )
{

  saveValPixel = color;

}

unsigned int pvColorClass::nullColor ( void )
{

  return saveValPixel;

}

void pvColorClass::changeColor (
  unsigned int color,
  colorInfoClass *ci )
{

  pixel = color;
  saveValPixel = color;
  invalidPixel = (int) ci->getSpecialColor( COLORINFO_K_INVALID );
  minorPixel = (int) ci->getSpecialColor( COLORINFO_K_MINOR );
  majorPixel = (int) ci->getSpecialColor( COLORINFO_K_MAJOR );
  disconnectedPixel = (int) ci->getSpecialColor( COLORINFO_K_DISCONNECTED );
  noalarmPixel = (int) ci->getSpecialColor( COLORINFO_K_NOALARM );

  if ( !connected && connectSensitive ) {

    effectivePixel = disconnectedPixel;

  }
  else {

   if ( !alarmSensitive ) {

     if ( ruleMode ) {
       effectivePixel = rulePixel;
     }
     else {
       effectivePixel = pixel;
     }

    }
    else {

      switch ( severity ) {

      case INVALID_ALARM:
        effectivePixel = invalidPixel;
        break;

      case MINOR_ALARM:
        effectivePixel = minorPixel;
        break;

      case MAJOR_ALARM:
        effectivePixel = majorPixel;
        break;

      default:
        if ( noalarmPixel == -1 )
          if ( ruleMode ) {
            effectivePixel = rulePixel;
          }
          else {
            effectivePixel = pixel;
          }
        else
          effectivePixel = noalarmPixel;
        break;

      }

    }

  }

}

void pvColorClass::setColor (
  unsigned int color,
  colorInfoClass *ci )
{

  staticPixel = color;
  changeColor( color, ci );

}

void pvColorClass::setConnected ( void ) {

  connected = 1;
  this->setStatus( this->status, this->severity );

}

void pvColorClass::setDisconnected ( void ) {

  connected = 0;
  effectivePixel = disconnectedPixel;

}

void pvColorClass::setAlarmSensitive ( void ) {

  alarmSensitive = 1;

}

void pvColorClass::setAlarmInsensitive ( void ) {

  alarmSensitive = 0;

}

void pvColorClass::setConnectSensitive ( void ) {

  connectSensitive = 1;

}

void pvColorClass::setConnectInsensitive ( void ) {

  connectSensitive = 0;

}

void pvColorClass::setNull ( void )
{

  null = 1;

  if ( !alarmed ) {
    effectivePixel = saveValPixel;
  }

}

void pvColorClass::setNotNull ( void )
{

  null = 0;

  if ( !alarmed ) {
    effectivePixel = pixel;
  }

}

void pvColorClass::setRuleMode ( void )
{

  ruleMode = 1;

}

void pvColorClass::setNotRuleMode ( void )
{

  ruleMode = 0;

}

void pvColorClass::setRuleColor (
  unsigned int color,
  colorInfoClass *ci )
{

  rulePixel = color;
  changeColor( color, ci );

}
