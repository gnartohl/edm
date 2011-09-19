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

#define __coefTable_cc 1

#include "coefTable.h"
#include <sys/stat.h>
#include <unistd.h>
#include "app_pkg.h"
#include "act_win.h"

static void coefTablec_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeCoefTableClass *coefTableo = (activeCoefTableClass *) client;

  coefTableo->actWin->setChanged();

  coefTableo->eraseSelectBoxCorners();
  coefTableo->erase();

  coefTableo->fgColor.setColorIndex(
   coefTableo->eBuf->bufFgColor, coefTableo->actWin->ci );

  coefTableo->bgColor.setColorIndex(
   coefTableo->eBuf->bufBgColor, coefTableo->actWin->ci );

  coefTableo->oddBgColor.setColorIndex(
   coefTableo->eBuf->bufOddBgColor, coefTableo->actWin->ci );

  coefTableo->evenBgColor.setColorIndex(
   coefTableo->eBuf->bufEvenBgColor, coefTableo->actWin->ci );

  coefTableo->topShadowColor.setColorIndex(
   coefTableo->eBuf->bufTopShadowColor, coefTableo->actWin->ci );

  coefTableo->botShadowColor.setColorIndex(
   coefTableo->eBuf->bufBotShadowColor, coefTableo->actWin->ci );

  coefTableo->readPvExpStr.setRaw( coefTableo->eBuf->bufReadPvName );

  coefTableo->labelsExpStr.setRaw( coefTableo->eBuf->bufLabels );

  coefTableo->efFirstEle = coefTableo->eBuf->bufEfFirstEle;
  if ( coefTableo->efFirstEle.isNull() )
    coefTableo->firstEle = 0;
  else
    coefTableo->firstEle = coefTableo->eBuf->bufEfFirstEle.value();

  coefTableo->efNumEle = coefTableo->eBuf->bufEfNumEle;
  if ( coefTableo->efNumEle.isNull() )
    coefTableo->numEle = 0;
  else
    coefTableo->numEle = coefTableo->eBuf->bufEfNumEle.value();

  coefTableo->formatExpStr.setRaw( coefTableo->eBuf->bufFormat );

  strncpy( coefTableo->fontTag, coefTableo->fm.currentFontTag(), 63 );
  coefTableo->fontTag[63] = 0;
  coefTableo->actWin->fi->loadFontTag( coefTableo->fontTag );
  coefTableo->fs =
   coefTableo->actWin->fi->getXFontStruct( coefTableo->fontTag );
  coefTableo->actWin->drawGc.setFontTag(
   coefTableo->fontTag, coefTableo->actWin->fi );

  coefTableo->x = coefTableo->eBuf->bufX;
  coefTableo->sboxX = coefTableo->eBuf->bufX;

  coefTableo->y = coefTableo->eBuf->bufY;
  coefTableo->sboxY = coefTableo->eBuf->bufY;

  coefTableo->w = coefTableo->eBuf->bufW;
  coefTableo->sboxW = coefTableo->eBuf->bufW;

  coefTableo->h = coefTableo->eBuf->bufH;
  coefTableo->sboxH = coefTableo->eBuf->bufH;

  if ( coefTableo->h < 10 ) {
    coefTableo->h = 10;
    coefTableo->sboxH = 10;
  }

}

static void coefTablec_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeCoefTableClass *coefTableo = (activeCoefTableClass *) client;

  coefTablec_edit_update ( w, client, call );
  coefTableo->refresh( coefTableo );

}

static void coefTablec_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeCoefTableClass *coefTableo = (activeCoefTableClass *) client;

  coefTablec_edit_update ( w, client, call );
  coefTableo->ef.popdown();
  coefTableo->operationComplete();

}

static void coefTablec_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeCoefTableClass *coefTableo = (activeCoefTableClass *) client;

  coefTableo->ef.popdown();
  coefTableo->operationCancel();

}

static void coefTablec_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeCoefTableClass *coefTableo = (activeCoefTableClass *) client;

  coefTableo->ef.popdown();
  coefTableo->operationCancel();
  coefTableo->erase();
  coefTableo->deleteRequest = 1;
  coefTableo->drawAll();

}

