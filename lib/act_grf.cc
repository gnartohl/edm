//  edm - extensible display manager

//  edm - Copyright (C) 1999 John W. Sinclair

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

#define __act_grf_cc

#include "act_grf.h"
#include "act_win.h"
#include "app_pkg.h"
#include "undo.h"

#include "thread.h"

activeGraphicClass::activeGraphicClass ( void ) {

  baseName = new char[strlen("base")+1];
  strcpy( baseName, "base" );
  name = baseName;

  selected = 0;
  deleteRequest = 0;
  xOrigin = 0;
  yOrigin = 0;
  nextToEdit = NULL;
  nextSelectedToEdit = NULL;
  inGroup = 0;
  editMode = AGC_K_EDIT_PROPERTIES;
  strcpy( id, "" );
  currentDragIndex = 0;

}

void activeGraphicClass::clone ( const activeGraphicClass *source ) {

  actWin = source->actWin;
  xOrigin = source->xOrigin;
  yOrigin =source->yOrigin;
  x = source->x;
  y = source->y;
  w = source->w;
  h = source->h;
  sboxX = source->sboxX;
  sboxY = source->sboxY;
  sboxW = source->sboxW;
  sboxH = source->sboxH;
  orientation = source->orientation;
  nextToEdit = NULL; // we cannot copy this from source; must build list later
  nextSelectedToEdit = NULL;
  inGroup = source->inGroup;
  editMode = source->editMode;
  strncpy( id, source->id, 31 );

  selected = 0;
  deleteRequest = 0;
  currentDragIndex = 0;

}

activeGraphicClass::~activeGraphicClass ( void ) {

  if ( baseName ) {
    delete baseName;
    baseName = NULL;
  }

  if ( ef.formIsPoppedUp() ) {
    ef.popdown();
  }

}

int activeGraphicClass::destroy ( void ) {

  return 1;

}

void activeGraphicClass::updateDimensions ( void ) {

}

int activeGraphicClass::snapToGrid ( void )
{

  actWin->filterPosition( &x, &y, x, y );

  sboxX = x;
  sboxY = y;

  return 1;

}

int activeGraphicClass::snapSizeToGrid ( void )
{

int x1, y1;

  actWin->filterPosition( &x, &y, x, y );

  x1 = x + w;
  y1 = y + h;

  actWin->filterPosition( &x1, &y1, x1, y1 );

  if ( ( x1 > x ) && ( y1 > y ) ) {
    w = x1 - x;
    h = y1 - y;
  }

  sboxX = x;
  sboxY = y;
  sboxW = w;
  sboxH = h;

  return 1;

}

int activeGraphicClass::move (
  int _x,
  int _y ) {

  x += _x;
  y += _y;

  updateDimensions();

  return 1;

}

int activeGraphicClass::moveAbs (
  int _x,
  int _y ) {

  x = _x;
  y = _y;

  updateDimensions();

  return 1;

}

int activeGraphicClass::moveMidpointAbs (
  int _x,
  int _y ) {

  x = _x - w/2;
  y = _y - h/2;

  updateDimensions();

  return 1;

}

int activeGraphicClass::rotate (
  int xOrigin,
  int yOrigin,
  char direction ) // '+'=clockwise, '-'=counter clockwise
{

double dx0, dy0, dx1, dy1, dxOrig, dyOrig, dxPrime0, dyPrime0,
 dxPrime1, dyPrime1;

  //printf( "activeGraphicClass::rotate %c, xO=%-d, yO=%-d\n",
  // direction, xOrigin, yOrigin );

  dxOrig = (double) xOrigin;
  dyOrig = (double) yOrigin;


  if ( direction == '+' ) { // clockwise

    // translate
    dx0 = (double) ( x - dxOrig );
    dy0 = (double) ( dyOrig - y );

    //printf( "1 dx0=%-g, dy0=%-g\n", dx0, dy0 );

    // rotate
    dxPrime0 = dy0;
    dyPrime0 = dx0 * -1.0;

    //printf( "2 dxPrime0=%-g, dyPrime0=%-g\n", dxPrime0, dyPrime0 );

    // translate
    dxPrime0 += dxOrig;
    dyPrime0 = dyOrig - dyPrime0;

    //printf( "3 dxPrime0=%-g, dyPrime0=%-g\n", dxPrime0, dyPrime0 );


    // translate
    dx1 = (double) ( getX1() - dxOrig );
    dy1 = (double) ( dyOrig - getY1() );

    //printf( "1 dx1=%-g, dy1=%-g\n", dx1, dy1 );

    // rotate
    dxPrime1 = dy1;
    dyPrime1 = dx1 * -1.0;

    //printf( "2 dxPrime1=%-g, dyPrime1=%-g\n", dxPrime1, dyPrime1 );

    // translate
    dxPrime1 += dxOrig;
    dyPrime1 = dyOrig - dyPrime1;

    //printf( "3 dxPrime1=%-g, dyPrime1=%-g\n", dxPrime1, dyPrime1 );

    // compute final x, y, w, & h
    x = (int) dxPrime0;
    y = (int) dyPrime0;
    w = (int) ( dxPrime0 - dxPrime1 + 0.5 );
    h = (int) ( dyPrime1 - dyPrime0 + 0.5 );
    // adjust x;
    x -= w;

  }
  else { // counterclockwise

    // translate
    dx0 = (double) ( x - dxOrig );
    dy0 = (double) ( dyOrig - y );

    //printf( "1 dx0=%-g, dy0=%-g\n", dx0, dy0 );

    // rotate
    dxPrime0 = dy0 * -1.0;
    dyPrime0 = dx0;

    //printf( "2 dxPrime0=%-g, dyPrime0=%-g\n", dxPrime0, dyPrime0 );

    // translate
    dxPrime0 += dxOrig;
    dyPrime0 = dyOrig - dyPrime0;

    //printf( "3 dxPrime0=%-g, dyPrime0=%-g\n", dxPrime0, dyPrime0 );


    // translate
    dx1 = (double) ( getX1() - dxOrig );
    dy1 = (double) ( dyOrig - getY1() );

    //printf( "1 dx1=%-g, dy1=%-g\n", dx1, dy1 );

    // rotate
    dxPrime1 = dy1 * -1.0;
    dyPrime1 = dx1;

    //printf( "2 dxPrime1=%-g, dyPrime1=%-g\n", dxPrime1, dyPrime1 );

    // translate
    dxPrime1 += dxOrig;
    dyPrime1 = dyOrig - dyPrime1;

    //printf( "3 dxPrime1=%-g, dyPrime1=%-g\n", dxPrime1, dyPrime1 );

    // compute final x, y, w, & h
    x = (int) dxPrime0;
    y = (int) dyPrime0;
    w = (int) ( dxPrime1 - dxPrime0 + 0.5 );
    h = (int) ( dyPrime0 - dyPrime1 + 0.5 );
    // adjust y;
    y -= h;

  }

  return 1;

}

