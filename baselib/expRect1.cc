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

#define __expRect1_cc 1

#include "expRect1.h"
#include "app_pkg.h"
#include "act_win.h"

#include "thread.h"

// This is the EPICS specific line right now:
static PV_Factory *pv_factory = new EPICS_PV_Factory();

#ifdef __epics__

static void bgRuleConnectCb (
  void *ptr,
  int ruleId,
  int op
) {

expRectClass *aro = (expRectClass *) ptr;

  if ( op == CA_OP_CONN_UP ) {
  }

}

static void bgRuleCb (
  void *ptr,
  int ruleId,
  int value
) {

expRectClass *aro = (expRectClass *) ptr;

  aro->bufInvalidate();
  if ( value != -1 ) {
    //printf( "set special color\n" );
    aro->bgRuleColor = aro->actWin->ci->getPixelByIndex( value );
    aro->useBgRule = 1;
  }
  else {
    //printf( "set normal color\n" );
    aro->useBgRule = 0;
  }
  aro->needColorUpdate = 1;
  aro->actWin->addDefExeNode( aro->aglPtr );

}

void aroMonitorAlarmPvConnectState (
  struct connection_handler_args arg )
{

expRectClass *aro = (expRectClass *) ca_puser(arg.chid);

  if ( arg.op == CA_OP_CONN_UP ) {

    aro->needAlarmConnectInit = 1;

  }
  else { // lost connection

    aro->alarmPvConnected = 0;
    aro->active = 0;
    aro->lineColor.setDisconnected();
    aro->fillColor.setDisconnected();
    aro->bufInvalidate();
    aro->needDraw = 1;

  }

  aro->actWin->addDefExeNode( aro->aglPtr );

}

void rectangleAlarmUpdate (
  struct event_handler_args ast_args )
{

class expRectClass *aro;
struct dbr_sts_float statusRec;

  aro = (expRectClass *) ast_args.usr;

  statusRec = *( (struct dbr_sts_float *) ast_args.dbr );
  aro->lineColor.setStatus( statusRec.status, statusRec.severity );
  aro->fillColor.setStatus( statusRec.status, statusRec.severity );

  if ( aro->active ) {
    aro->bufInvalidate();
    aro->needRefresh = 1;
    aro->actWin->addDefExeNode( aro->aglPtr );
  }

}

void aroMonitorVisPvConnectState (
  struct connection_handler_args arg )
{

expRectClass *aro = (expRectClass *) ca_puser(arg.chid);

  if ( arg.op == CA_OP_CONN_UP ) {

    aro->needVisConnectInit = 1;

  }
  else { // lost connection

    aro->visPvConnected = 0;
    aro->active = 0;
    aro->lineColor.setDisconnected();
    aro->fillColor.setDisconnected();
    aro->bufInvalidate();
    aro->needDraw = 1;

  }

  aro->actWin->addDefExeNode( aro->aglPtr );

}

void rectangleVisUpdate (
  struct event_handler_args ast_args )
{

pvValType pvV;
class expRectClass *aro = (expRectClass *) ast_args.usr;

  pvV.d = *( (double *) ast_args.dbr );
  if ( ( pvV.d >= aro->minVis.d ) && ( pvV.d < aro->maxVis.d ) )
    aro->visibility = 1 ^ aro->visInverted;
  else
    aro->visibility = 0 ^ aro->visInverted;

  if ( aro->active ) {

    if ( aro->visibility ) {

      aro->needRefresh = 1;

    }
    else {

      aro->needErase = 1;
      aro->needRefresh = 1;

    }

    aro->actWin->addDefExeNode( aro->aglPtr );

  }

}

#endif

