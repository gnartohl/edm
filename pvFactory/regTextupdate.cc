// -*- C++ -*-
// EDM RegTextupdate Widget
//
// Applies regular expression to text before displaying it
//
// pamgurd@sns.gov after kasemir@lanl.gov

#include "regTextupdate.h"
#include "app_pkg.h"
#include "act_win.h"
#include "epics_pv_factory.h"
#include "cvtFast.h"
#include <libgen.h>

edmRegTextupdateClass::edmRegTextupdateClass()
{
    init(REGTEXTUPDATE_CLASSNAME);
    strcpy( regExpStr, "");
    re_valid = false;
}

edmRegTextupdateClass::edmRegTextupdateClass(edmRegTextupdateClass *rhs)
{
    clone(rhs, REGTEXTUPDATE_CLASSNAME);
}

void edmRegTextupdateClass::clone(const edmRegTextupdateClass *rhs,
                                  const char *classname)
{
    edmTextupdateClass::clone(rhs, classname);
    strncpy( regExpStr, rhs->regExpStr, 39);
    re_valid = false;
}

// --------------------------------------------------------
// Load/save
// --------------------------------------------------------
int edmRegTextupdateClass::save(FILE *f)
{

    edmTextupdateClass::save(f); // save all the normal textupdate stuff

    writeStringToFile(f, regExpStr);
    				// plus what RegTextupdate adds
    
    return 1;
}

int edmRegTextupdateClass::createFromFile(FILE *f, char *name,
                                          activeWindowClass *_actWin)
{
    edmTextupdateClass::createFromFile(f, name, _actWin);

    // read regular expression from file
    readStringFromFile(regExpStr, 39, f);     
    actWin->incLine();
    return 1;
}

// --------------------------------------------------------
// Edit Mode
// --------------------------------------------------------

// Idea of next two and helper methods:
// createInteractive -> editCreate -> genericEdit (delete on cancel)
// edit -> genericEdit (ignore changes on cancel)
int edmRegTextupdateClass::createInteractive(activeWindowClass *aw_obj,
                                             int _x, int _y, int _w, int _h)
{   // required
    actWin = (activeWindowClass *) aw_obj;
    x = _x; y = _y; w = _w; h = _h;
    // Honor display scheme
    displayMode = dm_default;
    precision = 0;
    textColor = actWin->defaultFg1Color;
    line_width.setNull(1);
    fillColor = actWin->defaultBgColor;
    strcpy(fontTag, actWin->defaultCtlFontTag);
    alignment = actWin->defaultCtlAlignment;
    fs = actWin->fi->getXFontStruct(fontTag);
    updateFont(fontTag, &fs, &fontAscent, &fontDescent, &fontHeight);

    // initialize and draw some kind of default image for the user
    draw();
    editCreate();
    return 1;
}

int edmRegTextupdateClass::edit()
{   // Popup property dialog, cancel -> no delete
    genericEdit();
    ef.finished(edit_ok, edit_apply, edit_cancel, this);
    actWin->currentEf = &ef;
    ef.popup();
    return 1;
}

int edmRegTextupdateClass::editCreate()
{
    // Popup property dialog, cancel -> delete
    genericEdit();
    ef.finished(edit_ok, edit_apply, edit_cancel_delete, this);
    actWin->currentEf = NULL;
    ef.popup();
    return 1;
}

int edmRegTextupdateClass::genericEdit() // create Property Dialog
{
    edmTextupdateClass::genericEdit();
    // add dialog box entry field for regular expression
    if (regExpStr)
    	strncpy(bufRegExp, regExpStr, 39);
    else
    	strncpy(bufRegExp, "", 39);
    ef.addTextField("Reg. Exp", 27, bufRegExp, 39);

    return 1;
}

// Callbacks from property dialog
void edmRegTextupdateClass::edit_update(Widget w, XtPointer client,
                                        XtPointer call)
{
    edmRegTextupdateClass *me = (edmRegTextupdateClass *) client;
    edmTextupdateClass::edit_update(w, client, call);
    strncpy(me->regExpStr, me->bufRegExp, 40 );
}

void edmRegTextupdateClass::edit_ok(Widget w, XtPointer client, XtPointer call)
{
    edit_update(w, client, call);
    edmTextupdateClass::edit_ok(w, client, call);
}

void edmRegTextupdateClass::edit_apply(Widget w, XtPointer client,
                                       XtPointer call)
{
    edit_update(w, client, call);
    edmTextupdateClass::edit_apply(w, client, call);
}
    
// --------------------------------------------------------
// Execute
// --------------------------------------------------------

int edmRegTextupdateClass::activate(int pass, void *ptr)
{
    if (pass == 1)
    {
        re_valid = false;
        if (strlen(regExpStr) > 0)
        {
            int res = regcomp(&compiled_re, regExpStr, REG_EXTENDED);
            if (res)
            {
                char buf[100];
                regerror(res, &compiled_re, buf, sizeof buf);
                printf("Error in regular expression: %s\n", buf);
            }
            else
                re_valid = true;
        }
    }
    return edmTextupdateClass::activate(pass, ptr);
}

int edmRegTextupdateClass::deactivate(int pass)
{
    if (pass == 1 && re_valid)
    {
        regfree(&compiled_re);
        re_valid = false;
    }
    return edmTextupdateClass::deactivate(pass);
}

int edmRegTextupdateClass::drawActive()
{
    if (!is_executing)
        return 1;
    actWin->executeGc.saveFg();

    double color_value;
    char text[80];
    size_t len = 80;
    if (get_current_values(text, len, color_value) &&
        re_valid)
    {
        regmatch_t pmatch[2];
        if (regexec(&compiled_re, text, 2, pmatch, 0) == 0)
        {
            // copy matched substring into display string
            // match 0 is always the full match,
            // match 1 is the first selected substring
            int start = pmatch[1].rm_so;
            int size = pmatch[1].rm_eo - pmatch[1].rm_so;
            
            if (start >= 0)
            {
                memmove(text, text+start, size);
                text[size] = '\0';
                len = size;
            }
            else
            {
                text[0] = '\0';
                len = 0;
            }
        }
    }

    redraw_text(actWin->d,
                XtWindow(actWin->executeWidget),
                actWin->executeGc,
                actWin->executeGc.normGC(),
                text, len, color_value);
   
    actWin->executeGc.restoreFg();
    return 1;
}
