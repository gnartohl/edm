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

#ifndef __x_text_dsp_gen_h
#define __x_text_dsp_gen_h 1

#include <Xm/TextF.h>
#include <Xm/Text.h>
#include <math.h>

#include "ulBindings.h"
#include "act_grf.h"
#include "entry_form.h"

// #ifdef __epics__
// #include "cadef.h"
// #endif

#include "pv.h"

// #ifndef __epics__
#define MAX_ENUM_STATES 4
#define MAX_ENUM_STRING_SIZE 63
// #endif

#define XTDC_K_FORMAT_NATURAL 0
#define XTDC_K_FORMAT_FLOAT 1
#define XTDC_K_FORMAT_EXPONENTIAL 2
#define XTDC_K_FORMAT_DECIMAL 3
#define XTDC_K_FORMAT_HEX 4
#define XTDC_K_FORMAT_STRING 5

#define XTDC_K_COLORMODE_STATIC 0
#define XTDC_K_COLORMODE_ALARM 1

#define XTDC_MAJOR_VERSION 1
#define XTDC_MINOR_VERSION 8
#define XTDC_RELEASE 0

#ifdef __x_text_dsp_gen_cc

static void xtdo_monitor_connect_state (
  pvClass *classPtr,
  void *clientData,
  void *args );

static void xtdo_monitor_fg_connect_state (
  pvClass *classPtr,
  void *clientData,
  void *args );

static void XtextDspInfoUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args );

static void XtextAlarmUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args );

static void XtextDspUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args );

static void XtextDspFgUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args );

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

friend void xtdo_monitor_connect_state (
  pvClass *classPtr,
  void *clientData,
  void *args );

friend void xtdo_monitor_fg_connect_state (
  pvClass *classPtr,
  void *clientData,
  void *args );

friend void XtextDspInfoUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args );

friend void XtextAlarmUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args );

friend void XtextDspUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args );

friend void XtextDspFgUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args );

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

friend void xtdoSetSelection (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void xtdoSetValueChanged (
  Widget w,
  XtPointer client,
  XtPointer call );

int bufX, bufY, bufW, bufH;

int numDecimals, bufNumDecimals, formatType, bufFormatType, colorMode,
 bufColorMode, pvType;
char format[15+1];
unsigned char pvNotConnectedMask;
int opComplete, activeMode, pvConnected, init;
int smartRefresh, bufSmartRefresh;
double dvalue;
char curValue[256+1], value[256+1], bufValue[256+1];
fontMenuClass fm;
char fontTag[63+1], bufFontTag[63+1];
int useDisplayBg, bufUseDisplayBg, alignment, autoHeight, bufAutoHeight;
int limitsFromDb, bufLimitsFromDb;
int precision;
efInt efPrecision, bufEfPrecision;
unsigned int bgColor, bufBgColor;
pvColorClass fgColor;
unsigned int bufFgColor;
colorButtonClass fgCb, bgCb;
XFontStruct *fs;
int fontAscent, fontDescent, fontHeight, stringLength, stringWidth,
 stringY, stringX, bufInvalid;

VPFUNC changeCallback, activateCallback, deactivateCallback;
int changeCallbackFlag, activateCallbackFlag, deactivateCallbackFlag,
 anyCallbackFlag;
int bufChangeCallbackFlag, bufActivateCallbackFlag, bufDeactivateCallbackFlag;

int pvExists, fgPvExists;

// #ifdef __epics__
// chid pvId;
// evid eventId, alarmEventId;
// #endif

pvClass *pvId, *fgPvId;
pvEventClass *eventId, *alarmEventId, *fgEventId;

expStringClass pvExpStr, fgPvExpStr;
char pvName[256+1], bufPvName[256+1];

char *stateString[MAX_ENUM_STATES]; // allocated at run-time
int numStates;

// #ifdef __epics__
// struct dbr_gr_enum enumRec;
// #endif

int isWidget, bufIsWidget;
int editable, bufEditable;
entryFormClass textEntry;
int teX, teY, teW, teH, teLargestH;
char entryValue[256+1];
int entryState, editDialogIsActive;

Widget tf_widget;
int widget_value_changed;

int needConnectInit, needInfoInit, needErase, needDraw, needRefresh,
 needUpdate, deferredCount;

char pvClassName[15+1], pvUserClassName[15+1], pvOptionList[255+1];
int numPvTypes, pvNameIndex;

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

void activeXTextDspClass::show ( void );

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

void activeXTextDspClass::btnDown (
  int x,
  int y,
  int buttonState,
  int buttonNumber );

int activeXTextDspClass::getButtonActionRequest (
  int *up,
  int *down,
  int *drag );

void activeXTextDspClass::executeDeferred ( void );

int activeXTextDspClass::getProperty (
  char *prop,
  int bufSize,
  char *value );

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
