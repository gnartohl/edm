// -*- C++ -*-
// EDM strip Widget
//
// kasemir@lanl.gov

#include "strip.h"
#include "app_pkg.h"
#include "act_win.h"
#include "pv_factory.h"
#ifdef SCIPLOT
#include "SciPlot.h"
#endif

edmStripClass::edmStripClass()
{
    name = strdup(STRIP_CLASSNAME);
    checkBaseClassVersion( activeGraphicClass::MAJOR_VERSION, name );
    is_executing = false;
    for (size_t i=0; i<num_pvs; ++i)
    {
        pv[i] = 0;
        use_pv_time[i] = true;
#ifdef SCIPLOT
        list_id[i] = 0;
        ylist[i] = 0;
#endif
    }
    seconds = 60.0;
    update_ms = 1000;
    strcpy(font_tag, "");
    fs = 0;
    alignment = XmALIGNMENT_BEGINNING;
    strip_data = 0;
#ifdef SCIPLOT
    plot_widget = 0;
    xlist = 0;
#else
    pixmap = 0;
#endif
}

edmStripClass::edmStripClass(const edmStripClass *rhs)
{
    // This next line must always be included
    activeGraphicClass *ago = (activeGraphicClass *) this;
    ago->clone((activeGraphicClass *)rhs);

    name = strdup(STRIP_CLASSNAME);

    is_executing = false;
    for (size_t i=0; i<num_pvs; ++i)
    {
        pv[i] = 0;
        pv_name[i].setRaw(rhs->pv_name[i].rawString);
        pv_color[i] = rhs->pv_color[i];
        use_pv_time[i] = rhs->use_pv_time[i];
#ifdef SCIPLOT
        list_id[i] = 0;
        ylist[i] = 0;
#endif
    }
    seconds = rhs->seconds;
    update_ms = rhs->update_ms;
    line_width = rhs->line_width;
    bgColor = rhs->bgColor;
    textColor = rhs->textColor;
    fgColor = rhs->fgColor;
    strncpy(font_tag, rhs->font_tag, 63);
    font_tag[63] = 0;
    fs = actWin->fi->getXFontStruct(font_tag);
    fontAscent = rhs->fontAscent;
    fontDescent = rhs->fontDescent;
    fontHeight = rhs->fontHeight;
    alignment = rhs->alignment;
    strip_data = 0;
#ifdef SCIPLOT
    plot_widget = 0;
    xlist = 0;
#else
    pixmap = 0;
#endif
}

edmStripClass::~edmStripClass()
{
    for (size_t i=0; i<num_pvs; ++i)
    {
        if (pv[i])
        {
            pv[i]->remove_conn_state_callback(pv_conn_state_callback, this);
            pv[i]->remove_value_callback(pv_value_callback, this);
            pv[i]->release();
            pv[i] = 0;
        }
    }
    free(name);
}

char *edmStripClass::objName()
{   return STRIP_CLASSNAME; }

const char *edmStripClass::PVName(size_t i, bool expanded)
{
    char *s = expanded ? pv_name[i].getExpanded() : pv_name[i].getRaw();
    return s ? s : "";
}

// --------------------------------------------------------
// Load/save
// --------------------------------------------------------
int edmStripClass::save(FILE *f)
{

    int i, stat, major, minor, release;

    int tmp[num_pvs];

    tagClass tag;

    int zero = 0;
    int n = (int) num_pvs;
    double dzero = 0;
    char *emptyStr = "";

    int left = XmALIGNMENT_BEGINNING;
    static char *alignEnumStr[3] = {
      "left",
      "center",
      "right"
    };
    static int alignEnum[3] = {
      XmALIGNMENT_BEGINNING,
      XmALIGNMENT_CENTER,
      XmALIGNMENT_END
    };

    major = STRIP_MAJOR;
    minor = STRIP_MINOR;
    release = STRIP_RELEASE;

    tag.init();
    tag.loadW( "beginObjectProperties" );
    tag.loadW( "major", &major );
    tag.loadW( "minor", &minor );
    tag.loadW( "release", &release );

    tag.loadW( "# Geometry" );
    tag.loadW( "x", &x );
    tag.loadW( "y", &y );
    tag.loadW( "w", &w );
    tag.loadW( "h", &h );

    tag.loadW( "# Trace Properties" );
    tag.loadW( "numPvs", &n );
    tag.loadW( "yPv", pv_name, (int) num_pvs, emptyStr );
    tag.loadW( "plotColor", actWin->ci, pv_color, (int) num_pvs );
    for ( i=0; i<num_pvs; i++ ) {
      tmp[i] = (int) ( use_pv_time[i] ? 1 : 0 );
    }
    tag.loadW( "usePvTime", tmp, (int) num_pvs, &zero );

    tag.loadW( "# Operating Modes" );
    tag.loadW( "updateTime", &seconds, &dzero );

    tag.loadW( "# Appearance" );
    tag.loadW( "lineThickness", &line_width );
    tag.loadW( "fgColor", actWin->ci, &fgColor );
    tag.loadW( "bgColor", actWin->ci, &bgColor );
    tag.loadW( "textColor", actWin->ci, &textColor );
    tag.loadW( "font", font_tag );
    tag.loadW( "fontAlign", 3, alignEnumStr, alignEnum, &alignment, &left );

    tag.loadW( "updateMs", &update_ms );

    tag.loadW( unknownTags );
    tag.loadW( "endObjectProperties" );
    tag.loadW( "" );
    
    stat = tag.writeTags( f );

    return stat;

}

