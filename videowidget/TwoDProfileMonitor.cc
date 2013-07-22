// g++ -shared -o TwoDProfileMonitor.so TwoDProfileMonitor.cc -g -O -Wall -ansi
// -pedantic -I/ade/epics/supTop/extensions/R3.14.4/src/edm/util
// -I/ade/epics/supTop/extensions/R3.14.4/src/edm/lib 
// -I/ade/epics/supTop/extensions/R3.14.4/src/edm/pvlib
// -I /ade/epics/supTop/base/R3.14.4/include -I/usr/X11R6/include
// -L/usr/X11R6/lib widget.cc -lXpm

// To Do: class initialization, serialization, configuration (all related)
// Any chance of allowing overlapped widgets to be on top?

#define VIDEO_MAX_LOAD_FACTOR 4
#define VIDEO_MAX_DATA_WIDTH 10000
#define VIDEO_MAX_DATA_HEIGHT 10000
#define VIDEO_NBITSPERPIXEL_DEFAULT 8
#define VIDEO_MAJOR_VERSION 4
#define VIDEO_MINOR_VERSION 2
#define VIDEO_RELEASE 1

#include <time.h>
#include <iostream>

//#ifdef SOLARIS
//#include <iostream.h>
//#else
//#include <stream.h>
//#endif

#include <act_grf.h>
#include <act_win.h>
#include <app_pkg.h>
#include <entry_form.h>
#include <pv_factory.h>
#include "edm.version"

//#include "widget.h"
#include "image.h"

void _edmDebug ( void );

// our widget class
class TwoDProfileMonitor : public activeGraphicClass
{

    // standard colours for various PV states
    pvColorClass pvColour; 

    // width of data (fixed or from PV)
    int dataWidth, dataHeight;
    int pvBasedDataSize;

    // globals for the edit popup
    int xBuf;
    int yBuf;
    int wBuf;
    int hBuf;
    int nBitsPerPixelBuf;
    char dataPvBuf[activeGraphicClass::MAX_PV_NAME+1];
    char widthPvBuf[activeGraphicClass::MAX_PV_NAME+1];
    char heightPvBuf[activeGraphicClass::MAX_PV_NAME+1];

    expStringClass dataPvStr, widthPvStr, heightPvStr;
    ProcessVariable *dataPv, *widthPv, *heightPv;

    // text stuff (for edit mode drawing)
    char *textFontTag;
    int textAlignment;
    int textColour;

    int initialDataConnection, initialWidthConnection, initialHeightConnection;
    int needConnectInit, needInfoInit, needDraw, needRefresh;
    unsigned char pvNotConnectedMask;
    int dataPvExists, widthPvExists, heightPvExists;
    int init, active, activeMode;
    struct timeval lasttv;
    unsigned long average_time_usec;
    int opComplete;
    int nBitsPerPixel;

    // widget-specific stuff
    //widgetData wd;
    //Widget twoDWidget;

    // image object
    imageClass *img;

    // constructor "helper" function
    void constructCommon (void);

public:

    // constructors/destructor
    TwoDProfileMonitor (void);
    TwoDProfileMonitor (const TwoDProfileMonitor &s);
    virtual ~TwoDProfileMonitor (void);

    // Called when the data process variable connects or disconnects
    static void monitorDataConnectState (ProcessVariable *pv,
                                         void *userarg );
    // Called when the width process variable connects or disconnects
    static void monitorWidthConnectState (ProcessVariable *pv,
                                          void *userarg );
    // Called when the height process variable connects or disconnects
    static void monitorHeightConnectState (ProcessVariable *pv,
                                          void *userarg );
  
    // Called when the value of the data process variable changes
    static void dataUpdate (ProcessVariable *pv,
                            void *userarg );
    
    // Called when the value of the width or height process variable changes
    static void sizeUpdate (ProcessVariable *pv,
                            void *userarg );
  
    static void grabButtonEvent (Widget w, XtPointer closure, XEvent* event,
                                 Boolean* b)
    {
    
        TwoDProfileMonitor *me = (TwoDProfileMonitor *) closure;
        // normalize dimensions (button and motion events store x,y in the
        // same location)
        event->xbutton.x += me->getX0 ();
        event->xbutton.y += me->getY0 ();
    
        // now send this event to EDM
        XtDispatchEventToWidget (me->actWin->executeWidget, event);
        *b = False; // terminate this "dispatch path"
    }
  

    virtual int draw ( void ); 
  
    virtual int drawActive ( void ); 
  
    // called in response to "cut" command
    virtual int erase ( void );  
  
    virtual int activate (int pass, void *ptr);
  
    // apply the results of either "Apply" or "OK" buttons
    void applyEditChanges (void);
  
    // user hit the "OK" button on the edit popup
    static void editOK (Widget w,
                        XtPointer client,
                        XtPointer call );
    // user hit the "Apply" button on the edit popup
    static void editApply (Widget w,
                           XtPointer client,
                           XtPointer call );
    // user hit the "Cancel" button on the edit popup
    static void editCancel (Widget w,
                            XtPointer client,
                            XtPointer call );
 
    // user hit the "Cancel" button on the edit popup during widget creation
    static void editCancelCreate (Widget w,
                                  XtPointer client,
                                  XtPointer call );
 
    // "helper" function for editing widget under a variety of circumstances
    void editCommon ( activeWindowClass *actWin, entryFormClass *ef,
                      int create = 0 ) ;

    // user created object from GUI (after drawing rectangle)
    virtual int createInteractive (activeWindowClass *actWin,
                                   int x,
                                   int y,
                                   int w,
                                   int h );

    // object created from saved description on disk
    virtual int createFromFile (FILE *fptr,
                                char *name,
                                activeWindowClass *actWin );

    // object created from ????
    virtual int importFromXchFile (FILE *fptr,
                                   char *name,
                                   activeWindowClass *actWin );

    // save to disk
    virtual int save ( FILE *fptr );

    virtual int edit ( void );
  
    // ========================================================
    // execute mode widget functions
  
    virtual int deactivate ( int pass );

