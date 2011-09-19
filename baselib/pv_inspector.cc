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



#define __pv_inspector_cc 1

#define SMALL_SYM_ARRAY_SIZE 10
#define SMALL_SYM_ARRAY_LEN 31

#include "pv_inspector.h"
#include "app_pkg.h"
#include "act_win.h"

#include "thread.h"
#include "crc.h"

static char *G_SpecTypeNameReal = "Real";
static char *G_SpecTypeNameFloat = "Float";
static char *G_SpecTypeNameInt = "Int";
static char *G_SpecTypeNameShort = "Short";
static char *G_SpecTypeNameChar = "Char";
static char *G_SpecTypeNameEnum = "Enum";
static char *G_SpecTypeNameText = "Text";
static char *G_SpecTypeNameUnknown = "Unknown";

static char *G_TypeNameReal = "Real";
static char *G_TypeNameInt = "Int";
static char *G_TypeNameEnum = "Enum";
static char *G_TypeNameText = "Text";
static char *G_TypeNameUnknown = "Unknown";

static char *G_VectorID = "V";
static char *G_ScalarID = "S";

static void unconnectedTimeout (
  XtPointer client,
  XtIntervalId *id )
{

pvInspectorClass *pio = (pvInspectorClass *) client;

  pio->needTimeout = 1;
  pio->actWin->addDefExeNode( pio->aglPtr );
  pio->unconnectedTimer = 0;

}

static void rtypeUnconnectedTimeout (
  XtPointer client,
  XtIntervalId *id )
{

pvInspectorClass *pio = (pvInspectorClass *) client;

  pio->needRtypeTimeout = 1;
  pio->actWin->addDefExeNode( pio->aglPtr );
  pio->rtypeUnconnectedTimer = 0;

}

static void pioUpdateValue (
  Widget w,
  XtPointer client,
  XtPointer call )
{

pvInspectorClass *pio = (pvInspectorClass *) client;
char *buf;

  if ( pio->resolvingName ) {
    XBell( pio->actWin->d, 50 );
    return;
  }

  buf = XmTextGetString( pio->tf_widget );
  strncpy( pio->entryValue, buf, PV_Factory::MAX_PV_NAME );
  pio->entryValue[PV_Factory::MAX_PV_NAME] = 0;
  XtFree( buf );

  pio->needResolvePvName = 1;
  pio->actWin->addDefExeNode( pio->aglPtr );

}

static void pioSetValueChanged (
  Widget w,
  XtPointer client,
  XtPointer call )
{

pvInspectorClass *pio = (pvInspectorClass *) client;

  pio->widget_value_changed = 1;

}

static void pioGrabUpdate (
  Widget w,
  XtPointer client,
  XtPointer call )
{

pvInspectorClass *pio = (pvInspectorClass *) client;

  if ( !pio->grabUpdate ) {

    XSetInputFocus( pio->actWin->display(),
     XtWindow(pio->tf_widget), RevertToNone, CurrentTime );

  }

  pio->grabUpdate = 1;

}

static void pioSetSelection (
  Widget w,
  XtPointer client,
  XtPointer call )
{

pvInspectorClass *pio = (pvInspectorClass *) client;
int l;
char *buf;
Arg args[10];
int n;

  pio->widget_value_changed = 0;

  buf = XmTextGetString( pio->tf_widget );
  l = strlen(buf);
  XtFree( buf );

  n = 0;
  XtSetArg( args[n], XmNcursorPositionVisible, (XtArgVal) True ); n++;
  XtSetValues( pio->tf_widget, args, n );

#if 0
  if ( pio->autoSelect ) {
    XmTextSetSelection( pio->tf_widget, 0, l,
    XtLastTimestampProcessed( pio->actWin->display() ) );
  }
#endif

  XmTextSetInsertionPosition( pio->tf_widget, l );

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

pvInspectorClass *pio = (pvInspectorClass *) clientData;
char *str = (char *) value;

  if ( !pio ) return;

  if ( pio->resolvingName ) {
    XBell( pio->actWin->d, 50 );
    return;
  }

  if ( *type == XA_STRING ) {

    if ( str ) {

      strncpy( pio->entryValue, str, PV_Factory::MAX_PV_NAME );
      pio->entryValue[PV_Factory::MAX_PV_NAME] = 0;

      XmTextFieldSetString( pio->tf_widget, str );

      pio->needResolvePvName = 1;
      pio->actWin->addDefExeNode( pio->aglPtr );

    }

  }

}

static void handleDrop (
  Widget w,
  XtPointer client,
  XtPointer call )
{

pvInspectorClass *pio;
XmDropProcCallback ptr = (XmDropProcCallback) call;
XmDropTransferEntryRec transferEntries[2];
XmDropTransferEntry transferList;
int n;
Arg args[10];
Widget dc;

  n = 0;
  XtSetArg( args[n], XmNuserData, (XtPointer) &pio ); n++;
  XtGetValues( w, args, n );
  if ( !pio ) return;

  dc = ptr->dragContext;

  n = 0;
  if ( ptr->dropAction != XmDROP ) {
    XtSetArg( args[n], XmNtransferStatus, XmTRANSFER_FAILURE ); n++;
    XtSetArg( args[n], XmNnumDropTransfers, 0 ); n++;
  }
  else {
    transferEntries[0].target = XA_STRING;
    transferEntries[0].client_data = (XtPointer) pio;
    transferList = transferEntries;
    XtSetArg( args[n], XmNdropTransfers, transferList ); n++;
    XtSetArg( args[n], XmNnumDropTransfers, 1 ); n++;
    XtSetArg( args[n], XmNtransferProc, dropTransferProc ); n++;
  }

  //  XtSetArg( args[n], XmN ); n++;

  XmDropTransferStart( dc, args, n );

}

static void menu_cb (
  Widget w,
  XtPointer client,
  XtPointer call )
{

int i;
pvInspectorClass *pio = (pvInspectorClass *) client;
char syms[255+1];

  for ( i=0; i<pio->maxDsps; i++ ) {

    if ( w == pio->pb[i] ) {

      if ( pio->useRtype[i] ) {
        snprintf( syms, 255,
         "name=%s,rtype=%s,type=%s,specType=%s,dim=%s",
         pio->entryValue, pio->rtype, pio->pvTypeName(pio->pvType),
         pio->pvSpecificTypeName(pio->pvSpecificType),
         pio->vectorId(pio->isVector) );
      }
      else {
        snprintf( syms, 255,
         "name=%s,type=%s,specType=%s,dim=%s",
         pio->entryValue, pio->pvTypeName(pio->pvType),
         pio->pvSpecificTypeName(pio->pvSpecificType),
         pio->vectorId(pio->isVector) );
      }
      pio->symbolsExpStr[i].setRaw( syms );

      pio->popupDisplay( i );

      return;

    }

  }

}

static void monitor_pv_connect_state (
  ProcessVariable *pv,
  void *userarg )
{

pvInspectorClass *pio = (pvInspectorClass *) userarg;

  if ( pv->is_valid() ) {

  }
  else {

    pio->pvConnected = 0;

  }

}

static void monitor_rtype_pv_connect_state (
  ProcessVariable *pv,
  void *userarg )
{

pvInspectorClass *pio = (pvInspectorClass *) userarg;

  if ( pv->is_valid() ) {

  }
  else {

    pio->rtypePvConnected = 0;

  }

}

static void pv_update (
  ProcessVariable *pv,
  void *userarg )
{

pvInspectorClass *pio = (pvInspectorClass *) userarg;

  if ( pv->is_valid() ) {

    pio->pvConnected = 1;
    pio->pvType = (int) pv->get_type().type;
    pio->pvSpecificType = (int) pv->get_specific_type().type;
    if ( pv->get_dimension() > 1 ) {
      pio->isVector = 1;
    }
    else {
      pio->isVector = 0;
    }
    pio->needConnect = 1;
    pio->actWin->addDefExeNode( pio->aglPtr );

  }

}

static void rtype_pv_update (
  ProcessVariable *pv,
  void *userarg )
{

pvInspectorClass *pio = (pvInspectorClass *) userarg;

  if ( pv->is_valid() ) {

    pv->get_string( pio->rtype, 63 );
    pio->rtype[63] = 0;
    pio->rtypePvConnected = 1;
    pio->needRtypeConnect = 1;
    pio->actWin->addDefExeNode( pio->aglPtr );

  }

}

static void pic_edit_ok1 (
  Widget w,
  XtPointer client,
  XtPointer call )
{

pvInspectorClass *pio = (pvInspectorClass *) client;

  //pic_edit_update1 ( w, client, call );
  pio->ef1->popdownNoDestroy();

}

