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

#define __menu_button_cc 1

#include "menu_button.h"
#include "app_pkg.h"
#include "act_win.h"

#include "thread.h"

static void unconnectedTimeout (
  XtPointer client,
  XtIntervalId *id )
{

activeMenuButtonClass *ambo = (activeMenuButtonClass *) client;

  if ( !ambo->connection.pvsConnected() ) {
    ambo->needToDrawUnconnected = 1;
    ambo->needDraw = 1;
    ambo->actWin->addDefExeNode( ambo->aglPtr );
  }

  ambo->unconnectedTimer = 0;

}

static void menu_cb (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeMenuButtonClass *mbto = (activeMenuButtonClass *) client;
int i, stat;
short value;

  for ( i=0; i<mbto->numStates; i++ ) {

    if ( w == mbto->pb[i] ) {
      value = (short) i;
#ifdef __epics__
      stat = ca_put( DBR_ENUM, mbto->controlPvId, &value );
#endif
      break;
    }

  }

}

#ifdef __epics__

static void mbt_monitor_control_connect_state (
  struct connection_handler_args arg )
{

activeMenuButtonClass *mbto = (activeMenuButtonClass *) ca_puser(arg.chid);

  if ( arg.op == CA_OP_CONN_UP ) {

    mbto->connection.setPvConnected( (void *) mbto->controlPvConnection );
    mbto->needConnectInit = 1;

    if ( mbto->connection.pvsConnected() ) {
      mbto->fgColor.setConnected();
    }

  }
  else {

    mbto->connection.setPvDisconnected( (void *) mbto->controlPvConnection );
    mbto->fgColor.setDisconnected();
    mbto->controlValid = 0;
    mbto->needDraw = 1;
    mbto->active = 0;

  }

  mbto->actWin->appCtx->proc->lock();
  mbto->actWin->addDefExeNode( mbto->aglPtr );
  mbto->actWin->appCtx->proc->unlock();

}

static void mbt_infoUpdate (
  struct event_handler_args ast_args )
{

int i;
activeMenuButtonClass *mbto = (activeMenuButtonClass *) ast_args.usr;
struct dbr_gr_enum enumRec;

  enumRec = *( (struct dbr_gr_enum *) ast_args.dbr );

  mbto->numStates = enumRec.no_str;

  for ( i=0; i<mbto->numStates; i++ ) {

    if ( mbto->stateString[i] == NULL ) {
      mbto->stateString[i] = new char[MAX_ENUM_STRING_SIZE+1];
    }

    strncpy( mbto->stateString[i], enumRec.strs[i], MAX_ENUM_STRING_SIZE );

  }

  mbto->curValue = enumRec.value;

  mbto->needInfoInit = 1;
  mbto->actWin->appCtx->proc->lock();
  mbto->actWin->addDefExeNode( mbto->aglPtr );
  mbto->actWin->appCtx->proc->unlock();

}

static void mbt_controlUpdate (
  struct event_handler_args ast_args )
{

activeMenuButtonClass *mbto = (activeMenuButtonClass *) ast_args.usr;

  mbto->curValue = *( (short *) ast_args.dbr );

  mbto->controlValid = 1;
  mbto->needRefresh = 1;
  mbto->needDraw = 1;
  mbto->actWin->appCtx->proc->lock();
  mbto->actWin->addDefExeNode( mbto->aglPtr );
  mbto->actWin->appCtx->proc->unlock();

}

static void mbt_alarmUpdate (
  struct event_handler_args ast_args )
{

activeMenuButtonClass *mbto = (activeMenuButtonClass *) ast_args.usr;
struct dbr_sts_enum statusRec;

  if ( !mbto->readExists ) {

    statusRec = *( (struct dbr_sts_enum *) ast_args.dbr );

    mbto->fgColor.setStatus( statusRec.status, statusRec.severity );
    mbto->bgColor.setStatus( statusRec.status, statusRec.severity );

    mbto->needDraw = 1;
    mbto->actWin->appCtx->proc->lock();
    mbto->actWin->addDefExeNode( mbto->aglPtr );
    mbto->actWin->appCtx->proc->unlock();

  }

}

static void mbt_monitor_read_connect_state (
  struct connection_handler_args arg )
{

activeMenuButtonClass *mbto = (activeMenuButtonClass *) ca_puser(arg.chid);

  if ( arg.op == CA_OP_CONN_UP ) {

    mbto->connection.setPvConnected( (void *) mbto->readPvConnection );
    mbto->needReadConnectInit = 1;

    if ( mbto->connection.pvsConnected() ) {
      mbto->fgColor.setConnected();
    }

  }
  else {

    mbto->connection.setPvDisconnected( (void *) mbto->readPvConnection );
    mbto->fgColor.setDisconnected();
    mbto->readValid = 0;
    mbto->needDraw = 1;
    mbto->active = 0;

  }

  mbto->actWin->appCtx->proc->lock();
  mbto->actWin->addDefExeNode( mbto->aglPtr );
  mbto->actWin->appCtx->proc->unlock();

}

static void mbt_readInfoUpdate (
  struct event_handler_args ast_args )
{

int i;
activeMenuButtonClass *mbto = (activeMenuButtonClass *) ast_args.usr;
struct dbr_gr_enum enumRec;

  enumRec = *( (struct dbr_gr_enum *) ast_args.dbr );

  if ( !mbto->controlExists ) {

  mbto->numStates = enumRec.no_str;

    for ( i=0; i<mbto->numStates; i++ ) {

      if ( mbto->stateString[i] == NULL ) {
        mbto->stateString[i] = new char[MAX_ENUM_STRING_SIZE+1];
      }

      strncpy( mbto->stateString[i], enumRec.strs[i],
       MAX_ENUM_STRING_SIZE );

    }

  }

  mbto->curReadValue = enumRec.value;

  mbto->needReadInfoInit = 1;
  mbto->actWin->appCtx->proc->lock();
  mbto->actWin->addDefExeNode( mbto->aglPtr );
  mbto->actWin->appCtx->proc->unlock();

}

static void mbt_readUpdate (
  struct event_handler_args ast_args )
{

activeMenuButtonClass *mbto = (activeMenuButtonClass *) ast_args.usr;

  mbto->curReadValue = *( (short *) ast_args.dbr );

  mbto->readValid = 1;
  mbto->needRefresh = 1;
  mbto->needDraw = 1;
  mbto->actWin->appCtx->proc->lock();
  mbto->actWin->addDefExeNode( mbto->aglPtr );
  mbto->actWin->appCtx->proc->unlock();

}

static void mbt_readAlarmUpdate (
  struct event_handler_args ast_args )
{

activeMenuButtonClass *mbto = (activeMenuButtonClass *) ast_args.usr;
struct dbr_sts_enum statusRec;

