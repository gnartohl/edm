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

#define __menu_mux_gen_cc 1

#include "menu_mux_gen.h"
#include "app_pkg.h"
#include "act_win.h"

#include "thread.h"

static void mmuxSetItem (
  Widget w,
  XtPointer client,
  XtPointer call )
{

efSetItemCallbackDscPtr dsc = (efSetItemCallbackDscPtr) client;
entryFormClass *ef = (entryFormClass *) dsc->ef;
menuMuxClass *mmo = (menuMuxClass *) dsc->obj;
int i;

  mmo->elbt->setValue( mmo->bufTag[ef->index] );

  for ( i=0; i<MMUX_MAX_ENTRIES; i++ ) {
    mmo->elbm[i]->setValue( mmo->bufM[ef->index][i] );
    mmo->elbe[i]->setValue( mmo->bufE[ef->index][i] );
  }

}

static void mmux_putValueNoPv (
  Widget w,
  XtPointer client,
  XtPointer call )
{

menuMuxClass *mmuxo = (menuMuxClass *) client;
int i;

  if ( !mmuxo->active ) return;

  for ( i=0; i<mmuxo->numStates; i++ ) {

    if ( w == mmuxo->pb[i] ) {

      mmuxo->actWin->appCtx->proc->lock();

      mmuxo->curControlV = i;

      if ( mmuxo->curControlV < 0 )
        mmuxo->curControlV = 0;
      else if ( mmuxo->curControlV >= mmuxo->numStates )
        mmuxo->curControlV = mmuxo->numStates - 1;

      mmuxo->needUpdate = 1;

      mmuxo->actWin->addDefExeNode( mmuxo->aglPtr );

      mmuxo->actWin->appCtx->proc->unlock();

    }

  }

}

static void mmux_putValue (
  Widget w,
  XtPointer client,
  XtPointer call )
{

menuMuxClass *mmuxo = (menuMuxClass *) client;
int i, stat, value;

  mmuxo->actWin->appCtx->proc->lock();

  if ( !mmuxo->activeMode ) {
    mmuxo->actWin->appCtx->proc->unlock();
    return;
  }

  for ( i=0; i<mmuxo->numStates; i++ ) {

    if ( w == mmuxo->pb[i] ) {
      value = i;
      if (!mmuxo->connectOnly) {
        stat = mmuxo->controlPvId->put( mmuxo->controlPvId->pvrLong(), &value );
      }
      mmuxo->actWin->appCtx->proc->unlock();
      return;
    }

  }

  mmuxo->actWin->appCtx->proc->unlock();

}

static void mmux_monitor_control_connect_state (
  pvClass *classPtr,
  void *clientData,
  void *args )
{

menuMuxClass *mmuxo = (menuMuxClass *) clientData;

  mmuxo->actWin->appCtx->proc->lock();

  if ( !mmuxo->activeMode ) {
    mmuxo->actWin->appCtx->proc->unlock();
    return;
  }

  if ( classPtr->getOp( args ) == classPtr->pvkOpConnUp() ) {

    mmuxo->needConnectInit = 1;

  }
  else {

    mmuxo->needDisconnect = 1;

  }

  mmuxo->actWin->addDefExeNode( mmuxo->aglPtr );

  mmuxo->actWin->appCtx->proc->unlock();

}

static void mmux_infoUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args )
{

// struct dbr_gr_long longRec;
menuMuxClass *mmuxo = (menuMuxClass *) clientData;

//  longRec = *( (struct dbr_gr_long *) ast_args.dbr );

  mmuxo->actWin->appCtx->proc->lock();

  if ( !mmuxo->activeMode ) {
    mmuxo->actWin->appCtx->proc->unlock();
    return;
  }

  mmuxo->curControlV = *( (long *) mmuxo->controlPvId->getValue( args ) );
  // mmuxo->curControlV = longRec.value;

  mmuxo->needInfoInit = 1;

  mmuxo->actWin->addDefExeNode( mmuxo->aglPtr );

  mmuxo->actWin->appCtx->proc->unlock();


}

static void mmux_controlUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args )
{

menuMuxClass *mmuxo = (menuMuxClass *) clientData;

  mmuxo->actWin->appCtx->proc->lock();

  if ( !mmuxo->activeMode ) {
    mmuxo->actWin->appCtx->proc->unlock();
    return;
  }

  mmuxo->needUpdate = 1;

  mmuxo->curControlV = *( (int *) mmuxo->controlPvId->getValue( args ) );
  if ( mmuxo->curControlV < 0 )
    mmuxo->curControlV = 0;
  else if ( mmuxo->curControlV >= mmuxo->numStates )
    mmuxo->curControlV = mmuxo->numStates - 1;

  mmuxo->actWin->addDefExeNode( mmuxo->aglPtr );

  mmuxo->actWin->appCtx->proc->unlock();

}

