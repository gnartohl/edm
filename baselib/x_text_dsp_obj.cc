//  edm - extensible display manager

//  Copyright (C) 1999 John W. Sinclair

//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.

//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.

//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#define __x_text_dsp_obj_cc 1

#include "x_text_dsp_obj.h"
#include "app_pkg.h"
#include "act_win.h"

#include "thread.h"

static int g_transInit = 1;
static XtTranslations g_parsedTrans;

static int g_initTextBorderCheck = 1;
static int g_showTextBorderAlways = 0;

static char g_dragTrans[] =
  "#override\n\
   ~Ctrl~Shift<Btn2Down>: startDrag()\n\
   Ctrl~Shift<Btn2Down>: pvInfo()\n\
   Shift Ctrl<Btn2Down>: dummy()\n\
   Shift Ctrl<Btn2Up>: selectActions()\n\
   Shift~Ctrl<Btn2Up>: selectDrag()";

static XtActionsRec g_dragActions[] = {
  { "startDrag", (XtActionProc) drag },
  { "pvInfo", (XtActionProc) pvInfo },
  { "dummy", (XtActionProc) dummy },
  { "selectActions", (XtActionProc) selectActions },
  { "selectDrag", (XtActionProc) selectDrag }
};

static void checkTextBorderAlways ( void ) {

  if ( g_initTextBorderCheck ) {

    g_initTextBorderCheck = 0;

    char *envPtr = getenv( environment_str16 );
    if ( envPtr ) {
      g_showTextBorderAlways = 1;
    }

  }

}

static int stringPut (
  ProcessVariable *id,
  int size,
  char *string
) {

  if ( size == 1 ) {

    id->putText( string );

  }
  else {

    id->putArrayText( string );

  }

  return 1;

}

static void eventHandler (
  Widget w,
  XtPointer client,
  XEvent *e,
  Boolean *continueToDispatch ) {

activeXTextDspClass *axtdo = (activeXTextDspClass *) client;
int stat;
char string[XTDC_K_MAX+1];
char *buf;

  *continueToDispatch = True;

  axtdo = (activeXTextDspClass *) client;

  if ( e->type == EnterNotify ) {

    if ( !axtdo->enabled ) return;

    if ( !axtdo->grabUpdate ) {

      XSetInputFocus( axtdo->actWin->display(),
       XtWindow(axtdo->tf_widget), RevertToNone, CurrentTime );

    }

    axtdo->grabUpdate = 1;

    *continueToDispatch = False;

  }
  else if ( e->type == LeaveNotify ) {

    if ( axtdo->changeValOnLoseFocus ) {

      buf = XmTextGetString( axtdo->tf_widget );
      strncpy( axtdo->entryValue, buf, XTDC_K_MAX );
      axtdo->entryValue[XTDC_K_MAX] = 0;
      XtFree( buf );
      strncpy( axtdo->curValue, axtdo->entryValue, XTDC_K_MAX );
      axtdo->curValue[XTDC_K_MAX] = 0;
      strncpy( string, axtdo->entryValue, XTDC_K_MAX );
      string[XTDC_K_MAX] = 0;

      if ( axtdo->pvExists ) {
        stat = stringPut( axtdo->pvId, axtdo->pvCount, string );
      }
      else {
        axtdo->grabUpdate = 0;
        axtdo->bufInvalidate();
        axtdo->actWin->appCtx->proc->lock();
        axtdo->needUpdate = 1;
        axtdo->actWin->addDefExeNode( axtdo->aglPtr );
        axtdo->actWin->appCtx->proc->unlock();
      }

    }
    else {

      axtdo->grabUpdate = 0;
      axtdo->bufInvalidate();
      axtdo->actWin->appCtx->proc->lock();
      axtdo->needUpdate = 1;
      axtdo->actWin->addDefExeNode( axtdo->aglPtr );
      axtdo->actWin->appCtx->proc->unlock();

    }

    *continueToDispatch = False;

  }

}

static void dropTransferProc (
  Widget w,
  XtPointer clientData,
  Atom *selType,
  Atom *type,
  XtPointer value,
  unsigned long *length,
  int format )
{

activeXTextDspClass *axtdo = (activeXTextDspClass *) clientData;
char *str = (char *) value;
int stat, ivalue, doPut;
double dvalue;
char string[XTDC_K_MAX+1], tmp[XTDC_K_MAX+1];

  if ( !axtdo ) return;

  if ( *type == XA_STRING ) {

    if ( str ) {

      switch ( axtdo->pvType ) {

      case ProcessVariable::specificType::real:
      case ProcessVariable::specificType::flt:

        doPut = 0;
        if ( axtdo->formatType == XTDC_K_FORMAT_HEX ) {
          if ( strlen( str ) > 2 ) {
            if ( ( strncmp( str, "0x", 2 ) != 0 ) &&
                 ( strncmp( str, "0X", 2 ) != 0 ) ) {
              strcpy( tmp, "0x" );
            }
            else {
              strcpy( tmp, "" );
            }
            Strncat( tmp, str, 15 );
            tmp[15] = 0;
	  }
	  else {
            strcpy( tmp, "0x" );
            Strncat( tmp, str, 15 );
            tmp[15] = 0;
          }
          if ( isLegalInteger(tmp) ) {
            doPut = 1;
            ivalue = strtol( tmp, NULL, 0 );
            dvalue = (double) ivalue;
	  }
	}
	else {
          if ( isLegalFloat(str) ) {
            doPut = 1;
            dvalue = atof( str );
          }
	}

        if ( doPut ) {

          if ( axtdo->pvExists ) {
	    //axtdo->pvId->put( dvalue );
            axtdo->putValueWithClip( dvalue );
          }
          else {
            axtdo->needUpdate = 1;
            axtdo->actWin->appCtx->proc->lock();
            axtdo->actWin->addDefExeNode( axtdo->aglPtr );
            axtdo->actWin->appCtx->proc->unlock();
          }

        }

        break;

      case ProcessVariable::specificType::shrt:
      case ProcessVariable::specificType::integer:

        if ( axtdo->formatType == XTDC_K_FORMAT_HEX ) {
          if ( strlen( str ) > 2 ) {
            if ( ( strncmp( str, "0x", 2 ) != 0 ) &&
                 ( strncmp( str, "0X", 2 ) != 0 ) ) {
              strcpy( tmp, "0x" );
            }
            else {
              strcpy( tmp, "" );
            }
            Strncat( tmp, str, 15 );
            tmp[15] = 0;
	  }
	  else {
            strcpy( tmp, "0x" );
            Strncat( tmp, str, 15 );
            tmp[15] = 0;
          }
	}
	else {
          strncpy( tmp, str, XTDC_K_MAX );
          tmp[XTDC_K_MAX] = 0;
	}
        if ( isLegalInteger(tmp) ) {
          ivalue = strtol( tmp, NULL, 0 );
          if ( axtdo->pvExists ) {
	    //axtdo->pvId->put( ivalue );
            axtdo->putValueWithClip( ivalue );
          }
          else {
            axtdo->needUpdate = 1;
            axtdo->actWin->appCtx->proc->lock();
            axtdo->actWin->addDefExeNode( axtdo->aglPtr );
            axtdo->actWin->appCtx->proc->unlock();
          }

        }
        break;

      case ProcessVariable::specificType::text:

        strncpy( string, str, XTDC_K_MAX );
        string[XTDC_K_MAX] = 0;
        if ( axtdo->pvExists ) {
	  stat = stringPut( axtdo->pvId, axtdo->pvCount, string );
        }
        else {
          axtdo->needUpdate = 1;
          axtdo->actWin->appCtx->proc->lock();
          axtdo->actWin->addDefExeNode( axtdo->aglPtr );
          axtdo->actWin->appCtx->proc->unlock();
        }

        break;

      }

    }

  }

}

static void handleDrop (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeXTextDspClass *axtdo;
XmDropProcCallback ptr = (XmDropProcCallback) call;
XmDropTransferEntryRec transferEntries[2];
XmDropTransferEntry transferList;
int n;
Arg args[10];
Widget dc;

  n = 0;
  XtSetArg( args[n], XmNuserData, (XtPointer) &axtdo ); n++;
  XtGetValues( w, args, n );
  if ( !axtdo ) return;

  dc = ptr->dragContext;

  n = 0;
  if ( ptr->dropAction != XmDROP ) {
    XtSetArg( args[n], XmNtransferStatus, XmTRANSFER_FAILURE ); n++;
    XtSetArg( args[n], XmNnumDropTransfers, 0 ); n++;
  }
  else {
    transferEntries[0].target = XA_STRING;
    transferEntries[0].client_data = (XtPointer) axtdo;
    transferList = transferEntries;
    XtSetArg( args[n], XmNdropTransfers, transferList ); n++;
    XtSetArg( args[n], XmNnumDropTransfers, 1 ); n++;
    XtSetArg( args[n], XmNtransferProc, dropTransferProc ); n++;
  }

  //  XtSetArg( args[n], XmN ); n++;

  XmDropTransferStart( dc, args, n );

}

static void doBlink (
  void *ptr
) {

activeXTextDspClass *axtdo = (activeXTextDspClass *) ptr;

  if ( !axtdo->activeMode ) {
    if ( axtdo->isSelected() ) axtdo->drawSelectBoxCorners(); // erase via xor
    if ( axtdo->smartRefresh ) {
      axtdo->smartDrawAll();
    }
    else {
      axtdo->draw();
    }
    if ( axtdo->isSelected() ) axtdo->drawSelectBoxCorners();
  }
  else {
    axtdo->bufInvalidate();
    if ( axtdo->smartRefresh ) {
      axtdo->smartDrawAllActive();
    }
    else {
      axtdo->drawActive();
    }
  }

}

static void unconnectedTimeout (
  XtPointer client,
  XtIntervalId *id )
{

activeXTextDspClass *axtdo = (activeXTextDspClass *) client;

  if ( !axtdo->init ) {
    axtdo->needToDrawUnconnected = 1;
    axtdo->needRefresh = 1;
    axtdo->actWin->addDefExeNode( axtdo->aglPtr );
  }

  axtdo->unconnectedTimer = 0;

}

static void xtdoRestoreValue (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeXTextDspClass *axtdo = (activeXTextDspClass *) client;
Arg args[10];
int n, l;
char *buf;

  axtdo->actWin->appCtx->proc->lock();
  axtdo->needRefresh = 1;
  axtdo->actWin->addDefExeNode( axtdo->aglPtr );
  axtdo->actWin->appCtx->proc->unlock();

  n = 0;
  XtSetArg( args[n], XmNcursorPositionVisible, (XtArgVal) False ); n++;
  XtSetValues( axtdo->tf_widget, args, n );

  buf = XmTextGetString( axtdo->tf_widget );
  l = strlen(buf);
  XtFree( buf );

  axtdo->grabUpdate = 0;

}

static void xtdoCancelStr (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeXTextDspClass *axtdo = (activeXTextDspClass *) client;

  axtdo->editDialogIsActive = 0;

}

static void xtdoSetCpValue (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeXTextDspClass *axtdo = (activeXTextDspClass *) client;
int stat;
unsigned int i, ii;
char tmp[XTDC_K_MAX+1];

  if ( axtdo->dateAsFileName ) {

    axtdo->cp.getDate( tmp, XTDC_K_MAX );
    tmp[XTDC_K_MAX] = 0;

    for ( i=0, ii=0; i<strlen(tmp)+1; i++ ) {

      if ( tmp[i] == '-' ) {
        // do nothing
      }
      else if ( tmp[i] == ' ' ) {
        axtdo->entryValue[ii] = '_';
        ii++;
      }
      else if ( tmp[i] == ':' ) {
        // do nothing
      }
      else {
        axtdo->entryValue[ii] = tmp[i];
        ii++;
      }

    }

  }
  else {

    axtdo->cp.getDate( axtdo->entryValue, XTDC_K_MAX );

  }

  strncpy( axtdo->curValue, axtdo->entryValue, XTDC_K_MAX );
  axtdo->curValue[XTDC_K_MAX] = 0;

  axtdo->editDialogIsActive = 0;

  if ( axtdo->pvExists ) {
    stat = stringPut( axtdo->pvId, axtdo->pvCount, (char *) &axtdo->curValue );
  }

  axtdo->actWin->appCtx->proc->lock();
  axtdo->needUpdate = 1;
  axtdo->actWin->addDefExeNode( axtdo->aglPtr );
  axtdo->actWin->appCtx->proc->unlock();

}

static void xtdoSetFsValue (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeXTextDspClass *axtdo = (activeXTextDspClass *) client;
int stat;
char tmp[XTDC_K_MAX+1], name[XTDC_K_MAX+1], *tk;

  if ( axtdo->fileComponent != XTDC_K_FILE_FULL_PATH ) {

    axtdo->fsel.getSelection( tmp, XTDC_K_MAX );

    tk = strtok( tmp, "/" );
    if ( tk ) {
      strncpy( name, tk, XTDC_K_MAX );
      name[XTDC_K_MAX] = 0;
    }
    else {
      strcpy( name, "" );
    }
    while ( tk ) {

      tk = strtok( NULL, "/" );
      if ( tk ) {
        strncpy( name, tk, XTDC_K_MAX );
        name[XTDC_K_MAX] = 0;
      }

    }

    if ( axtdo->fileComponent == XTDC_K_FILE_NAME ) {

      strncpy( tmp, name, XTDC_K_MAX );
      tmp[XTDC_K_MAX] = 0;
      tk = strtok( tmp, "." );
      if ( tk ) {
        strncpy( name, tk, XTDC_K_MAX );
        name[XTDC_K_MAX] = 0;
      }

    }

    strncpy( axtdo->entryValue, name, XTDC_K_MAX );

  }
  else {

    axtdo->fsel.getSelection( axtdo->entryValue, XTDC_K_MAX );

  }

  strncpy( axtdo->curValue, axtdo->entryValue, XTDC_K_MAX );
  axtdo->curValue[XTDC_K_MAX] = 0;

  axtdo->editDialogIsActive = 0;

  if ( axtdo->pvExists ) {
    stat = stringPut( axtdo->pvId, axtdo->pvCount, (char *) &axtdo->curValue );
  }

  axtdo->actWin->appCtx->proc->lock();
  axtdo->needUpdate = 1;
  axtdo->actWin->addDefExeNode( axtdo->aglPtr );
  axtdo->actWin->appCtx->proc->unlock();

}

static void xtdoCancelKp (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeXTextDspClass *axtdo = (activeXTextDspClass *) client;

  axtdo->editDialogIsActive = 0;

}

static void xtdoSetKpDoubleValue (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeXTextDspClass *axtdo = (activeXTextDspClass *) client;

  axtdo->editDialogIsActive = 0;
  //axtdo->pvId->put( axtdo->kpDouble );
  axtdo->putValueWithClip( axtdo->kpDouble );

}

static void xtdoSetKpIntValue (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeXTextDspClass *axtdo = (activeXTextDspClass *) client;

  axtdo->editDialogIsActive = 0;
  //axtdo->pvId->put( axtdo->kpInt );
  axtdo->putValueWithClip( axtdo->kpInt );

}

static void xtdoSetValueChanged (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeXTextDspClass *axtdo = (activeXTextDspClass *) client;

  axtdo->widget_value_changed = 1;

  if ( axtdo->changeCallback ) {
    (*axtdo->changeCallback)( axtdo );
  }

}

static void xtdoGrabUpdate (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeXTextDspClass *axtdo = (activeXTextDspClass *) client;

  if ( axtdo->inputFocusUpdatesAllowed ) return;

  if ( !axtdo->enabled ) return;

  if ( !axtdo->grabUpdate ) {

    //XSetInputFocus( axtdo->actWin->display(),
     // XtWindow(axtdo->actWin->executeWidget), RevertToNone, CurrentTime );

    XSetInputFocus( axtdo->actWin->display(),
     XtWindow(axtdo->tf_widget), RevertToNone, CurrentTime );

  }

  axtdo->grabUpdate = 1;

}

static void xtdoSetSelection (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeXTextDspClass *axtdo = (activeXTextDspClass *) client;
int l;
char *buf;
Arg args[10];
int n;

  axtdo->widget_value_changed = 0;

  buf = XmTextGetString( axtdo->tf_widget );
  l = strlen(buf);
  XtFree( buf );

  n = 0;
  XtSetArg( args[n], XmNcursorPositionVisible, (XtArgVal) True ); n++;
  XtSetValues( axtdo->tf_widget, args, n );

  if ( axtdo->autoSelect ) {
    XmTextSetSelection( axtdo->tf_widget, 0, l,
    XtLastTimestampProcessed( axtdo->actWin->display() ) );
  }

  XmTextSetInsertionPosition( axtdo->tf_widget, l );

}

static void xtdoTextFieldToStringA (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeXTextDspClass *axtdo = (activeXTextDspClass *) client;
int stat;
char string[XTDC_K_MAX+1];
char *buf;

  buf = XmTextGetString( axtdo->tf_widget );
  strncpy( axtdo->entryValue, buf, XTDC_K_MAX );
  axtdo->entryValue[XTDC_K_MAX] = 0;
  XtFree( buf );

  strncpy( axtdo->curValue, axtdo->entryValue, XTDC_K_MAX );
  axtdo->curValue[XTDC_K_MAX] = 0;
  strncpy( string, axtdo->entryValue, XTDC_K_MAX );
  string[XTDC_K_MAX] = 0;
  if ( axtdo->pvExists ) {
    stat = stringPut( axtdo->pvId, axtdo->pvCount, string );
  }
  else {
    axtdo->actWin->appCtx->proc->lock();
    axtdo->needUpdate = 1;
    axtdo->actWin->addDefExeNode( axtdo->aglPtr );
    axtdo->actWin->appCtx->proc->unlock();
  }

  //XmTextSetInsertionPosition( axtdo->tf_widget, 0 );

  if ( !axtdo->inputFocusUpdatesAllowed ) axtdo->grabUpdate = 0;

}

static void xtdoTextFieldToStringLF (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeXTextDspClass *axtdo = (activeXTextDspClass *) client;
int stat;
char string[XTDC_K_MAX+1];
char *buf;
Arg args[10];
int n;

  axtdo->grabUpdate = 0;

  n = 0;
  XtSetArg( args[n], XmNcursorPositionVisible, (XtArgVal) False ); n++;
  XtSetValues( axtdo->tf_widget, args, n );

  //XmTextSetInsertionPosition( axtdo->tf_widget, 0 );

  if ( !axtdo->widget_value_changed ) return;
 
  buf = XmTextGetString( axtdo->tf_widget );
  strncpy( axtdo->entryValue, buf, XTDC_K_MAX );
  axtdo->entryValue[XTDC_K_MAX] = 0;
  XtFree( buf );

  strncpy( axtdo->curValue, axtdo->entryValue, XTDC_K_MAX );
  axtdo->curValue[XTDC_K_MAX] = 0;
  strncpy( string, axtdo->entryValue, XTDC_K_MAX );
  string[XTDC_K_MAX] = 0;
  if ( axtdo->pvExists ) {
    stat = stringPut( axtdo->pvId, axtdo->pvCount, string );
  }
  else {
    axtdo->needUpdate = 1;
    axtdo->actWin->appCtx->proc->lock();
    axtdo->actWin->addDefExeNode( axtdo->aglPtr );
    axtdo->actWin->appCtx->proc->unlock();
  }

}