  statusRec = *( (struct dbr_sts_enum *) ast_args.dbr );

  mbto->fgColor.setStatus( statusRec.status, statusRec.severity );
  mbto->bgColor.setStatus( statusRec.status, statusRec.severity );

  mbto->needDraw = 1;
  mbto->actWin->appCtx->proc->lock();
  mbto->actWin->addDefExeNode( mbto->aglPtr );
  mbto->actWin->appCtx->proc->unlock();

}

static void mbt_monitor_vis_connect_state (
  struct connection_handler_args arg )
{

activeMenuButtonClass *mbto = (activeMenuButtonClass *) ca_puser(arg.chid);

  if ( arg.op == CA_OP_CONN_UP ) {

    mbto->needVisConnectInit = 1;

  }
  else {

    mbto->connection.setPvDisconnected( (void *) mbto->visPvConnection );
    mbto->fgColor.setDisconnected();
    mbto->active = 0;
    mbto->needDraw = 1;

  }

  mbto->actWin->appCtx->proc->lock();
  mbto->actWin->addDefExeNode( mbto->aglPtr );
  mbto->actWin->appCtx->proc->unlock();

}

static void mbt_visInfoUpdate (
  struct event_handler_args arg )
{

activeMenuButtonClass *mbto = (activeMenuButtonClass *) ca_puser(arg.chid);

struct dbr_gr_double controlRec = *( (dbr_gr_double *) arg.dbr );

  mbto->curVisValue = controlRec.value;

  mbto->actWin->appCtx->proc->lock();
  mbto->needVisInit = 1;
  mbto->actWin->addDefExeNode( mbto->aglPtr );
  mbto->actWin->appCtx->proc->unlock();

}

static void mbt_visUpdate (
  struct event_handler_args arg )
{

activeMenuButtonClass *mbto = (activeMenuButtonClass *) ca_puser(arg.chid);

  mbto->curVisValue = * ( (double *) arg.dbr );

  mbto->actWin->appCtx->proc->lock();
  mbto->needVisUpdate = 1;
  mbto->actWin->addDefExeNode( mbto->aglPtr );
  mbto->actWin->appCtx->proc->unlock();

}

#endif

static void mbtc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeMenuButtonClass *mbto = (activeMenuButtonClass *) client;

  mbto->actWin->setChanged();

  mbto->eraseSelectBoxCorners();
  mbto->erase();

  strncpy( mbto->fontTag, mbto->fm.currentFontTag(), 63 );
  mbto->actWin->fi->loadFontTag( mbto->fontTag );
  mbto->actWin->drawGc.setFontTag( mbto->fontTag, mbto->actWin->fi );
  mbto->actWin->fi->getTextFontList( mbto->fontTag, &mbto->fontList );
  mbto->fs = mbto->actWin->fi->getXFontStruct( mbto->fontTag );

  mbto->topShadowColor = mbto->bufTopShadowColor;
  mbto->botShadowColor = mbto->bufBotShadowColor;

  mbto->fgColorMode = mbto->bufFgColorMode;
  if ( mbto->fgColorMode == MBTC_K_COLORMODE_ALARM )
    mbto->fgColor.setAlarmSensitive();
  else
    mbto->fgColor.setAlarmInsensitive();
  mbto->fgColor.setColorIndex( mbto->bufFgColor, mbto->actWin->ci );

  mbto->bgColorMode = mbto->bufBgColorMode;
  if ( mbto->bgColorMode == MBTC_K_COLORMODE_ALARM )
    mbto->bgColor.setAlarmSensitive();
  else
    mbto->bgColor.setAlarmInsensitive();
  mbto->bgColor.setColorIndex( mbto->bufBgColor, mbto->actWin->ci );

  mbto->inconsistentColor.setColorIndex( mbto->bufInconsistentColor,
   mbto->actWin->ci );

  mbto->visPvExpString.setRaw( mbto->bufVisPvName );
  strncpy( mbto->minVisString, mbto->bufMinVisString, 39 );
  strncpy( mbto->maxVisString, mbto->bufMaxVisString, 39 );

  if ( mbto->bufVisInverted )
    mbto->visInverted = 0;
  else
    mbto->visInverted = 1;

  mbto->x = mbto->bufX;
  mbto->sboxX = mbto->bufX;

  mbto->y = mbto->bufY;
  mbto->sboxY = mbto->bufY;

  mbto->w = mbto->bufW;
  mbto->sboxW = mbto->bufW;

  mbto->h = mbto->bufH;
  mbto->sboxH = mbto->bufH;

  mbto->controlPvExpStr.setRaw( mbto->bufControlPvName );

  mbto->readPvExpStr.setRaw( mbto->bufReadPvName );

  mbto->updateDimensions();

}

static void mbtc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeMenuButtonClass *mbto = (activeMenuButtonClass *) client;

  mbtc_edit_update ( w, client, call );
  mbto->refresh( mbto );

}

static void mbtc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeMenuButtonClass *mbto = (activeMenuButtonClass *) client;

  mbtc_edit_update ( w, client, call );
  mbto->ef.popdown();
  mbto->operationComplete();

}

static void mbtc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeMenuButtonClass *mbto = (activeMenuButtonClass *) client;

  mbto->ef.popdown();
  mbto->operationCancel();

}

static void mbtc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeMenuButtonClass *mbto = (activeMenuButtonClass *) client;

  mbto->ef.popdown();
  mbto->operationCancel();
  mbto->erase();
  mbto->deleteRequest = 1;
  mbto->drawAll();

}

activeMenuButtonClass::activeMenuButtonClass ( void ) {

int i;

  name = new char[strlen("activeMenuButtonClass")+1];
  strcpy( name, "activeMenuButtonClass" );

  numStates = 0;

  for ( i=0; i<MAX_ENUM_STATES; i++ ) {
    stateString[i] = NULL;
    pb[i] = NULL;
  }

  fgColorMode = MBTC_K_COLORMODE_STATIC;
  bgColorMode = MBTC_K_COLORMODE_STATIC;

  active = 0;
  activeMode = 0;
  widgetsCreated = 0;

  fontList = NULL;

  connection.setMaxPvs( 3 );

  unconnectedTimer = 0;

  visibility = 0;
  prevVisibility = -1;
  visInverted = 0;
  strcpy( minVisString, "" );
  strcpy( maxVisString, "" );

}

activeMenuButtonClass::~activeMenuButtonClass ( void ) {

  if ( name ) delete name;
  if ( fontList ) XmFontListFree( fontList );

  if ( unconnectedTimer ) {
    XtRemoveTimeOut( unconnectedTimer );
    unconnectedTimer = 0;
  }

}

