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

#define __radio_button_cc 1

#include "radio_button.h"
#include "app_pkg.h"
#include "act_win.h"

#include "thread.h"

#include "Xm/CascadeBG.h"

static void unconnectedTimeout (
  XtPointer client,
  XtIntervalId *id )
{

activeRadioButtonClass *rbto = (activeRadioButtonClass *) client;

  if ( !rbto->connection.pvsConnected() ) {
    rbto->needToDrawUnconnected = 1;
    rbto->needDraw = 1;
    rbto->actWin->addDefExeNode( rbto->aglPtr );
  }

  rbto->unconnectedTimer = 0;

}

static void radioBoxEventHandler (
  Widget w,
  XtPointer client,
  XEvent *e,
  Boolean *continueToDispatch ) {

activeRadioButtonClass *rbto = (activeRadioButtonClass *) client;

  if ( !rbto->active ) return;

  if ( e->type == EnterNotify ) {
    if ( !ca_write_access( rbto->controlPvId ) ) {
      rbto->actWin->cursor.set( XtWindow(rbto->actWin->executeWidget),
       CURSOR_K_NO );
    }
    else {
      rbto->actWin->cursor.set( XtWindow(rbto->actWin->executeWidget),
       CURSOR_K_DEFAULT );
    }
  }

  if ( e->type == LeaveNotify ) {
    rbto->actWin->cursor.set( XtWindow(rbto->actWin->executeWidget),
     CURSOR_K_DEFAULT );
  }

}

#ifdef __epics__

static void putValue (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeRadioButtonClass *rbto = (activeRadioButtonClass *) client;
int i, stat;

  for ( i=0; i<rbto->numStates; i++ ) {

    if ( w == rbto->pb[i] ) {
      rbto->curValue = i;
      stat = ca_put( DBR_ENUM, rbto->controlPvId, &rbto->curValue );
      break;
    }

  }

#if 0
  rbto->actWin->appCtx->proc->lock();
  rbto->needRefresh = 1;
  rbto->needDraw = 1;
  rbto->actWin->addDefExeNode( rbto->aglPtr );
  rbto->actWin->appCtx->proc->unlock();
#endif

}

static void rbt_monitor_control_connect_state (
  struct connection_handler_args arg )
{

activeRadioButtonClass *rbto = (activeRadioButtonClass *) ca_puser(arg.chid);

  if ( arg.op == CA_OP_CONN_UP ) {

    rbto->connection.setPvConnected( rbto->controlPvId );
    rbto->needConnectInit = 1;

    if ( rbto->connection.pvsConnected() ) {
      rbto->fgColor.setConnected();
    }

  }
  else {

    rbto->connection.setPvDisconnected( rbto->controlPvId );
    rbto->fgColor.setDisconnected();
    rbto->needRefresh = 1;
    rbto->needDraw = 1;
    rbto->active = 0;

  }

  rbto->actWin->appCtx->proc->lock();
  rbto->actWin->addDefExeNode( rbto->aglPtr );
  rbto->actWin->appCtx->proc->unlock();

}

static void rbt_infoUpdate (
  struct event_handler_args ast_args )
{

int i;
activeRadioButtonClass *rbto = (activeRadioButtonClass *) ast_args.usr;
struct dbr_gr_enum enumRec;

  enumRec = *( (struct dbr_gr_enum *) ast_args.dbr );

  rbto->numStates = enumRec.no_str;

  for ( i=0; i<rbto->numStates; i++ ) {

    if ( rbto->stateString[i] == NULL ) {
      rbto->stateString[i] = new char[MAX_ENUM_STRING_SIZE+1];
    }

    strncpy( rbto->stateString[i], enumRec.strs[i], MAX_ENUM_STRING_SIZE );

  }

  rbto->curValue = enumRec.value;

  rbto->needInfoInit = 1;
  rbto->actWin->appCtx->proc->lock();
  rbto->actWin->addDefExeNode( rbto->aglPtr );
  rbto->actWin->appCtx->proc->unlock();

}

static void rbt_controlUpdate (
  struct event_handler_args ast_args )
{

activeRadioButtonClass *rbto = (activeRadioButtonClass *) ast_args.usr;