static void pic_edit_update (

  Widget w,
  XtPointer client,
  XtPointer call )
{

int i, more;
pvInspectorClass *pio = (pvInspectorClass *) client;

  pio->actWin->setChanged();

  pio->eraseSelectBoxCorners();
  pio->erase();

  pio->useAnyRtype = 0;
  pio->displayFileName[0].setRaw( pio->buf->bufDisplayFileName[0] );
  if ( blank( pio->displayFileName[0].getRaw() ) ) {
    pio->displayFileExt[0].setRaw( "" );
    pio->setPostion[0] = 0;
    pio->allowDups[0] = 0;
    pio->label[0].setRaw( "" );
    pio->useRtype[0] = 0;
    pio->useType[0] = 0;
    pio->useSpecType[0] = 0;
    pio->useDim[0] = 0;
    pio->numDsps = 0;
  }
  else {
    pio->displayFileExt[0].setRaw( pio->buf->bufDisplayFileExt[0] );
    pio->setPostion[0] = pio->buf->bufSetPostion[0];
    pio->allowDups[0] = pio->buf->bufAllowDups[0];
    pio->label[0].setRaw( pio->buf->bufLabel[0] );
    pio->useRtype[0] = pio->buf->bufUseRtype[0];
    if ( pio->useRtype[0] ) pio->useAnyRtype = 1;
    pio->useType[0] = pio->buf->bufUseType[0];
    pio->useSpecType[0] = pio->buf->bufUseSpecType[0];
    pio->useDim[0] = pio->buf->bufUseDim[0];
    pio->numDsps = 1;
  }

  if ( pio->numDsps ) {
    more = 1;
    for ( i=1; (i<pio->maxDsps) && more; i++ ) {
      pio->displayFileName[i].setRaw( pio->buf->bufDisplayFileName[i] );
      if ( blank( pio->displayFileName[i].getRaw() ) ) {
        pio->displayFileExt[i].setRaw( "" );
        pio->setPostion[i] = 0;
        pio->allowDups[i] = 0;
        pio->label[i].setRaw( "" );
        pio->useRtype[i] = 0;
        pio->useType[i] = 0;
        pio->useSpecType[i] = 0;
        pio->useDim[i] = 0;
        more = 0;
      }
      else {
        pio->displayFileExt[i].setRaw( pio->buf->bufDisplayFileExt[i] );
        pio->setPostion[i] = pio->buf->bufSetPostion[i];
        pio->allowDups[i] = pio->buf->bufAllowDups[i];
        pio->label[i].setRaw( pio->buf->bufLabel[i] );
        pio->useRtype[i] = pio->buf->bufUseRtype[i];
        if ( pio->useRtype[i] ) pio->useAnyRtype = 1;
        pio->useType[i] = pio->buf->bufUseType[i];
        pio->useSpecType[i] = pio->buf->bufUseSpecType[i];
        pio->useDim[i] = pio->buf->bufUseDim[i];
        (pio->numDsps)++;
      }
    }
  }

  for ( i=pio->numDsps; i<pio->maxDsps; i++ ) {
    pio->setPostion[i] = 0;
    pio->allowDups[i] = 0;
    pio->label[i].setRaw( "" );
    pio->useRtype[i] = 0;
    pio->useType[i] = 0;
    pio->useSpecType[i] = 0;
    pio->useDim[i] = 0;
  }

  strncpy( pio->fontTag, pio->fm.currentFontTag(), 63 );
  pio->actWin->fi->loadFontTag( pio->fontTag );
  pio->actWin->drawGc.setFontTag( pio->fontTag, pio->actWin->fi );
  pio->actWin->fi->getTextFontList( pio->fontTag, &pio->fontList );
  pio->fs = pio->actWin->fi->getXFontStruct( pio->fontTag );

  pio->topShadowColor = pio->buf->bufTopShadowColor;
  pio->botShadowColor = pio->buf->bufBotShadowColor;

  pio->fgColor.setColorIndex( pio->buf->bufFgColor, pio->actWin->ci );

  pio->bgColor.setColorIndex( pio->buf->bufBgColor, pio->actWin->ci );

  pio->ofsX = pio->buf->bufOfsX;

  pio->ofsY = pio->buf->bufOfsY;

  pio->noEdit = pio->buf->bufNoEdit;

  pio->x = pio->buf->bufX;
  pio->sboxX = pio->buf->bufX;

  pio->y = pio->buf->bufY;
  pio->sboxY = pio->buf->bufY;

  pio->w = pio->buf->bufW;
  pio->sboxW = pio->buf->bufW;

  pio->h = pio->buf->bufH;
  pio->sboxH = pio->buf->bufH;

  pio->buttonLabel.setRaw( pio->buf->bufButtonLabel );

  pio->updateDimensions();

  if ( pio->fs ) {
    pio->h = pio->fontHeight;
    pio->sboxH = pio->h;
  }

}

static void pic_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call )
{

pvInspectorClass *pio = (pvInspectorClass *) client;

  pic_edit_update ( w, client, call );
  pio->refresh( pio );

}

static void pic_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call )
{

pvInspectorClass *pio = (pvInspectorClass *) client;

  pic_edit_update ( w, client, call );
  pio->ef.popdown();
  pio->operationComplete();

  delete pio->buf;
  pio->buf = NULL;

}

static void pic_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call )
{

pvInspectorClass *pio = (pvInspectorClass *) client;

  pio->ef.popdown();
  pio->operationCancel();

  delete pio->buf;
  pio->buf = NULL;

}

static void pic_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call )
{

pvInspectorClass *pio = (pvInspectorClass *) client;

  delete pio->buf;
  pio->buf = NULL;

  pio->ef.popdown();
  pio->operationCancel();
  pio->erase();
  pio->deleteRequest = 1;
  pio->drawAll();

}

pvInspectorClass::pvInspectorClass ( void ) {

int i;

  name = new char[strlen("pvInspectorClass")+1];
  strcpy( name, "pvInspectorClass" );
  checkBaseClassVersion( activeGraphicClass::MAJOR_VERSION, name );

  activeMode = 0;
  ofsX = 0;
  ofsY = 0;
  noEdit = 0;
  useAnyRtype = 0;

  for ( i=0; i<maxDsps; i++ ) {
    setPostion[i] = 0;
    allowDups[i] = 0;
    propagateMacros[i] = 1;
    replaceSymbols[i] = 0;
    useRtype[i] = 0;
    useType[i] = 0;
    useSpecType[i] = 0;
    useDim[i] = 0;
  }

  numDsps = 0;

  fontList = NULL;
  aw = NULL;
  buf = NULL;
  unconnectedTimer = 0;
  rtypeUnconnectedTimer = 0;

}

pvInspectorClass::~pvInspectorClass ( void ) {

int okToClose;
activeWindowListPtr cur;

  if ( aw ) {

    okToClose = 0;
    // make sure the window is currently in list
    cur = actWin->appCtx->head->flink;
    while ( cur != actWin->appCtx->head ) {
      if ( &cur->node == aw ) {
        okToClose = 1;
        break;
      }
      cur = cur->flink;
    }

    if ( okToClose ) {
      if ( aw->okToDeactivate() ) {
        aw->returnToEdit( 1 );
        aw = NULL;
      }
      else {
        aw->closeDeferred( 20 );
        aw = NULL;
      }
    }

  }

  if ( name ) delete[] name;
  if ( fontList ) XmFontListFree( fontList );
  if ( buf ) {
    delete buf;
    buf = NULL;
  }

}

// copy constructor
pvInspectorClass::pvInspectorClass
 ( const pvInspectorClass *source ) {

int i;
activeGraphicClass *pio = (activeGraphicClass *) this;

  pio->clone( (activeGraphicClass *) source );

  name = new char[strlen("pvInspectorClass")+1];
  strcpy( name, "pvInspectorClass" );

  strncpy( fontTag, source->fontTag, 63 );
  fs = actWin->fi->getXFontStruct( fontTag );
  actWin->fi->getTextFontList( fontTag, &fontList );

  fontAscent = source->fontAscent;
  fontDescent = source->fontDescent;
  fontHeight = source->fontHeight;

  topShadowColor = source->topShadowColor;
  botShadowColor = source->botShadowColor;
  topShadowCb = source->topShadowCb;
  botShadowCb = source->botShadowCb;

  fgColor.copy(source->fgColor);
  bgColor.copy(source->bgColor);
  fgCb = source->fgCb;
  bgCb = source->bgCb;

  ofsX = source->ofsX;
  ofsY = source->ofsY;
  noEdit = source->noEdit;
  useAnyRtype = 0;

  for ( i=0; i<maxDsps; i++ ) {
    setPostion[i] = source->setPostion[i];
    allowDups[i] = source->allowDups[i];
    propagateMacros[i] = source->propagateMacros[i];
    displayFileName[i].copy( source->displayFileName[i] );
    displayFileExt[i].copy( source->displayFileExt[i] );
    label[i].copy( source->label[i] );
    symbolsExpStr[i].copy( source->symbolsExpStr[i] );
    replaceSymbols[i] = source->replaceSymbols[i];
    useRtype[i] = source->useRtype[i];
    useType[i] = source->useType[i];
    useSpecType[i] = source->useSpecType[i];
    useDim[i] = source->useDim[i];
  }

  numDsps = source->numDsps;

  buttonLabel.copy( source->buttonLabel );

  activeMode = 0;

  aw = NULL;
  buf = NULL;
  unconnectedTimer = 0;
  rtypeUnconnectedTimer = 0;

  doAccSubs( buttonLabel );
  for ( i=0; i<maxDsps; i++ ) {
    doAccSubs( label[i] );
    doAccSubs( displayFileName[i] );
    doAccSubs( displayFileExt[i] );
  }

}

int pvInspectorClass::createInteractive (
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

  strcpy( fontTag, actWin->defaultBtnFontTag );
  actWin->fi->loadFontTag( fontTag );
  fs = actWin->fi->getXFontStruct( fontTag );
  actWin->fi->getTextFontList( fontTag, &fontList );

  updateDimensions();

  topShadowColor = actWin->defaultTopShadowColor;
  botShadowColor = actWin->defaultBotShadowColor;

  fgColor.setColorIndex( actWin->defaultTextFgColor, actWin->ci );
  bgColor.setColorIndex( actWin->defaultBgColor, actWin->ci );

  this->draw();

  this->editCreate();

  return 1;

}

