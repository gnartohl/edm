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

#include "group.h"
#include "app_pkg.h"
#include "act_win.h"

#include "thread.h"

static char *groupDragName = "?";

activeGroupClass::activeGroupClass ( void ) {

activeGraphicListPtr head;

  name = new char[strlen("activeGroupClass")+1];
  strcpy( name, "activeGroupClass" );
  deleteRequest = 0;

  head = new activeGraphicListType;
  head->flink = head;
  head->blink = head;

  voidHead = (void *) head;

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

  if ( name ) delete name;

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
        tailNode->setNextToEdit( next->node );
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

    cur->node->clearInGroup();
    cur->node->clearNextToEdit();

    if ( strcmp( cur->node->objName(), "activeGroupClass" ) == 0 )
      isGroup = 1;
    else
      isGroup = 0;

    if ( isGroup ) {
      tailNode = cur->node->getTail();
      tailNode->clearNextToEdit();
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
int stat;

  fprintf( f, "%-d\n", x );
  fprintf( f, "%-d\n", y );
  fprintf( f, "%-d\n", w );
  fprintf( f, "%-d\n", h );
  fprintf( f, "{\n" );

  cur = head->flink;
  while( cur != head ) {

    fprintf( f, "%s\n", cur->node->objName() );
    stat = cur->node->save( f );
    fprintf( f, "E\002O\002D\n" );

    cur = cur->flink;

  }

  fprintf( f, "}\n" );

  return 1;

}

int activeGroupClass::createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int l, more, stat;
char itemName[127+1], *gotOne;
activeGraphicListPtr cur, next;
activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
int isGroup;
activeGraphicClass *tailNode;

  this->actWin = _actWin;
  this->selected = 0;
  this->deleteRequest = 0;

  fscanf( f, "%d\n", &x ); actWin->incLine();
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
        printf( activeGroupClass_str1 );
        return 0;
      }

      cur->node = actWin->obj.createNew( itemName );

      if ( cur->node ) {

        cur->node->createFromFile( f, itemName, actWin );

        stat = actWin->readUntilEndOfData( f ); // for forward compatibility
        if ( !( stat & 1 ) ) return stat; // memory leak here

        cur->blink = head->blink;
        head->blink->flink = cur;
        head->blink = cur;
        cur->flink = head;

      }
      else {
        fclose( f );
        printf( activeGroupClass_str1 );
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
        tailNode->setNextToEdit( next->node );
      }
    }
    else
      cur->node->clearNextToEdit();

    cur = next;

  }

  return 1;

}

int activeGroupClass::edit ( void ) {

activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
activeGraphicListPtr cur;

  cur = head->flink;
  if ( cur ) {
    addUndoEditNode( curUndoObj );
    cur->node->doEdit( &undoObj );
  }

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

  curBtn = btnDownActionHead->flink;
  while ( curBtn != btnDownActionHead ) {

    if ( ( obj = curBtn->node->enclosingObject( _x, _y ) ) != NULL ) {
      return obj;
    }

    curBtn = curBtn->flink;

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

    //printf( "group ... " );
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
  int x,
  int y,
  int buttonState,
  int buttonNumber,
  int *action )
{

btnActionListPtr curBtn;

  curBtn = btnDownActionHead->flink;
  while ( curBtn != btnDownActionHead ) {

    if ( ( x > curBtn->node->getX0() ) &&
         ( x < curBtn->node->getX1() ) &&
         ( y > curBtn->node->getY0() ) &&
         ( y < curBtn->node->getY1() ) ) {

      curBtn->node->btnDown( x, y, buttonState, buttonNumber, action );

    }

    curBtn = curBtn->flink;

  }

}

void activeGroupClass::btnUp (
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

      curBtn->node->btnUp( x, y, buttonState, buttonNumber, action );

    }

    curBtn = curBtn->flink;

  }

}

void activeGroupClass::btnDrag (
  int x,
  int y,
  int buttonState,
  int buttonNumber )
{

btnActionListPtr curBtn;

  curBtn = btnMotionActionHead->flink;
  while ( curBtn != btnMotionActionHead ) {

    if ( ( x > curBtn->node->getX0() ) &&
         ( x < curBtn->node->getX1() ) &&
         ( y > curBtn->node->getY0() ) &&
         ( y < curBtn->node->getY1() ) ) {

      curBtn->node->btnDrag( x, y, buttonState, buttonNumber );

    }

    curBtn = curBtn->flink;

  }

}

