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

#ifndef __act_win_h
#define __act_win_h 1

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <Xm/Xm.h>
#include <X11/Xlib.h>
#include <Xm/DrawingA.h>
#include <Xm/RowColumn.h>
#include <Xm/PushB.h>
#include <Xm/CascadeB.h>
#include <Xm/FileSB.h>
#include <Xm/Protocols.h>
#include <Xm/Separator.h>
#include <X11/keysym.h>
#include <Xm/MwmUtil.h>
#include "color_pkg.h"
#include "color_button.h"
#include "font_pkg.h"
#include "font_menu.h"
#include "gc_pkg.h"
#include "utility.h"
#include "bindings.h"

#include "ulBindings.h"
#include "pvBindings.h"
#include "entry_form.h"
#include "confirm_dialog.h"
#include "cursor.h"
#include "scheme.h"
#include "undo.h"
#include "msg_dialog.h"

#include "sys_types.h"
#include "thread.h"
#include "avl.h"

#define AWC_MAJOR_VERSION 3
#define AWC_MINOR_VERSION 1
#define AWC_RELEASE 0

#define AWC_EDIT 1
#define AWC_EXECUTE 2

#define AWC_POPUP_RAISE 101
#define AWC_POPUP_LOWER 102
#define AWC_POPUP_REFRESH 103
#define AWC_POPUP_PROPERTIES 104
#define AWC_POPUP_CUT 105
#define AWC_POPUP_PASTE 106
#define AWC_POPUP_COPY 107
#define AWC_POPUP_ALIGN_LEFT 108
#define AWC_POPUP_ALIGN_RIGHT 109
#define AWC_POPUP_ALIGN_TOP 110
#define AWC_POPUP_ALIGN_BOTTOM 111
#define AWC_POPUP_ALIGN_CENTER_HORZ 112
#define AWC_POPUP_ALIGN_CENTER_VERT 113
#define AWC_POPUP_DISTRIBUTE_VERTICALLY 114
#define AWC_POPUP_DISTRIBUTE_HORIZONTALLY 115
#define AWC_POPUP_DISTRIBUTE_MIDPT_VERTICALLY 116
#define AWC_POPUP_DISTRIBUTE_MIDPT_HORIZONTALLY 117
#define AWC_POPUP_EXECUTE 118
#define AWC_POPUP_SAVE 119
#define AWC_POPUP_SAVE_AS 120
#define AWC_POPUP_OPEN 121
#define AWC_POPUP_EDIT 122
#define AWC_POPUP_GROUP 123
#define AWC_POPUP_UNGROUP 124
#define AWC_POPUP_CLOSE 125
#define AWC_POPUP_TOGGLE_TITLE 126
#define AWC_POPUP_SAVE_SCHEME 127
#define AWC_POPUP_LOAD_SCHEME 128
#define AWC_POPUP_MAKESYMBOL 129
#define AWC_POPUP_EDIT_LINE_PROP 130
#define AWC_POPUP_EDIT_LINE_SEG 131
#define AWC_POPUP_OPEN_USER 132
#define AWC_POPUP_DESELECT 133
#define AWC_POPUP_OUTLIERS 134
#define AWC_POPUP_FINDTOP 135
#define AWC_POPUP_ALIGN_CENTER 136
#define AWC_POPUP_ALIGN_SIZE_VERT 137
#define AWC_POPUP_ALIGN_SIZE_HORZ 138
#define AWC_POPUP_ALIGN_SIZE 139
#define AWC_POPUP_CHANGE_DSP_PARAMS 140
#define AWC_POPUP_CHANGE_PV_NAMES 141
#define AWC_POPUP_PASTE_IN_PLACE 142
#define AWC_POPUP_UNDO 143
#define AWC_POPUP_ROTATE_CW 144
#define AWC_POPUP_ROTATE_CCW 145
#define AWC_POPUP_FLIP_H 146
#define AWC_POPUP_FLIP_V 147
#define AWC_POPUP_HELP 148
#define AWC_POPUP_SELECT_ALL 149
#define AWC_POPUP_SELECT_SCHEME_SET 150
#define AWC_POPUP_DISTRIBUTE_MIDPT_BOTH 151