void arc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call )
{

expRectClass *aro = (expRectClass *) client;

  aro->actWin->setChanged();

  aro->eraseSelectBoxCorners();
  aro->erase();

  aro->lineColorMode = aro->bufLineColorMode;
  if ( aro->lineColorMode == ARC_K_COLORMODE_ALARM )
    aro->lineColor.setAlarmSensitive();
  else
    aro->lineColor.setAlarmInsensitive();
  aro->lineColor.setColor( aro->bufLineColor, aro->actWin->ci );

  aro->fill = aro->bufFill;

  aro->fillColorMode = aro->bufFillColorMode;
  if ( aro->fillColorMode == ARC_K_COLORMODE_ALARM )
    aro->fillColor.setAlarmSensitive();
  else
    aro->fillColor.setAlarmInsensitive();
  aro->fillColor.setColor( aro->bufFillColor, aro->actWin->ci );

  aro->lineWidth = aro->bufLineWidth;

  if ( aro->bufLineStyle == 0 )
    aro->lineStyle = LineSolid;
  else if ( aro->bufLineStyle == 1 )
    aro->lineStyle = LineOnOffDash;

  aro->alarmPvExpStr.setRaw( aro->bufAlarmPvName );

  aro->visPvExpStr.setRaw( aro->bufVisPvName );

  if ( aro->bufVisInverted )
    aro->visInverted = 0;
  else
    aro->visInverted = 1;

  strncpy( aro->minVisString, aro->bufMinVisString, 39 );
  strncpy( aro->maxVisString, aro->bufMaxVisString, 39 );

  aro->invisible = aro->bufInvisible;

  aro->x = aro->bufX;
  aro->sboxX = aro->bufX;

  aro->y = aro->bufY;
  aro->sboxY = aro->bufY;

  aro->w = aro->bufW;
  aro->sboxW = aro->bufW;

  aro->h = aro->bufH;
  aro->sboxH = aro->bufH;

}

void arc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call )
{

expRectClass *aro = (expRectClass *) client;

  arc_edit_update( w, client, call );
  aro->refresh( aro );

}

void arc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call )
{

expRectClass *aro = (expRectClass *) client;

  arc_edit_update( w, client, call );
  aro->ef.popdown();
  aro->operationComplete();

}

void arc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call )
{

expRectClass *aro = (expRectClass *) client;

  aro->ef.popdown();
  aro->operationCancel();

}

void arc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call )
{

expRectClass *aro = (expRectClass *) client;

  aro->erase();
  aro->deleteRequest = 1;
  aro->ef.popdown();
  aro->operationCancel();
  aro->drawAll();

}

void expRectClass::pv_status_callback (
  ProcessVariable *pv,
  void *userarg
) {

class expRectClass *aro = (expRectClass *) userarg;

  if ( pv->is_valid() ) {

    if ( !aro->needAlarmConnectInit ) {
      aro->needAlarmConnectInit = 1;
      aro->lineColor.setConnected();
      aro->fillColor.setConnected();
    }

    aro->lineColor.setStatus( pv->get_status(), pv->get_severity() );
    aro->fillColor.setStatus( pv->get_status(), pv->get_severity() );
    aro->bufInvalidate();
    aro->needRefresh = 1;

  }
  else {

    aro->alarmPvConnected = 0;
    aro->active = 0;
    aro->lineColor.setDisconnected();
    aro->fillColor.setDisconnected();
    aro->bufInvalidate();
    aro->needDraw = 1;

  }

  aro->actWin->addDefExeNode( aro->aglPtr );

}

void expRectClass::pv_value_callback (
  ProcessVariable *pv,
  void *userarg
) {

pvValType pvV;
class expRectClass *aro = (expRectClass *) userarg;

  if ( pv->is_valid() ) {

    aro->lineColor.setConnected();
    aro->fillColor.setConnected();
    if ( !aro->visPvConnected ) {
      aro->needVisConnectInit = 1;
    }

    pvV.d = pv->get_double();
    if ( ( pvV.d >= aro->minVis.d ) && ( pvV.d < aro->maxVis.d ) )
      aro->visibility = 1 ^ aro->visInverted;
    else
      aro->visibility = 0 ^ aro->visInverted;

    if ( aro->visibility ) {

      aro->needRefresh = 1;

    }
    else {

      aro->needErase = 1;
      aro->needRefresh = 1;

    }

  }
  else {

    aro->lineColor.setDisconnected();
    aro->fillColor.setDisconnected();
    aro->visPvConnected = 0;
    aro->active = 0;
    aro->bufInvalidate();
    aro->needDraw = 1;

  }

  aro->actWin->addDefExeNode( aro->aglPtr );

}