static void mmux_alarmUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args )
{

menuMuxClass *mmuxo = (menuMuxClass *) clientData;

  mmuxo->actWin->appCtx->proc->lock();

  if ( !mmuxo->activeMode ) {
    mmuxo->actWin->appCtx->proc->unlock();
    return;
  }

  mmuxo->fgColor.setStatus( classPtr->getStatus( args ),
   classPtr->getSeverity( args ) );
  mmuxo->bgColor.setStatus( classPtr->getStatus( args ),
   classPtr->getSeverity( args ) );

  mmuxo->needDraw = 1;

  mmuxo->actWin->addDefExeNode( mmuxo->aglPtr );

  mmuxo->actWin->appCtx->proc->unlock();

}

static void mmuxc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call )
{

menuMuxClass *mmuxo = (menuMuxClass *) client;
int i, ii;

  mmuxo->actWin->setChanged();

  mmuxo->eraseSelectBoxCorners();
  mmuxo->erase();

  strncpy( mmuxo->fontTag, mmuxo->fm.currentFontTag(), 63+1 );
  mmuxo->actWin->fi->loadFontTag( mmuxo->fontTag );
  mmuxo->actWin->drawGc.setFontTag( mmuxo->fontTag, mmuxo->actWin->fi );
  mmuxo->actWin->fi->getTextFontList( mmuxo->fontTag, &mmuxo->fontList );
  mmuxo->fs = mmuxo->actWin->fi->getXFontStruct( mmuxo->fontTag );

  mmuxo->topShadowColor = mmuxo->bufTopShadowColor;
  mmuxo->botShadowColor = mmuxo->bufBotShadowColor;

  mmuxo->fgColorMode = mmuxo->bufFgColorMode;
  if ( mmuxo->fgColorMode == MMUXC_K_COLORMODE_ALARM )
    mmuxo->fgColor.setAlarmSensitive();
  else
    mmuxo->fgColor.setAlarmInsensitive();
  mmuxo->fgColor.setColor( mmuxo->bufFgColor, mmuxo->actWin->ci );

  mmuxo->bgColorMode = mmuxo->bufBgColorMode;
  if ( mmuxo->bgColorMode == MMUXC_K_COLORMODE_ALARM )
    mmuxo->bgColor.setAlarmSensitive();
  else
    mmuxo->bgColor.setAlarmInsensitive();
  mmuxo->bgColor.setColor( mmuxo->bufBgColor, mmuxo->actWin->ci );

  mmuxo->x = mmuxo->bufX;
  mmuxo->sboxX = mmuxo->bufX;

  mmuxo->y = mmuxo->bufY;
  mmuxo->sboxY = mmuxo->bufY;

  mmuxo->w = mmuxo->bufW;
  mmuxo->sboxW = mmuxo->bufW;

  mmuxo->h = mmuxo->bufH;
  mmuxo->sboxH = mmuxo->bufH;

  mmuxo->controlPvExpStr.setRaw( mmuxo->bufControlPvName );

  mmuxo->connectOnly = mmuxo->bufConnectOnly;

  strncpy( mmuxo->pvUserClassName,
   mmuxo->actWin->pvObj.getPvName(mmuxo->pvNameIndex), 15 );

  strncpy( mmuxo->pvClassName,
   mmuxo->actWin->pvObj.getPvClassName(mmuxo->pvNameIndex), 15 );


  for ( i=0; i<MMUX_MAX_STATES; i++ ) {
    strncpy( mmuxo->tag[i], mmuxo->bufTag[i], MMUX_MAX_STRING_SIZE+1 );
    if ( strlen(mmuxo->tag[i]) == 0 ) {
      strcpy( mmuxo->tag[i], "?" );
    }
  }

  for ( i=0; i<MMUX_MAX_STATES; i++ ) {
    for ( ii=0; ii<MMUX_MAX_ENTRIES; ii++ ) {
      strncpy( mmuxo->m[i][ii], mmuxo->bufM[i][ii], MMUX_MAX_STRING_SIZE+1 );
      strncpy( mmuxo->e[i][ii], mmuxo->bufE[i][ii], MMUX_MAX_STRING_SIZE+1 );
    }
  }

  mmuxo->numItems = mmuxo->ef.numItems;

  mmuxo->updateDimensions();

}

