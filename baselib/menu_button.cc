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

#ifdef __epics__

static void putValue (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeMenuButtonClass *mbto = (activeMenuButtonClass *) client;
int i, stat;
short value;
Arg args[2];
int n;

  for ( i=0; i<mbto->numStates; i++ ) {

    if ( w == mbto->pb[i] ) {
      value = (short) i;
      stat = ca_put( DBR_ENUM, mbto->controlPvId, &value );
      break;
    }

  }

  if ( ( mbto->curValue >= 0 ) && ( mbto->curValue < mbto->numStates ) ) {
    mbto->curHistoryWidget = mbto->pb[mbto->curValue];
    n = 0;
    XtSetArg( args[n], XmNmenuHistory, (XtArgVal) mbto->curHistoryWidget );
    n++;
    XtSetValues( mbto->optionMenu, args, n );
  }

}

static void mbt_monitor_control_connect_state (
  struct connection_handler_args arg )
{

activeMenuButtonClass *mbto = (activeMenuButtonClass *) ca_puser(arg.chid);

  if ( arg.op == CA_OP_CONN_UP ) {

    mbto->connection.setPvConnected( mbto->controlPvId );
    mbto->needConnectInit = 1;

    if ( mbto->connection.pvsConnected() ) {
      mbto->fgColor.setConnected();
    }

  }
  else {

    mbto->connection.setPvDisconnected( mbto->controlPvId );
    mbto->fgColor.setDisconnected();
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

    mbto->connection.setPvConnected( mbto->readPvId );
    mbto->needReadConnectInit = 1;

    if ( mbto->connection.pvsConnected() ) {
      mbto->fgColor.setConnected();
    }

  }
  else {

    mbto->connection.setPvDisconnected( mbto->readPvId );
    mbto->fgColor.setDisconnected();
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

  mbto->curValue = enumRec.value;

  mbto->needReadInfoInit = 1;
  mbto->actWin->appCtx->proc->lock();
  mbto->actWin->addDefExeNode( mbto->aglPtr );
  mbto->actWin->appCtx->proc->unlock();

}

static void mbt_readUpdate (
  struct event_handler_args ast_args )
{

activeMenuButtonClass *mbto = (activeMenuButtonClass *) ast_args.usr;

  mbto->curValue = *( (short *) ast_args.dbr );

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

