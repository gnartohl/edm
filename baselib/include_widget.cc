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



// Cascade option is no longer used



#define __include_widget_cc 1

#define MAX_NOOF_MACROS 100

#include "utility.h"
#include "include_widget.h"
#include "app_pkg.h"
#include "act_win.h"

#include "thread.h"
#include "crc.h"

#include <string>
#include <list>
#include <iostream>

#ifdef TRIUMF
#endif

using namespace std;

// debug flags for selective printing
int includeWidgetClass::debug = 0;   // other debug
int includeWidgetClass::debugm = 0;  // debug macros
int includeWidgetClass::debugr = 0;  // debug recursive

// for detecting infinite recursion
string includeWidgetClass::repeatFileName = "";
int includeWidgetClass::includeLevel = 0;
int includeWidgetClass::doubleLevel = 0;
int includeWidgetClass::existLevel = -1;

static char *groupDragName = "?";

static void doBlink (
  void *ptr
) {

includeWidgetClass *iwo= (includeWidgetClass *) ptr;

  if ( !iwo->activeMode ) {
    if ( iwo->isSelected() ) iwo->drawSelectBoxCorners(); // erase via xor
    iwo->smartDrawAll();
    if ( iwo->isSelected() ) iwo->drawSelectBoxCorners();
  }
  else {
    iwo->bufInvalidate();
    iwo->smartDrawAllActive();
  }

}

static void incW_monitor_dest_connect_state (
  ProcessVariable *pv,
  void *userarg )
{

  objAndIndexType *ptr = (objAndIndexType *) userarg;
  includeWidgetClass *iwo= (includeWidgetClass *) ptr->obj;
  int i = ptr->index;

  if ( pv->is_valid() ) {

    if ( !iwo->connection.pvsConnected() ) {

      iwo->connection.setPvConnected( (void *) ptr->index );
      if ( iwo->connection.pvsConnected() ) {

        iwo->actWin->appCtx->proc->lock();
        iwo->destType[i] = (int) pv->get_type().type;
        iwo->needConnect = 1;
        iwo->actWin->addDefExeNode( iwo->aglPtr );
        iwo->actWin->appCtx->proc->unlock();

      }
      else {

        iwo->connection.setPvDisconnected( (void *) ptr->index );
        iwo->actWin->appCtx->proc->lock();
        iwo->needRefresh = 1;
        iwo->actWin->addDefExeNode( iwo->aglPtr );
        iwo->actWin->appCtx->proc->unlock();

      }

    }

  }

}

static void incW_monitor_color_connect_state (
  ProcessVariable *pv,
  void *userarg )
{

  includeWidgetClass *iwo= (includeWidgetClass *) userarg;

  if ( pv->is_valid() ) {

    if ( !iwo->connection.pvsConnected() ) {

      iwo->connection.setPvConnected( (void *) includeWidgetClass::NUMPVS );
      if ( iwo->connection.pvsConnected() ) {

        iwo->actWin->appCtx->proc->lock();
	      iwo->needConnect = 1;
        iwo->actWin->addDefExeNode( iwo->aglPtr );
        iwo->actWin->appCtx->proc->unlock();

      }

    }

  }
  else {

    iwo->connection.setPvDisconnected( (void *) includeWidgetClass::NUMPVS );
    iwo->fgColor.setDisconnected();
    iwo->actWin->appCtx->proc->lock();
    iwo->active = 0;
    iwo->needRefresh = 1;
    iwo->actWin->addDefExeNode( iwo->aglPtr );
    iwo->actWin->appCtx->proc->unlock();

  }

}

static void incW_color_value_update (
  ProcessVariable *pv,
  void *userarg )
{

  includeWidgetClass *iwo= (includeWidgetClass *) userarg;

  iwo->actWin->appCtx->proc->lock();
  iwo->needUpdate = 1;
  iwo->actWin->addDefExeNode( iwo->aglPtr );
  iwo->actWin->appCtx->proc->unlock();

}

static void iw_edit_ok1 (
  Widget w,
  XtPointer client,
  XtPointer call )
{

  includeWidgetClass *iwo= (includeWidgetClass *) client;
  iwo->ef1->popdownNoDestroy();

}

static void iw_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call )
{

  includeWidgetClass *iwo= (includeWidgetClass *) client;
  activeGraphicListPtr head;
  activeGraphicListPtr cur;
  int minX, maxX, minY, maxY;
  int ii, stat;

  iwo->actWin->setChanged();

  iwo->eraseSelectBoxCorners();
  iwo->erase();

  trimWhiteSpace( iwo->buf->bufIncludeFileName );
  strncpy( iwo->includeFileName, iwo->buf->bufIncludeFileName, 127 );
  iwo->drawFrame = iwo->buf->bufDrawFrame;
  iwo->symbolsExpStr.setRaw( iwo->buf->bufSymbols );
  ii = 1;

  if (includeWidgetClass::debugm) fprintf( stderr, "after edit, buf->bufSymbols = [%s]\n", iwo->buf->bufSymbols );

  iwo->fgColor.setColorIndex( iwo->buf->bufFgColor, iwo->actWin->ci );

  iwo->x = iwo->buf->bufX;

  iwo->sboxX = iwo->buf->bufX;

  iwo->y = iwo->buf->bufY;
  iwo->sboxY = iwo->buf->bufY;

  //iwo->w = iwo->buf->bufW;
  //iwo->sboxW = iwo->buf->bufW;

  //iwo->h = iwo->buf->bufH;
  //iwo->sboxH = iwo->buf->bufH;

  iwo->helpCommandExpString.setRaw( iwo->buf->bufHelpCommand );

  stat = iwo->readIncludeFile(iwo->getParentList());
  if ( !( stat & 1 ) ) {
    char msg[255+1];
    snprintf( msg, 255, includeWidgetClass_str50, iwo->actWin->fileName,
      iwo->includeFileName );
    iwo->actWin->appCtx->postMessage( msg );
  }
  else {
    
  // update w, h

    head = (activeGraphicListPtr) iwo->voidHead;

    cur = head->flink;
    if ( cur != head ) {
      minX = cur->node->getX0();
      maxX = cur->node->getX1();
      minY = cur->node->getY0();
      maxY = cur->node->getY1();
    }

  while ( cur != head ) {

    if ( cur->node->getX0() < minX ) minX = cur->node->getX0();
    if ( cur->node->getX1() > maxX ) maxX = cur->node->getX1();
    if ( cur->node->getY0() < minY ) minY = cur->node->getY0();
    if ( cur->node->getY1() > maxY ) maxY = cur->node->getY1();

    cur = cur->flink;

  }

  iwo->w = iwo->sboxW = maxX - iwo->x; // - minX;
  
  iwo->h = iwo->sboxH = maxY - iwo->y; // - minY;

  iwo->initSelectBox(); // call after getting x,y,w,h

  }
  
}

static void iw_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call )
{

  includeWidgetClass *iwo = (includeWidgetClass *) client;

  iw_edit_update ( w, client, call );
  iwo->refresh( iwo );

}

static void iw_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call )
{
  includeWidgetClass *iwo= (includeWidgetClass *) client;

  iw_edit_update ( w, client, call );
  iwo->ef.popdown();
  iwo->operationComplete();

  delete iwo->buf;
  iwo->buf = NULL;

}

static void iw_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call )
{

  includeWidgetClass *iwo= (includeWidgetClass *) client;

  iwo->ef.popdown();
  iwo->operationCancel();

  delete iwo->buf;
  iwo->buf = NULL;

}

static void iw_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call )
{

  includeWidgetClass *iwo= (includeWidgetClass *) client;

  delete iwo->buf;
  iwo->buf = NULL;

  iwo->ef.popdown();
  iwo->operationCancel();
  iwo->erase();
  iwo->deleteRequest = 1;
  iwo->drawAll();

}

includeWidgetClass::includeWidgetClass ( void ) {

  activeGraphicListPtr head;

  name = new char[strlen("includeWidgetClass")+1];
  strcpy( name, "includeWidgetClass" );
  checkBaseClassVersion( activeGraphicClass::MAJOR_VERSION, name );

  activeMode = 0;
  buf = NULL;

  head = new activeGraphicListType;
  head->flink = head;
  head->blink = head;

  voidHead = (void *) head;

  strcpy( includeFileName, "" );

  curCrawlerNode = NULL;
  curCrawlerState = GETTING_FIRST_CRAWLER_PV;

  relatedDisplayNodeHead = new RelatedDisplayNodeType;
  relatedDisplayNodeHead->flink = relatedDisplayNodeHead;
  relatedDisplayNodeHead->blink = relatedDisplayNodeHead;

  btnDownActionHead = new btnActionListType;
  btnDownActionHead->flink = btnDownActionHead;
  btnDownActionHead->blink = btnDownActionHead;

  btnUpActionHead = new btnActionListType;
  btnUpActionHead->flink = btnUpActionHead;
  btnUpActionHead->blink = btnUpActionHead;

  btnMotionActionHead = new btnActionListType;
  btnMotionActionHead->flink = btnMotionActionHead;
  btnMotionActionHead->blink = btnMotionActionHead;

  btnFocusActionHead = new btnActionListType;
  btnFocusActionHead->flink = btnFocusActionHead;
  btnFocusActionHead->blink = btnFocusActionHead;
  
  ofsX = 0;
  ofsY = 0;
  drawFrame = 0;
  aw = NULL;
  helpItem = -1;

  if (debug) fprintf(stderr, "creating includeWidgetClass\n");

}

