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

#define __table_cc 1

#include "table.h"
#include <sys/stat.h>
#include <unistd.h>
#include "app_pkg.h"
#include "act_win.h"

static void tablec_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeTableClass *tableo = (activeTableClass *) client;

  tableo->actWin->setChanged();

  tableo->eraseSelectBoxCorners();
  tableo->erase();

  tableo->fgColor.setColorIndex(
   tableo->eBuf->bufFgColor, tableo->actWin->ci );

  tableo->bgColor.setColorIndex(
   tableo->eBuf->bufBgColor, tableo->actWin->ci );

  tableo->oddBgColor.setColorIndex(
   tableo->eBuf->bufOddBgColor, tableo->actWin->ci );

  tableo->evenBgColor.setColorIndex(
   tableo->eBuf->bufEvenBgColor, tableo->actWin->ci );

  tableo->topShadowColor.setColorIndex(
   tableo->eBuf->bufTopShadowColor, tableo->actWin->ci );

  tableo->botShadowColor.setColorIndex(
   tableo->eBuf->bufBotShadowColor, tableo->actWin->ci );

  tableo->readPvExpStr.setRaw( tableo->eBuf->bufReadPvName );

  strncpy( tableo->fontTag, tableo->fm.currentFontTag(), 63 );
  tableo->fontTag[63] = 0;
  tableo->actWin->fi->loadFontTag( tableo->fontTag );
  tableo->fs =
   tableo->actWin->fi->getXFontStruct( tableo->fontTag );
  tableo->actWin->drawGc.setFontTag(
   tableo->fontTag, tableo->actWin->fi );

  tableo->x = tableo->eBuf->bufX;
  tableo->sboxX = tableo->eBuf->bufX;

  tableo->y = tableo->eBuf->bufY;
  tableo->sboxY = tableo->eBuf->bufY;

  tableo->w = tableo->eBuf->bufW;
  tableo->sboxW = tableo->eBuf->bufW;

  tableo->h = tableo->eBuf->bufH;
  tableo->sboxH = tableo->eBuf->bufH;

  if ( tableo->h < 10 ) {
    tableo->h = 10;
    tableo->sboxH = 10;
  }

}

static void tablec_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeTableClass *tableo = (activeTableClass *) client;

  tablec_edit_update ( w, client, call );
  tableo->refresh( tableo );

}

static void tablec_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeTableClass *tableo = (activeTableClass *) client;

  tablec_edit_update ( w, client, call );
  tableo->ef.popdown();
  tableo->operationComplete();

}

static void tablec_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeTableClass *tableo = (activeTableClass *) client;

  tableo->ef.popdown();
  tableo->operationCancel();

}

static void tablec_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeTableClass *tableo = (activeTableClass *) client;

  tableo->ef.popdown();
  tableo->operationCancel();
  tableo->erase();
  tableo->deleteRequest = 1;
  tableo->drawAll();

}

static void table_monitor_read_connect_state (
  ProcessVariable *pv,
  void *userarg )
{

activeTableClass *tableo =
 (activeTableClass *) userarg;

  if ( pv->is_valid() ) {

    tableo->needConnectInit = 1;

  }
  else {

    tableo->readPvConnected = 0;
    tableo->active = 0;
    tableo->fgColor.setDisconnected();
    tableo->needDraw = 1;

  }

  tableo->actWin->appCtx->proc->lock();
  tableo->actWin->addDefExeNode( tableo->aglPtr );
  tableo->actWin->appCtx->proc->unlock();

}

static void table_readUpdate (
  ProcessVariable *pv,
  void *userarg )
{

activeTableClass *tableo = (activeTableClass *) userarg;

  if ( tableo->active ) {

#if 0
    if ( tableo->firstReadUpdate ) {
      tableo->firstReadUpdate = 0;
      pv->putText( "" );
      return;
    }
#endif

    pv->get_string( tableo->curReadV, 39 );

    if ( !blank( tableo->curReadV ) ) {
      tableo->actWin->appCtx->proc->lock();
      tableo->needUpdate = 1;
      tableo->actWin->addDefExeNode( tableo->aglPtr );
      tableo->actWin->appCtx->proc->unlock();
    }

  }

}