int edmStripClass::old_save(FILE *f)
{
    // Version, bounding box
    fprintf(f, "%-d %-d %-d\n",
            STRIP_MAJOR, STRIP_MINOR, STRIP_RELEASE);
    fprintf(f, "%-d\n", x);
    fprintf(f, "%-d\n", y);
    fprintf(f, "%-d\n", w);
    fprintf(f, "%-d\n", h);

    // PV Names & color
    fprintf(f, "%-d\n", num_pvs);
    for (size_t i=0; i<num_pvs; ++i)
    {
        writeStringToFile(f, (char *)PVName(i));
        writeStringToFile(f, actWin->ci->colorName(pv_color[i]));
        fprintf(f, "%-d\n", (int)(use_pv_time[i] ? 1 : 0));
    }
    fprintf(f, "%.1f\n", seconds);
    line_width.write(f);

    writeStringToFile(f, actWin->ci->colorName(bgColor));
    writeStringToFile(f, actWin->ci->colorName(textColor));
    writeStringToFile(f, actWin->ci->colorName(fgColor));
    
    writeStringToFile(f, font_tag);
    fprintf(f, "%-d\n", alignment);
    
    return 1;
}

int edmStripClass::createFromFile(FILE *f, char *filename,
                                  activeWindowClass *_actWin)
{

    int i, n, stat, major, minor, release;

    int tmp[num_pvs];

    tagClass tag;

    int zero = 0;
    int hundred = 100;
    double dzero = 0;
    char *emptyStr = "";

    int left = XmALIGNMENT_BEGINNING;
    static char *alignEnumStr[3] = {
      "left",
      "center",
      "right"
    };
    static int alignEnum[3] = {
      XmALIGNMENT_BEGINNING,
      XmALIGNMENT_CENTER,
      XmALIGNMENT_END
    };

    int file_num_pvs;

    actWin = _actWin;

    tag.init();
    tag.loadR( "beginObjectProperties" );
    tag.loadR( unknownTags );
    tag.loadR( "major", &major );
    tag.loadR( "minor", &minor );
    tag.loadR( "release", &release );

    tag.loadR( "# Geometry" );
    tag.loadR( "x", &x );
    tag.loadR( "y", &y );
    tag.loadR( "w", &w );
    tag.loadR( "h", &h );

    tag.loadR( "# Trace Properties" );
    tag.loadR( "numPvs", &file_num_pvs, &zero );
    tag.loadR( "yPv", (int) num_pvs, pv_name, &n, emptyStr );
    tag.loadR( "plotColor", actWin->ci, (int) num_pvs, pv_color,
     &n );
    tag.loadR( "usePvTime", (int) num_pvs, tmp, &n,
     &zero );
    for ( i=0; i<num_pvs; i++ ) {
      use_pv_time[i] = tmp[i] != 0;
    }

    tag.loadR( "# Operating Modes" );
    tag.loadR( "updateTime", &seconds, &dzero );

    tag.loadR( "# Appearance" );
    tag.loadR( "lineThickness", &line_width );
    tag.loadR( "fgColor", actWin->ci, &fgColor );
    tag.loadR( "bgColor", actWin->ci, &bgColor );
    tag.loadR( "textColor", actWin->ci, &textColor );
    tag.loadR( "font", 63, font_tag );
    tag.loadR( "fontAlign", 3, alignEnumStr, alignEnum, &alignment, &left );

    tag.loadR( "updateMs", &update_ms, &hundred );

    tag.loadR( "endObjectProperties" );

    stat = tag.readTags( f, "endObjectProperties" );

    if ( !( stat & 1 ) ) {
      actWin->appCtx->postMessage( tag.errMsg() );
    }

   if ( major > STRIP_MAJOR ) {
      postIncompatable();
      return 0;
    }

    if ( major < 4 ) {
      postIncompatable();
      return 0;
    }

    this->initSelectBox(); // call after getting x,y,w,h

    if (file_num_pvs != num_pvs)
    {
        fprintf(stderr,"File has Stripchart with %d PVs, can only handle %d\n",
                file_num_pvs, num_pvs);
        return 0;
    }

    actWin->fi->loadFontTag(font_tag);
    fs = actWin->fi->getXFontStruct(font_tag);
    updateFont(font_tag, &fs,
               &fontAscent, &fontDescent, &fontHeight);
    
    return 1;

}