includeWidgetClass::~includeWidgetClass ( void ) {

activeGraphicListPtr head;
activeGraphicListPtr cur, next;
btnActionListPtr curBtnAction, nextBtnAction;
RelatedDisplayNodePtr currd, nextrd;

  head = (activeGraphicListPtr) voidHead;

  cur = head->flink;
  while ( cur != head ) {
    next = cur->flink;
    delete cur->node;
    delete cur;
    cur = next;
  }
  head->flink = NULL;
  head->blink = NULL;
  delete head;

  currd = relatedDisplayNodeHead->flink;
  while ( currd != relatedDisplayNodeHead ) {
    nextrd = currd->flink;
    delete currd;
    currd = nextrd;
  }
  relatedDisplayNodeHead->flink = NULL;
  relatedDisplayNodeHead->blink = NULL;
  delete relatedDisplayNodeHead;

  // btn down action list

  curBtnAction = btnDownActionHead->flink;
  while ( curBtnAction != btnDownActionHead ) {
    nextBtnAction = curBtnAction->flink;
    //if ( curBtnAction->node ) delete curBtnAction->node;
    delete curBtnAction;
    curBtnAction = nextBtnAction;
  }
  btnDownActionHead->flink = NULL;
  btnDownActionHead->blink = NULL;
  delete btnDownActionHead;

  // btn up action list

  curBtnAction = btnUpActionHead->flink;
  while ( curBtnAction != btnUpActionHead ) {
    nextBtnAction = curBtnAction->flink;
    //if ( curBtnAction->node ) delete curBtnAction->node;
    delete curBtnAction;
    curBtnAction = nextBtnAction;
  }
  btnUpActionHead->flink = NULL;
  btnUpActionHead->blink = NULL;
  delete btnUpActionHead;

  // btn motion action list

  curBtnAction = btnMotionActionHead->flink;
  while ( curBtnAction != btnMotionActionHead ) {
    nextBtnAction = curBtnAction->flink;
    //if ( curBtnAction->node ) delete curBtnAction->node;
    delete curBtnAction;
    curBtnAction = nextBtnAction;
  }
  btnMotionActionHead->flink = NULL;
  btnMotionActionHead->blink = NULL;
  delete btnMotionActionHead;

  // btn focus action list

  curBtnAction = btnFocusActionHead->flink;
  while ( curBtnAction != btnFocusActionHead ) {
    nextBtnAction = curBtnAction->flink;
    //if ( curBtnAction->node ) delete curBtnAction->node;
    delete curBtnAction;
    curBtnAction = nextBtnAction;
  }
  btnFocusActionHead->flink = NULL;
  btnFocusActionHead->blink = NULL;
  delete btnFocusActionHead;

  if ( name ) delete[] name;

  if ( buf ) {
    delete buf;
    buf = NULL;
  }

}

// copy constructor
includeWidgetClass::includeWidgetClass
 ( const includeWidgetClass *source ) {

  activeGraphicClass *ago = (activeGraphicClass *) this;
  activeGraphicListPtr head, cur, curSource, sourceHead;
  int i;

  ago->clone( (activeGraphicClass *) source );

  name = new char[strlen("includeWidgetClass")+1];
  strcpy( name, "includeWidgetClass" );

  activeMode = 0;
  buf = NULL;

  deleteRequest = 0;

  head = new activeGraphicListType;
  head->flink = head;
  head->blink = head;

  sourceHead = (activeGraphicListPtr) source->voidHead;
  curSource = sourceHead->flink;
  while ( curSource != sourceHead ) {

  cur = new activeGraphicListType;
    cur->node = actWin->obj.clone( curSource->node->objName(),
     curSource->node );

    cur->blink = head->blink;
    head->blink->flink = cur;
    head->blink = cur;
    cur->flink = head;

    curSource = curSource->flink;

  }

  voidHead = (void *) head;
  
  curCrawlerNode = NULL;
  curCrawlerState = GETTING_FIRST_CRAWLER_PV;

  relatedDisplayNodeHead = new RelatedDisplayNodeType;
  relatedDisplayNodeHead->flink = relatedDisplayNodeHead;
  relatedDisplayNodeHead->blink = relatedDisplayNodeHead;

  btnDownActionHead = new btnActionListType;
  btnDownActionHead->flink = btnDownActionHead;
  btnDownActionHead->blink = btnDownActionHead;

  btnUpActionHead = new btnActionListType;
  btnUpActionHead->flink = btnUpActionHead;
  btnUpActionHead->blink = btnUpActionHead;

  btnMotionActionHead = new btnActionListType;
  btnMotionActionHead->flink = btnMotionActionHead;
  btnMotionActionHead->blink = btnMotionActionHead;

  btnFocusActionHead = new btnActionListType;
  btnFocusActionHead->flink = btnFocusActionHead;
  btnFocusActionHead->blink = btnFocusActionHead;

  fgColor.copy(source->fgColor);

  ofsX = source->ofsX;
  ofsY = source->ofsY;
  drawFrame = source->drawFrame;
  strcpy(includeFileName, source->includeFileName );
  symbolsExpStr.copy( source->symbolsExpStr );
  
  helpCommandExpString.copy( source->helpCommandExpString );
  helpItem = -1;

  doAccSubs( includeFileName, 127 );
  doAccSubs( symbolsExpStr );
  
}

void includeWidgetClass::setHelpItem ( void ) {

char *ctx, *tk, *err, buf[255+1];
int item;

  helpItem = -1;

  if ( !blank( helpCommandExpString.getExpanded() ) ) {

    strncpy( buf, helpCommandExpString.getExpanded(), 255 );
    buf[255] = 0;

    ctx = NULL;
    tk = strtok_r( buf, " \t", &ctx );

    if ( tk ) {

      if ( strcmp( tk, "item" ) == 0 ) {

        tk = strtok_r( NULL, " \t", &ctx );

        if ( tk ) {

          errno = 0;
          item = strtol( tk, &err, 0 );
          if ( !errno ) helpItem = item;

	}

      }

    }

  }

}

int includeWidgetClass::createInteractive (
  activeWindowClass *aw_obj,
  int _x,
  int _y,
  int _w,
  int _h ) {

char *ptr;

  actWin = (activeWindowClass *) aw_obj;
  x = sboxX= _x;
  y = sboxY = _y;
  w = sboxW = _w;
  h = sboxH = _h;

  if (debug) fprintf(stderr, "createInteractive\n");
  ptr = getenv( "EDMRDDHS" );
  if ( ptr ) {
    helpCommandExpString.setRaw( ptr );
  }

  fgColor.setColorIndex( actWin->defaultTextFgColor, actWin->ci );

  this->draw();

  this->editCreate();

  return 1;

}

int includeWidgetClass::save (
  FILE *f )
{

  int stat, numPvs, major, minor, release;

  tagClass tag;

  int zero = 0;
  char *emptyStr = (char *) "";

  if (debug) fprintf(stderr, "save\n");
  major = IW_MAJOR_VERSION;
  minor = IW_MINOR_VERSION;
  release = IW_RELEASE;

  numPvs = NUMPVS;

  tag.init();
  tag.loadW( (char *) "beginObjectProperties" );
  tag.loadW( (char *) "major", &major );
  tag.loadW( (char *) "minor", &minor );
  tag.loadW( (char *) "release", &release );
  tag.loadW( (char *) "x", &x );
  tag.loadW( (char *) "y", &y );
  tag.loadW( (char *) "w", &w );
  tag.loadW( (char *) "h", &h );
  tag.loadW( (char *) "fgColor", actWin->ci, &fgColor );
  tag.loadW( (char *) "includeFileName", includeFileName, emptyStr );
  tag.loadW( (char *) "symbols", &symbolsExpStr, emptyStr );
  tag.loadW( (char *) "drawFrame", &drawFrame, &zero );
  tag.loadW( (char *) "helpCommand", &helpCommandExpString, emptyStr );
  tag.loadW( unknownTags );
  tag.loadW( (char *) "endObjectProperties" );
  tag.loadW( (char *) "" );
  stat = tag.writeTags( f );
  if (debug) fprintf(stderr, "tags written\n");

  return stat;

}


int includeWidgetClass::createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{
  return createFromFile(f, name, _actWin, parentList);
}

