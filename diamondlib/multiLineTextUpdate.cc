// -*- C++ -*-
// EDM textupdate Widget
//
// Ideas for TextEntry stolen from MEDM
//
// kasemir@lanl.gov

// #define DEBUG_TEXTWIDGETS

#include "multiLineTextUpdate.h"
#include "app_pkg.h"
#include "act_win.h"
#include "pv_factory.h"
#include "cvtFast.h"

static int g_transInit = 1;
static XtTranslations g_parsedTrans;

static char g_dragTrans[] =
  "#override\n\
  ~Ctrl~Shift<Btn2Down>: startDrag()\n\
  Ctrl~Shift<Btn2Up>: selectActions()\n\
  Shift Ctrl<Btn2Down>: pvInfo()\n\
  Shift~Ctrl<Btn2Up>: selectDrag()";

static XtActionsRec g_dragActions[] =
{
    { "startDrag", (XtActionProc) drag },
    { "pvInfo", (XtActionProc) pvInfo },
    { "selectActions", (XtActionProc) selectActions },
    { "selectDrag", (XtActionProc) selectDrag }
};

// Stolen from dm2k updateMonitors.c, Mark Andersion, Frederick Vong:
// Display number in enineering notaion (power of ten is n*3)
static void localCvtDoubleToExpNotationString (double value,
                                               char *textField,
                                               unsigned short precision)
{
    double absVal, newVal;
    bool minus;
    int exp, k, l;
    char TF[PV_Factory::MAX_PV_NAME + 1];
    
    absVal = fabs (value);
    minus = value < 0.0;
    newVal = absVal;
    if (absVal < 1.)
    {
        exp = 0;
        if (absVal != 0.)
        {    /* really ought to test against some epsilon */
            do
            {
                newVal *= 1000.0;
                exp += 3;
            }
            while (newVal < 1.);
        }
        cvtDoubleToString (newVal, TF , precision);
        k = 0; l = 0;
        if (minus)
            textField[k++] = '-';
        while (TF[l] != '\0')
            textField[k++] = TF[l++];
        textField[k++] = 'e';
        if (exp == 0)
        {
            textField[k++] = '+';    /* want e+00 for consistency with norms */
        } else {
            textField[k++] = '-';
        }
        textField[k++] = '0' + exp / 10;
        textField[k++] = '0' + exp % 10;
        textField[k++] = '\0';
        
    }
    else
    {
        /* absVal >= 1. */
        exp = 0;
        while (newVal >= 1000.)
        {
            newVal *= 0.001;
                     /* since multiplying is usually faster than dividing */
            exp += 3;
        }
        cvtDoubleToString (newVal, TF, precision);
        k = 0; l = 0;
        if (minus)
            textField[k++] = '-';
        while (TF[l] != '\0')
            textField[k++] = TF[l++];
        textField[k++] = 'e';
        textField[k++] = '+';
        textField[k++] = '0' + exp / 10;
        textField[k++] = '0' + exp % 10;
        textField[k++] = '\0';
    }
}

inline const char *getRawName (expStringClass &es)
{
    char *s = es.getRaw ();
    return s ? s : "";
}

inline const char *getExpandedName (expStringClass &es)
{
    char *s = es.getExpanded ();
    return s ? s : "";
}

edmmultiLineTextUpdateClass::edmmultiLineTextUpdateClass ()
{
    init (MULTILINE_TEXTUPDATE_CLASSNAME);
}

void edmmultiLineTextUpdateClass::init (const char *classname)
{
    name = strdup (classname);
    checkBaseClassVersion( activeGraphicClass::MAJOR_VERSION, name );
    is_executing = false;
    data_pv = 0;

    colour_pv = 0;
    is_filled = true;
    strcpy (fontTag, "");
    fs = 0;
    alignment = XmALIGNMENT_BEGINNING;
    is_line_alarm_sensitive = false;
}

edmmultiLineTextUpdateClass::edmmultiLineTextUpdateClass (
                                           edmmultiLineTextUpdateClass *rhs)
{
    clone (rhs, MULTILINE_TEXTUPDATE_CLASSNAME);
    doAccSubs( data_pv_name );
    doAccSubs( colour_pv_name );
}

void edmmultiLineTextUpdateClass::clone (const edmmultiLineTextUpdateClass *rhs,
                                         const char *classname)
{
    // This next line must always be included
    activeGraphicClass *ago = (activeGraphicClass *) this;
    ago->clone ((activeGraphicClass *)rhs);

    name = strdup (classname);

    is_executing = false;
    data_pv = 0;
    colour_pv = 0;
    data_pv_name.setRaw (rhs->data_pv_name.rawString);
    colour_pv_name.setRaw (rhs->colour_pv_name.rawString);
    displayMode = rhs->displayMode;
    precision = rhs->precision;
    textColour = rhs->textColour;
    fillColour = rhs->fillColour;
    is_filled = rhs->is_filled;
    strncpy (fontTag, rhs->fontTag, 63);
    fontTag[63] = 0;
    fs = actWin->fi->getXFontStruct (fontTag);
    fontAscent = rhs->fontAscent;
    fontDescent = rhs->fontDescent;
    fontHeight = rhs->fontHeight;
    alignment = rhs->alignment;
    line_width = rhs->line_width;
    lineColour = rhs->lineColour;
    is_line_alarm_sensitive = rhs->is_line_alarm_sensitive;
}

edmmultiLineTextUpdateClass::~edmmultiLineTextUpdateClass ()
{
    if (colour_pv)
    {
        colour_pv->remove_conn_state_callback (pv_conn_state_callback, this);
        colour_pv->remove_value_callback (pv_value_callback, this);
        colour_pv->release ();
        colour_pv = 0;
    }
    if (data_pv)
    {
        data_pv->remove_conn_state_callback (pv_conn_state_callback, this);
        data_pv->remove_access_security_callback(access_security_change, this);
        data_pv->remove_value_callback (pv_value_callback, this);
        data_pv->release ();
        data_pv = 0;
    }
    free (name);
}

char *edmmultiLineTextUpdateClass::objName ()
{   return name; }

// --------------------------------------------------------
// Load/save
// --------------------------------------------------------
int edmmultiLineTextUpdateClass::save (FILE *f)
{

int major, minor, release, alarmSens, oneDspMode, stat;
pvColorClass tc, fc;

tagClass tag;

int zero = 0;
char *emptyStr = "";

int deflt = 0;
static char *dspModeEnumStr[5] = 
{
    "default",
    "decimal",
    "hex",
    "engineer",
    "exp"
};
static int dspMode[5] = 
{
    0,
    1,
    2,
    3,
    4,
};

int left = XmALIGNMENT_BEGINNING;
static char *alignEnumStr[3] = 
{
    "left",
    "center",
    "right"
};
static int alignEnum[3] = 
{
    XmALIGNMENT_BEGINNING,
    XmALIGNMENT_CENTER,
    XmALIGNMENT_END
};

    major = TEXT_MAJOR;
    minor = TEXT_MINOR;
    release = TEXT_RELEASE;

    alarmSens = textColour.isAlarmSensitive ();

    oneDspMode = (int) displayMode;

    tc.setColorIndex ( textColour.getIndex (), actWin->ci );
    fc.setColorIndex ( fillColour.getIndex (), actWin->ci );

    tag.init ();
    tag.loadW ( "beginObjectProperties" );
    tag.loadW ( "major", &major );
    tag.loadW ( "minor", &minor );
    tag.loadW ( "release", &release );
    tag.loadW ( "x", &x );
    tag.loadW ( "y", &y );
    tag.loadW ( "w", &w );
    tag.loadW ( "h", &h );
    tag.loadW ( "controlPv", &data_pv_name, emptyStr );
    tag.loadW ( "displayMode", 5, dspModeEnumStr, dspMode, &oneDspMode,
                &deflt );
    tag.loadW ( "precision", &precision, &zero );
    tag.loadW ( "fgColour", actWin->ci, &tc );
    tag.loadBoolW ( "fgAlarm", &alarmSens, &zero );
    tag.loadW ( "bgColour", actWin->ci, &fc );
    tag.loadW ( "colourPv", &colour_pv_name, emptyStr );
    tag.loadBoolW ( "fill", &is_filled, &zero );
    tag.loadW ( "font", fontTag );
    tag.loadW ( "fontAlign", 3, alignEnumStr, alignEnum, &alignment, &left );
    tag.loadW ( "lineWidth", &line_width );
    tag.loadBoolW ( "lineAlarm", &is_line_alarm_sensitive, &zero );
    tag.loadW ( "endObjectProperties" );
    tag.loadW ( "" );

    stat = tag.writeTags ( f );

    return stat;
}