// copy constructor
activeMenuButtonClass::activeMenuButtonClass
 ( const activeMenuButtonClass *source ) {

int i;
activeGraphicClass *mbto = (activeGraphicClass *) this;

  mbto->clone( (activeGraphicClass *) source );

  name = new char[strlen("activeMenuButtonClass")+1];
  strcpy( name, "activeMenuButtonClass" );

  numStates = 0;

  for ( i=0; i<MAX_ENUM_STATES; i++ ) {
    stateString[i] = NULL;
    pb[i] = NULL;
  }

  strncpy( fontTag, source->fontTag, 63 );
  fs = actWin->fi->getXFontStruct( fontTag );
  actWin->fi->getTextFontList( fontTag, &fontList );

  fontAscent = source->fontAscent;
  fontDescent = source->fontDescent;
  fontHeight = source->fontHeight;

  topShadowColor = source->topShadowColor;
  botShadowColor = source->botShadowColor;
  topShadowCb = source->topShadowCb;
  botShadowCb = source->botShadowCb;

  fgColor.copy(source->fgColor);
  bgColor.copy(source->bgColor);
  inconsistentColor = source->inconsistentColor;
  fgCb = source->fgCb;
  bgCb = source->bgCb;
  inconsistentCb = source->inconsistentCb;

  fgColorMode = source->fgColorMode;
  bgColorMode = source->bgColorMode;

  controlPvExpStr.setRaw( source->controlPvExpStr.rawString );
  readPvExpStr.setRaw( source->readPvExpStr.rawString );

  widgetsCreated = 0;
  active = 0;
  activeMode = 0;

  connection.setMaxPvs( 3 );

  unconnectedTimer = 0;

  visibility = 0;
  prevVisibility = -1;
  visInverted = source->visInverted;
  strncpy( minVisString, source->minVisString, 39 );
  strncpy( maxVisString, source->maxVisString, 39 );

}

int activeMenuButtonClass::createInteractive (
  activeWindowClass *aw_obj,
  int _x,
  int _y,
  int _w,
  int _h ) {

  actWin = (activeWindowClass *) aw_obj;
  x = _x;
  y = _y;
  w = _w;
  h = _h;

  strcpy( fontTag, actWin->defaultBtnFontTag );
  actWin->fi->loadFontTag( fontTag );
  fs = actWin->fi->getXFontStruct( fontTag );
  actWin->fi->getTextFontList( fontTag, &fontList );

  updateDimensions();

  topShadowColor = actWin->defaultTopShadowColor;
  botShadowColor = actWin->defaultBotShadowColor;

  fgColor.setColorIndex( actWin->defaultTextFgColor, actWin->ci );
  bgColor.setColorIndex( actWin->defaultBgColor, actWin->ci );
  inconsistentColor.setColorIndex( actWin->defaultOffsetColor,
   actWin->ci );

  this->draw();

  this->editCreate();

  return 1;

}

int activeMenuButtonClass::save (
  FILE *f )
{

int index;

  fprintf( f, "%-d %-d %-d\n", MBTC_MAJOR_VERSION, MBTC_MINOR_VERSION,
   MBTC_RELEASE );

  fprintf( f, "%-d\n", x );
  fprintf( f, "%-d\n", y );
  fprintf( f, "%-d\n", w );
  fprintf( f, "%-d\n", h );

  index = fgColor.pixelIndex();
  actWin->ci->writeColorIndex( f, index );
  //fprintf( f, "%-d\n", index );

  fprintf( f, "%-d\n", fgColorMode );

  index = bgColor.pixelIndex();
  actWin->ci->writeColorIndex( f, index );
  //fprintf( f, "%-d\n", index );

  fprintf( f, "%-d\n", bgColorMode );

  index = topShadowColor;
  actWin->ci->writeColorIndex( f, index );
  //fprintf( f, "%-d\n", index );

  index = botShadowColor;
  actWin->ci->writeColorIndex( f, index );
  //fprintf( f, "%-d\n", index );

  if ( controlPvExpStr.getRaw() )
    writeStringToFile( f, controlPvExpStr.getRaw() );
  else
    writeStringToFile( f, "" );

  writeStringToFile( f, fontTag );

  if ( readPvExpStr.getRaw() )
    writeStringToFile( f, readPvExpStr.getRaw() );
  else
    writeStringToFile( f, "" );

  // version 2.1.0
  index = inconsistentColor.pixelIndex();
  fprintf( f, "%-d\n", index );

  //ver 2.3.0
  if ( visPvExpString.getRaw() )
    writeStringToFile( f, visPvExpString.getRaw() );
  else
    writeStringToFile( f, "" );
  fprintf( f, "%-d\n", visInverted );
  writeStringToFile( f, minVisString );
  writeStringToFile( f, maxVisString );

  return 1;

}