    virtual int expandTemplate (int numMacros,
                           char *macros[],
                           char *expansions[] ) 
    {

        expStringClass tmpStr;

        tmpStr.setRaw( dataPvStr.getRaw() );
        tmpStr.expand1st( numMacros, macros, expansions );
        dataPvStr.setRaw( tmpStr.getExpanded() );

        tmpStr.setRaw( widthPvStr.getRaw() );
        tmpStr.expand1st( numMacros, macros, expansions );
        widthPvStr.setRaw( tmpStr.getExpanded() );

        tmpStr.setRaw( heightPvStr.getRaw() );
        tmpStr.expand1st( numMacros, macros, expansions );
        heightPvStr.setRaw( tmpStr.getExpanded() );

        return 1;
    
    }
  
    virtual int expand1st (int numMacros,
                           char *macros[],
                           char *expansions[] ) 
    {
        int stat; 
        stat = dataPvStr.expand1st (numMacros, macros, expansions);
        if (stat)
            stat = widthPvStr.expand1st (numMacros, macros, expansions);
        if (stat)
            stat = heightPvStr.expand1st (numMacros, macros, expansions);
        return stat;
    
    }
  
    // currently only used by mux devices (which we are not) 
    virtual int expand2nd (int numMacros,
                           char *macros[],
                           char *expansions[] )
    {
        int stat; 
        stat = dataPvStr.expand2nd (numMacros, macros, expansions);
        if (stat)
            stat = widthPvStr.expand2nd (numMacros, macros, expansions);
        if (stat)
            stat = heightPvStr.expand2nd (numMacros, macros, expansions);
        return stat;
    
    }
  
    // currently only used by mux devices (which we are not) 
    virtual int containsMacros ( void )
    {
        return dataPvStr.containsPrimaryMacros () ? 1 : 0;
    }

    template<class T> double * to_double (unsigned size, const T * data)
    {
        double * temp = (double *) malloc (sizeof (double) * size);
        for (unsigned s = 0; s < size; ++s)
        {
            temp[s] = data[s];
        }
        return temp;
    }

