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

#ifndef __shell_cmd_h
#define __shell_cmd_h 1

#include "act_grf.h"
#include "entry_form.h"

#define SHCMDC_MAJOR_VERSION 4
#define SHCMDC_MINOR_VERSION 3
#define SHCMDC_RELEASE 0

#ifdef __shell_cmd_cc

#include "shell_cmd.str"

#ifdef __linux__
static void *shellCmdThread (
  THREAD_HANDLE h );
#endif

#ifdef darwin
static void *shellCmdThread (
  THREAD_HANDLE h );
#endif

#ifdef __solaris__
static void *shellCmdThread (
  THREAD_HANDLE h );
#endif

#ifdef __osf__
static void shellCmdThread (
  THREAD_HANDLE h );
#endif

#ifdef HP_UX
static void *shellCmdThread (
  THREAD_HANDLE h );
#endif

static void menu_cb (
  Widget w,
  XtPointer client,
  XtPointer call );

static void shcmdc_executeCmd (
  XtPointer client,
  XtIntervalId *id );

static void pw_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

static void pw_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

static void pw_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

static void shcmdc_edit_ok1 (
  Widget w,
  XtPointer client,
  XtPointer call );

static void shcmdc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

static void shcmdc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

static void shcmdc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

static void shcmdc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

static void shcmdc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

#endif

class shellCmdClass : public activeGraphicClass {

public:

static const int maxCmds = 20;

private:

#ifdef __linux__
friend void *shellCmdThread (
  THREAD_HANDLE h );
#endif

#ifdef darwin
	friend void *shellCmdThread (
  THREAD_HANDLE h );
#endif

#ifdef __solaris__
friend void *shellCmdThread (
  THREAD_HANDLE h );
#endif

#ifdef __osf__
friend void shellCmdThread (
  THREAD_HANDLE h );
#endif

#ifdef HP_UX
friend void *shellCmdThread (
  THREAD_HANDLE h );
#endif

friend void menu_cb (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void shcmdc_executeCmd (
  XtPointer client,
  XtIntervalId *id );

friend void pw_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void pw_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void pw_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void shcmdc_edit_ok1 (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void shcmdc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void shcmdc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void shcmdc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void shcmdc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void shcmdc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

typedef struct bufTag {
  int bufX;
  int bufY;
  int bufW;
  int bufH;
  int bufTopShadowColor;
  int bufBotShadowColor;
  int bufFgColor;
  int bufBgColor;
  int bufInvisible;
  int bufCloseAction;
  char bufShellCommand[maxCmds][2550+1];
  char bufLabel[maxCmds][127+1];
  char bufButtonLabel[127+1];
  char bufFontTag[63+1];;
  int bufLock;
  double bufThreadSecondsToDelay;
  double bufAutoExecInterval;
  int bufMultipleInstancesAllowed;
  char bufRequiredHostName[63+1];
  int bufOneShot;
  int bufSwapButtons;
  int bufIncludeHelpIcon;
  int bufExecCursor;
} bufType, *bufPtr;

// static char * const nullHost = "";

bufPtr buf;

char bufPw1[31+1];
char bufPw2[31+1];

int topShadowColor, botShadowColor;
pvColorClass fgColor, bgColor;
colorButtonClass fgCb, bgCb, topShadowCb, botShadowCb;
int invisible;

int closeAction;

fontMenuClass fm;
char fontTag[63+1];
XmFontList fontList;
XFontStruct *fs;
int fontAscent, fontDescent, fontHeight;

int cmdIndex;
int numCmds;
expStringClass shellCommand[maxCmds];
expStringClass label[maxCmds];
expStringClass buttonLabel;

char pw[31+1];
int usePassword;
int lock;

int activeMode;
int opComplete;

double threadSecondsToDelay, autoExecInterval;
XtIntervalId timer;
int oneShot, timerActive, timerValue, multipleInstancesAllowed,
 swapButtons, includeHelpIcon, execCursor;
THREAD_HANDLE thread;

int pwFormX, pwFormY, pwFormW, pwFormH, pwFormMaxH;

int needExecute, needWarning;

Widget popUpMenu, pullDownMenu, pb[maxCmds];

entryFormClass *ef1;

char *hostName;
char requiredHostName[15+1];

public:

shellCmdClass ( void );

shellCmdClass
 ( const shellCmdClass *source );

~shellCmdClass ( void );

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
  FILE *f,
  char *name,
  activeWindowClass *_actWin );

int genericEdit ( void );

int edit ( void );

int editCreate ( void );

int draw ( void );

int erase ( void );

int drawActive ( void );

int eraseActive ( void );

int activate (
  int pass,
  void *ptr );

int deactivate ( int pass );

void updateDimensions ( void );

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

void btnUp (
  XButtonEvent *be,
  int x,
  int y,
  int buttonState,
  int buttonNumber,
  int *action );

void executeCmd ( void );

void btnDown (
  XButtonEvent *be,
  int x,
  int y,
  int buttonState,
  int buttonNumber,
  int *action );

void pointerIn (
  XMotionEvent *me,
  int _x,
  int _y,
  int buttonState );

int getButtonActionRequest (
  int *up,
  int *down,
  int *drag,
  int *focus );

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

void executeDeferred ( void );

char *getSearchString (
  int i
);

void replaceString (
  int i,
  int max,
  char *string
);

};

#ifdef __cplusplus
extern "C" {
#endif

void *create_shellCmdClassPtr ( void );
void *clone_shellCmdClassPtr ( void * );

#ifdef __cplusplus
}
#endif

#endif
