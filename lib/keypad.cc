#define __keypad_cc

#include "keypad.h"

static void keypadPress (
  Widget w,
  XtPointer client,
  XtPointer call,
  int dataType )
{

keypadClass *kp = (keypadClass *) client;

  if ( w == kp->pbCancel ) {

    kp->popdown();
    if ( kp->cancelFunc ) {
      (*kp->cancelFunc)( w, (XtPointer) kp->userPtr, client );
    }
    XtDestroyWidget( kp->shell );
    kp->shell = NULL;

  }
  else if ( w == kp->pbOK ) {

    if ( strcmp( &kp->buf[1], "." ) == 0 ) { // only single "." was entered

      kp->popdown();
      if ( kp->cancelFunc ) {
        (*kp->cancelFunc)( w, (XtPointer) kp->userPtr, client );
      }
      XtDestroyWidget( kp->shell );
      kp->shell = NULL;

      return;

    }

    if ( kp->state == keypadClass::ISNULL ) {

      kp->popdown();
      if ( kp->cancelFunc ) {
        (*kp->cancelFunc)( w, (XtPointer) kp->userPtr, client );
      }
      XtDestroyWidget( kp->shell );
      kp->shell = NULL;

      return;

    }

    switch ( dataType ) {

    case keypadClass::INT:

      if ( kp->positive ) {
        *(kp->intDest) = atol( &kp->buf[1] );
      }
      else {
        *(kp->intDest) = atol( kp->buf );
      }
      break;

    case keypadClass::DOUBLE:

      if ( kp->positive ) {
        *(kp->doubleDest) = atof( &kp->buf[1] );
      }
      else {
        *(kp->doubleDest) = atof( kp->buf );
      }
      break;

    }

    kp->popdown();
    if ( kp->okFunc ) {
      (*kp->okFunc)( w, (XtPointer) kp->userPtr, client );
    }
    XtDestroyWidget( kp->shell );
    kp->shell = NULL;

  }

  else {

    switch ( kp->state ) {

    case keypadClass::ISNULL:

      if ( w == kp->pb1 ) {
        strcpy( kp->buf, "-1" );
        kp->state = keypadClass::NODECPOINT;
        kp->count++;
      }
      else if ( w == kp->pb2 ) {
        strcpy( kp->buf, "-2" );
        kp->state = keypadClass::NODECPOINT;
        kp->count++;
      }
      else if ( w == kp->pb3 ) {
        strcpy( kp->buf, "-3" );
        kp->state = keypadClass::NODECPOINT;
        kp->count++;
      }
      else if ( w == kp->pb4 ) {
        strcpy( kp->buf, "-4" );
        kp->state = keypadClass::NODECPOINT;
        kp->count++;
      }
      else if ( w == kp->pb5 ) {
        strcpy( kp->buf, "-5" );
        kp->state = keypadClass::NODECPOINT;
        kp->count++;
      }
      else if ( w == kp->pb6 ) {
        strcpy( kp->buf, "-6" );
        kp->state = keypadClass::NODECPOINT;
        kp->count++;
      }
      else if ( w == kp->pb7 ) {
        strcpy( kp->buf, "-7" );
        kp->state = keypadClass::NODECPOINT;
        kp->count++;
      }
      else if ( w == kp->pb8 ) {
        strcpy( kp->buf, "-8" );
        kp->state = keypadClass::NODECPOINT;
        kp->count++;
      }
      else if ( w == kp->pb9 ) {
        strcpy( kp->buf, "-9" );
        kp->state = keypadClass::NODECPOINT;
        kp->count++;
      }
      else if ( w == kp->pb0 ) {
        strcpy( kp->buf, "-0" );
        kp->state = keypadClass::ZERO;
        kp->count = 0;
      }
      else if ( w == kp->pbPoint ) {
        strcpy( kp->buf, "-." );
        kp->state = keypadClass::DECPOINT;
        kp->count++;
      }

      kp->positive = 1;

      break;

    case keypadClass::ZERO:

      if ( w == kp->pb1 ) {
        strcpy( kp->buf, "-1" );
        kp->state = keypadClass::NODECPOINT;
        kp->count++;
      }
      else if ( w == kp->pb2 ) {
        strcpy( kp->buf, "-2" );
        kp->state = keypadClass::NODECPOINT;
        kp->count++;
      }
      else if ( w == kp->pb3 ) {
        strcpy( kp->buf, "-3" );
        kp->state = keypadClass::NODECPOINT;
        kp->count++;
      }
      else if ( w == kp->pb4 ) {
        strcpy( kp->buf, "-4" );
        kp->state = keypadClass::NODECPOINT;
        kp->count++;
      }
      else if ( w == kp->pb5 ) {
        strcpy( kp->buf, "-5" );
        kp->state = keypadClass::NODECPOINT;
        kp->count++;
      }
      else if ( w == kp->pb6 ) {
        strcpy( kp->buf, "-6" );
        kp->state = keypadClass::NODECPOINT;
        kp->count++;
      }
      else if ( w == kp->pb7 ) {
        strcpy( kp->buf, "-7" );
        kp->state = keypadClass::NODECPOINT;
        kp->count++;
      }
      else if ( w == kp->pb8 ) {
        strcpy( kp->buf, "-8" );
        kp->state = keypadClass::NODECPOINT;
        kp->count++;
      }
      else if ( w == kp->pb9 ) {
        strcpy( kp->buf, "-9" );
        kp->state = keypadClass::NODECPOINT;
        kp->count++;
      }
      else if ( w == kp->pbPoint ) {
        strcpy( kp->buf, "-." );
        kp->state = keypadClass::DECPOINT;
        kp->count++;
      }
      else if ( w == kp->pbBksp ) {
        kp->positive = 1;
        kp->state = keypadClass::ISNULL;
        kp->count = 0;
        strcpy( kp->buf, "-" );
      }

      kp->positive = 1;

      break;

    case keypadClass::NODECPOINT:

      if ( w == kp->pb0 ) {
        strncat( &kp->buf[1], "0", keypadClass::MAXCHARS-1 );
        kp->buf[keypadClass::MAXCHARS] = 0;
        kp->count++;
      }
      else if ( w == kp->pb1 ) {
        strncat( &kp->buf[1], "1", keypadClass::MAXCHARS-1 );
        kp->buf[keypadClass::MAXCHARS] = 0;
        kp->count++;
      }
      else if ( w == kp->pb2 ) {
        strncat( &kp->buf[1], "2", keypadClass::MAXCHARS-1 );
        kp->buf[keypadClass::MAXCHARS] = 0;
        kp->count++;
      }
      else if ( w == kp->pb3 ) {
        strncat( &kp->buf[1], "3", keypadClass::MAXCHARS-1 );
        kp->buf[keypadClass::MAXCHARS] = 0;
        kp->count++;
      }
      else if ( w == kp->pb4 ) {
        strncat( &kp->buf[1], "4", keypadClass::MAXCHARS-1 );
        kp->buf[keypadClass::MAXCHARS] = 0;
        kp->count++;
      }
      else if ( w == kp->pb5 ) {
        strncat( &kp->buf[1], "5", keypadClass::MAXCHARS-1 );
        kp->buf[keypadClass::MAXCHARS] = 0;
        kp->count++;
      }
      else if ( w == kp->pb6 ) {
        strncat( &kp->buf[1], "6", keypadClass::MAXCHARS-1 );
        kp->buf[keypadClass::MAXCHARS] = 0;
        kp->count++;
      }
      else if ( w == kp->pb7 ) {
        strncat( &kp->buf[1], "7", keypadClass::MAXCHARS-1 );
        kp->buf[keypadClass::MAXCHARS] = 0;
        kp->count++;
      }
      else if ( w == kp->pb8 ) {
        strncat( &kp->buf[1], "8", keypadClass::MAXCHARS-1 );
        kp->buf[keypadClass::MAXCHARS] = 0;
        kp->count++;
      }
      else if ( w == kp->pb9 ) {
        strncat( &kp->buf[1], "9", keypadClass::MAXCHARS-1 );
        kp->buf[keypadClass::MAXCHARS] = 0;
        kp->count++;
      }
      else if ( w == kp->pbPoint ) {
        strncat( &kp->buf[1], ".", keypadClass::MAXCHARS-1 );
        kp->buf[keypadClass::MAXCHARS] = 0;
        kp->count++;
        kp->state = keypadClass::DECPOINT;
      }
      else if ( w == kp->pbSign ) {
        if ( kp->positive ) {
          kp->positive = 0;
	}
	else {
          kp->positive = 1;
	}
      }
      else if ( w == kp->pbBksp ) {
        if ( kp->count == 1 ) {
          kp->positive = 1;
          kp->state = keypadClass::ISNULL;
          kp->count = 0;
          strcpy( kp->buf, "-" );
	}
        else {
          kp->buf[kp->count] = 0;
          kp->count--;
	}
      }

      break;

    case keypadClass::DECPOINT:

      if ( w == kp->pb0 ) {
        strncat( &kp->buf[1], "0", keypadClass::MAXCHARS-1 );
        kp->buf[keypadClass::MAXCHARS] = 0;
        kp->count++;
      }
      else if ( w == kp->pb1 ) {
        strncat( &kp->buf[1], "1", keypadClass::MAXCHARS-1 );
        kp->buf[keypadClass::MAXCHARS] = 0;
        kp->count++;
      }
      else if ( w == kp->pb2 ) {
        strncat( &kp->buf[1], "2", keypadClass::MAXCHARS-1 );
        kp->buf[keypadClass::MAXCHARS] = 0;
        kp->count++;
      }
      else if ( w == kp->pb3 ) {
        strncat( &kp->buf[1], "3", keypadClass::MAXCHARS-1 );
        kp->buf[keypadClass::MAXCHARS] = 0;
        kp->count++;
      }
      else if ( w == kp->pb4 ) {
        strncat( &kp->buf[1], "4", keypadClass::MAXCHARS-1 );
        kp->buf[keypadClass::MAXCHARS] = 0;
        kp->count++;
      }
      else if ( w == kp->pb5 ) {
        strncat( &kp->buf[1], "5", keypadClass::MAXCHARS-1 );
        kp->buf[keypadClass::MAXCHARS] = 0;
        kp->count++;
      }
      else if ( w == kp->pb6 ) {
        strncat( &kp->buf[1], "6", keypadClass::MAXCHARS-1 );
        kp->buf[keypadClass::MAXCHARS] = 0;
        kp->count++;
      }
      else if ( w == kp->pb7 ) {
        strncat( &kp->buf[1], "7", keypadClass::MAXCHARS-1 );
        kp->buf[keypadClass::MAXCHARS] = 0;
        kp->count++;
      }
      else if ( w == kp->pb8 ) {
        strncat( &kp->buf[1], "8", keypadClass::MAXCHARS-1 );
        kp->buf[keypadClass::MAXCHARS] = 0;
        kp->count++;
      }
      else if ( w == kp->pb9 ) {
        strncat( &kp->buf[1], "9", keypadClass::MAXCHARS-1 );
        kp->buf[keypadClass::MAXCHARS] = 0;
        kp->count++;
      }
      else if ( w == kp->pbSign ) {
        if ( kp->positive ) {
          kp->positive = 0;
	}
	else {
          kp->positive = 1;
	}
      }
      else if ( w == kp->pbBksp ) {
        if ( kp->count == 1 ) {
          kp->positive = 1;
          kp->state = keypadClass::ISNULL;
          kp->count = 0;
          strcpy( kp->buf, "-" );
	}
        else if ( kp->buf[kp->count] == '.' ) {
          kp->buf[kp->count] = 0;
          kp->state = keypadClass::NODECPOINT;
          kp->count--;
	}
        else {
          kp->buf[kp->count] = 0;
          kp->count--;
	}
      }

      break;

    }

    if ( kp->positive ) {
      XmTextFieldSetString( kp->text, &kp->buf[1] );
    }
    else {
      XmTextFieldSetString( kp->text, kp->buf );
    }

  }

}

