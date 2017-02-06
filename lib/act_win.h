
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
#include "pv_factory.h"
#include "entry_form.h"
#include "confirm_dialog.h"
#include "cursor.h"
#include "scheme.h"
#include "undo.h"
#include "msg_dialog.h"
#include "pv_action.h"
#include "dimDialog.h"

#include "tag_pkg.h"

#include "sys_types.h"
#include "thread.h"
#include "avl.h"

#define AWC_MAXTMPLPARAMS 30
#define AWC_TMPLPARAMSIZE 35
#define AWC_MAXTEMPLINFO 600

#define AWC_MAJOR_VERSION 4
#define AWC_MINOR_VERSION 0
#define AWC_RELEASE 1

#define AWC_EDIT 1
#define AWC_EXECUTE 2

#define AWC_BGPIXMAP_PER_ENV_VAR 0
#define AWC_BGPIXMAP_NEVER 1
#define AWC_BGPIXMAP_ALWAYS 2

#define AWC_INIT 1000
#define AWC_START_EXECUTE 1001
#define AWC_COMPLETE_EXECUTE 1002
#define AWC_START_DEACTIVATE 1003
#define AWC_COMPLETE_DEACTIVATE 1004
#define AWC_TERMINATED 1005

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
#define AWC_POPUP_PRINT 152
#define AWC_POPUP_COPY_GROUP_INFO 153
#define AWC_POPUP_PASTE_GROUP_INFO 154
#define AWC_POPUP_SAVE_TO_PATH 155
#define AWC_POPUP_DUMP_PVLIST 156
#define AWC_POPUP_OPEN_SELF 157
#define AWC_POPUP_SHOW_MACROS 158
#define AWC_POPUP_INSERT_TEMPLATE 159
#define AWC_POPUP_TOGGLE_VIEW_DIMS 160
#define AWC_POPUP_RECORD_DIMS 161
#define AWC_POPUP_SET_PASTE_INDEX 162
#define AWC_POPUP_SAR 163

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
#define AWC_CREATING_POINTS 22
#define AWC_MOVING_CREATE_POINT 23

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

static void awc_do_save_cb (
  Widget w,
  XtPointer client,
  XtPointer call );

static void awc_do_save_and_exit_cb (
  Widget w,
  XtPointer client,
  XtPointer call );

static void awc_do_save_new_path_cb (
  Widget w,
  XtPointer client,
  XtPointer call );

