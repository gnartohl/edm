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

#ifndef __act_grf_h
#define __act_grf_h 1

#include <Xm/Xm.h>
#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <math.h>
#include <Xm/AtomMgr.h>
#include <Xm/DragDrop.h>
#include <X11/Xos.h>

#include "color_pkg.h"
#include "pvColor.h"
#include "expString.h"
#include "color_button.h"
#include "font_pkg.h"
#include "font_menu.h"
#include "gc_pkg.h"
#include "entry_form.h"
#include "pvConnection.h"

class undoOpClass;
class undoClass;

#ifdef __act_grf_cc

#include "act_grf.str"

static void dragFin (
  Widget w,
  XtPointer clientData,
  XtPointer call_data );

static Boolean cvt (
  Widget w,
  Atom *selection,
  Atom *target,
  Atom *type_return,
  XtPointer *value_return,
  unsigned long *len_return,
  int *format_return );

#endif

class activeWindowClass;

#define AGC_K_EDIT_PROPERTIES 1
#define AGC_K_EDIT_SEGMENTS 2

#define AGC_MOVE_OP 1
#define AGC_LEFT_OP 2
#define AGC_TOP_OP 3
#define AGC_BOTTOM_OP 4
#define AGC_RIGHT_OP 5
#define AGC_LEFT_TOP_OP 6
#define AGC_LEFT_BOTTOM_OP 7
#define AGC_RIGHT_TOP_OP 8
#define AGC_RIGHT_BOTTOM_OP 9

typedef union pvValTag {
  int l;
  double d;
  short s;
  char str[39+1];
} pvValType, *pvValPtr;

typedef struct pointTag {
  struct pointTag *flink;
  struct pointTag *blink;
  int x;
  int y;
} pointType, *pointPtr;

