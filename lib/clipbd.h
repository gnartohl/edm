/* $Id$ */
#ifndef CLIPBD_H
#define CLIPBD_H

/* AUTHOR: Till Straumann (PTB/1999) */

/********************************************
 *
 * Provide a simple clipboard for requestors
 * of the XA_PRIMARY selection (only string
 * targets are currently supported).
 *
 ********************************************/

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>

#ifdef __clipdb_cc

static Boolean ConvertSelection (
  Widget w,
  Atom *selection,
  Atom *target,
  Atom *type,
  XtPointer *valp,
  unsigned long *length,
  int *format
);

#endif

class clipBdClass {

public:

friend Boolean ConvertSelection (
  Widget w,
  Atom *selection,
  Atom *target,
  Atom *type,
  XtPointer *valp,
  unsigned long *length,
  int *format
);

clipBdClass ( void );

~clipBdClass ( void );

/* initialize the clipboard facility.
 *
 * NOTE: The clipboardParent must exist and be
 *		 realized when calling any of the clipboard
 *		 routines.
 */
void clipbdInit (
  Widget clipboardParent
);

/*
 * disown the XA_PRIMARY selection (if currently owned) and
 * prepare the clipboard for adding data by subsequent calls
 * to clipbdAdd().
 */
void clipbdStart ( void );

/*
 * append a string to the clipboard.
 *
 */
void clipbdAdd (
  char *string
);

/*
 * assert the ownership of the XA_PRIMARY selection.
 *
 * Returns 0 on success, -1 if the ownership was not granted
 * (in this case, calling clipbdHold() may be retried).
 */

int clipbdHold ( void );

/*
 * Give up the ownership of the XA_PRIMARY selection
 * (without changing the clipboard contents).
 */
void clipbdGiveup ( void );

static const int CLIPBD_SIZE=1024;

private:

/* private clipboard data */
typedef struct ClipBdRec_ {
  char   buffer[CLIPBD_SIZE];
  int    index;
  Widget owner;
} ClipBdRec, *ClipBd;

ClipBdRec clipboard;

};

#endif