int edmStripClass::old_createFromFile(FILE *f, char *filename,
                                  activeWindowClass *_actWin)
{
    int major, minor, release;
    int index;
    char name[PV_Factory::MAX_PV_NAME+1];

    actWin = _actWin;
    // Version, bounding box
    fscanf(f, "%d %d %d\n", &major, &minor, &release); actWin->incLine();

    if ( major > STRIP_MAJOR ) {
      postIncompatable();
      return 0;
    }

    fscanf(f, "%d\n", &x); actWin->incLine();
    fscanf(f, "%d\n", &y); actWin->incLine();
    fscanf(f, "%d\n", &w); actWin->incLine();
    fscanf(f, "%d\n", &h); actWin->incLine();
    this->initSelectBox(); // call after getting x,y,w,h
    // PV Name
    int tmpi;
    size_t file_num_pvs;
    fscanf(f, "%d\n", &tmpi); actWin->incLine();
    file_num_pvs = (size_t) tmpi;
    if (file_num_pvs != num_pvs)
    {
        fprintf(stderr,"File has Stripchart with %d PVs, can only handle %d\n",
                file_num_pvs, num_pvs);
        return 0;
    }
    for (size_t i=0; i<num_pvs; ++i)
    {
        readStringFromFile(name, PV_Factory::MAX_PV_NAME, f);
        actWin->incLine();
        pv_name[i].setRaw(name);
        if (major < 3)
        {
            fscanf(f, "%d\n", &pv_color[i]); actWin->incLine();
        }
        else
        {
            readStringFromFile(name, PV_Factory::MAX_PV_NAME, f);
            actWin->incLine();
            pv_color[i] = actWin->ci->colorIndexByName(name);
        }
        fscanf(f, "%d\n", &index); actWin->incLine();
        use_pv_time[i] = index != 0;
    }
    fscanf(f, "%lf\n", &seconds);

    line_width.read(f); actWin->incLine();

    if (major < 2)
    {
        fscanf(f, "%d\n", &index); actWin->incLine();
        bgColor = index;
        textColor = actWin->defaultTextFgColor;
        fgColor   = actWin->defaultFg1Color;
    }
    else if (major < 3)
    {
        fscanf(f, "%d\n", &index); actWin->incLine();
        bgColor = index;
        fscanf(f, "%d\n", &index); actWin->incLine();
        textColor = index;
        fscanf(f, "%d\n", &index); actWin->incLine();
        fgColor = index;
    }
    else
    {
        readStringFromFile(name, PV_Factory::MAX_PV_NAME, f);
        actWin->incLine();
        bgColor = actWin->ci->colorIndexByName(name);
        readStringFromFile(name, PV_Factory::MAX_PV_NAME, f);
        actWin->incLine();
        textColor = actWin->ci->colorIndexByName(name);
        readStringFromFile(name, PV_Factory::MAX_PV_NAME, f);
        actWin->incLine();
        fgColor = actWin->ci->colorIndexByName(name);
    }
    
    readStringFromFile(font_tag, 63, f); actWin->incLine();
    fscanf( f, "%d\n", &alignment ); actWin->incLine();
    actWin->fi->loadFontTag(font_tag);
    fs = actWin->fi->getXFontStruct(font_tag);
    updateFont(font_tag, &fs,
               &fontAscent, &fontDescent, &fontHeight);

    update_ms = 100;
    
    return 1;
}

// --------------------------------------------------------
// Edit Mode
// --------------------------------------------------------

static int default_RGB[][3] =
{
    { 65535, 30000, 30000 },
    { 30000, 65535, 30000 },
    { 30000, 30000, 65535 }, 
    { 65535, 65535, 30000 },
    { 65535, 32639, 30000 },
    { 8995, 36494, 8995 },
    { 30000, 43947, 59624  },
    { 44461, 60138, 60138 },
};

// Idea of next two and helper methods:
// createInteractive -> editCreate -> genericEdit (delete on cancel)
// edit -> genericEdit (ignore changes on cancel)
int edmStripClass::createInteractive(activeWindowClass *aw_obj,
                                     int _x, int _y, int _w, int _h)
{   // required
    actWin = (activeWindowClass *) aw_obj;
    x = _x; y = _y; w = _w; h = _h;

    unsigned int pixel;
    for (size_t i=0; i<num_pvs; ++i)
    {
        actWin->ci->colorInfoClass::setRGB(default_RGB[i][0],
                                           default_RGB[i][1],
                                           default_RGB[i][2],
                                           &pixel);
        pv_color[i] = actWin->ci->pixIndex(pixel);
    }
    bgColor = actWin->defaultBgColor;
    textColor = actWin->defaultTextFgColor;
    fgColor = actWin->defaultFg1Color;
    strcpy(font_tag, actWin->defaultCtlFontTag);
    alignment = actWin->defaultCtlAlignment;
    fs = actWin->fi->getXFontStruct(font_tag);
    updateFont(font_tag, &fs, &fontAscent, &fontDescent, &fontHeight);

    // initialize and draw some kind of default image for the user
    draw();
    editCreate();
    return 1;
}

int edmStripClass::edit()
{   // Popup property dialog, cancel -> no delete
    genericEdit();
    ef.finished(edit_ok, edit_apply, edit_cancel, this);
    actWin->currentEf = &ef;
    ef.popup();
    return 1;
}

