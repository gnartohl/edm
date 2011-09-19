// -*- C++ -*-
// EDM textupdate Widget
//
// kasemir@lanl.gov
//
// Changes:
// 7.0.0  Added alarm-sensitive border
// 6.0.0  Mods for site-independent color
// 5.0.0  Use color index instead of name as per EDM video conference
// 4.0.0  Added "Alarm Sensitive" to text color
// 3.0.0  Added colorPv for color rules
// 2.0.0  Use color name, fall back to index
// 1.1.0  Added displayMode & precision

#ifndef __TEXTUPDATE_H__
#define __TEXTUPDATE_H__

#include "act_grf.h"
#include "entry_form.h"
#include "color_helper.h"

#define TEXTUPDATE_CLASSNAME "TextupdateClass"
#define TEXTENTRY_CLASSNAME  "TextentryClass"
#define TEXT_MAJOR 10
#define TEXT_MINOR 0
#define TEXT_RELEASE 0

static void drag(Widget w, XEvent *e, String *params, Cardinal numParams);
static void selectDrag(Widget w, XEvent *e, String *params,
 Cardinal numParams);
static void selectActions(Widget w, XEvent *e, String *params,
 Cardinal numParams);
static void pvInfo(Widget w, XEvent *e, String *params,
 Cardinal numParams);

class edmTextupdateClass : public activeGraphicClass
{
public:
    edmTextupdateClass();
    edmTextupdateClass(edmTextupdateClass *rhs);
    virtual ~edmTextupdateClass();
    char *objName();
    
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

    void getPvs(int max,
      ProcessVariable *pvs[],
      int *n);

    char *getSearchString (
      int i
    );

    void replaceString (
      int i,
      int max,
      char *string
    );

    char *crawlerGetFirstPv ( void );
    char *crawlerGetNextPv ( void );
    
    // Macro support
    int containsMacros();
    int expandTemplate (int numMacros, char *macros[], char *expansions[]);
    int expand1st(int numMacros, char *macros[], char *expansions[]);
    int expand2nd(int numMacros, char *macros[], char *expansions[]);
    
    // Execute
    int activate(int pass, void *ptr);
    int deactivate(int pass);
    int drawActive();
    int eraseActive();
    void executeDeferred();

    // Drag & drop support
    char *firstDragName();
    char *nextDragName();
    char *dragValue(int i);
    
protected:
    void init(const char *classname);
    void clone(const edmTextupdateClass *rhs, const char *classname);

    bool is_executing;          // edit or execute mode?
    bool is_pv_valid, is_color_pv_valid;
    ProcessVariable *pv, *color_pv;
    
    // Properties
    expStringClass pv_name;  // PV names as macro-expandable string
    expStringClass color_pv_name;

    typedef enum
    { dm_default, dm_decimal, dm_hex, dm_eng, dm_exp } DisplayMode;
    DisplayMode displayMode;
    int precision;

    // line color == text color except for alarm sensitivity
    ColorHelper textColor, fillColor, lineColor;
    efInt line_width;
    int is_line_alarm_sensitive;
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
    char bufPvName[PV_Factory::MAX_PV_NAME+1];
    char bufColorPvName[PV_Factory::MAX_PV_NAME+1];
    int buf_displayMode;
    int buf_precision;
    int buf_alarm_sensitive, buf_alarm_sensitive_line;
    int bufTextColor, bufFillColor;
    
    efInt buf_line_width;
    colorButtonClass textCb;
    colorButtonClass fillCb;
    int bufIsFilled;

    // Get text & color value.
    // len has to be initialized with the text buffer size.
    // Returns 1 if PV is valid
    bool get_current_values(char *text, size_t &len);

    entryListBase *lineEntry, *alarmSensLineEntry;
    entryListBase *fillEntry, *fillColorEntry;

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
    static void pv_conn_state_callback(ProcessVariable *pv, void *userarg);
    static void pv_value_callback(ProcessVariable *pv, void *userarg);
};

class edmTextentryClass : public edmTextupdateClass
{
public:
    edmTextentryClass();
    edmTextentryClass(const edmTextentryClass *rhs);
protected:
    Widget widget;
    bool editing;
    int activate(int pass, void *ptr);
    int deactivate(int pass);
    int drawActive();
    int eraseActive();
    void map( void );
    void unmap( void );
private:
    static void text_edit_callback(Widget, XtPointer, XtPointer);
    static void text_noedit_callback(Widget, XtPointer, XtPointer);
    static void text_entered_callback(Widget, XtPointer, XtPointer);
};

#endif
