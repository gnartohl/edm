/* $Id$ */

/* AUTHOR: Till Straumann (PTB/1999) */

/********************************************
 *
 * Provide a simple clipboard for requestors
 * of the XA_PRIMARY selection (only string
 * targets are currently supported).
 *
 ********************************************/

#include "clipbd.h"
#include <X11/Xatom.h>
#include <X11/Xmu/Atoms.h>
#include <X11/Xmu/StdSel.h>

#include <string.h>

#define CLIPBD_SIZE 1024

/* private clipboard data */
typedef struct ClipBdRec_ {
	char 	buffer[CLIPBD_SIZE];
	int		index;
	Widget	owner;
} ClipBdRec, *ClipBd;

static ClipBdRec clipboard = { { 0 }, 0, NULL };

static void
ownerDestroyedCB(Widget w, XtPointer cld, XtPointer cad)
{
	/* clear the owner, if we are destroyed */
//	XtWarning("Oops, destroying the clipboard!");
	((ClipBd)cld)->owner=0;
}

/* initialize the clipboard facility.
 */
void
clipbdInit(Widget parent)
{
	clipboard.index=0;
	clipboard.buffer[sizeof(clipboard.buffer)-1]=(char)0;
	clipboard.buffer[clipboard.index]=(char)0;
	clipboard.owner=XtVaCreateWidget("clipboard",
										widgetClass,
										parent,
										XtNwidth,1,
										XtNheight,1,
										0);
	XtAddCallback(clipboard.owner,
			  	XtNdestroyCallback,
			  	ownerDestroyedCB,
			  	(XtPointer)&clipboard);
}

/*
 * disown the XA_PRIMARY selection (if currently owned) and
 * prepare the clipboard for adding data by subsequent calls
 * to clipbdAdd().
 */
void
clipbdStart()
{
	clipbdGiveup();
	clipboard.index=0;
	clipboard.buffer[clipboard.index]=(char)0;
}

/*
 * append a string to the clipboard.
 * (silently, we clip the data to sizeof(clipboard.buffer))
 */
void
clipbdAdd(char *string)
{
int remaining=sizeof(clipboard.buffer)-1 - clipboard.index;

	if (remaining <= 0) return;

	if (!string) string="";

	strncpy(clipboard.buffer+clipboard.index,
			string,
			remaining);

	clipboard.index += strlen(string);
	if (clipboard.index >= sizeof(clipboard.buffer))
		clipboard.index = sizeof(clipboard.buffer) - 1;
}

/*
 * This does the real work; it is copied from my Athena
 * TextField widget and based on examples from the
 * MIT distribution and O'Reilly's Xt Intrinsics Programming
 * Manual
 */

static Boolean ConvertSelection
#if NeedFunctionPrototypes
(Widget w, Atom *selection, Atom *target, Atom *type,
 XtPointer *valp, unsigned long *length, int *format)
#else
(w, selection, target, type, valp, length, format)
    Widget w;
    Atom *selection, *target, *type;
    XtPointer *valp;
    unsigned long *length;
    int *format;
#endif
{
    char **value=(char**)valp;
    Display* d = XtDisplay(w);
    XSelectionRequestEvent* req =
	XtGetSelectionRequest(w, *selection, (XtRequestId)NULL);
    unsigned int tmp;

    if (*target == XA_TARGETS(d)) {
	Atom* targetP;
	Atom* std_targets;
	unsigned long std_length;
	XmuConvertStandardSelection(w, req->time, selection, target, type,
				  (XPointer*)&std_targets, &std_length, format);
	*value = (char*)(XtPointer)XtMalloc(sizeof(Atom)*((unsigned)std_length + 5));
	targetP = *(Atom**)value;
	*targetP++ = XA_STRING;
	*targetP++ = XA_TEXT(d);
	*length = std_length + (targetP - (*(Atom **) value));
	(void)memcpy( (void*)targetP, (void*)std_targets,
		      (size_t)(sizeof(Atom)*std_length));
	XtFree((char*)std_targets);
	*type = XA_ATOM;
	*format = 32;
	return True;
    }

    
    
    if (*target == XA_STRING ||
      *target == XA_TEXT(d) ||
      *target == XA_COMPOUND_TEXT(d))
    {
    	if (*target == XA_COMPOUND_TEXT(d))
	    *type = *target;
    	else
	    *type = XA_STRING;
        tmp=      clipboard.index;
		*length=  tmp;
    	*value =  (char*)(XtPointer)strncpy(XtMalloc(tmp+1),clipboard.buffer, (int) tmp);
        (*value)[tmp]=(char)0;
    	*format = 8;
    	return True;
    }
    
    if (XmuConvertStandardSelection(w, req->time, selection, target, type,
				    (XPointer *)value, length, format))
		return True;

    return False;
}

/*
 * assert the ownership of the XA_PRIMARY selection for
 * `widget'. 
 *
 * Returns 0 on success, -1 if the ownership was not granted
 * (in this case, calling clipbdHold() may be retried).
 */

int
clipbdHold()
{
  	if ( ! clipboard.owner) {
		XtWarning("No clipboard");
		return -1;
  	}
  	if ( XtOwnSelection(clipboard.owner, XA_PRIMARY,CurrentTime,
					 ConvertSelection,
					 (XtLoseSelectionProc)0,
					 (XtSelectionDoneProc)0) ) {
		return 0;
  	}
  	return -1;
}

/*
 * Give up the ownership of the XA_PRIMARY selection
 * (without changing the clipboard contents).
 */
void
clipbdGiveup()
{
	if ( ! clipboard.owner) {
		XtWarning("No clipboard");
		return;
	}
	XtDisownSelection(clipboard.owner, XA_PRIMARY, CurrentTime);
}