class activeGraphicClass {

protected:

friend void dragFin (
  Widget w,
  XtPointer clientData,
  XtPointer call_data );

friend Boolean cvt (
  Widget w,
  Atom *selection,
  Atom *target,
  Atom *type_return,
  XtPointer *value_return,
  unsigned long *len_return,
  int *format_return );

entryFormClass ef;

char *baseName;
char *name;
int xOrigin, yOrigin;
int x, y, w, h;
int sboxX, sboxY, sboxW, sboxH;
float orientation;
int selected; // if true, then is selected
int editMode;

activeGraphicClass *nextToEdit; // for group edits
int inGroup;

activeGraphicClass *nextSelectedToEdit; // for selection group edits

char id[31+1], bufId[31+1];

Widget dc; // drag context
char *dragData;
int dragIndex, currentDragIndex;

int objType;

static const int UNKNOWN = -1;
static const int GRAPHICS = 1;
static const int MONITORS = 2;
static const int CONTROLS = 3;

public:

static const int MAX_PV_NAME = 100;

activeWindowClass *actWin;
void *aglPtr; // will hold the activeGraphicListPtr container
undoClass *curUndoObj;
int startEdit, editConfirmed;

int deleteRequest; // if true, then wants to be deleted

activeGraphicClass::activeGraphicClass ( void );

void activeGraphicClass::clone ( const activeGraphicClass *source );

virtual activeGraphicClass::~activeGraphicClass ( void );

virtual void activeGraphicClass::setObjType (
  char *strObjType );

virtual void activeGraphicClass::getObjType (
  int maxLen,
  char *strObjType );

virtual char *activeGraphicClass::objName ( void ) {

  return name;

}

virtual int activeGraphicClass::destroy ( void );

virtual char *activeGraphicClass::idName( void );

// -----------------

virtual int activeGraphicClass::setProperty (
  char *property,
  char *value );

virtual int activeGraphicClass::setProperty (
  char *property,
  double *value );

virtual int activeGraphicClass::setProperty (
  char *property,
  int *value );

virtual int activeGraphicClass::getProperty (
  char *property,
  int bufSize,
  char *value );

virtual int activeGraphicClass::getProperty (
  char *property,
  double *value );

virtual int activeGraphicClass::getProperty (
  char *property,
  int *value );

// -----------------

virtual int activeGraphicClass::setProperty (
  char *itemId,
  char *property,
  char *value );

virtual int activeGraphicClass::setProperty (
  char *itemId,
  char *property,
  double *value );

virtual int activeGraphicClass::setProperty (
  char *itemId,
  char *property,
  int *value );

virtual int activeGraphicClass::getProperty (
  char *itemId,
  char *property,
  int bufSize,
  char *value );

virtual int activeGraphicClass::getProperty (
  char *itemId,
  char *property,
  double *value );

virtual int activeGraphicClass::getProperty (
  char *itemId,
  char *property,
  int *value );

// -------------------

virtual int activeGraphicClass::setProperty (
  char *winId,
  char *itemId,
  char *property,
  char *value );

virtual int activeGraphicClass::setProperty (
  char *winId,
  char *itemId,
  char *property,
  double *value );

virtual int activeGraphicClass::setProperty (
  char *winId,
  char *itemId,
  char *property,
  int *value );

virtual int activeGraphicClass::getProperty (
  char *winId,
  char *itemId,
  char *property,
  int bufSize,
  char *value );

virtual int activeGraphicClass::getProperty (
  char *winId,
  char *itemId,
  char *property,
  double *value );

virtual int activeGraphicClass::getProperty (
  char *winId,
  char *itemId,
  char *property,
  int *value );

// -----------------

virtual int activeGraphicClass::openDisplay (
  char *fileName,
  int setPosition );

virtual void activeGraphicClass::updateDimensions ( void );

virtual int activeGraphicClass::move (
  int x,
  int y );

virtual int activeGraphicClass::moveAbs (
  int x,
  int y );

virtual int activeGraphicClass::moveMidpointAbs (
  int x,
  int y );

virtual int activeGraphicClass::snapToGrid ( void );

int activeGraphicClass::snapSizeToGrid ( void );

virtual int activeGraphicClass::rotate (
  int xOrigin,
  int yOrigin,
  char direction );

virtual int activeGraphicClass::flip (
  int xOrigin,
  int yOrigin,
  char direction );

virtual int activeGraphicClass::resize (
  int _x,
  int _y,
  int _w,
  int _h );

virtual int activeGraphicClass::resizeAbs (
  int _x,
  int _y,
  int _w,
  int _h );

virtual int activeGraphicClass::resizeAbsFromUndo (
  int _x,
  int _y,
  int _w,
  int _h );

virtual int activeGraphicClass::draw ( void );

virtual int activeGraphicClass::draw (
  int _x,
  int _y,
  int _w,
  int _h );

int activeGraphicClass::drawAll ( void );

int activeGraphicClass::clear ( void );

int activeGraphicClass::refresh ( void );

int activeGraphicClass::refresh (
  int _x,
  int _y,
  int _w,
  int _h );

int activeGraphicClass::refresh ( activeGraphicClass *oneNode );

virtual void activeGraphicClass::initSelectBox ( void );

virtual int activeGraphicClass::eraseSelectBoxCorners ( void );

virtual int activeGraphicClass::eraseSelectBox ( void );

virtual int activeGraphicClass::drawSelectBoxCorners ( void );

virtual int activeGraphicClass::drawSelectBox ( void );

virtual int activeGraphicClass::getSelectBoxOperation (
  int _x,
  int _y );

virtual int activeGraphicClass::moveSelectBox (
  int _x,
  int _y );

virtual int activeGraphicClass::moveSelectBoxAbs (
  int _x,
  int _y );

virtual int activeGraphicClass::moveSelectBoxMidpointAbs (
  int _x,
  int _y );

virtual int activeGraphicClass::checkResizeSelectBox (
  int _x,
  int _y,
  int _w,
  int _h );

virtual int activeGraphicClass::checkResizeSelectBoxAbs (
  int _x,
  int _y,
  int _w,
  int _h );

virtual int activeGraphicClass::resizeSelectBox (
  int _x,
  int _y,
  int _w,
  int _h );

virtual int activeGraphicClass::resizeSelectBoxAbs (
  int _x,
  int _y,
  int _w,
  int _h );

virtual int activeGraphicClass::resizeSelectBoxAbsFromUndo (
  int _x,
  int _y,
  int _w,
  int _h );

virtual void activeGraphicClass::setSelected ( void );

virtual int activeGraphicClass::select (
  int x,
  int y );

virtual int activeGraphicClass::selectEnclosed (
  int x,
  int y,
  int w,
  int h );

virtual int activeGraphicClass::selectTouching (
  int x,
  int y,
  int w,
  int h );

virtual void activeGraphicClass::deselect ( void );

virtual int activeGraphicClass::isSelected ( void ) {
  if ( deleteRequest ) return 0;
  return selected;
}

virtual int activeGraphicClass::erase ( void );

virtual void activeGraphicClass::bufInvalidate ( void );

virtual int activeGraphicClass::activate (
  int pass );

virtual int activeGraphicClass::activate (
  int pass,
  void *ptr );

virtual int activeGraphicClass::reactivate (
  int pass );

virtual int activeGraphicClass::reactivate (
  int pass,
  void *ptr );

virtual int activeGraphicClass::raise ( void );

virtual int activeGraphicClass::lower ( void );

virtual int activeGraphicClass::createInteractive (
  activeWindowClass *actWin,
  int x,
  int y,
  int w,
  int h );

virtual int activeGraphicClass::createFromFile (
  FILE *fptr,
  char *name,
  activeWindowClass *actWin );

virtual int activeGraphicClass::importFromXchFile (
  FILE *fptr,
  char *name,
  activeWindowClass *actWin );

virtual int activeGraphicClass::save (
  FILE *fptr );

virtual activeGraphicClass *activeGraphicClass::copy ( void );

virtual activeGraphicClass *activeGraphicClass::cut ( void );

virtual int activeGraphicClass::paste ( void );

virtual int activeGraphicClass::edit ( void );

virtual int activeGraphicClass::doEdit ( undoClass *_undoObj );

void activeGraphicClass::operationComplete ( void );

void activeGraphicClass::operationCancel ( void );

// for multipoint objects

virtual void activeGraphicClass::lineEditBegin ( void );

virtual int activeGraphicClass::addPoint (
  int x,
  int y );

virtual int activeGraphicClass::removeLastPoint ( void );

virtual pointPtr activeGraphicClass::selectPoint (
  int x,
  int y );

virtual int activeGraphicClass::movePoint (
  pointPtr curPoint,
  int x,
  int y );

virtual int activeGraphicClass::lineEditComplete ( void );

virtual int activeGraphicClass::lineEditCancel ( void );

// end of multipoint functions

virtual void activeGraphicClass::show ( void ) {

}

virtual int activeGraphicClass::getXOrigin ( void ) {

  return xOrigin;

}

virtual int activeGraphicClass::getW ( void ) {

  return w;

}

virtual int activeGraphicClass::getH ( void ) {

  return h;

}

virtual void activeGraphicClass::setXOrigin (
  int value ) {

  xOrigin = value;

}

virtual int activeGraphicClass::getX0 ( void ) {

  return x;

}

virtual int activeGraphicClass::getX1 ( void ) {

  return x + w;

}

virtual int activeGraphicClass::getXMid ( void ) {

  return x + w/2;

}

virtual int activeGraphicClass::getYOrigin ( void ) {

  return yOrigin;

}

virtual void activeGraphicClass::setYOrigin (
  int value ) {

  yOrigin = value;

}

virtual int activeGraphicClass::getY0 ( void ) {

  return y;

}

virtual int activeGraphicClass::getY1 ( void ) {

  return y + h;

}

virtual int activeGraphicClass::getYMid ( void ) {

  return y + h/2;

}

virtual int activeGraphicClass::setValue ( void *ptr ) {

  return 1;

}

// ========================================================
// active widget functions

virtual int activeGraphicClass::eraseActive ( void );

virtual int activeGraphicClass::eraseUnconditional ( void );

virtual int activeGraphicClass::drawActive ( void );

virtual int activeGraphicClass::drawActive (
  int _x,
  int _y,
  int _w,
  int _h );

int activeGraphicClass::drawAllActive ( void );

int activeGraphicClass::intersects (
  int x0,
  int y0,
  int x1,
  int y1 );

int activeGraphicClass::smartDrawAllActive ( void );

int activeGraphicClass::smartDrawAll ( void );

int activeGraphicClass::clearActive ( void );

int activeGraphicClass::refreshActive ( void );

int activeGraphicClass::refreshActive (
  int _x,
  int _y,
  int _w,
  int _h );

void activeGraphicClass::flushActive ( void );

virtual int activeGraphicClass::deactivate ( void );

virtual int activeGraphicClass::deactivate (
  int pass
);

virtual int activeGraphicClass::preReactivate ( int pass );

virtual int activeGraphicClass::createGroup (
  activeWindowClass *actWin );

virtual int activeGraphicClass::ungroup (
  void *curListNode );

virtual void activeGraphicClass::adjustCoordinates (
  int _xOrigin,
  int _yOrigin );

virtual int activeGraphicClass::getButtonActionRequest (
  int *up,
  int *down,
  int *drag );

virtual int activeGraphicClass::getButtonActionRequest (
  int *up,
  int *down,
  int *drag,
  int *focus );

virtual void activeGraphicClass::btnUp (
  int x,
  int y,
  int buttonState,
  int buttonNumber );

virtual void activeGraphicClass::btnUp (
  int x,
  int y,
  int buttonState,
  int buttonNumber,
  int *action );

virtual void activeGraphicClass::btnDown (
  int x,
  int y,
  int buttonState,
  int buttonNumber );

virtual void activeGraphicClass::btnDown (
  int x,
  int y,
  int buttonState,
  int buttonNumber,
  int *action );

virtual void activeGraphicClass::btnDrag (
  int x,
  int y,
  int buttonState,
  int buttonNumber );

virtual void activeGraphicClass::pointerIn (
  int x,
  int y,
  int buttonState );

virtual void activeGraphicClass::pointerOut (
  int x,
  int y,
  int buttonState );

virtual int activeGraphicClass::initDefExeNode (
  void *ptr );

virtual int activeGraphicClass::expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] ) {

  return 1;

}