int activeMenuButtonClass::createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int r, g, b, index;
int major, minor, release;
unsigned int pixel;
char oneName[activeGraphicClass::MAX_PV_NAME+1];

  this->actWin = _actWin;

  fscanf( f, "%d %d %d\n", &major, &minor, &release ); actWin->incLine();

  fscanf( f, "%d\n", &x ); actWin->incLine();
  fscanf( f, "%d\n", &y ); actWin->incLine();
  fscanf( f, "%d\n", &w ); actWin->incLine();
  fscanf( f, "%d\n", &h ); actWin->incLine();

  this->initSelectBox(); // call after getting x,y,w,h

  if ( ( major > 2 ) || ( ( major == 2 ) && ( minor > 1 ) ) ) {

    actWin->ci->readColorIndex( f, &index );
    actWin->incLine(); actWin->incLine();
    fgColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d\n", &fgColorMode ); actWin->incLine();

    if ( fgColorMode == MBTC_K_COLORMODE_ALARM )
      fgColor.setAlarmSensitive();
    else
      fgColor.setAlarmInsensitive();

    actWin->ci->readColorIndex( f, &index );
    actWin->incLine(); actWin->incLine();
    bgColor.setColorIndex( index, actWin->ci );

  }
  else if ( major > 1 ) {

    fscanf( f, "%d\n", &index ); actWin->incLine();
    fgColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d\n", &fgColorMode ); actWin->incLine();

    if ( fgColorMode == MBTC_K_COLORMODE_ALARM )
      fgColor.setAlarmSensitive();
    else
      fgColor.setAlarmInsensitive();

    fscanf( f, "%d\n", &index ); actWin->incLine();
    bgColor.setColorIndex( index, actWin->ci );

  }
  else {

    fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
    if ( ( major < 2 ) && ( minor < 2 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    actWin->ci->setRGB( r, g, b, &pixel );
    index = actWin->ci->pixIndex( pixel );
    fgColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d\n", &fgColorMode ); actWin->incLine();

    if ( fgColorMode == MBTC_K_COLORMODE_ALARM )
      fgColor.setAlarmSensitive();
    else
      fgColor.setAlarmInsensitive();

    fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
    if ( ( major < 2 ) && ( minor < 2 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    actWin->ci->setRGB( r, g, b, &pixel );
    index = actWin->ci->pixIndex( pixel );
    bgColor.setColorIndex( index, actWin->ci );

  }

  fscanf( f, "%d\n", &bgColorMode ); actWin->incLine();

  if ( bgColorMode == MBTC_K_COLORMODE_ALARM )
    bgColor.setAlarmSensitive();
  else
    bgColor.setAlarmInsensitive();

  if ( ( major > 1 ) || ( minor > 0 ) ) {

    if ( ( major > 2 ) || ( ( major == 2 ) && ( minor > 1 ) ) ) {

      actWin->ci->readColorIndex( f, &index );
      actWin->incLine(); actWin->incLine();
      topShadowColor = index;

      actWin->ci->readColorIndex( f, &index );
      actWin->incLine(); actWin->incLine();
      botShadowColor = index;

    }
    else if ( major > 1 ) {

      fscanf( f, "%d\n", &index ); actWin->incLine();
      topShadowColor = index;

      fscanf( f, "%d\n", &index ); actWin->incLine();
      botShadowColor = index;

    }
    else {

      fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
      if ( ( major < 2 ) && ( minor < 2 ) ) {
        r *= 256;
        g *= 256;
        b *= 256;
      }
      actWin->ci->setRGB( r, g, b, &pixel );
      topShadowColor = actWin->ci->pixIndex( pixel );

      fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
      if ( ( major < 2 ) && ( minor < 2 ) ) {
        r *= 256;
        g *= 256;
        b *= 256;
      }
      actWin->ci->setRGB( r, g, b, &pixel );
      botShadowColor = actWin->ci->pixIndex( pixel );

    }

  }
  else {

    topShadowColor = actWin->ci->pixIndex( WhitePixel( actWin->display(),
     DefaultScreen(actWin->display()) ) );

    botShadowColor = actWin->ci->pixIndex( BlackPixel( actWin->display(),
     DefaultScreen(actWin->display()) ) );

  }

  readStringFromFile( oneName, activeGraphicClass::MAX_PV_NAME+1, f );
   actWin->incLine();
  controlPvExpStr.setRaw( oneName );

  readStringFromFile( fontTag, 63+1, f ); actWin->incLine();

  if ( ( major > 1 ) || ( minor > 2 ) ) {
    readStringFromFile( oneName, activeGraphicClass::MAX_PV_NAME+1, f );
     actWin->incLine();
    readPvExpStr.setRaw( oneName );
  }
  else {
    readPvExpStr.setRaw( "" );
  }

  if ( ( major > 2 ) || ( major == 2 ) && ( minor > 0 ) ) {
    fscanf( f, "%d\n", &index ); actWin->incLine();
    inconsistentColor.setColorIndex( index, actWin->ci );
  }
  else {
    inconsistentColor.setColorIndex( bgColor.pixelIndex(), actWin->ci );
  }

  actWin->fi->loadFontTag( fontTag );
  actWin->drawGc.setFontTag( fontTag, actWin->fi );

  fs = actWin->fi->getXFontStruct( fontTag );
  actWin->fi->getTextFontList( fontTag, &fontList );

  if ( ( major > 2 ) || ( ( major == 2 ) && ( minor > 2 ) ) ) {

    readStringFromFile( oneName, activeGraphicClass::MAX_PV_NAME+1, f );
     actWin->incLine();
    visPvExpString.setRaw( oneName );

    fscanf( f, "%d\n", &visInverted ); actWin->incLine();

    readStringFromFile( minVisString, 39+1, f ); actWin->incLine();

    readStringFromFile( maxVisString, 39+1, f ); actWin->incLine();

  }

  updateDimensions();

  return 1;

}

int activeMenuButtonClass::genericEdit ( void ) {

char title[32], *ptr;

  ptr = actWin->obj.getNameFromClass( "activeMenuButtonClass" );
  if ( ptr )
    strncpy( title, ptr, 31 );
  else
    strncpy( title, activeMenuButtonClass_str2, 31 );

  strncat( title, activeMenuButtonClass_str3, 31 );

  bufX = x;
  bufY = y;
  bufW = w;
  bufH = h;

  strncpy( bufFontTag, fontTag, 63 );

  bufTopShadowColor = topShadowColor;
  bufBotShadowColor = botShadowColor;

  bufFgColor = fgColor.pixelIndex();
  bufFgColorMode = fgColorMode;

  bufBgColor = bgColor.pixelIndex();
  bufBgColorMode = bgColorMode;

  bufInconsistentColor = inconsistentColor.pixelIndex();

  if ( controlPvExpStr.getRaw() )
    strncpy( bufControlPvName, controlPvExpStr.getRaw(),
     activeGraphicClass::MAX_PV_NAME );
  else
    strcpy( bufControlPvName, "" );

  if ( readPvExpStr.getRaw() )
    strncpy( bufReadPvName, readPvExpStr.getRaw(),
     activeGraphicClass::MAX_PV_NAME );
  else
    strcpy( bufReadPvName, "" );

  if ( visPvExpString.getRaw() )
    strncpy( bufVisPvName, visPvExpString.getRaw(),
     activeGraphicClass::MAX_PV_NAME );
  else
    strcpy( bufVisPvName, "" );

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

  ef.addTextField( activeMenuButtonClass_str4, 35, &bufX );
  ef.addTextField( activeMenuButtonClass_str5, 35, &bufY );
  ef.addTextField( activeMenuButtonClass_str6, 35, &bufW );
  ef.addTextField( activeMenuButtonClass_str7, 35, &bufH );
  ef.addTextField( activeMenuButtonClass_str17, 35, bufControlPvName,
   activeGraphicClass::MAX_PV_NAME );
  ef.addTextField( activeMenuButtonClass_str18, 35, bufReadPvName,
   activeGraphicClass::MAX_PV_NAME );

  ef.addColorButton( activeMenuButtonClass_str8, actWin->ci, &fgCb,
   &bufFgColor );
  ef.addToggle( activeMenuButtonClass_str10, &bufFgColorMode );

  ef.addColorButton( activeMenuButtonClass_str11, actWin->ci, &bgCb,
   &bufBgColor );

  ef.addColorButton( activeMenuButtonClass_str29, actWin->ci,
   &inconsistentCb, &bufInconsistentColor );

  ef.addColorButton( activeMenuButtonClass_str14, actWin->ci, &topShadowCb,
   &bufTopShadowColor );

  ef.addColorButton( activeMenuButtonClass_str15, actWin->ci, &botShadowCb,
   &bufBotShadowColor );

  ef.addFontMenu( activeMenuButtonClass_str16, actWin->fi, &fm, fontTag );

  XtUnmanageChild( fm.alignWidget() ); // no alignment info

  ef.addTextField( activeMenuButtonClass_str30, 30, bufVisPvName,
   activeGraphicClass::MAX_PV_NAME );
  ef.addOption( " ", activeMenuButtonClass_str31, &bufVisInverted );
  ef.addTextField( activeMenuButtonClass_str32, 30, bufMinVisString, 39 );
  ef.addTextField( activeMenuButtonClass_str33, 30, bufMaxVisString, 39 );

  return 1;

}

int activeMenuButtonClass::editCreate ( void ) {

  this->genericEdit();
  ef.finished( mbtc_edit_ok, mbtc_edit_apply, mbtc_edit_cancel_delete, this );
  actWin->currentEf = NULL;
  ef.popup();

  return 1;

}

int activeMenuButtonClass::edit ( void ) {

  this->genericEdit();
  ef.finished( mbtc_edit_ok, mbtc_edit_apply, mbtc_edit_cancel, this );
  actWin->currentEf = &ef;
  ef.popup();

  return 1;

}

int activeMenuButtonClass::erase ( void ) {

  if ( deleteRequest || activeMode ) return 1;

  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  return 1;

}

int activeMenuButtonClass::eraseActive ( void ) {

  if ( !init || !activeMode ) return 1;

  if ( prevVisibility == 0 ) {
    prevVisibility = visibility;
    return 1;
  }

  prevVisibility = visibility;

  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  return 1;

}

int activeMenuButtonClass::draw ( void ) {

int tX, tY, bumpX, bumpY;
XRectangle xR = { x+3, y, w-23, h };

  if ( deleteRequest || activeMode ) return 1;

  actWin->drawGc.saveFg();

  actWin->drawGc.setFG( bgColor.pixelColor() );

  actWin->drawGc.setLineStyle( LineSolid );
  actWin->drawGc.setLineWidth( 1 );

  XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x, y, w, h );

  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x, y, w, h );

  actWin->drawGc.setFG( actWin->ci->pix(botShadowColor) );

  XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x, y, x+w, y );

  XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x, y, x, y+h );

   actWin->drawGc.setFG( actWin->ci->pix(topShadowColor) );

   XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
    actWin->drawGc.normGC(), x, y+h, x+w, y+h );

   XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
    actWin->drawGc.normGC(), x+w, y, x+w, y+h );

  actWin->drawGc.setFG( actWin->ci->pix(topShadowColor) );

  XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x+1, y+1, x+w-1, y+1 );

  XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x+2, y+2, x+w-2, y+2 );

  XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x+1, y+1, x+1, y+h-1 );

  XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x+2, y+2, x+2, y+h-2 );

  actWin->drawGc.setFG( actWin->ci->pix(botShadowColor) );

  XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x+1, y+h-1, x+w-1, y+h-1 );

  XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x+2, y+h-2, x+w-2, y+h-2 );

  XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x+w-1, y+1, x+w-1, y+h-1 );

  XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x+w-2, y+2, x+w-2, y+h-2 );

  // draw bump

  bumpX = x+w-10-10;
  bumpY = y+h/2-5;

  actWin->drawGc.setFG( actWin->ci->pix(topShadowColor) );

  XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), bumpX, bumpY+10, bumpX, bumpY );

  XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), bumpX, bumpY, bumpX+10, bumpY );

  actWin->drawGc.setFG( actWin->ci->pix(botShadowColor) );

  XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), bumpX+10, bumpY, bumpX+10, bumpY+10 );

  XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), bumpX+10, bumpY+10, bumpX, bumpY+10 );

  if ( fs ) {

    actWin->drawGc.addNormXClipRectangle( xR );

    actWin->drawGc.setFG( fgColor.getColor() );
    actWin->drawGc.setFontTag( fontTag, actWin->fi );

    tX = x + w/2 - 10;
    tY = y + h/2 - fontAscent/2;

    drawText( actWin->drawWidget, &actWin->drawGc, fs, tX, tY,
     XmALIGNMENT_CENTER, "Menu" );

    actWin->drawGc.removeNormXClipRectangle();

  }

  actWin->drawGc.restoreFg();

  return 1;

}