int pvInspectorClass::save (
  FILE *f )
{

int stat, major, minor, release;

tagClass tag;

int zero = 0;
char *emptyStr = "";

int setPosOriginal = 0;
static char *setPosEnumStr[3] = {
  "original",
  "button",
  "parentWindow"
};
static int setPosEnum[3] = {
  0,
  1,
  2
};

  major = PIC_MAJOR_VERSION;
  minor = PIC_MINOR_VERSION;
  release = PIC_RELEASE;

  tag.init();
  tag.loadW( "beginObjectProperties" );
  tag.loadW( "major", &major );
  tag.loadW( "minor", &minor );
  tag.loadW( "release", &release );
  tag.loadW( "x", &x );
  tag.loadW( "y", &y );
  tag.loadW( "w", &w );
  tag.loadW( "h", &h );
  tag.loadW( "fgColor", actWin->ci, &fgColor );
  tag.loadW( "bgColor", actWin->ci, &bgColor );
  tag.loadW( "topShadowColor", actWin->ci, &topShadowColor );
  tag.loadW( "botShadowColor", actWin->ci, &botShadowColor );
  tag.loadW( "font", fontTag );
  tag.loadW( "xPosOffset", &ofsX, &zero );
  tag.loadW( "yPosOffset", &ofsY, &zero );
  tag.loadBoolW( "noEdit", &noEdit, &zero );
  tag.loadW( "buttonLabel", &buttonLabel, emptyStr );
  tag.loadW( "numDsps", &numDsps );
  tag.loadW( "displayFileName", displayFileName, numDsps, emptyStr );
  tag.loadW( "displayFileExt", displayFileExt, numDsps, emptyStr );
  tag.loadW( "menuLabel", label, numDsps, emptyStr );
  tag.loadW( "setPosition", 3, setPosEnumStr, setPosEnum, setPostion, 
   numDsps, &setPosOriginal );
  tag.loadW( "allowDups", allowDups, numDsps, &zero );
  tag.loadW( "appendRtype", useRtype, numDsps, &zero );
  tag.loadW( "appendType", useType, numDsps, &zero );
  tag.loadW( "appendSpecificType", useSpecType, numDsps, &zero );
  tag.loadW( "appendDimension", useDim, numDsps, &zero );
  tag.loadW( unknownTags );
  tag.loadW( "endObjectProperties" );
  tag.loadW( "" );

  stat = tag.writeTags( f );

  return stat;

}

int pvInspectorClass::old_save (
  FILE *f )
{

int i, index;

  fprintf( f, "%-d %-d %-d\n", PIC_MAJOR_VERSION, PIC_MINOR_VERSION,
   PIC_RELEASE );

  fprintf( f, "%-d\n", x );
  fprintf( f, "%-d\n", y );
  fprintf( f, "%-d\n", w );
  fprintf( f, "%-d\n", h );

  index = fgColor.pixelIndex();
  actWin->ci->writeColorIndex( f, index );
  //fprintf( f, "%-d\n", index );

  index = bgColor.pixelIndex();
  actWin->ci->writeColorIndex( f, index );
  //fprintf( f, "%-d\n", index );

  index = topShadowColor;
  actWin->ci->writeColorIndex( f, index );
  //fprintf( f, "%-d\n", index );

  index = botShadowColor;
  actWin->ci->writeColorIndex( f, index );
  //fprintf( f, "%-d\n", index );

  if ( displayFileName[0].getRaw() )
    writeStringToFile( f, displayFileName[0].getRaw() );
  else
    writeStringToFile( f, "" );

  if ( label[0].getRaw() )
    writeStringToFile( f, label[0].getRaw() );
  else
    writeStringToFile( f, "" );

  writeStringToFile( f, fontTag );

  fprintf( f, "%-d\n", setPostion[0] );

  fprintf( f, "%-d\n", allowDups[0] );

  fprintf( f, "%-d\n", numDsps );

  for ( i=1; i<numDsps; i++ ) {

    if ( displayFileName[i].getRaw() )
      writeStringToFile( f, displayFileName[i].getRaw() );
    else
      writeStringToFile( f, "" );

    if ( label[i].getRaw() )
      writeStringToFile( f, label[i].getRaw() );
    else
      writeStringToFile( f, "" );

    fprintf( f, "%-d\n", setPostion[i] );

    fprintf( f, "%-d\n", allowDups[i] );

  }

  if ( buttonLabel.getRaw() ) {
    writeStringToFile( f, buttonLabel.getRaw() );
  }
  else {
    writeStringToFile( f, "" );
  }

  fprintf( f, "%-d\n", noEdit );

  fprintf( f, "%-d\n", ofsX );

  fprintf( f, "%-d\n", ofsY );

  return 1;

}

int pvInspectorClass::createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int i, n1, stat, major, minor, release;

tagClass tag;

int zero = 0;
char *emptyStr = "";

int setPosOriginal = 0;
static char *setPosEnumStr[3] = {
  "original",
  "button",
  "parentWindow"
};
static int setPosEnum[3] = {
  0,
  1,
  2
};

  major = PIC_MAJOR_VERSION;
  minor = PIC_MINOR_VERSION;
  release = PIC_RELEASE;

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
  tag.loadR( "fgColor", actWin->ci, &fgColor );
  tag.loadR( "bgColor", actWin->ci, &bgColor );
  tag.loadR( "topShadowColor", actWin->ci, &topShadowColor );
  tag.loadR( "botShadowColor", actWin->ci, &botShadowColor );
  tag.loadR( "font", 63, fontTag );
  tag.loadR( "xPosOffset", &ofsX, &zero );
  tag.loadR( "yPosOffset", &ofsY, &zero );
  tag.loadR( "noEdit", &noEdit, &zero );
  tag.loadR( "buttonLabel", &buttonLabel, emptyStr );
  tag.loadR( "numDsps", &numDsps, &zero );
  tag.loadR( "displayFileName", maxDsps, displayFileName, &n1, emptyStr );
  tag.loadR( "displayFileExt", maxDsps, displayFileExt, &n1, emptyStr );
  tag.loadR( "menuLabel", maxDsps, label, &n1, emptyStr );
  tag.loadR( "setPosition", 3, setPosEnumStr, setPosEnum, maxDsps, setPostion, 
   &n1, &setPosOriginal );
  tag.loadR( "allowDups", maxDsps, allowDups, &n1, &zero );
  tag.loadR( "appendRtype", maxDsps, useRtype, &n1, &zero );
  tag.loadR( "appendType", maxDsps, useType, &n1, &zero );
  tag.loadR( "appendSpecificType", maxDsps, useSpecType, &n1, &zero );
  tag.loadR( "appendDimension", maxDsps, useDim, &n1, &zero );
  tag.loadR( "endObjectProperties" );

  stat = tag.readTags( f, "endObjectProperties" );

  if ( !( stat & 1 ) ) {
    actWin->appCtx->postMessage( tag.errMsg() );
  }

  useAnyRtype = 0;
  for ( i=0; i<numDsps; i++ ) {
    if ( useRtype[i] ) {
      useAnyRtype = 1;
    }
  }

  if ( major > PIC_MAJOR_VERSION ) {
    postIncompatable();
    return 0;
  }

  if ( major < 4 ) {
    postIncompatable();
    return 0;
  }

  this->initSelectBox(); // call after getting x,y,w,h

  actWin->fi->loadFontTag( fontTag );
  actWin->drawGc.setFontTag( fontTag, actWin->fi );

  fs = actWin->fi->getXFontStruct( fontTag );
  actWin->fi->getTextFontList( fontTag, &fontList );

  updateDimensions();

  return stat;

}