static void xtdoTextFieldToIntA (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeXTextDspClass *axtdo = (activeXTextDspClass *) client;
int ivalue;
char *buf, tmp[XTDC_K_MAX+1];

  buf = XmTextGetString( axtdo->tf_widget );
  strncpy( axtdo->entryValue, buf, XTDC_K_MAX );
  axtdo->entryValue[XTDC_K_MAX] = 0;
  XtFree( buf );

  if ( axtdo->formatType == XTDC_K_FORMAT_HEX ) {
    if ( strlen( axtdo->entryValue ) > 2 ) {
      if ( ( strncmp( axtdo->entryValue, "0x", 2 ) != 0 ) &&
           ( strncmp( axtdo->entryValue, "0X", 2 ) != 0 ) ) {
        strcpy( tmp, "0x" );
      }
      else {
        strcpy( tmp, "" );
      }
      Strncat( tmp, axtdo->entryValue, 15 );
      tmp[15] = 0;
    }
    else {
      strcpy( tmp, "0x" );
      Strncat( tmp, axtdo->entryValue, 15 );
      tmp[15] = 0;
    }
  }
  else {
    strncpy( tmp, axtdo->entryValue, XTDC_K_MAX );
    tmp[XTDC_K_MAX] = 0;
  }

  if ( isLegalInteger(tmp) ) {

    strncpy( axtdo->curValue, tmp, XTDC_K_MAX );
    axtdo->curValue[XTDC_K_MAX] = 0;

    ivalue = strtol( tmp, NULL, 0 );
    if ( axtdo->pvExists ) {
      //axtdo->pvId->put( ivalue );
      axtdo->putValueWithClip( ivalue );

    }
    else {
      axtdo->needUpdate = 1;
      axtdo->actWin->appCtx->proc->lock();
      axtdo->actWin->addDefExeNode( axtdo->aglPtr );
      axtdo->actWin->appCtx->proc->unlock();
    }

    //XmTextSetInsertionPosition( axtdo->tf_widget, 0 );

  }

  if ( !axtdo->inputFocusUpdatesAllowed ) axtdo->grabUpdate = 0;

}

static void xtdoTextFieldToIntLF (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeXTextDspClass *axtdo = (activeXTextDspClass *) client;
int ivalue;
char *buf;
Arg args[10];
int n;
char tmp[XTDC_K_MAX+1];

  axtdo->grabUpdate = 0;

  n = 0;
  XtSetArg( args[n], XmNcursorPositionVisible, (XtArgVal) False ); n++;
  XtSetValues( axtdo->tf_widget, args, n );

  //XmTextSetInsertionPosition( axtdo->tf_widget, 0 );

  if ( !axtdo->widget_value_changed ) return;

  buf = XmTextGetString( axtdo->tf_widget );
  strncpy( axtdo->entryValue, buf, XTDC_K_MAX );
  axtdo->entryValue[XTDC_K_MAX] = 0;
  XtFree( buf );

  if ( axtdo->formatType == XTDC_K_FORMAT_HEX ) {
    if ( strlen( axtdo->entryValue ) > 2 ) {
      if ( ( strncmp( axtdo->entryValue, "0x", 2 ) != 0 ) &&
           ( strncmp( axtdo->entryValue, "0X", 2 ) != 0 ) ) {
        strcpy( tmp, "0x" );
      }
      else {
        strcpy( tmp, "" );
      }
      Strncat( tmp, axtdo->entryValue, 15 );
      tmp[15] = 0;
    }
    else {
      strcpy( tmp, "0x" );
      Strncat( tmp, axtdo->entryValue, 15 );
      tmp[15] = 0;
    }
  }
  else {
    strncpy( tmp, axtdo->entryValue, XTDC_K_MAX );
    tmp[XTDC_K_MAX] = 0;
  }

  if ( isLegalInteger(tmp) ) {

    strncpy( axtdo->curValue, tmp, XTDC_K_MAX );
    axtdo->curValue[XTDC_K_MAX] = 0;

    ivalue = strtol( tmp, NULL, 0 );
    if ( axtdo->pvExists ) {
      //axtdo->pvId->put( ivalue );
      axtdo->putValueWithClip( ivalue );
    }
    else {
      axtdo->needUpdate = 1;
      axtdo->actWin->appCtx->proc->lock();
      axtdo->actWin->addDefExeNode( axtdo->aglPtr );
      axtdo->actWin->appCtx->proc->unlock();
    }

  }

}

static void xtdoTextFieldToDoubleA (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeXTextDspClass *axtdo = (activeXTextDspClass *) client;
int ivalue, doPut;
double dvalue;
char *buf, tmp[XTDC_K_MAX+1];

  buf = XmTextGetString( axtdo->tf_widget );
  strncpy( axtdo->entryValue, buf, XTDC_K_MAX );
  axtdo->entryValue[XTDC_K_MAX] = 0;
  XtFree( buf );

  doPut = 0;

  if ( axtdo->formatType == XTDC_K_FORMAT_HEX ) {

    if ( strlen( axtdo->entryValue ) > 2 ) {
      if ( ( strncmp( axtdo->entryValue, "0x", 2 ) != 0 ) &&
           ( strncmp( axtdo->entryValue, "0X", 2 ) != 0 ) ) {
        strcpy( tmp, "0x" );
      }
      else {
        strcpy( tmp, "" );
      }
      Strncat( tmp, axtdo->entryValue, 15 );
      tmp[15] = 0;
    }
    else {
      strcpy( tmp, "0x" );
      Strncat( tmp, axtdo->entryValue, 15 );
      tmp[15] = 0;
    }

    if ( isLegalInteger(tmp) ) {
      doPut = 1;
      ivalue = strtol( tmp, NULL, 0 );
      dvalue = (double) ivalue;
    }

  }
  else {

    strncpy( tmp, axtdo->entryValue, XTDC_K_MAX );
    tmp[XTDC_K_MAX] = 0;

    if ( isLegalFloat(tmp) ) {
      doPut = 1;
      dvalue = atof( tmp );
    }

  }

  if ( doPut ) {

    strncpy( axtdo->curValue, tmp, XTDC_K_MAX );
    axtdo->curValue[XTDC_K_MAX] = 0;

    if ( axtdo->pvExists ) {
      //axtdo->pvId->put( dvalue );
      axtdo->putValueWithClip( dvalue );
    }
    else {
      axtdo->needUpdate = 1;
      axtdo->actWin->appCtx->proc->lock();
      axtdo->actWin->addDefExeNode( axtdo->aglPtr );
      axtdo->actWin->appCtx->proc->unlock();
    }

    //XmTextSetInsertionPosition( axtdo->tf_widget, 0 );

  }

  if ( !axtdo->inputFocusUpdatesAllowed ) axtdo->grabUpdate = 0;

}

static void xtdoTextFieldToDoubleLF (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeXTextDspClass *axtdo = (activeXTextDspClass *) client;
int ivalue, doPut;
double dvalue;
char *buf, tmp[XTDC_K_MAX+1];
Arg args[10];
int n;

  axtdo->grabUpdate = 0;

  n = 0;
  XtSetArg( args[n], XmNcursorPositionVisible, (XtArgVal) False ); n++;
  XtSetValues( axtdo->tf_widget, args, n );

  //XmTextSetInsertionPosition( axtdo->tf_widget, 0 );

  if ( !axtdo->widget_value_changed ) return;

  buf = XmTextGetString( axtdo->tf_widget );
  strncpy( axtdo->entryValue, buf, XTDC_K_MAX );
  axtdo->entryValue[XTDC_K_MAX] = 0;
  XtFree( buf );

  doPut = 0;

  if ( axtdo->formatType == XTDC_K_FORMAT_HEX ) {

    if ( strlen( axtdo->entryValue ) > 2 ) {
      if ( ( strncmp( axtdo->entryValue, "0x", 2 ) != 0 ) &&
           ( strncmp( axtdo->entryValue, "0X", 2 ) != 0 ) ) {
        strcpy( tmp, "0x" );
      }
      else {
        strcpy( tmp, "" );
      }
      Strncat( tmp, axtdo->entryValue, 15 );
      tmp[15] = 0;
    }
    else {
      strcpy( tmp, "0x" );
      Strncat( tmp, axtdo->entryValue, 15 );
      tmp[15] = 0;
    }

    if ( isLegalInteger(tmp) ) {
      doPut = 1;
      ivalue = strtol( tmp, NULL, 0 );
      dvalue = (double) ivalue;
    }

  }
  else {

    strncpy( tmp, axtdo->entryValue, XTDC_K_MAX );
    tmp[XTDC_K_MAX] = 0;

    if ( isLegalFloat(tmp) ) {
      doPut = 1;
      dvalue = atof( tmp );
    }

  }

  if ( doPut ) {

    strncpy( axtdo->curValue, tmp, XTDC_K_MAX );
    axtdo->curValue[XTDC_K_MAX] = 0;

    if ( axtdo->pvExists ) {
      //axtdo->pvId->put( dvalue );
      axtdo->putValueWithClip( dvalue );
    }
    else {
      axtdo->needUpdate = 1;
      axtdo->actWin->appCtx->proc->lock();
      axtdo->actWin->addDefExeNode( axtdo->aglPtr );
      axtdo->actWin->appCtx->proc->unlock();
    }

  }

}

static void xtdo_monitor_connect_state (
  ProcessVariable *pv,
  void *userarg
) {

activeXTextDspClass *axtdo = (activeXTextDspClass *) userarg;

  axtdo->actWin->appCtx->proc->lock();

  if ( axtdo->activeMode ) {

    if ( pv->is_valid() ) {

      axtdo->pvType = (int) pv->get_specific_type().type;
      axtdo->pvCount = (int) pv->get_dimension();

      if ( axtdo->pvType == ProcessVariable::specificType::chr ) {
        if ( axtdo->pvCount == 1 ) {
          axtdo->pvType = ProcessVariable::specificType::integer;
	}
        else {
          axtdo->pvType = ProcessVariable::specificType::text;
	}
      }

      // if format is hex, force double/float type to long
      if ( axtdo->formatType == XTDC_K_FORMAT_HEX ) {
        if ( ( axtdo->pvType == ProcessVariable::specificType::real ) ||
             ( axtdo->pvType == ProcessVariable::specificType::flt ) ) {
          axtdo->pvType = ProcessVariable::specificType::integer;
	}
      }

      axtdo->connection.setPvConnected( (void *) axtdo->pvConnection );

      if ( axtdo->connection.pvsConnected() ) {
        axtdo->needConnectInit = 1;
        axtdo->actWin->addDefExeNode( axtdo->aglPtr );
      }

    }
    else {

      axtdo->connection.setPvDisconnected( (void *) axtdo->pvConnection );
      axtdo->fgColor.setDisconnected();
      axtdo->needRefresh = 1;
      axtdo->actWin->addDefExeNode( axtdo->aglPtr );

    }

  }

  axtdo->actWin->appCtx->proc->unlock();

}

static void xtdo_monitor_sval_connect_state (
  ProcessVariable *pv,
  void *userarg
) {

activeXTextDspClass *axtdo = (activeXTextDspClass *) userarg;

  axtdo->actWin->appCtx->proc->lock();

  if ( axtdo->activeMode ) {

    if ( pv->is_valid() ) {

      axtdo->svalPvType = (int) pv->get_specific_type().type;
      axtdo->svalPvCount = (int) pv->get_dimension();

      if ( axtdo->svalPvType == ProcessVariable::specificType::chr ) {
        if ( axtdo->svalPvCount == 1 ) {
          axtdo->svalPvType = ProcessVariable::specificType::integer;
	}
        else {
          axtdo->svalPvType = ProcessVariable::specificType::text;
	}
      }

      axtdo->connection.setPvConnected( (void *) axtdo->svalPvConnection );

      if ( axtdo->connection.pvsConnected() ) {
        axtdo->needConnectInit = 1;
        axtdo->actWin->addDefExeNode( axtdo->aglPtr );
      }

    }
    else {

      axtdo->connection.setPvDisconnected( (void *) axtdo->svalPvConnection );
      axtdo->fgColor.setDisconnected();
      axtdo->needRefresh = 1;
      axtdo->actWin->addDefExeNode( axtdo->aglPtr );

    }

  }

  axtdo->actWin->appCtx->proc->unlock();

}

static void xtdo_monitor_fg_connect_state (
  ProcessVariable *pv,
  void *userarg
) {

activeXTextDspClass *axtdo = (activeXTextDspClass *) userarg;

  axtdo->actWin->appCtx->proc->lock();

  if ( axtdo->activeMode ) {

    if ( pv->is_valid() ) {

      axtdo->connection.setPvConnected( (void *) axtdo->fgPvConnection );

      if ( axtdo->connection.pvsConnected() ) {
        axtdo->needConnectInit = 1;
        axtdo->actWin->addDefExeNode( axtdo->aglPtr );
      }

    }
    else {

      axtdo->connection.setPvDisconnected( (void *) axtdo->fgPvConnection );
      axtdo->fgColor.setDisconnected();
      axtdo->needRefresh = 1;
      axtdo->actWin->addDefExeNode( axtdo->aglPtr );

    }

  }

  axtdo->actWin->appCtx->proc->unlock();

}

static void XtextDspUpdate (
  ProcessVariable *pv,
  void *userarg
) {

activeXTextDspClass *axtdo = (activeXTextDspClass *) userarg;
int ivalue, st, sev;
unsigned short svalue;

  axtdo->actWin->appCtx->proc->lock();

  if ( axtdo->activeMode ) {

    switch ( axtdo->pvType ) {

    case ProcessVariable::specificType::text:

      pv->get_string( axtdo->curValue, XTDC_K_MAX );
      axtdo->curValue[XTDC_K_MAX] = 0;

      break;

    case ProcessVariable::specificType::flt:

      axtdo->curDoubleValue = pv->get_double();
      sprintf( axtdo->curValue, axtdo->format, axtdo->curDoubleValue );
      if ( !axtdo->noSval ) {
        if ( axtdo->nullDetectMode == 0 ) {
          if ( axtdo->curDoubleValue == axtdo->curSvalValue ) {
            axtdo->fgColor.setNull();
            axtdo->bufInvalidate();
	  }
	  else {
            axtdo->fgColor.setNotNull();
            axtdo->bufInvalidate();
	  }
	}
	else if ( axtdo->nullDetectMode == 1 ) {
          if ( axtdo->curSvalValue == 0 ) {
            axtdo->fgColor.setNull();
            axtdo->bufInvalidate();
	  }
	  else {
            axtdo->fgColor.setNotNull();
            axtdo->bufInvalidate();
	  }
	}
        else {
          axtdo->fgColor.setNotNull();
        }
      }
      break;

    case ProcessVariable::specificType::real:

      axtdo->curDoubleValue = pv->get_double();
      sprintf( axtdo->curValue, axtdo->format, axtdo->curDoubleValue );

      if ( !axtdo->noSval ) {
        if ( axtdo->nullDetectMode == 0 ) {
          if ( axtdo->curDoubleValue == axtdo->curSvalValue ) {
            axtdo->fgColor.setNull();
            axtdo->bufInvalidate();
	  }
	  else {
            axtdo->fgColor.setNotNull();
            axtdo->bufInvalidate();
	  }
	}
	else if ( axtdo->nullDetectMode == 1 ) {
          if ( axtdo->curSvalValue == 0 ) {
            axtdo->fgColor.setNull();
            axtdo->bufInvalidate();
	  }
	  else {
            axtdo->fgColor.setNotNull();
            axtdo->bufInvalidate();
	  }
	}
        else {
          axtdo->fgColor.setNotNull();
        }
      }

      break;

    case ProcessVariable::specificType::shrt:
    case ProcessVariable::specificType::integer:

      ivalue = pv->get_int();
      sprintf( axtdo->curValue, axtdo->format, ivalue );
      axtdo->curDoubleValue = pv->get_int();

      if ( !axtdo->noSval ) {
        if ( axtdo->nullDetectMode == 0 ) {
          if ( axtdo->curDoubleValue == axtdo->curSvalValue ) {
            axtdo->fgColor.setNull();
            axtdo->bufInvalidate();
	  }
	  else {
            axtdo->fgColor.setNotNull();
            axtdo->bufInvalidate();
	  }
	}
	else if ( axtdo->nullDetectMode == 1 ) {
          if ( axtdo->curSvalValue == 0 ) {
            axtdo->fgColor.setNull();
            axtdo->bufInvalidate();
	  }
	  else {
            axtdo->fgColor.setNotNull();
            axtdo->bufInvalidate();
	  }
	}
        else {
          axtdo->fgColor.setNotNull();
        }
      }

      break;

    case ProcessVariable::specificType::enumerated:

      svalue = (unsigned short) pv->get_int();
      if ( svalue < pv->get_enum_count() ) {
        strncpy( axtdo->curValue, pv->get_enum( svalue ), XTDC_K_MAX );
	axtdo->curValue[XTDC_K_MAX] = 0;
        axtdo->entryState = (int) svalue;
      }
      else {
        strcpy( axtdo->curValue, "" );
      }
      axtdo->curDoubleValue = pv->get_int();

      if ( !axtdo->noSval ) {
        if ( axtdo->nullDetectMode == 0 ) {
          if ( axtdo->curDoubleValue == axtdo->curSvalValue ) {
            axtdo->fgColor.setNull();
            axtdo->bufInvalidate();
	  }
	  else {
            axtdo->fgColor.setNotNull();
            axtdo->bufInvalidate();
	  }
	}
	else if ( axtdo->nullDetectMode == 1 ) {
          if ( axtdo->curSvalValue == 0 ) {
            axtdo->fgColor.setNull();
            axtdo->bufInvalidate();
	  }
	  else {
            axtdo->fgColor.setNotNull();
            axtdo->bufInvalidate();
	  }
	}
        else {
          axtdo->fgColor.setNotNull();
        }
      }

      break;

    default:
      strcpy( axtdo->curValue, "" );
      axtdo->curDoubleValue = 0;
      break;

    } // end switch

    if ( !blank( axtdo->curValue ) ) {
      if ( axtdo->showUnits && !blank( axtdo->units ) ) {
        Strncat( axtdo->curValue, " ", XTDC_K_MAX );
        Strncat( axtdo->curValue, axtdo->units, XTDC_K_MAX );
      }
    }

    st = pv->get_status();
    sev = pv->get_severity();
    if ( ( st != axtdo->oldStat ) || ( sev != axtdo->oldSev ) ) {
      axtdo->oldStat = st;
      axtdo->oldSev = sev;
      axtdo->fgColor.setStatus( st, sev );
      axtdo->bufInvalidate();
      axtdo->needRefresh = 1;
    }

    axtdo->needUpdate = 1;
    axtdo->actWin->addDefExeNode( axtdo->aglPtr );

  }

  axtdo->actWin->appCtx->proc->unlock();

}

