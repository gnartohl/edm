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

#define __buttonGen_cc 1

#include "buttonGen.h"
#include "app_pkg.h"
#include "act_win.h"

#include "thread.h"

static void btc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeButtonClass *bto = (activeButtonClass *) client;

  bto->actWin->setChanged();

  bto->eraseSelectBoxCorners();
  bto->erase();

  bto->fgColorMode = bto->bufFgColorMode;
  if ( bto->fgColorMode == BTC_K_COLORMODE_ALARM )
    bto->fgColor.setAlarmSensitive();
  else
    bto->fgColor.setAlarmInsensitive();
  bto->fgColor.setColor( bto->bufFgColor, bto->actWin->ci );

  bto->onColor.setColor( bto->bufOnColor, bto->actWin->ci );

  bto->offColor.setColor( bto->bufOffColor, bto->actWin->ci );

  bto->inconsistentColor.setColor( bto->bufInconsistentColor,
   bto->actWin->ci );

  bto->topShadowColor = bto->bufTopShadowColor;
  bto->botShadowColor = bto->bufBotShadowColor;

  
  bto->controlPvName.setRaw( bto->controlBufPvName );
  bto->readPvName.setRaw( bto->readBufPvName );

  strncpy( bto->onLabel, bto->bufOnLabel, MAX_ENUM_STRING_SIZE );
  strncpy( bto->offLabel, bto->bufOffLabel, MAX_ENUM_STRING_SIZE );

  strncpy( bto->pvUserClassName, bto->actWin->pvObj.getPvName(bto->pvNameIndex), 15);
  strncpy( bto->pvClassName, bto->actWin->pvObj.getPvClassName(bto->pvNameIndex), 15);

  if ( strcmp( bto->labelTypeString, "PV State" ) == 0 )
    bto->labelType = BTC_K_PV_STATE;
  else
    bto->labelType = BTC_K_LITERAL;

  strncpy( bto->fontTag, bto->fm.currentFontTag(), 63 );
  bto->actWin->fi->loadFontTag( bto->fontTag );
  bto->fs = bto->actWin->fi->getXFontStruct( bto->fontTag );

  if ( strcmp( bto->buttonTypeStr, "Push" ) == 0 ) {
    bto->toggle = 0;
    bto->buttonType = BTC_K_PUSH;
  }
  else {
    bto->toggle = 1;
    bto->buttonType = BTC_K_TOGGLE;
  }

  if ( strcmp( bto->_3DString, "Yes" ) == 0 )
    bto->_3D = 1;
  else
    bto->_3D = 0;

  if ( strcmp( bto->invisibleString, "Yes" ) == 0 )
    bto->invisible = 1;
  else
    bto->invisible = 0;

  strncpy( bto->id, bto->bufId, 31 );
  bto->downCallbackFlag = bto->bufDownCallbackFlag;
  bto->upCallbackFlag = bto->bufUpCallbackFlag;
  bto->activateCallbackFlag = bto->bufActivateCallbackFlag;
  bto->deactivateCallbackFlag = bto->bufDeactivateCallbackFlag;
  bto->anyCallbackFlag = bto->downCallbackFlag || bto->upCallbackFlag ||
   bto->activateCallbackFlag || bto->deactivateCallbackFlag;

  bto->x = bto->bufX;
  bto->sboxX = bto->bufX;

  bto->y = bto->bufY;
  bto->sboxY = bto->bufY;

  bto->w = bto->bufW;
  bto->sboxW = bto->bufW;

  bto->h = bto->bufH;
  bto->sboxH = bto->bufH;

  bto->updateDimensions();

}

static void btc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeButtonClass *bto = (activeButtonClass *) client;

  btc_edit_update( w, client, call );
  bto->refresh( bto );

}

static void btc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeButtonClass *bto = (activeButtonClass *) client;

  btc_edit_apply ( w, client, call );
  bto->ef.popdown();
  bto->operationComplete();

}

static void btc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeButtonClass *bto = (activeButtonClass *) client;

  bto->ef.popdown();
  bto->operationCancel();

}

static void btc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeButtonClass *bto = (activeButtonClass *) client;

  bto->ef.popdown();
  bto->operationCancel();
  bto->erase();
  bto->deleteRequest = 1;
  bto->drawAll();

}

static void bt_monitor_control_connect_state (
  pvClass *classPtr,
  void *clientData,
  void *args )
{

activeButtonClass *bto = (activeButtonClass *) clientData;

  bto->actWin->appCtx->proc->lock();

  if ( !bto->activeMode ) {
    bto->actWin->appCtx->proc->unlock();
    return;
  }

  if ( classPtr->getOp( args ) == classPtr->pvkOpConnUp() ) {

    // Gets pvType, so other functions can use it directly.
    // bto->controlType = bto->controlPvId->getType();

    bto->needCtlConnectInit = 1;

  }
  else {

    bto->controlValid = 0;
    bto->controlPvConnected = 0;
    bto->active = 0;
    bto->onColor.setDisconnected();
    bto->offColor.setDisconnected();
    bto->inconsistentColor.setDisconnected();
    bto->needDraw = 1;

  }

  bto->actWin->addDefExeNode( bto->aglPtr );
  bto->actWin->appCtx->proc->unlock();

}

static void bt_controlInfoUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args )
{

activeButtonClass *bto = (activeButtonClass *) clientData;
// struct dbr_gr_enum controlRec = *( (dbr_gr_enum *) ast_args.dbr );

int enumStringSize = bto->controlPvId->enumStringSize();

  // bto->curControlV = controlRec.value;

  bto->actWin->appCtx->proc->lock();

  if ( !bto->activeMode ) {
    bto->actWin->appCtx->proc->unlock();
    return;
  }

  bto->curControlV = *( (short *) bto->controlPvId->getValue( args ) );

  if ( !(bto->readExists) ) {

//      bto->active = 1;
//      bto->init = 1;

    bto->no_str = bto->controlPvId->getNumStates( args );

     if ( bto->no_str > 0 ) {
       strncpy( bto->stateString[0], 
        bto->controlPvId->getStateString( args, 0 ), enumStringSize );
       bto->stateString[0][enumStringSize] = '\0';
       // strncpy( bto->stateString[0], controlRec.strs[0], MAX_ENUM_STRING_SIZE );
     }
     else {
       strncpy( bto->stateString[0], "?0?", enumStringSize );
     }
     if ( bto->no_str > 1 ) {
       strncpy( bto->stateString[1], 
        bto->controlPvId->getStateString( args, 1 ), enumStringSize );
       bto->stateString[1][enumStringSize] = '\0';
       // strncpy( bto->stateString[1], controlRec.strs[1], MAX_ENUM_STRING_SIZE );
     }
     else {
       strncpy( bto->stateString[1], "?1?", enumStringSize );
     }

  }

  bto->needCtlInfoInit = 1;
  bto->actWin->addDefExeNode( bto->aglPtr );
  bto->actWin->appCtx->proc->unlock();

}