expRectClass::expRectClass ( void ) {

  name = new char[strlen("expRectClass")+1];
  strcpy( name, "expRectClass" );
  invisible = 0;
  visibility = 0;
  prevVisibility = -1;
  visInverted = 0;
  visPvConnected = alarmPvConnected = 0;
  visPvExists = alarmPvExists = 0;
  active = 0;
  activeMode = 0;
  fill = 0;
  lineColorMode = ARC_K_COLORMODE_STATIC;
  fillColorMode = ARC_K_COLORMODE_STATIC;
  lineWidth = 1;
  lineStyle = LineSolid;
  strcpy( minVisString, "" );
  strcpy( maxVisString, "" );

  //  bgRule = new ruleClass( (void *) this, BG_RULE, bgRuleCb );

  bgRule = new ruleClass();
  bgRule->init( (void *) this, BG_RULE, bgRuleConnectCb, bgRuleCb );
  useBgRule = 0;

}

// copy constructor
expRectClass::expRectClass
 ( const expRectClass *source ) {

activeGraphicClass *ago = (activeGraphicClass *) this;

  ago->clone( (activeGraphicClass *) source );

  name = new char[strlen("expRectClass")+1];
  strcpy( name, "expRectClass" );

  lineColor.copy(source->lineColor);
  fillColor.copy(source->fillColor);
  lineCb = source->lineCb;
  fillCb = source->fillCb;
  fill = source->fill;
  lineColorMode = source->lineColorMode;
  fillColorMode = source->fillColorMode;
  visInverted = source->visInverted;
  invisible = source->invisible;

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

  //bgRule = new ruleClass( (void *) this, BG_RULE, bgRuleCb );

  bgRule = new ruleClass();
  bgRule->init( (void *) this, BG_RULE, bgRuleConnectCb, bgRuleCb );
  useBgRule = 0;

}