int activeGraphicClass::flip (
  int xOrigin,
  int yOrigin,
  char direction ) // 'H' or 'V'
{

double dx0, dy0, dxOrig, dyOrig, dxPrime0, dyPrime0;

  //printf( "flip %c, xO=%-d, yO=%-d\n", direction, xOrigin, yOrigin );

  dxOrig = (double) xOrigin;
  dyOrig = (double) yOrigin;


  if ( direction == 'H' ) { // horizontal

    // translate
    dx0 = (double) ( x - dxOrig );

    //printf( "1 dx0=%-g, dy0=%-g\n", dx0, dy0 );

    // move
    dxPrime0 = dx0 * -1.0;

    //printf( "2 dxPrime0=%-g, dyPrime0=%-g\n", dxPrime0, dyPrime0 );

    // translate
    dxPrime0 += dxOrig;

    //printf( "3 dxPrime0=%-g, dyPrime0=%-g\n", dxPrime0, dyPrime0 );

    // compute final x
    x = (int) dxPrime0 - w;

  }
  else { // vertical

    // translate
    dy0 = (double) ( dyOrig - y );

    //printf( "1 dx0=%-g, dy0=%-g\n", dx0, dy0 );

    // move
    dyPrime0 = dy0 * -1.0;

    //printf( "2 dxPrime0=%-g, dyPrime0=%-g\n", dxPrime0, dyPrime0 );

    // translate
    dyPrime0 = dyOrig - dyPrime0;

    //printf( "3 dxPrime0=%-g, dyPrime0=%-g\n", dxPrime0, dyPrime0 );

    // compute final y
    y = (int) dyPrime0 - h;

  }

  return 1;

}

int activeGraphicClass::resize (
  int _x,
  int _y,
  int _w,
  int _h ) {

  x += _x;
  y += _y;
  w += _w;
  h += _h;

  updateDimensions();

  return 1;

}

int activeGraphicClass::resizeAbs (
  int _x,
  int _y,
  int _w,
  int _h ) {

  if ( _x >= 0 ) x = _x;
  if ( _y >= 0 ) y = _y;
  if ( _w >= 0 ) w = _w;
  if ( _h >= 0 ) h = _h;

  updateDimensions();

  return 1;

}

int activeGraphicClass::resizeAbsFromUndo (
  int _x,
  int _y,
  int _w,
  int _h ) {

int stat;

  stat = resizeAbs ( _x, _y, _w, _h );
  return stat;

}

int activeGraphicClass::draw ( void ) {

  return 1;

}

int activeGraphicClass::draw (
  int _x,
  int _y,
  int _w,
  int _h )
{

int stat;

  stat = draw();
  return stat;

}

int activeGraphicClass::drawActive ( void ) {

  return 1;

}

int activeGraphicClass::drawActive (
  int _x,
  int _y,
  int _w,
  int _h )
{

int stat;

  stat = drawActive();
  return stat;

}

int activeGraphicClass::drawAll ( void ) {

activeGraphicListPtr cur;

  cur = actWin->head->flink;
  while ( cur != actWin->head ) {
    cur->node->draw();
    cur = cur->flink;
  }

  return 1;

}

int activeGraphicClass::drawAllActive ( void ) {

activeGraphicListPtr cur;

//  actWin->appCtx->proc->lock();

  cur = actWin->head->flink;
  while ( cur != actWin->head ) {
    cur->node->bufInvalidate();
    cur->node->drawActive();
    cur = cur->flink;
  }

  //  actWin->appCtx->proc->unlock();

  return 1;

}

int activeGraphicClass::intersects (
  int x0,
  int y0,
  int x1,
  int y1 )
{

int xx0, yy0, xx1, yy1;

  xx0 = this->getX0();
  xx1 = this->getX1();
  yy0 = this->getY0();
  yy1 = this->getY1();

  if ( ( x0 <= xx0 ) &&
       ( x1 >= xx1 ) &&
       ( yy0 <= y0 ) &&
       ( yy1 >= y1 ) ) {

    return 1;

  }

  if ( ( y0 <= yy0 ) &&
       ( y1 >= yy1 ) &&
       ( xx0 <= x0 ) &&
       ( xx1 >= x1 ) ) {

    return 1;

  }

  if ( ( x0 >= xx0 ) &&
       ( x0 <= xx1 ) &&
       ( y0 >= yy0 ) &&
       ( y0 <= yy1 ) ) {

    return 1;

  }

  if ( ( x0 >= xx0 ) &&
       ( x0 <= xx1 ) &&
       ( y1 >= yy0 ) &&
       ( y1 <= yy1 ) ) {

    return 1;

  }

  if ( ( x1 >= xx0 ) &&
       ( x1 <= xx1 ) &&
       ( y0 >= yy0 ) &&
       ( y0 <= yy1 ) ) {

    return 1;

  }

  if ( ( x1 >= xx0 ) &&
       ( x1 <= xx1 ) &&
       ( y1 >= yy0 ) &&
       ( y1 <= yy1 ) ) {

    return 1;

  }

  if ( ( xx0 >= x0 ) &&
       ( xx0 <= x1 ) &&
       ( yy0 >= y0 ) &&
       ( yy0 <= y1 ) ) {

    return 1;

  }

  if ( ( xx0 >= x0 ) &&
       ( xx0 <= x1 ) &&
       ( yy1 >= y0 ) &&
       ( yy1 <= y1 ) ) {

    return 1;

  }

  if ( ( xx1 >= x0 ) &&
       ( xx1 <= x1 ) &&
       ( yy0 >= y0 ) &&
       ( yy0 <= y1 ) ) {

    return 1;

  }

  if ( ( xx1 >= x0 ) &&
       ( xx1 <= x1 ) &&
       ( yy1 >= y0 ) &&
       ( yy1 <= y1 ) ) {

    return 1;

  }

  return 0;

}

int activeGraphicClass::smartDrawAll ( void ) {

activeGraphicListPtr cur;
int x0, x1, y0, y1;
XRectangle xR = { this->x-5, this->y-5, this->w+10, this->h+10 };

//  actWin->appCtx->proc->lock();

  x0 = this->getX0()-5;
  y0 = this->getY0()-5;
  x1 = this->getX1()+5;
  y1 = this->getY1()+5;

  this->bufInvalidate();
  this->erase();

  actWin->drawGc.addNormXClipRectangle( xR );

  cur = actWin->head->flink;
  while ( cur != actWin->head ) {
    if ( cur->node->intersects( x0-5, y0-5, x1+5, y1+5 ) ) {
      cur->node->bufInvalidate();
      cur->node->draw( x0, y0, x1, y1 );
    }
    cur = cur->flink;
  }

  actWin->drawGc.removeNormXClipRectangle();

  //  actWin->appCtx->proc->unlock();

  return 1;

}

int activeGraphicClass::smartDrawAllActive ( void ) {

activeGraphicListPtr cur;
int x0, x1, y0, y1;
XRectangle xR = { this->x-5, this->y-5, this->w+10, this->h+10 };

//  actWin->appCtx->proc->lock();

  x0 = this->getX0()-5;
  y0 = this->getY0()-5;
  x1 = this->getX1()+5;
  y1 = this->getY1()+5;

  this->bufInvalidate();
  this->eraseActive();

  actWin->executeGc.addNormXClipRectangle( xR );

  cur = actWin->head->flink;
  while ( cur != actWin->head ) {
    if ( cur->node->intersects( x0-5, y0-5, x1+5, y1+5 ) ) {
      cur->node->bufInvalidate();
      cur->node->drawActive( x0, y0, x1, y1 );
    }
    cur = cur->flink;
  }

  actWin->executeGc.removeNormXClipRectangle();

  //  actWin->appCtx->proc->unlock();

  return 1;

}

