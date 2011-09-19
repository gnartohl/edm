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
#define REGTEXT_MAJOR 10
#define REGTEXT_MINOR 0
#define REGTEXT_RELEASE 0

class edmRegTextupdateClass : protected edmTextupdateClass {
public:
    edmRegTextupdateClass();
    edmRegTextupdateClass(edmRegTextupdateClass *rhs);
    
    // Load/save
    int save(FILE *f);
    int old_save(FILE *f);
    int createFromFile(FILE *fptr, char *name, activeWindowClass *actWin);
    int old_createFromFile(FILE *fptr, char *name, activeWindowClass *actWin);

    // Edit Mode
    int edit();
    
    // Execute
    int activate(int pass, void *ptr);
    int deactivate(int pass);
    int drawActive();

    char *getSearchString (
      int i
    );
    
    void replaceString (
      int i,
      int max,
      char *string
    );

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

    // Callbacks for property edit
    static void edit_update(Widget w, XtPointer client, XtPointer call);
    static void edit_ok(Widget w, XtPointer client, XtPointer call);
    static void edit_apply(Widget w, XtPointer client, XtPointer call);
};


#endif



