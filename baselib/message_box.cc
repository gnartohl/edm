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

#define __message_box_cc 1

#include "message_box.h"
#include <sys/stat.h>
#include <unistd.h>
#include "app_pkg.h"
#include "act_win.h"

#include "thread.h"

static void messageboxc_flush_log_file (
  XtPointer client,
  XtIntervalId *id )
{

activeMessageBoxClass *messageboxo = (activeMessageBoxClass *) client;

  if ( messageboxo->logFileExists ) fflush( messageboxo->logFile );

  messageboxo->flushTimer = appAddTimeOut(
   messageboxo->actWin->appCtx->appContext(),
   messageboxo->flushTimerValue*1000, messageboxc_flush_log_file, client );

}

static void messageboxc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeMessageBoxClass *messageboxo = (activeMessageBoxClass *) client;

  messageboxo->actWin->setChanged();

  messageboxo->eraseSelectBoxCorners();
  messageboxo->erase();

  messageboxo->fgColor.setColorIndex(
   messageboxo->bufFgColor, messageboxo->actWin->ci );

  messageboxo->bgColor.setColorIndex(
   messageboxo->bufBgColor, messageboxo->actWin->ci );

  messageboxo->bg2Color.setColorIndex(
   messageboxo->bufBg2Color, messageboxo->actWin->ci );

  messageboxo->topShadowColor.setColorIndex(
   messageboxo->bufTopShadowColor, messageboxo->actWin->ci );

  messageboxo->botShadowColor.setColorIndex(
   messageboxo->bufBotShadowColor, messageboxo->actWin->ci );

  messageboxo->readPvExpStr.setRaw( messageboxo->eBuf->bufReadPvName );

  strncpy( messageboxo->fontTag, messageboxo->fm.currentFontTag(), 63 );
  messageboxo->actWin->fi->loadFontTag( messageboxo->fontTag );
  messageboxo->fs =
   messageboxo->actWin->fi->getXFontStruct( messageboxo->fontTag );
  messageboxo->actWin->drawGc.setFontTag(
   messageboxo->fontTag, messageboxo->actWin->fi );

  messageboxo->size = messageboxo->bufSize;

  messageboxo->fileSize = messageboxo->bufFileSize;

  messageboxo->fileIsReadOnly = messageboxo->bufFileIsReadOnly;

  messageboxo->logFileName.setRaw( messageboxo->eBuf->bufLogFileName );

  messageboxo->flushTimerValue = messageboxo->bufFlushTimerValue;
  if ( messageboxo->flushTimerValue < 5 ) messageboxo->flushTimerValue = 5;

  messageboxo->x = messageboxo->bufX;
  messageboxo->sboxX = messageboxo->bufX;

  messageboxo->y = messageboxo->bufY;
  messageboxo->sboxY = messageboxo->bufY;

  messageboxo->w = messageboxo->bufW;
  messageboxo->sboxW = messageboxo->bufW;

  messageboxo->h = messageboxo->bufH;
  messageboxo->sboxH = messageboxo->bufH;

  if ( messageboxo->h < messageboxo->minH ) {
    messageboxo->h = messageboxo->minH;
    messageboxo->sboxH = messageboxo->minH;
  }

}

static void messageboxc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeMessageBoxClass *messageboxo = (activeMessageBoxClass *) client;

  messageboxc_edit_update ( w, client, call );
  messageboxo->refresh( messageboxo );

}

static void messageboxc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeMessageBoxClass *messageboxo = (activeMessageBoxClass *) client;

  messageboxc_edit_update ( w, client, call );
  messageboxo->ef.popdown();
  messageboxo->operationComplete();

}

static void messageboxc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeMessageBoxClass *messageboxo = (activeMessageBoxClass *) client;

  messageboxo->ef.popdown();
  messageboxo->operationCancel();

}

static void messageboxc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeMessageBoxClass *messageboxo = (activeMessageBoxClass *) client;

  messageboxo->ef.popdown();
  messageboxo->operationCancel();
  messageboxo->erase();
  messageboxo->deleteRequest = 1;
  messageboxo->drawAll();

}