int pvInspectorClass::old_createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int i, r, g, b, index, more, md;
int major, minor, release;
unsigned int pixel;
char oneName[255+1];

  this->actWin = _actWin;

  fscanf( f, "%d %d %d\n", &major, &minor, &release ); actWin->incLine();

  if ( major > PIC_MAJOR_VERSION ) {
    postIncompatable();
    return 0;
  }

  fscanf( f, "%d\n", &x ); actWin->incLine();
  fscanf( f, "%d\n", &y ); actWin->incLine();
  fscanf( f, "%d\n", &w ); actWin->incLine();
  fscanf( f, "%d\n", &h ); actWin->incLine();

  this->initSelectBox(); // call after getting x,y,w,h

  if ( ( major > 2 ) || ( ( major == 2 ) && ( minor > 4 ) ) ) {

    actWin->ci->readColorIndex( f, &index );
    actWin->incLine(); actWin->incLine();
    fgColor.setColorIndex( index, actWin->ci );

    actWin->ci->readColorIndex( f, &index );
    actWin->incLine(); actWin->incLine();
    bgColor.setColorIndex( index, actWin->ci );

    actWin->ci->readColorIndex( f, &index );
    actWin->incLine(); actWin->incLine();
    topShadowColor = index;

    actWin->ci->readColorIndex( f, &index );
    actWin->incLine(); actWin->incLine();
    botShadowColor = index;

  }
  else if ( major > 1 ) {

    fscanf( f, "%d\n", &index ); actWin->incLine();
    fgColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d\n", &index ); actWin->incLine();
    bgColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d\n", &index ); actWin->incLine();
    topShadowColor = index;

    fscanf( f, "%d\n", &index ); actWin->incLine();
    botShadowColor = index;

  }
  else {

    fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
    if ( ( major < 2 ) && ( minor < 2 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    actWin->ci->setRGB( r, g, b, &pixel );
    index = actWin->ci->pixIndex( pixel );
    fgColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
    if ( ( major < 2 ) && ( minor < 2 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    actWin->ci->setRGB( r, g, b, &pixel );
    index = actWin->ci->pixIndex( pixel );
    bgColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
    if ( ( major < 2 ) && ( minor < 2 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    actWin->ci->setRGB( r, g, b, &pixel );
    topShadowColor = actWin->ci->pixIndex( pixel );

    fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
    if ( ( major < 2 ) && ( minor < 2 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    actWin->ci->setRGB( r, g, b, &pixel );
    botShadowColor = actWin->ci->pixIndex( pixel );

  }

  readStringFromFile( oneName, 127+1, f ); actWin->incLine();
  displayFileName[0].setRaw( oneName );

  if ( blank( displayFileName[0].getRaw() ) ) {
    more = 0;
    numDsps = 0;
  }
  else {
    more = 1;
    numDsps = 1;
  }

  displayFileExt[0].setRaw( "" );

  readStringFromFile( oneName, 127+1, f ); actWin->incLine();
  label[0].setRaw( oneName );

  readStringFromFile( fontTag, 63+1, f ); actWin->incLine();

  if ( ( major > 1 ) || ( minor > 3 ) ) {
    fscanf( f, "%d\n", &setPostion[0] ); actWin->incLine();
  }
  else {
    setPostion[0] = 0;
  }

  if ( ( major > 1 ) || ( minor > 6 ) ) {
    fscanf( f, "%d\n", &allowDups[0] ); actWin->incLine();
  }
  else {
    allowDups[0] = 0;
  }

  // after v 2.3 read numDsps and then the data
  if ( ( major < 2 ) || ( ( major == 2 ) && ( minor < 4 ) ) ) {

    md = 8;

    if ( ( major > 2 ) || ( ( major == 2 ) && ( minor > 0 ) ) ) {

      for ( i=1; i<md; i++ ) { // for forward compatibility

        readStringFromFile( oneName, 127+1, f ); actWin->incLine();
        displayFileName[i].setRaw( oneName );

        if ( more && !blank(displayFileName[i].getRaw() ) ) {
          numDsps++;
        }
        else {
          more = 0;
        }

        displayFileExt[i].setRaw( "" );

        readStringFromFile( oneName, 127+1, f ); actWin->incLine();
        label[i].setRaw( oneName );

        fscanf( f, "%d\n", &setPostion[i] );

        fscanf( f, "%d\n", &allowDups[i] );

      }

      for ( i=numDsps; i<maxDsps; i++ ) {
        setPostion[i] = 0;
        allowDups[i] = 0;
        label[i].setRaw( "" );
      }

    }

    if ( ( major > 2 ) || ( ( major == 2 ) && ( minor > 1 ) ) ) {
      readStringFromFile( oneName, 127+1, f ); actWin->incLine();
      buttonLabel.setRaw( oneName );
    }
    else {
      buttonLabel.setRaw( label[0].getRaw() );
    }

    if ( ( major > 2 ) || ( ( major == 2 ) && ( minor > 2 ) ) ) {
      fscanf( f, "%d\n", &noEdit ); actWin->incLine();
    }
    else {
      noEdit = 0;
    }

  }
  else {

    fscanf( f, "%d\n", &numDsps ); actWin->incLine();

    for ( i=1; i<numDsps; i++ ) {

      readStringFromFile( oneName, 127+1, f ); actWin->incLine();
      displayFileName[i].setRaw( oneName );

      if ( blank(displayFileName[i].getRaw() ) ) {
        more = 0;
      }

      displayFileExt[i].setRaw( "" );

      readStringFromFile( oneName, 127+1, f ); actWin->incLine();
      label[i].setRaw( oneName );

      fscanf( f, "%d\n", &setPostion[i] );

      fscanf( f, "%d\n", &allowDups[i] );

    }

    for ( i=numDsps; i<maxDsps; i++ ) {
      setPostion[i] = 0;
      allowDups[i] = 0;
      label[i].setRaw( "" );
    }

    readStringFromFile( oneName, 127+1, f ); actWin->incLine();
    buttonLabel.setRaw( oneName );

    fscanf( f, "%d\n", &noEdit ); actWin->incLine();

  }

  if ( ( major > 2 ) || ( ( major == 2 ) && ( minor > 5 ) ) ) {
    fscanf( f, "%d\n", &ofsX ); actWin->incLine();
    fscanf( f, "%d\n", &ofsY ); actWin->incLine();
  }
  else {
    ofsX = 0;
    ofsY = 0;
  }

  actWin->fi->loadFontTag( fontTag );
  actWin->drawGc.setFontTag( fontTag, actWin->fi );

  fs = actWin->fi->getXFontStruct( fontTag );
  actWin->fi->getTextFontList( fontTag, &fontList );

  updateDimensions();

  return 1;

}

int pvInspectorClass::genericEdit ( void ) {

int i;
char title[32], *ptr;

  buf = new bufType;

  ptr = actWin->obj.getNameFromClass( "pvInspectorClass" );
  if ( ptr )
    strncpy( title, ptr, 31 );
  else
    strncpy( title, pvInspectorClass_str1, 31 );

  Strncat( title, pvInspectorClass_str2, 31 );

  buf->bufX = x;
  buf->bufY = y;
  buf->bufW = w;
  buf->bufH = h;

  strncpy( buf->bufFontTag, fontTag, 63 );

  buf->bufTopShadowColor = topShadowColor;
  buf->bufBotShadowColor = botShadowColor;

  buf->bufFgColor = fgColor.pixelIndex();

  buf->bufBgColor = bgColor.pixelIndex();

  buf->bufOfsX = ofsX;

  buf->bufOfsY = ofsY;

  buf->bufNoEdit = noEdit;

  for ( i=0; i<maxDsps; i++ ) {

    if ( displayFileName[i].getRaw() )
      strncpy( buf->bufDisplayFileName[i], displayFileName[i].getRaw(), 127 );
    else
      strncpy( buf->bufDisplayFileName[i], "", 127 );

    if ( displayFileExt[i].getRaw() )
      strncpy( buf->bufDisplayFileExt[i], displayFileExt[i].getRaw(), 15 );
    else
      strncpy( buf->bufDisplayFileExt[i], "", 15 );

    if ( label[i].getRaw() )
      strncpy( buf->bufLabel[i], label[i].getRaw(), 127 );
    else
      strncpy( buf->bufLabel[i], "", 127 );

    buf->bufSetPostion[i] = setPostion[i];

    buf->bufAllowDups[i] = allowDups[i];

    buf->bufUseRtype[i] = useRtype[i];
    buf->bufUseType[i] = useType[i];
    buf->bufUseSpecType[i] = useSpecType[i];
    buf->bufUseDim[i] = useDim[i];

  }

  if ( buttonLabel.getRaw() ) {
    strncpy( buf->bufButtonLabel, buttonLabel.getRaw(), 127 );
    buf->bufButtonLabel[127] = 0;
  }
  else {
    strncpy( buf->bufButtonLabel, "", 127 );
  }

  ef.create( actWin->top, actWin->appCtx->ci.getColorMap(),
   &actWin->appCtx->entryFormX,
   &actWin->appCtx->entryFormY, &actWin->appCtx->entryFormW,
   &actWin->appCtx->entryFormH, &actWin->appCtx->largestH,
   title, NULL, NULL, NULL );

  ef.addTextField( pvInspectorClass_str4, 35, &buf->bufX );
  ef.addTextField( pvInspectorClass_str5, 35, &buf->bufY );
  ef.addTextField( pvInspectorClass_str6, 35, &buf->bufW );
  ef.addTextField( pvInspectorClass_str7, 35, &buf->bufH );

  ef.addTextField( pvInspectorClass_str20, 35, buf->bufLabel[0], 127 );
  ef.addTextField( pvInspectorClass_str21, 35, buf->bufDisplayFileName[0],
   127 );
  ef.addTextField( pvInspectorClass_str38, 35, buf->bufDisplayFileExt[0],
   15 );
  ef.addOption( pvInspectorClass_str16, pvInspectorClass_str17,
   &buf->bufSetPostion[0] );
  ef.addTextField( pvInspectorClass_str18, 35, &buf->bufOfsX );
  ef.addTextField( pvInspectorClass_str19, 35, &buf->bufOfsY );
  ef.addToggle( pvInspectorClass_str3, &buf->bufAllowDups[0] );

  ef.addEmbeddedEf( pvInspectorClass_str14, "...", &ef1 );

  ef1->create( actWin->top, actWin->appCtx->ci.getColorMap(),
   &actWin->appCtx->entryFormX,
   &actWin->appCtx->entryFormY, &actWin->appCtx->entryFormW,
   &actWin->appCtx->entryFormH, &actWin->appCtx->largestH,
   title, NULL, NULL, NULL );

  for ( i=1; i<maxDsps; i++ ) {

    ef1->beginSubForm();
    ef1->addTextField( pvInspectorClass_str22, 35, buf->bufLabel[i], 127 );
    ef1->addLabel( pvInspectorClass_str23 );
    ef1->addTextField( "", 35, buf->bufDisplayFileName[i], 127 );
    ef1->addLabel( pvInspectorClass_str38 );
    ef1->addTextField( "", 15, buf->bufDisplayFileExt[i], 15 );
    ef1->endSubForm();

    ef1->beginLeftSubForm();
    ef1->addLabel( pvInspectorClass_str16 );
    ef1->addOption( " ", pvInspectorClass_str17, &buf->bufSetPostion[i] );
    ef1->addLabel( " " );
    ef1->addToggle( pvInspectorClass_str24, &buf->bufAllowDups[i] );
    ef1->addToggle( pvInspectorClass_str32, &buf->bufUseRtype[i] );
    ef1->addToggle( pvInspectorClass_str33, &buf->bufUseType[i] );
    ef1->addToggle( pvInspectorClass_str34, &buf->bufUseSpecType[i] );
    ef1->addToggle( pvInspectorClass_str35, &buf->bufUseDim[i] );
    ef1->endSubForm();

  }

  //ef1->finished( pic_edit_ok1, pic_edit_apply1, pic_edit_cancel1, this );
  ef1->finished( pic_edit_ok1, this );

  ef.addTextField( pvInspectorClass_str13, 35, buf->bufButtonLabel, 127 );

  ef.addToggle( pvInspectorClass_str15, &buf->bufNoEdit );

  ef.addToggle( pvInspectorClass_str28, &buf->bufUseRtype[0] );
  ef.addToggle( pvInspectorClass_str29, &buf->bufUseType[0] );
  ef.addToggle( pvInspectorClass_str30, &buf->bufUseSpecType[0] );
  ef.addToggle( pvInspectorClass_str31, &buf->bufUseDim[0] );

  ef.addColorButton( pvInspectorClass_str8, actWin->ci, &fgCb, &buf->bufFgColor );
  ef.addColorButton( pvInspectorClass_str9, actWin->ci, &bgCb, &buf->bufBgColor );
  ef.addColorButton( pvInspectorClass_str10, actWin->ci, &topShadowCb,
   &buf->bufTopShadowColor );
  ef.addColorButton( pvInspectorClass_str11, actWin->ci, &botShadowCb,
   &buf->bufBotShadowColor );
  ef.addFontMenu( pvInspectorClass_str12, actWin->fi, &fm, fontTag );

  XtUnmanageChild( fm.alignWidget() ); // no alignment info

  return 1;

}

int pvInspectorClass::editCreate ( void ) {

  this->genericEdit();
  ef.finished( pic_edit_ok, pic_edit_apply, pic_edit_cancel_delete, this );
  actWin->currentEf = NULL;
  ef.popup();

  return 1;

}

int pvInspectorClass::edit ( void ) {

  this->genericEdit();
  ef.finished( pic_edit_ok, pic_edit_apply, pic_edit_cancel, this );
  actWin->currentEf = &ef;
  ef.popup();

  return 1;

}

int pvInspectorClass::erase ( void ) {

  if ( deleteRequest ) return 1;

  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  return 1;

}

int pvInspectorClass::eraseActive ( void ) {

  if ( !enabled || !activeMode ) return 1;

  XDrawRectangle( actWin->d, drawable(actWin->executeWidget),
   actWin->executeGc.eraseGC(), x, y, w, h );

  XFillRectangle( actWin->d, drawable(actWin->executeWidget),
   actWin->executeGc.eraseGC(), x, y, w, h );

  return 1;

}

int pvInspectorClass::draw ( void ) {

int tX, tY;
XRectangle xR = { x, y, w, h };

  if ( deleteRequest ) return 1;

  actWin->drawGc.saveFg();

  actWin->drawGc.setFG( bgColor.pixelColor() );

  XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x, y, w, h );

  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x, y, w, h );

  actWin->drawGc.setFG( actWin->ci->pix(botShadowColor) );

  XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x, y, x+w, y );

  XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x, y, x, y+h );

   actWin->drawGc.setFG( actWin->ci->pix(topShadowColor) );

   XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
    actWin->drawGc.normGC(), x, y+h, x+w, y+h );

   XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
    actWin->drawGc.normGC(), x+w, y, x+w, y+h );

  actWin->drawGc.setFG( actWin->ci->pix(topShadowColor) );

  XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x+1, y+1, x+w-1, y+1 );

  XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x+2, y+2, x+w-2, y+2 );

  XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x+1, y+1, x+1, y+h-1 );

  XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x+2, y+2, x+2, y+h-2 );

  actWin->drawGc.setFG( actWin->ci->pix(botShadowColor) );

  XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x+1, y+h-1, x+w-1, y+h-1 );

  XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x+2, y+h-2, x+w-2, y+h-2 );

  XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x+w-1, y+1, x+w-1, y+h-1 );

  XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x+w-2, y+2, x+w-2, y+h-2 );

  if ( fs ) {

    actWin->drawGc.addNormXClipRectangle( xR );

    actWin->drawGc.setFG( fgColor.pixelColor() );
    actWin->drawGc.setFontTag( fontTag, actWin->fi );

    tX = x + w/2;
    tY = y + h/2 - fontAscent/2;

    if ( buttonLabel.getRaw() )
      drawText( actWin->drawWidget, &actWin->drawGc, fs, tX, tY,
       XmALIGNMENT_CENTER, buttonLabel.getRaw() );
    else
      drawText( actWin->drawWidget, &actWin->drawGc, fs, tX, tY,
       XmALIGNMENT_CENTER, "" );

    actWin->drawGc.removeNormXClipRectangle();

  }

  actWin->drawGc.restoreFg();

  return 1;

}