int edmStripClass::editCreate()
{
    // Popup property dialog, cancel -> delete
    genericEdit();
    ef.finished(edit_ok, edit_apply, edit_cancel_delete, this);
    actWin->currentEf = NULL;
    ef.popup();
    return 1;
}

int edmStripClass::genericEdit() // create Property Dialog
{
    char title[32], *ptr;
    size_t i;
    // required
    ptr = actWin->obj.getNameFromClass(STRIP_CLASSNAME);
    if (ptr)
        strncpy(title, ptr, 31);
    else
        strncpy(title, "Unknown object Properties", 31 );
   
    // Copy data member contents into edit buffers
    bufX = x; bufY = y; bufW = w; bufH = h;
    for (i=0; i<num_pvs; ++i)
    {
        strncpy(buf_pv_name[i], PVName(i), PV_Factory::MAX_PV_NAME);
        buf_pv_color[i] = pv_color[i];
        buf_use_pv_time[i] = use_pv_time[i] ? 1 : 0;
    }
    buf_seconds = seconds;
    buf_update_ms = update_ms;
    buf_line_width = line_width;
    buf_bgColor = bgColor;
    buf_textColor = textColor;
    buf_fgColor = fgColor;

    // create entry form dialog box
    ef.create(actWin->top, actWin->appCtx->ci.getColorMap(),
              &actWin->appCtx->entryFormX, &actWin->appCtx->entryFormY,
              &actWin->appCtx->entryFormW, &actWin->appCtx->entryFormH,
              &actWin->appCtx->largestH,
              title, NULL, NULL, NULL);
    // add dialog box entry fields
    ef.addTextField("X", 35, &bufX);
    ef.addTextField("Y", 35, &bufY);
    ef.addTextField("Width", 35, &bufW);
    ef.addTextField("Height", 35, &bufH);
    for (i=0; i<num_pvs; ++i)
    {
        ef.beginSubForm();
        ef.addTextField("PV", 25, buf_pv_name[i], PV_Factory::MAX_PV_NAME);
        ef.addColorButton("", actWin->ci,
                          &pv_color_cb[i], &buf_pv_color[i]);
        ef.addLabel(" CA Time");
        ef.addToggle("", &buf_use_pv_time[i]);
        ef.endSubForm();
    }
    ef.addTextField("Period [s]", 35, &buf_seconds);
    ef.addTextField("Update Rate [ms]", 35, &buf_update_ms );
    ef.addTextField("Line Width", 35, &buf_line_width);
    ef.addColorButton("Background", actWin->ci,
                      &bgCb, &buf_bgColor);
    ef.addColorButton("Text", actWin->ci,
                      &textCb, &buf_textColor);
    ef.addColorButton("Foreground", actWin->ci,
                      &fgCb, &buf_fgColor);
    ef.addFontMenu("Font", actWin->fi, &fm, font_tag);
    fm.setFontAlignment(alignment);

    return 1;
}


int edmStripClass::draw()  // render the edit-mode image
{
    // required
    if (is_executing || deleteRequest)
        return 1;
    actWin->drawGc.saveFg();
 
    Display *dis = actWin->d;
    Drawable drw = XtWindow(actWin->drawWidget);
    gcClass &gcc = actWin->drawGc;
    GC gc = actWin->drawGc.normGC();

    // Background
    gcc.setFG(actWin->ci->pix(bgColor));
    XFillRectangle(dis, drw, gc, x, y, w, h);
    // Text
    gcc.setFG(actWin->ci->pix(fgColor)); // not used
    gcc.setFontTag(font_tag, actWin->fi);
    int ty = y;
    for (size_t i=0; i<num_pvs; ++i)
    {
        gcc.setFG(actWin->ci->pix(pv_color[i]));
        const char *text = PVName(i);
        size_t len = strlen(text);
        ty += fontAscent + 2*fontDescent;
        XDrawString(dis, drw, gc, x, ty, text, len);
    }

    actWin->drawGc.restoreFg();
    
    return 1;
}

int edmStripClass::erase()  // erase edit-mode image
{
    // required
    if (is_executing || deleteRequest )
        return 1;

    Display *dis = actWin->d;
    Drawable drw = XtWindow(actWin->drawWidget);
    gcClass &gcc =actWin->drawGc;
    GC gc = actWin->drawGc.eraseGC();
    
    gcc.setFG(actWin->ci->pix(bgColor));
    XFillRectangle(dis, drw, gc, x, y, w, h);
    
    return 1;
}

int edmStripClass::checkResizeSelectBox(int _x, int _y, int _w, int _h)
{   // Assert minimum size
    return checkResizeSelectBoxAbs(_x, _y, w+_w, h+_h);
}

int edmStripClass::checkResizeSelectBoxAbs(int _x, int _y, int _w, int _h)
{   // Similar, but absolute sizes. -1 is also possible
    if (_w != -1  &&  _w < 100)
        return 0;
    if (_h != -1  &&  _h < 100)
        return 0;
    return 1;
}

