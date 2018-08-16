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



// Cascade option is no longer used



#define __related_display_cc 1

#define SMALL_SYM_ARRAY_SIZE 10
#define SMALL_SYM_ARRAY_LEN 31

#include "utility.h"
#include "related_display.h"
#include "app_pkg.h"
#include "act_win.h"

#include "thread.h"
#include "crc.h"

#ifdef TRIUMF
#endif

static void doBlink (
  void *ptr
) {

relatedDisplayClass *rdo = (relatedDisplayClass *) ptr;

  if ( !rdo->activeMode ) {
    if ( rdo->isSelected() ) rdo->drawSelectBoxCorners(); // erase via xor
    rdo->smartDrawAll();
    if ( rdo->isSelected() ) rdo->drawSelectBoxCorners();
  }
  else {
    rdo->bufInvalidate();
    rdo->smartDrawAllActive();
  }

}

static void unconnectedTimeout (
  XtPointer client,
  XtIntervalId *id )
{

relatedDisplayClass *rdo = (relatedDisplayClass *) client;

  if ( !rdo->init ) {
    rdo->needToDrawUnconnected = 1;
    rdo->needRefresh = 1;
    rdo->actWin->addDefExeNode( rdo->aglPtr );
  }

  rdo->unconnectedTimer = 0;

}

static void menu_cb (
  Widget w,
  XtPointer client,
  XtPointer call )
{

int i;
relatedDisplayClass *rdo = (relatedDisplayClass *) client;

 for ( i=0; i<rdo->maxDsps; i++ ) {
   if ( w == rdo->pb[i] ) {
     rdo->popupDisplay( i );
     return;
   }
 }

}

static void relDsp_monitor_dest_connect_state (
  ProcessVariable *pv,
  void *userarg )
{

objAndIndexType *ptr = (objAndIndexType *) userarg;
relatedDisplayClass *rdo = (relatedDisplayClass *) ptr->obj;
int i = ptr->index;

  if ( pv->is_valid() ) {

    if ( !rdo->connection.pvsConnected() ) {

      rdo->connection.setPvConnected( (void *) ptr->index );
      if ( rdo->connection.pvsConnected() ) {

        rdo->actWin->appCtx->proc->lock();
        rdo->destType[i] = (int) pv->get_type().type;
	rdo->needConnect = 1;
        rdo->actWin->addDefExeNode( rdo->aglPtr );
        rdo->actWin->appCtx->proc->unlock();

      }
      else {

        rdo->connection.setPvDisconnected( (void *) ptr->index );
        rdo->actWin->appCtx->proc->lock();
        rdo->needRefresh = 1;
        rdo->actWin->addDefExeNode( rdo->aglPtr );
        rdo->actWin->appCtx->proc->unlock();

      }

    }

  }

}

static void relDsp_monitor_color_connect_state (
  ProcessVariable *pv,
  void *userarg )
{

relatedDisplayClass *rdo = (relatedDisplayClass *) userarg;

  if ( pv->is_valid() ) {

    if ( !rdo->connection.pvsConnected() ) {

      rdo->connection.setPvConnected( (void *) relatedDisplayClass::NUMPVS );
      if ( rdo->connection.pvsConnected() ) {

        rdo->actWin->appCtx->proc->lock();
	rdo->needConnect = 1;
        rdo->actWin->addDefExeNode( rdo->aglPtr );
        rdo->actWin->appCtx->proc->unlock();

      }

    }

  }
  else {

    rdo->connection.setPvDisconnected( (void *) relatedDisplayClass::NUMPVS );
    rdo->fgColor.setDisconnected();
    rdo->bgColor.setDisconnected();
    rdo->actWin->appCtx->proc->lock();
    rdo->active = 0;
    rdo->needRefresh = 1;
    rdo->actWin->addDefExeNode( rdo->aglPtr );
    rdo->actWin->appCtx->proc->unlock();

  }

}

static void relDsp_color_value_update (
  ProcessVariable *pv,
  void *userarg )
{

relatedDisplayClass *rdo = (relatedDisplayClass *) userarg;

  rdo->actWin->appCtx->proc->lock();
  rdo->needUpdate = 1;
  rdo->actWin->addDefExeNode( rdo->aglPtr );
  rdo->actWin->appCtx->proc->unlock();

}

static void rdc_edit_ok1 (
  Widget w,
  XtPointer client,
  XtPointer call )
{

relatedDisplayClass *rdo = (relatedDisplayClass *) client;

  //rdc_edit_update1 ( w, client, call );
  rdo->ef1->popdownNoDestroy();

}

static void rdc_edit_update (

  Widget w,
  XtPointer client,
  XtPointer call )
{

int i, ii;
relatedDisplayClass *rdo = (relatedDisplayClass *) client;

  rdo->actWin->setChanged();

  rdo->eraseSelectBoxCorners();
  rdo->erase();

  trimWhiteSpace( rdo->buf->bufDisplayFileName[0] );
  rdo->displayFileName[0].setRaw( rdo->buf->bufDisplayFileName[0] );
  if ( blank( rdo->displayFileName[0].getRaw() ) ) {
    rdo->closeAction[0] = 0;
    rdo->setPostion[0] = 0;
    rdo->allowDups[0] = 0;
    rdo->cascade[0] = 0;
    rdo->propagateMacros[0] = 1;
    rdo->label[0].setRaw( "" );
    rdo->symbolsExpStr[0].setRaw( "" );
    rdo->replaceSymbols[0] = 0;
    rdo->numDsps = 0;
    ii = 0;
  }
  else {
    rdo->closeAction[0] = rdo->buf->bufCloseAction[0];
    rdo->setPostion[0] = rdo->buf->bufSetPostion[0];
    rdo->allowDups[0] = rdo->buf->bufAllowDups[0];
    rdo->cascade[0] = rdo->buf->bufCascade[0];
    rdo->propagateMacros[0] = rdo->buf->bufPropagateMacros[0];
    rdo->label[0].setRaw( rdo->buf->bufLabel[0] );
    rdo->symbolsExpStr[0].setRaw( rdo->buf->bufSymbols[0] );
    rdo->replaceSymbols[0] = rdo->buf->bufReplaceSymbols[0];
    rdo->numDsps = 1;
    ii = 1;
  }

  for ( i=ii; i<rdo->maxDsps; i++ ) {
    rdo->displayFileName[i].setRaw( rdo->buf->bufDisplayFileName[i] );
    if ( !blank( rdo->buf->bufDisplayFileName[i] ) ) {
      trimWhiteSpace( rdo->buf->bufDisplayFileName[i] );
      rdo->displayFileName[ii].setRaw( rdo->buf->bufDisplayFileName[i] );
      rdo->closeAction[ii] = rdo->buf->bufCloseAction[i];
      rdo->setPostion[ii] = rdo->buf->bufSetPostion[i];
      rdo->allowDups[ii] = rdo->buf->bufAllowDups[i];
      rdo->cascade[ii] = rdo->buf->bufCascade[i];
      rdo->propagateMacros[ii] = rdo->buf->bufPropagateMacros[i];
      rdo->label[ii].setRaw( rdo->buf->bufLabel[i] );
      rdo->symbolsExpStr[ii].setRaw( rdo->buf->bufSymbols[i] );
      rdo->replaceSymbols[ii] = rdo->buf->bufReplaceSymbols[i];
      (rdo->numDsps)++;
      ii++;
    }
  }

  for ( i=rdo->numDsps; i<rdo->maxDsps; i++ ) {
    rdo->closeAction[i] = 0;
    rdo->setPostion[i] = 0;
    rdo->allowDups[i] = 0;
    rdo->cascade[i] = 0;
    rdo->propagateMacros[i] = 1;
    rdo->label[i].setRaw( "" );
    rdo->symbolsExpStr[i].setRaw( "" );
    rdo->replaceSymbols[i] = 0;
  }

  strncpy( rdo->fontTag, rdo->fm.currentFontTag(), 63 );
  rdo->actWin->fi->loadFontTag( rdo->fontTag );
  rdo->actWin->drawGc.setFontTag( rdo->fontTag, rdo->actWin->fi );
  rdo->actWin->fi->getTextFontList( rdo->fontTag, &rdo->fontList );
  rdo->fs = rdo->actWin->fi->getXFontStruct( rdo->fontTag );

  rdo->topShadowColor = rdo->buf->bufTopShadowColor;
  rdo->botShadowColor = rdo->buf->bufBotShadowColor;

  rdo->fgColor.setColorIndex( rdo->buf->bufFgColor, rdo->actWin->ci );

  rdo->bgColor.setColorIndex( rdo->buf->bufBgColor, rdo->actWin->ci );

  rdo->invisible = rdo->buf->bufInvisible;

  rdo->ofsX = rdo->buf->bufOfsX;

  rdo->ofsY = rdo->buf->bufOfsY;

  rdo->noEdit = rdo->buf->bufNoEdit;

  rdo->useFocus = rdo->buf->bufUseFocus;

  if ( !rdo->useFocus ) {
    rdo->button3Popup = rdo->buf->bufButton3Popup;
  }
  else {
    rdo->button3Popup = 0;
  }

  rdo->icon = rdo->buf->bufIcon;

  rdo->swapButtons = rdo->buf->bufSwapButtons;

  rdo->x = rdo->buf->bufX;
  rdo->sboxX = rdo->buf->bufX;

  rdo->y = rdo->buf->bufY;
  rdo->sboxY = rdo->buf->bufY;

  rdo->w = rdo->buf->bufW;
  rdo->sboxW = rdo->buf->bufW;

  rdo->h = rdo->buf->bufH;
  rdo->sboxH = rdo->buf->bufH;

  rdo->buttonLabel.setRaw( rdo->buf->bufButtonLabel );

  for ( i=0; i<rdo->NUMPVS; i++ ) {
    rdo->destPvExpString[i].setRaw( rdo->buf->bufDestPvName[i] );
    rdo->sourceExpString[i].setRaw( rdo->buf->bufSource[i] );
  }

  rdo->colorPvExpString.setRaw( rdo->buf->bufColorPvName );

  rdo->helpCommandExpString.setRaw( rdo->buf->bufHelpCommand );

  rdo->updateDimensions();

}

static void rdc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call )
{

relatedDisplayClass *rdo = (relatedDisplayClass *) client;

  rdc_edit_update ( w, client, call );
  rdo->refresh( rdo );

}

static void rdc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call )
{

relatedDisplayClass *rdo = (relatedDisplayClass *) client;

  rdc_edit_update ( w, client, call );
  rdo->ef.popdown();
  rdo->operationComplete();

  delete rdo->buf;
  rdo->buf = NULL;

}

static void rdc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call )
{

relatedDisplayClass *rdo = (relatedDisplayClass *) client;

  rdo->ef.popdown();
  rdo->operationCancel();

  delete rdo->buf;
  rdo->buf = NULL;

}

