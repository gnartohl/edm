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

#define __arc_obj_cc 1

#include "arc_obj.h"
#include "app_pkg.h"
#include "act_win.h"

#include "thread.h"

#ifdef __epics__

static void aaoMonitorAlarmPvConnectState (
  struct connection_handler_args arg )
{

activeArcClass *aao = (activeArcClass *) ca_puser(arg.chid);

  if ( arg.op == CA_OP_CONN_UP ) {

    aao->needAlarmConnectInit = 1;

  }
  else { // lost connection

    aao->alarmPvConnected = 0;
    aao->active = 0;
    aao->lineColor.setDisconnected();
    aao->fillColor.setDisconnected();
    aao->bufInvalidate();
    aao->needDraw = 1;

  }

  aao->actWin->appCtx->proc->lock();
  aao->actWin->addDefExeNode( aao->aglPtr );
  aao->actWin->appCtx->proc->unlock();

}

static void arcAlarmUpdate (
  struct event_handler_args ast_args )
{

class activeArcClass *aao;
struct dbr_sts_float statusRec;

  aao = (activeArcClass *) ast_args.usr;

  statusRec = *( (struct dbr_sts_float *) ast_args.dbr );
  aao->lineColor.setStatus( statusRec.status, statusRec.severity );
  aao->fillColor.setStatus( statusRec.status, statusRec.severity );

  if ( aao->active ) {
    aao->bufInvalidate();
    aao->needRefresh = 1;
    aao->actWin->appCtx->proc->lock();
    aao->actWin->addDefExeNode( aao->aglPtr );
    aao->actWin->appCtx->proc->unlock();
  }

}

static void aaoMonitorVisPvConnectState (
  struct connection_handler_args arg )
{

activeArcClass *aao = (activeArcClass *) ca_puser(arg.chid);

  if ( arg.op == CA_OP_CONN_UP ) {

    aao->needVisConnectInit = 1;

  }
  else { // lost connection

    aao->visPvConnected = 0;
    aao->active = 0;
    aao->lineColor.setDisconnected();
    aao->fillColor.setDisconnected();
    aao->bufInvalidate();
    aao->needDraw = 1;

  }

  aao->actWin->appCtx->proc->lock();
  aao->actWin->addDefExeNode( aao->aglPtr );
  aao->actWin->appCtx->proc->unlock();

}

static void arcVisUpdate (
  struct event_handler_args ast_args )
{

pvValType pvV;
class activeArcClass *aao = (activeArcClass *) ast_args.usr;

  pvV.d = *( (double *) ast_args.dbr );
  if ( ( pvV.d >= aao->minVis.d ) && ( pvV.d < aao->maxVis.d ) )
    aao->visibility = 1 ^ aao->visInverted;
  else
    aao->visibility = 0 ^ aao->visInverted;

  if ( aao->active ) {

    if ( aao->visibility ) {

      aao->needRefresh = 1;

    }
    else {

      aao->needErase = 1;
      aao->needRefresh = 1;

    }

    aao->actWin->appCtx->proc->lock();
    aao->actWin->addDefExeNode( aao->aglPtr );
    aao->actWin->appCtx->proc->unlock();

  }

}

#endif