static void mmuxc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call )
{

menuMuxClass *mmuxo = (menuMuxClass *) client;

  mmuxc_edit_update( w, client, call );
  mmuxo->refresh( mmuxo );


}

static void mmuxc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call )
{

menuMuxClass *mmuxo = (menuMuxClass *) client;

  mmuxc_edit_apply ( w, client, call );
  mmuxo->ef.popdown();
  mmuxo->operationComplete();

}

static void mmuxc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call )
{

menuMuxClass *mmuxo = (menuMuxClass *) client;

  mmuxo->ef.popdown();
  mmuxo->operationCancel();

}

static void mmuxc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call )
{

menuMuxClass *mmuxo = (menuMuxClass *) client;

  mmuxo->ef.popdown();
  mmuxo->operationCancel();
  mmuxo->erase();
  mmuxo->deleteRequest = 1;
  mmuxo->drawAll();

}

menuMuxClass::menuMuxClass ( void ) {

int i, ii;

  name = new char[strlen("menuMuxClass")+1];
  strcpy( name, "menuMuxClass" );

  numStates = 0;

  for ( i=0; i<MMUX_MAX_STATES; i++ ) {
    stateString[i] = NULL;
    pb[i] = NULL;
  }

  for ( i=0; i<MMUX_MAX_STATES; i++ ) {
    strcpy( tag[i], "" );
  }
  for ( i=0; i<MMUX_MAX_STATES; i++ ) {
    for ( ii=0; ii<MMUX_MAX_ENTRIES; ii++ ) {
      strcpy( m[i][ii], "" );
      strcpy( e[i][ii], "" );
    }
  }

  numItems = 2;
  numMac = 0;
  mac = NULL;
  exp = NULL;

  fgColorMode = MMUXC_K_COLORMODE_STATIC;
  bgColorMode = MMUXC_K_COLORMODE_STATIC;

  strcpy( pvClassName, "" );
  strcpy( pvUserClassName, "" );

  connectOnly = 0;

  active = 0;
  activeMode = 0;
  widgetsCreated = 0;
  fontList = NULL;

}

// copy constructor
menuMuxClass::menuMuxClass
 ( const menuMuxClass *source ) {

int i, ii;
activeGraphicClass *mmuxo = (activeGraphicClass *) this;

  mmuxo->clone( (activeGraphicClass *) source );

  name = new char[strlen("menuMuxClass")+1];
  strcpy( name, "menuMuxClass" );

  numItems = source->numItems;

  for ( i=0; i<MMUX_MAX_STATES; i++ ) {
    stateString[i] = NULL;
    pb[i] = NULL;
  }

  for ( i=0; i<MMUX_MAX_STATES; i++ ) {
    strncpy( tag[i], source->tag[i], MMUX_MAX_STRING_SIZE+1 );
  }
  for ( i=0; i<MMUX_MAX_STATES; i++ ) {
    for ( ii=0; ii<MMUX_MAX_ENTRIES; ii++ ) {
      strncpy( m[i][ii], source->m[i][ii], MMUX_MAX_STRING_SIZE+1 );
      strncpy( e[i][ii], source->e[i][ii], MMUX_MAX_STRING_SIZE+1 );
    }
  }

  numMac = 0;
  mac = NULL;
  exp = NULL;

  strncpy( fontTag, source->fontTag, 63+1 );
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
  connectOnly = source->connectOnly;

  strncpy( pvClassName, source->pvClassName, 15 );
  strncpy( pvUserClassName, source->pvUserClassName, 15 );

  widgetsCreated = 0;
  active = 0;
  activeMode = 0;

}

menuMuxClass::~menuMuxClass ( void ) {

int i;

  if ( name ) delete name;

  if ( mac && exp ) {
    for ( i=0; i<numMac; i++ ) {
      if ( mac[i] ) {
	delete mac[i];
      }
      if ( exp[i] ) {
	delete exp[i];
      }
    }
  }
  if ( mac ) delete mac;
  if ( exp ) delete exp;

  for ( i=0; i<MMUX_MAX_STATES; i++ ) {
    if ( stateString[i] ) delete stateString[i];
  }

  if ( fontList ) XmFontListFree( fontList );

}

int menuMuxClass::createInteractive (
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

  strncpy( fontTag, actWin->defaultFontTag, 63+1 );
  actWin->fi->loadFontTag( fontTag );
  fs = actWin->fi->getXFontStruct( fontTag );
  actWin->fi->getTextFontList( fontTag, &fontList );

  updateDimensions();

  topShadowColor = actWin->defaultTopShadowColor;
  botShadowColor = actWin->defaultBotShadowColor;

  fgColor.setColor( actWin->defaultTextFgColor, actWin->ci );
  bgColor.setColor( actWin->defaultBgColor, actWin->ci );

  strncpy( pvUserClassName, actWin->defaultPvType, 15 );

  this->draw();

  this->editCreate();

  return 1;

}