#define AWC_NONE_SELECTED 1
#define AWC_ONE_SELECTED 2
#define AWC_MANY_SELECTED 3
#define AWC_START_DEFINE_REGION 4
#define AWC_DEFINE_REGION 5
#define AWC_EDITING 6
#define AWC_MOVE 7
#define AWC_RESIZE_LEFT 8
#define AWC_RESIZE_TOP 9
#define AWC_RESIZE_BOTTOM 10
#define AWC_RESIZE_RIGHT 11
#define AWC_RESIZE_LEFT_TOP 12
#define AWC_RESIZE_LEFT_BOTTOM 13
#define AWC_RESIZE_RIGHT_TOP 14
#define AWC_RESIZE_RIGHT_BOTTOM 15
#define AWC_START_DEFINE_SELECT_REGION 16
#define AWC_DEFINE_SELECT_REGION 17
#define AWC_EDITING_POINTS 18
#define AWC_MOVING_POINT 19
#define AWC_CHOOSING_LINE_OP 20
#define AWC_WAITING 21
/* #define AWC_ */

// if a call is made to this routine from a sharable library and then
// in xxgdb a "b _edmDebug" is performed, you may easily break a some
// location inside the sharable by stepping back to the caller
void _edmDebug ( void );

#ifdef __act_win_cc

#include "act_win.str"
#include "environment.str"

static void acw_autosave (
  XtPointer client,
  XtIntervalId *id );

static void awc_dont_save_cb (
  Widget w,
  XtPointer client,
  XtPointer call );

static void awc_dont_save_cb (
  Widget w,
  XtPointer client,
  XtPointer call );

static void awc_do_save_cb (
  Widget w,
  XtPointer client,
  XtPointer call );

static void awc_do_save_and_exit_cb (
  Widget w,
  XtPointer client,
  XtPointer call );

static void awc_fileSelectOk_cb (
  Widget w,
  XtPointer client,
  XtPointer call );

static void awc_fileSelectCancel_cb (
  Widget w,
  XtPointer client,
  XtPointer call );

static void awc_fileSelectKill_cb (
  Widget w,
  XtPointer client,
  XtPointer call );

static void awc_saveFileSelectOk_cb (
  Widget w,
  XtPointer client,
  XtPointer call );

static void awc_saveFileSelectCancel_cb (
  Widget w,
  XtPointer client,
  XtPointer call );

static void awc_saveFileSelectKill_cb (
  Widget w,
  XtPointer client,
  XtPointer call );

static void awc_saveSchemeSelectOk_cb (
  Widget w,
  XtPointer client,
  XtPointer call );

static void awc_saveSchemeSelectCancel_cb (
  Widget w,
  XtPointer client,
  XtPointer call );

static void awc_saveSchemeSelectKill_cb (
  Widget w,
  XtPointer client,
  XtPointer call );

static void awc_loadSchemeSelectOk_cb (
  Widget w,
  XtPointer client,
  XtPointer call );

static void awc_loadSchemeSelectCancel_cb (
  Widget w,
  XtPointer client,
  XtPointer call );

static void awc_loadSchemeSelectKill_cb (
  Widget w,
  XtPointer client,
  XtPointer call );

static void awc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

static void awc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

static void awc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

static void awc_WMExit_cb (
   Widget w,
  XtPointer client,
  XtPointer call );

static void selectScheme_cb (
   Widget w,
  XtPointer client,
  XtPointer call );

static void b1ReleaseOneSelect_cb (
   Widget w,
  XtPointer client,
  XtPointer call );

static void b2ReleaseNoneSelect_cb (
   Widget w,
  XtPointer client,
  XtPointer call );

static void b2ReleaseOneSelect_cb (
   Widget w,
  XtPointer client,
  XtPointer call );

static void b2ReleaseManySelect_cb (
   Widget w,
  XtPointer client,
  XtPointer call );

static void b2ReleaseExecute_cb (
   Widget w,
  XtPointer client,
  XtPointer call );

static void createPopup_cb (
   Widget w,
  XtPointer client,
  XtPointer call );