static void XtextDspSvalUpdate (
  ProcessVariable *pv,
  void *userarg
) {

activeXTextDspClass *axtdo = (activeXTextDspClass *) userarg;

  axtdo->actWin->appCtx->proc->lock();

  if ( axtdo->activeMode ) {

    switch ( axtdo->svalPvType ) {

    case ProcessVariable::specificType::integer:
    case ProcessVariable::specificType::enumerated:
    case ProcessVariable::specificType::shrt:

      axtdo->curSvalValue = pv->get_int();

      if ( axtdo->nullDetectMode == 0 ) {
        if ( axtdo->curDoubleValue == axtdo->curSvalValue ) {
          axtdo->fgColor.setNull();
        }
        else {
          axtdo->fgColor.setNotNull();
        }
      }
      else if ( axtdo->nullDetectMode == 1 ) {
        if ( axtdo->curSvalValue == 0 ) {
          axtdo->fgColor.setNull();
        }
        else {
          axtdo->fgColor.setNotNull();
        }
      }
      else {
        axtdo->fgColor.setNotNull();
      }

      break;

    case ProcessVariable::specificType::flt:

      axtdo->curSvalValue = pv->get_double();

      if ( axtdo->nullDetectMode == 0 ) {
        if ( axtdo->curDoubleValue == axtdo->curSvalValue ) {
          axtdo->fgColor.setNull();
        }
        else {
          axtdo->fgColor.setNotNull();
        }
      }
      else if ( axtdo->nullDetectMode == 1 ) {
        if ( axtdo->curSvalValue == 0 ) {
          axtdo->fgColor.setNull();
        }
        else {
          axtdo->fgColor.setNotNull();
        }
      }
      else {
        axtdo->fgColor.setNotNull();
      }

      break;

    case ProcessVariable::specificType::real:

      axtdo->curSvalValue = pv->get_double();

      if ( axtdo->nullDetectMode == 0 ) {
        if ( axtdo->curDoubleValue == axtdo->curSvalValue ) {
          axtdo->fgColor.setNull();
        }
        else {
          axtdo->fgColor.setNotNull();
        }
      }
      else if ( axtdo->nullDetectMode == 1 ) {
        if ( axtdo->curSvalValue == 0 ) {
          axtdo->fgColor.setNull();
        }
        else {
          axtdo->fgColor.setNotNull();
        }
      }
      else {
        axtdo->fgColor.setNotNull();
      }

      break;

    default:
      axtdo->curSvalValue = 0;
      break;

    } // end switch

    axtdo->noSval = 0;
    axtdo->bufInvalidate();
    axtdo->needRefresh = 1;
    axtdo->actWin->addDefExeNode( axtdo->aglPtr );

  }

  axtdo->actWin->appCtx->proc->unlock();

}

static void XtextDspFgUpdate (
  ProcessVariable *pv,
  void *userarg
) {

activeXTextDspClass *axtdo = (activeXTextDspClass *) userarg;
double val;
int index;

  axtdo->actWin->appCtx->proc->lock();

  if ( axtdo->activeMode ) {

    val = pv->get_double();
    index = axtdo->actWin->ci->evalRule( axtdo->fgColor.pixelIndex(), val );
    axtdo->fgColor.changeIndex( index, axtdo->actWin->ci );
    axtdo->bufInvalidate();
    axtdo->needRefresh = 1;
    axtdo->actWin->addDefExeNode( axtdo->aglPtr );

  }

  axtdo->actWin->appCtx->proc->unlock();

}

static void axtdc_value_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeXTextDspClass *axtdo = (activeXTextDspClass *) client;
double dvalue;
int ivalue, stat, doPut;
short svalue;
char string[XTDC_K_MAX+1], tmp[XTDC_K_MAX+1];

  strncpy( axtdo->curValue, axtdo->entryValue, XTDC_K_MAX );
  axtdo->curValue[XTDC_K_MAX] = 0;

  switch ( axtdo->pvType ) {

  case ProcessVariable::specificType::real:
  case ProcessVariable::specificType::flt:

    doPut = 0;

    if ( axtdo->formatType == XTDC_K_FORMAT_HEX ) {

      if ( strlen( axtdo->entryValue ) > 2 ) {
        if ( ( strncmp( axtdo->entryValue, "0x", 2 ) != 0 ) &&
             ( strncmp( axtdo->entryValue, "0X", 2 ) != 0 ) ) {
          strcpy( tmp, "0x" );
        }
        else {
          strcpy( tmp, "" );
        }
        Strncat( tmp, axtdo->entryValue, 15 );
        tmp[15] = 0;
      }
      else {
        strcpy( tmp, "0x" );
        Strncat( tmp, axtdo->entryValue, 15 );
        tmp[15] = 0;
      }

      if ( isLegalInteger(tmp) ) {
        doPut = 1;
        ivalue = strtol( tmp, NULL, 0 );
        dvalue = (double) ivalue;
      }

    }
    else {

      if ( isLegalFloat(axtdo->entryValue) ) {
        doPut = 1;
        dvalue = atof( axtdo->entryValue );
      }

    }

    if ( doPut ) {
      if ( axtdo->pvExists ) {
        //axtdo->pvId->put( dvalue );
        axtdo->putValueWithClip( dvalue );
      }
      else {
        axtdo->needUpdate = 1;
        axtdo->actWin->appCtx->proc->lock();
        axtdo->actWin->addDefExeNode( axtdo->aglPtr );
        axtdo->actWin->appCtx->proc->unlock();
      }

    }
    break;

  case ProcessVariable::specificType::shrt:
  case ProcessVariable::specificType::integer:

    if ( axtdo->formatType == XTDC_K_FORMAT_HEX ) {
      if ( strlen( axtdo->entryValue ) > 2 ) {
        if ( ( strncmp( axtdo->entryValue, "0x", 2 ) != 0 ) &&
             ( strncmp( axtdo->entryValue, "0X", 2 ) != 0 ) ) {
          strcpy( tmp, "0x" );
        }
        else {
          strcpy( tmp, "" );
        }
        Strncat( tmp, axtdo->entryValue, 15 );
        tmp[15] = 0;
      }
      else {
        strcpy( tmp, "0x" );
        Strncat( tmp, axtdo->entryValue, 15 );
        tmp[15] = 0;
      }
    }
    else {
      strncpy( tmp, axtdo->entryValue, XTDC_K_MAX );
      tmp[XTDC_K_MAX] = 0;
    }

    if ( isLegalInteger(tmp) ) {
      ivalue = strtol( tmp, NULL, 0 );
      if ( axtdo->pvExists ) {
        //axtdo->pvId->put( ivalue );
        axtdo->putValueWithClip( ivalue );
      }
      else {
        axtdo->needUpdate = 1;
        axtdo->actWin->appCtx->proc->lock();
        axtdo->actWin->addDefExeNode( axtdo->aglPtr );
        axtdo->actWin->appCtx->proc->unlock();
      }

    }
    break;

  case ProcessVariable::specificType::text:

    strncpy( string, axtdo->entryValue, XTDC_K_MAX );
    string[XTDC_K_MAX] = 0;
    if ( axtdo->pvExists ) {
      stat = stringPut( axtdo->pvId, axtdo->pvCount, string );
    }
    else {
      axtdo->needUpdate = 1;
      axtdo->actWin->appCtx->proc->lock();
      axtdo->actWin->addDefExeNode( axtdo->aglPtr );
      axtdo->actWin->appCtx->proc->unlock();
    }

    break;

  case ProcessVariable::specificType::enumerated:

    svalue = (short) axtdo->entryState;
    if ( axtdo->pvExists ) {
      axtdo->pvId->put( svalue );
    }
    else {
      axtdo->needUpdate = 1;
      axtdo->actWin->appCtx->proc->lock();
      axtdo->actWin->addDefExeNode( axtdo->aglPtr );
      axtdo->actWin->appCtx->proc->unlock();
    }

    break;

  }

}

static void axtdc_value_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeXTextDspClass *axtdo = (activeXTextDspClass *) client;

  axtdc_value_edit_apply ( w, client, call );
  axtdo->textEntry.popdown();
  axtdo->editDialogIsActive = 0;

}

static void axtdc_value_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeXTextDspClass *axtdo = (activeXTextDspClass *) client;

  axtdo->textEntry.popdown();
  axtdo->editDialogIsActive = 0;

}

static void axtdc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeXTextDspClass *axtdo = (activeXTextDspClass *) client;

  axtdo->actWin->setChanged();

  axtdo->eraseSelectBoxCorners();
  axtdo->erase();

  strncpy( axtdo->value, axtdo->eBuf->bufPvName, axtdo->minStringSize() );
  axtdo->value[axtdo->minStringSize()] = 0;
  strncpy( axtdo->curValue, axtdo->eBuf->bufPvName, axtdo->minStringSize() );
  axtdo->curValue[axtdo->minStringSize()] = 0;

  strncpy( axtdo->pvName, axtdo->eBuf->bufPvName, PV_Factory::MAX_PV_NAME );
  axtdo->pvName[PV_Factory::MAX_PV_NAME] = 0;
  axtdo->pvExpStr.setRaw( axtdo->pvName );

  axtdo->svalPvExpStr.setRaw( axtdo->eBuf->bufSvalPvName );

  axtdo->fgPvExpStr.setRaw( axtdo->eBuf->bufColorPvName );

  axtdo->defDir.setRaw( axtdo->eBuf->bufDefDir );
  axtdo->pattern.setRaw( axtdo->eBuf->bufPattern );

  strncpy( axtdo->fontTag, axtdo->fm.currentFontTag(), 63 );
  axtdo->fontTag[63] = 0;
  axtdo->actWin->fi->loadFontTag( axtdo->fontTag );
  axtdo->actWin->drawGc.setFontTag( axtdo->fontTag, axtdo->actWin->fi );

  axtdo->stringLength = strlen( axtdo->curValue );

  axtdo->fs = axtdo->actWin->fi->getXFontStruct( axtdo->fontTag );

  axtdo->updateFont( axtdo->curValue, axtdo->fontTag, &axtdo->fs,
   &axtdo->fontAscent, &axtdo->fontDescent, &axtdo->fontHeight,
   &axtdo->stringWidth );

  axtdo->useDisplayBg = axtdo->eBuf->bufUseDisplayBg;

  axtdo->autoHeight = axtdo->eBuf->bufAutoHeight;

  axtdo->formatType = axtdo->eBuf->bufFormatType;

  axtdo->limitsFromDb = axtdo->eBuf->bufLimitsFromDb;

  axtdo->changeValOnLoseFocus = axtdo->eBuf->bufChangeValOnLoseFocus;

  axtdo->fastUpdate = axtdo->eBuf->bufFastUpdate;

  axtdo->efPrecision = axtdo->eBuf->bufEfPrecision;

  if ( axtdo->efPrecision.isNull() )
    axtdo->precision = 2;
  else
    axtdo->precision = axtdo->efPrecision.value();

  strncpy( axtdo->fieldLenInfo, axtdo->eBuf->bufFieldLenInfo, 7 );
  axtdo->fieldLenInfo[7] = 0;

  axtdo->clipToDspLimits = axtdo->eBuf->bufClipToDspLimits;

  axtdo->fgColor.setConnectSensitive();

  axtdo->colorMode = axtdo->eBuf->bufColorMode;

  axtdo->editable = axtdo->eBuf->bufEditable;

  axtdo->isWidget = axtdo->eBuf->bufIsWidget;

  axtdo->isDate = axtdo->eBuf->bufIsDate;

  axtdo->isFile = axtdo->eBuf->bufIsFile;

  axtdo->useKp = axtdo->eBuf->bufUseKp;

  if ( axtdo->colorMode == XTDC_K_COLORMODE_ALARM )
    axtdo->fgColor.setAlarmSensitive();
  else
    axtdo->fgColor.setAlarmInsensitive();

  axtdo->fgColor.setColorIndex( axtdo->eBuf->bufFgColor, axtdo->actWin->ci );
  axtdo->fgColor.setNullIndex ( axtdo->eBuf->bufSvalColor, axtdo->actWin->ci );
  axtdo->bgColor = axtdo->eBuf->bufBgColor;

  axtdo->nullDetectMode = axtdo->eBuf->bufNullDetectMode;

  axtdo->smartRefresh = axtdo->eBuf->bufSmartRefresh;

  axtdo->autoSelect = axtdo->eBuf->bufAutoSelect;

  axtdo->updatePvOnDrop = axtdo->eBuf->bufUpdatePvOnDrop;

  axtdo->useHexPrefix = axtdo->eBuf->bufUseHexPrefix;

  axtdo->fileComponent = axtdo->eBuf->bufFileComponent;

  axtdo->dateAsFileName = axtdo->eBuf->bufDateAsFileName;

  axtdo->showUnits = axtdo->eBuf->bufShowUnits;
  if ( axtdo->editable ) {
    axtdo->showUnits = 0;
  }

  axtdo->useAlarmBorder = axtdo->eBuf->bufUseAlarmBorder;

  axtdo->inputFocusUpdatesAllowed = axtdo->eBuf->bufInputFocusUpdatesAllowed;

  strncpy( axtdo->id, axtdo->bufId, 31 );
  axtdo->id[31] = 0;
  axtdo->changeCallbackFlag = axtdo->eBuf->bufChangeCallbackFlag;
  axtdo->activateCallbackFlag = axtdo->eBuf->bufActivateCallbackFlag;
  axtdo->deactivateCallbackFlag = axtdo->eBuf->bufDeactivateCallbackFlag;
  axtdo->anyCallbackFlag = axtdo->changeCallbackFlag ||
   axtdo->activateCallbackFlag || axtdo->deactivateCallbackFlag;

  axtdo->x = axtdo->eBuf->bufX;
  axtdo->sboxX = axtdo->eBuf->bufX;

  axtdo->y = axtdo->eBuf->bufY;
  axtdo->sboxY = axtdo->eBuf->bufY;

  axtdo->w = axtdo->eBuf->bufW;
  axtdo->sboxW = axtdo->eBuf->bufW;

  axtdo->h = axtdo->eBuf->bufH;
  axtdo->sboxH = axtdo->eBuf->bufH;

  axtdo->updateDimensions();

  if ( axtdo->autoHeight && axtdo->fs ) {
    axtdo->h = axtdo->fontHeight;
    if ( axtdo->isWidget ) axtdo->h += 4;
    axtdo->sboxH = axtdo->h;
  }

  axtdo->stringY = axtdo->y + axtdo->fontAscent + axtdo->h/2 -
   axtdo->fontHeight/2;

  axtdo->alignment = axtdo->fm.currentFontAlignment();

  if ( axtdo->alignment == XmALIGNMENT_BEGINNING )
    axtdo->stringX = axtdo->x;
  else if ( axtdo->alignment == XmALIGNMENT_CENTER )
    axtdo->stringX = axtdo->x + axtdo->w/2 - axtdo->stringWidth/2;
  else if ( axtdo->alignment == XmALIGNMENT_END )
    axtdo->stringX = axtdo->x + axtdo->w - axtdo->stringWidth;

}

static void axtdc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeXTextDspClass *axtdo = (activeXTextDspClass *) client;

  axtdc_edit_update( w, client, call );
  axtdo->refresh( axtdo );

}

static void axtdc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeXTextDspClass *axtdo = (activeXTextDspClass *) client;

  axtdc_edit_update( w, client, call );
  axtdo->ef.popdown();
  axtdo->operationComplete();

}

static void axtdc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeXTextDspClass *axtdo = (activeXTextDspClass *) client;

  axtdo->ef.popdown();
  axtdo->operationCancel();

}

static void axtdc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeXTextDspClass *axtdo = (activeXTextDspClass *) client;

  axtdo->ef.popdown();
  axtdo->operationCancel();
  axtdo->erase();
  axtdo->deleteRequest = 1;
  axtdo->drawAll();

}

activeXTextDspClass::activeXTextDspClass ( void ) {

  name = new char[strlen("activeXTextDspClass")+1];
  strcpy( name, "activeXTextDspClass" );
  strcpy( value, "" );

  editable = 0;
  smartRefresh = 0;
  isWidget = 0;
  useKp = 0;
  isDate = 0;
  isFile = 0;
  fileComponent = XTDC_K_FILE_FULL_PATH;
  dateAsFileName = 0;
  numStates = 0;
  entryState = 0;
  limitsFromDb = 1;
  changeValOnLoseFocus = 0;
  autoSelect = 0;
  updatePvOnDrop = 0;
  useHexPrefix = 1;
  fastUpdate = 0;

  efPrecision.setNull(1);
  precision = 3;
  strcpy( fieldLenInfo, "" );

  clipToDspLimits = 0;
  upperLim = lowerLim = 0.0;

  activeMode = 0;

  strcpy( id, "" );

  changeCallbackFlag = 0;
  activateCallbackFlag = 0;
  deactivateCallbackFlag = 0;
  anyCallbackFlag = 0;
  changeCallback = NULL;
  activateCallback = NULL;
  deactivateCallback = NULL;

  nullDetectMode = 0;

  showUnits = 0;
  strcpy( units, "" );

  useAlarmBorder = 0;

  inputFocusUpdatesAllowed = 0;

  newPositioning = 1;

  prevAlarmSeverity = -1;
  pvCount = svalPvCount = 1;

  connection.setMaxPvs( 3 );

  unconnectedTimer = 0;

  eBuf = NULL;

  pvIndex = 0;

  if ( g_initTextBorderCheck ) checkTextBorderAlways();

  setBlinkFunction( (void *) doBlink );

}

