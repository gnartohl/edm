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

#define __message_button_cc 1

#include "message_button.h"
#include "app_pkg.h"
#include "act_win.h"

#include "thread.h"

static void doBlink (
  void *ptr
) {

activeMessageButtonClass *msgbto = (activeMessageButtonClass *) ptr;

  if ( !msgbto->activeMode ) {
    if ( msgbto->isSelected() ) msgbto->drawSelectBoxCorners(); //erase via xor
    msgbto->smartDrawAll();
    if ( msgbto->isSelected() ) msgbto->drawSelectBoxCorners();
  }
  else {
    msgbto->bufInvalidate();
    msgbto->needDraw = 1;
    msgbto->actWin->addDefExeNode( msgbto->aglPtr );
  }

}

static void pw_ok (
  Widget w,
  XtPointer client,
  XtPointer call ) {

activeMessageButtonClass *msgbto = (activeMessageButtonClass *) client;

  msgbto->ef.popdown();

  if ( strcmp( msgbto->bufPw1, msgbto->pw ) == 0 ) {
    msgbto->needPerformDownAction = 1;
    msgbto->actWin->addDefExeNode( msgbto->aglPtr );
  }
  else {
    msgbto->needWarning = 1;
    msgbto->actWin->addDefExeNode( msgbto->aglPtr );
  }

}

static void pw_apply (
  Widget w,
  XtPointer client,
  XtPointer call ) {

}

static void pw_cancel (
  Widget w,
  XtPointer client,
  XtPointer call ) {

activeMessageButtonClass *msgbto = (activeMessageButtonClass *) client;

  msgbto->ef.popdown();

}

static void unconnectedTimeout (
  XtPointer client,
  XtIntervalId *id )
{

activeMessageButtonClass *msgbto = (activeMessageButtonClass *) client;

  if ( !msgbto->init ) {
    msgbto->needToDrawUnconnected = 1;
    msgbto->needDraw = 1;
    msgbto->actWin->addDefExeNode( msgbto->aglPtr );
  }

  msgbto->unconnectedTimer = 0;

}

static void msgbtc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeMessageButtonClass *msgbto = (activeMessageButtonClass *) client;

  msgbto->actWin->setChanged();

  msgbto->eraseSelectBoxCorners();
  msgbto->erase();

  msgbto->fgColor.setColorIndex( msgbto->bufFgColor, msgbto->actWin->ci );

  msgbto->onColor.setColorIndex( msgbto->bufOnColor, msgbto->actWin->ci );

  msgbto->offColor.setColorIndex( msgbto->bufOffColor, msgbto->actWin->ci );

  msgbto->topShadowColor = msgbto->bufTopShadowColor;
  msgbto->botShadowColor = msgbto->bufBotShadowColor;

  msgbto->destPvExpString.setRaw( msgbto->bufDestPvName );
  msgbto->sourcePressPvExpString.setRaw( msgbto->bufSourcePressPvName );
  msgbto->sourceReleasePvExpString.setRaw( msgbto->bufSourceReleasePvName );

//    strncpy( msgbto->sourcePressPvName, msgbto->bufSourcePressPvName, 39 );
//    strncpy( msgbto->sourceReleasePvName, msgbto->bufSourceReleasePvName, 39 );

  msgbto->onLabel.setRaw( msgbto->bufOnLabel );
  msgbto->offLabel.setRaw( msgbto->bufOffLabel );
  // strncpy( msgbto->onLabel, msgbto->bufOnLabel, MAX_ENUM_STRING_SIZE );
  // strncpy( msgbto->offLabel, msgbto->bufOffLabel, MAX_ENUM_STRING_SIZE );

  strncpy( msgbto->fontTag, msgbto->fm.currentFontTag(), 63 );
  msgbto->actWin->fi->loadFontTag( msgbto->fontTag );
  msgbto->fs = msgbto->actWin->fi->getXFontStruct( msgbto->fontTag );

  msgbto->toggle = msgbto->bufToggle;

  msgbto->pressAction = msgbto->bufPressAction;
  msgbto->releaseAction = msgbto->bufReleaseAction;

  msgbto->_3D = msgbto->buf3D;

  msgbto->invisible = msgbto->bufInvisible;

  msgbto->x = msgbto->bufX;
  msgbto->sboxX = msgbto->bufX;

  msgbto->y = msgbto->bufY;
  msgbto->sboxY = msgbto->bufY;

  msgbto->w = msgbto->bufW;
  msgbto->sboxW = msgbto->bufW;

  msgbto->h = msgbto->bufH;
  msgbto->sboxH = msgbto->bufH;

  if ( blank(msgbto->bufPw1) || blank(msgbto->bufPw2) ) {
    if ( blank(msgbto->pw) ) {
      msgbto->usePassword = 0;
    }
    else {
      msgbto->usePassword = 1;
    }
  }
  else if ( strcmp( msgbto->bufPw1, msgbto->bufPw2 ) != 0 ) {
    msgbto->actWin->appCtx->postMessage( activeMessageButtonClass_str33 );
    if ( blank(msgbto->pw) ) {
      msgbto->usePassword = 0;
    }
    else if ( strcmp( msgbto->pw, "*" ) == 0 ) {
      strcpy( msgbto->pw, "" );
      msgbto->usePassword = 0;
    }
    else {
      msgbto->usePassword = 1;
    }
  }
  else {
    strcpy( msgbto->pw, msgbto->bufPw2 );
    if ( strcmp( msgbto->pw, "*" ) == 0 ) {
      strcpy( msgbto->pw, "" );
      msgbto->usePassword = 0;
    }
    else {
      msgbto->usePassword = 1;
    }
  }

  msgbto->lock = msgbto->bufLock;

  msgbto->visPvExpString.setRaw( msgbto->bufVisPvName );
  strncpy( msgbto->minVisString, msgbto->bufMinVisString, 39 );
  strncpy( msgbto->maxVisString, msgbto->bufMaxVisString, 39 );

  if ( msgbto->bufVisInverted )
    msgbto->visInverted = 0;
  else
    msgbto->visInverted = 1;

  msgbto->useEnumNumeric = msgbto->bufUseEnumNumeric;

  msgbto->colorPvExpString.setRaw( msgbto->bufColorPvName );

  msgbto->updateDimensions();

}

static void msgbtc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeMessageButtonClass *msgbto = (activeMessageButtonClass *) client;

  msgbtc_edit_update ( w, client, call );
  msgbto->refresh( msgbto );

}

static void msgbtc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeMessageButtonClass *msgbto = (activeMessageButtonClass *) client;

  msgbtc_edit_update ( w, client, call );
  msgbto->ef.popdown();
  msgbto->operationComplete();

}

static void msgbtc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeMessageButtonClass *msgbto = (activeMessageButtonClass *) client;

  msgbto->ef.popdown();
  msgbto->operationCancel();

}

static void msgbtc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeMessageButtonClass *msgbto = (activeMessageButtonClass *) client;

  msgbto->ef.popdown();
  msgbto->operationCancel();
  msgbto->erase();
  msgbto->deleteRequest = 1;
  msgbto->drawAll();

}

#ifdef __epics__

static void msgbt_monitor_dest_connect_state (
  struct connection_handler_args arg )
{

activeMessageButtonClass *msgbto = (activeMessageButtonClass *) ca_puser(arg.chid);

  if ( arg.op == CA_OP_CONN_UP ) {

    msgbto->needConnectInit = 1;

  }
  else {

    
    msgbto->connection.setPvDisconnected( (void *) msgbto->destPvConnection );
    msgbto->active = 0;
    msgbto->onColor.setDisconnected();
    msgbto->offColor.setDisconnected();
    msgbto->needDraw = 1;

  }

  msgbto->actWin->appCtx->proc->lock();
  msgbto->actWin->addDefExeNode( msgbto->aglPtr );
  msgbto->actWin->appCtx->proc->unlock();

}

static void msgbt_monitor_vis_connect_state (
  struct connection_handler_args arg )
{

activeMessageButtonClass *msgbto =
 (activeMessageButtonClass *) ca_puser(arg.chid);

  if ( arg.op == CA_OP_CONN_UP ) {

    msgbto->needVisConnectInit = 1;

  }
  else {

    msgbto->connection.setPvDisconnected( (void *) msgbto->visPvConnection );
    msgbto->active = 0;
    msgbto->onColor.setDisconnected();
    msgbto->offColor.setDisconnected();
    msgbto->needDraw = 1;

  }