int activeGraphicClass::clear ( void ) {

  XClearWindow( actWin->d, XtWindow(actWin->drawWidget) );

  return 1;

}

int activeGraphicClass::clearActive ( void ) {

  XClearWindow( actWin->d, XtWindow(actWin->executeWidget) );

  return 1;

}

int activeGraphicClass::refresh (
  int _x,
  int _y,
  int _w,
  int _h )
{

activeGraphicListPtr cur, next;
int clipStat, x0, x1, y0, y1, x0sb, x1sb, y0sb, y1sb;
XRectangle xR = { _x, _y, _w, _h };

  x0 = _x;
  y0 = _y;
  x1 = _x + _w;
  y1 = _y + _h;

  x0sb = _x - 3;
  y0sb = _y - 3;
  x1sb = _x + _w + 3;
  y1sb = _y + _h + 3;

  cur = actWin->head->flink;
  while ( cur != actWin->head ) {

    next = cur->flink;

    if ( cur->node->deleteRequest ) {

      cur->blink->flink = cur->flink;
      cur->flink->blink = cur->blink;

      delete cur->node;
      delete cur;

    }
    else {

      if ( cur->node->isSelected() ) {
        if ( cur->node->intersects( x0sb, y0sb, x1sb, y1sb ) ) {
          cur->node->eraseSelectBoxCorners();
	}
      }

    }

    cur = next;

  }

  clipStat = actWin->drawGc.addNormXClipRectangle( xR );

  cur = actWin->head->flink;
  while ( cur != actWin->head ) {
    if ( cur->node->intersects( x0, y0, x1, y1 ) ) {
      cur->node->bufInvalidate();
      cur->node->draw( x0, y0, x1, y1 );
    }
    cur = cur->flink;
  }

  if ( clipStat & 1 )
    actWin->drawGc.removeNormXClipRectangle();

  cur = actWin->head->flink;
  while ( cur != actWin->head ) {
    if ( cur->node->isSelected() ) {
      if ( cur->node->intersects( x0sb, y0sb, x1sb, y1sb ) ) {
        cur->node->drawSelectBoxCorners();
      }
    }
    cur = cur->flink;
  }

  return 1;

}

int activeGraphicClass::refresh ( void ) {

activeGraphicListPtr cur, next;

  cur = actWin->head->flink;
  while ( cur != actWin->head ) {

    next = cur->flink;

    if ( cur->node->deleteRequest ) {

      cur->blink->flink = cur->flink;
      cur->flink->blink = cur->blink;

      delete cur->node;
      delete cur;

    }
    else {

      if ( cur->node->isSelected() ) {
        cur->node->eraseSelectBoxCorners();
      }

    }

    cur = next;

  }

  cur = actWin->head->flink;
  while ( cur != actWin->head ) {
    cur->node->draw();
    cur = cur->flink;
  }

  cur = actWin->head->flink;
  while ( cur != actWin->head ) {
    if ( cur->node->isSelected() ) {
      cur->node->drawSelectBoxCorners();
    }
    cur = cur->flink;
  }

  return 1;

}

int activeGraphicClass::refresh (
  activeGraphicClass *oneNode ) {

activeGraphicListPtr cur, next;

  cur = actWin->head->flink;
  while ( cur != actWin->head ) {

    next = cur->flink;

    if ( cur->node->deleteRequest ) {

      cur->blink->flink = cur->flink;
      cur->flink->blink = cur->blink;

      delete cur->node;
      delete cur;

    }

    cur = next;

  }

  cur = actWin->head->flink;
  while ( cur != actWin->head ) {
    cur->node->draw();
    cur = cur->flink;
  }

  cur = actWin->head->flink;
  while ( cur != actWin->head ) {
    if ( cur->node->isSelected() && cur->node == oneNode ) {
      cur->node->drawSelectBoxCorners();
    }
    cur = cur->flink;
  }

  return 1;

}

void activeGraphicClass::flushActive ( void ) {

  XFlush( actWin->d );

}

int activeGraphicClass::refreshActive (
  int _x,
  int _y,
  int _w,
  int _h )
{

activeGraphicListPtr cur;
int clipStat, x0, x1, y0, y1;
XRectangle xR = { _x, _y, _w, _h };

  x0 = _x;
  y0 = _y;
  x1 = _x + _w;
  y1 = _y + _h;

  //  actWin->appCtx->proc->lock();

  cur = actWin->head->flink;
  while ( cur != actWin->head ) {
    if ( cur->node->intersects( x0, y0, x1, y1 ) ) {
      clipStat = actWin->executeGc.addNormXClipRectangle( xR );
      cur->node->bufInvalidate();
      cur->node->drawActive( x0, y0, x1, y1 );
      if ( clipStat & 1 ) actWin->executeGc.removeNormXClipRectangle();
    }
    cur = cur->flink;
  }

  //  actWin->appCtx->proc->unlock();

  return 1;

}

int activeGraphicClass::refreshActive ( void ) {

activeGraphicListPtr cur;

//  actWin->appCtx->proc->lock();

  cur = actWin->head->flink;
  while ( cur != actWin->head ) {
    cur->node->bufInvalidate();
    cur->node->drawActive();
    cur = cur->flink;
  }

  //  actWin->appCtx->proc->unlock();

  return 1;

}

void activeGraphicClass::initSelectBox ( void ) {

  sboxX = x;
  sboxY = y;
  sboxW = w;
  sboxH = h;

}

int activeGraphicClass::eraseSelectBoxCorners ( void ) {

int boxX;
int boxY;
int boxW;
int boxH;

  if ( deleteRequest ) return 1;

  actWin->executeGc.setLineWidth( 1 );
  actWin->drawGc.setLineStyle( LineSolid );

  boxX = sboxX-3;
  boxY = sboxY-3;
  boxW = 6;
  boxH = 6;
  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), boxX, boxY, boxW, boxH );

  boxY = sboxY+sboxH/2-3;
  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), boxX, boxY, boxW, boxH );   

  boxY = sboxY+sboxH-3;
  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), boxX, boxY, boxW, boxH );   

  boxX = sboxX+sboxW-3;
  boxY = sboxY-3;
  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), boxX, boxY, boxW, boxH );

  boxY = sboxY+sboxH/2-3;
  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), boxX, boxY, boxW, boxH );   

  boxY = sboxY+sboxH-3;
  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), boxX, boxY, boxW, boxH );   

  boxX = sboxX+sboxW/2-3;
  boxY = sboxY-3;
  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), boxX, boxY, boxW, boxH );

  boxY = sboxY+sboxH-3;
  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), boxX, boxY, boxW, boxH );   

  return 1;

}

int activeGraphicClass::eraseSelectBox ( void ) {

  if ( deleteRequest ) return 1;

  actWin->drawGc.setLineStyle( LineSolid );

  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), sboxX, sboxY, sboxW, sboxH );

  return 1;

}