activeTableClass::activeTableClass ( void ) {

  name = new char[strlen("activeTableClass")+1];
  strcpy( name, "activeTableClass" );
  checkBaseClassVersion( activeGraphicClass::MAJOR_VERSION, name );
  strcpy( fontTag, "" );
  fs = NULL;
  activeMode = 0;
  eBuf = NULL;

}

// copy constructor
activeTableClass::activeTableClass
 ( const activeTableClass *source ) {

activeGraphicClass *tableo = (activeGraphicClass *) this;

  tableo->clone( (activeGraphicClass *) source );

  name = new char[strlen("activeTableClass")+1];
  strcpy( name, "activeTableClass" );

  strncpy( fontTag, source->fontTag, 63 );
  fontTag[63] = 0;
  fs = actWin->fi->getXFontStruct( fontTag );

  fgColor.copy( source->fgColor );
  bgColor.copy( source->bgColor );
  oddBgColor.copy( source->oddBgColor );
  evenBgColor.copy( source->evenBgColor );
  topShadowColor.copy( source->topShadowColor );
  botShadowColor.copy( source->botShadowColor );

  readPvExpStr.copy( source->readPvExpStr );

  activeMode = 0;

  eBuf = NULL;

  doAccSubs( readPvExpStr );

}

activeTableClass::~activeTableClass ( void ) {

  if ( name ) delete[] name;

  if ( eBuf ) delete eBuf;

}

int activeTableClass::createInteractive (
  activeWindowClass *aw_obj,
  int _x,
  int _y,
  int _w,
  int _h ) {

  actWin = (activeWindowClass *) aw_obj;

  if ( _w < 10 )
    w = 10;
  else
    w = _w;

  if ( _h < 10 )
    h = 10;
  else
    h = _h;

  x = _x;
  y = _y;

  fgColor.setColorIndex( actWin->defaultTextFgColor, actWin->ci );
  oddBgColor.setColorIndex( actWin->defaultBgColor, actWin->ci );
  evenBgColor.setColorIndex( actWin->defaultOffsetColor, actWin->ci );
  bgColor.setColorIndex( actWin->defaultBgColor, actWin->ci );
  topShadowColor.setColorIndex( actWin->defaultTopShadowColor, actWin->ci );
  botShadowColor.setColorIndex( actWin->defaultBotShadowColor, actWin->ci );

  strcpy( fontTag, actWin->defaultFontTag );
  actWin->fi->loadFontTag( fontTag );
  fs = actWin->fi->getXFontStruct( fontTag );

  this->draw();

  this->editCreate();

  return 1;

}

int activeTableClass::save (
  FILE *f )
{


int stat, major, minor, release;

tagClass tag;

char *emptyStr = "";

  major = TABLEC_MAJOR_VERSION;
  minor = TABLEC_MINOR_VERSION;
  release = TABLEC_RELEASE;

  tag.init();
  tag.loadW( "beginObjectProperties" );
  tag.loadW( "major", &major );
  tag.loadW( "minor", &minor );
  tag.loadW( "release", &release );
  tag.loadW( "x", &x );
  tag.loadW( "y", &y );
  tag.loadW( "w", &w );
  tag.loadW( "h", &h );
  tag.loadW( "fgColor", actWin->ci, &fgColor );
  tag.loadW( "bgColor", actWin->ci, &bgColor );
  tag.loadW( "oddColBgColor", actWin->ci, &oddBgColor );
  tag.loadW( "evenColBgColor", actWin->ci, &evenBgColor );
  tag.loadW( "topShadowColor", actWin->ci, &topShadowColor );
  tag.loadW( "botShadowColor", actWin->ci, &botShadowColor );
  tag.loadW( "pv", &readPvExpStr, emptyStr );
  tag.loadW( "font", fontTag );
  tag.loadW( unknownTags );
  tag.loadW( "endObjectProperties" );
  tag.loadW( "" );

  stat = tag.writeTags( f );

  return stat;

}