static void topWinEventHandler (
  Widget w,
  XtPointer client,
  XEvent *e,
  Boolean *continueToDispatch );

static void activeWinEventHandler (
  Widget w,
  XtPointer client,
  XEvent *e,
  Boolean *continueToDispatch );

static void drawWinEventHandler (
  Widget w,
  XtPointer client,
  XEvent *e,
  Boolean *continueToDispatch );

static void awc_continue_cb (
  Widget w,
  XtPointer client,
  XtPointer call );

static void awc_abort_cb (
  Widget w,
  XtPointer client,
  XtPointer call );

static void awc_save_and_exit_cb (
  Widget w,
  XtPointer client,
  XtPointer call );

#endif

class appContextClass;
class activeGraphicClass;

typedef struct widgetAndPointerTag {
  Widget w;
  XtPointer client;
} widgetAndPointerType, *widgetAndPointerPtr;

typedef struct popupBlockTag {
  Widget w;
  void *ptr;
  class activeWindowClass *awo;
} popupBlockType, *popupBlockPtr;

typedef struct dragPopupBlockTag {
  Widget w;
  int num;
  void *ago;
} dragPopupBlockType, *dragPopupBlockPtr;

typedef struct popupBlockListTag {
  struct popupBlockListTag *flink;
  struct popupBlockListTag *blink;
  popupBlockType block;
} popupBlockListType, *popupBlockListPtr;

typedef struct pollListTag {
  struct pollListTag *flink;
  struct pollListTag *blink;
} pollListType, *pollListPtr;

typedef struct eventListTag {
  struct eventListTag *flink;
  struct eventListTag *blink;
} eventListType, *eventListPtr;

typedef struct activeGraphicListTag {
  struct activeGraphicListTag *flink;
  struct activeGraphicListTag *blink;
  struct activeGraphicListTag *selFlink; // for list of selected nodes
  struct activeGraphicListTag *selBlink; // for list of selected nodes
  struct activeGraphicListTag *defExeFlink; // deferred execution list
  struct activeGraphicListTag *defExeBlink;
  activeGraphicClass *node;
} activeGraphicListType, *activeGraphicListPtr;

// the following defines btnActionListType & btnActionListPtr
#include "btnActionListType.h"

typedef struct objNameListTag {
  struct objNameListTag *flink;
  struct objNameListTag *blink;
  Widget w;
  char *objName;
  char *objType;
} objNameListType, *objNameListPtr;

typedef struct commentLinesTag {
  struct commentLinesTag *flink;
  char *line;
} commentLinesType, *commentLinesPtr;