static void rdc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call )
{

relatedDisplayClass *rdo = (relatedDisplayClass *) client;

  delete rdo->buf;
  rdo->buf = NULL;

  rdo->ef.popdown();
  rdo->operationCancel();
  rdo->erase();
  rdo->deleteRequest = 1;
  rdo->drawAll();

}

relatedDisplayClass::relatedDisplayClass ( void ) {

int i;

  name = new char[strlen("relatedDisplayClass")+1];
  strcpy( name, "relatedDisplayClass" );
  checkBaseClassVersion( activeGraphicClass::MAJOR_VERSION, name );

  activeMode = 0;
  invisible = 0;
  ofsX = 0;
  ofsY = 0;
  noEdit = 0;
  useFocus = 0;
  button3Popup = 0;
  icon = 0;
  swapButtons = 0;

  for ( i=0; i<maxDsps; i++ ) {
    closeAction[i] = 0;
    setPostion[i] = 0;
    allowDups[i] = 0;
    cascade[i] = 0;
    propagateMacros[i] = 1;
    replaceSymbols[i] = 0;
  }

  numDsps = 0;

  fontList = NULL;
  aw = NULL;
  buf = NULL;

  helpItem = -1;

  unconnectedTimer = 0;

  connection.setMaxPvs( NUMPVS + 1 );

  setBlinkFunction( (void *) doBlink );

}

relatedDisplayClass::~relatedDisplayClass ( void ) {

int okToClose;
activeWindowListPtr cur;

/*   fprintf( stderr, "In relatedDisplayClass::~relatedDisplayClass\n" ); */

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

  if ( unconnectedTimer ) {
    XtRemoveTimeOut( unconnectedTimer );
    unconnectedTimer = 0;
  }

  updateBlink( 0 );

}

// copy constructor
relatedDisplayClass::relatedDisplayClass
 ( const relatedDisplayClass *source ) {

int i;
activeGraphicClass *rdo = (activeGraphicClass *) this;

  rdo->clone( (activeGraphicClass *) source );

  name = new char[strlen("relatedDisplayClass")+1];
  strcpy( name, "relatedDisplayClass" );

  strncpy( fontTag, source->fontTag, 63 );
  fs = actWin->fi->getXFontStruct( fontTag );
  actWin->fi->getTextFontList( fontTag, &fontList );

  fontAscent = source->fontAscent;
  fontDescent = source->fontDescent;
  fontHeight = source->fontHeight;

  topShadowColor = source->topShadowColor;
  botShadowColor = source->botShadowColor;

  fgColor.copy(source->fgColor);
  bgColor.copy(source->bgColor);

  invisible = source->invisible;
  ofsX = source->ofsX;
  ofsY = source->ofsY;
  noEdit = source->noEdit;
  useFocus = source->useFocus;
  button3Popup = source->button3Popup;
  icon = source->icon;
  swapButtons = source->swapButtons;

  for ( i=0; i<maxDsps; i++ ) {
    closeAction[i] = source->closeAction[i];
    setPostion[i] = source->setPostion[i];
    allowDups[i] = source->allowDups[i];
    cascade[i] = source->cascade[i];
    propagateMacros[i] = source->propagateMacros[i];
    displayFileName[i].copy( source->displayFileName[i] );
    label[i].copy( source->label[i] );
    symbolsExpStr[i].copy( source->symbolsExpStr[i] );
    replaceSymbols[i] = source->replaceSymbols[i];
  }

  numDsps = source->numDsps;

  buttonLabel.copy( source->buttonLabel );

  activeMode = 0;

  for ( i=0; i<NUMPVS; i++ ) {
    destPvExpString[i].copy( source->destPvExpString[i] );
    sourceExpString[i].copy( source->sourceExpString[i] );
  }

  colorPvExpString.copy( source->colorPvExpString );

  helpCommandExpString.copy( source->helpCommandExpString );

  aw = NULL;
  buf = NULL;

  helpItem = -1;

  unconnectedTimer = 0;

  connection.setMaxPvs( NUMPVS + 1 );

  setBlinkFunction( (void *) doBlink );

  doAccSubs( buttonLabel );
  doAccSubs( helpCommandExpString );
  doAccSubs( colorPvExpString );
  for ( i=0; i<NUMPVS; i++ ) {
    doAccSubs( destPvExpString[i] );
  }
  for ( i=0; i<maxDsps; i++ ) {
    doAccSubs( displayFileName[i] );
    doAccSubs( symbolsExpStr[i] );
    doAccSubs( label[i] );
  }

}

void relatedDisplayClass::setHelpItem ( void ) {

char *ctx, *tk, *err, buf[255+1];
int item;

  helpItem = -1;

  if ( !blank( helpCommandExpString.getExpanded() ) ) {

    strncpy( buf, helpCommandExpString.getExpanded(), 255 );
    buf[255] = 0;

    ctx = NULL;
    tk = strtok_r( buf, " \t", &ctx );

    if ( tk ) {

      if ( strcmp( tk, "item" ) == 0 ) {

        tk = strtok_r( NULL, " \t", &ctx );

        if ( tk ) {

          errno = 0;
          item = strtol( tk, &err, 0 );
          if ( !errno ) helpItem = item;

	}

      }

    }

  }

}

int relatedDisplayClass::createInteractive (
  activeWindowClass *aw_obj,
  int _x,
  int _y,
  int _w,
  int _h ) {

char *ptr;

  actWin = (activeWindowClass *) aw_obj;
  x = _x;
  y = _y;
  w = _w;
  h = _h;

  ptr = getenv( "EDMRDDHS" );
  if ( ptr ) {
    helpCommandExpString.setRaw( ptr );
  }

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

int relatedDisplayClass::save (
  FILE *f )
{

int stat, numPvs, major, minor, release;

tagClass tag;

int zero = 0;
int one = 1;
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

  major = RDC_MAJOR_VERSION;
  minor = RDC_MINOR_VERSION;
  release = RDC_RELEASE;

  numPvs = NUMPVS;

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
  tag.loadBoolW( "useFocus", &useFocus, &zero );
  tag.loadBoolW( "button3Popup", &button3Popup, &zero );
  tag.loadBoolW( "invisible", &invisible, &zero );
  tag.loadW( "buttonLabel", &buttonLabel, emptyStr );
  tag.loadW( "numPvs", &numPvs );
  tag.loadW( "pv", destPvExpString, NUMPVS, emptyStr );
  tag.loadW( "value", sourceExpString, NUMPVS, emptyStr );
  tag.loadW( "numDsps", &numDsps );
  tag.loadW( "displayFileName", displayFileName, numDsps, emptyStr );
  tag.loadW( "menuLabel", label, numDsps, emptyStr );
  tag.loadW( "closeAction", closeAction, numDsps, &zero );
  tag.loadW( "setPosition", 3, setPosEnumStr, setPosEnum, setPostion, 
   numDsps, &setPosOriginal );
  tag.loadW( "allowDups", allowDups, numDsps, &zero );
  //tag.loadW( "cascade", cascade, numDsps, &zero );
  tag.loadW( "symbols", symbolsExpStr, numDsps, emptyStr );
  tag.loadW( "replaceSymbols", replaceSymbols, numDsps, &zero );
  tag.loadW( "propagateMacros", propagateMacros, numDsps, &one );
  tag.loadW( "closeDisplay", closeAction, numDsps, &zero );
  tag.loadW( "colorPv", &colorPvExpString, emptyStr );
  tag.loadBoolW( "icon", &icon, &zero );
  tag.loadBoolW( "swapButtons", &swapButtons, &zero );
  tag.loadW( "helpCommand", &helpCommandExpString, emptyStr );
  tag.loadW( unknownTags );
  tag.loadW( "endObjectProperties" );
  tag.loadW( "" );

  stat = tag.writeTags( f );

  return stat;

}

int relatedDisplayClass::old_save (
  FILE *f )
{

int i, index;

  fprintf( f, "%-d %-d %-d\n", RDC_MAJOR_VERSION, RDC_MINOR_VERSION,
   RDC_RELEASE );

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

  fprintf( f, "%-d\n", invisible );

  fprintf( f, "%-d\n", closeAction[0] );

  fprintf( f, "%-d\n", setPostion[0] );

  fprintf( f, "%-d\n", NUMPVS );

  for ( i=0; i<NUMPVS; i++ ) {
    if ( destPvExpString[i].getRaw() ) {
      writeStringToFile( f, destPvExpString[i].getRaw() );
    }
    else {
      writeStringToFile( f, "" );
    }
    if ( sourceExpString[i].getRaw() ) {
      writeStringToFile( f, sourceExpString[i].getRaw() );
    }
    else {
      writeStringToFile( f, "" );
    }
  }

  fprintf( f, "%-d\n", allowDups[0] );

  fprintf( f, "%-d\n", cascade[0] );

  if ( symbolsExpStr[0].getRaw() ) {
    writeStringToFile( f, symbolsExpStr[0].getRaw() );
  }
  else {
    writeStringToFile( f, "" );
  }

  fprintf( f, "%-d\n", replaceSymbols[0] );

  fprintf( f, "%-d\n", propagateMacros[0] );

  fprintf( f, "%-d\n", useFocus );

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

    fprintf( f, "%-d\n", closeAction[i] );

    fprintf( f, "%-d\n", setPostion[i] );

    fprintf( f, "%-d\n", allowDups[i] );

    fprintf( f, "%-d\n", cascade[i] );

    if ( symbolsExpStr[i].getRaw() ) {
      writeStringToFile( f, symbolsExpStr[i].getRaw() );
    }
    else {
      writeStringToFile( f, "" );
    }

    fprintf( f, "%-d\n", replaceSymbols[i] );

    fprintf( f, "%-d\n", propagateMacros[i] );

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

  fprintf( f, "%-d\n", button3Popup );

  return 1;

}

int relatedDisplayClass::createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int i, n1, n2, numPvs, stat, major, minor, release;

tagClass tag;

int zero = 0;
int one = 1;
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

  major = RDC_MAJOR_VERSION;
  minor = RDC_MINOR_VERSION;
  release = RDC_RELEASE;

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
  tag.loadR( "useFocus", &useFocus, &zero );
  tag.loadR( "button3Popup", &button3Popup, &zero );
  tag.loadR( "invisible", &invisible, &zero );
  tag.loadR( "buttonLabel", &buttonLabel, emptyStr );
  tag.loadR( "numPvs", &numPvs, &zero );
  tag.loadR( "pv", NUMPVS, destPvExpString, &n1, emptyStr );
  tag.loadR( "value", NUMPVS, sourceExpString, &n1, emptyStr );
  tag.loadR( "numDsps", &numDsps, &zero );
  tag.loadR( "displayFileName", maxDsps, displayFileName, &n2, emptyStr );
  tag.loadR( "menuLabel", maxDsps, label, &n2, emptyStr );
  tag.loadR( "closeAction", maxDsps, closeAction, &n2, &zero );
  tag.loadR( "setPosition", 3, setPosEnumStr, setPosEnum, maxDsps, setPostion, 
   &n2, &setPosOriginal );
  tag.loadR( "allowDups", maxDsps, allowDups, &n2, &zero );
  tag.loadR( "cascade", maxDsps, cascade, &n2, &zero );
  tag.loadR( "symbols", maxDsps, symbolsExpStr, &n2, emptyStr );
  tag.loadR( "replaceSymbols", maxDsps, replaceSymbols, &n2, &zero );
  tag.loadR( "propagateMacros", maxDsps, propagateMacros, &n2, &one );
  tag.loadR( "closeDisplay", maxDsps, closeAction, &n2, &zero );
  tag.loadR( "colorPv", &colorPvExpString, emptyStr );
  tag.loadR( "icon", &icon, &zero );
  tag.loadR( "swapButtons", &swapButtons, &zero );
  tag.loadW( "helpCommand", &helpCommandExpString, emptyStr );
  tag.loadR( "endObjectProperties" );

  stat = tag.readTags( f, "endObjectProperties" );

  if ( !( stat & 1 ) ) {
    actWin->appCtx->postMessage( tag.errMsg() );
  }

  if ( major > RDC_MAJOR_VERSION ) {
    postIncompatable();
    return 0;
  }

  if ( major < 4 ) {
    postIncompatable();
    return 0;
  }

  this->initSelectBox(); // call after getting x,y,w,h

  for ( i=numPvs; i<NUMPVS; i++ ) {
    destPvExpString[i].setRaw( "" );
    sourceExpString[i].setRaw( "" );
  }

  actWin->fi->loadFontTag( fontTag );
  actWin->drawGc.setFontTag( fontTag, actWin->fi );

  fs = actWin->fi->getXFontStruct( fontTag );
  actWin->fi->getTextFontList( fontTag, &fontList );

  updateDimensions();

  return stat;

}