static void coefTable_monitor_read_connect_state (
  ProcessVariable *pv,
  void *userarg )
{

activeCoefTableClass *coefTableo =
 (activeCoefTableClass *) userarg;

  if ( pv->is_valid() ) {

    coefTableo->needConnectInit = 1;

  }
  else {

    coefTableo->readPvConnected = 0;
    coefTableo->active = 0;
    coefTableo->fgColor.setDisconnected();
    coefTableo->needDraw = 1;

  }

  coefTableo->actWin->appCtx->proc->lock();
  coefTableo->actWin->addDefExeNode( coefTableo->aglPtr );
  coefTableo->actWin->appCtx->proc->unlock();

}

static void coefTable_readUpdate (
  ProcessVariable *pv,
  void *userarg )
{

activeCoefTableClass *coefTableo = (activeCoefTableClass *) userarg;

  if ( coefTableo->active ) {

    if ( pv->get_type().type !=  ProcessVariable::Type::text ) {

      coefTableo->actWin->appCtx->proc->lock();
      coefTableo->needUpdate = 1;
      coefTableo->actWin->addDefExeNode( coefTableo->aglPtr );
      coefTableo->actWin->appCtx->proc->unlock();

    }

  }

}

activeCoefTableClass::activeCoefTableClass ( void ) {

  name = new char[strlen("activeCoefTableClass")+1];
  strcpy( name, "activeCoefTableClass" );
  checkBaseClassVersion( activeGraphicClass::MAJOR_VERSION, name );
  strcpy( fontTag, "" );
  fs = NULL;
  activeMode = 0;
  eBuf = NULL;
  arraySize = 0;

  efFirstEle.setNull(1);
  firstEle = 0;
  efNumEle.setNull(1);
  numEle = 0;

}

// copy constructor
activeCoefTableClass::activeCoefTableClass
 ( const activeCoefTableClass *source ) {

activeGraphicClass *coefTableo = (activeGraphicClass *) this;

  coefTableo->clone( (activeGraphicClass *) source );

  name = new char[strlen("activeCoefTableClass")+1];
  strcpy( name, "activeCoefTableClass" );

  strncpy( fontTag, source->fontTag, 63 );
  fontTag[63] = 0;
  fs = actWin->fi->getXFontStruct( fontTag );

  fgColor.copy( source->fgColor );
  bgColor.copy( source->bgColor );
  oddBgColor.copy( source->oddBgColor );
  evenBgColor.copy( source->evenBgColor );
  topShadowColor.copy( source->topShadowColor );
  botShadowColor.copy( source->botShadowColor );
  arraySize = 0;

  readPvExpStr.copy( source->readPvExpStr );
  labelsExpStr.copy( source->labelsExpStr );

  efFirstEle = source->efFirstEle;
  firstEle = source->firstEle;
  efNumEle = source->efNumEle;
  numEle = source->numEle;

  formatExpStr.copy( source->formatExpStr );

  activeMode = 0;

  eBuf = NULL;

  doAccSubs( readPvExpStr );
  doAccSubs( labelsExpStr );

}

activeCoefTableClass::~activeCoefTableClass ( void ) {

  if ( name ) delete[] name;

  if ( eBuf ) delete eBuf;

}