int menuMuxClass::save (
  FILE *f )
{

int r, g, b, i, ii;

  fprintf( f, "%-d %-d %-d\n", MMUXC_MAJOR_VERSION, MMUXC_MINOR_VERSION,
   MMUXC_RELEASE );

  fprintf( f, "%-d\n", x );
  fprintf( f, "%-d\n", y );
  fprintf( f, "%-d\n", w );
  fprintf( f, "%-d\n", h );

  actWin->ci->getRGB( fgColor.pixelColor(), &r, &g, &b );
  fprintf( f, "%-d %-d %-d\n", r, g, b );

  fprintf( f, "%-d\n", fgColorMode );

  actWin->ci->getRGB( bgColor.pixelColor(), &r, &g, &b );
  fprintf( f, "%-d %-d %-d\n", r, g, b );

  fprintf( f, "%-d\n", bgColorMode );

  actWin->ci->getRGB( topShadowColor, &r, &g, &b );
  fprintf( f, "%-d %-d %-d\n", r, g, b );

  actWin->ci->getRGB( botShadowColor, &r, &g, &b );
  fprintf( f, "%-d %-d %-d\n", r, g, b );

  if ( controlPvExpStr.getRaw() )
    writeStringToFile( f, controlPvExpStr.getRaw() );
  else
    writeStringToFile( f, "" );

  fprintf( f, "%-d\n", connectOnly );

  writeStringToFile( f, fontTag );

  fprintf( f, "%-d\n", numItems );

  for ( i=0; i<numItems; i++ ) {
    writeStringToFile( f, tag[i] );
  }

  for ( i=0; i<numItems; i++ ) {
    for ( ii=0; ii<MMUX_MAX_ENTRIES; ii++ ) {
      writeStringToFile( f, m[i][ii] );
      writeStringToFile( f, e[i][ii] );
    }
  }

  writeStringToFile( f, pvClassName );

  return 1;

}

int menuMuxClass::createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int r, g, b, i, ii;
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

  fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
  if ( ( major < 2 ) && ( minor < 1 ) ) {
    r *= 256;
    g *= 256;
    b *= 256;
  }
  actWin->ci->setRGB( r, g, b, &pixel );
  fgColor.setColor( pixel, actWin->ci );

  fscanf( f, "%d\n", &fgColorMode ); actWin->incLine();

  if ( fgColorMode == MMUXC_K_COLORMODE_ALARM )
    fgColor.setAlarmSensitive();
  else
    fgColor.setAlarmInsensitive();

  fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
  if ( ( major < 2 ) && ( minor < 1 ) ) {
    r *= 256;
    g *= 256;
    b *= 256;
  }
  actWin->ci->setRGB( r, g, b, &pixel );
  bgColor.setColor( pixel, actWin->ci );

  fscanf( f, "%d\n", &bgColorMode ); actWin->incLine();

  if ( bgColorMode == MMUXC_K_COLORMODE_ALARM )
    bgColor.setAlarmSensitive();
  else
    bgColor.setAlarmInsensitive();

  fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
  if ( ( major < 2 ) && ( minor < 1 ) ) {
    r *= 256;
    g *= 256;
    b *= 256;
  }
  actWin->ci->setRGB( r, g, b, &topShadowColor );

  fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
  if ( ( major < 2 ) && ( minor < 1 ) ) {
    r *= 256;
    g *= 256;
    b *= 256;
  }
  actWin->ci->setRGB( r, g, b, &botShadowColor );

  readStringFromFile( oneName, 39, f ); actWin->incLine();
  controlPvExpStr.setRaw( oneName );

  if ( ( major > 1 ) || ( minor > 2 ) ) {
    fscanf( f, "%d\n", &connectOnly ); actWin->incLine();
  }

  readStringFromFile( fontTag, 63, f ); actWin->incLine();
  
  actWin->fi->loadFontTag( fontTag );
  actWin->drawGc.setFontTag( fontTag, actWin->fi );

  fs = actWin->fi->getXFontStruct( fontTag );
  actWin->fi->getTextFontList( fontTag, &fontList );

  updateDimensions();

  for ( i=0; i<MMUX_MAX_STATES; i++ ) {
    strcpy( tag[i], "" );
  }
  for ( i=0; i<MMUX_MAX_STATES; i++ ) {
    for ( ii=0; ii<MMUX_MAX_ENTRIES; ii++ ) {
      strcpy( m[i][ii], "" );
      strcpy( e[i][ii], "" );
    }
  }

  fscanf( f, "%d\n", &numItems ); actWin->incLine();

  for ( i=0; i<numItems; i++ ) {
    readStringFromFile( tag[i], MMUX_MAX_STRING_SIZE, f ); actWin->incLine();
  }

  for ( i=0; i<numItems; i++ ) {
    for ( ii=0; ii<MMUX_MAX_ENTRIES; ii++ ) {
      readStringFromFile( m[i][ii], MMUX_MAX_STRING_SIZE, f );
      actWin->incLine();
      readStringFromFile( e[i][ii], MMUX_MAX_STRING_SIZE, f );
      actWin->incLine();
    }
  }

  numMac = 0;
  mac = NULL;
  exp = NULL;

  if ( ( major > 1 ) || ( minor > 1 ) ) {

    readStringFromFile( pvClassName, 15, f ); actWin->incLine();

    strncpy( pvUserClassName, actWin->pvObj.getNameFromClass( pvClassName ),
     15 );

  }

  return 1;

}