// --------------------------------------------------------
// Load/save
// --------------------------------------------------------
int edmmultiLineTextUpdateClass::old_save (FILE *f)
{
    // Version, bounding box
    fprintf (f, "%-d %-d %-d\n",
            TEXT_MAJOR, TEXT_MINOR, TEXT_RELEASE);
    fprintf (f, "%-d\n", x);
    fprintf (f, "%-d\n", y);
    fprintf (f, "%-d\n", w);
    fprintf (f, "%-d\n", h);
    // PV Name
    writeStringToFile (f, (char *)getRawName (data_pv_name));
    // Mode, precision
    fprintf (f, "%-d\n", (int) displayMode);
    fprintf (f, "%-d\n", (int) precision);
    // textcolour, fillcolour
    actWin->ci->writeColorIndex ( f, textColour.getIndex () );
    fprintf (f, "%-d\n", textColour.isAlarmSensitive ());
    actWin->ci->writeColorIndex ( f, fillColour.getIndex () );
    writeStringToFile (f, (char *)getRawName (colour_pv_name));
    // fill mode, fonts
    fprintf (f, "%-d\n", is_filled);
    writeStringToFile (f, fontTag);
    fprintf (f, "%-d\n", alignment);
    // Line
    line_width.write (f);
    fprintf (f, "%-d\n", is_line_alarm_sensitive);
   
    return 1;
}

int edmmultiLineTextUpdateClass::createFromFile (FILE *f, char *filename,
                                                 activeWindowClass *_actWin)

{

int major, minor, release, alarmSens, oneDspMode, stat;
pvColorClass tc, fc;

tagClass tag;

int zero = 0;
char *emptyStr = "";

int deflt = 0;
static char *dspModeEnumStr[5] = 
{
    "default",
    "decimal",
    "hex",
    "engineer",
    "exp"
};
static int dspMode[5] = 
{
    0,
    1,
    2,
    3,
    4,
};

int left = XmALIGNMENT_BEGINNING;
static char *alignEnumStr[3] = 
{
    "left",
    "center",
    "right"
};
static int alignEnum[3] = 
{
    XmALIGNMENT_BEGINNING,
    XmALIGNMENT_CENTER,
    XmALIGNMENT_END
};

    actWin = _actWin;

    tag.init ();
    tag.loadR ( "beginObjectProperties" );
    tag.loadR ( "major", &major );
    tag.loadR ( "minor", &minor );
    tag.loadR ( "release", &release );
    tag.loadR ( "x", &x );
    tag.loadR ( "y", &y );
    tag.loadR ( "w", &w );
    tag.loadR ( "h", &h );
    tag.loadR ( "controlPv", &data_pv_name, emptyStr );
    tag.loadR ( "displayMode", 5, dspModeEnumStr, dspMode, &oneDspMode,
                &deflt );
    tag.loadR ( "precision", &precision, &zero );
    tag.loadR ( "fgColour", actWin->ci, &tc );
    tag.loadR ( "fgAlarm", &alarmSens, &zero );
    tag.loadR ( "bgColour", actWin->ci, &fc );
    tag.loadR ( "colourPv", &colour_pv_name, emptyStr );
    tag.loadR ( "fill", &is_filled, &zero );
    tag.loadR ( "font", 63, fontTag );
    tag.loadR ( "fontAlign", 3, alignEnumStr, alignEnum, &alignment, &left );
    tag.loadR ( "lineWidth", &line_width );
    tag.loadR ( "lineAlarm", &is_line_alarm_sensitive, &zero );
    tag.loadR ( "endObjectProperties" );

    stat = tag.readTags ( f, "endObjectProperties" );

    if ( !( stat & 1 ) ) 
    {
        actWin->appCtx->postMessage ( tag.errMsg () );
    }

    if ( major > TEXT_MAJOR ) 
    {
        postIncompatable ();
        return 0;
    }

    if ( major < 10 ) 
    {
        postIncompatable ();
        return 0;
    }

    this->initSelectBox (); // call after getting x, y, w, h

    displayMode = (DisplayMode) oneDspMode;

    textColour.setIndex ( tc.pixelIndex () );
    lineColour.setIndex ( tc.pixelIndex () );

    textColour.setAlarmSensitive ( alarmSens );

    fillColour.setIndex ( fc.pixelIndex () );

    actWin->fi->loadFontTag (fontTag);
    fs = actWin->fi->getXFontStruct (fontTag);
    updateFont (fontTag, &fs,
                &fontAscent, &fontDescent, &fontHeight);

    if (is_line_alarm_sensitive && line_width.value () <= 0)
        line_width.setValue (1);

    lineColour.setAlarmSensitive (is_line_alarm_sensitive);

    return stat;

}

