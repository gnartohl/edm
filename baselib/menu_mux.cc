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

#define __menu_mux_cc 1

#include "menu_mux.h"
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

#ifdef __epics__

static void mmux_putValue (
  Widget w,
  XtPointer client,
  XtPointer call )
{

menuMuxClass *mmuxo = (menuMuxClass *) client;
int i, stat, value;

  for ( i=0; i<mmuxo->numStates; i++ ) {

    if ( w == mmuxo->pb[i] ) {
      value = i;
      stat = ca_put( DBR_LONG, mmuxo->controlPvId, &value );
      return;
    }

  }

}

static void mmux_monitor_control_connect_state (
  struct connection_handler_args arg )
{

menuMuxClass *mmuxo = (menuMuxClass *) ca_puser(arg.chid);

  if ( arg.op == CA_OP_CONN_UP ) {

    mmuxo->needConnectInit = 1;

  }
  else {

    mmuxo->needDisconnect = 1;

  }

  mmuxo->actWin->appCtx->proc->lock();
  mmuxo->actWin->addDefExeNode( mmuxo->aglPtr );
  mmuxo->actWin->appCtx->proc->unlock();

}

static void mmux_infoUpdate (
  struct event_handler_args ast_args )
{

struct dbr_gr_long longRec;
menuMuxClass *mmuxo = (menuMuxClass *) ast_args.usr;

  longRec = *( (struct dbr_gr_long *) ast_args.dbr );

  mmuxo->needInfoInit = 1;

  mmuxo->actWin->appCtx->proc->lock();

  mmuxo->curControlV = longRec.value;
  mmuxo->actWin->addDefExeNode( mmuxo->aglPtr );

  mmuxo->actWin->appCtx->proc->unlock();


}

static void mmux_controlUpdate (
  struct event_handler_args ast_args )
{

menuMuxClass *mmuxo = (menuMuxClass *) ast_args.usr;

  if ( !mmuxo->active ) return;

  mmuxo->needUpdate = 1;

  mmuxo->actWin->appCtx->proc->lock();

  mmuxo->curControlV = *( (int *) ast_args.dbr );
  if ( mmuxo->curControlV < 0 )
    mmuxo->curControlV = 0;
  else if ( mmuxo->curControlV >= mmuxo->numStates )
    mmuxo->curControlV = mmuxo->numStates - 1;

  mmuxo->actWin->addDefExeNode( mmuxo->aglPtr );

  mmuxo->actWin->appCtx->proc->unlock();

}

static void mmux_alarmUpdate (
  struct event_handler_args ast_args )
{

menuMuxClass *mmuxo = (menuMuxClass *) ast_args.usr;
struct dbr_sts_enum statusRec;

  statusRec = *( (struct dbr_sts_enum *) ast_args.dbr );

  mmuxo->fgColor.setStatus( statusRec.status, statusRec.severity );
  mmuxo->bgColor.setStatus( statusRec.status, statusRec.severity );

  mmuxo->needDraw = 1;

  mmuxo->actWin->appCtx->proc->lock();

  mmuxo->actWin->addDefExeNode( mmuxo->aglPtr );

  mmuxo->actWin->appCtx->proc->unlock();

}

#endif

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

  mmuxo->initialStateExpStr.setRaw( mmuxo->bufInitialState );

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

  mmuxc_edit_update( w, client, call );
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

  mmuxo->erase();
  mmuxo->deleteRequest = 1;
  mmuxo->ef.popdown();
  mmuxo->operationCancel();
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

  controlPvExpStr.copy ( source->controlPvExpStr );

  initialStateExpStr.copy( source->initialStateExpStr );

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

  strncpy( fontTag, actWin->defaultBtnFontTag, 63+1 );
  actWin->fi->loadFontTag( fontTag );
  fs = actWin->fi->getXFontStruct( fontTag );
  actWin->fi->getTextFontList( fontTag, &fontList );

  updateDimensions();

  topShadowColor = actWin->defaultTopShadowColor;
  botShadowColor = actWin->defaultBotShadowColor;

  fgColor.setColor( actWin->defaultTextFgColor, actWin->ci );
  bgColor.setColor( actWin->defaultBgColor, actWin->ci );

  this->draw();

  this->editCreate();

  return 1;

}