static void bt_controlUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args )
{

activeButtonClass *bto = (activeButtonClass *) clientData;

  bto->controlValid = 1;
  bto->actWin->appCtx->proc->lock();

  if ( !bto->activeMode ) {
    bto->actWin->appCtx->proc->unlock();
    return;
  }

  bto->curControlV = *( (short *) bto->controlPvId->getValue( args ) );
  bto->needCtlRefresh = 1;
  bto->actWin->addDefExeNode( bto->aglPtr );
  bto->actWin->appCtx->proc->unlock();

}

static void bt_monitor_read_connect_state (
  pvClass *classPtr,
  void *clientData,
  void *args )
{

activeButtonClass *bto = (activeButtonClass *) clientData;

  if ( classPtr->getOp( args ) == classPtr->pvkOpConnUp() ) {

    bto->needReadConnectInit = 1;

  }
  else {

    bto->readValid = 0;
    bto->readPvConnected = 0;
    bto->active = 0;
    bto->onColor.setDisconnected();
    bto->offColor.setDisconnected();
    bto->inconsistentColor.setDisconnected();
    bto->needDraw = 1;

  }

  bto->actWin->appCtx->proc->lock();
  bto->actWin->addDefExeNode( bto->aglPtr );
  bto->actWin->appCtx->proc->unlock();

}

static void bt_readInfoUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args )
{


activeButtonClass *bto = (activeButtonClass *) clientData;
// struct dbr_gr_enum readRec = *( (dbr_gr_enum *) ast_args.dbr );

int enumStringSize = bto->readPvId->enumStringSize();


  // bto->curControlV = controlRec.value;

  bto->actWin->appCtx->proc->lock();

  if ( !bto->activeMode ) {
    bto->actWin->appCtx->proc->unlock();
    return;
  }

  bto->curReadV = *( (short *) bto->readPvId->getValue( args ) );

  bto->no_str = enumStringSize;

  if ( bto->no_str > 0 ) {
    strncpy( bto->stateString[0], 
     bto->readPvId->getStateString( args, 0 ), enumStringSize );
    bto->stateString[0][enumStringSize] = '\0';
    //strncpy( bto->stateString[0], readRec.strs[0], MAX_ENUM_STRING_SIZE );
  }
  else {
    strncpy( bto->stateString[0], "?", enumStringSize );
  }
  if ( bto->no_str > 1 ) {
    strncpy( bto->stateString[1], 
     bto->readPvId->getStateString( args, 1 ), enumStringSize );
    bto->stateString[1][enumStringSize] = '\0';
    //strncpy( bto->stateString[1], readRec.strs[1], MAX_ENUM_STRING_SIZE );
  }
  else {
    strncpy( bto->stateString[1], "?", enumStringSize );
  }

  bto->needReadInfoInit = 1;
  bto->actWin->addDefExeNode( bto->aglPtr );
  bto->actWin->appCtx->proc->unlock();

}

static void bt_readUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args )
{

activeButtonClass *bto = (activeButtonClass *) clientData;

  bto->readValid = 1;
  bto->actWin->appCtx->proc->lock();

  if ( !bto->activeMode ) {
    bto->actWin->appCtx->proc->unlock();
    return;
  }

  bto->curReadV = *( (short *) bto->readPvId->getValue( args ) );
  bto->needReadRefresh = 1;
  bto->actWin->addDefExeNode( bto->aglPtr );
  bto->actWin->appCtx->proc->unlock();

}

static void bt_alarmUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args )
{

activeButtonClass *bto = (activeButtonClass *) clientData;

  bto->actWin->appCtx->proc->lock();

  if ( !bto->activeMode ) {
    bto->actWin->appCtx->proc->unlock();
    return;
  }

  // statusRec = *( (struct dbr_sts_enum *) ast_args.dbr );

  if (!bto->readExists) {
    bto->fgColor.setStatus( bto->controlPvId->getStatus( args ),
                            bto->controlPvId->getSeverity( args ) );
  }
  else {
    bto->fgColor.setStatus( bto->readPvId->getStatus( args ),
                            bto->readPvId->getSeverity( args ) );
  }

  bto->needErase = 1;
  bto->needDraw = 1;
  bto->actWin->addDefExeNode( bto->aglPtr );
  bto->actWin->appCtx->proc->unlock();

}

activeButtonClass::activeButtonClass ( void ) {

  name = new char[strlen("activeButtonClass")+1];
  strcpy( name, "activeButtonClass" );
  deleteRequest = 0;
  selected = 0;
  strcpy( stateString[0], "" );
  strcpy( stateString[1], "" );

  strcpy( id, "" );
  downCallbackFlag = 0;
  upCallbackFlag = 0;
  activateCallbackFlag = 0;
  deactivateCallbackFlag = 0;
  anyCallbackFlag = 0;
  downCallback = NULL;
  upCallback = NULL;
  activateCallback = NULL;
  deactivateCallback = NULL;

  fgColorMode = BTC_K_COLORMODE_STATIC;

}

// copy constructor
activeButtonClass::activeButtonClass
 ( const activeButtonClass *source ) {

activeGraphicClass *bto = (activeGraphicClass *) this;

  bto->clone( (activeGraphicClass *) source );

  name = new char[strlen("activeButtonClass")+1];
  strcpy( name, "activeButtonClass" );
  strcpy( stateString[0], "" );
  strcpy( stateString[1], "" );

  deleteRequest = 0;

  strcpy( id, source->id );

  downCallbackFlag = source->downCallbackFlag;
  upCallbackFlag = source->upCallbackFlag;
  activateCallbackFlag = source->activateCallbackFlag;
  deactivateCallbackFlag = source->deactivateCallbackFlag;
  anyCallbackFlag = downCallbackFlag || upCallbackFlag ||
   activateCallbackFlag || deactivateCallbackFlag;
  downCallback = NULL;
  upCallback = NULL;
  activateCallback = NULL;
  deactivateCallback = NULL;

  fgCb = source->fgCb;
  onCb = source->onCb;
  offCb = source->offCb;
  inconsistentCb = source->inconsistentCb;
  topShadowCb = source->topShadowCb;
  botShadowCb = source->botShadowCb;

  strncpy( fontTag, source->fontTag, 63 );

  fs = actWin->fi->getXFontStruct( fontTag );

  fgColor.copy( source->fgColor );
  offColor.copy( source->offColor );
  onColor.copy( source->onColor );
  inconsistentColor.copy( source->inconsistentColor );

  topShadowColor = source->topShadowColor;
  botShadowColor = source->botShadowColor;

  controlPvName.copy( source->controlPvName );
  readPvName.copy( source->readPvName );

  strncpy( pvClassName, source->pvClassName, 15 );
  strncpy( pvUserClassName, source->pvUserClassName, 15 );

  strncpy( onLabel, source->onLabel, MAX_ENUM_STRING_SIZE );
  strncpy( offLabel, source->offLabel, MAX_ENUM_STRING_SIZE );

  labelType = source->labelType;

  buttonType = source->buttonType;
  if ( buttonType == BTC_K_TOGGLE )
    toggle = 1;
  else
    toggle = 0;

  _3D = source->_3D;
  invisible = source->invisible;

  updateDimensions();

}

