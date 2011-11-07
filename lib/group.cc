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

#define __group_cc 1

#include "group.h"
#include "app_pkg.h"
#include "act_win.h"

#include "thread.h"

static char *groupDragName = "?";

#if 0
static void groupUnconnectedTimeout (
  XtPointer client,
  XtIntervalId *id )
{

activeGroupClass *ago = (activeGroupClass *) client;

  if ( !ago->activeMode ) return;

  if ( !ago->init ) {
    ago->needToDrawUnconnected = 1;
    ago->needRefresh = 1;
    ago->actWin->addDefExeNode( ago->aglPtr );
  }

  ago->unconnectedTimer = 0;

}
#endif

void agc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeGroupClass *ago = (activeGroupClass *) client;

activeGraphicListPtr head = (activeGraphicListPtr) ago->voidHead;
activeGraphicListPtr cur;
int dx, dy;

  ago->actWin->setChanged();

  ago->eraseSelectBoxCorners();
  ago->erase();

  ago->visPvExpStr.setRaw( ago->eBuf->bufVisPvName );

  if ( ago->bufVisInverted )
    ago->visInverted = 0;
  else
    ago->visInverted = 1;

  strncpy( ago->minVisString, ago->bufMinVisString, 39 );
  strncpy( ago->maxVisString, ago->bufMaxVisString, 39 );

  dx = ago->bufX - ago->x;
  dy = ago->bufY - ago->y;

  ago->x = ago->bufX;
  ago->sboxX = ago->bufX;

  ago->y = ago->bufY;
  ago->sboxY = ago->bufY;

  cur = head->flink;
  while ( cur != head ) {

    cur->node->move( dx, dy );
    cur->node->moveSelectBox( dx, dy );
    cur->node->updateDimensions();
    cur->node->setDefaultEnable( 1 );
    cur->node->initEnable();

    cur = cur->flink;

  }

  ago->smartDrawAll();

}

void agc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeGroupClass *ago = (activeGroupClass *) client;

  agc_edit_update( w, client, call );
  ago->refresh( ago );

}

void agc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeGroupClass *ago = (activeGroupClass *) client;
activeGraphicListPtr head = (activeGraphicListPtr) ago->voidHead;
activeGraphicListPtr cur;

  agc_edit_update( w, client, call );
  ago->ef.popdown();

  cur = head->flink;
  if ( cur && ( cur != head ) ) {
    cur->node->doEdit( &ago->undoObj );
  }
  else {
    ago->operationComplete();
  }

}

void agc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeGroupClass *ago = (activeGroupClass *) client;

  ago->ef.popdown();
  ago->operationCancel();

}

void activeGroupClass::visPvConnectStateCallback (
  ProcessVariable *pv,
  void *userarg
) {

activeGroupClass *ago = (activeGroupClass *) userarg;

  if ( pv->is_valid() ) {

  }
  else { // lost connection

    ago->connection.setPvDisconnected( (void *) ago->visPvConnection );

    ago->actWin->appCtx->proc->lock();
    ago->needRefresh = 1;
    ago->actWin->addDefExeNode( ago->aglPtr );
    ago->actWin->appCtx->proc->unlock();

  }

}

void activeGroupClass::visPvValueCallback (
  ProcessVariable *pv,
  void *userarg
) {

activeGroupClass *ago = (activeGroupClass *) userarg;

  if ( pv->is_valid() ) {

    if ( !ago->connection.pvsConnected() ) {

      ago->connection.setPvConnected( (void *) visPvConnection );

      if ( ago->connection.pvsConnected() ) {
        ago->actWin->appCtx->proc->lock();
        ago->needConnectInit = 1;
        ago->actWin->addDefExeNode( ago->aglPtr );
        ago->actWin->appCtx->proc->unlock();
      }

    }
    else {

      ago->actWin->appCtx->proc->lock();
      ago->needVisUpdate = 1;
      ago->actWin->addDefExeNode( ago->aglPtr );
      ago->actWin->appCtx->proc->unlock();

    }

  }

}

activeGroupClass::activeGroupClass ( void ) {

activeGraphicListPtr head;

  name = new char[strlen("activeGroupClass")+1];
  strcpy( name, "activeGroupClass" );
  checkBaseClassVersion( activeGraphicClass::MAJOR_VERSION, name );
  deleteRequest = 0;

  head = new activeGraphicListType;
  head->flink = head;
  head->blink = head;

  voidHead = (void *) head;

  relatedDisplayNodeHead = new RelatedDisplayNodeType;
  relatedDisplayNodeHead->flink = relatedDisplayNodeHead;
  relatedDisplayNodeHead->blink = relatedDisplayNodeHead;

  curCrawlerNode = NULL;
  curCrawlerState = GETTING_FIRST_CRAWLER_PV;

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

  connection.setMaxPvs( 1 );
  unconnectedTimer = 0;

  visInverted = 0;
  strcpy( minVisString, "" );
  strcpy( maxVisString, "" );

  activeMode = 0;

  eBuf = NULL;

}

activeGroupClass::~activeGroupClass ( void ) {

activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
activeGraphicListPtr cur, next;
btnActionListPtr curBtnAction, nextBtnAction;

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

  RelatedDisplayNodePtr currd, nextrd;

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
    delete curBtnAction->node;
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
    delete curBtnAction->node;
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
    delete curBtnAction->node;
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
    delete curBtnAction->node;
    delete curBtnAction;
    curBtnAction = nextBtnAction;
  }
  btnFocusActionHead->flink = NULL;
  btnFocusActionHead->blink = NULL;
  delete btnFocusActionHead;

  if ( name ) delete[] name;
  if ( eBuf ) delete eBuf;

#if 0
  if ( unconnectedTimer ) {
    XtRemoveTimeOut( unconnectedTimer );
    unconnectedTimer = 0;
  }
#endif

}

// copy constructor
activeGroupClass::activeGroupClass
 ( const activeGroupClass *source ) {

activeGraphicClass *ago = (activeGraphicClass *) this;
activeGraphicListPtr head, cur, curSource, sourceHead;

  ago->clone( (activeGraphicClass *) source );

  name = new char[strlen("activeGroupClass")+1];
  strcpy( name, "activeGroupClass" );

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

  visInverted = source->visInverted;
  visPvExpStr.setRaw( source->visPvExpStr.rawString );
  strncpy( minVisString, source->minVisString, 39 );
  strncpy( maxVisString, source->maxVisString, 39 );

  connection.setMaxPvs( 1 );
  unconnectedTimer = 0;

  activeMode = 0;

  eBuf = NULL;

  doAccSubs( visPvExpStr );
  doAccSubs( minVisString, 39 );
  doAccSubs( maxVisString, 39 );

}

