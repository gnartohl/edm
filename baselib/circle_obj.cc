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

#define __circle_obj_cc 1

#include "circle_obj.h"
#include "app_pkg.h"
#include "act_win.h"

#include "thread.h"

#ifdef __epics__

static void acoMonitorAlarmPvConnectState (
  struct connection_handler_args arg )
{

activeCircleClass *aco = (activeCircleClass *) ca_puser(arg.chid);

  if ( arg.op == CA_OP_CONN_UP ) {

    aco->needAlarmConnectInit = 1;

  }
  else { // lost connection

    aco->alarmPvConnected = 0;
    aco->active = 0;
    aco->lineColor.setDisconnected();
    aco->fillColor.setDisconnected();
    aco->bufInvalidate();
    aco->needDraw = 1;

  }

  aco->actWin->appCtx->proc->lock();
  aco->actWin->addDefExeNode( aco->aglPtr );
  aco->actWin->appCtx->proc->unlock();

}

static void circleAlarmUpdate (
  struct event_handler_args ast_args )
{

class activeCircleClass *aco;
struct dbr_sts_float statusRec;

  aco = (activeCircleClass *) ast_args.usr;

  statusRec = *( (struct dbr_sts_float *) ast_args.dbr );
  aco->lineColor.setStatus( statusRec.status, statusRec.severity );
  aco->fillColor.setStatus( statusRec.status, statusRec.severity );

  if ( aco->active ) {
    aco->bufInvalidate();
    aco->needRefresh = 1;
    aco->actWin->appCtx->proc->lock();
    aco->actWin->addDefExeNode( aco->aglPtr );
    aco->actWin->appCtx->proc->unlock();
  }

}

static void acoMonitorVisPvConnectState (
  struct connection_handler_args arg )
{

activeCircleClass *aco = (activeCircleClass *) ca_puser(arg.chid);

  if ( arg.op == CA_OP_CONN_UP ) {

    aco->needVisConnectInit = 1;

  }
  else { // lost connection

    aco->visPvConnected = 0;
    aco->active = 0;
    aco->lineColor.setDisconnected();
    aco->fillColor.setDisconnected();
    aco->bufInvalidate();
    aco->needDraw = 1;

  }

  aco->actWin->appCtx->proc->lock();
  aco->actWin->addDefExeNode( aco->aglPtr );
  aco->actWin->appCtx->proc->unlock();

}

static void circleVisUpdate (
  struct event_handler_args ast_args )
{

pvValType pvV;
class activeCircleClass *aco = (activeCircleClass *) ast_args.usr;

  pvV.d = *( (double *) ast_args.dbr );
  if ( ( pvV.d >= aco->minVis.d ) && ( pvV.d < aco->maxVis.d ) )
    aco->visibility = 1 ^ aco->visInverted;
  else
    aco->visibility = 0 ^ aco->visInverted;

  if ( aco->active ) {

    if ( aco->visibility ) {

      aco->needRefresh = 1;

    }
    else {

      aco->needErase = 1;
      aco->needRefresh = 1;

    }

    aco->actWin->appCtx->proc->lock();
    aco->actWin->addDefExeNode( aco->aglPtr );
    aco->actWin->appCtx->proc->unlock();

  }

}

#endif

static void acc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeCircleClass *aco = (activeCircleClass *) client;

  aco->actWin->setChanged();

  aco->eraseSelectBoxCorners();
  aco->erase();

  aco->fill = aco->bufFill;

  aco->lineColorMode = aco->bufLineColorMode;
  if ( aco->lineColorMode == ACC_K_COLORMODE_ALARM )
    aco->lineColor.setAlarmSensitive();
  else
    aco->lineColor.setAlarmInsensitive();
  aco->lineColor.setColor( aco->bufLineColor, aco->actWin->ci );

  aco->fillColorMode = aco->bufFillColorMode;
  if ( aco->fillColorMode == ACC_K_COLORMODE_ALARM )
    aco->fillColor.setAlarmSensitive();
  else
    aco->fillColor.setAlarmInsensitive();
  aco->fillColor.setColor( aco->bufFillColor, aco->actWin->ci );

  aco->lineWidth = aco->bufLineWidth;

  if ( aco->bufLineStyle == 0 )
    aco->lineStyle = LineSolid;
  else if ( aco->bufLineStyle == 1 )
    aco->lineStyle = LineOnOffDash;

  aco->alarmPvExpStr.setRaw( aco->bufAlarmPvName );

  aco->visPvExpStr.setRaw( aco->bufVisPvName );

  if ( aco->bufVisInverted )
    aco->visInverted = 0;
  else
    aco->visInverted = 1;

  strncpy( aco->minVisString, aco->bufMinVisString, 39 );
  strncpy( aco->maxVisString, aco->bufMaxVisString, 39 );

  aco->x = aco->bufX;
  aco->sboxX = aco->bufX;

  aco->y = aco->bufY;
  aco->sboxY = aco->bufY;

  aco->w = aco->bufW;
  aco->sboxW = aco->bufW;

  aco->h = aco->bufH;
  aco->sboxH = aco->bufH;

}