static void aac_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeArcClass *aao = (activeArcClass *) client;

  aao->actWin->setChanged();

  aao->eraseSelectBoxCorners();
  aao->erase();

  aao->fill = aao->bufFill;

  aao->lineColorMode = aao->bufLineColorMode;
  if ( aao->lineColorMode == AAC_K_COLORMODE_ALARM )
    aao->lineColor.setAlarmSensitive();
  else
    aao->lineColor.setAlarmInsensitive();
  aao->lineColor.setColorIndex( aao->bufLineColor, aao->actWin->ci );

  aao->fillColorMode = aao->bufFillColorMode;
  if ( aao->fillColorMode == AAC_K_COLORMODE_ALARM )
    aao->fillColor.setAlarmSensitive();
  else
    aao->fillColor.setAlarmInsensitive();
  aao->fillColor.setColorIndex( aao->bufFillColor, aao->actWin->ci );

  aao->lineWidth = aao->bufLineWidth;

  if ( aao->bufLineStyle == 0 )
    aao->lineStyle = LineSolid;
  else if ( aao->bufLineStyle == 1 )
    aao->lineStyle = LineOnOffDash;

  aao->alarmPvExpStr.setRaw( aao->bufAlarmPvName );

  aao->visPvExpStr.setRaw( aao->bufVisPvName );

  if ( aao->bufVisInverted )
    aao->visInverted = 0;
  else
    aao->visInverted = 1;

  strncpy( aao->minVisString, aao->bufMinVisString, 39 );
  strncpy( aao->maxVisString, aao->bufMaxVisString, 39 );

  aao->efStartAngle = aao->bufEfStartAngle;
  if ( aao->efStartAngle.isNull() ) {
    aao->startAngle = 0;
  }
  else {
    aao->startAngle = (int) ( aao->efStartAngle.value() * 64.0 +0.5 );
  }

  aao->efTotalAngle = aao->bufEfTotalAngle;
  if ( aao->efTotalAngle.isNull() ) {
    aao->totalAngle = 180 * 64;
  }
  else {
    aao->totalAngle = (int) ( aao->efTotalAngle.value() * 64.0 +0.5 );
  }

  aao->fillMode = aao->bufFillMode;

  aao->x = aao->bufX;
  aao->sboxX = aao->bufX;

  aao->y = aao->bufY;
  aao->sboxY = aao->bufY;

  aao->w = aao->bufW;
  aao->sboxW = aao->bufW;

  aao->h = aao->bufH;
  aao->sboxH = aao->bufH;

}

static void aac_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeArcClass *aao = (activeArcClass *) client;

  aac_edit_update( w, client, call );
  aao->refresh( aao );

}

static void aac_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeArcClass *aao = (activeArcClass *) client;

  aac_edit_update( w, client, call );
  aao->ef.popdown();
  aao->operationComplete();

}

static void aac_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeArcClass *aao = (activeArcClass *) client;

  aao->ef.popdown();
  aao->operationCancel();

}

static void aac_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeArcClass *aao = (activeArcClass *) client;

  aao->erase();
  aao->deleteRequest = 1;
  aao->ef.popdown();
  aao->operationCancel();
  aao->drawAll();

}

activeArcClass::activeArcClass ( void ) {

  name = new char[strlen("activeArcClass")+1];
  strcpy( name, "activeArcClass" );
  visibility = 0;
  prevVisibility = -1;
  visInverted = 0;
  visPvConnected = alarmPvConnected = 0;
  visPvExists = alarmPvExists = 0;
  active = 0;
  activeMode = 0;
  fill = 0;
  lineColorMode = AAC_K_COLORMODE_STATIC;
  fillColorMode = AAC_K_COLORMODE_STATIC;
  lineWidth = 1;
  lineStyle = LineSolid;
  strcpy( minVisString, "" );
  strcpy( maxVisString, "" );
  efStartAngle.setNull(1);
  startAngle = 0;
  efTotalAngle.setNull(1);
  totalAngle = 180 * 64;
  fillMode = 0;

}

// copy constructor
activeArcClass::activeArcClass
( const activeArcClass *source ) {

activeGraphicClass *ago = (activeGraphicClass *) this;

  ago->clone( (activeGraphicClass *) source );

  name = new char[strlen("activeArcClass")+1];
  strcpy( name, "activeArcClass" );

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

  efStartAngle = source->efStartAngle;
  if ( efStartAngle.isNull() ) {
    startAngle = 0;
  }
  else {
    startAngle = (int) ( efStartAngle.value() * 64.0 +0.5 );
  }

  efTotalAngle = source->efTotalAngle;
  if ( efTotalAngle.isNull() ) {
    totalAngle = 180 * 64;
  }
  else {
    totalAngle = (int) ( efTotalAngle.value() * 64.0 +0.5 );
  }

  fillMode = source->fillMode;

}

