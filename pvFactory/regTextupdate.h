// -*- C++ -*-
// EDM regTextupdate Widget
//
// pamgurd@sns.gov after kasemir@lanl.gov

#ifndef __REGTEXTUPDATE_H__
#define __REGTEXTUPDATE_H__

#include"textupdate.h"
extern "C"
{
#include<regex.h>
}

#define REGTEXTUPDATE_CLASSNAME "RegTextupdateClass"
#define REGTEXTENTRY_CLASSNAME "RegTextentryClass"
#define REGTEXT_MAJOR 1
#define REGTEXT_MINOR 1
#define REGTEXT_RELEASE 0

class edmRegTextupdateClass : protected edmTextupdateClass {
public:
    edmRegTextupdateClass();
    edmRegTextupdateClass(edmRegTextupdateClass *rhs);
//    virtual ~edmRegTextupdateClass();
//    char *objName();
//    const char *getRawPVName();
//    const char *getExpandedPVName();
    
    // Load/save
    int save(FILE *f);
    int createFromFile(FILE *fptr, char *name, activeWindowClass *actWin);

    // Edit Mode
    int createInteractive(activeWindowClass *aw_obj,
                          int x, int y, int w, int h);
    int edit();
//   int draw();
//   int erase();
//    int checkResizeSelectBox(int _x, int _y, int _w, int _h);
//    int checkResizeSelectBoxAbs(int _x, int _y, int _w, int _h);
    
        // Group Edit
//    void changeDisplayParams(unsigned int flag,
//                             char *fontTag,
//                            int alignment,
//                             char *ctlFontTag,
//                             int ctlAlignment,
//                             char *btnFontTag,
//                             int btnAlignment,
//                             int textFgColor,
//                             int fg1Color,
//                             int fg2Color,
//                             int offsetColor,
//                             int bgColor,
//                             int topShadowColor,
//                             int botShadowColor);

//    void changePvNames(int flag,
//                       int numCtlPvs,
//                       char *ctlPvs[],
//                       int numReadbackPvs,
//                       char *readbackPvs[],
//                       int numNullPvs,
//                       char *nullPvs[],
//                       int numVisPvs,
//                       char *visPvs[],
//                       int numAlarmPvs,
//                       char *alarmPvs[]);
    
    // Macro support
//    int containsMacros();
//    int expand1st(int numMacros, char *macros[], char *expansions[]);
//    int expand2nd(int numMacros, char *macros[], char *expansions[]);
    
    // Execute
    int activate(int pass, void *ptr);
    int deactivate(int pass);
    int drawActive();
//    int eraseActive();
//    void executeDeferred();
    
protected:
//    void init(const char *classname);
    void clone(const edmRegTextupdateClass *rhs, const char *classname);

    // Helpers for createInteractive & edit
    int genericEdit();
    int editCreate();
    // buffers for property dialog
    char regExpStr[PV_Factory::MAX_PV_NAME+1];
    char bufRegExp[PV_Factory::MAX_PV_NAME+1];

    regex_t compiled_re;
    bool    re_valid;

//    void redraw_text(Display *dis,
//                     Drawable drw,
//                     gcClass &gcc,
//                     GC gc,
//                     const char *text,
//                     size_t len);
//    void remove_text(Display *dis,
//                     Drawable drw,
//                     gcClass &gcc,
//                     GC gc);

    // Callbacks for property edit
    static void edit_update(Widget w, XtPointer client, XtPointer call);
    static void edit_ok(Widget w, XtPointer client, XtPointer call);
    static void edit_apply(Widget w, XtPointer client, XtPointer call);
//    static void edit_cancel(Widget w, XtPointer client, XtPointer call);
//    static void edit_cancel_delete(Widget w, XtPointer client,
//                                   XtPointer call);
    // CA callbacks
//    static void pv_status_callback(ProcessVariable *pv, void *userarg);
//    static void pv_value_callback(ProcessVariable *pv, void *userarg);
};


#endif