static void acc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeCircleClass *aco = (activeCircleClass *) client;

  acc_edit_update( w, client, call );
  aco->refresh( aco );

}

static void acc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeCircleClass *aco = (activeCircleClass *) client;

  acc_edit_update( w, client, call );
  aco->ef.popdown();
  aco->operationComplete();

}

static void acc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeCircleClass *aco = (activeCircleClass *) client;

  aco->ef.popdown();
  aco->operationCancel();

}

static void acc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeCircleClass *aco = (activeCircleClass *) client;

  aco->erase();
  aco->deleteRequest = 1;
  aco->ef.popdown();
  aco->operationCancel();
  aco->drawAll();

}

activeCircleClass::activeCircleClass ( void ) {

  name = new char[strlen("activeCircleClass")+1];
  strcpy( name, "activeCircleClass" );
  visibility = 0;
  prevVisibility = -1;
  visInverted = 0;
  visPvConnected = alarmPvConnected = 0;
  visPvExists = alarmPvExists = 0;
  active = 0;
  activeMode = 0;
  fill = 0;
  lineColorMode = ACC_K_COLORMODE_STATIC;
  fillColorMode = ACC_K_COLORMODE_STATIC;
  lineWidth = 1;
  lineStyle = LineSolid;
  strcpy( minVisString, "" );
  strcpy( maxVisString, "" );

}

// copy constructor
activeCircleClass::activeCircleClass
( const activeCircleClass *source ) {

activeGraphicClass *ago = (activeGraphicClass *) this;

  ago->clone( (activeGraphicClass *) source );

  name = new char[strlen("activeCircleClass")+1];
  strcpy( name, "activeCircleClass" );

  lineColor.copy(source->lineColor);
  fillColor.copy(source->fillColor);
  lineCb = source->lineCb;
  fillCb = source->fillCb;
  fill = source->fill;
  lineColorMode = source->lineColorMode;
  fillColorMode = source->fillColorMode;
  visInverted = source->visInverted;

  alarmPvExpStr.setRaw( source->alarmPvExpStr.rawString );
  visPvExpStr.setRaw( source->visPvExpStr.rawString );

  visibility = 0;
  prevVisibility = -1;
  visPvConnected = alarmPvConnected = 0;
  visPvExists = alarmPvExists = 0;
  active = 0;
  activeMode = 0;

  strncpy( minVisString, source->minVisString, 39 );
  strncpy( maxVisString, source->maxVisString, 39 );

  lineWidth = source->lineWidth;
  lineStyle = source->lineStyle;

}

int activeCircleClass::createInteractive (
  activeWindowClass *aw_obj,
  int _x,
  int _y,
  int _w,
  int _h ) {

//   printf( "In activeCircleClass::createInteractive\n" );

  actWin = (activeWindowClass *) aw_obj;
  x = _x;
  y = _y;
  w = _w;
  h = _h;

  lineColor.setColor( actWin->defaultTextFgColor, actWin->ci );
  fillColor.setColor( actWin->defaultBgColor, actWin->ci );

  this->draw();

  this->editCreate();

  return 1;

}

