#ifndef __fileSelect_h
#define __fileSelect_h 1

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <X11/Xlib.h>
#include <Xm/Xm.h>
#include <Xm/MainW.h>
#include <Xm/FileSB.h>
#include <Xm/AtomMgr.h>
#include <Xm/Protocols.h>

#include "utility.h"

#ifdef __fileSelect_cc

static void fselectOk (
  Widget w,
  XtPointer client,
  XtPointer call );

static void fselectCancel (
  Widget w,
  XtPointer client,
  XtPointer call );

#endif

class fselectClass {

private:

friend void fselectOk (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void fselectCancel (
  Widget w,
  XtPointer client,
  XtPointer call );

int x, y, poppedUp;
char *entryTag, *actionTag, selection[255+1];
Display *display;
Widget fs;

void *userPtr;
void (*okFunc)(Widget,XtPointer,XtPointer);
void (*cancelFunc)(Widget,XtPointer,XtPointer);

public:

fselectClass ();

~fselectClass ( void );

void popup ( void );

void popdown ( void );

int isPoppedUp ( void );

int setDefDir (
  char *str );

int setPattern (
  char *str );

int create (
  Widget top,
  int _x,
  int _y,
  char *_defDir,
  char *_pattern,
  void *_userPtr,
  XtCallbackProc _okFunc,
  XtCallbackProc _cancelFunc );

char *getSelection (
  char *str,
  int maxLen );

};

#endif
