// -*- C++ -*-
// EDM strip Widget
//
// kasemir@lanl.gov
//
// Changes:
// 3.0.0  Use color name, fall back to index
// 2.0.0  Add text & line color properties

#ifndef __STRIP_H__
#define __STRIP_H__

#define SCIPLOT 

#include "act_grf.h"
#include "entry_form.h"
#include "pv_factory.h"
#include "strip_data.h"

#define STRIP_CLASSNAME "StripClass"
#define STRIP_MAJOR 4
#define STRIP_MINOR 0
#define STRIP_RELEASE 0

class edmStripClass : public activeGraphicClass
{
public:
    edmStripClass();
    edmStripClass(const edmStripClass *rhs);
    ~edmStripClass();
    char *objName();
    const char *PVName(size_t i, bool expanded=false);
    
    // Load/save
    int save(FILE *f);
    int old_save(FILE *f);
    int createFromFile(FILE *fptr, char *name, activeWindowClass *actWin);
    int old_createFromFile(FILE *fptr, char *name, activeWindowClass *actWin);

    // Edit Mode
    int createInteractive(activeWindowClass *aw_obj,
                          int x, int y, int w, int h);
    int edit();
    int draw();
    int erase();
    int checkResizeSelectBox(int _x, int _y, int _w, int _h);
    int checkResizeSelectBoxAbs(int _x, int _y, int _w, int _h);
    
        // Group Edit
    void changeDisplayParams(unsigned int flag,
                             char *fontTag,
                             int alignment,
                             char *ctlFontTag,
                             int ctlAlignment,
                             char *btnFontTag,
                             int btnAlignment,
                             int textFgColor,
                             int fg1Color,
                             int fg2Color,
                             int offsetColor,
                             int bgColor,
                             int topShadowColor,
                             int botShadowColor);
    void changePvNames(int flag,
                       int numCtlPvs,
                       char *ctlPvs[],
                       int numReadbackPvs,
                       char *readbackPvs[],
                       int numNullPvs,
                       char *nullPvs[],
                       int numVisPvs,
                       char *visPvs[],
                       int numAlarmPvs,
                       char *alarmPvs[]);
    
    // Macro support
    int containsMacros();
    int expandTemplate (int numMacros, char *macros[], char *expansions[]);
    int expand1st(int numMacros, char *macros[], char *expansions[]);
    int expand2nd(int numMacros, char *macros[], char *expansions[]);

    char *crawlerGetFirstPv ( void );
    char *crawlerGetNextPv ( void );
    
    // Execute
    int activate(int pass, void *ptr);
    int deactivate(int pass);
    int drawActive();
    int eraseActive();
    void executeDeferred();
    
private:
    bool is_executing;          // edit or execute mode?
    XtIntervalId timer;

    // Properties
    enum { num_pvs=6 }; // fixed for now
    expStringClass pv_name[num_pvs];
    int pv_color[num_pvs];
    bool use_pv_time[num_pvs];
    double seconds;
    int update_ms;
    efInt line_width;
    int bgColor, textColor, fgColor;
    
    char font_tag[63+1];
    int alignment;

    // Helpers for createInteractive & edit,
    // buffers for property dialog
    int genericEdit();
    int editCreate();
    int bufX, bufY, bufW, bufH;
    // ***** SJS Modification 04/04/2003 *****
    // ***** Replace *****
    // char buf_pv_name[num_pvs][39+1];
    // ***** by *****
    char buf_pv_name[num_pvs][PV_Factory::MAX_PV_NAME + 1];
    // ***** End of modification *****
    int buf_pv_color[num_pvs];
    colorButtonClass pv_color_cb[num_pvs];
    int buf_use_pv_time[num_pvs];
    double buf_seconds;
    int buf_update_ms;
    efInt buf_line_width;
    int buf_bgColor, buf_textColor, buf_fgColor;
    colorButtonClass bgCb, textCb, fgCb;

    fontMenuClass fm;
    XFontStruct *fs;
    int fontAscent, fontDescent, fontHeight;

    bool is_pvname_valid[num_pvs];
    ProcessVariable *pv[num_pvs];        // ChannelAccess, PV
    StripData *strip_data;

#ifdef SCIPLOT
    Widget plot_widget;
    int list_id[num_pvs];
    double *xlist, *ylist[num_pvs];
    Cursor cursor;
#else
    Pixmap pixmap;
    GC     pixmap_GC;
#endif
    
    // Callbacks for property edit
    static void edit_update(Widget w, XtPointer client, XtPointer call);
    static void edit_ok(Widget w, XtPointer client, XtPointer call);
    static void edit_apply(Widget w, XtPointer client, XtPointer call);
    static void edit_cancel(Widget w, XtPointer client, XtPointer call);
    static void edit_cancel_delete(Widget w, XtPointer client,
                                   XtPointer call);
    // CA callbacks
    static void pv_conn_state_callback(ProcessVariable *pv, void *userarg);
    static void pv_value_callback(ProcessVariable *pv, void *userarg);

    // X callbacks
    static void timer_callback(XtPointer call, XtIntervalId *id);
    static void button_callback(Widget w, XtPointer call, XButtonEvent *event);
};

#endif