// Callbacks from property dialog
void edmStripClass::edit_update(Widget w, XtPointer client,XtPointer call)
{
    edmStripClass *me = (edmStripClass *) client;
    // required
    me->actWin->setChanged();
    me->eraseSelectBoxCorners();
    me->erase();

    me->x = me->bufX;
    me->sboxX = me->bufX;
    me->y = me->bufY;
    me->sboxY = me->bufY;
    me->w = me->bufW;
    me->sboxW = me->bufW;
    me->h = me->bufH;
    me->sboxH = me->bufH;

    for (size_t i=0; i<num_pvs; ++i)
    {
        me->pv_name[i].setRaw(me->buf_pv_name[i]);
        me->pv_color[i] = me->buf_pv_color[i];
        me->use_pv_time[i] = me->buf_use_pv_time[i] != 0;
    }
    me->seconds = me->buf_seconds;
    me->update_ms = me->buf_update_ms;
    me->line_width = me->buf_line_width;
    me->bgColor = me->buf_bgColor;
    me->textColor = me->buf_textColor;
    me->fgColor = me->buf_fgColor;

    strncpy(me->font_tag, me->fm.currentFontTag(), 63);
    me->font_tag[63] = 0;
    me->actWin->fi->loadFontTag(me->font_tag);
    me->fs = me->actWin->fi->getXFontStruct(me->font_tag);
    me->alignment = me->fm.currentFontAlignment();
    me->fs = me->actWin->fi->getXFontStruct(me->font_tag);
    me->updateFont(me->font_tag, &me->fs,
                   &me->fontAscent, &me->fontDescent, &me->fontHeight);
}

void edmStripClass::edit_ok(Widget w, XtPointer client, XtPointer call)
{
    edmStripClass *me = (edmStripClass *) client;
    edit_update(w, client, call);
    // required
    me->ef.popdown();
    me->operationComplete();
}

void edmStripClass::edit_apply(Widget w, XtPointer client, XtPointer call)
{
    edmStripClass *me = (edmStripClass *) client;
    edit_update(w, client, call);
    // required
    me->refresh(me);
}

void edmStripClass::edit_cancel(Widget w, XtPointer client,XtPointer call)
{
    edmStripClass *me = (edmStripClass *) client;
    // next two lines required
    me->ef.popdown();
    me->operationCancel();
}

void edmStripClass::edit_cancel_delete(Widget w, XtPointer client,
                                            XtPointer cal)
{
    edmStripClass *me = (edmStripClass *) client;
    // all lines required
    me->ef.popdown();
    me->operationCancel();
    me->erase();
    me->deleteRequest = 1;
    me->drawAll();
}

// --------------------------------------------------------
// GroupEdit
// --------------------------------------------------------
// edm-components.doc shows wrong prototype for these!
void edmStripClass::changeDisplayParams(unsigned int flag,
                                             char *_font_tag,
                                             int _alignment,
                                             char *_ctlFontTag,
                                             int _ctlAlignment,
                                             char *_btnFontTag,
                                             int _btnAlignment,
                                             int _textFgColor,
                                             int _fg1Color,
                                             int _fg2Color,
                                             int _offsetColor,
                                             int _bgColor,
                                             int _topShadowColor,
                                             int _botShadowColor)
{
    if (flag & ACTGRF_BGCOLOR_MASK)
        bgColor = _bgColor;
    if (flag & ACTGRF_TEXTFGCOLOR_MASK)
        textColor = _textFgColor;
    if (flag & ACTGRF_FG1COLOR_MASK)
        fgColor = _fg1Color;
    if (flag & ACTGRF_FONTTAG_MASK)
    {
        strcpy(font_tag, _font_tag);
        alignment = _alignment;
        fs = actWin->fi->getXFontStruct(font_tag);
        updateFont(font_tag, &fs,
                   &fontAscent, &fontDescent, &fontHeight);
    }
}

void edmStripClass::changePvNames(int flag,
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
        if (numReadbackPvs > num_pvs)
            numReadbackPvs = num_pvs;
        for (int i=0; i<numReadbackPvs; ++i)
            pv_name[i].setRaw(readbackPvs[i]);
    }
}
    
// --------------------------------------------------------
// Macro support
// --------------------------------------------------------
int edmStripClass::containsMacros()
{
    for (size_t i=0; i<num_pvs; ++i)
        if (pv_name[i].containsPrimaryMacros())
            return 1;
    return 0;
}

int edmStripClass::expandTemplate (
  int numMacros,
  char *macros[],
  char *expansions[]
) {

int i;
expStringClass tmpStr;

  for ( size_t i=0; i<num_pvs; ++i ) {

    tmpStr.setRaw( pv_name[i].getRaw() );
    tmpStr.expand1st( numMacros, macros, expansions );
    pv_name[i].setRaw( tmpStr.getExpanded() );

  }

  return 1;

}

int edmStripClass::expand1st(int numMacros, char *macros[],
                             char *expansions[])
{
    int state, sum = EXPSTR_SUCCESS;
    for (size_t i=0; i<num_pvs; ++i)
    {
        state = pv_name[i].expand1st(numMacros, macros, expansions);
        if (state != EXPSTR_SUCCESS)
            sum = state;
    }
    return sum;
}

