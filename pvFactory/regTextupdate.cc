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
    textColor.setColorIndex(actWin->defaultFg1Color, actWin->ci);
    line_width.setNull(1);
    fillColor.setColorIndex(actWin->defaultBgColor, actWin->ci);
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
void edmRegTextupdateClass::edit_update(Widget w, XtPointer client,XtPointer call)
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

void edmRegTextupdateClass::edit_apply(Widget w, XtPointer client, XtPointer call)
{
    edit_update(w, client, call);
    edmTextupdateClass::edit_apply(w, client, call);
}
    
// --------------------------------------------------------
// Execute
// --------------------------------------------------------

int edmRegTextupdateClass::drawActive()
{
    if (!is_executing)
        return 1;
    actWin->executeGc.saveFg();

    char text[80];
    int len;
    if (pv && pv->is_valid())
    {
        switch (displayMode)
        {
            case dm_hex:
                if (pv->get_type().type < ProcessVariable::Type::enumerated)
                {
                    cvtLongToHexString(pv->get_int(), text);
                    len = strlen(text);
                    break;
                }
            case dm_decimal:
                if (pv->get_type().type < ProcessVariable::Type::enumerated)
                {
                    cvtDoubleToString(pv->get_double(), text,
                                      (unsigned short) precision);
                    len = strlen(text);
                    break;
                }
            default:
                len = pv->get_string(text, 80);
        }
        if (strcmp(regExpStr, "")) {
    	    if (char* cregex = regcmp(regExpStr, 0)) {
    	        char newText[80];
    	        if ( regex(cregex, text, newText) ) {
    		    strncpy(text, newText, 79);
    	        }
    	        free(cregex);
    	    }
        }
    }
    else
    {
        text[0] = '<';
        strcpy(text+1, getExpandedPVName());
        strcat(text, ">");
        len = strlen(text);
    }
    redraw_text(actWin->d,
                XtWindow(actWin->executeWidget),
                actWin->executeGc,
                actWin->executeGc.normGC(),
                text, len);
   
    actWin->executeGc.restoreFg();
    return 1;
}