int activeCircleClass::genericEdit ( void ) {

char title[32], *ptr;

  ptr = actWin->obj.getNameFromClass( "activeCircleClass" );
  if ( ptr )
    strncpy( title, ptr, 31 );
  else
    strncpy( title, activeCircleClass_str4, 31 );

  strncat( title, activeCircleClass_str5, 31 );

  bufX = x;
  bufY = y;
  bufW = w;
  bufH = h;

  bufLineColor = lineColor.pixelColor();
  bufLineColorMode = lineColorMode;

  bufFillColor = fillColor.pixelColor();
  bufFillColorMode = fillColorMode;

  bufFill = fill;
  bufLineWidth = lineWidth;
  bufLineStyle = lineStyle;

  if ( alarmPvExpStr.getRaw() )
    strncpy( bufAlarmPvName, alarmPvExpStr.getRaw(), 39 );
  else
    strncpy( bufAlarmPvName, "", 39 );

  if ( visPvExpStr.getRaw() )
    strncpy( bufVisPvName, visPvExpStr.getRaw(), 39 );
  else
    strncpy( bufVisPvName, "", 39 );

  if ( visInverted )
    bufVisInverted = 0;
  else
    bufVisInverted = 1;

  strncpy( bufMinVisString, minVisString, 39 );
  strncpy( bufMaxVisString, maxVisString, 39 );

  ef.create( actWin->top, actWin->appCtx->ci.getColorMap(),
   &actWin->appCtx->entryFormX,
   &actWin->appCtx->entryFormY, &actWin->appCtx->entryFormW,
   &actWin->appCtx->entryFormH, &actWin->appCtx->largestH,
   title, NULL, NULL, NULL );

  ef.addTextField( activeCircleClass_str6, 25, &bufX );
  ef.addTextField( activeCircleClass_str7, 25, &bufY );
  ef.addTextField( activeCircleClass_str8, 25, &bufW );
  ef.addTextField( activeCircleClass_str9, 25, &bufH );
  ef.addOption( activeCircleClass_str10, activeCircleClass_str11, &bufLineWidth );
  ef.addOption( activeCircleClass_str12, activeCircleClass_str13, &bufLineStyle );
  ef.addColorButton( activeCircleClass_str14, actWin->ci, &lineCb, &bufLineColor );
  ef.addToggle( activeCircleClass_str15, &bufLineColorMode );
  ef.addToggle( activeCircleClass_str16, &bufFill );
  ef.addColorButton( activeCircleClass_str17, actWin->ci, &fillCb, &bufFillColor );
  ef.addToggle( activeCircleClass_str18, &bufFillColorMode );
  ef.addTextField( activeCircleClass_str19, 27, bufAlarmPvName, 39 );
  ef.addTextField( activeCircleClass_str20, 27, bufVisPvName, 39 );
  ef.addOption( " ", activeCircleClass_str21, &bufVisInverted );
  ef.addTextField( activeCircleClass_str22, 27, bufMinVisString, 39 );
  ef.addTextField( activeCircleClass_str23, 27, bufMaxVisString, 39 );

  return 1;

}

int activeCircleClass::editCreate ( void ) {

  this->genericEdit();
  ef.finished( acc_edit_ok, acc_edit_apply, acc_edit_cancel_delete, this );
  actWin->currentEf = NULL;
  ef.popup();

  return 1;

}

int activeCircleClass::edit ( void ) {

  this->genericEdit();
  ef.finished( acc_edit_ok, acc_edit_apply, acc_edit_cancel, this );
  actWin->currentEf = &ef;
  ef.popup();

  return 1;

}

