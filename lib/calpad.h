#ifndef __calpad_h
#define __calpad_h 1

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <X11/Xlib.h>
#include <Xm/Xm.h>
#include <Xm/MainW.h>
#include <Xm/BulletinB.h>
#include <Xm/DrawingA.h>
#include <Xm/PushBG.h>
#include <Xm/PushB.h>
#include <Xm/ToggleB.h>
#include <Xm/ArrowBG.h>
#include <Xm/Label.h>
#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/RowColumn.h>
#include <Xm/DialogS.h>
#include <Xm/ScrolledW.h>
#include <Xm/PanedW.h>
#include <Xm/TextF.h>
#include <Xm/Text.h>
#include <Xm/Separator.h>
#include <Xm/Scale.h>
#include <Xm/ArrowB.h>

#include "utility.h"

typedef struct calpadWidgetListTag {
  struct widgetListTag *flink;
  Widget pb;
} calpadWidgetListType, *calpadWidgetListPtr;

#ifdef __calpad_cc

static void changeTime (
  Widget w,
  XtPointer client,
  XtPointer call );

static void decYear (
  Widget w,
  XtPointer client,
  XtPointer call );

static void incYear (
  Widget w,
  XtPointer client,
  XtPointer call );

static void decMon (
  Widget w,
  XtPointer client,
  XtPointer call );

static void incMon (
  Widget w,
  XtPointer client,
  XtPointer call );

static void calpadPress (
  Widget w,
  XtPointer client,
  XtPointer call );

static void charCalpadPress (
  Widget w,
  XtPointer client,
  XtPointer call );

#endif

class calpadClass {

private:

friend void changeTime (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void decYear (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void incYear (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void decMon (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void incMon (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void calpadPress (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void charCalpadPress (
  Widget w,
  XtPointer client,
  XtPointer call );

static const int MAXCHARS = 14;

int x, y, count, poppedUp, startDay, cb[42];
char *entryTag, *actionTag, months[12][3+1], weekDayString[7][1+1];
Display *display;
Widget shell, rowcol, kprowcol, topForm, mainForm, bottomForm,
 YearForm, MonForm, YearDec, YearInc, MonDec, MonInc,
 YearLabel, MonLabel, sep1, sep2, sep3, pb[42], weekDayPb[7],
 hourScale, minScale, secScale,
 scaleForm, buttonForm,
 pbOK, pbApply, pbCancel;
calpadWidgetListPtr wlHead, wlTail;
char buf[MAXCHARS+1];

char *charDest;

void *userPtr;
void (*okFunc)(Widget,XtPointer,XtPointer);
void (*cancelFunc)(Widget,XtPointer,XtPointer);

int month, year, weekDay, day, startDayOfWeek, lastDayOfMon;
int hour, min, sec, nanoSec;
char monthString[3+1];
int numDays[12];

public:

calpadClass ();

~calpadClass ( void );

void popup ( void );

void popdown ( void );

int isPoppedUp ( void );

int setMonthDayYear ( void );

int setDay (
  int day );

int setMonth (
  char *mon );

int setMonth (
  int mon );

int setYear (
  int yr );

int setHour (
  int h );

int setMin (
  int m );

int setSec (
  int s );

int setNanosec (
  int ns );

int setDate (
  char *str );

int create (
  Widget top,
  int _x,
  int _y,
  char *destination,
  int maxLen,
  void *_userPtr,
  XtCallbackProc _okFunc,
  XtCallbackProc _cancelFunc );

char *getDate (
  char *dateString,
  int maxLen );

};

#endif




