  msgbto->actWin->appCtx->proc->lock();
  msgbto->actWin->addDefExeNode( msgbto->aglPtr );
  msgbto->actWin->appCtx->proc->unlock();

}

static void msgbt_destInfoUpdate (
  struct event_handler_args ast_args )
{

  if ( ast_args.status == ECA_DISCONN ) {
    return;
  }

int i;
activeMessageButtonClass *msgbto =
 (activeMessageButtonClass *) ast_args.usr;
struct dbr_gr_enum enumRec;

  enumRec = *( (struct dbr_gr_enum *) ast_args.dbr );

  msgbto->numStates = enumRec.no_str;

  for ( i=0; i<msgbto->numStates; i++ ) {

    if ( msgbto->stateString[i] == NULL ) {
      msgbto->stateString[i] = new char[MAX_ENUM_STRING_SIZE+1];
    }

    strncpy( msgbto->stateString[i], enumRec.strs[i], MAX_ENUM_STRING_SIZE );

  }

}

static void msgbt_visInfoUpdate (
  struct event_handler_args ast_args )
{

  if ( ast_args.status == ECA_DISCONN ) {
    return;
  }

activeMessageButtonClass *msgbto =
 (activeMessageButtonClass *) ast_args.usr;

struct dbr_gr_double controlRec = *( (dbr_gr_double *) ast_args.dbr );

  msgbto->curVisValue = controlRec.value;

  msgbto->actWin->appCtx->proc->lock();
  msgbto->needVisInit = 1;
  msgbto->actWin->addDefExeNode( msgbto->aglPtr );
  msgbto->actWin->appCtx->proc->unlock();

}

static void msgbt_visUpdate (
  struct event_handler_args ast_args )
{

activeMessageButtonClass *msgbto =
 (activeMessageButtonClass *) ast_args.usr;

  msgbto->curVisValue = * ( (double *) ast_args.dbr );

  msgbto->actWin->appCtx->proc->lock();
  msgbto->needVisUpdate = 1;
  msgbto->actWin->addDefExeNode( msgbto->aglPtr );
  msgbto->actWin->appCtx->proc->unlock();

}

static void msgbt_monitor_color_connect_state (
  struct connection_handler_args arg )
{

activeMessageButtonClass *msgbto =
 (activeMessageButtonClass *) ca_puser(arg.chid);

  if ( arg.op == CA_OP_CONN_UP ) {

    msgbto->needColorConnectInit = 1;

  }
  else {

    msgbto->connection.setPvDisconnected( (void *) msgbto->colorPvConnection );
    msgbto->active = 0;
    msgbto->onColor.setDisconnected();
    msgbto->offColor.setDisconnected();
    msgbto->needDraw = 1;

  }

  msgbto->actWin->appCtx->proc->lock();
  msgbto->actWin->addDefExeNode( msgbto->aglPtr );
  msgbto->actWin->appCtx->proc->unlock();

}

static void msgbt_colorInfoUpdate (
  struct event_handler_args ast_args )
{

  if ( ast_args.status == ECA_DISCONN ) {
    return;
  }

activeMessageButtonClass *msgbto =
 (activeMessageButtonClass *) ast_args.usr;

struct dbr_gr_double controlRec = *( (dbr_gr_double *) ast_args.dbr );

  msgbto->curColorValue = controlRec.value;

  msgbto->actWin->appCtx->proc->lock();
  msgbto->needColorInit = 1;
  msgbto->actWin->addDefExeNode( msgbto->aglPtr );
  msgbto->actWin->appCtx->proc->unlock();

}

static void msgbt_colorUpdate (
  struct event_handler_args ast_args )
{

activeMessageButtonClass *msgbto =
 (activeMessageButtonClass *) ast_args.usr;

  msgbto->curColorValue = * ( (double *) ast_args.dbr );

  msgbto->actWin->appCtx->proc->lock();
  msgbto->needColorUpdate = 1;
  msgbto->actWin->addDefExeNode( msgbto->aglPtr );
  msgbto->actWin->appCtx->proc->unlock();

}

#endif

activeMessageButtonClass::activeMessageButtonClass ( void ) {

  name = new char[strlen("activeMessageButtonClass")+1];
  strcpy( name, "activeMessageButtonClass" );
  buttonPressed = 0;

  // strcpy( sourcePressPvName, "" );
  // strcpy( sourceReleasePvName, "" );
  // strcpy( onLabel, "" );
  // strcpy( offLabel, "" );
  toggle = 0;
  pressAction = 0;
  releaseAction = 0;
  _3D = 1;
  invisible = 0;
  unconnectedTimer = 0;
  strcpy( pw, "" );
  usePassword = 0;
  lock = 0;
  visibility = 0;
  prevVisibility = -1;
  visInverted = 0;
  strcpy( minVisString, "" );
  strcpy( maxVisString, "" );
  connection.setMaxPvs( 3 );
  useEnumNumeric = 0;
  activeMode = 0;

  setBlinkFunction( (void *) doBlink );

}

// copy constructor
activeMessageButtonClass::activeMessageButtonClass
 ( const activeMessageButtonClass *source ) {

activeGraphicClass *msgbto = (activeGraphicClass *) this;

  msgbto->clone( (activeGraphicClass *) source );

  name = new char[strlen("activeMessageButtonClass")+1];
  strcpy( name, "activeMessageButtonClass" );

  buttonPressed = 0;

  fgCb = source->fgCb;
  onCb = source->onCb;
  offCb = source->offCb;
  topShadowCb = source->topShadowCb;
  botShadowCb = source->botShadowCb;

  strncpy( fontTag, source->fontTag, 63 );

  fs = actWin->fi->getXFontStruct( fontTag );

  fgColor.copy( source->fgColor );
  offColor.copy( source->offColor );
  onColor.copy( source->onColor );

  topShadowColor = source->topShadowColor;
  botShadowColor = source->botShadowColor;

  destPvExpString.copy( source->destPvExpString );
  sourcePressPvExpString.copy( source->sourcePressPvExpString );
  sourceReleasePvExpString.copy( source->sourceReleasePvExpString );
  visPvExpString.copy( source->visPvExpString );
  colorPvExpString.copy( source->colorPvExpString );

  // strncpy( sourcePressPvName, source->sourcePressPvName, 39 );
  // strncpy( sourceReleasePvName, source->sourceReleasePvName, 39 );

  onLabel.copy( source->onLabel );
  offLabel.copy( source->offLabel );
  // strncpy( onLabel, source->onLabel, MAX_ENUM_STRING_SIZE );
  // strncpy( offLabel, source->offLabel, MAX_ENUM_STRING_SIZE );

  toggle = source->toggle;
  pressAction = source->pressAction;
  releaseAction = source->releaseAction;
  _3D = source->_3D;
  invisible = source->invisible;
  unconnectedTimer = 0;

  strcpy( pw, source->pw );
  usePassword = source->usePassword;
  lock = source->lock;

  visibility = 0;
  prevVisibility = -1;
  visInverted = source->visInverted;
  strncpy( minVisString, source->minVisString, 39 );
  strncpy( maxVisString, source->maxVisString, 39 );
  useEnumNumeric = source->useEnumNumeric;

  activeMode = 0;

  connection.setMaxPvs( 3 );

  setBlinkFunction( (void *) doBlink );

  updateDimensions();

}

activeMessageButtonClass::~activeMessageButtonClass ( void ) {

  if ( name ) delete name;

  if ( unconnectedTimer ) {
    XtRemoveTimeOut( unconnectedTimer );
    unconnectedTimer = 0;
  }

  updateBlink( 0 );

}

int activeMessageButtonClass::createInteractive (
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

  fgColor.setColorIndex( actWin->defaultTextFgColor, actWin->ci );
  onColor.setColorIndex( actWin->defaultBgColor, actWin->ci );
  offColor.setColorIndex( actWin->defaultBgColor, actWin->ci );
  topShadowColor = actWin->defaultTopShadowColor;
  botShadowColor = actWin->defaultBotShadowColor;

  strcpy( fontTag, actWin->defaultBtnFontTag );

  actWin->fi->loadFontTag( fontTag );
  fs = actWin->fi->getXFontStruct( fontTag );

  updateDimensions();

  // strcpy( onLabel, "" );
  // strcpy( offLabel, "" );

  toggle = 0;
  pressAction = 0;
  releaseAction = 0;
  _3D = 1;
  invisible = 0;

  this->draw();

  this->editCreate();

  return 1;

}