int activeArcClass::createInteractive (
  activeWindowClass *aw_obj,
  int _x,
  int _y,
  int _w,
  int _h ) {

//   printf( "In activeArcClass::createInteractive\n" );

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

int activeArcClass::genericEdit ( void ) {

char title[32], *ptr;

  ptr = actWin->obj.getNameFromClass( "activeArcClass" );
  if ( ptr )
    strncpy( title, ptr, 31 );
  else
    strncpy( title, activeArcClass_str4, 31 );

  strncat( title, activeArcClass_str5, 31 );

  bufX = x;
  bufY = y;
  bufW = w;
  bufH = h;

  bufLineColor = lineColor.pixelIndex();
  bufLineColorMode = lineColorMode;

  bufFillColor = fillColor.pixelIndex();
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

  bufEfStartAngle = efStartAngle;
  bufEfTotalAngle = efTotalAngle;
  bufFillMode = fillMode;

  ef.create( actWin->top, actWin->appCtx->ci.getColorMap(),
   &actWin->appCtx->entryFormX,
   &actWin->appCtx->entryFormY, &actWin->appCtx->entryFormW,
   &actWin->appCtx->entryFormH, &actWin->appCtx->largestH,
   title, NULL, NULL, NULL );

  ef.addTextField( activeArcClass_str6, 25, &bufX );
  ef.addTextField( activeArcClass_str7, 25, &bufY );
  ef.addTextField( activeArcClass_str8, 25, &bufW );
  ef.addTextField( activeArcClass_str9, 25, &bufH );
  ef.addTextField( activeArcClass_str10, 25, &bufEfStartAngle );
  ef.addTextField( activeArcClass_str11, 25, &bufEfTotalAngle );
  ef.addOption( activeArcClass_str12, activeArcClass_str13, &bufLineWidth );
  ef.addOption( activeArcClass_str14, activeArcClass_str15, &bufLineStyle );
  ef.addColorButton( activeArcClass_str16, actWin->ci, &lineCb, &bufLineColor );
  ef.addToggle( activeArcClass_str17, &bufLineColorMode );
  ef.addToggle( activeArcClass_str18, &bufFill );
  ef.addOption( activeArcClass_str19, activeArcClass_str20, &bufFillMode );
  ef.addColorButton( activeArcClass_str21, actWin->ci, &fillCb, &bufFillColor );
  ef.addToggle( activeArcClass_str22, &bufFillColorMode );
  ef.addTextField( activeArcClass_str23, 27, bufAlarmPvName, 39 );
  ef.addTextField( activeArcClass_str24, 27, bufVisPvName, 39 );
  ef.addOption( " ", activeArcClass_str25, &bufVisInverted );
  ef.addTextField( activeArcClass_str26, 27, bufMinVisString, 39 );
  ef.addTextField( activeArcClass_str27, 27, bufMaxVisString, 39 );

  return 1;

}

int activeArcClass::editCreate ( void ) {

  this->genericEdit();
  ef.finished( aac_edit_ok, aac_edit_apply, aac_edit_cancel_delete, this );
  actWin->currentEf = NULL;
  ef.popup();

  return 1;

}

int activeArcClass::edit ( void ) {

  this->genericEdit();
  ef.finished( aac_edit_ok, aac_edit_apply, aac_edit_cancel, this );
  actWin->currentEf = &ef;
  ef.popup();

  return 1;

}

int activeArcClass::createFromFile (
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
    lineColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d\n", &lineColorMode ); actWin->incLine();

    if ( lineColorMode == AAC_K_COLORMODE_ALARM )
      lineColor.setAlarmSensitive();
    else
      lineColor.setAlarmInsensitive();

    fscanf( f, "%d\n", &fill ); actWin->incLine();

    fscanf( f, "%d\n", &index ); actWin->incLine();
    fillColor.setColorIndex( index, actWin->ci );

  }
  else {

    fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
    actWin->ci->setRGB( r, g, b, &pixel );
    index = actWin->ci->pixIndex( pixel );
    lineColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d\n", &lineColorMode ); actWin->incLine();

    if ( lineColorMode == AAC_K_COLORMODE_ALARM )
      lineColor.setAlarmSensitive();
    else
      lineColor.setAlarmInsensitive();

    fscanf( f, "%d\n", &fill ); actWin->incLine();

    fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
    actWin->ci->setRGB( r, g, b, &pixel );
    index = actWin->ci->pixIndex( pixel );
    fillColor.setColorIndex( index, actWin->ci );

  }

  fscanf( f, "%d\n", &fillColorMode ); actWin->incLine();

  if ( fillColorMode == AAC_K_COLORMODE_ALARM )
    fillColor.setAlarmSensitive();
  else
    fillColor.setAlarmInsensitive();

  readStringFromFile( oneName, 39, f ); actWin->incLine();
  alarmPvExpStr.setRaw( oneName );

  readStringFromFile( oneName, 39, f ); actWin->incLine();
  visPvExpStr.setRaw( oneName );

  fscanf( f, "%d\n", &visInverted ); actWin->incLine();

  readStringFromFile( minVisString, 39, f ); actWin->incLine();
  readStringFromFile( maxVisString, 39, f ); actWin->incLine();

  fscanf( f, "%d\n", &lineWidth ); actWin->incLine();
  fscanf( f, "%d\n", &lineStyle ); actWin->incLine();

  efStartAngle.read( f ); actWin->incLine();
  if ( efStartAngle.isNull() ) {
    startAngle = 0;
  }
  else {
    startAngle = (int) ( efStartAngle.value() * 64.0 +0.5 );
  }

  efTotalAngle.read( f ); actWin->incLine();
  if ( efTotalAngle.isNull() ) {
    totalAngle = 180 * 64;
  }
  else {
    totalAngle = (int) ( efTotalAngle.value() * 64.0 +0.5 );
  }

  fscanf( f, "%d\n", &fillMode ); actWin->incLine();

  return 1;

}