int activeGroupClass::createGroup (
  activeWindowClass *aw_obj )
{

activeGraphicListPtr curSel;
int minX, maxX, minY, maxY;
activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
activeGraphicListPtr cur, next;
int isGroup;
activeGraphicClass *tailNode;

  actWin = (activeWindowClass *) aw_obj;
  xOrigin = 0;
  yOrigin = 0;

  // find bounding box for all selected items
  curSel = actWin->selectedHead->selFlink;
  if ( !curSel ) {
    deleteRequest = 1;
    return 0;
  }

  minX = curSel->node->getX0();
  maxX = curSel->node->getX1();
  minY = curSel->node->getY0();
  maxY = curSel->node->getY1();

  while ( curSel != actWin->selectedHead ) {

    if ( strcmp( curSel->node->objName(), "menuMuxClass" ) == 0 ) {
      actWin->appCtx->postMessage( activeGroupClass_str2 );
    }

    if ( curSel->node->getX0() < minX ) minX = curSel->node->getX0();
    if ( curSel->node->getX1() > maxX ) maxX = curSel->node->getX1();
    if ( curSel->node->getY0() < minY ) minY = curSel->node->getY0();
    if ( curSel->node->getY1() > maxY ) maxY = curSel->node->getY1();

    curSel = curSel->selFlink;

  }

  x = minX;
  y = minY;
  w = maxX - minX;
  h = maxY - minY;

  this->initSelectBox();

  // take over contents of the select list

  // remove nodes off main list
  curSel = actWin->selectedHead->selBlink;
  while ( curSel != actWin->selectedHead ) {

    curSel->node->eraseSelectBoxCorners();

    // deselect
    curSel->node->deselect();

    // remove
    curSel->blink->flink = curSel->flink;
    curSel->flink->blink = curSel->blink;

    // insert into this group list
    curSel->blink = head->blink;
    head->blink->flink = curSel;
    head->blink = curSel;
    curSel->flink = head;

    // adjust coordinates
    curSel->node->adjustCoordinates( x, y );
    curSel->node->updateDimensions();

    curSel = curSel->selBlink;

  }

  // traverse list and set nextToEdit for group editing
  cur = head->flink;
  while ( cur != head ) {

    if ( strcmp( cur->node->objName(), "activeGroupClass" ) == 0 )
      isGroup = 1;
    else
      isGroup = 0;

    next = cur->flink;

    cur->node->setInGroup();

    if ( next != head ) {
      cur->node->setNextToEdit( next->node );
      if ( isGroup ) {
        tailNode = cur->node->getTail();
        if ( tailNode ) tailNode->setNextToEdit( next->node );
      }
    }
    else
      cur->node->clearNextToEdit();

    cur = next;

  }

  // make selected list empty
  actWin->selectedHead->selFlink = actWin->selectedHead;
  actWin->selectedHead->selBlink = actWin->selectedHead;

  return 1;

}

int activeGroupClass::ungroup (
  void *curListNode )
{

activeGraphicListPtr curSel, cur, next;
activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
int isGroup;
activeGraphicClass *tailNode;

  curSel = (activeGraphicListPtr) curListNode;
  curSel->node->eraseSelectBoxCorners();
  curSel->node->deselect();

  // move all items to main list
  cur = head->flink;
  while ( cur != head ) {

    next = cur->flink;

    cur->node->setDefaultEnable( 1 );
    cur->node->initEnable();
    cur->node->clearInGroup();
    cur->node->clearNextToEdit();

    if ( strcmp( cur->node->objName(), "activeGroupClass" ) == 0 )
      isGroup = 1;
    else
      isGroup = 0;

    if ( isGroup ) {
      tailNode = cur->node->getTail();
      if ( tailNode ) tailNode->clearNextToEdit();
    }

    // unlink from group
    cur->blink->flink = cur->flink;
    cur->flink->blink = cur->blink;

    // link into main
    cur->blink = actWin->head->blink;
    actWin->head->blink->flink = cur;
    actWin->head->blink = cur;
    cur->flink = actWin->head;

    cur = next;

  }

  // remove current group node from select list
  curSel->selBlink->selFlink = curSel->selFlink;
  curSel->selFlink->selBlink = curSel->selBlink;

  // make current group node empty and set deleteRequest
  head->flink = head;
  head->blink = head;
  this->deleteRequest = 1;

  return 1;

}

int activeGroupClass::save (
  FILE *f )
{

activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
activeGraphicListPtr cur;
char fullName[255+1], *description;
int stat;
int retStat = 1;
int major, minor, release;
tagClass tag;

int zero = 0;
char *emptyStr = "";

  major = AGC_MAJOR_VERSION;
  minor = AGC_MINOR_VERSION;
  release = AGC_RELEASE;

  // read file and process each "object" tag
  tag.init();
  tag.loadW( "beginObjectProperties" );
  tag.loadW( "major", &major );
  tag.loadW( "minor", &minor );
  tag.loadW( "release", &release );
  tag.loadW( "x", &x );
  tag.loadW( "y", &y );
  tag.loadW( "w", &w );
  tag.loadW( "h", &h );
  tag.loadW( unknownTags );
  tag.loadW( "" );
  tag.loadW( "beginGroup" );
  tag.loadW( "" );

  stat = tag.writeTags( f );
  if ( !( stat & 1 ) ) retStat = stat;

  cur = head->flink;
  while( cur != head ) {

    if ( !cur->node->deleteRequest ) {

      if ( strcmp( cur->node->getCreateParam(), "" ) == 0 ) {
        strncpy( fullName, cur->node->objName(), 255 );
        description = actWin->obj.getNameFromClass( fullName );
        fprintf( f, "# (%s)\n", description );
        fprintf( f, "object %s\n", cur->node->objName() );
      }
      else {
        strncpy( fullName, cur->node->objName(), 255 );
        Strncat( fullName, ":", 255 );
        Strncat( fullName, cur->node->getCreateParam(), 255 );
        description = actWin->obj.getNameFromClass( fullName );
        fprintf( f, "# (%s)\n", description );
        fprintf( f, "object %s:%s\n", cur->node->objName(),
         cur->node->getCreateParam() );
      }

      stat = cur->node->save( f );
      if ( !( stat & 1 ) ) retStat = stat;

    }

    cur = cur->flink;

  }

  tag.init();

  tag.loadW( "endGroup" );
  tag.loadW( "" );

  tag.loadW( "visPv", &visPvExpStr, emptyStr );
  tag.loadBoolW( "visInvert", &visInverted, &zero );
  tag.loadW( "visMin", minVisString, emptyStr );
  tag.loadW( "visMax", maxVisString, emptyStr );

  tag.loadW( "endObjectProperties" );
  tag.loadW( "" );

  stat = tag.writeTags( f );
  if ( !( stat & 1 ) ) retStat = stat;

  return retStat;

}

int activeGroupClass::old_save (
  FILE *f )
{

activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
activeGraphicListPtr cur;
int stat;

  fprintf( f, "%-d %-d %-d\n", AGC_MAJOR_VERSION, AGC_MINOR_VERSION,
   AGC_RELEASE );
  fprintf( f, "%-d\n", x );
  fprintf( f, "%-d\n", y );
  fprintf( f, "%-d\n", w );
  fprintf( f, "%-d\n", h );
  fprintf( f, "{\n" );

  cur = head->flink;
  while( cur != head ) {

    fprintf( f, "%s\n", cur->node->objName() );
    stat = cur->node->save( f );
    fprintf( f, "<<<E~O~D>>>\n" );

    cur = cur->flink;

  }

  fprintf( f, "}\n" );

  if ( visPvExpStr.getRaw() )
    writeStringToFile( f, visPvExpStr.getRaw() );
  else
    writeStringToFile( f, "" );

  fprintf( f, "%-d\n", visInverted );
  writeStringToFile( f, minVisString );
  writeStringToFile( f, maxVisString );

  return 1;

}

int activeGroupClass::createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int major, minor, release, stat, retStat = 1;

char *gotOne, tagName[255+1], val[4095+1];
int isCompound;
tagClass tag;

int more;
char itemName[63+1];
activeGraphicListPtr cur, next;
activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
int isGroup;
activeGraphicClass *tailNode;

