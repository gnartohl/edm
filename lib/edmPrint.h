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

#ifndef __edmPrint_h
#define __edmPrint_h 1

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <X11/Xlib.h>

#include "utility.h"
#include "entry_form.h"
#include "confirm_dialog.h"
#include "msg_dialog.h"

#include "thread.h"

class edmPrintClass; // forward declaration for the following typedef

typedef struct edmPrintThreadParamBlockTag {
  edmPrintClass *epo;
  char *cmd;
} edmPrintThreadParamBlockType, *edmPrintThreadParamBlockPtr;

#ifdef __edmPrint_cc

#include "edmPrint.str"

#ifdef __linux__
static void *printThread (
  THREAD_HANDLE h );
#endif

#ifdef __solaris__
static void *printThread (
  THREAD_HANDLE h );
#endif

#ifdef __osf__
static void printThread (
  THREAD_HANDLE h );
#endif

#ifdef HP_UX
static void *printThread (
  THREAD_HANDLE h );
#endif

#ifdef darwin
static void *printThread (
  THREAD_HANDLE h );
#endif

static void ok (
  Widget w,
  XtPointer client,
  XtPointer call );

static void apply (
  Widget w,
  XtPointer client,
  XtPointer call );

static void cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

#endif

class edmPrintClass {

public:

#ifdef __linux__
friend void *printThread (
  THREAD_HANDLE h );
#endif

#ifdef __solaris__
friend void *printThread (
  THREAD_HANDLE h );
#endif

#ifdef __osf__
friend void printThread (
  THREAD_HANDLE h );
#endif

#ifdef HP_UX
friend void *printThread (
  THREAD_HANDLE h );
#endif

#ifdef darwin
friend void *printThread (
  THREAD_HANDLE h );
#endif

friend void ok (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void apply (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

static const int SUCCESS = 1;
static const int IN_PROGRESS = 3;
static const int NOT_READY = 5;
static const int FAILURE = 100;
static const int NO_PRINT_CMD = 102;

static const int ERR_MSG_SIZE = 511;
static const int MAX_OPTION_CHARS = 1023;
static const int MAX_OPTIONS = 11;
static const int MAX_ACTIONS = 20;
static const int MAX_FIELDS = 40;

static const int MAX_LINE_SIZE = 1023;

static const int INT_TYPE = 1;
static const int STRING_TYPE = 2;

static const int OP_EQUAL = 1;
static const int OP_PLUS_EQUAL = 2;

static const int FIELD_TYPE_MENU = 1;
static const int FIELD_TYPE_TOGGLE = 2;
static const int FIELD_TYPE_TEXT = 3;

// static char * const nullString = "";

edmPrintClass ( void );

~edmPrintClass ( void );

char *errorMsg ( void );

void setErrorMsg (
  char *msg
);

void setErrorMsg (
  char *msg,
  char *arg
);

void setErrorMsg (
  char *msg,
  int arg
);

void setErrorMsg (
  char *msg,
  char *sarg,
  int iarg
);

int printStatus ( void );

int printStatusOK ( void );

int printEvent ( void );

int printFailure ( void );

int printDefFileError ( void );

int printFinished ( void );

int printCmdReady ( void );

int printDialog (
  char *displayName,
  Widget top,
  Colormap cmap,
  int x,
  int y
);

int doPrint ( void );

private:

char *getTok(
  char *buf,
  char **ctx
);

void putTkBack (
  char *tk
);

char *nextTk ( void );

int parsePrintDefinition ( void );

int openPrintDefFile ( void );

int closePrintDefFile ( void );

int numOptions; // 0 to MAX_OPTIONS
int numFields; // 0 to MAX_FIELDS

char *option[MAX_OPTIONS];
char *optionDefault[MAX_OPTIONS];

int numActions[MAX_FIELDS]; // 0 to MAX_ACTIONS-1
int optionIndex[MAX_FIELDS]; // 0 to MAX_OPTIONS-1
int optionType[MAX_FIELDS];
int fieldType[MAX_FIELDS];
int optionIntValue[MAX_FIELDS];
char optionStringValue[MAX_FIELDS][31+1];
int actionOperator[MAX_FIELDS][MAX_ACTIONS]; // 1 -> = , 2 -> +=
char *action[MAX_FIELDS][MAX_ACTIONS];
char *label[MAX_FIELDS];
char *menu[MAX_FIELDS];
int printToFile;

FILE *printDefFile;
int lineNo;
char *printCmd, *newCmd;
char *printToFileCmd;
entryFormClass ef;
int efX, efY, efW, efH, efMaxH;
confirmDialogClass confirm;
msgDialogClass msg;
int status;
char *errMsg;
char *lineBuf; // allocated with size of MAX_LINE_SIZE+1
char *lineBuf2; // allocated with size of MAX_LINE_SIZE+1
char *ctx, *ctx2; // for getTok
 int scanState; // for getTok
int tokenInBuffer;
int needFileRead;
int printInProgress;
int cmdReady;
int event;
int finished;
int printFailureFlag;
int fileDefError;
char xwinIdBuf[31+1];
edmPrintThreadParamBlockType threadBlock;
char displayName[63+1];

};

#endif