int activeCoefTableClass::createInteractive (
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

int activeCoefTableClass::save (
  FILE *f )
{


int stat, major, minor, release;

tagClass tag;

char *emptyStr = "";

  major = COEFTABLEC_MAJOR_VERSION;
  minor = COEFTABLEC_MINOR_VERSION;
  release = COEFTABLEC_RELEASE;

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
  tag.loadW( "labels", &labelsExpStr, emptyStr );
  tag.loadW( "firstElement", &efFirstEle );
  tag.loadW( "numElements", &efNumEle );
  tag.loadW( "font", fontTag );
  tag.loadW( "format", &formatExpStr, emptyStr );
  tag.loadW( unknownTags );
  tag.loadW( "endObjectProperties" );
  tag.loadW( "" );

  stat = tag.writeTags( f );

  return stat;

}

int activeCoefTableClass::createFromFile (
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
  tag.loadR( "labels", &labelsExpStr, emptyStr );
  tag.loadR( "firstElement", &efFirstEle );
  tag.loadR( "numElements", &efNumEle );
  tag.loadR( "font", 63, fontTag );
  tag.loadR( "format", &formatExpStr, emptyStr );
  tag.loadR( "endObjectProperties" );
  tag.loadR( "" );

  stat = tag.readTags( f, "endObjectProperties" );

  if ( !( stat & 1 ) ) {
    actWin->appCtx->postMessage( tag.errMsg() );
  }

  if ( major > COEFTABLEC_MAJOR_VERSION ) {
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

  if ( efFirstEle.isNull() )
    firstEle = 0;
  else
    firstEle = efFirstEle.value();

  if ( efNumEle.isNull() )
    numEle = 0;
  else
    numEle = efNumEle.value();


  return stat;

}

int activeCoefTableClass::genericEdit ( void ) {

char title[32], *ptr;

  if ( !eBuf ) {
    eBuf = new editBufType;
  }

  ptr = actWin->obj.getNameFromClass( "activeCoefTableClass" );
  if ( ptr ) {
    strncpy( title, ptr, 31 );
    title[31] = 0;
  }
  else {
    strncpy( title, activeCoefTableClass_str4, 31 );
    title[31] = 0;
  }

  Strncat( title, activeCoefTableClass_str5, 31 );

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

  if ( readPvExpStr.getRaw() ) {
    strncpy( eBuf->bufReadPvName, readPvExpStr.getRaw(),
     PV_Factory::MAX_PV_NAME );
    eBuf->bufReadPvName[PV_Factory::MAX_PV_NAME] = 0;
  }
  else
    strcpy( eBuf->bufReadPvName, "" );

  if ( labelsExpStr.getRaw() ) {
    strncpy( eBuf->bufLabels, labelsExpStr.getRaw(),
     MaxLabelSize );
    eBuf->bufLabels[MaxLabelSize] = 0;
  }
  else
    strcpy( eBuf->bufLabels, "" );

  eBuf->bufEfFirstEle = efFirstEle;
  eBuf->bufEfNumEle = efNumEle;

  if ( formatExpStr.getRaw() ) {
    strncpy( eBuf->bufFormat, formatExpStr.getRaw(), 15 );
    eBuf->bufFormat[15] = 0;
  }
  else
    strcpy( eBuf->bufFormat, "" );

  ef.create( actWin->top, actWin->appCtx->ci.getColorMap(),
   &actWin->appCtx->entryFormX,
   &actWin->appCtx->entryFormY, &actWin->appCtx->entryFormW,
   &actWin->appCtx->entryFormH, &actWin->appCtx->largestH,
   title, NULL, NULL, NULL );

  ef.addTextField( activeCoefTableClass_str6, 35, &eBuf->bufX );
  ef.addTextField( activeCoefTableClass_str7, 35, &eBuf->bufY );
  ef.addTextField( activeCoefTableClass_str8, 35, &eBuf->bufW );
  ef.addTextField( activeCoefTableClass_str9, 35, &eBuf->bufH );
  ef.addTextField( activeCoefTableClass_str11, 35, eBuf->bufReadPvName,
   PV_Factory::MAX_PV_NAME );
  ef.addTextField( activeCoefTableClass_str26, 35, eBuf->bufLabels,
   MaxLabelSize );
  ef.addTextField( activeCoefTableClass_str27, 35, &eBuf->bufEfFirstEle );
  ef.addTextField( activeCoefTableClass_str28, 35, &eBuf->bufEfNumEle );
  ef.addTextField( activeCoefTableClass_str29, 35, eBuf->bufFormat, 15 );
  ef.addColorButton( activeCoefTableClass_str16, actWin->ci, &eBuf->fgCb, &eBuf->bufFgColor );
  ef.addColorButton( activeCoefTableClass_str17, actWin->ci, &eBuf->bgCb, &eBuf->bufBgColor );
  ef.addColorButton( activeCoefTableClass_str18, actWin->ci, &eBuf->oddBgCb, &eBuf->bufOddBgColor );
  ef.addColorButton( activeCoefTableClass_str25, actWin->ci, &eBuf->evenBgCb, &eBuf->bufEvenBgColor );
  ef.addColorButton( activeCoefTableClass_str19, actWin->ci, &eBuf->topCb,
   &eBuf->bufTopShadowColor );
  ef.addColorButton( activeCoefTableClass_str20, actWin->ci, &eBuf->botCb,
   &eBuf->bufBotShadowColor );
  ef.addFontMenu( activeCoefTableClass_str15, actWin->fi, &fm, fontTag );

  XtUnmanageChild( fm.alignWidget() ); // no alignment info

  return 1;

}

int activeCoefTableClass::editCreate ( void ) {

  this->genericEdit();
  ef.finished( coefTablec_edit_ok, coefTablec_edit_apply, coefTablec_edit_cancel_delete, this );
  actWin->currentEf = NULL;
  ef.popup();

  return 1;

}

int activeCoefTableClass::edit ( void ) {

  this->genericEdit();
  ef.finished( coefTablec_edit_ok, coefTablec_edit_apply, coefTablec_edit_cancel, this );
  actWin->currentEf = &ef;
  ef.popup();

  return 1;

}

int activeCoefTableClass::erase ( void ) {

  if ( activeMode ) return 1;
  if ( deleteRequest ) return 1;

  XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  return 1;

}

int activeCoefTableClass::eraseActive ( void ) {

  return 1;

}

int activeCoefTableClass::draw ( void ) {

XRectangle xR = { x, y, w, h };
int clipStat;

  if ( activeMode ) return 1;
  if ( deleteRequest ) return 1;

  actWin->drawGc.saveFg();

  clipStat = actWin->drawGc.addNormXClipRectangle( xR );

  actWin->drawGc.setFG( bgColor.pixelColor() );
  XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x, y, w, h );

  actWin->drawGc.setFG( fgColor.pixelColor() );
  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x, y, w, h );

  actWin->drawGc.setFontTag( fontTag, actWin->fi );

  if ( fs ) {
    updateFont( " ", fontTag, &fs,
     &fontAscent, &fontDescent, &fontHeight,
     &stringWidth );
  }
  else {
    fontHeight = 10;
  }

  drawText( actWin->drawWidget, &actWin->drawGc,
   fs, x+w/2, y+h/2-fontHeight/2, XmALIGNMENT_CENTER,
   activeCoefTableClass_str21 );

  if ( clipStat & 1 ) actWin->drawGc.removeNormXClipRectangle();

  actWin->drawGc.restoreFg();

  return 1;

}