static void messagebox_monitor_read_connect_state (
  ProcessVariable *pv,
  void *userarg )
{

activeMessageBoxClass *messageboxo =
 (activeMessageBoxClass *) userarg;

  if ( pv->is_valid() ) {

    messageboxo->needConnectInit = 1;

  }
  else {

    messageboxo->readPvConnected = 0;
    messageboxo->active = 0;
    messageboxo->fgColor.setDisconnected();
    messageboxo->needDraw = 1;

  }

  messageboxo->actWin->appCtx->proc->lock();
  messageboxo->actWin->addDefExeNode( messageboxo->aglPtr );
  messageboxo->actWin->appCtx->proc->unlock();

}

static void messagebox_readUpdate (
  ProcessVariable *pv,
  void *userarg )
{

activeMessageBoxClass *messageboxo = (activeMessageBoxClass *) userarg;

  if ( messageboxo->active ) {

    if ( messageboxo->firstReadUpdate ) {
      messageboxo->firstReadUpdate = 0;
      return;
    }

    pv->get_string( messageboxo->curReadV, 39 );

    messageboxo->actWin->appCtx->proc->lock();
    messageboxo->needUpdate = 1;
    messageboxo->actWin->addDefExeNode( messageboxo->aglPtr );
    messageboxo->actWin->appCtx->proc->unlock();

  }

}

activeMessageBoxClass::activeMessageBoxClass ( void ) {

  name = new char[strlen("activeMessageBoxClass")+1];
  strcpy( name, "activeMessageBoxClass" );
  checkBaseClassVersion( activeGraphicClass::MAJOR_VERSION, name );
  minW = 200;
  minH = 100;
  strcpy( fontTag, "" );
  fs = NULL;
  activeMode = 0;
  size = 1000;
  fileSize = 100000;
  fileIsReadOnly = 1;
  flushTimerValue = 600;
  logFileOpen = 0;
  eBuf = NULL;

}

// copy constructor
activeMessageBoxClass::activeMessageBoxClass
 ( const activeMessageBoxClass *source ) {

activeGraphicClass *messageboxo = (activeGraphicClass *) this;

  messageboxo->clone( (activeGraphicClass *) source );

  name = new char[strlen("activeMessageBoxClass")+1];
  strcpy( name, "activeMessageBoxClass" );

  fgCb = source->fgCb;
  bgCb = source->bgCb;
  bg2Cb = source->bg2Cb;
  topCb = source->topCb;
  botCb = source->botCb;

  strncpy( fontTag, source->fontTag, 63 );
  fs = actWin->fi->getXFontStruct( fontTag );

  fgColor.copy( source->fgColor );
  bgColor.copy( source->bgColor );
  bg2Color.copy( source->bg2Color );
  topShadowColor.copy( source->topShadowColor );
  botShadowColor.copy( source->botShadowColor );

  readPvExpStr.copy( source->readPvExpStr );

  minW = 50;
  minH = 20;
  activeMode = 0;
  logFileOpen = 0;

  size = source->size;
  fileSize = source->fileSize;
  fileIsReadOnly = source->fileIsReadOnly;
  logFileName.copy( source->logFileName );
  flushTimerValue = source->flushTimerValue;

  eBuf = NULL;

  doAccSubs( readPvExpStr );
  doAccSubs( logFileName );

}

int activeMessageBoxClass::rotateLogFile ( void ) {

char newName[256];
int stat;

// fprintf( stderr, "in rotateLogFile\n" );

  if ( logFileExists ) {

    strncpy( newName, logFileName.getExpanded(), 255 );
    Strncat( newName, "_2", 255 );

    //fprintf( stderr, "unlink %s\n", newName );
    stat = unlink( newName );

    fclose( logFile );

    //fprintf( stderr, "rename %s to %s\n", logFileName.getExpanded(), newName );
    stat = rename( logFileName.getExpanded(), newName );
    if ( stat < 0 ) {
      fprintf( stderr, activeMessageBoxClass_str1, logFileName.getExpanded(), newName );
    }

    logFile = fopen( logFileName.getExpanded(), "a" );
    if ( !logFile ) {
      logFileExists = 0;
      logFileOpen = 0;
    }
    else {
      logFileOpen = 1;
    }
    curFileSize = 0;

    if ( logFileOpen ) {
      stat = lockFile( logFile );
      if ( !( stat & 1 ) ) {
        fprintf( stderr, activeMessageBoxClass_str2 );
        fclose( logFile );
        logFileExists = 0;
        logFileOpen = 0;
      }
    }
    else {
      fprintf( stderr, activeMessageBoxClass_str3 );
    }

  }

  return 1;

}