int edmmultiLineTextUpdateClass::old_createFromFile (FILE *f, char *filename,
                                                     activeWindowClass *_actWin)
{
    int major, minor, release;
    int index;
    char name[PV_Factory::MAX_PV_NAME + 1];

    actWin = _actWin;
    // Version, bounding box
    fscanf (f, "%d %d %d\n", &major, &minor, &release); actWin->incLine ();

    if ( major > TEXT_MAJOR ) 
    {
      postIncompatable ();
      return 0;
    }

    fscanf (f, "%d\n", &x); actWin->incLine ();
    fscanf (f, "%d\n", &y); actWin->incLine ();
    fscanf (f, "%d\n", &w); actWin->incLine ();
    fscanf (f, "%d\n", &h); actWin->incLine ();
    this->initSelectBox (); // call after getting x, y, w, h
    // PV Name
    readStringFromFile (name, PV_Factory::MAX_PV_NAME, f); actWin->incLine ();
                        data_pv_name.setRaw (name);
    // Added in 1.1.0: displayMode & precision
    if (major == 1  && minor == 0)
    {
        displayMode = dm_default;
        precision = 0;
    }
    else
    {
        fscanf (f, "%d\n", &index ); actWin->incLine ();
        if (index >= 0 && index <= dm_exp)
            displayMode = (DisplayMode)index;
        else
            displayMode = dm_default;
        fscanf (f, "%d\n", &index ); actWin->incLine ();
        precision = index;
    }
    // text colour. Changed for 2.0.0: Use names
    // Since 5.0.0: back to indices
    if ( major > 5 ) 
    {
        actWin->ci->readColorIndex ( f, &index );
        actWin->incLine (); actWin->incLine ();
        textColour.setIndex (index);
        lineColour.setIndex (index);
    }
    else if (major < 2 || major == 5)
    {
        fscanf (f, "%d\n", &index ); actWin->incLine ();
        textColour.setIndex (index);
    }
    else
    {
        readStringFromFile (name, PV_Factory::MAX_PV_NAME, f);
        actWin->incLine ();
        textColour.setName (actWin->ci, name);
    }
    // Since 4.0: alarm_sensitive
    if (major >= 4)
    {
        fscanf (f, "%d\n", &index); actWin->incLine ();
        textColour.setAlarmSensitive (index > 0);
    }
    // fillcolour index & mode
    if ( major > 5 ) 
    {
        actWin->ci->readColorIndex ( f, &index );
        actWin->incLine (); actWin->incLine ();
        fillColour.setIndex (index);
    }
    else if (major < 2 || major == 5)
    {
        fscanf (f, "%d\n", &index ); actWin->incLine ();
        fillColour.setIndex (index);
    }
    else
    {
        readStringFromFile (name, PV_Factory::MAX_PV_NAME, f);
        actWin->incLine ();
        fillColour.setName (actWin->ci, name);
    }
    // Since 3.0.0: Use colour pv
    if (major < 3)
        colour_pv_name.setRaw (0);
    else
    {
        readStringFromFile (name, PV_Factory::MAX_PV_NAME, f);
        actWin->incLine ();
        colour_pv_name.setRaw (name);
    }

    fscanf (f, "%d\n", &is_filled); actWin->incLine ();
    readStringFromFile (fontTag, 63, f); actWin->incLine ();
    fscanf ( f, "%d\n", &alignment ); actWin->incLine ();
    actWin->fi->loadFontTag (fontTag);
    fs = actWin->fi->getXFontStruct (fontTag);
    updateFont (fontTag, &fs, &fontAscent, &fontDescent, &fontHeight);
    if (major >= 1)
    {
        line_width.read (f); actWin->incLine ();
    }
    else
    {
        line_width.setNull (1);
    }

    // Since 7.0.0: line can be alarm sensitive
    if (major >= 7)
    {
        fscanf (f, "%d\n", &is_line_alarm_sensitive); actWin->incLine ();
        if (is_line_alarm_sensitive && line_width.value () <= 0)
            line_width.setValue (1);
        lineColour.setAlarmSensitive (is_line_alarm_sensitive);
    }
    else
        is_line_alarm_sensitive = false;

    return 1;
}

// --------------------------------------------------------
// Edit Mode
// --------------------------------------------------------

// Idea of next two and helper methods:
// createInteractive -> editCreate -> genericEdit (delete on cancel)
// edit -> genericEdit (ignore changes on cancel)
int edmmultiLineTextUpdateClass::createInteractive (activeWindowClass *aw_obj,
                                                    int _x, int _y,
                                                    int _w, int _h)
{   // required
    actWin = (activeWindowClass *) aw_obj;
    x = _x; y = _y; w = _w; h = _h;
    // Honor display scheme
    displayMode = dm_default;
    precision = 0;
    textColour.setIndex (actWin->defaultFg1Color);
    lineColour.setIndex (actWin->defaultFg1Color);
    line_width.setNull (1);
    fillColour.setIndex (actWin->defaultBgColor);
    strcpy (fontTag, actWin->defaultCtlFontTag);
    alignment = actWin->defaultCtlAlignment;
    fs = actWin->fi->getXFontStruct (fontTag);
    updateFont (fontTag, &fs, &fontAscent, &fontDescent, &fontHeight);

    // initialize and draw some kind of default image for the user
    draw ();
    editCreate ();
    return 1;
}

int edmmultiLineTextUpdateClass::edit ()
{   // Popup property dialog, cancel -> no delete
    genericEdit ();
    ef.finished (edit_ok, edit_apply, edit_cancel, this);
    actWin->currentEf = &ef;
    ef.popup ();
    return 1;
}

int edmmultiLineTextUpdateClass::editCreate ()
{
    // Popup property dialog, cancel -> delete
    genericEdit ();
    ef.finished (edit_ok, edit_apply, edit_cancel_delete, this);
    actWin->currentEf = NULL;
    ef.popup ();
    return 1;
}

int edmmultiLineTextUpdateClass::genericEdit () // create Property Dialog
{
    char title[80 + 1], *ptr;
    // required
    ptr = actWin->obj.getNameFromClass (name);
    if (ptr)
    {
        strncpy (title, ptr, 80);
        title[80] = 0;
        Strncat (title, " Properties", 80);
    }
    else
        strncpy (title, "Unknown object Properties", 80);

    // Copy data member contents into edit buffers
    bufX = x; bufY = y; bufW = w; bufH = h;
    strncpy (bufDataPvName, getRawName (data_pv_name),
             PV_Factory::MAX_PV_NAME);
    strncpy (bufColourPvName, getRawName (colour_pv_name),
             PV_Factory::MAX_PV_NAME);
    buf_displayMode          = (int)displayMode;
    buf_precision            = precision;
    buf_line_width           = line_width;
    bufTextColour            = textColour.getIndex ();
    buf_alarm_sensitive      = textColour.isAlarmSensitive ();
    bufFillColour            = fillColour.getIndex ();
    bufIsFilled              = is_filled;
    buf_alarm_sensitive_line = is_line_alarm_sensitive;

    // create entry form dialog box
    ef.create (actWin->top, actWin->appCtx->ci.getColorMap (),
               &actWin->appCtx->entryFormX, &actWin->appCtx->entryFormY,
               &actWin->appCtx->entryFormW, &actWin->appCtx->entryFormH,
               &actWin->appCtx->largestH,
               title, NULL, NULL, NULL);

    // add dialog box entry fields
    ef.addTextField ("X", 35, &bufX);
    ef.addTextField ("Y", 35, &bufY);
    ef.addTextField ("Width", 35, &bufW);
    ef.addTextField ("Height", 35, &bufH);
    ef.addTextField ("PV", 35, bufDataPvName, PV_Factory::MAX_PV_NAME);
    ef.addOption ("Mode", "default|decimal|hex|engineer|exp", &buf_displayMode);
    ef.addTextField ("Precision", 35, &buf_precision);
    ef.addTextField ("Line Width", 35, &buf_line_width);
    lineEntry = ef.getCurItem();
    ef.addToggle ("Alarm Sensitive Line", &buf_alarm_sensitive_line);
    alarmSensLineEntry = ef.getCurItem();
    lineEntry->addDependency( alarmSensLineEntry );
    lineEntry->addDependencyCallbacks();
    ef.addColorButton ("Fg Colour", actWin->ci, &textCb, &bufTextColour);
    ef.addToggle ("Alarm Sensitive Text", &buf_alarm_sensitive);
    ef.addToggle ("Filled?", &bufIsFilled);
    fillEntry = ef.getCurItem();
    ef.addColorButton ("Bg Colour", actWin->ci, &fillCb, &bufFillColour);
    fillColorEntry = ef.getCurItem();
    fillEntry->addDependency( fillColorEntry );
    fillEntry->addDependencyCallbacks();
    ef.addTextField ("Colour PV", 35, bufColourPvName, PV_Factory::MAX_PV_NAME);
    ef.addFontMenu ("Font", actWin->fi, &fm, fontTag );
    fm.setFontAlignment (alignment);

    return 1;
}