int pvInspectorClass::drawActive ( void ) {

int tX, tY;
char string[39+1];
XRectangle xR = { x, y, w, h };

  if ( !enabled || !activeMode ) return 1;

  return 1;

  actWin->executeGc.saveFg();

  actWin->executeGc.setFG( bgColor.getColor() );

  XFillRectangle( actWin->d, drawable(actWin->executeWidget),
   actWin->executeGc.normGC(), x, y, w, h );

  XDrawRectangle( actWin->d, drawable(actWin->executeWidget),
   actWin->executeGc.normGC(), x, y, w, h );

  if ( buttonLabel.getExpanded() )
    strncpy( string, buttonLabel.getExpanded(), 39 );
  else
    strncpy( string, "", 39 );

  actWin->executeGc.setFG( actWin->ci->pix(botShadowColor) );

  XDrawLine( actWin->d, drawable(actWin->executeWidget),
   actWin->executeGc.normGC(), x, y, x+w, y );

  XDrawLine( actWin->d, drawable(actWin->executeWidget),
   actWin->executeGc.normGC(), x, y, x, y+h );

  actWin->executeGc.setFG( actWin->ci->pix(topShadowColor) );

  XDrawLine( actWin->d, drawable(actWin->executeWidget),
   actWin->executeGc.normGC(), x, y+h, x+w, y+h );

  XDrawLine( actWin->d, drawable(actWin->executeWidget),
   actWin->executeGc.normGC(), x+w, y, x+w, y+h );

  // top
  actWin->executeGc.setFG( actWin->ci->pix(topShadowColor) );

  XDrawLine( actWin->d, drawable(actWin->executeWidget),
   actWin->executeGc.normGC(), x+1, y+1, x+w-1, y+1 );

  XDrawLine( actWin->d, drawable(actWin->executeWidget),
   actWin->executeGc.normGC(), x+2, y+2, x+w-2, y+2 );

  // left
  XDrawLine( actWin->d, drawable(actWin->executeWidget),
   actWin->executeGc.normGC(), x+1, y+1, x+1, y+h-1 );

  XDrawLine( actWin->d, drawable(actWin->executeWidget),
   actWin->executeGc.normGC(), x+2, y+2, x+2, y+h-2 );

  // bottom
  actWin->executeGc.setFG( actWin->ci->pix(botShadowColor) );

  XDrawLine( actWin->d, drawable(actWin->executeWidget),
   actWin->executeGc.normGC(), x+1, y+h-1, x+w-1, y+h-1 );

  XDrawLine( actWin->d, drawable(actWin->executeWidget),
   actWin->executeGc.normGC(), x+2, y+h-2, x+w-2, y+h-2 );

  // right
  XDrawLine( actWin->d, drawable(actWin->executeWidget),
   actWin->executeGc.normGC(), x+w-1, y+1, x+w-1, y+h-1 );

  XDrawLine( actWin->d, drawable(actWin->executeWidget),
   actWin->executeGc.normGC(), x+w-2, y+2, x+w-2, y+h-2 );

  if ( fs ) {

    actWin->executeGc.addNormXClipRectangle( xR );

    actWin->executeGc.setFG( fgColor.getColor() );
    actWin->executeGc.setFontTag( fontTag, actWin->fi );

    tX = x + w/2;
    tY = y + h/2 - fontAscent/2;

    drawText( actWin->executeWidget, drawable(actWin->executeWidget),
     &actWin->executeGc, fs, tX, tY, XmALIGNMENT_CENTER, string );

    actWin->executeGc.removeNormXClipRectangle();

  }

  actWin->executeGc.restoreFg();

  return 1;

}

int pvInspectorClass::activate (
  int pass,
  void *ptr )
{

int ii, opStat, n;
Arg args[5];
XmString str;
XmFontList textFontList = NULL;
Cardinal numImportTargets;
Atom importList[2];

  switch ( pass ) {

  case 1:

    aglPtr = ptr;
    aw = NULL;
    needClose = needResolvePvName = needConnect =
     needRtypeConnect = needTimeout = needRtypeTimeout = 0;
    tf_widget = NULL;
    grabUpdate = 0;
    resolvingName = 0;
    pvId = NULL;
    unconnectedTimer = 0;
    rtypeUnconnectedTimer = 0;
    msgDialogPoppedUp = 0;
    pvType = 0;
    pvSpecificType = 0;
    isVector = 0;
    pvConnected = 0;
    rtypePvConnected = 0;

    opComplete = 0;

    activeMode = 1;

    break;

  case 2:

    opStat = 1;

    msgDialog.create( actWin->executeWidgetId() );

    if ( !tf_widget ) {

      if ( fontTag ) {
        actWin->fi->getTextFontList( fontTag, &textFontList );
      }
      else {
        textFontList = NULL;
      }

      strcpy( entryValue, "" );

      tf_widget = XtVaCreateManagedWidget( "", xmTextFieldWidgetClass,
       actWin->executeWidget,
       XmNx, x,
       XmNy, y-3,
       XmNforeground, fgColor.getColor(),
       XmNbackground, bgColor.getColor(),
       XmNhighlightThickness, 0,
       XmNwidth, w,
       XmNvalue, entryValue,
       XmNmaxLength, (short) PV_Factory::MAX_PV_NAME,
       XmNpendingDelete, True,
       XmNmarginHeight, 0,
       XmNfontList, textFontList,
       XmNuserData, this,
       XmNcursorPositionVisible, False,
       NULL );

      if ( !enabled ) {
        XtUnmapWidget( tf_widget );
      }

      if ( textFontList ) XmFontListFree( textFontList );

      XtAddCallback( tf_widget, XmNfocusCallback,
       pioSetSelection, this );

      XtAddCallback( tf_widget, XmNmotionVerifyCallback,
       pioGrabUpdate, this );

      XtAddCallback( tf_widget, XmNvalueChangedCallback,
       pioSetValueChanged, this );

      XtAddCallback( tf_widget, XmNactivateCallback,
       pioUpdateValue, this );

      // change drop behavior

      importList[0] = XA_STRING;
      numImportTargets = 1;
      n = 0;
      XtSetArg( args[n], XmNimportTargets, importList ); n++;
      XtSetArg( args[n], XmNnumImportTargets, numImportTargets ); n++;
      XtSetArg( args[n], XmNdropProc, handleDrop ); n++;
      XmDropSiteUpdate( tf_widget, args, n );

    }

    if ( !opComplete ) {

      initEnable();

      n = 0;
      XtSetArg( args[n], XmNpopupEnabled, (XtArgVal) False ); n++;
      popUpMenu = XmCreatePopupMenu( actWin->topWidgetId(), "", args, n );

      pullDownMenu = XmCreatePulldownMenu( popUpMenu, "", NULL, 0 );

      for ( ii=0; ii<numDsps; ii++ ) {

        if ( label[ii].getExpanded() ) {
          str = XmStringCreateLocalized( label[ii].getExpanded() );
	}
	else {
          str = XmStringCreateLocalized( " " );
	}
        pb[ii] = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
         popUpMenu,
         XmNlabelString, str,
         NULL );
        XmStringFree( str );

        XtAddCallback( pb[ii], XmNactivateCallback, menu_cb,
         (XtPointer) this );

      }

      opComplete = 1;

    }

    return opStat;

    break;

  case 3:
  case 4:
  case 5:
  case 6:

    break;

  }

  return 1;

}