int activeTableClass::old_save (
  FILE *f )
{

int index;

  fprintf( f, "%-d %-d %-d\n", TABLEC_MAJOR_VERSION,
   TABLEC_MINOR_VERSION, TABLEC_RELEASE );

  fprintf( f, "%-d\n", x );
  fprintf( f, "%-d\n", y );
  fprintf( f, "%-d\n", w );
  fprintf( f, "%-d\n", h );

  index = fgColor.pixelIndex();
  actWin->ci->writeColorIndex( f, index );
  //fprintf( f, "%-d\n", index );

  index = bgColor.pixelIndex();
  actWin->ci->writeColorIndex( f, index );
  //fprintf( f, "%-d\n", index );

  index = oddBgColor.pixelIndex();
  actWin->ci->writeColorIndex( f, index );
  //fprintf( f, "%-d\n", index );

  index = evenBgColor.pixelIndex();
  actWin->ci->writeColorIndex( f, index );
  //fprintf( f, "%-d\n", index );

  index = topShadowColor.pixelIndex();
  actWin->ci->writeColorIndex( f, index );
  //fprintf( f, "%-d\n", index );

  index = botShadowColor.pixelIndex();
  actWin->ci->writeColorIndex( f, index );
  //fprintf( f, "%-d\n", index );

  if ( readPvExpStr.getRaw() )
    writeStringToFile( f, readPvExpStr.getRaw() );
  else
    writeStringToFile( f, "" );

  writeStringToFile( f, fontTag );

  return 1;

}

int activeTableClass::createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int major, minor, release, stat;

tagClass tag;

char *emptyStr = "";

  this->actWin = _actWin;

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
  tag.loadR( "fgColor", actWin->ci, &fgColor );
  tag.loadR( "bgColor", actWin->ci, &bgColor );
  tag.loadR( "oddColBgColor", actWin->ci, &oddBgColor );
  tag.loadR( "evenColBgColor", actWin->ci, &evenBgColor );
  tag.loadR( "topShadowColor", actWin->ci, &topShadowColor );
  tag.loadR( "botShadowColor", actWin->ci, &botShadowColor );
  tag.loadR( "pv", &readPvExpStr, emptyStr );
  tag.loadR( "font", 63, fontTag );
  tag.loadR( "endObjectProperties" );
  tag.loadR( "" );

  stat = tag.readTags( f, "endObjectProperties" );

  if ( !( stat & 1 ) ) {
    actWin->appCtx->postMessage( tag.errMsg() );
  }

  if ( major > TABLEC_MAJOR_VERSION ) {
    postIncompatable();
    return 0;
  }

  if ( major < 4 ) {
    postIncompatable();
    return 0;
  }

  this->initSelectBox(); // call after getting x,y,w,h

  actWin->fi->loadFontTag( fontTag );
  fs = actWin->fi->getXFontStruct( fontTag );

  return stat;

}

