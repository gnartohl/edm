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

#ifndef __x_text_dsp_obj_h
#define __x_text_dsp_obj_h 1

#include <Xm/TextF.h>
#include <Xm/Text.h>
#include <math.h>

#include "ulBindings.h"
#include "act_grf.h"
#include "entry_form.h"
#include "keypad.h"
#include "calpad.h"
#include "fileSelect.h"

#include "cadef.h"

#define XTDC_K_FORMAT_NATURAL 0
#define XTDC_K_FORMAT_FLOAT 1
#define XTDC_K_FORMAT_EXPONENTIAL 2
#define XTDC_K_FORMAT_DECIMAL 3
#define XTDC_K_FORMAT_HEX 4
#define XTDC_K_FORMAT_STRING 5

#define XTDC_K_COLORMODE_STATIC 0
#define XTDC_K_COLORMODE_ALARM 1

#define XTDC_MAJOR_VERSION 2
#define XTDC_MINOR_VERSION 5
#define XTDC_RELEASE 0

#ifdef __x_text_dsp_obj_cc

#include "x_text_dsp_obj.str"

static char *dragName[] = {
  activeXTextDspClass_str1,
  activeXTextDspClass_str2
};

static void xtdoCancelStr (
  Widget w,
  XtPointer client,
  XtPointer call );

static void xtdoSetCpValue (
  Widget w,
  XtPointer client,
  XtPointer call );

static void xtdoSetFsValue (
  Widget w,
  XtPointer client,
  XtPointer call );

static void xtdoRestoreValue (
  Widget w,
  XtPointer client,
  XtPointer call );

static void xtdoCancelKp (
  Widget w,
  XtPointer client,
  XtPointer call );

static void xtdoSetKpDoubleValue (
  Widget w,
  XtPointer client,
  XtPointer call );

static void xtdoSetKpIntValue (
  Widget w,
  XtPointer client,
  XtPointer call );

static void selectDrag (
   Widget w,
   XEvent *e,
   String *params,
   Cardinal numParams );

static void xtdo_monitor_connect_state (
  struct connection_handler_args arg );

static void xtdo_monitor_sval_connect_state (
  struct connection_handler_args arg );

static void xtdo_monitor_fg_connect_state (
  struct connection_handler_args arg );

static void XtextDspInfoUpdate (
  struct event_handler_args ast_args );

static void XtextAlarmUpdate (
  struct event_handler_args ast_args );

static void XtextDspUpdate (
  struct event_handler_args ast_args );

static void XtextDspSvalUpdate (
  struct event_handler_args ast_args );

static void XtextDspFgUpdate (
  struct event_handler_args ast_args );

static void axtdc_value_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

static void axtdc_value_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

static void axtdc_value_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

static void axtdc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

static void axtdc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

static void axtdc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

static void axtdc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

static void axtdc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

static void xtdoTextFieldToStringA (
  Widget w,
  XtPointer client,
  XtPointer call );

static void xtdoTextFieldToStringLF (
  Widget w,
  XtPointer client,
  XtPointer call );

static void xtdoTextFieldToIntA (
  Widget w,
  XtPointer client,
  XtPointer call );

static void xtdoTextFieldToIntLF (
  Widget w,
  XtPointer client,
  XtPointer call );

static void xtdoTextFieldToDoubleA (
  Widget w,
  XtPointer client,
  XtPointer call );

static void xtdoTextFieldToDoubleLF (
  Widget w,
  XtPointer client,
  XtPointer call );

static void xtdoGrabUpdate (
  Widget w,
  XtPointer client,
  XtPointer call );

static void xtdoSetSelection (
  Widget w,
  XtPointer client,
  XtPointer call );

static void xtdoSetValueChanged (
  Widget w,
  XtPointer client,
  XtPointer call );

#endif