int activeButtonClass::createInteractive (
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

  fgColor.setColor( actWin->defaultTextFgColor, actWin->ci );
  onColor.setColor( actWin->defaultBgColor, actWin->ci );
  offColor.setColor( actWin->defaultBgColor, actWin->ci );
  inconsistentColor.setColor( actWin->defaultOffsetColor,
   actWin->ci );
  topShadowColor = actWin->defaultTopShadowColor;
  botShadowColor = actWin->defaultBotShadowColor;

  strcpy( stateString[0], "" );
  strcpy( stateString[1], "" );

  strcpy( fontTag, actWin->defaultFontTag );
  actWin->fi->loadFontTag( fontTag );
  fs = actWin->fi->getXFontStruct( fontTag );

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

  updateDimensions();

  strcpy( onLabel, "" );
  strcpy( offLabel, "" );

  labelType = BTC_K_PV_STATE;
  buttonType = BTC_K_TOGGLE;
  toggle = 1;
  _3D = 1;
  invisible = 0;

  strncpy( pvUserClassName, actWin->defaultPvType, 15 );

  this->draw();

  this->editCreate();

  return 1;

}

int activeButtonClass::save (
  FILE *f )
{

int r, g, b;

  fprintf( f, "%-d %-d %-d\n", BTC_MAJOR_VERSION, BTC_MINOR_VERSION,
   BTC_RELEASE );

  fprintf( f, "%-d\n", x );
  fprintf( f, "%-d\n", y );
  fprintf( f, "%-d\n", w );
  fprintf( f, "%-d\n", h );

  actWin->ci->getRGB( fgColor.pixelColor(), &r, &g, &b );
  fprintf( f, "%-d %-d %-d\n", r, g, b );

  fprintf( f, "%-d\n", fgColorMode );

  actWin->ci->getRGB( onColor.pixelColor(), &r, &g, &b );
  fprintf( f, "%-d %-d %-d\n", r, g, b );

  actWin->ci->getRGB( offColor.pixelColor(), &r, &g, &b );
  fprintf( f, "%-d %-d %-d\n", r, g, b );

  actWin->ci->getRGB( inconsistentColor.pixelColor(), &r, &g, &b );
  fprintf( f, "%-d %-d %-d\n", r, g, b );

  actWin->ci->getRGB( topShadowColor, &r, &g, &b );
  fprintf( f, "%-d %-d %-d\n", r, g, b );

  actWin->ci->getRGB( botShadowColor, &r, &g, &b );
  fprintf( f, "%-d %-d %-d\n", r, g, b );

  if ( controlPvName.getRaw() )
    writeStringToFile( f, controlPvName.getRaw() );
  else
    writeStringToFile( f, "" );

  if ( readPvName.getRaw() )
    writeStringToFile( f, readPvName.getRaw() );
  else
    writeStringToFile( f, "" );

  writeStringToFile( f, onLabel );
  writeStringToFile( f, offLabel );

  fprintf( f, "%-d\n", labelType );

  if ( toggle )
    buttonType = BTC_K_TOGGLE;
  else
    buttonType = BTC_K_PUSH;

  fprintf( f, "%-d\n", buttonType );

  fprintf( f, "%-d\n", _3D );

  fprintf( f, "%-d\n", invisible );

  writeStringToFile( f, fontTag );

  // version 1.3.0
  writeStringToFile( f, id );
  fprintf( f, "%-d\n", downCallbackFlag );
  fprintf( f, "%-d\n", upCallbackFlag );
  fprintf( f, "%-d\n", activateCallbackFlag );
  fprintf( f, "%-d\n", deactivateCallbackFlag );

  // version 1.4.0
  writeStringToFile( f, pvClassName );

  return 1;

}