int relatedDisplayClass::old_createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int i, numPvs, r, g, b, index, more, md;
int major, minor, release;
unsigned int pixel;
char oneName[255+1];
char onePvName[PV_Factory::MAX_PV_NAME+1];

  this->actWin = _actWin;

  fscanf( f, "%d %d %d\n", &major, &minor, &release ); actWin->incLine();

  if ( major > RDC_MAJOR_VERSION ) {
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

  readStringFromFile( oneName, 127+1, f ); actWin->incLine();
  label[0].setRaw( oneName );

  readStringFromFile( fontTag, 63+1, f ); actWin->incLine();

  if ( ( major > 1 ) || ( minor > 2 ) ) {
    fscanf( f, "%d\n", &invisible ); actWin->incLine();
    fscanf( f, "%d\n", &closeAction[0] ); actWin->incLine();
  }
  else {
    invisible = 0;
    closeAction[0] = 0;
  }

  if ( ( major > 1 ) || ( minor > 3 ) ) {
    fscanf( f, "%d\n", &setPostion[0] ); actWin->incLine();
  }
  else {
    setPostion[0] = 0;
  }

  if ( ( major > 1 ) || ( minor > 4 ) ) {
    fscanf( f, "%d\n", &numPvs ); actWin->incLine();
    for ( i=0; i<numPvs; i++ ) {
      if ( i >= NUMPVS ) i = NUMPVS - 1;
      readStringFromFile( onePvName, PV_Factory::MAX_PV_NAME+1, f );
       actWin->incLine();
      destPvExpString[i].setRaw( onePvName );
      readStringFromFile( onePvName, 39+1, f ); actWin->incLine();
      sourceExpString[i].setRaw( onePvName );
    }
    for ( i=numPvs; i<NUMPVS; i++ ) {
      destPvExpString[i].setRaw( "" );
      sourceExpString[i].setRaw( "" );
    }
  }
  else {
    for ( i=numPvs; i<NUMPVS; i++ ) {
      destPvExpString[i].setRaw( "" );
      sourceExpString[i].setRaw( "" );
    }
  }

  if ( ( major > 1 ) || ( minor > 6 ) ) {
    fscanf( f, "%d\n", &allowDups[0] ); actWin->incLine();
  }
  else {
    allowDups[0] = 0;
  }

  if ( ( major > 1 ) || ( minor > 7 ) ) {
    fscanf( f, "%d\n", &cascade[0] ); actWin->incLine();
  }
  else {
    cascade[0] = 0;
  }

  if ( ( major > 1 ) || ( minor > 8 ) ) {
    readStringFromFile( oneName, 255+1, f ); actWin->incLine();
    symbolsExpStr[0].setRaw( oneName );
    fscanf( f, "%d\n", &replaceSymbols[0] ); actWin->incLine();
  }
  else {
    symbolsExpStr[0].setRaw( "" );
    replaceSymbols[0]  = 0;
  }

  if ( ( major > 1 ) || ( minor > 9 ) ) {
    fscanf( f, "%d\n", &propagateMacros[0] ); actWin->incLine();
  }
  else {
    propagateMacros[0] = 0;
  }

  if ( ( major > 1 ) || ( minor > 10 ) ) {
    fscanf( f, "%d\n", &useFocus ); actWin->incLine();
  }
  else {
    useFocus = 0;
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

        readStringFromFile( oneName, 127+1, f ); actWin->incLine();
        label[i].setRaw( oneName );

        fscanf( f, "%d\n", &closeAction[i] );

        fscanf( f, "%d\n", &setPostion[i] );

        fscanf( f, "%d\n", &allowDups[i] );

        fscanf( f, "%d\n", &cascade[i] );

        readStringFromFile( oneName, 255+1, f ); actWin->incLine();
        symbolsExpStr[i].setRaw( oneName );

        fscanf( f, "%d\n", &replaceSymbols[i] );

        fscanf( f, "%d\n", &propagateMacros[i] );

      }

      for ( i=numDsps; i<maxDsps; i++ ) {
        closeAction[i] = 0;
        setPostion[i] = 0;
        allowDups[i] = 0;
        cascade[i] = 0;
        propagateMacros[i] = 1;
        replaceSymbols[i] = 0;
        label[i].setRaw( "" );
        symbolsExpStr[i].setRaw( "" );
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

      readStringFromFile( oneName, 127+1, f ); actWin->incLine();
      label[i].setRaw( oneName );

      fscanf( f, "%d\n", &closeAction[i] );

      fscanf( f, "%d\n", &setPostion[i] );

      fscanf( f, "%d\n", &allowDups[i] );

      fscanf( f, "%d\n", &cascade[i] );

      readStringFromFile( oneName, 255+1, f ); actWin->incLine();
      symbolsExpStr[i].setRaw( oneName );

      fscanf( f, "%d\n", &replaceSymbols[i] );

      fscanf( f, "%d\n", &propagateMacros[i] );

    }

    for ( i=numDsps; i<maxDsps; i++ ) {
      closeAction[i] = 0;
      setPostion[i] = 0;
      allowDups[i] = 0;
      cascade[i] = 0;
      propagateMacros[i] = 1;
      replaceSymbols[i] = 0;
      label[i].setRaw( "" );
      symbolsExpStr[i].setRaw( "" );
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

  if ( ( major > 2 ) || ( ( major == 2 ) && ( minor > 6 ) ) ) {
    fscanf( f, "%d\n", &button3Popup ); actWin->incLine();
  }
  else {
    button3Popup = 0;
  }

  if ( useFocus ) {
    button3Popup = 0;
  }

  icon = 0;
  swapButtons = 0;

  actWin->fi->loadFontTag( fontTag );
  actWin->drawGc.setFontTag( fontTag, actWin->fi );

  fs = actWin->fi->getXFontStruct( fontTag );
  actWin->fi->getTextFontList( fontTag, &fontList );

  updateDimensions();

  return 1;

}

int relatedDisplayClass::importFromXchFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin ) {

int fgR, fgG, fgB, bgR, bgG, bgB, more, index;
unsigned int pixel;
char *tk, *gotData, *context, buf[255+1];

  fgR = 0xffff;
  fgG = 0xffff;
  fgB = 0xffff;

  bgR = 0xffff;
  bgG = 0xffff;
  bgB = 0xffff;

  this->actWin = _actWin;

  strcpy( fontTag, actWin->defaultBtnFontTag );
  actWin->fi->loadFontTag( fontTag );

  topShadowColor = actWin->defaultTopShadowColor;
  botShadowColor = actWin->defaultBotShadowColor;

  fgColor.setColorIndex( actWin->defaultTextFgColor, actWin->ci );
  bgColor.setColorIndex( actWin->defaultBgColor, actWin->ci );

  // continue until tag is <eod>

  do {

    gotData = getNextDataString( buf, 255, f );
    if ( !gotData ) {
      actWin->appCtx->postMessage( relatedDisplayClass_str1 );
      return 0;
    }

    context = NULL;

    tk = strtok_r( buf, " \t\n", &context );
    if ( !tk ) {
      actWin->appCtx->postMessage( relatedDisplayClass_str1 );
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
          actWin->appCtx->postMessage( relatedDisplayClass_str1 );
          return 0;
        }

        x = atol( tk );

      }
      else if ( strcmp( tk, "y" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( relatedDisplayClass_str1 );
          return 0;
        }

        y = atol( tk );

      }
      else if ( strcmp( tk, "w" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( relatedDisplayClass_str1 );
          return 0;
        }

        w = atol( tk );

      }
      else if ( strcmp( tk, "h" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( relatedDisplayClass_str1 );
          return 0;
        }

        h = atol( tk );

      }
            
      else if ( strcmp( tk, "fgred" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( relatedDisplayClass_str1 );
          return 0;
        }

        fgR = atol( tk );

      }
            
      else if ( strcmp( tk, "fggreen" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( relatedDisplayClass_str1 );
          return 0;
        }

        fgG = atol( tk );

      }
            
      else if ( strcmp( tk, "fgblue" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( relatedDisplayClass_str1 );
          return 0;
        }

        fgB = atol( tk );

      }
            
      else if ( strcmp( tk, "bgred" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( relatedDisplayClass_str1 );
          return 0;
        }

        bgR = atol( tk );

      }
            
      else if ( strcmp( tk, "bggreen" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( relatedDisplayClass_str1 );
          return 0;
        }

        bgG = atol( tk );

      }
            
      else if ( strcmp( tk, "bgblue" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( relatedDisplayClass_str1 );
          return 0;
        }

        bgB = atol( tk );

      }
            
      else if ( strcmp( tk, "closecurrent" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( relatedDisplayClass_str1 );
          return 0;
        }

        closeAction[0] = atol( tk );

      }
            
      else if ( strcmp( tk, "invisible" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( relatedDisplayClass_str1 );
          return 0;
        }

        invisible = atol( tk );

      }
            
      else if ( strcmp( tk, "font" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( relatedDisplayClass_str1 );
          return 0;
        }

        strncpy( fontTag, tk, 63 );

      }

      else if ( strcmp( tk, "displayname" ) == 0 ) {

        tk = strtok_r( NULL, "\"", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( relatedDisplayClass_str1 );
          return 0;
        }

        displayFileName[0].setRaw( tk );

      }

      else if ( strcmp( tk, "label" ) == 0 ) {

        tk = strtok_r( NULL, "\"", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( relatedDisplayClass_str1 );
          return 0;
        }

        buttonLabel.setRaw( tk );
        label[0].setRaw( tk );

      }

    }

  } while ( more );

  this->initSelectBox(); // call after getting x,y,w,h

  actWin->ci->setRGB( fgR, fgG, fgB, &pixel );
  index = actWin->ci->pixIndex( pixel );
  fgColor.setColorIndex( index, actWin->ci );

  actWin->ci->setRGB( bgR, bgG, bgB, &pixel );
  index = actWin->ci->pixIndex( pixel );
  bgColor.setColorIndex( index, actWin->ci );

  actWin->fi->loadFontTag( fontTag );
  actWin->drawGc.setFontTag( fontTag, actWin->fi );

  fs = actWin->fi->getXFontStruct( fontTag );
  actWin->fi->getTextFontList( fontTag, &fontList );

  updateDimensions();

  return 1;

}