int activeMessageButtonClass::save (
  FILE *f )
{

int index;

  fprintf( f, "%-d %-d %-d\n", MSGBTC_MAJOR_VERSION, MSGBTC_MINOR_VERSION,
   MSGBTC_RELEASE );

  fprintf( f, "%-d\n", x );
  fprintf( f, "%-d\n", y );
  fprintf( f, "%-d\n", w );
  fprintf( f, "%-d\n", h );

  index = fgColor.pixelIndex();
  actWin->ci->writeColorIndex( f, index );
  //fprintf( f, "%-d\n", index );

  index = onColor.pixelIndex();
  actWin->ci->writeColorIndex( f, index );
  //fprintf( f, "%-d\n", index );

  index = offColor.pixelIndex();
  actWin->ci->writeColorIndex( f, index );
  //fprintf( f, "%-d\n", index );

  index = topShadowColor;
  actWin->ci->writeColorIndex( f, index );
  //fprintf( f, "%-d\n", index );

  index = botShadowColor;
  actWin->ci->writeColorIndex( f, index );
  //fprintf( f, "%-d\n", index );

  if ( destPvExpString.getRaw() )
    writeStringToFile( f, destPvExpString.getRaw() );
  else
    writeStringToFile( f, "" );

  if ( sourcePressPvExpString.getRaw() )
    writeStringToFile( f, sourcePressPvExpString.getRaw() );
  else
    writeStringToFile( f, "" );

  if ( sourceReleasePvExpString.getRaw() )
    writeStringToFile( f, sourceReleasePvExpString.getRaw() );
  else
    writeStringToFile( f, "" );

  // writeStringToFile( f, sourcePressPvName );
  // writeStringToFile( f, sourceReleasePvName );

  if ( onLabel.getRaw() )
    writeStringToFile( f, onLabel.getRaw() );
  else
    writeStringToFile( f, "" );

  // writeStringToFile( f, onLabel );

  if ( offLabel.getRaw() )
    writeStringToFile( f, offLabel.getRaw() );
  else
    writeStringToFile( f, "" );

  // writeStringToFile( f, offLabel );

  fprintf( f, "%-d\n", toggle );

  fprintf( f, "%-d\n", pressAction );

  fprintf( f, "%-d\n", releaseAction );

  fprintf( f, "%-d\n", _3D );

  fprintf( f, "%-d\n", invisible );

  writeStringToFile( f, fontTag );

  // ver 2.1.0
  writeStringToFile( f, pw );
  fprintf( f, "%-d\n", lock );

  //ver 2.3.0
  if ( visPvExpString.getRaw() )
    writeStringToFile( f, visPvExpString.getRaw() );
  else
    writeStringToFile( f, "" );
  fprintf( f, "%-d\n", visInverted );
  writeStringToFile( f, minVisString );
  writeStringToFile( f, maxVisString );

  //ver 2.4.0
  if ( colorPvExpString.getRaw() )
    writeStringToFile( f, colorPvExpString.getRaw() );
  else
    writeStringToFile( f, "" );

  //ver 2.5.0
  fprintf( f, "%-d\n", useEnumNumeric );

  return 1;

}