int activeTableClass::old_createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int r, g, b, index;
int major, minor, release;
unsigned int pixel;
char oneName[PV_Factory::MAX_PV_NAME+1];

  this->actWin = _actWin;

  fscanf( f, "%d %d %d\n", &major, &minor, &release );

  if ( major > TABLEC_MAJOR_VERSION ) {
    postIncompatable();
    return 0;
  }
  fscanf( f, "%d\n", &x );
  fscanf( f, "%d\n", &y );
  fscanf( f, "%d\n", &w );
  fscanf( f, "%d\n", &h );

  this->initSelectBox();

  if ( ( major > 2 ) || ( ( major == 2 ) && ( minor > 1 ) ) ) {

    actWin->ci->readColorIndex( f, &index );
    actWin->incLine(); actWin->incLine();
    fgColor.setColorIndex( index, actWin->ci );

    actWin->ci->readColorIndex( f, &index );
    actWin->incLine(); actWin->incLine();
    bgColor.setColorIndex( index, actWin->ci );

    actWin->ci->readColorIndex( f, &index );
    actWin->incLine(); actWin->incLine();
    oddBgColor.setColorIndex( index, actWin->ci );

    actWin->ci->readColorIndex( f, &index );
    actWin->incLine(); actWin->incLine();
    evenBgColor.setColorIndex( index, actWin->ci );

    actWin->ci->readColorIndex( f, &index );
    actWin->incLine(); actWin->incLine();
    topShadowColor.setColorIndex( index, actWin->ci );

    actWin->ci->readColorIndex( f, &index );
    actWin->incLine(); actWin->incLine();
    botShadowColor.setColorIndex( index, actWin->ci );

  }
  else if ( major > 1 ) {

    fscanf( f, "%d\n", &index );
    fgColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d\n", &index );
    bgColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d\n", &index );
    oddBgColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d\n", &index );
    evenBgColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d\n", &index );
    topShadowColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d\n", &index );
    botShadowColor.setColorIndex( index, actWin->ci );

  }
  else {

    fscanf( f, "%d %d %d\n", &r, &g, &b );
    actWin->ci->setRGB( r, g, b, &pixel );
    index = actWin->ci->pixIndex( pixel );
    fgColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d %d %d\n", &r, &g, &b );
    actWin->ci->setRGB( r, g, b, &pixel );
    index = actWin->ci->pixIndex( pixel );
    bgColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d %d %d\n", &r, &g, &b );
    actWin->ci->setRGB( r, g, b, &pixel );
    index = actWin->ci->pixIndex( pixel );
    oddBgColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d %d %d\n", &r, &g, &b );
    actWin->ci->setRGB( r, g, b, &pixel );
    index = actWin->ci->pixIndex( pixel );
    evenBgColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d %d %d\n", &r, &g, &b );
    actWin->ci->setRGB( r, g, b, &pixel );
    index = actWin->ci->pixIndex( pixel );
    topShadowColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d %d %d\n", &r, &g, &b );
    actWin->ci->setRGB( r, g, b, &pixel );
    index = actWin->ci->pixIndex( pixel );
    botShadowColor.setColorIndex( index, actWin->ci );

  }

  readStringFromFile( oneName, PV_Factory::MAX_PV_NAME+1, f );
  readPvExpStr.setRaw( oneName );

  readStringFromFile( fontTag, 63+1, f );

  actWin->fi->loadFontTag( fontTag );
  fs = actWin->fi->getXFontStruct( fontTag );

  return 1;

}