  connection.setMaxPvs( 2 );

}

activeMenuButtonClass::~activeMenuButtonClass ( void ) {

  if ( name ) delete name;
  if ( fontList ) XmFontListFree( fontList );

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
  fgCb = source->fgCb;
  bgCb = source->bgCb;

  fgColorMode = source->fgColorMode;
  bgColorMode = source->bgColorMode;

  controlPvExpStr.setRaw( source->controlPvExpStr.rawString );
  readPvExpStr.setRaw( source->readPvExpStr.rawString );

  widgetsCreated = 0;
  active = 0;
  activeMode = 0;

  connection.setMaxPvs( 2 );

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
  fprintf( f, "%-d\n", index );

  fprintf( f, "%-d\n", fgColorMode );

  index = bgColor.pixelIndex();
  fprintf( f, "%-d\n", index );

  fprintf( f, "%-d\n", bgColorMode );

  index = topShadowColor;
  fprintf( f, "%-d\n", index );

  index = botShadowColor;
  fprintf( f, "%-d\n", index );

  if ( controlPvExpStr.getRaw() )
    writeStringToFile( f, controlPvExpStr.getRaw() );
  else
    writeStringToFile( f, "" );

  writeStringToFile( f, fontTag );

  if ( readPvExpStr.getRaw() )
    writeStringToFile( f, readPvExpStr.getRaw() );
  else
    writeStringToFile( f, "" );

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

    if ( major > 1 ) {

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

  readStringFromFile( oneName, 39, f ); actWin->incLine();
  controlPvExpStr.setRaw( oneName );

  readStringFromFile( fontTag, 63, f ); actWin->incLine();

  if ( ( major > 1 ) || ( minor > 2 ) ) {
    readStringFromFile( oneName, 39, f ); actWin->incLine();
    readPvExpStr.setRaw( oneName );
  }
  else {
    readPvExpStr.setRaw( "" );
  }

  actWin->fi->loadFontTag( fontTag );
  actWin->drawGc.setFontTag( fontTag, actWin->fi );

  fs = actWin->fi->getXFontStruct( fontTag );
  actWin->fi->getTextFontList( fontTag, &fontList );

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

  if ( controlPvExpStr.getRaw() )
    strncpy( bufControlPvName, controlPvExpStr.getRaw(), 39 );
  else
    strncpy( bufControlPvName, "", 39 );

  if ( readPvExpStr.getRaw() )
    strncpy( bufReadPvName, readPvExpStr.getRaw(), 39 );
  else
    strncpy( bufReadPvName, "", 39 );

  ef.create( actWin->top, actWin->appCtx->ci.getColorMap(),
   &actWin->appCtx->entryFormX,
   &actWin->appCtx->entryFormY, &actWin->appCtx->entryFormW,
   &actWin->appCtx->entryFormH, &actWin->appCtx->largestH,
   title, NULL, NULL, NULL );

  ef.addTextField( activeMenuButtonClass_str4, 30, &bufX );
  ef.addTextField( activeMenuButtonClass_str5, 30, &bufY );
  ef.addTextField( activeMenuButtonClass_str6, 30, &bufW );
  ef.addTextField( activeMenuButtonClass_str7, 30, &bufH );
  ef.addTextField( activeMenuButtonClass_str17, 30, bufControlPvName, 39 );
  ef.addTextField( activeMenuButtonClass_str18, 30, bufReadPvName, 39 );

  ef.addColorButton( activeMenuButtonClass_str8, actWin->ci, &fgCb,
   &bufFgColor );
  ef.addToggle( activeMenuButtonClass_str10, &bufFgColorMode );

  ef.addColorButton( activeMenuButtonClass_str11, actWin->ci, &bgCb,
   &bufBgColor );

  ef.addColorButton( activeMenuButtonClass_str14, actWin->ci, &topShadowCb,
   &bufTopShadowColor );

  ef.addColorButton( activeMenuButtonClass_str15, actWin->ci, &botShadowCb,
   &bufBotShadowColor );

  ef.addFontMenu( activeMenuButtonClass_str16, actWin->fi, &fm, fontTag );

  XtUnmanageChild( fm.alignWidget() ); // no alignment info

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

  return 1;

}

int activeMenuButtonClass::draw ( void ) {

int tX, tY;
XRectangle xR = { x, y, w, h };

  actWin->drawGc.saveFg();

  if ( deleteRequest || activeMode ) return 1;

  actWin->drawGc.setFG( bgColor.pixelColor() );
  XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x, y, w, h );

  actWin->drawGc.setFG( fgColor.pixelColor() );
  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x, y, w, h );

  if ( fs ) {

    actWin->drawGc.addNormXClipRectangle( xR );

    actWin->drawGc.setFontTag( fontTag, actWin->fi );

    tX = x + w/2;
    tY = y + h/2 - fontAscent/2;

    drawText( actWin->drawWidget, &actWin->drawGc, fs, tX, tY,
	      XmALIGNMENT_CENTER, activeMenuButtonClass_str19 );

    actWin->drawGc.removeNormXClipRectangle();

  }

  actWin->drawGc.restoreFg();

  return 1;

}

int activeMenuButtonClass::drawActive ( void ) {

Arg args[10];
int n;

  if ( !activeMode || !widgetsCreated ) return 1;

  // set color
  n = 0;
  XtSetArg( args[n], XmNforeground, fgColor.getColor() ); n++;
  XtSetValues( optionMenu, args, n );

  return 1;

}

