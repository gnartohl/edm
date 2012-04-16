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

static void doBlink (
  void *ptr
) {

activeCircleClass *aco = (activeCircleClass *) ptr;

  if ( !aco->activeMode ) {
    if ( aco->isSelected() ) aco->drawSelectBoxCorners(); // erase via xor
    aco->smartDrawAll();
    if ( aco->isSelected() ) aco->drawSelectBoxCorners();
  }
  else {
    aco->bufInvalidate();
    aco->smartDrawAllActive();
  }

}

static void unconnectedTimeout (
  XtPointer client,
  XtIntervalId *id )
{

activeCircleClass *aco = (activeCircleClass *) client;

  if ( !aco->init ) {
    aco->needToDrawUnconnected = 1;
    aco->needRefresh = 1;
    aco->actWin->addDefExeNode( aco->aglPtr );
  }

  aco->unconnectedTimer = 0;

}

static void acc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeCircleClass *aco = (activeCircleClass *) client;

  aco->actWin->setChanged();

  aco->eraseSelectBoxCorners();
  aco->erase();

  aco->fill = aco->eBuf->bufFill;

  aco->lineColorMode = aco->eBuf->bufLineColorMode;
  if ( aco->lineColorMode == ACC_K_COLORMODE_ALARM )
    aco->lineColor.setAlarmSensitive();
  else
    aco->lineColor.setAlarmInsensitive();
  aco->lineColor.setColorIndex( aco->eBuf->bufLineColor, aco->actWin->ci );

  aco->fillColorMode = aco->eBuf->bufFillColorMode;
  if ( aco->fillColorMode == ACC_K_COLORMODE_ALARM )
    aco->fillColor.setAlarmSensitive();
  else
    aco->fillColor.setAlarmInsensitive();
  aco->fillColor.setColorIndex( aco->eBuf->bufFillColor, aco->actWin->ci );

  aco->lineWidth = aco->eBuf->bufLineWidth;

  if ( aco->eBuf->bufLineStyle == 0 )
    aco->lineStyle = LineSolid;
  else if ( aco->eBuf->bufLineStyle == 1 )
    aco->lineStyle = LineOnOffDash;

  aco->alarmPvExpStr.setRaw( aco->eBuf->bufAlarmPvName );

  aco->visPvExpStr.setRaw( aco->eBuf->bufVisPvName );

  if ( aco->eBuf->bufVisInverted )
    aco->visInverted = 0;
  else
    aco->visInverted = 1;

  strncpy( aco->minVisString, aco->eBuf->bufMinVisString, 39 );
  strncpy( aco->maxVisString, aco->eBuf->bufMaxVisString, 39 );

  aco->x = aco->eBuf->bufX;
  aco->sboxX = aco->eBuf->bufX;

  aco->y = aco->eBuf->bufY;
  aco->sboxY = aco->eBuf->bufY;

  aco->w = aco->eBuf->bufW;
  aco->sboxW = aco->eBuf->bufW;

  aco->h = aco->eBuf->bufH;
  aco->sboxH = aco->eBuf->bufH;

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

  aco->ef.popdown();
  aco->operationCancel();
  aco->erase();
  aco->deleteRequest = 1;
  aco->drawAll();

}

void activeCircleClass::alarmPvConnectStateCallback (
  ProcessVariable *pv,
  void *userarg
) {

activeCircleClass *aco = (activeCircleClass *) userarg;

  if ( pv->is_valid() ) {

  }
  else { // lost connection

    aco->connection.setPvDisconnected( (void *) aco->alarmPvConnection );
    aco->lineColor.setDisconnected();
    aco->fillColor.setDisconnected();

    aco->actWin->appCtx->proc->lock();
    aco->needRefresh = 1;
    aco->actWin->addDefExeNode( aco->aglPtr );
    aco->actWin->appCtx->proc->unlock();

  }

}

void activeCircleClass::alarmPvValueCallback (
  ProcessVariable *pv,
  void *userarg
) {

activeCircleClass *aco = (activeCircleClass *) userarg;

  if ( !aco->connection.pvsConnected() ) {

    if ( pv->is_valid() ) {

      aco->connection.setPvConnected( (void *) alarmPvConnection );

      if ( aco->connection.pvsConnected() ) {
        aco->actWin->appCtx->proc->lock();
        aco->needConnectInit = 1;
        aco->actWin->addDefExeNode( aco->aglPtr );
        aco->actWin->appCtx->proc->unlock();
      }

    }

  }
  else {

    aco->actWin->appCtx->proc->lock();
    aco->needAlarmUpdate = 1;
    aco->actWin->addDefExeNode( aco->aglPtr );
    aco->actWin->appCtx->proc->unlock();

  }

}

void activeCircleClass::visPvConnectStateCallback (
  ProcessVariable *pv,
  void *userarg
) {

activeCircleClass *aco = (activeCircleClass *) userarg;

  if ( pv->is_valid() ) {

  }
  else { // lost connection

    aco->connection.setPvDisconnected( (void *) aco->visPvConnection );
    aco->lineColor.setDisconnected();
    aco->fillColor.setDisconnected();

    aco->actWin->appCtx->proc->lock();
    aco->needRefresh = 1;
    aco->actWin->addDefExeNode( aco->aglPtr );
    aco->actWin->appCtx->proc->unlock();

  }

}

