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

#ifndef __x_text_obj_h
#define __x_text_obj_h 1

#include "act_grf.h"
#include "entry_form.h"
#include "pv_factory.h"
#include "epics_pv_factory.h"
#include "cvtFast.h"

#define AXTC_K_COLORMODE_STATIC 0
#define AXTC_K_COLORMODE_ALARM 1

#define AXTC_MAJOR_VERSION 2
#define AXTC_MINOR_VERSION 0
#define AXTC_RELEASE 0

#ifdef __x_text_obj_cc

#include "x_text_obj.str"

static char *dragName[] = {
  activeXTextClass_str1,
  activeXTextClass_str2,
  activeXTextClass_str3
};

static void unconnectedTimeout (
  XtPointer client,
  XtIntervalId *id );

static void axtc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

static void axtc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

static void axtc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

static void axtc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

static void axtc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

#endif

class activeXTextClass : public activeGraphicClass {

private:

friend void unconnectedTimeout (
  XtPointer client,
  XtIntervalId *id );

friend void axtc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void axtc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void axtc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void axtc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void axtc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

int bufX, bufY, bufW, bufH;

pvColorClass fgColor;
int bufFgColor;
colorButtonClass fgCb;

int fgColorMode;
int bufFgColorMode;

pvColorClass bgColor;
int bufBgColor;
colorButtonClass bgCb;

int bgColorMode;
int bufBgColorMode;

int pvType;
pvValType pvValue, minVis, maxVis;
char minVisString[39+1], bufMinVisString[39+1];
char maxVisString[39+1], bufMaxVisString[39+1];

int prevVisibility, visibility, visInverted, bufVisInverted;
int fgVisibility, prevFgVisibility;
int bgVisibility, prevBgVisibility;

ProcessVariable *alarmPvId;
ProcessVariable *visPvId;

expStringClass alarmPvExpStr;
char bufAlarmPvName[PV_Factory::MAX_PV_NAME+1];

expStringClass visPvExpStr;
char bufVisPvName[PV_Factory::MAX_PV_NAME+1];

int alarmPvExists, visPvExists;
int activeMode, init, opComplete;

expStringClass value;
char bufValue[255+1];

fontMenuClass fm;
char fontTag[63+1], bufFontTag[63+1];
int useDisplayBg, bufUseDisplayBg, alignment;
XFontStruct *fs;
int fontAscent, fontDescent, fontHeight, stringLength, stringWidth,
 stringY, stringX;
int autoSize, bufAutoSize;

int needConnectInit, needAlarmUpdate, needVisUpdate, needRefresh,
 needPropertyUpdate, needToDrawUnconnected, needToEraseUnconnected;
int unconnectedTimer;

int curFgColorIndex, curBgColorIndex, curStatus, curSeverity;
static const int alarmPvConnection = 1;
static const int visPvConnection = 2;
pvConnectionClass connection;

public:

static void activeXTextClass::alarmPvConnectStateCallback (
  ProcessVariable *pv,
  void *userarg
);

static void activeXTextClass::alarmPvValueCallback (
  ProcessVariable *pv,
  void *userarg
);

static void activeXTextClass::visPvConnectStateCallback (
  ProcessVariable *pv,
  void *userarg
);

static void activeXTextClass::visPvValueCallback (
  ProcessVariable *pv,
  void *userarg
);

activeXTextClass::activeXTextClass ( void );

activeXTextClass::activeXTextClass
 ( const activeXTextClass *source );

activeXTextClass::~activeXTextClass ( void );

char *activeXTextClass::objName ( void ) {

  return name;

}

int activeXTextClass::createInteractive (
  activeWindowClass *aw_obj,
  int x,
  int y,
  int w,
  int h );

int activeXTextClass::save (
  FILE *f );

int activeXTextClass::createFromFile (
  FILE *fptr,
  char *name,
  activeWindowClass *actWin );

int activeXTextClass::importFromXchFile (
  FILE *fptr,
  char *name,
  activeWindowClass *actWin );

int activeXTextClass::genericEdit ( void );

int activeXTextClass::edit ( void );

int activeXTextClass::editCreate ( void );

int activeXTextClass::draw ( void );

int activeXTextClass::erase ( void );

int activeXTextClass::drawActive ( void );

int activeXTextClass::eraseActive ( void );

int activeXTextClass::eraseUnconditional ( void );

int activeXTextClass::expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] );

int activeXTextClass::expand2nd (
  int numMacros,
  char *macros[],
  char *expansions[] );

int activeXTextClass::containsMacros ( void );

int activeXTextClass::activate (
  int pass,
  void *ptr );

int activeXTextClass::deactivate (
  int pass );

void activeXTextClass::updateDimensions ( void );

void activeXTextClass::executeDeferred ( void );

int activeXTextClass::setProperty (
  char *prop,
  char *value );

char *activeXTextClass::firstDragName ( void );

char *activeXTextClass::nextDragName ( void );

char *activeXTextClass::dragValue (
  int i );

void activeXTextClass::changeDisplayParams (
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

void activeXTextClass::changePvNames (
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

void activeXTextClass::updateColors (
  double colorValue );

};

#ifdef __cplusplus
extern "C" {
#endif

void *create_activeXTextClassPtr ( void );
void *clone_activeXTextClassPtr ( void * );

#ifdef __cplusplus
}
#endif

#endif