int includeWidgetClass::createFromFile (
    FILE *f,
    char *name,
    activeWindowClass *_actWin,
    std::list<string>& parentListBefore )
  {

activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
activeGraphicListPtr cur;
int minX, maxX, minY, maxY;
int numPvs, stat, major, minor, release;
tagClass tag;

  int zero = 0;
  char *emptyStr = (char *) "";

  int readIncludeStat;

  major = IW_MAJOR_VERSION;
  minor = IW_MINOR_VERSION;
  release = IW_RELEASE;

  parentList = parentListBefore;

  numPvs = 0;
  this->actWin = _actWin;

  tag.init();
  tag.loadR( (char *) "beginObjectProperties" );
  tag.loadR( unknownTags );
  tag.loadR( (char *) "major", &major );
  tag.loadR( (char *) "minor", &minor );
  tag.loadR( (char *) "release", &release );
  tag.loadR( (char *) "x", &x );
  tag.loadR( (char *) "y", &y );
  tag.loadR( (char *) "w", &w );
  tag.loadR( (char *) "h", &h );
  tag.loadR( (char *) "fgColor", actWin->ci, &fgColor );
  tag.loadR( (char *) "includeFileName", 127, includeFileName, emptyStr );
  tag.loadR( (char *) "symbols", &symbolsExpStr, emptyStr );
  tag.loadR( (char *) "drawFrame", &drawFrame, &zero );
  tag.loadR( (char *) "helpCommand", &helpCommandExpString, emptyStr );
  tag.loadR( (char *) "endObjectProperties" );
  stat = tag.readTags( f, (char *) "endObjectProperties" );

  if ( !( stat & 1 ) ) {
    actWin->appCtx->postMessage( tag.errMsg() );
  }

  if ( major > IW_MAJOR_VERSION ) {
    postIncompatable();
    return 0;
  }

  if ( major < 4 ) {
    postIncompatable();
    return 0;
  }

  if (debugm) fprintf( stderr, "name = [%s]\n", name );
  if (debugm) fprintf( stderr, "symbolsExpStr (raw) = [%s]\n", symbolsExpStr.getRaw() );
  if (debugm) fprintf( stderr, "symbolsExpStr (exp) = [%s]\n", symbolsExpStr.getExpanded() );

  if (doubleLevel > 0 && includeLevel > 0) {
      return -1;
  }
  includeLevel++;       // 1 for top-level include widget
  existLevel = -1;
  if (debugr) {
      cerr << "trying to create include widget (level" << includeLevel << "): " << includeFileName << endl;
      cerr << "parent widgets" << endl;
  }
  std::list<string>::iterator it;
  int level = 1;
  for ( it = parentList.begin(); it != parentList.end(); ++it) {
    if (debugr)
      cout << "    " << *it << " include level: " << includeLevel << endl;
    if (*it == includeFileName) {
      existLevel = level;
      break;
    }
    level++;
  }

  if (existLevel <= 0) {
    if (debugr) {
        cerr << "Registering include file (level " << includeLevel << "): "<< includeFileName << endl;
    }
    parentList.push_back(includeFileName);
    doubleLevel = 0;
  } else {
    if (doubleLevel == 0) {
      includeLevel--;
      doubleLevel = includeLevel;
      repeatFileName = includeFileName;
      if (debugr) fprintf(stderr, "includeWidget::createFromFile *** infinite recursion *** - set doubleLevel %d\n", doubleLevel);
    }
    return -1;
  }
  readIncludeStat = readIncludeFile(parentList);

  // update w, h

    head = (activeGraphicListPtr) voidHead;

    cur = head->flink;
    if ( cur != head ) {
      minX = cur->node->getX0();
      maxX = cur->node->getX1();
      minY = cur->node->getY0();
      maxY = cur->node->getY1();
    }

  while ( cur != head ) {

    if ( cur->node->getX0() < minX ) minX = cur->node->getX0();
    if ( cur->node->getX1() > maxX ) maxX = cur->node->getX1();
    if ( cur->node->getY0() < minY ) minY = cur->node->getY0();
    if ( cur->node->getY1() > maxY ) maxY = cur->node->getY1();

    cur = cur->flink;

  }

  w = maxX - x; // - minX;
  h = maxY - y; // - minY;

  this->initSelectBox(); // call after getting x,y,w,h

  if (debugr) fprintf(stderr, "return from readInclude file status %d level %d\n", readIncludeStat, includeLevel);
  // generate message only at level where infinite recursion is detected
  if (readIncludeStat < 0 && doubleLevel == includeLevel) {
    char msg[255+1];
    snprintf( msg, 255, includeWidgetClass_str101, repeatFileName.c_str(), includeLevel+1, existLevel);
    actWin->appCtx->postMessage( msg );
  }
  includeLevel--;
  if (includeLevel == 0) {
    // reset everything do be ready for the next include widget at level 1
    if (debugr) fprintf(stderr, "Reset level counting\n");
    repeatFileName = "";
    includeLevel = 0;
    doubleLevel = 0;
    existLevel = -1;
  }

  if ( !( readIncludeStat & 1 ) ) {
    char msg[255+1];
    snprintf( msg, 255, includeWidgetClass_str50, actWin->fileName,
      includeFileName );
    actWin->appCtx->postMessage( msg );
    return 0;
  }

  return stat;

}

int includeWidgetClass::createSpecial (
  char *fname,
  activeWindowClass *_actWin )
{

int i;

 if (debug) fprintf( stderr, "includeWidgetClass::createSpecial\n" );

  x = -100;
  y = 0;
  w = 5;
  h = 5;

  this->actWin = _actWin;
  ofsX = ofsY = 0;
  drawFrame = 0;
  strcpy( includeFileName, fname );
  
  this->initSelectBox(); // call after getting x,y,w,h

  for ( i=0; i<NUMPVS; i++ ) {
    destPvExpString[i].setRaw( (char *) "" );
    sourceExpString[i].setRaw( (char *) "" );
  }

  return 1;

}

int includeWidgetClass::genericEdit ( void ) {

char title[32], *ptr;

  // fprintf(stderr, "genericEdit file %s buf %p\n", includeFileName, buf);
  if ( !buf ) {
    buf = new bufType;
  }
  ptr = actWin->obj.getNameFromClass( (char *) "includeWidgetClass" );
  if ( ptr )
    strncpy( title, ptr, 31 );
  else
    strncpy( title, includeWidgetClass_str17, 31 );

  Strncat( title, includeWidgetClass_str3, 31 );

  buf->bufX = x;
  buf->bufY = y;
  //buf->bufW = w;
  //buf->bufH = h;

  buf->bufFgColor = fgColor.pixelIndex();

  strncpy( buf->bufIncludeFileName, includeFileName, 127 );

  buf->bufDrawFrame = drawFrame;

  if ( symbolsExpStr.getRaw() ) {
    strncpy( buf->bufSymbols, symbolsExpStr.getRaw(), maxSymbolLen );
    buf->bufSymbols[maxSymbolLen] = 0;
  }
  else {
    strncpy( buf->bufSymbols, "", maxSymbolLen );
  }

  if (debugm) fprintf( stderr, "buf->bufSymbols = [%s]\n", buf->bufSymbols );

  if ( helpCommandExpString.getRaw() )
    strncpy( buf->bufHelpCommand, helpCommandExpString.getRaw(), 255 );
  else
    strncpy( buf->bufHelpCommand, "", 255 );

  ef.create( actWin->top, actWin->appCtx->ci.getColorMap(),
   &actWin->appCtx->entryFormX,
   &actWin->appCtx->entryFormY, &actWin->appCtx->entryFormW,
   &actWin->appCtx->entryFormH, &actWin->appCtx->largestH,
   title, NULL, NULL, NULL );

  ef.addTextField( includeWidgetClass_str4, 35, &buf->bufX );
  ef.addTextField( includeWidgetClass_str5, 35, &buf->bufY );
  //ef.addTextField( includeWidgetClass_str6, 35, &buf->bufW );
  //ef.addTextField( includeWidgetClass_str7, 35, &buf->bufH );

  ef.addTextField( includeWidgetClass_str37, 35, buf->bufIncludeFileName,
   127 );
  fileEntry = ef.getCurItem();
  ef.addTextField( includeWidgetClass_str26, 35, buf->bufSymbols,
   maxSymbolLen );
  macrosEntry = ef.getCurItem();
  ef.addToggle( includeWidgetClass_str52, &buf->bufDrawFrame );
  drawFrameEntry = ef.getCurItem();

  ef.addColorButton( includeWidgetClass_str8, actWin->ci, &buf->fgCb,
   &buf->bufFgColor );
    
  fileEntry->addDependency( macrosEntry );
  fileEntry->addDependency( drawFrameEntry );
  fileEntry->addDependencyCallbacks();

  //ef.addTextField( includeWidgetClass_str49, 35, buf->bufHelpCommand, 255 );

  return 1;

}

int includeWidgetClass::editCreate ( void ) {

  this->genericEdit();
  ef.finished( iw_edit_ok, iw_edit_apply, iw_edit_cancel_delete, this );
  actWin->currentEf = NULL;
  ef.popup();

  return 1;

}