int activeButtonClass::createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int r, g, b;
int major, minor, release;
unsigned int pixel;
char oneName[39+1];

  this->actWin = _actWin;

  fscanf( f, "%d %d %d\n", &major, &minor, &release ); actWin->incLine();

  fscanf( f, "%d\n", &x ); actWin->incLine();
  fscanf( f, "%d\n", &y ); actWin->incLine();
  fscanf( f, "%d\n", &w ); actWin->incLine();
  fscanf( f, "%d\n", &h ); actWin->incLine();

  fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
  if ( ( major < 2 ) && ( minor < 2 ) ) {
    r *= 256;
    g *= 256;
    b *= 256;
  }
  actWin->ci->setRGB( r, g, b, &pixel );
  fgColor.setColor( pixel, actWin->ci );

  fscanf( f, "%d\n", &fgColorMode ); actWin->incLine();

  if ( fgColorMode == BTC_K_COLORMODE_ALARM )
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
  onColor.setColor( pixel, actWin->ci );

  fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
  if ( ( major < 2 ) && ( minor < 2 ) ) {
    r *= 256;
    g *= 256;
    b *= 256;
  }
  actWin->ci->setRGB( r, g, b, &pixel );
  offColor.setColor( pixel, actWin->ci );

  fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
  if ( ( major < 2 ) && ( minor < 2 ) ) {
    r *= 256;
    g *= 256;
    b *= 256;
  }
  actWin->ci->setRGB( r, g, b, &pixel );
  inconsistentColor.setColor( pixel, actWin->ci );

  fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
  if ( ( major < 2 ) && ( minor < 2 ) ) {
    r *= 256;
    g *= 256;
    b *= 256;
  }
  actWin->ci->setRGB( r, g, b, &topShadowColor );

  fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
  if ( ( major < 2 ) && ( minor < 2 ) ) {
    r *= 256;
    g *= 256;
    b *= 256;
  }
  actWin->ci->setRGB( r, g, b, &botShadowColor );

  readStringFromFile( oneName, 39, f ); actWin->incLine();
  controlPvName.setRaw( oneName );

  readStringFromFile( oneName, 39, f ); actWin->incLine();
  readPvName.setRaw( oneName );

  readStringFromFile( onLabel, MAX_ENUM_STRING_SIZE, f ); actWin->incLine();

  readStringFromFile( offLabel, MAX_ENUM_STRING_SIZE, f ); actWin->incLine();

  fscanf( f, "%d\n", &labelType ); actWin->incLine();

  fscanf( f, "%d\n", &buttonType ); actWin->incLine();

  if ( buttonType == BTC_K_TOGGLE )
    toggle = 1;
  else
    toggle = 0;

  fscanf( f, "%d\n", &_3D ); actWin->incLine();

  fscanf( f, "%d\n", &invisible ); actWin->incLine();

  readStringFromFile( fontTag, 63, f ); actWin->incLine();

  if ( ( major > 1 ) || ( minor > 2 ) ) {
    readStringFromFile( this->id, 31, f ); actWin->incLine();
    fscanf( f, "%d\n", &downCallbackFlag ); actWin->incLine();
    fscanf( f, "%d\n", &upCallbackFlag ); actWin->incLine();
    fscanf( f, "%d\n", &activateCallbackFlag ); actWin->incLine();
    fscanf( f, "%d\n", &deactivateCallbackFlag ); actWin->incLine();
    anyCallbackFlag = downCallbackFlag || upCallbackFlag ||
     activateCallbackFlag || deactivateCallbackFlag;
  }
  else {
    strcpy( this->id, "" );
    downCallbackFlag = 0;
    upCallbackFlag = 0;
    activateCallbackFlag = 0;
    deactivateCallbackFlag = 0;
    anyCallbackFlag = 0;
  }

  this->initSelectBox();

  actWin->fi->loadFontTag( fontTag );
  fs = actWin->fi->getXFontStruct( fontTag );

  if ( ( major > 1 ) || ( minor > 3 ) ) {
    readStringFromFile( pvClassName, 15, f ); actWin->incLine();
    strncpy( pvUserClassName, actWin->pvObj.getNameFromClass( pvClassName ),
      15 );
  }

  updateDimensions();

  return 1;

}

int activeButtonClass::importFromXchFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int r, g, b, more, param;
char *tk, *gotData, *context, buf[255+1];

  r = 0xffff;
  g = 0xffff;
  b = 0xffff;

  this->actWin = _actWin;

  fgColor.setColor( actWin->defaultTextFgColor, actWin->ci );
  onColor.setColor( actWin->defaultBgColor, actWin->ci );
  offColor.setColor( actWin->defaultBgColor, actWin->ci );
  inconsistentColor.setColor( actWin->defaultOffsetColor,
   actWin->ci );
  topShadowColor = actWin->defaultTopShadowColor;
  botShadowColor = actWin->defaultBotShadowColor;

  strcpy( stateString[0], "" );
  strcpy( stateString[1], "" );

  strcpy( fontTag, actWin->defaultFontTag );
  actWin->fi->loadFontTag( fontTag );
  fs = actWin->fi->getXFontStruct( fontTag );

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

  updateDimensions();

  strcpy( onLabel, "" );
  strcpy( offLabel, "" );

  labelType = BTC_K_PV_STATE;
  buttonType = BTC_K_TOGGLE;
  toggle = 1;
  _3D = 1;
  invisible = 0;

  // continue until tag is <eod>

  do {

    gotData = getNextDataString( buf, 255, f );
    if ( !gotData ) {
      actWin->appCtx->postMessage( "import file syntax error" );
      return 0;
    }

    context = NULL;

    tk = strtok_r( buf, " \t\n", &context );
    if ( !tk ) {
      actWin->appCtx->postMessage( "import file syntax error" );
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
          actWin->appCtx->postMessage( "import file syntax error" );
          return 0;
        }

        x = atol( tk );

      }
      else if ( strcmp( tk, "y" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( "import file syntax error" );
          return 0;
        }

        y = atol( tk );

      }
      else if ( strcmp( tk, "w" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( "import file syntax error" );
          return 0;
        }

        w = atol( tk );

      }
      else if ( strcmp( tk, "h" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( "import file syntax error" );
          return 0;
        }

        h = atol( tk );

      }
            
      else if ( strcmp( tk, "ctlpv" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );

        if ( tk )
          controlPvName.setRaw( tk );
        else
          controlPvName.setRaw( "" );

      }
            
      else if ( strcmp( tk, "readpv" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );

        if ( tk )
          readPvName.setRaw( tk );
        else
          readPvName.setRaw( "" );

      }
            
      else if ( strcmp( tk, "truelabel" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );

        if ( tk ) {
          strncpy( onLabel, tk, MAX_ENUM_STRING_SIZE );
          onLabel[MAX_ENUM_STRING_SIZE] = 0;
	}
	else {
          strcpy( onLabel, "" );
	}

      }
            
      else if ( strcmp( tk, "falselabel" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );

        if ( tk ) {
          strncpy( offLabel, tk, MAX_ENUM_STRING_SIZE );
          offLabel[MAX_ENUM_STRING_SIZE] = 0;
	}
	else {
          strcpy( offLabel, "" );
	}

      }
            
      else if ( strcmp( tk, "font" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( "import file syntax error" );
          return 0;
        }

        strncpy( fontTag, tk, 63 );

      }
            
      else if ( strcmp( tk, "push" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( "import file syntax error" );
          return 0;
        }

        param = atol( tk );
        if ( param == 1 ) {
          buttonType = BTC_K_PUSH;
          toggle = 0;
	}
	else {
          buttonType = BTC_K_TOGGLE;
          toggle = 1;
	}

      }
            
      else if ( strcmp( tk, "3d" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( "import file syntax error" );
          return 0;
        }

        _3D = atol( tk );

      }
            
      else if ( strcmp( tk, "labelfrompv" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( "import file syntax error" );
          return 0;
        }

        param = atol( tk );
        if ( param == 1 ) {
          labelType = BTC_K_PV_STATE;
	}
	else {
          labelType = BTC_K_LITERAL;
	}

      }
            
      else if ( strcmp( tk, "invisible" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( "import file syntax error" );
          return 0;
        }

        invisible = atol( tk );

      }
            
    }

  } while ( more );

  this->initSelectBox();

  fgColor.setAlarmInsensitive();

  actWin->fi->loadFontTag( fontTag );
  fs = actWin->fi->getXFontStruct( fontTag );

  updateDimensions();

  return 1;

}