int menuMuxClass::genericEdit ( void ) {

int i, ii;
char title[32], *ptr;

  ptr = actWin->obj.getNameFromClass( "menuMuxClass" );
  if ( ptr )
    strncpy( title, ptr, 31+1 );
  else
    strncpy( title, "Unknown object", 31+1 );

  strncat( title, " Properties", 31+1 );

  bufX = x;
  bufY = y;
  bufW = w;
  bufH = h;

  strncpy( bufFontTag, fontTag, 63+1 );

  bufTopShadowColor = topShadowColor;
  bufBotShadowColor = botShadowColor;

  bufFgColor = fgColor.pixelColor();
  bufFgColorMode = fgColorMode;

  bufBgColor = bgColor.pixelColor();
  bufBgColorMode = bgColorMode;

  if ( controlPvExpStr.getRaw() )
    strncpy( bufControlPvName, controlPvExpStr.getRaw(), 39+1 );
  else
    strncpy( bufControlPvName, "", 39+1 );

  bufConnectOnly = connectOnly;

  for ( i=0; i<MMUX_MAX_STATES; i++ ) {
    strncpy( bufTag[i], tag[i], MMUX_MAX_STRING_SIZE+1 );
  }
  for ( i=0; i<MMUX_MAX_STATES; i++ ) {
    for ( ii=0; ii<MMUX_MAX_ENTRIES; ii++ ) {
      strncpy( bufM[i][ii], m[i][ii], MMUX_MAX_STRING_SIZE+1 );
      strncpy( bufE[i][ii], e[i][ii], MMUX_MAX_STRING_SIZE+1 );
    }
  }

  ef.create( actWin->top, actWin->appCtx->ci.getColorMap(),
   &actWin->appCtx->entryFormX,
   &actWin->appCtx->entryFormY, &actWin->appCtx->entryFormW,
   &actWin->appCtx->entryFormH, &actWin->appCtx->largestH,
   title, MMUX_MAX_STATES, numItems,
   mmuxSetItem, (void *) this, NULL, NULL, NULL );

  ef.addTextField( "X", 27, &bufX );
  ef.addTextField( "Y", 27, &bufY );
  ef.addTextField( "Width", 27, &bufW );
  ef.addTextField( "Height", 27, &bufH );
  ef.addColorButton( "FG Color", actWin->ci, &fgCb, &bufFgColor );
  ef.addOption( "FG Color Mode", "Static|Alarm", &bufFgColorMode );
  ef.addColorButton( "BG Color", actWin->ci, &bgCb, &bufBgColor );
//   ef.addOption( "BG Color Mode", "Static|Alarm", &bufBgColorMode );
  ef.addColorButton( "Top Shadow", actWin->ci, &topShadowCb,
   &bufTopShadowColor );
  ef.addColorButton( "Bottom Shadow", actWin->ci, &botShadowCb,
   &bufBotShadowColor );
  ef.addFontMenu( "Font", actWin->fi, &fm, fontTag );
  ef.addTextField( "PV Name", 27, bufControlPvName, 39 );
  ef.addToggle( "Connect Only", &bufConnectOnly );

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

  for ( i=0; i<MMUX_MAX_STATES; i++ ) {
    tagPtr[i] = (char *) &bufTag[i];
  }

  for ( i=0; i<MMUX_MAX_STATES; i++ ) {
    for ( ii=0; ii<MMUX_MAX_ENTRIES; ii++ ) {
      mPtr[ii][i] = (char *) &bufM[i][ii];
      ePtr[ii][i] = (char *) &bufE[i][ii];
    }
  }

  ef.addTextFieldArray( "Tag", 25, tagPtr, MMUX_MAX_STRING_SIZE, &elbt );

  for ( i=0; i<MMUX_MAX_ENTRIES; i++ ) {
    ef.addTextFieldArray( "Symbol", 25, mPtr[i], MMUX_MAX_STRING_SIZE,
     &elbm[i] );
    ef.addTextFieldArray( "Value", 25, ePtr[i], MMUX_MAX_STRING_SIZE,
     &elbe[i] );
  }

  return 1;

}