int expRectClass::createInteractive (
  activeWindowClass *aw_obj,
  int _x,
  int _y,
  int _w,
  int _h ) {

  actWin = (activeWindowClass *) aw_obj;
  xOrigin = 0;
  yOrigin = 0;
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

int expRectClass::genericEdit ( void ) {

char title[32], *ptr;

  ptr = actWin->obj.getNameFromClass( "expRectClass" );
  if ( ptr )
    strncpy( title, ptr, 31 );
  else
    strncpy( title, expRectClass_str4, 31 );

  strncat( title, expRectClass_str5, 31 );

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

  bufInvisible = invisible;

  strncpy( bufMinVisString, minVisString, 39 );
  strncpy( bufMaxVisString, maxVisString, 39 );

  ef.create( actWin->top, actWin->appCtx->ci.getColorMap(),
   &actWin->appCtx->entryFormX,
   &actWin->appCtx->entryFormY, &actWin->appCtx->entryFormW,
   &actWin->appCtx->entryFormH, &actWin->appCtx->largestH,
   title, NULL, NULL, NULL );

  ef.addTextField( expRectClass_str6, 27, &bufX );
  ef.addTextField( expRectClass_str7, 27, &bufY );
  ef.addTextField( expRectClass_str8, 27, &bufW );
  ef.addTextField( expRectClass_str9, 27, &bufH );
  ef.addOption( expRectClass_str10, expRectClass_str11, &bufLineWidth );
  ef.addOption( expRectClass_str12, expRectClass_str13, &bufLineStyle );
  ef.addColorButton( expRectClass_str14, actWin->ci, &lineCb, &bufLineColor );
  ef.addToggle( expRectClass_str15, &bufLineColorMode );
  ef.addToggle( expRectClass_str16, &bufFill );
  ef.addColorButton( expRectClass_str17, actWin->ci, &fillCb, &bufFillColor );
  ef.addToggle( expRectClass_str18, &bufFillColorMode );
  ef.addToggle( expRectClass_str19, &bufInvisible );
  ef.addTextField( expRectClass_str20, 27, bufAlarmPvName, 39 );
  ef.addTextField( expRectClass_str21, 27, bufVisPvName, 39 );
  ef.addOption( " ", expRectClass_str22, &bufVisInverted );
  ef.addTextField( expRectClass_str23, 27, bufMinVisString, 39 );
  ef.addTextField( expRectClass_str24, 27, bufMaxVisString, 39 );

  return 1;

}

int expRectClass::editCreate ( void ) {

  this->genericEdit();
  ef.finished( arc_edit_ok, arc_edit_apply, arc_edit_cancel_delete, this );
  actWin->currentEf = NULL;
  ef.popup();

  return 1;

}

int expRectClass::edit ( void ) {

  this->genericEdit();
  ef.finished( arc_edit_ok, arc_edit_apply, arc_edit_cancel, this );
  actWin->currentEf = &ef;
  ef.popup();

  return 1;

}

int expRectClass::createFromFile (
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

    if ( lineColorMode == ARC_K_COLORMODE_ALARM )
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

    if ( lineColorMode == ARC_K_COLORMODE_ALARM )
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

  if ( fillColorMode == ARC_K_COLORMODE_ALARM )
    fillColor.setAlarmSensitive();
  else
    fillColor.setAlarmInsensitive();

  readStringFromFile( oneName, 39, f ); actWin->incLine();
  alarmPvExpStr.setRaw( oneName );

  readStringFromFile( oneName, 39, f ); actWin->incLine();
  visPvExpStr.setRaw( oneName );

  fscanf( f, "%d\n", &visInverted ); actWin->incLine();

  if ( ( major > 1 ) || ( minor > 0 ) ) {
    readStringFromFile( minVisString, 39, f ); actWin->incLine();
    readStringFromFile( maxVisString, 39, f ); actWin->incLine();
  }
  else {
    strcpy( minVisString, "1" );
    strcpy( maxVisString, "1" );
  }

  fscanf( f, "%d\n", &lineWidth ); actWin->incLine();

  fscanf( f, "%d\n", &lineStyle ); actWin->incLine();

  if ( ( major > 1 ) || ( minor > 1 ) ) {
    fscanf( f, "%d\n", &invisible ); actWin->incLine();
  }
  else {
    invisible = 0;
  }

  return 1;

}

int expRectClass::importFromXchFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

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
      actWin->appCtx->postMessage( expRectClass_str25 );
      return 0;
    }

    context = NULL;

    tk = strtok_r( buf, " \t\n", &context );
    if ( !tk ) {
      actWin->appCtx->postMessage( expRectClass_str25 );
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
          actWin->appCtx->postMessage( expRectClass_str25 );
          return 0;
        }

        x = atol( tk );

      }
      else if ( strcmp( tk, "y" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( expRectClass_str25 );
          return 0;
        }

        y = atol( tk );

      }
      else if ( strcmp( tk, "w" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( expRectClass_str25 );
          return 0;
        }

        w = atol( tk );

      }
      else if ( strcmp( tk, "h" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( expRectClass_str25 );
          return 0;
        }

        h = atol( tk );

      }
            
      else if ( strcmp( tk, "fgred" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( expRectClass_str25 );
          return 0;
        }

        fgR = atol( tk );

      }
            
      else if ( strcmp( tk, "fggreen" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( expRectClass_str25 );
          return 0;
        }

        fgG = atol( tk );

      }
            
      else if ( strcmp( tk, "fgblue" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( expRectClass_str25 );
          return 0;
        }

        fgB = atol( tk );

      }
            
      else if ( strcmp( tk, "bgred" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( expRectClass_str25 );
          return 0;
        }

        bgR = atol( tk );

      }
            
      else if ( strcmp( tk, "bggreen" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( expRectClass_str25 );
          return 0;
        }

        bgG = atol( tk );

      }
            
      else if ( strcmp( tk, "bgblue" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( expRectClass_str25 );
          return 0;
        }

        bgB = atol( tk );

      }
            
      else if ( strcmp( tk, "linewidth" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( expRectClass_str25 );
          return 0;
        }

       lineWidth  = atol( tk );

      }
            
      else if ( strcmp( tk, "fill" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( expRectClass_str25 );
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

int expRectClass::save (
  FILE *f )
{

int index;

  fprintf( f, "%-d %-d %-d\n", ARC_MAJOR_VERSION, ARC_MINOR_VERSION,
   ARC_RELEASE );
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

  fprintf( f, "%-d\n", invisible );

  return 1;

}

int expRectClass::drawActive ( void ) {

  if ( !init || !activeMode || invisible || !visibility ) return 1;

  prevVisibility = visibility;

  actWin->executeGc.saveFg();

  if ( fill ) {
    actWin->executeGc.setFG( fillColor.getColor() );
    XFillRectangle( actWin->d, XtWindow(actWin->executeWidget),
     actWin->executeGc.normGC(), x, y, w, h );
  }

  actWin->executeGc.setFG( lineColor.getColor() );
  actWin->executeGc.setLineWidth( lineWidth );
  actWin->executeGc.setLineStyle( lineStyle );

  XDrawRectangle( actWin->d, XtWindow(actWin->executeWidget),
   actWin->executeGc.normGC(), x, y, w, h );

  actWin->executeGc.setLineWidth( 1 );
  actWin->executeGc.setLineStyle( LineSolid );
  actWin->executeGc.restoreFg();

  return 1;

}

int expRectClass::eraseUnconditional ( void ) {

  if ( fill ) {
    XFillRectangle( actWin->d, XtWindow(actWin->executeWidget),
     actWin->executeGc.eraseGC(), x, y, w, h );
  }

  actWin->executeGc.setLineWidth( lineWidth );
  actWin->executeGc.setLineStyle( lineStyle );

  XDrawRectangle( actWin->d, XtWindow(actWin->executeWidget),
   actWin->executeGc.eraseGC(), x, y, w, h );

  actWin->executeGc.setLineWidth( 1 );
  actWin->executeGc.setLineStyle( LineSolid );

  return 1;

}

int expRectClass::eraseActive ( void ) {

  if ( !init || !activeMode || invisible ) return 1;

  if ( prevVisibility == 0 ) {
    prevVisibility = visibility;
    return 1;
  }

  prevVisibility = visibility;

  if ( fill ) {
    XFillRectangle( actWin->d, XtWindow(actWin->executeWidget),
     actWin->executeGc.eraseGC(), x, y, w, h );
  }

  actWin->executeGc.setLineWidth( lineWidth );
  actWin->executeGc.setLineStyle( lineStyle );

  XDrawRectangle( actWin->d, XtWindow(actWin->executeWidget),
   actWin->executeGc.eraseGC(), x, y, w, h );

  actWin->executeGc.setLineWidth( 1 );
  actWin->executeGc.setLineStyle( LineSolid );

  return 1;

}

int expRectClass::expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int stat;

  stat = alarmPvExpStr.expand1st( numMacros, macros, expansions );
  stat = visPvExpStr.expand1st( numMacros, macros, expansions );

  return stat;

}

int expRectClass::expand2nd (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int stat;

  stat = alarmPvExpStr.expand2nd( numMacros, macros, expansions );
  stat = visPvExpStr.expand2nd( numMacros, macros, expansions );

  return stat;

}

int expRectClass::containsMacros ( void ) {

  if ( alarmPvExpStr.containsPrimaryMacros() ) return 1;
  if ( visPvExpStr.containsPrimaryMacros() ) return 1;

  return 0;

}

int expRectClass::activate (
  int pass,
  void *ptr )
{

int stat;
char string[255+1];

  switch ( pass ) {

  case 1: // initialize

    visPv = 0;
    alarmPv = 0;
    needVisConnectInit = 0;
    needAlarmConnectInit = 0;
    needErase = needDraw = needRefresh = 0;
    firstRuleInit = 1;
    aglPtr = ptr;
    opComplete = 0;

    alarmPvConnected = visPvConnected = 0;
    activeMode = 1;
    pvType = -1;
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

      if ( alarmPvExists ) {
        alarmPv = pv_factory->create( alarmPvExpStr.getExpanded() );
	if ( alarmPv ) {
	  alarmPv->add_status_callback( pv_status_callback, this );
        }
      }

      if ( visPvExists ) {
        visPv = pv_factory->create( visPvExpStr.getExpanded() );
        if ( visPv ) {
          visPv->add_value_callback( pv_value_callback, this );
        }
      }

      opComplete = 1;
      this->bufInvalidate();

    }

    break;

  case 3:
    strcpy( string, "testRule(" );
    strncat( string, visPvExpStr.getExpanded(), 255 );
    strncat( string, ",", 255 );
    strncat( string, alarmPvExpStr.getExpanded(), 255 );
    strncat( string, ")", 255 );
    bgRule->activate( &actWin->appCtx->userLibObject, string );
    break;

  case 4:
  case 5:
  case 6:

    break;

  }

  return 1;

}

