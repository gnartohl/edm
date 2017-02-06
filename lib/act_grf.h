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
#include "tag_pkg.h"
#include "pv_factory.h"

#define MAX_UNITS_SIZE 8
#define MAX_ENUM_STRING_SIZE 26
#define MAX_ENUM_STATES 16

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

///////////// start of class declaration: activeGraphicClass

class activeGraphicClass {

protected:

// Change major version whenever this or any dependent class changes in an
// incompatible manner with respect to a derived edm widget class. This
// will help avoid base class poisoning run-time issues for libraries built
// at one site and utilized at another.
//
static const int MAJOR_VERSION = 5;
static const int MINOR_VERSION = 0;

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
char *createParam;
int xOrigin, yOrigin;
int x, y, w, h;
int sboxX, sboxY, sboxW, sboxH;
float orientation;
int selected; // if true, then is selected
int editMode;

unknownTagList unknownTags;

activeGraphicClass *nextToEdit; // for group edits
int inGroup;

activeGraphicClass *nextSelectedToEdit; // for selection group edits

char id[31+1], bufId[31+1];

Widget dc; // drag context
char *dragData;
int dragIndex, currentDragIndex;

int objType;

int onBlinkList;
void *blinkFunc;
int blinkDisable;

int mouseOver;

int defaultEnabled,
 enabled, prevEnabled; // this is a run-time attribute;
                       // if not enabled, widget should be
                       // unmapped/invisible and nonfunctional

public:

static const int UNKNOWN = 0;
static const int GRAPHICS = 1;
static const int MONITORS = 2;
static const int CONTROLS = 3;

static const int MAX_PV_NAME = 100;

activeWindowClass *actWin;
void *aglPtr; // will hold the activeGraphicListPtr container
undoClass *curUndoObj;
int startEdit, editConfirmed;
int startSar, sarConfirmed;

int deleteRequest; // if true, then wants to be deleted

// act_win.cc processes hidden objects as follows:
//  o they are not included in a "select all" operation
//  o they are not included in a "edit outliers" operation
//  o they are not included in a "save" operation
int hidden;

int needSmartDraw;

activeGraphicClass ( void );

void clone ( const activeGraphicClass *source );

virtual ~activeGraphicClass ( void );

Drawable drawable ( Widget w );

int baseMajorVersion ( void );

int baseMinorVersion ( void );

void checkBaseClassVersion (
  int ver,
  char *name
);

virtual void setObjType (
  char *strObjType );

virtual void getObjType (
  int maxLen,
  char *strObjType );

virtual char *objName ( void ) {

  return name;

}

virtual int destroy ( void );

virtual char *idName( void );

// -----------------

virtual int setProperty (
  char *property,
  char *value );

virtual int setProperty (
  char *property,
  double *value );

virtual int setProperty (
  char *property,
  int *value );

virtual int getProperty (
  char *property,
  int bufSize,
  char *value );

virtual int getProperty (
  char *property,
  double *value );

virtual int getProperty (
  char *property,
  int *value );

// -----------------

virtual int setProperty (
  char *itemId,
  char *property,
  char *value );

virtual int setProperty (
  char *itemId,
  char *property,
  double *value );

virtual int setProperty (
  char *itemId,
  char *property,
  int *value );

virtual int getProperty (
  char *itemId,
  char *property,
  int bufSize,
  char *value );

virtual int getProperty (
  char *itemId,
  char *property,
  double *value );

virtual int getProperty (
  char *itemId,
  char *property,
  int *value );

// -------------------

virtual int setProperty (
  char *winId,
  char *itemId,
  char *property,
  char *value );

virtual int setProperty (
  char *winId,
  char *itemId,
  char *property,
  double *value );

virtual int setProperty (
  char *winId,
  char *itemId,
  char *property,
  int *value );

virtual int getProperty (
  char *winId,
  char *itemId,
  char *property,
  int bufSize,
  char *value );

virtual int getProperty (
  char *winId,
  char *itemId,
  char *property,
  double *value );

virtual int getProperty (
  char *winId,
  char *itemId,
  char *property,
  int *value );

// -----------------

virtual int openDisplay (
  char *fileName,
  int setPosition );

virtual void updateDimensions ( void );

virtual int move (
  int x,
  int y );

virtual int moveAbs (
  int x,
  int y );

virtual int moveMidpointAbs (
  int x,
  int y );

virtual int snapToGrid ( void );

int snapSizeToGrid ( void );

virtual int rotate (
  int xOrigin,
  int yOrigin,
  char direction );

virtual int flip (
  int xOrigin,
  int yOrigin,
  char direction );

virtual int resize (
  int _x,
  int _y,
  int _w,
  int _h );

virtual int resizeAbs (
  int _x,
  int _y,
  int _w,
  int _h );

virtual int resizeAbsFromUndo (
  int _x,
  int _y,
  int _w,
  int _h );

virtual int draw ( void );

virtual int draw (
  int _x,
  int _y,
  int _w,
  int _h );

int drawAll ( void );

int clear ( void );

int refresh ( void );

int refresh (
  int _x,
  int _y,
  int _w,
  int _h );

int refresh ( activeGraphicClass *oneNode );

virtual void initSelectBox ( void );

virtual int eraseSelectBoxCorners ( void );

virtual int eraseSelectBox ( void );

virtual int drawSelectBoxCorners ( void );

virtual int drawSelectBox ( void );

virtual int getSelectBoxOperation (
  int controlKeyPressed,
  int _x,
  int _y );

virtual int getSelectBoxOperation (
  int _x,
  int _y );

virtual int moveSelectBox (
  int _x,
  int _y );

virtual int moveSelectBoxAbs (
  int _x,
  int _y );

virtual int moveSelectBoxMidpointAbs (
  int _x,
  int _y );

virtual int checkResizeSelectBox (
  int _x,
  int _y,
  int _w,
  int _h );

virtual int checkResizeSelectBoxAbs (
  int _x,
  int _y,
  int _w,
  int _h );

virtual int resizeSelectBox (
  int _x,
  int _y,
  int _w,
  int _h );

virtual int resizeSelectBoxAbs (
  int _x,
  int _y,
  int _w,
  int _h );

virtual int resizeSelectBoxAbsFromUndo (
  int _x,
  int _y,
  int _w,
  int _h );

virtual void setSelected ( void );

virtual int select (
  int x,
  int y );

virtual int selectEnclosed (
  int x,
  int y,
  int w,
  int h );

virtual int selectTouching (
  int x,
  int y,
  int w,
  int h );

virtual void deselect ( void );

virtual int isSelected ( void ) {
  if ( deleteRequest ) return 0;
  return selected;
}

virtual int erase ( void );

virtual void bufInvalidate ( void );

virtual int activate (
  int pass );

virtual int activate (
  int pass,
  void *ptr );

virtual int activate (
  int pass,
  void *ptr,
  int *numSubObjects // for groups & symbols
);

virtual int reactivate (
  int pass );

virtual int reactivate (
  int pass,
  void *ptr );

virtual int reactivate (
  int pass,
  void *ptr,
  int *numSubObjects );

virtual int raise ( void );

virtual int lower ( void );

virtual int createInteractive (
  activeWindowClass *actWin,
  int x,
  int y,
  int w,
  int h );

virtual int createFromFile (
  FILE *fptr,
  char *name,
  activeWindowClass *actWin );

virtual int old_createFromFile (
  FILE *fptr,
  char *name,
  activeWindowClass *actWin );

virtual int importFromXchFile (
  FILE *fptr,
  char *name,
  activeWindowClass *actWin );

virtual int createSpecial (
  char *param,
  activeWindowClass *actWin );

virtual void sendMsg (
  char *param );

virtual int save (
  FILE *fptr );

virtual int old_save (
  FILE *fptr );

virtual activeGraphicClass *copy ( void );

virtual activeGraphicClass *cut ( void );

virtual int paste ( void );

virtual int edit ( void );

virtual int doEdit (
  undoClass *_undoObj );

void operationComplete ( void );

void operationCancel ( void );

// for multipoint objects

virtual void lineEditBegin ( void );

virtual void lineCreateBegin ( void );

virtual int addPoint (
  int x,
  int y );

virtual int insertPoint (
  int x,
  int y );

virtual int removePoint (
  int x,
  int y );

virtual int removeLastPoint ( void );

virtual pointPtr selectPoint (
  int x,
  int y );

virtual void deselectAllPoints ( void ) {}

virtual int movePoint (
  pointPtr curPoint,
  int x,
  int y );

virtual int movePointRel (
  pointPtr curPoint,
  int xofs,
  int yofs );

virtual int lineEditComplete ( void );

virtual int lineEditCancel ( void );

// end of multipoint functions

virtual void show ( void ) {

}

virtual int getXOrigin ( void ) {

  return xOrigin;

}

virtual int getW ( void ) {

  return w;

}

virtual int getH ( void ) {

  return h;

}

virtual void setXOrigin (
  int value ) {

  xOrigin = value;

}

virtual int getX0 ( void ) {

  return x;

}

virtual int getX1 ( void ) {

  return x + w;

}

virtual int getXMid ( void ) {

  return x + w/2;

}

virtual int getYOrigin ( void ) {

  return yOrigin;

}

virtual void setYOrigin (
  int value ) {

  yOrigin = value;

}

virtual int getY0 ( void ) {

  return y;

}

virtual int getY1 ( void ) {

  return y + h;

}

virtual int getYMid ( void ) {

  return y + h/2;

}

virtual int setValue ( void *ptr ) {

  return 1;

}

// ========================================================
// active widget functions

virtual int eraseActive ( void );

virtual int eraseUnconditional ( void );

virtual int drawActive ( void );

virtual int drawActive (
  int _x,
  int _y,
  int _w,
  int _h );

virtual int drawAllActive ( void );

virtual activeGraphicClass *enclosingObject (
  int x0,
  int y0 );

virtual int intersects (
  int x0,
  int y0,
  int x1,
  int y1 );

virtual int smartDrawAllActive ( void );

virtual int doSmartDrawAllActive ( void );

virtual int doSmartDrawAllButMeActive ( void );

virtual int drawActiveIfIntersects (
  int x0,
  int y0,
  int x1,
  int y1 );

virtual int smartDrawCount ( void );

virtual void resetSmartDrawCount ( void );

virtual int smartDrawAll ( void );

virtual int clearActive ( void );

virtual int refreshActive ( void );

virtual int refreshActive (
  int _x,
  int _y,
  int _w,
  int _h );

virtual void flushActive ( void );

virtual int deactivate ( void );

virtual int deactivate (
  int pass
);

virtual int deactivate (
  int pass,
  int *numSubObjects // for groups & symbols
);

virtual int preReactivate (
  int pass );

virtual int preReactivate (
  int pass,
  int *numSubObjects // for groups & symbols
);

virtual int createGroup (
  activeWindowClass *actWin );

virtual int ungroup (
  void *curListNode );

virtual void adjustCoordinates (
  int _xOrigin,
  int _yOrigin );

virtual int getButtonActionRequest (
  int *up,
  int *down,
  int *drag );

virtual int getButtonActionRequest (
  int *up,
  int *down,
  int *drag,
  int *focus );

virtual void btnUp (
  int x,
  int y,
  int buttonState,
  int buttonNumber );

virtual void btnUp (
  int x,
  int y,
  int buttonState,
  int buttonNumber,
  int *action );

virtual void btnUp (
  XButtonEvent *be,
  int x,
  int y,
  int buttonState,
  int buttonNumber,
  int *action );

virtual void btnDown (
  int x,
  int y,
  int buttonState,
  int buttonNumber );

virtual void btnDown (
  int x,
  int y,
  int buttonState,
  int buttonNumber,
  int *action );

virtual void btnDown (
  XButtonEvent *be,
  int x,
  int y,
  int buttonState,
  int buttonNumber,
  int *action );

virtual void btnDrag (
  int x,
  int y,
  int buttonState,
  int buttonNumber );

virtual void btnDrag (
  XMotionEvent *me,
  int x,
  int y,
  int buttonState,
  int buttonNumber );

virtual void pointerIn (
  int x,
  int y,
  int buttonState );

virtual void pointerIn (
  XMotionEvent *me,
  int x,
  int y,
  int buttonState );

virtual void pointerOut (
  int x,
  int y,
  int buttonState );

virtual void pointerOut (
  XMotionEvent *me,
  int x,
  int y,
  int buttonState );

virtual void checkMouseOver (
  int x,
  int y,
  int buttonState );

virtual void checkMouseOver (
  XMotionEvent *me,
  int x,
  int y,
  int buttonState );

virtual void mousePointerIn (
  int _x,
  int _y,
  int buttonState );

virtual void mousePointerIn (
  XMotionEvent *me,
  int _x,
  int _y,
  int buttonState );

virtual void mousePointerOut (
  int _x,
  int _y,
  int buttonState );

virtual void mousePointerOut (
  XMotionEvent *me,
  int _x,
  int _y,
  int buttonState );

virtual int mouseIsOver ( void );

virtual void setMouseOver ( void );

virtual void clearMouseOver ( void );

virtual int initDefExeNode (
  void *ptr );

virtual int expandTemplate (
  int numMacros,
  char *macros[],
  char *expansions[] ) {

  return 1;

}

virtual int expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] ) {

  return 1;

}