int relatedDisplayClass::createSpecial (
  char *fname,
  activeWindowClass *_actWin )
{

int i;

  x = -100;
  y = 0;
  w = 5;
  h = 5;

  this->actWin = _actWin;
  strcpy( fontTag, "" );
  ofsX = ofsY = useFocus = button3Popup = 0;
  noEdit = invisible = numDsps = 1;
  setPostion[0] = 0;
  allowDups[0] = 0;
  cascade[0] = 0;
  replaceSymbols[0] = 0;
  propagateMacros[0] = 1;
  closeAction[0] = 0;
  icon = 0;
  swapButtons = 0;
  displayFileName[0].setRaw( fname );
  
  this->initSelectBox(); // call after getting x,y,w,h

  for ( i=0; i<NUMPVS; i++ ) {
    destPvExpString[i].setRaw( "" );
    sourceExpString[i].setRaw( "" );
  }

  return 1;

}

void relatedDisplayClass::sendMsg (
  char *param )
{

  if ( param ) {
    //fprintf( stderr, "  msg = [%s]\n", param );
    if ( strcmp( param, "popup" ) == 0 ) {
      noEdit = 0;
      popupDisplay( 0 );
    }
    else if ( strcmp( param, "popupNoEdit" ) == 0 ) {
      noEdit = 1;
      popupDisplay( 0 );
    }
  }

}

int relatedDisplayClass::genericEdit ( void ) {

int i;
char title[32], *ptr;

  if ( !buf ) {
    buf = new bufType;
  }

  ptr = actWin->obj.getNameFromClass( "relatedDisplayClass" );
  if ( ptr )
    strncpy( title, ptr, 31 );
  else
    strncpy( title, relatedDisplayClass_str17, 31 );

  Strncat( title, relatedDisplayClass_str3, 31 );

  buf->bufX = x;
  buf->bufY = y;
  buf->bufW = w;
  buf->bufH = h;

  strncpy( buf->bufFontTag, fontTag, 63 );

  buf->bufTopShadowColor = topShadowColor;
  buf->bufBotShadowColor = botShadowColor;

  buf->bufFgColor = fgColor.pixelIndex();

  buf->bufBgColor = bgColor.pixelIndex();

  buf->bufInvisible = invisible;

  buf->bufOfsX = ofsX;

  buf->bufOfsY = ofsY;

  buf->bufNoEdit = noEdit;

  buf->bufUseFocus = useFocus;

  for ( i=0; i<maxDsps; i++ ) {

    if ( displayFileName[i].getRaw() )
      strncpy( buf->bufDisplayFileName[i], displayFileName[i].getRaw(), 127 );
    else
      strncpy( buf->bufDisplayFileName[i], "", 127 );

    if ( label[i].getRaw() )
      strncpy( buf->bufLabel[i], label[i].getRaw(), 127 );
    else
      strncpy( buf->bufLabel[i], "", 127 );

    buf->bufCloseAction[i] = closeAction[i];

    buf->bufSetPostion[i] = setPostion[i];

    buf->bufAllowDups[i] = allowDups[i];

    buf->bufCascade[i] = cascade[i];

    buf->bufPropagateMacros[i] = propagateMacros[i];

    if ( symbolsExpStr[i].getRaw() ) {
      strncpy( buf->bufSymbols[i], symbolsExpStr[i].getRaw(), maxSymbolLen );
      buf->bufSymbols[i][maxSymbolLen] = 0;
    }
    else {
      strncpy( buf->bufSymbols[i], "", maxSymbolLen );
    }

    buf->bufReplaceSymbols[i] = replaceSymbols[i];

  }

  for ( i=0; i<NUMPVS; i++ ) {
    if ( destPvExpString[i].getRaw() ) {
      strncpy( buf->bufDestPvName[i], destPvExpString[i].getRaw(),
       PV_Factory::MAX_PV_NAME );
      buf->bufDestPvName[i][PV_Factory::MAX_PV_NAME] = 0;
    }
    else {
      strcpy( buf->bufDestPvName[i], "" );
    }
    if ( sourceExpString[i].getRaw() ) {
      strncpy( buf->bufSource[i], sourceExpString[i].getRaw(), 39 );
      buf->bufSource[i][39] = 0;
    }
    else {
      strcpy( buf->bufSource[i], "" );
    }
  }

  if ( colorPvExpString.getRaw() ) {
    strncpy( buf->bufColorPvName, colorPvExpString.getRaw(),
     PV_Factory::MAX_PV_NAME );
    buf->bufColorPvName[PV_Factory::MAX_PV_NAME] = 0;
  }
  else {
    strcpy( buf->bufColorPvName, "" );
  }

  if ( buttonLabel.getRaw() ) {
    strncpy( buf->bufButtonLabel, buttonLabel.getRaw(), 127 );
    buf->bufButtonLabel[127] = 0;
  }
  else {
    strncpy( buf->bufButtonLabel, "", 127 );
  }

  buf->bufButton3Popup = button3Popup;

  buf->bufIcon = icon;

  buf->bufSwapButtons = swapButtons;

  if ( helpCommandExpString.getRaw() )
    strncpy( buf->bufHelpCommand, helpCommandExpString.getRaw(), 255 );
  else
    strncpy( buf->bufHelpCommand, "", 255 );

  ef.create( actWin->top, actWin->appCtx->ci.getColorMap(),
   &actWin->appCtx->entryFormX,
   &actWin->appCtx->entryFormY, &actWin->appCtx->entryFormW,
   &actWin->appCtx->entryFormH, &actWin->appCtx->largestH,
   title, NULL, NULL, NULL );

  i = 0;

  ef.addTextField( relatedDisplayClass_str4, 35, &buf->bufX );
  ef.addTextField( relatedDisplayClass_str5, 35, &buf->bufY );
  ef.addTextField( relatedDisplayClass_str6, 35, &buf->bufW );
  ef.addTextField( relatedDisplayClass_str7, 35, &buf->bufH );

  ef.addTextField( relatedDisplayClass_str37, 35, buf->bufDisplayFileName[0],
   127 );
  fileEntry[i] = ef.getCurItem();
  ef.addTextField( relatedDisplayClass_str36, 35, buf->bufLabel[0], 127 );
  labelEntry[i] = ef.getCurItem();
  ef.addTextField( relatedDisplayClass_str26, 35, buf->bufSymbols[0],
   maxSymbolLen );
  macrosEntry[i] = ef.getCurItem();
  ef.addOption( relatedDisplayClass_str23, relatedDisplayClass_str24,
   &buf->bufReplaceSymbols[0] );
  modeEntry[i] = ef.getCurItem();
  ef.addToggle( relatedDisplayClass_str25, &buf->bufPropagateMacros[0] );
  propagateEntry[i] = ef.getCurItem();
  ef.addOption( relatedDisplayClass_str30, relatedDisplayClass_str31,
   &buf->bufSetPostion[0] );
  positionEntry[i] = ef.getCurItem();
  ef.addTextField( relatedDisplayClass_str32, 35, &buf->bufOfsX );
  xOfsEntry[i] = ef.getCurItem();
  ef.addTextField( relatedDisplayClass_str33, 35, &buf->bufOfsY );
  yOfsEntry[i] = ef.getCurItem();
  ef.addToggle( relatedDisplayClass_str20, &buf->bufCloseAction[0] );
  closeCurEntry[i] = ef.getCurItem();
  ef.addToggle( relatedDisplayClass_str21, &buf->bufAllowDups[0] );
  dupsAllowedEntry[i] = ef.getCurItem();

  fileEntry[i]->addDependency( labelEntry[i] );
  fileEntry[i]->addDependency( macrosEntry[i] );
  fileEntry[i]->addDependency( modeEntry[i] );
  fileEntry[i]->addDependency( propagateEntry[i] );
  fileEntry[i]->addDependency( positionEntry[i] );
  fileEntry[i]->addDependency( xOfsEntry[i] );
  fileEntry[i]->addDependency( yOfsEntry[i] );
  fileEntry[i]->addDependency( closeCurEntry[i] );
  fileEntry[i]->addDependency( dupsAllowedEntry[i] );
  fileEntry[i]->addDependencyCallbacks();

  ef.addEmbeddedEf( relatedDisplayClass_str14, "...", &ef1 );

  ef1->create( actWin->top, actWin->appCtx->ci.getColorMap(),
   &actWin->appCtx->entryFormX,
   &actWin->appCtx->entryFormY, &actWin->appCtx->entryFormW,
   &actWin->appCtx->entryFormH, &actWin->appCtx->largestH,
   title, NULL, NULL, NULL );

  for ( i=1; i<maxDsps; i++ ) {

    ef1->beginLeftSubForm();
    ef1->addTextField( relatedDisplayClass_str39, 35, buf->bufDisplayFileName[i], 127 );
    fileEntry[i] = ef1->getCurItem();
    ef1->addLabel( relatedDisplayClass_str38 );
    ef1->addTextField( "", 35, buf->bufLabel[i], 127 );
    labelEntry[i] = ef1->getCurItem();
    ef1->addLabel( relatedDisplayClass_str40 );
    ef1->addTextField( "", 35, buf->bufSymbols[i], maxSymbolLen );
    macrosEntry[i] = ef1->getCurItem();
    ef1->endSubForm();

    ef1->beginLeftSubForm();
    ef1->addLabel( relatedDisplayClass_str41 );
    ef1->addOption( "", relatedDisplayClass_str24,
     &buf->bufReplaceSymbols[i] );
    modeEntry[i] = ef1->getCurItem();
    ef1->addLabel( " " );
    ef1->addToggle( " ", &buf->bufPropagateMacros[i] );
    propagateEntry[i] = ef1->getCurItem();
    ef1->addLabel( relatedDisplayClass_str42 );
    ef1->addLabel( relatedDisplayClass_str30 );
    ef1->addOption( " ", relatedDisplayClass_str31, &buf->bufSetPostion[i] );
    positionEntry[i] = ef1->getCurItem();
    ef1->addLabel( " " );
    ef1->addToggle( " ", &buf->bufCloseAction[i] );
    closeCurEntry[i] = ef1->getCurItem();
    ef1->addLabel( relatedDisplayClass_str35 );
    ef1->addToggle( " ", &buf->bufAllowDups[i] );
    dupsAllowedEntry[i] = ef1->getCurItem();
    ef1->addLabel( relatedDisplayClass_str43 );
    //ef1->addToggle( " ", &buf->bufCascade[i] );
    //ef1->addLabel( relatedDisplayClass_str22 );
    ef1->endSubForm();

    fileEntry[i]->addDependency( labelEntry[i] );
    fileEntry[i]->addDependency( macrosEntry[i] );
    fileEntry[i]->addDependency( modeEntry[i] );
    fileEntry[i]->addDependency( propagateEntry[i] );
    fileEntry[i]->addDependency( positionEntry[i] );
    fileEntry[i]->addDependency( closeCurEntry[i] );
    fileEntry[i]->addDependency( dupsAllowedEntry[i] );
    fileEntry[i]->addDependencyCallbacks();

  }

  //ef1->finished( rdc_edit_ok1, rdc_edit_apply1, rdc_edit_cancel1, this );
  ef1->finished( rdc_edit_ok1, this );

  ef.addTextField( relatedDisplayClass_str49, 35, buf->bufHelpCommand, 255 );

  ef.addTextField( relatedDisplayClass_str13, 35, buf->bufButtonLabel, 127 );

  ef.addToggle( relatedDisplayClass_str17, &buf->bufUseFocus );
  ef.addToggle( relatedDisplayClass_str19, &buf->bufInvisible );
  ef.addToggle( relatedDisplayClass_str29, &buf->bufNoEdit );
  ef.addToggle( relatedDisplayClass_str34, &buf->bufButton3Popup );
  ef.addToggle( relatedDisplayClass_str47, &buf->bufIcon );
  ef.addToggle( relatedDisplayClass_str48, &buf->bufSwapButtons );

  ef.addTextField( "Color PV", 35, buf->bufColorPvName,
   PV_Factory::MAX_PV_NAME );

  for ( i=0; i<NUMPVS; i++ ) {
    ef.addTextField( relatedDisplayClass_str15, 35, buf->bufDestPvName[i],
     PV_Factory::MAX_PV_NAME );
    pvEntry[i] = ef.getCurItem();
    ef.addTextField( relatedDisplayClass_str16, 35, buf->bufSource[i], 39 );
    valEntry[i] = ef.getCurItem();
    pvEntry[i]->addDependency( valEntry[i] );
    pvEntry[i]->addDependencyCallbacks();
  }

  ef.addColorButton( relatedDisplayClass_str8, actWin->ci, &fgCb, &buf->bufFgColor );
  ef.addColorButton( relatedDisplayClass_str9, actWin->ci, &bgCb, &buf->bufBgColor );
  ef.addColorButton( relatedDisplayClass_str10, actWin->ci, &topShadowCb,
   &buf->bufTopShadowColor );
  ef.addColorButton( relatedDisplayClass_str11, actWin->ci, &botShadowCb,
   &buf->bufBotShadowColor );
  ef.addFontMenu( relatedDisplayClass_str12, actWin->fi, &fm, fontTag );

  XtUnmanageChild( fm.alignWidget() ); // no alignment info

  return 1;

}