int activeGraphicClass::drawSelectBoxCorners ( void ) {

int boxX;
int boxY;
int boxW;
int boxH;

  if ( deleteRequest ) return 1;

  actWin->drawGc.saveFg();
  actWin->drawGc.setFG( actWin->fgColor );

  actWin->executeGc.setLineWidth( 1 );
  actWin->drawGc.setLineStyle( LineSolid );

  boxX = sboxX-3;
  boxY = sboxY-3;
  boxW = 6;
  boxH = 6;
  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.xorGC(), boxX, boxY, boxW, boxH );

  boxY = sboxY+sboxH/2-3;
  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.xorGC(), boxX, boxY, boxW, boxH );   

  boxY = sboxY+sboxH-3;
  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.xorGC(), boxX, boxY, boxW, boxH );   

  boxX = sboxX+sboxW-3;
  boxY = sboxY-3;
  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.xorGC(), boxX, boxY, boxW, boxH );

  boxY = sboxY+sboxH/2-3;
  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.xorGC(), boxX, boxY, boxW, boxH );   

  boxY = sboxY+sboxH-3;
  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.xorGC(), boxX, boxY, boxW, boxH );   

  boxX = sboxX+sboxW/2-3;
  boxY = sboxY-3;
  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.xorGC(), boxX, boxY, boxW, boxH );

  boxY = sboxY+sboxH-3;
  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.xorGC(), boxX, boxY, boxW, boxH );   

  actWin->drawGc.restoreFg();

  return 1;

}

int activeGraphicClass::drawSelectBox ( void ) {

  if ( deleteRequest ) return 1;

  actWin->drawGc.saveFg();
  actWin->drawGc.setFG( actWin->fgColor );

  actWin->drawGc.setLineStyle( LineSolid );

  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.xorGC(), sboxX, sboxY, sboxW, sboxH );

  actWin->drawGc.restoreFg();

  return 1;

}

int activeGraphicClass::moveSelectBox (
  int _x,
  int _y )
{

  sboxX += _x;
  sboxY += _y;

  return 1;

}

int activeGraphicClass::moveSelectBoxAbs (
  int _x,
  int _y )
{

  sboxX = _x;
  sboxY = _y;

  return 1;

}

int activeGraphicClass::moveSelectBoxMidpointAbs (
  int _x,
  int _y )
{

  sboxX = _x - sboxW/2;
  sboxY = _y - sboxH/2;

  return 1;

}

int activeGraphicClass::checkResizeSelectBox (
  int _x,
  int _y,
  int _w,
  int _h )
{

int tmpx, tmpy, tmpw, tmph, ret_stat;

  tmpx = sboxX;
  tmpy = sboxY;
  tmpw = sboxW;
  tmph = sboxH;

  ret_stat = 1;

  tmpx += _x;
  tmpy += _y;

  tmpw += _w;
  if ( tmpw < 2 ) {
    ret_stat = 0;
  }

  tmph += _h;
  if ( tmph < 2 ) {
    ret_stat = 0;
  }

  return ret_stat;

}

int activeGraphicClass::resizeSelectBox (
  int _x,
  int _y,
  int _w,
  int _h )
{

int savex, savey, savew, saveh, ret_stat;

  savex = sboxX;
  savey = sboxY;
  savew = sboxW;
  saveh = sboxH;

  ret_stat = 1;

  sboxX += _x;
  sboxY += _y;

  sboxW += _w;
  if ( sboxW < 2 ) {
    sboxX = savex;
    sboxW = savew;
    ret_stat = 0;
  }

  sboxH += _h;
  if ( sboxH < 2 ) {
    sboxY = savey;
    sboxH = saveh;
    ret_stat = 0;
  }

  return ret_stat;

}

int activeGraphicClass::checkResizeSelectBoxAbs (
  int _x,
  int _y,
  int _w,
  int _h )
{

int tmpx, tmpy, tmpw, tmph, ret_stat;

  ret_stat = 1;

  tmpx = _x;
  tmpy = _y;

  tmpw = _w;
  if ( tmpw != -1 ) {
    if ( tmpw < 2 ) {
      ret_stat = 0;
    }
  }

  tmph = _h;
  if ( tmph != -1 ) {
    if ( tmph < 2 ) {
      ret_stat = 0;
    }
  }

  return ret_stat;

}

int activeGraphicClass::resizeSelectBoxAbs (
  int _x,
  int _y,
  int _w,
  int _h )
{

  if ( _x >= 0 ) sboxX = _x;
  if ( _y >= 0 ) sboxY = _y;
  if ( _w >= 0 ) sboxW = _w;
  if ( _h >= 0 ) sboxH = _h;

  return 1;

}

int activeGraphicClass::resizeSelectBoxAbsFromUndo (
  int _x,
  int _y,
  int _w,
  int _h )
{

int stat;

  stat = resizeSelectBoxAbs( _x, _y, _w, _h );
  return stat;

}

int activeGraphicClass::getSelectBoxOperation (
  int _x,
  int _y )
{

int x0, y0, x1, y1, xx0, yy0, xx1, yy1;
int boxW = 6;
int boxH = 6;

  x0 = x - 5;
  x1 = x + w + 5;
  y0 = y - 5;
  y1 = y + h + 5;

  if ( ( _x >= x0 ) && ( _x <= x1 ) && ( _y >= y0 ) && ( _y <= y1 ) ) {

    // pointer is close enough for further checks

    // upper left
    xx0 = x - boxW/2 - 1;
    xx1 = xx0 + boxW + 2;
    yy0 = y - boxH/2 - 1;
    yy1 = yy0 + boxH + 2;

    if ( ( _x >= xx0 ) && ( _x <= xx1 ) && ( _y >= yy0 ) && ( _y <= yy1 ) ) {

      return AGC_LEFT_TOP_OP;

    }

    // left
    xx0 = x - boxW/2 - 1;
    xx1 = xx0 + boxW + 2;
    yy0 = y + h/2 - boxH/2 - 1;
    yy1 = yy0 + boxH + 2;

    if ( ( _x >= xx0 ) && ( _x <= xx1 ) && ( _y >= yy0 ) && ( _y <= yy1 ) ) {

      return AGC_LEFT_OP;

    }

    // lower left
    xx0 = x - boxW/2 - 1;
    xx1 = xx0 + boxW + 2;
    yy0 = y + h - boxH/2 - 1;
    yy1 = yy0 + boxH + 2;

    if ( ( _x >= xx0 ) && ( _x <= xx1 ) && ( _y >= yy0 ) && ( _y <= yy1 ) ) {

      return AGC_LEFT_BOTTOM_OP;

    }

    // top
    xx0 = x + w/2 - boxW/2 - 1;
    xx1 = xx0 + boxW + 2;
    yy0 = y - boxH/2 - 1;
    yy1 = yy0 + boxH + 2;

    if ( ( _x >= xx0 ) && ( _x <= xx1 ) && ( _y >= yy0 ) && ( _y <= yy1 ) ) {

      return AGC_TOP_OP;

    }

    // bottom
    xx0 = x + w/2 - boxW/2 - 1;
    xx1 = xx0 + boxW + 2;
    yy0 = y + h - boxH/2 - 1;
    yy1 = yy0 + boxH + 2;

    if ( ( _x >= xx0 ) && ( _x <= xx1 ) && ( _y >= yy0 ) && ( _y <= yy1 ) ) {

      return AGC_BOTTOM_OP;

    }

    // upper right
    xx0 = x + w - boxW/2 - 1;
    xx1 = xx0 + boxW + 2;
    yy0 = y - boxH/2 - 1;
    yy1 = yy0 + boxH + 2;

    if ( ( _x >= xx0 ) && ( _x <= xx1 ) && ( _y >= yy0 ) && ( _y <= yy1 ) ) {

      return AGC_RIGHT_TOP_OP;

    }

    // right
    xx0 = x + w - boxW/2 - 1;
    xx1 = xx0 + boxW + 2;
    yy0 = y + h/2 - boxH/2 - 1;
    yy1 = yy0 + boxH + 2;

    if ( ( _x >= xx0 ) && ( _x <= xx1 ) && ( _y >= yy0 ) && ( _y <= yy1 ) ) {

      return AGC_RIGHT_OP;

    }

    // lower right
    xx0 = x + w - boxW/2 - 1;
    xx1 = xx0 + boxW + 2;
    yy0 = y + h - boxH/2 - 1;
    yy1 = yy0 + boxH + 2;

    if ( ( _x >= xx0 ) && ( _x <= xx1 ) && ( _y >= yy0 ) && ( _y <= yy1 ) ) {

      return AGC_RIGHT_BOTTOM_OP;

    }

  }

  // check for move grab
  x1 = x + w;
  y1 = y + h;

  if ( ( _x >= x ) && ( _x <= x1 ) && ( _y >= y ) && ( _y <= y1 ) ) {

    return AGC_MOVE_OP;

  }

  return 0;

}