int menuMuxClass::save (
  FILE *f )
{

int index, i, ii;

  fprintf( f, "%-d %-d %-d\n", MMUXC_MAJOR_VERSION, MMUXC_MINOR_VERSION,
   MMUXC_RELEASE );

  fprintf( f, "%-d\n", x );
  fprintf( f, "%-d\n", y );
  fprintf( f, "%-d\n", w );
  fprintf( f, "%-d\n", h );

  actWin->ci->getIndex( fgColor.pixelColor(), &index );
  fprintf( f, "%-d\n", index );

  fprintf( f, "%-d\n", fgColorMode );

  actWin->ci->getIndex( bgColor.pixelColor(), &index );
  fprintf( f, "%-d\n", index );

  fprintf( f, "%-d\n", bgColorMode );

  actWin->ci->getIndex( topShadowColor, &index );
  fprintf( f, "%-d\n", index );

  actWin->ci->getIndex( botShadowColor, &index );
  fprintf( f, "%-d\n", index );

  if ( controlPvExpStr.getRaw() )
    writeStringToFile( f, controlPvExpStr.getRaw() );
  else
    writeStringToFile( f, "" );

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

  // version 1.2.0
  if ( initialStateExpStr.getRaw() )
    writeStringToFile( f, initialStateExpStr.getRaw() );
  else
    writeStringToFile( f, "" );

  return 1;

}

int menuMuxClass::createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int r, g, b, i, ii, index;
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
    fgColor.setColor( pixel, actWin->ci );

    fscanf( f, "%d\n", &fgColorMode ); actWin->incLine();

    if ( fgColorMode == MMUXC_K_COLORMODE_ALARM )
      fgColor.setAlarmSensitive();
    else
      fgColor.setAlarmInsensitive();

    fscanf( f, "%d\n", &index ); actWin->incLine();
    actWin->ci->setIndex( index, &pixel );
    bgColor.setColor( pixel, actWin->ci );

    fscanf( f, "%d\n", &bgColorMode ); actWin->incLine();

    if ( bgColorMode == MMUXC_K_COLORMODE_ALARM )
      bgColor.setAlarmSensitive();
    else
      bgColor.setAlarmInsensitive();

    fscanf( f, "%d\n", &index ); actWin->incLine();
    actWin->ci->setIndex( index, &topShadowColor );

    fscanf( f, "%d\n", &index ); actWin->incLine();
    actWin->ci->setIndex( index, &botShadowColor );

  }
  else {

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

  }

  readStringFromFile( oneName, 39, f ); actWin->incLine();
  controlPvExpStr.setRaw( oneName );

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

  if ( ( major > 1 ) || ( minor > 1 ) ) {
    readStringFromFile( oneName, 39, f ); actWin->incLine();
    initialStateExpStr.setRaw( oneName );
  }
  else {
    initialStateExpStr.setRaw( "0" );
  }

  numMac = 0;
  mac = NULL;
  exp = NULL;

  return 1;

}