    static double *int_to_double (size_t s, const int *i)
    {

        double *d = (double *) malloc (sizeof (double)* s);
        if (!d) return d;
        for (size_t index = 0; index < s; index++)
        {
            d[index] = i[index];
        }
        return d;
    }
    // here is where we deal with updating the execute-mode widget (data and
    //  connect state)
    virtual void executeDeferred ( void )
    {
        int ni, nc, nr;
        struct timeval tv;
#ifdef DEBUG
        printf ("Start of TwoDProfMon::executeDeferred\n");
#endif
        if (actWin->isIconified) 
        {
#ifdef DEBUG
             printf ("TwoDProfMon::execDeferred return - window iconified\n");
#endif
             return;
        }
        // The widget may not be able to handle video data as fast as it is
        // produced (particularly if it is displaying a large image on a
        // remote X display).  If this happens, unprocessed channel access
        // data is queued for transmission in the IOC and eventually some 
        // has to be discarded.  What is lost is not necessarily video data, 
        // with the result that other edm widgets appear to stop working.
        // To avoid this happening, we must make sure we receive all screen
        // images that are produced and ignore any we haven't time to put out.
        // The following code does this.
        gettimeofday (&tv, 0);
        unsigned long elapsedusec = (tv.tv_sec - lasttv.tv_sec) * 1000000
                                     + tv.tv_usec - lasttv.tv_usec;
#ifdef DEBUG
        printf ("TwoDProfMon::executeDeferred - elapsed time = %lu\n",
                elapsedusec);
#endif
        if (elapsedusec < average_time_usec * VIDEO_MAX_LOAD_FACTOR)
        {
#ifdef DEBUG
            printf ("TwoDProfMon::execDef return - elapsed time too short\n");
#endif
            return;
        }
        lasttv.tv_sec = tv.tv_sec;
        lasttv.tv_usec = tv.tv_usec;

#ifdef DEBUG
        printf ("Start of processing for TwoDProfMon::executeDeferred\n");
#endif
        actWin->appCtx->proc->lock ();
        nc = needConnectInit; needConnectInit = 0;
        ni = needInfoInit; needInfoInit = 0;
        nr = needRefresh; needRefresh = 0;
        actWin->remDefExeNode (aglPtr);
        actWin->appCtx->proc->unlock ();
        if (!activeMode)
        {
#ifdef DEBUG
            printf ("TwoDProfMon::executeDeferred return - not active mode\n");
#endif
            return;
        }
        lasttv.tv_sec = tv.tv_sec;
        lasttv.tv_usec = tv.tv_usec;


        if (nc)
        {
            ni = 1;
        }
 
        if (ni)
        {
            active = 1;
            init = 1;
            if (initialDataConnection)
            {
                initialDataConnection = 0;
#ifdef DEBUG
                printf (
                    "TwoDProfMon::execDeferred - add data value callback\n");
#endif
                dataPv->add_value_callback ( dataUpdate, this );
            }
            if (initialWidthConnection)
            {
                initialWidthConnection = 0;
#ifdef DEBUG
                printf (
                    "TwoDProfMon::execDeferred - add width value callback\n");
#endif
                widthPv->add_value_callback ( sizeUpdate, this );
            }
            if (initialHeightConnection)
            {
                initialHeightConnection = 0;
#ifdef DEBUG
                printf (
                    "TwoDProfMon::execDeferred - add height value callback\n");
#endif
                heightPv->add_value_callback ( sizeUpdate, this );
            }
        }
        
        // need to check if we're being updated because of width change
#ifdef DEBUG
        printf ("executeDeferred (TwoDMon.cc) - pvBasedDataWidth = %d\n",
                 pvBasedDataWidth);
        printf ("executeDeferred (TwoDMon.cc) - widthPv = %x\n", (int) widthPv);
        if ( widthPv ) {
          printf ("executeDeferred (TwoDMon.cc) - is_valid = %d\n",
                   widthPv->is_valid ());
	}
#endif
        if (pvBasedDataSize && widthPv && widthPv->is_valid ())
        {
#ifdef DEBUG
            printf ("executeDeferred (TwoDMon.cc) - pv based width\n");
#endif
            dataWidth = (int) widthPv->get_int ();
#if 0
            switch (widthPv->get_type ().type)
            {
            case ProcessVariable::Type::real:
                dataWidth = (int) widthPv->get_double ();
                break;
            case ProcessVariable::Type::integer:
                dataWidth = (int) widthPv->get_int ();
                break;
            default:
                dataWidth = -1;
                break;
            }
#endif
#ifdef DEBUG
            printf ("executeDeferred (TwoDMon.cc) - width = %d\n", dataWidth);
#endif
            if (heightPv && heightPv->is_valid ())
            {
                dataHeight = (int) heightPv->get_int ();
#if 0
                switch (heightPv->get_type ().type)
                {
                case ProcessVariable::Type::real:
                    dataHeight = (int) heightPv->get_double ();
                    break;
                case ProcessVariable::Type::integer:
                    dataHeight = (int) heightPv->get_int ();
                    break;
                default:
                    dataHeight = -1;
                }
#endif
            }
        }
    
        if (dataWidth <= 0 || dataWidth > VIDEO_MAX_DATA_WIDTH ||
            dataHeight < 0 || dataHeight > VIDEO_MAX_DATA_HEIGHT)
        {
            printf (
                "TwoDProfMon::execDef - return dataWidth %d dataHeight %d\n",
                dataWidth, dataHeight);
            return; 
        }   

#if 0
        if ((dataHeight != 0) &&
            ((dataHeight * dataWidth) != (int)dataPv->get_dimension ()))
        {
            // Data height or width has changed.  Ignore the current data which
            // is the wrong size.  Cancel subscription and restart it to make
            // IOC / Channel Access send data of the correct size.
#ifdef DEBUG
            printf (
                "TwoDProfMon::execDef - resubscribe - new dataW %d dataH %d\n",
                dataWidth, dataHeight);
#endif
            //actWin->appCtx->proc->lock ();
            dataPv->remove_value_callback ( dataUpdate, this );
            dataPv->remove_conn_state_callback (monitorDataConnectState, this);
            dataPv->release ();
            dataPv = the_PV_Factory->create ( dataPvStr.getExpanded () );
            dataPv->add_conn_state_callback ( monitorDataConnectState, this );
            dataPv->add_value_callback ( dataUpdate, this );
            //actWin->appCtx->proc->unlock ();
            return;
        }
        //actWin->appCtx->proc->lock ();
#endif

#ifdef DEBUG
        printf ("executeDeferred (TwoDMon.cc) - before data PV type switch\n");
#endif
        if (dataPv && dataPv->is_valid ())
        {
      
            switch (dataPv->get_type ().type)
            {
        
            case ProcessVariable::Type::real:
                // printf ("real\n");
                // ??????????
                //widgetNewDisplayData (
                //    wd, dataPv->get_time_t (), dataPv->get_nano (),
                //    (unsigned long) w, (unsigned long) h, dataWidth,
                //    (dataHeight > 0 ? dataHeight 
                //                    : dataPv->get_dimension () / dataWidth),
                //    (const double *) dataPv->get_double_array ());

                img->update( dataWidth,
                 (dataHeight > 0 ? dataHeight 
                                   : dataPv->get_dimension () / dataWidth),
                 (double *) dataPv->get_double_array() );
		if ( img->validImage() ) {
                  XPutImage( actWin->d, drawable(actWin->executeWidget),
                   actWin->executeGc.normGC(), img->ximage(),
		   0, 0, x, y, w, h );
		}

                break;

            case ProcessVariable::Type::text:
                {
                    double * temp = to_double<char>(
                                        dataPv->get_dimension (),
                                        dataPv->get_char_array ());
#ifdef DEBUG
                    printf ("TwoDProfMon::execDef calling widgetNewDispData\n");
#endif
		    // ????????????????????
                    //widgetNewDisplayData (
                    //    wd, dataPv->get_time_t (), dataPv->get_nano (),
                    //    (unsigned long)w, (unsigned long) h, dataWidth,
                    //    (dataHeight > 0 ? dataHeight 
                    //                    : dataPv->get_dimension() / dataWidth),
                    //    temp);
                   img->update( dataWidth,
                    (dataHeight > 0 ? dataHeight 
                                      : dataPv->get_dimension () / dataWidth),
                    temp );
		   free (temp);
                   if ( img->validImage() ) {
                     XPutImage( actWin->d, drawable(actWin->executeWidget),
                      actWin->executeGc.normGC(), img->ximage(),
                      0, 0, x, y, w, h );
		   }
                }
                break;

            case ProcessVariable::Type::integer:
                {

                    double *temp;

                    if ( dataPv->get_specific_type().type == ProcessVariable::specificType::shrt ) {
                      temp = to_double<short>( dataPv->get_dimension(), dataPv->get_short_array() );
                    }
                    else if ( dataPv->get_specific_type().type == ProcessVariable::specificType::integer ) {
                      temp = int_to_double( dataPv->get_dimension(), dataPv->get_int_array() );
                    }

		    // ?????????????????
                    //widgetNewDisplayData (
                    // wd, dataPv->get_time_t (), dataPv->get_nano (),
                    // (unsigned long) w, (unsigned long) h, dataWidth,
                    // (dataHeight > 0 ? dataHeight 
                    //                 : dataPv->get_dimension() / dataWidth),
                    // temp);
                    img->update( dataWidth,
                     (dataHeight > 0 ? dataHeight 
                                       : dataPv->get_dimension () / dataWidth),
                     temp );
                    free (temp);
                    if ( img->validImage() ) {
                      XPutImage( actWin->d, drawable(actWin->executeWidget),
                       actWin->executeGc.normGC(), img->ximage(),
                       0, 0, x, y, w, h );
		    }
                }
                break;

            default:
                // nothing to do!
                break;
            }
	    // ??????????????????????
            //widgetNewDisplayInfo (wd, true, dataPv->get_status (),
            //                      dataPv->get_severity ());
        }
        else
        {
            // ????????????????
            //widgetNewDisplayInfo (wd, false, 0, 0);
        }
    
        // actWin->remDefExeNode (aglPtr);
    
        // actWin->appCtx->proc->unlock ();
    
        // Get approx average elapsed time for call - no point in being precise
        gettimeofday (&tv, 0);
        elapsedusec = (tv.tv_sec - lasttv.tv_sec) * 1000000 + tv.tv_usec - lasttv.tv_usec;
        if (!average_time_usec)
            average_time_usec = elapsedusec;
        else
            average_time_usec = (average_time_usec * 9 + elapsedusec) / 10;
#ifdef DEBUG
        printf ("End of TwoDProfMon::execDef - average elapsed time = %lu\n",
                average_time_usec);
#endif
    }
  