int activeMessageBoxClass::createInteractive (
  activeWindowClass *aw_obj,
  int _x,
  int _y,
  int _w,
  int _h ) {

  actWin = (activeWindowClass *) aw_obj;

  if ( _w < minW )
    w = minW;
  else
    w = _w;

  if ( _h < minH )
    h = minH;
  else
    h = _h;

  x = _x;
  y = _y;

  fgColor.setColorIndex( actWin->defaultTextFgColor, actWin->ci );
  bg2Color.setColorIndex( actWin->defaultBgColor, actWin->ci );
  bgColor.setColorIndex( actWin->defaultOffsetColor, actWin->ci );
  topShadowColor.setColorIndex( actWin->defaultTopShadowColor, actWin->ci );
  botShadowColor.setColorIndex( actWin->defaultBotShadowColor, actWin->ci );

  strcpy( fontTag, actWin->defaultFontTag );
  actWin->fi->loadFontTag( fontTag );
  fs = actWin->fi->getXFontStruct( fontTag );

  this->draw();

  this->editCreate();

  return 1;

}

int activeMessageBoxClass::save (
  FILE *f )
{


int stat, major, minor, release;

tagClass tag;

int zero = 0;
char *emptyStr = "";

  major = MESSAGEBOXC_MAJOR_VERSION;
  minor = MESSAGEBOXC_MINOR_VERSION;
  release = MESSAGEBOXC_RELEASE;

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
  tag.loadW( "2ndBgColor", actWin->ci, &bg2Color );
  tag.loadW( "topShadowColor", actWin->ci, &topShadowColor );
  tag.loadW( "botShadowColor", actWin->ci, &botShadowColor );
  tag.loadW( "indicatorPv", &readPvExpStr, emptyStr );
  tag.loadW( "font", fontTag );
  tag.loadW( "bufferSize", &size );
  tag.loadW( "fileSize", &fileSize, &zero );
  tag.loadW( "flushTimerValue", &flushTimerValue, &zero );
  tag.loadW( "logFileName", &logFileName, emptyStr );
  tag.loadBoolW( "readOnly", &fileIsReadOnly, &zero );
  tag.loadW( unknownTags );
  tag.loadW( "endObjectProperties" );
  tag.loadW( "" );

  stat = tag.writeTags( f );

  return stat;

}

int activeMessageBoxClass::old_save (
  FILE *f )
{

int index;

  fprintf( f, "%-d %-d %-d\n", MESSAGEBOXC_MAJOR_VERSION,
   MESSAGEBOXC_MINOR_VERSION, MESSAGEBOXC_RELEASE );

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

  index = bg2Color.pixelIndex();
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

  fprintf( f, "%-d\n", size );

  fprintf( f, "%-d\n", fileSize );

  fprintf( f, "%-d\n", flushTimerValue );

  if ( logFileName.getRaw() )
    writeStringToFile( f, logFileName.getRaw() );
  else
    writeStringToFile( f, "" );

  // version 2.1.0
  fprintf( f, "%-d\n", fileIsReadOnly );

  return 1;

}

int activeMessageBoxClass::createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int major, minor, release, stat;

tagClass tag;