int includeWidgetClass::edit ( void ) {

  this->genericEdit();
  ef.finished( iw_edit_ok, iw_edit_apply, iw_edit_cancel, this );
  actWin->currentEf = &ef;
  ef.popup();

  return 1;

}

int includeWidgetClass::erase ( void ) {

  if ( deleteRequest ) return 1;

  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  return 1;

}

int includeWidgetClass::eraseActive ( void ) {

  if ( !enabled || !activeMode || !init ) return 1;

  XDrawRectangle( actWin->d, drawable(actWin->executeWidget),
   actWin->executeGc.eraseGC(), x, y, w, h );

  XFillRectangle( actWin->d, drawable(actWin->executeWidget),
   actWin->executeGc.eraseGC(), x, y, w, h );

  return 1;

}


int includeWidgetClass::readIncludeFile ( std::list<string> parentList )
{

  int more, maxW, maxH, saveLine;
  int winMajor, winMinor, winRelease;
  char itemName[127+1];
  activeGraphicListPtr head, cur, next;
  FILE *f;
  char name[127+1];
  expStringClass expStr;

  int retStat = 1;
  char *gotOne, tagName[255+1], val[4095+1];
  int isCompound;
  tagClass tag;

  int i, max, st;
  char *sbuf;
  char *newMacros[MAX_NOOF_MACROS];
  char *newValues[MAX_NOOF_MACROS];
  int numNewMacros;

  max = MAX_NOOF_MACROS;
  sbuf = strdup( this->symbolsExpStr.getExpanded() );
  if (debugm) fprintf( stderr, "sbuf = [%s]\n", sbuf );
  st = parseSymbolsAndValues( sbuf, max,
   newMacros, newValues, &numNewMacros );

  for ( i=0; i<numNewMacros; i++ ) {
    if (debugm) fprintf( stderr, "m[%-d] = [%s], v[%-d] = [%s]\n", i, newMacros[i], i, newValues[i] );
  }
  
  // these are discarded if read
  expStringClass visPvExpStr;

  tagClass::pushLevel();
  tagClass::setFileName( includeFileName );

  saveLine = tag.line();

  if ( strcmp( includeFileName, "" ) == 0 ) {
    free( sbuf );
    return 0;
  }

  actWin->substituteSpecial( 127, includeFileName, name );

  expStr.setRaw( name );
  expStr.expand1st( numNewMacros, newMacros, newValues );
  if (debugm) fprintf( stderr, "expStr.getExpanded() = [%s]\n", expStr.getExpanded() );

  f = actWin->openAnySymFile( expStr.getExpanded(), (char *) "r" );
  if ( !f ) {
    // numStates = 0;
    free( sbuf );
    return 0;
  }

  actWin->discardWinLoadData( f, &winMajor, &winMinor, &winRelease );

  if ( winMajor < 4 ) {
    free( sbuf );
    return 0;     // no compatibility with old format
  }

  maxW = 0;
  maxH = 0;

  // delete old and init for new
  head = (activeGraphicListPtr) voidHead;
  cur = head->flink;
  while ( cur != head ) {
    next = cur->flink;
    delete cur->node;
    delete cur;
    cur = next;
  }
  head->flink = head;
  head->blink = head;

  tag.init();
  tag.loadR( (char *) "object", 127, itemName );

  more = 0;
  
  do {

    // read and create sub-objects until EOF is found

    gotOne = tag.getName( tagName, 255, f );
    if ( !gotOne ) {
      fileClose( f );
      tag.setLine( saveLine );
      tagClass::popLevel();
      free( sbuf );
      return 1;        // return success on EOF
    }

    if ( gotOne ) {

      if ( strcmp( tagName, "object" ) == 0 ) {

        tag.getValue( val, 4095, f, &isCompound );
        tag.decode( tagName, val, isCompound );

        // =======================================================
        // Create object

        more = 1;

        cur = new activeGraphicListType;
        if ( !cur ) {
          fileClose( f );
          fprintf( stderr, "Insufficient virtual memory - abort\n" );
          fileClose( f );
          tag.setLine( saveLine );
          tagClass::popLevel();
          free( sbuf );
          return 0;
        }
        cur->node = actWin->obj.createNew( itemName );
        if ( cur->node ) {
          if (debug) fprintf( stderr, "created [%s]\n", cur->node->objName() );
          if ( cur->node->isIncludeWidget() ) {
            if ( ((includeWidgetClass *) cur->node)->createFromFile( f, itemName, actWin, parentList ) < 0) {
              free( sbuf );
              return -1;
            }
          } else {
            cur->node->createFromFile( f, itemName, actWin );
          }

          cur->node->expandTemplate( numNewMacros, newMacros, newValues );

          if ( cur->node->isRelatedDisplay() ) {
            cur->node->augmentRelatedDisplayMacros( sbuf );
          }

          // adjust origin
          
          cur->node->move( this->x, this->y );

          cur->blink = head->blink;
          head->blink->flink = cur;
          head->blink = cur;
          cur->flink = head;

        }
        else {

          fileClose( f );
          fprintf( stderr, "Insufficient virtual memory - abort\n" );
          //numStates = 0;
          tag.setLine( saveLine );
	  tagClass::popLevel();
          free( sbuf );
          return 0;

        }

      }
      else {
        //numStates = 0;
        fileClose( f );
        tag.setLine( saveLine );
	tagClass::popLevel();
        free( sbuf );
        return 0;
      }

    }

  } while ( more );


  fileClose( f );

  w = maxW;
  sboxW = w;
  h = maxH;
  sboxH = h;

  tag.setLine( saveLine );
  tagClass::popLevel();

  free( sbuf );
  
  return retStat;

}


int includeWidgetClass::draw ( void ) {
activeGraphicListPtr head;
activeGraphicListPtr cur;

//  fprintf(stderr, "draw activeMode %d deleteRequest %d symbols %s\n", activeMode, deleteRequest, symbolsExpStr.getRaw());
  if ( activeMode || deleteRequest ) return 1;

  if ( drawFrame ) {

    actWin->drawGc.saveFg();

    actWin->drawGc.setLineWidth( 1 );
    actWin->drawGc.setLineStyle( LineSolid );

    actWin->drawGc.setFG( fgColor.pixelColor() );
    XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
     actWin->drawGc.normGC(), x, y, w, h );

    actWin->drawGc.restoreFg();

  }
    
  head = (activeGraphicListPtr) voidHead;

  cur = head->flink;
  while ( cur != head ) {

    cur->node->draw();

    cur = cur->flink;

  }

  return 1;

}

int includeWidgetClass::drawActive ( void ) {
  activeGraphicListPtr head;
  activeGraphicListPtr cur;
  pvColorClass tmpColor;

  if ( !enabled || !init || !activeMode) return 1;

  if ( drawFrame ) {

    actWin->executeGc.saveFg();

    actWin->executeGc.setLineWidth( 1 );
    actWin->executeGc.setLineStyle( LineSolid );

    actWin->executeGc.setFG( fgColor.pixelColor() );
    XDrawRectangle( actWin->d, XtWindow(actWin->executeWidget),
      actWin->executeGc.normGC(), x, y, w, h );

  }

  head = (activeGraphicListPtr) voidHead;

  cur = head->flink;
  while ( cur != head ) {

    cur->node->drawActive();

    cur = cur->flink;

  }
  return 1;

}

activeGraphicClass *includeWidgetClass::enclosingObject (
  int _x,
  int _y )
{

btnActionListPtr curBtn;
activeGraphicClass *obj;

  if ( !enabled ) return 0;

  curBtn = btnDownActionHead->blink;
  while ( curBtn != btnDownActionHead ) {

    if ( ( obj = curBtn->node->enclosingObject( _x, _y ) ) != NULL ) {
      return obj;
    }

    curBtn = curBtn->blink;

  }

  return NULL;

}

int includeWidgetClass::doSmartDrawAllActive ( void ) {

activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
activeGraphicListPtr cur;

  cur = head->flink;
  while ( cur != head ) {

    if ( cur->node->smartDrawCount() ) {
      cur->node->doSmartDrawAllActive();
    }

    cur = cur->flink;

  }

  return 1;

}

int includeWidgetClass::drawActiveIfIntersects (
  int x0,
  int y0,
  int x1,
  int y1 )
{

activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
activeGraphicListPtr cur;

  if ( deleteRequest ) return 1;

  cur = head->flink;
  while ( cur != head ) {

    //fprintf( stderr, "group ... " );
    cur->node->drawActiveIfIntersects( x0, y0, x1, y1 );

    cur = cur->flink;

  }

  return 1;

}

int includeWidgetClass::smartDrawCount ( void ) {

activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
activeGraphicListPtr cur;
int n = 0;

  cur = head->flink;
  while ( cur != head ) {
    n += cur->node->smartDrawCount();
    cur = cur->flink;
  }

  return n;

}

void includeWidgetClass::resetSmartDrawCount ( void ) {

activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
activeGraphicListPtr cur;

  cur = head->flink;
  while ( cur != head ) {
    cur->node->resetSmartDrawCount();
    cur = cur->flink;
  }

}