  rbto->curValue = *( (short *) ast_args.dbr );

  rbto->needRefresh = 1;
  rbto->needDraw = 1;
  rbto->actWin->appCtx->proc->lock();
  rbto->actWin->addDefExeNode( rbto->aglPtr );
  rbto->actWin->appCtx->proc->unlock();

}

static void rbt_alarmUpdate (
  struct event_handler_args ast_args )
{

activeRadioButtonClass *rbto = (activeRadioButtonClass *) ast_args.usr;
struct dbr_sts_enum statusRec;

  statusRec = *( (struct dbr_sts_enum *) ast_args.dbr );

  rbto->fgColor.setStatus( statusRec.status, statusRec.severity );
  rbto->bgColor.setStatus( statusRec.status, statusRec.severity );

  rbto->needDraw = 1;
  rbto->actWin->appCtx->proc->lock();
  rbto->actWin->addDefExeNode( rbto->aglPtr );
  rbto->actWin->appCtx->proc->unlock();

}

#endif

static void rbtc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeRadioButtonClass *rbto = (activeRadioButtonClass *) client;

  rbto->actWin->setChanged();

  rbto->eraseSelectBoxCorners();
  rbto->erase();

  strncpy( rbto->fontTag, rbto->fm.currentFontTag(), 63 );
  rbto->actWin->fi->loadFontTag( rbto->fontTag );
  rbto->actWin->drawGc.setFontTag( rbto->fontTag, rbto->actWin->fi );
  rbto->actWin->fi->getTextFontList( rbto->fontTag, &rbto->fontList );
  rbto->fs = rbto->actWin->fi->getXFontStruct( rbto->fontTag );

  rbto->buttonColor = rbto->bufButtonColor;
  rbto->topShadowColor = rbto->bufTopShadowColor;
  rbto->botShadowColor = rbto->bufBotShadowColor;

  rbto->fgColorMode = rbto->bufFgColorMode;
  if ( rbto->fgColorMode == RBTC_K_COLORMODE_ALARM )
    rbto->fgColor.setAlarmSensitive();
  else
    rbto->fgColor.setAlarmInsensitive();
  rbto->fgColor.setColorIndex( rbto->bufFgColor, rbto->actWin->ci );

  rbto->bgColorMode = rbto->bufBgColorMode;
  if ( rbto->bgColorMode == RBTC_K_COLORMODE_ALARM )
    rbto->bgColor.setAlarmSensitive();
  else
    rbto->bgColor.setAlarmInsensitive();
  rbto->bgColor.setColorIndex( rbto->bufBgColor, rbto->actWin->ci );

  rbto->x = rbto->bufX;
  rbto->sboxX = rbto->bufX;

  rbto->y = rbto->bufY;
  rbto->sboxY = rbto->bufY;

  rbto->w = rbto->bufW;
  rbto->sboxW = rbto->bufW;

  rbto->h = rbto->bufH;
  rbto->sboxH = rbto->bufH;

  rbto->controlPvExpStr.setRaw( rbto->bufControlPvName );

  rbto->updateDimensions();

}

static void rbtc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeRadioButtonClass *rbto = (activeRadioButtonClass *) client;

  rbtc_edit_update ( w, client, call );
  rbto->refresh( rbto );

}

static void rbtc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeRadioButtonClass *rbto = (activeRadioButtonClass *) client;

  rbtc_edit_update ( w, client, call );
  rbto->ef.popdown();
  rbto->operationComplete();

}

static void rbtc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeRadioButtonClass *rbto = (activeRadioButtonClass *) client;

  rbto->ef.popdown();
  rbto->operationCancel();

}

static void rbtc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeRadioButtonClass *rbto = (activeRadioButtonClass *) client;

  rbto->ef.popdown();
  rbto->operationCancel();
  rbto->erase();
  rbto->deleteRequest = 1;
  rbto->drawAll();

}