int activeButtonClass::genericEdit ( void ) {

char title[32], *ptr;

  ptr = actWin->obj.getNameFromClass( "activeButtonClass" );
  if ( ptr )
    strncpy( title, ptr, 31 );
  else
    strncpy( title, "Unknown object", 31 );

  Strncat( title, " Properties", 31 );

  strncpy( bufId, id, 31 );

  bufX = x;
  bufY = y;
  bufW = w;
  bufH = h;

  bufFgColor = fgColor.pixelColor();
  bufFgColorMode = fgColorMode;

  bufOnColor = onColor.pixelColor();

  bufOffColor = offColor.pixelColor();

  bufInconsistentColor = inconsistentColor.pixelColor();

  bufTopShadowColor = topShadowColor;
  bufBotShadowColor = botShadowColor;
  strncpy( bufFontTag, fontTag, 63 );

  if ( controlPvName.getRaw() )
    strncpy( controlBufPvName, controlPvName.getRaw(), 39 );
  else
    strncpy( controlBufPvName, "", 39 );

  if ( readPvName.getRaw() )
    strncpy( readBufPvName, readPvName.getRaw(), 39 );
  else
    strncpy( readBufPvName, "", 39 );

  strncpy( bufOnLabel, onLabel, MAX_ENUM_STRING_SIZE );
  strncpy( bufOffLabel, offLabel, MAX_ENUM_STRING_SIZE );

  bufDownCallbackFlag = downCallbackFlag;
  bufUpCallbackFlag = upCallbackFlag;
  bufActivateCallbackFlag = activateCallbackFlag;
  bufDeactivateCallbackFlag = deactivateCallbackFlag;

  if ( labelType == BTC_K_PV_STATE )
    strcpy( labelTypeString, "PV State" );
  else
    strcpy( labelTypeString, "Literal" );

  if ( toggle )
    strcpy( buttonTypeStr, "Toggle" );
  else
    strcpy( buttonTypeStr, "Push" );

  if ( _3D )
    strcpy( _3DString, "Yes" );
  else
    strcpy( _3DString, "No" );

  if ( invisible )
    strcpy( invisibleString, "Yes" );
  else
    strcpy( invisibleString, "No" );

  ef.create( actWin->top, actWin->appCtx->ci.getColorMap(),
   &actWin->appCtx->entryFormX,
   &actWin->appCtx->entryFormY, &actWin->appCtx->entryFormW,
   &actWin->appCtx->entryFormH, &actWin->appCtx->largestH,
   title, NULL, NULL, NULL );

  ef.addTextField( "ID", 27, bufId, 31 );
  ef.addTextField( "X", 27, &bufX );
  ef.addTextField( "Y", 27, &bufY );
  ef.addTextField( "Width", 27, &bufW );
  ef.addTextField( "Height", 27, &bufH );
  ef.addTextField( "Control PV", 27, controlBufPvName, 39 );
  ef.addTextField( "Readback PV", 27, readBufPvName, 39 );
  ef.addOption( "Button Type", "Push|Toggle", buttonTypeStr, 7 );
  ef.addOption( "3-D Look", "Yes|No", _3DString, 7 );
  ef.addOption( "Invisible", "Yes|No", invisibleString, 7 );
  ef.addOption( "Label Type", "PV State|Literal", labelTypeString, 15 );
  ef.addTextField( "On Label", 27, bufOnLabel, MAX_ENUM_STRING_SIZE );
  ef.addTextField( "Off Label", 27, bufOffLabel, MAX_ENUM_STRING_SIZE );
  ef.addToggle( "Activate Callback", &bufActivateCallbackFlag );
  ef.addToggle( "Deactivate Callback", &bufDeactivateCallbackFlag );
  ef.addToggle( "Down Callback", &bufDownCallbackFlag );
  ef.addToggle( "Up Callback", &bufUpCallbackFlag );
  ef.addFontMenu( "Label Font", actWin->fi, &fm, fontTag );
  ef.addColorButton( "Fg", actWin->ci, &fgCb, &bufFgColor );
  ef.addOption( "FG Mode", "Static|Alarm", &bufFgColorMode );
  ef.addColorButton( "On", actWin->ci, &onCb, &bufOnColor );
  ef.addColorButton( "Off", actWin->ci, &offCb, &bufOffColor );
  ef.addColorButton( "Inconsistent", actWin->ci,
   &inconsistentCb, &bufInconsistentColor );
  ef.addColorButton( "Top Shadow", actWin->ci, &topShadowCb, &bufTopShadowColor );
  ef.addColorButton( "Bottom Shadow", actWin->ci, &botShadowCb, &bufBotShadowColor );

  actWin->pvObj.getOptionMenuList( pvOptionList, 255, &numPvTypes );
  if ( numPvTypes == 1 ) {
    pvNameIndex= 0;
  }
  else {
    // printf( "pvUserClassName = [%s]\n", pvUserClassName );
    pvNameIndex = actWin->pvObj.getNameNum( pvUserClassName );
    if ( pvNameIndex < 0 ) pvNameIndex = 0;
    // printf( "pvOptionList = [%s]\n", pvOptionList );
    // printf( "pvNameIndex = %-d\n", pvNameIndex );
    ef.addOption( "PV Type", pvOptionList, &pvNameIndex );
  }

  XtUnmanageChild( fm.alignWidget() ); // no alignment info

  return 1;

}

int activeButtonClass::editCreate ( void ) {

  this->genericEdit();
  ef.finished( btc_edit_ok, btc_edit_apply, btc_edit_cancel_delete, this );
  actWin->currentEf = NULL;
  ef.popup();

  return 1;

}

int activeButtonClass::edit ( void ) {

  this->genericEdit();
  ef.finished( btc_edit_ok, btc_edit_apply, btc_edit_cancel, this );
  actWin->currentEf = &ef;
  ef.popup();

  return 1;

}

int activeButtonClass::erase ( void ) {

  if ( deleteRequest ) return 1;

  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  return 1;

}

int activeButtonClass::eraseActive ( void ) {

  if ( !init || !activeMode || invisible ) return 1;

  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  return 1;

}