void includeWidgetClass::bufInvalidate ( void ) {

activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
activeGraphicListPtr cur;

  cur = head->flink;
  while ( cur != head ) {

    cur->node->bufInvalidate();

    cur = cur->flink;

  }

}

int includeWidgetClass::getButtonActionRequest (
  int *up,
  int *down,
  int *drag )
{

activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
activeGraphicListPtr cur;
btnActionListPtr curBtn;

int OneUp, OneDown, OneDrag, OneFocus, stat;

  *up = 0;
  *down = 0;
  *drag = 0;

  cur = head->flink;
  while ( cur != head ) {

    stat = cur->node->getButtonActionRequest( &OneUp, &OneDown, &OneDrag,
     &OneFocus );

    if ( OneUp ) {
      *up = 1;
      curBtn = new btnActionListType;
      curBtn->node = cur->node;
      curBtn->blink = btnUpActionHead->blink;
      btnUpActionHead->blink->flink = curBtn;
      btnUpActionHead->blink = curBtn;
      curBtn->flink = btnUpActionHead;
    }

    if ( OneDown ) {
      *down = 1;
      curBtn = new btnActionListType;
      curBtn->node = cur->node;
      curBtn->blink = btnDownActionHead->blink;
      btnDownActionHead->blink->flink = curBtn;
      btnDownActionHead->blink = curBtn;
      curBtn->flink = btnDownActionHead;
    }

    if ( OneDrag ) {
      *drag = 1;
      curBtn = new btnActionListType;
      curBtn->node = cur->node;
      curBtn->blink = btnMotionActionHead->blink;
      btnMotionActionHead->blink->flink = curBtn;
      btnMotionActionHead->blink = curBtn;
      curBtn->flink = btnMotionActionHead;
    }

    cur = cur->flink;

  }

  return 1;

}

int includeWidgetClass::getButtonActionRequest (
  int *up,
  int *down,
  int *drag,
  int *focus )
{

activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
activeGraphicListPtr cur;
btnActionListPtr curBtn;

int OneUp, OneDown, OneDrag, OneFocus, stat;

  *up = 0;
  *down = 0;
  *drag = 0;
  *focus = 0;

  cur = head->flink;
  while ( cur != head ) {

    stat = cur->node->getButtonActionRequest( &OneUp, &OneDown, &OneDrag,
     &OneFocus );

    if ( OneUp ) {
      *up = 1;
      curBtn = new btnActionListType;
      curBtn->node = cur->node;
      curBtn->blink = btnUpActionHead->blink;
      btnUpActionHead->blink->flink = curBtn;
      btnUpActionHead->blink = curBtn;
      curBtn->flink = btnUpActionHead;
    }

    if ( OneDown ) {
      *down = 1;
      curBtn = new btnActionListType;
      curBtn->node = cur->node;
      curBtn->blink = btnDownActionHead->blink;
      btnDownActionHead->blink->flink = curBtn;
      btnDownActionHead->blink = curBtn;
      curBtn->flink = btnDownActionHead;
    }

    if ( OneDrag ) {
      *drag = 1;
      curBtn = new btnActionListType;
      curBtn->node = cur->node;
      curBtn->blink = btnMotionActionHead->blink;
      btnMotionActionHead->blink->flink = curBtn;
      btnMotionActionHead->blink = curBtn;
      curBtn->flink = btnMotionActionHead;
    }

    if ( OneFocus ) {
      *focus = 1;
      curBtn = new btnActionListType;
      curBtn->in = -1;
      curBtn->node = cur->node;
      curBtn->blink = btnFocusActionHead->blink;
      btnFocusActionHead->blink->flink = curBtn;
      btnFocusActionHead->blink = curBtn;
      curBtn->flink = btnFocusActionHead;
    }

    cur = cur->flink;

  }

  return 1;

}

void includeWidgetClass::btnDown (
  XButtonEvent *be,
  int x,
  int y,
  int buttonState,
  int buttonNumber,
  int *action )
{

btnActionListPtr curBtn;

  curBtn = btnDownActionHead->flink;
  while ( curBtn != btnDownActionHead ) {

    if ( ( be->x > curBtn->node->getX0() ) &&
         ( be->x < curBtn->node->getX1() ) &&
         ( be->y > curBtn->node->getY0() ) &&
         ( be->y < curBtn->node->getY1() ) ) {

      curBtn->node->btnDown( be, x, y, buttonState, buttonNumber, action );

    }

    curBtn = curBtn->flink;

  }

}

void includeWidgetClass::btnUp (
  XButtonEvent *be,
  int x,
  int y,
  int buttonState,
  int buttonNumber,
  int *action )
{

btnActionListPtr curBtn;

  curBtn = btnUpActionHead->flink;
  while ( curBtn != btnUpActionHead ) {

    if ( ( x > curBtn->node->getX0() ) &&
         ( x < curBtn->node->getX1() ) &&
         ( y > curBtn->node->getY0() ) &&
         ( y < curBtn->node->getY1() ) ) {

      curBtn->node->btnUp( be, x, y, buttonState, buttonNumber, action );

    }

    curBtn = curBtn->flink;

  }

}

void includeWidgetClass::btnDrag (
  XMotionEvent *me,
  int x,
  int y,
  int buttonState,
  int buttonNumber )
{

btnActionListPtr curBtn;

  curBtn = btnMotionActionHead->flink;
  while ( curBtn != btnMotionActionHead ) {

    if ( ( me->x > curBtn->node->getX0() ) &&
         ( me->x < curBtn->node->getX1() ) &&
         ( me->y > curBtn->node->getY0() ) &&
         ( me->y < curBtn->node->getY1() ) ) {

      curBtn->node->btnDrag( me, x, y, buttonState, buttonNumber );

    }

    curBtn = curBtn->flink;

  }

}

void includeWidgetClass::pointerIn (
  XMotionEvent *me,
  int _x,
  int _y,
  int buttonState )
{

btnActionListPtr curBtn;
activeGraphicClass *ptr;

  if ( !enabled ) return;

  curBtn = btnFocusActionHead->flink;
  while ( curBtn != btnFocusActionHead ) {

    ptr = curBtn->node->enclosingObject( me->x, me->y );
    if ( ptr ) {

      if ( curBtn->in != 1 ) {
        curBtn->in = 1;
        ptr->pointerIn( me, _x, _y, buttonState );
      }

    }

    curBtn = curBtn->flink;

  }

}

void includeWidgetClass::pointerOut (
  XMotionEvent *me,
  int _x,
  int _y,
  int buttonState )
{

btnActionListPtr curBtn;
activeGraphicClass *ptr;

  curBtn = btnFocusActionHead->flink;
  while ( curBtn != btnFocusActionHead ) {

    ptr = curBtn->node->enclosingObject( me->x, me->y );
    if ( !ptr ) {
      if ( curBtn->in == 1 ) {
        curBtn->in = 0;
        curBtn->node->pointerOut( me, _x, _y, buttonState );
      }

    }

    curBtn = curBtn->flink;

  }

}

void includeWidgetClass::checkMouseOver (
  XMotionEvent *me,
  int _x,
  int _y,
  int buttonState )
{

btnActionListPtr curBtn;

  if ( !enabled ) return;

  curBtn = btnFocusActionHead->flink;
  while ( curBtn != btnFocusActionHead ) {

    curBtn->node->checkMouseOver( me, _x, _y, buttonState );

    curBtn = curBtn->flink;

  }

}

int includeWidgetClass::activateBeforePreReexecuteComplete ( void ) {

  //return 1;

  pend_io( 5.0 );
  pend_event( 0.01 );
  return activateComplete();

}

int includeWidgetClass::activateComplete ( void ) {

activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
activeGraphicListPtr cur;

  cur = head->flink;
  while ( cur != head ) {
    if ( !(cur->node->activateComplete()) ) {
      //printf( "%s at %-d,%-d not ready\n", cur->node->objName(),
      // cur->node->getX0(), cur->node->getY0() );
      return 0;
    }
    cur = cur->flink;
  }

  return 1;

}

int includeWidgetClass::activate (
  int pass,
  void *ptr,
  int *numSubObjects ) {

activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
activeGraphicListPtr cur;
int num;

  switch ( pass ) {

  case 1: // initialize

    opComplete1 = 0;

    break;

  case 2: // connect to pv's

    if ( !opComplete1 ) {

      initEnable();
      needRefresh = 0;
      aglPtr = ptr;
      opComplete1 = 1;
      init = 1;

    }

    break;

  case 3:
  case 4:
  case 5:

    break;

  }

  *numSubObjects = 0;
  cur = head->flink;
  while ( cur != head ) {

    cur->node->activate( pass, (void *) cur, &num );

    (*numSubObjects) += num;
    if ( *numSubObjects >= activeWindowClass::NUM_PER_PENDIO ) {
      pend_io( 5.0 );
      pend_event( 0.01 );
      //processAllEvents( actWin->appCtx->appContext(), actWin->d );
      *numSubObjects = 0;
    }

    cur = cur->flink;

  }

  switch ( pass ) {

  case 1: // initialize

    op2Complete1 = 0;

    break;

  case 2:
  case 3:
  case 4:
  case 5:

    break;

  case 6:

    if ( !op2Complete1 ) {

      enabled = init = activeMode = 1;

      op2Complete1 = 1;

      pend_io( 5.0 );
      pend_event( 0.01 );

    }

    break;

  }

  return 1;

}

