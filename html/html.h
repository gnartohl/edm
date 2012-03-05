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

#ifndef __html_h
#define __html_h 1

#include <Xm/XmHTML.h>
#include <Xm/BulletinB.h>

#include <sys/stat.h>
#include <unistd.h>

#include "act_grf.h"
#include "entry_form.h"
#include "pv_factory.h"
#include "cvtFast.h"

#define HTMLC_K_COLORMODE_STATIC 0
#define HTMLC_K_COLORMODE_ALARM 1

#define HTMLC_MAJOR_VERSION 4
#define HTMLC_MINOR_VERSION 0
#define HTMLC_RELEASE 1

#ifdef __html_cc

#include "html.str"

static char *dragName[] = {
  htmlClass_str2,
  htmlClass_str3,
  htmlClass_str25
};

static void unconnectedTimeout (
  XtPointer client,
  XtIntervalId *id );

static void htmlc_anchor (
  Widget w,
  XtPointer client,
  XmHTMLAnchorCallbackStruct *cbs );

static void htmlc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

static void htmlc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

static void htmlc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

static void htmlc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

static void htmlc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

#endif

class htmlClass : public activeGraphicClass {

private:

static const int MAX_TEXT_LEN = 4000;
static const int MAX_FILENAME_LEN = 255;

friend void unconnectedTimeout (
  XtPointer client,
  XtIntervalId *id );

friend void htmlc_anchor (
  Widget w,
  XtPointer client,
  XmHTMLAnchorCallbackStruct *cbs );

friend void htmlc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void htmlc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void htmlc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void htmlc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void htmlc_edit_cancel_delete (
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
  int bufBgColor;
  int bufVisInverted;
  int bufUseFile;
  colorButtonClass fgCb;
  colorButtonClass bgCb;
  char bufMinVisString[39+1];
  char bufMaxVisString[39+1];
  char bufAlarmPvName[PV_Factory::MAX_PV_NAME+1];
  char bufVisPvName[PV_Factory::MAX_PV_NAME+1];
  char bufContentPvName[PV_Factory::MAX_PV_NAME+1];
} editBufType, *editBufPtr;

editBufPtr eBuf;

pvColorClass fgColor;

pvColorClass bgColor;

int pvType;
pvValType pvValue, minVis, maxVis;
char minVisString[39+1];
char maxVisString[39+1];

int prevVisibility, visibility, visInverted;
int fgVisibility, prevFgVisibility;
int bgVisibility, prevBgVisibility;

ProcessVariable *alarmPvId;
ProcessVariable *visPvId;
ProcessVariable *contentPvId;

expStringClass alarmPvExpStr;

expStringClass visPvExpStr;

expStringClass contentPvExpStr;

static const int contentSize = 39;
expStringClass contentStringExpStr;

int alarmPvExists, visPvExists, contentPvExists;
int activeMode, init, opComplete;

expStringClass value, docRoot;
char *bufValue, *fileContents, *hrefFileName, *bufDocRoot;

int useFile;

int needConnectInit, needAlarmUpdate, needVisUpdate, needRefresh,
 needPropertyUpdate, needToDrawUnconnected, needToEraseUnconnected,
 needLink, needOpenFile, needContentUpdate;
XtIntervalId unconnectedTimer;

int curFgColorIndex, curBgColorIndex, curStatus, curSeverity;
static const int alarmPvConnection = 1;
static const int visPvConnection = 2;
static const int contentPvConnection = 3;
pvConnectionClass connection;

int stringLength;

int widgetsCreated, widgetsMapped;
Widget bulBrd, htmlBox;

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

static void contentPvConnectStateCallback (
  ProcessVariable *pv,
  void *userarg
);

static void contentPvValueCallback (
  ProcessVariable *pv,
  void *userarg
);

htmlClass ( void );

htmlClass
 ( const htmlClass *source );

~htmlClass ( void );

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

void loadFile (
  char *name
);

void map ( void );

void unmap ( void );

void getPvs (
  int max,
  ProcessVariable *pvs[],
  int *n );

};

#ifdef __cplusplus
extern "C" {
#endif

void *create_htmlPtr ( void );
void *clone_htmlPtr ( void * );

#ifdef __cplusplus
}
#endif

#endif
