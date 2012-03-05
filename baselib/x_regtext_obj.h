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
#include "cvtFast.h"

#if 0
// defined in x_text_obj.h
#define AXTC_K_COLORMODE_STATIC 0
#define AXTC_K_COLORMODE_ALARM 1

#define AXTC_MAJOR_VERSION 4
#define AXTC_MINOR_VERSION 0
#define AXTC_RELEASE 1
#endif

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

static void edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

static void edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

static void edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

static void edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

static void edit_cancel_delete (
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
char fontTag[63+1];
int useDisplayBg, bufUseDisplayBg, alignment;
XFontStruct *fs;
int fontAscent, fontDescent, fontHeight, stringLength, stringWidth,
 stringY, stringX, stringBoxWidth, stringBoxHeight;
int autoSize, bufAutoSize;

int needConnectInit, needAlarmUpdate, needVisUpdate, needRefresh,
 needPropertyUpdate;

int curFgColorIndex, curBgColorIndex, curStatus, curSeverity;
static const int alarmPvConnection = 1;
static const int visPvConnection = 2;
pvConnectionClass connection;

int bufInvalid;

entryListBase *invisPvEntry, *visInvEntry, *minVisEntry, *maxVisEntry;

entryListBase *fillEntry, *fillColorEntry, *fillAlarmSensEntry;

//----------------------------------
    char regExpStr[39+1], bufRegExp[39+1];

    regex_t compiled_re;
    bool    re_valid;
//----------------------------------

public:

static void alarmPvConnectStateCallback (
  ProcessVariable *pv,
  void *userarg
);

static void alarmPvValueCallback (
  ProcessVariable *pv,
  void *userarg
);

static void visPvConnectStateCallback (
  ProcessVariable *pv,
  void *userarg
);

static void visPvValueCallback (
  ProcessVariable *pv,
  void *userarg
);

activeXRegTextClass ( void );

activeXRegTextClass
 ( const activeXRegTextClass *source );

~activeXRegTextClass ( void ) {

  if ( name ) delete[] name;

}

char *objName ( void ) {

  return name;

}

int createInteractive (
  activeWindowClass *aw_obj,
  int x,
  int y,
  int w,
  int h );

int save (
  FILE *f );

int old_save (
  FILE *f );

int createFromFile (
  FILE *fptr,
  char *name,
  activeWindowClass *actWin );

int old_createFromFile (
  FILE *fptr,
  char *name,
  activeWindowClass *actWin );

int importFromXchFile (
  FILE *fptr,
  char *name,
  activeWindowClass *actWin );

int genericEdit ( void );

int edit ( void );

int editCreate ( void );

int draw ( void );

int erase ( void );

int drawActive ( void );

char * getProcessedText(char *text);

int eraseActive ( void );

int eraseUnconditional ( void );

int expandTemplate (
  int numMacros,
  char *macros[],
  char *expansions[] );

int expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] );

int expand2nd (
  int numMacros,
  char *macros[],
  char *expansions[] );

int containsMacros ( void );

int activate (
  int pass,
  void *ptr );

int deactivate (
  int pass );

void updateDimensions ( void );

void executeDeferred ( void );

int setProperty (
  char *prop,
  char *value );

char *firstDragName ( void );

char *nextDragName ( void );

char *dragValue (
  int i );

void changeDisplayParams (
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

void changePvNames (
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

void updateColors (
  double colorValue );

void bufInvalidate ( void );

void getPvs (
  int max,
  ProcessVariable *pvs[],
  int *n );

char *getSearchString (
  int i
);

void replaceString (
  int i,
  int max,
  char *string
);

char *crawlerGetFirstPv ( void );

char *crawlerGetNextPv ( void );

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