void edmmultiLineTextUpdateClass::redraw_text (Display *dis,
                                     Drawable drw,
                                     gcClass &gcc,
                                     GC gc,
                                     const char *text,
                                     size_t len)
{
    int fg_pixel = textColour.getPixel (actWin->ci);
    
    // Background fill?
    if (is_filled)
    {
        gcc.setFG (fillColour.getPixel (actWin->ci));
        XFillRectangle (dis, drw, gc, x, y, w, h);
    }
    // Border: if line width > 0, but don't show the
    // line if we're alarm sensitive and there is no alarm
    if (line_width.value () > 0 &&
        !(is_line_alarm_sensitive &&
          data_pv && data_pv->is_valid () && data_pv->get_severity () == 0))
    {
        int line_pixel = lineColour.getPixel (actWin->ci);
        gcc.setFG (line_pixel);
        gcc.setLineWidth (line_width.value ());
        XDrawRectangle (dis, drw, gc, x, y, w, h);
        gcc.setLineWidth (1);
    }
    gcc.setFG (fg_pixel);
    if (len > 0)
    {
        // Text
        // Positioning is calculated for each call
        // -> not terrible for critical situation?
        //    a) called during edit -> good enough
        //    b) called for changed value -> have to do it, text is different
        //    c) called for total redraw after window hidden -> hmmm.
        // Right now I care most about b) to assert best update rate.
        XRectangle clip;
        clip.x = x;
        clip.y = y;
        clip.width = w;
        clip.height = h;
        gcc.addNormXClipRectangle (clip);
        gcc.setFontTag (fontTag, actWin->fi);
        int txt_width = (fs)?XTextWidth (fs, text, len):10;
        int tx;
        switch (alignment)
        {
        case XmALIGNMENT_BEGINNING:
            tx = x;
            break;
        case XmALIGNMENT_CENTER:
            tx = x + (w - txt_width)/2;
            break;
        default:
            tx = x + w - txt_width;
        }
        int ty = y + (fontAscent + h)/2;
#ifdef DEBUG
        printf ("edmmultiLineTextUpdateClass::redraw_text - text is %s\n",
                text);
#endif
        XDrawString (dis, drw, gc, tx, ty, text, len);
        gcc.removeNormXClipRectangle ();
    }
}

void  edmmultiLineTextUpdateClass::remove_text (Display *dis,
                                                Drawable drw,
                                                gcClass &gcc,
                                                GC gc)
{
    XFillRectangle (dis, drw, gc, x, y, w, h);
    if (!line_width.isNull ())
    {
        gcc.setLineWidth (line_width.value ());
        XDrawRectangle (dis, drw, gc, x, y, w, h);
        gcc.setLineWidth (1);
    }
}

int edmmultiLineTextUpdateClass::draw ()  // render the edit-mode image
{
    // required
    if (is_executing || deleteRequest)
        return 1;
    actWin->drawGc.saveFg ();
    
    const char *pvname = getRawName (data_pv_name);
    size_t len = strlen (pvname);
    textColour.reset ();
    lineColour.reset ();
    redraw_text (actWin->d,
                XtWindow (actWin->drawWidget),
                actWin->drawGc,
                actWin->drawGc.normGC (),
                pvname, len);
    
    actWin->drawGc.restoreFg ();
    return 1;
}

int edmmultiLineTextUpdateClass::erase ()  // erase edit-mode image
{
    // required
    if (is_executing || deleteRequest )
        return 1;
    remove_text (actWin->d,
                 XtWindow (actWin->drawWidget),
                 actWin->drawGc,
                 actWin->drawGc.eraseGC ());
    return 1;
}

int edmmultiLineTextUpdateClass::checkResizeSelectBox (int _x, int _y,
                                                       int _w, int _h)
{   // Assert minimum size
    return checkResizeSelectBoxAbs (_x, _y, w+_w, h+_h);
}

int edmmultiLineTextUpdateClass::checkResizeSelectBoxAbs (int _x, int _y,
                                                          int _w, int _h)
{   // Similar, but absolute sizes. -1 is also possible
    if (_w != -1  &&  _w < 10)
        return 0;
    if (_h != -1  &&  _h < 10)
        return 0;
    return 1;
}

// Callbacks from property dialog
void edmmultiLineTextUpdateClass::edit_update (Widget w, XtPointer client,
                                               XtPointer call)
{
    edmmultiLineTextUpdateClass *me = (edmmultiLineTextUpdateClass *) client;
    // required
    me->actWin->setChanged ();
    me->eraseSelectBoxCorners ();
    me->erase ();

    me->x = me->bufX;
    me->sboxX = me->bufX;
    me->y = me->bufY;
    me->sboxY = me->bufY;
    me->w = me->bufW;
    me->sboxW = me->bufW;
    me->h = me->bufH;
    me->sboxH = me->bufH;

    me->data_pv_name.setRaw (me->bufDataPvName);
    me->colour_pv_name.setRaw (me->bufColourPvName);
    me->displayMode     = (DisplayMode) me->buf_displayMode;
    me->precision       = me->buf_precision;
    me->line_width      = me->buf_line_width;
    me->is_line_alarm_sensitive = me->buf_alarm_sensitive_line;
    me->textColour.setIndex (me->bufTextColour);
    me->textColour.setAlarmSensitive (me->buf_alarm_sensitive > 0);
    me->lineColour.setIndex (me->bufTextColour);
    me->lineColour.setAlarmSensitive (me->buf_alarm_sensitive_line > 0);
    me->fillColour.setIndex (me->bufFillColour);
    me->is_filled       = me->bufIsFilled;

    strncpy (me->fontTag, me->fm.currentFontTag (), 63);
    me->fontTag[63] = 0;
    me->actWin->fi->loadFontTag (me->fontTag);
    me->fs          = me->actWin->fi->getXFontStruct (me->fontTag);
    me->alignment   = me->fm.currentFontAlignment ();
    me->fs          = me->actWin->fi->getXFontStruct (me->fontTag);
    me->updateFont (me->fontTag, &me->fs,
                    &me->fontAscent, &me->fontDescent, &me->fontHeight);
}

void edmmultiLineTextUpdateClass::edit_ok (Widget w, XtPointer client,
                                           XtPointer call)
{
    edmmultiLineTextUpdateClass *me = (edmmultiLineTextUpdateClass *) client;
    edit_update (w, client, call);
    // required
    me->ef.popdown ();
    me->operationComplete ();
}

void edmmultiLineTextUpdateClass::edit_apply (Widget w, XtPointer client,
                                              XtPointer call)
{
    edmmultiLineTextUpdateClass *me = (edmmultiLineTextUpdateClass *) client;
    edit_update (w, client, call);
    // required
    me->refresh (me);
}

void edmmultiLineTextUpdateClass::edit_cancel (Widget w, XtPointer client,
                                               XtPointer call)
{
    edmmultiLineTextUpdateClass *me = (edmmultiLineTextUpdateClass *) client;
    // next two lines required
    me->ef.popdown ();
    me->operationCancel ();
}