activeRadioButtonClass::activeRadioButtonClass ( void ) {

int i;

  name = new char[strlen("activeRadioButtonClass")+1];
  strcpy( name, "activeRadioButtonClass" );

  numStates = 0;

  for ( i=0; i<MAX_ENUM_STATES; i++ ) {
    stateString[i] = NULL;
    pb[i] = NULL;
  }

  fgColorMode = RBTC_K_COLORMODE_STATIC;
  bgColorMode = RBTC_K_COLORMODE_STATIC;

  active = 0;
  activeMode = 0;
  widgetsCreated = 0;

  fontList = NULL;

  connection.setMaxPvs( 2 );

  unconnectedTimer = 0;

}

activeRadioButtonClass::~activeRadioButtonClass ( void ) {

  if ( name ) delete name;
  if ( fontList ) XmFontListFree( fontList );

  if ( unconnectedTimer ) {
    XtRemoveTimeOut( unconnectedTimer );
    unconnectedTimer = 0;
  }

}

// copy constructor
activeRadioButtonClass::activeRadioButtonClass
 ( const activeRadioButtonClass *source ) {

int i;
activeGraphicClass *rbto = (activeGraphicClass *) this;

  rbto->clone( (activeGraphicClass *) source );

  name = new char[strlen("activeRadioButtonClass")+1];
  strcpy( name, "activeRadioButtonClass" );

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

  buttonColor = source->buttonColor;
  topShadowColor = source->topShadowColor;
  botShadowColor = source->botShadowColor;
  buttonCb = source->buttonCb;
  topShadowCb = source->topShadowCb;
  botShadowCb = source->botShadowCb;

  fgColor.copy(source->fgColor);
  bgColor.copy(source->bgColor);
  fgCb = source->fgCb;
  bgCb = source->bgCb;

  fgColorMode = source->fgColorMode;
  bgColorMode = source->bgColorMode;

  controlPvExpStr.setRaw( source->controlPvExpStr.rawString );

  widgetsCreated = 0;
  active = 0;
  activeMode = 0;

  connection.setMaxPvs( 1 );

  unconnectedTimer = 0;

}

int activeRadioButtonClass::createInteractive (
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

  buttonColor = actWin->defaultOffsetColor;
  topShadowColor = actWin->defaultTopShadowColor;
  botShadowColor = actWin->defaultBotShadowColor;

  fgColor.setColorIndex( actWin->defaultTextFgColor, actWin->ci );
  bgColor.setColorIndex( actWin->defaultBgColor, actWin->ci );

  this->draw();

  this->editCreate();

  return 1;

}

int activeRadioButtonClass::save (
  FILE *f )
{

int index;

  fprintf( f, "%-d %-d %-d\n", RBTC_MAJOR_VERSION, RBTC_MINOR_VERSION,
   RBTC_RELEASE );

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

  index = buttonColor;
  actWin->ci->writeColorIndex( f, index );
  //fprintf( f, "%-d\n", index );

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

  return 1;

}