    // let the user select among a field of functional names for drag-n-drop
    virtual char *firstDragName ( void ){ return "data PV"; };
    virtual char *nextDragName ( void ){ return NULL; } ;
  

    virtual char *dragValue ( int i )
    { return i ? NULL : dataPvStr.getExpanded (); };
  
    // yes we use PVs, therefore we support drag-n-drop and info dialogs
    virtual int atLeastOneDragPv (int x,
                                  int y ){ return 1; };
 
    // this one is to support an info dialog about widget-related PVs
    virtual void getPvs (int max,
                         ProcessVariable *pvs[],
                         int *n ){ *n = 1; pvs[0] = dataPv;};

    virtual char *getSearchString (
      int i
    ) {

      if ( i == 0 ) {
        return dataPvStr.getRaw();
      }

      return NULL;

    }

    virtual void replaceString (
      int i,
      int max,
      char *string
      ) {

      if ( i == 0 ) {
        dataPvStr.setRaw( string );
      }

    }
  
    // This is a funny interface. It seems that the idea is to have a generic
    // interface to all widgets with a "standard" set of parameters (e.g.
    // control PV).  However, there doesn't seem to be a clean way to associate
    // widget values with their generic equivalents. I.e. the generic "control
    // PV" name is maintained in actWin->allSelectedCtlPvName[0]. One *could*
    // use that data storage for the control PV for a widget, but it is not
    // obvious (to me) that that interface is encouraged (or is guaranteed to
    // be supported in the future).

    // what I will do here is what most of the widgets do, which is to pick
    // out of the user supplied values anything that I see an obvious equivalent
    // in my "private" parameters, and copy in that data. Note that my "private"
    // equivalent might change (via the "edit" popup, and that change will not
    // be reflected in this popup. I *could* "blank out" the fields I use after
    // copying out the data, but that is not the behavior implemented in other
    // widgets.

    // I think that the *good* thing about this interface is that it hints at
    // what parameters each widget should support

    virtual void changePvNames (int flag,
                                int numCtlPvs,
                                char *ctlPvs[],
                                int numReadbackPvs,
                                char *readbackPvs[],
                                int numNullPvs,
                                char *nullPvs[],
                                int numVisPvs,
                                char *visPvs[],
                                int numAlarmPvs,
                                char *alarmPvs[] )
    {

        if ((flag & ACTGRF_READBACKPVS_MASK) && numReadbackPvs)
            dataPvStr.setRaw (readbackPvs[0]);

    }

    // see previous comments
  
    virtual void changeDisplayParams (unsigned int flag,
                                      char *fontTag,
                                      int alignment,
                                      char *ctlFontTag,
                                      int ctlAlignment,
                                      char *btnFontTag,
                                      int btnAlignment,
                                      int textFgColour,
                                      int fg1Colour,
                                      int fg2Colour,
                                      int offsetColour,
                                      int bgColour,
                                      int topShadowColour,
                                      int botShadowColour )
    {

        if (flag & ACTGRF_FONTTAG_MASK) textFontTag = fontTag; // strcpy???
        if (flag & ACTGRF_ALIGNMENT_MASK) textAlignment = alignment; 
        if (flag & ACTGRF_TEXTFGCOLOR_MASK) textColour = textFgColour;
    }
  
private:
  
    TwoDProfileMonitor &operator=(const TwoDProfileMonitor &s);
  
};


// class for read/write tags
// I like to break this out because it forces me to
// enumerate all the data memebers that are saved
// It is more work, but less error-prone (IMHO)
class TwoDProfileMonitorTags : public tagClass 
{

public:
    TwoDProfileMonitorTags (void){ init (); }
    ~TwoDProfileMonitorTags (){}
  
    int read (TwoDProfileMonitor* mon,
              FILE *fptr,
              int *x, int *y, int *w, int *h, int *nBitsPerPixel,
              expStringClass *dataPvStr,
              expStringClass *widthPvStr,
              expStringClass *heightPvStr,
              int *dataWidth,
              int *pvBasedDataSize)
    {
        int major, minor, release;
        int stat;
        loadR ("beginObjectProperties" );
        loadR ( "major", &major );
        loadR ( "minor", &minor );
        loadR ( "release", &release );
        loadR ( "x", x );
        loadR ( "y", y );
        loadR ( "w", w );
        loadR ( "h", h );
        loadR ( "dataPvStr", dataPvStr, (char *) "" );
        loadR ( "widthPvStr", widthPvStr, (char *) "" );
        loadR ( "heightPvStr", heightPvStr, (char *) "" ); 
        loadR ( "dataWidth", dataWidth);
        loadR ( "pvBasedDataSize", pvBasedDataSize);
        loadR ( "nBitsPerPixel", nBitsPerPixel );
        stat = readTags ( fptr, "endObjectProperties" );
        if (major > VIDEO_MAJOR_VERSION ||
            (major == VIDEO_MAJOR_VERSION && minor > VIDEO_MINOR_VERSION))
        {
             // edl file was produced by a more recent version of edm than 
             // this and we can't predict the future
             mon->postIncompatable ();
             return 0;
        }
        if (major < VIDEO_MAJOR_VERSION)
        {
             // Major version changes render old edl files incompatible
             mon->postIncompatable ();
             return 0;
        }
        return stat;
    }
    int write (FILE *fptr,
               int *x, int *y, int *w, int *h, int *nBitsPerPixel,
               expStringClass *dataPvStr,
               expStringClass *widthPvStr,
               expStringClass *heightPvStr,
               int *dataWidth,
               int *pvBasedDataSize)
    {
        int major, minor, release;
        major = VIDEO_MAJOR_VERSION;
        minor = VIDEO_MINOR_VERSION;
        release = VIDEO_RELEASE;
        loadW ("beginObjectProperties" );
        loadW ( "major", &major );
        loadW ( "minor", &minor );
        loadW ( "release", &release );
        loadW ( "x", x );
        loadW ( "y", y );
        loadW ( "w", w );
        loadW ( "h", h );
        loadW ( "dataPvStr", dataPvStr, (char *) "" );
        loadW ( "widthPvStr", widthPvStr, (char *) "" );
        loadW ( "heightPvStr", heightPvStr, (char *) "" ); 
        loadW ( "dataWidth", dataWidth);
        loadW ( "pvBasedDataSize", pvBasedDataSize);
        loadW ( "nBitsPerPixel", nBitsPerPixel );
        loadW ( "endObjectProperties" );
        loadW ( "" );
  
        return writeTags ( fptr );
    }

private:
    TwoDProfileMonitorTags (const TwoDProfileMonitorTags &s);
    TwoDProfileMonitorTags &operator= (const TwoDProfileMonitorTags &s);

};