void edmmultiLineTextUpdateClass::edit_cancel_delete (Widget w,
                                                      XtPointer client,
                                                      XtPointer call)
{
    edmmultiLineTextUpdateClass *me = (edmmultiLineTextUpdateClass *) client;
    // all lines required
    me->ef.popdown ();
    me->operationCancel ();
    me->erase ();
    me->deleteRequest = 1;
    me->drawAll ();
}

// --------------------------------------------------------
// GroupEdit
// --------------------------------------------------------
// edm-components.doc shows wrong prototype for these!
void edmmultiLineTextUpdateClass::changeDisplayParams (unsigned int flag,
                                                       char *_fontTag,
                                                       int _alignment,
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
                                                       int botShadowColour)
{
    if (flag & ACTGRF_FG1COLOR_MASK)
    {
        textColour.setIndex (fg1Colour);
        lineColour.setIndex (fg1Colour);
    }
    if (flag & ACTGRF_BGCOLOR_MASK)
        fillColour.setIndex (bgColour);
    if (flag & ACTGRF_FONTTAG_MASK)
    {
        strcpy (fontTag, _fontTag);
        alignment = _alignment;
        fs = actWin->fi->getXFontStruct (fontTag);
        updateFont (fontTag, &fs,
                    &fontAscent, &fontDescent, &fontHeight);
    }
}

void edmmultiLineTextUpdateClass::changePvNames (int flag,
                                                 int numCtlPvs,
                                                 char *ctlPvs[],
                                                 int numReadbackPvs,
                                                 char *readbackPvs[],
                                                 int numNullPvs,
                                                 char *nullPvs[],
                                                 int numVisPvs,
                                                 char *visPvs[],
                                                 int numAlarmPvs,
                                                 char *alarmPvs[])
{
    if (flag & ACTGRF_READBACKPVS_MASK)
    {
        if (numReadbackPvs)
            data_pv_name.setRaw (readbackPvs[0]);
    }
    // Note: There is no "colour" PV.
    // We use the "vis" PV for now
    if (flag & ACTGRF_VISPVS_MASK)
    {
        if (numVisPvs)
            colour_pv_name.setRaw (visPvs[0]);
    }
}

void edmmultiLineTextUpdateClass::getPvs (int max,
                                          ProcessVariable *pvs[],
                                          int *n)
{
    if ( max < 2 ) 
    {
        *n = 0;
        return;
    }
    *n = 2;
    pvs[0] = data_pv;
    pvs[1] = colour_pv;
}

char *edmmultiLineTextUpdateClass::getSearchString (
  int i
) {

  if ( i == 0 ) {
    return data_pv_name.getRaw();
  }
  else if ( i == 1 ) {
    return colour_pv_name.getRaw();
  }

  return NULL;

}

void edmmultiLineTextUpdateClass::replaceString (
  int i,
  int max,
  char *string
) {

  if ( i == 0 ) {
    data_pv_name.setRaw( string );
  }
  else if ( i == 1 ) {
    colour_pv_name.setRaw( string );
  }

}
    
// --------------------------------------------------------
// Macro support
// --------------------------------------------------------
int edmmultiLineTextUpdateClass::containsMacros ()
{
    return data_pv_name.containsPrimaryMacros () ||
        colour_pv_name.containsPrimaryMacros ();
}