int activeMessageButtonClass::createFromFile (
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

  if ( major > MSGBTC_MAJOR_VERSION ) {
    postIncompatable();
    return 0;
  }

  fscanf( f, "%d\n", &x ); actWin->incLine();
  fscanf( f, "%d\n", &y ); actWin->incLine();
  fscanf( f, "%d\n", &w ); actWin->incLine();
  fscanf( f, "%d\n", &h ); actWin->incLine();

  this->initSelectBox(); // call after getting x,y,w,h

  if ( ( major > 2 ) || ( ( major == 2 ) && ( minor > 1 ) ) ) {

    actWin->ci->readColorIndex( f, &index );
    actWin->incLine(); actWin->incLine();
    fgColor.setColorIndex( index, actWin->ci );

    actWin->ci->readColorIndex( f, &index );
    actWin->incLine(); actWin->incLine();
    onColor.setColorIndex( index, actWin->ci );

    actWin->ci->readColorIndex( f, &index );
    actWin->incLine(); actWin->incLine();
    offColor.setColorIndex( index, actWin->ci );

    actWin->ci->readColorIndex( f, &index );
    actWin->incLine(); actWin->incLine();
    topShadowColor = index;

    actWin->ci->readColorIndex( f, &index );
    actWin->incLine(); actWin->incLine();
    botShadowColor = index;

  }
  else if ( major > 1 ) {

    fscanf( f, "%d\n", &index ); actWin->incLine();
    fgColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d\n", &index ); actWin->incLine();
    onColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d\n", &index ); actWin->incLine();
    offColor.setColorIndex( index, actWin->ci );

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
    index = actWin->ci->pixIndex( pixel );
    fgColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
    if ( ( major < 2 ) && ( minor < 2 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    actWin->ci->setRGB( r, g, b, &pixel );
    index = actWin->ci->pixIndex( pixel );
    onColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
    if ( ( major < 2 ) && ( minor < 2 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    actWin->ci->setRGB( r, g, b, &pixel );
    index = actWin->ci->pixIndex( pixel );
    offColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
    if ( ( major < 2 ) && ( minor < 2 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    actWin->ci->setRGB( r, g, b, &pixel );
    topShadowColor = actWin->ci->pix(pixel);

    fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
    if ( ( major < 2 ) && ( minor < 2 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    actWin->ci->setRGB( r, g, b, &pixel );
    botShadowColor = actWin->ci->pix(pixel);

  }

  readStringFromFile( oneName, activeGraphicClass::MAX_PV_NAME+1, f );
   actWin->incLine();
  destPvExpString.setRaw( oneName );

  readStringFromFile( oneName, 39+1, f ); actWin->incLine();
  sourcePressPvExpString.setRaw( oneName );

  readStringFromFile( oneName, 39+1, f ); actWin->incLine();
  sourceReleasePvExpString.setRaw( oneName );

  readStringFromFile( oneName, MAX_ENUM_STRING_SIZE+1, f ); actWin->incLine();
  onLabel.setRaw( oneName );
  //readStringFromFile( onLabel, MAX_ENUM_STRING_SIZE+1, f ); actWin->incLine();

  readStringFromFile( oneName, MAX_ENUM_STRING_SIZE+1, f ); actWin->incLine();
  offLabel.setRaw( oneName );
  //readStringFromFile( offLabel, MAX_ENUM_STRING_SIZE+1, f ); actWin->incLine();

  fscanf( f, "%d\n", &toggle ); actWin->incLine();

  if ( ( major > 1 ) || ( minor > 2 ) ) {
    fscanf( f, "%d\n", &pressAction ); actWin->incLine();
    fscanf( f, "%d\n", &releaseAction ); actWin->incLine();
  }
  else {
    pressAction = 0;
    releaseAction = 0;
  }

  fscanf( f, "%d\n", &_3D ); actWin->incLine();

  fscanf( f, "%d\n", &invisible ); actWin->incLine();

  readStringFromFile( fontTag, 63+1, f ); actWin->incLine();

  actWin->fi->loadFontTag( fontTag );
  fs = actWin->fi->getXFontStruct( fontTag );

  if ( ( major > 2 ) || ( ( major == 2 ) && ( minor > 0 ) ) ) {
    readStringFromFile( pw, 31+1, f ); actWin->incLine();
    if ( blank(pw) ) {
      usePassword = 0;
    }
    else if ( strcmp( pw, "*" ) == 0 ) {
      usePassword = 0;
    }
    else {
      usePassword = 1;
    }
    fscanf( f, "%d\n", &lock );
  }
  else {
    strcpy( pw, "" );
    usePassword = 0;
    lock = 0;
  }

  if ( ( major > 2 ) || ( ( major == 2 ) && ( minor > 2 ) ) ) {

    readStringFromFile( oneName, activeGraphicClass::MAX_PV_NAME+1, f );
     actWin->incLine();
    visPvExpString.setRaw( oneName );

    fscanf( f, "%d\n", &visInverted ); actWin->incLine();

    readStringFromFile( minVisString, 39+1, f ); actWin->incLine();

    readStringFromFile( maxVisString, 39+1, f ); actWin->incLine();

  }

  if ( ( major > 2 ) || ( ( major == 2 ) && ( minor > 3 ) ) ) {

    readStringFromFile( oneName, activeGraphicClass::MAX_PV_NAME+1, f );
     actWin->incLine();
    colorPvExpString.setRaw( oneName );

  }

  if ( ( major > 2 ) || ( ( major == 2 ) && ( minor > 4 ) ) ) {

    fscanf( f, "%d\n", &useEnumNumeric ); actWin->incLine();

  }
  else {

    useEnumNumeric = 1;

  }

  updateDimensions();

  return 1;

}

int activeMessageButtonClass::importFromXchFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin ){

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

  fgColor.setColorIndex( actWin->defaultTextFgColor, actWin->ci );
  onColor.setColorIndex( actWin->defaultBgColor, actWin->ci );
  offColor.setColorIndex( actWin->defaultBgColor, actWin->ci );
  topShadowColor = actWin->defaultTopShadowColor;
  botShadowColor = actWin->defaultBotShadowColor;
  strcpy( fontTag, actWin->defaultBtnFontTag );

  onLabel.setRaw( "" );
  // strcpy( onLabel, "" );
  offLabel.setRaw( "" );
  // strcpy( offLabel, "" );

  // continue until tag is <eod>

  do {

    gotData = getNextDataString( buf, 255, f );
    if ( !gotData ) {
      actWin->appCtx->postMessage( activeMessageButtonClass_str1 );
      return 0;
    }

    context = NULL;

    tk = strtok_r( buf, " \t\n", &context );
    if ( !tk ) {
      actWin->appCtx->postMessage( activeMessageButtonClass_str1 );
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
          actWin->appCtx->postMessage( activeMessageButtonClass_str1 );
          return 0;
        }

        x = atol( tk );

      }
      else if ( strcmp( tk, "y" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( activeMessageButtonClass_str1 );
          return 0;
        }

        y = atol( tk );

      }
      else if ( strcmp( tk, "w" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( activeMessageButtonClass_str1 );
          return 0;
        }

        w = atol( tk );

      }
      else if ( strcmp( tk, "h" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( activeMessageButtonClass_str1 );
          return 0;
        }

        h = atol( tk );

      }
            
      else if ( strcmp( tk, "fgred" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( activeMessageButtonClass_str1 );
          return 0;
        }

        fgR = atol( tk );

      }
            
      else if ( strcmp( tk, "fggreen" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( activeMessageButtonClass_str1 );
          return 0;
        }

        fgG = atol( tk );

      }
            
      else if ( strcmp( tk, "fgblue" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( activeMessageButtonClass_str1 );
          return 0;
        }

        fgB = atol( tk );

      }
            
      else if ( strcmp( tk, "bgred" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( activeMessageButtonClass_str1 );
          return 0;
        }

        bgR = atol( tk );

      }
            
      else if ( strcmp( tk, "bggreen" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( activeMessageButtonClass_str1 );
          return 0;
        }

        bgG = atol( tk );

      }
            
      else if ( strcmp( tk, "bgblue" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( activeMessageButtonClass_str1 );
          return 0;
        }

        bgB = atol( tk );

      }
            
      else if ( strcmp( tk, "closecurrentonpress" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( activeMessageButtonClass_str1 );
          return 0;
        }

        pressAction = atol( tk );

      }
            
      else if ( strcmp( tk, "invisible" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( activeMessageButtonClass_str1 );
          return 0;
        }

        invisible = atol( tk );

      }
            
      else if ( strcmp( tk, "font" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( activeMessageButtonClass_str1 );
          return 0;
        }

        strncpy( fontTag, tk, 63 );

      }

      else if ( strcmp( tk, "pressvalue" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n \t", &context );
        if ( tk ) {
          strncpy( bufSourcePressPvName, tk, 39 );
          bufSourcePressPvName[39] = 0;
          sourcePressPvExpString.setRaw( bufSourcePressPvName );
	}

      }

      else if ( strcmp( tk, "presspv" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n \t", &context );
        if ( tk ) {
          strncpy( bufDestPvName, tk, activeGraphicClass::MAX_PV_NAME );
          bufDestPvName[activeGraphicClass::MAX_PV_NAME] = 0;
          destPvExpString.setRaw( bufDestPvName );
	}

      }

      else if ( strcmp( tk, "onlabel" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n \t", &context );
        if ( tk ) {
          onLabel.setRaw( tk );
          // strncpy( onLabel, tk, MAX_ENUM_STRING_SIZE );
          // onLabel[MAX_ENUM_STRING_SIZE] = 0;
	}

      }

      else if ( strcmp( tk, "offlabel" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n \t", &context );
        if ( tk ) {
          offLabel.setRaw( tk );
          // strncpy( offLabel, tk, MAX_ENUM_STRING_SIZE );
          // offLabel[MAX_ENUM_STRING_SIZE] = 0;
	}

      }

    }

  } while ( more );

  this->initSelectBox(); // call after getting x,y,w,h

  actWin->ci->setRGB( fgR, fgG, fgB, &pixel );
  index = actWin->ci->pixIndex( pixel );
  fgColor.setColorIndex( index, actWin->ci );

  actWin->ci->setRGB( bgR, bgG, bgB, &pixel );
  index = actWin->ci->pixIndex( pixel );
  onColor.setColorIndex( index, actWin->ci );
  offColor.setColorIndex( index, actWin->ci );

  actWin->fi->loadFontTag( fontTag );
  actWin->drawGc.setFontTag( fontTag, actWin->fi );
  fs = actWin->fi->getXFontStruct( fontTag );

  updateDimensions();

  return 1;

}

int activeMessageButtonClass::genericEdit ( void ) {

char title[32], *ptr, *envPtr, saveLock;

  envPtr = getenv( "EDMSUPERVISORMODE" );
  if ( envPtr ) {
    if ( strcmp( envPtr, "TRUE" ) == 0 ) {
      if ( lock ) {
        actWin->appCtx->postMessage( activeMessageButtonClass_str34 );
      }
      saveLock = lock;
      lock = 0;
    }
  }

  ptr = actWin->obj.getNameFromClass( "activeMessageButtonClass" );
  if ( ptr )
    strncpy( title, ptr, 31 );
  else
    strncpy( title, activeMessageButtonClass_str2, 31 );

  Strncat( title, activeMessageButtonClass_str3, 31 );

  bufX = x;
  bufY = y;
  bufW = w;
  bufH = h;

  bufFgColor = fgColor.pixelIndex();

  bufOnColor = onColor.pixelIndex();

  bufOffColor = offColor.pixelIndex();

  bufTopShadowColor = topShadowColor;
  bufBotShadowColor = botShadowColor;
  strncpy( bufFontTag, fontTag, 63 );

  if ( destPvExpString.getRaw() )
    strncpy( bufDestPvName, destPvExpString.getRaw(),
     activeGraphicClass::MAX_PV_NAME );
  else
    strcpy( bufDestPvName, "" );

  if ( sourcePressPvExpString.getRaw() )
    strncpy( bufSourcePressPvName, sourcePressPvExpString.getRaw(), 39 );
  else
    strncpy( bufSourcePressPvName, "", 39 );

  if ( sourceReleasePvExpString.getRaw() )
    strncpy( bufSourceReleasePvName, sourceReleasePvExpString.getRaw(), 39 );
  else
    strncpy( bufSourceReleasePvName, "", 39 );

  // strncpy( bufSourcePressPvName, sourcePressPvName, 39 );
  // strncpy( bufSourceReleasePvName, sourceReleasePvName, 39 );

  if ( onLabel.getRaw() )
    strncpy( bufOnLabel, onLabel.getRaw(), MAX_ENUM_STRING_SIZE );
  else
    strncpy( bufOnLabel, "", MAX_ENUM_STRING_SIZE );

  // strncpy( bufOnLabel, onLabel, MAX_ENUM_STRING_SIZE );

  if ( offLabel.getRaw() )
    strncpy( bufOffLabel, offLabel.getRaw(), MAX_ENUM_STRING_SIZE );
  else
    strncpy( bufOffLabel, "", MAX_ENUM_STRING_SIZE );

  // strncpy( bufOffLabel, offLabel, MAX_ENUM_STRING_SIZE );

  if ( visPvExpString.getRaw() )
    strncpy( bufVisPvName, visPvExpString.getRaw(),
     activeGraphicClass::MAX_PV_NAME );
  else
    strcpy( bufVisPvName, "" );

  if ( colorPvExpString.getRaw() )
    strncpy( bufColorPvName, colorPvExpString.getRaw(),
     activeGraphicClass::MAX_PV_NAME );
  else
    strcpy( bufColorPvName, "" );

  bufToggle = toggle;
  bufPressAction = pressAction;
  bufReleaseAction = releaseAction;
  buf3D = _3D;
  bufInvisible = invisible;

  strcpy( bufPw1, "" );
  strcpy( bufPw2, "" );

  if ( envPtr ) {
    if ( strcmp( envPtr, "TRUE" ) == 0 ) {
      bufLock = saveLock;
    }
  }
  else {
    bufLock = lock;
  }

  if ( visInverted )
    bufVisInverted = 0;
  else
    bufVisInverted = 1;

  strncpy( bufMinVisString, minVisString, 39 );
  strncpy( bufMaxVisString, maxVisString, 39 );

  bufUseEnumNumeric = useEnumNumeric;

  ef.create( actWin->top, actWin->appCtx->ci.getColorMap(),
   &actWin->appCtx->entryFormX,
   &actWin->appCtx->entryFormY, &actWin->appCtx->entryFormW,
   &actWin->appCtx->entryFormH, &actWin->appCtx->largestH,
   title, NULL, NULL, NULL );

  ef.addTextField( activeMessageButtonClass_str4, 35, &bufX );
  ef.addTextField( activeMessageButtonClass_str5, 35, &bufY );
  ef.addTextField( activeMessageButtonClass_str6, 35, &bufW );
  ef.addTextField( activeMessageButtonClass_str7, 35, &bufH );

  if ( !lock ) {
    ef.addTextField( activeMessageButtonClass_str18, 35, bufDestPvName,
     activeGraphicClass::MAX_PV_NAME );
  }
  else {
    ef.addLockedField( activeMessageButtonClass_str18, 35, bufDestPvName,
     activeGraphicClass::MAX_PV_NAME );
  }

  ef.addOption( activeMessageButtonClass_str8, activeMessageButtonClass_str9,
   &bufToggle );
  ef.addToggle( activeMessageButtonClass_str10, &buf3D );
  ef.addToggle( activeMessageButtonClass_str11, &bufInvisible );
  ef.addToggle( activeMessageButtonClass_str12, &bufPressAction );
  ef.addToggle( activeMessageButtonClass_str13, &bufReleaseAction );
  ef.addToggle( activeMessageButtonClass_str35, &bufUseEnumNumeric );

  ef.addTextField( activeMessageButtonClass_str14, 35, bufOnLabel,
   MAX_ENUM_STRING_SIZE );

  if ( !lock ) {
    ef.addTextField( activeMessageButtonClass_str16, 35, bufSourcePressPvName,
     39 );
  }
  else {
    ef.addLockedField( activeMessageButtonClass_str16, 35,
     bufSourcePressPvName, 39 );
  }

  ef.addTextField( activeMessageButtonClass_str15, 35, bufOffLabel,
   MAX_ENUM_STRING_SIZE );

  if ( !lock ) {

    ef.addTextField( activeMessageButtonClass_str17, 35,
     bufSourceReleasePvName, 39 );

    ef.addPasswordField( activeMessageButtonClass_str36, 35, bufPw1, 31 );
    ef.addPasswordField( activeMessageButtonClass_str37, 35, bufPw2, 31 );
    ef.addToggle( activeMessageButtonClass_str38, &bufLock );

  }
  else {

    ef.addLockedField( activeMessageButtonClass_str17, 35,
     bufSourceReleasePvName, 39 );

    ef.addLockedField( activeMessageButtonClass_str36, 35, bufPw1, 31 );
    ef.addLockedField( activeMessageButtonClass_str37, 35, bufPw2, 31 );

  }

  ef.addColorButton( activeMessageButtonClass_str20, actWin->ci, &fgCb,
   &bufFgColor );

  ef.addColorButton( activeMessageButtonClass_str21, actWin->ci, &onCb,
   &bufOnColor );

  ef.addColorButton( activeMessageButtonClass_str22, actWin->ci, &offCb,
   &bufOffColor );

  ef.addColorButton( activeMessageButtonClass_str23, actWin->ci, &topShadowCb,
   &bufTopShadowColor );

  ef.addColorButton( activeMessageButtonClass_str24, actWin->ci, &botShadowCb,
    &bufBotShadowColor );

  ef.addFontMenu( activeMessageButtonClass_str19, actWin->fi, &fm, fontTag );

  XtUnmanageChild( fm.alignWidget() ); // no alignment info

  ef.addTextField( activeMessageButtonClass_str32, 30, bufColorPvName,
   activeGraphicClass::MAX_PV_NAME );

  ef.addTextField( activeMessageButtonClass_str28, 30, bufVisPvName,
   activeGraphicClass::MAX_PV_NAME );
  ef.addOption( " ", activeMessageButtonClass_str29, &bufVisInverted );
  ef.addTextField( activeMessageButtonClass_str30, 30, bufMinVisString, 39 );
  ef.addTextField( activeMessageButtonClass_str31, 30, bufMaxVisString, 39 );

  if ( envPtr ) {
    if ( strcmp( envPtr, "TRUE" ) == 0 ) {
      lock = saveLock;
    }
  }

  return 1;

}

int activeMessageButtonClass::editCreate ( void ) {

  this->genericEdit();
  ef.finished( msgbtc_edit_ok, msgbtc_edit_apply, msgbtc_edit_cancel_delete, this );
  actWin->currentEf = NULL;
  ef.popup();

  return 1;

}

int activeMessageButtonClass::edit ( void ) {

  this->genericEdit();
  ef.finished( msgbtc_edit_ok, msgbtc_edit_apply, msgbtc_edit_cancel, this );
  actWin->currentEf = &ef;
  ef.popup();

  return 1;

}

int activeMessageButtonClass::erase ( void ) {

  if ( deleteRequest ) return 1;

  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  return 1;

}

int activeMessageButtonClass::eraseActive ( void ) {

  if ( !init || !activeMode || invisible ) return 1;

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

int activeMessageButtonClass::draw ( void ) {

int tX, tY;
XRectangle xR = { x, y, w, h };
int blink = 0;

  if ( deleteRequest ) return 1;

  actWin->drawGc.saveFg();

  //actWin->drawGc.setFG( onColor.pixelColor() );
  actWin->drawGc.setFG( onColor.pixelIndex(), &blink );

  XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x, y, w, h );

  if ( !_3D ) {

    actWin->drawGc.setFG( actWin->ci->pix(botShadowColor) );

  }

  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x, y, w, h );

  if ( _3D ) {

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

  }

  if ( fs ) {

    actWin->drawGc.addNormXClipRectangle( xR );

    //actWin->drawGc.setFG( fgColor.pixelColor() );
    actWin->drawGc.setFG( fgColor.pixelIndex(), &blink );
    actWin->drawGc.setFontTag( fontTag, actWin->fi );

    tX = x + w/2;
    tY = y + h/2 - fontAscent/2;

    if ( onLabel.getRaw() )
      drawText( actWin->drawWidget, &actWin->drawGc, fs, tX, tY,
       XmALIGNMENT_CENTER, onLabel.getRaw() );
    else
      drawText( actWin->drawWidget, &actWin->drawGc, fs, tX, tY,
       XmALIGNMENT_CENTER, "" );

    // drawText( actWin->drawWidget, &actWin->drawGc, fs, tX, tY,
    //  XmALIGNMENT_CENTER, onLabel );

    actWin->drawGc.removeNormXClipRectangle();

  }

  actWin->drawGc.restoreFg();

  updateBlink( blink );

  return 1;

}

int activeMessageButtonClass::drawActive ( void ) {

int tX, tY;
char string[39+1];
XRectangle xR = { x, y, w, h };
int blink = 0;

  if ( !init ) {
    if ( needToDrawUnconnected ) {
      actWin->executeGc.saveFg();
      //actWin->executeGc.setFG( onColor.getDisconnected() );
      actWin->executeGc.setFG( onColor.getDisconnectedIndex(), &blink );
      actWin->executeGc.setLineWidth( 1 );
      actWin->executeGc.setLineStyle( LineSolid );
      XDrawRectangle( actWin->d, XtWindow(actWin->executeWidget),
       actWin->executeGc.normGC(), x, y, w, h );
      actWin->executeGc.restoreFg();
      needToEraseUnconnected = 1;
      updateBlink( blink );
    }
  }
  else if ( needToEraseUnconnected ) {
    actWin->executeGc.setLineWidth( 1 );
    actWin->executeGc.setLineStyle( LineSolid );
    XDrawRectangle( actWin->d, XtWindow(actWin->executeWidget),
     actWin->executeGc.eraseGC(), x, y, w, h );
    needToEraseUnconnected = 0;
    if ( invisible ) {
      eraseActive();
      smartDrawAllActive();
    }
  }

  if ( !init || !activeMode || invisible || !visibility ) return 1;

  prevVisibility = visibility;

  actWin->executeGc.saveFg();
  actWin->executeGc.setLineWidth( 1 );
  actWin->executeGc.setLineStyle( LineSolid );

  if ( !buttonPressed ) {
    //actWin->executeGc.setFG( offColor.getColor() );
    actWin->executeGc.setFG( offColor.getIndex(), &blink );
  }
  else {
    //actWin->executeGc.setFG( onColor.getColor() );
    actWin->executeGc.setFG( onColor.getIndex(), &blink );
  }

  XFillRectangle( actWin->d, XtWindow(actWin->executeWidget),
   actWin->executeGc.normGC(), x, y, w, h );

  if ( !_3D ) {

    actWin->executeGc.setFG( actWin->ci->pix(botShadowColor) );

  }

  XDrawRectangle( actWin->d, XtWindow(actWin->executeWidget),
   actWin->executeGc.normGC(), x, y, w, h );

  if ( !buttonPressed ) {

    if ( offLabel.getExpanded() )
      strncpy( string, offLabel.getExpanded(), MAX_ENUM_STRING_SIZE );
    else
      strncpy( string, "", MAX_ENUM_STRING_SIZE );

    // strncpy( string, offLabel, MAX_ENUM_STRING_SIZE );

    if ( _3D ) {

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

    }

  }
  else {

    if ( onLabel.getExpanded() )
      strncpy( string, onLabel.getExpanded(), MAX_ENUM_STRING_SIZE );
    else
      strncpy( string, "", MAX_ENUM_STRING_SIZE );

    // strncpy( string, onLabel, MAX_ENUM_STRING_SIZE );

    if ( _3D ) {

    actWin->executeGc.setFG( actWin->ci->pix(botShadowColor) );

    XDrawLine( actWin->d, XtWindow(actWin->executeWidget),
     actWin->executeGc.normGC(), x, y, x+w, y );

    XDrawLine( actWin->d, XtWindow(actWin->executeWidget),
     actWin->executeGc.normGC(), x, y, x, y+h );

    // top

    actWin->executeGc.setFG( actWin->ci->pix(topShadowColor) );

    // bottom

    actWin->executeGc.setFG( actWin->ci->pix(topShadowColor) );

    XDrawLine( actWin->d, XtWindow(actWin->executeWidget),
     actWin->executeGc.normGC(), x, y+h, x+w, y+h );

    //right

    actWin->executeGc.setFG( actWin->ci->pix(topShadowColor) );

    XDrawLine( actWin->d, XtWindow(actWin->executeWidget),
     actWin->executeGc.normGC(), x+w, y, x+w, y+h );

    }

  }

  if ( fs ) {

    actWin->executeGc.addNormXClipRectangle( xR );

    //actWin->executeGc.setFG( fgColor.getColor() );
    actWin->executeGc.setFG( fgColor.getIndex(), &blink );
    actWin->executeGc.setFontTag( fontTag, actWin->fi );

    tX = x + w/2;
    tY = y + h/2 - fontAscent/2;

    drawText( actWin->executeWidget, &actWin->executeGc, fs, tX, tY,
     XmALIGNMENT_CENTER, string );

    actWin->executeGc.removeNormXClipRectangle();

  }

  actWin->executeGc.restoreFg();

  updateBlink( blink );

  return 1;

}

int activeMessageButtonClass::activate (
  int pass,
  void *ptr )
{

int stat, opStat, i, l;
char tmpPvName[activeGraphicClass::MAX_PV_NAME+1];

  switch ( pass ) {

  case 1: // initialize

    opComplete = 0;

    break;

  case 2:

    if ( !opComplete ) {

      connection.init();

      numStates = 0;
      for ( i=0; i<MAX_ENUM_STATES; i++ ) {
        stateString[i] = NULL;
      }

      needConnectInit = needErase = needDraw = needPerformDownAction =
       needPerformUpAction = needWarning = needVisConnectInit =
       needVisInit = needVisUpdate = needColorConnectInit =
       needColorInit = needColorUpdate = 0;
       needToEraseUnconnected = 0;
      needToDrawUnconnected = 0;
      unconnectedTimer = 0;
      init = 0;
      aglPtr = ptr;

#ifdef __epics__
      sourcePressEventId = 0;
      sourceReleaseEventId = 0;
      visEventId = 0;
      colorEventId = 0;
#endif

      sourcePressExists = sourceReleaseExists = 0;

      destPvConnected = sourcePressPvConnected = sourceReleasePvConnected =
       active = buttonPressed = 0;
      activeMode = 1;

      if ( !unconnectedTimer ) {
        unconnectedTimer = appAddTimeOut( actWin->appCtx->appContext(),
         2000, unconnectedTimeout, this );
      }

      if ( !destPvExpString.getExpanded() ||
         ( strcmp( destPvExpString.getExpanded(), "" ) == 0 ) ) {
        destExists = 0;
      }
      else {
        destExists = 1;
        connection.addPv();
      }

      if ( !visPvExpString.getExpanded() ||
         ( strcmp( visPvExpString.getExpanded(), "" ) == 0 ) ) {
        visExists = 0;
        visibility = 1;
      }
      else {
        visExists = 1;
        connection.addPv();
      }

      if ( !colorPvExpString.getExpanded() ||
         ( strcmp( colorPvExpString.getExpanded(), "" ) == 0 ) ) {
        colorExists = 0;
      }
      else {
        colorExists = 1;
        connection.addPv();
      }

      opStat = 1;

#ifdef __epics__

      destIsAckS = 0;

      if ( destExists ) {

        strncpy( tmpPvName, destPvExpString.getExpanded(),
         activeGraphicClass::MAX_PV_NAME );

        l = strlen(tmpPvName);
        if ( l > 5 ) {
          i = l - 5;
          if ( strcmp( &tmpPvName[i], ".ACKS" ) == 0 ) {
            destIsAckS = 1;
            tmpPvName[i] = 0;
	  }
	}

        stat = ca_search_and_connect( tmpPvName, &destPvId,
         msgbt_monitor_dest_connect_state, this );
        if ( stat != ECA_NORMAL ) {
          printf( activeMessageButtonClass_str25 );
          opStat = 0;
        }

      }
      else {

        init = 1;
        smartDrawAllActive();

      }

      if ( visExists ) {

        stat = ca_search_and_connect( visPvExpString.getExpanded(), &visPvId,
         msgbt_monitor_vis_connect_state, this );
        if ( stat != ECA_NORMAL ) {
          printf( activeMessageButtonClass_str25 );
          opStat = 0;
        }
      }

      if ( colorExists ) {

        stat = ca_search_and_connect( colorPvExpString.getExpanded(),
         &colorPvId, msgbt_monitor_color_connect_state, this );
        if ( stat != ECA_NORMAL ) {
          printf( activeMessageButtonClass_str25 );
          opStat = 0;
        }
      }

      if ( opStat & 1 ) opComplete = 1;

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

int activeMessageButtonClass::deactivate (
  int pass
) {

int i, stat;

  if ( pass == 1 ) {

  active = 0;
  activeMode = 0;

  if ( unconnectedTimer ) {
    XtRemoveTimeOut( unconnectedTimer );
    unconnectedTimer = 0;
  }

  updateBlink( 0 );

#ifdef __epics__

  if ( destExists ) {
    stat = ca_clear_channel( destPvId );
    if ( stat != ECA_NORMAL ) printf( activeMessageButtonClass_str26 );
  }

  if ( visExists ) {
    stat = ca_clear_channel( visPvId );
    if ( stat != ECA_NORMAL ) printf( activeMessageButtonClass_str26 );
  }

  if ( colorExists ) {
    stat = ca_clear_channel( colorPvId );
    if ( stat != ECA_NORMAL ) printf( activeMessageButtonClass_str26 );
  }

#endif

  if ( numStates ) {
    for ( i=0; i<numStates; i++ ) {
      delete stateString[i];
      stateString[i] = NULL;
    }
    numStates = 0;
  }

  }

  return 1;

}

void activeMessageButtonClass::updateDimensions ( void )
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

void activeMessageButtonClass::performBtnUpAction ( void ) {

int stat;

  if ( toggle ) return;

  buttonPressed = 0;
  smartDrawAllActive();

  if ( strcmp( sourceReleasePvExpString.getExpanded(), "" ) == 0 ) return;

#ifdef __epics__

  if ( destIsAckS ) {

    destV.s = (short) atol( sourceReleasePvExpString.getExpanded() );
    stat = ca_put( DBR_PUT_ACKS, destPvId, &destV.s );

  }
  else {

    switch ( destType ) {

    case DBR_DOUBLE:
      destV.d = atof( sourceReleasePvExpString.getExpanded() );
      stat = ca_put( DBR_DOUBLE, destPvId, &destV.d );
      break;

    case DBR_LONG:
      destV.l = atol( sourceReleasePvExpString.getExpanded() );
      stat = ca_put( DBR_LONG, destPvId, &destV.l );
      break;

    case DBR_SHORT:
      destV.s = (short) atol( sourceReleasePvExpString.getExpanded() );
      stat = ca_put( DBR_SHORT, destPvId, &destV.s );
      break;

    case DBR_CHAR:
      destV.str[0] = (char) atol( sourceReleasePvExpString.getExpanded() );
      stat = ca_put( DBR_CHAR, destPvId, &destV.str[0] );
      break;

    case DBR_STRING:
      strncpy( destV.str, sourceReleasePvExpString.getExpanded(), 39 );
      stat = ca_put( DBR_STRING, destPvId, &destV.str );
      break;

    case DBR_ENUM:
      if ( useEnumNumeric ) {
        destV.s = (short) atol( sourceReleasePvExpString.getExpanded() );
        stat = ca_put( DBR_ENUM, destPvId, &destV.s );
      }
      else {
        stat = getEnumNumeric( sourceReleasePvExpString.getExpanded(),
         &destV.s );
        if ( !( stat & 1 ) ) {
          actWin->appCtx->postMessage( activeMessageButtonClass_str40 );
        }
        else {
          stat = ca_put( DBR_ENUM, destPvId, &destV.s );
        }
      }
      break;

    }

  }

#endif

}

void activeMessageButtonClass::btnUp (
  int x,
  int y,
  int buttonState,
  int buttonNumber,
  int *action )
{

  if ( !visibility ) {
    *action = 0;
    return;
  }

  if ( usePassword ) {
    *action = 0;
    return;
  }

  performBtnUpAction();

  *action = releaseAction;

}

void activeMessageButtonClass::performBtnDownAction ( void ) {

int stat;
char labelValue[39+1];

  if ( toggle ) {
    if ( buttonPressed ) {
      buttonPressed = 0;
    }
    else {
      buttonPressed = 1;
    }
  }
  else {
    buttonPressed = 1;
  }

  if ( buttonPressed ) {
    strncpy( labelValue, sourcePressPvExpString.getExpanded(), 39 );
  }
  else {
    strncpy( labelValue, sourceReleasePvExpString.getExpanded(), 39 );
  }

  smartDrawAllActive();

  if ( strcmp( labelValue, "" ) == 0 ) return;

#ifdef __epics__

  if ( destIsAckS ) {

    destV.s = (short) atol( labelValue );
    stat = ca_put( DBR_PUT_ACKS, destPvId, &destV.s );

  }
  else {

    switch ( destType ) {

    case DBR_FLOAT:
    case DBR_DOUBLE:
      destV.d = atof( labelValue );
      stat = ca_put( DBR_DOUBLE, destPvId, &destV.d );
      break;

    case DBR_LONG:
      destV.l = atol( labelValue );
      stat = ca_put( DBR_LONG, destPvId, &destV.l );
      break;

    case DBR_SHORT:
      destV.s = (short) atol( labelValue );
      stat = ca_put( DBR_SHORT, destPvId, &destV.s );
      break;

    case DBR_CHAR:
      destV.str[0] = (char) atol( labelValue );
      stat = ca_put( DBR_CHAR, destPvId, &destV.str[0] );
      break;

    case DBR_STRING:
      strncpy( destV.str, labelValue, 39 );
      stat = ca_put( DBR_STRING, destPvId, destV.str );
      break;

    case DBR_ENUM:
      if ( useEnumNumeric ) {
        destV.s = (short) atol( labelValue );
        stat = ca_put( DBR_ENUM, destPvId, &destV.s );
      }
      else {
        stat = getEnumNumeric( labelValue, &destV.s );
        if ( !( stat & 1 ) ) {
          actWin->appCtx->postMessage( activeMessageButtonClass_str39 );
        }
        else {
          stat = ca_put( DBR_ENUM, destPvId, &destV.s );
        }
      }
      break;

    }

  }

#endif

}

void activeMessageButtonClass::btnDown (
  int x,
  int y,
  int buttonState,
  int buttonNumber,
  int *action )
{

  if ( buttonNumber != 1 ) return;

  if ( !visibility ) {
    *action = 0;
    return;
  }

  if ( usePassword ) {

    if ( !ef.formIsPoppedUp() ) {

      pwFormX = actWin->x + x;
      pwFormY = actWin->y + y;
      pwFormW = 0;
      pwFormH = 0;
      pwFormMaxH = 600;

      ef.create( actWin->top,
       actWin->appCtx->ci.getColorMap(),
       &pwFormX, &pwFormY,
       &pwFormW, &pwFormH, &pwFormMaxH,
       "", NULL, NULL, NULL );

      strcpy( bufPw1, "" );

      ef.addPasswordField( activeMessageButtonClass_str36, 35, bufPw1, 31 );

      ef.finished( pw_ok, pw_apply, pw_cancel, this );

      ef.popup();

      *action = 0;
      return;

    }
    else {

      *action = 0;
      return;

    }

  }

  performBtnDownAction();

  *action = pressAction;

}

void activeMessageButtonClass::pointerIn (
  int _x,
  int _y,
  int buttonState )
{

  if ( !active || !visibility ) return;

  if ( !ca_write_access( destPvId ) ) {
    actWin->cursor.set( XtWindow(actWin->executeWidget), CURSOR_K_NO );
  }
  else {
    actWin->cursor.set( XtWindow(actWin->executeWidget), CURSOR_K_DEFAULT );
  }

  activeGraphicClass::pointerIn( _x, _y, buttonState );

}

int activeMessageButtonClass::getButtonActionRequest (
  int *up,
  int *down,
  int *drag,
  int *focus )
{

  *drag = 0;

  if ( destExists )
    *focus = 1;
  else
    *focus = 0;

  if ( !destExists ) {
    *up = 0;
    *down = 0;
    return 1;
  }

  *down = 1;
  *up = 1;

  return 1;

}

int activeMessageButtonClass::expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int stat, retStat = 1;

  stat = destPvExpString.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = sourcePressPvExpString.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = sourceReleasePvExpString.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = onLabel.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = offLabel.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = visPvExpString.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = colorPvExpString.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;

  return retStat;

}

int activeMessageButtonClass::expand2nd (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int stat, retStat = 1;

  stat = destPvExpString.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = sourcePressPvExpString.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = sourceReleasePvExpString.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = onLabel.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = offLabel.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = visPvExpString.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = colorPvExpString.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;

  return stat;

}

int activeMessageButtonClass::containsMacros ( void ) {

  if ( destPvExpString.containsPrimaryMacros() ) return 1;

  if ( sourcePressPvExpString.containsPrimaryMacros() ) return 1;

  if ( sourceReleasePvExpString.containsPrimaryMacros() ) return 1;

  if ( onLabel.containsPrimaryMacros() ) return 1;

  if ( offLabel.containsPrimaryMacros() ) return 1;

  if ( visPvExpString.containsPrimaryMacros() ) return 1;

  if ( colorPvExpString.containsPrimaryMacros() ) return 1;

  return 0;

}

void activeMessageButtonClass::executeDeferred ( void ) {

int nc, nd, ne, npda, npua, nw, nvc, nvi, nvu, ncc, nci, ncu;
int stat, index, invisColor;

  if ( actWin->isIconified ) return;

  actWin->appCtx->proc->lock();
  nc = needConnectInit; needConnectInit = 0;
  nd = needDraw; needDraw = 0;
  ne = needErase; needErase = 0;
  npda = needPerformDownAction; needPerformDownAction = 0;
  npua = needPerformUpAction; needPerformUpAction = 0;
  nw = needWarning; needWarning = 0;
  nvc = needVisConnectInit; needVisConnectInit = 0;
  nvi = needVisInit; needVisInit = 0;
  nvu = needVisUpdate; needVisUpdate = 0;
  ncc = needColorConnectInit; needColorConnectInit = 0;
  nci = needColorInit; needColorInit = 0;
  ncu = needColorUpdate; needColorUpdate = 0;
  visValue = curVisValue;
  colorValue = curColorValue;
  actWin->remDefExeNode( aglPtr );
  actWin->appCtx->proc->unlock();

  if ( !activeMode ) return;

//----------------------------------------------------------------------------

#ifdef __epics__

  if ( nc ) {

    connection.setPvConnected( (void *) destPvConnection );
    destType = ca_field_type( destPvId );

    if ( destType == DBR_ENUM ) {
      stat = ca_get_callback( DBR_GR_ENUM, destPvId,
       msgbt_destInfoUpdate, (void *) this );
    }

    if ( connection.pvsConnected() ) {
      active = 1;
      init = 1;
      onColor.setConnected();
      offColor.setConnected();
      smartDrawAllActive();
    }

  }

  if ( nvc ) {

    minVis = atof( minVisString );
    maxVis = atof( maxVisString );

    stat = ca_get_callback( DBR_GR_DOUBLE, visPvId,
     msgbt_visInfoUpdate, (void *) this );

  }

  if ( nvi ) {

    stat = ca_add_masked_array_event( DBR_DOUBLE, 1, visPvId,
     msgbt_visUpdate, (void *) this, (float) 0.0, (float) 0.0, (float) 0.0,
     &visEventId, DBE_VALUE );
    if ( stat != ECA_NORMAL ) printf( activeMessageButtonClass_str27 );

    if ( ( visValue >= minVis ) &&
         ( visValue < maxVis ) )
      visibility = 1 ^ visInverted;
    else
      visibility = 0 ^ visInverted;

    if ( prevVisibility != visibility ) {
      if ( !visibility ) eraseActive();
    }

    connection.setPvConnected( (void *) visPvConnection );

    if ( connection.pvsConnected() ) {
      active = 1;
      init = 1;
      onColor.setConnected();
      offColor.setConnected();
      smartDrawAllActive();
    }

  }

  if ( ncc ) {

    stat = ca_get_callback( DBR_GR_DOUBLE, colorPvId,
     msgbt_colorInfoUpdate, (void *) this );

  }

  if ( nci ) {

    stat = ca_add_masked_array_event( DBR_DOUBLE, 1, colorPvId,
     msgbt_colorUpdate, (void *) this, (float) 0.0, (float) 0.0, (float) 0.0,
     &colorEventId, DBE_VALUE );
    if ( stat != ECA_NORMAL ) printf( activeMessageButtonClass_str27 );

    invisColor = 0;

    index = actWin->ci->evalRule( onColor.pixelIndex(), colorValue );
    invisColor |= actWin->ci->isInvisible( index );
    onColor.changeIndex( index, actWin->ci );

    index = actWin->ci->evalRule( offColor.pixelIndex(), colorValue );
    invisColor |= actWin->ci->isInvisible( index );
    offColor.changeIndex( index, actWin->ci );

    index = actWin->ci->evalRule( fgColor.pixelIndex(), colorValue );
    invisColor |= actWin->ci->isInvisible( index );
    fgColor.changeIndex( index, actWin->ci );

    if ( !visExists ) {

      if ( invisColor ) {
        visibility = 0;
      }
      else {
        visibility = 1;
      }

      if ( prevVisibility != visibility ) {
        if ( !visibility ) eraseActive();
      }

    }

    connection.setPvConnected( (void *) colorPvConnection );

    if ( connection.pvsConnected() ) {
      active = 1;
      init = 1;
      onColor.setConnected();
      offColor.setConnected();
      smartDrawAllActive();
    }

  }

#endif

//----------------------------------------------------------------------------

  if ( nd ) {

    smartDrawAllActive();

  }

//----------------------------------------------------------------------------

  if ( ne ) {

    eraseActive();

  }

//----------------------------------------------------------------------------

  if ( npda ) {

    performBtnDownAction();

    if ( pressAction ) {

      actWin->closeDeferred( 2 );

    }
    else {

      if ( !toggle ) {
        actWin->appCtx->proc->lock();
        needPerformUpAction = 1;
        actWin->addDefExeNode( aglPtr );
        actWin->appCtx->proc->unlock();
      }

    }

  }

//----------------------------------------------------------------------------

  if ( npua ) {

    performBtnUpAction();

    if ( releaseAction ) actWin->closeDeferred( 2 );

  }

//----------------------------------------------------------------------------

  if ( nw ) {

    actWin->appCtx->postMessage( activeMessageButtonClass_str41 );

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

//----------------------------------------------------------------------------

  if ( ncu ) {

    invisColor = 0;

    index = actWin->ci->evalRule( onColor.pixelIndex(), colorValue );
    invisColor |= actWin->ci->isInvisible( index );
    onColor.changeIndex( index, actWin->ci );

    index = actWin->ci->evalRule( offColor.pixelIndex(), colorValue );
    invisColor |= actWin->ci->isInvisible( index );
    offColor.changeIndex( index, actWin->ci );

    index = actWin->ci->evalRule( fgColor.pixelIndex(), colorValue );
    invisColor |= actWin->ci->isInvisible( index );
    fgColor.changeIndex( index, actWin->ci );

    if ( !visExists ) {

      if ( invisColor ) {
        visibility = 0;
      }
      else {
        visibility = 1;
      }

      if ( prevVisibility != visibility ) {
        if ( !visibility ) eraseActive();
      }

    }

    stat = smartDrawAllActive();

  }

}

char *activeMessageButtonClass::firstDragName ( void ) {

  dragIndex = 0;
  return dragName[dragIndex];

}

char *activeMessageButtonClass::nextDragName ( void ) {

  if ( dragIndex < (int) ( sizeof(dragName) / sizeof(char *) ) - 1 ) {
    dragIndex++;
    return dragName[dragIndex];
  }
  else {
    return NULL;
  }

}

char *activeMessageButtonClass::dragValue (
  int i ) {

  if ( i == 0 ) {
    return destPvExpString.getExpanded();
  }
  else if ( i == 1 ) {
    return colorPvExpString.getExpanded();
  }
  else {
    return visPvExpString.getExpanded();
  }

}

void activeMessageButtonClass::changeDisplayParams (
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
    onColor.setColorIndex( _bgColor, actWin->ci );

  if ( _flag & ACTGRF_BGCOLOR_MASK )
    offColor.setColorIndex( _bgColor, actWin->ci );

  if ( _flag & ACTGRF_TOPSHADOWCOLOR_MASK )
    topShadowColor = _topShadowColor;

  if ( _flag & ACTGRF_BOTSHADOWCOLOR_MASK )
    botShadowColor = _botShadowColor;

  if ( _flag & ACTGRF_BTNFONTTAG_MASK ) {
    strncpy( fontTag, _btnFontTag, 63 );
    fontTag[63] = 0;
    actWin->fi->loadFontTag( fontTag );
    fs = actWin->fi->getXFontStruct( fontTag );
    updateDimensions();
  }

}

void activeMessageButtonClass::changePvNames (
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
      destPvExpString.setRaw( ctlPvs[0] );
    }
  }

  if ( flag & ACTGRF_VISPVS_MASK ) {
    if ( numVisPvs ) {
      visPvExpString.setRaw( ctlPvs[0] );
    }
  }

}

int activeMessageButtonClass::getEnumNumeric (
  char *string,
  short *value ) {

  int i;

  for ( i=0; i<numStates; i++ ) {
    if ( strcmp( string, stateString[i] ) == 0 ) {
      *value = (short) i;
      return 1;
    }
  }

  *value = 0;
  return 0;

}

#ifdef __cplusplus
extern "C" {
#endif

void *create_activeMessageButtonClassPtr ( void ) {

activeMessageButtonClass *ptr;

  ptr = new activeMessageButtonClass;
  return (void *) ptr;

}

void *clone_activeMessageButtonClassPtr (
  void *_srcPtr )
{

activeMessageButtonClass *ptr, *srcPtr;

  srcPtr = (activeMessageButtonClass *) _srcPtr;

  ptr = new activeMessageButtonClass( srcPtr );

  return (void *) ptr;

}

#ifdef __cplusplus
}
#endif