int relatedDisplayClass::editCreate ( void ) {

  this->genericEdit();
  ef.finished( rdc_edit_ok, rdc_edit_apply, rdc_edit_cancel_delete, this );
  actWin->currentEf = NULL;
  ef.popup();

  return 1;

}

int relatedDisplayClass::edit ( void ) {

  this->genericEdit();
  ef.finished( rdc_edit_ok, rdc_edit_apply, rdc_edit_cancel, this );
  actWin->currentEf = &ef;
  ef.popup();

  return 1;

}

int relatedDisplayClass::erase ( void ) {

  if ( deleteRequest ) return 1;

  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  return 1;

}

int relatedDisplayClass::eraseActive ( void ) {

  if ( !enabled || !activeMode || !init || invisible ) return 1;

  XDrawRectangle( actWin->d, drawable(actWin->executeWidget),
   actWin->executeGc.eraseGC(), x, y, w, h );

  XFillRectangle( actWin->d, drawable(actWin->executeWidget),
   actWin->executeGc.eraseGC(), x, y, w, h );

  return 1;

}

int relatedDisplayClass::draw ( void ) {

char *ptr;
int tX, tY, cx, cy, min, ofs, size, strW;
XRectangle xR = { x, y, w, h };
int blink = 0;

  if ( deleteRequest ) return 1;

  actWin->drawGc.saveFg();

  actWin->drawGc.setFG( bgColor.pixelIndex(), &blink );

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

    if ( buttonLabel.getRaw() && !blank( buttonLabel.getRaw() ) ) {

      ptr = buttonLabel.getRaw();

      if ( !icon ) {

	// no icon

        tX = x + w/2;
        tY = y + h/2 - fontAscent/2;

        actWin->drawGc.setFG( fgColor.pixelIndex(), &blink );
        actWin->drawGc.setFontTag( fontTag, actWin->fi );

        drawText( actWin->drawWidget, &actWin->drawGc, fs, tX, tY,
         XmALIGNMENT_CENTER, ptr );

      }
      else {

	// draw icon and text

        strW = XTextWidth( fs, ptr, strlen(ptr) ) +
         (int) ( 1.2*fontAscent+0.5 );

        tX = x + (int) ( w/2+0.5 ) - (int) ( strW/2+0.5 ) +
         (int) ( 1.2*fontAscent+0.5 );
        tY = y + (int) ( h/2+0.5 ) - (int) ( fontAscent/2+0.5 );

        ofs = (int) ( fontAscent*0.1+0.5 );
        size = (int) ( fontAscent-2.0*ofs+0.5 );
        cx = tX - (int) ( 1.5*fontAscent+0.5 );
        cy = tY + ofs;

        actWin->drawGc.setFG( fgColor.pixelIndex(), &blink );

        XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
         actWin->drawGc.normGC(), cx+2*ofs, cy+ofs, size, size );

        actWin->drawGc.setFG( bgColor.pixelIndex(), &blink );

        XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
         actWin->drawGc.normGC(), cx, cy-ofs, size, size );

        actWin->drawGc.setFG( fgColor.pixelIndex(), &blink );

        XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
         actWin->drawGc.normGC(), cx, cy-ofs, size, size );

        actWin->drawGc.setFG( fgColor.pixelIndex(), &blink );
        actWin->drawGc.setFontTag( fontTag, actWin->fi );

        drawText( actWin->drawWidget, &actWin->drawGc, fs, tX, tY,
         XmALIGNMENT_BEGINNING, ptr );

      }

    }
    else {

      if ( icon ) {

        // draw icon on button without text

        min = (int) ( w*3/5 );
        if ( (int) ( h*3/5 ) < min ) min = (int) ( h*3/5 );
        cx = x + w/2;
        cy = y + h/2;
        ofs = (int) ( min*2/5+0.5 );
        size = (int) ( min*3/5-0.5 );

        actWin->drawGc.setFG( fgColor.pixelIndex(), &blink );

        XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
         actWin->drawGc.normGC(), cx+ofs-size, cy+ofs-size, size, size );

        actWin->drawGc.setFG( bgColor.pixelIndex(), &blink );

        XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
         actWin->drawGc.normGC(), cx-ofs, cy-ofs, size, size );

        actWin->drawGc.setFG( fgColor.pixelIndex(), &blink );

        XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
         actWin->drawGc.normGC(), cx-ofs, cy-ofs, size, size );

      }

    }

    actWin->drawGc.removeNormXClipRectangle();

  }

  actWin->drawGc.restoreFg();

  updateBlink( blink );

  return 1;

}

int relatedDisplayClass::drawActive ( void ) {

int tX, tY, cx, cy, min, ofs, size, strW;
char string[39+1];
XRectangle xR = { x, y, w, h };
int blink = 0;

  if ( !init ) {
    if ( needToDrawUnconnected ) {
      actWin->executeGc.saveFg();
      actWin->executeGc.setFG( fgColor.getDisconnectedIndex(), &blink );
      actWin->executeGc.setLineWidth( 1 );
      actWin->executeGc.setLineStyle( LineSolid );
      XDrawRectangle( actWin->d, drawable(actWin->executeWidget),
       actWin->executeGc.normGC(), x, y, w, h );
      actWin->executeGc.restoreFg();
      needToEraseUnconnected = 1;
      updateBlink( blink );
    }
  }
  else if ( needToEraseUnconnected ) {
    actWin->executeGc.setLineWidth( 1 );
    actWin->executeGc.setLineStyle( LineSolid );
    XDrawRectangle( actWin->d, drawable(actWin->executeWidget),
     actWin->executeGc.eraseGC(), x, y, w, h );
    needToEraseUnconnected = 0;
    if ( invisible ) {
      eraseActive();
      smartDrawAllActive();
    }
  }

  if ( !enabled || !activeMode || !init || invisible ) return 1;

  actWin->executeGc.saveFg();

  actWin->executeGc.setFG( bgColor.getIndex(), &blink );

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

    if ( !blank( string ) ) {

      if ( !icon ) {

	// no icon

        tX = x + w/2;
        tY = y + h/2 - fontAscent/2;

        actWin->executeGc.setFG( fgColor.pixelIndex(), &blink );
        actWin->executeGc.setFontTag( fontTag, actWin->fi );

        drawText( actWin->executeWidget, drawable(actWin->executeWidget),
         &actWin->executeGc, fs, tX, tY, XmALIGNMENT_CENTER, string );

      }
      else {

	// draw icon and text

        strW = XTextWidth( fs, string, strlen(string) ) +
         (int) ( 1.2*fontAscent+0.5 );

        tX = x + (int) ( w/2+0.5 ) - (int) ( strW/2+0.5 ) +
         (int) ( 1.2*fontAscent+0.5 );
        tY = y + (int) ( h/2+0.5 ) - (int) ( fontAscent/2+0.5 );

        ofs = (int) ( fontAscent*0.1+0.5 );
        size = (int) ( fontAscent-2.0*ofs+0.5 );
        cx = tX - (int) ( 1.5*fontAscent+0.5 );
        cy = tY + ofs;

        actWin->executeGc.setFG( fgColor.pixelIndex(), &blink );

        XDrawRectangle( actWin->d, drawable(actWin->executeWidget),
         actWin->executeGc.normGC(), cx+2*ofs, cy+ofs, size, size );

        actWin->executeGc.setFG( bgColor.pixelIndex(), &blink );

        XFillRectangle( actWin->d, drawable(actWin->executeWidget),
         actWin->executeGc.normGC(), cx, cy-ofs, size, size );

        actWin->executeGc.setFG( fgColor.pixelIndex(), &blink );

        XDrawRectangle( actWin->d, drawable(actWin->executeWidget),
         actWin->executeGc.normGC(), cx, cy-ofs, size, size );

        actWin->executeGc.setFG( fgColor.pixelIndex(), &blink );
        actWin->executeGc.setFontTag( fontTag, actWin->fi );

        drawText( actWin->executeWidget, drawable(actWin->executeWidget),
         &actWin->executeGc, fs, tX, tY, XmALIGNMENT_BEGINNING, string );

      }

    }
    else {

      if ( icon ) {

        // draw icon on button without text

        min = (int) ( w*3/5 );
        if ( (int) ( h*3/5 ) < min ) min = (int) ( h*3/5 );
        cx = x + w/2;
        cy = y + h/2;
        ofs = (int) ( min*2/5+0.5 );
        size = (int) ( min*3/5-0.5 );

        actWin->executeGc.setFG( fgColor.pixelIndex(), &blink );

        XDrawRectangle( actWin->d, drawable(actWin->executeWidget),
         actWin->executeGc.normGC(), cx+ofs-size, cy+ofs-size, size, size );

        actWin->executeGc.setFG( bgColor.pixelIndex(), &blink );

        XFillRectangle( actWin->d, drawable(actWin->executeWidget),
         actWin->executeGc.normGC(), cx-ofs, cy-ofs, size, size );

        actWin->executeGc.setFG( fgColor.pixelIndex(), &blink );

        XDrawRectangle( actWin->d, drawable(actWin->executeWidget),
         actWin->executeGc.normGC(), cx-ofs, cy-ofs, size, size );

      }

    }

    actWin->executeGc.removeNormXClipRectangle();

  }

  actWin->executeGc.restoreFg();

  updateBlink( blink );

  return 1;

}