void activeCircleClass::visPvValueCallback (
  ProcessVariable *pv,
  void *userarg
) {

activeCircleClass *aco = (activeCircleClass *) userarg;

  if ( !aco->connection.pvsConnected() ) {

    if ( pv->is_valid() ) {

      aco->connection.setPvConnected( (void *) visPvConnection );

      if ( aco->connection.pvsConnected() ) {
        aco->actWin->appCtx->proc->lock();
        aco->needConnectInit = 1;
        aco->actWin->addDefExeNode( aco->aglPtr );
        aco->actWin->appCtx->proc->unlock();
      }

    }

  }
  else {

    aco->actWin->appCtx->proc->lock();
    aco->needVisUpdate = 1;
    aco->actWin->addDefExeNode( aco->aglPtr );
    aco->actWin->appCtx->proc->unlock();

    }

}

activeCircleClass::activeCircleClass ( void ) {

  name = new char[strlen("activeCircleClass")+1];
  strcpy( name, "activeCircleClass" );
  checkBaseClassVersion( activeGraphicClass::MAJOR_VERSION, name );
  visibility = 0;
  prevVisibility = -1;
  visInverted = 0;
  visPvExists = alarmPvExists = 0;
  activeMode = 0;
  fill = 0;
  lineColorMode = ACC_K_COLORMODE_STATIC;
  fillColorMode = ACC_K_COLORMODE_STATIC;
  lineWidth = 1;
  lineStyle = LineSolid;
  strcpy( minVisString, "" );
  strcpy( maxVisString, "" );
  connection.setMaxPvs( 2 );
  unconnectedTimer = 0;
  eBuf = NULL;
  setBlinkFunction( (void *) doBlink );

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
  fill = source->fill;
  lineColorMode = source->lineColorMode;
  fillColorMode = source->fillColorMode;
  visInverted = source->visInverted;

  alarmPvExpStr.setRaw( source->alarmPvExpStr.rawString );
  visPvExpStr.setRaw( source->visPvExpStr.rawString );

  visibility = 0;
  prevVisibility = -1;
  visPvExists = alarmPvExists = 0;
  activeMode = 0;

  strncpy( minVisString, source->minVisString, 39 );
  strncpy( maxVisString, source->maxVisString, 39 );

  lineWidth = source->lineWidth;
  lineStyle = source->lineStyle;

  connection.setMaxPvs( 2 );

  unconnectedTimer = 0;

  eBuf = NULL;

  setBlinkFunction( (void *) doBlink );

  doAccSubs( alarmPvExpStr );
  doAccSubs( visPvExpStr );
  doAccSubs( minVisString, 39 );
  doAccSubs( maxVisString, 39 );

}

activeCircleClass::~activeCircleClass ( void ) {

  if ( name ) delete[] name;

  if ( eBuf ) delete eBuf;

  if ( unconnectedTimer ) {
    XtRemoveTimeOut( unconnectedTimer );
    unconnectedTimer = 0;
  }

  updateBlink( 0 );

}

int activeCircleClass::createInteractive (
  activeWindowClass *aw_obj,
  int _x,
  int _y,
  int _w,
  int _h ) {

//   fprintf( stderr, "In activeCircleClass::createInteractive\n" );

  actWin = (activeWindowClass *) aw_obj;
  x = _x;
  y = _y;
  w = _w;
  h = _h;

  lineColor.setColorIndex( actWin->defaultTextFgColor, actWin->ci );
  fillColor.setColorIndex( actWin->defaultBgColor, actWin->ci );

  this->draw();

  this->editCreate();

  return 1;

}