int zero = 0;
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
  tag.loadR( "2ndBgColor", actWin->ci, &bg2Color );
  tag.loadR( "topShadowColor", actWin->ci, &topShadowColor );
  tag.loadR( "botShadowColor", actWin->ci, &botShadowColor );
  tag.loadR( "indicatorPv", &readPvExpStr, emptyStr );
  tag.loadR( "font", 63, fontTag );
  tag.loadR( "bufferSize", &size );
  tag.loadR( "fileSize", &fileSize, &zero );
  tag.loadR( "flushTimerValue", &flushTimerValue, &zero );
  tag.loadR( "logFileName", &logFileName, emptyStr );
  tag.loadR( "readOnly", &fileIsReadOnly, &zero );
  tag.loadR( "endObjectProperties" );
  tag.loadR( "" );

  stat = tag.readTags( f, "endObjectProperties" );

  if ( !( stat & 1 ) ) {
    actWin->appCtx->postMessage( tag.errMsg() );
  }

  if ( major > MESSAGEBOXC_MAJOR_VERSION ) {
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

  logFileOpen = 0;

  return stat;

}

int activeMessageBoxClass::old_createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int r, g, b, index;
int major, minor, release;
unsigned int pixel;
char oneName[PV_Factory::MAX_PV_NAME+1];
char oneFileName[127+1];

  this->actWin = _actWin;

  fscanf( f, "%d %d %d\n", &major, &minor, &release );

  if ( major > MESSAGEBOXC_MAJOR_VERSION ) {
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
    bg2Color.setColorIndex( index, actWin->ci );

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
    bg2Color.setColorIndex( index, actWin->ci );

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
    bg2Color.setColorIndex( index, actWin->ci );

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

  fscanf( f, "%d\n", &size );

  fscanf( f, "%d\n", &fileSize );

  fscanf( f, "%d\n", &flushTimerValue );

  readStringFromFile( oneFileName, 127+1, f );
  logFileName.setRaw( oneFileName );

  if ( ( major > 2 ) || ( ( major == 2 ) && ( minor == 1 ) ) ) {
    fscanf( f, "%d\n", &fileIsReadOnly );
  }
  else {
    fileIsReadOnly = 0;
  }

  logFileOpen = 0;

  return 1;

}

int activeMessageBoxClass::genericEdit ( void ) {

char title[32], *ptr;

  if ( !eBuf ) {
    eBuf = new editBufType;
  }

  ptr = actWin->obj.getNameFromClass( "activeMessageBoxClass" );
  if ( ptr )
    strncpy( title, ptr, 31 );
  else
    strncpy( title, activeMessageBoxClass_str4, 31 );

  Strncat( title, activeMessageBoxClass_str5, 31 );

  bufX = x;
  bufY = y;
  bufW = w;
  bufH = h;

  bufFgColor = fgColor.pixelIndex();
  bufBgColor = bgColor.pixelIndex();
  bufBg2Color = bg2Color.pixelIndex();
  bufTopShadowColor = topShadowColor.pixelIndex();
  bufBotShadowColor = botShadowColor.pixelIndex();

  if ( readPvExpStr.getRaw() )
    strncpy( eBuf->bufReadPvName, readPvExpStr.getRaw(),
     PV_Factory::MAX_PV_NAME );
  else
    strcpy( eBuf->bufReadPvName, "" );

  bufSize = size;
  bufFileSize = fileSize;
  bufFileIsReadOnly = fileIsReadOnly;
  bufFlushTimerValue = flushTimerValue;

  if ( logFileName.getRaw() )
    strncpy( eBuf->bufLogFileName, logFileName.getRaw(), 127 );
  else
    strcpy( eBuf->bufLogFileName, "" );

  ef.create( actWin->top, actWin->appCtx->ci.getColorMap(),
   &actWin->appCtx->entryFormX,
   &actWin->appCtx->entryFormY, &actWin->appCtx->entryFormW,
   &actWin->appCtx->entryFormH, &actWin->appCtx->largestH,
   title, NULL, NULL, NULL );

  ef.addTextField( activeMessageBoxClass_str6, 35, &bufX );
  ef.addTextField( activeMessageBoxClass_str7, 35, &bufY );
  ef.addTextField( activeMessageBoxClass_str8, 35, &bufW );
  ef.addTextField( activeMessageBoxClass_str9, 35, &bufH );
  ef.addTextField( activeMessageBoxClass_str10, 35, &bufSize );
  ef.addTextField( activeMessageBoxClass_str11, 35, eBuf->bufReadPvName,
   PV_Factory::MAX_PV_NAME );
  ef.addTextField( activeMessageBoxClass_str12, 35, eBuf->bufLogFileName, 127 );
  ef.addToggle( activeMessageBoxClass_str26, &bufFileIsReadOnly );
  ef.addTextField( activeMessageBoxClass_str13, 35, &bufFileSize );
  ef.addTextField( activeMessageBoxClass_str14, 35, &bufFlushTimerValue );
  ef.addColorButton( activeMessageBoxClass_str16, actWin->ci, &fgCb, &bufFgColor );
  ef.addColorButton( activeMessageBoxClass_str17, actWin->ci, &bg2Cb, &bufBg2Color );
  ef.addColorButton( activeMessageBoxClass_str18, actWin->ci, &bgCb, &bufBgColor );
  ef.addColorButton( activeMessageBoxClass_str19, actWin->ci, &topCb,
   &bufTopShadowColor );
  ef.addColorButton( activeMessageBoxClass_str20, actWin->ci, &botCb,
   &bufBotShadowColor );
  ef.addFontMenu( activeMessageBoxClass_str15, actWin->fi, &fm, fontTag );

  XtUnmanageChild( fm.alignWidget() ); // no alignment info

  return 1;

}

