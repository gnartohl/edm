#include <sys/time.h>
#include <stream.h> // cout
#include <stdio.h> // sprintf ()
#include <X11/X.h> // Xlib
#include <X11/Intrinsic.h> // Xt
#include <X11/xpm.h> // XPM
#include <Xm/Xm.h> // Motif
#include <Xm/Label.h> // Motif Label widget

#include "widget.h"

typedef struct 
{
    XtAppContext app;
    Widget w;
    Pixmap p;
    Display *d;
    Colormap cm;
    Widget parent;
    unsigned long widgetw_old;
    unsigned long widgeth_old;
    unsigned long dataw_old;
    unsigned long datah_old;
} myData;

widgetData widgetCreate (void)
{
    myData *md = new myData;

    /* brutally set everything to NULL */
    memset (md, 0, sizeof (*md));

    return (widgetData) md;
}

static Pixmap makePixmap (Display *d, Colormap cm, Widget w, char **data)
{

    XpmAttributes attr;
    Pixmap p = 0;

   /* tell XPM to allocate fairly close (not exact) colors from the window's
      colormap */
    attr.valuemask = XpmSize | XpmCloseness | XpmAllocCloseColors |
                     XpmExactColors | XpmColormap | XpmDepth | XpmVisual;
    attr.closeness = 30000;
    attr.alloc_close_colors = True;
    attr.exactColors = False;
    attr.colormap = cm;
    attr.depth = DefaultDepth ( d, DefaultScreen (d));
    attr.visual = DefaultVisual ( d, DefaultScreen (d));
  
    /* while it's at it, tell us what size the pixmap is */
    if (XpmCreatePixmapFromData (d, XtWindow (w), data, &p, NULL, &attr) !=
        XpmSuccess)
        return 0;
  
    return p;
}

#include "twoD.xpm"

Widget widgetCreateWidget (widgetData wd, XtAppContext app, Display *d,
                           Colormap cm, Widget parent, int x, int y,
                           int h, int w)
{
  
    Widget widget;
  
    myData *md = (myData *) wd;
  
    md->app = app;
    md->d = d;
    md->cm = cm;
    md->parent = parent;
    if (md->p == 0)
        md->p = makePixmap (d, cm, parent, twoD);
  
    /* Here's our picture, no border around our nice pixmap */
    widget = XtVaCreateManagedWidget ("2D widget", xmLabelWidgetClass, parent, 
                                     XmNshadowThickness, 0,
                                     XmNhighlightThickness, 0,
                                     XmNmarginHeight, 0,
                                     XmNmarginWidth, 0,
                                     XmNx, x,
                                     XmNy, y,
                                     XmNheight, h,
                                     XmNwidth, w,
                                     XmNrecomputeSize, False,
                                     NULL);

#ifdef COMMENT_OUT  
    /* show the picture (centered) */
    XtVaSetValues (widget, XmNlabelType, XmPIXMAP, XmNlabelPixmap, md->p, NULL);
#endif
  
    md->w = widget;

    return widget;
}

#include "pixmap.xpm"
#include <stdlib.h> // malloc ()