int expRectClass::deactivate (
  int pass )
{

int stat;

  if ( pass == 1 ) {

    activeMode = 0;
    fillColor.setNotRuleMode();

    bgRule->deactivate();

    if ( visPv ) {
      visPv->remove_value_callback( pv_value_callback, this );
      visPv->release();
      visPv = 0;
    }

    if ( alarmPv ) {
      alarmPv->remove_status_callback( pv_status_callback, this );
      alarmPv->release();
      alarmPv = 0;
    }

  }

  return 1;

}

int expRectClass::draw ( void ) {

  if ( activeMode ) return 1;
  if ( deleteRequest ) return 1;

  actWin->drawGc.saveFg();

  if ( fill ) {
    actWin->drawGc.setFG( fillColor.pixelColor() );
    XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
     actWin->drawGc.normGC(), x, y, w, h );
  }

  actWin->drawGc.setFG( lineColor.pixelColor() );
  actWin->drawGc.setLineWidth( lineWidth );
  actWin->drawGc.setLineStyle( lineStyle );

  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x, y, w, h );

  actWin->drawGc.setLineWidth( 1 );
  actWin->drawGc.setLineStyle( LineSolid );
  actWin->drawGc.restoreFg();

  return 1;

}

int expRectClass::erase ( void ) {

  if ( activeMode ) return 1;
  if ( deleteRequest ) return 1;

  if ( fill ) {
    XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
     actWin->drawGc.eraseGC(), x, y, w, h );
  }

  actWin->drawGc.setLineWidth( lineWidth );
  actWin->drawGc.setLineStyle( lineStyle );

  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  actWin->drawGc.setLineWidth( 1 );
  actWin->drawGc.setLineStyle( LineSolid );

  return 1;

}