// stuff needed for EDM to load from DLL
extern "C" 
{

    void *create_TwoDProfileMonitorClassPtr ( void )
    {

        return (new TwoDProfileMonitor ());

    }

}

extern "C"
{

    void *clone_TwoDProfileMonitorClassPtr ( void *s )
    {

        return (new TwoDProfileMonitor (*(TwoDProfileMonitor *)s));

    }

}

// Support registration
#include "environment.str"

typedef struct libRecTag
{
    char *className;
    char *typeName;
    char *text;
} libRecType, *libRecPtr;

static int libRecIndex = 0;

static libRecType libRec[] = 
{
    { "TwoDProfileMonitorClass", global_str2, "Hoff Video" }
};

extern "C" 
{

    char *version ( void ) {

    static char *v = VERSION;

      return v;

    }

    char *author ( void ) {

    static char *a = "Lawrence T. Hoff (hoff@bnl.gov)";

      return a;

    }

    int firstRegRecord (char **className,
                        char **typeName,
                        char **text )
    {
    
        libRecIndex = 0;
    
        *className = libRec[libRecIndex].className;
        *typeName = libRec[libRecIndex].typeName;
        *text = libRec[libRecIndex].text;
    
        return 0; // OK
    
    }
  
    int nextRegRecord (char **className,
                       char **typeName,
                       char **text )
    {
    
        if (libRecIndex >= sizeof (libRec)/sizeof (libRecType) - 1)
            return -1; // done
        ++libRecIndex;
    
        *className = libRec[libRecIndex].className;
        *typeName = libRec[libRecIndex].typeName;
        *text = libRec[libRecIndex].text;
    
        return 0; // OK
    
    }
  
}

void TwoDProfileMonitor::constructCommon (void)
{

    /* start off not knowing image data width */
    pvBasedDataSize = 0;
    dataWidth = -1;
    dataHeight = 0; // 0 if no PV supplied which is OK, -1 if invalid PV
    activeMode = 0;

    // ???????????????
    //wd = widgetCreate ();
    //twoDWidget = NULL;

    name = "TwoDProfileMonitorClass";
  
    dataPvStr.setRaw ("");
    widthPvStr.setRaw ("");
    heightPvStr.setRaw ("");
  
    dataPv = NULL;
    widthPv = NULL;
    heightPv = NULL;

    strcpy (dataPvBuf, ""); // just to be safe
    strcpy (widthPvBuf, ""); // just to be safe
    strcpy (heightPvBuf, ""); // just to be safe
    //twoDWidget = NULL;

    average_time_usec = 0;

#if (0)
    // text stuff (for edit mode drawing)
    char *textFontTag;
    int textAlignment;
    int textColour;
#endif

    img = NULL;

}

TwoDProfileMonitor::TwoDProfileMonitor (void) : activeGraphicClass () 
{ 
  
    constructCommon ();
    checkBaseClassVersion( activeGraphicClass::MAJOR_VERSION, name );

}


TwoDProfileMonitor::TwoDProfileMonitor (const TwoDProfileMonitor &s)
{ 
  
    // clone base class data
    // why doesn't activeGraphicClass copy constructor do this?
    activeGraphicClass::clone ( &s );

    constructCommon ();

    // does the copy constructor work?
    // dataPvStr = s.dataPvStr;
    // widthPvStr = s.widthPvStr;
    dataPvStr.setRaw (s.dataPvStr.rawString);
    widthPvStr.setRaw (s.widthPvStr.rawString);
    heightPvStr.setRaw (s.heightPvStr.rawString);

    pvBasedDataSize = s.pvBasedDataSize;
    dataWidth = s.dataWidth;

    doAccSubs( dataPvStr );

}

TwoDProfileMonitor::~TwoDProfileMonitor (void) {

  // ?????????????????
  //widgetDestroy (wd);

}