static void intKeypadPress (
  Widget w,
  XtPointer client,
  XtPointer call )
{

  keypadPress( w, client, call, keypadClass::INT );

}

static void doubleKeypadPress (
  Widget w,
  XtPointer client,
  XtPointer call )
{

  keypadPress( w, client, call, keypadClass::DOUBLE );

}

keypadClass::keypadClass () {

  entryTag = NULL;
  actionTag = NULL;
  shell = NULL;
  poppedUp = 0;

}

keypadClass::~keypadClass ( void ) {

  if ( shell ) {
    if ( isPoppedUp() ) popdown();
    XtDestroyWidget( shell );
  }

}

void keypadClass::popup ( void ) {

  XtPopup( shell, XtGrabNone );
  poppedUp = 1;

}

void keypadClass::popdown ( void ) {

  XtPopdown( shell );
  poppedUp = 0;

}

int keypadClass::isPoppedUp ( void ) {

  return poppedUp;

}

int keypadClass::create (
  Widget top,
  int _x,
  int _y,
  char *label,
  //fontInfoClass *fi,
  //const char *entryFontTag,
  //const char *actionFontTag,
  int dataType,
  void *destination,
  void *_userPtr,
  XtCallbackProc _okFunc,
  XtCallbackProc _cancelFunc ) {

  XmString str;
  XtCallbackProc func = 0;

  x = _x;
  y = _y;
  userPtr = _userPtr,
  okFunc = _okFunc;
  cancelFunc = _cancelFunc;
  display = XtDisplay( top );
  poppedUp = 0;

    switch ( dataType ) {

    case keypadClass::INT:
      intDest = (int *) destination;
      func = (XtCallbackProc)intKeypadPress;
      break;

    case keypadClass::DOUBLE:
      doubleDest = (double *) destination;
      func = (XtCallbackProc)doubleKeypadPress;
      break;

    }

#if 0
  if ( fi ) {

    if ( entryFontTag ) {
      entryTag = new char[strlen(entryFontTag)+1];
      strcpy( entryTag, entryFontTag );
      fi->getTextFontList( entryTag, &entryFontList );
    }

    if ( actionFontTag ) {
      actionTag = new char[strlen(actionFontTag)+1];
      strcpy( actionTag, actionFontTag );
      fi->getTextFontList( actionTag, &actionFontList );
    }

  }
#endif

  if ( shell ) {
    XtDestroyWidget( shell );
  }

  shell = XtVaCreatePopupShell( "", xmDialogShellWidgetClass,
   top,
   XmNmappedWhenManaged, False,
   XmNmwmDecorations, 0,
   XmNx, x,
   XmNy, y,
   NULL );

  rowcol = XtVaCreateWidget( "", xmRowColumnWidgetClass, shell,
   XmNorientation, XmVERTICAL,
   XmNnumColumns, 1,
   NULL );

  topForm = XtVaCreateWidget( "", xmFormWidgetClass, rowcol, NULL );

  kprowcol = XtVaCreateWidget( "", xmRowColumnWidgetClass, rowcol,
   XmNorientation, XmHORIZONTAL,
   XmNnumColumns, 4,
   XmNpacking, XmPACK_COLUMN,
   NULL );

  bottomForm = XtVaCreateWidget( "", xmFormWidgetClass, rowcol, NULL );

  text = XtVaCreateManagedWidget( "", xmTextFieldWidgetClass, topForm,
   XmNcolumns, (short) keypadClass::MAXCHARS,
   XmNmaxLength, (short) keypadClass::MAXCHARS,
   XmNleftAttachment, XmATTACH_FORM,
   XmNrightAttachment, XmATTACH_FORM,
   XmNeditable, 0,
   XmNcursorPositionVisible, 0,
   NULL );

  positive = 1;
  count = 0;

#if 0
  state = ZERO;
  strcpy( this->buf, "-0" );
  XmTextFieldSetString( text, "0" );
#endif
  state = ISNULL;
  strcpy( this->buf, "-" );
  XmTextFieldSetString( text, "" );

// ---------------------------------------

  if ( entryTag )
    str = XmStringCreate( "7", entryTag );
  else
    str = XmStringCreateLocalized( "7" );

  pb7 = XtVaCreateManagedWidget(
   "", xmPushButtonWidgetClass,
   kprowcol,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  XtAddCallback( pb7, XmNactivateCallback, (XtCallbackProc) func, this );

// ---------------------------------------

  if ( entryTag )
    str = XmStringCreate( "8", entryTag );
  else
    str = XmStringCreateLocalized( "8" );

  pb8 = XtVaCreateManagedWidget(
   "", xmPushButtonWidgetClass,
   kprowcol,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  XtAddCallback( pb8, XmNactivateCallback, (XtCallbackProc) func, this );

// ---------------------------------------

  if ( entryTag )
    str = XmStringCreate( "9", entryTag );
  else
    str = XmStringCreateLocalized( "9" );

  pb9 = XtVaCreateManagedWidget(
   "", xmPushButtonWidgetClass,
   kprowcol,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  XtAddCallback( pb9, XmNactivateCallback, (XtCallbackProc) func, this );

// ---------------------------------------




// ---------------------------------------

  if ( entryTag )
    str = XmStringCreate( "4", entryTag );
  else
    str = XmStringCreateLocalized( "4" );

  pb4 = XtVaCreateManagedWidget(
   "", xmPushButtonWidgetClass,
   kprowcol,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  XtAddCallback( pb4, XmNactivateCallback, (XtCallbackProc) func, this );

// ---------------------------------------

  if ( entryTag )
    str = XmStringCreate( "5", entryTag );
  else
    str = XmStringCreateLocalized( "5" );

  pb5 = XtVaCreateManagedWidget(
   "", xmPushButtonWidgetClass,
   kprowcol,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  XtAddCallback( pb5, XmNactivateCallback, (XtCallbackProc) func, this );

// ---------------------------------------

  if ( entryTag )
    str = XmStringCreate( "6", entryTag );
  else
    str = XmStringCreateLocalized( "6" );

  pb6 = XtVaCreateManagedWidget(
   "", xmPushButtonWidgetClass,
   kprowcol,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  XtAddCallback( pb6, XmNactivateCallback, (XtCallbackProc) func, this );

// ---------------------------------------




// ---------------------------------------

  if ( entryTag )
    str = XmStringCreate( "1", entryTag );
  else
    str = XmStringCreateLocalized( "1" );

  pb1 = XtVaCreateManagedWidget(
   "", xmPushButtonWidgetClass,
   kprowcol,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  XtAddCallback( pb1, XmNactivateCallback, (XtCallbackProc) func, this );

// ---------------------------------------

  if ( entryTag )
    str = XmStringCreate( "2", entryTag );
  else
    str = XmStringCreateLocalized( "2" );

  pb2 = XtVaCreateManagedWidget(
   "", xmPushButtonWidgetClass,
   kprowcol,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  XtAddCallback( pb2, XmNactivateCallback, (XtCallbackProc) func, this );

// ---------------------------------------

  if ( entryTag )
    str = XmStringCreate( "3", entryTag );
  else
    str = XmStringCreateLocalized( "3" );

  pb3 = XtVaCreateManagedWidget(
   "", xmPushButtonWidgetClass,
   kprowcol,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  XtAddCallback( pb3, XmNactivateCallback, (XtCallbackProc) func, this );

// ---------------------------------------




// ---------------------------------------

  if ( entryTag )
    str = XmStringCreate( "0", entryTag );
  else
    str = XmStringCreateLocalized( "0" );

  pb0 = XtVaCreateManagedWidget(
   "", xmPushButtonWidgetClass,
   kprowcol,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  XtAddCallback( pb0, XmNactivateCallback, (XtCallbackProc) func, this );

// ---------------------------------------

  if ( entryTag )
    str = XmStringCreate( ".", entryTag );
  else
    str = XmStringCreateLocalized( "." );

  pbPoint = XtVaCreateManagedWidget(
   "", xmPushButtonWidgetClass,
   kprowcol,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  XtAddCallback( pbPoint, XmNactivateCallback, (XtCallbackProc) func, this );

// ---------------------------------------

  if ( entryTag )
    str = XmStringCreate( "+/-", entryTag );
  else
    str = XmStringCreateLocalized( "+/-" );

  pbSign = XtVaCreateManagedWidget(
   "", xmPushButtonWidgetClass,
   kprowcol,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  XtAddCallback( pbSign, XmNactivateCallback, (XtCallbackProc) func, this );

// ---------------------------------------




// ---------------------------------------

  if ( entryTag )
    str = XmStringCreate( "Cancel", entryTag );
  else
    str = XmStringCreateLocalized( "Cancel" );

  pbCancel = XtVaCreateManagedWidget(
   "", xmPushButtonWidgetClass,
   bottomForm,
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
   "", xmPushButtonWidgetClass,
   bottomForm,
   XmNlabelString, str,
   XmNbottomAttachment, XmATTACH_FORM,
   XmNrightAttachment, XmATTACH_FORM,
   NULL );

  XmStringFree( str );

  XtAddCallback( pbOK, XmNactivateCallback, (XtCallbackProc) func, this );

// ---------------------------------------

  if ( entryTag )
    str = XmStringCreate( "BS", entryTag );
  else
    str = XmStringCreateLocalized( "BS" );

  pbBksp = XtVaCreateManagedWidget(
   "", xmPushButtonWidgetClass,
   bottomForm,
   XmNlabelString, str,
   XmNbottomAttachment, XmATTACH_FORM,
   XmNrightAttachment, XmATTACH_WIDGET,
   XmNrightWidget, pbOK,
   XmNleftAttachment, XmATTACH_WIDGET,
   XmNleftWidget, pbCancel,
   NULL );

  XmStringFree( str );

  XtAddCallback( pbBksp, XmNactivateCallback, (XtCallbackProc) func, this );

// ---------------------------------------

  XtManageChild( topForm );
  XtManageChild( kprowcol );
  XtManageChild( bottomForm );
  XtManageChild( rowcol );
  XtRealizeWidget( shell );
  popup();

  return 1;

}

int keypadClass::create (
  Widget top,
  int _x,
  int _y,
  char *label,
  //fontInfoClass *fi,
  //const char *entryFontTag,
  //const char *actionFontTag,
  int *destination,
  void *_userPtr,
  XtCallbackProc _okFunc,
  XtCallbackProc _cancelFunc ) {

int stat;

  stat = create ( top, _x, _y, label, keypadClass::INT,
   (void *) destination, _userPtr, _okFunc, _cancelFunc );
  return stat;

}

int keypadClass::create (
  Widget top,
  int _x,
  int _y,
  char *label,
  //fontInfoClass *fi,
  //const char *entryFontTag,
  //const char *actionFontTag,
  double *destination,
  void *_userPtr,
  XtCallbackProc _okFunc,
  XtCallbackProc _cancelFunc ) {

int stat;

  stat = create ( top, _x, _y, label, keypadClass::DOUBLE,
   (void *) destination, _userPtr, _okFunc, _cancelFunc );
  return stat;

}

#if 0

main() {

XtAppContext app;
Display *display;
Widget appTop, mainWin;
keypadClass kp, kp1;
int dest, old;
double dest1, old1;
XEvent Xev;
int result, isXEvent, argc;

  XtSetLanguageProc( NULL, NULL, NULL );

  argc = 0;
  appTop = XtVaAppInitialize( &app, "test", NULL, 0, &argc,
   NULL, NULL, XmNiconic, False, NULL );

  mainWin = XtVaCreateManagedWidget( "", xmMainWindowWidgetClass,
   appTop,
   XmNwidth, 500,
   XmNheight, 500,
   XmNscrollBarDisplayPolicy, XmAS_NEEDED,
   XmNscrollingPolicy, XmAUTOMATIC,
   NULL );

  XtRealizeWidget( appTop );

  display = XtDisplay( appTop );

  old = -1;
  dest = 0;

  kp.create( appTop, 100, 100, "label", &dest );

  old1 = -1;
  dest1 = 0;

  kp1.create( appTop, 300, 200, "label", &dest1 );

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

  if ( dest != old ) {
    printf( "dest = %-d\n", dest );
    old = dest;
  }

  if ( dest1 != old1 ) {
    printf( "dest1 = %-g\n", dest1 );
    old1 = dest1;
  }

  }

}
#endif