int zero = 0;
char *emptyStr = "";

  this->actWin = _actWin;
  this->selected = 0;
  this->deleteRequest = 0;

  // read file and process each "object" tag
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
  tag.loadR( "beginGroup" );

  stat = tag.readTags( f, "beginGroup" );

  if ( !( stat & 1 ) ) {
    retStat = stat;
    actWin->appCtx->postMessage( tag.errMsg() );
  }

  if ( major > AGC_MAJOR_VERSION ) {
    postIncompatable();
    return 0;
  }

  if ( major < 4 ) {
    postIncompatable();
    return 0;
  }

  this->initSelectBox();

  tag.init();
  tag.loadR( "object", 63, itemName );
  tag.loadR( "endGroup" );
  more = 1;
  do {

    gotOne = tag.getName( tagName, 255, f );

    while ( gotOne ) {

      //fprintf( stderr, "name = [%s]\n", tagName );

      if ( strcmp( tagName, "object" ) == 0 ) {

        tag.getValue( val, 4095, f, &isCompound );
        tag.decode( tagName, val, isCompound );

        // ==============================================================
        // Create sub-object

        //fprintf( stderr, "objName = [%s]\n", itemName );

        cur = new activeGraphicListType;
        if ( !cur ) {
          fclose( f );
          fprintf( stderr, activeGroupClass_str1 );
          return 0;
        }

        cur->node = actWin->obj.createNew( itemName );

        if ( cur->node ) {

          stat = cur->node->createFromFile( f, itemName, actWin );
          if ( !( stat & 1 ) ) return stat; // memory leak here

          cur->blink = head->blink;
          head->blink->flink = cur;
          head->blink = cur;
          cur->flink = head;

        }
        else {

          fclose( f );
          fprintf( stderr, activeGroupClass_str1 );
          return 0;

        }

        gotOne = tag.getName( tagName, 255, f );

      }
      else if ( strcmp( tagName, "endGroup" ) == 0 ) {

        more = 0;
        gotOne = NULL;

      }

    }

  } while ( more );

  tag.init();
  tag.loadR( "visPv", &visPvExpStr, emptyStr );
  tag.loadR( "visInvert", &visInverted, &zero );
  tag.loadR( "visMin", 39, minVisString, emptyStr );
  tag.loadR( "visMax", 39, maxVisString, emptyStr );
  tag.loadR( "endObjectProperties" );

  stat = tag.readTags( f, "endObjectProperties" );

  if ( !( stat & 1 ) ) {
    retStat = stat;
    actWin->appCtx->postMessage( tag.errMsg() );
  }

  // traverse list and set nextToEdit for group editing
  cur = head->flink;
  while ( cur != head ) {

    if ( strcmp( cur->node->objName(), "activeGroupClass" ) == 0 )
      isGroup = 1;
    else
      isGroup = 0;

    next = cur->flink;

    cur->node->setInGroup();

    if ( next != head ) {
      cur->node->setNextToEdit( next->node );
      if ( isGroup ) {
        tailNode = cur->node->getTail();
        if ( tailNode ) tailNode->setNextToEdit( next->node );
      }
    }
    else
      cur->node->clearNextToEdit();

    cur = next;

  }

  if ( head->flink == head ) { // group is empty, delete it
    this->deleteRequest = 1;
  }

  return stat;

}

int activeGroupClass::old_createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int l, more, stat;
char itemName[127+1], buf[63+1], *tk1, *tk2, *tk3, *context, *gotOne;
activeGraphicListPtr cur, next;
activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
int isGroup;
activeGraphicClass *tailNode;
int major, minor, release;
char oneName[PV_Factory::MAX_PV_NAME+1];

  this->actWin = _actWin;
  this->selected = 0;
  this->deleteRequest = 0;

  // ------------------------------------------------------------------
  // Initial problem: I failed to include the standard version info
  // in the group object. As a result, we have to do the following:

  fgets( buf, 63, f ); actWin->incLine();
  context = NULL;
  tk1 = strtok_r( buf, " ", &context );
  tk2 = strtok_r( NULL, " ", &context );
  tk3 = strtok_r( NULL, " ", &context );

  if ( !tk2 ) {

    major = 1;
    minor = 0;
    release = 0;
    if ( tk1 ) {
      x = atol( tk1 );
    }
    else {
      x = 0;
    }

  }
  else {

    if ( tk1 ) {
      major = atol( tk1 );
    }
    else {
      major = 1;
    }

    if ( tk2 ) {
      minor = atol( tk2 );
    }
    else {
      minor = 0;
    }

    if ( tk3 ) {
      release = atol( tk3 );
    }
    else {
      release = 0;
    }

    fscanf( f, "%d\n", &x ); actWin->incLine();

  }

  // ------------------------------------------------------------------

  fscanf( f, "%d\n", &y ); actWin->incLine();
  fscanf( f, "%d\n", &w ); actWin->incLine();
  fscanf( f, "%d\n", &h ); actWin->incLine();

  this->initSelectBox();

  fgets( itemName, 127, f ); actWin->incLine(); // discard "{"

  do {

    // read and create sub-objects until a "}" is found

    gotOne = fgets( itemName, 127, f ); actWin->incLine();
    if ( !gotOne ) return 0;

    l = strlen(itemName);
    if ( l > 127 ) l = 127;
    itemName[l-1] = 0;

    if ( strcmp( itemName, "}" ) == 0 )
      more = 0;
    else
      more = 1;

    if ( more ) {

      cur = new activeGraphicListType;
      if ( !cur ) {
        fclose( f );
        fprintf( stderr, activeGroupClass_str1 );
        return 0;
      }

      cur->node = actWin->obj.createNew( itemName );

      if ( cur->node ) {

        cur->node->old_createFromFile( f, itemName, actWin );

        stat = actWin->readUntilEndOfData( f ); // for forward compatibility
        if ( !( stat & 1 ) ) return stat; // memory leak here

        cur->blink = head->blink;
        head->blink->flink = cur;
        head->blink = cur;
        cur->flink = head;

      }
      else {
        fclose( f );
        fprintf( stderr, activeGroupClass_str1 );
        return 0;
      }

    }

  } while ( more );

  // traverse list and set nextToEdit for group editing
  cur = head->flink;
  while ( cur != head ) {

    if ( strcmp( cur->node->objName(), "activeGroupClass" ) == 0 )
      isGroup = 1;
    else
      isGroup = 0;

    next = cur->flink;

    cur->node->setInGroup();

    if ( next != head ) {
      cur->node->setNextToEdit( next->node );
      if ( isGroup ) {
        tailNode = cur->node->getTail();
        if ( tailNode ) tailNode->setNextToEdit( next->node );
      }
    }
    else
      cur->node->clearNextToEdit();

    cur = next;

  }

  if ( major >= 2 ) {

    readStringFromFile( oneName, PV_Factory::MAX_PV_NAME+1, f );
     actWin->incLine();
    visPvExpStr.setRaw( oneName );
    fscanf( f, "%d\n", &visInverted ); actWin->incLine();
    readStringFromFile( minVisString, 39+1, f ); actWin->incLine();
    readStringFromFile( maxVisString, 39+1, f ); actWin->incLine();

  }
  else {

    visPvExpStr.setRaw( "" );
    visInverted = 0;
    strcpy( minVisString, "" );
    strcpy( maxVisString, "" );

  }

  return 1;

}

int activeGroupClass::edit ( void ) {

char title[32], *ptr;
activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
activeGraphicListPtr cur;

  if ( !eBuf ) {
    eBuf = new editBufType;
  }

  cur = head->flink;
  if ( cur && ( cur != head ) ) {
    addUndoEditNode( curUndoObj );
  }

  ptr = actWin->obj.getNameFromClass( "activeGroupClass" );
  if ( ptr )
    strncpy( title, ptr, 31 );
  else
    strncpy( title, "Unknown Object" , 31 );

  Strncat( title, "Properties", 31 );

  bufX = x;
  bufY = y;

  if ( visPvExpStr.getRaw() )
    strncpy( eBuf->bufVisPvName, visPvExpStr.getRaw(), PV_Factory::MAX_PV_NAME );
  else
    strcpy( eBuf->bufVisPvName, "" );

  if ( visInverted )
    bufVisInverted = 0;
  else
    bufVisInverted = 1;

  strncpy( bufMinVisString, minVisString, 39 );
  strncpy( bufMaxVisString, maxVisString, 39 );

  ef.create( actWin->top, actWin->appCtx->ci.getColorMap(),
   &actWin->appCtx->entryFormX,
   &actWin->appCtx->entryFormY, &actWin->appCtx->entryFormW,
   &actWin->appCtx->entryFormH, &actWin->appCtx->largestH,
   title, NULL, NULL, NULL );

  ef.addTextField( "X", 30, &bufX );
  ef.addTextField( "Y", 30, &bufY );
  ef.addTextField( "Visibility PV", 30, eBuf->bufVisPvName,
   PV_Factory::MAX_PV_NAME );
  invisPvEntry = ef.getCurItem();
  ef.addOption( " ", "Not Visible if|Visible if", &bufVisInverted );
  visInvEntry = ef.getCurItem();
  invisPvEntry->addDependency( visInvEntry );
  ef.addTextField( ">=", 30, bufMinVisString, 39 );
  minVisEntry = ef.getCurItem();
  invisPvEntry->addDependency( minVisEntry );
  ef.addTextField( "and <", 30, bufMaxVisString, 39 );
  maxVisEntry = ef.getCurItem();
  invisPvEntry->addDependency( maxVisEntry );
  invisPvEntry->addDependencyCallbacks();
  ef.finished( agc_edit_ok, agc_edit_apply, agc_edit_cancel, this );
  actWin->currentEf = &ef;
  ef.popup();

  return 1;

}