virtual int activeGraphicClass::expand2nd (
  int numMacros,
  char *macros[],
  char *expansions[] ) {

  return 1;

}

virtual int activeGraphicClass::containsMacros ( void ) {

  return 0;

}

virtual int activeGraphicClass::getMacros (
  int *numMacros,
  char ***macro,
  char ***expansion ) {

  *numMacros = 0;

  return 1;

}

virtual int activeGraphicClass::isMux ( void ) { return 0; }

virtual int activeGraphicClass::createWidgets ( void );

void activeGraphicClass::updateFont (
  char *string,
  char *fontTag,
  XFontStruct **fs,
  int *ascent,
  int *descent,
  int *height,
  int *width );

void activeGraphicClass::updateFont (
  char *fontTag,
  XFontStruct **fs,
  int *ascent,
  int *descent,
  int *height );

virtual void activeGraphicClass::updateGroup ( void );

virtual int isInvisible ( void )
{
  return 0;
}

virtual void activeGraphicClass::executeDeferred ( void );

virtual void activeGraphicClass::executeFromDeferredQueue ( void );

virtual void activeGraphicClass::clearNextToEdit ( void );

virtual void activeGraphicClass::setNextToEdit (
  activeGraphicClass *ptr
);

virtual void activeGraphicClass::setNextSelectedToEdit (
  activeGraphicClass *ptr );