// copy constructor
activeXTextDspClass::activeXTextDspClass
 ( const activeXTextDspClass *source ) {

activeGraphicClass *ago = (activeGraphicClass *) this;

  ago->clone( (activeGraphicClass *) source );

  name = new char[strlen("activeXTextDspClass")+1];
  strcpy( name, "activeXTextDspClass" );

  numStates = 0;
  entryState = 0;

  useDisplayBg = source->useDisplayBg;

  autoHeight = source->autoHeight;

  formatType = source->formatType;

  colorMode = source->colorMode;

  editable = source->editable;

  smartRefresh = source->smartRefresh;

  isWidget = source->isWidget;

  useKp = source->useKp;

  isDate = source->isDate;

  isFile = source->isFile;

  fileComponent = source->fileComponent;

  dateAsFileName = source->dateAsFileName;

  bgColor = source->bgColor;

  fgColor.copy(source->fgColor);

  strncpy( fontTag, source->fontTag, 63 );
  fontTag[63] = 0;

  fs = actWin->fi->getXFontStruct( fontTag );

  strncpy( value, source->value, XTDC_K_MAX );
  value[XTDC_K_MAX] = 0;

  alignment = source->alignment;

  stringLength = source->stringLength;
  fontAscent = source->fontAscent;
  fontDescent = source->fontDescent;
  fontHeight = source->fontHeight;
  stringWidth = source->stringWidth;
  stringY = source->stringY;
  stringX = source->stringX;

  strncpy( pvName, source->pvName, PV_Factory::MAX_PV_NAME );
  pvName[PV_Factory::MAX_PV_NAME] = 0;

  pvExpStr.copy( source->pvExpStr );
  svalPvExpStr.copy( source->svalPvExpStr );
  fgPvExpStr.copy( source->fgPvExpStr );

  defDir.copy( source->defDir );
  pattern.copy( source->pattern );

  limitsFromDb = source->limitsFromDb;
  changeValOnLoseFocus = source->changeValOnLoseFocus;
  autoSelect = source->autoSelect;
  updatePvOnDrop  = source->updatePvOnDrop;
  useHexPrefix = source->useHexPrefix;
  fastUpdate = source->fastUpdate;
  precision = source->precision;
  efPrecision = source->efPrecision;
  strncpy( fieldLenInfo, source->fieldLenInfo, 7 );
  fieldLenInfo[7] = 0;
  clipToDspLimits = source->clipToDspLimits;
  upperLim = source->upperLim;
  lowerLim = source->lowerLim;

  showUnits = source->showUnits;
  strcpy( units, "" );

  activeMode = 0;

  strcpy( id, source->id );

  changeCallbackFlag = source->changeCallbackFlag;
  activateCallbackFlag = source->activateCallbackFlag;
  deactivateCallbackFlag = source->deactivateCallbackFlag;
  anyCallbackFlag = changeCallbackFlag ||
   activateCallbackFlag || deactivateCallbackFlag;
  changeCallback = NULL;
  activateCallback = NULL;
  deactivateCallback = NULL;

  nullDetectMode = source->nullDetectMode;

  useAlarmBorder = source->useAlarmBorder;

  inputFocusUpdatesAllowed = source->inputFocusUpdatesAllowed;

  newPositioning = 1;

  prevAlarmSeverity = -1;
  pvCount = svalPvCount = 1;

  connection.setMaxPvs( 3 );

  unconnectedTimer = 0;

  eBuf = NULL;

  if ( g_initTextBorderCheck ) checkTextBorderAlways();

  setBlinkFunction( (void *) doBlink );

}

activeXTextDspClass::~activeXTextDspClass ( void ) {

  if ( name ) delete[] name;

  if ( eBuf ) delete eBuf;

  if ( unconnectedTimer ) {
    XtRemoveTimeOut( unconnectedTimer );
    unconnectedTimer = 0;
  }

  updateBlink( 0 );

}

void activeXTextDspClass::putValueWithClip (
  double val
) {

  if ( clipToDspLimits ) {

    if ( ( val >= lowerLim ) && ( val <= upperLim ) ) {
      pvId->put( val );
    }

  }
  else {

    pvId->put( val );

  }

}

void activeXTextDspClass::putValueWithClip (
  int val
) {

  if ( clipToDspLimits ) {

    if ( ( val >= (int) lowerLim ) && ( val <= (int) upperLim ) ) {
      pvId->put( val );
    }

  }
  else {

    pvId->put( val );

  }

}

int activeXTextDspClass::minStringSize( void ) {

  // In edit mode, pv name string is copied to pv value string. The
  // max length of these strings are declared with different constants
  // so this function returns the minimum of the two.

  if ( PV_Factory::MAX_PV_NAME < XTDC_K_MAX ) {
    return PV_Factory::MAX_PV_NAME;
  }
  else {
    return XTDC_K_MAX;
  }

}

int activeXTextDspClass::createInteractive (
  activeWindowClass *aw_obj,
  int _x,
  int _y,
  int _w,
  int _h ) {

  actWin = (activeWindowClass *) aw_obj;
  x = _x;
  y = _y;
  w = _w;
  h = _h;

  strcpy( value, "" );
  strcpy( pvName, "" );

  fgColor.setColorIndex( actWin->defaultTextFgColor, actWin->ci );
  fgColor.setNullIndex( actWin->defaultFg2Color, actWin->ci );
  bgColor = actWin->defaultBgColor;

  useDisplayBg = 1;

  autoHeight = 1;

  formatType = XTDC_K_FORMAT_NATURAL;

  colorMode = XTDC_K_COLORMODE_STATIC;

  editable = 0;
  smartRefresh = 0;
  isWidget = 0;
  useKp = 0;
  isDate = 0;
  isFile = 0;

  strcpy( fontTag, actWin->defaultFontTag );

  actWin->fi->loadFontTag( fontTag );

  fs = actWin->fi->getXFontStruct( fontTag );

  alignment = actWin->defaultAlignment;

  if ( fs ) {
    fontAscent = fs->ascent;
    fontDescent = fs->descent;
    fontHeight = fontAscent + fontDescent;
  }
  else {
    fontAscent = 0;
    fontDescent = 0;
    fontHeight = 0;
  }

  updateDimensions();

  this->draw();

  this->editCreate();

  return 1;

}

int activeXTextDspClass::save (
  FILE *f )
{

int stat, major, minor, release, index;

tagClass tag;

int zero = 0;
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

int nullCondNullEqCtl = 0;
static char *nullCondEnumStr[3] = {
  "nullEqCtl",
  "nullEq0",
  "disabled"
};
static int nullCondEnum[3] = {
  0,
  1,
  2
};

int formatTypeDefault = 0;
static char *formatTypeEnumStr[6] = {
  "default",
  "float",
  "exponential",
  "decimal",
  "hex",
  "string"
};
static int formatTypeEnum[6] = {
  0,
  1,
  2,
  3,
  4,
  5
};

int fileCompFullPath = 0;
static char *fileCompEnumStr[3] = {
  "fullPath",
  "nameAndExt",
  "name"
};
static int fileCompEnum[3] = {
  0,
  1,
  2
};

int objTypeUnknown = activeGraphicClass::UNKNOWN;
static char *objTypeEnumStr[4] = {
  "unknown",
  "graphics",
  "monitors",
  "controls"
};
static int objTypeEnum[4] = {
  activeGraphicClass::UNKNOWN,
  activeGraphicClass::GRAPHICS,
  activeGraphicClass::MONITORS,
  activeGraphicClass::CONTROLS
};

  major = XTDC_MAJOR_VERSION;
  minor = XTDC_MINOR_VERSION;
  release = XTDC_RELEASE;

  tag.init();
  tag.loadW( "beginObjectProperties" );
  tag.loadW( "major", &major );
  tag.loadW( "minor", &minor );
  tag.loadW( "release", &release );
  tag.loadW( "x", &x );
  tag.loadW( "y", &y );
  tag.loadW( "w", &w );
  tag.loadW( "h", &h );
  tag.loadW( "controlPv", &pvExpStr, emptyStr );
  tag.loadW( "format", 6, formatTypeEnumStr, formatTypeEnum, &formatType,
   &formatTypeDefault );
  tag.loadW( "font", fontTag );
  tag.loadW( "fontAlign", 3, alignEnumStr, alignEnum, &alignment, &left );
  tag.loadW( "fgColor", actWin->ci, &fgColor );
  tag.loadBoolW( "fgAlarm", &colorMode, &zero );
  tag.loadW( "bgColor", actWin->ci, &bgColor );
  tag.loadBoolW( "useDisplayBg", &useDisplayBg, &zero );
  tag.loadBoolW( "editable", &editable, &zero );
  tag.loadBoolW( "autoHeight", &autoHeight, &zero );
  tag.loadBoolW( "motifWidget", &isWidget, &zero );
  tag.loadBoolW( "limitsFromDb", &limitsFromDb, &zero );
  tag.loadW( "precision", &efPrecision );
  tag.loadW( "fieldLen", fieldLenInfo, emptyStr );
  tag.loadW( "nullPv", &svalPvExpStr, emptyStr );
  index = fgColor.nullIndex();
  tag.loadW( "nullColor", actWin->ci, &index );
  tag.loadW( "nullCondition", 3, nullCondEnumStr, nullCondEnum,
   &nullDetectMode, &nullCondNullEqCtl );
  tag.loadW( "colorPv", &fgPvExpStr, emptyStr );
  tag.loadBoolW( "smartRefresh", &smartRefresh, &zero );
  tag.loadBoolW( "useKp", &useKp, &zero );
  tag.loadBoolW( "changeValOnLoseFocus", &changeValOnLoseFocus, &zero );
  tag.loadBoolW( "fastUpdate", &fastUpdate, &zero );
  tag.loadBoolW( "date", &isDate, &zero );
  tag.loadBoolW( "file", &isFile, &zero );
  tag.loadW( "defDir", &defDir, emptyStr );
  tag.loadW( "pattern", &pattern, emptyStr );
  tag.loadBoolW( "autoSelect", &autoSelect, &zero );
  tag.loadBoolW( "updatePvOnDrop", &updatePvOnDrop, &zero );
  tag.loadBoolW( "useHexPrefix", &useHexPrefix, &zero );
  tag.loadW( "fileComponent", 3, fileCompEnumStr, fileCompEnum,
   &fileComponent, &fileCompFullPath );
  tag.loadBoolW( "dateAsFileName", &dateAsFileName, &zero );
  tag.loadBoolW( "showUnits", &showUnits, &zero );
  tag.loadBoolW( "useAlarmBorder", &useAlarmBorder, &zero );
  tag.loadBoolW( "newPos", &newPositioning, &zero );
  tag.loadBoolW( "inputFocusUpdates", &inputFocusUpdatesAllowed, &zero );
  tag.loadW( "objType", 4, objTypeEnumStr, objTypeEnum, &objType,
   &objTypeUnknown );
  tag.loadBoolW( "clipToDspLimits", &clipToDspLimits, &zero );
  tag.loadW( unknownTags );
  tag.loadW( "endObjectProperties" );
  tag.loadW( "" );

  stat = tag.writeTags( f );

  return stat;

}

int activeXTextDspClass::old_save (
  FILE *f )
{

int index, stat;

  fprintf( f, "%-d %-d %-d\n", XTDC_MAJOR_VERSION, XTDC_MINOR_VERSION,
   XTDC_RELEASE );

  fprintf( f, "%-d\n", x );
  fprintf( f, "%-d\n", y );
  fprintf( f, "%-d\n", w );
  fprintf( f, "%-d\n", h );
  writeStringToFile( f, pvName );
  writeStringToFile( f, fontTag );
  fprintf( f, "%-d\n", useDisplayBg );
  fprintf( f, "%-d\n", alignment );
  index = fgColor.pixelIndex();
  actWin->ci->writeColorIndex( f, index );
  //fprintf( f, "%-d\n", index );
  index = bgColor;
  actWin->ci->writeColorIndex( f, index );
  //fprintf( f, "%-d\n", index );
  fprintf( f, "%-d\n", formatType );
  fprintf( f, "%-d\n", colorMode );
  fprintf( f, "%-d\n", editable );
  fprintf( f, "%-d\n", autoHeight );

  fprintf( f, "%-d\n", isWidget );

  fprintf( f, "%-d\n", limitsFromDb );
  stat = efPrecision.write( f );

  // version 1.5.0
  writeStringToFile( f, id );
  fprintf( f, "%-d\n", changeCallbackFlag );
  fprintf( f, "%-d\n", activateCallbackFlag );
  fprintf( f, "%-d\n", deactivateCallbackFlag );

  // version 1.6.0
  if ( svalPvExpStr.getRaw() )
    writeStringToFile( f, svalPvExpStr.getRaw() );
  else
    writeStringToFile( f, "" );

  // version 1.7.0
  index = fgColor.nullIndex();
  fprintf( f, "%-d\n", index );

  fprintf( f, "%-d\n", nullDetectMode );

  // version 1.8.0
  if ( fgPvExpStr.getRaw() )
    writeStringToFile( f, fgPvExpStr.getRaw() );
  else
    writeStringToFile( f, "" );

  // version 1.9.0
  fprintf( f, "%-d\n", smartRefresh );

  // version 2.1.0
  fprintf( f, "%-d\n", useKp );

  // version 2.2.0
  fprintf( f, "%-d\n", changeValOnLoseFocus );

  // version 2.3.0
  fprintf( f, "%-d\n", fastUpdate );

  // version 2.3.0
  fprintf( f, "%-d\n", isDate );

  fprintf( f, "%-d\n", isFile );

  if ( defDir.getRaw() )
    writeStringToFile( f, defDir.getRaw() );
  else
    writeStringToFile( f, "" );

  if ( pattern.getRaw() )
    writeStringToFile( f, pattern.getRaw() );
  else
    writeStringToFile( f, "" );

  // version 2.5
  fprintf( f, "%-d\n", objType );

  // version 2.6
  fprintf( f, "%-d\n", autoSelect );

  // version 2.8
  fprintf( f, "%-d\n", updatePvOnDrop );

  // version 2.9
  fprintf( f, "%-d\n", useHexPrefix );

  // version 2.10
  fprintf( f, "%-d\n", fileComponent );
  fprintf( f, "%-d\n", dateAsFileName );

  // version 2.11
  fprintf( f, "%-d\n", showUnits );

  // version 2.12
  fprintf( f, "%-d\n", useAlarmBorder );

  return 1;

}

int activeXTextDspClass::createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int major, minor, release, index, stat;

tagClass tag;

int zero = 0;
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

int nullCondNullEqCtl = 0;
static char *nullCondEnumStr[3] = {
  "nullEqCtl",
  "nullEq0",
  "disabled"
};
static int nullCondEnum[3] = {
  0,
  1,
  2
};

int formatTypeDefault = 0;
static char *formatTypeEnumStr[6] = {
  "default",
  "float",
  "exponential",
  "decimal",
  "hex",
  "string"
};
static int formatTypeEnum[6] = {
  0,
  1,
  2,
  3,
  4,
  5
};

int fileCompFullPath = 0;
static char *fileCompEnumStr[3] = {
  "fullPath",
  "nameAndExt",
  "name"
};
static int fileCompEnum[3] = {
  0,
  1,
  2
};

int objTypeUnknown = activeGraphicClass::UNKNOWN;
static char *objTypeEnumStr[4] = {
  "graphics",
  "monitors",
  "controls",
  "unknown"
};
static int objTypeEnum[4] = {
  activeGraphicClass::GRAPHICS,
  activeGraphicClass::MONITORS,
  activeGraphicClass::CONTROLS,
  activeGraphicClass::UNKNOWN
};

  this->actWin = _actWin;

  tag.init();
  tag.loadR( "beginObjectProperties" );
  tag.loadR( unknownTags );
  tag.loadR( "major", &major );
  tag.loadR( "minor", &minor );
  tag.loadR( "release", &release );
  tag.loadR( "x", &x );
  tag.loadR( "y", &y );
  tag.loadR( "w", &w );
  tag.loadR( "h", &h );
  tag.loadR( "controlPv", &pvExpStr, emptyStr );
  tag.loadR( "format", 6, formatTypeEnumStr, formatTypeEnum, &formatType,
   &formatTypeDefault );
  tag.loadR( "font", 63, fontTag );
  tag.loadR( "fontAlign", 3, alignEnumStr, alignEnum, &alignment, &left );
  tag.loadR( "fgColor", actWin->ci, &fgColor );
  tag.loadR( "fgAlarm", &colorMode, &zero );
  tag.loadR( "bgColor", actWin->ci, &bgColor );
  tag.loadR( "useDisplayBg", &useDisplayBg, &zero );
  tag.loadR( "editable", &editable, &zero );
  tag.loadR( "autoHeight", &autoHeight, &zero );
  tag.loadR( "motifWidget", &isWidget, &zero );
  tag.loadR( "limitsFromDb", &limitsFromDb, &zero );
  tag.loadR( "precision", &efPrecision );
  tag.loadR( "fieldLen", 7, fieldLenInfo, emptyStr );
  tag.loadR( "nullPv", &svalPvExpStr, emptyStr );
  tag.loadR( "nullColor", actWin->ci, &index );
  tag.loadR( "nullCondition", 3, nullCondEnumStr, nullCondEnum,
   &nullDetectMode, &nullCondNullEqCtl );
  tag.loadR( "colorPv", &fgPvExpStr, emptyStr );
  tag.loadR( "smartRefresh", &smartRefresh, &zero );
  tag.loadR( "useKp", &useKp, &zero );
  tag.loadR( "changeValOnLoseFocus", &changeValOnLoseFocus, &zero );
  tag.loadR( "fastUpdate", &fastUpdate, &zero );
  tag.loadR( "date", &isDate, &zero );
  tag.loadR( "file", &isFile, &zero );
  tag.loadR( "defDir", &defDir, emptyStr );
  tag.loadR( "pattern", &pattern, emptyStr );
  tag.loadR( "autoSelect", &autoSelect, &zero );
  tag.loadR( "updatePvOnDrop", &updatePvOnDrop, &zero );
  tag.loadR( "useHexPrefix", &useHexPrefix, &zero );
  tag.loadR( "fileComponent", 3, fileCompEnumStr, fileCompEnum,
   &fileComponent, &fileCompFullPath );
  tag.loadR( "dateAsFileName", &dateAsFileName, &zero );
  tag.loadR( "showUnits", &showUnits, &zero );
  tag.loadR( "useAlarmBorder", &useAlarmBorder, &zero );
  tag.loadR( "newPos", &newPositioning, &zero );
  tag.loadR( "inputFocusUpdates", &inputFocusUpdatesAllowed, &zero );
  tag.loadR( "objType", 4, objTypeEnumStr, objTypeEnum, &objType,
   &objTypeUnknown );
  tag.loadR( "clipToDspLimits", &clipToDspLimits, &zero );
  tag.loadR( "endObjectProperties" );

  stat = tag.readTags( f, "endObjectProperties" );

  if ( !( stat & 1 ) ) {
    actWin->appCtx->postMessage( tag.errMsg() );
  }

  if ( major > XTDC_MAJOR_VERSION ) {
    postIncompatable();
    return 0;
  }

  if ( major < 4 ) {
    postIncompatable();
    return 0;
  }

  if ( !newPositioning ) {
    newPositioning = 1;
    if ( isWidget ) {
      y -= 3;
      autoHeight = 1;
    }
  }
    
  this->initSelectBox(); // call after getting x,y,w,h


  strcpy( this->id, "" );
  changeCallbackFlag = 0;
  activateCallbackFlag = 0;
  deactivateCallbackFlag = 0;
  anyCallbackFlag = 0;

  precision = efPrecision.value();

  fgColor.setNullIndex( index, actWin->ci );

  if ( colorMode == XTDC_K_COLORMODE_ALARM )
    fgColor.setAlarmSensitive();
  else
    fgColor.setAlarmInsensitive();

  strncpy( pvName, pvExpStr.getRaw(), PV_Factory::MAX_PV_NAME );
  pvName[PV_Factory::MAX_PV_NAME] = 0;
  strncpy( value, pvName, minStringSize() );
  value[minStringSize()] = 0;

  actWin->fi->loadFontTag( fontTag );
  actWin->drawGc.setFontTag( fontTag, actWin->fi );

  stringLength = strlen( value );

  fs = actWin->fi->getXFontStruct( fontTag );

  updateFont( value, fontTag, &fs, &fontAscent, &fontDescent, &fontHeight,
   &stringWidth );

  stringY = y + fontAscent + h/2 - fontHeight/2;

  if ( alignment == XmALIGNMENT_BEGINNING )
    stringX = x;
  else if ( alignment == XmALIGNMENT_CENTER )
    stringX = x + w/2 - stringWidth/2;
  else if ( alignment == XmALIGNMENT_END )
    stringX = x + w - stringWidth;

  return stat;

}