int activeMessageBoxClass::editCreate ( void ) {

  this->genericEdit();
  ef.finished( messageboxc_edit_ok, messageboxc_edit_apply, messageboxc_edit_cancel_delete, this );
  actWin->currentEf = NULL;
  ef.popup();

  return 1;

}

int activeMessageBoxClass::edit ( void ) {

  this->genericEdit();
  ef.finished( messageboxc_edit_ok, messageboxc_edit_apply, messageboxc_edit_cancel, this );
  actWin->currentEf = &ef;
  ef.popup();

  return 1;

}

int activeMessageBoxClass::erase ( void ) {

  if ( deleteRequest ) return 1;

  XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  return 1;

}

int activeMessageBoxClass::eraseActive ( void ) {

  return 1;

}

int activeMessageBoxClass::draw ( void ) {

int textHeight, h1;

  if ( fs )
    textHeight = fs->ascent + fs->descent;
  else
    textHeight = 10;

  if ( deleteRequest ) return 1;
  actWin->drawGc.saveFg();

  actWin->drawGc.setFG( bg2Color.pixelColor() );
  XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x, y, w, h );

  actWin->drawGc.setFG( bgColor.pixelColor() );
  XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x+8, y+textHeight+40, w-28, h-textHeight-40-20 );

  actWin->drawGc.setFG( fgColor.pixelColor() );
  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x, y, w, textHeight+32 );

  actWin->drawGc.setFG( fgColor.pixelColor() );
  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x+w-50, y+10, 40, textHeight+10 );

  actWin->drawGc.setFG( fgColor.pixelColor() );
  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x+8, y+h-16, w-28, 12 );

  actWin->drawGc.setFG( fgColor.pixelColor() );
  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x+w-16, y+textHeight+40, 12, h-textHeight-40-20 );

  actWin->drawGc.setFG( fgColor.pixelColor() );
  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x, y, w, h );

  actWin->drawGc.setFontTag( fontTag, actWin->fi );

  h1 = h-textHeight-40-20;

  drawText( actWin->drawWidget, &actWin->drawGc,
   fs, x+w/2, y+textHeight+40+h1/2, XmALIGNMENT_CENTER, activeMessageBoxClass_str21 );

  actWin->drawGc.restoreFg();

  return 1;

}

int activeMessageBoxClass::drawActive ( void ) {

int n;
Arg args[10];

  if ( !enabled || !activeMode || !init ) return 1;

  if ( scrolledText.textWidget() ) {
    n = 0;
    XtSetArg( args[n], XmNforeground, fgColor.getColor() ); n++;
    XtSetValues( scrolledText.textWidget(), args, n );
  }

  return 1;

}