int activeRadioButtonClass::createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int index;
int major, minor, release;
char oneName[activeGraphicClass::MAX_PV_NAME+1];

  this->actWin = _actWin;

  fscanf( f, "%d %d %d\n", &major, &minor, &release ); actWin->incLine();

  fscanf( f, "%d\n", &x ); actWin->incLine();
  fscanf( f, "%d\n", &y ); actWin->incLine();
  fscanf( f, "%d\n", &w ); actWin->incLine();
  fscanf( f, "%d\n", &h ); actWin->incLine();

  this->initSelectBox(); // call after getting x,y,w,h

  if ( ( major > 1 ) || ( ( major == 1 ) && ( minor > 0 ) ) ) {

    actWin->ci->readColorIndex( f, &index );
    actWin->incLine(); actWin->incLine();
    fgColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d\n", &fgColorMode ); actWin->incLine();

    if ( fgColorMode == RBTC_K_COLORMODE_ALARM )
      fgColor.setAlarmSensitive();
    else
      fgColor.setAlarmInsensitive();

    actWin->ci->readColorIndex( f, &index );
    actWin->incLine(); actWin->incLine();
    bgColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d\n", &bgColorMode ); actWin->incLine();

    if ( bgColorMode == RBTC_K_COLORMODE_ALARM )
      bgColor.setAlarmSensitive();
    else
      bgColor.setAlarmInsensitive();

    actWin->ci->readColorIndex( f, &index );
    actWin->incLine(); actWin->incLine();
    buttonColor = index;

    actWin->ci->readColorIndex( f, &index );
    actWin->incLine(); actWin->incLine();
    topShadowColor = index;

    actWin->ci->readColorIndex( f, &index );
    actWin->incLine(); actWin->incLine();
    botShadowColor = index;

  }
  else {

    fscanf( f, "%d\n", &index ); actWin->incLine();
    fgColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d\n", &fgColorMode ); actWin->incLine();

    if ( fgColorMode == RBTC_K_COLORMODE_ALARM )
      fgColor.setAlarmSensitive();
    else
      fgColor.setAlarmInsensitive();

    fscanf( f, "%d\n", &index ); actWin->incLine();
    bgColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d\n", &bgColorMode ); actWin->incLine();

    if ( bgColorMode == RBTC_K_COLORMODE_ALARM )
      bgColor.setAlarmSensitive();
    else
      bgColor.setAlarmInsensitive();

    fscanf( f, "%d\n", &index ); actWin->incLine();
    buttonColor = index;

    fscanf( f, "%d\n", &index ); actWin->incLine();
    topShadowColor = index;

    fscanf( f, "%d\n", &index ); actWin->incLine();
    botShadowColor = index;

  }

  readStringFromFile( oneName, activeGraphicClass::MAX_PV_NAME+1, f );
   actWin->incLine();
  controlPvExpStr.setRaw( oneName );

  readStringFromFile( fontTag, 63+1, f ); actWin->incLine();

  actWin->fi->loadFontTag( fontTag );
  actWin->drawGc.setFontTag( fontTag, actWin->fi );

  fs = actWin->fi->getXFontStruct( fontTag );
  actWin->fi->getTextFontList( fontTag, &fontList );

  updateDimensions();

  return 1;

}

int activeRadioButtonClass::genericEdit ( void ) {

char title[32], *ptr;

  ptr = actWin->obj.getNameFromClass( "activeRadioButtonClass" );
  if ( ptr )
    strncpy( title, ptr, 31 );
  else
    strncpy( title, activeRadioButtonClass_str2, 31 );

  strncat( title, activeRadioButtonClass_str3, 31 );

  bufX = x;
  bufY = y;
  bufW = w;
  bufH = h;

  strncpy( bufFontTag, fontTag, 63 );

  bufButtonColor = buttonColor;
  bufTopShadowColor = topShadowColor;
  bufBotShadowColor = botShadowColor;

  bufFgColor = fgColor.pixelIndex();
  bufFgColorMode = fgColorMode;

  bufBgColor = bgColor.pixelIndex();
  bufBgColorMode = bgColorMode;

  if ( controlPvExpStr.getRaw() )
    strncpy( bufControlPvName, controlPvExpStr.getRaw(),
     activeGraphicClass::MAX_PV_NAME );
  else
    strcpy( bufControlPvName, "" );

  ef.create( actWin->top, actWin->appCtx->ci.getColorMap(),
   &actWin->appCtx->entryFormX,
   &actWin->appCtx->entryFormY, &actWin->appCtx->entryFormW,
   &actWin->appCtx->entryFormH, &actWin->appCtx->largestH,
   title, NULL, NULL, NULL );

  ef.addTextField( activeRadioButtonClass_str4, 35, &bufX );
  ef.addTextField( activeRadioButtonClass_str5, 35, &bufY );
  ef.addTextField( activeRadioButtonClass_str6, 35, &bufW );
  ef.addTextField( activeRadioButtonClass_str7, 35, &bufH );
  ef.addTextField( activeRadioButtonClass_str17, 35, bufControlPvName,
   activeGraphicClass::MAX_PV_NAME );

  ef.addColorButton( activeRadioButtonClass_str8, actWin->ci, &fgCb,
   &bufFgColor );
  ef.addToggle( activeRadioButtonClass_str10, &bufFgColorMode );

  ef.addColorButton( activeRadioButtonClass_str11, actWin->ci, &bgCb,
   &bufBgColor );

  ef.addColorButton( activeRadioButtonClass_str28, actWin->ci, &buttonCb,
   &bufButtonColor );

  ef.addColorButton( activeRadioButtonClass_str14, actWin->ci, &topShadowCb,
   &bufTopShadowColor );

  ef.addColorButton( activeRadioButtonClass_str15, actWin->ci, &botShadowCb,
   &bufBotShadowColor );

  ef.addFontMenu( activeRadioButtonClass_str16, actWin->fi, &fm, fontTag );

  XtUnmanageChild( fm.alignWidget() ); // no alignment info

  return 1;

}