int activeTableClass::genericEdit ( void ) {

char title[32], *ptr;

  if ( !eBuf ) {
    eBuf = new editBufType;
  }

  ptr = actWin->obj.getNameFromClass( "activeTableClass" );
  if ( ptr )
    strncpy( title, ptr, 31 );
  else
    strncpy( title, activeTableClass_str4, 31 );

  Strncat( title, activeTableClass_str5, 31 );

  eBuf->bufX = x;
  eBuf->bufY = y;
  eBuf->bufW = w;
  eBuf->bufH = h;

  eBuf->bufFgColor = fgColor.pixelIndex();
  eBuf->bufBgColor = bgColor.pixelIndex();
  eBuf->bufOddBgColor = oddBgColor.pixelIndex();
  eBuf->bufEvenBgColor = evenBgColor.pixelIndex();
  eBuf->bufTopShadowColor = topShadowColor.pixelIndex();
  eBuf->bufBotShadowColor = botShadowColor.pixelIndex();

  if ( readPvExpStr.getRaw() )
    strncpy( eBuf->bufReadPvName, readPvExpStr.getRaw(),
     PV_Factory::MAX_PV_NAME );
  else
    strcpy( eBuf->bufReadPvName, "" );

  ef.create( actWin->top, actWin->appCtx->ci.getColorMap(),
   &actWin->appCtx->entryFormX,
   &actWin->appCtx->entryFormY, &actWin->appCtx->entryFormW,
   &actWin->appCtx->entryFormH, &actWin->appCtx->largestH,
   title, NULL, NULL, NULL );

  ef.addTextField( activeTableClass_str6, 35, &eBuf->bufX );
  ef.addTextField( activeTableClass_str7, 35, &eBuf->bufY );
  ef.addTextField( activeTableClass_str8, 35, &eBuf->bufW );
  ef.addTextField( activeTableClass_str9, 35, &eBuf->bufH );
  ef.addTextField( activeTableClass_str11, 35, eBuf->bufReadPvName,
   PV_Factory::MAX_PV_NAME );
  ef.addColorButton( activeTableClass_str16, actWin->ci, &eBuf->fgCb, &eBuf->bufFgColor );
  ef.addColorButton( activeTableClass_str17, actWin->ci, &eBuf->bgCb, &eBuf->bufBgColor );
  ef.addColorButton( activeTableClass_str18, actWin->ci, &eBuf->oddBgCb, &eBuf->bufOddBgColor );
  ef.addColorButton( activeTableClass_str25, actWin->ci, &eBuf->evenBgCb, &eBuf->bufEvenBgColor );
  ef.addColorButton( activeTableClass_str19, actWin->ci, &eBuf->topCb,
   &eBuf->bufTopShadowColor );
  ef.addColorButton( activeTableClass_str20, actWin->ci, &eBuf->botCb,
   &eBuf->bufBotShadowColor );
  ef.addFontMenu( activeTableClass_str15, actWin->fi, &fm, fontTag );

  XtUnmanageChild( fm.alignWidget() ); // no alignment info

  return 1;

}

int activeTableClass::editCreate ( void ) {

  this->genericEdit();
  ef.finished( tablec_edit_ok, tablec_edit_apply, tablec_edit_cancel_delete, this );
  actWin->currentEf = NULL;
  ef.popup();

  return 1;

}

int activeTableClass::edit ( void ) {

  this->genericEdit();
  ef.finished( tablec_edit_ok, tablec_edit_apply, tablec_edit_cancel, this );
  actWin->currentEf = &ef;
  ef.popup();

  return 1;

}

int activeTableClass::erase ( void ) {

  if ( deleteRequest ) return 1;

  XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  return 1;

}

int activeTableClass::eraseActive ( void ) {

  return 1;

}

int activeTableClass::draw ( void ) {

  if ( deleteRequest ) return 1;
  actWin->drawGc.saveFg();

  actWin->drawGc.setFG( bgColor.pixelColor() );
  XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x, y, w, h );

  actWin->drawGc.setFG( fgColor.pixelColor() );
  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x, y, w, h );

  actWin->drawGc.setFontTag( fontTag, actWin->fi );

  drawText( actWin->drawWidget, &actWin->drawGc,
   fs, x+w/2, y+h/2, XmALIGNMENT_CENTER, activeTableClass_str21 );

  actWin->drawGc.restoreFg();

  return 1;

}

int activeTableClass::drawActive ( void ) {

  return 1;

}

