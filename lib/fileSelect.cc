#define __fileSelect_cc

#include "fileSelect.h"

#if 0

int blank (
  char *string )
{

unsigned int i;

  for ( i=0; i<strlen(string); i++ ) {
    if ( !isspace( (int) string[i] ) ) return 0;
  }

  return 1;

}

#endif

static void fselectOk (
  Widget w,
  XtPointer client,
  XtPointer call
) {

fselectClass *fso = (fselectClass *) client;
XmFileSelectionBoxCallbackStruct *cbs =
 (XmFileSelectionBoxCallbackStruct *) call;
char *fName;

  if ( !XmStringGetLtoR( cbs->value, XmFONTLIST_DEFAULT_TAG, &fName ) ) {
    strcpy( fso->selection, "" );
    goto done;
  }

  if ( !*fName ) {
    XtFree( fName );
    strcpy( fso->selection, "" );
    goto done;
  }

  strncpy( fso->selection, fName, 255 );
  fso->selection[255] = 0;

  XtFree( fName );

done:

  fso->popdown();
  if ( fso->okFunc ) {
    (*fso->okFunc)( w, (XtPointer) fso->userPtr, call );
  }
  XtDestroyWidget( fso->fs );
  fso->fs = NULL;

}

static void fselectCancel (
  Widget w,
  XtPointer client,
  XtPointer call
) {

fselectClass *fso = (fselectClass *) client;

  fso->popdown();
  if ( fso->cancelFunc ) {
    (*fso->cancelFunc)( w, (XtPointer) fso->userPtr, call );
  }
  XtDestroyWidget( fso->fs );
  fso->fs = NULL;

}

fselectClass::fselectClass () {

  entryTag = NULL;
  actionTag = NULL;
  fs = NULL;
  poppedUp = 0;

}

fselectClass::~fselectClass ( void ) {

  if ( fs ) {
    if ( isPoppedUp() ) popdown();
    XtDestroyWidget( fs );
  }

}

void fselectClass::popup ( void ) {

  XtManageChild( fs );
  poppedUp = 1;

}

void fselectClass::popdown ( void ) {

  XtUnmanageChild( fs );
  poppedUp = 0;

}

int fselectClass::isPoppedUp ( void ) {

  return poppedUp;

}

int fselectClass::setDefDir (
  char *str
) {

  return 1;

}

int fselectClass::setPattern (
  char *str
) {

  return 1;

}

int fselectClass::create (
  Widget top,
  int _x,
  int _y,
  char *_defDir,
  char *_pattern,
  void *_userPtr,
  XtCallbackProc _okFunc,
  XtCallbackProc _cancelFunc
) {

int n;
Arg args[10];
XmString str1, str2;
Atom wm_delete_window;

  userPtr = _userPtr;
  okFunc = _okFunc;
  cancelFunc = _cancelFunc;

  n = 0;
  str1 = str2 = NULL;

  XtSetArg( args[n], XmNx, _x ); n++;
  XtSetArg( args[n], XmNy, _y ); n++;
  XtSetArg( args[n], XmNdefaultPosition, False ); n++;

  if ( _pattern ) {
    if ( !blank( _pattern ) ) {
      str1 = XmStringCreateLocalized( _pattern );
      XtSetArg( args[n], XmNpattern, str1 ); n++;
    }
  }

  if ( _defDir ) {
    if ( !blank( _defDir ) ) {
      str2 = XmStringCreateLocalized( _defDir );
      XtSetArg( args[n], XmNdirectory, str2 ); n++;
    }
  }

  fs = XmCreateFileSelectionDialog( top, "", args,
   n );

  if ( str1 ) XmStringFree( str1 );
  if ( str2 ) XmStringFree( str2 );

  XtAddCallback( fs, XmNcancelCallback, fselectCancel, (void *) this );

  XtAddCallback( fs, XmNokCallback, fselectOk, (void *) this );

  wm_delete_window = XmInternAtom( XtDisplay(top),
   "WM_DELETE_WINDOW", False );

  XmAddWMProtocolCallback( XtParent(fs),
   wm_delete_window, fselectCancel, (void *) this );

  XtVaSetValues( XtParent(fs), XmNdeleteResponse, XmDO_NOTHING, NULL );

  popup();

  return 1;

}

char *fselectClass::getSelection (
  char *str,
  int maxLen
) {

  strncpy( str, selection, maxLen );
  str[maxLen] = 0;

  return selection;

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
fselectClass *fs = (fselectClass *) client;

  fprintf( stderr, "ok, selection = [%s]\n", fs->getSelection( str, 127 ) );

}

main() {

XtAppContext app;
Display *display;
Widget appTop, mainWin;
fselectClass fs;
XEvent Xev;
int i, result, isXEvent, argc, stat;

  argc = 0;
  appTop = XtVaAppInitialize( &app, "test", NULL, 0, &argc,
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

  fs.create( appTop, 100, 100, "/home/sinclair", "*.cc",
   ok, cancel );

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
