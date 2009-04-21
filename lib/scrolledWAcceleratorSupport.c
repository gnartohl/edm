/* $Id$ */

/* Recursively install Scrollbar accelerators to all children of
 * a Scrolled Window. This is useful to add Wheel Mouse Support
 * for scrolling.
 *
 * We do this at 'realize' time, i.e., when hopefully all children
 * have been created.
 *
 * The proper way would be supporting a dedicated new widget class
 * derived from ScrolledW.
 *
 * Instead, we just hook into the ScrolledW's realize method which
 * is sort of a hack. However, my conscience is OK because EDM
 * screws Xt/Motif in such bad ways that this addition can be considered
 * 'beautiful'...
 *
 * RESOURCE FILE NOTE:
 * In order for the wheel mouse to actually work, proper resources
 * must be present:
 *
 *  *XmScrolledWindow.VertScrollBar.accelerators:#override\n\
 *               ~Shift<Btn4Up>: IncrementUpOrLeft(Up)\n\
 *               ~Shift<Btn5Up>: IncrementDownOrRight(Down)\n
 *
 *  *XmScrolledWindow.HorScrollBar.accelerators:#override\n\
 *               Shift<Btn4Up>:  IncrementUpOrLeft(Left)\n\
 *               Shift<Btn5Up>:  IncrementDownOrRight(Right)\n
 *
 * Author: Till Straumann <strauman|at|slac.stanford.edu>, 2004
 */

#include <stdlib.h>
#include <X11/IntrinsicP.h>
#include <X11/CoreP.h>
#include <Xm/ScrolledWP.h>

static XtRealizeProc origRealize = 0;
static int giveUp = 0;

/* install the accelerators of the NULL terminated widget list
 * 'srcs' recursively onto the children of 'destTop'.
 */
static void recursiveInstallAccs(Widget destTop, WidgetList srcs)
{
WidgetList c;
Cardinal   n;

	/* mustn't try to install if it's not a widget (then it's certainly
	 * not composite either).
     */

    if ( !XtIsWidget(destTop) )
        return;

	/* recurse */
    if (XtIsComposite(destTop)) {
        XtVaGetValues(destTop, XmNchildren, &c, XmNnumChildren, &n, NULL);
        while ( n-- > 0 ) {
            recursiveInstallAccs(*c++, srcs);
        }
    }

	/* and finally install all accelerator sources onto this child */
    while (*srcs)
        XtInstallAccelerators(destTop, *srcs++);
}
                                                                                                                           
/* Our 'realize' method that is hooked into 'ScrolledWindowWidgetClass->core_class.realize' */
static void installAccsRealize(Widget w, XtValueMask *pm, XSetWindowAttributes *atts)
{
XmScrolledWindowWidget sw = (XmScrolledWindowWidget)w;
Widget	bars[3];
int 	i = 0;

        if ( giveUp ) return;
	
	if ( XtInheritRealize == origRealize ) {
	/* If the 'realize' method was still 'XtInherit' at the time they installed
	 * the hack, we must search for the superclass method.
	 */
		WidgetClass wc = w->core.widget_class;
		do {
			wc = wc->core_class.superclass;
			origRealize = wc->core_class.realize;
			/* a subclass might have inherited us; skip */
		} while ( installAccsRealize == origRealize );
	}

	/* paranoia check */
	if ( XtInheritRealize == origRealize ) {
	  /*XtError("scrollWinAccSupport(): Fatal error; unable to hook realize method");*/
	  /*exit(1);*/
	  XtWarning("scrollWinAccSupport(): Error; unable to hook realize method");
	  giveUp = 1;
	  return;
	}

	/* Call superclass realize *before* installing accelerators
	 * to avoid recursion problems (realize is recursive)
	 */
	if ( origRealize )
		origRealize( w, pm, atts );

	/* construct a source widget list */
	if ( (bars[i]=(Widget)sw->swindow.hScrollBar) )
		i++;
	if ( (bars[i]=(Widget)sw->swindow.vScrollBar) )
		i++;
	bars[i]=0;

	/* install */
	recursiveInstallAccs(w, bars);
}

void
scrolledWAcceleratorSupportInstall()
{

#if 0
	if ( origRealize ) {
		XtError("scrolled window accelerator support already installed\n");
	} else {
			origRealize = xmScrolledWindowWidgetClass->core_class.realize;
			xmScrolledWindowWidgetClass->core_class.realize = installAccsRealize;
	}
#endif

	if ( !origRealize ) {
			origRealize = xmScrolledWindowWidgetClass->core_class.realize;
			xmScrolledWindowWidgetClass->core_class.realize = installAccsRealize;
	}

}