int relatedDisplayClass::activate (
  int pass,
  void *ptr )
{

int i, ii, n;
Arg args[5];
XmString str;

#ifdef TRIUMF
#endif

  switch ( pass ) {

  case 1:

    connection.init();
    needToEraseUnconnected = 0;
    needToDrawUnconnected = 0;
    unconnectedTimer = 0;
    atLeastOneExists = 0;
    init = 0;
    active = 0;

    setHelpItem();

  case 2:

    aglPtr = ptr;
    aw = NULL;
    needClose = needConnect = needUpdate = needRefresh = 0;

    singleOpComplete = 0;

    if ( !colorPvExpString.getExpanded() ||
     blankOrComment( colorPvExpString.getExpanded() ) ) {
      colorExists = 0;
    }
    else {
      colorExists = 1;
      atLeastOneExists = 1;
      fgColor.setConnectSensitive();
      bgColor.setConnectSensitive();
    }

    for ( i=0; i<NUMPVS; i++ ) {

      opComplete[i] = 0;

      if ( !destPvExpString[i].getExpanded() ||
       blankOrComment( destPvExpString[i].getExpanded() ) ) {
        destExists[i] = 0;
      }
      else {
        destExists[i] = 1;
        atLeastOneExists = 1;
      }

    }

    activeMode = 1;

    break;

  case 3:

    if ( !singleOpComplete ) {

      if ( atLeastOneExists ) {
	init = 0;
        if ( !unconnectedTimer ) {
          unconnectedTimer = appAddTimeOut( actWin->appCtx->appContext(),
           2000, unconnectedTimeout, this );
        }
      }
      else {
	init = 1; // no pvs to connect
        active = 1;
        singleOpComplete = 1;
      }

      colorPvId = NULL;

      if ( colorExists ) {

        connection.addPv();

        colorPvId = the_PV_Factory->create( colorPvExpString.getExpanded() );
	if ( colorPvId ) {
	  colorPvId->add_conn_state_callback(
           relDsp_monitor_color_connect_state, (void *) this );
	}
	else {
          fprintf( stderr, relatedDisplayClass_str27 );
        }

      }

      singleOpComplete = 1;

    }

    for ( i=0; i<NUMPVS; i++ ) {

      if ( !opComplete[i] ) {

        initEnable();

        initialConnection[i] = 1;
        destPvId[i] = NULL;

        if ( i == 0 ) {

          n = 0;
          XtSetArg( args[n], XmNpopupEnabled, (XtArgVal) False ); n++;
          popUpMenu = XmCreatePopupMenu( actWin->topWidgetId(), "", args, n );

          pullDownMenu = XmCreatePulldownMenu( popUpMenu, "", NULL, 0 );

          numMenuItems = 0;

          for ( ii=0; ii<numDsps; ii++ ) {

	    if ( ii != helpItem ) {

              numMenuItems++;

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

	  }

	}

        if ( destExists[i] ) {

          objAndIndex[i].obj = (void *) this;
          objAndIndex[i].index = i;

          connection.addPv();

	  destPvId[i] = the_PV_Factory->create(
           destPvExpString[i].getExpanded() );
	  if ( destPvId[i] ) {
	    destPvId[i]->add_conn_state_callback(
             relDsp_monitor_dest_connect_state, (void *) &objAndIndex[i] );
	  }
	  else {
            fprintf( stderr, relatedDisplayClass_str27 );
          }

        }

        opComplete[i] = 1;

      }

    }

    break;

  case 4:
  case 5:
  case 6:

    break;

  }

  return 1;

}

int relatedDisplayClass::deactivate (
  int pass )
{

int i;

  active = 0;
  activeMode = 0;

  if ( pass == 1 ) {

    if ( unconnectedTimer ) {
      XtRemoveTimeOut( unconnectedTimer );
      unconnectedTimer = 0;
    }

    XtDestroyWidget( popUpMenu );

    if ( colorExists ) {
      if ( colorPvId ) {
        colorPvId->remove_conn_state_callback(
         relDsp_monitor_color_connect_state, (void *) this );
        colorPvId->remove_value_callback(
         relDsp_color_value_update, (void *) this );
	colorPvId->release();
	colorPvId = NULL;
      }
    }

    for ( i=0; i<NUMPVS; i++ ) {

      if ( destExists[i] ) {
        if ( destPvId[i] ) {
	  destPvId[i]->remove_conn_state_callback(
           relDsp_monitor_dest_connect_state, (void *) &objAndIndex[i] );
	  destPvId[i]->release();
	  destPvId[i] = NULL;
	}
      }

    }

  }

  return 1;

}

void relatedDisplayClass::updateDimensions ( void )
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

int relatedDisplayClass::isRelatedDisplay ( void ) {

  return 1;

}

void relatedDisplayClass::augmentRelatedDisplayMacros (
  char *buf
) {

  int i, l;
  char *newm;
  
  for ( i=0; i<numDsps; i++ ) {
    l = strlen( buf ) + strlen( symbolsExpStr[i].getRaw() );
    if ( l ) {
      newm = new char[l+1];
      strcpy( newm, "" );
      Strncat( newm, symbolsExpStr[i].getRaw(), l );
      trimWhiteSpace( newm );
      if ( strlen(newm) ) Strncat( newm, ",", l );
      Strncat( newm, buf, l );
      symbolsExpStr[i].setRaw( newm );
      delete[] newm;
    }
  }

}

int relatedDisplayClass::getNumRelatedDisplays ( void ) {

  return numDsps;

}

int relatedDisplayClass::getRelatedDisplayProperty (
  int index,
  char *key
) {

  if ( strcmp( key, "propagate" ) == 0 ) {
    return propagateMacros[index];
  }
  else if ( strcmp( key, "replace" ) == 0 ) {
    return replaceSymbols[index];
  }

  return 0;

}

char *relatedDisplayClass::getRelatedDisplayName (
  int index
) {

  if ( ( index < 0 ) || ( index >= numDsps ) ) {
    return NULL;
  }

  return displayFileName[index].getExpanded();

}

char *relatedDisplayClass::getRelatedDisplayMacros (
  int index
) {

  if ( ( index < 0 ) || ( index >= numDsps ) ) {
    return NULL;
  }

  return symbolsExpStr[index].getExpanded();

}

int relatedDisplayClass::expandTemplate (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int i;
expStringClass tmpStr;

  tmpStr.setRaw( colorPvExpString.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  colorPvExpString.setRaw( tmpStr.getExpanded() );

  for ( i=0; i<NUMPVS; i++ ) {

    tmpStr.setRaw( destPvExpString[i].getRaw() );
    tmpStr.expand1st( numMacros, macros, expansions );
    destPvExpString[i].setRaw( tmpStr.getExpanded() );

    tmpStr.setRaw( sourceExpString[i].getRaw() );
    tmpStr.expand1st( numMacros, macros, expansions );
    sourceExpString[i].setRaw( tmpStr.getExpanded() );

  }

  for ( i=0; i<maxDsps; i++ ) {

    tmpStr.setRaw( symbolsExpStr[i].getRaw() );
    tmpStr.expand1st( numMacros, macros, expansions );
    symbolsExpStr[i].setRaw( tmpStr.getExpanded() );

    tmpStr.setRaw( label[i].getRaw() );
    tmpStr.expand1st( numMacros, macros, expansions );
    label[i].setRaw( tmpStr.getExpanded() );

    tmpStr.setRaw( displayFileName[i].getRaw() );
    tmpStr.expand1st( numMacros, macros, expansions );
    displayFileName[i].setRaw( tmpStr.getExpanded() );

  }

  tmpStr.setRaw( buttonLabel.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  buttonLabel.setRaw( tmpStr.getExpanded() );

  return 1;

}

int relatedDisplayClass::expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int i;

int n;
char *m[255], *e[255];

  colorPvExpString.expand1st( numMacros, macros, expansions );

  for ( i=0; i<NUMPVS; i++ ) {
    destPvExpString[i].expand1st( numMacros, macros, expansions );
    sourceExpString[i].expand1st( numMacros, macros, expansions );
  }

  for ( i=0; i<maxDsps; i++ ) {
    symbolsExpStr[i].expand1st( numMacros, macros, expansions );
    label[i].expand1st( numMacros, macros, expansions );
    displayFileName[i].expand1st( numMacros, macros, expansions );
  }

  buttonLabel.expand1st( numMacros, macros, expansions );

  for ( i=0, n=0; i<numMacros; i++, n++ ) {
    m[i] = new char[strlen(macros[i])+1];
    strcpy( m[i], macros[i] );
    e[i] = new char[strlen(expansions[i])+1];
    strcpy( e[i], expansions[i] );
  }

  if ( i+1 < 255 ) {
    m[i] = new char[strlen("!label")+1];
    strcpy( m[i], "!label" );
    e[i] = new char[strlen(buttonLabel.getExpanded())+1];
    strcpy( e[i], buttonLabel.getExpanded() );
    n++;
  }

  helpCommandExpString.expand1st( n, m, e );

  for ( i=0; i<n; i++ ) {
    delete[] m[i];
    delete[] e[i];
  }

  return 1;

}

int relatedDisplayClass::expand2nd (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int i;

  colorPvExpString.expand2nd( numMacros, macros, expansions );

  for ( i=0; i<NUMPVS; i++ ) {
    destPvExpString[i].expand2nd( numMacros, macros, expansions );
    sourceExpString[i].expand2nd( numMacros, macros, expansions );
  }

  for ( i=0; i<maxDsps; i++ ) {
    symbolsExpStr[i].expand2nd( numMacros, macros, expansions );
    label[i].expand2nd( numMacros, macros, expansions );
    displayFileName[i].expand2nd( numMacros, macros, expansions );
  }

  buttonLabel.expand2nd( numMacros, macros, expansions );

  helpCommandExpString.expand2nd( numMacros, macros, expansions );

  return 1;

}

int relatedDisplayClass::containsMacros ( void ) {

int i;

  if ( colorPvExpString.containsPrimaryMacros() ) return 1;

  for ( i=0; i<NUMPVS; i++ ) {
    if ( destPvExpString[i].containsPrimaryMacros() ) return 1;
    if ( sourceExpString[i].containsPrimaryMacros() ) return 1;
  }

  for ( i=0; i<maxDsps; i++ ) {
    if ( symbolsExpStr[i].containsPrimaryMacros() ) return 1;
    if ( label[i].containsPrimaryMacros() ) return 1;
    if ( displayFileName[i].containsPrimaryMacros() ) return 1;
  }

  if ( buttonLabel.containsPrimaryMacros() ) return 1;

  if ( helpCommandExpString.containsPrimaryMacros() ) return 1;

  return 0;

}

void relatedDisplayClass::popupDisplay (
  int index )
{

activeWindowListPtr cur;
int i, ii, dup, numDeleted, l, stat, newX, newY;
char name[127+1], symbolsWithSubs[maxSymbolLen+1], nameWithSubs[maxSymbolLen+1];
pvValType destV;
unsigned int crc;
char *tk, *context, buf[maxSymbolLen+1], *fileTk, *fileContext, fileBuf[maxSymbolLen+1],
 *result, msg[79+1];
FILE *f;
expStringClass symbolsFromFile;
int gotSymbolsFromFile;

int useSmallArrays, symbolCount, maxSymbolLength, focus;

char smallNewMacros[SMALL_SYM_ARRAY_SIZE+1][SMALL_SYM_ARRAY_LEN+1+1];
char smallNewValues[SMALL_SYM_ARRAY_SIZE+1][SMALL_SYM_ARRAY_LEN+1+1];

char *newMacros[100];
char *newValues[100];
int numNewMacros, max, numFound;

char prefix[127+1];

activeWindowClass *aw0, *aw1;

  focus = useFocus;
  //if ( numDsps > 1 ) {
  if ( numMenuItems > 1 ) {
    focus = 0;
  }

  // allow the syntax: @filename s1=v1,s2=v2,...
  // which means read symbols from file and append list
  gotSymbolsFromFile = 0;
  strncpy( buf, symbolsExpStr[index].getExpanded(), maxSymbolLen );
  buf[maxSymbolLen] = 0;
  context = NULL;
  tk = strtok_r( buf, " \t\n", &context );
  if ( tk ) {
    if ( tk[0] == '@' ) {
      if ( tk[1] ) {
        f = actWin->openAnyGenericFile( &tk[1], "r", name, 127 );
	      if ( !f ) {
          snprintf( msg, 79, relatedDisplayClass_str44, &tk[1] );
	        msg[79] = 0;
          actWin->appCtx->postMessage( msg );
          symbolsFromFile.setRaw( "" );
	      }
	      else {
	        result = fgets( fileBuf, maxSymbolLen, f );
	        if ( result ) {
            fileContext = NULL;
            fileTk = strtok_r( fileBuf, "\n", &fileContext );
            if ( fileTk ) {
              symbolsFromFile.setRaw( fileTk );
	          }
	          else {
              snprintf( msg, 79, relatedDisplayClass_str45, name );
              msg[79] = 0;
              actWin->appCtx->postMessage( msg );
              symbolsFromFile.setRaw( "" );
	          }
	        }
	        else {
            if ( errno ) {
              snprintf( msg, 79, relatedDisplayClass_str46, name );
	          }
	          else {
              snprintf( msg, 79, relatedDisplayClass_str45, name );
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
        strncpy( fileBuf, symbolsFromFile.getRaw(), maxSymbolLen );
        fileBuf[maxSymbolLen] = 0;
        if ( blank(fileBuf) ) {
          strcpy( fileBuf, "" );
	      }
        else {
          Strncat( fileBuf, ",", maxSymbolLen );
	      }
	      Strncat( fileBuf, tk, maxSymbolLen );
        symbolsFromFile.setRaw( fileBuf );
      }
      // do special substitutions
      actWin->substituteSpecial( maxSymbolLen, symbolsFromFile.getExpanded(),
       symbolsWithSubs );
      gotSymbolsFromFile = 1;
    }
  }

  if ( !gotSymbolsFromFile ) {
    // do special substitutions
    actWin->substituteSpecial( maxSymbolLen,
     symbolsExpStr[index].getExpanded(),
     symbolsWithSubs );
  }

  // set all existing pvs
  for ( i=0; i<NUMPVS; i++ ) {

    if ( destExists[i] && connection.pvsConnected() ) {

      switch ( destType[i] ) {

      case ProcessVariable::Type::real:
        destV.d = atof( sourceExpString[i].getExpanded() );
        destPvId[i]->put(
         XDisplayName(actWin->appCtx->displayName), destV.d );
        break;

      case ProcessVariable::Type::integer:
        destV.l = atol( sourceExpString[i].getExpanded() );
        destPvId[i]->put(
         XDisplayName(actWin->appCtx->displayName), destV.l );
        break;

      case ProcessVariable::Type::text:
        strncpy( destV.str, sourceExpString[i].getExpanded(), 39 );
        destV.str[39] = 0;
        destPvId[i]->putText(
         XDisplayName(actWin->appCtx->displayName), destV.str );
        break;

      case ProcessVariable::Type::enumerated:
        destV.s = (short) atol( sourceExpString[i].getExpanded() );
        destPvId[i]->put(
         XDisplayName(actWin->appCtx->displayName), destV.s );
        break;

      }

    }

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
          newMacros[i][l-1] = 0;

          l = strlen(actWin->expansions[i]) + 1;
          newValues[i] = (char *) new char[l];
          strcpy( newValues[i], actWin->expansions[i] );
          newValues[i][l-1] = 0;

          numNewMacros++;

        }

      }
      else {

        for ( i=0; i<actWin->appCtx->numMacros; i++ ) {

          l = strlen(actWin->appCtx->macros[i]) + 1;
          newMacros[i] = (char *) new char[l];
          strcpy( newMacros[i], actWin->appCtx->macros[i] );
          newMacros[i][l-1] = 0;

          l = strlen(actWin->appCtx->expansions[i]) + 1;
          newValues[i] = (char *) new char[l];
          strcpy( newValues[i], actWin->appCtx->expansions[i] );
          newValues[i][l-1] = 0;

          numNewMacros++;

        }

      }

    }

    max = 100 - numNewMacros;
    stat = parseSymbolsAndValues( symbolsWithSubs, max,
     &newMacros[numNewMacros], &newValues[numNewMacros], &numFound );

    numNewMacros += numFound;

  }

  // ??????????????????????
  //if ( numNewMacros > 0 ) fprintf( stderr, "\n" );
  //for ( i=0; i<numNewMacros; i++ ) {
  //  fprintf( stderr, "[%s]=[%s]", newMacros[i], newValues[i] );
  //  if ( i < numNewMacros-1 ) fprintf( stderr, "," );
  //}
  //if ( numNewMacros > 0 ) fprintf( stderr, "\n" );

  // Eliminate duplicate symbols

  numDeleted = 0;

  for ( i=numNewMacros-1; i>0; i-- ) {

    dup = 0;
    ii = 0;
    while ( ii < i ) {

      if ( strcmp( newMacros[ii], newMacros[i] ) == 0 ) {
        dup = 1;
	      break;
      }

      ii++;

    }

    if ( dup ) {

      // delete entry i

      if ( !useSmallArrays ) {
        delete[] newMacros[i];
        delete[] newValues[i];
      }

      for ( ii=i; ii<numNewMacros-1; ii++ ) {
        newMacros[ii] = newMacros[ii+1];
        newValues[ii] = newValues[ii+1];
      }

      numDeleted++;

    }

  }

  numNewMacros -= numDeleted;

  // ??????????????????
  //if ( numDeleted ) {
  //  fprintf( stderr, "Removed %-d duplicate symbol(s)\n", numDeleted );
  //  for ( i=0; i<numNewMacros; i++ ) {
  //    fprintf( stderr, "[%s]=[%s]", newMacros[i], newValues[i] );
  //    if ( i < numNewMacros-1 ) fprintf( stderr, "," );
  //  }
  //  if ( numNewMacros > 0 ) fprintf( stderr, "\n" );
  //}

  stat = getFileName( name, displayFileName[index].getExpanded(), 127 );
  stat = getFilePrefix( prefix, displayFileName[index].getExpanded(), 127 );

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
        if ( setPostion[index] == RDC_BUTTON_POS ) {
          newX = actWin->xPos()+posX+ofsX;
	  newY = actWin->yPos()+posY+ofsY;
          cur->node.move( newX, newY );
        }
        else if ( setPostion[index] == RDC_PARENT_OFS_POS ) {
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
            delete[] newMacros[i];
            delete[] newValues[i];
          }
        }
        goto done;
      }
      cur = cur->flink;
    }
  }

  cur = new activeWindowListType;
  actWin->appCtx->addActiveWindow( cur );

  if ( focus || button3Popup ) {
    cur->node.createAutoPopup( actWin->appCtx, NULL, 0, 0, 0, 0,
     numNewMacros, newMacros, newValues );
  }
  else {
    if ( noEdit ) {
      cur->node.createNoEdit( actWin->appCtx, NULL, 0, 0, 0, 0,
       numNewMacros, newMacros, newValues );
    }
    else {
      cur->node.create( actWin->appCtx, NULL, 0, 0, 0, 0,
       numNewMacros, newMacros, newValues );
    }
  }

  if ( !useSmallArrays ) {

    for ( i=0; i<numNewMacros; i++ ) {
      delete[] newMacros[i];
      delete[] newValues[i];
    }

  }

  cur->node.realize();
  cur->node.setGraphicEnvironment( &actWin->appCtx->ci, &actWin->appCtx->fi );

  cur->node.storeFileName( displayFileName[index].getExpanded() );

  if ( setPostion[index] == RDC_BUTTON_POS ) {
    actWin->appCtx->openActivateActiveWindow( &cur->node,
     //actWin->xPos()+x+ofsX, actWin->yPos()+y+ofsY );
     actWin->xPos()+posX+ofsX, actWin->yPos()+posY+ofsY );
  }
  else if ( setPostion[index] == RDC_PARENT_OFS_POS ) {
    actWin->appCtx->openActivateActiveWindow( &cur->node,
     actWin->xPos()+ofsX, actWin->yPos()+ofsY );
  }
  else {
    actWin->appCtx->openActivateActiveWindow( &cur->node );
  }

  if ( focus || button3Popup ) {
    aw = &cur->node;
  }
  else {
    aw = NULL;
  }