int activeXTextDspClass::old_createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int r, g, b, index;
int major, minor, release;
int stat = 1;
char oneName[XTDC_K_MAX+1], onePv[PV_Factory::MAX_PV_NAME+1];
unsigned int pixel;
int tmpFgColor, tmpSvalColor;

  this->actWin = _actWin;

  fscanf( f, "%d %d %d\n", &major, &minor, &release ); actWin->incLine();

  if ( major > XTDC_MAJOR_VERSION ) {
    postIncompatable();
    return 0;
  }

  fscanf( f, "%d\n", &x ); actWin->incLine();
  fscanf( f, "%d\n", &y ); actWin->incLine();
  fscanf( f, "%d\n", &w ); actWin->incLine();
  fscanf( f, "%d\n", &h ); actWin->incLine();

  this->initSelectBox();

  readStringFromFile( pvName, PV_Factory::MAX_PV_NAME+1, f );
   actWin->incLine();
  pvExpStr.setRaw( pvName );

  readStringFromFile( fontTag, 63+1, f ); actWin->incLine();
  fscanf( f, "%d\n", &useDisplayBg ); actWin->incLine();
  fscanf( f, "%d\n", &alignment ); actWin->incLine();

  if ( ( major > 2 ) || ( ( major == 2 ) && ( minor > 6 ) ) ) {

    actWin->ci->readColorIndex( f, &index );
    actWin->incLine(); actWin->incLine();
    tmpFgColor = index;
    fgColor.setColorIndex( tmpFgColor, actWin->ci );

    actWin->ci->readColorIndex( f, &index );
    actWin->incLine(); actWin->incLine();
    bgColor = index;

  }
  else if ( major > 1 ) {

    fscanf( f, "%d\n", &index ); actWin->incLine();
    tmpFgColor = index;
    fgColor.setColorIndex( tmpFgColor, actWin->ci );

    fscanf( f, "%d\n", &index ); actWin->incLine();
    bgColor = index;

  }
  else {

    fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
    if ( ( major < 2 ) && ( minor < 2 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    actWin->ci->setRGB( r, g, b, &pixel );
    tmpFgColor = actWin->ci->pixIndex( pixel );
    fgColor.setColorIndex( tmpFgColor, actWin->ci );

    fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
    if ( ( major < 2 ) && ( minor < 2 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    actWin->ci->setRGB( r, g, b, &pixel );
    bgColor = actWin->ci->pixIndex( pixel );

  }

  fscanf( f, "%d\n", &formatType ); actWin->incLine();
  fscanf( f, "%d\n", &colorMode ); actWin->incLine();
  fscanf( f, "%d\n", &editable ); actWin->incLine();

  if ( ( major > 1 ) || ( minor > 0 ) ) {
    fscanf( f, "%d\n", &autoHeight ); actWin->incLine();
  }
  else {
    autoHeight = 0;
  }

  if ( ( major > 1 ) || ( minor > 2 ) ) {
    fscanf( f, "%d\n", &isWidget ); actWin->incLine();
  }
  else {
    isWidget = 0;
  }

  if ( ( major > 1 ) || ( minor > 3 ) ) {
    fscanf( f, "%d\n", &limitsFromDb ); actWin->incLine();
    stat = efPrecision.read( f ); actWin->incLine();
    if ( limitsFromDb || efPrecision.isNull() )
      precision = 3;
    else
      precision = efPrecision.value();
  }
  else {
    limitsFromDb = 1;
    precision = 3;
    efPrecision.setValue( 3 );
  }

  if ( ( major > 1 ) || ( minor > 4 ) ) {
    readStringFromFile( this->id, 31+1, f ); actWin->incLine();
    fscanf( f, "%d\n", &changeCallbackFlag ); actWin->incLine();
    fscanf( f, "%d\n", &activateCallbackFlag ); actWin->incLine();
    fscanf( f, "%d\n", &deactivateCallbackFlag ); actWin->incLine();
    anyCallbackFlag = changeCallbackFlag ||
     activateCallbackFlag || deactivateCallbackFlag;
  }
  else {
    strcpy( this->id, "" );
    changeCallbackFlag = 0;
    activateCallbackFlag = 0;
    deactivateCallbackFlag = 0;
    anyCallbackFlag = 0;
  }

  if ( colorMode == XTDC_K_COLORMODE_ALARM )
    fgColor.setAlarmSensitive();
  else
    fgColor.setAlarmInsensitive();

  strncpy( value, pvName, minStringSize() );
  value[minStringSize()] = 0;

  if ( ( major > 1 ) || ( minor > 5 ) ) {

    readStringFromFile( onePv, PV_Factory::MAX_PV_NAME+1, f );
     actWin->incLine();
    svalPvExpStr.setRaw( onePv );

    if ( major > 1 ) {

      fscanf( f, "%d\n", &index ); actWin->incLine();
      tmpSvalColor = index;
      fgColor.setNullIndex( tmpSvalColor, actWin->ci );

    }
    else {

      fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
      actWin->ci->setRGB( r, g, b, &pixel );
      tmpSvalColor = actWin->ci->pixIndex( pixel );
      fgColor.setNullIndex( tmpSvalColor, actWin->ci );

    }

  }
  else {

    svalPvExpStr.setRaw( "" );
    tmpSvalColor = tmpFgColor;

  }

  if ( ( major > 1 ) || ( minor > 6 ) ) {

    fscanf( f, "%d\n", &nullDetectMode ); actWin->incLine();

  }
  else {

    nullDetectMode = 0;

  }

  if ( ( major > 1 ) || ( minor > 7 ) ) {

    readStringFromFile( onePv, PV_Factory::MAX_PV_NAME+1, f );
     actWin->incLine();
    fgPvExpStr.setRaw( onePv );

  }
  else {

    fgPvExpStr.setRaw( "" );

  }

  if ( ( major > 1 ) || ( minor > 8 ) ) {

    fscanf( f, "%d\n", &smartRefresh ); actWin->incLine();

  }
  else {

    smartRefresh = 0;

  }

  if ( ( ( major == 1 ) && ( minor > 0 ) ) || ( major > 1 ) ) {

    fscanf( f, "%d\n", &useKp ); actWin->incLine();

  }
  else {

    useKp = 0;

  }

  if ( ( ( major == 2 ) && ( minor > 1 ) ) || ( major > 2 ) ) {

    fscanf( f, "%d\n", &changeValOnLoseFocus ); actWin->incLine();

  }
  else {

    changeValOnLoseFocus = 1; // older version behavior

  }

  if ( ( ( major == 2 ) && ( minor > 2 ) ) || ( major > 2 ) ) {

    fscanf( f, "%d\n", &fastUpdate ); actWin->incLine();

  }
  else {

    fastUpdate = 0; // older version behavior

  }

  if ( ( ( major == 2 ) && ( minor > 3 ) ) || ( major > 2 ) ) {

    fscanf( f, "%d\n", &isDate );
    fscanf( f, "%d\n", &isFile );

    readStringFromFile( oneName, XTDC_K_MAX+1, f ); actWin->incLine();
    defDir.setRaw( oneName );

    readStringFromFile( oneName, XTDC_K_MAX+1, f ); actWin->incLine();
    pattern.setRaw( oneName );

  }
  else {

    isDate = 0;
    isFile = 0;

  }

  if ( ( ( major == 2 ) && ( minor > 4 ) ) || ( major > 2 ) ) {
    fscanf( f, "%d\n", &objType );
  }
  else {
    objType = -1;
  }

  if ( ( ( major == 2 ) && ( minor > 5 ) ) || ( major > 2 ) ) {
    fscanf( f, "%d\n", &autoSelect );
  }
  else {
    autoSelect = 0;
  }

  if ( ( ( major == 2 ) && ( minor > 7 ) ) || ( major > 2 ) ) {
    fscanf( f, "%d\n", &updatePvOnDrop );
  }
  else {
    updatePvOnDrop = 0;
  }

  if ( ( ( major == 2 ) && ( minor > 8 ) ) || ( major > 2 ) ) {
    fscanf( f, "%d\n", &useHexPrefix );
  }
  else {
    useHexPrefix = 1;
  }

  if ( ( ( major == 2 ) && ( minor > 9 ) ) || ( major > 2 ) ) {
    fscanf( f, "%d\n", &fileComponent );
    fscanf( f, "%d\n", &dateAsFileName );
  }
  else {
    fileComponent = XTDC_K_FILE_FULL_PATH;
    dateAsFileName = 0;
  }

  if ( ( ( major == 2 ) && ( minor > 10 ) ) || ( major > 2 ) ) {
    fscanf( f, "%d\n", &showUnits );
  }
  else {
    showUnits = 0;
  }

  if ( editable ) {
    showUnits = 0;
  }

  if ( ( ( major == 2 ) && ( minor > 11 ) ) || ( major > 2 ) ) {
    fscanf( f, "%d\n", &useAlarmBorder );
  }
  else {
    useAlarmBorder = 0;
  }

  newPositioning = 1;
  if ( isWidget ) {
    y -= 3;
    autoHeight = 1;
  }

  actWin->fi->loadFontTag( fontTag );
  actWin->drawGc.setFontTag( fontTag, actWin->fi );

  stringLength = strlen( value );

  fs = actWin->fi->getXFontStruct( fontTag );

  updateFont( value, fontTag, &fs, &fontAscent, &fontDescent, &fontHeight,
   &stringWidth );

  stringY = y + fontAscent + h/2 - fontHeight/2;

  if ( alignment == XmALIGNMENT_BEGINNING )
    stringX = x;
  else if ( alignment == XmALIGNMENT_CENTER )
    stringX = x + w/2 - stringWidth/2;
  else if ( alignment == XmALIGNMENT_END )
    stringX = x + w - stringWidth;

  return stat;

}

int activeXTextDspClass::importFromXchFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int r, g, b, more;
int stat = 1;
char *tk, *gotData, *context, buf[255+1];
unsigned int pixel;
int tmpFgColor;

  r = 0xffff;
  g = 0xffff;
  b = 0xffff;

  this->actWin = _actWin;

  strcpy( value, "" );
  strcpy( pvName, "" );

  fgColor.setColorIndex( actWin->defaultTextFgColor, actWin->ci );
  bgColor = actWin->defaultBgColor;

  useDisplayBg = 1;

  autoHeight = 1;

  formatType = XTDC_K_FORMAT_NATURAL;

  colorMode = XTDC_K_COLORMODE_STATIC;

  editable = 0;
  smartRefresh = 0;
  isWidget = 0;
  useKp = 0;
  isDate = 0;
  isFile = 0;

  strcpy( fontTag, actWin->defaultFontTag );

  alignment = actWin->defaultAlignment;

  // continue until tag is <eod>

  do {

    gotData = getNextDataString( buf, 255, f );
    if ( !gotData ) {
      actWin->appCtx->postMessage( activeXTextDspClass_str3 );
      return 0;
    }

    context = NULL;

    tk = strtok_r( buf, " \t\n", &context );
    if ( !tk ) {
      actWin->appCtx->postMessage( activeXTextDspClass_str3 );
      return 0;
    }

    if ( strcmp( tk, "<eod>" ) == 0 ) {

      more = 0;

    }
    else {

      more = 1;

      if ( strcmp( tk, "x" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( activeXTextDspClass_str3 );
          return 0;
        }

        x = atol( tk );

      }
      else if ( strcmp( tk, "y" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( activeXTextDspClass_str3 );
          return 0;
        }

        y = atol( tk );

      }
      else if ( strcmp( tk, "w" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( activeXTextDspClass_str3 );
          return 0;
        }

        w = atol( tk );

      }
      else if ( strcmp( tk, "h" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( activeXTextDspClass_str3 );
          return 0;
        }

        h = atol( tk );

      }
            
      else if ( strcmp( tk, "ctlpv" ) == 0 ) {

        tk = strtok_r( NULL, "\"", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( activeXTextDspClass_str3 );
          return 0;
        }

        strncpy( pvName, tk, 28 );
        pvName[28] = 0;

      }
            
      else if ( strcmp( tk, "font" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( activeXTextDspClass_str3 );
          return 0;
        }

        strncpy( fontTag, tk, 63 );
	fontTag[63] = 0;

      }
            
      else if ( strcmp( tk, "justify" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( activeXTextDspClass_str3 );
          return 0;
        }

        alignment = atol( tk );

      }
            
      else if ( strcmp( tk, "red" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( activeXTextDspClass_str3 );
          return 0;
        }

        r = atol( tk );

      }
            
      else if ( strcmp( tk, "green" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( activeXTextDspClass_str3 );
          return 0;
        }

        g = atol( tk );

      }
            
      else if ( strcmp( tk, "blue" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( activeXTextDspClass_str3 );
          return 0;
        }

        b = atol( tk );

      }
            
    }

  } while ( more );

  actWin->ci->setRGB( r, g, b, &pixel );
  tmpFgColor = actWin->ci->pixIndex( pixel );
  fgColor.setColorIndex( tmpFgColor, actWin->ci );

  limitsFromDb = 1;
  changeValOnLoseFocus = 0;
  fastUpdate = 0;
  precision = 3;
  efPrecision.setValue( 3 );
  clipToDspLimits = 0;
  upperLim = lowerLim = 0.0;

  fgColor.setAlarmInsensitive();

  strncpy( value, pvName, minStringSize() );
  value[minStringSize()] = 0;

  pvExpStr.setRaw( pvName );

  actWin->fi->loadFontTag( fontTag );
  actWin->drawGc.setFontTag( fontTag, actWin->fi );

  stringLength = strlen( value );

  fs = actWin->fi->getXFontStruct( fontTag );

  updateFont( value, fontTag, &fs, &fontAscent, &fontDescent, &fontHeight,
   &stringWidth );

  y = y + fontDescent;

  this->initSelectBox(); // call after getting x,y,w,h

  if ( alignment == XmALIGNMENT_BEGINNING )
    stringX = x;
  else if ( alignment == XmALIGNMENT_CENTER )
    stringX = x + w/2 - stringWidth/2;
  else if ( alignment == XmALIGNMENT_END )
    stringX = x + w - stringWidth;

  stringY = y + fontAscent + h/2 - fontHeight/2;

  return stat;

}

int activeXTextDspClass::genericEdit ( void ) {

char title[32], *ptr;
int noedit;

  if ( !eBuf ) {
    eBuf = new editBufType;
  }

  strcpy( title, "activeXTextDspClass" );
  if ( strcmp( this->getCreateParam(), "noedit" ) == 0 ) {
    noedit = 1;
    Strncat( title, ":noedit", 31 );
  }
  else {
    noedit = 0;
  }

  ptr = actWin->obj.getNameFromClass( title );
  if ( ptr ) {
    strncpy( title, ptr, 31 );
    title[31] = 0;
  }
  else {
    strncpy( title, activeXTextDspClass_str4, 31 );
    title[31] = 0;
  }

  Strncat( title, activeXTextDspClass_str5, 31 );
  title[31] = 0;

  strncpy( bufId, id, 31 );
  bufId[31] = 0;

  eBuf->bufX = x;
  eBuf->bufY = y;
  eBuf->bufW = w;
  eBuf->bufH = h;
  eBuf->bufFgColor = fgColor.pixelIndex();
  eBuf->bufBgColor = bgColor;
  strncpy( eBuf->bufFontTag, fontTag, 63 );
  eBuf->bufFontTag[63] = 0;
  eBuf->bufUseDisplayBg = useDisplayBg;
  eBuf->bufAutoHeight = autoHeight;
  eBuf->bufFormatType = formatType;
  eBuf->bufColorMode = colorMode;
  strncpy( bfrValue, value, XTDC_K_MAX );
  bfrValue[XTDC_K_MAX] = 0;
  strncpy( eBuf->bufPvName, pvName, PV_Factory::MAX_PV_NAME );
  eBuf->bufPvName[PV_Factory::MAX_PV_NAME] = 0;

  if ( fgPvExpStr.getRaw() ) {
    strncpy( eBuf->bufColorPvName, fgPvExpStr.getRaw(),
     PV_Factory::MAX_PV_NAME );
    eBuf->bufColorPvName[PV_Factory::MAX_PV_NAME] = 0;
  }
  else {
    strcpy( eBuf->bufColorPvName, "" );
  }

  if ( svalPvExpStr.getRaw() ) {
    strncpy( eBuf->bufSvalPvName, svalPvExpStr.getRaw(),
     PV_Factory::MAX_PV_NAME );
    eBuf->bufSvalPvName[PV_Factory::MAX_PV_NAME] = 0;
  }
  else {
    strcpy( eBuf->bufSvalPvName, "" );
  }

  if ( defDir.getRaw() ) {
    strncpy( eBuf->bufDefDir, defDir.getRaw(), XTDC_K_MAX );
    eBuf->bufDefDir[XTDC_K_MAX] = 0;
  }
  else {
    strcpy( eBuf->bufDefDir, "" );
  }

  if ( pattern.getRaw() ) {
    strncpy( eBuf->bufPattern, pattern.getRaw(), XTDC_K_MAX );
    eBuf->bufPattern[XTDC_K_MAX] = 0;
  }
  else {
    strcpy( eBuf->bufPattern, "" );
  }

  eBuf->bufSvalColor = fgColor.nullIndex();
  eBuf->bufNullDetectMode = nullDetectMode;

  eBuf->bufEditable = editable;
  eBuf->bufSmartRefresh = smartRefresh;
  eBuf->bufIsWidget = isWidget;
  eBuf->bufUseKp = useKp;
  eBuf->bufIsDate = isDate;
  eBuf->bufDateAsFileName = dateAsFileName;
  eBuf->bufIsFile = isFile;
  eBuf->bufFileComponent = fileComponent;
  eBuf->bufLimitsFromDb = limitsFromDb;
  eBuf->bufChangeValOnLoseFocus = changeValOnLoseFocus;
  eBuf->bufFastUpdate = fastUpdate;
  eBuf->bufEfPrecision = efPrecision;
  strncpy( eBuf->bufFieldLenInfo, fieldLenInfo, 7 );
  eBuf->bufFieldLenInfo[7] = 0;
  eBuf->bufClipToDspLimits = clipToDspLimits;
  eBuf->bufChangeCallbackFlag = changeCallbackFlag;
  eBuf->bufActivateCallbackFlag = activateCallbackFlag;
  eBuf->bufDeactivateCallbackFlag = deactivateCallbackFlag;
  eBuf->bufAutoSelect = autoSelect;
  eBuf->bufUpdatePvOnDrop = updatePvOnDrop;
  eBuf->bufUseHexPrefix = useHexPrefix;
  eBuf->bufShowUnits = showUnits;
  eBuf->bufUseAlarmBorder = useAlarmBorder;
  eBuf->bufInputFocusUpdatesAllowed = inputFocusUpdatesAllowed;

  ef.create( actWin->top, actWin->appCtx->ci.getColorMap(),
   &actWin->appCtx->entryFormX,
   &actWin->appCtx->entryFormY, &actWin->appCtx->entryFormW,
   &actWin->appCtx->entryFormH, &actWin->appCtx->largestH,
   title, NULL, NULL, NULL );

  //ef.addTextField( activeXTextDspClass_str6, 35, bufId, 31 );
  ef.addTextField( activeXTextDspClass_str7, 35, &eBuf->bufX );
  ef.addTextField( activeXTextDspClass_str8, 35, &eBuf->bufY );
  ef.addTextField( activeXTextDspClass_str9, 35, &eBuf->bufW );
  ef.addTextField( activeXTextDspClass_str10, 35, &eBuf->bufH );
  ef.addTextField( activeXTextDspClass_str22, 35, eBuf->bufPvName,
   PV_Factory::MAX_PV_NAME );
  ef.addTextField( activeXTextDspClass_str74, 35, eBuf->bufColorPvName,
   PV_Factory::MAX_PV_NAME );
  ef.addTextField( activeXTextDspClass_str25, 35, eBuf->bufSvalPvName,
   PV_Factory::MAX_PV_NAME );
  ef.addOption( activeXTextDspClass_str23, activeXTextDspClass_str24,
   &eBuf->bufNullDetectMode );
  ef.addOption( activeXTextDspClass_str18,
   activeXTextDspClass_str19, &eBuf->bufFormatType );
  ef.addToggle( activeXTextDspClass_str77, &eBuf->bufUseHexPrefix );
  ef.addToggle( activeXTextDspClass_str20, &eBuf->bufLimitsFromDb );
  ef.addTextField( activeXTextDspClass_str85, 35, eBuf->bufFieldLenInfo, 7 );
  ef.addTextField( activeXTextDspClass_str21, 35, &eBuf->bufEfPrecision );
  ef.addToggle( activeXTextDspClass_str84, &eBuf->bufClipToDspLimits );
  ef.addToggle( activeXTextDspClass_str81, &eBuf->bufShowUnits );
  ef.addToggle( activeXTextDspClass_str11, &eBuf->bufAutoHeight );

  if ( !noedit ) {
    ef.addToggle( activeXTextDspClass_str27, &eBuf->bufEditable );
  }
  else {
    eBuf->bufEditable = editable = 0;
  }

  if ( !noedit ) {
    ef.addToggle( activeXTextDspClass_str67, &eBuf->bufUseKp );
  }
  else {
    eBuf->bufUseKp = useKp = 0;
  }

  ef.addToggle( activeXTextDspClass_str28, &eBuf->bufSmartRefresh );
  ef.addToggle( activeXTextDspClass_str29, &eBuf->bufIsWidget );

  if ( !noedit ) {
    ef.addToggle( activeXTextDspClass_str83, &eBuf->bufInputFocusUpdatesAllowed );
    ef.addToggle( activeXTextDspClass_str68, &eBuf->bufChangeValOnLoseFocus );
    ef.addToggle( activeXTextDspClass_str75, &eBuf->bufAutoSelect );
    ef.addToggle( activeXTextDspClass_str76, &eBuf->bufUpdatePvOnDrop );
  }
  else {
    eBuf->bufInputFocusUpdatesAllowed = inputFocusUpdatesAllowed = 0;
    eBuf->bufChangeValOnLoseFocus = changeValOnLoseFocus = 0;
    eBuf->bufAutoSelect = autoSelect = 0;
    eBuf->bufUpdatePvOnDrop = updatePvOnDrop = 0;
  }

  ef.addToggle( activeXTextDspClass_str69, &eBuf->bufFastUpdate );

  if ( !noedit ) {
    ef.addToggle( activeXTextDspClass_str70, &eBuf->bufIsDate );
    ef.addToggle( activeXTextDspClass_str80, &eBuf->bufDateAsFileName );
    ef.addToggle( activeXTextDspClass_str71, &eBuf->bufIsFile );
    ef.addOption( activeXTextDspClass_str78,
     activeXTextDspClass_str79, &eBuf->bufFileComponent );
    ef.addTextField( activeXTextDspClass_str72, 35, eBuf->bufDefDir, XTDC_K_MAX );
    ef.addTextField( activeXTextDspClass_str73, 35, eBuf->bufPattern, XTDC_K_MAX );
  }
  else {
    eBuf->bufIsDate = isDate = 0;
    eBuf->bufIsFile = isFile = 0;
    fileComponent = XTDC_K_FILE_FULL_PATH;
    dateAsFileName = 0;
  }

  ef.addColorButton( activeXTextDspClass_str15, actWin->ci, &eBuf->fgCb,
   &eBuf->bufFgColor );
  ef.addToggle( activeXTextDspClass_str14, &eBuf->bufColorMode );
  ef.addToggle( activeXTextDspClass_str82, &eBuf->bufUseAlarmBorder );
  ef.addColorButton( activeXTextDspClass_str16, actWin->ci, &eBuf->bgCb,
   &eBuf->bufBgColor );
  ef.addToggle( activeXTextDspClass_str17, &eBuf->bufUseDisplayBg );
  ef.addColorButton( activeXTextDspClass_str26, actWin->ci, &eBuf->svalCb,
   &eBuf->bufSvalColor );
  ef.addFontMenu( activeXTextDspClass_str12, actWin->fi, &fm, eBuf->bufFontTag );
  fm.setFontAlignment( alignment );

  //ef.addToggle( activeXTextDspClass_str30, &eBuf->bufActivateCallbackFlag );
  //ef.addToggle( activeXTextDspClass_str31, &eBuf->bufDeactivateCallbackFlag );
  //ef.addToggle( activeXTextDspClass_str32, &eBuf->bufChangeCallbackFlag );

  return 1;

}

int activeXTextDspClass::editCreate ( void ) {

  this->genericEdit();
  ef.finished( axtdc_edit_ok, axtdc_edit_apply, axtdc_edit_cancel_delete, this );
  actWin->currentEf = NULL;
  ef.popup();

  return 1;

}

int activeXTextDspClass::edit ( void ) {

  this->genericEdit();
  ef.finished( axtdc_edit_ok, axtdc_edit_apply, axtdc_edit_cancel, this );
  fm.setFontAlignment( alignment );
  actWin->currentEf = &ef;
  ef.popup();

  return 1;

}

int activeXTextDspClass::erase ( void ) {

XRectangle xR = { x, y, w, h };

  if ( activeMode || deleteRequest ) return 1;

  actWin->drawGc.addEraseXClipRectangle( xR );

  if ( strcmp( fontTag, "" ) != 0 ) {
    actWin->drawGc.setFontTag( fontTag, actWin->fi );
  }

  if ( useDisplayBg ) {

    XDrawString( actWin->d, XtWindow(actWin->drawWidget),
     actWin->drawGc.eraseGC(), stringX, stringY,
     value, stringLength );

  }
  else {

    XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
     actWin->drawGc.eraseGC(), x, y, w, h );

    XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
     actWin->drawGc.eraseGC(), x, y, w, h );

    XDrawImageString( actWin->d, XtWindow(actWin->drawWidget),
     actWin->drawGc.eraseGC(), stringX, stringY,
     value, stringLength );

  }

  actWin->drawGc.removeEraseXClipRectangle();

  return 1;

}