int activeTableClass::activate (
  int pass,
  void *ptr )
{

int opStat;

  switch ( pass ) {

  case 1:

    opComplete = 0;

    break;

  case 2:

    if ( !opComplete ) {

      aglPtr = ptr;
      needConnectInit = needUpdate = needDraw = 0;
      readPvId = NULL;
      initialReadConnection = 1;
      firstReadUpdate = 1;
      readPvConnected = active = init = 0;
      activeMode = 1;
      strcpy( curReadV, "" );

      if ( !readPvExpStr.getExpanded() ||
	// ( strcmp( readPvExpStr.getExpanded(), "" ) == 0 ) ) {
        blankOrComment( readPvExpStr.getExpanded() ) ) {
        readExists = 0;
      }
      else {
        readExists = 1;
        fgColor.setConnectSensitive();
      }

      frameWidget = NULL;
      initEnable();

      opStat = 1;

      createTableWidgets();

      if ( readExists ) {
        readPvId = the_PV_Factory->create( readPvExpStr.getExpanded() );
	if ( readPvId ) {
	  readPvId->add_conn_state_callback(
           table_monitor_read_connect_state, this );
	}
	else {
          fprintf( stderr, activeTableClass_str22 );
          opStat = 0;
        }
      }

      if ( opStat & 1 ) opComplete = 1;

      return opStat;

    }

    break;

  case 3:
  case 4:

    break;

  case 5:

    opComplete = 0;
    break;

  case 6:

    if ( !opComplete ) {

      opComplete = 1;

    }

    break;

  }

  return 1;

}

int activeTableClass::deactivate (
  int pass
) {

  if ( pass == 1 ) {

    active = 0;
    activeMode = 0;

    table.destroy();
    if ( frameWidget ) XtDestroyWidget( frameWidget );
    frameWidget = NULL;

    if ( readExists ) {
      if ( readPvId ) {
        readPvId->remove_conn_state_callback(
         table_monitor_read_connect_state, this );
        readPvId->remove_value_callback(
         table_readUpdate, this );
	readPvId->release();
	readPvId = NULL;
      }
    }

  }

  return 1;

}

int activeTableClass::expandTemplate (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

expStringClass tmpStr;

  tmpStr.setRaw( readPvExpStr.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  readPvExpStr.setRaw( tmpStr.getExpanded() );

  return 1;

}

int activeTableClass::expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int stat;

  stat = readPvExpStr.expand1st( numMacros, macros, expansions );

  return stat;

}

int activeTableClass::expand2nd (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int stat;

  stat = readPvExpStr.expand2nd( numMacros, macros, expansions );

  return stat;

}

int activeTableClass::containsMacros ( void ) {

int stat;

  stat = readPvExpStr.containsPrimaryMacros();

  return stat;

}

int activeTableClass::createTableWidgets ( void ) {

  frameWidget = XtVaCreateManagedWidget( "", xmBulletinBoardWidgetClass,
   actWin->executeWidgetId(),
   XmNx, x,
   XmNy, y,
   XmNwidth, w,
   XmNheight, h,
   XmNmarginHeight, 0,
   XmNmarginWidth, 0,
   XmNborderColor, bgColor.pixelColor(),
   XmNhighlightColor, bgColor.pixelColor(),
   XmNforeground, bgColor.pixelColor(),
   XmNbackground, bgColor.pixelColor(),
   XmNshadowThickness, 0,
   XmNtopShadowColor, topShadowColor.pixelColor(),
   XmNbottomShadowColor, botShadowColor.pixelColor(),
   XmNshadowType, XmSHADOW_ETCHED_OUT,
   XmNmappedWhenManaged, False,
   NULL );

  if ( !frameWidget ) {
    fprintf( stderr, activeTableClass_str24 );
    return 0;
  }

  XtRealizeWidget( frameWidget );

  if ( enabled ) {
    XtMapWidget( frameWidget );
  }

  return 1;

}

int activeTableClass::checkResizeSelectBox (
  int _x,
  int _y,
  int _w,
  int _h ) {

int tmpw, tmph, ret_stat;

  tmpw = sboxW;
  tmph = sboxH;

  ret_stat = 1;

  tmpw += _w;
  if ( tmpw < 10 ) {
    ret_stat = 0;
  }

  tmph += _h;
  if ( tmph < 10 ) {
    ret_stat = 0;
  }

  return ret_stat;

}