// called when widget is made active as edm changes to "execute" mode,
// pass values are 0-6
int TwoDProfileMonitor::activate ( int pass,
				   void *ptr )
{

    switch (pass)
    {
    case 1:
        opComplete = 0;
	aglPtr = ptr;
        break;

    case 2:
        if ( !opComplete ) {
	  _edmDebug();
          img = new imageClass( actWin->d, actWin->ci->getColorMap(),
           actWin->executeGc.normGC(), w, h, nBitsPerPixel );
          opComplete = 1;
        }

        initialDataConnection = 1;
        initialWidthConnection = 0;
        initialHeightConnection = 0;
        needConnectInit = needInfoInit = needRefresh = 0;
        pvNotConnectedMask = active = init = 0;
        activeMode = 1;

        if (!dataPvStr.getExpanded () ||
            blankOrComment (dataPvStr.getExpanded ()))
        {
            dataPvExists = 0;
        }
        else
        {
            dataPvExists = 1;
            pvNotConnectedMask |= 1;
        }

        if (!pvBasedDataSize)
        {
            widthPvExists = 0;
            heightPvExists = 0;
        }
        else
        {
            if (!widthPvStr.getExpanded () ||
                blankOrComment (widthPvStr.getExpanded ()))
            {
                widthPvExists = 0;
            }
            else
            {
                widthPvExists = 1;
                initialWidthConnection = 1;
                pvNotConnectedMask |= 2;
            }
            if (!heightPvStr.getExpanded () ||
                blankOrComment (heightPvStr.getExpanded ()))
            {
                heightPvExists = 0;
            }
            else
            {
                heightPvExists = 1;
                initialHeightConnection = 1;
                pvNotConnectedMask |= 4;
            }
        }

#ifdef DEBUG
        printf (
            "TwoDProfileMonitor::activate pass 1 - pvNotConnectedMask = %d\n",
            pvNotConnectedMask);
#endif
        break;
    
        // connect PVs during pass 2
    case 3:
        {
            // assume the best!
            pvColour.setColorIndex ( actWin->defaultTextFgColor, actWin->ci );
      

            if (!dataPvExists) 
                break; // don't bother the factory

            dataPv = the_PV_Factory->create ( dataPvStr.getExpanded () );
            if ( dataPv )
            {
#ifdef DEBUG             
                printf (
                    "TwoDProfMon::activate pass 2 - add data connect cb\n"); 
#endif
                dataPv->add_conn_state_callback (monitorDataConnectState,
                                                 this);
                //dataPv->add_value_callback ( pvUpdate, this );
            }
     
            if (!widthPvExists)
                break; // no need to set up width PV
            // printf ("activate (TwoDMon.cc) - width PV = %s = %s\n", 
            //         widthPvStr.getRaw(),
            //         widthPvStr.getExpanded());
            widthPv = the_PV_Factory->create ( widthPvStr.getExpanded () );
            if ( widthPv )
            {
#ifdef DEBUG
                printf (
                    "TwoDProfMon::activate pass 2 - adding width connect cb\n");
#endif
                widthPv->add_conn_state_callback (monitorWidthConnectState,
                                                  this);
                //widthPv->add_value_callback ( pvUpdate, this );
            }
            if (!heightPvExists)
                break; // no need to set up height PV
            // printf ("activate (TwoDMon.cc) - height PV = %s = %s\n", 
            //         heightPvStr.getRaw(),
            //         heightPvStr.getExpanded());
            heightPv = the_PV_Factory->create ( heightPvStr.getExpanded () );
            if ( heightPv )
            {
#ifdef DEBUG
                printf (
                    "TwoDProfMon::activate pass 2 - adding height connect cb\n");
#endif
                heightPv->add_conn_state_callback (monitorHeightConnectState,
                                                  this);
                //heightPv->add_value_callback ( pvUpdate, this );
            }
#ifdef DEBUG
            if ( dataPv ) {
              printf ("activate (TwoDMon.cc) - dataPv->is_valid = %d\n",
                      dataPv->is_valid ());
	    }
            if ( widthPv ) {
              printf ("activate (TwoDMon.cc) - widthPv->is_valid = %d\n",
                      widthPv->is_valid ());
	    }
            if ( heightPv ) {
              printf ("activate (TwoDMon.cc) - heightPv->is_valid = %d\n",
                      heightPv->is_valid ());
	    }
#endif
        }
        break;

        // OK, now create the widget
    case 6:

        // create the execute-mode widget using XRT or other
        // standard Motif 2-D data widget
        // ????????????????
        //twoDWidget = widgetCreateWidget (
        //                 wd, actWin->appCtx->appContext (), actWin->d,
        //                 actWin->ci->getColorMap (), actWin->executeWidget,
        //                 x, y, h, w);

        // capture events to pass on to EDM
        //XtAddEventHandler (
        //    twoDWidget,
        //    LeaveWindowMask | EnterWindowMask | PointerMotionMask |
        //    ButtonPressMask |ButtonReleaseMask,
        //    False, grabButtonEvent, (XtPointer) this);
   
        // hand over control 
        //XtManageChild (twoDWidget);
   
        break;


    default:
        break;
    
    }
  
    return 1;
}

// user hit the "OK" button on the edit popup
void TwoDProfileMonitor::editOK (Widget w,
                                 XtPointer client,
                                 XtPointer call )
{
  
    TwoDProfileMonitor *me = (TwoDProfileMonitor *) client;

    // first apply any changes
    editApply (w, client, call); 

    me->ef.popdown ();
    me->operationComplete ();
  
}
  
// user hit the "Apply" button on the edit popup
void TwoDProfileMonitor::editApply (Widget w,
                                    XtPointer client,
                                    XtPointer call )
{
  
    TwoDProfileMonitor *me = (TwoDProfileMonitor *) client;

    me->eraseSelectBoxCorners ();
    me->erase ();

    me->x = me->xBuf;
    me->y = me->yBuf;
    me->w = me->wBuf;
    me->h = me->hBuf;
    me->nBitsPerPixel = me->nBitsPerPixelBuf;
    me->sboxX = me->xBuf;
    me->sboxY = me->yBuf;
    me->sboxW = me->wBuf;
    me->sboxH = me->hBuf;

    // do the PV name(s)
    me->dataPvStr.setRaw ( me->dataPvBuf );

    // now the width: if fixed is indicated, interpret PV string as int
    me->widthPvStr.setRaw ( me->widthPvBuf );
    if (!me->pvBasedDataSize)
         me->dataWidth = atoi ( me->widthPvBuf );
    else
         me->dataWidth = -1; // just to be safe

    me->heightPvStr.setRaw ( me->heightPvBuf );
    me->dataHeight = 0;  // correct if PV null, will be overwritten otherwise
    // support auto-save
    me->actWin->setChanged ();

  // let EDM know that "Apply" was invoked
  me->refresh ();
  
}

// user hit the "Cancel" button on the edit popup
void TwoDProfileMonitor::editCancel (Widget w,
                                     XtPointer client,
                                     XtPointer call )
{
  
    TwoDProfileMonitor *me = (TwoDProfileMonitor *) client;
    
    me->ef.popdown ();

    // no need for EDM to do anything
    me->operationCancel ();
    
}

// user hit the "Cancel" button on the edit popup
void TwoDProfileMonitor::editCancelCreate (Widget w,
                                           XtPointer client,
                                           XtPointer call )
{
  
    TwoDProfileMonitor *me = (TwoDProfileMonitor *) client;
  
    me->ef.popdown ();
  
    // remove all traces of our existence!
    me->erase ();
    me->deleteRequest = 1;
  
    // no need for EDM to do anything
    me->operationCancel ();
  
}