int activeArcClass::importFromXchFile (
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
      actWin->appCtx->postMessage( activeArcClass_str32 );
      return 0;
    }

    context = NULL;

    tk = strtok_r( buf, " \t\n", &context );
    if ( !tk ) {
      actWin->appCtx->postMessage( activeArcClass_str32 );
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
          actWin->appCtx->postMessage( activeArcClass_str32 );
          return 0;
        }

        x = atol( tk );

      }
      else if ( strcmp( tk, "y" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( activeArcClass_str32 );
          return 0;
        }

        y = atol( tk );

      }
      else if ( strcmp( tk, "w" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( activeArcClass_str32 );
          return 0;
        }

        w = atol( tk );

      }
      else if ( strcmp( tk, "h" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( activeArcClass_str32 );
          return 0;
        }

        h = atol( tk );

      }
            
      else if ( strcmp( tk, "fgred" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( activeArcClass_str32 );
          return 0;
        }

        fgR = atol( tk );

      }
            
      else if ( strcmp( tk, "fggreen" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( activeArcClass_str32 );
          return 0;
        }

        fgG = atol( tk );

      }
            
      else if ( strcmp( tk, "fgblue" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( activeArcClass_str32 );
          return 0;
        }

        fgB = atol( tk );

      }
            
      else if ( strcmp( tk, "bgred" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( activeArcClass_str32 );
          return 0;
        }

        bgR = atol( tk );

      }
            
      else if ( strcmp( tk, "bggreen" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( activeArcClass_str32 );
          return 0;
        }

        bgG = atol( tk );

      }
            
      else if ( strcmp( tk, "bgblue" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( activeArcClass_str32 );
          return 0;
        }

        bgB = atol( tk );

      }
            
      else if ( strcmp( tk, "linewidth" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( activeArcClass_str32 );
          return 0;
        }

       lineWidth  = atol( tk );

      }
            
      else if ( strcmp( tk, "fill" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( activeArcClass_str32 );
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

int activeArcClass::save (
  FILE *f )
{

int index;

  fprintf( f, "%-d %-d %-d\n", AAC_MAJOR_VERSION, AAC_MINOR_VERSION,
   AAC_RELEASE );
  fprintf( f, "%-d\n", x );
  fprintf( f, "%-d\n", y );
  fprintf( f, "%-d\n", w );
  fprintf( f, "%-d\n", h );

  index = lineColor.pixelIndex();
  fprintf( f, "%-d\n", index );

  fprintf( f, "%-d\n", lineColorMode );

  fprintf( f, "%-d\n", fill );

  index = fillColor.pixelIndex();
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

  efStartAngle.write( f );
  efTotalAngle.write( f );
  fprintf( f, "%-d\n", fillMode );

  return 1;

}

int activeArcClass::drawActive ( void )
{

  if ( !activeMode || !visibility ) return 1;

  prevVisibility = visibility;

  actWin->executeGc.setLineStyle( lineStyle );
  actWin->executeGc.setLineWidth( lineWidth );
  actWin->executeGc.saveFg();

  if ( fill ) {
    if ( fillMode ) {
      actWin->executeGc.setArcModePieSlice();
    }
    else {
      actWin->executeGc.setArcModeChord();
    }
    actWin->executeGc.setFG( fillColor.getColor() );
    XFillArc( actWin->d, XtWindow(actWin->executeWidget),
     actWin->executeGc.normGC(), x, y, w, h, startAngle, totalAngle );
  }

  actWin->executeGc.setFG( lineColor.getColor() );
  XDrawArc( actWin->d, XtWindow(actWin->executeWidget),
   actWin->executeGc.normGC(), x, y, w, h, startAngle, totalAngle );

  actWin->executeGc.setLineStyle( LineSolid );
  actWin->executeGc.setLineWidth( 1 );
  actWin->executeGc.restoreFg();

  return 1;

}

int activeArcClass::eraseUnconditional ( void )
{

  actWin->executeGc.setLineStyle( lineStyle );
  actWin->executeGc.setLineWidth( lineWidth );

  if ( fillMode ) {
    actWin->executeGc.setArcModePieSlice();
  }
  else {
    actWin->executeGc.setArcModeChord();
  }

  XDrawArc( actWin->d, XtWindow(actWin->executeWidget),
   actWin->executeGc.eraseGC(), x, y, w, h, startAngle, totalAngle );

  XFillArc( actWin->d, XtWindow(actWin->executeWidget),
   actWin->executeGc.eraseGC(), x, y, w, h, startAngle, totalAngle );

  actWin->executeGc.setLineStyle( LineSolid );
  actWin->executeGc.setLineWidth( 1 );

  return 1;

}

int activeArcClass::eraseActive ( void )
{

  if ( !activeMode ) return 1;

  if ( prevVisibility == 0 ) {
    prevVisibility = visibility;
    return 1;
  }

  prevVisibility = visibility;

  actWin->executeGc.setLineStyle( lineStyle );
  actWin->executeGc.setLineWidth( lineWidth );

  if ( fillMode ) {
    actWin->executeGc.setArcModePieSlice();
  }
  else {
    actWin->executeGc.setArcModeChord();
  }

  XDrawArc( actWin->d, XtWindow(actWin->executeWidget),
   actWin->executeGc.eraseGC(), x, y, w, h, startAngle, totalAngle );

  XFillArc( actWin->d, XtWindow(actWin->executeWidget),
   actWin->executeGc.eraseGC(), x, y, w, h, startAngle, totalAngle );

  actWin->executeGc.setLineStyle( LineSolid );
  actWin->executeGc.setLineWidth( 1 );

  return 1;

}

int activeArcClass::expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int stat;

  stat = alarmPvExpStr.expand1st( numMacros, macros, expansions );
  stat = visPvExpStr.expand1st( numMacros, macros, expansions );

  return stat;

}

int activeArcClass::expand2nd (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int stat;

  stat = alarmPvExpStr.expand2nd( numMacros, macros, expansions );
  stat = visPvExpStr.expand2nd( numMacros, macros, expansions );

  return stat;

}

int activeArcClass::containsMacros ( void ) {

  if ( alarmPvExpStr.containsPrimaryMacros() ) return 1;
  if ( visPvExpStr.containsPrimaryMacros() ) return 1;

  return 0;

}

int activeArcClass::activate (
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
         aaoMonitorAlarmPvConnectState, this );
        if ( stat != ECA_NORMAL ) {
          printf( activeArcClass_str28 );
          return 0;
        }
      }

      if ( visPvExists ) {
        stat = ca_search_and_connect( visPvExpStr.getExpanded(), &visPvId,
         aaoMonitorVisPvConnectState, this );
        if ( stat != ECA_NORMAL ) {
          printf( activeArcClass_str28 );
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

int activeArcClass::deactivate (
  int pass
) {

int stat;

  activeMode = 0;

  if ( pass == 1 ) {

#ifdef __epics__

  if ( alarmPvExists ) {
    stat = ca_clear_channel( alarmPvId );
    if ( stat != ECA_NORMAL )
      printf( activeArcClass_str30 );
  }

  if ( visPvExists ) {
    stat = ca_clear_channel( visPvId );
    if ( stat != ECA_NORMAL )
      printf( activeArcClass_str30 );
  }

#endif

  }

  return 1;

}

int activeArcClass::draw ( void ) {

  if ( activeMode ) return 1;
  if ( deleteRequest ) return 1;

  actWin->drawGc.setLineStyle( lineStyle );
  actWin->drawGc.setLineWidth( lineWidth );
  actWin->drawGc.saveFg();

  if ( fill ) {
    if ( fillMode ) {
      actWin->drawGc.setArcModePieSlice();
    }
    else {
      actWin->drawGc.setArcModeChord();
    }
    actWin->drawGc.setFG( fillColor.pixelColor() );
    XFillArc( actWin->d, XtWindow(actWin->drawWidget), actWin->drawGc.normGC(),
     x, y, w, h, startAngle, totalAngle );
  }

  actWin->drawGc.setFG( lineColor.pixelColor() );
  XDrawArc( actWin->d, XtWindow(actWin->drawWidget), actWin->drawGc.normGC(),
   x, y, w, h, startAngle, totalAngle );

  actWin->drawGc.setLineStyle( LineSolid );
  actWin->drawGc.setLineWidth( 1 );
  actWin->drawGc.restoreFg();

  return 1;

}

int activeArcClass::erase ( void ) {

  if ( activeMode ) return 1;
  if ( deleteRequest ) return 1;

  actWin->drawGc.setLineStyle( lineStyle );
  actWin->drawGc.setLineWidth( lineWidth );

  if ( fill ) {
    if ( fillMode ) {
      actWin->drawGc.setArcModePieSlice();
    }
    else {
      actWin->drawGc.setArcModeChord();
    }
    XFillArc( actWin->d, XtWindow(actWin->drawWidget),
     actWin->drawGc.eraseGC(), x, y, w, h, startAngle, totalAngle );
  }

  XDrawArc( actWin->d, XtWindow(actWin->drawWidget), actWin->drawGc.eraseGC(),
   x, y, w, h, startAngle, totalAngle );

  actWin->drawGc.setLineStyle( LineSolid );
  actWin->drawGc.setLineWidth( 1 );

  return 1;

}

void activeArcClass::executeDeferred ( void ) {

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
         arcVisUpdate, (void *) this, (float) 0.0, (float) 0.0,
         (float) 0.0, &visEventId, DBE_VALUE );
        if ( stat != ECA_NORMAL ) {
          printf( activeArcClass_str31 );
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
       arcAlarmUpdate, (void *) this, (float) 0.0, (float) 0.0,
       (float) 0.0, &alarmEventId, DBE_ALARM );
      if ( stat != ECA_NORMAL ) {
        printf( activeArcClass_str31 );
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

char *activeArcClass::firstDragName ( void ) {

  dragIndex = 0;
  return dragName[dragIndex];

}

char *activeArcClass::nextDragName ( void ) {

  if ( dragIndex < (int) ( sizeof(dragName) / sizeof(char *) ) - 1 ) {
    dragIndex++;
    return dragName[dragIndex];
  }
  else {
    return NULL;
  }

}

char *activeArcClass::dragValue (
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

void activeArcClass::changeDisplayParams (
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

void activeArcClass::changePvNames (
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

void *create_activeArcClassPtr ( void ) {

activeArcClass *ptr;

  ptr = new activeArcClass;
  return (void *) ptr;

}

void *clone_activeArcClassPtr (
  void *_srcPtr )
{

activeArcClass *ptr, *srcPtr;

  srcPtr = (activeArcClass *) _srcPtr;

  ptr = new activeArcClass( srcPtr );

  return (void *) ptr;

}

#ifdef __cplusplus
}
#endif