int menuMuxClass::editCreate ( void ) {

  this->genericEdit();
  ef.finished( mmuxc_edit_ok, mmuxc_edit_apply, mmuxc_edit_cancel_delete,
   this );
  actWin->currentEf = NULL;
  ef.popup();

  return 1;

}

int menuMuxClass::edit ( void ) {

  this->genericEdit();
  ef.finished( mmuxc_edit_ok, mmuxc_edit_apply, mmuxc_edit_cancel, this );
  actWin->currentEf = &ef;
  ef.popup();

  return 1;

}

int menuMuxClass::erase ( void ) {

  if ( deleteRequest || activeMode ) return 1;

  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  return 1;

}

int menuMuxClass::eraseActive ( void ) {

  return 1;

}

int menuMuxClass::draw ( void ) {

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
	      XmALIGNMENT_CENTER, "Mux" );

    actWin->drawGc.removeNormXClipRectangle();

  }

  actWin->drawGc.restoreFg();

  return 1;

}

int menuMuxClass::drawActive ( void ) {

Arg args[10];
int n;
// Widget w;

  if ( !activeMode || !widgetsCreated ) return 1;

  // puts("Draw MUX\n");

  // set colors

//   w = XmOptionButtonGadget( optionMenu );
//   n = 0;
//   XtSetArg( args[n], XmNbackground, bgColor.getColor() ); n++;
//   XtSetArg( args[n], XmNforeground, fgColor.getColor() ); n++;
//   XtSetValues( w, args, n );

  n = 0;
  XtSetArg( args[n], XmNforeground, fgColor.getColor() ); n++;
  XtSetValues( optionMenu, args, n );

  // puts("Draw Mux Out\n");

  return 1;

}

int menuMuxClass::expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int stat;

  stat = controlPvExpStr.expand1st( numMacros, macros, expansions );

  return stat;

}

int menuMuxClass::getMacros (
  int *numMacros,
  char ***macro,
  char ***expansion ) {

int i, ii, n, count;

  if ( controlV < 0 )
    n = 0;
  else if ( controlV >= numItems )
    n = numItems - 1;
  else
    n = controlV;

// count number of non-null entries
  count = 0;
  for ( i=0; i<MMUX_MAX_ENTRIES; i++ ) {
    if ( ( strcmp( m[n][i], "" ) != 0 ) &&
         ( strcmp( e[n][i], "" ) != 0 ) ) {
      count++;
    }
  }

  if ( numMac < count ) {

    for ( i=0; i<numMac; i++ ) {
      if ( mac[i] ) {
        delete mac[i];
        mac[i] = NULL;
      }
      if ( exp[i] ) {
        delete exp[i];
        exp[i] = NULL;
      }
    }
    if ( mac ) {
      delete mac;
      mac = NULL;
    }
    if ( exp ) {
      delete exp;
      exp = NULL;
    }

    numMac = count;

    mac = new char*[numMac];
    exp = new char*[numMac];

    for ( i=0; i<numMac; i++ ) {
      mac[i] = new char[MMUX_MAX_STRING_SIZE+1];
      exp[i] = new char[MMUX_MAX_STRING_SIZE+1];
    }

  }

  // populate ptr arrays
  ii = 0;
  for ( i=0; i<MMUX_MAX_ENTRIES; i++ ) {
    if ( ( strcmp( m[n][i], "" ) != 0 ) &&
         ( strcmp( e[n][i], "" ) != 0 ) ) {
      strncpy( mac[ii], m[n][i], MMUX_MAX_STRING_SIZE+1 );
      strncpy( exp[ii], e[n][i], MMUX_MAX_STRING_SIZE+1 );
      ii++;
    }
  }

  *numMacros = count;
  *macro = mac;
  *expansion = exp;

  return 1;

}