int activeTableClass::checkResizeSelectBoxAbs (
  int _x,
  int _y,
  int _w,
  int _h ) {

int tmpw, tmph, ret_stat;

  ret_stat = 1;

  if ( _w != -1 ) {
    tmpw = _w;
    if ( tmpw < 10 ) {
      ret_stat = 0;
    }
  }

  if ( _h != -1 ) {
    tmph = _h;
    if ( tmph < 10 ) {
      ret_stat = 0;
    }
  }

  return ret_stat;

}

void activeTableClass::executeDeferred ( void ) {

char v[39+1];
int nc, nu, nd, status, i, isComment;
FILE *f;
char *more;
char msg[4095+1];
tagClass tag;
int numCols;
char headerAlignStr[4095+1];
char alignStr[4095+1];
char sepStr[255+1], comment[7+1];
char *tk, *ctx;
Widget wdgt;

//----------------------------------------------------------------------------

  actWin->appCtx->proc->lock();
  nc = needConnectInit; needConnectInit = 0;
  nu = needUpdate; needUpdate = 0;
  nd = needDraw; needDraw = 0;
  strncpy( v, curReadV, 39 );
  actWin->remDefExeNode( aglPtr );
  actWin->appCtx->proc->unlock();

  if ( !activeMode ) return;

//----------------------------------------------------------------------------

  if ( nc ) {

    readPvConnected = 1;
    active = 1;
    init = 1;

    if ( initialReadConnection ) {

      initialReadConnection = 0;

      readPvId->add_value_callback( table_readUpdate, this );

    }

    fgColor.setConnected();
    drawActive();

  }

  if ( nu ) {

    strncpy( readV, v, 39 );

    // open file name in readV
    //fprintf( stderr, "open [%s]\n", readV );

    

    f = fopen( readV, "r" );
    if ( !f ) {

      table.destroy(); // its always safe to call destroy

      table.create( frameWidget, 0, 0, w, h, 1, "l", "l",
       actWin->fi, fontTag, fgColor.pixelColor(), bgColor.pixelColor(),
       oddBgColor.pixelColor(), evenBgColor.pixelColor(),
       topShadowColor.pixelColor(), botShadowColor.pixelColor() );

      snprintf( msg, 79, "File [%s] could not be opened", readV );
      msg[79] = 0;
      table.addCell( msg );

      table.endOfContent();

    }
    else {

      strcpy( comment, "" );

      tag.init();
      tag.initLine();
      tag.loadR( "begin" );
      tag.loadR( "numCols", &numCols );
      tag.loadR( "headerAlign", 4095, headerAlignStr );
      tag.loadR( "align", 4095, alignStr );
      tag.loadR( "separators", 253, sepStr );
      tag.loadR( "comment", 1, comment );
      tag.loadR( "end" );

      status = tag.readTags( f, "end" );

      if ( !( status & 1 ) ) {
        actWin->appCtx->postMessage( tag.errMsg() );
      }
      else {

	//fprintf( stderr, "numCols = %-d\n", numCols );
	//fprintf( stderr, "alignStr = [%s]\n", alignStr );
	//fprintf( stderr, "headerAlignStr = [%s]\n", headerAlignStr );
	//fprintf( stderr, "sepStr = [%s]\n", sepStr );

	Strncat( sepStr, "\n", 255 );

        table.destroy(); // its always safe to call destroy

        table.create( frameWidget, 0, 0, w, h, numCols, headerAlignStr,
         alignStr, actWin->fi, fontTag, fgColor.pixelColor(),
         bgColor.pixelColor(), oddBgColor.pixelColor(),
         evenBgColor.pixelColor(), topShadowColor.pixelColor(),
         botShadowColor.pixelColor() );

        do {

          isComment = 0;
          more = fgets( msg, 4095, f );
          msg[4095] = 0;

          if ( more && !blank(msg) ) {

            ctx = NULL;
            tk = strtok_r( msg, sepStr, &ctx );
            if ( tk ) {
	      //fprintf( stderr, "tk = [%s]\n", tk );
              if ( strncmp( tk, comment, 1 ) != 0 ) {
                wdgt = table.addCell( tk );
                //if ( !wdgt ) fprintf( stderr, "err\n" );
	      }
	      else {
		isComment = 1;
	      }
	    }
	    else {
	      table.addCell( "" );
	    }
	    if ( !isComment ) {
	      for ( i=1; i<numCols; i++ ) {
                tk = strtok_r( NULL, sepStr, &ctx );
                if ( tk ) {
	          //fprintf( stderr, "tk = [%s]\n", tk );
                  table.addCell( tk );
	        }
	        else {
	          table.addCell( "" );
	        }
	      }
	    }

          }

        } while ( more );

        table.endOfContent();

      }

      fclose( f );

    }

  }

//----------------------------------------------------------------------------

  if ( nd ) {
    strncpy( readV, v, 39 );
    drawActive();
  }

//----------------------------------------------------------------------------

}