virtual void activeGraphicClass::clearNextSelectedToEdit ( void );

virtual int activeGraphicClass::isInGroup ( void ) {

  return inGroup;

}

virtual void activeGraphicClass::setInGroup ( void );

virtual void activeGraphicClass::clearInGroup ( void );

virtual int activeGraphicClass::isMultiPointObject ( void ) {
  return 0;
}

virtual activeGraphicClass *activeGraphicClass::getTail ( void );

virtual void activeGraphicClass::setEditProperties ( void );

virtual void activeGraphicClass::setEditSegments ( void );

virtual int activeGraphicClass::editPropertiesSet ( void );

virtual int activeGraphicClass::editSegmentsSet ( void );

virtual int activeGraphicClass::editLineSegments ( void );

virtual int activeGraphicClass::selectDragValue (
  int x,
  int y );

virtual int activeGraphicClass::startDrag (
  int x,
  int y );

// for motif widgets
virtual int activeGraphicClass::startDrag (
  Widget w,
  XEvent *e );

virtual char *activeGraphicClass::firstDragName ( void );

virtual char *activeGraphicClass::nextDragName ( void );

virtual char *activeGraphicClass::dragValue (
  int i );

virtual void activeGraphicClass::setCurrentDragIndex (
  int num );

int activeGraphicClass::getCurrentDragIndex ( void );

