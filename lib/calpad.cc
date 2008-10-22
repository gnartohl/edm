#define __calpad_cc

#include <time.h>
#include <sys/time.h>

#include "calpad.h"
#include "sys_types.h"

static int leap(
  int yr )
{

  if ( !( yr % 2000 ) ) return 0;

  if ( !( yr % 400 ) ) return 1;

  if ( !( yr % 100 ) ) return 0;

  if ( !( yr % 4 ) ) return 1;

  return 0;

}

static void changeTime (
  Widget w,
  XtPointer client,
  XtPointer call )
{

calpadClass *kp = (calpadClass *) client;

  //fprintf( stderr, "changeTime\n" );

  if ( w == kp->hourScale ) {
    XmScaleGetValue( w, &kp->hour );
  }
  else if ( w == kp->minScale ) {
    XmScaleGetValue( w, &kp->min );
  }
  else if ( w == kp->secScale ) {
    XmScaleGetValue( w, &kp->sec );
  }

  kp->setMonthDayYear();

}

static void decYear (
  Widget w,
  XtPointer client,
  XtPointer call )
{

calpadClass *kp = (calpadClass *) client;

  if ( kp->year > 1900 ) kp->year--;
  kp->setMonthDayYear();

}

static void incYear (
  Widget w,
  XtPointer client,
  XtPointer call )
{

calpadClass *kp = (calpadClass *) client;

  kp->year++;
  kp->setMonthDayYear();

}

static void decMon (
  Widget w,
  XtPointer client,
  XtPointer call )
{

calpadClass *kp = (calpadClass *) client;

  kp->month--;
  if ( kp->month < 0 ) {
    kp->month = 11;
    if ( kp->year > 1900 ) kp->year--;
  }
  strcpy( kp->monthString, kp->months[kp->month] );
  kp->setMonthDayYear();

}

static void incMon (
  Widget w,
  XtPointer client,
  XtPointer call )
{

calpadClass *kp = (calpadClass *) client;

  kp->month++;
  if ( kp->month > 11 ) {
    kp->month = 0;
    kp->year++;
  }
  strcpy( kp->monthString, kp->months[kp->month] );
  kp->setMonthDayYear();

}

static void calpadPress (
  Widget w,
  XtPointer client,
  XtPointer call )
{

calpadClass *kp = (calpadClass *) client;
int i;

  if ( w == kp->pbCancel ) {

    kp->popdown();
    if ( kp->cancelFunc ) {
      (*kp->cancelFunc)( w, (XtPointer) kp->userPtr, client );
    }
    XtDestroyWidget( kp->shell );
    kp->shell = NULL;

  }
  else if ( w == kp->pbOK ) {

    kp->popdown();
    if ( kp->okFunc ) {
      (*kp->okFunc)( w, (XtPointer) kp->userPtr, client );
    }
    XtDestroyWidget( kp->shell );
    kp->shell = NULL;

  }

  else {

    for ( i=0; i<42; i++ ) {

      if ( w == kp->pb[i] ) {
        kp->setDay( i+1-kp->startDay );
      }

    }

  }

}

static void charCalpadPress (
  Widget w,
  XtPointer client,
  XtPointer call )
{

  calpadPress( w, client, call );

}