int edmStripClass::expand2nd(int numMacros, char *macros[],
                             char *expansions[])
{
    int state, sum = EXPSTR_SUCCESS;
    for (size_t i=0; i<num_pvs; ++i)
    {
        state = pv_name[i].expand2nd(numMacros, macros, expansions);
        if (state != EXPSTR_SUCCESS)
            sum = state;
    }
    return sum;
}

// --------------------------------------------------------
// Execute
// --------------------------------------------------------
int edmStripClass::activate(int pass, void *ptr)
{
    size_t i;
#ifdef SCIPLOT
    size_t j;
    int color;
#endif
    switch (pass) // ... up to 6
    {
        case 1: // initialize
            // Check PV names
            for (i=0; i<num_pvs; ++i)
	        // is_pvname_valid[i] = strcmp(PVName(i, true), "") != 0;
	        is_pvname_valid[i] = !blankOrComment( (char *) PVName(i, true) );
            time_t now;
            time(&now);
            strip_data = new StripData(num_pvs, w, seconds, now, 0);

#ifdef SCIPLOT
            plot_widget = XtVaCreateManagedWidget("plot",
                                                  sciplotWidgetClass,
                                                  actWin->executeWidgetId(),
                                                  XtNx, (XtArgVal)x,
                                                  XtNy, (XtArgVal)y,
                                                  XtNheight,(XtArgVal)h,
                                                  XtNwidth, (XtArgVal)w,
                                                  XtNshowTitle, False,
                                                  XtNxLabel, "Time [s]",
                                                  XtNyLabel, "Value",
                                                  XtNchartType, XtCARTESIAN,
                                                  XtNdegrees, False,
                                                  NULL);

            if (!plot_widget)
            {
                fprintf( stderr,"Cannot create SciPlot widget\n");
                return 0;
            }
            XtMapWidget(plot_widget);
            color = SciPlotStoreAllocatedColor(plot_widget,
                                               actWin->ci->pix(bgColor));
            SciPlotSetBackgroundColor(plot_widget, color);
            color = SciPlotStoreAllocatedColor(plot_widget,
                                               actWin->ci->pix(textColor));
            SciPlotSetTextColor(plot_widget, color);
            color = SciPlotStoreAllocatedColor(plot_widget,
                                               actWin->ci->pix(fgColor));
            SciPlotSetForegroundColor(plot_widget, color);

            xlist = (double *)calloc(strip_data->getBucketCount()*3,
                                     sizeof(double));
            for (i=0, j=0; i<strip_data->getBucketCount(); ++i)
            {
                xlist[j] = strip_data->getBucketSecs(i);
                xlist[j+1] = xlist[j];
                xlist[j+2] = xlist[j];
                j += 3;
            }
            SciPlotSetXUserScale(plot_widget, xlist[0], xlist[j-1]);
            for (i=0; i<num_pvs; ++i)
            {
                if (is_pvname_valid[i])
                {
                    ylist[i]=(double *)calloc(strip_data->getBucketCount()*3,
                                              sizeof(double));
                    list_id[i] =
                        SciPlotListCreateDouble(plot_widget,
                                                strip_data->getBucketCount()*3,
                                                xlist, ylist[i],
                                                (char *)PVName(i, true));
                    color = SciPlotStoreAllocatedColor(
                        plot_widget, actWin->ci->pix(pv_color[i]));
                    SciPlotListSetStyle(plot_widget, list_id[i],
                                        color, XtMARKER_NONE,
                                        color, XtLINE_SOLID);
                }
            }
            SciPlotUpdate(plot_widget);
            XtAddEventHandler(plot_widget, ButtonPressMask, False,
                              (XtEventHandler)button_callback,
                              (XtPointer)this);
            actWin->cursor.set(XtWindow(plot_widget), CURSOR_K_CROSSHAIR);
#else
            // Create double-buffer pixmap and GC for this
            pixmap = XCreatePixmap(actWin->display(),
                                   XtWindow(actWin->executeWidget),
                                   w, h,
                                   DefaultDepth(
                                       actWin->display(),
                                       DefaultScreen(actWin->display()))
                                   );
            XGCValues values;                            
            pixmap_GC = XCreateGC(actWin->display(),
                                  pixmap,
                                  0, &values);
            XCopyGC(actWin->display(),
                    actWin->executeGc.normGC(),
                    GCFunction    | 
                    GCPlaneMask   |
                    GCForeground  |
                    GCBackground  |
                    GCLineStyle   |
                    GCJoinStyle		|
                    GCFillStyle		|
                    GCFillRule		|
                    GCFont 			,
                    pixmap_GC);
#endif
            aglPtr = ptr;
            is_executing = true;
            break;
        case 2: // connect to pv
            for (i=0; i<num_pvs; ++i)
            {
                if (pv[i])
                    fprintf( stderr,"strip::activate: pv %d already set to %s!\n",
                           i, pv[i]->get_name());
                if (is_pvname_valid[i])
                {
                    pv[i] = the_PV_Factory->create(PVName(i, true));
                    if (pv[i])
                    {
                        pv[i]->add_conn_state_callback(pv_conn_state_callback, this);
                        pv[i]->add_value_callback(pv_value_callback, this);
                    }
                }
            }
            break;
        case 3: // start scrolling
            timer = XtAppAddTimeOut(actWin->appCtx->appContext(),
                                    (unsigned long) update_ms,
                                    timer_callback,
                                    this);
            break;
    }
    return 1;
}