int activeButtonClass::draw ( void ) {

int tX, tY;
XRectangle xR = { x, y, w, h };

  if ( deleteRequest ) return 1;

  actWin->drawGc.saveFg();

  actWin->drawGc.setFG( onColor.pixelColor() );

  XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x, y, w, h );

  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x, y, w, h );

  if ( _3D ) {

  actWin->drawGc.setFG( botShadowColor );

  XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x, y, x+w, y );

  XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x, y, x, y+h );

   actWin->drawGc.setFG( topShadowColor );

   XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
    actWin->drawGc.normGC(), x, y+h, x+w, y+h );

   XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
    actWin->drawGc.normGC(), x+w, y, x+w, y+h );

  actWin->drawGc.setFG( topShadowColor );

  XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x+1, y+1, x+w-1, y+1 );

  XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x+2, y+2, x+w-2, y+2 );

  XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x+1, y+1, x+1, y+h-1 );

  XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x+2, y+2, x+2, y+h-2 );

  actWin->drawGc.setFG( botShadowColor );

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

    actWin->drawGc.setFG( fgColor.pixelColor() );
    actWin->drawGc.setFontTag( fontTag, actWin->fi );

    tX = x + w/2;
    tY = y + h/2 - fontAscent/2;

    drawText( actWin->drawWidget, &actWin->drawGc, fs, tX, tY,
     XmALIGNMENT_CENTER, onLabel );

    actWin->drawGc.removeNormXClipRectangle();

  }

  actWin->drawGc.restoreFg();

  return 1;

}