int activeCoefTableClass::drawActive ( void ) {

  return 1;

}

int activeCoefTableClass::activate (
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

      if ( !readPvExpStr.getExpanded() ||
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
           coefTable_monitor_read_connect_state, this );
	}
	else {
          fprintf( stderr, activeCoefTableClass_str22 );
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

int activeCoefTableClass::deactivate (
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
         coefTable_monitor_read_connect_state, this );
        readPvId->remove_value_callback(
         coefTable_readUpdate, this );
	readPvId->release();
	readPvId = NULL;
      }
    }

  }

  return 1;

}

int activeCoefTableClass::expandTemplate (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

expStringClass tmpStr;

  tmpStr.setRaw( readPvExpStr.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  readPvExpStr.setRaw( tmpStr.getExpanded() );

  tmpStr.setRaw( labelsExpStr.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  labelsExpStr.setRaw( tmpStr.getExpanded() );

  tmpStr.setRaw( formatExpStr.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  formatExpStr.setRaw( tmpStr.getExpanded() );

  return 1;

}

int activeCoefTableClass::expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int stat, retStat = 1;

  stat = readPvExpStr.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = labelsExpStr.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = formatExpStr.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;

  return retStat;

}

int activeCoefTableClass::expand2nd (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int stat, retStat = 1;;

  stat = readPvExpStr.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = labelsExpStr.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = formatExpStr.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;

  return retStat;

}

int activeCoefTableClass::containsMacros ( void ) {

int stat, retStat = 1;

  stat = readPvExpStr.containsPrimaryMacros();
  if ( !( stat & 1 ) ) retStat = stat;

  stat = labelsExpStr.containsPrimaryMacros();
  if ( !( stat & 1 ) ) retStat = stat;

  stat = formatExpStr.containsPrimaryMacros();
  if ( !( stat & 1 ) ) retStat = stat;

  return retStat;

}

int activeCoefTableClass::createTableWidgets ( void ) {

  frameWidget = XtVaCreateManagedWidget( "", xmBulletinBoardWidgetClass,
   actWin->executeWidgetId(),
   XmNx, x,
   XmNy, y,
   XmNwidth, 0,
   XmNheight, 0,
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
    fprintf( stderr, activeCoefTableClass_str24 );
    return 0;
  }

  XtRealizeWidget( frameWidget );

  if ( enabled ) {
    XtMapWidget( frameWidget );
  }

  return 1;

}

int activeCoefTableClass::checkResizeSelectBox (
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

int activeCoefTableClass::checkResizeSelectBoxAbs (
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

void activeCoefTableClass::executeDeferred ( void ) {

int nc, nu, nd, i, max, numLabels, first, last, num;
tagClass tag;
int numCols;
char headerAlignStr[7+1];
char alignStr[7+1];
char buf[MaxLabelSize+1];
char val[255+1];
char *tk, *ctx;
Widget wdgt;
const double *darray;
const int *larray;
const char *carray;

//----------------------------------------------------------------------------

  actWin->appCtx->proc->lock();
  nc = needConnectInit; needConnectInit = 0;
  nu = needUpdate; needUpdate = 0;
  nd = needDraw; needDraw = 0;
  actWin->remDefExeNode( aglPtr );
  actWin->appCtx->proc->unlock();

  if ( !activeMode ) return;

//----------------------------------------------------------------------------

  if ( nc ) {

    readPvConnected = 1;
    active = 1;
    init = 1;

    arraySize = readPvId->get_dimension();

    if ( initialReadConnection ) {

      initialReadConnection = 0;

      readPvId->add_value_callback( coefTable_readUpdate, this );

    }

    fgColor.setConnected();
    drawActive();

  }

  if ( nu ) {

    table.destroy(); // its always safe to call destroy

    numCols = 2;
    strcpy( headerAlignStr, "rl" );
    strcpy( alignStr, "rl" );

    table.create( frameWidget, 0, 0, w, h, numCols, headerAlignStr,
     alignStr, actWin->fi, fontTag, fgColor.pixelColor(),
     bgColor.pixelColor(), oddBgColor.pixelColor(),
     evenBgColor.pixelColor(), topShadowColor.pixelColor(),
     botShadowColor.pixelColor() );

    // count num labels
    numLabels = 0;
    strncpy( buf, labelsExpStr.getExpanded(), MaxLabelSize );
    buf[MaxLabelSize] = 0;
    ctx = NULL;
    tk = strtok_r( buf, ",", &ctx );
    while ( tk ) {
      numLabels++;
      tk = strtok_r( NULL, ",", &ctx );
    }

    strncpy( buf, labelsExpStr.getExpanded(), MaxLabelSize );
    buf[MaxLabelSize] = 0;
    ctx = NULL;
    tk = strtok_r( buf, ",", &ctx );

    carray = NULL;
    larray = NULL;
    darray = NULL;

    switch ( readPvId->get_specific_type().type ) {

    case ProcessVariable::specificType::flt:
    case ProcessVariable::specificType::real:
      darray = readPvId->get_double_array();
      break;

    case ProcessVariable::specificType::shrt:
    case ProcessVariable::specificType::integer:
      larray = readPvId->get_int_array();
      break;

    case ProcessVariable::specificType::chr:
      carray = readPvId->get_char_array();
      break;

    default:
      break;

    }

    max = readPvId->get_dimension();
    if ( max > 1000 ) max = 1000;

    first = firstEle;
    if ( first < 0 ) first = 0;
    if ( first > max-1 ) first = max - 1;

    if ( numEle == 0 ) {
      num = numLabels;
      if ( num == 0 ) num = max;
    }
    else {
      num = numEle;
    }

    last = first + num;
    if ( last > max ) last = max;

    for ( i=first; i<last; i++ ) {

      if ( tk ) {
        wdgt = table.addCell( tk );
      }
      else {
        snprintf( val, 255, "Coef %-d", i );
        wdgt = table.addCell( val );
      }

      switch ( readPvId->get_specific_type().type ) {

      case ProcessVariable::specificType::flt:
      case ProcessVariable::specificType::real:
        if ( darray ) {
          if ( !blank( formatExpStr.getExpanded() ) ) {
	    //printf( "1 using format [%s]\n", formatExpStr.getExpanded() );
            snprintf( val, 255, formatExpStr.getExpanded(), darray[i] );
	  }
	  else {
            snprintf( val, 255, "%-g", darray[i] );
	  }
        }
        else {
          strcpy( val, "Error" );
        }
        break;

      case ProcessVariable::specificType::shrt:
      case ProcessVariable::specificType::integer:
        if ( larray ) {
          if ( !blank( formatExpStr.getExpanded() ) ) {
	    //printf( "2 using format [%s]\n", formatExpStr.getExpanded() );
            snprintf( val, 255, formatExpStr.getExpanded(), larray[i] );
	  }
	  else {
            snprintf( val, 255, "%-d", larray[i] );
	  }

        }
        else {
          strcpy( val, "Error" );
        }
        break;

      case ProcessVariable::specificType::chr:
        if ( carray ) {
          if ( !blank( formatExpStr.getExpanded() ) ) {
	    //printf( "3 using format [%s]\n", formatExpStr.getExpanded() );
            snprintf( val, 255, formatExpStr.getExpanded(), (int) carray[i] );
	  }
	  else {
            snprintf( val, 255, "%-d", (int) carray[i] );
	  }
        }
        else {
          strcpy( val, "Error" );
        }
        break;

      default:
        strcpy( val, "Unsupported type" );
        break;


      }

      wdgt = table.addCell( val );

      tk = strtok_r( NULL, ",", &ctx );

    }

    table.endOfContent();

  }

//----------------------------------------------------------------------------

  if ( nd ) {
    drawActive();
  }

//----------------------------------------------------------------------------

}

void activeCoefTableClass::changeDisplayParams (
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

void activeCoefTableClass::changePvNames (
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

void activeCoefTableClass::map ( void ) {

  if ( frameWidget ) {
    XtMapWidget( frameWidget );
  }

}

void activeCoefTableClass::unmap ( void ) {

  if ( frameWidget ) {
    XtUnmapWidget( frameWidget );
  }

}

void activeCoefTableClass::getPvs (
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

char *activeCoefTableClass::getSearchString (
  int i
) {

  if ( i == 0 ) {
    return readPvExpStr.getRaw();
  }
  else if ( i == 1 ) {
    return labelsExpStr.getRaw();
  }
  else if ( i == 2 ) {
    return formatExpStr.getRaw();
  }

  return NULL;

}

void activeCoefTableClass::replaceString (
  int i,
  int max,
  char *string
) {

  if ( i == 0 ) {
    readPvExpStr.setRaw( string );
  }
  else if ( i == 1 ) {
    labelsExpStr.setRaw( string );
  }
  else if ( i == 2 ) {
    formatExpStr.setRaw( string );
  }

}

// crawler functions may return blank pv names
char *activeCoefTableClass::crawlerGetFirstPv ( void ) {

  crawlerPvIndex = 0;
  return readPvExpStr.getExpanded();

}

char *activeCoefTableClass::crawlerGetNextPv ( void ) {

  return NULL;

}

#ifdef __cplusplus
extern "C" {
#endif

void *create_activeCoefTableClassPtr ( void ) {

activeCoefTableClass *ptr;

  ptr = new activeCoefTableClass;
  return (void *) ptr;

}

void *clone_activeCoefTableClassPtr (
  void *_srcPtr )
{

activeCoefTableClass *ptr, *srcPtr;

  srcPtr = (activeCoefTableClass *) _srcPtr;

  ptr = new activeCoefTableClass( srcPtr );

  return (void *) ptr;

}

#ifdef __cplusplus
}
#endif