void activeTableClass::changeDisplayParams (
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
    fgColor.setColorIndex( actWin->defaultTextFgColor, actWin->ci );

  if ( _flag & ACTGRF_BGCOLOR_MASK ) {
    bgColor.setColorIndex( actWin->defaultBgColor, actWin->ci );
    oddBgColor.setColorIndex( actWin->defaultOffsetColor, actWin->ci );
  }

  if ( _flag & ACTGRF_OFFSETCOLOR_MASK )
    evenBgColor.setColorIndex( actWin->defaultOffsetColor, actWin->ci );

  if ( _flag & ACTGRF_TOPSHADOWCOLOR_MASK )
    topShadowColor.setColorIndex( actWin->defaultTopShadowColor, actWin->ci );

  if ( _flag & ACTGRF_BOTSHADOWCOLOR_MASK )
    botShadowColor.setColorIndex( actWin->defaultBotShadowColor, actWin->ci );

  if ( _flag & ACTGRF_FONTTAG_MASK ) {
    strcpy( fontTag, _fontTag );
    actWin->fi->loadFontTag( fontTag );
    fs = actWin->fi->getXFontStruct( fontTag );
  }

}

void activeTableClass::changePvNames (
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

  if ( flag & ACTGRF_READBACKPVS_MASK ) {
    if ( numReadbackPvs ) {
      readPvExpStr.setRaw( readbackPvs[0] );
    }
  }

}

void activeTableClass::map ( void ) {

  if ( frameWidget ) {
    XtMapWidget( frameWidget );
  }

}

void activeTableClass::unmap ( void ) {

  if ( frameWidget ) {
    XtUnmapWidget( frameWidget );
  }

}

void activeTableClass::getPvs (
  int max,
  ProcessVariable *pvs[],
  int *n ) {

  if ( max < 1 ) {
    *n = 0;
    return;
  }

  *n = 1;
  pvs[0] = readPvId;

}

char *activeTableClass::getSearchString (
  int i
) {

  if ( i == 0 ) {
    return readPvExpStr.getRaw();
  }

  return NULL;

}

void activeTableClass::replaceString (
  int i,
  int max,
  char *string
) {

  if ( i == 0 ) {
    readPvExpStr.setRaw( string );
  }

}

// crawler functions may return blank pv names
char *activeTableClass::crawlerGetFirstPv ( void ) {

  crawlerPvIndex = 0;
  return readPvExpStr.getExpanded();

}

char *activeTableClass::crawlerGetNextPv ( void ) {

  return NULL;

}

#ifdef __cplusplus
extern "C" {
#endif

void *create_activeTableClassPtr ( void ) {

activeTableClass *ptr;

  ptr = new activeTableClass;
  return (void *) ptr;

}

void *clone_activeTableClassPtr (
  void *_srcPtr )
{

activeTableClass *ptr, *srcPtr;

  srcPtr = (activeTableClass *) _srcPtr;

  ptr = new activeTableClass( srcPtr );

  return (void *) ptr;

}

#ifdef __cplusplus
}
#endif