int activeButtonClass::drawActive ( void ) {

int cV, rV, tX, tY;
XRectangle xR = { x, y, w, h };
char string[MAX_ENUM_STRING_SIZE+1];

  if ( !init || !activeMode || invisible ) return 1;

  cV = controlV;
  rV = readV;

  actWin->executeGc.saveFg();

  if ( controlExists && readExists ) {

    if ( ( cV != rV ) || !controlValid || !readValid ) {
      actWin->executeGc.setFG( inconsistentColor.getColor() );
    }
    else if ( cV == 0 ) {
      actWin->executeGc.setFG( offColor.getColor() );
    }
    else {
      actWin->executeGc.setFG( onColor.getColor() );
    }

  }
  else if ( readExists ) {

    cV = readV;

    if ( cV == 0 )
      actWin->executeGc.setFG( offColor.getColor() );
    else
      actWin->executeGc.setFG( onColor.getColor() );

  }
  else if ( controlExists ) {

    if ( cV == 0 )
      actWin->executeGc.setFG( offColor.getColor() );
    else
      actWin->executeGc.setFG( onColor.getColor() );

  }
  else if ( anyCallbackFlag ) {

    if ( cV != rV )
      actWin->executeGc.setFG( inconsistentColor.getColor() );
    else if ( cV == 0 )
      actWin->executeGc.setFG( offColor.getColor() );
    else
      actWin->executeGc.setFG( onColor.getColor() );

  }
  else {

    actWin->executeGc.setFG( inconsistentColor.getColor() );

  }

  XFillRectangle( actWin->d, XtWindow(actWin->executeWidget),
   actWin->executeGc.normGC(), x, y, w, h );

  XDrawRectangle( actWin->d, XtWindow(actWin->executeWidget),
   actWin->executeGc.normGC(), x, y, w, h );

  if ( cV == 0 ) {

    if ( labelType == BTC_K_LITERAL ) {
      strncpy( string, offLabel, MAX_ENUM_STRING_SIZE );
    }
    else {
      strncpy( string, stateString[0], MAX_ENUM_STRING_SIZE );
    }

    if ( _3D ) {

    actWin->executeGc.setFG( botShadowColor );

    XDrawLine( actWin->d, XtWindow(actWin->executeWidget),
     actWin->executeGc.normGC(), x, y, x+w, y );

    XDrawLine( actWin->d, XtWindow(actWin->executeWidget),
     actWin->executeGc.normGC(), x, y, x, y+h );

    actWin->executeGc.setFG( topShadowColor );

    XDrawLine( actWin->d, XtWindow(actWin->executeWidget),
     actWin->executeGc.normGC(), x, y+h, x+w, y+h );

    XDrawLine( actWin->d, XtWindow(actWin->executeWidget),
     actWin->executeGc.normGC(), x+w, y, x+w, y+h );

    // top
    actWin->executeGc.setFG( topShadowColor );

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
    actWin->executeGc.setFG( botShadowColor );

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

    if ( labelType == BTC_K_LITERAL ) {
      strncpy( string, onLabel, MAX_ENUM_STRING_SIZE );
    }
    else {
      strncpy( string, stateString[1], MAX_ENUM_STRING_SIZE );
    }

    if ( _3D ) {

    actWin->executeGc.setFG( botShadowColor );

    XDrawLine( actWin->d, XtWindow(actWin->executeWidget),
     actWin->executeGc.normGC(), x, y, x+w, y );

    XDrawLine( actWin->d, XtWindow(actWin->executeWidget),
     actWin->executeGc.normGC(), x, y, x, y+h );

    // top

    actWin->executeGc.setFG( topShadowColor );

//     XDrawLine( actWin->d, XtWindow(actWin->executeWidget),
//      actWin->executeGc.normGC(), x, y, x+w, y );

//      actWin->executeGc.setFG( botShadowColor );

//      XDrawLine( actWin->d, XtWindow(actWin->executeWidget),
//       actWin->executeGc.normGC(), x+1, y+1, x+w-1, y+1 );

    //left

//     actWin->executeGc.setFG( topShadowColor );

//     XDrawLine( actWin->d, XtWindow(actWin->executeWidget),
//      actWin->executeGc.normGC(), x, y, x, y+h );

//      actWin->executeGc.setFG( botShadowColor );

//      XDrawLine( actWin->d, XtWindow(actWin->executeWidget),
//       actWin->executeGc.normGC(), x+1, y+1, x+1, y+h-1 );

    // bottom

    actWin->executeGc.setFG( topShadowColor );

    XDrawLine( actWin->d, XtWindow(actWin->executeWidget),
     actWin->executeGc.normGC(), x, y+h, x+w, y+h );

    //right

    actWin->executeGc.setFG( topShadowColor );

    XDrawLine( actWin->d, XtWindow(actWin->executeWidget),
     actWin->executeGc.normGC(), x+w, y, x+w, y+h );

    }

  }

  if ( fs ) {

    actWin->executeGc.addNormXClipRectangle( xR );

    actWin->executeGc.setFG( fgColor.getColor() );
    actWin->executeGc.setFontTag( fontTag, actWin->fi );

    tX = x + w/2;
    tY = y + h/2 - fontAscent/2;

    if ( labelType == BTC_K_LITERAL ) {

      drawText( actWin->executeWidget, &actWin->executeGc, fs, tX, tY,
       XmALIGNMENT_CENTER, string );

    }
    else {

      drawText( actWin->executeWidget, &actWin->executeGc, fs, tX, tY,
       XmALIGNMENT_CENTER, string );

    }

    actWin->executeGc.removeNormXClipRectangle();

  }

  actWin->executeGc.restoreFg();

  return 1;

}

int activeButtonClass::activate (
  int pass,
  void *ptr )
{

int stat, opStat;
char callbackName[63+1];

  switch ( pass ) {

  case 1:

    aglPtr = ptr;
    needCtlConnectInit = needCtlInfoInit = needCtlRefresh =
     needReadConnectInit = needReadInfoInit = needReadRefresh =
     needErase = needDraw = 0;
    init = 0;
    opComplete = 0;
    controlValid = 0;
    readValid = 0;
    controlV = 0;

    controlEventId = readEventId = alarmEventId = NULL;

    controlPvConnected = readPvConnected = active = 0;
    activeMode = 1;

    if ( !controlPvName.getExpanded() ||
       ( strcmp( controlPvName.getExpanded(), "" ) == 0 ) ) {
      controlExists = 0;
    }
    else {
      controlExists = 1;
    }

    if ( !readPvName.getExpanded() ||
       ( strcmp( readPvName.getExpanded(), "" ) == 0 ) ) {
      readExists = 0;
    }
    else {
      readExists = 1;
    }

    break;

  case 2:

    if ( !opComplete ) {

      if ( anyCallbackFlag ) {

        if ( downCallbackFlag ) {
          strncpy( callbackName, id, 63 );
          Strncat( callbackName, "Down", 63 );
          downCallback =
           actWin->appCtx->userLibObject.getFunc( callbackName );
	}

        if ( upCallbackFlag ) {
          strncpy( callbackName, id, 63 );
          Strncat( callbackName, "Up", 63 );
          upCallback =
           actWin->appCtx->userLibObject.getFunc( callbackName );
	}

        if ( activateCallbackFlag ) {
          strncpy( callbackName, id, 63 );
          Strncat( callbackName, "Activate", 63 );
          activateCallback =
           actWin->appCtx->userLibObject.getFunc( callbackName );
	}

        if ( deactivateCallbackFlag ) {
          strncpy( callbackName, id, 63 );
          Strncat( callbackName, "Deactivate", 63 );
          deactivateCallback =
           actWin->appCtx->userLibObject.getFunc( callbackName );
	}

        if ( activateCallback ) {
          (*activateCallback)( this );
        }

      }

      opStat = 1;

      if ( controlExists ) {

        controlPvId = actWin->pvObj.createNew( pvClassName );
        if ( !controlPvId ) {
          printf( "Cannot create %s object", pvClassName );
          // actWin->appCtx->postMessage( msg );
          opComplete = 1;
          return 1;
	}

        controlPvId->createEventId( &controlEventId );
	controlPvId->createEventId( &alarmEventId );

        stat = controlPvId->searchAndConnect( &controlPvName,
         bt_monitor_control_connect_state, this );
        if ( stat != PV_E_SUCCESS ) {
          printf( "error from searchAndConnect\n" );
          return 0;
        }

      }

      if ( readExists ) {

        readPvId = actWin->pvObj.createNew( pvClassName );
        if ( !readPvId ) {
          printf( "Cannot create %s object", pvClassName );
          // actWin->appCtx->postMessage( msg );
          opComplete = 1;
          return 1;
	}

        readPvId->createEventId( &readEventId );

        stat = readPvId->searchAndConnect( &readPvName,
         bt_monitor_read_connect_state, this );
        if ( stat != PV_E_SUCCESS ) {
          printf( "error from searchAndConnect\n" );
          return 0;
        }

      }

      if ( !( opStat & 1 ) ) opComplete = 1;

      if ( !controlExists && !readExists ) {
        init = 1;
        active = 1;
        onColor.setConnected();
        offColor.setConnected();
        inconsistentColor.setConnected();
        controlV = readV = 0;
      }

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

int activeButtonClass::deactivate (
  int pass
) {

int stat;

  active = 0;
  activeMode = 0;

  if ( pass == 1 ) {

    if ( deactivateCallback ) {
      (*deactivateCallback)( this );
    }

    actWin->appCtx->proc->lock();

    // controlEventId = 0;
    // readEventId = 0;
    // alarmEventId = 0;

    if ( controlExists ) {

      stat = controlPvId->clearChannel();
      if ( stat != PV_E_SUCCESS )
        printf( "clearChannel failure\n" );

      stat = controlPvId->destroyEventId( &controlEventId );
      stat = controlPvId->destroyEventId( &alarmEventId );

      delete controlPvId;
      
      controlPvId = NULL;

    }

    if ( readExists ) {

      stat = readPvId->clearChannel();
      if ( stat != PV_E_SUCCESS )
        printf( "clearChannel failure\n" );

      stat = readPvId->destroyEventId( &readEventId );

      delete readPvId;
      
      readPvId = NULL;

    }
    
    actWin->appCtx->proc->unlock();

  }

  return 1;

}

void activeButtonClass::updateDimensions ( void )
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

void activeButtonClass::btnUp (
  int x,
  int y,
  int buttonState,
  int buttonNumber )
{

short value;
int stat;
int genericEnumType = controlPvId->pvrEnum();

  if ( toggle ) return;

  value = 0;

  if ( !controlExists ) controlV = 0;

  if ( upCallback ) {
    (*upCallback)( this );
  }

  if ( !controlExists ) return;
  stat = controlPvId->put( genericEnumType, &value );

}

void activeButtonClass::btnDown (
  int x,
  int y,
  int buttonState,
  int buttonNumber )
{

short value;
int stat;
int genericEnumType = controlPvId->pvrEnum();

  if ( toggle ) {
    if ( controlV == 0 ) {
      value = 1;
      if ( !controlExists ) controlV = 1;
      if ( downCallback ) {
        (*downCallback)( this );
      }
    }
    else {
      value = 0;
      if ( !controlExists ) controlV = 0;
      if ( upCallback ) {
        (*upCallback)( this );
      }
    }
  }
  else {
    value = 1;
    if ( !controlExists ) controlV = 1;
    if ( downCallback ) {
      (*downCallback)( this );
    }
  }

  if ( !controlExists ) return;
  stat = controlPvId->put( genericEnumType, &value );

}

int activeButtonClass::getButtonActionRequest (
  int *up,
  int *down,
  int *drag,
  int *focus )
{

  *drag = 0;

  *up = 1;
  *down = 1;

  if ( controlExists )
    *focus = 1;
  else
    *focus = 0;

  return 1;

}

int activeButtonClass::expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int stat;

  stat = controlPvName.expand1st( numMacros, macros, expansions );

  stat = readPvName.expand1st( numMacros, macros, expansions );

  return stat;

}

int activeButtonClass::expand2nd (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int stat;

  stat = controlPvName.expand2nd( numMacros, macros, expansions );

  stat = readPvName.expand2nd( numMacros, macros, expansions );

  return stat;

}

int activeButtonClass::containsMacros ( void ) {

int result;

  result = controlPvName.containsPrimaryMacros();

  if ( result ) return result;

  result = readPvName.containsPrimaryMacros();

  return result;

}

void activeButtonClass::executeDeferred ( void ) {

int stat, ncc, nci, ncr, nrc, nri, nrr, ne, nd;
short rv, cv;
char msg[79+1];

  if ( actWin->isIconified ) return;
  
  actWin->appCtx->proc->lock();

  if ( !activeMode ) {
    actWin->appCtx->proc->unlock();
    return;
  }

  int genericEnumType = controlPvId->pvrEnum();
  ncc = needCtlConnectInit; needCtlConnectInit = 0;
  nci = needCtlInfoInit; needCtlInfoInit = 0;
  ncr = needCtlRefresh; needCtlRefresh = 0;
  nrc = needReadConnectInit; needReadConnectInit = 0;
  nri = needReadInfoInit; needReadInfoInit = 0;
  nrr = needReadRefresh; needReadRefresh = 0;
  ne = needErase; needErase = 0;
  nd = needDraw; needDraw = 0;
  rv = curReadV;
  cv = curControlV;
  actWin->remDefExeNode( aglPtr );
  actWin->appCtx->proc->unlock();

  if ( ncc ) {
    
    if ( controlPvId->getType() != genericEnumType ) {
      strncpy( msg, actWin->obj.getNameFromClass( "activeButtonClass" ),
       79 );
      Strncat( msg, " - illegal pv type", 79 );
      actWin->appCtx->postMessage( msg );
      controlPvConnected = 0;
      active = 0;
      return;
    }

      stat = controlPvId->getCallback( controlPvId->pvrGrEnum(),
       bt_controlInfoUpdate, (void *) this );
      if ( stat != PV_E_SUCCESS ) {
        printf( "getCallback failed\n" );
      }

  }

  if ( nci ) {

    if ( !controlEventId->eventAdded() ) {
      stat = controlPvId->addEvent( controlPvId->pvrEnum(), 1,
       bt_controlUpdate, (void *) this, controlEventId, controlPvId->pveValue() );
      if ( stat != PV_E_SUCCESS ) {
        printf( "addEvent failed\n" );
      }
    }
    if ( !(readExists) ) {
      
      if ( !alarmEventId->eventAdded() ) {
        stat = controlPvId->addEvent( controlPvId->pvrStsEnum(), 1,
         bt_alarmUpdate, (void *) this, alarmEventId, controlPvId->pveAlarm() );
        if ( stat != PV_E_SUCCESS ) {
          printf( "addEvent failed\n" );
        }
      }
    
    }

    controlPvConnected = 1;

    if ( readPvConnected || !readExists ) {
      onColor.setConnected();
      offColor.setConnected();
      inconsistentColor.setConnected();
      init = 1;
      active = 1;
      eraseActive();
      readV = rv;
      controlV = cv;
      drawActive();
    }

  }

  if ( ncr ) {

    eraseActive();
    readV = rv; 
    controlV = cv;
    drawActive();

  }

  if ( nrc ) {

    if ( readPvId->getType() != genericEnumType ) {
      strncpy( msg, actWin->obj.getNameFromClass( "activeButtonClass" ),
       79 );
      Strncat( msg, " - illegal pv type", 79 );
      actWin->appCtx->postMessage( msg );
      readPvConnected = 0;
      active = 0;
      return;
    }

    stat = readPvId->getCallback( readPvId->pvrGrEnum(),
     bt_readInfoUpdate, (void *) this );
    if ( stat != PV_E_SUCCESS ) {
      printf( "getCallback failed\n" );
    }

  }

  if ( nri ) {

    if ( !readEventId->eventAdded() ) {

      stat = readPvId->addEvent( readPvId->pvrStsEnum(), 1,
       bt_readUpdate, (void *) this, readEventId, readPvId->pveValue() );
      if ( stat != PV_E_SUCCESS ) {
        printf( "addEvent failed\n" );
      }

    }

    if ( !alarmEventId->eventAdded() ) {
      stat = readPvId->addEvent( readPvId->pvrStsEnum(), 1,
       bt_alarmUpdate, (void *) this, alarmEventId, readPvId->pveAlarm() );
      if ( stat != PV_E_SUCCESS ) {
        printf( "addEvent failed\n" );
      }
    }

    readPvConnected = 1;

    if ( controlPvConnected || !controlExists ) {
      onColor.setConnected();
      offColor.setConnected();
      inconsistentColor.setConnected();
      init = 1;
      active = 1;
      eraseActive();
      controlV = cv;
      readV = rv;
      drawActive();
    }

  }

  if ( nrr ) {

    eraseActive();
    controlV = cv;
    readV = rv;
    drawActive();

  }

  if ( ne ) {

    eraseActive();

  }

  if ( nd ) {

    drawActive();

  }

}

int activeButtonClass::setProperty (
  char *prop,
  int *value )
{

  if ( strcmp( prop, "controlValue" ) == 0 ) {

    curControlV = (short) *value;
    needCtlRefresh = 1;
    actWin->appCtx->proc->lock();
    actWin->addDefExeNode( aglPtr );
    actWin->appCtx->proc->unlock();
    return 1;

  }
  else if ( strcmp( prop, "readValue" ) == 0 ) {

    curReadV = (short) *value;
    needReadRefresh = 1;
    actWin->appCtx->proc->lock();
    actWin->addDefExeNode( aglPtr );
    actWin->appCtx->proc->unlock();
    return 1;

  }

  return 0;

}

#ifdef __cplusplus
extern "C" {
#endif

void *create_activeButtonClassPtr ( void ) {

activeButtonClass *ptr;

  ptr = new activeButtonClass;
  return (void *) ptr;

}

void *clone_activeButtonClassPtr (
  void *_srcPtr )
{

activeButtonClass *ptr, *srcPtr;

  srcPtr = (activeButtonClass *) _srcPtr;

  ptr = new activeButtonClass( srcPtr );

  return (void *) ptr;

}

#ifdef __cplusplus
}
#endif