void activeGraphicClass::setSelected ( void )
{

    selected = 1;

}

int activeGraphicClass::select (
  int _x,
  int _y )
{

  if ( deleteRequest ) return 0;

  if ( ( _x >= x ) && ( _x <= x+w ) && ( _y >= y ) &&
       ( _y <= y+h ) ) {

    selected = 1;
    return 1;

  }
  else {

    return 0;

  }

}

int activeGraphicClass::selectEnclosed (
  int _x,
  int _y,
  int _w,
  int _h )
{

  if ( deleteRequest ) return 0;

  if ( ( x >= _x ) && ( x <= _x+_w ) &&
       ( x+w >= _x ) && ( x+w <= _x+_w ) &&
       ( y >= _y ) && ( y <= _y+_h ) &&
       ( y+h >= _y ) && ( y+h <= _y+_h ) ) {

    selected = 1;
    return 1;

  }
  else {

    return 0;

  }

}

int activeGraphicClass::selectTouching (
  int _x,
  int _y,
  int _w,
  int _h )
{

  if ( deleteRequest ) return 0;

  if ( ( x >= _x ) && ( x <= _x+_w ) &&
       ( y >= _y ) && ( y <= _y+_h ) ) {

    selected = 1;
    return 1;

  }

  if ( ( x+w >= _x ) && ( x+w <= _x+_w ) &&
       ( y >= _y ) && ( y <= _y+_h ) ) {

    selected = 1;
    return 1;

  }

  if ( ( x >= _x ) && ( x <= _x+_w ) &&
       ( y+h >= _y ) && ( y+h <= _y+_h ) ) {

    selected = 1;
    return 1;

  }

  if ( ( x+w >= _x ) && ( x+w <= _x+_w ) &&
       ( y+h >= _y ) && ( y+h <= _y+_h ) ) {

    selected = 1;
    return 1;

  }

  return 0;

}

void activeGraphicClass::deselect ( void ) {

  selected = 0;

}

int activeGraphicClass::erase ( void ) {

  return 1;

}

int activeGraphicClass::eraseActive ( void ) {

  return 1;

}

void activeGraphicClass::bufInvalidate ( void ) {

}

int activeGraphicClass::activate (
  int pass )
{

  return 1;

}

int activeGraphicClass::activate (
  int pass,
  void *ptr )
{

  activate( pass );

  return 1;

}

int activeGraphicClass::reactivate (
  int pass )
{

int stat;

  stat = activate( pass );
  return stat;

}

int activeGraphicClass::reactivate (
  int pass,
  void *ptr )
{

int stat;

  stat = activate( pass, ptr );
  return stat;

}

int activeGraphicClass::deactivate ( void ) {

  return 1;

}

int activeGraphicClass::deactivate (
  int pass )
{

int stat;

  if ( pass == 2 ) {
    stat = deactivate();
    return stat;
  }

  return 1;

}

int activeGraphicClass::preReactivate (
  int pass )
{

int stat;

  stat = deactivate( pass );
  return stat;

}

int activeGraphicClass::raise ( void ) {

  return 1;

}

int activeGraphicClass::lower ( void ) {

  return 1;

}

int activeGraphicClass::createInteractive (
  activeWindowClass *actWin,
  int x,
  int y,
  int w,
  int h ) {

  return 1;

}

int activeGraphicClass::createFromFile (
  FILE *fptr,
  char *name,
  activeWindowClass *actWin )
{

  return 1;

}

int activeGraphicClass::importFromXchFile (
  FILE *f,
  char *name,
  activeWindowClass *actWin )
{

int more;
char *gotData, *context, *tk, buf[255+1];

  // continue until tag is <eod>

  do {

    gotData = getNextDataString( buf, 255, f );
    if ( !gotData ) {
      actWin->appCtx->postMessage( activeGraphicClass_str1 );
      return 0;
    }

    context = NULL;

    tk = strtok_r( buf, " \t\n", &context );
    if ( !tk ) {
      actWin->appCtx->postMessage( activeGraphicClass_str1 );
      return 0;
    }

    if ( strcmp( tk, "<eod>" ) == 0 ) {

      more = 0;

    }
    else {

      more = 1;

    }

  } while ( more );

  return 1;

}

int activeGraphicClass::save (
  FILE *fptr )
{

  return 1;

}

activeGraphicClass *activeGraphicClass::copy ( void ) {

  return NULL;

}

activeGraphicClass *activeGraphicClass::cut ( void ) {

  return NULL;

}

int activeGraphicClass::paste ( void ) {

  return 1;

}

int activeGraphicClass::doEdit ( void ) {

activeGraphicListPtr cur;
int stat, isGroup;

  if ( strcmp( objName(), "activeGroupClass" ) == 0 )
    isGroup = 1;
  else
    isGroup = 0;

  if ( !isGroup ) {

    cur = actWin->selectedHead->selFlink;
    while ( cur != actWin->selectedHead ) {
      //if ( strcmp( cur->node->objName(), "activeGroupClass" ) != 0 ) {
        cur->node->drawSelectBoxCorners(); // erase all via xor
      //}
      cur = cur->selFlink;
    }

    actWin->setCurrentObject( this );

  }

  //  if ( inGroup && !isMultiPointObject() && !isGroup ) {
  //    drawSelectBoxCorners();
  //  }

  if ( !isGroup ) {
    drawSelectBoxCorners();
  }

  if ( isMultiPointObject() ) {

    if ( editPropertiesSet() ) {

      stat = edit();

    }
    else {

      stat = editLineSegments();

    }

  }
  else {

    stat = edit();

  }

  return stat;

}

int activeGraphicClass::edit ( void ) {

  actWin->state = actWin->savedState;

  return 1;

}

void activeGraphicClass::operationComplete ( void )
{

  this->actWin->refresh();

  if ( nextToEdit ) {
    nextToEdit->doEdit();
  }
  else if ( nextSelectedToEdit ) {
    nextSelectedToEdit->doEdit();
  }
  else {
    this->actWin->operationComplete();
  }

}