virtual int expand2nd (
  int numMacros,
  char *macros[],
  char *expansions[] ) {

  return 1;

}

virtual int containsMacros ( void ) {

  return 0;

}

virtual int getNumMacroSets ( void ) {

  return 0;

}

virtual int getMacrosSet (
  int *numMacros,
  char ***macro,
  char ***expansion,
  int n
) {

  *numMacros = 0;
  *macro = NULL;
  *expansion = NULL;

  return 1;

}

virtual int getMacros (
  int *numMacros,
  char ***macro,
  char ***expansion ) {

  *numMacros = 0;
  *macro = NULL;
  *expansion = NULL;

  return 1;

}

virtual int isMux ( void ) { return 0; }

virtual int isIncludeWidget( void ) { return 0; }


// related display functions

virtual int isRelatedDisplay ( void ) { return 0; }

virtual int getNumRelatedDisplays ( void ) {

  return 0;

}

virtual int getRelatedDisplayProperty (
  int index,
  char *key
) {

  return 0;

}


virtual char *getRelatedDisplayName (
  int index
) {

  return NULL;

}

virtual char *getRelatedDisplayMacros (
  int index
) {

  return NULL;

}

virtual void augmentRelatedDisplayMacros (
  char *buf
) {

}

void updateFont (
  char *string,
  char *fontTag,
  XFontStruct **fs,
  int *ascent,
  int *descent,
  int *height,
  int *width );