int edmStripClass::deactivate(int pass)
{
    is_executing = false;
    switch (pass)
    {
        case 1: // disconnect
            XtRemoveTimeOut(timer);
            for (size_t i=0; i<num_pvs; ++i)
            {
                if (pv[i])
                {
                    pv[i]->remove_conn_state_callback(pv_conn_state_callback, this);
                    pv[i]->remove_value_callback(pv_value_callback, this);
                    pv[i]->release();
                    pv[i] = 0;
                }
            }
            break;
        case 2: // E.g.: remove toolkit widgets
            delete strip_data;
            strip_data = 0;
#ifdef SCIPLOT
            if (plot_widget)
            {
                XtUnmapWidget(plot_widget);
                for (size_t i=0; i<num_pvs; ++i)
                {
                    if (!ylist[i])
                        continue;
                    SciPlotListDelete(plot_widget,list_id[i]);
                    list_id[i] = 0;
                    free(ylist[i]);
                    ylist[i] = 0;
                }
                free(xlist);
                xlist = 0;
                XtDestroyWidget(plot_widget);
                plot_widget = 0;
            }
#else
            XFreeGC(actWin->display(), pixmap_GC);
            XFreePixmap(actWin->display(), pixmap);
            pixmap = 0;
#endif
            break;
    }
    return 1;
}

int edmStripClass::drawActive()
{
    if (!is_executing)
        return 1;
#ifdef SCIPLOT
    size_t channel, i, j;
    const StripData::Bucket *b;
    
    strip_data->lock();
    for (channel=0; channel<num_pvs; ++channel)
    {
        if (!ylist[channel])
            continue;
        for (i=0, j=0; i<strip_data->getBucketCount(); ++i)
        {
            b = strip_data->getBucket(channel, i);
            if (b && b->state != StripData::Bucket::empty)
            {
                ylist[channel][j] = b->mini;
                ylist[channel][j+1] = b->maxi;
                ylist[channel][j+2] = b->last;
            }
            else
            {
                ylist[channel][j] = (double)SCIPLOT_SKIP_VAL;
                ylist[channel][j+1] = (double)SCIPLOT_SKIP_VAL;
                ylist[channel][j+2] = (double)SCIPLOT_SKIP_VAL;
            }
            j += 3;
        }
        SciPlotListUpdateDouble(plot_widget,list_id[channel],
                                strip_data->getBucketCount()*3,
                                xlist, ylist[channel]);
    }
    strip_data->unlock();
    //    if (!SciPlotQuickUpdate(plot_widget))
        SciPlotUpdate(plot_widget);
#else
    actWin->executeGc.saveFg();

    // ------------------------------------------------
    Display *dis = actWin->d;
    Drawable drw = pixmap;
    GC gc = pixmap_GC;
    int x0=0, y0=0;
    
    // Background fill?
    XSetForeground(dis, gc, actWin->ci->pix(bgColor));
    XFillRectangle(dis, drw, gc, x0, y0, w, h);
    
    unsigned int lw;
    if (line_width.isNull())
        lw = 0;
    else
        lw = line_width.value();
    XSetLineAttributes(dis, gc, lw, LineSolid, CapButt, JoinBevel);
    
    LinearTransformation val2y;
    val2y.setSource(-10.0, 10.0);
    val2y.setDestination(y0+h, y0);
    
    size_t channel, i;
    const StripData::Bucket *b;
    int ymin, ymid, ymax, l_ymid;

    strip_data->lock();
    for (channel=0; channel<num_pvs; ++channel)
    {
        l_ymid = -1;
        XSetForeground(dis, gc, pv_color[channel]);
        for (i=0; i<strip_data->getBucketCount(); ++i)
        {
            b = strip_data->getBucket(channel, i);
            if (b && b->state != StripData::Bucket::empty)
            {
                ymid = int(val2y.transform(b->last) + 0.5);
                if (l_ymid >= 0)
                    XDrawLine(dis, drw, gc, x0+i-1, l_ymid, x0+i, ymid);
                if (b->mini != b->last  ||
                    b->last != b->maxi)
                {
                    ymin = int(val2y.transform(b->mini) + 0.5);
                    ymax = int(val2y.transform(b->maxi) + 0.5);
                    XDrawLine(dis, drw, gc, x0+i, ymin, x0+i, ymax);
                }
                l_ymid = ymid;
            }
            else
                l_ymid = -1;
        }
    }
    strip_data->unlock();
    XCopyArea(actWin->display(), pixmap,
              drawable(actWin->executeWidget),
              actWin->executeGc.normGC(),
              0, 0, w, h, x, y);
    // --------------------------------------------------
    actWin->executeGc.restoreFg();
#endif
    return 1;
}