int activeMessageBoxClass::activate (
  int pass,
  void *ptr )
{

int status, opStat, thisFileSize, discard, l;
char *gotSome, line[256];
struct stat fileStat;

  switch ( pass ) {

  case 1:

    opComplete = 0;

    break;

  case 2:

    if ( !opComplete ) {

      curFileSize = 0;

      if ( strcmp( logFileName.getExpanded(), "" ) != 0 ) {

        status = stat( logFileName.getExpanded(), &fileStat );
        if ( status == 0 ) {
          thisFileSize = fileStat.st_size;
        }
        else {
          thisFileSize = 0;
        }

        logFileExists = 1;

        if ( thisFileSize ) {

          if ( thisFileSize > size )
            discard = thisFileSize - size;
          else
            discard = 0;

          logFile = fopen( logFileName.getExpanded(), "r" );
          if ( logFile ) {
            do {
              gotSome = fgets( line, 255, logFile );
              if ( gotSome ) {
                l = strlen( line );
                curFileSize += l;
                if ( discard > 0 ) {
                  discard -= l;
                }
                else {
                  scrolledText.addText( line );
                }
              }
            } while ( gotSome );
            fclose( logFile );
          }

        }

        logFileOpen = 0;

        if ( !fileIsReadOnly ) {

          logFile = fopen( logFileName.getExpanded(), "a" );
          if ( !logFile ) {
            logFileExists = 0;
            logFileOpen = 0;
          }
          else {
            logFileOpen = 1;
          }

          if ( logFileOpen ) {
            status = lockFile( logFile );
            if ( !( status & 1 ) ) {
              fclose( logFile );
              logFileExists = 0;
              logFileOpen = 0;
            }
          }

        }
        else {

          logFileExists = 0;

        }

      }
      else {

        logFileExists = 0;

      }

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

      createMessageBoxWidgets();

      if ( readExists ) {
        readPvId = the_PV_Factory->create( readPvExpStr.getExpanded() );
	if ( readPvId ) {
	  readPvId->add_conn_state_callback(
           messagebox_monitor_read_connect_state, this );
	}
	else {
          fprintf( stderr, activeMessageBoxClass_str22 );
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

      if ( logFileExists ) {
        flushTimer = appAddTimeOut( actWin->appCtx->appContext(),
         flushTimerValue*1000, messageboxc_flush_log_file, (void *) this );
      }

    }

    break;

  }

  return 1;

}

int activeMessageBoxClass::deactivate (
  int pass
) {

  if ( pass == 1 ) {

    active = 0;
    activeMode = 0;

    scrolledText.destroyEmbedded();
    if ( frameWidget ) XtDestroyWidget( frameWidget );
    frameWidget = NULL;

    if ( logFileExists ) {
      XtRemoveTimeOut( flushTimer );
    }

    if ( readExists ) {
      if ( readPvId ) {
        readPvId->remove_conn_state_callback(
         messagebox_monitor_read_connect_state, this );
        readPvId->remove_value_callback(
         messagebox_readUpdate, this );
	readPvId->release();
	readPvId = NULL;
      }
    }

    if ( logFileOpen ) {
      fclose( logFile );
      logFileOpen = 0;
    }

  }

  return 1;

}

int activeMessageBoxClass::expandTemplate (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

expStringClass tmpStr;

  tmpStr.setRaw( readPvExpStr.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  readPvExpStr.setRaw( tmpStr.getExpanded() );

  tmpStr.setRaw( logFileName.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  logFileName.setRaw( tmpStr.getExpanded() );

  return 1;

}

int activeMessageBoxClass::expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int retStat, stat;

  retStat = 1;

  stat = readPvExpStr.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;

  stat = logFileName.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;

  return retStat;

}

int activeMessageBoxClass::expand2nd (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int retStat, stat;

  retStat = 1;

  stat = readPvExpStr.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;

  stat = logFileName.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;

  return retStat;

}

int activeMessageBoxClass::containsMacros ( void ) {

  if ( readPvExpStr.containsPrimaryMacros()  ) return 1;
  if ( logFileName.containsPrimaryMacros()  ) return 1;

  return 0;

}

int activeMessageBoxClass::createMessageBoxWidgets ( void ) {

int n, textHeight;
Arg args[10];
Widget widget;

  frameWidget = XtVaCreateManagedWidget( "", xmFrameWidgetClass,
   actWin->executeWidgetId(),
   XmNx, x,
   XmNy, y,
   XmNmarginWidth, 0,
   XmNmarginHeight, 0,
   XmNshadowType, XmSHADOW_ETCHED_OUT,
   XmNmappedWhenManaged, False,
   //XmNmappedWhenManaged, True,
   NULL );

  if ( !frameWidget ) {
    fprintf( stderr, activeMessageBoxClass_str24 );
    return 0;
  }

  if ( fs )
    textHeight = fs->ascent + fs->descent;
  else
    textHeight = 10;

  scrolledText.createEmbeddedWH( frameWidget, x, y, w-8,
  h-textHeight-40, size, actWin->fi, fontTag );

  n = 0;
  XtSetArg( args[n], XmNbackground, bg2Color.pixelColor() ); n++;
  XtSetArg( args[n], XmNtopShadowColor, topShadowColor.pixelColor() ); n++;
  XtSetArg( args[n], XmNbottomShadowColor, botShadowColor.pixelColor() );n++;
  XtSetValues( frameWidget, args, n );

  n = 0;
  XtSetArg( args[n], XmNbackground, bg2Color.pixelColor() ); n++;
  XtSetArg( args[n], XmNtopShadowColor, topShadowColor.pixelColor() ); n++;
  XtSetArg( args[n], XmNbottomShadowColor, botShadowColor.pixelColor() );n++;
  XtSetValues( scrolledText.paneWidget(), args, n );

  n = 0;
  XtSetArg( args[n], XmNbackground, bg2Color.pixelColor() ); n++;
  XtSetArg( args[n], XmNtopShadowColor, topShadowColor.pixelColor() ); n++;
  XtSetArg( args[n], XmNbottomShadowColor, botShadowColor.pixelColor() );n++;
  XtSetValues( scrolledText.formWidget(), args, n );

  n = 0;
  XtSetArg( args[n], XmNbackground, bgColor.pixelColor() ); n++;
  XtSetArg( args[n], XmNtopShadowColor, topShadowColor.pixelColor() ); n++;
  XtSetArg( args[n], XmNbottomShadowColor, botShadowColor.pixelColor() );n++;
  XtSetValues( scrolledText.textWidget(), args, n );

  widget = XtParent( scrolledText.textWidget() );
  if ( widget ) {
    n = 0;
    XtSetArg( args[n], XmNbackground, bg2Color.pixelColor() ); n++;
    XtSetValues( widget, args, n );
  }

  n = 0;
  XtSetArg( args[n], XmNbackground, bg2Color.pixelColor() ); n++;
  XtSetArg( args[n], XmNtopShadowColor, topShadowColor.pixelColor() ); n++;
  XtSetArg( args[n], XmNbottomShadowColor, botShadowColor.pixelColor() );n++;
  XtSetValues( scrolledText.HorzScrollWidget(), args, n );

  n = 0;
  XtSetArg( args[n], XmNbackground, bg2Color.pixelColor() ); n++;
  XtSetArg( args[n], XmNtopShadowColor, topShadowColor.pixelColor() ); n++;
  XtSetArg( args[n], XmNbottomShadowColor, botShadowColor.pixelColor() );n++;
  XtSetValues( scrolledText.VertScrollWidget(), args, n );

  n = 0;
  XtSetArg( args[n], XmNforeground, fgColor.pixelColor() ); n++;
  XtSetArg( args[n], XmNtopShadowColor, topShadowColor.pixelColor() ); n++;
  XtSetArg( args[n], XmNbottomShadowColor, botShadowColor.pixelColor() );n++;
  XtSetArg( args[n], XmNhighlightColor, bg2Color.pixelColor() ); n++;
  XtSetArg( args[n], XmNbackground, bg2Color.pixelColor() ); n++;
  XtSetValues( scrolledText.clearPbWidget(), args, n );

  if ( enabled ) {
    XtMapWidget( frameWidget );
  }

  return 1;

}

int activeMessageBoxClass::checkResizeSelectBox (
  int _x,
  int _y,
  int _w,
  int _h ) {

int tmpw, tmph, ret_stat;

  tmpw = sboxW;
  tmph = sboxH;

  ret_stat = 1;

  tmpw += _w;
  if ( tmpw < minW ) {
    ret_stat = 0;
  }

  tmph += _h;
  if ( tmph < minH ) {
    ret_stat = 0;
  }

  return ret_stat;

}

int activeMessageBoxClass::checkResizeSelectBoxAbs (
  int _x,
  int _y,
  int _w,
  int _h ) {

int tmpw, tmph, ret_stat;

  ret_stat = 1;

  if ( _w != -1 ) {
    tmpw = _w;
    if ( tmpw < minW ) {
      ret_stat = 0;
    }
  }

  if ( _h != -1 ) {
    tmph = _h;
    if ( tmph < minH ) {
      ret_stat = 0;
    }
  }

  return ret_stat;

}

void activeMessageBoxClass::executeDeferred ( void ) {

char v[39+1];
int nc, nu, nd, l;

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

      readPvId->add_value_callback( messagebox_readUpdate, this );

    }

    fgColor.setConnected();
    drawActive();

  }

  if ( nu ) {

    strncpy( readV, v, 39 );

    if ( readV ) scrolledText.addTextNoNL( readV );

    if ( logFileExists ) {
      if ( readV ) {

        l = strlen( readV );
        curFileSize +=l;

        if ( curFileSize > fileSize )
          rotateLogFile();

        if ( l > 1 ) {
          if ( strcmp( &readV[l-1], "\n" ) == 0 ) {
            readV[l-1] = 0;
            fprintf( logFile, "%s\n", readV );
	  }
	  else {
            fprintf( logFile, "%s", readV );
	  }
	}
	else {
          fprintf( logFile, "%s", readV );
        }

      }
    }

  }

//----------------------------------------------------------------------------

  if ( nd ) {
    strncpy( readV, v, 39 );
    drawActive();
  }

//----------------------------------------------------------------------------

}

void activeMessageBoxClass::changeDisplayParams (
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

  if ( _flag & ACTGRF_BGCOLOR_MASK )
    bg2Color.setColorIndex( actWin->defaultBgColor, actWin->ci );

  if ( _flag & ACTGRF_OFFSETCOLOR_MASK )
    bgColor.setColorIndex( actWin->defaultOffsetColor, actWin->ci );

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

void activeMessageBoxClass::changePvNames (
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

void activeMessageBoxClass::map ( void ) {

  if ( frameWidget ) {
    XtMapWidget( frameWidget );
  }

}

void activeMessageBoxClass::unmap ( void ) {

  if ( frameWidget ) {
    XtUnmapWidget( frameWidget );
  }

}

void activeMessageBoxClass::getPvs (
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

char *activeMessageBoxClass::getSearchString (
  int i
) {

  if ( i == 0 ) {
    return readPvExpStr.getRaw();
  }
  else if ( i == 1 ) {
    return logFileName.getRaw();
  }

  return NULL;

}

void activeMessageBoxClass::replaceString (
  int i,
  int max,
  char *string
) {

  if ( i == 0 ) {
    readPvExpStr.setRaw( string );
  }
  else if ( i == 1 ) {
    logFileName.setRaw( string );
  }

}

// crawler functions may return blank pv names
char *activeMessageBoxClass::crawlerGetFirstPv ( void ) {

  crawlerPvIndex = 0;
  return readPvExpStr.getExpanded();

}

char *activeMessageBoxClass::crawlerGetNextPv ( void ) {

  return NULL;

}

#ifdef __cplusplus
extern "C" {
#endif

void *create_activeMessageBoxClassPtr ( void ) {

activeMessageBoxClass *ptr;

  ptr = new activeMessageBoxClass;
  return (void *) ptr;

}

void *clone_activeMessageBoxClassPtr (
  void *_srcPtr )
{

activeMessageBoxClass *ptr, *srcPtr;

  srcPtr = (activeMessageBoxClass *) _srcPtr;

  ptr = new activeMessageBoxClass( srcPtr );

  return (void *) ptr;

}

#ifdef __cplusplus
}
#endif