int includeWidgetClass::deactivate (
  int pass,
  int *numSubObjects ) {

  activeGraphicListPtr head= (activeGraphicListPtr) voidHead;
  activeGraphicListPtr cur;
  btnActionListPtr curBtn, nextBtn;
  int num;

  *numSubObjects = 0;

  if ( pass == 1 ) {

    activeMode = 0;

    curBtn = btnDownActionHead->flink;
    while ( curBtn != btnDownActionHead ) {
      nextBtn = curBtn->flink;
      delete curBtn;
      curBtn = nextBtn;
    }

    btnDownActionHead->flink = btnDownActionHead;
    btnDownActionHead->blink = btnDownActionHead;

    curBtn = btnUpActionHead->flink;
    while ( curBtn != btnUpActionHead ) {
      nextBtn = curBtn->flink;
      delete curBtn;
      curBtn = nextBtn;
    }

    btnUpActionHead->flink = btnUpActionHead;
    btnUpActionHead->blink = btnUpActionHead;

    curBtn = btnMotionActionHead->flink;
    while ( curBtn != btnMotionActionHead ) {
      nextBtn = curBtn->flink;
      delete curBtn;
      curBtn = nextBtn;
    }

    btnMotionActionHead->flink = btnMotionActionHead;
    btnMotionActionHead->blink = btnMotionActionHead;

    curBtn = btnFocusActionHead->flink;
    while ( curBtn != btnFocusActionHead ) {
      nextBtn = curBtn->flink;
      delete curBtn;
      curBtn = nextBtn;
    }

    btnFocusActionHead->flink = btnFocusActionHead;
    btnFocusActionHead->blink = btnFocusActionHead;

  }

  cur = head->flink;
  while ( cur != head ) {

    cur->node->deactivate( pass, &num );

    (*numSubObjects) += num;
    if ( *numSubObjects >= activeWindowClass::NUM_PER_PENDIO ) {
      pend_io( 5.0 );
      pend_event( 0.01 );
      //processAllEvents( actWin->appCtx->appContext(), actWin->d );
      *numSubObjects = 0;
    }

    cur = cur->flink;

  }

  return 1;

}

int includeWidgetClass::preReactivate (
  int pass,
  int *numSubObjects ) {

activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
activeGraphicListPtr cur;
int num;

  *numSubObjects = 0;

  if ( pass == 1 ) {

    activeMode = 0;

  }

  cur = head->flink;
  while ( cur != head ) {

    cur->node->preReactivate( pass, &num );

    (*numSubObjects) += num;
    if ( *numSubObjects >= activeWindowClass::NUM_PER_PENDIO ) {
      pend_io( 5.0 );
      pend_event( 0.01 );
      //processAllEvents( actWin->appCtx->appContext(), actWin->d );
      *numSubObjects = 0;
    }

    cur = cur->flink;

  }

  return 1;

}

int includeWidgetClass::reactivate (
  int pass,
  void *ptr,
  int *numSubObjects ) {

activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
activeGraphicListPtr cur;
int num;

  switch ( pass ) {

  case 1: // initialize

    opComplete1 = 0;

    break;

  case 2: // connect to pv's

    if ( !opComplete1 ) {

      initEnable();

      needRefresh = 0;
      aglPtr = ptr;

      init = 1; // this stays true if there are no pvs

      opComplete1 = 1;

    }

    break;

  case 3:
  case 4:
  case 5:

    break;

  }

  *numSubObjects = 0;
  cur = head->flink;
  while ( cur != head ) {

    cur->node->reactivate( pass, (void *) cur, &num );

    (*numSubObjects) += num;

    if ( *numSubObjects >= activeWindowClass::NUM_PER_PENDIO ) {
      pend_io( 5.0 );
      pend_event( 0.01 );
      //processAllEvents( actWin->appCtx->appContext(), actWin->d );
      *numSubObjects = 0;
    }

    cur = cur->flink;

  }

  switch ( pass ) {

  case 1: // initialize

    op2Complete1 = 0;

    break;

  case 2:
  case 3:
  case 4:
  case 5:

    break;

  case 6:

    if ( !op2Complete1 ) {

      activeMode = 1;

      op2Complete1 = 1;

      //pend_io( 5.0 );
      //pend_event( 0.01 );

    }

    break;

  }

  return 1;

}

void includeWidgetClass::updateGroup ( void ) { // for paste operation

activeGraphicListPtr head;
activeGraphicListPtr cur;

  if ( deleteRequest ) return;

  head = (activeGraphicListPtr) voidHead;

  cur = head->flink;
  while ( cur != head ) {

    cur->node->actWin = actWin;

    cur = cur->flink;

  }

}

int includeWidgetClass::moveSelectBox (
  int _x,
  int _y )
{

  activeGraphicListPtr head;
  activeGraphicListPtr cur;

  sboxX += _x;
  sboxY += _y;

  head = (activeGraphicListPtr) voidHead;

  cur = head->flink;
  while ( cur != head ) {

    cur->node->moveSelectBox( _x, _y );
    cur->node->updateDimensions();

    cur = cur->flink;

  }
  return 1;
}

int includeWidgetClass::moveSelectBoxAbs (
  int _x,
  int _y )
{

  activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
  activeGraphicListPtr cur;
  int dx, dy;

  dx = _x - sboxX;
  dy = _y - sboxY;

  sboxX = _x;
  sboxY = _y;

  head = (activeGraphicListPtr) voidHead;

  cur = head->flink;
  while ( cur != head ) {

    cur->node->moveSelectBox( dx, dy );
    cur->node->updateDimensions();

    cur = cur->flink;

  }

  return 1;

}

int includeWidgetClass::moveSelectBoxMidpointAbs (
  int _x,
  int _y )
{

  activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
  activeGraphicListPtr cur;
  int dx, dy;

  dx = _x - sboxW/2 - sboxX;
  dy = _y - sboxH/2 - sboxY;

  sboxX = _x - sboxW/2;
  sboxY = _y - sboxH/2;

  head = (activeGraphicListPtr) voidHead;

  cur = head->flink;
  while ( cur != head ) {

    cur->node->moveSelectBox( dx, dy );
    cur->node->updateDimensions();

    cur = cur->flink;
  }
  return 1;

}

int includeWidgetClass::checkResizeSelectBox (
  int _x,
  int _y,
  int _w,
  int _h ) {

    return 0; // no resize allowed

}

int includeWidgetClass::checkResizeSelectBoxAbs (
  int _x,
  int _y,
  int _w,
  int _h ) {

  return 0; // no resize allowed

}

int includeWidgetClass::move (
  int _x,
  int _y ) {

  activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
  activeGraphicListPtr cur;

  x += _x;
  y += _y;

  head = (activeGraphicListPtr) voidHead;

  cur = head->flink;
  while ( cur != head ) {

    cur->node->move( _x, _y );
    cur->node->updateDimensions();

    cur = cur->flink;
  }

  return 1;

}

int includeWidgetClass::moveAbs (
  int _x,
  int _y ) {

  activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
  activeGraphicListPtr cur;
  int dx, dy;

  dx = _x - x;
  dy = _y - y;

  x = _x;
  y = _y;

  head = (activeGraphicListPtr) voidHead;

  cur = head->flink;
  while ( cur != head ) {

    cur->node->move( dx, dy );
    cur->node->updateDimensions();

    cur = cur->flink;
  }

  return 1;

}

int includeWidgetClass::moveMidpointAbs (
  int _x,
  int _y ) {

  activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
  activeGraphicListPtr cur;
  int dx, dy;

  dx = _x - w/2 - x;
  dy = _y - h/2 - y;

  x = _x - w/2;
  y = _y - h/2;

  head = (activeGraphicListPtr) voidHead;

  cur = head->flink;
  while ( cur != head ) {

    cur->node->move( dx, dy );
    cur->node->updateDimensions();

    cur = cur->flink;

  }
  return 1;
}

int includeWidgetClass::getIncludeWidgetProperty (
  char *key
) {

  return 0;

}

char *includeWidgetClass::getIncludeFileName (
) {
 return includeFileName;

}

char *includeWidgetClass::getIncludeWidgetMacros (
) {
  return symbolsExpStr.getExpanded();

}

int includeWidgetClass::expandTemplate (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

  expStringClass tmpStr;
  activeGraphicListPtr head;
  activeGraphicListPtr cur;

  if ( deleteRequest ) return 1;

  if (debug) fprintf(stderr, "expandTemplate numMacros %d\n", numMacros);
  for (int i=0; i < numMacros; i++) {
    if (debugm) fprintf(stderr, "%s=%s\n", macros[i], expansions[i]);
  }

  head = (activeGraphicListPtr) voidHead;

  cur = head->flink;
  while ( cur != head ) {

    cur->node->expandTemplate( numMacros, macros, expansions );

    cur = cur->flink;

  }

  return 1;

}