void activeGraphicClass::operationCancel ( void )
{

  if ( inGroup && !isMultiPointObject() ) {
    drawSelectBoxCorners(); // erase via xor
    refresh();
  }
  this->actWin->refresh();
  this->actWin->operationComplete();

}

void activeGraphicClass::lineEditBegin ( void )
{

  this->actWin->lineEditBegin();

}

int activeGraphicClass::addPoint (
  int x,
  int y )
{

  return 1;

}

int activeGraphicClass::removeLastPoint ( void )
{

  return 1;

}

pointPtr activeGraphicClass::selectPoint (
  int x,
  int y )
{

  return (pointPtr) NULL;

}

int activeGraphicClass::movePoint (
  pointPtr curPoint,
  int x,
  int y )
{

  return 1;

}

int activeGraphicClass::lineEditComplete ( void ) {

  return 1;

}

int activeGraphicClass::lineEditCancel ( void ) {

  return 1;

}

int activeGraphicClass::createGroup (
  activeWindowClass *actWin )
{

  return 1;

}

int activeGraphicClass::ungroup (
  void *curListNode )
{

  return 1;

}

void activeGraphicClass::adjustCoordinates (
  int _xOrigin,
  int _yOrigin )
{

}

void activeGraphicClass::btnUp (
  int x,
  int y,
  int buttonState,
  int buttonNumber )
{

}

void activeGraphicClass::btnUp (
  int x,
  int y,
  int buttonState,
  int buttonNumber,
  int *action )
{

  *action = 0;
  btnUp( x, y, buttonState, buttonNumber );

}

void activeGraphicClass::btnDown (
  int x,
  int y,
  int buttonState,
  int buttonNumber )
{

}

void activeGraphicClass::btnDown (
  int x,
  int y,
  int buttonState,
  int buttonNumber,
  int *action )
{

  *action = 0;
  btnDown( x, y, buttonState, buttonNumber );

}

void activeGraphicClass::btnDrag (
  int x,
  int y,
  int buttonState,
  int buttonNumber )
{

}

void activeGraphicClass::pointerIn (
  int _x,
  int _y,
  int buttonState )
{

  actWin->executeGc.saveFg();

  actWin->executeGc.setFG( actWin->fgColor );
  actWin->executeGc.setLineWidth( 2 );
  actWin->executeGc.setLineStyle( LineSolid );

  XDrawRectangle( actWin->d, XtWindow(actWin->executeWidget),
   actWin->executeGc.normGC(), x-2, y-2, w+4, h+4 );

  actWin->executeGc.setLineWidth( 1 );

  actWin->executeGc.restoreFg();

}

void activeGraphicClass::pointerOut (
  int _x,
  int _y,
  int buttonState )
{

  actWin->executeGc.setLineWidth( 2 );
  actWin->executeGc.setLineStyle( LineSolid );

  XDrawRectangle( actWin->d, XtWindow(actWin->executeWidget),
   actWin->executeGc.eraseGC(), x-2, y-2, w+4, h+4 );

  refreshActive( x-4, y-4, w+8, h+8 );

  actWin->executeGc.setLineWidth( 1 );

}

int activeGraphicClass::initDefExeNode (
  void *ptr )
{

int stat;

  stat = actWin->initDefExeNode( ptr );

  return stat;

}

int activeGraphicClass::getButtonActionRequest (
  int *up,
  int *down,
  int *drag )
{

  *up = 0;
  *down = 0;
  *drag = 0;

  return 1;

}

int activeGraphicClass::getButtonActionRequest (
  int *up,
  int *down,
  int *drag,
  int *focus )
{

  *focus = 0;

  return getButtonActionRequest( up, down, drag );

}

int activeGraphicClass::createWidgets ( void ) {

  return 1;

}

void activeGraphicClass::updateFont (
  char *string,
  char *fontTag,
  XFontStruct **fs,
  int *ascent,
  int *descent,
  int *height,
  int *width )
{

int l;
char msg[127+1];

  if ( string )
    l = strlen(string);
  else
    l = 0;

  if ( *fs ) {

    *ascent = (*fs)->ascent;
    *descent = (*fs)->descent;
    *height = *ascent + *descent;
    *width = XTextWidth( *fs, string, l );

  }
  else {

    sprintf( msg, activeGraphicClass_str2, fontTag );
    actWin->appCtx->postMessage( msg );
    strcpy( fontTag, actWin->defaultFontTag );
    sprintf( msg, activeGraphicClass_str3, fontTag );
    actWin->appCtx->postMessage( msg );
    *fs = actWin->fi->getXFontStruct( fontTag );
    if ( *fs ) {

      *ascent = (*fs)->ascent;
      *descent = (*fs)->descent;
      *height = *ascent + *descent;
      *width = XTextWidth( *fs, string, l );
      actWin->setChanged();

    }
    else {

      sprintf( msg, activeGraphicClass_str4, fontTag );
      actWin->appCtx->postMessage( msg );
      strcpy( fontTag, actWin->fi->defaultSiteFont() );
      sprintf( msg, activeGraphicClass_str5, fontTag );
      actWin->appCtx->postMessage( msg );
      *fs = actWin->fi->getXFontStruct( fontTag );
      if ( *fs ) {

        *ascent = (*fs)->ascent;
        *descent = (*fs)->descent;
        *height = *ascent + *descent;
        *width = XTextWidth( *fs, string, l );
        actWin->setChanged();

      }
      else {

        sprintf( msg, activeGraphicClass_str6, fontTag );
        actWin->appCtx->postMessage( msg );
        *ascent = 15;
        *descent = 5;
        *height = *ascent + *descent;
        *width = 20;

      }

    }

  }

}

void activeGraphicClass::updateFont (
  char *fontTag,
  XFontStruct **fs,
  int *ascent,
  int *descent,
  int *height )
{

char msg[127+1];

  if ( *fs ) {

    *ascent = (*fs)->ascent;
    *descent = (*fs)->descent;
    *height = *ascent + *descent;

  }
  else {

    sprintf( msg, activeGraphicClass_str2, fontTag );
    actWin->appCtx->postMessage( msg );
    strcpy( fontTag, actWin->defaultFontTag );
    sprintf( msg, activeGraphicClass_str3, fontTag );
    actWin->appCtx->postMessage( msg );
    *fs = actWin->fi->getXFontStruct( fontTag );
    if ( *fs ) {

      *ascent = (*fs)->ascent;
      *descent = (*fs)->descent;
      *height = *ascent + *descent;
      actWin->setChanged();

    }
    else {

      sprintf( msg, activeGraphicClass_str4, fontTag );
      actWin->appCtx->postMessage( msg );
      strcpy( fontTag, actWin->fi->defaultSiteFont() );
      sprintf( msg, activeGraphicClass_str5, fontTag );
      actWin->appCtx->postMessage( msg );
      *fs = actWin->fi->getXFontStruct( fontTag );
      if ( *fs ) {

        *ascent = (*fs)->ascent;
        *descent = (*fs)->descent;
        *height = *ascent + *descent;
        actWin->setChanged();

      }
      else {

        sprintf( msg, activeGraphicClass_str6, fontTag );
        actWin->appCtx->postMessage( msg );
        *ascent = 15;
        *descent = 5;
        *height = *ascent + *descent;

      }

    }

  }

}