int activeCircleClass::createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int r, g, b, index;
int major, minor, release;
unsigned int pixel;
char oneName[39+1];

  this->actWin = _actWin;

  fscanf( f, "%d %d %d\n", &major, &minor, &release ); actWin->incLine();

  fscanf( f, "%d\n", &x ); actWin->incLine();
  fscanf( f, "%d\n", &y ); actWin->incLine();
  fscanf( f, "%d\n", &w ); actWin->incLine();
  fscanf( f, "%d\n", &h ); actWin->incLine();

  this->initSelectBox(); // call after getting x,y,w,h

  if ( major > 1 ) {

    fscanf( f, "%d\n", &index ); actWin->incLine();
    actWin->ci->setIndex( index, &pixel );
    lineColor.setColor( pixel, actWin->ci );

    fscanf( f, "%d\n", &lineColorMode ); actWin->incLine();

    if ( lineColorMode == ACC_K_COLORMODE_ALARM )
      lineColor.setAlarmSensitive();
    else
      lineColor.setAlarmInsensitive();

    fscanf( f, "%d\n", &fill ); actWin->incLine();

    fscanf( f, "%d\n", &index ); actWin->incLine();
    actWin->ci->setIndex( index, &pixel );
    fillColor.setColor( pixel, actWin->ci );

  }
  else {

    fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
    if ( ( major < 2 ) && ( minor < 3 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    actWin->ci->setRGB( r, g, b, &pixel );
    lineColor.setColor( pixel, actWin->ci );

    fscanf( f, "%d\n", &lineColorMode ); actWin->incLine();

    if ( lineColorMode == ACC_K_COLORMODE_ALARM )
      lineColor.setAlarmSensitive();
    else
      lineColor.setAlarmInsensitive();

    fscanf( f, "%d\n", &fill ); actWin->incLine();

    fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
    if ( ( major < 2 ) && ( minor < 3 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    actWin->ci->setRGB( r, g, b, &pixel );
    fillColor.setColor( pixel, actWin->ci );

  }

  fscanf( f, "%d\n", &fillColorMode ); actWin->incLine();

  if ( fillColorMode == ACC_K_COLORMODE_ALARM )
    fillColor.setAlarmSensitive();
  else
    fillColor.setAlarmInsensitive();

  readStringFromFile( oneName, 39, f ); actWin->incLine();
  alarmPvExpStr.setRaw( oneName );

  readStringFromFile( oneName, 39, f ); actWin->incLine();
  visPvExpStr.setRaw( oneName );

  fscanf( f, "%d\n", &visInverted ); actWin->incLine();

  if ( ( major > 1 ) || ( minor > 1 ) ) {
    readStringFromFile( minVisString, 39, f ); actWin->incLine();
    readStringFromFile( maxVisString, 39, f ); actWin->incLine();
  }
  else {
    strcpy( minVisString, "1" );
    strcpy( maxVisString, "1" );
  }

  if ( ( major > 1 ) || ( minor > 0 ) ) {
    fscanf( f, "%d\n", &lineWidth ); actWin->incLine();
    fscanf( f, "%d\n", &lineStyle ); actWin->incLine();
  }
  else {
    lineWidth = 1;
    lineStyle = LineSolid;
  }

  return 1;

}

int activeCircleClass::importFromXchFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin ) {

int fgR, fgG, fgB, bgR, bgG, bgB, more;
unsigned int pixel;
char *tk, *gotData, *context, buf[255+1];

  fgR = 0xffff;
  fgG = 0xffff;
  fgB = 0xffff;

  bgR = 0xffff;
  bgG = 0xffff;
  bgB = 0xffff;

  this->actWin = _actWin;

  lineColor.setColor( actWin->defaultFg1Color, actWin->ci );
  fillColor.setColor( actWin->defaultBgColor, actWin->ci );

  // continue until tag is <eod>

  do {

    gotData = getNextDataString( buf, 255, f );
    if ( !gotData ) {
      actWin->appCtx->postMessage( activeCircleClass_str32 );
      return 0;
    }

    context = NULL;

    tk = strtok_r( buf, " \t\n", &context );
    if ( !tk ) {
      actWin->appCtx->postMessage( activeCircleClass_str32 );
      return 0;
    }

    if ( strcmp( tk, "<eod>" ) == 0 ) {

      more = 0;

    }
    else {

      more = 1;

      if ( strcmp( tk, "x" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( activeCircleClass_str32 );
          return 0;
        }

        x = atol( tk );

      }
      else if ( strcmp( tk, "y" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( activeCircleClass_str32 );
          return 0;
        }

        y = atol( tk );

      }
      else if ( strcmp( tk, "w" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( activeCircleClass_str32 );
          return 0;
        }

        w = atol( tk );

      }
      else if ( strcmp( tk, "h" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( activeCircleClass_str32 );
          return 0;
        }

        h = atol( tk );

      }
            
      else if ( strcmp( tk, "fgred" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( activeCircleClass_str32 );
          return 0;
        }

        fgR = atol( tk );

      }
            
      else if ( strcmp( tk, "fggreen" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( activeCircleClass_str32 );
          return 0;
        }

        fgG = atol( tk );

      }
            
      else if ( strcmp( tk, "fgblue" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( activeCircleClass_str32 );
          return 0;
        }

        fgB = atol( tk );

      }
            
      else if ( strcmp( tk, "bgred" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( activeCircleClass_str32 );
          return 0;
        }

        bgR = atol( tk );

      }
            
      else if ( strcmp( tk, "bggreen" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( activeCircleClass_str32 );
          return 0;
        }

        bgG = atol( tk );

      }
            
      else if ( strcmp( tk, "bgblue" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( activeCircleClass_str32 );
          return 0;
        }

        bgB = atol( tk );

      }
            
      else if ( strcmp( tk, "linewidth" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( activeCircleClass_str32 );
          return 0;
        }

       lineWidth  = atol( tk );

      }
            
      else if ( strcmp( tk, "fill" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( activeCircleClass_str32 );
          return 0;
        }

        fill = atol( tk );

      }
            
    }

  } while ( more );

  this->initSelectBox(); // call after getting x,y,w,h

  actWin->ci->setRGB( fgR, fgG, fgB, &pixel );
  lineColor.setColor( pixel, actWin->ci );
  lineColor.setAlarmInsensitive();

  actWin->ci->setRGB( bgR, bgG, bgB, &pixel );
  fillColor.setColor( pixel, actWin->ci );
  fillColor.setAlarmSensitive();

  return 1;

}

int activeCircleClass::save (
  FILE *f )
{

int index;

  fprintf( f, "%-d %-d %-d\n", ACC_MAJOR_VERSION, ACC_MINOR_VERSION,
   ACC_RELEASE );
  fprintf( f, "%-d\n", x );
  fprintf( f, "%-d\n", y );
  fprintf( f, "%-d\n", w );
  fprintf( f, "%-d\n", h );

  actWin->ci->getIndex( lineColor.pixelColor(), &index );
  fprintf( f, "%-d\n", index );

  fprintf( f, "%-d\n", lineColorMode );

  fprintf( f, "%-d\n", fill );

  actWin->ci->getIndex( fillColor.pixelColor(), &index );
  fprintf( f, "%-d\n", index );

  fprintf( f, "%-d\n", fillColorMode );

  if ( alarmPvExpStr.getRaw() )
    writeStringToFile( f, alarmPvExpStr.getRaw() );
  else
    writeStringToFile( f, "" );

  if ( visPvExpStr.getRaw() )
    writeStringToFile( f, visPvExpStr.getRaw() );
  else
    writeStringToFile( f, "" );

  fprintf( f, "%-d\n", visInverted );
  writeStringToFile( f, minVisString );
  writeStringToFile( f, maxVisString );

  fprintf( f, "%-d\n", lineWidth );

  fprintf( f, "%-d\n", lineStyle );

  return 1;

}

int activeCircleClass::drawActive ( void )
{

  if ( !activeMode || !visibility ) return 1;

  prevVisibility = visibility;

  actWin->executeGc.setLineStyle( lineStyle );
  actWin->executeGc.setLineWidth( lineWidth );
  actWin->executeGc.saveFg();

  if ( fill ) {
    actWin->executeGc.setFG( fillColor.getColor() );
    XFillArc( actWin->d, XtWindow(actWin->executeWidget),
     actWin->executeGc.normGC(), x, y, w, h, 0, 23040 );
  }

  actWin->executeGc.setFG( lineColor.getColor() );
  XDrawArc( actWin->d, XtWindow(actWin->executeWidget),
   actWin->executeGc.normGC(), x, y, w, h, 0, 23040 );

  actWin->executeGc.setLineStyle( LineSolid );
  actWin->executeGc.setLineWidth( 1 );
  actWin->executeGc.restoreFg();

  return 1;

}

int activeCircleClass::eraseUnconditional ( void )
{

  actWin->executeGc.setLineStyle( lineStyle );
  actWin->executeGc.setLineWidth( lineWidth );

  if ( fill ) {
    XFillArc( actWin->d, XtWindow(actWin->executeWidget),
     actWin->executeGc.eraseGC(), x, y, w, h, 0, 23040 );
  }

  XDrawArc( actWin->d, XtWindow(actWin->executeWidget),
   actWin->executeGc.eraseGC(), x, y, w, h, 0, 23040 );

  XFillArc( actWin->d, XtWindow(actWin->executeWidget),
   actWin->executeGc.eraseGC(), x, y, w, h, 0, 23040 );

  actWin->executeGc.setLineStyle( LineSolid );
  actWin->executeGc.setLineWidth( 1 );

  return 1;

}

int activeCircleClass::eraseActive ( void )
{

  if ( !activeMode ) return 1;

  if ( prevVisibility == 0 ) {
    prevVisibility = visibility;
    return 1;
  }

  prevVisibility = visibility;

  actWin->executeGc.setLineStyle( lineStyle );
  actWin->executeGc.setLineWidth( lineWidth );

  if ( fill ) {
    XFillArc( actWin->d, XtWindow(actWin->executeWidget),
     actWin->executeGc.eraseGC(), x, y, w, h, 0, 23040 );
  }

  XDrawArc( actWin->d, XtWindow(actWin->executeWidget),
   actWin->executeGc.eraseGC(), x, y, w, h, 0, 23040 );

  XFillArc( actWin->d, XtWindow(actWin->executeWidget),
   actWin->executeGc.eraseGC(), x, y, w, h, 0, 23040 );

  actWin->executeGc.setLineStyle( LineSolid );
  actWin->executeGc.setLineWidth( 1 );

  return 1;

}

int activeCircleClass::expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int stat;

  stat = alarmPvExpStr.expand1st( numMacros, macros, expansions );
  stat = visPvExpStr.expand1st( numMacros, macros, expansions );

  return stat;

}

int activeCircleClass::expand2nd (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int stat;

  stat = alarmPvExpStr.expand2nd( numMacros, macros, expansions );
  stat = visPvExpStr.expand2nd( numMacros, macros, expansions );

  return stat;

}

int activeCircleClass::containsMacros ( void ) {

  if ( alarmPvExpStr.containsPrimaryMacros() ) return 1;
  if ( visPvExpStr.containsPrimaryMacros() ) return 1;

  return 0;

}

int activeCircleClass::activate (
  int pass,
  void *ptr )
{

int stat;

  switch ( pass ) {

  case 1: // initialize

    needVisConnectInit = 0;
    needAlarmConnectInit = 0;
    needErase = needDraw = needRefresh = 0;
    aglPtr = ptr;
    opComplete = 0;

#ifdef __epics__
    alarmEventId = visEventId = 0;
#endif

    alarmPvConnected = visPvConnected = 0;
    active = 0;
    activeMode = 1;
    prevVisibility = -1;

    init = 1;
    active = 1;

    if ( !alarmPvExpStr.getExpanded() ||
         ( strcmp( alarmPvExpStr.getExpanded(), "" ) == 0 ) ) {
      alarmPvExists = 0;
    }
    else {
      alarmPvExists = 1;
      lineColor.setConnectSensitive();
      fillColor.setConnectSensitive();
      init = 0;
      active = 0;
    }

    if ( !visPvExpStr.getExpanded() ||
         ( strcmp( visPvExpStr.getExpanded(), "" ) == 0 ) ) {
      visPvExists = 0;
      visibility = 1;
    }
    else {
      visPvExists = 1;
      visibility = 0;
      lineColor.setConnectSensitive();
      fillColor.setConnectSensitive();
      init = 0;
      active = 0;
    }

    break;

  case 2: // connect to pv's

    if ( !opComplete ) {

#ifdef __epics__

      if ( alarmPvExists ) {
        stat = ca_search_and_connect( alarmPvExpStr.getExpanded(), &alarmPvId,
         acoMonitorAlarmPvConnectState, this );
        if ( stat != ECA_NORMAL ) {
          printf( activeCircleClass_str24 );
          return 0;
        }
      }

      if ( visPvExists ) {
        stat = ca_search_and_connect( visPvExpStr.getExpanded(), &visPvId,
         acoMonitorVisPvConnectState, this );
        if ( stat != ECA_NORMAL ) {
          printf( activeCircleClass_str25 );
          return 0;
        }
      }

      opComplete = 1;
      this->bufInvalidate();

#endif

    }

    break;

  case 3:
  case 4:
  case 5:
  case 6:

    break;

  }

  return 1;

}

int activeCircleClass::deactivate (
  int pass
) {

int stat;

  activeMode = 0;

  if ( pass == 1 ) {

#ifdef __epics__

  if ( alarmPvExists ) {
    stat = ca_clear_channel( alarmPvId );
    if ( stat != ECA_NORMAL )
      printf( activeCircleClass_str28 );
  }

  if ( visPvExists ) {
    stat = ca_clear_channel( visPvId );
    if ( stat != ECA_NORMAL )
      printf( activeCircleClass_str29 );
  }

#endif

  }

  return 1;

}

int activeCircleClass::draw ( void ) {

  if ( activeMode ) return 1;
  if ( deleteRequest ) return 1;

  actWin->drawGc.setLineStyle( lineStyle );
  actWin->drawGc.setLineWidth( lineWidth );
  actWin->drawGc.saveFg();

  if ( fill ) {
    actWin->drawGc.setFG( fillColor.pixelColor() );
    XFillArc( actWin->d, XtWindow(actWin->drawWidget), actWin->drawGc.normGC(),
     x, y, w, h, 0, 23040 );
  }

  actWin->drawGc.setFG( lineColor.pixelColor() );
  XDrawArc( actWin->d, XtWindow(actWin->drawWidget), actWin->drawGc.normGC(),
   x, y, w, h, 0, 23040 );

  actWin->drawGc.setLineStyle( LineSolid );
  actWin->drawGc.setLineWidth( 1 );
  actWin->drawGc.restoreFg();

  return 1;

}

int activeCircleClass::erase ( void ) {

  if ( activeMode ) return 1;
  if ( deleteRequest ) return 1;

  actWin->drawGc.setLineStyle( lineStyle );
  actWin->drawGc.setLineWidth( lineWidth );

  if ( fill ) {
    XFillArc( actWin->d, XtWindow(actWin->drawWidget),
     actWin->drawGc.eraseGC(), x, y, w, h, 0, 23040 );
  }

  XDrawArc( actWin->d, XtWindow(actWin->drawWidget), actWin->drawGc.eraseGC(),
   x, y, w, h, 0, 23040 );

  actWin->drawGc.setLineStyle( LineSolid );
  actWin->drawGc.setLineWidth( 1 );

  return 1;

}

void activeCircleClass::executeDeferred ( void ) {

int stat, nvc, nac, ne, nd, nr;

  if ( actWin->isIconified ) return;

  actWin->appCtx->proc->lock();
  nvc = needVisConnectInit; needVisConnectInit = 0;
  nac = needAlarmConnectInit; needAlarmConnectInit = 0;
  ne = needErase; needErase = 0;
  nd = needDraw; needDraw = 0;
  nr = needRefresh; needRefresh = 0;
  actWin->remDefExeNode( aglPtr );
  actWin->appCtx->proc->unlock();

#ifdef __epics__

  if ( nvc ) {

    if ( ( ca_field_type(visPvId) == DBR_ENUM ) ||
         ( ca_field_type(visPvId) == DBR_INT ) ||
         ( ca_field_type(visPvId) == DBR_LONG ) ||
         ( ca_field_type(visPvId) == DBR_FLOAT ) ||
         ( ca_field_type(visPvId) == DBR_DOUBLE ) ) {

      visPvConnected = 1;

      pvType = ca_field_type( visPvId );

      minVis.d = (double) atof( minVisString );
      maxVis.d = (double) atof( maxVisString );

      if ( ( visPvConnected || !visPvExists ) &&
           ( alarmPvConnected || !alarmPvExists ) ) {

        active = 1;
        lineColor.setConnected();
        fillColor.setConnected();
        bufInvalidate();

        if ( init ) {
          eraseUnconditional();
	}

        init = 1;

        actWin->requestActiveRefresh();

      }

      if ( !visEventId ) {
        stat = ca_add_masked_array_event( DBR_DOUBLE, 1, visPvId,
         circleVisUpdate, (void *) this, (float) 0.0, (float) 0.0,
         (float) 0.0, &visEventId, DBE_VALUE );
        if ( stat != ECA_NORMAL ) {
          printf( activeCircleClass_str30 );
        }
      }

    }
    else { // force a draw in the non-active state

      active = 0;
      lineColor.setDisconnected();
      fillColor.setDisconnected();
      bufInvalidate();
      drawActive();

    }

  }

  if ( nac ) {

    alarmPvConnected = 1;

    if ( ( visPvConnected || !visPvExists ) &&
         ( alarmPvConnected || !alarmPvExists ) ) {

      active = 1;
      lineColor.setConnected();
      fillColor.setConnected();
      bufInvalidate();

      if ( init ) {
        eraseUnconditional();
      }

      init = 1;

      actWin->requestActiveRefresh();

    }

    if ( !alarmEventId ) {
      stat = ca_add_masked_array_event( DBR_STS_FLOAT, 1, alarmPvId,
       circleAlarmUpdate, (void *) this, (float) 0.0, (float) 0.0,
       (float) 0.0, &alarmEventId, DBE_ALARM );
      if ( stat != ECA_NORMAL ) {
        printf( activeCircleClass_str31 );
      }
    }

  }

#endif

  if ( ne ) {
    eraseActive();
  }

  if ( nd ) {
//      drawActive();
    stat = smartDrawAllActive();
  }

  if ( nr ) {
//      actWin->requestActiveRefresh();
    stat = smartDrawAllActive();
  }

}

char *activeCircleClass::firstDragName ( void ) {

  dragIndex = 0;
  return dragName[dragIndex];

}

char *activeCircleClass::nextDragName ( void ) {

  if ( dragIndex < (int) ( sizeof(dragName) / sizeof(char *) ) - 1 ) {
    dragIndex++;
    return dragName[dragIndex];
  }
  else {
    return NULL;
  }

}

char *activeCircleClass::dragValue (
  int i ) {

  switch ( i ) {

  case 1:
    return alarmPvExpStr.getExpanded();
    break;

  case 2:
    return visPvExpStr.getExpanded();
    break;

  }

  // else, disabled

  return (char *) NULL;

}

void activeCircleClass::changeDisplayParams (
  unsigned int _flag,
  char *_fontTag,
  int _alignment,
  char *_ctlFontTag,
  int _ctlAlignment,
  char *_btnFontTag,
  int _btnAlignment,
  unsigned int _textFgColor,
  unsigned int _fg1Color,
  unsigned int _fg2Color,
  unsigned int _offsetColor,
  unsigned int _bgColor,
  unsigned int _topShadowColor,
  unsigned int _botShadowColor )
{

  if ( _flag & ACTGRF_FG1COLOR_MASK )
    lineColor.setColor( _fg1Color, actWin->ci );

  if ( _flag & ACTGRF_BGCOLOR_MASK )
    fillColor.setColor( _bgColor, actWin->ci );

}

void activeCircleClass::changePvNames (
  int flag,
  int numCtlPvs,
  char *ctlPvs[],
  int numReadbackPvs,
  char *readbackPvs[],
  int numNullPvs,
  char *nullPvs[],
  int numVisPvs,
  char *visPvs[],
  int numAlarmPvs,
  char *alarmPvs[] )
{

  if ( flag & ACTGRF_VISPVS_MASK ) {
    if ( numVisPvs ) {
      visPvExpStr.setRaw( visPvs[0] );
    }
  }

  if ( flag & ACTGRF_ALARMPVS_MASK ) {
    if ( numAlarmPvs ) {
      alarmPvExpStr.setRaw( alarmPvs[0] );
    }
  }

}

#ifdef __cplusplus
extern "C" {
#endif

void *create_activeCircleClassPtr ( void ) {

activeCircleClass *ptr;

  ptr = new activeCircleClass;
  return (void *) ptr;

}

void *clone_activeCircleClassPtr (
  void *_srcPtr )
{

activeCircleClass *ptr, *srcPtr;

  srcPtr = (activeCircleClass *) _srcPtr;

  ptr = new activeCircleClass( srcPtr );

  return (void *) ptr;

}

#ifdef __cplusplus
}
#endif