int includeWidgetClass::expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] )
{
  activeGraphicListPtr head;
  activeGraphicListPtr cur;
  char buf[maxSymbolLen+1];
  int i, err;

  if ( deleteRequest ) return 1;
  
  if (debugm) fprintf(stderr, "expand1st %s numMacros %d\n", includeFileName, numMacros);

  head = (activeGraphicListPtr) voidHead;

  cur = head->flink;
  while ( cur != head ) {

    cur->node->expand1st( numMacros, macros, expansions );

    cur = cur->flink;

  }

  debug = 0;
  
  return 1;

}

int includeWidgetClass::expand2nd (
  int numMacros,
  char *macros[],
  char *expansions[] )
{
  activeGraphicListPtr head;
  activeGraphicListPtr cur;

  if ( deleteRequest ) return 1;

  if (debugm) fprintf(stderr, "expand2nd numMacros %d\n", numMacros);
  for (int i=0; i < numMacros; i++) {
    if (debugm) fprintf(stderr, "%s=%s\n", macros[i], expansions[i]);
  }
  
  head = (activeGraphicListPtr) voidHead;

  cur = head->flink;
  while ( cur != head ) {

    cur->node->expand2nd( numMacros, macros, expansions );

    cur = cur->flink;

  }

  return 1;
}

int includeWidgetClass::containsMacros ( void )
{
  activeGraphicListPtr head;
  activeGraphicListPtr cur;

  if ( deleteRequest ) return 1;

  head = (activeGraphicListPtr) voidHead;

  cur = head->flink;
  while ( cur != head ) {

    if ( cur->node->containsMacros() ) return 1;

    cur = cur->flink;

  }
  return 0;

}

void includeWidgetClass::getPvs (
  int max,
  ProcessVariable *pvs[],
  int *n )
{

activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
activeGraphicListPtr cur;
int curN, curMax = max;

  *n = 0;
  cur = head->blink;
  while ( cur != head ) {

    cur->node->getPvs( curMax, &pvs[*n], &curN );
    (*n) += curN;
    curMax -= curN;

    cur = cur->blink;

  }

}

int includeWidgetClass::showPvInfo (
  XButtonEvent *be,
  int x,
  int y )
{

activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
activeGraphicListPtr cur;
int stat = 0;

  cur = head->blink;
  while ( cur != head ) {

    if ( ( x > cur->node->getX0() ) &&
         ( x < cur->node->getX1() ) &&
         ( y > cur->node->getY0() ) &&
         ( y < cur->node->getY1() ) ) {

      if ( cur->node->atLeastOneDragPv( x, y ) ) {
        stat = cur->node->showPvInfo( be, x, y );
        if ( stat ) break;
      }

    }

    cur = cur->blink;

  }

  return stat;

}

void includeWidgetClass::changeWidgetParams (
  unsigned int _flag,
  int _alignment,
  int _ctlAlignment,
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

  //if ( _flag & ACTGRF_BGCOLOR_MASK )
  //  bgColor.setColorIndex( _bgColor, actWin->ci );

  //if ( _flag & ACTGRF_TOPSHADOWCOLOR_MASK )
  //  topShadowColor = _topShadowColor;

  //if ( _flag & ACTGRF_BOTSHADOWCOLOR_MASK )
  //  botShadowColor = _botShadowColor;

}

int includeWidgetClass::initDefExeNode (
  void *ptr )
{

int stat;
activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
activeGraphicListPtr cur;

  stat = actWin->initDefExeNode( ptr );

  cur = head->flink;
  while ( cur != head ) {

    stat = cur->node->initDefExeNode( cur );

    cur = cur->flink;

  }

  return 1;

}

void includeWidgetClass::executeDeferred ( void ) {

int nr;
activeWindowListPtr cur;

  if ( actWin->isIconified ) return;

  actWin->appCtx->proc->lock();
  nr = needRefresh; needRefresh = 0;
  actWin->remDefExeNode( aglPtr );
  actWin->appCtx->proc->unlock();

  if ( nr ) {

    smartDrawAllActive();

  }

}

//???????????????????

int includeWidgetClass::startDrag (
  XButtonEvent *be,
  int x,
  int y )
{

activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
activeGraphicListPtr cur;
int stat = 0;

  cur = head->blink;
  while ( cur != head ) {

    if ( ( x > cur->node->getX0() ) &&
         ( x < cur->node->getX1() ) &&
         ( y > cur->node->getY0() ) &&
         ( y < cur->node->getY1() ) ) {

      if ( cur->node->atLeastOneDragPv( x, y ) ) {
        stat = cur->node->startDrag( be, x, y );
        if ( stat ) break;
      }

    }

    cur = cur->blink;

  }

  return stat;

}

int includeWidgetClass::selectDragValue (
  XButtonEvent *be
) {

activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
activeGraphicListPtr cur;
char *firstName, *nextName;

  cur = head->blink;
  while ( cur != head ) {

    if ( ( be->x > cur->node->getX0() ) &&
         ( be->x < cur->node->getX1() ) &&
         ( be->y > cur->node->getY0() ) &&
         ( be->y < cur->node->getY1() ) ) {

      if ( cur->node->atLeastOneDragPv( be->x, be->y ) ) {

        currentDragIndex = 0;

        firstName = cur->node->firstDragName( be->x, be->y );
        if ( !firstName ) return 0;

        actWin->popupDragBegin(
        actWin->obj.getNameFromClass( cur->node->objName() ) );
        actWin->popupDragAddItem( (void *) cur->node, firstName );

        nextName = cur->node->nextDragName( be->x, be->y );
        while ( nextName ) {

          actWin->popupDragAddItem( (void *) cur->node, nextName );
          nextName = cur->node->nextDragName( be->x, be->y );

        }

        actWin->popupDragFinish( be );

        break; // out of while loop

      }

    }

    cur = cur->blink;

  }

  return 1;

}

char *includeWidgetClass::firstDragName (
  int x,
  int y
) {

activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
activeGraphicListPtr cur;

  cur = head->blink;
  while ( cur != head ) {

    if ( ( x > cur->node->getX0() ) &&
         ( x < cur->node->getX1() ) &&
         ( y > cur->node->getY0() ) &&
         ( y < cur->node->getY1() ) ) {

      if ( cur->node->atLeastOneDragPv( x, y ) ) {

        return cur->node->firstDragName( x, y );

      }

    }

    cur = cur->blink;

  }

  return NULL;

}

char *includeWidgetClass::nextDragName (
  int x,
  int y
) {

activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
activeGraphicListPtr cur;

  cur = head->blink;
  while ( cur != head ) {

    if ( ( x > cur->node->getX0() ) &&
         ( x < cur->node->getX1() ) &&
         ( y > cur->node->getY0() ) &&
         ( y < cur->node->getY1() ) ) {

      if ( cur->node->atLeastOneDragPv( x, y ) ) {

        return cur->node->nextDragName( x, y );

      }

    }

    cur = cur->blink;

  }

  return NULL;

}

char *includeWidgetClass::dragValue (
  int x,
  int y,
  int i
) {

activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
activeGraphicListPtr cur;

  cur = head->blink;
  while ( cur != head ) {

    if ( ( x > cur->node->getX0() ) &&
         ( x < cur->node->getX1() ) &&
         ( y > cur->node->getY0() ) &&
         ( y < cur->node->getY1() ) ) {

      if ( cur->node->atLeastOneDragPv( x, y ) ) {

        return cur->node->dragValue( x, y, i );

      }

    }

    cur = cur->blink;

  }

  return groupDragName;

}

int includeWidgetClass::atLeastOneDragPv (
  int x,
  int y
) {

activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
activeGraphicListPtr cur;

  cur = head->blink;
  while ( cur != head ) {

    if ( ( x > cur->node->getX0() ) &&
         ( x < cur->node->getX1() ) &&
         ( y > cur->node->getY0() ) &&
         ( y < cur->node->getY1() ) ) {

      if ( cur->node->atLeastOneDragPv( x, y ) ) {
        return 1;
      }

    }

    cur = cur->blink;

  }

  return 0;

}

void includeWidgetClass::initEnable ( void ) {

activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
activeGraphicListPtr cur;

  cur = head->flink;
  while ( cur != head ) {

    cur->node->initEnable();

    cur = cur->flink;

  }

}

void includeWidgetClass::enable ( void ) {

activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
activeGraphicListPtr cur;

  activeGraphicClass::enable();

  cur = head->flink;
  while ( cur != head ) {

    cur->node->enable();
    cur = cur->flink;

  }

  actWin->requestActiveRefresh();

}