int activeCircleClass::genericEdit ( void ) {

char title[32], *ptr;

  if ( !eBuf ) {
    eBuf = new editBufType;
  }

  ptr = actWin->obj.getNameFromClass( "activeCircleClass" );
  if ( ptr )
    strncpy( title, ptr, 31 );
  else
    strncpy( title, activeCircleClass_str4, 31 );

  Strncat( title, activeCircleClass_str5, 31 );

  eBuf->bufX = x;
  eBuf->bufY = y;
  eBuf->bufW = w;
  eBuf->bufH = h;

  eBuf->bufLineColor = lineColor.pixelIndex();
  eBuf->bufLineColorMode = lineColorMode;

  eBuf->bufFillColor = fillColor.pixelIndex();
  eBuf->bufFillColorMode = fillColorMode;

  eBuf->bufFill = fill;
  eBuf->bufLineWidth = lineWidth;
  eBuf->bufLineStyle = lineStyle;

  if ( alarmPvExpStr.getRaw() )
    strncpy( eBuf->bufAlarmPvName, alarmPvExpStr.getRaw(), PV_Factory::MAX_PV_NAME );
  else
    strcpy( eBuf->bufAlarmPvName, "" );

  if ( visPvExpStr.getRaw() )
    strncpy( eBuf->bufVisPvName, visPvExpStr.getRaw(), PV_Factory::MAX_PV_NAME );
  else
    strcpy( eBuf->bufVisPvName, "" );

  if ( visInverted )
    eBuf->bufVisInverted = 0;
  else
    eBuf->bufVisInverted = 1;

  strncpy( eBuf->bufMinVisString, minVisString, 39 );
  strncpy( eBuf->bufMaxVisString, maxVisString, 39 );

  ef.create( actWin->top, actWin->appCtx->ci.getColorMap(),
   &actWin->appCtx->entryFormX,
   &actWin->appCtx->entryFormY, &actWin->appCtx->entryFormW,
   &actWin->appCtx->entryFormH, &actWin->appCtx->largestH,
   title, NULL, NULL, NULL );

  ef.addTextField( activeCircleClass_str6, 30, &eBuf->bufX );
  ef.addTextField( activeCircleClass_str7, 30, &eBuf->bufY );
  ef.addTextField( activeCircleClass_str8, 30, &eBuf->bufW );
  ef.addTextField( activeCircleClass_str9, 30, &eBuf->bufH );
  ef.addOption( activeCircleClass_str10, activeCircleClass_str11, &eBuf->bufLineWidth );
  ef.addOption( activeCircleClass_str12, activeCircleClass_str13, &eBuf->bufLineStyle );
  ef.addColorButton( activeCircleClass_str14, actWin->ci, &eBuf->lineCb, &eBuf->bufLineColor );
  ef.addToggle( activeCircleClass_str15, &eBuf->bufLineColorMode );

  ef.addToggle( activeCircleClass_str16, &eBuf->bufFill );
  fillEntry = ef.getCurItem();
  ef.addColorButton( activeCircleClass_str17, actWin->ci, &eBuf->fillCb, &eBuf->bufFillColor );
  fillColorEntry = ef.getCurItem();
  fillEntry->addDependency( fillColorEntry );
  ef.addToggle( activeCircleClass_str18, &eBuf->bufFillColorMode );
  fillAlarmSensEntry = ef.getCurItem();
  fillEntry->addDependency( fillAlarmSensEntry );
  fillEntry->addDependencyCallbacks();

  ef.addTextField( activeCircleClass_str19, 30, eBuf->bufAlarmPvName,
   PV_Factory::MAX_PV_NAME );

  ef.addTextField( activeCircleClass_str20, 30, eBuf->bufVisPvName,
   PV_Factory::MAX_PV_NAME );
  invisPvEntry = ef.getCurItem();
  ef.addOption( " ", activeCircleClass_str21, &eBuf->bufVisInverted );
  visInvEntry = ef.getCurItem();
  invisPvEntry->addDependency( visInvEntry );
  ef.addTextField( activeCircleClass_str22, 30, eBuf->bufMinVisString, 39 );
  minVisEntry = ef.getCurItem();
  invisPvEntry->addDependency( minVisEntry );
  ef.addTextField( activeCircleClass_str23, 30, eBuf->bufMaxVisString, 39 );
  maxVisEntry = ef.getCurItem();
  invisPvEntry->addDependency( maxVisEntry );
  invisPvEntry->addDependencyCallbacks();

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

int major, minor, release, stat;

tagClass tag;

int zero = 0;
int one = 1;
static char *emptyStr = "";

int solid = LineSolid;
static char *styleEnumStr[2] = {
  "solid",
  "dash"
};
static int styleEnum[2] = {
  LineSolid,
  LineOnOffDash
};

  this->actWin = _actWin;

  // read file and process each "object" tag
  tag.init();
  tag.loadR( "beginObjectProperties" );
  tag.loadR( unknownTags );
  tag.loadR( "major", &major );
  tag.loadR( "minor", &minor );
  tag.loadR( "release", &release );
  tag.loadR( "x", &x );
  tag.loadR( "y", &y );
  tag.loadR( "w", &w );
  tag.loadR( "h", &h );
  tag.loadR( "lineColor", actWin->ci, &lineColor );
  tag.loadR( "lineAlarm", &lineColorMode, &zero );
  tag.loadR( "fill", &fill, &zero );
  tag.loadR( "fillColor", actWin->ci, &fillColor );
  tag.loadR( "fillAlarm", &fillColorMode, &zero );
  tag.loadR( "lineWidth", &lineWidth, &one );
  tag.loadR( "lineStyle", 2, styleEnumStr, styleEnum, &lineStyle, &solid );
  tag.loadR( "alarmPv", &alarmPvExpStr, emptyStr );
  tag.loadR( "visPv", &visPvExpStr, emptyStr );
  tag.loadR( "visInvert", &visInverted, &zero );
  tag.loadR( "visMin", 39, minVisString, emptyStr );
  tag.loadR( "visMax", 39, maxVisString, emptyStr );
  tag.loadR( "endObjectProperties" );

  stat = tag.readTags( f, "endObjectProperties" );

  if ( !( stat & 1 ) ) {
    actWin->appCtx->postMessage( tag.errMsg() );
  }

  if ( major > ACC_MAJOR_VERSION ) {
    postIncompatable();
    return 0;
  }

  if ( major < 4 ) {
    postIncompatable();
    return 0;
  }

  this->initSelectBox(); // call after getting x,y,w,h

  if ( lineColorMode == ACC_K_COLORMODE_ALARM )
    lineColor.setAlarmSensitive();
  else
    lineColor.setAlarmInsensitive();

  if ( fillColorMode == ACC_K_COLORMODE_ALARM )
    fillColor.setAlarmSensitive();
  else
    fillColor.setAlarmInsensitive();

  return stat;

}