void activeGroupClass::pointerIn (
  int _x,
  int _y,
  int buttonState )
{

btnActionListPtr curBtn;

  curBtn = btnFocusActionHead->flink;
  while ( curBtn != btnFocusActionHead ) {

    if ( ( _x > curBtn->node->getX0() ) &&
         ( _x < curBtn->node->getX1() ) &&
         ( _y > curBtn->node->getY0() ) &&
         ( _y < curBtn->node->getY1() ) ) {

      if ( curBtn->in != 1 ) {
        curBtn->in = 1;
        curBtn->node->pointerIn( _x, _y, buttonState );
      }

    }

    curBtn = curBtn->flink;

  }

}

void activeGroupClass::pointerOut (
  int _x,
  int _y,
  int buttonState )
{

btnActionListPtr curBtn;

  curBtn = btnFocusActionHead->flink;
  while ( curBtn != btnFocusActionHead ) {

    if ( ( _x <= curBtn->node->getX0() ) ||
         ( _x >= curBtn->node->getX1() ) ||
         ( _y <= curBtn->node->getY0() ) ||
         ( _y >= curBtn->node->getY1() ) ) {

      if ( curBtn->in == 1 ) {
        curBtn->in = 0;
        curBtn->node->pointerOut( _x, _y, buttonState );
      }

    }

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

  *numSubObjects = 0;
  cur = head->flink;
  while ( cur != head ) {

    cur->node->activate( pass, (void *) cur, &num );

    (*numSubObjects) += num;
    if ( *numSubObjects >= activeWindowClass::NUM_PER_PENDIO ) {
      ca_pend_io( 5.0 );
      ca_pend_event( 0.01 );
      processAllEvents( actWin->appCtx->appContext(), actWin->d );
      *numSubObjects = 0;
    }

    cur = cur->flink;

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
      ca_pend_io( 5.0 );
      ca_pend_event( 0.01 );
      processAllEvents( actWin->appCtx->appContext(), actWin->d );
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
  cur = head->flink;
  while ( cur != head ) {

    cur->node->preReactivate( pass, &num );

    (*numSubObjects) += num;
    if ( *numSubObjects >= activeWindowClass::NUM_PER_PENDIO ) {
      ca_pend_io( 5.0 );
      ca_pend_event( 0.01 );
      processAllEvents( actWin->appCtx->appContext(), actWin->d );
      *numSubObjects = 0;
    }

    cur = cur->flink;

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
  if ( !cur ) return NULL;

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
        tailNode->setNextToEdit( next->node );
      }
    }
    else
      if ( !cur->node->isInGroup() ) cur->node->clearNextToEdit();

    if ( isGroup ) {
      depth++;
      cur->node->updateGroup(); // invoke recursively
      depth--;
    }

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

int activeGroupClass::containsMacros ( void )
{

activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
activeGraphicListPtr cur;

  if ( deleteRequest ) return 1;

  cur = head->flink;
  while ( cur != head ) {

    if ( cur->node->containsMacros() ) return 1;

    cur = cur->flink;

  }

  return 0;

}

int activeGroupClass::expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
activeGraphicListPtr cur;

  if ( deleteRequest ) return 1;

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

int activeGroupClass::startDrag (
  int x,
  int y )
{

activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
activeGraphicListPtr cur;

  cur = head->blink;
  while ( cur != head ) {

    if ( ( x > cur->node->getX0() ) &&
         ( x < cur->node->getX1() ) &&
         ( y > cur->node->getY0() ) &&
         ( y < cur->node->getY1() ) ) {

      // only the highest object may participate
      if ( cur->node->dragValue( cur->node->getCurrentDragIndex() ) ) {
        cur->node->startDrag( x, y );
      }
      break; // out of while loop

    }

    cur = cur->blink;

  }

  return 1;

}

int activeGroupClass::selectDragValue (
  int x,
  int y )
{

activeGraphicListPtr head = (activeGraphicListPtr) voidHead;
activeGraphicListPtr cur;
char *firstName, *nextName;

  cur = head->blink;
  while ( cur != head ) {

    if ( ( x > cur->node->getX0() ) &&
         ( x < cur->node->getX1() ) &&
         ( y > cur->node->getY0() ) &&
         ( y < cur->node->getY1() ) ) {

      currentDragIndex = 0;

      firstName = cur->node->firstDragName();
      if ( !firstName ) return 0;

      actWin->popupDragBegin(
       actWin->obj.getNameFromClass( cur->node->objName() ) );
      actWin->popupDragAddItem( (void *) cur->node, firstName );

      nextName = cur->node->nextDragName();
      while ( nextName ) {

        actWin->popupDragAddItem( (void *) cur->node, nextName );
        nextName = cur->node->nextDragName();

      }

      actWin->popupDragFinish( x, y );

      // only the highest object may participate
      break; // out of while loop

    }

    cur = cur->blink;

  }

  return 1;

}

char *activeGroupClass::dragValue (
  int i )
{

  return groupDragName;

}