int menuMuxClass::genericEdit ( void ) {

int i, ii;
char title[32], *ptr;

  ptr = actWin->obj.getNameFromClass( "menuMuxClass" );
  if ( ptr )
    strncpy( title, ptr, 31+1 );
  else
    strncpy( title, menuMuxClass_str2, 31+1 );

  strncat( title, menuMuxClass_str3, 31+1 );

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

  for ( i=0; i<MMUX_MAX_STATES; i++ ) {
    strncpy( bufTag[i], tag[i], MMUX_MAX_STRING_SIZE+1 );
  }
  for ( i=0; i<MMUX_MAX_STATES; i++ ) {
    for ( ii=0; ii<MMUX_MAX_ENTRIES; ii++ ) {
      strncpy( bufM[i][ii], m[i][ii], MMUX_MAX_STRING_SIZE+1 );
      strncpy( bufE[i][ii], e[i][ii], MMUX_MAX_STRING_SIZE+1 );
    }
  }

  if ( initialStateExpStr.getRaw() )
    strncpy( bufInitialState, initialStateExpStr.getRaw(), 15+1 );
  else
    strncpy( bufInitialState, "0", 15+1 );

  ef.create( actWin->top, actWin->appCtx->ci.getColorMap(),
   &actWin->appCtx->entryFormX,
   &actWin->appCtx->entryFormY, &actWin->appCtx->entryFormW,
   &actWin->appCtx->entryFormH, &actWin->appCtx->largestH,
   title, MMUX_MAX_STATES, numItems,
   mmuxSetItem, (void *) this, NULL, NULL, NULL );

  ef.addTextField( menuMuxClass_str4, 27, &bufX );
  ef.addTextField( menuMuxClass_str5, 27, &bufY );
  ef.addTextField( menuMuxClass_str6, 27, &bufW );
  ef.addTextField( menuMuxClass_str7, 27, &bufH );
  ef.addColorButton( menuMuxClass_str8, actWin->ci, &fgCb, &bufFgColor );
  ef.addOption( menuMuxClass_str9, menuMuxClass_str10, &bufFgColorMode );
  ef.addColorButton( menuMuxClass_str11, actWin->ci, &bgCb, &bufBgColor );
//   ef.addOption( menuMuxClass_str12, menuMuxClass_str13, &bufBgColorMode );
  ef.addColorButton( menuMuxClass_str14, actWin->ci, &topShadowCb,
   &bufTopShadowColor );
  ef.addColorButton( menuMuxClass_str15, actWin->ci, &botShadowCb,
   &bufBotShadowColor );
  ef.addFontMenu( menuMuxClass_str16, actWin->fi, &fm, fontTag );
  ef.addTextField( menuMuxClass_str17, 27, bufControlPvName, 39 );
  ef.addTextField( menuMuxClass_str18, 15, bufInitialState, 15 );

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

  ef.addTextFieldArray( menuMuxClass_str19, 25, tagPtr, MMUX_MAX_STRING_SIZE, &elbt );

  for ( i=0; i<MMUX_MAX_ENTRIES; i++ ) {
    ef.addTextFieldArray( menuMuxClass_str20, 25, mPtr[i], MMUX_MAX_STRING_SIZE,
     &elbm[i] );
    ef.addTextFieldArray( menuMuxClass_str21, 25, ePtr[i], MMUX_MAX_STRING_SIZE,
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
	      XmALIGNMENT_CENTER, menuMuxClass_str22 );

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

  // set colors

//   w = XmOptionButtonGadget( optionMenu );
//   n = 0;
//   XtSetArg( args[n], XmNbackground, bgColor.getColor() ); n++;
//   XtSetArg( args[n], XmNforeground, fgColor.getColor() ); n++;
//   XtSetValues( w, args, n );

  n = 0;
  XtSetArg( args[n], XmNforeground, fgColor.getColor() ); n++;
  XtSetValues( optionMenu, args, n );

  return 1;

}

int menuMuxClass::expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int stat;

  stat = controlPvExpStr.expand1st( numMacros, macros, expansions );
  stat = initialStateExpStr.expand1st( numMacros, macros, expansions );

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

int menuMuxClass::createWidgets ( void ) {

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

#ifdef __epics__
    alarmEventId = controlEventId = 0;
#endif

    controlPvConnected = active = 0;
    activeMode = 1;

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

#ifdef __epics__

        stat = ca_search_and_connect( controlPvExpStr.getExpanded(),
         &controlPvId, mmux_monitor_control_connect_state, this );
        if ( stat != ECA_NORMAL ) {
          printf( menuMuxClass_str23 );
          opStat = 0;
        }

#endif

      }
      else {

        actWin->appCtx->proc->lock();
        if ( initialStateExpStr.getExpanded() ) {
          curControlV = atol( initialStateExpStr.getExpanded() );
	}
	else {
          curControlV = 0;
	}
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
  activeMode = 0;

  if ( pass == 1 ) {

#ifdef __epics__

    if ( controlExists ) {
      stat = ca_clear_channel( controlPvId );
      if ( stat != ECA_NORMAL )
        printf( menuMuxClass_str24 );
    }

#endif

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

class menuMuxClass *mmo;
int stat;

  XtVaGetValues( w, XmNuserData, &mmo, NULL );

  stat = mmo->startDrag( w, e );

}

static void selectDrag (
   Widget w,
   XEvent *e,
   String *params,
   Cardinal numParams )
{

class menuMuxClass *mmo;
int stat;
XButtonEvent *be = (XButtonEvent *) e;

  XtVaGetValues( w, XmNuserData, &mmo, NULL );

  stat = mmo->selectDragValue( mmo->x + be->x, mmo->y + be->y );

}

void menuMuxClass::executeDeferred ( void ) {

int v;
int stat, i, nc, ndis, ni, nu, nd;
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

//----------------------------------------------------------------------------

  actWin->appCtx->proc->lock();
  nc = needConnectInit; needConnectInit = 0;
  ndis = needDisconnect; needDisconnect = 0;
  ni = needInfoInit; needInfoInit = 0;
  nu = needUpdate; needUpdate = 0;
  nd = needDraw; needDraw = 0;
  v = curControlV;
  actWin->remDefExeNode( aglPtr );
  actWin->appCtx->proc->unlock();

//----------------------------------------------------------------------------

#ifdef __epics__

  if ( nc ) {

    stat = ca_get_callback( DBR_GR_LONG, controlPvId,
     mmux_infoUpdate, (void *) this );

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

      if ( controlExists ) {
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

    parsedTrans = XtParseTranslationTable( dragTrans );
    XtAppAddActions( actWin->appCtx->appContext(), dragActions,
     XtNumber(dragActions) );

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
    XtSetArg( args[n], XmNtranslations, parsedTrans ); n++;
    XtSetArg( args[n], XmNuserData, this ); n++;

    optionMenu = XmCreateOptionMenu( actWin->executeWidgetId(), "",
     args, n );

    XtManageChild( optionMenu );

    widgetsCreated = 1;

    active = 1;

    if ( controlExists ) {

      stat = ca_add_masked_array_event( DBR_LONG, 1, controlPvId,
       mmux_controlUpdate, (void *) this, (float) 0.0, (float) 0.0,
       (float) 0.0, &controlEventId, DBE_VALUE );
      if ( stat != ECA_NORMAL ) {
        printf( menuMuxClass_str25 );
      }

      stat = ca_add_masked_array_event( DBR_STS_LONG, 1, controlPvId,
       mmux_alarmUpdate, (void *) this, (float) 0.0, (float) 0.0,
       (float) 0.0, &alarmEventId, DBE_ALARM );
      if ( stat != ECA_NORMAL ) {
        printf( menuMuxClass_str26 );
      }

    }
    else {

      firstEvent = 0;

    }

    nu = 1;

  }

#endif

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

char *menuMuxClass::firstDragName ( void ) {

  dragIndex = 0;
  return dragName[dragIndex];

}

char *menuMuxClass::nextDragName ( void ) {

  return NULL;

}

char *menuMuxClass::dragValue (
  int i ) {

  return controlPvExpStr.getExpanded();

}

void menuMuxClass::changeDisplayParams (
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

  if ( _flag & ACTGRF_TEXTFGCOLOR_MASK )
    fgColor.setColor( _textFgColor, actWin->ci );

  if ( _flag & ACTGRF_BGCOLOR_MASK )
    bgColor.setColor( _bgColor, actWin->ci );

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

void menuMuxClass::changePvNames (
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