int activeXTextDspClass::eraseActive ( void ) {

int len;

  if ( !enabled || !init || !activeMode ) return 1;

  if ( isWidget ) return 1;

  if ( !bufInvalid && ( strlen(value) == strlen(bfrValue) ) ) {
    if ( strcmp( value, bfrValue ) == 0 ) return 1;
  }

  if ( strcmp( fontTag, "" ) != 0 ) {
    actWin->executeGc.setFontTag( fontTag, actWin->fi );
  }

  len = strlen(bfrValue);

  if ( bufInvalid ) {

    if ( colorMode == XTDC_K_COLORMODE_ALARM ) {

      if ( useAlarmBorder ) {

        if ( fgColor.getSeverity() != prevAlarmSeverity ) {

          actWin->executeGc.setLineWidth( 2 );
          actWin->executeGc.setLineStyle( LineSolid );

          XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
           actWin->executeGc.eraseGC(), x, y, w, h );

          actWin->executeGc.setLineWidth( 1 );

	}

      }

    }

  }
  else {

    if ( colorMode == XTDC_K_COLORMODE_ALARM ) {

      if ( fgColor.getSeverity() != prevAlarmSeverity ) {

        if ( useAlarmBorder ) {

          actWin->executeGc.setLineWidth( 2 );
          actWin->executeGc.setLineStyle( LineSolid );

          XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
           actWin->executeGc.eraseGC(), x, y, w, h );

          actWin->executeGc.setLineWidth( 1 );

        }

      }

    }

  }

  if ( useDisplayBg ) {

    XDrawString( actWin->d, XtWindow(actWin->executeWidget),
     actWin->executeGc.eraseGC(), stringX, stringY,
     bfrValue, len );

  }
  else {

    actWin->executeGc.saveFg();
    actWin->executeGc.saveBg();

    actWin->executeGc.setFG( actWin->ci->pix(bgColor) );
    actWin->executeGc.setBG( actWin->ci->pix(bgColor) );

    if ( bufInvalid ) {

      XDrawRectangle( actWin->d, XtWindow(actWin->executeWidget),
      actWin->executeGc.eraseGC(), x, y, w, h );

      XFillRectangle( actWin->d, XtWindow(actWin->executeWidget),
      actWin->executeGc.eraseGC(), x, y, w, h );

    }
    else {

      XDrawRectangle( actWin->d, XtWindow(actWin->executeWidget),
       actWin->executeGc.normGC(), x, y, w, h );

      XFillRectangle( actWin->d, XtWindow(actWin->executeWidget),
       actWin->executeGc.normGC(), x, y, w, h );

      XDrawImageString( actWin->d, XtWindow(actWin->executeWidget),
       actWin->executeGc.normGC(), stringX, stringY,
       bfrValue, len );

    }

    actWin->executeGc.restoreFg();
    actWin->executeGc.restoreBg();

  }

  return 1;

}

int activeXTextDspClass::draw ( void ) {

XRectangle xR = { x, y, w, h };
int clipStat;
int blink = 0;

  if ( activeMode || deleteRequest ) return 1;

  actWin->drawGc.saveFg();
  actWin->drawGc.saveBg();

  clipStat = actWin->drawGc.addNormXClipRectangle( xR );

  if ( strcmp( fontTag, "" ) != 0 ) {
    actWin->drawGc.setFontTag( fontTag, actWin->fi );
  }

  if ( useDisplayBg ) {

    actWin->drawGc.setFG( fgColor.pixelIndex(), &blink );
    actWin->drawGc.setBG( actWin->ci->pix(bgColor) );

    XDrawString( actWin->d, XtWindow(actWin->drawWidget),
     actWin->drawGc.normGC(), stringX, stringY,
     value, stringLength );

  }
  else {

    actWin->drawGc.setFG( actWin->ci->pix(bgColor) );

    XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
     actWin->drawGc.normGC(), x, y, w, h );

    XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
     actWin->drawGc.normGC(), x, y, w, h );

    actWin->drawGc.setFG( fgColor.pixelIndex(), &blink );
    actWin->drawGc.setBG( actWin->ci->pix(bgColor) );

    XDrawImageString( actWin->d, XtWindow(actWin->drawWidget),
     actWin->drawGc.normGC(), stringX, stringY,
     value, stringLength );

  }

  if ( clipStat & 1 ) actWin->drawGc.removeNormXClipRectangle();

  actWin->drawGc.restoreFg();
  actWin->drawGc.restoreBg();

  updateBlink( blink );

  return 1;

}

int activeXTextDspClass::drawActive ( void ) {

Arg args[10];
int n;
int blink = 0;
unsigned int color;

  if ( !init && !connection.pvsConnected() ) {
    if ( needToDrawUnconnected ) {
      actWin->executeGc.saveFg();
      //actWin->executeGc.setFG( fgColor.getDisconnected() );
      actWin->executeGc.setFG( fgColor.getDisconnectedIndex(), &blink );
      actWin->executeGc.setLineWidth( 2 );
      actWin->executeGc.setLineStyle( LineSolid );
      XDrawRectangle( actWin->d, XtWindow(actWin->executeWidget),
       actWin->executeGc.normGC(), x, y, w, h );
      actWin->executeGc.restoreFg();
      needToEraseUnconnected = 1;
      updateBlink( blink );
    }
  }
  else if ( needToEraseUnconnected ) {
    actWin->executeGc.setLineWidth( 2 );
    actWin->executeGc.setLineStyle( LineSolid );
    XDrawRectangle( actWin->d, XtWindow(actWin->executeWidget),
     actWin->executeGc.eraseGC(), x, y, w, h );
    needToEraseUnconnected = 0;
  }

  if ( !enabled || !activeMode || !init ) return 1;

  if ( !bufInvalid && ( strlen(value) == strlen(bfrValue) ) ) {
    if ( strcmp( value, bfrValue ) == 0 ) return 1;
  }

  if ( isWidget ) {

    if ( tf_widget ) {

      if ( !grabUpdate || updatePvOnDrop ) {

        if ( bufInvalid ) {
          n = 0;
          XtSetArg( args[n], XmNvalue, (XtArgVal) value ); n++;
          if ( useAlarmBorder && ( colorMode == XTDC_K_COLORMODE_ALARM ) ) {
	    color = fgColor.pixelColor();
            XtSetArg( args[n], XmNforeground, (XtArgVal) color ); n++;
          }
          else {
	    color = fgColor.getColor();
            XtSetArg( args[n], XmNforeground, (XtArgVal) color ); n++;
          }
          if ( colorMode == XTDC_K_COLORMODE_ALARM ) {
            if ( fgColor.getSeverity() != prevAlarmSeverity ) {
              if ( ( ( g_showTextBorderAlways && actWin->ci->shouldShowNoAlarmState() ) ||
                     fgColor.getSeverity() ) && useAlarmBorder ) {
                XtSetArg( args[n], XmNborderWidth, (XtArgVal) 2 ); n++;
	        color = fgColor.getColor();
                XtSetArg( args[n], XmNborderColor, (XtArgVal) color ); n++;
              }
              else {
                XtSetArg( args[n], XmNborderWidth, (XtArgVal) 0 ); n++;
              }
            }
          }
          XtSetValues( tf_widget, args, n );
        }
        else {
          XmTextFieldSetString( tf_widget, value );
        }

      }

    }

    strncpy( entryValue, value, XTDC_K_MAX );
    entryValue[XTDC_K_MAX] = 0;

    strncpy( bfrValue, value, XTDC_K_MAX );
    bfrValue[XTDC_K_MAX] = 0;

    if ( bufInvalid ) {
      bufInvalid = 0;
    }

    if ( fgColor.getSeverity() != prevAlarmSeverity ) {
      prevAlarmSeverity = fgColor.getSeverity();
    }

    return 1;

  }

  actWin->executeGc.saveFg();
  actWin->executeGc.saveBg();

  if ( strcmp( fontTag, "" ) != 0 ) {
    actWin->executeGc.setFontTag( fontTag, actWin->fi );
  }

  updateDimensions();

  if ( useDisplayBg ) {

    if ( useAlarmBorder && ( colorMode == XTDC_K_COLORMODE_ALARM ) ) {
      actWin->executeGc.setFG( fgColor.pixelIndex(), &blink );
    }
    else {
      actWin->executeGc.setFG( fgColor.getIndex(), &blink );
    }

    actWin->executeGc.setBG( actWin->ci->pix(bgColor) );

    XDrawString( actWin->d, XtWindow(actWin->executeWidget),
     actWin->executeGc.normGC(), stringX, stringY,
     value, stringLength );

  }
  else {

    actWin->executeGc.setFG( actWin->ci->pix(bgColor) );

    XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
     actWin->executeGc.normGC(), x, y, w, h );

    XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
     actWin->executeGc.normGC(), x, y, w, h );

    if ( useAlarmBorder && ( colorMode == XTDC_K_COLORMODE_ALARM ) ) {
      actWin->executeGc.setFG( fgColor.pixelIndex(), &blink );
    }
    else {
      actWin->executeGc.setFG( fgColor.getIndex(), &blink );
    }

    actWin->executeGc.setBG( actWin->ci->pix(bgColor) );

    XDrawImageString( actWin->d, XtWindow(actWin->executeWidget),
     actWin->executeGc.normGC(), stringX, stringY, 
     value, stringLength );

  }

  if ( colorMode == XTDC_K_COLORMODE_ALARM ) {

    if ( ( ( g_showTextBorderAlways && actWin->ci->shouldShowNoAlarmState() ) ||
           fgColor.getSeverity() ) && useAlarmBorder ) {

      actWin->executeGc.setFG( fgColor.getIndex(), &blink );
      actWin->executeGc.setLineWidth( 2 );
      actWin->executeGc.setLineStyle( LineSolid );

      XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
       actWin->executeGc.normGC(), x, y, w, h );

      actWin->executeGc.setLineWidth( 1 );

    }

  }

  actWin->executeGc.restoreFg();
  actWin->executeGc.restoreBg();

  strncpy( bfrValue, value, XTDC_K_MAX );
  bfrValue[XTDC_K_MAX] = 0;

  updateBlink( blink );

  if ( bufInvalid ) {
    bufInvalid = 0;
  }

  if ( fgColor.getSeverity() != prevAlarmSeverity ) {
    prevAlarmSeverity = fgColor.getSeverity();
  }

  return 1;

}

void activeXTextDspClass::bufInvalidate ( void ) {

  bufInvalid = 1;

}

int activeXTextDspClass::expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] ) {

int stat;

  stat = pvExpStr.expand1st( numMacros, macros, expansions );
  stat = svalPvExpStr.expand1st( numMacros, macros, expansions );
  stat = fgPvExpStr.expand1st( numMacros, macros, expansions );
  stat = defDir.expand1st( numMacros, macros, expansions );
  stat = pattern.expand1st( numMacros, macros, expansions );

  return stat;

}

int activeXTextDspClass::expand2nd (
  int numMacros,
  char *macros[],
  char *expansions[] ) {

int stat;

  stat = pvExpStr.expand2nd( numMacros, macros, expansions );
  stat = svalPvExpStr.expand2nd( numMacros, macros, expansions );
  stat = fgPvExpStr.expand2nd( numMacros, macros, expansions );
  stat = defDir.expand2nd( numMacros, macros, expansions );
  stat = pattern.expand2nd( numMacros, macros, expansions );

  return stat;

}

