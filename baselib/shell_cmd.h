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

#define SHCMDC_MAJOR_VERSION 2
#define SHCMDC_MINOR_VERSION 3
#define SHCMDC_RELEASE 0

#ifdef __shell_cmd_cc

#include "shell_cmd.str"

#ifdef __linux__
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

private:

#ifdef __linux__
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

int bufX, bufY, bufW, bufH;

int topShadowColor, bufTopShadowColor;
int botShadowColor, bufBotShadowColor;
int bufFgColor, bufBgColor;
pvColorClass fgColor, bgColor;
colorButtonClass fgCb, bgCb, topShadowCb, botShadowCb;
int invisible, bufInvisible;
int closeAction, bufCloseAction;

fontMenuClass fm;
char fontTag[63+1], bufFontTag[63+1];
XmFontList fontList;
XFontStruct *fs;
int fontAscent, fontDescent, fontHeight;

char bufShellCommand[127+1];
expStringClass shellCommand;

char bufLabel[127+1];
expStringClass label;
//char label[127+1];

char pw[31+1];
char bufPw1[31+1];
char bufPw2[31+1];
int usePassword;

int lock, bufLock;

int activeMode;

double threadSecondsToDelay, bufThreadSecondsToDelay;
double autoExecInterval, bufAutoExecInterval;
XtIntervalId timer;
int timerActive, timerValue;
int multipleInstancesAllowed, bufMultipleInstancesAllowed;
THREAD_HANDLE thread;

int pwFormX, pwFormY, pwFormW, pwFormH, pwFormMaxH;

int needExecute, needWarning;

public:

shellCmdClass::shellCmdClass ( void );

shellCmdClass::shellCmdClass
 ( const shellCmdClass *source );

shellCmdClass::~shellCmdClass ( void );

char *shellCmdClass::objName ( void ) {

  return name;

}

int shellCmdClass::createInteractive (
  activeWindowClass *aw_obj,
  int x,
  int y,
  int w,
  int h );

int shellCmdClass::save (
  FILE *f );

int shellCmdClass::createFromFile (
  FILE *fptr,
  char *name,
  activeWindowClass *actWin );

int shellCmdClass::importFromXchFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin );

int shellCmdClass::genericEdit ( void );

int shellCmdClass::edit ( void );

int shellCmdClass::editCreate ( void );

int shellCmdClass::draw ( void );

int shellCmdClass::erase ( void );

int shellCmdClass::drawActive ( void );

int shellCmdClass::eraseActive ( void );

int shellCmdClass::activate (
  int pass,
  void *ptr );

int shellCmdClass::deactivate ( int pass );

void shellCmdClass::updateDimensions ( void );

int shellCmdClass::expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] );

int shellCmdClass::expand2nd (
  int numMacros,
  char *macros[],
  char *expansions[] );

int shellCmdClass::containsMacros ( void );

void shellCmdClass::btnUp (
  int x,
  int y,
  int buttonState,
  int buttonNumber,
  int *action );

void shellCmdClass::executeCmd ( void );

void shellCmdClass::btnDown (
  int x,
  int y,
  int buttonState,
  int buttonNumber,
  int *action );

int shellCmdClass::getButtonActionRequest (
  int *up,
  int *down,
  int *drag,
  int *focus );

void shellCmdClass::changeDisplayParams (
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

void shellCmdClass::executeDeferred ( void );


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