int pvInspectorClass::deactivate (
  int pass )
{

  if ( pass == 1 ) {

    activeMode = 0;

    if ( msgDialogPoppedUp ) {
      msgDialogPoppedUp = 0;
      msgDialog.popdown();
    }

    msgDialog.destroy(); 

    if ( unconnectedTimer ) {
      XtRemoveTimeOut( unconnectedTimer );
      unconnectedTimer = 0;
    }

    if ( rtypeUnconnectedTimer ) {
      XtRemoveTimeOut( rtypeUnconnectedTimer );
      rtypeUnconnectedTimer = 0;
    }

    if ( tf_widget ) {
      XtDestroyWidget( tf_widget );
      tf_widget = NULL;
    }

    XtDestroyWidget( popUpMenu );

  }



  return 1;

}

void pvInspectorClass::updateDimensions ( void )
{

  if ( fs ) {
    fontAscent = fs->ascent;
    fontDescent = fs->descent;
    fontHeight = fontAscent + fontDescent;
  }
  else {
    fontAscent = 10;
    fontDescent = 5;
    fontHeight = fontAscent + fontDescent;
  }

}

int pvInspectorClass::expandTemplate (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int i;
expStringClass tmpStr;

  for ( i=0; i<maxDsps; i++ ) {

    tmpStr.setRaw( label[i].getRaw() );
    tmpStr.expand1st( numMacros, macros, expansions );
    label[i].setRaw( tmpStr.getExpanded() );

    tmpStr.setRaw( displayFileName[i].getRaw() );
    tmpStr.expand1st( numMacros, macros, expansions );
    displayFileName[i].setRaw( tmpStr.getExpanded() );

    tmpStr.setRaw( displayFileExt[i].getRaw() );
    tmpStr.expand1st( numMacros, macros, expansions );
    displayFileExt[i].setRaw( tmpStr.getExpanded() );

  }

  tmpStr.setRaw( buttonLabel.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  buttonLabel.setRaw( tmpStr.getExpanded() );

  return 1;

}

int pvInspectorClass::expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int i;

  for ( i=0; i<maxDsps; i++ ) {
    label[i].expand1st( numMacros, macros, expansions );
    displayFileName[i].expand1st( numMacros, macros, expansions );
    displayFileExt[i].expand1st( numMacros, macros, expansions );
  }

  buttonLabel.expand1st( numMacros, macros, expansions );

  return 1;

}

int pvInspectorClass::expand2nd (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int i;

  for ( i=0; i<maxDsps; i++ ) {
    label[i].expand2nd( numMacros, macros, expansions );
    displayFileName[i].expand2nd( numMacros, macros, expansions );
    displayFileExt[i].expand2nd( numMacros, macros, expansions );
  }

  buttonLabel.expand2nd( numMacros, macros, expansions );

  return 1;

}

int pvInspectorClass::containsMacros ( void ) {

int i;

  for ( i=0; i<maxDsps; i++ ) {
    if ( label[i].containsPrimaryMacros() ) return 1;
    if ( displayFileName[i].containsPrimaryMacros() ) return 1;
    if ( displayFileExt[i].containsPrimaryMacros() ) return 1;
  }

  if ( buttonLabel.containsPrimaryMacros() ) return 1;

  return 0;

}

char *pvInspectorClass::vectorId (
  int isVector )
{

  if ( isVector ) {
    return G_VectorID;
  }
  else {
    return G_ScalarID;
  }

}

char *pvInspectorClass::pvSpecificTypeName (
  int pvSpecificTypeNum )
{

  if ( pvSpecificTypeNum == (int) ProcessVariable::specificType::real ) {
    return G_SpecTypeNameReal;
  }
  else if ( pvSpecificTypeNum == (int) ProcessVariable::specificType::flt ) {
    return G_SpecTypeNameFloat;
  }
  else if ( pvSpecificTypeNum == (int) ProcessVariable::specificType::integer ) {
    return G_SpecTypeNameInt;
  }
  else if ( pvSpecificTypeNum == (int) ProcessVariable::specificType::shrt ) {
    return G_SpecTypeNameShort;
  }
  else if ( pvSpecificTypeNum == (int) ProcessVariable::specificType::chr ) {
    return G_SpecTypeNameChar;
  }
  else if ( pvSpecificTypeNum == (int) ProcessVariable::specificType::enumerated ) {
    return G_SpecTypeNameEnum;
  }
  else if ( pvSpecificTypeNum == (int) ProcessVariable::specificType::text ) {
    return G_SpecTypeNameText;
  }

  return G_SpecTypeNameUnknown;

}

char *pvInspectorClass::pvTypeName (
  int pvTypeNum )
{

  if ( pvTypeNum == (int) ProcessVariable::Type::real ) {
    return G_TypeNameReal;
  }
  else if ( pvTypeNum == (int) ProcessVariable::Type::integer ) {
    return G_TypeNameInt;
  }
  else if ( pvTypeNum == (int) ProcessVariable::Type::enumerated ) {
    return G_TypeNameEnum;
  }
  else if ( pvTypeNum == (int) ProcessVariable::Type::text ) {
    return G_TypeNameText;
  }

  return G_TypeNameUnknown;

}