int activeXTextDspClass::containsMacros ( void ) {

int result;

  result = pvExpStr.containsPrimaryMacros();
  if ( result ) return 1;

  result = svalPvExpStr.containsPrimaryMacros();
  if ( result ) return 1;

  result = fgPvExpStr.containsPrimaryMacros();
  if ( result ) return 1;

  result = defDir.containsPrimaryMacros();
  if ( result ) return 1;

  result = pattern.containsPrimaryMacros();
  if ( result ) return 1;

  return 0;

}

int activeXTextDspClass::activate (
  int pass,
  void *ptr )
{

char callbackName[63+1];

  switch ( pass ) {

  case 1:

    opComplete = 0;

    break;

  case 2:

    if ( !opComplete ) {

      deferredCount = 0;
      needConnectInit = needInfoInit = needErase = needDraw = needRefresh =
       needUpdate = 0;
      needToEraseUnconnected = 0;
      needToDrawUnconnected = 0;
      initialConnection = 1;
      unconnectedTimer = 0;
      aglPtr = ptr;
      strcpy( curValue, "" );
      strcpy( value, "" );
      strcpy( bfrValue, "" );
      updateDimensions();
      tf_widget = NULL;
      numStates = 0; // for enum type
      editDialogIsActive = 0;
      activeMode = 1;
      init = 0;
      curDoubleValue = 0.0;
      curSvalValue = 0.0;
      noSval = 1;
      grabUpdate = 0;
      pvExistCheck = 0;
      connection.init();
      pvId = svalPvId = fgPvId = NULL;
      prevAlarmSeverity = -1;
      pvCount = svalPvCount = 1;
      oldStat = -1;
      oldSev = -1;

      initEnable();

      if ( !unconnectedTimer ) {
        unconnectedTimer = appAddTimeOut( actWin->appCtx->appContext(),
         2000, unconnectedTimeout, this );
      }

      fgColor.setNotNull();

      if ( !pvExistCheck ) {

        pvExistCheck = 1;

        if ( pvExpStr.getExpanded() ) {
	  //if ( strcmp( pvExpStr.getExpanded(), "" ) != 0 ) {
          if ( !blankOrComment( pvExpStr.getExpanded() ) ) {
            pvExists = 1;
            connection.addPv(); // must do this only once per pv
          }
          else {
            pvExists = 0;
          }
        }
        else {
          pvExists = 0;
        }

        if ( svalPvExpStr.getExpanded() ) {
	  //if ( strcmp( svalPvExpStr.getExpanded(), "" ) != 0 ) {
          if ( !blankOrComment( svalPvExpStr.getExpanded() ) ) {
            svalPvExists = 1;
            connection.addPv(); // must do this only once per pv
          }
          else {
            svalPvExists = 0;
          }
        }
        else {
          svalPvExists = 0;
        }

        if ( fgPvExpStr.getExpanded() ) {
          //if ( strcmp( fgPvExpStr.getExpanded(), "" ) != 0 ) {
          if ( !blankOrComment( fgPvExpStr.getExpanded() ) ) {
            fgPvExists = 1;
            connection.addPv(); // must do this only once per pv
          }
          else {
            fgPvExists = 0;
          }
        }
        else {
          fgPvExists = 0;
        }

      }

      if ( pvExists ) {

	pvId = the_PV_Factory->create( pvExpStr.getExpanded() );
	if ( pvId ) {
	  pvId->add_conn_state_callback( xtdo_monitor_connect_state, this );
	}
	else {
          fprintf( stderr, activeXTextDspClass_str33 );
          return 0;
        }

        if ( svalPvExists ) {

          svalPvId = the_PV_Factory->create( svalPvExpStr.getExpanded() );
          if ( svalPvId ) {
            svalPvId->add_conn_state_callback( xtdo_monitor_sval_connect_state,
             this );
          }
          else {
            fprintf( stderr, activeXTextDspClass_str33 );
            return 0;
          }

        }

        if ( fgPvExists ) {

          fgPvId = the_PV_Factory->create( fgPvExpStr.getExpanded() );
          if ( fgPvId ) {
            fgPvId->add_conn_state_callback( xtdo_monitor_fg_connect_state,
             this );
          }
          else {
            fprintf( stderr, activeXTextDspClass_str33 );
            return 0;
          }

        }

      }
      else if ( anyCallbackFlag ) {

        needInfoInit = 1;
        actWin->appCtx->proc->lock();
        actWin->addDefExeNode( aglPtr );
        actWin->appCtx->proc->unlock();

      }

      if ( anyCallbackFlag ) {

        if ( changeCallbackFlag ) {
          strncpy( callbackName, id, 63 );
	  callbackName[63] = 0;
          Strncat( callbackName, activeXTextDspClass_str36, 63 );
          callbackName[63] = 0;
          changeCallback =
           actWin->appCtx->userLibObject.getFunc( callbackName );
	}

        if ( activateCallbackFlag ) {
          strncpy( callbackName, id, 63 );
	  callbackName[63] = 0;
          Strncat( callbackName, activeXTextDspClass_str37, 63 );
          callbackName[63] = 0;
          activateCallback =
           actWin->appCtx->userLibObject.getFunc( callbackName );
	}

        if ( deactivateCallbackFlag ) {
          strncpy( callbackName, id, 63 );
	  callbackName[63] = 0;
          Strncat( callbackName, activeXTextDspClass_str38, 63 );
          callbackName[63] = 0;
          deactivateCallback =
           actWin->appCtx->userLibObject.getFunc( callbackName );
	}

        if ( activateCallback ) {
          (*activateCallback)( this );
        }

      }

      opComplete = 1;

    }

    break;

  case 3:
  case 4:
  case 5:
  case 6:
    break;

  } // end switch

  return 1;

}

int activeXTextDspClass::deactivate (
  int pass
) {

  if ( pass == 1 ) {

  activeMode = 0;

  if ( unconnectedTimer ) {
    XtRemoveTimeOut( unconnectedTimer );
    unconnectedTimer = 0;
  }

  //updateBlink( 0 );

  if ( kp.isPoppedUp() ) {
    kp.popdown();
  }

  if ( cp.isPoppedUp() ) {
    cp.popdown();
  }

  if ( fsel.isPoppedUp() ) {
    fsel.popdown();
  }

  if ( textEntry.formIsPoppedUp() ) {
    textEntry.popdown();
    editDialogIsActive = 0;
  }

  if ( deactivateCallback ) {
    (*deactivateCallback)( this );
  }

  if ( pvExists ) {

    if ( pvId ) {
      pvId->remove_conn_state_callback( xtdo_monitor_connect_state, this );
      pvId->remove_value_callback( XtextDspUpdate, this );
      pvId->release();
      pvId = NULL;
    }

  }

  if ( svalPvExists ) {

    if ( svalPvId ) {
      svalPvId->remove_conn_state_callback( xtdo_monitor_sval_connect_state,
       this );
      svalPvId->remove_value_callback( XtextDspSvalUpdate, this );
      svalPvId->release();
      svalPvId = NULL;
    }

  }

  if ( fgPvExists ) {

    if ( fgPvId ) {
      fgPvId->remove_conn_state_callback( xtdo_monitor_fg_connect_state,
       this );
      fgPvId->remove_value_callback( XtextDspFgUpdate, this );
      fgPvId->release();
      fgPvId = NULL;
    }

  }

  }
  else if ( pass == 2 ) {

  if ( tf_widget ) {
    XtDestroyWidget( tf_widget );
    tf_widget = NULL;
  }

  strcpy( value, pvName );
  strcpy( curValue, pvName );
  updateDimensions();

  }

  return 1;

}

void activeXTextDspClass::updateDimensions ( void )
{

  stringLength = strlen( value );

  if ( fs ) {
    stringWidth = XTextWidth( fs, value, stringLength );
  }
  else {
    stringWidth = 0;
  }

  stringY = y + fontAscent + h/2 - fontHeight/2;

  stringX = x;

  if ( alignment == XmALIGNMENT_BEGINNING ) {
    // no change
  }
  else if ( alignment == XmALIGNMENT_CENTER )
    stringX = x + w/2 - stringWidth/2;
  else if ( alignment == XmALIGNMENT_END )
    stringX = x + w - stringWidth;

}

void activeXTextDspClass::btnUp (
  XButtonEvent *be,
  int x,
  int y,
  int buttonState,
  int buttonNumber,
  int *action )
{

  *action = 0;

}

void activeXTextDspClass::btnDown (
  XButtonEvent *be,
  int x,
  int y,
  int buttonState,
  int buttonNumber,
  int *action )
{

char selectString[XTDC_K_MAX+1], tmpDir[XTDC_K_MAX+1], tmpPat[XTDC_K_MAX+1];
int i;

  *action = 0;

  if ( !enabled || !editable || isWidget || !pvId->have_write_access() )
   return;

  if ( buttonNumber != 1 ) return;

  if ( editDialogIsActive ) {

    if ( useKp ) {

      if ( kp.isPoppedUp() ) {
        kp.popdown();
        editDialogIsActive = 0;
      }

    }

    return;

  }

  teX = be->x_root;
  teY = be->y_root;
  teW = w;
  teH = h;
  teLargestH = 600;

  if ( useKp ) {

    if ( ( pvType == ProcessVariable::specificType::flt ) ||
         ( pvType == ProcessVariable::specificType::real ) ) {
      if ( formatType == XTDC_K_FORMAT_HEX ) {
        kp.createHex( actWin->top, teX, teY, "", &kpDouble,
         (void *) this,
         (XtCallbackProc) xtdoSetKpDoubleValue,
         (XtCallbackProc) xtdoCancelKp );
      }
      else {
        kp.create( actWin->top, teX, teY, "", &kpDouble,
         (void *) this,
         (XtCallbackProc) xtdoSetKpDoubleValue,
         (XtCallbackProc) xtdoCancelKp );
      }
      editDialogIsActive = 1;
      return;
    }
    else if ( ( pvType == ProcessVariable::specificType::shrt ) ||
              ( pvType == ProcessVariable::specificType::integer ) ) {
      if ( formatType == XTDC_K_FORMAT_HEX ) {
        kp.createHex( actWin->top, teX, teY, "", &kpInt,
         (void *) this,
         (XtCallbackProc) xtdoSetKpIntValue,
         (XtCallbackProc) xtdoCancelKp );
      }
      else {
        kp.create( actWin->top, teX, teY, "", &kpInt,
         (void *) this,
         (XtCallbackProc) xtdoSetKpIntValue,
         (XtCallbackProc) xtdoCancelKp );
      }
      editDialogIsActive = 1;
      return;
    }
    else if ( pvType == ProcessVariable::specificType::text ) {

      if ( isFile ) {

        if ( defDir.getExpanded() ) {
          strncpy( tmpDir, defDir.getExpanded(), XTDC_K_MAX );
          tmpDir[XTDC_K_MAX] = 0;
	}
        else {
          strcpy( tmpDir, "" );
	}

        if ( pattern.getExpanded() ) {
          strncpy( tmpPat, pattern.getExpanded(), XTDC_K_MAX );
          tmpPat[XTDC_K_MAX] = 0;
	}
        else {
          strcpy( tmpPat, "" );
	}

        fsel.create( actWin->top, teX, teY,
         tmpDir, tmpPat,
         (void *) this,
         (XtCallbackProc) xtdoSetFsValue,
         (XtCallbackProc) xtdoCancelStr );
        editDialogIsActive = 1;
        return;

      }
      else if ( isDate ) {

        cp.create( actWin->top, teX, teY, entryValue, XTDC_K_MAX,
         (void *) this,
         (XtCallbackProc) xtdoSetCpValue,
         (XtCallbackProc) xtdoCancelStr );
        cp.setDate( curValue );
        editDialogIsActive = 1;
        return;

      }

    }

  }

  strncpy( entryValue, value, XTDC_K_MAX );
  entryValue[XTDC_K_MAX] = 0;

  textEntry.create( actWin->top, &teX, &teY, &teW, &teH, &teLargestH, "",
  NULL, NULL, NULL );

  if ( pvType != ProcessVariable::specificType::enumerated ) {
    textEntry.addTextField( activeXTextDspClass_str44, 25, entryValue,
     XTDC_K_MAX );
  }
  else {
    strcpy( selectString, "" );
    for ( i=0; i<numStates; i++ ) {
      Strncat( selectString, (char *) pvId->get_enum( i ), XTDC_K_MAX );
      selectString[XTDC_K_MAX] = 0;
      if ( i != numStates-1 ) {
        Strncat( selectString, "|", XTDC_K_MAX );
        selectString[XTDC_K_MAX] = 0;
      }
    }
    textEntry.addOption( activeXTextDspClass_str45, selectString, &entryState );
  }

  textEntry.finished( axtdc_value_edit_ok, axtdc_value_edit_apply,
   axtdc_value_edit_cancel, this );

  textEntry.popup();
  editDialogIsActive = 1;

}

void activeXTextDspClass::pointerIn (
  int _x,
  int _y,
  int buttonState )
{

  if ( !enabled || !init ) return;

  if ( !pvId->have_write_access() ) {

    if ( isWidget ) {
      XtVaSetValues( tf_widget,
       XmNeditable, (XtArgVal) False,
       XmNcursorPositionVisible, (XtArgVal) False,
       NULL );
    }

    actWin->cursor.set( XtWindow(actWin->executeWidget), CURSOR_K_NO );

  }
  else {

    actWin->cursor.set( XtWindow(actWin->executeWidget), CURSOR_K_DEFAULT );

  }

  if ( !isWidget ) {
    activeGraphicClass::pointerIn( _x, _y, buttonState );
  }

}

int activeXTextDspClass::getButtonActionRequest (
  int *up,
  int *down,
  int *drag,
  int *focus )
{

  if ( pvExists && editable ) {
    *down = 1;
    *focus = 1;
  }
  else {
    *down = 0;
    *focus = 0;
  }

  *up = 0;
  *drag = 0;

  return 1;

}

static void dummy (
   Widget w,
   XEvent *e,
   String *params,
   Cardinal numParams )
{

}

static void drag (
   Widget w,
   XEvent *e,
   String *params,
   Cardinal numParams )
{

activeXTextDspClass *atdo;
int stat;

  XtVaGetValues( w, XmNuserData, &atdo, NULL );

  stat = atdo->startDrag( w, e );

}

static void selectDrag (
   Widget w,
   XEvent *e,
   String *params,
   Cardinal numParams )
{

activeXTextDspClass *atdo;
int stat;
XButtonEvent *be = (XButtonEvent *) e;

  XtVaGetValues( w, XmNuserData, &atdo, NULL );

  stat = atdo->selectDragValue( be );

}

static void selectActions (
   Widget w,
   XEvent *e,
   String *params,
   Cardinal numParams )
{

activeXTextDspClass *atdo;
XButtonEvent *be = (XButtonEvent *) e;

  XtVaGetValues( w, XmNuserData, &atdo, NULL );

  atdo->doActions( be, be->x, be->y );

}

static void pvInfo (
   Widget w,
   XEvent *e,
   String *params,
   Cardinal numParams )
{

activeXTextDspClass *atdo;
XButtonEvent *be = (XButtonEvent *) e;

  XtVaGetValues( w, XmNuserData, &atdo, NULL );

  atdo->showPvInfo( be, be->x, be->y );

}