class activeXTextDspClass : public activeGraphicClass {

private:

friend void xtdoCancelStr (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void xtdoSetCpValue (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void xtdoSetFsValue (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void xtdoRestoreValue (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void xtdoCancelKp (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void xtdoSetKpDoubleValue (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void xtdoSetKpIntValue (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void selectDrag (
   Widget w,
   XEvent *e,
   String *params,
   Cardinal numParams );

friend void xtdo_monitor_connect_state (
  struct connection_handler_args arg );

friend void xtdo_monitor_sval_connect_state (
  struct connection_handler_args arg );

friend void xtdo_monitor_fg_connect_state (
  struct connection_handler_args arg );

friend void XtextDspInfoUpdate (
  struct event_handler_args ast_args );

friend void XtextAlarmUpdate (
  struct event_handler_args ast_args );

friend void XtextDspUpdate (
  struct event_handler_args ast_args );

friend void XtextDspSvalUpdate (
  struct event_handler_args ast_args );

friend void XtextDspFgUpdate (
  struct event_handler_args ast_args );

friend void axtdc_value_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void axtdc_value_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void axtdc_value_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void axtdc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void axtdc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void axtdc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void axtdc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void axtdc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void xtdoTextFieldToStringA (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void xtdoTextFieldToStringLF (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void xtdoTextFieldToIntA (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void xtdoTextFieldToIntLF (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void xtdoTextFieldToDoubleA (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void xtdoTextFieldToDoubleLF (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void xtdoGrabUpdate (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void xtdoSetSelection (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void xtdoSetValueChanged (
  Widget w,
  XtPointer client,
  XtPointer call );

pvConnectionClass connection;

int bufX, bufY, bufW, bufH;

int numDecimals, bufNumDecimals, formatType, bufFormatType, colorMode,
 bufColorMode, pvType, svalPvType, noSval;
char format[15+1];
int opComplete, pvExistCheck, activeMode, init, noSvalYet;
int smartRefresh, bufSmartRefresh;
double dvalue, curDoubleValue, curSvalValue;
char curValue[127+1], value[127+1], bufValue[127+1];
fontMenuClass fm;
char fontTag[63+1], bufFontTag[63+1];
int useDisplayBg, bufUseDisplayBg, alignment, autoHeight, bufAutoHeight;
int limitsFromDb, bufLimitsFromDb;
int changeValOnLoseFocus, bufChangeValOnLoseFocus;
int fastUpdate, bufFastUpdate;
int precision;
efInt efPrecision, bufEfPrecision;
int bgColor, bufBgColor;
pvColorClass fgColor;
int bufFgColor, bufSvalColor;
colorButtonClass fgCb, bgCb, svalCb;
XFontStruct *fs;
int fontAscent, fontDescent, fontHeight, stringLength, stringWidth,
 stringY, stringX, bufInvalid;

VPFUNC changeCallback, activateCallback, deactivateCallback;
int changeCallbackFlag, activateCallbackFlag, deactivateCallbackFlag,
 anyCallbackFlag;
int bufChangeCallbackFlag, bufActivateCallbackFlag, bufDeactivateCallbackFlag;

int pvExists, svalPvExists, fgPvExists;

// nullDetectMode: 0=null when saved value pv equals cur vale
//                 1=null when saved value pv is 0
int nullDetectMode, bufNullDetectMode;

chid pvId, svalPvId, fgPvId;
evid eventId, alarmEventId, svalEventId, fgEventId;

expStringClass pvExpStr, svalPvExpStr, fgPvExpStr;
char pvName[39+1], bufPvName[39+1], bufSvalPvName[39+1],
 bufColorPvName[39+1];

expStringClass defDir, pattern;
char bufDefDir[127+1], bufPattern[127+1];

char *stateString[MAX_ENUM_STATES]; // allocated at run-time
int numStates;

struct dbr_gr_enum enumRec;

int isWidget, bufIsWidget;
int editable, bufEditable;
entryFormClass textEntry;
int teX, teY, teW, teH, teLargestH;
char entryValue[127+1];
int entryState, editDialogIsActive;
int isDate, bufIsDate;
int isFile, bufIsFile;

Widget tf_widget;
int widget_value_changed;

int needConnectInit, needInfoInit, needErase, needDraw, needRefresh,
 needUpdate, deferredCount;

keypadClass kp;
int kpInt;
double kpDouble;
int useKp, bufUseKp;
calpadClass cp;
fselectClass fsel;

int grabUpdate;

public:

activeXTextDspClass::activeXTextDspClass ( void );

activeXTextDspClass::activeXTextDspClass
 ( const activeXTextDspClass *source );

activeXTextDspClass::~activeXTextDspClass ( void );

char *activeXTextDspClass::objName ( void ) {

  return name;

}

int activeXTextDspClass::createInteractive (
  activeWindowClass *aw_obj,
  int x,
  int y,
  int w,
  int h );

int activeXTextDspClass::createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin );

int activeXTextDspClass::importFromXchFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin );

int activeXTextDspClass::save (
  FILE *f );

int activeXTextDspClass::genericEdit ( void );

int activeXTextDspClass::edit ( void );

int activeXTextDspClass::editCreate ( void );

int activeXTextDspClass::draw ( void );

int activeXTextDspClass::erase ( void );

int activeXTextDspClass::drawActive ( void );

int activeXTextDspClass::eraseActive ( void );

void activeXTextDspClass::bufInvalidate ( void );

int activeXTextDspClass::expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] );

int activeXTextDspClass::expand2nd (
  int numMacros,
  char *macros[],
  char *expansions[] );

int activeXTextDspClass::containsMacros ( void );

int activeXTextDspClass::activate (
  int pass,
  void *ptr );

int activeXTextDspClass::deactivate ( int pass );

void activeXTextDspClass::updateDimensions ( void );

void activeXTextDspClass::btnUp (
  int x,
  int y,
  int buttonState,
  int buttonNumber );

void activeXTextDspClass::btnDown (
  int x,
  int y,
  int buttonState,
  int buttonNumber );

void activeXTextDspClass::pointerIn (
  int _x,
  int _y,
  int buttonState );

int activeXTextDspClass::getButtonActionRequest (
  int *up,
  int *down,
  int *drag,
  int *focus );

void activeXTextDspClass::executeDeferred ( void );

int activeXTextDspClass::getProperty (
  char *prop,
  int bufSize,
  char *value );

char *activeXTextDspClass::firstDragName ( void );

char *activeXTextDspClass::nextDragName ( void );

char *activeXTextDspClass::dragValue (
  int i );

void activeXTextDspClass::changeDisplayParams (
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

void activeXTextDspClass::changePvNames (
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

};

#ifdef __cplusplus
extern "C" {
#endif

void *create_activeXTextDspClassPtr ( void );
void *clone_activeXTextDspClassPtr ( void * );

#ifdef __cplusplus
}
#endif

#endif
