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

#define __related_display_cc 1

#define SMALL_SYM_ARRAY_SIZE 10
#define SMALL_SYM_ARRAY_LEN 31

#include "related_display.h"
#include "app_pkg.h"
#include "act_win.h"

#include "thread.h"
#include "crc.h"

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

#ifdef __epics__

static void relDsp_monitor_dest_connect_state (
  struct connection_handler_args arg )
{

objAndIndexType *ptr = (objAndIndexType *) ca_puser(arg.chid);
relatedDisplayClass *rdo = (relatedDisplayClass *) ptr->obj;
int i = ptr->index;

  if ( arg.op == CA_OP_CONN_UP ) {

    rdo->destConnected[i] = 1;
    rdo->destType[i] = ca_field_type( rdo->destPvId[i] );

  }
  else {

    rdo->destConnected[i] = 0;

  }

}

#endif

static void rdc_edit_update1 (
  Widget w,
  XtPointer client,
  XtPointer call )
{

int i;
relatedDisplayClass *rdo = (relatedDisplayClass *) client;

  rdo->actWin->setChanged();

  rdo->numDsps = 0;
  for ( i=0; i<rdo->maxDsps; i++ ) {
    rdo->displayFileName[i].setRaw( rdo->buf->bufDisplayFileName[i] );
    if ( blank( rdo->displayFileName[i].getRaw() ) ) {
      rdo->closeAction[i] = 0;
      rdo->setPostion[i] = 0;
      rdo->allowDups[i] = 0;
      rdo->cascade[i] = 0;
      rdo->propagateMacros[i] = 1;
      rdo->label[i].setRaw( "" );
      rdo->symbolsExpStr[i].setRaw( "" );
      rdo->replaceSymbols[i] = 0;
    }
    else {
      rdo->closeAction[i] = rdo->buf->bufCloseAction[i];
      rdo->setPostion[i] = rdo->buf->bufSetPostion[i];
      rdo->allowDups[i] = rdo->buf->bufAllowDups[i];
      rdo->cascade[i] = rdo->buf->bufCascade[i];
      rdo->propagateMacros[i] = rdo->buf->bufPropagateMacros[i];
      rdo->label[i].setRaw( rdo->buf->bufLabel[i] );
      rdo->symbolsExpStr[i].setRaw( rdo->buf->bufSymbols[i] );
      rdo->replaceSymbols[i] = rdo->buf->bufReplaceSymbols[i];
      (rdo->numDsps)++;
    }
  }

}

static void rdc_edit_apply1 (
  Widget w,
  XtPointer client,
  XtPointer call )
{

  rdc_edit_update1 ( w, client, call );

}

static void rdc_edit_ok1 (
  Widget w,
  XtPointer client,
  XtPointer call )
{

relatedDisplayClass *rdo = (relatedDisplayClass *) client;

  rdc_edit_update1 ( w, client, call );
  rdo->ef1->popdownNoDestroy();

}

static void rdc_edit_cancel1 (
  Widget w,
  XtPointer client,
  XtPointer call )
{

relatedDisplayClass *rdo = (relatedDisplayClass *) client;

  rdo->ef1->popdownNoDestroy();

}

static void rdc_edit_update (

  Widget w,
  XtPointer client,
  XtPointer call )
{

int i;
relatedDisplayClass *rdo = (relatedDisplayClass *) client;

  rdo->actWin->setChanged();

  rdo->eraseSelectBoxCorners();
  rdo->erase();

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

  rdo->noEdit = rdo->buf->bufNoEdit;

  rdo->useFocus = rdo->buf->bufUseFocus;

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

  activeMode = 0;
  invisible = 0;
  noEdit = 0;
  useFocus = 0;

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

}