calpadClass::calpadClass () {

int i;

int nd[12] = {
  31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

  for ( i=0; i<12; i++ ) {
    numDays[i] = nd[i];
  }

  for ( i=0; i<42; i++ ) {
    cb[i] = 0;
  }

  strcpy( months[0], "jan" );
  strcpy( months[1], "feb" );
  strcpy( months[2], "mar" );
  strcpy( months[3], "apr" );
  strcpy( months[4], "may" );
  strcpy( months[5], "jun" );
  strcpy( months[6], "jul" );
  strcpy( months[7], "aug" );
  strcpy( months[8], "sep" );
  strcpy( months[9], "oct" );
  strcpy( months[10], "nov" );
  strcpy( months[11], "dec" );

  strcpy( weekDayString[0], "S" );
  strcpy( weekDayString[1], "M" );
  strcpy( weekDayString[2], "T" );
  strcpy( weekDayString[3], "W" );
  strcpy( weekDayString[4], "T" );
  strcpy( weekDayString[5], "F" );
  strcpy( weekDayString[6], "S" );

  entryTag = NULL;
  actionTag = NULL;
  shell = NULL;
  poppedUp = 0;
  strcpy( monthString, "jan" );
  month = 1;
  day = 1;
  year = 2000;
  hour = 0;
  min = 0;
  sec = 0;
  nanoSec = 0;

}

calpadClass::~calpadClass ( void ) {

  if ( shell ) {
    if ( isPoppedUp() ) popdown();
    XtDestroyWidget( shell );
  }

}

void calpadClass::popup ( void ) {

  XtPopup( shell, XtGrabNone );
  poppedUp = 1;

}

void calpadClass::popdown ( void ) {

  XtPopdown( shell );
  poppedUp = 0;

}

int calpadClass::isPoppedUp ( void ) {

  return poppedUp;

}

int calpadClass::setMonthDayYear ( void ) {

int stat, i, count, numCols, d;
XmString str;
XtCallbackProc func = (XtCallbackProc) charCalpadPress;
char string[63+1];
SYS_TIME_TYPE sysTime;

  sprintf( string, "%s-01-%-d 12:00:00", monthString, year );
  stat = sys_cvt_string_to_time( string, strlen(string), &sysTime );

  d = sysTime.tm_time.tm_mday;
  month = sysTime.tm_time.tm_mon;
  weekDay = sysTime.tm_time.tm_wday;

  startDayOfWeek = ( weekDay - d + 1 ) % 7;
  lastDayOfMon = numDays[month];
  if ( month == 1 ) lastDayOfMon += leap(year);

  numCols = ( startDayOfWeek + lastDayOfMon ) / 7;
  if ( ( startDayOfWeek + lastDayOfMon ) % 7 ) numCols++;

  if ( numCols == 4 ) startDayOfWeek += 7;
  startDay = startDayOfWeek;

  count = 0;
  for ( i=0; i<42; i++ ) {

    if ( cb[i] ) {
      cb[i] = 0;
      XtRemoveCallback( pb[i], XmNactivateCallback, (XtCallbackProc) func,
      this );
    }

    if ( ( i < startDayOfWeek ) || ( count+1 >lastDayOfMon ) ) {

      str = XmStringCreateLocalized( "" );

      XtVaSetValues( pb[i],
       XmNshadowThickness, 0,
       XmNborderWidth, 0,
       XmNsensitive, False,
       XmNlabelString, str,
       NULL );

      XmStringFree( str );

    }
    else if ( count < lastDayOfMon ) {

      count++;
      sprintf( buf, "%-d", count );
      str = XmStringCreateLocalized( buf );

      if ( count == day ) {

        XtVaSetValues( pb[i],
         XmNshadowThickness, 1,
         XmNborderWidth, 1,
         XmNsensitive, True,
         XmNlabelString, str,
         NULL );

      }
      else {

        XtVaSetValues( pb[i],
         XmNshadowThickness, 0,
         XmNborderWidth, 0,
         XmNsensitive, True,
         XmNlabelString, str,
         XmNshowAsDefault, 0,
         NULL );

      }

      XmStringFree( str );

      XtAddCallback( pb[i], XmNactivateCallback, (XtCallbackProc) func, this );

      cb[i] = 1;

    }

  }

  sprintf( string, "%-d", year );
  str = XmStringCreateLocalized( string );
  XtVaSetValues( YearLabel,
  XmNlabelString, str,
  NULL );
  XmStringFree( str );

  strcpy( string, monthString );
  string[0] = toupper( string[0] );
  str = XmStringCreateLocalized( string );
  XtVaSetValues( MonLabel,
  XmNlabelString, str,
  NULL );
  XmStringFree( str );

  XtVaSetValues( hourScale,
   XmNvalue, hour,
   NULL );

  XtVaSetValues( minScale,
   XmNvalue, min,
   NULL );

  XtVaSetValues( secScale,
   XmNvalue, sec,
   NULL );

  return 1;

}

int calpadClass::setMonth (
  char *mon
) {

int stat;

  strncpy( monthString, mon, 3 );
  monthString[3] = 0;
  stat = setMonthDayYear();
  return stat;

}

int calpadClass::setMonth (
  int mon
) {

int stat;

 if ( ( mon < 1 ) || ( mon > 12 ) ) return 0;

  month = mon - 1;
  strncpy( monthString, months[month], 3 );
  monthString[3] = 0;
  stat = setMonthDayYear();
  return stat;

}

int calpadClass::setDay (
  int d
) {

int stat;

  day = d;
  stat = setMonthDayYear();
  return stat;

}

int calpadClass::setYear (
  int yr
) {

int stat;

  year = yr;
  stat = setMonthDayYear();
  return stat;

}

int calpadClass::setHour (
  int h
) {

int stat;

  hour = h;
  stat = setMonthDayYear();
  return stat;

}

int calpadClass::setMin (
  int m
) {

int stat;

  min = m;
  stat = setMonthDayYear();
  return stat;

}

int calpadClass::setSec (
  int s
) {

int stat;

  sec = s;
  stat = setMonthDayYear();
  return stat;

}

int calpadClass::setNanosec (
  int ns
) {

int stat;

  nanoSec = ns;
  stat = setMonthDayYear();
  return stat;

}

int calpadClass::setDate (
  char *str )
{

SYS_TIME_TYPE sysTime;
int stat;

  stat = sys_cvt_string_to_time( str, strlen(str), &sysTime );
  if ( !( stat & 1 ) ) {
    sys_get_time( &sysTime );
  }

  day = sysTime.tm_time.tm_mday;
  month = sysTime.tm_time.tm_mon;
  year = sysTime.tm_time.tm_year + 1900;
  hour = sysTime.tm_time.tm_hour;
  min = sysTime.tm_time.tm_min;
  sec = sysTime.tm_time.tm_sec;

  strncpy( monthString, months[month], 3 );
  monthString[3] = 0;

  stat = setMonthDayYear();

  return stat;

}

int calpadClass::create (
  Widget top,
  int _x,
  int _y,
  char *destination,
  int maxLen,
  void *_userPtr,
  XtCallbackProc _okFunc,
  XtCallbackProc _cancelFunc ) {

int stat, i, numCols, d;
XmString str;
XtCallbackProc func = 0;
SYS_TIME_TYPE sysTime;

  stat = sys_get_time( &sysTime );
  month = sysTime.tm_time.tm_mon;
  year = sysTime.tm_time.tm_year + 1900;
  weekDay = sysTime.tm_time.tm_wday;
  d = sysTime.tm_time.tm_mday;

  startDayOfWeek = ( weekDay - d + 1 ) % 7;
  lastDayOfMon = numDays[month];
  if ( month == 1 ) lastDayOfMon += leap(year);

  x = _x;
  y = _y;
  userPtr = _userPtr,
  okFunc = _okFunc;
  cancelFunc = _cancelFunc;
  display = XtDisplay( top );
  poppedUp = 0;

  charDest = (char *) destination;
  func = (XtCallbackProc) charCalpadPress;

  if ( shell ) {
    XtDestroyWidget( shell );
  }

  shell = XtVaCreatePopupShell( "calpad", xmDialogShellWidgetClass,
   top,
   XmNmappedWhenManaged, False,
   XmNmwmDecorations, 0,
   XmNx, x,
   XmNy, y,
   NULL );

  rowcol = XtVaCreateWidget( "rowcol", xmRowColumnWidgetClass, shell,
   XmNorientation, XmVERTICAL,
   XmNnumColumns, 1,
   NULL );

  topForm = XtVaCreateWidget( "topform", xmFormWidgetClass, rowcol, NULL );

  YearForm = XtVaCreateWidget( "yearform", xmFormWidgetClass, topForm,
   XmNtopAttachment, XmATTACH_FORM,
   XmNleftAttachment, XmATTACH_FORM,
   XmNrightAttachment, XmATTACH_FORM,
   NULL );

  YearDec = XtVaCreateManagedWidget(
   "yeardec", xmArrowButtonWidgetClass,
   YearForm,
   XmNtopAttachment, XmATTACH_FORM,
   XmNleftAttachment, XmATTACH_FORM,
   XmNarrowDirection, XmARROW_LEFT,
   XmNshadowThickness, 0,
   XmNborderWidth, 0,
   NULL );

  XtAddCallback( YearDec, XmNactivateCallback, (XtCallbackProc) decYear,
   this );

  YearInc = XtVaCreateManagedWidget(
   "yearinc", xmArrowButtonWidgetClass,
   YearForm,
   XmNtopAttachment, XmATTACH_FORM,
   XmNrightAttachment, XmATTACH_FORM,
   XmNarrowDirection, XmARROW_RIGHT,
   XmNshadowThickness, 0,
   XmNborderWidth, 0,
   NULL );

  XtAddCallback( YearInc, XmNactivateCallback, (XtCallbackProc) incYear,
   this );

  str = XmStringCreateLocalized( "Year" );
  YearLabel = XtVaCreateManagedWidget( "yearlabel", xmLabelWidgetClass,
   YearForm,
   XmNleftAttachment, XmATTACH_WIDGET,
   XmNleftWidget, YearDec,
   XmNrightAttachment, XmATTACH_WIDGET,
   XmNrightWidget, YearInc,
   XmNlabelString, str,
   XmNalignment, XmALIGNMENT_CENTER,
   NULL );
  XmStringFree( str );

  MonForm = XtVaCreateWidget( "monform", xmFormWidgetClass, topForm,
   XmNtopAttachment, XmATTACH_WIDGET,
   XmNtopWidget, YearForm,
   XmNleftAttachment, XmATTACH_FORM,
   XmNrightAttachment, XmATTACH_FORM,
   NULL );

  MonDec = XtVaCreateManagedWidget(
   "mondec", xmArrowButtonWidgetClass,
   MonForm,
   XmNtopAttachment, XmATTACH_FORM,
   XmNleftAttachment, XmATTACH_FORM,
   XmNarrowDirection, XmARROW_LEFT,
   XmNshadowThickness, 0,
   XmNborderWidth, 0,
   NULL );

  XtAddCallback( MonDec, XmNactivateCallback, (XtCallbackProc) decMon,
   this );

  MonInc = XtVaCreateManagedWidget(
   "moninc", xmArrowButtonWidgetClass,
   MonForm,
   XmNtopAttachment, XmATTACH_FORM,
   XmNrightAttachment, XmATTACH_FORM,
   XmNarrowDirection, XmARROW_RIGHT,
   XmNshadowThickness, 0,
   XmNborderWidth, 0,
   NULL );

  XtAddCallback( MonInc, XmNactivateCallback, (XtCallbackProc) incMon,
   this );

  str = XmStringCreateLocalized( "Month" );
  MonLabel = XtVaCreateManagedWidget( "monlabel", xmLabelWidgetClass,
   MonForm,
   XmNleftAttachment, XmATTACH_WIDGET,
   XmNleftWidget, MonDec,
   XmNrightAttachment, XmATTACH_WIDGET,
   XmNrightWidget, MonInc,
   XmNlabelString, str,
   XmNalignment, XmALIGNMENT_CENTER,
   NULL );
  XmStringFree( str );

  sep1 = XtVaCreateManagedWidget( "sep1", xmSeparatorWidgetClass, topForm,
   XmNtopAttachment, XmATTACH_WIDGET,
   XmNtopWidget, MonForm,
   XmNleftAttachment, XmATTACH_FORM,
   XmNrightAttachment, XmATTACH_FORM,
   NULL );

  numCols = ( startDayOfWeek + lastDayOfMon ) / 7;
  if ( ( startDayOfWeek + lastDayOfMon ) % 7 ) numCols++;

  if ( numCols == 4 ) startDayOfWeek += 7;
  startDay = startDayOfWeek;
  numCols = 6;

  kprowcol = XtVaCreateWidget( "kprowcol", xmRowColumnWidgetClass, rowcol,
   XmNorientation, XmHORIZONTAL,
   XmNnumColumns, numCols+1,
   XmNpacking, XmPACK_COLUMN,
   NULL );

  bottomForm = XtVaCreateWidget( "bottomform", xmFormWidgetClass, rowcol, NULL );

  scaleForm = XtVaCreateWidget( "scaleform", xmFormWidgetClass, bottomForm,
   XmNtopAttachment, XmATTACH_FORM,
   XmNleftAttachment, XmATTACH_FORM,
   XmNrightAttachment, XmATTACH_FORM,
   NULL );

  buttonForm = XtVaCreateWidget( "buttonform", xmFormWidgetClass, bottomForm,
   XmNtopAttachment, XmATTACH_WIDGET,
   XmNtopWidget, scaleForm,
   XmNleftAttachment, XmATTACH_FORM,
   XmNrightAttachment, XmATTACH_FORM,
   XmNbottomAttachment, XmATTACH_FORM,
   NULL );

// ---------------------------------------

  for ( i=0; i<7; i++ ) {

    str = XmStringCreateLocalized( weekDayString[i] );
    weekDayPb[i] = XtVaCreateManagedWidget(
    "pb", xmLabelWidgetClass,
     kprowcol,
     XmNlabelString, str,
     NULL );
    XmStringFree( str );

  }

  count = 0;
  for ( i=0; i<42; i++ ) {

    if ( ( i < startDayOfWeek ) || ( count+1 >lastDayOfMon ) ) {

      pb[i] = XtVaCreateManagedWidget(
       "pb", xmPushButtonWidgetClass,
       kprowcol,
       XmNshadowThickness, 0,
       XmNborderWidth, 0,
       XmNsensitive, False,
       NULL );

    }
    else {

      count++;
      sprintf( buf, "%-d", count );
      str = XmStringCreateLocalized( buf );

      pb[i] = XtVaCreateManagedWidget(
       "pb", xmPushButtonWidgetClass,
       kprowcol,
       XmNshadowThickness, 0,
       XmNborderWidth, 0,
       XmNsensitive, False,
       XmNlabelString, str,
       NULL );

      XmStringFree( str );

    }

    cb[i] = 0;

  }

// ---------------------------------------

  sep2 = XtVaCreateManagedWidget( "sep2", xmSeparatorWidgetClass, scaleForm,
   XmNtopAttachment, XmATTACH_FORM,
   XmNleftAttachment, XmATTACH_FORM,
   XmNrightAttachment, XmATTACH_FORM,
   NULL );

  str = XmStringCreateLocalized( "Hour" );
  hourScale = XtVaCreateManagedWidget(
   "hourscale", xmScaleWidgetClass,
   scaleForm,
   XmNtopAttachment, XmATTACH_WIDGET,
   XmNtopWidget, sep2,
   XmNleftAttachment, XmATTACH_FORM,
   XmNrightAttachment, XmATTACH_FORM,
   XmNminimum, 0,
   XmNmaximum, 23,
   XmNorientation, XmHORIZONTAL,
   XmNshowValue, True,
   XmNvalue, 0,
   //XmNtitleString, str,
   NULL );
  XmStringFree( str );

  XtAddCallback( hourScale, XmNvalueChangedCallback,
   (XtCallbackProc) changeTime, this );

  str = XmStringCreateLocalized( "Min" );
  minScale = XtVaCreateManagedWidget(
   "minscale", xmScaleWidgetClass,
   scaleForm,
   XmNtopAttachment, XmATTACH_WIDGET,
   XmNtopWidget, hourScale,
   XmNleftAttachment, XmATTACH_FORM,
   XmNrightAttachment, XmATTACH_FORM,
   XmNminimum, 0,
   XmNmaximum, 59,
   XmNorientation, XmHORIZONTAL,
   XmNshowValue, True,
   XmNvalue, 0,
   //XmNtitleString, str,
   NULL );
  XmStringFree( str );

  XtAddCallback( minScale, XmNvalueChangedCallback,
   (XtCallbackProc) changeTime, this );

  str = XmStringCreateLocalized( "Hour Min Sec" );
  secScale = XtVaCreateManagedWidget(
   "secscale", xmScaleWidgetClass,
   scaleForm,
   XmNtopAttachment, XmATTACH_WIDGET,
   XmNtopWidget, minScale,
   XmNleftAttachment, XmATTACH_FORM,
   XmNrightAttachment, XmATTACH_FORM,
   XmNminimum, 0,
   XmNmaximum, 59,
   XmNorientation, XmHORIZONTAL,
   XmNshowValue, True,
   XmNvalue, 0,
   XmNtitleString, str,
   NULL );
  XmStringFree( str );

  XtAddCallback( secScale, XmNvalueChangedCallback,
   (XtCallbackProc) changeTime, this );

  sep3 = XtVaCreateManagedWidget( "sep3", xmSeparatorWidgetClass, scaleForm,
   XmNtopAttachment, XmATTACH_WIDGET,
   XmNtopWidget, secScale,
   XmNleftAttachment, XmATTACH_FORM,
   XmNrightAttachment, XmATTACH_FORM,
   NULL );

  if ( entryTag )
    str = XmStringCreate( "Cancel", entryTag );
  else
    str = XmStringCreateLocalized( "Cancel" );

  pbCancel = XtVaCreateManagedWidget(
   "pbcancel", xmPushButtonWidgetClass,
   buttonForm,
   XmNlabelString, str,
   XmNbottomAttachment, XmATTACH_FORM,
   XmNleftAttachment, XmATTACH_FORM,
   NULL );

  XmStringFree( str );

  XtAddCallback( pbCancel, XmNactivateCallback, (XtCallbackProc) func, this );

// ---------------------------------------



  if ( entryTag )
    str = XmStringCreate( "OK", entryTag );
  else
    str = XmStringCreateLocalized( "OK" );

  pbOK = XtVaCreateManagedWidget(
   "pbok", xmPushButtonWidgetClass,
   buttonForm,
   XmNlabelString, str,
   XmNbottomAttachment, XmATTACH_FORM,
   XmNrightAttachment, XmATTACH_FORM,
   NULL );

  XmStringFree( str );

  XtAddCallback( pbOK, XmNactivateCallback, (XtCallbackProc) func, this );

// ---------------------------------------

  XtManageChild( topForm );
  XtManageChild( YearForm );
  XtManageChild( MonForm );
  XtManageChild( kprowcol );
  XtManageChild( scaleForm );
  XtManageChild( buttonForm );
  XtManageChild( bottomForm );
  XtManageChild( rowcol );
  XtRealizeWidget( shell );
  popup();

  return 1;

}

char *calpadClass::getDate (
  char *dateString,
  int maxLen )
{

char buf[127+1];

  sprintf( buf, "%s-%02d-%-d %02d:%02d:%02d", monthString, day, year,
   hour, min, sec );
  strncpy( dateString, buf, maxLen );
  dateString[maxLen] = 0;

  return dateString;

}

#if 0

static void cancel (
  Widget w,
  XtPointer client,
  XtPointer call )
{

  fprintf( stderr, "cancel\n" );

}

static void ok (
  Widget w,
  XtPointer client,
  XtPointer call )
{

char str[127+1];
calpadClass *kp = (calpadClass *) client;

  fprintf( stderr, "ok, date = [%s]\n", kp->getDate( str, 127 ) );

}

main() {

XtAppContext app;
Display *display;
Widget appTop, mainWin;
calpadClass kp, kp1;
char dest[127+1];
char dest1[127+1];
XEvent Xev;
int i, result, isXEvent, argc, stat;

  argc = 0;
  appTop = XtVaAppInitialize( &app, "edm", NULL, 0, &argc,
   NULL, NULL, XmNiconic, False, NULL );

  mainWin = XtVaCreateManagedWidget( "", xmMainWindowWidgetClass,
   appTop,
   XmNwidth, 100,
   XmNheight, 50,
   XmNscrollBarDisplayPolicy, XmAS_NEEDED,
   XmNscrollingPolicy, XmAUTOMATIC,
   NULL );

  XtRealizeWidget( appTop );

  display = XtDisplay( appTop );

  kp.create( appTop, 100, 100, dest, 127, (void *) &kp, ok, cancel );
  kp.setDate( "feb-13-2000 1:55:39" );

  kp1.create( appTop, 300, 200, dest1, 127, (void *) &kp1, ok, cancel );
  kp1.setDate( "dec-25-2001 12:10:05" );

  while ( 1 ) {

  do {

    result = XtAppPending( app );
    if ( result ) {
      isXEvent = XtAppPeekEvent( app, &Xev );
      if ( isXEvent ) {
        if ( Xev.type != Expose ) {
          XtAppProcessEvent( app, result );
	}
        else {
          XtAppProcessEvent( app, result );
	}
      }
      else { // process all timer or alternate events
        XtAppProcessEvent( app, result );
      }
    }
  } while ( result );

  }

}
#endif