int activeMenuButtonClass::expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int stat;

  stat = controlPvExpStr.expand1st( numMacros, macros, expansions );
  stat = readPvExpStr.expand1st( numMacros, macros, expansions );

  return stat;

}

int activeMenuButtonClass::expand2nd (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int stat;

  stat = controlPvExpStr.expand2nd( numMacros, macros, expansions );
  stat = readPvExpStr.expand2nd( numMacros, macros, expansions );

  return stat;

}

int activeMenuButtonClass::containsMacros ( void ) {

  if ( controlPvExpStr.containsPrimaryMacros() ) return 1;

  if ( readPvExpStr.containsPrimaryMacros() ) return 1;

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

    aglPtr = ptr;
    needConnectInit = needInfoInit = needReadConnectInit = needReadInfoInit =
     needRefresh = needDraw = 0;
    opComplete = 0;

    controlExists = readExists = 0;

    pvCheckExists = 0;
    connection.init();

#ifdef __epics__
    alarmEventId = controlEventId = readAlarmEventId = readEventId = 0;
#endif

    active = 0;
    activeMode = 1;
    numStates = 0;

    optionMenu = (Widget) NULL;
    pulldownMenu = (Widget) NULL;

    break;

  case 2:

    if ( !opComplete ) {

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

    if ( controlExists && !readExists ) {
      stat = ca_clear_channel( controlPvId );
      if ( stat != ECA_NORMAL )
        printf( activeMenuButtonClass_str22 );
    }

    if ( readExists ) {
      stat = ca_clear_channel( readPvId );
      if ( stat != ECA_NORMAL )
        printf( activeMenuButtonClass_str23 );
    }

#endif

    break;

  case 2:

    if ( widgetsCreated ) {
      if ( optionMenu ) {
        XtUnmapWidget( optionMenu );
        XtDestroyWidget( optionMenu );
      }
      if ( pulldownMenu ) {
        XtDestroyWidget( pulldownMenu );
      }
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

static void dummy (
   Widget w,
   XEvent *e,
   String *params,
   Cardinal numParams )
{

}

static void drag (
   Widget w,
   XEvent *e,
   String *params,
   Cardinal numParams )
{

class activeMenuButtonClass *ambo;
int stat;

  XtVaGetValues( w, XmNuserData, &ambo, NULL );

  stat = ambo->startDrag( w, e );

}

static void selectDrag (
   Widget w,
   XEvent *e,
   String *params,
   Cardinal numParams )
{

class activeMenuButtonClass *ambo;
int stat;
XButtonEvent *be = (XButtonEvent *) e;

  XtVaGetValues( w, XmNuserData, &ambo, NULL );

  stat = ambo->selectDragValue( ambo->x + be->x, ambo->y + be->y );

}

void activeMenuButtonClass::executeDeferred ( void ) {

short v;
int stat, i, nc, nrc, ni, nri, nr, nd;
XmString str;
Arg args[15];
int n;
XtTranslations parsedTrans;
Widget widget;

static char dragTrans[] =
  "#override\n\
   ~Shift<Btn2Down>: startDrag()\n\
   Shift<Btn2Down>: dummy()\n\
   Shift<Btn2Up>: selectDrag()";

static XtActionsRec dragActions[] = {
  { "startDrag", (XtActionProc) drag },
  { "dummy", (XtActionProc) dummy },
  { "selectDrag", (XtActionProc) selectDrag }
};

  if ( actWin->isIconified ) return;

//----------------------------------------------------------------------------

  actWin->appCtx->proc->lock();
  nc = needConnectInit; needConnectInit = 0;
  nrc = needReadConnectInit; needReadConnectInit = 0;
  ni = needInfoInit; needInfoInit = 0;
  nri = needReadInfoInit; needReadInfoInit = 0;
  nr = needRefresh; needRefresh = 0;
  nd = needDraw; needDraw = 0;
  v = curValue;
  actWin->remDefExeNode( aglPtr );
  actWin->appCtx->proc->unlock();

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
      if ( optionMenu ) {
        XtUnmapWidget( optionMenu );
        XtDestroyWidget( optionMenu );
      }
      if ( pulldownMenu ) {
        XtDestroyWidget( pulldownMenu );
      }
      widgetsCreated = 0;
    }

    pulldownMenu = XmCreatePulldownMenu( actWin->executeWidgetId(),
     "", NULL, 0 );

    for ( i=0; i<numStates; i++ ) {

      str = XmStringCreate( stateString[i], fontTag );

      pb[i] = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
       pulldownMenu,
       XmNlabelString, str,
       XmNfontList, fontList,
       NULL );

      n = 0;
      XtSetArg( args[n], XmNwidth, w-42 ); n++;
      XtSetArg( args[n], XmNheight, h-14 ); n++;

      if ( controlExists ) {
        if ( ca_write_access(controlPvId) ) {
          XtSetArg( args[n], XmNsensitive, True ); n++;
	}
	else {
          XtSetArg( args[n], XmNsensitive, False ); n++;
	}
      }
      else {
        XtSetArg( args[n], XmNsensitive, False ); n++;
      }

      XtSetValues( pb[i], args, n );

      XtAddCallback( pb[i], XmNactivateCallback, putValue,
       (XtPointer) this );

      XmStringFree( str );

    }

    curHistoryWidget = pb[value];

    parsedTrans = XtParseTranslationTable( dragTrans );
    XtAppAddActions( actWin->appCtx->appContext(), dragActions,
     XtNumber(dragActions) );

    n = 0;
    XtSetArg( args[n], XmNsubMenuId, (XtArgVal) pulldownMenu ); n++;
    XtSetArg( args[n], XmNmenuHistory, (XtArgVal) curHistoryWidget ); n++;
    XtSetArg( args[n], XmNx, (XtArgVal) x ); n++;
    XtSetArg( args[n], XmNy, (XtArgVal) y ); n++;
    XtSetArg( args[n], XmNbackground,
     (XtArgVal) actWin->executeGc.getBaseBG() ); n++;
    XtSetArg( args[n], XmNhighlightColor,
     (XtArgVal) actWin->executeGc.getBaseBG() ); n++;
    XtSetArg( args[n], XmNhighlightPixmap, (XtArgVal) None ); n++;
    XtSetArg( args[n], XmNtopShadowColor,
     (XtArgVal) actWin->ci->pix(topShadowColor) ); n++;
    XtSetArg( args[n], XmNbottomShadowColor,
     (XtArgVal) actWin->ci->pix(botShadowColor) );
     n++;
    XtSetArg( args[n], XmNtranslations, parsedTrans ); n++;
    XtSetArg( args[n], XmNuserData, this ); n++;

    optionMenu = XmCreateOptionMenu( actWin->executeWidgetId(), "",
     args, n );

    widget = XmOptionButtonGadget( optionMenu );
    n = 0;
    XtSetArg( args[n], XmNtranslations, parsedTrans ); n++;
    XtSetArg( args[n], XmNuserData, this ); n++;
    XtSetValues( widget, args, n );

    XtManageChild( optionMenu );

    widgetsCreated = 1;

    if ( !readExists ) {

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

    }

    active = 1;

    if ( ( value >= 0 ) && ( value < numStates ) ) {
      curHistoryWidget = pb[value];
      n = 0;
      XtSetArg( args[n], XmNmenuHistory, (XtArgVal) curHistoryWidget );
      n++;
      XtSetValues( optionMenu, args, n );
    }

    drawActive();

  }

  if ( nri ) {

    value = v;

    if ( !controlExists ) {

      if ( widgetsCreated ) {
        if ( optionMenu ) {
          XtUnmapWidget( optionMenu );
          XtDestroyWidget( optionMenu );
        }
        if ( pulldownMenu ) {
          XtDestroyWidget( pulldownMenu );
        }
        widgetsCreated = 0;
      }

      pulldownMenu = XmCreatePulldownMenu( actWin->executeWidgetId(),
       "", NULL, 0 );

      for ( i=0; i<numStates; i++ ) {

        str = XmStringCreate( stateString[i], fontTag );

        pb[i] = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
         pulldownMenu,
         XmNlabelString, str,
         XmNfontList, fontList,
         NULL );

        n = 0;
        XtSetArg( args[n], XmNwidth, w-42 ); n++;
        XtSetArg( args[n], XmNheight, h-14 ); n++;
        XtSetArg( args[n], XmNsensitive, FALSE ); n++;
        XtSetValues( pb[i], args, n );

        XmStringFree( str );

      }

      curHistoryWidget = pb[value];

      parsedTrans = XtParseTranslationTable( dragTrans );
      XtAppAddActions( actWin->appCtx->appContext(), dragActions,
       XtNumber(dragActions) );

      n = 0;
      XtSetArg( args[n], XmNsubMenuId, (XtArgVal) pulldownMenu ); n++;
      XtSetArg( args[n], XmNmenuHistory, (XtArgVal) curHistoryWidget ); n++;
      XtSetArg( args[n], XmNx, (XtArgVal) x ); n++;
      XtSetArg( args[n], XmNy, (XtArgVal) y ); n++;
      XtSetArg( args[n], XmNbackground,
       (XtArgVal) actWin->executeGc.getBaseBG() ); n++;
      XtSetArg( args[n], XmNhighlightColor,
       (XtArgVal) actWin->executeGc.getBaseBG() ); n++;
      XtSetArg( args[n], XmNhighlightPixmap, (XtArgVal) None ); n++;
      XtSetArg( args[n], XmNtopShadowColor,
       (XtArgVal) actWin->ci->pix(topShadowColor) ); n++;
      XtSetArg( args[n], XmNbottomShadowColor,
       (XtArgVal) actWin->ci->pix(botShadowColor) );
       n++;
      XtSetArg( args[n], XmNtranslations, parsedTrans ); n++;
      XtSetArg( args[n], XmNuserData, this ); n++;

      optionMenu = XmCreateOptionMenu( actWin->executeWidgetId(), "",
       args, n );

      widget = XmOptionButtonGadget( optionMenu );
      n = 0;
      XtSetArg( args[n], XmNtranslations, parsedTrans ); n++;
      XtSetArg( args[n], XmNuserData, this ); n++;
      XtSetValues( widget, args, n );

      XtManageChild( optionMenu );

      widgetsCreated = 1;

    }

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

    if ( !controlExists ) {

      active = 1;

      if ( ( value >= 0 ) && ( value < numStates ) ) {
        curHistoryWidget = pb[value];
        n = 0;
        XtSetArg( args[n], XmNmenuHistory, (XtArgVal) curHistoryWidget );
        n++;
        XtSetValues( optionMenu, args, n );
      }

      drawActive();

    }

  }

#endif

//----------------------------------------------------------------------------

  if ( nr ) {

    value = v;
    if ( ( value >= 0 ) && ( value < numStates ) ) {
      curHistoryWidget = pb[value];
      n = 0;
      XtSetArg( args[n], XmNmenuHistory, (XtArgVal) curHistoryWidget );
      n++;
      XtSetValues( optionMenu, args, n );
    }

  }

//----------------------------------------------------------------------------

  if ( nd ) {
    drawActive();
  }

//----------------------------------------------------------------------------

}

char *activeMenuButtonClass::firstDragName ( void ) {

  dragIndex = 0;
  return dragName[dragIndex];

}

char *activeMenuButtonClass::nextDragName ( void ) {

  return NULL;

}

char *activeMenuButtonClass::dragValue (
  int i ) {

  return controlPvExpStr.getExpanded();

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