void expRectClass::executeDeferred ( void ) {

int stat, nvc, nac, ne, nd, nr, ncu;

  if ( actWin->isIconified ) return;

  actWin->appCtx->proc->lock();
  nvc = needVisConnectInit; needVisConnectInit = 0;
  nac = needAlarmConnectInit; needAlarmConnectInit = 0;
  ncu = needColorUpdate; needColorUpdate = 0;
  ne = needErase; needErase = 0;
  nd = needDraw; needDraw = 0;
  nr = needRefresh; needRefresh = 0;
  actWin->remDefExeNode( aglPtr );
  actWin->appCtx->proc->unlock();

#ifdef __epics__

  if ( nvc ) {

    if ( ( visPv->get_type().type == ProcessVariable::Type::enumerated ) ||
         ( visPv->get_type().type == ProcessVariable::Type::integer ) ||
         ( visPv->get_type().type == ProcessVariable::Type::real ) ) {

      visPvConnected = 1;

      pvType = visPv->get_type().type;

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

  }

#endif

  if ( ne ) {
    eraseActive();
  }

  if ( nd ) {
    stat = smartDrawAllActive();
  }

  if ( nr ) {
    stat = smartDrawAllActive();
  }

  if ( ncu ) {
    if ( useBgRule ) {
      fillColor.setRuleColor( bgRuleColor, actWin->ci );
      fillColor.setRuleMode();
    }
    else {
      fillColor.setNotRuleMode();
      fillColor.setColor( fillColor.pixelColor(), actWin->ci );
    }
    stat = smartDrawAllActive();
  }

}

char *expRectClass::firstDragName ( void ) {

  dragIndex = 0;
  return dragName[dragIndex];

}

char *expRectClass::nextDragName ( void ) {

  if ( dragIndex < (int) ( sizeof(dragName) / sizeof(char *) ) - 1 ) {
    dragIndex++;
    return dragName[dragIndex];
  }
  else {
    return NULL;
  }

}

char *expRectClass::dragValue (
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

void expRectClass::changeDisplayParams (
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

void expRectClass::changePvNames (
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

void *create_expRectClassPtr ( void ) {

expRectClass *ptr;

  ptr = new expRectClass;
  return (void *) ptr;

}

void *clone_expRectClassPtr (
  void *_srcPtr )
{

expRectClass *ptr, *srcPtr;

  srcPtr = (expRectClass *) _srcPtr;

  ptr = new expRectClass( srcPtr );

  return (void *) ptr;

}

#ifdef __cplusplus
}
#endif
