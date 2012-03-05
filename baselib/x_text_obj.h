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
#include "cvtFast.h"

#define AXTC_K_COLORMODE_STATIC 0
#define AXTC_K_COLORMODE_ALARM 1

#define AXTC_MAJOR_VERSION 4
#define AXTC_MINOR_VERSION 1
#define AXTC_RELEASE 1

#ifdef __x_text_obj_cc

#include "x_text_obj.str"

static char *dragName[] = {
  activeXTextClass_str2,
  activeXTextClass_str3
};

static void doBlink (
  void *ptr
);

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

static const int MAX_TEXT_LEN = 4000;

friend void doBlink (
  void *ptr
);

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

typedef struct editBufTag {
// edit buffer
  int bufX;
  int bufY;
  int bufW;
  int bufH;
  int bufFgColor;
  colorButtonClass fgCb;
  int bufFgColorMode;
  int bufBgColor;
  colorButtonClass bgCb;
  int bufBgColorMode;
  char bufMinVisString[39+1];
  char bufMaxVisString[39+1];
  int bufVisInverted;
  char bufAlarmPvName[PV_Factory::MAX_PV_NAME+1];
  char bufVisPvName[PV_Factory::MAX_PV_NAME+1];
  char bufFontTag[63+1];
  int bufUseDisplayBg;
  int bufAutoSize;
  int bufBorder;
  int bufLineThk;
} editBufType, *editBufPtr;

editBufPtr eBuf;

entryListBase *invisPvEntry, *visInvEntry, *minVisEntry, *maxVisEntry;

entryListBase *fillEntry, *fillColorEntry, *fillAlarmSensEntry;

pvColorClass fgColor;
int fgColorMode;

pvColorClass bgColor;
int bgColorMode;

int pvType;
pvValType pvValue, minVis, maxVis;
char minVisString[39+1];
char maxVisString[39+1];

int prevVisibility, visibility, visInverted;
int fgVisibility, prevFgVisibility;
int bgVisibility, prevBgVisibility;

ProcessVariable *alarmPvId;
ProcessVariable *visPvId;

expStringClass alarmPvExpStr;

expStringClass visPvExpStr;

int alarmPvExists, visPvExists;
int activeMode, init, opComplete;

expStringClass value;
char *bufValue;

fontMenuClass fm;
char fontTag[63+1];
int useDisplayBg, alignment;
XFontStruct *fs;
int fontAscent, fontDescent, fontHeight, stringLength, stringWidth,
 stringY, stringX, stringBoxWidth, stringBoxHeight;
int autoSize;
int border;
int lineThk;

int needConnectInit, needAlarmUpdate, needVisUpdate, needRefresh,
 needPropertyUpdate, needToDrawUnconnected, needToEraseUnconnected;
XtIntervalId unconnectedTimer;

int curFgColorIndex, curBgColorIndex, curStatus, curSeverity;
static const int alarmPvConnection = 1;
static const int visPvConnection = 2;
pvConnectionClass connection;

int bufInvalid;

int savedX, savedW, savedH, savedDims;

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

activeXTextClass ( void );

activeXTextClass
 ( const activeXTextClass *source );

~activeXTextClass ( void );

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

int createFromFile (
  FILE *fptr,
  char *name,
  activeWindowClass *actWin );

int old_createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin );

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

void *create_activeXTextClassPtr ( void );
void *clone_activeXTextClassPtr ( void * );

#ifdef __cplusplus
}
#endif

#endif