void updateFont (
  char *fontTag,
  XFontStruct **fs,
  int *ascent,
  int *descent,
  int *height );

virtual void updateGroup ( void );

virtual int isInvisible ( void )
{
  return 0;
}

virtual void executeDeferred ( void );

virtual void executeFromDeferredQueue ( void );

virtual void clearNextToEdit ( void );

virtual void setNextToEdit (
  activeGraphicClass *ptr
);

virtual void setNextSelectedToEdit (
  activeGraphicClass *ptr );

virtual void clearNextSelectedToEdit ( void );

virtual int isInGroup ( void ) {

  return inGroup;

}

virtual void setInGroup ( void );

virtual void clearInGroup ( void );

virtual int isMultiPointObject ( void ) {
  return 0;
}

virtual int isWindowContainer ( void ) {
  return 0;
}

virtual int activateComplete ( void ) {
  return 1;
}

virtual int activateBeforePreReexecuteComplete ( void ) {
  return activateComplete();
}

virtual activeGraphicClass *getTail ( void );

virtual void setEditProperties ( void );

virtual void setEditSegments ( void );

virtual int editPropertiesSet ( void );

virtual int editSegmentsSet ( void );

virtual int editLineSegments ( void );

virtual int selectDragValue (
  XButtonEvent *be );

