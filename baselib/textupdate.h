// -*- C++ -*-
// EDM textupdate Widget
//
// kasemir@lanl.gov

#ifndef __TEXTUPDATE_H__
#define __TEXTUPDATE_H__

#include "act_grf.h"
#include "entry_form.h"
#include "pv_factory.h"

#define TEXTUPDATE_CLASSNAME "TextupdateClass"
#define TEXTENTRY_CLASSNAME "TextentryClass"
#define TEXT_MAJOR 1
#define TEXT_MINOR 1
#define TEXT_RELEASE 0

class edmTextupdateClass : public activeGraphicClass
{
public:
    edmTextupdateClass();
    edmTextupdateClass(edmTextupdateClass *rhs);
    virtual ~edmTextupdateClass();
    char *objName();
    const char *getRawPVName();
    const char *getExpandedPVName();
    
    // Load/save
    int save(FILE *f);
    int createFromFile(FILE *fptr, char *name, activeWindowClass *actWin);

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
                             unsigned int textFgColor,
                             unsigned int fg1Color,
                             unsigned int fg2Color,
                             unsigned int offsetColor,
                             unsigned int bgColor,
                             unsigned int topShadowColor,
                             unsigned int botShadowColor);

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
    int expand1st(int numMacros, char *macros[], char *expansions[]);
    int expand2nd(int numMacros, char *macros[], char *expansions[]);
    
    // Execute
    int activate(int pass, void *ptr);
    int deactivate(int pass);
    int drawActive();
    int eraseActive();
    void executeDeferred();
    
protected:
    void init(const char *classname);
    void clone(const edmTextupdateClass *rhs, const char *classname);

    bool is_executing;          // edit or execute mode?
    bool is_pvname_valid;       
    ProcessVariable *pv;        // ChannelAccess, PV
    
    // Properties
    expStringClass pv_exp_str;  // PV name as macro-expandable string

    typedef enum { dm_default, dm_decimal, dm_hex } DisplayMode;
    DisplayMode displayMode;
    int precision;
    
    pvColorClass lineColor;
    efInt line_width;
    pvColorClass fillColor;
    int is_filled;
    fontMenuClass fm;
    char fontTag[63+1], bufFontTag[63+1];
    XFontStruct *fs;
    int alignment, fontAscent, fontDescent, fontHeight;

    // Helpers for createInteractive & edit,
    // buffers for property dialog
    int genericEdit();
    int editCreate();
    int bufX, bufY, bufW, bufH;
    char bufPvName[39+1];
    int buf_displayMode;
    int buf_precision;
    unsigned int bufLineColor;
    efInt buf_line_width;
    colorButtonClass lineCb;
    unsigned int bufFillColor;
    colorButtonClass fillCb;
    int bufIsFilled;

    void redraw_text(Display *dis,
                     Drawable drw,
                     gcClass &gcc,
                     GC gc,
                     const char *text,
                     size_t len);
    void remove_text(Display *dis,
                     Drawable drw,
                     gcClass &gcc,
                     GC gc);

    // Callbacks for property edit
    static void edit_update(Widget w, XtPointer client, XtPointer call);
    static void edit_ok(Widget w, XtPointer client, XtPointer call);
    static void edit_apply(Widget w, XtPointer client, XtPointer call);
    static void edit_cancel(Widget w, XtPointer client, XtPointer call);
    static void edit_cancel_delete(Widget w, XtPointer client,
                                   XtPointer call);
    // CA callbacks
    static void pv_status_callback(ProcessVariable *pv, void *userarg);
    static void pv_value_callback(ProcessVariable *pv, void *userarg);
};

class edmTextentryClass : public edmTextupdateClass
{
public:
    edmTextentryClass();
    edmTextentryClass(const edmTextentryClass *rhs);
protected:
    Widget widget;
    int activate(int pass, void *ptr);
    int deactivate(int pass);
    int drawActive();
    int eraseActive();

private:
    static void text_entered_callback(Widget, XtPointer, XtPointer);
    static void text_restore_callback(Widget, XtPointer, XtPointer);
};


#endif