void pvInspectorClass::popupDisplay (
  int index )
{

activeWindowListPtr cur;
int i, l, stat, newX, newY;
char name[127+1], nameAndExt[127+1], nameWithParams[127+1], symbolsWithSubs[255+1];
unsigned int crc;
char *tk, *context, buf[255+1], *fileTk, *fileContext, fileBuf[255+1],
 *result, msg[79+1];
FILE *f;
expStringClass symbolsFromFile;
int gotSymbolsFromFile;

int useSmallArrays, symbolCount, maxSymbolLength;

char smallNewMacros[SMALL_SYM_ARRAY_SIZE+1][SMALL_SYM_ARRAY_LEN+1+1];
char smallNewValues[SMALL_SYM_ARRAY_SIZE+1][SMALL_SYM_ARRAY_LEN+1+1];

char *newMacros[100];
char *newValues[100];
int numNewMacros, max, numFound;

char prefix[127+1];

  posX = x;
  posY = y;

  // allow the syntax: @filename s1=v1,s2=v2,...
  // which means read symbols from file and append list
  gotSymbolsFromFile = 0;
  strncpy( buf, symbolsExpStr[index].getExpanded(), 255 );
  buf[255] = 0;
  context = NULL;
  tk = strtok_r( buf, " \t\n", &context );
  if ( tk ) {
    if ( tk[0] == '@' ) {
      if ( tk[1] ) {
        f = actWin->openAnyGenericFile( &tk[1], "r", name, 127 );
	if ( !f ) {
          snprintf( msg, 79, pvInspectorClass_str25, &tk[1] );
	  msg[79] = 0;
          actWin->appCtx->postMessage( msg );
          symbolsFromFile.setRaw( "" );
	}
	else {
	  result = fgets( fileBuf, 255, f );
	  if ( result ) {
            fileContext = NULL;
            fileTk = strtok_r( fileBuf, "\n", &fileContext );
            if ( fileTk ) {
              symbolsFromFile.setRaw( fileTk );
	    }
	    else {
              snprintf( msg, 79, pvInspectorClass_str26, name );
              msg[79] = 0;
              actWin->appCtx->postMessage( msg );
              symbolsFromFile.setRaw( "" );
	    }
	  }
	  else {
            if ( errno ) {
              snprintf( msg, 79, pvInspectorClass_str27, name );
	    }
	    else {
              snprintf( msg, 79, pvInspectorClass_str26, name );
	    }
            msg[79] = 0;
            actWin->appCtx->postMessage( msg );
            symbolsFromFile.setRaw( "" );
	  }
	  fclose( f );
	}
      }
      // append inline list to file contents
      tk = strtok_r( NULL, "\n", &context );
      if ( tk ) {
        strncpy( fileBuf, symbolsFromFile.getRaw(), 255 );
        fileBuf[255] = 0;
        if ( blank(fileBuf) ) {
          strcpy( fileBuf, "" );
	}
        else {
          Strncat( fileBuf, ",", 255 );
	}
	Strncat( fileBuf, tk, 255 );
        symbolsFromFile.setRaw( fileBuf );
      }
      // do special substitutions
      actWin->substituteSpecial( 255, symbolsFromFile.getExpanded(),
       symbolsWithSubs );
      gotSymbolsFromFile = 1;
    }
  }

  if ( !gotSymbolsFromFile ) {
    // do special substitutions
    actWin->substituteSpecial( 255, symbolsExpStr[index].getExpanded(),
     symbolsWithSubs );
  }

  numNewMacros = 0;

  // get info on whether to use the small local array for symbols
  stat = countSymbolsAndValues( symbolsWithSubs, &symbolCount,
   &maxSymbolLength );

  if ( !replaceSymbols[index] ) {

    if ( propagateMacros[index] ) {

      for ( i=0; i<actWin->numMacros; i++ ) {

        l = strlen(actWin->macros[i]);
        if ( l > maxSymbolLength ) maxSymbolLength = l;

        l = strlen(actWin->expansions[i]);
        if ( l > maxSymbolLength ) maxSymbolLength = l;

      }

      symbolCount += actWin->numMacros;

    }
    else {

      for ( i=0; i<actWin->appCtx->numMacros; i++ ) {

        l = strlen(actWin->appCtx->macros[i]);
        if ( l > maxSymbolLength ) maxSymbolLength = l;

        l = strlen(actWin->appCtx->expansions[i]);
        if ( l > maxSymbolLength ) maxSymbolLength = l;

      }

      symbolCount += actWin->appCtx->numMacros;

    }

  }

  useSmallArrays = 1;
  if ( symbolCount > SMALL_SYM_ARRAY_SIZE ) useSmallArrays = 0;
  if ( maxSymbolLength > SMALL_SYM_ARRAY_LEN ) useSmallArrays = 0;

  if ( useSmallArrays ) {

    for ( i=0; i<SMALL_SYM_ARRAY_SIZE; i++ ) {
      newMacros[i] = &smallNewMacros[i][0];
      newValues[i] = &smallNewValues[i][0];
    }

    if ( !replaceSymbols[index] ) {

      if ( propagateMacros[index] ) {

        for ( i=0; i<actWin->numMacros; i++ ) {

          strcpy( newMacros[i], actWin->macros[i] );

          strcpy( newValues[i], actWin->expansions[i] );

          numNewMacros++;

        }

      }
      else {

        for ( i=0; i<actWin->appCtx->numMacros; i++ ) {

          strcpy( newMacros[i], actWin->appCtx->macros[i] );

          strcpy( newValues[i], actWin->appCtx->expansions[i] );

          numNewMacros++;

        }

      }

    }

    max = SMALL_SYM_ARRAY_SIZE - numNewMacros;
    stat = parseLocalSymbolsAndValues( symbolsWithSubs, max,
     SMALL_SYM_ARRAY_LEN, &newMacros[numNewMacros], &newValues[numNewMacros],
     &numFound );
    numNewMacros += numFound;

  }
  else {

    if ( !replaceSymbols[index] ) {

      if ( propagateMacros[index] ) {

        for ( i=0; i<actWin->numMacros; i++ ) {

          l = strlen(actWin->macros[i]) + 1;
          newMacros[i] = (char *) new char[l];
          strcpy( newMacros[i], actWin->macros[i] );

          l = strlen(actWin->expansions[i]) + 1;
          newValues[i] = (char *) new char[l];
          strcpy( newValues[i], actWin->expansions[i] );

          numNewMacros++;

        }

      }
      else {

        for ( i=0; i<actWin->appCtx->numMacros; i++ ) {

          l = strlen(actWin->appCtx->macros[i]) + 1;
          newMacros[i] = (char *) new char[l];
          strcpy( newMacros[i], actWin->appCtx->macros[i] );

          l = strlen(actWin->appCtx->expansions[i]) + 1;
          newValues[i] = (char *) new char[l];
          strcpy( newValues[i], actWin->appCtx->expansions[i] );

          numNewMacros++;

        }

      }

    }

    max = 100 - numNewMacros;
    stat = parseSymbolsAndValues( symbolsWithSubs, max,
     &newMacros[numNewMacros], &newValues[numNewMacros], &numFound );
    numNewMacros += numFound;

  }

  strncpy( nameWithParams, displayFileName[index].getExpanded(), 127 );
  nameWithParams[127] = 0;
  if ( useRtype[index]) Strncat( nameWithParams, rtype, 127 );
  if ( useType[index] ) Strncat( nameWithParams, pvTypeName(pvType), 127 );
  if ( useSpecType[index] )
   Strncat( nameWithParams, pvSpecificTypeName(pvSpecificType), 127 );
  if ( useDim[index] ) Strncat( nameWithParams, vectorId(isVector), 127 );

  stat = getFileName( name, nameWithParams, 127 );

  strcpy( nameAndExt, name );

  if ( displayFileExt[index].getExpanded() ) {
    if ( !blank( displayFileExt[index].getExpanded() ) ) {
      Strncat( nameAndExt, displayFileExt[index].getExpanded(), 127 );
    }
  }

  stat = getFilePrefix( prefix, nameWithParams, 127 );

  // calc crc

  crc = 0;
  for ( i=0; i<numNewMacros; i++ ) {
    crc = updateCRC( crc, newMacros[i], strlen(newMacros[i]) );
    crc = updateCRC( crc, newValues[i], strlen(newValues[i]) );
  }

  if ( !allowDups[index] ) {
    cur = actWin->appCtx->head->flink;
    while ( cur != actWin->appCtx->head ) {
      if ( ( strcmp( name, cur->node.displayName ) == 0 ) &&
           ( strcmp( prefix, cur->node.prefix ) == 0 ) &&
           ( crc == cur->node.crc ) && !cur->node.isEmbedded ) {
        // display is already open; don't open another instance
	// move (maybe)
        if ( setPostion[index] == PIC_BUTTON_POS ) {
          newX = actWin->xPos()+posX+ofsX;
	  newY = actWin->yPos()+posY+ofsY;
          cur->node.move( newX, newY );
        }
        else if ( setPostion[index] == PIC_PARENT_OFS_POS ) {
          newX = actWin->xPos()+ofsX;
	  newY = actWin->yPos()+ofsY;
          cur->node.move( newX, newY );
        }
        // deiconify
        XMapWindow( cur->node.d, XtWindow(cur->node.topWidgetId()) );
        // raise
        XRaiseWindow( cur->node.d, XtWindow(cur->node.topWidgetId()) );
	// cleanup
        if ( !useSmallArrays ) {
          for ( i=0; i<numNewMacros; i++ ) {
            delete newMacros[i];
            delete newValues[i];
          }
        }
        goto done;
      }
      cur = cur->flink;
    }
  }

  cur = new activeWindowListType;
  actWin->appCtx->addActiveWindow( cur );

  if ( noEdit ) {
    cur->node.createNoEdit( actWin->appCtx, NULL, 0, 0, 0, 0,
     numNewMacros, newMacros, newValues );
  }
  else {
    cur->node.create( actWin->appCtx, NULL, 0, 0, 0, 0,
     numNewMacros, newMacros, newValues );
  }

  if ( !useSmallArrays ) {

    for ( i=0; i<numNewMacros; i++ ) {
      delete newMacros[i];
      delete newValues[i];
    }

  }

  cur->node.realize();
  cur->node.setGraphicEnvironment( &actWin->appCtx->ci, &actWin->appCtx->fi );

  cur->node.storeFileName( nameAndExt );

  if ( setPostion[index] == PIC_BUTTON_POS ) {
    actWin->appCtx->openActivateActiveWindow( &cur->node,
     //actWin->xPos()+x+ofsX, actWin->yPos()+y+ofsY );
     actWin->xPos()+posX+ofsX, actWin->yPos()+posY+ofsY );
  }
  else if ( setPostion[index] == PIC_PARENT_OFS_POS ) {
    actWin->appCtx->openActivateActiveWindow( &cur->node,
     actWin->xPos()+ofsX, actWin->yPos()+ofsY );
  }
  else {
    actWin->appCtx->openActivateActiveWindow( &cur->node );
  }

  aw = NULL;

done:

  return;

}

void pvInspectorClass::btnUp (
  XButtonEvent *be,
  int _x,
  int _y,
  int buttonState,
  int buttonNumber,
  int *action )
{

  *action = 0;

  if ( !enabled ) return;

  if ( numDsps < 2 ) return;

  if ( buttonNumber != 1 ) return;

  posX = x + _x - be->x;
  posY = y + _y - be->y;

  XmMenuPosition( popUpMenu, be );
  XtManageChild( popUpMenu );

}

void pvInspectorClass::btnDown (
  XButtonEvent *be,
  int _x,
  int _y,
  int buttonState,
  int buttonNumber,
  int *action )
{

  *action = 0;

  if ( !enabled ) return;

  if ( buttonNumber != 1 ) return;

  if ( numDsps < 1 ) return;

 if ( numDsps == 1 ) {
    posX = x + _x - be->x;
    posY = y + _y - be->y;
    popupDisplay( 0 );
  }

}

void pvInspectorClass::pointerIn (
  XMotionEvent *me,
  int _x,
  int _y,
  int buttonState )
{

  if ( !enabled ) return;

  activeGraphicClass::pointerIn( me, me->x, me->y, buttonState );

}

void pvInspectorClass::pointerOut (
  XMotionEvent *me,
  int _x,
  int _y,
  int buttonState )
{

  if ( !enabled ) return;

  activeGraphicClass::pointerOut( me, me->x, me->y, buttonState );

}

void pvInspectorClass::mousePointerIn (
  XMotionEvent *me,
  int _x,
  int _y,
  int buttonState )
{

  if ( !enabled ) return;

}

void pvInspectorClass::mousePointerOut (
  XMotionEvent *me,
  int _x,
  int _y,
  int buttonState )
{

  if ( !enabled ) return;

}

int pvInspectorClass::getButtonActionRequest (
  int *up,
  int *down,
  int *drag,
  int *focus )
{

  *drag = 0;
  *down = 0;
  *up = 0;
  *focus = 0;

  return 1;

}

void pvInspectorClass::changeDisplayParams (
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

  if ( _flag & ACTGRF_BGCOLOR_MASK )
    bgColor.setColorIndex( _bgColor, actWin->ci );

  if ( _flag & ACTGRF_TOPSHADOWCOLOR_MASK )
    topShadowColor = _topShadowColor;

  if ( _flag & ACTGRF_BOTSHADOWCOLOR_MASK )
    botShadowColor = _botShadowColor;

  if ( _flag & ACTGRF_BTNFONTTAG_MASK ) {
    strncpy( fontTag, _btnFontTag, 63 );
    fontTag[63] = 0;
    actWin->fi->loadFontTag( fontTag );
    fs = actWin->fi->getXFontStruct( fontTag );
    updateDimensions();
  }

}