virtual int startDrag (
  XButtonEvent *be,
  int x,
  int y );

// for motif widgets
virtual void doActions (
  XButtonEvent *be,
  int x,
  int y
);

virtual int startDrag (
  Widget w,
  XEvent *e );

virtual char *firstDragName (
  int x,
  int y );

virtual char *firstDragName ( void );

virtual char *nextDragName (
  int x,
  int y );

virtual char *nextDragName ( void );

virtual char *dragValue (
  int x,
  int y,
  int i );

virtual char *dragValue (
  int i );

virtual int atLeastOneDragPv (
  int x,
  int y );

virtual void setCurrentDragIndex (
  int num );

virtual int getCurrentDragIndex ( void );

virtual void getPvs (
  int max,
  ProcessVariable *pvs[],
  int *n );

virtual char *getSearchString (
  int index
) {

  return NULL;

}

virtual void replaceString (
  int index,
  int max,
  char *string
) {

}

virtual int showPvInfo (
  XButtonEvent *be,
  int x,
  int y );

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

virtual void changeDisplayParams (
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
#define ACTGRF_NULLPVS_MASK		4
#define ACTGRF_VISPVS_MASK		8
#define ACTGRF_ALARMPVS_MASK		0x10

virtual void changePvNames (
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

void setUndoText (
  char *string );

virtual void flushUndo ( void );

virtual int addUndoCreateNode ( undoClass *_undoObj );

virtual int addUndoMoveNode ( undoClass *_undoObj );

virtual int addUndoResizeNode ( undoClass *_undoObj );

virtual int addUndoCopyNode ( undoClass *_undoObj );

virtual int addUndoCutNode ( undoClass *_undoObj );

virtual int addUndoPasteNode ( undoClass *_undoObj );

virtual int addUndoReorderNode ( undoClass *_undoObj );

virtual int addUndoEditNode ( undoClass *_undoObj );

virtual int addUndoGroupNode ( undoClass *_undoObj );

virtual int addUndoRotateNode ( undoClass *_undoObj );

virtual int addUndoFlipNode ( undoClass *_undoObj );

virtual int undoCreate (
  undoOpClass *opPtr
);

virtual int undoMove (
  undoOpClass *opPtr,
  int x,
  int y );

virtual int undoResize (
  undoOpClass *opPtr,
  int x,
  int y,
  int w,
  int h );

virtual int undoCopy (
  undoOpClass *opPtr
);

virtual int undoCut (
  undoOpClass *opPtr
);

virtual int undoPaste (
  undoOpClass *opPtr
);

virtual int undoReorder (
  undoOpClass *opPtr
);

virtual int undoEdit (
  undoOpClass *opPtr
);

virtual int undoGroup (
  undoOpClass *opPtr
);

virtual int undoRotate (
  undoOpClass *opPtr,
  int x,
  int y,
  int w,
  int h );

virtual int undoFlip (
  undoOpClass *opPtr,
  int x,
  int y,
  int w,
  int h );

virtual void updateColors (
  double colorValue );

virtual void confirmEdit ( void );

virtual void beginEdit ( void );

virtual int checkEditStatus ( void );

int blink ( void );

void setBlink ( void );

void setNotBlink ( void );

void *blinkFunction ( void );

void setBlinkFunction (
  void *addr
);

void updateExecuteModeBlink (
  int blinkFlag
);

void updateEditModeBlink (
  int blinkFlag
);

void updateBlink (
  int blinkFlag
);

void removeBlink ( void );

void disableBlink ( void );

void enableBlink ( void );

void setCreateParam (
  char *param
);

char *getCreateParam ( void );

void postIncompatable ( void );

virtual void setDefaultEnable (
  int flag
);

virtual void initEnable ( void );

virtual void enable ( void );

virtual void disable ( void );

virtual int isEnabled ( void );

virtual int isDisabled ( void );

virtual void map ( void );

virtual void unmap ( void );

virtual int getGroupVisInfo ( // for group objects
  expStringClass *visStr,
  int *visInv,
  int maxLen,
  char *minVis,
  char *maxVis
);

virtual int putGroupVisInfo ( // for group objects
  expStringClass *visStr,
  int visInv,
  int maxLen,
  char *minVis,
  char *maxVis
);

int crawlerPvIndex; // for crawler

// pv crawler functions
virtual char *crawlerGetFirstPv ( void );
virtual char *crawlerGetNextPv ( void );

void getSelBoxDims (
  int *x,
  int *y,
  int *w,
  int *h
);

};

#endif