int menuMuxClass::activate (
  int pass,
  void *ptr )
{

int stat, opStat;

  switch ( pass ) {

  case 1:

    aglPtr = ptr;
    needConnectInit = needDisconnect = needInfoInit = needUpdate =
     needDraw = 0;
    widgetsCreated = 0;
    opComplete = 0;
    firstEvent = 1;

    alarmEventId = NULL;
    controlEventId = NULL;

    controlPvConnected = active = 0;

    actWin->appCtx->proc->lock();
    activeMode = 1;
    actWin->appCtx->proc->unlock();

    optionMenu = (Widget) NULL;

    if ( strcmp( controlPvExpStr.getRaw(), "" ) != 0 )
      controlExists = 1;
    else
      controlExists = 0;

    break;

  case 2:

    if ( !opComplete ) {

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

        stat = controlPvId->searchAndConnect( &controlPvExpStr,
         mmux_monitor_control_connect_state, this );
        if ( stat != PV_E_SUCCESS ) {
          printf( "error from searchAndConnect\n" );
          return 0;
        }

      }
      else {

        actWin->appCtx->proc->lock();
        curControlV = 0;
        needInfoInit = 1;
        actWin->addDefExeNode( aglPtr );
        actWin->appCtx->proc->unlock();

      }


      if ( !( opStat & 1 ) ) opComplete = 1;

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

int menuMuxClass::deactivate (
  int pass
) {

int stat;

  active = 0;

  if ( pass == 1 ) {

    actWin->appCtx->proc->lock();

    activeMode = 0;

    if ( controlExists ) {

      stat = controlPvId->clearChannel();
      if ( stat != PV_E_SUCCESS )
        printf( "clearChannel failure\n" );

      stat = controlPvId->destroyEventId ( &controlEventId );
      stat = controlPvId->destroyEventId ( &alarmEventId );

      delete controlPvId;

      controlPvId = NULL;

    }

    actWin->appCtx->proc->unlock();

  }
  else if ( pass == 2 ) {

    if ( widgetsCreated ) {
      if ( optionMenu ) {
        XtUnmapWidget( optionMenu );
        XtDestroyWidget( optionMenu );
        optionMenu = NULL;
      }
      if ( pulldownMenu ) {
        XtDestroyWidget( pulldownMenu );
        pulldownMenu = NULL;
      }
      widgetsCreated = 0;
    }

  }

  return 1;

}

void menuMuxClass::updateDimensions ( void )
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

void menuMuxClass::executeDeferred ( void ) {

int v;
int stat, i, nc, ndis, ni, nu, nd;
XmString str;
Arg args[10];
int n;

//----------------------------------------------------------------------------

  if ( actWin->isIconified ) return;

  actWin->appCtx->proc->lock();

  if ( !activeMode ) {
    actWin->remDefExeNode( aglPtr );
    actWin->appCtx->proc->unlock();
    return;
  }

  nc = needConnectInit; needConnectInit = 0;
  ndis = needDisconnect; needDisconnect = 0;
  ni = needInfoInit; needInfoInit = 0;
  nu = needUpdate; needUpdate = 0;
  nd = needDraw; needDraw = 0;
  v = curControlV;

  actWin->remDefExeNode( aglPtr );

  actWin->appCtx->proc->unlock();
  
  // printf("NC: %i NI: %i  NU: %i  ND: %i\n", nc, ni, nu, nd);
//----------------------------------------------------------------------------

  if ( nc ) {
  
    stat = controlPvId->getCallback( controlPvId->pvrGrLong(),
     mmux_infoUpdate, (void *) this );
    if ( stat != PV_E_SUCCESS ) {
      printf( "getCallback failed\n" );
    }

    controlPvConnected = 1;
    fgColor.setConnected();

  }

//----------------------------------------------------------------------------

  if ( ndis ) {

    controlPvConnected = 0;
    fgColor.setDisconnected();
    active = 0;

    if ( widgetsCreated ) {
      if ( optionMenu ) {
        XtUnmapWidget( optionMenu );
        XtDestroyWidget( optionMenu );
        optionMenu = NULL;
      }
      if ( pulldownMenu ) {
        XtDestroyWidget( pulldownMenu );
        pulldownMenu = NULL;
      }
      widgetsCreated = 0;
    }

  }

//----------------------------------------------------------------------------

  if ( ni ) {

    controlV = v;

    if ( widgetsCreated ) {
      if ( optionMenu ) {
        XtUnmapWidget( optionMenu );
        XtDestroyWidget( optionMenu );
        optionMenu = NULL;
      }
      if ( pulldownMenu ) {
        XtDestroyWidget( pulldownMenu );
        pulldownMenu = NULL;
      }
      widgetsCreated = 0;
    }

    pulldownMenu = XmCreatePulldownMenu( actWin->executeWidgetId(),
     "", NULL, 0 );

    numStates = numItems;

    for ( i=0; i<numStates; i++ ) {

      stateString[i] = new char[strlen(tag[i])+1];
      strncpy( stateString[i], tag[i], strlen(tag[i])+1 );

      str = XmStringCreate( stateString[i], fontTag );

      pb[i] = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
       pulldownMenu,
       XmNlabelString, str,
       XmNfontList, fontList,
       NULL );

      XmStringFree( str );

      n = 0;
      XtSetArg( args[n], XmNwidth, w-42 ); n++;
      XtSetArg( args[n], XmNheight, h-14 ); n++;
      XtSetValues( pb[i], args, n );


      if ( controlExists && !connectOnly ) {
        XtAddCallback( pb[i], XmNactivateCallback, mmux_putValue,
         (XtPointer) this );
      }
      else {
        XtAddCallback( pb[i], XmNactivateCallback, mmux_putValueNoPv,
         (XtPointer) this );
      }

    }

    if ( controlV < numStates ) {
      curHistoryWidget = pb[controlV];
    }
    else {
      curHistoryWidget = pb[0];
    }

    n = 0;
    XtSetArg( args[n], XmNsubMenuId, (XtArgVal) pulldownMenu ); n++;
    XtSetArg( args[n], XmNmenuHistory, (XtArgVal) curHistoryWidget );
     n++;
    XtSetArg( args[n], XmNx, (XtArgVal) x ); n++;
    XtSetArg( args[n], XmNy, (XtArgVal) y ); n++;
    XtSetArg( args[n], XmNbackground,
     (XtArgVal) actWin->executeGc.getBaseBG() ); n++;
    XtSetArg( args[n], XmNhighlightColor,
     (XtArgVal) actWin->executeGc.getBaseBG() ); n++;
    XtSetArg( args[n], XmNhighlightPixmap, (XtArgVal) None ); n++;
    XtSetArg( args[n], XmNtopShadowColor, (XtArgVal) topShadowColor );
     n++;
    XtSetArg( args[n], XmNbottomShadowColor, (XtArgVal) botShadowColor );
     n++;
    optionMenu = XmCreateOptionMenu( actWin->executeWidgetId(), "",
     args, n );

    XtManageChild( optionMenu );

    widgetsCreated = 1;

    active = 1;

    if ( controlExists && !connectOnly) {

      // pvrLong tells channel access to send the value of the pv
      // as a long to the event callback
      if ( !controlEventId->eventAdded() ) {
        stat = controlPvId->addEvent( controlPvId->pvrLong(), 1,
         mmux_controlUpdate, (void *) this, controlEventId,
         controlPvId->pveValue() );
        if ( stat != PV_E_SUCCESS ) {
          printf( "addEvent failed\n" );
        }
      }

      // pvrStsDouble tells channel access to send a structure which
      // includes the value of the pv as a double and all alarm information
      // to the event callback
      if ( !alarmEventId->eventAdded() ) {
        stat = controlPvId->addEvent( controlPvId->pvrStsLong(), 1,
         mmux_alarmUpdate, (void *) this, alarmEventId,
         controlPvId->pveAlarm() );
        if ( stat != PV_E_SUCCESS ) {
          printf( "addEvent failed\n" );
        }
      }

    }
    else {

      firstEvent = 0;

    }

    nu = 1;

  }

//----------------------------------------------------------------------------

  if ( nu ) {

    controlV = v;

    if ( ( controlV >= 0 ) && ( controlV < numStates ) ) {
      curHistoryWidget = pb[controlV];
      n = 0;
      XtSetArg( args[n], XmNmenuHistory, (XtArgVal) curHistoryWidget );
      n++;
      XtSetValues( optionMenu, args, n );
    }

    stat = drawActive();

    if ( !firstEvent ) {
      actWin->preReexecute();
      actWin->setNoRefresh();
      actWin->appCtx->reactivateActiveWindow( actWin );
    }
    firstEvent = 0;

  }

//----------------------------------------------------------------------------

  if ( nd ) {
    controlV = v;
    drawActive();
  }

//----------------------------------------------------------------------------
}

#ifdef __cplusplus
extern "C" {
#endif

void *create_menuMuxClassPtr ( void ) {

menuMuxClass *ptr;

  ptr = new menuMuxClass;
  return (void *) ptr;

}

void *clone_menuMuxClassPtr (
  void *_srcPtr )
{

menuMuxClass *ptr, *srcPtr;

  srcPtr = (menuMuxClass *) _srcPtr;

  ptr = new menuMuxClass( srcPtr );

  return (void *) ptr;

}

#ifdef __cplusplus
}
#endif
