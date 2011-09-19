// -*- C++ -*-
// EDM RegTextupdate Widget
//
// Applies regular expression to text before displaying it
//
// pamgurd@sns.gov after kasemir@lanl.gov

#include "regTextupdate.h"
#include "app_pkg.h"
#include "act_win.h"
#include "pv_factory.h"
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
    strncpy( regExpStr, rhs->regExpStr, PV_Factory::MAX_PV_NAME);
    re_valid = false;
}

// --------------------------------------------------------
// Load/save
// --------------------------------------------------------
int edmRegTextupdateClass::save(FILE *f)
{

int stat;

tagClass tag;

char *emptyStr = "";

    edmTextupdateClass::save(f); // save all the normal textupdate stuff

    // plus what RegTextupdate adds
    tag.init();
    tag.loadW( "# Additional properties" );
    tag.loadW( "beginObjectProperties" );
    tag.loadW( "regExpr", regExpStr, emptyStr );
    tag.loadW( "endObjectProperties" );
    tag.loadW( "" );

    stat = tag.writeTags( f );

    return stat;

}

int edmRegTextupdateClass::old_save(FILE *f)
{

    edmTextupdateClass::save(f); // save all the normal textupdate stuff

    writeStringToFile(f, regExpStr);
    				// plus what RegTextupdate adds
    
    return 1;
}

int edmRegTextupdateClass::createFromFile(FILE *f, char *name,
                                          activeWindowClass *_actWin)
{

int stat;

tagClass tag;

char *emptyStr = "";

    edmTextupdateClass::createFromFile(f, name, _actWin);

    tag.init();
    tag.loadR( "beginObjectProperties" );
    tag.loadR( "regExpr", PV_Factory::MAX_PV_NAME, regExpStr, emptyStr );
    tag.loadR( "endObjectProperties" );

    stat = tag.readTags( f, "endObjectProperties" );

    if ( !( stat & 1 ) ) {
      actWin->appCtx->postMessage( tag.errMsg() );
    }

    return stat;

}

int edmRegTextupdateClass::old_createFromFile(FILE *f, char *name,
                                          activeWindowClass *_actWin)
{
    edmTextupdateClass::old_createFromFile(f, name, _actWin);

    // read regular expression from file
    readStringFromFile(regExpStr, PV_Factory::MAX_PV_NAME, f);
    actWin->incLine();
    return 1;
}

// --------------------------------------------------------
// Edit Mode
// --------------------------------------------------------

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
    	strncpy(bufRegExp, regExpStr, PV_Factory::MAX_PV_NAME);
    else
    	strncpy(bufRegExp, "", PV_Factory::MAX_PV_NAME);
    ef.addTextField("Reg. Exp", 30, bufRegExp, PV_Factory::MAX_PV_NAME);

    return 1;
}

// Callbacks from property dialog
void edmRegTextupdateClass::edit_update(Widget w, XtPointer client,
                                        XtPointer call)
{
    edmRegTextupdateClass *me = (edmRegTextupdateClass *) client;
    edmTextupdateClass::edit_update(w, client, call);
    strncpy(me->regExpStr, me->bufRegExp, PV_Factory::MAX_PV_NAME);
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
                fprintf( stderr,"Error in regular expression: %s\n", buf);
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

    char text[80];
    size_t len = 80;
    
    if (get_current_values(text, len) &&
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
                drawable(actWin->executeWidget),
                actWin->executeGc,
                actWin->executeGc.normGC(),
                text, len);
   
    actWin->executeGc.restoreFg();
    return 1;
}

char *edmRegTextupdateClass::getSearchString (
  int i
) {

  if ( i == 0 ) {
    return pv_name.getRaw();
  }
  else if ( i == 1 ) {
    return color_pv_name.getRaw();
  }
  else if ( i == 2 ) {
    return regExpStr;
  }

  return NULL;

}

void edmRegTextupdateClass::replaceString (
  int i,
  int max,
  char *string
) {

  if ( i == 0 ) {
    pv_name.setRaw( string );
  }
  else if ( i == 1 ) {
    color_pv_name.setRaw( string );
  }
  else if ( i == 2 ) {
    int l = max;
    if ( PV_Factory::MAX_PV_NAME < max ) l = PV_Factory::MAX_PV_NAME;
    strncpy( regExpStr, string, l );
    regExpStr[l] = 0;
  }

}