int activeRadioButtonClass::editCreate ( void ) {

  this->genericEdit();
  ef.finished( rbtc_edit_ok, rbtc_edit_apply, rbtc_edit_cancel_delete, this );
  actWin->currentEf = NULL;
  ef.popup();

  return 1;

}

int activeRadioButtonClass::edit ( void ) {

  this->genericEdit();
  ef.finished( rbtc_edit_ok, rbtc_edit_apply, rbtc_edit_cancel, this );
  actWin->currentEf = &ef;
  ef.popup();

  return 1;

}

int activeRadioButtonClass::erase ( void ) {

  if ( deleteRequest || activeMode ) return 1;

  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  return 1;

}

int activeRadioButtonClass::eraseActive ( void ) {

  return 1;

}

int activeRadioButtonClass::draw ( void ) {

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
	      XmALIGNMENT_CENTER, activeRadioButtonClass_str19 );

    actWin->drawGc.removeNormXClipRectangle();

  }

  actWin->drawGc.restoreFg();

  return 1;

}

int activeRadioButtonClass::drawActive ( void ) {

  if ( !connection.pvsConnected() ) {
    if ( needToDrawUnconnected ) {
      actWin->executeGc.saveFg();
      actWin->executeGc.setFG( fgColor.getDisconnected() );
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
  }

  return 1;

}

int activeRadioButtonClass::expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int stat;

  stat = controlPvExpStr.expand1st( numMacros, macros, expansions );

  return stat;

}

int activeRadioButtonClass::expand2nd (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int stat;

  stat = controlPvExpStr.expand2nd( numMacros, macros, expansions );

  return stat;

}

int activeRadioButtonClass::containsMacros ( void ) {

  if ( controlPvExpStr.containsPrimaryMacros() ) return 1;

  return 0;

}

int activeRadioButtonClass::createWidgets ( void ) {

  return 1;

}