class activeWindowClass {

public:

char curSchemeSet[63+1];

char startSignature[15+1];

int dragItemIndex;
dragPopupBlockType dragPopupBlock[10];

int major, minor, release, fileLineNumber;

int buttonPressX, buttonPressY;

commentLinesPtr commentHead, commentTail;

int showActive;
unsigned int crc; // crc of all symbols/values

char defaultPvType[15+1], bufDefaultPvType[15+1];

VPFUNC activateCallback, deactivateCallback;
int activateCallbackFlag, deactivateCallbackFlag;
int bufActivateCallbackFlag, bufDeactivateCallbackFlag;

Boolean isIconified;

XtIntervalId autosaveTimer, restoreTimer;
int changeSinceAutoSave, doAutoSave;
char autosaveName[255+1];

int doClose;

widgetAndPointerType wpFileSelect, wpSchemeSelect;

friend class activeGraphicClass;
friend class activeGroupClass;

friend void acw_autosave (
  XtPointer client,
  XtIntervalId *id );

friend void awc_dont_save_cb (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void awc_do_save_cb (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void awc_do_save_and_exit_cb (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void awc_fileSelectOk_cb (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void awc_fileSelectCancel_cb (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void awc_saveFileSelectOk_cb (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void awc_saveFileSelectCancel_cb (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void awc_saveSchemeSelectOk_cb (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void awc_saveSchemeSelectCancel_cb (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void awc_loadSchemeSelectOk_cb (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void awc_loadSchemeSelectCancel_cb (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void *pv_poll_thread (
  THREAD_HANDLE h );

friend void awc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void awc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void awc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void awc_load_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void awc_load_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void awc_load_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void awc_save_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void awc_save_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void awc_save_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void awc_WMExit_cb (
   Widget w,
  XtPointer client,
  XtPointer call );

friend void selectScheme_cb (
   Widget w,
  XtPointer client,
  XtPointer call );

friend void b1ReleaseOneSelect_cb (
   Widget w,
  XtPointer client,
  XtPointer call );

friend void b2ReleaseNoneSelect_cb (
   Widget w,
  XtPointer client,
  XtPointer call );

friend void b2ReleaseOneSelect_cb (
   Widget w,
  XtPointer client,
  XtPointer call );

friend void b2ReleaseManySelect_cb (
   Widget w,
  XtPointer client,
  XtPointer call );

friend void b2ReleaseExecute_cb (
   Widget w,
  XtPointer client,
  XtPointer call );

friend void createPopup_cb (
   Widget w,
  XtPointer client,
  XtPointer call );

friend void topWinEventHandler (
  Widget w,
  XtPointer client,
  XEvent *e,
  Boolean *continueToDispatch );

friend void activeWinEventHandler (
  Widget w,
  XtPointer client,
  XEvent *e,
  Boolean *continueToDispatch );

friend void drawWinEventHandler (
  Widget w,
  XtPointer client,
  XEvent *e,
  Boolean *continueToDispatch );

friend void awc_continue_cb (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void awc_abort_cb (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void awc_save_and_exit_cb (
  Widget w,
  XtPointer client,
  XtPointer call );

msgDialogClass msgDialog;
int msgDialogCreated, msgDialogPoppedUp;

msgDialogClass objNameDialog;
int objNameDialogCreated, objNameDialogPoppedUp;

char id[31+1], bufId[31+1];
char title[127+1], bufTitle[127+1], restoreTitle[127+1];
int showName;

cursorClass cursor;

Widget drawWidget, executeWidget, fileSelectBox, schemeSelectBox;
Display *d;
int bufX, bufY, bufW, bufH;

activeGraphicListPtr head;
activeGraphicListPtr cutHead;
activeGraphicListPtr selectedHead;
activeGraphicListPtr defExeHead;

btnActionListPtr enterActionHead;
btnActionListPtr leaveActionHead;
btnActionListPtr btnDownActionHead;
btnActionListPtr btnUpActionHead;
btnActionListPtr btnMotionActionHead;
btnActionListPtr btnFocusActionHead;

popupBlockListPtr popupBlockHead;
objNameListPtr objNameHead;
fontMenuClass defaultFm, defaultCtlFm, defaultBtnFm;
int defaultAlignment;
int defaultCtlAlignment;
int defaultBtnAlignment;
char bufGridActiveStr[8], gridActiveStr[8];
char bufGridShowStr[8], gridShowStr[8];
int gridActive;
int gridShow;
int orthogonal, bufOrthogonal;
int orthoMove, bufOrthoMove;
int bufGridSpacing, gridSpacing, oldGridSpacing;
char windowControlName[127+1];
char windowIdName[127+1];
int ruler;
char rulerUnits[31+1];
int coordsShow;
eventListPtr eventHead;
eventListPtr limEventHead;
pollListPtr pollHead;
int mode; // AW_EDIT or AW_EXECUTE
int waiting;
int change;
int exit_after_save;

int list_array_size;
activeGraphicListType *list_array;

objBindingClass obj;
pvBindingClass pvObj;

Widget b1OneSelectPopup, b1ManySelectPopup, b1NoneSelectPopup,
 b2NoneSelectPopup, b2OneSelectPopup, b2ManySelectPopup, b3NoneSelectPopup,
 b2ExecutePopup, chPd, grPd, grCb, mnPd, mnCb, ctlPd, ctlCb, alignPd, alignCb,
 centerPd, centerCb, distributePd, distributeCb, sizePd, sizeCb, orientPd1,
 orientPdM, orientCb1, orientCbM, editPd1, editPdM, editCb1, editCbM,
 dragPopup, undoPb1, undoPb2, undoPb3, setSchemePd, setSchemeCb;

int state;
int savedState;
int oldState;
int startx;
int starty;
int width;
int height;
int oldx;
int oldy;

int usingArrowKeys;

int masterSelectX0, masterSelectY0, masterSelectX1, masterSelectY1;

int useFirstSelectedAsReference;

int fgColor, bufFgColor;
colorButtonClass fgCb;

int bgColor, bufBgColor;
colorButtonClass bgCb;
Pixmap bgPixmap;

int defaultTextFgColor, bufDefaultTextFgColor;
colorButtonClass defaultTextFgCb;

int defaultFg1Color, bufDefaultFg1Color;
colorButtonClass defaultFg1Cb;

int defaultFg2Color, bufDefaultFg2Color;
colorButtonClass defaultFg2Cb;

int defaultBgColor, bufDefaultBgColor;
colorButtonClass defaultBgCb;

int defaultTopShadowColor, bufDefaultTopShadowColor;
colorButtonClass defaultTopShadowCb;

int defaultBotShadowColor, bufDefaultBotShadowColor;
colorButtonClass defaultBotShadowCb;

int defaultOffsetColor, bufDefaultOffsetColor;
colorButtonClass defaultOffsetCb;

int useComponentScheme;

int allSelectedTextFgColor;
int allSelectedTextFgColorFlag;

int allSelectedFg1Color;
int allSelectedFg1ColorFlag;

int allSelectedFg2Color;
int allSelectedFg2ColorFlag;

int allSelectedBgColor;
int allSelectedBgColorFlag;

int allSelectedOffsetColor;
int allSelectedOffsetColorFlag;

int allSelectedTopShadowColor;
int allSelectedTopShadowColorFlag;

int allSelectedBotShadowColor;
int allSelectedBotShadowColorFlag;

char allSelectedFontTag[127+1];
int allSelectedFontTagFlag;

int allSelectedAlignment;
int allSelectedAlignmentFlag;

char allSelectedCtlFontTag[127+1];
int allSelectedCtlFontTagFlag;

int allSelectedCtlAlignment;
int allSelectedCtlAlignmentFlag;

char allSelectedBtnFontTag[127+1];
int allSelectedBtnFontTagFlag;

int allSelectedBtnAlignment;
int allSelectedBtnAlignmentFlag;

entryFormClass ef;
confirmDialogClass confirm, confirm1;

int noRefresh;

char defaultFontTag[127+1];
char defaultCtlFontTag[127+1];
char defaultBtnFontTag[127+1];

char allSelectedCtlPvName[1][100+1];
char allSelectedReadbackPvName[1][100+1];
char allSelectedNullPvName[1][100+1];
char allSelectedVisPvName[1][100+1];
char allSelectedAlarmPvName[1][100+1];
int allSelectedCtlPvNameFlag;
int allSelectedReadbackPvNameFlag;
int allSelectedNullPvNameFlag;
int allSelectedVisPvNameFlag;
int allSelectedAlarmPvNameFlag;

appContextClass *appCtx;

int numMacros;
int actualNumMacros;
char **macros;
char **expansions;

char fileName[255+1];
char prefix[127+1], displayName[127+1], postfix[127+1];
expStringClass expStrTitle;

colorInfoClass *ci;
entryFormClass *currentEf;
activeGraphicClass *currentObject, *currentPointObject;
pointPtr currentPoint;

int x, y, w, h;
Widget top;
gcClass drawGc, executeGc;
fontInfoClass *fi;

int versionStack[11][3];
int versionStackPtr;

int b2PressX, b2PressY, b2PressXRoot, b2PressYRoot;

undoClass undoObj;

Time buttonClickTime, deltaTime;

int noRaise;

activeWindowClass::activeWindowClass ( void );

activeWindowClass::~activeWindowClass ( void );

char *activeWindowClass::idName( void );

int activeWindowClass::setProperty (
  char *id,
  char *property,
  char *value );

int activeWindowClass::setProperty (
  char *id,
  char *property,
  double *value );

int activeWindowClass::setProperty (
  char *id,
  char *property,
  int *value );

int activeWindowClass::getProperty (
  char *id,
  char *property,
  int bufSize,
  char *value );

int activeWindowClass::getProperty (
  char *id,
  char *property,
  double *value );

int activeWindowClass::getProperty (
  char *id,
  char *property,
  int *value );

void activeWindowClass::updateAllSelectedDisplayInfo ( void );

void activeWindowClass::setTitle ( void );

void activeWindowClass::setTitleUsingTitle ( void );

void activeWindowClass::expandTitle (
  int phase,
  int nMac,
  char **mac,
  char **exp
);

void activeWindowClass::filterPosition (
  int *_x,
  int *_y,
  int oldX,
  int oldY );

int activeWindowClass::drawAfterResize (
  activeWindowClass *actWin,
  int deltax,
  int deltay,
  int deltaw,
  int deltah );

int activeWindowClass::drawAfterResizeAbs (
  activeWindowClass *actWin,
  int deltaX,
  double xScaleFactor,
  int deltaY,
  double yScaleFactor );

activeGraphicListPtr activeWindowClass::list ( void ) {

  return head;

}

int activeWindowClass::createAutoPopup (
  appContextClass *ctx,
  Widget parent,
  int OneX,
  int OneY,
  int OneW,
  int OneH,
  int _numMacros,
  char **_macros,
  char **_expansions );

int activeWindowClass::create (
  appContextClass *ctx,
  Widget parent,
  int x,
  int y,
  int w,
  int h,
  int nMacros,
  char **macros,
  char **expansions );

int activeWindowClass::createNoEdit (
  appContextClass *ctx,
  Widget parent,
  int x,
  int y,
  int w,
  int h,
  int nMacros,
  char **macros,
  char **expansions );

int activeWindowClass::genericCreate (
  appContextClass *ctx,
  Widget parent,
  int OneX,
  int OneY,
  int OneW,
  int OneH,
  int windowDecorations,
  int _noEdit,
  int closeAllowed,
  int _numMacros,
  char **_macros,
  char **_expansions );

void activeWindowClass::realize ( void );

int activeWindowClass::setGraphicEnvironment (
  colorInfoClass *Oneci,
  fontInfoClass *Onefi );

Display *activeWindowClass::display ( void );

Widget activeWindowClass::topWidgetId ( void );

Widget activeWindowClass::drawWidgetId ( void );

Widget activeWindowClass::executeWidgetId ( void );

int activeWindowClass::changed ( void );

void activeWindowClass::setChanged ( void );

void activeWindowClass::setUnchanged ( void );

int activeWindowClass::genericLoadScheme (
  char *fName,
  int includeDisplayProperties );

int activeWindowClass::loadScheme (
  char *fName );

int activeWindowClass::loadComponentScheme (
  char *fName );

int activeWindowClass::saveScheme (
  char *fileName );

int activeWindowClass::save (
  char *fileName );

int activeWindowClass::saveNoChange (
  char *fileName );

int activeWindowClass::genericSave (
  char *fileName,
  int resetChangeFlag,
  int appendExtensionFlag,
  int backupFlag );

int activeWindowClass::loadCascade ( void );

int activeWindowClass::loadCascade (
  int x,
  int y );

int activeWindowClass::load ( void );

int activeWindowClass::load (
  int x,
  int y );

int activeWindowClass::import ( void );

int activeWindowClass::importWin (
  FILE *f );

int activeWindowClass::refreshGrid ( void );

int activeWindowClass::clear ( void );

int activeWindowClass::refresh ( void );

int activeWindowClass::refresh (
  int _x,
  int _y,
  int _w,
  int _h );

void activeWindowClass::displayGrid ( void );

void activeWindowClass::displayGrid (
  int _x,
  int _y,
  int _w,
  int _h );

int activeWindowClass::execute ( void );

int activeWindowClass::reexecute ( void );

int activeWindowClass::executeMux ( void );

int activeWindowClass::returnToEdit (
  int closeFlag );

int activeWindowClass::preReexecute ( void );

void activeWindowClass::setState (
  int _state )
{

  state = _state;

}

void activeWindowClass::setCurrentPointObject (
  activeGraphicClass *cur ) {
  currentPointObject = cur;
}

void activeWindowClass::setCurrentObject (
  activeGraphicClass *cur ) {
  currentObject = cur;
}

void activeWindowClass::lineEditBegin ( void );

void activeWindowClass::operationComplete ( void );

int activeWindowClass::clearActive ( void );

int activeWindowClass::refreshActive ( void );

int activeWindowClass::refreshActive (
  int _x,
  int _y,
  int _w,
  int _h );

int activeWindowClass::requestActiveRefresh ( void );

int activeWindowClass::saveWin (
  FILE *fptr );

int activeWindowClass::pushVersion ( void );

int activeWindowClass::popVersion ( void );

void activeWindowClass::readCommentsAndVersion (
  FILE *f );

void activeWindowClass::discardCommentsAndVersion (
  FILE *f,
  int *_major,
  int *_minor,
  int *_release );

int activeWindowClass::loadWin (
  FILE *fptr );

int activeWindowClass::loadWin (
  FILE *fptr,
  int x,
  int y );

int activeWindowClass::discardWinLoadData (
  FILE *fptr,
  int *_major,
  int *_minor,
  int *_release );

int activeWindowClass::createWidgets ( void );

void activeWindowClass::setNoRefresh ( void ) {
  noRefresh = 1;
}

void activeWindowClass::setRefresh ( void ) {
  noRefresh = 0;
}

int activeWindowClass::isActive ( void ) {
  if ( mode == AWC_EXECUTE )
    return 1;
  else
    return 0;
}

char *activeWindowClass::curFileName ( void ) {

  return this->fileName;

}

int activeWindowClass::fileExists (
  char *fname );

int activeWindowClass::edlFileExists (
  char *fname );

int activeWindowClass::renameToBackupFile (
  char *fname );

void activeWindowClass::setDisplayScheme (
  displaySchemeClass *displayScheme );

 void activeWindowClass::updateEditSelectionPointers ( void );

void activeWindowClass::updateMasterSelection ( void );

void activeWindowClass::showSelectionObject ( void );

int activeWindowClass::initDefExeNode (
  void *node );

int activeWindowClass::addDefExeNode (
  void *node );

int activeWindowClass::remDefExeNode (
  void *node );

/*  int activeWindowClass::remDefExeNode ( */
/*    void **node ); */

void activeWindowClass::processObjects ( void );

/* new new new */

void activeWindowClass::storeFileName (
  char *inName );

FILE *activeWindowClass::openAny (
  char *name,
  char *mode );

FILE *activeWindowClass::openAnySymFile (
  char *name,
  char *mode );

/* new new new */

FILE *activeWindowClass::openExchangeFile (
  char *name,
  char *mode );

void activeWindowClass::executeFromDeferredQueue( void );

int activeWindowClass::readUntilEndOfData (
  FILE *f );

int activeWindowClass::readUntilEndOfData (
  FILE *f,
  int _major,
  int _minor,
  int _release );

void activeWindowClass::initLine ( void );

void activeWindowClass::incLine ( void );

int activeWindowClass::line ( void );

void activeWindowClass::setLine (
  int _line );

void activeWindowClass::substituteSpecial (
  int max,
  char *bufIn,
  char *bufOut );

void activeWindowClass::popupDragBegin ( void );

void activeWindowClass::popupDragBegin (
  char *label );

void activeWindowClass::popupDragAddItem (
  void *actGrfPtr,
  char *item );

void activeWindowClass::popupDragFinish (
  int x,
  int y );

void activeWindowClass::enableBuffering ( void );

void activeWindowClass::disableBuffering ( void );

void activeWindowClass::setUndoText (
  char *string );

void activeWindowClass::closeDeferred (
  int cycles );

int activeWindowClass::checkPoint (
  int primaryServer,
  FILE *fptr );

void activeWindowClass::openExecuteSysFile (
  char *fName );

void activeWindowClass::reloadSelf ( void );

char endSignature[15+1];

};

#endif
