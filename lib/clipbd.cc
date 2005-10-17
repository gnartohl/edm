/* $Id$ */

/* AUTHOR: Till Straumann (PTB/1999) */

/********************************************
 *
 * Provide a simple clipboard for requestors
 * of the XA_PRIMARY selection (only string
 * targets are currently supported).
 *
 ********************************************/

#define __clipdb_cc 1

#include "clipbd.h"
#include <X11/Xatom.h>
#include <X11/Xmu/Atoms.h>
#include <X11/Xmu/StdSel.h>
#include <Xm/Xm.h>

#include <string.h>

clipBdClass::clipBdClass ( void ) {

  strcpy( clipboard.buffer, "" );
  clipboard.index = 0;
  clipboard.owner = NULL;

}

clipBdClass::~clipBdClass ( void ) {

}

/* initialize the clipboard facility.
 */
void clipBdClass::clipbdInit (
  Widget parent
) {

  clipboard.index = 0;
  clipboard.buffer[sizeof(clipboard.buffer)-1] = (char) 0;
  clipboard.buffer[clipboard.index] = (char) 0;

  clipboard.owner = XtVaCreateWidget( "clipboard",
   xmPrimitiveWidgetClass,
   parent,
   XtNwidth, 1,
   XtNheight, 1,
   XmNuserData, (XtPointer) this,
   NULL );

}

/*
 * disown the XA_PRIMARY selection (if currently owned) and
 * prepare the clipboard for adding data by subsequent calls
 * to clipbdAdd().
 */
void clipBdClass::clipbdStart ( void ) {

  clipbdGiveup();
  clipboard.index=0;
  clipboard.buffer[clipboard.index]=(char)0;

}

/*
 * append a string to the clipboard.
 * (silently, we clip the data to sizeof(clipboard.buffer))
 */
void clipBdClass::clipbdAdd (
  char *string
) {

int remaining=sizeof(clipboard.buffer)-1 - clipboard.index;

  if ( remaining <= 0 ) return;

  if ( !string ) string="";

  strncpy( clipboard.buffer+clipboard.index, string, remaining );

  clipboard.index += strlen(string);

  if ( (unsigned int) clipboard.index >= sizeof(clipboard.buffer) )
    clipboard.index = sizeof(clipboard.buffer) - 1;

}

/*
 * This does the real work; it is copied from my Athena
 * TextField widget and based on examples from the
 * MIT distribution and O'Reilly's Xt Intrinsics Programming
 * Manual
 */

static Boolean ConvertSelection (
  Widget w,
  Atom *selection,
  Atom *target,
  Atom *type,
  XtPointer *valp,
  unsigned long *length,
  int *format
) {

char **value = (char**) valp;
Display* d = XtDisplay( w );
unsigned int tmp;

XSelectionRequestEvent* req =
 XtGetSelectionRequest( w, *selection, (XtRequestId) NULL );

clipBdClass *cb;

  XtVaGetValues( w, XmNuserData, &cb, NULL );

  if (*target == XA_TARGETS(d)) {

    Atom* targetP;
    Atom* std_targets;
    unsigned long std_length;

    XmuConvertStandardSelection( w, req->time, selection, target, type,
     (XPointer*) &std_targets, &std_length, format );

    *value =
     (char*) XtMalloc( sizeof(Atom) * ( (unsigned) std_length + 5 ) );
    targetP = *( (Atom**) value );
    *targetP++ = XA_STRING;
    *targetP++ = XA_TEXT(d);
    *length = std_length + ( targetP - ( *(Atom **) value) );

    memcpy( (void*) targetP, (void*) std_targets,
     (size_t)( sizeof(Atom) * std_length ) );

    XtFree( (char*) std_targets );

    *type = XA_ATOM;
    *format = 32;

    return True;

  }

  if ( *target == XA_STRING ||
       *target == XA_TEXT(d) ||
       *target == XA_COMPOUND_TEXT(d) ) {

    if ( *target == XA_COMPOUND_TEXT(d) ) {
      *type = *target;
    }
    else {
      *type = XA_STRING;
    }

    tmp = cb->clipboard.index;
    *length =  tmp;
    *value = (char*) strncpy( XtMalloc(tmp+1), cb->clipboard.buffer, (int) tmp );
    (*value)[tmp] = (char) 0;
    *format = 8;
    return True;

  }

  if ( XmuConvertStandardSelection( w, req->time, selection, target, type,
        (XPointer *) value, length, format ) ) {
    return True;
  }

  return False;

}

/*
 * assert the ownership of the XA_PRIMARY selection for
 * `widget'. 
 *
 * Returns 0 on success, -1 if the ownership was not granted
 * (in this case, calling clipbdHold() may be retried).
 */

int clipBdClass::clipbdHold ( void ) {

  if ( ! clipboard.owner) {
    XtWarning("No clipboard");
    return -1;
  }

  if ( XtOwnSelection( clipboard.owner, XA_PRIMARY, CurrentTime,
        ConvertSelection, (XtLoseSelectionProc) 0,
        (XtSelectionDoneProc) 0 ) ) {
    return 0;
  }

  return -1;

}

/*
 * Give up the ownership of the XA_PRIMARY selection
 * (without changing the clipboard contents).
 */
void clipBdClass::clipbdGiveup ( void ) {

  if ( !clipboard.owner ) {
    XtWarning( "No clipboard" );
    return;
  }

  XtDisownSelection( clipboard.owner, XA_PRIMARY, CurrentTime );

}