void includeWidgetClass::disable ( void ) {

activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
activeGraphicListPtr cur;

  enabled = 0;

  cur = head->flink;
  while ( cur != head ) {

    cur->node->disable();
    cur = cur->flink;

  }

  actWin->requestActiveRefresh();

}

char *includeWidgetClass::crawlerGetFirstPv ( void ) {

activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
activeGraphicListPtr cur;
char *crawlerPv = NULL;

  curCrawlerState = GETTING_FIRST_CRAWLER_PV;

  cur = head->flink;
  if ( cur == head ) {
    curCrawlerState = NO_MORE_CRAWLER_PVS;
    goto done;
  }

  while ( ( crawlerPv == NULL ) && ( curCrawlerState != NO_MORE_CRAWLER_PVS ) ) {

    if ( curCrawlerState == GETTING_FIRST_CRAWLER_PV ) {

      crawlerPv = cur->node->crawlerGetFirstPv();
      if ( crawlerPv ) {
        if ( strcmp( crawlerPv, "" ) == 0 ) crawlerPv = NULL;
      }
      if ( crawlerPv ) {
        curCrawlerState = GETTING_NEXT_CRAWLER_PV;
      }
      else {
        cur = cur->flink;
        if ( cur == head ) {
          curCrawlerState = NO_MORE_CRAWLER_PVS;
        }
      }

    }
    else if ( curCrawlerState == GETTING_NEXT_CRAWLER_PV ) {

      crawlerPv = cur->node->crawlerGetNextPv();
      if ( crawlerPv ) {
        if ( strcmp( crawlerPv, "" ) == 0 ) crawlerPv = NULL;
      }
      if ( !crawlerPv ) {
        cur = cur->flink;
        if ( cur == head ) {
          curCrawlerState = NO_MORE_CRAWLER_PVS;
        }
	else {
          curCrawlerState = GETTING_FIRST_CRAWLER_PV;
	}
      }

    }

  }

done:
  curCrawlerNode = (void *) cur;
  return crawlerPv;

}

char *includeWidgetClass::crawlerGetNextPv ( void ) {

activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
activeGraphicListPtr cur = (activeGraphicListPtr) curCrawlerNode;
char *crawlerPv = NULL;

  if ( ( cur == head ) || ( curCrawlerState == NO_MORE_CRAWLER_PVS ) ) {
    goto done;
  }

  while ( ( crawlerPv == NULL ) && ( curCrawlerState != NO_MORE_CRAWLER_PVS ) ) {

    if ( curCrawlerState == GETTING_FIRST_CRAWLER_PV ) {

      crawlerPv = cur->node->crawlerGetFirstPv();
      if ( crawlerPv ) {
        if ( strcmp( crawlerPv, "" ) == 0 ) crawlerPv = NULL;
      }
      if ( crawlerPv ) {
        curCrawlerState = GETTING_NEXT_CRAWLER_PV;
      }
      else {
        cur = cur->flink;
        if ( cur == head ) {
          curCrawlerState = NO_MORE_CRAWLER_PVS;
        }
      }

    }
    else if ( curCrawlerState == GETTING_NEXT_CRAWLER_PV ) {

      crawlerPv = cur->node->crawlerGetNextPv();
      if ( crawlerPv ) {
        if ( strcmp( crawlerPv, "" ) == 0 ) crawlerPv = NULL;
      }
      if ( !crawlerPv ) {
        cur = cur->flink;
        if ( cur == head ) {
          curCrawlerState = NO_MORE_CRAWLER_PVS;
        }
	else {
          curCrawlerState = GETTING_FIRST_CRAWLER_PV;
	}
      }

    }

  }

done:
  curCrawlerNode = (void *) cur;
  return crawlerPv;

}

std::list<string>& includeWidgetClass::getParentList() {
  return parentList;
}

int includeWidgetClass::isRelatedDisplay ( void ) {

activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
activeGraphicListPtr cur;

  cur = head->flink;
  while ( cur != head ) {
    if ( cur->node->isRelatedDisplay() ) {
      return 1;
    }
    cur = cur->flink;
  }

  return 0;

}

int includeWidgetClass::getNumRelatedDisplays ( void ) {

  // build list of related display nodes and record extent of indices

activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
activeGraphicListPtr cur;
RelatedDisplayNodePtr currd;
int index = 0;

  cur = head->flink;
  while ( cur != head ) {

    if ( cur->node->isRelatedDisplay() ) {

      currd = new RelatedDisplayNodeType;

      currd->ptr = (activeGraphicListPtr) cur;
      currd->first = index;
      index += cur->node->getNumRelatedDisplays();
      currd->last = index - 1;

      currd->blink = relatedDisplayNodeHead->blink;
      relatedDisplayNodeHead->blink->flink = currd;
      relatedDisplayNodeHead->blink = currd;
      currd->flink = relatedDisplayNodeHead;

    }

    cur = cur->flink;

  }

  return index;

}

int includeWidgetClass::getRelatedDisplayProperty (
  int index,
  char *name
) {

// translate index to node and node specific index

RelatedDisplayNodePtr head = relatedDisplayNodeHead;
RelatedDisplayNodePtr cur;
activeGraphicListPtr ptr;

  cur = head->flink;
  while ( cur != head ) {

    if ( index <= cur->last ) {
      index -= cur->first;
      ptr = (activeGraphicListPtr) cur->ptr;
      return ptr->node->getRelatedDisplayProperty( index, name );
    }

    cur = cur->flink;

  }

  return 0;

}

char *includeWidgetClass::getRelatedDisplayName (
  int index
) {

// translate index to node and node specific index

RelatedDisplayNodePtr head = relatedDisplayNodeHead;
RelatedDisplayNodePtr cur;
activeGraphicListPtr ptr;

  cur = head->flink;
  while ( cur != head ) {

    if ( index <= cur->last ) {
      index -= cur->first;
      ptr = (activeGraphicListPtr) cur->ptr;
      return ptr->node->getRelatedDisplayName( index );
    }

    cur = cur->flink;

  }

  return NULL;

}

char *includeWidgetClass::getRelatedDisplayMacros (
  int index
) {

// translate index to node and node specific index

RelatedDisplayNodePtr head = relatedDisplayNodeHead;
RelatedDisplayNodePtr cur;
activeGraphicListPtr ptr;

  cur = head->flink;
  while ( cur != head ) {

    if ( index <= cur->last ) {
      index -= cur->first;
      ptr = (activeGraphicListPtr) cur->ptr;
      return ptr->node->getRelatedDisplayMacros( index );
    }

    cur = cur->flink;

  }

  return NULL;

}

//???????????????????

char *includeWidgetClass::getSearchString (
  int i
) {

int num1 = 1 + 1 + 1 + NUMPVS;
int num2 = 1 + 1 + 1 + NUMPVS + 1 + 1 + 1;
int ii, selector, index;

  if ( i == 0 ) {
    return "";
  }
  else if ( i == 1 ) {
    return helpCommandExpString.getRaw();
  }
  else if ( i == 2 ) {
    return colorPvExpString.getRaw();
  }
  else if ( ( i > 2 ) && ( i < num1 ) ) {
    index = i - 3;
    return destPvExpString[index].getRaw();
  }
  else if ( ( i >= num1 ) && ( i < num2 ) ) {
    ii = i - num1;
    selector = ii % 3;
    if ( selector == 0 ) {
      return includeFileName;
    }
    else if ( selector == 1 ) {
      return symbolsExpStr.getRaw();
    }
    else if ( selector == 2 ) {
      return "";
    }
  }

  return NULL;

}

void includeWidgetClass::replaceString (
  int i,
  int max,
  char *string
) {

  int num1 = 1 + 1 + 1 + NUMPVS;
  int num2 = 1 + 1 + 1 + NUMPVS + 1 + 1 + 1;
  int ii, selector, index;

  if ( i == 0 ) {
    ;
  }
  else if ( i == 1 ) {
    helpCommandExpString.setRaw( string );
  }
  else if ( i == 2 ) {
    colorPvExpString.setRaw( string );
  }
  else if ( ( i > 2 ) && ( i < num1 ) ) {
    index = i - 3;
    destPvExpString[index].setRaw( string );
  }
  else if ( ( i >= num1 ) && ( i < num2 ) ) {
    ii = i - num1;
    selector = ii % 3;
    if ( selector == 0 ) {
      strcpy( includeFileName, string );
    }
    else if ( selector == 1 ) {
      symbolsExpStr.setRaw( string );
    }
    else if ( selector == 2 ) {
      ;;
    }
  }

}


#ifdef __cplusplus
extern "C" {
#endif

void *create_includeWidgetClassPtr ( void ) {

includeWidgetClass *ptr;

  ptr = new includeWidgetClass;
  return (void *) ptr;

}

void *clone_includeWidgetClassPtr (
  void *_srcPtr )
{

includeWidgetClass *ptr, *srcPtr;

  srcPtr = (includeWidgetClass *) _srcPtr;

  ptr = new includeWidgetClass( srcPtr );

  return (void *) ptr;

}

#ifdef __cplusplus
}
#endif
