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

/* initialize the clipboard facility.
 *
 * NOTE: The clipboardParent must exist and be
 *		 realized when calling any of the clipboard
 *		 routines.
 */
void
clipbdInit(Widget clipboardParent);

/*
 * disown the XA_PRIMARY selection (if currently owned) and
 * prepare the clipboard for adding data by subsequent calls
 * to clipbdAdd().
 */
void
clipbdStart();

/*
 * append a string to the clipboard.
 *
 */
void
clipbdAdd(char *string);

/*
 * assert the ownership of the XA_PRIMARY selection.
 *
 * Returns 0 on success, -1 if the ownership was not granted
 * (in this case, calling clipbdHold() may be retried).
 */

int
clipbdHold();

/*
 * Give up the ownership of the XA_PRIMARY selection
 * (without changing the clipboard contents).
 */
void
clipbdGiveup();

#endif