void widgetNewDisplayData (widgetData wd,
                           time_t time,
                           unsigned long nano,
                           unsigned long widgetw,
                           unsigned long widgeth,
                           unsigned long dataw,
                           unsigned long datah,
                           const double *data)
{
#ifdef DEBUG
    printf ("Start of widgetNewDisplayData\n");
#endif

    if (!data || !widgetw || !widgeth || !dataw || !datah)
    {
        printf ("widgetNewDisplayData - missing data, width or height\n");
        return;
    }

    myData *md = (myData *) wd;

    /****** The following code block fixes the aspect ratio of the ******
     ****** pixmap to that of the image data                       ******/
    if (widgeth * dataw > widgetw * datah)
        widgeth = widgetw * datah / dataw;
    else
    if (widgetw * datah > widgeth * dataw) 
        widgetw = widgeth * dataw / datah; 
    /* else if ratios are equal do nothing */

    char ** pixmapData =
                (char **) malloc (sizeof (char *) * (widgeth + 256 + 1));
    char* pixmapLine;
    const double* dataLine;

    pixmapData[0] = (char *) malloc (1024);

    sprintf (pixmapData[0], "%6ld%6ld%9d%13d", widgetw, widgeth,
             256 /* colours */, 2 /* chars per colour */);

    unsigned int index;
    for (index = 1; index < 257; index++)
        pixmapData[index] = pixmap[index];
  
    while (index < (widgeth + 256 + 1))
    {
        pixmapData[index] = (char *) malloc (2 * widgetw + 1);
        pixmapData[index][2 * widgetw] = 0;
        index++;
    }

    // gettimeofday (&tv, 0);
    // printf ("widgetNDD after mallocs - time %ld %ld\n", tv.tv_sec,
    //         tv.tv_usec);
    for (unsigned long j = 0; j < widgeth; j++)
    {
        pixmapLine = pixmapData[257 + j];
        unsigned long dataj = (j * datah / widgeth);
        dataLine = &data[dataj * dataw];
        for (unsigned long i = 0; i < widgetw; i++)
        {
            unsigned long datai = i * dataw / widgetw;
            pixmapLine[i * 2] = (((unsigned char)dataLine[datai]) / 16) + 'a';
            pixmapLine[1 + i * 2] =
                                (((unsigned char)dataLine[datai]) % 16) + 'a';
        }
    } 
    // gettimeofday (&tv, 0);
    // printf ("widgetNDD after create pixmapData - time %ld %ld\n", tv.tv_sec,
    //         tv.tv_usec);
    register Pixmap temp = makePixmap (md->d, md->cm, md->parent, pixmapData);

    // gettimeofday (&tv, 0);
    // printf ("widgetNDD after makePixmap - time %ld %ld\n", tv.tv_sec,
    //         tv.tv_usec);
    free (pixmapData[0]);
    for (index = 257; index < widgeth + 256 + 1; index++)
        free (pixmapData[index]);
    free (pixmapData);

    // just in case something goes horribly wrong
    if (!temp)
    {
        printf ("widgetNewDisplayData - makePixmap fail\n");
        return;
    }

    // gettimeofday (&tv, 0);
    // printf ("widgetNDD before redraw - time %ld %ld\n", tv.tv_sec,
    //         tv.tv_usec);
#define DEMO_MODE
#ifdef DEMO_MODE
    // performance hack
    // If pixmap is same size, redraw and expose rather than assigning new
    // pixmap
    if (dataw != md->dataw_old || datah != md->datah_old)
    {
#ifdef DEBUG
        printf ("widgetNDD - new data dimensions w %lu h %lu (old w %lu h %lu)\n",
                dataw, datah, md->dataw_old, md->datah_old);
#endif
        md->dataw_old = dataw;
        md->datah_old = datah;
    }
    if (widgetw != md->widgetw_old || widgeth != md->widgeth_old)
    {
        md->widgetw_old = widgetw; md->widgeth_old = widgeth;
#endif

        Pixmap old = md->p;
        md->p = temp;

        XtVaSetValues (md->w, XmNlabelType, XmPIXMAP, XmNlabelPixmap, md->p,
                       NULL);
  
        XFreePixmap (md->d, old);


#ifdef DEMO_MODE

    }
    else
    {
        XCopyArea (md->d, temp, md->p, DefaultGC (md->d, 0), 0, 0,
                   widgetw, widgeth, 0, 0);
        XFreePixmap (md->d, temp);

        static XEvent event = {Expose};
        XtDispatchEventToWidget (md->w, &event);
    }
#endif
    // gettimeofday (&tv, 0);
    // printf ("widgetNDD end - time %ld %ld\n", tv.tv_sec, tv.tv_usec);
}


void widgetNewDisplayInfo (widgetData wd, bool valid, short status,
                           short severity)
{

    myData *md = (myData *) wd;

    // if anything is wrong, disable image and reset width to
    // force re-draw on recovery
    if (!valid || status || severity)
    {
        XtVaSetValues (md->w, XmNlabelType, XmSTRING, NULL);
        md->widgetw_old = 0;
        md->widgeth_old = 0;
    }

}

// nothing we need to do (like shut off X timers)
void widgetDestroyWidget (widgetData wd)
{

}

// application is all done (this may never actually get called)
void widgetDestroy (widgetData wd)
{
  
    myData *md = (myData *) wd;
  
    if (md->p)
        XFreePixmap (md->d, md->p);
  
    delete md;
}