#define ACTGRF_FONTTAG_MASK		1
#define ACTGRF_ALIGNMENT_MASK		2
#define ACTGRF_CTLFONTTAG_MASK		4
#define ACTGRF_CTLALIGNMENT_MASK	8
#define ACTGRF_TEXTFGCOLOR_MASK		0x10
#define ACTGRF_FG1COLOR_MASK		0x20
#define ACTGRF_FG2COLOR_MASK		0x40
#define ACTGRF_OFFSETCOLOR_MASK		0x80
#define ACTGRF_BGCOLOR_MASK		0x100
#define ACTGRF_TOPSHADOWCOLOR_MASK	0x200
#define ACTGRF_BOTSHADOWCOLOR_MASK	0x400
#define ACTGRF_BTNFONTTAG_MASK		0x800
#define ACTGRF_BTNALIGNMENT_MASK	0x1000

virtual void activeGraphicClass::changeDisplayParams (
  unsigned int flag,
  char *fontTag,
  int alignment,
  char *ctlFontTag,
  int ctlAlignment,
  char *btnFontTag,
  int btnAlignment,
  int textFgColor,
  int fg1Color,
  int fg2Color,
  int offsetColor,
  int bgColor,
  int topShadowColor,
  int botShadowColor );

#define ACTGRF_CTLPVS_MASK		1
#define ACTGRF_READBACKPVS_MASK		2
#define ACTGRF_NULLPVS_MASK		3
#define ACTGRF_VISPVS_MASK		8
#define ACTGRF_ALARMPVS_MASK		0x10

virtual void activeGraphicClass::changePvNames (
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
  char *alarmPvs[] );

void activeGraphicClass::setUndoText (
  char *string );

virtual void activeGraphicClass::flushUndo ( void );

virtual int activeGraphicClass::addUndoCreateNode ( undoClass *_undoObj );

virtual int activeGraphicClass::addUndoMoveNode ( undoClass *_undoObj );

virtual int activeGraphicClass::addUndoResizeNode ( undoClass *_undoObj );

virtual int activeGraphicClass::addUndoCopyNode ( undoClass *_undoObj );

virtual int activeGraphicClass::addUndoCutNode ( undoClass *_undoObj );

virtual int activeGraphicClass::addUndoPasteNode ( undoClass *_undoObj );

virtual int activeGraphicClass::addUndoReorderNode ( undoClass *_undoObj );

virtual int activeGraphicClass::addUndoEditNode ( undoClass *_undoObj );

virtual int activeGraphicClass::addUndoGroupNode ( undoClass *_undoObj );

virtual int activeGraphicClass::addUndoRotateNode ( undoClass *_undoObj );

virtual int activeGraphicClass::addUndoFlipNode ( undoClass *_undoObj );

virtual int activeGraphicClass::undoCreate (
  undoOpClass *opPtr
);

virtual int activeGraphicClass::undoMove (
  undoOpClass *opPtr,
  int x,
  int y );

virtual int activeGraphicClass::undoResize (
  undoOpClass *opPtr,
  int x,
  int y,
  int w,
  int h );

virtual int activeGraphicClass::undoCopy (
  undoOpClass *opPtr
);

virtual int activeGraphicClass::undoCut (
  undoOpClass *opPtr
);

virtual int activeGraphicClass::undoPaste (
  undoOpClass *opPtr
);

virtual int activeGraphicClass::undoReorder (
  undoOpClass *opPtr
);

virtual int activeGraphicClass::undoEdit (
  undoOpClass *opPtr
);

virtual int activeGraphicClass::undoGroup (
  undoOpClass *opPtr
);

virtual int activeGraphicClass::undoRotate (
  undoOpClass *opPtr,
  int x,
  int y,
  int w,
  int h );

virtual int activeGraphicClass::undoFlip (
  undoOpClass *opPtr,
  int x,
  int y,
  int w,
  int h );

virtual void activeGraphicClass::updateColors (
  double colorValue );

virtual void confirmEdit ( void );

virtual void beginEdit ( void );

virtual int checkEditStatus ( void );

};

#endif