int activeMenuButtonClass::drawActive ( void ) {

int tX, tY, bumpX, bumpY;
short v;
XRectangle xR = { x+3, y, w-23, h };
char string[MAX_ENUM_STRING_SIZE+1];

  if ( !init ) {
    if ( needToDrawUnconnected ) {
      actWin->executeGc.saveFg();
      actWin->executeGc.setFG( bgColor.getDisconnected() );
      actWin->executeGc.setLineWidth( 1 );
      actWin->executeGc.setLineStyle( LineSolid );
      XDrawRectangle( actWin->d, XtWindow(actWin->executeWidget),
       actWin->executeGc.normGC(), x, y, w, h );
      actWin->executeGc.restoreFg();
      needToEraseUnconnected = 1;
    }
  }
  else if ( needToEraseUnconnected ) {
    actWin->executeGc.setLineWidth( 1 );
    actWin->executeGc.setLineStyle( LineSolid );
    XDrawRectangle( actWin->d, XtWindow(actWin->executeWidget),
     actWin->executeGc.eraseGC(), x, y, w, h );
    needToEraseUnconnected = 0;
    eraseActive();
    smartDrawAllActive();
  }

  if ( !init || !activeMode || !visibility ) return 1;

  prevVisibility = visibility;

  actWin->executeGc.saveFg();
  actWin->executeGc.setLineWidth( 1 );
  actWin->executeGc.setLineStyle( LineSolid );

  if ( controlExists && readExists ) {

    if ( ( value != readValue ) || !controlValid || !readValid ) {
      actWin->executeGc.setFG( inconsistentColor.getColor() );
    }
    else {
      actWin->executeGc.setFG( bgColor.getColor() );
    }

    v = readValue;

  }
  else if ( readExists ) {

    actWin->executeGc.setFG( bgColor.getColor() );

    v = readValue;

  }
  else if ( controlExists ) {

    actWin->executeGc.setFG( bgColor.getColor() );

    v = value;

  }
  else {

    actWin->executeGc.setFG( inconsistentColor.getColor() );
    v = -1;
    init = 1;

  }

  XFillRectangle( actWin->d, XtWindow(actWin->executeWidget),
   actWin->executeGc.normGC(), x, y, w, h );

  XDrawRectangle( actWin->d, XtWindow(actWin->executeWidget),
   actWin->executeGc.normGC(), x, y, w, h );

  actWin->executeGc.setFG( actWin->ci->pix(botShadowColor) );

  XDrawLine( actWin->d, XtWindow(actWin->executeWidget),
   actWin->executeGc.normGC(), x, y, x+w, y );

  XDrawLine( actWin->d, XtWindow(actWin->executeWidget),
   actWin->executeGc.normGC(), x, y, x, y+h );

  actWin->executeGc.setFG( actWin->ci->pix(topShadowColor) );

  XDrawLine( actWin->d, XtWindow(actWin->executeWidget),
   actWin->executeGc.normGC(), x, y+h, x+w, y+h );

  XDrawLine( actWin->d, XtWindow(actWin->executeWidget),
   actWin->executeGc.normGC(), x+w, y, x+w, y+h );

  // top
  actWin->executeGc.setFG( actWin->ci->pix(topShadowColor) );

  XDrawLine( actWin->d, XtWindow(actWin->executeWidget),
   actWin->executeGc.normGC(), x+1, y+1, x+w-1, y+1 );

  XDrawLine( actWin->d, XtWindow(actWin->executeWidget),
   actWin->executeGc.normGC(), x+2, y+2, x+w-2, y+2 );

  // left
  XDrawLine( actWin->d, XtWindow(actWin->executeWidget),
   actWin->executeGc.normGC(), x+1, y+1, x+1, y+h-1 );

  XDrawLine( actWin->d, XtWindow(actWin->executeWidget),
   actWin->executeGc.normGC(), x+2, y+2, x+2, y+h-2 );

  // bottom
  actWin->executeGc.setFG( actWin->ci->pix(botShadowColor) );

  XDrawLine( actWin->d, XtWindow(actWin->executeWidget),
   actWin->executeGc.normGC(), x+1, y+h-1, x+w-1, y+h-1 );

  XDrawLine( actWin->d, XtWindow(actWin->executeWidget),
   actWin->executeGc.normGC(), x+2, y+h-2, x+w-2, y+h-2 );

  // right
  XDrawLine( actWin->d, XtWindow(actWin->executeWidget),
   actWin->executeGc.normGC(), x+w-1, y+1, x+w-1, y+h-1 );

  XDrawLine( actWin->d, XtWindow(actWin->executeWidget),
   actWin->executeGc.normGC(), x+w-2, y+2, x+w-2, y+h-2 );

  // draw bump

  bumpX = x+w-10-10;
  bumpY = y+h/2-5;

  actWin->executeGc.setFG( actWin->ci->pix(topShadowColor) );

  XDrawLine( actWin->d, XtWindow(actWin->executeWidget),
   actWin->executeGc.normGC(), bumpX, bumpY+10, bumpX, bumpY );

  XDrawLine( actWin->d, XtWindow(actWin->executeWidget),
   actWin->executeGc.normGC(), bumpX, bumpY, bumpX+10, bumpY );

  actWin->executeGc.setFG( actWin->ci->pix(botShadowColor) );

  XDrawLine( actWin->d, XtWindow(actWin->executeWidget),
   actWin->executeGc.normGC(), bumpX+10, bumpY, bumpX+10, bumpY+10 );

  XDrawLine( actWin->d, XtWindow(actWin->executeWidget),
   actWin->executeGc.normGC(), bumpX+10, bumpY+10, bumpX, bumpY+10 );

  if ( fs ) {

    actWin->executeGc.addNormXClipRectangle( xR );

    actWin->executeGc.setFG( fgColor.getColor() );
    actWin->executeGc.setFontTag( fontTag, actWin->fi );

    tX = x + w/2 - 10;
    tY = y + h/2 - fontAscent/2;

    if ( ( v >= 0 ) && ( v < numStates ) ) {
      strncpy( string, stateString[v], MAX_ENUM_STRING_SIZE );
    }
    else {
      strcpy( string, "?" );
    }

    drawText( actWin->executeWidget, &actWin->executeGc, fs, tX, tY,
     XmALIGNMENT_CENTER, string );

    actWin->executeGc.removeNormXClipRectangle();

  }

  actWin->executeGc.restoreFg();

  return 1;

}