void TwoDProfileMonitor::editCommon ( activeWindowClass *actWin,
                                      entryFormClass *_ef, int create )
{

    // create edit box
    ef.create ( actWin->top, actWin->appCtx->ci.getColorMap (),
                &actWin->appCtx->entryFormX,
                &actWin->appCtx->entryFormY, &actWin->appCtx->entryFormW,
                &actWin->appCtx->entryFormH, &actWin->appCtx->largestH,
                "2D Profile Monitor Properties", NULL, NULL, NULL );

    xBuf = x;
    yBuf = y;
    wBuf = w;
    hBuf = h;
    nBitsPerPixelBuf = nBitsPerPixel;

    ef.addTextField ("X", 30, &xBuf);
    ef.addTextField ("Y", 30, &yBuf);
    ef.addTextField ("Widget Width", 30, &wBuf);
    ef.addTextField ("Widget Height", 30, &hBuf);
    ef.addTextField ("Bits per pixel", 30, &nBitsPerPixelBuf);
    // copy out, we'll copy in during "Apply"
    strncpy (dataPvBuf, dataPvStr.getRaw (), sizeof (dataPvBuf) - 1);
    ef.addTextField ("Data PV", 30, dataPvBuf, sizeof (dataPvBuf) - 1);

    // copy out, we'll copy in during "Apply"
    strncpy (widthPvBuf, widthPvStr.getRaw (), sizeof (widthPvBuf) - 1);
    ef.addTextField ("Data Width (Fixed/PV)", 30, widthPvBuf,
                     sizeof (widthPvBuf) - 1);
    strncpy (heightPvBuf,heightPvStr.getRaw (), sizeof (heightPvBuf) - 1);
    ef.addTextField ("Data Height PV (ignored for fixed size)",
                     30, heightPvBuf, sizeof (heightPvBuf) - 1);

    ef.addOption ("Data Size Type", "Fixed|PV-based", &pvBasedDataSize);
    // ef.addToggle ("PV-based Width", &height);

    // Map dialog box form buttons to callbacks
    ef.finished ( editOK, editApply, create ? editCancelCreate : editCancel,
                  this );

    // Required by display engine
    actWin->currentEf = _ef;

    // popup the dialog box
    ef.popup ();

}

int TwoDProfileMonitor::edit ( void )
{

    editCommon ( actWin, &ef );

    return 1;
}

int TwoDProfileMonitor::createInteractive (activeWindowClass *actWin,
                                           int x,
                                           int y,
                                           int w,
                                           int h )
{
  
    this->actWin = actWin;
    this->x = x;
    this->y = y;
    this->w = w;
    this->h = h;
    this->nBitsPerPixel = VIDEO_NBITSPERPIXEL_DEFAULT;

    draw ();
  
    editCommon ( actWin, NULL, ~0 );

    return 1;
}

int TwoDProfileMonitor::createFromFile (FILE *fptr,
                                        char *name,
                                        activeWindowClass *actWin )
{
  
    this->actWin = actWin;
  
    // use tag class and name to read from file
    TwoDProfileMonitorTags tag;
  
    nBitsPerPixel = 8;  
    if ( !(1 & tag.read ( this,
                          fptr, &x, &y, &w, &h, &nBitsPerPixel, &dataPvStr,
                          &widthPvStr, &heightPvStr,
                          &dataWidth, &pvBasedDataSize ) ) )
    {
        actWin->appCtx->postMessage ( tag.errMsg () );
    }
  
    updateDimensions ();
    initSelectBox ();
  
    return 1;
}

// What is an exchange file?
int TwoDProfileMonitor::importFromXchFile (FILE *fptr,
                                           char *name,
                                           activeWindowClass *actWin )
{

    cerr << "Import from eXchange file not supported" << endl;

    return 0;
}

int TwoDProfileMonitor::save ( FILE *fptr )
{
    // use tag class to serialize data
    TwoDProfileMonitorTags tag;
    return tag.write ( fptr, &x, &y, &w, &h, &nBitsPerPixel, &dataPvStr,
                       &widthPvStr, &heightPvStr,
                       &dataWidth, &pvBasedDataSize );
  
}

// called any time the widget needs to draw or re-draw itself
// in edit-mode 
int TwoDProfileMonitor::draw (void)
{ 
  
    // draw rectangle using X primitives
    XFillRectangle ( actWin->d, XtWindow (actWin->drawWidget),
                     actWin->drawGc.eraseGC (), x, y, w, h );
    XDrawRectangle ( actWin->d, XtWindow (actWin->drawWidget),
                     actWin->drawGc.normGC (), x, y, w, h );
    // Draw label text (crude because we can escape widget boundaries)
    XDrawImageString ( actWin->d, XtWindow (actWin->drawWidget),
                       actWin->drawGc.normGC (), x + 5, y + h / 2,
                       dataPvStr.getRaw (), strlen (dataPvStr.getRaw ()) );

    return 1;
    //return activeGraphicClass::draw (); 
} ;

// erase widget in responce to "cut" command
// in edit-mode 
int TwoDProfileMonitor::erase (void)
{ 

    // draw rectangle using X primitives
    XDrawRectangle ( actWin->d, XtWindow (actWin->drawWidget),
                    actWin->drawGc.eraseGC (), x, y, w, h );

    return 1;
    //return activeGraphicClass::erase (); 
} ;

int TwoDProfileMonitor::drawActive (void)
{

  if ( img->validImage() ) {
    XPutImage( actWin->d, drawable(actWin->executeWidget),
     actWin->executeGc.normGC(), img->ximage(),
     0, 0, x, y, w, h );
  }

  return 1;

}

