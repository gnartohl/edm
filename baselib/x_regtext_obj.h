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

#ifndef __x_regtext_obj_h
#define __x_regtext_obj_h 1

#include "x_text_obj.h"

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

extern "C"
{
#include<regex.h>
}

#include "x_text_obj.str"

static char *dragName[] = {
  activeXTextClass_str1,
  activeXTextClass_str2,
  activeXTextClass_str3
};

class activeXRegTextClass : public activeGraphicClass {

protected:

static void activeXRegTextClass::edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

static void activeXRegTextClass::edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

static void activeXRegTextClass::edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

static void activeXRegTextClass::edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

static void activeXRegTextClass::edit_cancel_delete (
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
char bufAlarmPvName[39+1];

expStringClass visPvExpStr;
char bufVisPvName[39+1];

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
 needPropertyUpdate;

int curFgColorIndex, curBgColorIndex, curStatus, curSeverity;
static const int alarmPvConnection = 1;
static const int visPvConnection = 2;
pvConnectionClass connection;

//----------------------------------
    char regExpStr[39+1], bufRegExp[39+1];

    regex_t compiled_re;
    bool    re_valid;
//----------------------------------

public:

static void activeXRegTextClass::alarmPvConnectStateCallback (
  ProcessVariable *pv,
  void *userarg
);

static void activeXRegTextClass::alarmPvValueCallback (
  ProcessVariable *pv,
  void *userarg
);

static void activeXRegTextClass::visPvConnectStateCallback (
  ProcessVariable *pv,
  void *userarg
);

static void activeXRegTextClass::visPvValueCallback (
  ProcessVariable *pv,
  void *userarg
);

activeXRegTextClass::activeXRegTextClass ( void );

activeXRegTextClass::activeXRegTextClass
 ( const activeXRegTextClass *source );

activeXRegTextClass::~activeXRegTextClass ( void ) {

  if ( name ) delete name;

}

char *activeXRegTextClass::objName ( void ) {

  return name;

}

int activeXRegTextClass::createInteractive (
  activeWindowClass *aw_obj,
  int x,
  int y,
  int w,
  int h );

int activeXRegTextClass::save (
  FILE *f );

int activeXRegTextClass::createFromFile (
  FILE *fptr,
  char *name,
  activeWindowClass *actWin );

int activeXRegTextClass::importFromXchFile (
  FILE *fptr,
  char *name,
  activeWindowClass *actWin );

int activeXRegTextClass::genericEdit ( void );

int activeXRegTextClass::edit ( void );

int activeXRegTextClass::editCreate ( void );

int activeXRegTextClass::draw ( void );

int activeXRegTextClass::erase ( void );

int activeXRegTextClass::drawActive ( void );

int activeXRegTextClass::eraseActive ( void );

int activeXRegTextClass::eraseUnconditional ( void );

int activeXRegTextClass::expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] );

int activeXRegTextClass::expand2nd (
  int numMacros,
  char *macros[],
  char *expansions[] );

int activeXRegTextClass::containsMacros ( void );

int activeXRegTextClass::activate (
  int pass,
  void *ptr );

int activeXRegTextClass::deactivate (
  int pass );

void activeXRegTextClass::updateDimensions ( void );

void activeXRegTextClass::executeDeferred ( void );

int activeXRegTextClass::setProperty (
  char *prop,
  char *value );

char *activeXRegTextClass::firstDragName ( void );

char *activeXRegTextClass::nextDragName ( void );

char *activeXRegTextClass::dragValue (
  int i );

void activeXRegTextClass::changeDisplayParams (
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

void activeXRegTextClass::changePvNames (
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

void activeXRegTextClass::updateColors (
  double colorValue );

};

#ifdef __cplusplus
extern "C" {
#endif

void *create_activeXRegTextClassPtr ( void );
void *clone_activeXRegTextClassPtr ( void * );

#ifdef __cplusplus
}
#endif

#endif