void activeXTextDspClass::executeDeferred ( void ) {

int n, numCols, width, csrPos;
int nc, ni, nu, nr, nd, ne;
short svalue;
Arg args[10];
unsigned int bg, pixel;
XmFontList textFontList = NULL;
Cardinal numImportTargets;
Atom importList[2];
char locFieldLenInfo[7+1];

  if ( actWin->isIconified ) return;

  if ( !fastUpdate ) {
    actWin->appCtx->proc->lock();
    if ( !needConnectInit && !needInfoInit && !needRefresh ) {
      deferredCount--;
      if ( deferredCount > 0 ) {
        actWin->appCtx->proc->unlock();
        return;
      }
      deferredCount = actWin->appCtx->proc->halfSecCount;
    }
    actWin->appCtx->proc->unlock();
  }

  actWin->appCtx->proc->lock();
  nc = needConnectInit; needConnectInit = 0;
  ni = needInfoInit; needInfoInit = 0;
  nr = needRefresh; needRefresh = 0;
  nu = needUpdate; needUpdate = 0;
  nd = needDraw; needDraw = 0;
  ne = needErase; needErase = 0;
  strncpy( value, curValue, XTDC_K_MAX );
  value[XTDC_K_MAX] = 0;
  actWin->remDefExeNode( aglPtr );
  actWin->appCtx->proc->unlock();

  if ( !activeMode ) return;

  if ( nc ) {

    ni = 1;

    switch ( pvType ) {

    case ProcessVariable::specificType::real:
    case ProcessVariable::specificType::flt:

      strncpy( units, pvId->get_units(), MAX_UNITS_SIZE );
      units[MAX_UNITS_SIZE] = 0;

      if ( limitsFromDb || efPrecision.isNull() ) {
        precision = pvId->get_precision();
      }

      if ( clipToDspLimits ) {
	upperLim = pvId->get_upper_disp_limit();
        lowerLim = pvId->get_lower_disp_limit();
      }
      else {
	upperLim = 0.0;
        lowerLim = 0.0;
      }

      fgColor.setStatus( pvId->get_status(), pvId->get_severity() );

      isDate = 0;

      isFile = 0;

      break;

    case ProcessVariable::specificType::shrt:
    case ProcessVariable::specificType::integer:

      strncpy( units, pvId->get_units(), MAX_UNITS_SIZE );
      units[MAX_UNITS_SIZE] = 0;

      if ( limitsFromDb || efPrecision.isNull() ) {
        precision = pvId->get_precision();
      }

      if ( clipToDspLimits ) {
	upperLim = (double) pvId->get_upper_disp_limit();
        lowerLim = (double) pvId->get_lower_disp_limit();
      }
      else {
	upperLim = 0.0;
        lowerLim = 0.0;
      }

      fgColor.setStatus( pvId->get_status(), pvId->get_severity() );

      isDate = 0;

      isFile = 0;

      break;

    case ProcessVariable::specificType::enumerated:

      strcpy( units, "" );
      showUnits = 0;

      numStates = pvId->get_enum_count();

      svalue = (short) pvId->get_int();
      if ( ( svalue >= 0 ) && ( svalue < numStates ) ) {
        strncpy( value, pvId->get_enum( svalue ), XTDC_K_MAX );
        value[XTDC_K_MAX] = 0;
        entryState = (int) svalue;
      }
      else {
        strcpy( value, "" );
      }

      strncpy( curValue, value, XTDC_K_MAX );
      curValue[XTDC_K_MAX] = 0;

      isWidget = 0;

      fgColor.setStatus( pvId->get_status(), pvId->get_severity() );

      isDate = 0;

      isFile = 0;

      break;

    case ProcessVariable::specificType::text:
    default:

      if ( pvCount == 1 ) {

        strcpy( units, "" );
        showUnits = 0;
        fgColor.setStatus( pvId->get_status(), pvId->get_severity() );

      }

      break;

    }

    bufInvalidate();

  }

  if ( ni ) {

    if ( initialConnection ) {

      initialConnection = 0;

      if ( fgPvExists ) {

	if ( fgPvId ) {
	  fgPvId->add_value_callback( XtextDspFgUpdate, this );
	}

      }

      if ( pvExists ) {

        switch ( pvType ) {

        case ProcessVariable::specificType::text:

          sprintf( format, "%%s" );

          break;

        case ProcessVariable::specificType::flt:

          if ( blank(fieldLenInfo) ) {
            strcpy( locFieldLenInfo, "" );
          }
          else {
            strncpy( locFieldLenInfo, fieldLenInfo, 7 );
            locFieldLenInfo[7] = 0;
          }

          switch( formatType ) {
          case XTDC_K_FORMAT_FLOAT:
            sprintf( format, "%%%s.%-df", locFieldLenInfo, precision );
            break;
          case XTDC_K_FORMAT_EXPONENTIAL:
            sprintf( format, "%%%s.%-de", locFieldLenInfo, precision );
            break;
          default:
            sprintf( format, "%%%s.%-df", locFieldLenInfo, precision );
            break;
          } // end switch( formatType )
  
          if ( svalPvExists ) {
	    if ( svalPvId ) {
              svalPvId->add_value_callback( XtextDspSvalUpdate, this );
	    }
	  }

          break;

        case ProcessVariable::specificType::real:

          if ( blank(fieldLenInfo) ) {
            strcpy( locFieldLenInfo, "" );
          }
          else {
            strncpy( locFieldLenInfo, fieldLenInfo, 7 );
            locFieldLenInfo[7] = 0;
          }

          switch( formatType ) {
          case XTDC_K_FORMAT_FLOAT:
            sprintf( format, "%%%s.%-df", locFieldLenInfo, precision );
            break;
          case XTDC_K_FORMAT_EXPONENTIAL:
            sprintf( format, "%%%s.%-de", locFieldLenInfo, precision );
            break;
          default:
            sprintf( format, "%%%s.%-df", locFieldLenInfo, precision );
            break;
          } // end switch( formatType )

          if ( svalPvExists ) {
	    if ( svalPvId ) {
              svalPvId->add_value_callback( XtextDspSvalUpdate, this );
	    }
	  }

          break;

        case ProcessVariable::specificType::shrt:

          if ( blank(fieldLenInfo) ) {
            strcpy( locFieldLenInfo, "-" );
          }
          else {
            strncpy( locFieldLenInfo, fieldLenInfo, 7 );
            locFieldLenInfo[7] = 0;
          }

          switch( formatType ) {
          case XTDC_K_FORMAT_DECIMAL:
            sprintf( format, "%%%sd", locFieldLenInfo );
            break;
          case XTDC_K_FORMAT_HEX:
            if ( useHexPrefix ) {
              sprintf( format, "0x%%%sX", locFieldLenInfo );
            }
            else {
              sprintf( format, "%%%sX", locFieldLenInfo );
            }
            break;
          default:
            sprintf( format, "%%%sd", locFieldLenInfo );
            break;
          } // end switch( formatType )

          if ( svalPvExists ) {
	    if ( svalPvId ) {
              svalPvId->add_value_callback( XtextDspSvalUpdate, this );
	    }
	  }

          break;

        case ProcessVariable::specificType::integer:

          if ( blank(fieldLenInfo) ) {
            strcpy( locFieldLenInfo, "-" );
          }
          else {
            strncpy( locFieldLenInfo, fieldLenInfo, 7 );
            locFieldLenInfo[7] = 0;
          }

          switch( formatType ) {
          case XTDC_K_FORMAT_DECIMAL:
            sprintf( format, "%%%sd", locFieldLenInfo );
            break;
          case XTDC_K_FORMAT_HEX:
            if ( useHexPrefix ) {
              sprintf( format, "0x%%%sX", locFieldLenInfo );
            }
            else {
              sprintf( format, "%%%sX", locFieldLenInfo );
            }
            break;
          default:
            sprintf( format, "%%%sd", locFieldLenInfo );
            break;
          } // end switch( formatType )

          if ( svalPvExists ) {
	    if ( svalPvId ) {
              svalPvId->add_value_callback( XtextDspSvalUpdate, this );
	    }
	  }

          break;

        case ProcessVariable::specificType::enumerated:

          sprintf( format, "%%s" );

          if ( svalPvExists ) {
	    if ( svalPvId ) {
              svalPvId->add_value_callback( XtextDspSvalUpdate, this );
	    }
	  }

          break;

        default:
          sprintf( format, "%%s" );
          break;

        } // end switch ( pvType )

        pvId->add_value_callback( XtextDspUpdate, this );

      }
      else {

        pvType = ProcessVariable::specificType::text;
        pvCount = 1;
        sprintf( format, "%%s" );

      }

    }

    if ( isWidget ) {

      if ( fontTag ) {
        actWin->fi->getTextFontList( fontTag, &textFontList );
      }
      else {
        textFontList = NULL;
      }

      if ( fs ) {
        width = XTextWidth( fs, "1", 1 );
        numCols = (int) ( (float) ( w / width ) + 0.5 );
        if ( numCols < 1 ) numCols = 1;
      }
      else {
        numCols = 6;
      }

      strncpy( entryValue, value, XTDC_K_MAX );
      entryValue[XTDC_K_MAX] = 0;
      csrPos = strlen(entryValue);

      widget_value_changed = 0;

      if ( useDisplayBg )
        bg = actWin->executeGc.getBaseBG();
      else
        bg = actWin->ci->pix(bgColor);

      if ( !tf_widget ) {

      if ( g_transInit ) {
        g_transInit = 0;
        g_parsedTrans = XtParseTranslationTable( g_dragTrans );
      }
      actWin->appCtx->addActions( g_dragActions, XtNumber(g_dragActions) );

      if ( useAlarmBorder && ( colorMode == XTDC_K_COLORMODE_ALARM ) ) {
        pixel = fgColor.pixelColor();
      }
      else {
        pixel = fgColor.getColor();
      }

      if ( newPositioning ) {

        if ( autoHeight ) {

          tf_widget = XtVaCreateManagedWidget( "", xmTextFieldWidgetClass,
           actWin->executeWidget,
           XmNx, x,
           XmNy, y,
           XmNforeground, pixel,
           XmNbackground, bg,
           XmNhighlightThickness, 0,
           XmNwidth, w,
           XmNvalue, entryValue,
           XmNmaxLength, (short) XTDC_K_MAX,
           XmNpendingDelete, True,
           XmNmarginHeight, 0,
           XmNfontList, textFontList,
           XmNtranslations, g_parsedTrans,
           XmNuserData, this,
           XmNcursorPositionVisible, False,
           NULL );

        }
        else {

          tf_widget = XtVaCreateManagedWidget( "", xmTextFieldWidgetClass,
           actWin->executeWidget,
           XmNx, x,
           XmNy, y,
           XmNforeground, pixel,
           XmNbackground, bg,
           XmNhighlightThickness, 0,
           XmNwidth, w,
           XmNheight, h,
           XmNvalue, entryValue,
           XmNmaxLength, (short) XTDC_K_MAX,
           XmNpendingDelete, True,
           XmNmarginHeight, 0,
           XmNfontList, textFontList,
           XmNtranslations, g_parsedTrans,
           XmNuserData, this,
           XmNcursorPositionVisible, False,
           NULL );

        }

      }
      else {

        tf_widget = XtVaCreateManagedWidget( "", xmTextFieldWidgetClass,
         actWin->executeWidget,
         XmNx, x,
         XmNy, y-3,
         XmNforeground, pixel,
         XmNbackground, bg,
         XmNhighlightThickness, 0,
         XmNwidth, w,
         XmNvalue, entryValue,
         XmNmaxLength, (short) XTDC_K_MAX,
         XmNpendingDelete, True,
         XmNmarginHeight, 0,
         XmNfontList, textFontList,
         XmNtranslations, g_parsedTrans,
         XmNuserData, this,
         XmNcursorPositionVisible, False,
         NULL );

      }

      if ( !enabled ) {
        XtUnmapWidget( tf_widget );
      }

      if ( textFontList ) XmFontListFree( textFontList );

      if ( !editable ) {

        n = 0;
        XtSetArg( args[n], XmNeditable, (XtArgVal) False ); n++;
        XtSetArg( args[n], XmNnavigationType, (XtArgVal) XmNONE ); n++;
        //XtSetArg( args[n], XmNcursorPositionVisible, (XtArgVal) False ); n++;
        XtSetValues( tf_widget, args, n );

      }
      else {

        //XmTextSetInsertionPosition( tf_widget, csrPos );

        XtAddCallback( tf_widget, XmNfocusCallback,
         xtdoSetSelection, this );

        XtAddCallback( tf_widget, XmNmotionVerifyCallback,
         xtdoGrabUpdate, this );

        XtAddCallback( tf_widget, XmNvalueChangedCallback,
         xtdoSetValueChanged, this );

        if ( updatePvOnDrop ) {

	  // change drop behavior

	  importList[0] = XA_STRING;
          numImportTargets = 1;
	  n = 0;
	  XtSetArg( args[n], XmNimportTargets, importList ); n++;
	  XtSetArg( args[n], XmNnumImportTargets, numImportTargets ); n++;
	  XtSetArg( args[n], XmNdropProc, handleDrop ); n++;
	  XmDropSiteUpdate( tf_widget, args, n );

	}

        if ( inputFocusUpdatesAllowed ) {
          XtAddEventHandler( tf_widget, EnterWindowMask|LeaveWindowMask, False,
           eventHandler, (XtPointer) this );
        }

        switch ( pvType ) {

        case ProcessVariable::specificType::text:

          XtAddCallback( tf_widget, XmNactivateCallback,
           xtdoTextFieldToStringA, this );

          if ( changeValOnLoseFocus ) {
            XtAddCallback( tf_widget, XmNlosingFocusCallback,
             xtdoTextFieldToStringLF, this );
	  }
	  else {
            XtAddCallback( tf_widget, XmNlosingFocusCallback,
             xtdoRestoreValue, this );
	  }

          break;

        case ProcessVariable::specificType::shrt:
        case ProcessVariable::specificType::integer:

          XtAddCallback( tf_widget, XmNactivateCallback,
           xtdoTextFieldToIntA, this );

          if ( changeValOnLoseFocus ) {
            XtAddCallback( tf_widget, XmNlosingFocusCallback,
             xtdoTextFieldToIntLF, this );
	  }
	  else {
            XtAddCallback( tf_widget, XmNlosingFocusCallback,
             xtdoRestoreValue, this );
	  }

          break;

        case ProcessVariable::specificType::flt:
        case ProcessVariable::specificType::real:

          XtAddCallback( tf_widget, XmNactivateCallback,
           xtdoTextFieldToDoubleA, this );

          if ( changeValOnLoseFocus ) {
            XtAddCallback( tf_widget, XmNlosingFocusCallback,
             xtdoTextFieldToDoubleLF, this );
	  }
	  else {
            XtAddCallback( tf_widget, XmNlosingFocusCallback,
             xtdoRestoreValue, this );
	  }

          break;

        } // end switch

      }

      } // end if ( !tf_widget )

    } // end if ( isWidget )

    fgColor.setConnected();
    init = 1;

    bufInvalidate();
    eraseActive();
    drawActive();

  }

  if ( nr ) {

    bufInvalidate();
    eraseActive();
    if (smartRefresh) {
      smartDrawAllActive();
    }
    else {
      drawActive();
    }

  }

  if ( nu ) {

    eraseActive();
    if (smartRefresh) {
      smartDrawAllActive();
    }
    else {
      drawActive();
    }

    if ( changeCallback ) {
      (*changeCallback)( this );
    }

  }

  if ( ne ) {

    eraseActive();

  }

  if ( nd ) {

    drawActive();

  }

}

int activeXTextDspClass::getProperty (
  char *prop,
  int bufSize,
  char *_value )
{

int l;
char *buf;

  if ( strcmp( prop, activeXTextDspClass_str65 ) == 0 ) {

    if ( !tf_widget ) {

      l = strlen(curValue);
      if ( l > bufSize ) l = bufSize;

      strncpy( _value, curValue, l );
      _value[l] = 0;

    }
    else {

      buf = XmTextGetString( tf_widget );

      l = strlen(buf);
      if ( l > bufSize ) l = bufSize;

      strncpy( _value, buf, l );
      _value[l] = 0;

      XtFree( buf );

    }

    return 1;

  }
  else if ( strcmp( prop, activeXTextDspClass_str66 ) == 0 ) {

    if ( !tf_widget ) {
      strncpy( _value, "", bufSize );
      _value[bufSize] = 0;
      return 0;
    }

    buf = XmTextGetString( tf_widget );

    l = strlen(buf);
    if ( l > bufSize ) l = bufSize;

    strncpy( _value, buf, l );
    _value[l] = 0;

    XtFree( buf );

    return 1;

  }

  return 0;

}

char *activeXTextDspClass::firstDragName ( void ) {

  if ( !enabled ) return NULL;

  dragIndex = 0;
  return dragName[dragIndex];

}

char *activeXTextDspClass::nextDragName ( void ) {

  if ( !enabled ) return NULL;

  if ( dragIndex < (int) ( sizeof(dragName) / sizeof(char *) ) - 1 ) {
    dragIndex++;
    return dragName[dragIndex];
  }
  else {
    return NULL;
  }

}

char *activeXTextDspClass::dragValue (
  int i ) {

  if ( !enabled ) return NULL;

  if ( actWin->mode == AWC_EXECUTE ) {

    if ( i == 0 ) {
      return pvExpStr.getExpanded();
    }
    else {
      return svalPvExpStr.getExpanded();
    }

  }
  else {

    if ( i == 0 ) {
      return pvExpStr.getRaw();
    }
    else {
      return svalPvExpStr.getRaw();
    }

  }

}

void activeXTextDspClass::changeDisplayParams (
  unsigned int _flag,
  char *_fontTag,
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
  int _botShadowColor )
{

  if ( _flag & ACTGRF_TEXTFGCOLOR_MASK )
    fgColor.setColorIndex( _textFgColor, actWin->ci );

  if ( _flag & ACTGRF_FG2COLOR_MASK )
    fgColor.setNullIndex( _fg2Color, actWin->ci );

  if ( _flag & ACTGRF_BGCOLOR_MASK )
    bgColor = _bgColor;

  if ( _flag & ACTGRF_ALIGNMENT_MASK )
    alignment = _alignment;

  if ( _flag & ACTGRF_FONTTAG_MASK ) {

    strcpy( fontTag, _fontTag );
    actWin->fi->loadFontTag( fontTag );
    fs = actWin->fi->getXFontStruct( fontTag );

    if ( fs ) {
      fontAscent = fs->ascent;
      fontDescent = fs->descent;
      fontHeight = fontAscent + fontDescent;
    }
    else {
      fontAscent = 0;
      fontDescent = 0;
      fontHeight = 0;
    }

    updateDimensions();

  }

}

void activeXTextDspClass::changePvNames (
  int flag,
  int numCtlPvs,
  char *ctlPvs[],
  int numReadbackPvs,
  char *readbackPvs[],
  int numNullPvs,
  char *nullPvs[],
  int numVisPvs,
  char *visPvs[],
  int numAlarmPvs,
  char *alarmPvs[] )
{

int changed = 0;

  if ( editable ) {

    if ( flag & ACTGRF_CTLPVS_MASK ) {

      if ( numCtlPvs ) {

        changed = 1;

        strncpy( value, ctlPvs[0], minStringSize() );
        value[minStringSize()] = 0;
        strncpy( curValue, ctlPvs[0], minStringSize() );
        curValue[minStringSize()] = 0;

        strncpy( pvName, ctlPvs[0], PV_Factory::MAX_PV_NAME );
        pvName[PV_Factory::MAX_PV_NAME] = 0;
        pvExpStr.setRaw( pvName );

      }

    }

  }
  else {

    if ( flag & ACTGRF_READBACKPVS_MASK ) {

      if ( numReadbackPvs ) {

        changed = 1;

        strncpy( value, readbackPvs[0], minStringSize() );
        value[minStringSize()] = 0;
        strncpy( curValue, readbackPvs[0], minStringSize() );
        curValue[minStringSize()] = 0;

        strncpy( pvName, readbackPvs[0], PV_Factory::MAX_PV_NAME );
        pvName[PV_Factory::MAX_PV_NAME] = 0;
        pvExpStr.setRaw( pvName );

      }

    }

  }

  if ( changed ) {

    stringLength = strlen( curValue );

    updateFont( curValue, fontTag, &fs,
     &fontAscent, &fontDescent, &fontHeight,
     &stringWidth );

    stringY = y + fontAscent + h/2 - fontHeight/2;

    if ( alignment == XmALIGNMENT_BEGINNING )
      stringX = x;
    else if ( alignment == XmALIGNMENT_CENTER )
      stringX = x + w/2 - stringWidth/2;
    else if ( alignment == XmALIGNMENT_END )
      stringX = x + w - stringWidth;

    updateDimensions();

  }

  if ( flag & ACTGRF_NULLPVS_MASK ) {
    if ( numNullPvs ) {
      svalPvExpStr.setRaw( nullPvs[0] );
    }
  }

}

void activeXTextDspClass::map ( void ) {

  if ( isWidget ) {
    if ( tf_widget ) {
      XtMapWidget( tf_widget );
    }
  }

}

void activeXTextDspClass::unmap ( void ) {

  if ( isWidget ) {
    if ( tf_widget ) {
      XtUnmapWidget( tf_widget );
    }
  }

}

void activeXTextDspClass::getPvs (
  int max,
  ProcessVariable *pvs[],
  int *n ) {

  if ( max < 3 ) {
    *n = 0;
    return;
  }

  *n = 3;
  pvs[0] = pvId;
  pvs[1] = svalPvId;
  pvs[2] = fgPvId;

}

char *activeXTextDspClass::crawlerGetFirstPv ( void ) {

  pvIndex = 0;
  return pvExpStr.getExpanded();

}

char *activeXTextDspClass::crawlerGetNextPv ( void ) {

  if ( pvIndex >= 2 ) return NULL;

  pvIndex++;

  switch ( pvIndex ) {
  case 1:
    return svalPvExpStr.getExpanded();
    break;
  case 2:
    return fgPvExpStr.getExpanded();
    break;
  }

  return NULL;

}

#ifdef __cplusplus
extern "C" {
#endif

void *create_activeXTextDspClassPtr ( void ) {

activeXTextDspClass *ptr;

  ptr = new activeXTextDspClass;
  return (void *) ptr;

}

void *clone_activeXTextDspClassPtr (
  void *_srcPtr )
{

activeXTextDspClass *ptr, *srcPtr;

  srcPtr = (activeXTextDspClass *) _srcPtr;

  ptr = new activeXTextDspClass( srcPtr );

  return (void *) ptr;

}

#ifdef __cplusplus
}
#endif