void pvInspectorClass::executeDeferred ( void ) {

int nc, nrpv, ncon, nrtcon, nto, nrto, okToClose;
char value[PV_Factory::MAX_PV_NAME+1], syms[255+1];
activeWindowListPtr cur;

  actWin->appCtx->proc->lock();
  nrpv = needResolvePvName; needResolvePvName = 0;
  ncon = needConnect; needConnect = 0;
  nrtcon = needRtypeConnect; needRtypeConnect = 0;
  nto = needTimeout; needTimeout = 0;
  nrto = needRtypeTimeout; needRtypeTimeout = 0;
  nc = needClose; needClose = 0;
  strncpy( value, entryValue, PV_Factory::MAX_PV_NAME );
  value[PV_Factory::MAX_PV_NAME] = 0;
  actWin->remDefExeNode( aglPtr );
  actWin->appCtx->proc->unlock();

  if ( nrpv && !blank(value) ) {

    pvConnected = rtypePvConnected = displayOpen = isVector = 0;
    pvType = pvSpecificType = -1;
    resolvingName = 1;
    strcpy( rtypeFieldName, "" );
    Strncat( rtypeFieldName, value, PV_Factory::MAX_PV_NAME );
    Strncat( rtypeFieldName, ".RTYP", PV_Factory::MAX_PV_NAME );

    if ( useAnyRtype ) {
      rtypePvId = the_PV_Factory->create( rtypeFieldName );
      rtypePvId->add_conn_state_callback( monitor_rtype_pv_connect_state,
       (void *) this );
      rtypePvId->add_value_callback( rtype_pv_update,
       (void *) this );
      if ( !unconnectedTimer ) {
        rtypeUnconnectedTimer = appAddTimeOut( actWin->appCtx->appContext(),
         5000, rtypeUnconnectedTimeout, this );
      }
    }
    else {
      rtypePvId = NULL;
      strcpy( rtype, "" );
    }

    pvId = the_PV_Factory->create( value );
    pvId->add_conn_state_callback( monitor_pv_connect_state,
     (void *) this );
    pvId->add_value_callback( pv_update,
     (void *) this );

    if ( !unconnectedTimer ) {
      unconnectedTimer = appAddTimeOut( actWin->appCtx->appContext(),
       5000, unconnectedTimeout, this );
    }

    msgDialog.popup( pvInspectorClass_str36, x+actWin->xPos(),
     y+actWin->yPos() );
    msgDialogPoppedUp = 1;

  }

  if ( ncon ) {
    if ( pvId ) {
      if ( unconnectedTimer ) {
        XtRemoveTimeOut( unconnectedTimer );
        unconnectedTimer = 0;
      }
      if ( ( rtypePvConnected || !useAnyRtype ) && !displayOpen ) {
        if ( msgDialogPoppedUp ) {
          msgDialog.popdown();
          msgDialogPoppedUp = 0;
	}
        displayOpen = 1;
        if ( numDsps == 1 ) {
          if ( useAnyRtype ) {
            snprintf( syms, 255,
             "name=%s,rtype=%s,type=%s,specType=%s,dim=%s",
             value, rtype, pvTypeName(pvType),
             pvSpecificTypeName(pvSpecificType),
             vectorId(isVector) );
	  }
	  else {
            snprintf( syms, 255,
             "name=%s,type=%s,specType=%s,dim=%s",
             value, pvTypeName(pvType), pvSpecificTypeName(pvSpecificType),
             vectorId(isVector) );
	  }
          symbolsExpStr[0].setRaw( syms );
          popupDisplay( 0 );
	}
	else {
          XButtonEvent be;
	  memset( (char *) &be, 0, sizeof(be) );
          be.x = x;
          be.y = y;
          be.x_root = x+actWin->xPos();
          be.y_root = y+actWin->yPos();
          XmMenuPosition( popUpMenu, &be );
          XtManageChild( popUpMenu );
	}
      }
      pvId->remove_conn_state_callback( monitor_pv_connect_state,
       (void *) this );
      pvId->remove_value_callback( pv_update,
       (void *) this );
      pvId->release();
      pvId = NULL;
      resolvingName = 0;
    }
  }

  if ( nrtcon ) {
    if ( rtypePvId ) {
      if ( rtypeUnconnectedTimer ) {
        XtRemoveTimeOut( rtypeUnconnectedTimer );
        rtypeUnconnectedTimer = 0;
      }
      if ( pvConnected && !displayOpen ) {
        if ( msgDialogPoppedUp ) {
          msgDialog.popdown();
          msgDialogPoppedUp = 0;
	}
        displayOpen = 1;
        if ( numDsps == 1 ) {
          snprintf( syms, 255,
           "name=%s,rtype=%s,type=%s,specType=%s,dim=%s",
           value, rtype, pvTypeName(pvType),
           pvSpecificTypeName(pvSpecificType),
           vectorId(isVector) );
          symbolsExpStr[0].setRaw( syms );
          popupDisplay( 0 );
	}
	else {
          XButtonEvent be;
	  memset( (char *) &be, 0, sizeof(be) );
          be.x = x;
          be.y = y;
          be.x_root = x+actWin->xPos();
          be.y_root = y+actWin->yPos();
          XmMenuPosition( popUpMenu, &be );
          XtManageChild( popUpMenu );
	}
      }
      rtypePvId->remove_conn_state_callback( monitor_rtype_pv_connect_state,
       (void *) this );
      rtypePvId->remove_value_callback( rtype_pv_update,
       (void *) this );
      rtypePvId->release();
      rtypePvId = NULL;
    }
  }

  if ( nto ) {
    if ( pvId ) {
      pvId->remove_conn_state_callback( monitor_pv_connect_state,
       (void *) this );
      pvId->release();
      pvId = NULL;
      resolvingName = 0;
      actWin->appCtx->postMessage( pvInspectorClass_str37 );
      if ( msgDialogPoppedUp ) {
        msgDialog.popdown();
        msgDialogPoppedUp = 0;
      }
    }
  }

  if ( nrto ) {
    if ( rtypePvId ) {
      rtypePvId->remove_conn_state_callback( monitor_rtype_pv_connect_state,
       (void *) this );
      rtypePvId->release();
      rtypePvId = NULL;
      strcpy( rtype, "" );
      if ( pvConnected && !displayOpen ) {
        if ( msgDialogPoppedUp ) {
          msgDialog.popdown();
          msgDialogPoppedUp = 0;
        }
        displayOpen = 1;
        if ( numDsps == 1 ) {
          snprintf( syms, 255,
           "name=%s,rtype=N/A,type=%s,specType=%s,dim=%s",
           value, pvTypeName(pvType), pvSpecificTypeName(pvSpecificType),
           vectorId(isVector) );
          symbolsExpStr[0].setRaw( syms );
          popupDisplay( 0 );
	}
	else {
          XButtonEvent be;
	  memset( (char *) &be, 0, sizeof(be) );
          be.x = x;
          be.y = y;
          be.x_root = x+actWin->xPos();
          be.y_root = y+actWin->yPos();
          XmMenuPosition( popUpMenu, &be );
          XtManageChild( popUpMenu );
	}
      }
    }
  }

  if ( nc ) {

    if ( aw ) {

      okToClose = 0;
      // make sure the window was successfully opened
      cur = actWin->appCtx->head->flink;
      while ( cur != actWin->appCtx->head ) {
        if ( &cur->node == aw ) {
          okToClose = 1;
          break;
        }
        cur = cur->flink;
      }

      if ( okToClose ) {
        if ( aw->okToDeactivate() ) {
          aw->returnToEdit( 1 );
          aw = NULL;
	}
        else {
          aw->closeDeferred( 20 );
          aw = NULL;
	}
      }
      else {
        aw = NULL;
      }

    }

  }

}

char *pvInspectorClass::getSearchString (
  int i
) {

int num = 3 * numDsps + 1;
int ii, selector, index;

  if ( i == 0 ) {
    return buttonLabel.getRaw();
  }
  else if ( ( i > 0 ) && ( i < num ) ) {
    ii = i - 1;
    selector = ii % 3;
    index = ii / 3;
    if ( selector == 0 ) {
      return label[index].getRaw();
    }
    else if ( selector == 1 ) {
      return displayFileName[index].getRaw();
    }
    else if ( selector == 2 ) {
      return displayFileExt[index].getRaw();
    }
  }

  return NULL;

}

void pvInspectorClass::replaceString (
  int i,
  int max,
  char *string
) {

int num = 3 * numDsps + 1;
int ii, selector, index;

  if ( i == 0 ) {
    buttonLabel.setRaw( string );
  }
  else if ( ( i > 0 ) && ( i < num ) ) {
    ii = i - 1;
    selector = ii % 3;
    index = ii / 3;
    if ( selector == 0 ) {
      label[index].setRaw( string );
    }
    else if ( selector == 1 ) {
      displayFileName[index].setRaw( string );
    }
    else if ( selector == 2 ) {
      displayFileExt[index].setRaw( string );
    }
  }

}

#ifdef __cplusplus
extern "C" {
#endif

void *create_pvInspectorClassPtr ( void ) {

pvInspectorClass *ptr;

  ptr = new pvInspectorClass;
  return (void *) ptr;

}

void *clone_pvInspectorClassPtr (
  void *_srcPtr )
{

pvInspectorClass *ptr, *srcPtr;

  srcPtr = (pvInspectorClass *) _srcPtr;

  ptr = new pvInspectorClass( srcPtr );

  return (void *) ptr;

}

#ifdef __cplusplus
}
#endif