int activeRadioButtonClass::activate (
  int pass,
  void *ptr
) {

int stat, opStat;

  switch ( pass ) {

  case 1:

    aglPtr = ptr;
    needConnectInit = needInfoInit = needRefresh = needDraw = 0;
    needToEraseUnconnected = 0;
    needToDrawUnconnected = 0;
    unconnectedTimer = 0;
    opComplete = 0;

    controlExists = 0;

    pvCheckExists = 0;
    connection.init();

#ifdef __epics__
    alarmEventId = controlEventId = 0;
#endif

    active = 0;
    activeMode = 1;
    numStates = 0;
    curValue = 0;

    radioBox = (Widget) NULL;

    break;

  case 2:

    if ( !opComplete ) {

      if ( !unconnectedTimer ) {
        unconnectedTimer = appAddTimeOut( actWin->appCtx->appContext(),
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

      }

      opStat = 1;

#ifdef __epics__

      if ( controlExists ) {
        stat = ca_search_and_connect( controlPvExpStr.getExpanded(),
         &controlPvId, rbt_monitor_control_connect_state, this );
        if ( stat != ECA_NORMAL ) {
          printf( activeRadioButtonClass_str20,
           controlPvExpStr.getExpanded() );
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

int activeRadioButtonClass::deactivate (
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
      if ( stat != ECA_NORMAL )
        printf( activeRadioButtonClass_str22 );
    }

#endif

    break;

  case 2:

    if ( widgetsCreated ) {
      if ( radioBox ) {
        XtUnmapWidget( radioBox );
        XtDestroyWidget( radioBox );
        radioBox = NULL;
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

void activeRadioButtonClass::updateDimensions ( void )
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

class activeRadioButtonClass *rbto;
int stat;

  XtVaGetValues( w, XmNuserData, &rbto, NULL );

  stat = rbto->startDrag( w, e );

}

static void selectDrag (
   Widget w,
   XEvent *e,
   String *params,
   Cardinal numParams )
{

class activeRadioButtonClass *rbto;
int stat;
XButtonEvent *be = (XButtonEvent *) e;

  XtVaGetValues( w, XmNuserData, &rbto, NULL );

  stat = rbto->selectDragValue( rbto->x + be->x, rbto->y + be->y );

}

void activeRadioButtonClass::executeDeferred ( void ) {

short value;
int stat, i, nc, ni, nr, nd, notify;
XmString str;
Arg args[15];
int n;
XtTranslations parsedTrans;

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
  ni = needInfoInit; needInfoInit = 0;
  nr = needRefresh; needRefresh = 0;
  nd = needDraw; needDraw = 0;
  value = curValue;
  actWin->remDefExeNode( aglPtr );
  actWin->appCtx->proc->unlock();

  if ( !activeMode ) return;

//----------------------------------------------------------------------------

#ifdef __epics__

  if ( nc ) {

    stat = ca_get_callback( DBR_GR_ENUM, controlPvId,
     rbt_infoUpdate, (void *) this );

  }

  if ( ni ) {

    if ( widgetsCreated ) {
      if ( radioBox ) {
        XtUnmapWidget( radioBox );
        XtDestroyWidget( radioBox );
        radioBox = NULL;
      }
      widgetsCreated = 0;
    }

    parsedTrans = XtParseTranslationTable( dragTrans );
    XtAppAddActions( actWin->appCtx->appContext(), dragActions,
     XtNumber(dragActions) );

    n = 0;
    XtSetArg( args[n], XmNx, (XtArgVal) x ); n++;
    XtSetArg( args[n], XmNy, (XtArgVal) y ); n++;
    XtSetArg( args[n], XmNwidth, (XtArgVal) w ); n++;
    XtSetArg( args[n], XmNheight, (XtArgVal) h ); n++;
    XtSetArg( args[n], XmNbackground, (XtArgVal) bgColor.getColor() ); n++;
    XtSetArg( args[n], XmNforeground, (XtArgVal) fgColor.getColor() ); n++;
    XtSetArg( args[n], XmNtranslations, parsedTrans ); n++;
    XtSetArg( args[n], XmNnavigationType, XmNONE ); n++;
    XtSetArg( args[n], XmNtraversalOn, False ); n++;
    XtSetArg( args[n], XmNuserData, this ); n++;


    radioBox = XmCreateRadioBox( actWin->executeWidgetId(),
     "", args, n );
    
    for ( i=0; i<numStates; i++ ) {

      str = XmStringCreate( stateString[i], fontTag );

      pb[i] = XtVaCreateManagedWidget( "", xmToggleButtonWidgetClass,
       radioBox,
       XmNlabelString, str,
       XmNfontList, fontList,
       XmNforeground, (XtArgVal) fgColor.getColor(),
       XmNbackground, (XtArgVal) bgColor.getColor(),
       XmNselectColor, (XtArgVal) actWin->ci->pix(buttonColor),
       XmNunselectColor, (XtArgVal) actWin->ci->pix(buttonColor),
       XmNtopShadowColor, (XtArgVal) actWin->ci->pix(topShadowColor),
       XmNbottomShadowColor, (XtArgVal) actWin->ci->pix(botShadowColor),
       XmNhighlightOnEnter, False,
       XmNindicatorType, XmN_OF_MANY,
       XmNindicatorOn, XmINDICATOR_CHECK_BOX,
       XmNtranslations, parsedTrans,
       XmNuserData, this,
       NULL );

      if ( controlExists ) {
        if ( ca_write_access(controlPvId) ) {
          n = 0;
          XtSetArg( args[n], XmNsensitive, True ); n++;
	}
	else {
          n = 0;
          XtSetArg( args[n], XmNsensitive, False ); n++;
	}
      }
      else {
        n = 0;
        XtSetArg( args[n], XmNsensitive, False ); n++;
      }

      XtSetValues( pb[i], args, n );

      XtAddCallback( pb[i], XmNvalueChangedCallback, putValue,
       (XtPointer) this );

      XmStringFree( str );

    }

{

WidgetList children;
Cardinal numChildren;
int ii;

    XtAddEventHandler( radioBox,
     EnterWindowMask|LeaveWindowMask,
     False, radioBoxEventHandler, (XtPointer) this );

    XtVaGetValues( radioBox,
     XmNnumChildren, &numChildren,
     XmNchildren, &children,
     NULL );

    for ( ii=0; ii<(int)numChildren; ii++ ) {

      XtAddEventHandler( children[ii],
       EnterWindowMask,
       False, radioBoxEventHandler, (XtPointer) this );

    }

}

    XtManageChild( radioBox );

    widgetsCreated = 1;

    if ( !controlEventId ) {

      stat = ca_add_masked_array_event( DBR_ENUM, 1, controlPvId,
       rbt_controlUpdate, (void *) this, (float) 0.0, (float) 0.0,
       (float) 0.0, &controlEventId, DBE_VALUE );
      if ( stat != ECA_NORMAL ) {
        printf( activeRadioButtonClass_str24 );
      }

    }

    if ( !alarmEventId ) {

      stat = ca_add_masked_array_event( DBR_STS_ENUM, 1, controlPvId,
       rbt_alarmUpdate, (void *) this, (float) 0.0, (float) 0.0,
       (float) 0.0, &alarmEventId, DBE_ALARM );
      if ( stat != ECA_NORMAL ) {
        printf( activeRadioButtonClass_str25 );
      }

    }

    active = 1;

    for ( i=0; i<numStates; i++ ) {
      if ( i == value )
        XmToggleButtonSetState( pb[i], (XtArgVal) True, (XtArgVal) True );
      else
        XmToggleButtonSetState( pb[i], (XtArgVal) False, (XtArgVal) True );
    }

    drawActive();

    nr = 1;

  }

#endif

//----------------------------------------------------------------------------

  if ( nr ) {

    notify = False;

    for ( i=0; i<numStates; i++ ) {
      if ( i == value ) {
        XmToggleButtonSetState( pb[i], (XtArgVal) True, (XtArgVal) notify );
        n = 0;
        XtSetArg( args[n], XmNforeground, fgColor.getColor() ); n++;
        XtSetValues( pb[i], args, n );
      }
      else {
        XmToggleButtonSetState( pb[i], (XtArgVal) False, (XtArgVal) notify );
        n = 0;
        XtSetArg( args[n], XmNforeground, fgColor.pixelColor() ); n++;
        XtSetValues( pb[i], args, n );
      }
    }

  }

//----------------------------------------------------------------------------

  if ( nd ) {
    drawActive();
  }

//----------------------------------------------------------------------------

}

char *activeRadioButtonClass::firstDragName ( void ) {

  dragIndex = 0;
  return dragName[dragIndex];

}

char *activeRadioButtonClass::nextDragName ( void ) {

  return NULL;

}

char *activeRadioButtonClass::dragValue (
  int i ) {

  return controlPvExpStr.getExpanded();

}

void activeRadioButtonClass::changeDisplayParams (
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
    buttonColor = _fg1Color;

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

void activeRadioButtonClass::changePvNames (
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

void *create_activeRadioButtonClassPtr ( void ) {

activeRadioButtonClass *ptr;

  ptr = new activeRadioButtonClass;
  return (void *) ptr;

}

void *clone_activeRadioButtonClassPtr (
  void *_srcPtr )
{

activeRadioButtonClass *ptr, *srcPtr;

  srcPtr = (activeRadioButtonClass *) _srcPtr;

  ptr = new activeRadioButtonClass( srcPtr );

  return (void *) ptr;

}

#ifdef __cplusplus
}
#endif