done:

  if ( !actWin->isEmbedded ) {

    if ( !focus && !button3Popup && closeAction[index] ) {
      actWin->closeDeferred( 2 );
    }

  }
  else {

    aw1 = NULL;
    aw0 = actWin->parent;
    while ( aw0 ) {

      aw1 = aw0;
      aw0 = aw0->parent;

    }

    if ( aw1 && !focus && !button3Popup && closeAction[index] ) {
      aw1->closeDeferred( 2 );
    }

  }

}

void relatedDisplayClass::btnUp (
  XButtonEvent *be,
  int _x,
  int _y,
  int buttonState,
  int buttonNumber,
  int *action )
{

  *action = 0;

  if ( !enabled ) return;

  if ( swapButtons ) {
    if ( buttonNumber == 1 ) {
      buttonNumber = 3;
    }
    else if ( buttonNumber == 3 ) {
      buttonNumber = 1;
    }
  }

  if ( numDsps == 1 ) {
  //if ( numMenuItems == 1 ) {
    if ( button3Popup ) {
      needClose = 1;
      actWin->addDefExeNode( aglPtr );
      return;
    }
  }

  //if ( numDsps < 2 ) return;
  if ( numMenuItems < 2 ) return;

  if ( buttonNumber != 1 ) return;

  posX = x + _x - be->x;
  posY = y + _y - be->y;

  XmMenuPosition( popUpMenu, be );
  XtManageChild( popUpMenu );

}