int edmStripClass::eraseActive()
{
    // Code like this would be used to erase the excecuting
    // image for a redraw
    // But this leads to flicker, so it's omitted
    return 1;
}

void edmStripClass::pv_conn_state_callback(ProcessVariable *cb_pv, void *userarg)
{
    if (! cb_pv)
        return;
    edmStripClass *me = (edmStripClass *)userarg;
    size_t i;
    me->actWin->appCtx->proc->lock();
    if (me->is_executing)
    {
        for (i=0; i<me->num_pvs; ++i)
        {
            if (cb_pv == me->pv[i])
            {
                if (cb_pv->is_valid())
                {
                    if (cb_pv->get_type().type >=
                        ProcessVariable::Type::text)
                    {
                        fprintf( stderr,"Stripchart: cannot plot PV '%s': type %s\n",
                               cb_pv->get_name(),
                               cb_pv->get_type().description);
                    }
                    else
                    {
                        // cannot use PV time here because that's the
                        // last PV process time which might be days ago
                        // for a passive input
                        struct timeval t;
                        gettimeofday(&t, 0);
                        me->strip_data->addSample(i,
                                                  (time_t) t.tv_sec,
                                                  t.tv_usec*1000,
                                                  cb_pv->get_double());
                    }
                }
                else
                    me->strip_data->discontinue(i);
                break;
            }
        }
        me->bufInvalidate();
        me->actWin->addDefExeNode(me->aglPtr);
    }
    me->actWin->appCtx->proc->unlock();
}

void edmStripClass::pv_value_callback(ProcessVariable *cb_pv, void *userarg)
{
    edmStripClass *me = (edmStripClass *)userarg;
    time_t secs;
    unsigned long nsecs;
    for (size_t i=0; i<me->num_pvs; ++i)
    {
        if (cb_pv == me->pv[i])
        {
            //fprintf( stderr,"new value: pv %d '%s'\n", i, cb_pv->get_name());
            if (me->use_pv_time[i])
            {
                secs = cb_pv->get_time_t();
                nsecs = cb_pv->get_nano();
            }
            else
            {
                struct timeval t;
                gettimeofday(&t, 0);
                secs = t.tv_sec;
                nsecs = t.tv_usec*1000;
            }
            me->strip_data->addSample(i, secs, nsecs, cb_pv->get_double());
            return;
        }
    }
}

void edmStripClass::timer_callback(XtPointer call, XtIntervalId *id)
{
    edmStripClass *me = (edmStripClass *)call;
    // Scroll: set now + extra space as new end of stripchart
    struct timeval t;
    gettimeofday(&t, 0);
    double extra_secs = me->seconds/20.0;
    long extra_s = (long)extra_secs;
    long extra_u = (long)((extra_secs - extra_s)*1e6);

    t.tv_sec += extra_s;
    t.tv_usec += extra_u;
    if (t.tv_usec > 1000000)
    {
        ++t.tv_sec;
        t.tv_usec -= 1000000;
    }
    me->strip_data->updateEnd((time_t)t.tv_sec, (unsigned long)1000*t.tv_usec);

    // Ask for redraw
    me->actWin->appCtx->proc->lock();
    if (me->is_executing)
        me->actWin->addDefExeNode(me->aglPtr);
    me->actWin->appCtx->proc->unlock();
    // schedule next scoll
    me->timer = XtAppAddTimeOut(me->actWin->appCtx->appContext(),
                                (unsigned long) me->update_ms,
                                timer_callback,
                                me);
}

void edmStripClass::button_callback(Widget w, XtPointer call, XButtonEvent *event)
{
    edmStripClass *me = (edmStripClass *) call;

    double x = SciPlotGetX(me->plot_widget, (int) event->x);
    double y = SciPlotGetY(me->plot_widget, (int) event->y);
    char xlabel[80], ylabel[80];
    if (event->button == 1)
    {
        sprintf(xlabel, "Time [s] @ %g", x);
        sprintf(ylabel, "Value @ %g", y);
    }
    else
    {
        sprintf(xlabel, "Time [s]");
        sprintf(ylabel, "Value");
    }
    XtVaSetValues(me->plot_widget,
                  XtNxLabel, xlabel,
                  XtNyLabel, ylabel,
                  NULL);
    
}

void edmStripClass::executeDeferred()
{   // Called as a result of addDefExeNode
    if (actWin->isIconified)
        return;
    actWin->appCtx->proc->lock();
    actWin->remDefExeNode(aglPtr);
    actWin->appCtx->proc->unlock();
    if (is_executing)
        smartDrawAllActive();
}

// crawler functions may return blank pv names
char *edmStripClass::crawlerGetFirstPv ( void ) {

  crawlerPvIndex = 0;
  if ( crawlerPvIndex < num_pvs ) return pv_name[crawlerPvIndex].getExpanded();

  return NULL;

}

char *edmStripClass::crawlerGetNextPv ( void ) {

  if ( crawlerPvIndex >= num_pvs ) return NULL;

  crawlerPvIndex++;
  return pv_name[crawlerPvIndex].getExpanded();

}