relatedDisplayClass::~relatedDisplayClass ( void ) {

int okToClose;
activeWindowListPtr cur;

/*   printf( "In relatedDisplayClass::~relatedDisplayClass\n" ); */

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
      aw->returnToEdit( 1 );
      aw = NULL;
    }

  }

  if ( name ) delete name;
  if ( fontList ) XmFontListFree( fontList );
  if ( buf ) {
    delete buf;
    buf = NULL;
  }

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
  topShadowCb = source->topShadowCb;
  botShadowCb = source->botShadowCb;

  fgColor.copy(source->fgColor);
  bgColor.copy(source->bgColor);
  fgCb = source->fgCb;
  bgCb = source->bgCb;

  invisible = source->invisible;
  noEdit = source->noEdit;
  useFocus = source->useFocus;

  for ( i=0; i<maxDsps; i++ ) {
    closeAction[i] = source->closeAction[i];
    setPostion[i] = source->setPostion[i];
    allowDups[i] = source->allowDups[i];
    cascade[i] = source->cascade[i];
    propagateMacros[i] = source->propagateMacros[i];
    displayFileName[i].copy( source->displayFileName[i] );
    //strncpy( displayFileName[i], source->displayFileName[i], 127 );
    label[i].copy( source->label[i] );
    // strncpy( label[i], source->label[i], 127 );
    symbolsExpStr[i].copy( source->symbolsExpStr[i] );
    // strncpy( symbols[i], source->symbols[i], 255 );
    replaceSymbols[i] = source->replaceSymbols[i];
  }

  numDsps = source->numDsps;

  buttonLabel.copy( source->buttonLabel );

  activeMode = 0;

  for ( i=0; i<NUMPVS; i++ ) {
    destPvExpString[i].copy( source->destPvExpString[i] );
    sourceExpString[i].copy( source->sourceExpString[i] );
  }

  aw = NULL;
  buf = NULL;

}