// returning to edit mode, pass values are 1 and 2
int TwoDProfileMonitor::deactivate ( int pass )
{ 
    active = 0;
    activeMode = 0;

    if ( dataPv != NULL )
    {
        actWin->appCtx->proc->lock ();
        dataPv->remove_conn_state_callback ( monitorDataConnectState, this );
#ifdef DEBUG
        printf ("TwoDProfileMonitor::deactivate - removing data callback\n");
#endif
        dataPv->remove_value_callback ( dataUpdate, this );
        dataPv->release ();
        dataPv = NULL;
        actWin->appCtx->proc->unlock ();
    }

    if ( widthPv != NULL )
    {

        actWin->appCtx->proc->lock ();
        widthPv->remove_conn_state_callback ( monitorWidthConnectState, this );
        widthPv->remove_value_callback ( sizeUpdate, this );
        widthPv->release ();
        widthPv = NULL;
        actWin->appCtx->proc->unlock ();
    }

    if ( heightPv != NULL )
    {

        actWin->appCtx->proc->lock ();
        heightPv->remove_conn_state_callback ( monitorHeightConnectState, this );
        heightPv->remove_value_callback ( sizeUpdate, this );
        heightPv->release ();
        heightPv = NULL;
        actWin->appCtx->proc->unlock ();
    }
    // disconnect PV timeout on pass 1
    if ( pass == 1 )
    {
        // disable deferred processing before anything else
        actWin->appCtx->proc->lock ();
        actWin->remDefExeNode (aglPtr);
        actWin->appCtx->proc->unlock ();
    }

    // now turn off the Widget
    if ( pass == 2 )
    {
#ifdef DEBUG
        printf ("TwoDProfileMonitor::deactivate - pass 2\n");
#endif
	// ???????????????????
        //widgetNewDisplayInfo (wd, false, 0, 0);
        //XtUnmanageChild (twoDWidget);
        //XtDestroyWidget (twoDWidget);
	// ????????????????????
        //widgetDestroyWidget (wd);

	_edmDebug();
	img->destroy();

    }
  
    return 1;
    //return activeGraphicClass::deactivate (pass); 
}


void TwoDProfileMonitor::monitorDataConnectState (ProcessVariable *pv,
                                                   void *userarg )
{

    TwoDProfileMonitor *me = ( TwoDProfileMonitor *) userarg;

    me->actWin->appCtx->proc->lock ();
    if (me->activeMode)
    {
        if (pv->is_valid())
        {
            me->pvNotConnectedMask &= ~((unsigned char) 1); 
#ifdef DEBUG
            printf ("TwoDProfMon::monDataConState - set pvNotConnMask to %d\n",
                    me->pvNotConnectedMask);
#endif
            if (!me->pvNotConnectedMask)
            {
                // All PVs connected
                me->needConnectInit = 1;
                me->actWin->addDefExeNode (me->aglPtr);
            }
        }
        else
        {
            me->pvNotConnectedMask |= 1;
            me->active = 0;
            me->bufInvalidate();
            me->needDraw = 1;
            me->actWin->addDefExeNode (me->aglPtr);
        }
    }
    me->actWin->appCtx->proc->unlock ();
}

void TwoDProfileMonitor::monitorWidthConnectState (ProcessVariable *pv,
                                                   void *userarg )
{

    TwoDProfileMonitor *me = ( TwoDProfileMonitor *) userarg;

    me->actWin->appCtx->proc->lock ();
    if (me->activeMode)
    {
        if (pv->is_valid())
        {
            me->pvNotConnectedMask &= ~((unsigned char) 2); 
#ifdef DEBUG
            printf ("TwoDProfMon::monWidthConState - set pvNotConMask to %d\n",
                    me->pvNotConnectedMask);
#endif
            if (!me->pvNotConnectedMask)
            {
                // All PVs connected
                me->needConnectInit = 1;
                me->actWin->addDefExeNode (me->aglPtr);
            }
        }
        else
        {
            me->pvNotConnectedMask |= 2;
            me->active = 0;
            me->bufInvalidate();
            me->needDraw = 1;
            me->actWin->addDefExeNode (me->aglPtr);
        }
    }
    me->actWin->appCtx->proc->unlock ();
}

void TwoDProfileMonitor::monitorHeightConnectState (ProcessVariable *pv,
                                                   void *userarg )
{

    TwoDProfileMonitor *me = ( TwoDProfileMonitor *) userarg;

    me->actWin->appCtx->proc->lock ();
    if (me->activeMode)
    {
        if (pv->is_valid())
        {
            me->pvNotConnectedMask &= ~((unsigned char) 4); 
#ifdef DEBUG
            printf ("TwoDProfMon::monHeightConState - set pvNotConMsk to %d\n",
                    me->pvNotConnectedMask);
#endif
            if (!me->pvNotConnectedMask)
            {
                // All PVs connected
                me->needConnectInit = 1;
                me->actWin->addDefExeNode (me->aglPtr);
            }
        }
        else
        {
            me->pvNotConnectedMask |= 2;
            me->active = 0;
            me->bufInvalidate();
            me->needDraw = 1;
            me->actWin->addDefExeNode (me->aglPtr);
        }
    }
    me->actWin->appCtx->proc->unlock ();
}

void TwoDProfileMonitor::dataUpdate (ProcessVariable *pv,
                                   void *userarg )
{
    TwoDProfileMonitor *me = ( TwoDProfileMonitor *) userarg;

    me->actWin->appCtx->proc->lock ();
    me->actWin->addDefExeNode (me->aglPtr);
    me->actWin->appCtx->proc->unlock ();
}


void TwoDProfileMonitor::sizeUpdate (ProcessVariable *pv,
                                      void *userarg )
{
    // This function no longer does anything.  Leave it in in case we need it
    // again later.
#ifdef COMMENT_OUT
    TwoDProfileMonitor *me = ( TwoDProfileMonitor *) userarg;
    // Delete the data PV and recreate it.
    // Hopefully this will reinitialise the element count to match
    // the new size.
    me->actWin->appCtx->proc->lock ();
    me->dataPv->remove_value_callback ( dataUpdate, me );
    me->dataPv->remove_conn_state_callback ( monitorDataConnectState, me );
    me->dataPv->release ();
    me->dataPv = the_PV_Factory->create ( me->dataPvStr.getExpanded () );
    me->dataPv->add_conn_state_callback ( monitorDataConnectState, me );
    me->dataPv->add_value_callback ( dataUpdate, me );
    me->actWin->appCtx->proc->unlock ();
#endif
}