void activeGraphicClass::updateGroup ( void ) {

}

void activeGraphicClass::executeDeferred ( void ) {

}

void activeGraphicClass::executeFromDeferredQueue ( void ) {

}

void activeGraphicClass::setNextSelectedToEdit (
  activeGraphicClass *ptr )
{

  nextSelectedToEdit = ptr;

}

void activeGraphicClass::clearNextSelectedToEdit ( void ) {

  nextSelectedToEdit = NULL;

}

void activeGraphicClass::setNextToEdit (
  activeGraphicClass *ptr )
{

  nextToEdit = ptr;

}

void activeGraphicClass::clearNextToEdit ( void ) {

  nextToEdit = NULL;

}

void activeGraphicClass::setInGroup ( void ) {

  inGroup = 1;

}

void activeGraphicClass::clearInGroup ( void ) {

  inGroup = 0;

}

activeGraphicClass *activeGraphicClass::getTail ( void ) {

  return NULL;

}

void activeGraphicClass::setEditProperties ( void ) {

  editMode = AGC_K_EDIT_PROPERTIES;

}

void activeGraphicClass::setEditSegments ( void ) {

  editMode = AGC_K_EDIT_SEGMENTS;

}

int activeGraphicClass::editPropertiesSet ( void ) {

  if ( editMode == AGC_K_EDIT_PROPERTIES )
    return 1;
  else
    return 0;

}

int activeGraphicClass::editSegmentsSet ( void ) {

  if ( editMode == AGC_K_EDIT_SEGMENTS )
    return 1;
  else
    return 0;

}

int activeGraphicClass::editLineSegments ( void ) {

  return 1;

}

char *activeGraphicClass::idName( void ) {

  return id;

}

int activeGraphicClass::setProperty (
  char *property,
  char *value )
{

  return 0;  // default error return (property not found)

}

int activeGraphicClass::setProperty (
  char *property,
  double *value )
{

  return 0;  // default error return (property not found)

}

int activeGraphicClass::setProperty (
  char *property,
  int *value )
{

  return 0;  // default error return (property not found)

}

int activeGraphicClass::getProperty (
  char *property,
  int bufSize,
  char *value )
{

  return 0;  // default error return (property not found)

}

int activeGraphicClass::getProperty (
  char *property,
  double *value )
{

  return 0;  // default error return (property not found)

}

int activeGraphicClass::getProperty (
  char *property,
  int *value )
{

  return 0;  // default error return (property not found)

}

// ------------------------

int activeGraphicClass::setProperty (
  char *itemId,
  char *property,
  char *value )
{

int stat;

  stat = actWin->setProperty( itemId, property, value );
  return stat;

}

int activeGraphicClass::setProperty (
  char *itemId,
  char *property,
  double *value )
{

int stat;

  stat = actWin->setProperty( itemId, property, value );
  return stat;

}

int activeGraphicClass::setProperty (
  char *itemId,
  char *property,
  int *value )
{

int stat;

  stat = actWin->setProperty( itemId, property, value );
  return stat;

}

int activeGraphicClass::getProperty (
  char *itemId,
  char *property,
  int bufSize,
  char *value )
{

int stat;

  stat = actWin->getProperty( itemId, property, bufSize, value );
  return stat;

}

int activeGraphicClass::getProperty (
  char *itemId,
  char *property,
  double *value )
{

int stat;

  stat = actWin->getProperty( itemId, property, value );
  return stat;

}

int activeGraphicClass::getProperty (
  char *itemId,
  char *property,
  int *value )
{

int stat;

  stat = actWin->getProperty( itemId, property, value );
  return stat;

}

// ---------------------------

int activeGraphicClass::setProperty (
  char *winId,
  char *itemId,
  char *property,
  char *value )
{

int stat;

  stat = actWin->appCtx->setProperty( winId, itemId, property, value );
  return stat;

}

int activeGraphicClass::setProperty (
  char *winId,
  char *itemId,
  char *property,
  double *value )
{

int stat;

  stat = actWin->appCtx->setProperty( winId, itemId, property, value );
  return stat;

}

int activeGraphicClass::setProperty (
  char *winId,
  char *itemId,
  char *property,
  int *value )
{

int stat;

  stat = actWin->appCtx->setProperty( winId, itemId, property, value );
  return stat;

}

int activeGraphicClass::getProperty (
  char *winId,
  char *itemId,
  char *property,
  int bufSize,
  char *value )
{

int stat;

  stat = actWin->appCtx->getProperty( winId, itemId, property, bufSize,
   value );
  return stat;

}

int activeGraphicClass::getProperty (
  char *winId,
  char *itemId,
  char *property,
  double *value )
{

int stat;

  stat = actWin->appCtx->getProperty( winId, itemId, property, value );
  return stat;

}

int activeGraphicClass::getProperty (
  char *winId,
  char *itemId,
  char *property,
  int *value )
{

int stat;

  stat = actWin->appCtx->getProperty( winId, itemId, property, value );
  return stat;

}

int activeGraphicClass::openDisplay (
  char *fileName,
  int setPosition )
{

activeWindowListPtr cur;

  cur = actWin->appCtx->head->flink;
  while ( cur != actWin->appCtx->head ) {
    if ( strcmp( fileName, cur->node.fileName ) == 0 ) {
      return 0;  // display is already open; don't open another instance
    }
    cur = cur->flink;
  }

  cur = new activeWindowListType;
  actWin->appCtx->addActiveWindow( cur );

  cur->node.create( actWin->appCtx, NULL, 0, 0, 0, 0,
   actWin->appCtx->numMacros, actWin->appCtx->macros,
  actWin->appCtx->expansions );
  cur->node.realize();
  cur->node.setGraphicEnvironment( &actWin->appCtx->ci, &actWin->appCtx->fi );

  strncpy( cur->node.fileName, fileName,
   sizeof(cur->node.fileName)-1 );

  if ( setPosition ) {
  actWin->appCtx->openActivateActiveWindow( &cur->node, actWin->x+x,
   actWin->y+y );
  }
  else {
    actWin->appCtx->openActivateActiveWindow( &cur->node );
  }

  return 1;

}

static void dragFin (
  Widget w,
  XtPointer clientData,
  XtPointer call_data )
{

}

static Boolean cvt (
  Widget w,
  Atom *selection,
  Atom *target,
  Atom *type_return,
  XtPointer *value_return,
  unsigned long *len_return,
  int *format_return )
{

Display *d = XtDisplay( w );
class activeGraphicClass *ago;
Atom MOTIF_DROP = XmInternAtom( d, "_MOTIF_DROP", false );
int l;

  if ( *selection != MOTIF_DROP ) {
    return false;
  }

  if ( *target != XA_STRING ) {
    return false;
  }

  XtVaGetValues( w, XmNclientData, &ago, NULL );

  if ( !ago->dragValue( ago->currentDragIndex ) ) {
    return false;
  }

  l = strlen( ago->dragValue( ago->currentDragIndex ) );
  if ( l < 1 ) {
    return false;
  }

  ago->dragData = new (char)[l+1];
  strncpy( ago->dragData, ago->dragValue( ago->currentDragIndex ), l );
  ago->dragData[l] = 0;

  *type_return = *target;
  *value_return = ago->dragData;
  *len_return = l+1;
  *format_return = 8;

  return true;

}