#ifdef TRIUMF
#endif

void relatedDisplayClass::btnDown (
  XButtonEvent *be,
  int _x,
  int _y,
  int buttonState,
  int buttonNumber,
  int *action )
{

int focus;

  *action = 0; // close screen via actWin->closeDeferred

  if ( !enabled ) return;

  if ( ( numDsps > 1 ) && button3Popup ) return;

  if ( !blank( helpCommandExpString.getExpanded() ) ) {
    if (  ( buttonNumber == 3 ) &&
         !( buttonState & ShiftMask ) &&
         !( buttonState & ControlMask ) ) {
      if ( helpItem != -1 ) {
        if ( helpItem < numDsps ) popupDisplay( helpItem );
      }
      else {
        executeCommandInThread( helpCommandExpString.getExpanded() );
      }
      return;
    }
  }

  if ( swapButtons ) {
    if ( buttonNumber == 1 ) {
      buttonNumber = 3;
    }
    else if ( buttonNumber == 3 ) {
      buttonNumber = 1;
    }
  }

  focus = useFocus;
  if ( numMenuItems > 1 ) focus = 0;

  if ( focus ) {

    if ( buttonNumber != -1 ) return;

  }
  else {

#ifdef TRIUMF
#endif

    if ( ( buttonNumber != 1 ) && ( buttonNumber != 3 ) ) return;
    if ( ( buttonNumber == 3 ) && !button3Popup ) return;
    if ( ( buttonNumber == 1 ) && button3Popup ) return;
    if ( button3Popup && aw ) return;

  }

  if ( numMenuItems < 1 ) return;

  if ( numMenuItems == 1 ) {
    posX = x + _x - be->x;
    posY = y + _y - be->y;
    popupDisplay( 0 );
  }

}

void relatedDisplayClass::pointerIn (
  XMotionEvent *me,
  int _x,
  int _y,
  int buttonState )
{

int focus;

  if ( !enabled ) return;

  focus = useFocus;

  if ( !focus ) {

    activeGraphicClass::pointerIn( me, me->x, me->y, buttonState );

    if ( !blankOrComment( helpCommandExpString.getExpanded() ) ) {
      actWin->cursor.set( XtWindow(actWin->executeWidget),
       CURSOR_K_WILL_OPEN_WITH_HELP );
    }
    else {
      actWin->cursor.set( XtWindow(actWin->executeWidget),
       CURSOR_K_WILL_OPEN );
    }

  }

}

void relatedDisplayClass::pointerOut (
  XMotionEvent *me,
  int _x,
  int _y,
  int buttonState )
{

int focus;

  if ( !enabled ) return;

  focus = useFocus;

  if ( !focus ) {

    activeGraphicClass::pointerOut( me, me->x, me->y, buttonState );

  }

}

void relatedDisplayClass::mousePointerIn (
  XMotionEvent *me,
  int _x,
  int _y,
  int buttonState )
{

int buttonNumber = -1;
int action;
int focus;
XButtonEvent *be;

  if ( !enabled ) return;

  focus = useFocus;
  //if ( numDsps > 1 ) focus = 0;
  if ( numMenuItems > 1 ) focus = 0;

  if ( focus ) {

    if ( aw ) return;

    be = (XButtonEvent *) me;
    btnDown( be, _x, _y, buttonState, buttonNumber, &action );

  }

}

void relatedDisplayClass::mousePointerOut (
  XMotionEvent *me,
  int _x,
  int _y,
  int buttonState )
{

int focus;

  if ( !enabled ) return;

  focus = useFocus;
  //if ( numDsps > 1 ) focus = 0;
  if ( numMenuItems > 1 ) focus = 0;

  if ( focus ) {

    needClose = 1;
    actWin->addDefExeNode( aglPtr );

  }

}

int relatedDisplayClass::getButtonActionRequest (
  int *up,
  int *down,
  int *drag,
  int *focus )
{

  *drag = 0;
  *down = 1;
  *up = 1;

  if ( !blank( displayFileName[0].getExpanded() ) )
    *focus = 1;
  else
    *focus = 0;

  return 1;

}

void relatedDisplayClass::changeDisplayParams (
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

void relatedDisplayClass::executeDeferred ( void ) {

int nc, ncon, nu, nr, okToClose, colorIndex;
activeWindowListPtr cur;

  actWin->appCtx->proc->lock();
  nc = needClose; needClose = 0;
  ncon = needConnect; needConnect = 0;
  nu = needUpdate; needUpdate = 0;
  nr = needRefresh; needRefresh = 0;
  actWin->remDefExeNode( aglPtr );
  actWin->appCtx->proc->unlock();

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

  if ( !activeMode ) return;

  if ( ncon ) {

    init = 1;
    active = 1;

    fgColor.setConnected();
    bgColor.setConnected();

    if ( colorPvId ) {
      colorPvId->add_value_callback( relDsp_color_value_update,
       (void *) this );
    }

    smartDrawAllActive();

  }

  if ( nu ) {

    colorIndex = actWin->ci->evalRule( fgColor.pixelIndex(),
     colorPvId->get_double() );
    fgColor.changeIndex( colorIndex, actWin->ci );

    colorIndex = actWin->ci->evalRule( bgColor.pixelIndex(),
     colorPvId->get_double() );
    bgColor.changeIndex( colorIndex, actWin->ci );

    smartDrawAllActive();

  }

  if ( nr ) {

    smartDrawAllActive();

  }

}

void relatedDisplayClass::getPvs (
  int max,
  ProcessVariable *pvs[],
  int *n ) {

int i, num = NUMPVS + 1;

  if ( max < num ) {
    *n = 0;
    return;
  }

  *n = num;

  for ( i=0; i<NUMPVS; i++ ) {
    pvs[i] = destPvId[i];
  }
  pvs[NUMPVS+1] = colorPvId;

}

char *relatedDisplayClass::getSearchString (
  int i
) {

int num1 = 1 + 1 + 1 + NUMPVS;
int num2 = 1 + 1 + 1 + NUMPVS + maxDsps + maxDsps + maxDsps;
int ii, selector, index;

  if ( i == 0 ) {
    return buttonLabel.getRaw();
  }
  else if ( i == 1 ) {
    return helpCommandExpString.getRaw();
  }
  else if ( i == 2 ) {
    return colorPvExpString.getRaw();
  }
  else if ( ( i > 2 ) && ( i < num1 ) ) {
    index = i - 3;
    return destPvExpString[index].getRaw();
  }
  else if ( ( i >= num1 ) && ( i < num2 ) ) {
    ii = i - num1;
    selector = ii % 3;
    index = ii / 3;
    if ( selector == 0 ) {
      return displayFileName[index].getRaw();
    }
    else if ( selector == 1 ) {
      return symbolsExpStr[index].getRaw();
    }
    else if ( selector == 2 ) {
      return label[index].getRaw();
    }
  }

  return NULL;

}

void relatedDisplayClass::replaceString (
  int i,
  int max,
  char *string
) {

int num1 = 1 + 1 + 1 + NUMPVS;
int num2 = 1 + 1 + 1 + NUMPVS + maxDsps + maxDsps + maxDsps;
int ii, selector, index;

  if ( i == 0 ) {
    buttonLabel.setRaw( string );
  }
  else if ( i == 1 ) {
    helpCommandExpString.setRaw( string );
  }
  else if ( i == 2 ) {
    colorPvExpString.setRaw( string );
  }
  else if ( ( i > 2 ) && ( i < num1 ) ) {
    index = i - 3;
    destPvExpString[index].setRaw( string );
  }
  else if ( ( i >= num1 ) && ( i < num2 ) ) {
    ii = i - num1;
    selector = ii % 3;
    index = ii / 3;
    if ( selector == 0 ) {
      displayFileName[index].setRaw( string );
    }
    else if ( selector == 1 ) {
      symbolsExpStr[index].setRaw( string );
    }
    else if ( selector == 2 ) {
      label[index].setRaw( string );
    }
  }

}

// crawler functions may return blank pv names
char *relatedDisplayClass::crawlerGetFirstPv ( void ) {

  crawlerPvIndex = 0;
  return colorPvExpString.getExpanded();

}

char *relatedDisplayClass::crawlerGetNextPv ( void ) {

  if ( crawlerPvIndex >= NUMPVS ) return NULL;
  crawlerPvIndex++;
  return destPvExpString[crawlerPvIndex-1].getExpanded();

}

#ifdef __cplusplus
extern "C" {
#endif

void *create_relatedDisplayClassPtr ( void ) {

relatedDisplayClass *ptr;

  ptr = new relatedDisplayClass;
  return (void *) ptr;

}

void *clone_relatedDisplayClassPtr (
  void *_srcPtr )
{

relatedDisplayClass *ptr, *srcPtr;

  srcPtr = (relatedDisplayClass *) _srcPtr;

  ptr = new relatedDisplayClass( srcPtr );

  return (void *) ptr;

}

#ifdef __cplusplus
}
#endif