static void awc_templateFileSelectOk_cb (
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

static void awc_pvlistFileSelectOk_cb (
  Widget w,
  XtPointer client,
  XtPointer call );

static void awc_pvlistFileSelectCancel_cb (
  Widget w,
  XtPointer client,
  XtPointer call );

static void awc_pvlistFileSelectKill_cb (
  Widget w,
  XtPointer client,
  XtPointer call );

static void awc_tedit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

static void awc_tedit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

static void awc_tedit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

static int getCurReplaceIndex (
  activeWindowClass *awo
);

static void awc_editReplace_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

static void awc_editReplace_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

static void awc_editReplace_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

static void awc_editSaR_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

static void awc_editSaR_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

static void awc_editSaR_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

static void awc_editSetAcc_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

static void awc_editSetAcc_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

static void awc_editSetAcc_cancel (
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

static void awc_edit_ok1 (
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

static void action_cb (
  Widget w,
  XtPointer client,
  XtPointer call );

#endif

class appContextClass;
class activeGraphicClass;
class dimDialogClass;

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

typedef struct pvDefTag {
  struct pvDefTag *flink;
  char *def;
  ProcessVariable *id;
} pvDefType, *pvDefPtr;

typedef struct refPointTag {
  char label[31+1];
  int x;
  int y;
} refPointType, *refPointPtr;

typedef struct refRectTag {
  char label[31+1];
  int x;
  int y;
  int w;
  int h;
} refRectType, *refRectPtr;

typedef struct showDimBufTag {
  int init;
  int x;
  int y;
  double dist;
  double theta;
  double relTheta;
  int objX;
  int objY;
  int objW;
  int objH;
  int objTopDist;
  int objBotDist;
  int objLeftDist;
  int objRightDist;
  int prev_x;
  int prev_y;
  double prev_dist;
  double prev_theta;
  double prev_relTheta;
  int prev_objX;
  int prev_objY;
  int prev_objW;
  int prev_objH;
  int prev_objTopDist;
  int prev_objBotDist;
  int prev_objLeftDist;
  int prev_objRightDist;
} showDimBufType, *showDimBufPtr;

static char stdEdlFileExt[63+1] = ".edl";
static char defEdlFileExt[63+1] = ".edl";
static char defEdlFileSearchMask[63+1] = "*.edl";

/////////////// start class declaration: activeWindowClass

class activeWindowClass {

unknownTagList unknownTags;

public:

dimDialogClass *dimDialog;
int viewDims;

showDimBufType showDimBuf;
int numRefPoints;
refPointType refPoint[2];
int recordedRefRect;
int numRefRects;
refRectType refRect[2];
XtIntervalId showDimTimer;

//static char stdEdlFileExt[63+1];


int clearEpicsPvTypeDefault;

static const int NUM_PER_PENDIO = 1000;

char curSchemeSet[63+1];

char startSignature[15+1];

static const int MAX_DRAG_ITEMS = 30;
int dragItemIndex;
dragPopupBlockType dragPopupBlock[MAX_DRAG_ITEMS];

int major, minor, release, fileLineNumber;

int buttonPressX, buttonPressY;

commentLinesPtr commentHead, commentTail;
pvDefPtr pvDefHead, pvDefTail;
int forceLocalPvs;

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

int doClose, doActiveClose;

widgetAndPointerType wpFileSelect, wpSchemeSelect, pvlistFileSelect;

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

friend void awc_do_save_new_path_cb (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void awc_templateFileSelectOk_cb (
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

friend void awc_pvlistFileSelectOk_cb (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void awc_pvlistFileSelectCancel_cb (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void awc_pvlistFaveFileSelectOk_cb (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void *pv_poll_thread (
  THREAD_HANDLE h );

friend void awc_tedit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void awc_tedit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void awc_tedit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

friend int getCurReplaceIndex (
  activeWindowClass *awo
);

friend void awc_editReplace_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void awc_editReplace_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void awc_editReplace_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void awc_editSaR_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void awc_editSaR_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void awc_editSaR_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void awc_editSetAcc_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void awc_editSetAcc_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void awc_editSetAcc_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

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

friend void awc_edit_ok1 (
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

friend void action_cb (
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

int accVal, bufAccVal;

int efSaRW, efSaRH, efSaRLargestH;
int sarCaseInsensivite, sarUseRegExpr;
char *sar1, *sar2;
activeGraphicListPtr sarCurSel;

int efReplaceW, efReplaceH, efReplaceLargestH;
char *replaceOld, *replaceNew;
int curReplaceIndex;
int seachStatus;

cursorClass cursor;

Widget drawWidget, executeWidget, fileSelectBox, schemeSelectBox,
 pvlistFileSelectBox, templateFileSelectBox;
Display *d;
int bufX, bufY, bufW, bufH;

activeGraphicClass *internalRelatedDisplay;

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
int bufGridActive, gridActive;
int bufGridShow, gridShow;
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
int mode; // AWC_EDIT or AWC_EXECUTE
int windowState; // AWC_INIT, AWC_START_EXECUTE, AWC_COMPLETE_EXECUTE,
                 // AWC_START_DEACTIVATE, AWC_COMPLETE_DEACTIVATE
int waiting;
int change;
int exit_after_save;

char paramValue[AWC_MAXTMPLPARAMS][AWC_TMPLPARAMSIZE+1];
char bufParamValue[AWC_MAXTMPLPARAMS][AWC_TMPLPARAMSIZE+1];
int bufNumParamValues, numParamValues;
char templInfo[AWC_MAXTEMPLINFO+1], *bufTemplInfo;

int list_array_size;
activeGraphicListType *list_array;

objBindingClass obj;
pvBindingClass pvObj;

Widget b1OneSelectPopup, b1ManySelectPopup, b1NoneSelectPopup,
 b2NoneSelectPopup, b2OneSelectPopup, b2ManySelectPopup, b3NoneSelectPopup,
 b2ExecutePopup, actionPopup, chPd, grPd, grCb, mnPd, mnCb, ctlPd, ctlCb,
 alignPd, alignCb, centerPd, centerCb, distributePd, distributeCb, sizePd,
 sizeCb, orientPd1, orientPdM, orientCb1, orientCbM, editPd1, editPdM,
 editCb1, editCbM, dragPopup, undoPb1, undoPb2, undoPb3, setSchemePd,
 setSchemeCb;

int b2NoneSelectX;
int b2NoneSelectY;
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
int usePixmap;
Pixmap bgPixmap;
int pixmapW, pixmapH;
int needCopy, needFullCopy;
int pixmapX0, pixmapX1, pixmapY0, pixmapY1;

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

entryFormClass ef, *ef1, tef, efSetAcc, efSaR, efReplace;
confirmDialogClass confirm, confirm1;

int noRefresh;

char defaultFontTag[127+1];
char defaultCtlFontTag[127+1];
char defaultBtnFontTag[127+1];

char allSelectedCtlPvName[1][PV_Factory::MAX_PV_NAME+1];
char allSelectedReadbackPvName[1][PV_Factory::MAX_PV_NAME+1];
char allSelectedNullPvName[1][PV_Factory::MAX_PV_NAME+1];
char allSelectedVisPvName[1][PV_Factory::MAX_PV_NAME+1];
char allSelectedAlarmPvName[1][PV_Factory::MAX_PV_NAME+1];
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

int numTemplateMacros;
char **templateMacros;
char **templateExpansions;

int haveComments;
char fileName[255+1], fileRev[31+1], fileNameAndRev[287+1], newPath[255+1];
char prefix[127+1], displayName[127+1], postfix[127+1];
char fileNameForSym[255+1], prefixForSym[127+1], displayNameForSym[127+1],
 postfixForSym[127+1];
expStringClass expStrTitle;

colorInfoClass *ci;
entryFormClass *currentEf;
activeGraphicClass *currentObject, *currentPointObject;
pointPtr currentPoint;

int x, y, w, h;
Widget top, scroll;
gcClass drawGc, executeGc;
fontInfoClass *fi;

int versionStack[11][4];
int versionStackPtr;

int b2PressX, b2PressY, b2PressXRoot, b2PressYRoot;

undoClass undoObj;

Time buttonClickTime, deltaTime;

int noRaise;

int noEdit; // used only to save the no-edit state for checkpointing

int closeAllowed;

time_t modTime; // time of last file modification when window was opened
int stale; // true if file has been modified since window was opened (or saved)

activeGraphicClass *highlightedObject;

int numChildren;
activeWindowClass *parent;

int isEmbedded, embeddedX, embeddedY, embeddedW, embeddedH, embSizeOfs,
 embSetSize, embCenter, embBg, embNoScroll;
Widget *widgetToDeallocate;

int btnDownX, btnDownY;

int loadFailure;

int bufDisableScroll, disableScroll;

int bufBgPixmapFlag, bgPixmapFlag; // 0=per env var, 1=never use, 2=always use

pvActionClass *pvAction;

int ctlKeyPressed;

int invalidFile, invalidBgColor;

int reloadRequestFlag;

bool frozen;

activeWindowClass ( void );

~activeWindowClass ( void );

static char* stdExt ( void ) {

  return stdEdlFileExt;

}

static char* defExt ( void ) {

char *envPtr;
static int init = 1;

  if ( init ) {
    init = 0;
    envPtr = getenv( environment_str32 );
    if ( envPtr ) {
      strncpy( defEdlFileExt, envPtr, 62 );
      defEdlFileExt[62] = 0;
    }
  }

  return defEdlFileExt;

}

static char* defMask ( void ) {

char *envPtr;
static int init = 1;

  if ( init ) {
    init = 0;
    envPtr = getenv( environment_str32 );
    if ( envPtr ) {
      strcpy( defEdlFileSearchMask, "*" );
      Strncat( defEdlFileSearchMask, envPtr, 63 );
      defEdlFileSearchMask[63] = 0;
    }
  }

  return defEdlFileSearchMask;

}

void select(activeGraphicListPtr cur);
void unselect(activeGraphicListPtr cur);

 int getRandFile (
  char *outStr,
  int outStrMaxLen
);

void dumpPvList ( void );

int okToDeactivate ( void );

void initCopy( void );

void updateCopyRegion (
  int _x0,
  int _y0,
  int _w,
  int _h
);

void doCopy ( void );

void doMinCopy ( void );

Drawable drawable (
  Widget w
);

int okToPreReexecute ( void );

char *idName( void );

void getModTime (
  char *oneFileName
);

void checkModTime (
  char *oneFileName
);

int setProperty (
  char *id,
  char *property,
  char *value );

int setProperty (
  char *id,
  char *property,
  double *value );

int setProperty (
  char *id,
  char *property,
  int *value );

int getProperty (
  char *id,
  char *property,
  int bufSize,
  char *value );

int getProperty (
  char *id,
  char *property,
  double *value );

int getProperty (
  char *id,
  char *property,
  int *value );

void updateAllSelectedDisplayInfo ( void );

void setTitle ( void );

void setTitleUsingTitle ( void );

void expandTitle (
  int phase,
  int nMac,
  char **mac,
  char **exp
);

void filterPosition (
  int *_x,
  int *_y,
  int oldX,
  int oldY );

int drawAfterResize (
  activeWindowClass *actWin,
  int deltax,
  int deltay,
  int deltaw,
  int deltah );

int drawAfterResizeAbs (
  activeWindowClass *actWin,
  int deltaX,
  double xScaleFactor,
  int deltaY,
  double yScaleFactor );

activeGraphicListPtr list ( void ) {

  return head;

}

int createAutoPopup (
  appContextClass *ctx,
  Widget parent,
  int OneX,
  int OneY,
  int OneW,
  int OneH,
  int _numMacros,
  char **_macros,
  char **_expansions );

int create (
  appContextClass *ctx,
  Widget parent,
  int x,
  int y,
  int w,
  int h,
  int nMacros,
  char **macros,
  char **expansions );

int createNoEdit (
  appContextClass *ctx,
  Widget parent,
  int x,
  int y,
  int w,
  int h,
  int nMacros,
  char **macros,
  char **expansions );

int createEmbedded (
  appContextClass *ctx,
  Widget *parent,
  int OneX,
  int OneY,
  int OneW,
  int OneH,
  int _embeddedX,
  int _embeddedY,
  int _embCenter,
  int _embSetSize,
  int _embSizeOfs,
  int _embNoScroll,
  int _numMacros,
  char **_macros,
  char **_expansions );

int genericCreate (
  appContextClass *ctx,
  Widget parent,
  int OneX,
  int OneY,
  int OneW,
  int OneH,
  int windowDecorations,
  int _noEdit,
  int closeAllowed,
  int _isEmbedded,
  int _noScroll,
  Widget *_widgetToDeallocate,
  int _numMacros,
  char **_macros,
  char **_expansions );

int createNodeForCrawler (
  appContextClass *ctx,
  char *filename
);

void map ( void );

void realize ( void );

void realizeNoMap ( void );

void realize (
  int doMap );

int setGraphicEnvironment (
  colorInfoClass *Oneci,
  fontInfoClass *Onefi );

Display *display ( void );

Widget topWidgetId ( void );

Widget actualTopWidgetId ( void );

activeWindowClass *actualTopObject ( void );

Widget drawWidgetId ( void );

Widget executeWidgetId ( void );

Widget scrollWidgetId() { return scroll; };

int changed ( void );

void setChanged ( void );

void setUnchanged ( void );

int genericLoadScheme (
  char *fName,
  int includeDisplayProperties );

int loadScheme (
  char *fName );

int loadComponentScheme (
  char *fName );

int saveScheme (
  char *fileName );

int save (
  char *fileName );

int saveNoChange (
  char *fileName );

int old_genericSave (
  char *fName,
  int resetChangeFlag,
  int appendExtensionFlag,
  int backupFlag );

int genericSave (
  char *fileName,
  int resetChangeFlag,
  int appendExtensionFlag,
  int backupFlag );

int old_loadGeneric (
  int x,
  int y,
  int setPosition );

int old_load ( void );

int old_load (
  int x,
  int y );

int loadDummy (
  int x,
  int y,
  int setPosition );

int getTemplateMacros ( void );

void deleteTemplateMacros ( void );

int loadTemplate (
  int x,
  int y,
  char *fname );

int loadGeneric (
  int x,
  int y,
  int setPosition );

int load ( void );

int load (
  int x,
  int y );

int import ( void );

int importWin (
  FILE *f );

int refreshGrid ( void );

int clear ( void );

int refresh ( void );

int refresh (
  int _x,
  int _y,
  int _w,
  int _h );

void displayGrid ( void );

void displayGrid (
  int _x,
  int _y,
  int _w,
  int _h );

int execute ( void );

int reexecute ( void );

int executeMux ( void );

int returnToEdit (
  int closeFlag );

int preReexecute ( void );

void setState (
  int _state )
{

  state = _state;

}

void setCurrentPointObject (
  activeGraphicClass *cur ) {
  currentPointObject = cur;
}

void setCurrentObject (
  activeGraphicClass *cur ) {
  currentObject = cur;
}

void lineEditBegin ( void );

void lineCreateBegin ( void );

void operationComplete ( void );

int clearActive ( void );

int refreshActive ( void );

int refreshActive (
  int _x,
  int _y,
  int _w,
  int _h );

int requestSmartDrawAllActive ( void );

int smartDrawAllActive ( void );

int requestActiveRefresh ( void );

int old_saveWin (
  FILE *fptr );

int saveWin (
  FILE *fptr );

int pushVersion ( void );

int popVersion ( void );

void readCommentsAndVersionGeneric (
  FILE *f,
  int isSymbolFile );

void readCommentsAndVersion (
  FILE *f );

void readSymbolCommentsAndVersion (
  FILE *f );

void discardCommentsAndVersion (
  FILE *f,
  int *_major,
  int *_minor,
  int *_release );

int loadWinDummy (
  FILE *f,
  int _x,
  int _y,
  int setPosition );

int loadWinGeneric (
  FILE *f,
  int _x,
  int _y,
  int setPosition );

int loadWin (
  FILE *f );

int loadWin (
  FILE *f,
  int _x,
  int _y );

int old_loadWinGeneric (
  FILE *f,
  int _x,
  int _y,
  int setPosition );

int old_loadWin (
  FILE *fptr );

int old_loadWin (
  FILE *fptr,
  int x,
  int y );

int discardWinLoadData (
  FILE *fptr,
  int *_major,
  int *_minor,
  int *_release );

void setNoRefresh ( void ) {
  noRefresh = 1;
}

void setRefresh ( void ) {
  noRefresh = 0;
}

int isActive ( void ) {
  if ( mode == AWC_EXECUTE )
    return 1;
  else
    return 0;
}

char *curFileName ( void ) {

  return this->fileName;

}

int fileExists (
  char *fname );

int edlFileExists (
  char *fname );

int renameToBackupFile (
  char *fname );

void setDisplayScheme (
  displaySchemeClass *displayScheme );

 void updateEditSelectionPointers ( void );

void updateMasterSelection ( void );

void showSelectionObject ( void );

int initDefExeNode (
  void *node );

int addDefExeNode (
  void *node );

int remDefExeNode (
  void *node );

/*  int remDefExeNode ( */
/*    void **node ); */

int processObjects ( void );

/* new new new */

void storeFileName (
  char *inName );

void storeFileNameForSymbols (
  char *inName );

FILE *openAny (
  char *name,
  char *mode );

FILE *openAnyTemplate (
  char *name,
  char *mode );

FILE *openAnyTemplateParam (
  char *name,
  char *mode );

FILE *openAnySymFile (
  char *name,
  char *mode );

/* new new new */

FILE *openExchangeFile (
  char *name,
  char *mode );

FILE *openAnyGenericFile (
  char *name,
  char *mode,
  char *fullName,
  int max );

void executeFromDeferredQueue( void );

int readUntilEndOfData (
  FILE *f );

int readUntilEndOfData (
  FILE *f,
  int _major,
  int _minor,
  int _release );

void initLine ( void );

void incLine ( void );

int line ( void );

void setLine (
  int _line );

void substituteSpecial (
  int max,
  char *bufIn,
  char *bufOut );

void popupDragBegin ( void );

void popupDragBegin (
  char *label );

void popupDragAddItem (
  void *actGrfPtr,
  char *item );

void popupDragFinish (
  XButtonEvent *be );

void enableBuffering ( void );

void disableBuffering ( void );

void setUndoText (
  char *string );

void closeDeferred (
  int cycles );

void closeAnyDeferred (
  int cycles );

int checkPoint (
  int primaryServer,
  FILE *fptr );

void openExecuteSysFile (
  char *fName );

void reloadSelf ( void );

void requestReload ( void );

int isExecuteMode ( void );

int xPos ( void );

int yPos ( void );

void move (
  int newX,
  int newY
);

void getDrawWinPos (
  int *curX,
  int *curY
);

int sameAncestorName (
  char *name
);

void reconfig ( void );

void clip ( void );

void freeze(bool);

bool is_frozen(void);

char endSignature[15+1];

};

#endif