int edmmultiLineTextUpdateClass::expandTemplate (
  int numMacros,
  char *macros[],
  char *expansions[]
) {

expStringClass tmpStr;

  tmpStr.setRaw( colour_pv_name.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  colour_pv_name.setRaw( tmpStr.getExpanded() );

  tmpStr.setRaw( data_pv_name.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  data_pv_name.setRaw( tmpStr.getExpanded() );

  return 1;

}

int edmmultiLineTextUpdateClass::expand1st (int numMacros, char *macros[],
                                            char *expansions[])
{
    colour_pv_name.expand1st (numMacros, macros, expansions);
    return data_pv_name.expand1st (numMacros, macros, expansions);
}

int edmmultiLineTextUpdateClass::expand2nd (int numMacros, char *macros[],
                                            char *expansions[])
{
    colour_pv_name.expand2nd (numMacros, macros, expansions);
    return data_pv_name.expand2nd (numMacros, macros, expansions);
}

// --------------------------------------------------------
// Execute
// --------------------------------------------------------
int edmmultiLineTextUpdateClass::setupPVs (int pass, void *ptr)
{
    switch (pass) // ... up to 6
    {
    case 1: // initialize
        aglPtr = ptr;
        is_executing = true;
        //is_data_pv_valid =
            // strcmp (getExpandedName (data_pv_name), "") != 0;
        is_data_pv_valid =
             !blankOrComment ( (char *) getExpandedName (data_pv_name) );
        //is_colour_pv_valid =
            // strcmp (getExpandedName (colour_pv_name), "") != 0;
        is_colour_pv_valid =
             !blankOrComment ( (char *) getExpandedName (colour_pv_name) );
        initEnable ();
        break;

    case 2: // connect to pv
        if (data_pv)
        {
            fprintf ( stderr, "textupdate::setupPVs: pv already set!\n");
            return 1;
        }
        if (is_data_pv_valid)
        {
            data_pv = the_PV_Factory->create (getExpandedName (data_pv_name));
            if (data_pv)
            {
                data_pv->add_conn_state_callback (pv_conn_state_callback, this);
                data_pv->add_access_security_callback(access_security_change, this);
                data_pv->add_value_callback (pv_value_callback, this);
            }
        }
        if (is_colour_pv_valid)
        {
            colour_pv = the_PV_Factory->create (getExpandedName (
                                                    colour_pv_name));
            if (colour_pv)
            {
                colour_pv->add_conn_state_callback (pv_conn_state_callback,
                                                    this);
                colour_pv->add_value_callback (pv_value_callback, this);
            }
        }
        if (!data_pv)
            drawActive ();
        break;
    }
    return 1;
}

int edmmultiLineTextUpdateClass::activate (int pass, void *ptr)
{
    XmFontList fonts;
    if (! setupPVs (pass, ptr))
        return 0;
    
    switch (pass) // ... up to 6
    {
    case 1: // initialize
        // from man XmTextField
        initEnable ();
        fonts = XmFontListCreate (fs, XmSTRING_DEFAULT_CHARSET);
        if ( g_transInit )
        {
            g_transInit = 0;
            g_parsedTrans = XtParseTranslationTable (g_dragTrans);
        }
        actWin->appCtx->addActions ( g_dragActions,
                                     XtNumber (g_dragActions) );

        widget = XtVaCreateManagedWidget ("TextUpdate",
//                                          xmTextFieldWidgetClass,
                                          xmTextWidgetClass,
                                          actWin->executeWidgetId (),
                                          XtNx, (XtArgVal)x,
                                          XtNy, (XtArgVal)y,
                                          XtNheight,(XtArgVal)h,
                                          XtNwidth, (XtArgVal)w,
                                          XmNforeground,
                                          (XtArgVal)
                                          textColour.getPixel (actWin->ci),
                                          XmNbackground, (XtArgVal)
                                          fillColour.getPixel (actWin->ci),
                                          XmNfontList, (XtArgVal)fonts,
                                          // next 2 seem to have no effect:
                                          XmNentryAlignment,
                                          (XtArgVal)alignment,
                                          XmNalignment,
                                          (XtArgVal)alignment,
                                          XmNtranslations, g_parsedTrans,
                                          XmNuserData,
                                          this,// obj accessible to d&d
                                          XmNhighlightThickness,
                                          (XtArgVal) 0,
                                          XmNeditMode, XmMULTI_LINE_EDIT,
                                          XmNwordWrap, True,
                                          XmNcursorPositionVisible, False,
                                          NULL);
        if ( !enabled ) 
        {
            if ( widget ) XtUnmapWidget ( widget );
        }

        break;
    }
    return 1;
}

int edmmultiLineTextUpdateClass::removeCallbacks (int pass)
{
    is_executing = false;
    switch (pass)
    {
    case 1: // disconnect
        if (colour_pv)
        {
            colour_pv->remove_conn_state_callback (pv_conn_state_callback,
                                                   this);
            colour_pv->remove_value_callback (pv_value_callback, this);
            colour_pv->release ();
            colour_pv = 0;
        }
        if (data_pv)
        {
            data_pv->remove_conn_state_callback (pv_conn_state_callback,
                                                 this);
            data_pv->remove_access_security_callback(access_security_change, this);
            data_pv->remove_value_callback (pv_value_callback, this);
            data_pv->release ();
            data_pv = 0;
        }
        break;
    case 2: // remove toolkit widgets
        break;
    }
    return 1;
}

int edmmultiLineTextUpdateClass::deactivate (int pass)
{
    is_executing = false;
    switch (pass)
    {
    case 2: // remove toolkit widgets
        if (widget)
        {
            XtUnmapWidget (widget);
            XtDestroyWidget (widget);
            widget = 0;
        }
        break;
    }
    return removeCallbacks (pass);
}
// Get text & colour value.
// len has to be initialized with the text buffer size.
// Returns 1 if PV is valid
bool edmmultiLineTextUpdateClass::get_current_values (char *text, size_t &len)
{
#ifdef DEBUG
    printf ("Start of edmMultiLineTextUpdateClass::get_current_values\n");
#endif
    textColour.updatePVStatus (data_pv);
    lineColour.updatePVStatus (data_pv);

    if (data_pv && data_pv->is_valid ())
    {
        if (colour_pv)
            textColour.updateColorValue (colour_pv);
        else
            textColour.updateColorValue (data_pv);
#ifdef DEBUG
        printf ("edmMultiLineTextUpdateClass::get_current_values - before get_char_array\n");
#endif
        const char* textPtr = data_pv->get_char_array ();
#ifdef DEBUG
        printf ("edmMultiLineTextUpdateClass::get_current_values - get_char_array returns %x\n", textPtr);
#endif
        if (textPtr)
            strncpy (text, textPtr, MAX_TEXT_LENGTH);
        else
            strcpy (text, "!! Invalid PV type !!");
        
        for (len = 0; len < MAX_TEXT_LENGTH && text[len] != 0; len++);
#ifdef DEBUG
        printf ("edmMultiLineTextUpdateClass::get_current_values returns true\n");
#endif
        return true;
    }
    // Disconnected: Display the PV name
    text[0] = '<';
    strcpy (text + 1, getExpandedName (data_pv_name));
    strcat (text, ">");
    len = strlen (text);
#ifdef DEBUG
    printf ("edmMultiLineTextUpdateClass::get_current_values returns false\n");
#endif
    return false;
}

#ifdef COMMENT_OUT
int edmmultiLineTextUpdateClass::drawActive ()
{
    if ( !enabled || !is_executing )
        return 1;
    actWin->executeGc.saveFg ();

    char text[PV_Factory::MAX_PV_NAME + 1];
    size_t len = PV_Factory::MAX_PV_NAME;
    get_current_values (text, len);
    redraw_text (actWin->d,
                 drawable (actWin->executeWidget),
                 actWin->executeGc,
                 actWin->executeGc.normGC (),
                 text, len);
   
    actWin->executeGc.restoreFg ();
    return 1;
}
#endif
int edmmultiLineTextUpdateClass::drawActive ()
{
    // ****** SJS Addition 05_12_06 - do not show if widget is part of a ******
    // ****** disabled group.  Note that "enabled" is not correctly set ******
    // ****** in the function activate *******
#ifdef DEBUG
    printf ("Start of multiLineTextUpdateClass::drawActive\n");
#endif
    if ( !enabled ) 
    {
        if ( widget ) XtUnmapWidget ( widget );
    }
    // ****** End of SJS addition ******
    if ( !enabled || !is_executing )
        return 1;

    char text[MAX_TEXT_LENGTH + 1];
    size_t len = MAX_TEXT_LENGTH;
    if (get_current_values (text, len))
    {
        XtVaSetValues (widget,
                       XmNeditable, False,
                       XmNforeground,
                       (XtArgVal)textColour.getPixel (actWin->ci),
                       NULL);
    }
    else
    {
        XtVaSetValues (widget,
                       XmNeditable, False,
                       XmNforeground,
                       (XtArgVal)textColour.getPixel (actWin->ci),
                       NULL);
    }
    XmTextSetString (widget, text);

#ifdef DEBUG
    printf ("End of multiLineTextUpdateClass::drawActive\n");
#endif
    return 1;
}
int edmmultiLineTextUpdateClass::eraseActive ()
{
#ifdef COMMENT_OUT
    if ( !enabled || !is_executing )
        return 1;
    remove_text (actWin->d,
                 drawable (actWin->executeWidget),
                 actWin->executeGc,
                 actWin->executeGc.eraseGC ());
#endif
    return 1;
}

void edmmultiLineTextUpdateClass::pv_conn_state_callback (ProcessVariable *pv,
                                                          void *userarg)
{
    edmmultiLineTextUpdateClass *me = (edmmultiLineTextUpdateClass *)userarg;
    me->actWin->appCtx->proc->lock ();
    if (me->is_executing)
    {
        me->bufInvalidate ();
        me->actWin->addDefExeNode (me->aglPtr);
    }
    me->actWin->appCtx->proc->unlock ();
}

void edmmultiLineTextUpdateClass::pv_value_callback (ProcessVariable *pv,
                                                     void *userarg)
{
#if 0
    fprintf ( stderr, "New Value for '%s': Type %s, Dim. %d\n",
           pv->get_name (),
           pv->get_type ().description,
           pv->get_dimension ());
    size_t i;
    switch (pv->get_type ().type)
    {
    case ProcessVariable::Type::integer:
        for (i = 0; i < pv->get_dimension (); ++i)
            fprintf ( stderr, "%d ", pv->get_int_array ()[i]);
        fprintf ( stderr, "\n");
        break;
    case ProcessVariable::Type::real:
        for (i = 0; i < pv->get_dimension (); ++i)
            fprintf ( stderr, "%g ", pv->get_double_array ()[i]);
        fprintf ( stderr, "\n");
        break;
    default:
        char buf[200];
        pv->get_string (buf, 200);
        fprintf ( stderr, "%s\n", buf);
    }
#endif
//  printf ("Start of edmMultiLineTextEditClass::pv_value_callback\n");
    edmmultiLineTextUpdateClass *me = (edmmultiLineTextUpdateClass *)userarg;
    me->actWin->appCtx->proc->lock ();
    if (me->is_executing)
    {
        me->bufInvalidate ();
        me->actWin->addDefExeNode (me->aglPtr);
    }
    me->actWin->appCtx->proc->unlock ();
}

void edmmultiLineTextUpdateClass::access_security_change (
  ProcessVariable *pv,
  void *userarg
) {

edmmultiLineTextUpdateClass *me = (edmmultiLineTextUpdateClass *)userarg;

  if ( me->data_pv ) {
    if ( me->data_pv->have_write_access() ) {
      if ( me->widget ) XtVaSetValues( me->widget,
       XmNeditable, True,
       NULL );
    }
    else {
      if ( me->widget ) XtVaSetValues( me->widget,
       XmNeditable, False,
       NULL );
    }
  }

}

void edmmultiLineTextUpdateClass::executeDeferred ()
{   // Called as a result of addDefExeNode
    if (actWin->isIconified)
        return;
    actWin->appCtx->proc->lock ();
    actWin->remDefExeNode (aglPtr);
    actWin->appCtx->proc->unlock ();
    if (is_executing)
        smartDrawAllActive ();
}

// Drag & drop support
char *edmmultiLineTextUpdateClass::firstDragName ()
{   
    if ( !enabled ) return NULL;
    return "PV";
}

char *edmmultiLineTextUpdateClass::nextDragName ()
{
    if ( !enabled ) return NULL;
   return NULL;
}

char *edmmultiLineTextUpdateClass::dragValue (int i)
{
    if ( !enabled ) return NULL;

    if ( actWin->mode == AWC_EXECUTE ) 
    {
        return (char *)getExpandedName (data_pv_name);
    }
    else 
    {
      return (char *)getRawName (data_pv_name);
    }
}

// --------------------------------------------------------
// Text Entry
// --------------------------------------------------------

edmmultiLineTextEntryClass::edmmultiLineTextEntryClass ()
{
    init (MULTILINE_TEXTENTRY_CLASSNAME);
    widget = 0;
    editing = false;
}

edmmultiLineTextEntryClass::edmmultiLineTextEntryClass (
                                         const edmmultiLineTextEntryClass *rhs)
{
    clone (rhs, MULTILINE_TEXTENTRY_CLASSNAME);
    widget = 0;
    editing = false;
}


// callbacks for drag & drop from Motif text widgets

static void drag (Widget w, XEvent *e, String *params, Cardinal numParams)
{
    activeGraphicClass *obj;
    XtVaGetValues (w, XmNuserData, &obj, NULL);
    
    obj->startDrag (w, e);
}

static void selectDrag (Widget w, XEvent *e, String *params, Cardinal numParams)
{
    activeGraphicClass *obj;
    XButtonEvent *be = (XButtonEvent *) e;

    XtVaGetValues (w, XmNuserData, &obj, NULL);
    obj->selectDragValue ( be );
}

static void selectActions ( Widget w,
                            XEvent *e,
                            String *params,
                            Cardinal numParams )
{
    activeGraphicClass *obj;
    XButtonEvent *be = (XButtonEvent *) e;

    XtVaGetValues ( w, XmNuserData, &obj, NULL );
    obj->doActions ( be, be->x, be->y );
}

static void pvInfo ( Widget w,
                     XEvent *e,
                     String *params,
                     Cardinal numParams )
{

    activeGraphicClass *obj;
    XButtonEvent *be = (XButtonEvent *) e;

    XtVaGetValues ( w, XmNuserData, &obj, NULL );

    obj->showPvInfo ( be, be->x, be->y );

}

// Get text & colour value.
// len has to be initialized with the text buffer size.
// Returns 1 if PV is valid
bool edmmultiLineTextEntryClass::get_current_values (char *text, size_t &len)
{
    bool result;
#ifdef DEBUG
    printf ("Start of multiLineTextEntryClass::get_current_values\n");
#endif
    if (!(data_pv && data_pv->is_valid ()))
    {
        text[0] = '<';
        strcpy (text + 1, getExpandedName (data_pv_name));
        strcat (text, ">");
        strncpy (old_text, text, MAX_TEXT_LENGTH);
    }
    result = edmmultiLineTextUpdateClass::get_current_values (text, len);
#ifdef DEBUG
    printf ("End of multiLineTextEntryClass::get_current_values\n");
#endif
    return result;
}
        
int edmmultiLineTextEntryClass::activate (int pass, void *ptr)
{
    XmFontList fonts;
    if (! edmmultiLineTextUpdateClass::setupPVs (pass, ptr))
        return 0;
    
    switch (pass) // ... up to 6
    {
    case 1: // initialize
        // from man XmTextField
        initEnable ();
        fonts = XmFontListCreate (fs, XmSTRING_DEFAULT_CHARSET);
        if ( g_transInit )
        {
            g_transInit = 0;
            g_parsedTrans = XtParseTranslationTable (g_dragTrans);
        }
        actWin->appCtx->addActions ( g_dragActions,
                                     XtNumber (g_dragActions) );

        widget = XtVaCreateManagedWidget ("TextEntry",
//                                          xmTextFieldWidgetClass,
                                          xmTextWidgetClass,
                                          actWin->executeWidgetId (),
                                          XtNx, (XtArgVal)x,
                                          XtNy, (XtArgVal)y,
                                          XtNheight,(XtArgVal)h,
                                          XtNwidth, (XtArgVal)w,
                                          XmNforeground,
                                          (XtArgVal)
                                          textColour.getPixel (actWin->ci),
                                          XmNbackground, (XtArgVal)
                                          fillColour.getPixel (actWin->ci),
                                          XmNfontList, (XtArgVal)fonts,
                                          // next 2 seem to have no effect:
                                          XmNentryAlignment,
                                          (XtArgVal)alignment,
                                          XmNalignment,
                                          (XtArgVal)alignment,
                                          XmNtranslations, g_parsedTrans,
                                          XmNuserData,
                                          this,// obj accessible to d&d
                                          XmNhighlightThickness,
                                          (XtArgVal) 3,
                                          XmNeditMode, XmMULTI_LINE_EDIT,
                                          XmNwordWrap, True,
                                          NULL);
        // callback: text entered ==> send it to the PV
        XtAddCallback (widget, XmNactivateCallback,
                      (XtCallbackProc)text_entered_callback,
                      (XtPointer)this);

        // callback: go into edit mode
        // MEDM uses XmNmodifyVerifyCallback,
        // John Sinclair uses XmNmotionVerifyCallback
        XtAddCallback (widget, XmNmotionVerifyCallback,
                       (XtCallbackProc)text_edit_callback,
                       (XtPointer)this);
        XtAddCallback (widget, XmNmodifyVerifyCallback,
                       (XtCallbackProc)text_edit_callback,
                       (XtPointer)this);

        if ( !enabled ) 
        {
            if ( widget ) XtUnmapWidget ( widget );
        }

        break;
    }
    return 1;
}

int edmmultiLineTextEntryClass::deactivate (int pass)
{
    is_executing = false;
    switch (pass)
    {
    case 2: // remove toolkit widgets
        if (widget)
        {
            XtUnmapWidget (widget);
            XtDestroyWidget (widget);
            widget = 0;
        }
        break;
    }
    return edmmultiLineTextUpdateClass::removeCallbacks (pass);
}

int edmmultiLineTextEntryClass::drawActive ()
{
    // ****** SJS Addition 05_12_06 - do not show if widget is part of a ******
    // ****** disabled group.  Note that "enabled" is not correctly set ******
    // ****** in the function activate *******
#ifdef DEBUG
    printf ("Start of multiLineTextEntryClass::drawActive\n");
#endif
    if ( !enabled ) 
    {
        if ( widget ) XtUnmapWidget ( widget );
    }
    // ****** End of SJS addition ******
    if ( !enabled || !is_executing )
        return 1;
    if (editing)
        return 1;

    char text[MAX_TEXT_LENGTH + 1];
    size_t len = MAX_TEXT_LENGTH;
    if (get_current_values (text, len))
    {
        XtVaSetValues (widget,
                       XmNeditable, True,
                       XmNforeground,
                       (XtArgVal)textColour.getPixel (actWin->ci),
                       NULL);
        if (data_pv->have_write_access ()) {
            actWin->cursor.set (XtWindow (widget), CURSOR_K_DEFAULT);
	}
        else {
            actWin->cursor.set (XtWindow (widget), CURSOR_K_NO);
            XtVaSetValues (widget, XmNeditable, False, NULL);
	}
    }
    else
    {
        XtVaSetValues (widget,
                       XmNeditable, False,
                       XmNforeground,
                       (XtArgVal)textColour.getPixel (actWin->ci),
                       NULL);
        actWin->cursor.set (XtWindow (widget), CURSOR_K_WAIT);
    }
    XmTextSetString (widget, text);

#ifdef DEBUG   
    printf ("End of multiLineTextEntryClass::drawActive\n");
#endif
    return 1;
}

int edmmultiLineTextEntryClass::eraseActive ()
{
    return 1;
}

// Switch to edit mode
//
// (Invoked by the ModifyVerify or MotionVerify callback)
void edmmultiLineTextEntryClass::text_edit_callback (Widget w,
                                                     XtPointer clientData,
                                                     XtPointer pCallbackData)
{
#ifdef DEBUG    
    printf ("Start of edmmultiLineTextEntryClass::text_edit_callback\n");
#endif
    edmmultiLineTextEntryClass *me = (edmmultiLineTextEntryClass *) clientData;
    XmTextVerifyCallbackStruct *pcbs =
        (XmTextVerifyCallbackStruct *) pCallbackData;
    /* NULL event means value changed programmatically; hence don't process */
    if (pcbs->event != NULL)
    {
        me->editing = true;
#ifdef DEBUG_TEXTWIDGETS
        fprintf ( stderr, "multiLineTextEntry: Editing '%s'\n",
                  (me->pv ? me->pv->get_name () : "<no PV>"));
#endif
        switch (XtHasCallbacks (w, XmNlosingFocusCallback))
        {
        case XtCallbackNoList:
        case XtCallbackHasNone:
//          XtAddCallback (w, XmNlosingFocusCallback,
//                        (XtCallbackProc)text_noedit_callback, me);
            XtAddCallback (w, XmNlosingFocusCallback,
                          (XtCallbackProc)text_entered_callback, me);
            /* John added this: Allows to enter text while
               the EDM window is not the active one ?!
               Confuses the Mac X server: steals keyboard
               from Striptool until EDM is stopped!
                   
               XSetInputFocus (me->actWin->display (), XtWindow (w),
               RevertToNone, CurrentTime);
            */
            break;
        case XtCallbackHasSome:
            /* Callback already installed */
            break;
        }
    }
    pcbs->doit = True;
    me->callback_common (w, clientData);
}

/* Text has been entered, send to PV */
 void edmmultiLineTextEntryClass::callback_common (Widget w,
                                                   XtPointer clientData)
{
    edmmultiLineTextEntryClass *me = (edmmultiLineTextEntryClass *) clientData;
    char *text = XmTextGetString (w);
#ifdef DEBUG
    printf ("Start of edmMultiLineTextEntryClass:callbk_common - text is %s\n",
            text);
    printf ("Old text is %s\n",
            me->old_text);
#endif
    if (strcmp (text, me->old_text))
    {
        // Text has changed - write it
        strncpy (me->old_text, text, MAX_TEXT_LENGTH);
       
    double num;
    int hexnum;

    // Just copied the current 'text' that we'll write
    // Allow updates from now on:
//  me->editing= false;
    XtVaSetValues (w, XmNcursorPosition, (XmTextPosition) 0, NULL);
#ifdef DEBUG_TEXTWIDGETS
    fprintf ( stderr, "multiLineTextEntry: Writing '%s'\n", text);
#endif
//  printf ( "multiLineTextEntry: Writing '%s'\n", text);
    if (me->data_pv && me->data_pv->is_valid ())
    {
        switch (me->displayMode)
        {
        case dm_default:
            if (me->data_pv->get_type ().type <
                ProcessVariable::Type::enumerated)
            {
                num = strtod (text, 0);
                me->data_pv->put (
                 XDisplayName(me->actWin->appCtx->displayName),
                 num);
            }
            else
            {
//              me->pv->put (
//               XDisplayName(me->actWin->appCtx->displayName),
//               text);
                int maxlen = me->data_pv->get_dimension ();
                // If text is longer than the size of the waveform PV to which
                // we are writing, put in a null character to truncate it to
                // the maximum length.
                if (strlen (text) >= maxlen)
                    text [maxlen - 1] = 0;
                me->data_pv->putArrayText (text);
            }
            break;

        case dm_hex:
            hexnum = strtol (text, 0, 16);
            // fprintf ( stderr, "Text: %s -> %d\n", text, hexnum);
            me->data_pv->put (
             XDisplayName(me->actWin->appCtx->displayName),
             hexnum);
            break;

        default:
            me->data_pv->put (
             XDisplayName(me->actWin->appCtx->displayName),
             text);
        }
    }
    XtFree (text);
    // Display current value again,
    // though we'll soon expect a monitor
    // from the PV which reflects the new value
    pv_value_callback (me->data_pv, me);
    }
}

/* Ignore editing, abort and repaint original value */
void edmmultiLineTextEntryClass::text_noedit_callback (Widget w,
                                                       XtPointer clientData,
                                                       XtPointer pCallbackData)
{
//  printf ("Start of edmmultiLineTextEntryClass::text_noedit_callback\n");
    edmmultiLineTextEntryClass *me = (edmmultiLineTextEntryClass *) clientData;
    XtRemoveCallback (w, XmNlosingFocusCallback,
                      (XtCallbackProc)text_noedit_callback, me);
    me->editing = false;
#ifdef DEBUG_TEXTWIDGETS
    fprintf ( stderr, "multiLineTextEntry: Quit editing '%s'\n",
              (me->data_pv ? me->data_pv->get_name () : "<no PV>"));
#endif
    pv_value_callback (me->data_pv, me);
}

 void edmmultiLineTextEntryClass::text_entered_callback (Widget w,
                                                         XtPointer clientData,
                                                         XtPointer)
{
#ifdef DEBUG
    printf ("Start of edmmultiLineTextEntryClass::text_entered_callback\n");
#endif
    edmmultiLineTextEntryClass *me = (edmmultiLineTextEntryClass *) clientData;
    me->callback_common (w, clientData);
    me->editing= false;
}

void edmmultiLineTextEntryClass::map ( void )
{
    if ( widget ) XtMapWidget ( widget );
}

void edmmultiLineTextEntryClass::unmap ( void )
{
    if ( widget ) XtUnmapWidget ( widget );
}

// crawler functions may return blank pv names
char *edmmultiLineTextUpdateClass::crawlerGetFirstPv ( void ) 
{
    crawlerPvIndex = 0;
    return data_pv_name.getExpanded ();

}

char *edmmultiLineTextUpdateClass::crawlerGetNextPv ( void ) 
{
    if ( crawlerPvIndex >= 1 ) return NULL;
    crawlerPvIndex++;
    if ( crawlerPvIndex == 1 ) 
    {
        return colour_pv_name.getExpanded ();
    }
    return NULL;
}