int relatedDisplayClass::createInteractive (
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

int relatedDisplayClass::save (
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
  fprintf( f, "%-d\n", index );

  index = bgColor.pixelIndex();
  fprintf( f, "%-d\n", index );

  index = topShadowColor;
  fprintf( f, "%-d\n", index );

  index = botShadowColor;
  fprintf( f, "%-d\n", index );

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

  return 1;

}

int relatedDisplayClass::createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int i, numPvs, r, g, b, index, more, md;
int major, minor, release;
unsigned int pixel;
char oneName[255+1];
char onePvName[127+1];

  this->actWin = _actWin;

  fscanf( f, "%d %d %d\n", &major, &minor, &release ); actWin->incLine();

  fscanf( f, "%d\n", &x ); actWin->incLine();
  fscanf( f, "%d\n", &y ); actWin->incLine();
  fscanf( f, "%d\n", &w ); actWin->incLine();
  fscanf( f, "%d\n", &h ); actWin->incLine();

  this->initSelectBox(); // call after getting x,y,w,h

  if ( major > 1 ) {

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

  readStringFromFile( oneName, 127, f ); actWin->incLine();
  displayFileName[0].setRaw( oneName );

  if ( blank( displayFileName[0].getRaw() ) ) {
    more = 0;
    numDsps = 0;
  }
  else {
    more = 1;
    numDsps = 1;
  }

  readStringFromFile( oneName, 127, f ); actWin->incLine();
  label[0].setRaw( oneName );

  readStringFromFile( fontTag, 63, f ); actWin->incLine();

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
      readStringFromFile( onePvName, 39, f ); actWin->incLine();
      destPvExpString[i].setRaw( onePvName );
      readStringFromFile( onePvName, 39, f ); actWin->incLine();
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
    readStringFromFile( oneName, 255, f ); actWin->incLine();
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
  if ( ( major < 2 ) || ( major == 2 ) && ( minor < 4 ) ) {

    md = 8;

    if ( ( major > 2 ) || ( major == 2 ) && ( minor > 0 ) ) {

      for ( i=1; i<md; i++ ) { // for forward compatibility

        readStringFromFile( oneName, 127, f ); actWin->incLine();
        displayFileName[i].setRaw( oneName );

        if ( more && !blank(displayFileName[i].getRaw() ) ) {
          numDsps++;
        }
        else {
          more = 0;
        }

        readStringFromFile( oneName, 127, f ); actWin->incLine();
        label[i].setRaw( oneName );

        fscanf( f, "%d\n", &closeAction[i] );

        fscanf( f, "%d\n", &setPostion[i] );

        fscanf( f, "%d\n", &allowDups[i] );

        fscanf( f, "%d\n", &cascade[i] );

        readStringFromFile( oneName, 255, f ); actWin->incLine();
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

    if ( ( major > 2 ) || ( major == 2 ) && ( minor > 1 ) ) {
      readStringFromFile( oneName, 127, f ); actWin->incLine();
      buttonLabel.setRaw( oneName );
    }
    else {
      buttonLabel.setRaw( label[0].getRaw() );
    }

    if ( ( major > 2 ) || ( major == 2 ) && ( minor > 2 ) ) {
      fscanf( f, "%d\n", &noEdit ); actWin->incLine();
    }
    else {
      noEdit = 0;
    }

  }
  else {

    fscanf( f, "%d\n", &numDsps ); actWin->incLine();

    for ( i=1; i<numDsps; i++ ) {

      readStringFromFile( oneName, 127, f ); actWin->incLine();
      displayFileName[i].setRaw( oneName );

      if ( blank(displayFileName[i].getRaw() ) ) {
        more = 0;
      }

      readStringFromFile( oneName, 127, f ); actWin->incLine();
      label[i].setRaw( oneName );

      fscanf( f, "%d\n", &closeAction[i] );

      fscanf( f, "%d\n", &setPostion[i] );

      fscanf( f, "%d\n", &allowDups[i] );

      fscanf( f, "%d\n", &cascade[i] );

      readStringFromFile( oneName, 255, f ); actWin->incLine();
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

    readStringFromFile( oneName, 127, f ); actWin->incLine();
    buttonLabel.setRaw( oneName );

    fscanf( f, "%d\n", &noEdit ); actWin->incLine();

  }

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

int relatedDisplayClass::genericEdit ( void ) {

int i;
char title[32], *ptr;

  buf = new bufType;

  ptr = actWin->obj.getNameFromClass( "relatedDisplayClass" );
  if ( ptr )
    strncpy( title, ptr, 31 );
  else
    strncpy( title, relatedDisplayClass_str17, 31 );

  strncat( title, relatedDisplayClass_str3, 31 );

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
      strncpy( buf->bufSymbols[i], symbolsExpStr[i].getRaw(), 255 );
    }
    else {
      strncpy( buf->bufSymbols[i], "", 255 );
    }

    buf->bufReplaceSymbols[i] = replaceSymbols[i];

  }

  for ( i=0; i<NUMPVS; i++ ) {
    if ( destPvExpString[i].getRaw() ) {
      strncpy( buf->bufDestPvName[i], destPvExpString[i].getRaw(), 39 );
      buf->bufDestPvName[i][39] = 0;
    }
    else {
      strncpy( buf->bufDestPvName[i], "", 39 );
    }
    if ( sourceExpString[i].getRaw() ) {
      strncpy( buf->bufSource[i], sourceExpString[i].getRaw(), 39 );
      buf->bufSource[i][39] = 0;
    }
    else {
      strncpy( buf->bufSource[i], "", 39 );
    }
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

  ef.addTextField( relatedDisplayClass_str4, 30, &buf->bufX );
  ef.addTextField( relatedDisplayClass_str5, 30, &buf->bufY );
  ef.addTextField( relatedDisplayClass_str6, 30, &buf->bufW );
  ef.addTextField( relatedDisplayClass_str7, 30, &buf->bufH );

  ef.addEmbeddedEf( relatedDisplayClass_str14, &ef1 );

  ef1->create( actWin->top, actWin->appCtx->ci.getColorMap(),
   &actWin->appCtx->entryFormX,
   &actWin->appCtx->entryFormY, &actWin->appCtx->entryFormW,
   &actWin->appCtx->entryFormH, &actWin->appCtx->largestH,
   title, NULL, NULL, NULL );

  for ( i=0; i<maxDsps; i++ ) {

    ef1->beginSubForm();
    ef1->addTextField( "Label", 30, buf->bufLabel[i], 127 );
    ef1->addLabel( "  File" );
    ef1->addTextField( "", 30, buf->bufDisplayFileName[i], 127 );
    ef1->addLabel( "  Macros" );
    ef1->addTextField( "", 30, buf->bufSymbols[i], 255 );
    ef1->endSubForm();

    ef1->beginLeftSubForm();
    ef1->addLabel( "  Mode" );
    ef1->addOption( "", "Append|Replace", &buf->bufReplaceSymbols[i] );
    ef1->addLabel( " " );
    ef1->addToggle( " ", &buf->bufPropagateMacros[i] );
    ef1->addLabel( "Propagate  " );
    ef1->addToggle( " ", &buf->bufSetPostion[i] );
    ef1->addLabel( "Set Position  " );
    ef1->addToggle( " ", &buf->bufCloseAction[i] );
    ef1->addLabel( "Close Current  " );
    ef1->addToggle( " ", &buf->bufAllowDups[i] );
    ef1->addLabel( "Dups Allowed  " );
    ef1->addToggle( " ", &buf->bufCascade[i] );
    ef1->addLabel( "Cascade" );
    ef1->endSubForm();

  }

  ef1->finished( rdc_edit_ok1, rdc_edit_apply1, rdc_edit_cancel1, this );

  ef.addTextField( relatedDisplayClass_str13, 30, buf->bufButtonLabel, 127 );

  ef.addToggle( relatedDisplayClass_str17, &buf->bufUseFocus );
  ef.addToggle( relatedDisplayClass_str19, &buf->bufInvisible );
  ef.addToggle( relatedDisplayClass_str29, &buf->bufNoEdit );

  for ( i=0; i<NUMPVS; i++ ) {
    ef.addTextField( relatedDisplayClass_str15, 30, buf->bufDestPvName[i],
     39 );
    ef.addTextField( relatedDisplayClass_str16, 30, buf->bufSource[i], 39 );
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

  if ( !activeMode || invisible ) return 1;

  XDrawRectangle( actWin->d, XtWindow(actWin->executeWidget),
   actWin->executeGc.eraseGC(), x, y, w, h );

  XFillRectangle( actWin->d, XtWindow(actWin->executeWidget),
   actWin->executeGc.eraseGC(), x, y, w, h );

  return 1;

}

int relatedDisplayClass::draw ( void ) {

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

int relatedDisplayClass::drawActive ( void ) {

int tX, tY;
char string[39+1];
XRectangle xR = { x, y, w, h };

  if ( !activeMode || invisible ) return 1;

  actWin->executeGc.saveFg();

  actWin->executeGc.setFG( bgColor.getColor() );

  XFillRectangle( actWin->d, XtWindow(actWin->executeWidget),
   actWin->executeGc.normGC(), x, y, w, h );

  XDrawRectangle( actWin->d, XtWindow(actWin->executeWidget),
   actWin->executeGc.normGC(), x, y, w, h );

  if ( buttonLabel.getExpanded() )
    strncpy( string, buttonLabel.getExpanded(), 39 );
  else
    strncpy( string, "", 39 );

  actWin->executeGc.setFG( actWin->ci->pix(botShadowColor) );

  XDrawLine( actWin->d, XtWindow(actWin->executeWidget),
   actWin->executeGc.normGC(), x, y, x+w, y );

  XDrawLine( actWin->d, XtWindow(actWin->executeWidget),
   actWin->executeGc.normGC(), x, y, x, y+h );

  actWin->executeGc.setFG( actWin->ci->pix(topShadowColor) );

  XDrawLine( actWin->d, XtWindow(actWin->executeWidget),
   actWin->executeGc.normGC(), x, y+h, x+w, y+h );

  XDrawLine( actWin->d, XtWindow(actWin->executeWidget),
   actWin->executeGc.normGC(), x+w, y, x+w, y+h );

  // top
  actWin->executeGc.setFG( actWin->ci->pix(topShadowColor) );

  XDrawLine( actWin->d, XtWindow(actWin->executeWidget),
   actWin->executeGc.normGC(), x+1, y+1, x+w-1, y+1 );

  XDrawLine( actWin->d, XtWindow(actWin->executeWidget),
   actWin->executeGc.normGC(), x+2, y+2, x+w-2, y+2 );

  // left
  XDrawLine( actWin->d, XtWindow(actWin->executeWidget),
   actWin->executeGc.normGC(), x+1, y+1, x+1, y+h-1 );

  XDrawLine( actWin->d, XtWindow(actWin->executeWidget),
   actWin->executeGc.normGC(), x+2, y+2, x+2, y+h-2 );

  // bottom
  actWin->executeGc.setFG( actWin->ci->pix(botShadowColor) );

  XDrawLine( actWin->d, XtWindow(actWin->executeWidget),
   actWin->executeGc.normGC(), x+1, y+h-1, x+w-1, y+h-1 );

  XDrawLine( actWin->d, XtWindow(actWin->executeWidget),
   actWin->executeGc.normGC(), x+2, y+h-2, x+w-2, y+h-2 );

  // right
  XDrawLine( actWin->d, XtWindow(actWin->executeWidget),
   actWin->executeGc.normGC(), x+w-1, y+1, x+w-1, y+h-1 );

  XDrawLine( actWin->d, XtWindow(actWin->executeWidget),
   actWin->executeGc.normGC(), x+w-2, y+2, x+w-2, y+h-2 );

  if ( fs ) {

    actWin->executeGc.addNormXClipRectangle( xR );

    actWin->executeGc.setFG( fgColor.getColor() );
    actWin->executeGc.setFontTag( fontTag, actWin->fi );

    tX = x + w/2;
    tY = y + h/2 - fontAscent/2;

    drawText( actWin->executeWidget, &actWin->executeGc, fs, tX, tY,
     XmALIGNMENT_CENTER, string );

    actWin->executeGc.removeNormXClipRectangle();

  }

  actWin->executeGc.restoreFg();

  return 1;

}

int relatedDisplayClass::activate (
  int pass,
  void *ptr )
{

int i, ii, stat, opStat, n;
Arg args[5];
XmString str;

  switch ( pass ) {

  case 1:

    aglPtr = ptr;
    aw = NULL;
    needClose = 0;

    for ( i=0; i<NUMPVS; i++ ) {

      opComplete[i] = 0;

      if ( !destPvExpString[i].getExpanded() ||
         ( strcmp( destPvExpString[i].getExpanded(), "" ) == 0 ) ) {
        destExists[i] = 0;
        destConnected[i] = 1;
      }
      else {
        destExists[i] = 1;
        destConnected[i] = 0;
      }

    }

    activeMode = 1;

    break;

  case 2:

    opStat = 1;

    for ( i=0; i<NUMPVS; i++ ) {

      if ( !opComplete[i] ) {

        if ( i == 0 ) {

          n = 0;
          XtSetArg( args[n], XmNmenuPost, (XtArgVal) "<Btn5Down>;" ); n++;
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

	}

#ifdef __epics__

        if ( destExists[i] ) {

          objAndIndex[i].obj = (void *) this;
          objAndIndex[i].index = i;

          stat = ca_search_and_connect(
           destPvExpString[i].getExpanded(), &destPvId[i],
           relDsp_monitor_dest_connect_state,
           (void *) &objAndIndex[i] );
          if ( stat != ECA_NORMAL ) {
            printf( relatedDisplayClass_str27 );
            opStat = 0;
          }
          else {
            opComplete[i] = 1;
	  }

        }

#endif

      }

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

int relatedDisplayClass::deactivate (
  int pass )
{

int i, stat;

  if ( pass == 1 ) {

    activeMode = 0;

    XtDestroyWidget( popUpMenu );

#ifdef __epics__

    for ( i=0; i<NUMPVS; i++ ) {

      if ( destExists[i] ) {

        stat = ca_clear_channel( destPvId[i] );
        if ( stat != ECA_NORMAL )
          printf( relatedDisplayClass_str28 );

      }

    }

#endif

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

int relatedDisplayClass::expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int i;

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

  return 1;

}

int relatedDisplayClass::expand2nd (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int i;

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

  return 1;

}

int relatedDisplayClass::containsMacros ( void ) {

int i;

  for ( i=0; i<NUMPVS; i++ ) {
    if ( destPvExpString[i].containsPrimaryMacros() ) return 1;
    if ( sourceExpString[i].containsPrimaryMacros() ) return 1;
  }

  for ( i=0; i<maxDsps; i++ ) {
    if ( symbolsExpStr[i].containsPrimaryMacros() ) return 1;
    if ( label[i].containsPrimaryMacros() ) return 1;
    if ( displayFileName[i].containsPrimaryMacros() ) return 1;
  }

  return 0;

}

void relatedDisplayClass::popupDisplay (
  int index )
{

activeWindowListPtr cur;
int i, l, stat;
char name[127+1], symbolsWithSubs[255+1];
pvValType destV;
unsigned int crc;

int useSmallArrays, symbolCount, maxSymbolLength, focus;

char smallNewMacros[SMALL_SYM_ARRAY_SIZE+1][SMALL_SYM_ARRAY_LEN+1+1];
char smallNewValues[SMALL_SYM_ARRAY_SIZE+1][SMALL_SYM_ARRAY_LEN+1+1];

char *newMacros[100];
char *newValues[100];
int numNewMacros, max, numFound;

  focus = useFocus;
  if ( numDsps > 1 ) focus = 0;

  // do special substitutions
  actWin->substituteSpecial( 255, symbolsExpStr[index].getExpanded(),
   symbolsWithSubs );

  // set all existing pvs
  for ( i=0; i<NUMPVS; i++ ) {

    if ( destExists[i] && destConnected[i] ) {

#ifdef __epics__

      switch ( destType[i] ) {

      case DBR_FLOAT:
      case DBR_DOUBLE:
        destV.d = atof( sourceExpString[i].getExpanded() );
        stat = ca_put( DBR_DOUBLE, destPvId[i], &destV.d );
        break;

      case DBR_LONG:
        destV.l = atol( sourceExpString[i].getExpanded() );
        stat = ca_put( DBR_LONG, destPvId[i], &destV.l );
        break;

      case DBR_STRING:
        strncpy( destV.str, sourceExpString[i].getExpanded(), 39 );
        stat = ca_put( DBR_STRING, destPvId[i], destV.str );
        break;

      case DBR_ENUM:
        destV.s = (short) atol( sourceExpString[i].getExpanded() );
        stat = ca_put( DBR_ENUM, destPvId[i], &destV.s );
        break;

      }

#endif

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
          newMacros[i] = (char *) new (char)[l];
          strcpy( newMacros[i], actWin->macros[i] );

          l = strlen(actWin->expansions[i]) + 1;
          newValues[i] = (char *) new (char)[l];
          strcpy( newValues[i], actWin->expansions[i] );

          numNewMacros++;

        }

      }
      else {

        for ( i=0; i<actWin->appCtx->numMacros; i++ ) {

          l = strlen(actWin->appCtx->macros[i]) + 1;
          newMacros[i] = (char *) new (char)[l];
          strcpy( newMacros[i], actWin->appCtx->macros[i] );

          l = strlen(actWin->appCtx->expansions[i]) + 1;
          newValues[i] = (char *) new (char)[l];
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

  stat = getFileName( name, displayFileName[index].getExpanded(), 127 );

  // calc crc

  crc = 0;
  for ( i=0; i<numNewMacros; i++ ) {
    crc = updateCRC( crc, newMacros[i], strlen(newMacros[i]) );
    crc = updateCRC( crc, newValues[i], strlen(newValues[i]) );
  }

  if ( !allowDups[index] ) {
    cur = actWin->appCtx->head->flink;
    while ( cur != actWin->appCtx->head ) {
      if ( ( strcmp( name, cur->node.name ) == 0 ) &&
           ( crc == cur->node.crc ) ) {
        // deiconify
        XMapWindow( cur->node.d, XtWindow(cur->node.topWidgetId()) );
        // raise
        XRaiseWindow( cur->node.d, XtWindow(cur->node.topWidgetId()) );
        if ( !useSmallArrays ) {
          for ( i=0; i<numNewMacros; i++ ) {
            delete newMacros[i];
            delete newValues[i];
          }
        }
        goto done; // display is already open; don't open another instance
      }
      cur = cur->flink;
    }
  }

  cur = new activeWindowListType;
  actWin->appCtx->addActiveWindow( cur );

  if ( focus ) {
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
      delete newMacros[i];
      delete newValues[i];
    }

  }

  cur->node.realize();
  cur->node.setGraphicEnvironment( &actWin->appCtx->ci, &actWin->appCtx->fi );

  cur->node.storeFileName( displayFileName[index].getExpanded() );

  if ( setPostion[index] ) {
    if ( cascade[index] ) {
      actWin->appCtx->openActivateCascadeActiveWindow( &cur->node,
       actWin->x+posX, actWin->y+posY );
    }
    else {
      actWin->appCtx->openActivateActiveWindow( &cur->node,
       actWin->x+posX, actWin->y+posY );
    }
  }
  else {
    if ( cascade[index] ) {
      actWin->appCtx->openActivateCascadeActiveWindow( &cur->node );
    }
    else {
      actWin->appCtx->openActivateActiveWindow( &cur->node );
    }
  }

  if ( focus ) {
    aw = &cur->node;
  }

done:

  if ( !focus && closeAction[index] ) {
    actWin->closeDeferred( 2 );
  }

}

void relatedDisplayClass::btnUp (
  int _x,
  int _y,
  int buttonState,
  int buttonNumber,
  int *action )
{

XButtonEvent be;

  *action = 0;

  if ( numDsps < 2 ) return;

  posX = _x;
  posY = _y;

  be.x_root = actWin->x+_x;
  be.y_root = actWin->y+_y;
  XmMenuPosition( popUpMenu, &be );
  XtManageChild( popUpMenu );

}

void relatedDisplayClass::btnDown (
  int _x,
  int _y,
  int buttonState,
  int buttonNumber,
  int *action )
{

int focus;

  focus = useFocus;
  if ( numDsps > 1 ) focus = 0;

  if ( focus ) {
    if ( buttonNumber != -1 ) return;
  }
  else {
    if ( buttonNumber != 1 ) return;
  }

  if ( numDsps < 1 ) return;

  if ( numDsps == 1 ) {
    posX = _x;
    posY = _y;
    popupDisplay( 0 );
  }

  *action = 0; // close screen via actWin->closeDeferred

}

void relatedDisplayClass::pointerIn (
  int _x,
  int _y,
  int buttonState )
{

int buttonNumber = -1;
int action;
int focus;

  focus = useFocus;
  if ( numDsps > 1 ) focus = 0;

  if ( focus ) {

    if ( aw ) return;

    btnDown( _x, _y, buttonState, buttonNumber, &action );

  }
  else {

    activeGraphicClass::pointerIn( _x, _y, buttonState );

  }

}

void relatedDisplayClass::pointerOut (
  int _x,
  int _y,
  int buttonState )
{

int focus;

  focus = useFocus;
  if ( numDsps > 1 ) focus = 0;

  if ( focus ) {

    needClose = 1;
    actWin->addDefExeNode( aglPtr );

  }
  else {

    activeGraphicClass::pointerOut( _x, _y, buttonState );

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

int nc, okToClose;
activeWindowListPtr cur;

  actWin->appCtx->proc->lock();
  nc = needClose; needClose = 0;
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
        aw->returnToEdit( 1 );
        aw = NULL;
      }
      else {
        aw = NULL;
      }

    }

  }

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