int activeCircleClass::old_createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int r, g, b, index;
int major, minor, release;
unsigned int pixel;
char oneName[PV_Factory::MAX_PV_NAME+1];

  this->actWin = _actWin;

  fscanf( f, "%d %d %d\n", &major, &minor, &release ); actWin->incLine();

  if ( major > ACC_MAJOR_VERSION ) {
    postIncompatable();
    return 0;
  }

  fscanf( f, "%d\n", &x ); actWin->incLine();
  fscanf( f, "%d\n", &y ); actWin->incLine();
  fscanf( f, "%d\n", &w ); actWin->incLine();
  fscanf( f, "%d\n", &h ); actWin->incLine();

  this->initSelectBox(); // call after getting x,y,w,h

  if ( ( major > 2 ) || ( ( major == 2 ) && ( minor > 0 ) ) ) {

    actWin->ci->readColorIndex( f, &index );
    actWin->incLine(); actWin->incLine();
    lineColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d\n", &lineColorMode ); actWin->incLine();

    if ( lineColorMode == ACC_K_COLORMODE_ALARM )
      lineColor.setAlarmSensitive();
    else
      lineColor.setAlarmInsensitive();

    fscanf( f, "%d\n", &fill ); actWin->incLine();

    actWin->ci->readColorIndex( f, &index );
    actWin->incLine(); actWin->incLine();
    fillColor.setColorIndex( index, actWin->ci );

  }
  else if ( major > 1 ) {

    fscanf( f, "%d\n", &index ); actWin->incLine();
    lineColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d\n", &lineColorMode ); actWin->incLine();

    if ( lineColorMode == ACC_K_COLORMODE_ALARM )
      lineColor.setAlarmSensitive();
    else
      lineColor.setAlarmInsensitive();

    fscanf( f, "%d\n", &fill ); actWin->incLine();

    fscanf( f, "%d\n", &index ); actWin->incLine();
    fillColor.setColorIndex( index, actWin->ci );

  }
  else {

    fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
    if ( ( major < 2 ) && ( minor < 3 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    actWin->ci->setRGB( r, g, b, &pixel );
    index = actWin->ci->pixIndex( pixel );
    lineColor.setColorIndex( index, actWin->ci );

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
    index = actWin->ci->pixIndex( pixel );
    fillColor.setColorIndex( index, actWin->ci );

  }

  fscanf( f, "%d\n", &fillColorMode ); actWin->incLine();

  if ( fillColorMode == ACC_K_COLORMODE_ALARM )
    fillColor.setAlarmSensitive();
  else
    fillColor.setAlarmInsensitive();

  readStringFromFile( oneName, PV_Factory::MAX_PV_NAME+1, f );
   actWin->incLine();
  alarmPvExpStr.setRaw( oneName );

  readStringFromFile( oneName, PV_Factory::MAX_PV_NAME+1, f );
   actWin->incLine();
  visPvExpStr.setRaw( oneName );

  fscanf( f, "%d\n", &visInverted ); actWin->incLine();

  if ( ( major > 1 ) || ( minor > 1 ) ) {
    readStringFromFile( minVisString, 39+1, f ); actWin->incLine();
    readStringFromFile( maxVisString, 39+1, f ); actWin->incLine();
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

int fgR, fgG, fgB, bgR, bgG, bgB, more, index;
unsigned int pixel;
char *tk, *gotData, *context, buf[255+1];

  fgR = 0xffff;
  fgG = 0xffff;
  fgB = 0xffff;

  bgR = 0xffff;
  bgG = 0xffff;
  bgB = 0xffff;

  this->actWin = _actWin;

  lineColor.setColorIndex( actWin->defaultFg1Color, actWin->ci );
  fillColor.setColorIndex( actWin->defaultBgColor, actWin->ci );

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
  index = actWin->ci->pixIndex( pixel );
  lineColor.setColorIndex( index, actWin->ci );
  lineColor.setAlarmInsensitive();

  actWin->ci->setRGB( bgR, bgG, bgB, &pixel );
  index = actWin->ci->pixIndex( pixel );
  fillColor.setColorIndex( index, actWin->ci );
  fillColor.setAlarmSensitive();

  return 1;

}

int activeCircleClass::save (
  FILE *f )
{

int major, minor, release, stat;

tagClass tag;

int zero = 0;
int one = 1;
static char *emptyStr = "";

int solid = LineSolid;
static char *styleEnumStr[2] = {
  "solid",
  "dash"
};
static int styleEnum[2] = {
  LineSolid,
  LineOnOffDash
};

  major = ACC_MAJOR_VERSION;
  minor = ACC_MINOR_VERSION;
  release = ACC_RELEASE;

  tag.init();
  tag.loadW( "beginObjectProperties" );
  tag.loadW( "major", &major );
  tag.loadW( "minor", &minor );
  tag.loadW( "release", &release );
  tag.loadW( "x", &x );
  tag.loadW( "y", &y );
  tag.loadW( "w", &w );
  tag.loadW( "h", &h );
  tag.loadW( "lineColor", actWin->ci, &lineColor );
  tag.loadBoolW( "lineAlarm", &lineColorMode, &zero );
  tag.loadBoolW( "fill", &fill, &zero );
  tag.loadW( "fillColor", actWin->ci, &fillColor );
  tag.loadBoolW( "fillAlarm", &fillColorMode, &zero );
  tag.loadW( "lineWidth", &lineWidth, &one );
  tag.loadW( "lineStyle", 2, styleEnumStr, styleEnum, &lineStyle, &solid );
  tag.loadW( "alarmPv", &alarmPvExpStr, emptyStr  );
  tag.loadW( "visPv", &visPvExpStr, emptyStr );
  tag.loadBoolW( "visInvert", &visInverted, &zero );
  tag.loadW( "visMin", minVisString, emptyStr );
  tag.loadW( "visMax", maxVisString, emptyStr );
  tag.loadW( unknownTags );

  tag.loadW( "endObjectProperties" );
  tag.loadW( "" );

  stat = tag.writeTags( f );

  return stat;

}

int activeCircleClass::old_save (
  FILE *f )
{

int index;

  fprintf( f, "%-d %-d %-d\n", ACC_MAJOR_VERSION, ACC_MINOR_VERSION,
   ACC_RELEASE );
  fprintf( f, "%-d\n", x );
  fprintf( f, "%-d\n", y );
  fprintf( f, "%-d\n", w );
  fprintf( f, "%-d\n", h );

  index = lineColor.pixelIndex();
  actWin->ci->writeColorIndex( f, index );
  //fprintf( f, "%-d\n", index );

  fprintf( f, "%-d\n", lineColorMode );

  fprintf( f, "%-d\n", fill );

  index = fillColor.pixelIndex();
  actWin->ci->writeColorIndex( f, index );
  //fprintf( f, "%-d\n", index );

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

int activeCircleClass::drawActiveIfIntersects (
  int x0,
  int y0,
  int x1,
  int y1 ) {

  int delta = lineWidth/2 + 1;

  if ( intersects( x0-delta, y0-delta, x1+delta, y1+delta ) ) {
    bufInvalidate();
    drawActive();
  }

  return 1;

}

int activeCircleClass::drawActive ( void )
{

int blink = 0;

  if ( !init ) {
    if ( needToDrawUnconnected ) {
      actWin->executeGc.saveFg();
      //actWin->executeGc.setFG( lineColor.getDisconnected() );
      actWin->executeGc.setFG( lineColor.getDisconnectedIndex(), &blink );
      actWin->executeGc.setLineWidth( 1 );
      actWin->executeGc.setLineStyle( LineSolid );
      XDrawRectangle( actWin->d, drawable(actWin->executeWidget),
       actWin->executeGc.normGC(), x, y, w, h );
      actWin->executeGc.restoreFg();
      needToEraseUnconnected = 1;
      updateBlink( blink );
    }
  }
  else if ( needToEraseUnconnected ) {
    actWin->executeGc.setLineWidth( 1 );
    actWin->executeGc.setLineStyle( LineSolid );
    XDrawRectangle( actWin->d, drawable(actWin->executeWidget),
     actWin->executeGc.eraseGC(), x, y, w, h );
    needToEraseUnconnected = 0;
  }

  if ( !enabled || !activeMode || !visibility ) return 1;

  prevVisibility = visibility;

  actWin->executeGc.setLineStyle( lineStyle );
  actWin->executeGc.setLineWidth( lineWidth );
  actWin->executeGc.saveFg();

  if ( fill && fillVisibility ) {
    //actWin->executeGc.setFG( fillColor.getColor() );
    actWin->executeGc.setFG( fillColor.getIndex(), &blink );
    XFillArc( actWin->d, drawable(actWin->executeWidget),
     actWin->executeGc.normGC(), x, y, w, h, 0, 23040 );
  }

  if ( lineVisibility ) {
    //actWin->executeGc.setFG( lineColor.getColor() );
    actWin->executeGc.setFG( lineColor.getIndex(), &blink );
    XDrawArc( actWin->d, drawable(actWin->executeWidget),
     actWin->executeGc.normGC(), x, y, w, h, 0, 23040 );
  }

  actWin->executeGc.setLineStyle( LineSolid );
  actWin->executeGc.setLineWidth( 1 );
  actWin->executeGc.restoreFg();

  updateBlink( blink );

  return 1;

}

int activeCircleClass::eraseUnconditional ( void )
{

  if ( !enabled ) return 1;

  actWin->executeGc.setLineStyle( lineStyle );
  actWin->executeGc.setLineWidth( lineWidth );

  if ( fill ) {
    XFillArc( actWin->d, drawable(actWin->executeWidget),
     actWin->executeGc.eraseGC(), x, y, w, h, 0, 23040 );
  }

  XDrawArc( actWin->d, drawable(actWin->executeWidget),
   actWin->executeGc.eraseGC(), x, y, w, h, 0, 23040 );

  XFillArc( actWin->d, drawable(actWin->executeWidget),
   actWin->executeGc.eraseGC(), x, y, w, h, 0, 23040 );

  actWin->executeGc.setLineStyle( LineSolid );
  actWin->executeGc.setLineWidth( 1 );

  return 1;

}

int activeCircleClass::eraseActive ( void )
{

  if ( !enabled || !activeMode ) return 1;

  if ( prevVisibility == 0 ) {
    prevVisibility = visibility;
    return 1;
  }

  prevVisibility = visibility;

  actWin->executeGc.setLineStyle( lineStyle );
  actWin->executeGc.setLineWidth( lineWidth );

  if ( fill ) {
    XFillArc( actWin->d, drawable(actWin->executeWidget),
     actWin->executeGc.eraseGC(), x, y, w, h, 0, 23040 );
  }

  XDrawArc( actWin->d, drawable(actWin->executeWidget),
   actWin->executeGc.eraseGC(), x, y, w, h, 0, 23040 );

  XFillArc( actWin->d, drawable(actWin->executeWidget),
   actWin->executeGc.eraseGC(), x, y, w, h, 0, 23040 );

  actWin->executeGc.setLineStyle( LineSolid );
  actWin->executeGc.setLineWidth( 1 );

  return 1;

}

int activeCircleClass::expandTemplate (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

expStringClass tmpStr;

  tmpStr.setRaw( alarmPvExpStr.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  alarmPvExpStr.setRaw( tmpStr.getExpanded() );

  tmpStr.setRaw( visPvExpStr.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  visPvExpStr.setRaw( tmpStr.getExpanded() );

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

  switch ( pass ) {

  case 1: // initialize

    opComplete = 0;

    break;

  case 2: // connect to pv's

    if ( !opComplete ) {

      connection.init();
      initEnable();

      curLineColorIndex = -1;
      curFillColorIndex = -1;
      curStatus = -1;
      curSeverity = -1;
      prevVisibility = -1;
      visibility = 0;
      prevLineVisibility = -1;
      lineVisibility = 0;
      prevFillVisibility = -1;
      fillVisibility = 0;

      needConnectInit = needAlarmUpdate = needVisUpdate = needRefresh = 0;

      needToEraseUnconnected = 0;
      needToDrawUnconnected = 0;
      unconnectedTimer = 0;

      if ( !unconnectedTimer ) {
        unconnectedTimer = appAddTimeOut( actWin->appCtx->appContext(),
         2000, unconnectedTimeout, this );
      }

      aglPtr = ptr;

      alarmPvId = visPvId = 0;

      activeMode = 1;
      pvType = -1;

      init = 1; // this stays true if there are no pvs

      if ( !alarmPvExpStr.getExpanded() ||
           // ( strcmp( alarmPvExpStr.getExpanded(), "" ) == 0 ) ) {
           blankOrComment( alarmPvExpStr.getExpanded() ) ) {
        alarmPvExists = 0;
        lineVisibility = fillVisibility = 1;
      }
      else {
        connection.addPv();
        alarmPvExists = 1;
        lineColor.setConnectSensitive();
        fillColor.setConnectSensitive();
        init = 0;
      }

      if ( !visPvExpStr.getExpanded() ||
           // ( strcmp( visPvExpStr.getExpanded(), "" ) == 0 ) ) {
           blankOrComment( visPvExpStr.getExpanded() ) ) {
        visPvExists = 0;
        visibility = 1;
      }
      else {
        connection.addPv();
        visPvExists = 1;
        visibility = 0;
        lineVisibility = fillVisibility = 1;
        lineColor.setConnectSensitive();
        fillColor.setConnectSensitive();
        init = 0;
      }

      if ( alarmPvExists ) {
        alarmPvId = the_PV_Factory->create( alarmPvExpStr.getExpanded() );
        if ( alarmPvId ) {
          alarmPvId->add_conn_state_callback( alarmPvConnectStateCallback,
           this );
          alarmPvId->add_value_callback( alarmPvValueCallback, this );
	}
      }

      if ( visPvExists ) {
        visPvId = the_PV_Factory->create( visPvExpStr.getExpanded() );
        if ( visPvId ) {
          visPvId->add_conn_state_callback( visPvConnectStateCallback, this );
          visPvId->add_value_callback( visPvValueCallback, this );
	}
      }

      opComplete = 1;

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

  if ( pass == 1 ) {

    activeMode = 0;

    if ( unconnectedTimer ) {
      XtRemoveTimeOut( unconnectedTimer );
      unconnectedTimer = 0;
    }

    //updateBlink( 0 );

    if ( alarmPvId ) {
      alarmPvId->remove_conn_state_callback( alarmPvConnectStateCallback,
       this );
      alarmPvId->remove_value_callback( alarmPvValueCallback, this );
      alarmPvId->release();
      alarmPvId = 0;
    }

    if ( visPvId ) {
      visPvId->remove_conn_state_callback( visPvConnectStateCallback, this );
      visPvId->remove_value_callback( visPvValueCallback, this );
      visPvId->release();
      visPvId = 0;
    }

  }

  return 1;

}

int activeCircleClass::draw ( void ) {

int blink = 0;

  if ( activeMode ) return 1;
  if ( deleteRequest ) return 1;

  actWin->drawGc.setLineStyle( lineStyle );
  actWin->drawGc.setLineWidth( lineWidth );
  actWin->drawGc.saveFg();

  if ( fill ) {
    //actWin->drawGc.setFG( fillColor.pixelColor() );
    actWin->drawGc.setFG( fillColor.pixelIndex(), &blink );
    XFillArc( actWin->d, XtWindow(actWin->drawWidget), actWin->drawGc.normGC(),
     x, y, w, h, 0, 23040 );
  }

  //actWin->drawGc.setFG( lineColor.pixelColor() );
  actWin->drawGc.setFG( lineColor.pixelIndex(), &blink );
  XDrawArc( actWin->d, XtWindow(actWin->drawWidget), actWin->drawGc.normGC(),
   x, y, w, h, 0, 23040 );

  actWin->drawGc.setLineStyle( LineSolid );
  actWin->drawGc.setLineWidth( 1 );
  actWin->drawGc.restoreFg();

  updateBlink( blink );

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

int stat, nc, nau, nvu, nr, index, change;
pvValType pvV;

  if ( actWin->isIconified ) return;

  actWin->appCtx->proc->lock();
  nc = needConnectInit; needConnectInit = 0;
  nau = needAlarmUpdate; needAlarmUpdate = 0;
  nvu = needVisUpdate; needVisUpdate = 0;
  nr = needRefresh; needRefresh = 0;
  actWin->remDefExeNode( aglPtr );
  actWin->appCtx->proc->unlock();

  if ( !activeMode ) return;

  if ( nc ) {

    minVis.d = (double) atof( minVisString );
    maxVis.d = (double) atof( maxVisString );

    lineColor.setConnected();
    fillColor.setConnected();

    if ( alarmPvExists ) {

      curStatus = alarmPvId->get_status();
      curSeverity = alarmPvId->get_severity();

      lineColor.setStatus( curStatus, curSeverity );
      fillColor.setStatus( curStatus, curSeverity );

      curLineColorIndex = actWin->ci->evalRule( lineColor.pixelIndex(),
       alarmPvId->get_double() );
      lineColor.changeIndex( curLineColorIndex, actWin->ci );

      curFillColorIndex = actWin->ci->evalRule( fillColor.pixelIndex(),
       alarmPvId->get_double() );
      fillColor.changeIndex( curFillColorIndex, actWin->ci );

      if ( !visPvExists ) {

        if ( actWin->ci->isInvisible( curLineColorIndex ) ) {
          prevLineVisibility = lineVisibility = 0;
        }
        else {
          prevLineVisibility = lineVisibility = 1;
        }

        if ( actWin->ci->isInvisible( curFillColorIndex ) ) {
          prevFillVisibility = fillVisibility = 0;
        }
        else {
          prevFillVisibility = fillVisibility = 1;
        }

      }

    }

    if ( visPvExists ) {

      pvV.d = visPvId->get_double();
      if ( ( pvV.d >= minVis.d ) && ( pvV.d < maxVis.d ) )
        visibility = 1 ^ visInverted;
      else
        visibility = 0 ^ visInverted;

      prevVisibility = visibility;

    }

    init = 1;

    eraseUnconditional();
    stat = smartDrawAllActive();

  }

  if ( nau ) {

    change = 0;

    if ( curStatus != alarmPvId->get_status() ) {
      curStatus = alarmPvId->get_status();
      change = 1;
    }

    if ( curSeverity != alarmPvId->get_severity() ) {
      curSeverity = alarmPvId->get_severity();
      change = 1;
    }

    if ( change ) {
      lineColor.setStatus( curStatus, curSeverity );
      fillColor.setStatus( curStatus, curSeverity );
    }

    index = actWin->ci->evalRule( lineColor.pixelIndex(),
    alarmPvId->get_double() );

    if ( curLineColorIndex != index ) {
      curLineColorIndex = index;
      change = 1;
    }

    index = actWin->ci->evalRule( fillColor.pixelIndex(),
    alarmPvId->get_double() );

    if ( curFillColorIndex != index ) {
      curFillColorIndex = index;
      change = 1;
    }

    if ( change ) {

      if ( !visPvExists ) {

        if ( actWin->ci->isInvisible( curLineColorIndex ) ) {
          lineVisibility = 0;
        }
        else {
          lineVisibility = 1;
        }

        if ( actWin->ci->isInvisible( curFillColorIndex ) ) {
          fillVisibility = 0;
        }
        else {
          fillVisibility = 1;
        }

      }

      lineColor.changeIndex( curLineColorIndex, actWin->ci );
      fillColor.changeIndex( curFillColorIndex, actWin->ci );
      if ( ( prevLineVisibility != lineVisibility ) ||
	   ( prevFillVisibility != fillVisibility ) ) {
	prevLineVisibility = lineVisibility;
	prevFillVisibility = fillVisibility;
        eraseActive();
      }
      smartDrawAllActive();

    }

  }

  if ( nvu ) {

    pvV.d = visPvId->get_double();
    if ( ( pvV.d >= minVis.d ) && ( pvV.d < maxVis.d ) )
      visibility = 1 ^ visInverted;
    else
      visibility = 0 ^ visInverted;

    if ( prevVisibility != visibility ) {
      if ( !visibility ) eraseActive();
      stat = smartDrawAllActive();
    }

  }

  if ( nr ) {
    stat = smartDrawAllActive();
  }

}

char *activeCircleClass::firstDragName ( void ) {

#define MAXDRAGNAMES 2

int i;
int present[MAXDRAGNAMES];

  if ( !enabled ) return NULL;

  if ( actWin->mode == AWC_EXECUTE ) {

    if ( !blank( alarmPvExpStr.getExpanded() ) ) {
      present[0] = 1;
    }
    else {
      present[0] = 0;
    }

    if ( !blank( visPvExpStr.getExpanded() ) ) {
      present[1] = 1;
    }
    else {
      present[1] = 0;
    }

  }
  else {

    if ( !blank( alarmPvExpStr.getRaw() ) ) {
      present[0] = 1;
    }
    else {
      present[0] = 0;
    }

    if ( !blank( visPvExpStr.getRaw() ) ) {
      present[1] = 1;
    }
    else {
      present[1] = 0;
    }

  }

  for ( i=0; i<MAXDRAGNAMES; i++ ) {
    if ( present[i] ) {
      dragIndex = i;
      return dragName[dragIndex];
    }
  }

  dragIndex = 0;
  return dragName[dragIndex];

}

char *activeCircleClass::nextDragName ( void ) {

  if ( !enabled ) return NULL;

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

int offset = 0;

  if ( !enabled ) return NULL;

  if ( actWin->mode == AWC_EXECUTE ) {

    if ( blank( alarmPvExpStr.getExpanded() ) ) {
      offset++;
      if ( blank( visPvExpStr.getExpanded() ) ) {
        offset++;
      }
    }

    switch ( i+offset ) {

    case 0:
      return alarmPvExpStr.getExpanded();

    case 1:
      return visPvExpStr.getExpanded();

    }

  }
  else {

    if ( blank( alarmPvExpStr.getRaw() ) ) {
      offset++;
      if ( blank( visPvExpStr.getRaw() ) ) {
        offset++;
      }
    }

    switch ( i+offset ) {

    case 0:
      return alarmPvExpStr.getRaw();

    case 1:
      return visPvExpStr.getRaw();

    }

  }

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
  int _textFgColor,
  int _fg1Color,
  int _fg2Color,
  int _offsetColor,
  int _bgColor,
  int _topShadowColor,
  int _botShadowColor )
{

  if ( _flag & ACTGRF_FG1COLOR_MASK )
    lineColor.setColorIndex( _fg1Color, actWin->ci );

  if ( _flag & ACTGRF_BGCOLOR_MASK )
    fillColor.setColorIndex( _bgColor, actWin->ci );

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

void activeCircleClass::updateColors (
  double colorValue )
{

int index, change=0;

  index = actWin->ci->evalRule( lineColor.pixelIndex(), colorValue );

  if ( curLineColorIndex != index ) {
    curLineColorIndex = index;
    change = 1;
  }

  index = actWin->ci->evalRule( fillColor.pixelIndex(), colorValue );

  if ( curFillColorIndex != index ) {
    curFillColorIndex = index;
    change = 1;
  }

  if ( change ) {

    if ( actWin->ci->isInvisible( curLineColorIndex ) ) {
      lineVisibility = 0;
    }
    else {
      lineVisibility = 1;
    }

    if ( actWin->ci->isInvisible( curFillColorIndex ) ) {
      fillVisibility = 0;
    }
    else {
      fillVisibility = 1;
    }

    lineColor.changeIndex( curLineColorIndex, actWin->ci );
    fillColor.changeIndex( curFillColorIndex, actWin->ci );
    if ( ( prevLineVisibility != lineVisibility ) ||
         ( prevFillVisibility != fillVisibility ) ) {
      prevLineVisibility = lineVisibility;
      prevFillVisibility = fillVisibility;
    }

  }

}

void activeCircleClass::getPvs (
  int max,
  ProcessVariable *pvs[],
  int *n ) {

  if ( max < 2 ) {
    *n = 0;
    return;
  }

  *n = 2;
  pvs[0] = alarmPvId;
  pvs[1] = visPvId;

}

char *activeCircleClass::getSearchString (
  int i
) {

  if ( i == 0 ) {
    return alarmPvExpStr.getRaw();
  }
  else if ( i == 1 ) {
    return visPvExpStr.getRaw();
  }
  else if ( i == 2 ) {
    return minVisString;
  }
  else if ( i == 3 ) {
    return maxVisString;
  }
  else {
    return NULL;
  }

}

void activeCircleClass::replaceString (
  int i,
  int max,
  char *string
) {

  if ( i == 0 ) {
    alarmPvExpStr.setRaw( string );
  }
  else if ( i == 1 ) {
    visPvExpStr.setRaw( string );
  }
  else if ( i == 2 ) {
    int l = max;
    if ( 39 < max ) l = 39;
    strncpy( minVisString, string, l );
    minVisString[l] = 0;
  }
  else if ( i == 3 ) {
    int l = max;
    if ( 39 < max ) l = 39;
    strncpy( maxVisString, string, l );
    maxVisString[l] = 0;
  }

}

// crawler functions may return blank pv names
char *activeCircleClass::crawlerGetFirstPv ( void ) {

  crawlerPvIndex = 0;
  return alarmPvExpStr.getExpanded();

}

char *activeCircleClass::crawlerGetNextPv ( void ) {

  if ( crawlerPvIndex >= 1 ) return NULL;
  crawlerPvIndex++;
  return visPvExpStr.getExpanded();

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