void activeGroupClass::beginEdit ( void ) {

activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
activeGraphicListPtr cur;

  cur = head->flink;
  while ( cur != head ) {
    cur->node->beginEdit();
    cur = cur->flink;
  }

}

int activeGroupClass::activateBeforePreReexecuteComplete ( void ) {

  //return 1;

  return activateComplete();

}

int activeGroupClass::activateComplete ( void ) {

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

int activeGroupClass::checkEditStatus ( void ) {

activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
activeGraphicListPtr cur;

  cur = head->flink;
  while ( cur != head ) {
    if ( cur->node->checkEditStatus() ) {
      return 1;
    }
    cur = cur->flink;
  }

  return 0;

}

int activeGroupClass::erase ( void ) {

activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
activeGraphicListPtr cur;

  if ( deleteRequest ) return 1;

  cur = head->flink;
  while ( cur != head ) {

    cur->node->erase();

    cur = cur->flink;

  }

  return 1;

}

int activeGroupClass::eraseActive ( void ) {

activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
activeGraphicListPtr cur;

  cur = head->flink;
  while ( cur != head ) {

    cur->node->eraseActive();

    cur = cur->flink;

  }

  return 1;

}

activeGraphicClass *activeGroupClass::enclosingObject (
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

int activeGroupClass::doSmartDrawAllActive ( void ) {

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

int activeGroupClass::drawActiveIfIntersects (
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

int activeGroupClass::smartDrawCount ( void ) {

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

void activeGroupClass::resetSmartDrawCount ( void ) {

activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
activeGraphicListPtr cur;

  cur = head->flink;
  while ( cur != head ) {
    cur->node->resetSmartDrawCount();
    cur = cur->flink;
  }

}

int activeGroupClass::draw ( void ) {

activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
activeGraphicListPtr cur;

  if ( deleteRequest ) return 1;

  cur = head->flink;
  while ( cur != head ) {

    cur->node->draw();

    cur = cur->flink;

  }

  return 1;

}

void activeGroupClass::bufInvalidate ( void ) {

activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
activeGraphicListPtr cur;

  cur = head->flink;
  while ( cur != head ) {

    cur->node->bufInvalidate();

    cur = cur->flink;

  }

}

int activeGroupClass::drawActive ( void ) {

activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
activeGraphicListPtr cur;

  cur = head->flink;
  while ( cur != head ) {

    cur->node->drawActive();

    cur = cur->flink;

  }

  return 1;

}

int activeGroupClass::getButtonActionRequest (
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

int activeGroupClass::getButtonActionRequest (
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

void activeGroupClass::btnDown (
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

void activeGroupClass::btnUp (
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

void activeGroupClass::btnDrag (
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

void activeGroupClass::pointerIn (
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

void activeGroupClass::pointerOut (
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

void activeGroupClass::checkMouseOver (
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

int activeGroupClass::activate (
  int pass,
  void *ptr,
  int *numSubObjects ) {

activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
activeGraphicListPtr cur;
int num;

  switch ( pass ) {

  case 1: // initialize

    opComplete = 0;

    break;

  case 2: // connect to pv's

    if ( !opComplete ) {

      initEnable();

      connection.init();

      needConnectInit = needVisUpdate = needRefresh = 0;

      needToEraseUnconnected = 0;
      needToDrawUnconnected = 0;
      unconnectedTimer = 0;

      aglPtr = ptr;
      visPvId = 0;
      prevVisibility = -1;
      visibility = 0;

      init = 1; // this stays true if there are no pvs

      if ( !visPvExpStr.getExpanded() ||
           blankOrComment( visPvExpStr.getExpanded() ) ) {
        visPvExists = 0;
      }
      else {
        connection.addPv();
        visPvExists = 1;
        init = 0;
      }

      opComplete = 1;

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

    if ( pass == 2 ) {
      if ( visPvExists ) {
        cur->node->setDefaultEnable( 0 );
      }
    }
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

    op2Complete = 0;

    break;

  case 2:
  case 3:
  case 4:
  case 5:

    break;

  case 6:

    if ( !op2Complete ) {

      activeMode = 1;

      if ( visPvExists ) {

        visPvId = the_PV_Factory->create( visPvExpStr.getExpanded() );
        visPvId->add_conn_state_callback( visPvConnectStateCallback, this );
        visPvId->add_value_callback( visPvValueCallback, this );

#if 0
        if ( !unconnectedTimer ) {
          unconnectedTimer = appAddTimeOut( actWin->appCtx->appContext(),
           2000, groupUnconnectedTimeout, this );
        }
#endif

      }

      op2Complete = 1;

    }

    break;

  }

  return 1;

}

int activeGroupClass::deactivate (
  int pass,
  int *numSubObjects ) {

activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
activeGraphicListPtr cur;
btnActionListPtr curBtn, nextBtn;
int num;

  *numSubObjects = 0;

  if ( pass == 1 ) {

    activeMode = 0;

#if 0
    if ( unconnectedTimer ) {
      XtRemoveTimeOut( unconnectedTimer );
      unconnectedTimer = 0;
    }
#endif

    if ( visPvId ) {
      visPvId->remove_conn_state_callback( visPvConnectStateCallback, this );
      visPvId->remove_value_callback( visPvValueCallback, this );
      visPvId->release();
      visPvId = 0;
    }

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

int activeGroupClass::preReactivate (
  int pass,
  int *numSubObjects ) {

activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
activeGraphicListPtr cur;
int num;

  *numSubObjects = 0;

  if ( pass == 1 ) {

    activeMode = 0;

#if 0
    if ( unconnectedTimer ) {
      XtRemoveTimeOut( unconnectedTimer );
      unconnectedTimer = 0;
    }
#endif

    if ( visPvId ) {
      visPvId->remove_conn_state_callback( visPvConnectStateCallback, this );
      visPvId->remove_value_callback( visPvValueCallback, this );
      visPvId->release();
      visPvId = 0;
    }

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

int activeGroupClass::reactivate (
  int pass,
  void *ptr,
  int *numSubObjects ) {

activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
activeGraphicListPtr cur;
int num;

  switch ( pass ) {

  case 1: // initialize

    opComplete = 0;

    break;

  case 2: // connect to pv's

    if ( !opComplete ) {

      initEnable();

      connection.init();

      needConnectInit = needVisUpdate = needRefresh = 0;

      needToEraseUnconnected = 0;
      needToDrawUnconnected = 0;
      unconnectedTimer = 0;

      aglPtr = ptr;
      visPvId = 0;
      prevVisibility = -1;
      visibility = 0;

      init = 1; // this stays true if there are no pvs

      if ( !visPvExpStr.getExpanded() ||
           blankOrComment( visPvExpStr.getExpanded() ) ) {
        visPvExists = 0;
      }
      else {
        connection.addPv();
        visPvExists = 1;
        init = 0;
      }

      opComplete = 1;

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

    if ( pass == 2 ) {
      if ( visPvExists ) {
        cur->node->setDefaultEnable( 0 );
      }
    }
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

    op2Complete = 0;

    break;

  case 2:
  case 3:
  case 4:
  case 5:

    break;

  case 6:

    if ( !op2Complete ) {

      activeMode = 1;

      if ( visPvExists ) {

        visPvId = the_PV_Factory->create( visPvExpStr.getExpanded() );
        visPvId->add_conn_state_callback( visPvConnectStateCallback, this );
        visPvId->add_value_callback( visPvValueCallback, this );

#if 0
        if ( !unconnectedTimer ) {
          unconnectedTimer = appAddTimeOut( actWin->appCtx->appContext(),
           2000, groupUnconnectedTimeout, this );
        }
#endif

      }

      op2Complete = 1;

    }

    break;

  }

  return 1;

}

int activeGroupClass::moveSelectBox (
  int _x,
  int _y )
{

activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
activeGraphicListPtr cur;

  sboxX += _x;
  sboxY += _y;

  cur = head->flink;
  while ( cur != head ) {

    cur->node->moveSelectBox( _x, _y );
    cur->node->updateDimensions();

    cur = cur->flink;

  }

  return 1;

}

int activeGroupClass::moveSelectBoxAbs (
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

  cur = head->flink;
  while ( cur != head ) {

    cur->node->moveSelectBox( dx, dy );
    cur->node->updateDimensions();

    cur = cur->flink;

  }

  return 1;

}

int activeGroupClass::moveSelectBoxMidpointAbs (
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

  cur = head->flink;
  while ( cur != head ) {

    cur->node->moveSelectBox( dx, dy );
    cur->node->updateDimensions();

    cur = cur->flink;

  }

  return 1;

}

int activeGroupClass::checkResizeSelectBoxAbs (
  int _x,
  int _y,
  int _w,
  int _h ) {

int stat;
activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
activeGraphicListPtr cur;
int deltaX, deltaY;
double xScaleFactor, yScaleFactor, newX, newY, newW, newH;

  if ( _x == -1 )
    deltaX = 0;
  else
    deltaX = _x - x;

  if ( _y == -1 )
    deltaY = 0;
  else
    deltaY = _y - y;

  if ( _w == -1 )
    xScaleFactor = 1.0;
  else
    xScaleFactor = (double) _w / (double) w;

  if ( _h == -1 )
    yScaleFactor = 1.0;
  else
    yScaleFactor = (double) _h / (double) h;

  cur = head->flink;
  while ( cur != head ) {

    newX = x + deltaX +
     (int) ( (double) ( cur->node->getX0() - x )
     * xScaleFactor + 0.5 );

    newW =
     (int) ( (double) cur->node->getW() * xScaleFactor + 0.5 );

    newY = y + deltaY +
     (int) ( (double) ( cur->node->getY0() - y )
     * yScaleFactor + 0.5 );

    newH =
     (int) ( (double) cur->node->getH() * yScaleFactor + 0.5 );

    stat = cur->node->checkResizeSelectBoxAbs( (int) newX, (int) newY,
     (int) newW, (int) newH );
    if ( !( stat & 1 ) ) {
      return stat;
    }

    cur = cur->flink;

  }

  return 1;

}

int activeGroupClass::resizeSelectBoxAbs (
  int _x,
  int _y,
  int _w,
  int _h )
{

activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
activeGraphicListPtr cur;
int deltaX, deltaY;
double xScaleFactor, yScaleFactor, newX, newY, newW, newH;
int stat, ret_stat;

  ret_stat = 1;

  if ( _w > 0 ) {
    if ( _w < 5 ) {
      return 0;
    }
  }

  if ( _h > 0 ) {
    if ( _h < 5 ) {
      return 0;
    }
  }

  if ( _x == -1 )
    deltaX = 0;
  else
    deltaX = _x - sboxX;

  if ( _y == -1 )
    deltaY = 0;
  else
    deltaY = _y - sboxY;

  if ( _w == -1 )
    xScaleFactor = 1.0;
  else
    xScaleFactor = (double) _w / (double) sboxW;

  if ( _h == -1 )
    yScaleFactor = 1.0;
  else
    yScaleFactor = (double) _h / (double) sboxH;

  cur = head->flink;
  while ( cur != head ) {

    newX = x + deltaX +
     (int) ( (double) ( cur->node->getX0() - x )
     * xScaleFactor + 0.5 );

    newW =
     (int) ( (double) cur->node->getW() * xScaleFactor + 0.5 );

    newY = y + deltaY +
     (int) ( (double) ( cur->node->getY0() - y )
     * yScaleFactor + 0.5 );

    newH =
     (int) ( (double) cur->node->getH() * yScaleFactor + 0.5 );

    stat = cur->node->resizeSelectBoxAbs( (int) newX, (int) newY,
     (int) newW, (int) newH );
    if ( stat & 1 ) {
      cur->node->updateDimensions();
    }
    else {
      ret_stat = stat;
    }

    cur = cur->flink;

  }

  if ( _x > 0 ) sboxX = _x;
  if ( _y > 0 ) sboxY = _y;
  if ( _w > 0 ) sboxW = _w;
  if ( _h > 0 ) sboxH = _h;

  return ret_stat;

}

int activeGroupClass::resizeSelectBoxAbsFromUndo (
  int _x,
  int _y,
  int _w,
  int _h )
{

  if ( _x > 0 ) sboxX = _x;
  if ( _y > 0 ) sboxY = _y;
  if ( _w > 0 ) sboxW = _w;
  if ( _h > 0 ) sboxH = _h;

  return 1;

}

int activeGroupClass::checkResizeSelectBox (
  int _x,
  int _y,
  int _w,
  int _h ) {

int stat;
activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
activeGraphicListPtr cur;

  cur = head->flink;
  while ( cur != head ) {

    stat = cur->node->checkResizeSelectBox( _x, _y, _w, _h );
    if ( !( stat & 1 ) ) {
      return stat;
    }

    cur = cur->flink;

  }

  return 1;

}

int activeGroupClass::resizeSelectBox (
  int _x,
  int _y,
  int _w,
  int _h )
{

activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
activeGraphicListPtr cur;
int savex, savey, savew, saveh, stat, ret_stat;

  savex = sboxX;
  savey = sboxY;
  savew = sboxW;
  saveh = sboxH;

  ret_stat = 1;

  sboxX += _x;
  sboxY += _y;

  sboxW += _w;
  if ( sboxW < 5 ) {
    sboxX = savex;
    sboxW = savew;
    ret_stat = 0;
  }

  sboxH += _h;
  if ( sboxH < 5 ) {
    sboxY = savey;
    sboxH = saveh;
    ret_stat = 0;
  }

  cur = head->flink;
  while ( cur != head ) {

    stat = cur->node->resizeSelectBox( _x, _y, _w, _h );
    if ( stat & 1 ) {
      cur->node->updateDimensions();
    }
    else {
      ret_stat = stat;
    }

    cur = cur->flink;

  }

  return ret_stat;

}

int activeGroupClass::move (
  int _x,
  int _y ) {

activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
activeGraphicListPtr cur;

  x += _x;
  y += _y;

  cur = head->flink;
  while ( cur != head ) {

    cur->node->move( _x, _y );
    cur->node->updateDimensions();

    cur = cur->flink;

  }

  return 1;

}

int activeGroupClass::moveAbs (
  int _x,
  int _y ) {

activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
activeGraphicListPtr cur;
int dx, dy;

  dx = _x - x;
  dy = _y - y;

  x = _x;
  y = _y;

  cur = head->flink;
  while ( cur != head ) {

    cur->node->move( dx, dy );
    cur->node->updateDimensions();

    cur = cur->flink;

  }

  return 1;

}

int activeGroupClass::moveMidpointAbs (
  int _x,
  int _y ) {

activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
activeGraphicListPtr cur;
int dx, dy;

  dx = _x - w/2 - x;
  dy = _y - h/2 - y;

  x = _x - w/2;
  y = _y - h/2;

  cur = head->flink;
  while ( cur != head ) {

    cur->node->move( dx, dy );
    cur->node->updateDimensions();

    cur = cur->flink;

  }

  return 1;

}

int activeGroupClass::rotate (
  int xOrigin,
  int yOrigin,
  char direction )
{

activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
activeGraphicListPtr cur;

  // execute base class rotate
  ((activeGraphicClass *)this)->activeGraphicClass::rotate(
   xOrigin, yOrigin, direction );

  cur = head->flink;
  while ( cur != head ) {

    cur->node->rotate( xOrigin, yOrigin, direction );
    cur->node->updateDimensions();

    cur->node->resizeSelectBoxAbsFromUndo( cur->node->getX0(),
     cur->node->getY0(), cur->node->getW(),
     cur->node->getH() );

    cur = cur->flink;

  }

  return 1;

}

int activeGroupClass::flip (
  int xOrigin,
  int yOrigin,
  char direction )
{

activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
activeGraphicListPtr cur;

  // execute base class flip
  ((activeGraphicClass *)this)->activeGraphicClass::flip(
   xOrigin, yOrigin, direction );

  cur = head->flink;
  while ( cur != head ) {

    cur->node->flip( xOrigin, yOrigin, direction );
    cur->node->updateDimensions();

    cur->node->resizeSelectBoxAbsFromUndo( cur->node->getX0(),
     cur->node->getY0(), cur->node->getW(),
     cur->node->getH() );

    cur = cur->flink;

  }

  return 1;

}

int activeGroupClass::resize (
  int _x,
  int _y,
  int _w,
  int _h ) {

activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
activeGraphicListPtr cur;

  x += _x;
  y += _y;
  w += _w;
  h += _h;

  cur = head->flink;
  while ( cur != head ) {

    cur->node->resize( _x, _y, _w, _h );
    cur->node->updateDimensions();

    cur = cur->flink;

  }

  return 1;

}

int activeGroupClass::resizeAbs (
  int _x,
  int _y,
  int _w,
  int _h ) {

activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
activeGraphicListPtr cur;
int deltaX, deltaY;
double xScaleFactor, yScaleFactor, newX, newY, newW, newH;

  if ( _x == -1 )
    deltaX = 0;
  else
    deltaX = _x - x;

  if ( _y == -1 )
    deltaY = 0;
  else
    deltaY = _y - y;

  if ( _w == -1 )
    xScaleFactor = 1.0;
  else
    xScaleFactor = (double) _w / (double) w;

  if ( _h == -1 )
    yScaleFactor = 1.0;
  else
    yScaleFactor = (double) _h / (double) h;

  cur = head->flink;
  while ( cur != head ) {

    newX = x + deltaX +
     (int) ( (double) ( cur->node->getX0() - x )
     * xScaleFactor + 0.5 );

    newW =
     (int) ( (double) cur->node->getW() * xScaleFactor + 0.5 );

    newY = y + deltaY +
     (int) ( (double) ( cur->node->getY0() - y )
     * yScaleFactor + 0.5 );

    newH =
     (int) ( (double) cur->node->getH() * yScaleFactor + 0.5 );

    cur->node->resizeAbs( (int) newX, (int) newY, (int) newW, (int) newH );
    cur->node->resizeSelectBoxAbs( (int) newX, (int) newY, (int) newW,
     (int) newH );
    cur->node->updateDimensions();

    cur = cur->flink;

  }

  if ( _x != -1 ) x = _x;
  if ( _y != -1 ) y = _y;
  if ( _w != -1 ) w = _w;
  if ( _h != -1 ) h = _h;

  return 1;

}

int activeGroupClass::resizeAbsFromUndo (
  int _x,
  int _y,
  int _w,
  int _h )
{

  if ( _x != -1 ) x = _x;
  if ( _y != -1 ) y = _y;
  if ( _w != -1 ) w = _w;
  if ( _h != -1 ) h = _h;

  return 1;

}

activeGraphicClass *activeGroupClass::getTail ( void ) {

activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
activeGraphicListPtr cur;
activeGraphicClass *tailNode;
int isGroup;

  cur = head->blink;
  if ( !cur || ( cur == head ) ) return NULL;

  if ( strcmp( cur->node->objName(), "activeGroupClass" ) == 0 )
    isGroup = 1;
  else
    isGroup = 0;

  if ( isGroup )
    tailNode = cur->node->getTail();
  else
    tailNode = cur->node;

  return tailNode;

}

void activeGroupClass::updateGroup ( void ) { // for paste operation

activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
activeGraphicListPtr cur, next;
activeGraphicClass *tailNode;
int isGroup;

  if ( deleteRequest ) return;

  depth = 0;
  cur = head->flink;
  while ( cur != head ) {

    if ( strcmp( cur->node->objName(), "activeGroupClass" ) == 0 )
      isGroup = 1;
    else
      isGroup = 0;

    next = cur->flink;

    cur->node->actWin = actWin;
    cur->node->setInGroup();

    if ( next != head ) {
      cur->node->setNextToEdit( next->node );
      if ( isGroup ) {
        tailNode = cur->node->getTail();
        if ( tailNode ) tailNode->setNextToEdit( next->node );
      }
    }
    else
      if ( !cur->node->isInGroup() ) cur->node->clearNextToEdit();

    depth++;
    cur->node->updateGroup(); // invoke recursively
    depth--;

    cur = next;

  }

}

int activeGroupClass::initDefExeNode (
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

void activeGroupClass::executeDeferred ( void ) {

int stat, nc, nvu, nr;
pvValType pvV;

  if ( actWin->isIconified ) return;

  actWin->appCtx->proc->lock();
  nc = needConnectInit; needConnectInit = 0;
  nvu = needVisUpdate; needVisUpdate = 0;
  nr = needRefresh; needRefresh = 0;
  actWin->remDefExeNode( aglPtr );
  actWin->appCtx->proc->unlock();

  if ( !activeMode ) return;

  if ( nc ) {

    minVis.d = (double) atof( minVisString );
    maxVis.d = (double) atof( maxVisString );

    nvu = 1;

    init = 1;

  }

  if ( nvu && visPvId->is_valid() ) {

    pvV.d = visPvId->get_double();
    if ( ( pvV.d >= minVis.d ) && ( pvV.d < maxVis.d ) )
      visibility = 1 ^ visInverted;
    else
      visibility = 0 ^ visInverted;

    if ( prevVisibility != visibility ) {
      if ( !visibility ) {
        disable();
      }
      else {
        enable();
      }

      prevVisibility = visibility;

    }

  }

  if ( nr ) {
    stat = smartDrawAllActive();
  }

}

int activeGroupClass::containsMacros ( void )
{

activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
activeGraphicListPtr cur;

  if ( deleteRequest ) return 1;

  if ( visPvExpStr.containsPrimaryMacros() ) return 1;

  cur = head->flink;
  while ( cur != head ) {

    if ( cur->node->containsMacros() ) return 1;

    cur = cur->flink;

  }

  return 0;

}

int activeGroupClass::expandTemplate (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

expStringClass tmpStr;
activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
activeGraphicListPtr cur;

  if ( deleteRequest ) return 1;

  tmpStr.setRaw( visPvExpStr.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  visPvExpStr.setRaw( tmpStr.getExpanded() );

  cur = head->flink;
  while ( cur != head ) {

    cur->node->expandTemplate( numMacros, macros, expansions );

    cur = cur->flink;

  }

  return 1;

}

int activeGroupClass::expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
activeGraphicListPtr cur;

  if ( deleteRequest ) return 1;

  visPvExpStr.expand1st( numMacros, macros, expansions );

  cur = head->flink;
  while ( cur != head ) {

    cur->node->expand1st( numMacros, macros, expansions );

    cur = cur->flink;

  }

  return 1;

}

int activeGroupClass::expand2nd (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
activeGraphicListPtr cur;

  if ( deleteRequest ) return 1;

  visPvExpStr.expand2nd( numMacros, macros, expansions );

  cur = head->flink;
  while ( cur != head ) {

    cur->node->expand2nd( numMacros, macros, expansions );

    cur = cur->flink;

  }

  return 1;

}

void activeGroupClass::setNextSelectedToEdit (
  activeGraphicClass *ptr )
{

activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
activeGraphicListPtr cur;

  nextSelectedToEdit = ptr;

  cur = head->flink;
  while( cur != head ) {

    cur->node->setNextSelectedToEdit( ptr );

    cur = cur->flink;

  }

}

void activeGroupClass::clearNextSelectedToEdit ( void ) {

activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
activeGraphicListPtr cur;

  nextSelectedToEdit = NULL;

  cur = head->flink;
  while( cur != head ) {

    cur->node->clearNextSelectedToEdit();

    cur = cur->flink;

  }

}

void activeGroupClass::changeDisplayParams (
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

activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
activeGraphicListPtr cur;

  cur = head->flink;
  while ( cur != head ) {

    cur->node->changeDisplayParams (
     _flag,
     _fontTag,
     _alignment,
     _ctlFontTag,
     _ctlAlignment,
     _btnFontTag,
     _btnAlignment,
     _textFgColor,
     _fg1Color,
     _fg2Color,
     _offsetColor,
     _bgColor,
     _topShadowColor,
     _botShadowColor );

    cur = cur->flink;

  }

}

void activeGroupClass::changePvNames (
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

activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
activeGraphicListPtr cur;

  cur = head->flink;
  while ( cur != head ) {

    cur->node->changePvNames(
     flag,
     numCtlPvs,
     ctlPvs,
     numReadbackPvs,
     readbackPvs,
     numNullPvs,
     nullPvs,
     numVisPvs,
     visPvs,
     numAlarmPvs,
     alarmPvs );

    cur = cur->flink;

  }

}

void activeGroupClass::flushUndo ( void ) {

activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
activeGraphicListPtr cur;

  undoObj.flush();

  cur = head->flink;
  while( cur != head ) {

    cur->node->flushUndo();

    cur = cur->flink;

  }


}

int activeGroupClass::addUndoCreateNode (
  undoClass *_undoObj )
{

int stat;
activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
activeGraphicListPtr cur;

 return 1;

  stat = _undoObj->addCreateNode( this, NULL );
  if ( !( stat & 1 ) ) return stat;

  undoObj.startNewUndoList( "" );

  cur = head->flink;
  while( cur != head ) {

    stat = cur->node->addUndoCreateNode( &(this->undoObj) );
    if ( !( stat & 1 ) ) return stat;

    cur = cur->flink;

  }

  return 1;

}

int activeGroupClass::addUndoMoveNode (
  undoClass *_undoObj )
{

int stat;
activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
activeGraphicListPtr cur;

  stat = _undoObj->addMoveNode( this, NULL, x, y );
  if ( !( stat & 1 ) ) return stat;

  undoObj.startNewUndoList( "" );

  cur = head->flink;
  while( cur != head ) {

    stat = cur->node->addUndoMoveNode( &(this->undoObj) );
    if ( !( stat & 1 ) ) return stat;

    cur = cur->flink;

  }

  return 1;

}

int activeGroupClass::addUndoResizeNode (
  undoClass *_undoObj )
{

int stat;
activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
activeGraphicListPtr cur;

  stat = _undoObj->addResizeNode( this, NULL, x, y, w, h );
  if ( !( stat & 1 ) ) return stat;

  undoObj.startNewUndoList( "" );

  cur = head->flink;
  while( cur != head ) {

    stat = cur->node->addUndoResizeNode( &(this->undoObj) );
    if ( !( stat & 1 ) ) return stat;

    cur = cur->flink;

  }

  return 1;

}

int activeGroupClass::addUndoCopyNode (
  undoClass *_undoObj )
{

int stat;
activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
activeGraphicListPtr cur;

 return 1;

  stat = _undoObj->addCopyNode( this, NULL );
  if ( !( stat & 1 ) ) return stat;

  undoObj.startNewUndoList( "" );

  cur = head->flink;
  while( cur != head ) {

    stat = cur->node->addUndoCopyNode( &(this->undoObj) );
    if ( !( stat & 1 ) ) return stat;

    cur = cur->flink;

  }

  return 1;

}

int activeGroupClass::addUndoCutNode (
  undoClass *_undoObj )
{

int stat;
activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
activeGraphicListPtr cur;

 return 1;

  stat = _undoObj->addCutNode( this, NULL );
  if ( !( stat & 1 ) ) return stat;

  undoObj.startNewUndoList( "" );

  cur = head->flink;
  while( cur != head ) {

    stat = cur->node->addUndoCutNode( &(this->undoObj) );
    if ( !( stat & 1 ) ) return stat;

    cur = cur->flink;

  }

  return 1;

}

int activeGroupClass::addUndoPasteNode (
  undoClass *_undoObj )
{

int stat;
activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
activeGraphicListPtr cur;

 return 1;

  stat = _undoObj->addPasteNode( this, NULL );
  if ( !( stat & 1 ) ) return stat;

  undoObj.startNewUndoList( "" );

  cur = head->flink;
  while( cur != head ) {

    stat = cur->node->addUndoPasteNode( &(this->undoObj) );
    if ( !( stat & 1 ) ) return stat;

    cur = cur->flink;

  }

  return 1;

}

int activeGroupClass::addUndoReorderNode (
  undoClass *_undoObj )
{

int stat;
activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
activeGraphicListPtr cur;

 return 1;

  stat = _undoObj->addReorderNode( this, NULL );
  if ( !( stat & 1 ) ) return stat;

  undoObj.startNewUndoList( "" );

  cur = head->flink;
  while( cur != head ) {

    stat = cur->node->addUndoReorderNode( &(this->undoObj) );
    if ( !( stat & 1 ) ) return stat;

    cur = cur->flink;

  }

  return 1;

}

int activeGroupClass::addUndoEditNode (
  undoClass *_undoObj )
{

int stat;

  stat = _undoObj->addEditNode( this, NULL );
  if ( !( stat & 1 ) ) return stat;

  undoObj.startNewUndoList( "" );

  return 1;

}

int activeGroupClass::addUndoGroupNode (
  undoClass *_undoObj )
{

int stat;
activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
activeGraphicListPtr cur;

 return 1;

  stat = _undoObj->addGroupNode( this, NULL );
  if ( !( stat & 1 ) ) return stat;

  undoObj.startNewUndoList( "" );

  cur = head->flink;
  while( cur != head ) {

    stat = cur->node->addUndoGroupNode( &(this->undoObj) );
    if ( !( stat & 1 ) ) return stat;

    cur = cur->flink;

  }

  return 1;

}

int activeGroupClass::addUndoRotateNode (
  undoClass *_undoObj )
{

int stat;
activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
activeGraphicListPtr cur;

  stat = _undoObj->addRotateNode( this, NULL, x, y, w, h );
  if ( !( stat & 1 ) ) return stat;

  undoObj.startNewUndoList( "" );

  cur = head->flink;
  while( cur != head ) {

    stat = cur->node->addUndoRotateNode( &(this->undoObj) );
    if ( !( stat & 1 ) ) return stat;

    cur = cur->flink;

  }

  return 1;

}

int activeGroupClass::addUndoFlipNode (
  undoClass *_undoObj )
{

int stat;
activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
activeGraphicListPtr cur;

  stat = _undoObj->addFlipNode( this, NULL, x, y, w, h );
  if ( !( stat & 1 ) ) return stat;

  undoObj.startNewUndoList( "" );

  cur = head->flink;
  while( cur != head ) {

    stat = cur->node->addUndoFlipNode( &(this->undoObj) );
    if ( !( stat & 1 ) ) return stat;

    cur = cur->flink;

  }

  return 1;

}

int activeGroupClass::undoCreate (
  undoOpClass *opPtr
) {

  return 1;

}

int activeGroupClass::undoMove (
  undoOpClass *opPtr,
  int x,
  int y )
{

int stat;

  moveAbs( x, y );
  moveSelectBoxAbs( x, y );

  stat = undoObj.performSubUndo();
  if ( !( stat & 1 ) ) XBell( actWin->d, 50 );

  return 1;

}

int activeGroupClass::undoResize (
  undoOpClass *opPtr,
  int x,
  int y,
  int w,
  int h )
{

int stat;

  resizeAbsFromUndo( x, y, w, h );
  resizeSelectBoxAbsFromUndo( x, y, w, h );

  stat = undoObj.performSubUndo();
  if ( !( stat & 1 ) ) XBell( actWin->d, 50 );

  return 1;

}

int activeGroupClass::undoCopy (
  undoOpClass *opPtr
) {

  return 1;

}

int activeGroupClass::undoCut (
  undoOpClass *opPtr
) {

  return 1;

}

int activeGroupClass::undoPaste (
  undoOpClass *opPtr
) {

  return 1;

}

int activeGroupClass::undoReorder (
  undoOpClass *opPtr
) {

  return 1;

}

int activeGroupClass::undoEdit (
  undoOpClass *opPtr
) {

int stat;

  stat = undoObj.performSubUndo();
  if ( !( stat & 1 ) ) XBell( actWin->d, 50 );

  return 1;

}

int activeGroupClass::undoGroup (
  undoOpClass *opPtr
) {

  return 1;

}

int activeGroupClass::undoRotate (
  undoOpClass *opPtr,
  int x,
  int y,
  int w,
  int h )
{

int stat;

  resizeAbsFromUndo( x, y, w, h );
  resizeSelectBoxAbsFromUndo( x, y, w, h );

  stat = undoObj.performSubUndo();
  if ( !( stat & 1 ) ) XBell( actWin->d, 50 );

  return 1;

}

int activeGroupClass::undoFlip (
  undoOpClass *opPtr,
  int x,
  int y,
  int w,
  int h )
{

int stat;

  resizeAbsFromUndo( x, y, w, h );
  resizeSelectBoxAbsFromUndo( x, y, w, h );

  stat = undoObj.performSubUndo();
  if ( !( stat & 1 ) ) XBell( actWin->d, 50 );

  return 1;

}

char *activeGroupClass::getSearchString (
  int i
) {

// this must be called such that i starts at zero and increments by one subsequently
// i.e. evil coupling exists between this function and the code in act_win.cc from
// which it is called

activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
activeGraphicListPtr cur;
char *str;

  if ( i == 0 ) {
    return visPvExpStr.getRaw();
  }
  else if ( i == 1 ) {
    return minVisString;
  }
  else if ( i == 2 ) {
    return maxVisString;
  }
  else if ( i == 3 ) {

    _edmDebug();

    sarNeedNextNode = 0;
    sarIndex = 0;
    sarItemIndexOffset = i;
    cur = head;
    sarNode = (void *) cur;
    if ( cur->flink == head ) {
      // empty
      return NULL;
    }

    do {

      cur = cur->flink;
      sarNode = (void *) cur;
      sarNeedNextNode = 0;
      if ( cur != head ) {
        str = cur->node->getSearchString( sarIndex );
        if ( !str ) {
          sarNeedNextNode = 1;
        }
        else {
          return str;
        }
      }
      else {
        // no more
        return NULL;
      }

    } while (  sarNeedNextNode );

  }
  else {

    cur = (activeGraphicListPtr) sarNode;
    if ( cur == head ) {
      // no more
      return NULL;
    }

    do {

      if ( sarNeedNextNode ) {

        cur = cur->flink;
        sarNode = (void *) cur;
        sarIndex = 0;
        sarNeedNextNode = 0;
        sarItemIndexOffset = i;
        if ( cur != head ) {
          str = cur->node->getSearchString( sarIndex );
          if ( !str ) {
            sarNeedNextNode = 1;
          }
          else {
            return str;
          }
        }
        else {
          // no more
          return NULL;
        }

      }
      else {

        sarIndex++;
        str = cur->node->getSearchString( sarIndex );
        if ( !str ) {
          sarNeedNextNode = 1;
        }
        else {
          return str;
        }

      }

    } while (  sarNeedNextNode );

  }

  return NULL;

}

void activeGroupClass::replaceString (
  int i,
  int max,
  char *string
) {

activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
activeGraphicListPtr cur;

  if ( i == 0 ) {
    visPvExpStr.setRaw( string );
  }
  else if ( i == 1 ) {
    int l = max;
    if ( 39 < max ) l = 39;
    strncpy( minVisString, string, l );
    minVisString[l] = 0;
  }
  else if ( i == 2 ) {
    int l = max;
    if ( 39 < max ) l = 39;
    strncpy( maxVisString, string, l );
    maxVisString[l] = 0;
  }
  else {
    cur = (activeGraphicListPtr) sarNode;
    if ( cur != head ) {
      cur->node->replaceString ( i - sarItemIndexOffset,
       max, string );
    }
  }

  updateDimensions();

}

void activeGroupClass::getPvs (
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

int activeGroupClass::showPvInfo (
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

int activeGroupClass::startDrag (
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

int activeGroupClass::selectDragValue (
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

char *activeGroupClass::firstDragName (
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

char *activeGroupClass::nextDragName (
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

char *activeGroupClass::dragValue (
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

int activeGroupClass::atLeastOneDragPv (
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

void activeGroupClass::initEnable ( void ) {

activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
activeGraphicListPtr cur;

  cur = head->flink;
  while ( cur != head ) {

    cur->node->initEnable();

    cur = cur->flink;

  }

}

void activeGroupClass::enable ( void ) {

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

void activeGroupClass::disable ( void ) {

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

void activeGroupClass::map ( void ) {

#if 0
activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
activeGraphicListPtr cur;

  cur = head->flink;
  while ( cur != head ) {
    cur->node->map();
    cur = cur->flink;
  }
#endif

}

void activeGroupClass::unmap ( void ) {

#if 0
activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
activeGraphicListPtr cur;

  cur = head->flink;
  while ( cur != head ) {
    cur->node->unmap();
    cur = cur->flink;
  }
#endif

}

int activeGroupClass::getGroupVisInfo (
  expStringClass *visStr,
  int *visInv,
  int maxLen,
  char *minVis,
  char *maxVis
) {

  if ( maxLen < 40 ) return 0; // error

  visStr->copy( visPvExpStr );
  *visInv = visInverted;
  strncpy( minVis, minVisString, 39 );
  minVis[39] = 0;
  strncpy( maxVis, maxVisString, 39 );
  maxVis[39] = 0;

  return 1; // success

}

int activeGroupClass::putGroupVisInfo (
  expStringClass *visStr,
  int visInv,
  int maxLen,
  char *minVis,
  char *maxVis
) {

  if ( maxLen < 40 ) return 0; // error

  visPvExpStr.copy( *visStr );
  visInverted = visInv;
  strncpy( minVisString, minVis, 39 );
  minVisString[39] = 0;
  strncpy( maxVisString, maxVis, 39 );
  maxVisString[39] = 0;

  return 1; // success

}

char *activeGroupClass::crawlerGetFirstPv ( void ) {

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

char *activeGroupClass::crawlerGetNextPv ( void ) {

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

int activeGroupClass::isRelatedDisplay ( void ) {

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

int activeGroupClass::getNumRelatedDisplays ( void ) {

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

int activeGroupClass::getRelatedDisplayProperty (
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

char *activeGroupClass::getRelatedDisplayName (
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

char *activeGroupClass::getRelatedDisplayMacros (
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