int activeMenuButtonClass::expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int stat, retStat = 1;;

  stat = controlPvExpStr.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = readPvExpStr.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = visPvExpString.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;

  return retStat;

}

int activeMenuButtonClass::expand2nd (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int stat, retStat = 1;

  stat = controlPvExpStr.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = readPvExpStr.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = visPvExpString.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;

  return retStat;

}

int activeMenuButtonClass::containsMacros ( void ) {

  if ( controlPvExpStr.containsPrimaryMacros() ) return 1;

  if ( readPvExpStr.containsPrimaryMacros() ) return 1;

  if ( visPvExpString.containsPrimaryMacros() ) return 1;

  return 0;

}

int activeMenuButtonClass::createWidgets ( void ) {

  return 1;

}

int activeMenuButtonClass::activate (
  int pass,
  void *ptr
) {

int stat, opStat;

  switch ( pass ) {

  case 1:

    opComplete = 0;

    break;

  case 2:

    if ( !opComplete ) {

      aglPtr = ptr;
      needConnectInit = needInfoInit = needReadConnectInit = needReadInfoInit =
       needRefresh = needDraw = needVisConnectInit = needVisInit =
       needVisUpdate = 0;
      needToEraseUnconnected = 0;
      needToDrawUnconnected = 0;
      unconnectedTimer = 0;
      controlValid = readValid = 0;

      controlExists = readExists = visExists = 0;

      pvCheckExists = 0;
      connection.init();

#ifdef __epics__
      alarmEventId = controlEventId = readAlarmEventId = readEventId =
       visEventId = 0;
#endif

      init = 0;
      active = 0;
      activeMode = 1;
      numStates = 0;

      buttonPressed = 0;

      popUpMenu = (Widget) NULL;

      if ( !unconnectedTimer ) {
        unconnectedTimer = XtAppAddTimeOut( actWin->appCtx->appContext(),
         2000, unconnectedTimeout, this );
      }

      if ( !pvCheckExists ) {

        pvCheckExists = 1;

        if ( strcmp( controlPvExpStr.getRaw(), "" ) != 0 ) {
          controlExists = 1;
          connection.addPv(); // must do this only once per pv
	}
        else {
          controlExists = 0;
	}

        if ( strcmp( readPvExpStr.getRaw(), "" ) != 0 ) {
          readExists = 1;
          connection.addPv(); // must do this only once per pv
	}
        else {
          readExists = 0;
	}

        if ( strcmp( visPvExpString.getRaw(), "" ) != 0 ) {
          visExists = 1;
          connection.addPv(); // must do this only once per pv
        }
        else {
          visExists = 0;
          visibility = 1;
        }

      }

      opStat = 1;

#ifdef __epics__

      if ( controlExists ) {
        stat = ca_search_and_connect( controlPvExpStr.getExpanded(),
         &controlPvId, mbt_monitor_control_connect_state, this );
        if ( stat != ECA_NORMAL ) {
          printf( activeMenuButtonClass_str20,
           controlPvExpStr.getExpanded() );
          opStat = 0;
        }
      }

      if ( readExists ) {
        stat = ca_search_and_connect( readPvExpStr.getExpanded(),
         &readPvId, mbt_monitor_read_connect_state, this );
        if ( stat != ECA_NORMAL ) {
          printf( activeMenuButtonClass_str21,
           readPvExpStr.getExpanded() );
          opStat = 0;
        }
      }

      if ( visExists ) {

        stat = ca_search_and_connect( visPvExpString.getExpanded(), &visPvId,
         mbt_monitor_vis_connect_state, this );
        if ( stat != ECA_NORMAL ) {
          printf( activeMenuButtonClass_str21,
           visPvExpString.getExpanded() );
          opStat = 0;
        }
      }

      opComplete = opStat;

#endif

      return opStat;

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

int activeMenuButtonClass::deactivate (
  int pass
) {

int stat, i;

  active = 0;
  activeMode = 0;

  switch ( pass ) {

  case 1:

#ifdef __epics__

    if ( controlExists ) {
      stat = ca_clear_channel( controlPvId );
      if ( stat != ECA_NORMAL ) printf( activeMenuButtonClass_str23 );
    }

    if ( readExists ) {
      stat = ca_clear_channel( readPvId );
      if ( stat != ECA_NORMAL ) printf( activeMenuButtonClass_str23 );
    }

  if ( visExists ) {
    stat = ca_clear_channel( visPvId );
    if ( stat != ECA_NORMAL ) printf( activeMenuButtonClass_str23 );
  }

#endif

    break;

  case 2:

    if ( widgetsCreated ) {
      for ( i=0; i<numStates; i++ ) {
        XtDestroyWidget( pb[i] );
      }
      XtDestroyWidget( pullDownMenu );
      XtDestroyWidget( popUpMenu );
      widgetsCreated = 0;
    }

    for ( i=0; i<numStates; i++ ) {
      if ( stateString[i] ) {
        delete stateString[i];
        stateString[i] = NULL;
      }
    }

    break;

  }

  return 1;

}

void activeMenuButtonClass::updateDimensions ( void )
{

  if ( fs ) {
    fontAscent = fs->ascent;
    fontDescent = fs->descent;
    fontHeight = fontAscent + fontDescent;
  }
  else {
    fontAscent = 10;
    fontDescent = 5;
    fontHeight = fontAscent + fontDescent;
  }

}

void activeMenuButtonClass::btnUp (
  int _x,
  int _y,
  int buttonState,
  int buttonNumber,
  int *action )
{

  *action = 0;

  if ( !init || !visibility ) return;

  if ( !buttonPressed ) return;

  buttonPressed = 0;

//    printf( "btn up\n" );

}

void activeMenuButtonClass::btnDown (
  int _x,
  int _y,
  int buttonState,
  int buttonNumber,
  int *action )
{

XButtonEvent be;

  *action = 0;

  if ( !init || !visibility ) return;

  if ( !controlExists ) return;

  if ( !ca_write_access( controlPvId ) ) return;

  // printf( "btn down, x=%-d, y=%-d\n", _x-x, _y-y );

  if ( buttonNumber == 1 ) {

    buttonPressed = 1;

    memset( (void *) &be, 0, sizeof(XButtonEvent) );
    be.x_root = actWin->x+_x;
    be.y_root = actWin->y+_y;
    XmMenuPosition( popUpMenu, &be );
    XtManageChild( popUpMenu );

  }

}

void activeMenuButtonClass::pointerIn (
  int _x,
  int _y,
  int buttonState )
{

  if ( !init || !visibility ) return;

  if ( !ca_write_access( controlPvId ) ) {
    actWin->cursor.set( XtWindow(actWin->executeWidget), CURSOR_K_NO );
  }
  else {
    actWin->cursor.set( XtWindow(actWin->executeWidget), CURSOR_K_DEFAULT );
  }

  activeGraphicClass::pointerIn( _x, _y, buttonState );

}

int activeMenuButtonClass::getButtonActionRequest (
  int *up,
  int *down,
  int *drag,
  int *focus )
{

  *drag = 0;

  if ( controlExists )
    *focus = 1;
  else
    *focus = 0;

  if ( !controlExists ) {
    *up = 0;
    *down = 0;
    return 1;
  }

  *down = 1;
  *up = 1;

  return 1;

}

void activeMenuButtonClass::executeDeferred ( void ) {

short v, rV;
int stat, i, nc, nrc, ni, nri, nr, nd, nvc, nvi, nvu;
XmString str;
Arg args[15];
int n;

  if ( actWin->isIconified ) return;

//----------------------------------------------------------------------------

  actWin->appCtx->proc->lock();
  nc = needConnectInit; needConnectInit = 0;
  nrc = needReadConnectInit; needReadConnectInit = 0;
  ni = needInfoInit; needInfoInit = 0;
  nri = needReadInfoInit; needReadInfoInit = 0;
  nr = needRefresh; needRefresh = 0;
  nd = needDraw; needDraw = 0;
  nvc = needVisConnectInit; needVisConnectInit = 0;
  nvi = needVisInit; needVisInit = 0;
  nvu = needVisUpdate; needVisUpdate = 0;
  v = curValue;
  rV = curReadValue;
  visValue = curVisValue;
  actWin->remDefExeNode( aglPtr );
  actWin->appCtx->proc->unlock();

  if ( !activeMode ) return;

//----------------------------------------------------------------------------

#ifdef __epics__

  if ( nc ) {

    stat = ca_get_callback( DBR_GR_ENUM, controlPvId,
     mbt_infoUpdate, (void *) this );

  }

  if ( nrc ) {

    stat = ca_get_callback( DBR_GR_ENUM, readPvId,
     mbt_readInfoUpdate, (void *) this );

  }

  if ( ni ) {

    value = v;

    if ( widgetsCreated ) {

      for ( i=0; i<numStates; i++ ) {
        XtDestroyWidget( pb[i] );
      }
      XtDestroyWidget( pullDownMenu );
      XtDestroyWidget( popUpMenu );

      widgetsCreated = 0;

    }

    n = 0;
    XtSetArg( args[n], XmNmenuPost, (XtArgVal) "<Btn5Down>;" ); n++;
    popUpMenu = XmCreatePopupMenu( actWin->topWidgetId(), "", args, n );

    pullDownMenu = XmCreatePulldownMenu( popUpMenu, "", NULL, 0 );

    for ( i=0; i<numStates; i++ ) {

      //str = XmStringCreate( stateString[i], fontTag );
      str = XmStringCreateLocalized( stateString[i] );

      pb[i] = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
       popUpMenu,
       XmNlabelString, str,
				       //XmNfontList, fontList,
       NULL );

      XmStringFree( str );

      XtAddCallback( pb[i], XmNactivateCallback, menu_cb,
       (XtPointer) this );

    }

    widgetsCreated = 1;

    if ( !controlEventId ) {

      stat = ca_add_masked_array_event( DBR_ENUM, 1, controlPvId,
       mbt_controlUpdate, (void *) this, (float) 0.0, (float) 0.0,
       (float) 0.0, &controlEventId, DBE_VALUE );
      if ( stat != ECA_NORMAL ) {
        printf( activeMenuButtonClass_str24 );
      }

    }

    if ( !alarmEventId ) {

      stat = ca_add_masked_array_event( DBR_STS_ENUM, 1, controlPvId,
       mbt_alarmUpdate, (void *) this, (float) 0.0, (float) 0.0,
       (float) 0.0, &alarmEventId, DBE_ALARM );
      if ( stat != ECA_NORMAL ) {
        printf( activeMenuButtonClass_str25 );
      }

    }

    if ( connection.pvsConnected() ) {
      init = 1;
      active = 1;
      drawActive();
    }

  }

  if ( nri ) {

    curReadValue = rV;

    if ( !readEventId ) {

      stat = ca_add_masked_array_event( DBR_ENUM, 1, readPvId,
       mbt_readUpdate, (void *) this, (float) 0.0, (float) 0.0,
       (float) 0.0, &readEventId, DBE_VALUE );
      if ( stat != ECA_NORMAL ) {
        printf( activeMenuButtonClass_str26 );
      }

    }

    if ( !readAlarmEventId ) {

      stat = ca_add_masked_array_event( DBR_STS_ENUM, 1, readPvId,
       mbt_readAlarmUpdate, (void *) this, (float) 0.0, (float) 0.0,
       (float) 0.0, &readAlarmEventId, DBE_ALARM );
      if ( stat != ECA_NORMAL ) {
        printf( activeMenuButtonClass_str27 );
      }

    }

    if ( connection.pvsConnected() ) {
      init = 1;
      active = 1;
      drawActive();
    }

  }

  if ( nvc ) {

    minVis = atof( minVisString );
    maxVis = atof( maxVisString );

    connection.setPvConnected( (void *) visPvConnection );

    stat = ca_get_callback( DBR_GR_DOUBLE, visPvId,
     mbt_visInfoUpdate, (void *) this );

  }

  if ( nvi ) {

    stat = ca_add_masked_array_event( DBR_DOUBLE, 1, visPvId,
     mbt_visUpdate, (void *) this, (float) 0.0, (float) 0.0, (float) 0.0,
     &visEventId, DBE_VALUE );
    if ( stat != ECA_NORMAL ) printf( activeMenuButtonClass_str27 );

    if ( ( visValue >= minVis ) &&
         ( visValue < maxVis ) )
      visibility = 1 ^ visInverted;
    else
      visibility = 0 ^ visInverted;

    if ( prevVisibility != visibility ) {
      if ( !visibility ) eraseActive();
    }

    if ( connection.pvsConnected() ) {
      active = 1;
      init = 1;
      fgColor.setConnected();
      smartDrawAllActive();
    }

  }

#endif

//----------------------------------------------------------------------------

  if ( nr ) {
    readValue = rV;
    value = v;
    eraseActive();
    drawActive();
  }

//----------------------------------------------------------------------------

  if ( nd ) {
    drawActive();
  }

//----------------------------------------------------------------------------

  if ( nvu ) {

    if ( ( visValue >= minVis ) &&
         ( visValue < maxVis ) )
      visibility = 1 ^ visInverted;
    else
      visibility = 0 ^ visInverted;

    if ( prevVisibility != visibility ) {
      if ( !visibility ) eraseActive();
      stat = smartDrawAllActive();
    }

  }

}

char *activeMenuButtonClass::firstDragName ( void ) {

  dragIndex = 0;
  return dragName[dragIndex];

}

char *activeMenuButtonClass::nextDragName ( void ) {

  if ( dragIndex < (int) ( sizeof(dragName) / sizeof(char *) ) - 1 ) {
    dragIndex++;
    return dragName[dragIndex];
  }
  else {
    return NULL;
  }

}

char *activeMenuButtonClass::dragValue (
  int i ) {

  if ( i == 0 ) {
    return controlPvExpStr.getExpanded();
  }
  else if ( i == 1 ) {
    return readPvExpStr.getExpanded();
  }
  else {
    return visPvExpString.getExpanded();
  }

}

void activeMenuButtonClass::changeDisplayParams (
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

  if ( _flag & ACTGRF_TEXTFGCOLOR_MASK )
    fgColor.setColorIndex( _textFgColor, actWin->ci );

  if ( _flag & ACTGRF_BGCOLOR_MASK )
    bgColor.setColorIndex( _bgColor, actWin->ci );

  if ( _flag & ACTGRF_TOPSHADOWCOLOR_MASK )
    topShadowColor = _topShadowColor;

  if ( _flag & ACTGRF_BOTSHADOWCOLOR_MASK )
    botShadowColor = _botShadowColor;

  if ( _flag & ACTGRF_BTNFONTTAG_MASK ) {

    strcpy( fontTag, _btnFontTag );
    actWin->fi->loadFontTag( fontTag );
    fs = actWin->fi->getXFontStruct( fontTag );
    actWin->fi->getTextFontList( fontTag, &fontList );

    updateDimensions();

  }

}

void activeMenuButtonClass::changePvNames (
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

  if ( flag & ACTGRF_CTLPVS_MASK ) {
    if ( numCtlPvs ) {
      controlPvExpStr.setRaw( ctlPvs[0] );
    }
  }

  if ( flag & ACTGRF_VISPVS_MASK ) {
    if ( numVisPvs ) {
      visPvExpString.setRaw( ctlPvs[0] );
    }
  }

}

#ifdef __cplusplus
extern "C" {
#endif

void *create_activeMenuButtonClassPtr ( void ) {

activeMenuButtonClass *ptr;

  ptr = new activeMenuButtonClass;
  return (void *) ptr;

}

void *clone_activeMenuButtonClassPtr (
  void *_srcPtr )
{

activeMenuButtonClass *ptr, *srcPtr;

  srcPtr = (activeMenuButtonClass *) _srcPtr;

  ptr = new activeMenuButtonClass( srcPtr );

  return (void *) ptr;

}

#ifdef __cplusplus
}
#endif