int activeGraphicClass::selectDragValue (
  int x,
  int y )
{

char *firstName, *nextName;

  currentDragIndex = 0;

  firstName = firstDragName();
  if ( !firstName ) return 0;

  actWin->popupDragBegin( actWin->obj.getNameFromClass( objName() ) );
  actWin->popupDragAddItem( (void *) this, firstName );

  nextName = nextDragName();
  while ( nextName ) {

    actWin->popupDragAddItem( (void *) this, nextName );
    nextName = nextDragName();

  }

  actWin->popupDragFinish( x, y );

  return 1;

}

int activeGraphicClass::startDrag ( void ) {

Atom expList[1];
int n;
Arg args[10];
XMotionEvent dragEvent;

  expList[0] = XA_STRING;
  n = 0;
  XtSetArg( args[n], XmNexportTargets, expList ); n++;
  XtSetArg( args[n], XmNnumExportTargets, 1 ); n++;
  XtSetArg( args[n], XmNdragOperations, XmDROP_COPY ); n++;
  XtSetArg( args[n], XmNconvertProc, cvt ); n++;
  XtSetArg( args[n], XmNclientData, this ); n++;
    
  memset( (char *) &dragEvent, 0, sizeof(dragEvent) );
  dragEvent.type = MotionNotify;

  dc = XmDragStart( actWin->executeWidget, (XEvent *) &dragEvent,
   args, n );
  XtAddCallback( dc, XmNdragDropFinishCallback, dragFin, this );

  return 1;

}

// for motif widgets
int activeGraphicClass::startDrag (
  Widget w,
  XEvent *e )
{

Atom expList[1];
int n;
Arg args[10];

  expList[0] = XA_STRING;
  n = 0;
  XtSetArg( args[n], XmNexportTargets, expList ); n++;
  XtSetArg( args[n], XmNnumExportTargets, 1 ); n++;
  XtSetArg( args[n], XmNdragOperations, XmDROP_COPY ); n++;
  XtSetArg( args[n], XmNconvertProc, cvt ); n++;
  XtSetArg( args[n], XmNclientData, this ); n++;
    
  dc = XmDragStart( w, e, args, n );
  XtAddCallback( dc, XmNdragDropFinishCallback, dragFin, this );

  return 1;

}

char *activeGraphicClass::firstDragName ( void ) {

  return NULL;

}

char *activeGraphicClass::nextDragName ( void ) {

  return NULL;

}

char *activeGraphicClass::dragValue (
  int i )
{

  return NULL;

}

void activeGraphicClass::setCurrentDragIndex (
  int num )
{

  currentDragIndex = num;

}

int activeGraphicClass::getCurrentDragIndex ( void ) {

  return currentDragIndex;

}

void activeGraphicClass::changeDisplayParams (
  unsigned int flag,
  char *fontTag,
  int alignment,
  char *ctlFontTag,
  int ctlAlignment,
  char *btnFontTag,
  int btnAlignment,
  unsigned int textFgColor,
  unsigned int fg1Color,
  unsigned int fg2Color,
  unsigned int offsetColor,
  unsigned int bgColor,
  unsigned int topShadowColor,
  unsigned int botShadowColor )
{

}

void activeGraphicClass::changePvNames (
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

}

void activeGraphicClass::setUndoText (
  char *string )
{

  actWin->setUndoText( string );

}

void activeGraphicClass::flushUndo ( void ) {

}

int activeGraphicClass::addUndoCreateNode ( undoClass *undoObj ) {

int stat;

  stat = undoObj->addCreateNode( this, NULL );
  return stat;

}

int activeGraphicClass::addUndoMoveNode ( undoClass *undoObj )
{

int stat;

  stat = undoObj->addMoveNode( this, NULL, x, y );
  return stat;

}

int activeGraphicClass::addUndoResizeNode ( undoClass *undoObj ) {

int stat;

  stat = undoObj->addResizeNode( this, NULL, x, y, w, h );
  return stat;

}

int activeGraphicClass::addUndoCopyNode ( undoClass *undoObj ) {

int stat;

  stat = undoObj->addCopyNode( this, NULL );
  return stat;

}

int activeGraphicClass::addUndoCutNode ( undoClass *undoObj ) {

int stat;

  stat = undoObj->addCutNode( this, NULL );
  return stat;

}

int activeGraphicClass::addUndoPasteNode ( undoClass *undoObj ) {

int stat;

  stat = undoObj->addPasteNode( this, NULL );
  return stat;

}

int activeGraphicClass::addUndoReorderNode ( undoClass *undoObj ) {

int stat;

  stat = undoObj->addReorderNode( this, NULL );
  return stat;

}

int activeGraphicClass::addUndoEditNode ( undoClass *undoObj ) {

int stat;

  stat = undoObj->addEditNode( this, NULL );
  return stat;

}

int activeGraphicClass::addUndoGroupNode ( undoClass *undoObj ) {

int stat;

  stat = undoObj->addGroupNode( this, NULL );
  return stat;

}

int activeGraphicClass::addUndoRotateNode ( undoClass *undoObj ) {

int stat;

  stat = undoObj->addRotateNode( this, NULL, x, y, w, h );
  return stat;

}

int activeGraphicClass::addUndoFlipNode ( undoClass *undoObj ) {

int stat;

  stat = undoObj->addFlipNode( this, NULL, x, y, w, h );
  return stat;

}

int activeGraphicClass::undoCreate (
  undoOpClass *opPtr
) {


  return 1;

}

int activeGraphicClass::undoMove (
  undoOpClass *opPtr,
  int x,
  int y )
{

  moveAbs( x, y );
  moveSelectBoxAbs( x, y );
  return 1;

}

int activeGraphicClass::undoResize (
  undoOpClass *opPtr,
  int x,
  int y,
  int w,
  int h )
{

  resizeAbs( x, y, w, h );
  updateDimensions();
  resizeSelectBoxAbs( x, y, w, h );
  return 1;

}

int activeGraphicClass::undoCopy (
  undoOpClass *opPtr
) {

  return 1;

}

int activeGraphicClass::undoCut (
  undoOpClass *opPtr
) {

  return 1;

}

int activeGraphicClass::undoPaste (
  undoOpClass *opPtr
) {

  return 1;

}

int activeGraphicClass::undoReorder (
  undoOpClass *opPtr
) {

  return 1;

}

int activeGraphicClass::undoEdit (
  undoOpClass *opPtr
) {

  return 1;

}

int activeGraphicClass::undoGroup (
  undoOpClass *opPtr
) {

  return 1;

}

int activeGraphicClass::undoRotate (
  undoOpClass *opPtr,
  int x,
  int y,
  int w,
  int h )
{

  resizeAbs( x, y, w, h );
  updateDimensions();
  resizeSelectBoxAbs( x, y, w, h );
  return 1;

}

int activeGraphicClass::undoFlip (
  undoOpClass *opPtr,
  int x,
  int y,
  int w,
  int h )
{

  resizeAbs( x, y, w, h );
  updateDimensions();
  resizeSelectBoxAbs( x, y, w, h );
  return 1;

}
