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

#include "related_display.h"
#include "app_pkg.h"
#include "act_win.h"

#include "thread.h"
#include "crc.h"

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

  rdo->topShadowColor = rdo->bufTopShadowColor;
  rdo->botShadowColor = rdo->bufBotShadowColor;

  rdo->fgColor.setColorIndex( rdo->bufFgColor, rdo->actWin->ci );

  rdo->bgColor.setColorIndex( rdo->bufBgColor, rdo->actWin->ci );

  rdo->invisible = rdo->bufInvisible;

  rdo->closeAction = rdo->bufCloseAction;

  rdo->useFocus = rdo->bufUseFocus;

  rdo->setPostion = rdo->bufSetPostion;

  rdo->allowDups = rdo->bufAllowDups;

  rdo->cascade = rdo->bufCascade;

  rdo->propagateMacros = rdo->bufPropagateMacros;

  rdo->x = rdo->bufX;
  rdo->sboxX = rdo->bufX;

  rdo->y = rdo->bufY;
  rdo->sboxY = rdo->bufY;

  rdo->w = rdo->bufW;
  rdo->sboxW = rdo->bufW;

  rdo->h = rdo->bufH;
  rdo->sboxH = rdo->bufH;

  strncpy( rdo->displayFileName, rdo->bufDisplayFileName, 127 );

  rdo->label.setRaw( rdo->bufLabel );
  //strncpy( rdo->label, rdo->bufLabel, 127 );

  rdo->symbolsExpStr.setRaw( rdo->bufSymbols );
  //  strncpy( rdo->symbols, rdo->bufSymbols, 255 );

  rdo->replaceSymbols = rdo->bufReplaceSymbols;

  for ( i=0; i<NUMPVS; i++ ) {
    rdo->destPvExpString[i].setRaw( rdo->bufDestPvName[i] );
    rdo->sourceExpString[i].setRaw( rdo->bufSource[i] );
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

}

static void rdc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call )
{

relatedDisplayClass *rdo = (relatedDisplayClass *) client;

  rdo->ef.popdown();
  rdo->operationCancel();

}

static void rdc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call )
{

relatedDisplayClass *rdo = (relatedDisplayClass *) client;

  rdo->erase();
  rdo->deleteRequest = 1;
  rdo->ef.popdown();
  rdo->operationCancel();
  rdo->drawAll();

}

relatedDisplayClass::relatedDisplayClass ( void ) {

  name = new char[strlen("relatedDisplayClass")+1];
  strcpy( name, "relatedDisplayClass" );

  activeMode = 0;
  strcpy( displayFileName, "" );
  // strcpy( label, "" );
  // strcpy( symbols, "" );
  invisible = 0;
  closeAction = 0;
  useFocus = 0;
  setPostion = 0;
  allowDups = 0;
  cascade = 0;
  propagateMacros = 1;
  replaceSymbols = 0;
  fontList = NULL;
  aw = NULL;

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

  closeAction = source->closeAction;

  useFocus = source->useFocus;

  setPostion = source->setPostion;

  allowDups = source->allowDups;

  cascade = source->cascade;

  propagateMacros = source->propagateMacros;

  strncpy( displayFileName, source->displayFileName, 127 );

  label.copy( source->label );
  // strncpy( label, source->label, 127 );

  symbolsExpStr.copy( source->symbolsExpStr );
  // strncpy( symbols, source->symbols, 255 );

  replaceSymbols = source->replaceSymbols;

  activeMode = 0;

  for ( i=0; i<NUMPVS; i++ ) {
    destPvExpString[i].copy( source->destPvExpString[i] );
    sourceExpString[i].copy( source->sourceExpString[i] );
  }

  aw = NULL;

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

  if ( displayFileName )
    writeStringToFile( f, displayFileName );
  else
    writeStringToFile( f, "" );

  if ( label.getRaw() )
    writeStringToFile( f, label.getRaw() );
  else
    writeStringToFile( f, "" );

  //if ( label )
  //  writeStringToFile( f, label );
  //else
  //  writeStringToFile( f, "" );

  writeStringToFile( f, fontTag );

  fprintf( f, "%-d\n", invisible );

  fprintf( f, "%-d\n", closeAction );

  fprintf( f, "%-d\n", setPostion );

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

  fprintf( f, "%-d\n", allowDups );

  fprintf( f, "%-d\n", cascade );

  if ( symbolsExpStr.getRaw() ) {
    writeStringToFile( f, symbolsExpStr.getRaw() );
  }
  else {
    writeStringToFile( f, "" );
  }

  //  writeStringToFile( f, symbols );

  fprintf( f, "%-d\n", replaceSymbols );

  fprintf( f, "%-d\n", propagateMacros );

  fprintf( f, "%-d\n", useFocus );

  return 1;

}

int relatedDisplayClass::createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int i, numPvs, r, g, b, index;
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
  strncpy( displayFileName, oneName, 127 );

  readStringFromFile( oneName, 127, f ); actWin->incLine();
  label.setRaw( oneName );
  //strncpy( label, oneName, 127 );

  readStringFromFile( fontTag, 63, f ); actWin->incLine();

  if ( ( major > 1 ) || ( minor > 2 ) ) {
    fscanf( f, "%d\n", &invisible ); actWin->incLine();
    fscanf( f, "%d\n", &closeAction ); actWin->incLine();
  }
  else {
    invisible = 0;
    closeAction = 0;
  }

  if ( ( major > 1 ) || ( minor > 3 ) ) {
    fscanf( f, "%d\n", &setPostion ); actWin->incLine();
  }
  else {
    setPostion = 0;
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
    fscanf( f, "%d\n", &allowDups ); actWin->incLine();
  }
  else {
    allowDups = 0;
  }

  if ( ( major > 1 ) || ( minor > 7 ) ) {
    fscanf( f, "%d\n", &cascade ); actWin->incLine();
  }
  else {
    cascade = 0;
  }

  if ( ( major > 1 ) || ( minor > 8 ) ) {
    readStringFromFile( oneName, 255, f ); actWin->incLine();
    symbolsExpStr.setRaw( oneName );
    //    strncpy( symbols, oneName, 255 );
    fscanf( f, "%d\n", &replaceSymbols ); actWin->incLine();
  }
  else {
    symbolsExpStr.setRaw( "" );
    //    strcpy( symbols, "" );
    replaceSymbols  = 0;
  }

  if ( ( major > 1 ) || ( minor > 9 ) ) {
    fscanf( f, "%d\n", &propagateMacros ); actWin->incLine();
  }
  else {
    propagateMacros = 0;
  }

  if ( ( major > 1 ) || ( minor > 10 ) ) {
    fscanf( f, "%d\n", &useFocus ); actWin->incLine();
  }
  else {
    useFocus = 0;
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

        closeAction = atol( tk );

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

        strncpy( displayFileName, tk, 127 );

      }

      else if ( strcmp( tk, "label" ) == 0 ) {

        tk = strtok_r( NULL, "\"", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( relatedDisplayClass_str1 );
          return 0;
        }

        label.setRaw( tk );
        // strncpy( label, tk, 127 );

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

  ptr = actWin->obj.getNameFromClass( "relatedDisplayClass" );
  if ( ptr )
    strncpy( title, ptr, 31 );
  else
    strncpy( title, relatedDisplayClass_str17, 31 );

  strncat( title, relatedDisplayClass_str3, 31 );

  bufX = x;
  bufY = y;
  bufW = w;
  bufH = h;

  strncpy( bufFontTag, fontTag, 63 );

  bufTopShadowColor = topShadowColor;
  bufBotShadowColor = botShadowColor;

  bufFgColor = fgColor.pixelIndex();

  bufBgColor = bgColor.pixelIndex();

  if ( displayFileName )
    strncpy( bufDisplayFileName, displayFileName, 127 );
  else
    strncpy( bufDisplayFileName, "", 127 );

  if ( label.getRaw() )
    strncpy( bufLabel, label.getRaw(), 127 );
  else
    strncpy( bufLabel, "", 127 );

  //if ( label )
  //  strncpy( bufLabel, label, 127 );
  //else
  //  strncpy( bufLabel, "", 127 );

  bufInvisible = invisible;

  bufCloseAction = closeAction;

  bufUseFocus = useFocus;

  bufSetPostion = setPostion;

  bufAllowDups = allowDups;

  bufCascade = cascade;

  bufPropagateMacros = propagateMacros;

  if ( symbolsExpStr.getRaw() ) {
    strncpy( bufSymbols, symbolsExpStr.getRaw(), 255 );
  }
  else {
    strncpy( bufSymbols, "", 255 );
  }

  //  if ( symbols )
  //    strncpy( bufSymbols, symbols, 255 );
  //  else
  //    strncpy( bufSymbols, "", 255 );

  bufReplaceSymbols = replaceSymbols;

  for ( i=0; i<NUMPVS; i++ ) {
    if ( destPvExpString[i].getRaw() ) {
      strncpy( bufDestPvName[i], destPvExpString[i].getRaw(), 39 );
      bufDestPvName[i][39] = 0;
    }
    else {
      strncpy( bufDestPvName[i], "", 39 );
    }
    if ( sourceExpString[i].getRaw() ) {
      strncpy( bufSource[i], sourceExpString[i].getRaw(), 39 );
      bufSource[i][39] = 0;
    }
    else {
      strncpy( bufSource[i], "", 39 );
    }
  }

  ef.create( actWin->top, actWin->appCtx->ci.getColorMap(),
   &actWin->appCtx->entryFormX,
   &actWin->appCtx->entryFormY, &actWin->appCtx->entryFormW,
   &actWin->appCtx->entryFormH, &actWin->appCtx->largestH,
   title, NULL, NULL, NULL );

  ef.addTextField( relatedDisplayClass_str4, 30, &bufX );
  ef.addTextField( relatedDisplayClass_str5, 30, &bufY );
  ef.addTextField( relatedDisplayClass_str6, 30, &bufW );
  ef.addTextField( relatedDisplayClass_str7, 30, &bufH );
  ef.addColorButton( relatedDisplayClass_str8, actWin->ci, &fgCb, &bufFgColor );
  ef.addColorButton( relatedDisplayClass_str9, actWin->ci, &bgCb, &bufBgColor );
  ef.addColorButton( relatedDisplayClass_str10, actWin->ci, &topShadowCb,
   &bufTopShadowColor );
  ef.addColorButton( relatedDisplayClass_str11, actWin->ci, &botShadowCb,
   &bufBotShadowColor );
  ef.addFontMenu( relatedDisplayClass_str12, actWin->fi, &fm, fontTag );
  ef.addTextField( relatedDisplayClass_str13, 30, bufLabel, 127 );
  ef.addTextField( relatedDisplayClass_str14, 30, bufDisplayFileName, 127 );

  for ( i=0; i<NUMPVS; i++ ) {
    ef.addTextField( relatedDisplayClass_str15, 30, bufDestPvName[i], 39 );
    ef.addTextField( relatedDisplayClass_str16, 30, bufSource[i], 39 );
  }

  ef.addToggle( relatedDisplayClass_str17, &bufUseFocus );
  ef.addToggle( relatedDisplayClass_str18, &bufSetPostion );
  ef.addToggle( relatedDisplayClass_str19, &bufInvisible );
  ef.addToggle( relatedDisplayClass_str20, &bufCloseAction );
  ef.addToggle( relatedDisplayClass_str21, &bufAllowDups );
  ef.addToggle( relatedDisplayClass_str22, &bufCascade );

  ef.addOption( relatedDisplayClass_str23, relatedDisplayClass_str24, &bufReplaceSymbols );

  ef.addToggle( relatedDisplayClass_str25, &bufPropagateMacros );

  ef.addTextField( relatedDisplayClass_str26, 30, bufSymbols, 255 );

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

    if ( label.getRaw() )
      drawText( actWin->drawWidget, &actWin->drawGc, fs, tX, tY,
       XmALIGNMENT_CENTER, label.getRaw() );
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

  if ( label.getExpanded() )
    strncpy( string, label.getExpanded(), 39 );
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

int i, stat, opStat;

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

  symbolsExpStr.expand1st( numMacros, macros, expansions );

  label.expand1st( numMacros, macros, expansions );

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

  symbolsExpStr.expand2nd( numMacros, macros, expansions );

  label.expand2nd( numMacros, macros, expansions );

  return 1;

}

int relatedDisplayClass::containsMacros ( void ) {

int i;

  for ( i=0; i<NUMPVS; i++ ) {
    if ( destPvExpString[i].containsPrimaryMacros() ) return 1;
    if ( sourceExpString[i].containsPrimaryMacros() ) return 1;
  }

  if ( symbolsExpStr.containsPrimaryMacros() ) return 1;

  if ( label.containsPrimaryMacros() ) return 1;

  return 0;

}

void relatedDisplayClass::btnUp (
  int x,
  int y,
  int buttonState,
  int buttonNumber,
  int *action )
{

  *action = 0;

}

#define SMALL_SYM_ARRAY_SIZE 10
#define SMALL_SYM_ARRAY_LEN 31

void relatedDisplayClass::btnDown (
  int x,
  int y,
  int buttonState,
  int buttonNumber,
  int *action )
{

activeWindowListPtr cur;
int i, l, stat;
char name[127+1], symbolsWithSubs[255+1];
pvValType destV;
unsigned int crc;

int useSmallArrays, symbolCount, maxSymbolLength;

char smallNewMacros[SMALL_SYM_ARRAY_SIZE+1][SMALL_SYM_ARRAY_LEN+1+1];
char smallNewValues[SMALL_SYM_ARRAY_SIZE+1][SMALL_SYM_ARRAY_LEN+1+1];

char *newMacros[100];
char *newValues[100];
int numNewMacros, max, numFound;

  if ( useFocus ) {
    if ( buttonNumber != -1 ) return;
  }

  // do special substitutions
  actWin->substituteSpecial( 255, symbolsExpStr.getExpanded(),
   symbolsWithSubs );
  //  actWin->substituteSpecial( 255, symbols, symbolsWithSubs );

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

  if ( !replaceSymbols ) {

    if ( propagateMacros ) {

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

    if ( !replaceSymbols ) {

      if ( propagateMacros ) {

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

    if ( !replaceSymbols ) {

      if ( propagateMacros ) {

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

  stat = getFileName( name, displayFileName, 127 );

  // calc crc

  crc = 0;
  for ( i=0; i<numNewMacros; i++ ) {
    crc = updateCRC( crc, newMacros[i], strlen(newMacros[i]) );
    crc = updateCRC( crc, newValues[i], strlen(newValues[i]) );
  }

  if ( !allowDups ) {
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
        return;  // display is already open; don't open another instance
      }
      cur = cur->flink;
    }
  }

  *action = closeAction;

  cur = new activeWindowListType;
  actWin->appCtx->addActiveWindow( cur );

  if ( useFocus ) {
    cur->node.createAutoPopup( actWin->appCtx, NULL, 0, 0, 0, 0,
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

  cur->node.storeFileName( displayFileName );

  if ( setPostion ) {
    if ( cascade ) {
      actWin->appCtx->openActivateCascadeActiveWindow( &cur->node, actWin->x+x,
       actWin->y+y );
    }
    else {
      actWin->appCtx->openActivateActiveWindow( &cur->node, actWin->x+x,
       actWin->y+y );
    }
  }
  else {
    if ( cascade ) {
      actWin->appCtx->openActivateCascadeActiveWindow( &cur->node );
    }
    else {
      actWin->appCtx->openActivateActiveWindow( &cur->node );
    }
  }

  if ( useFocus ) {
    aw = &cur->node;
  }

}

void relatedDisplayClass::pointerIn (
  int _x,
  int _y,
  int buttonState )
{

int buttonNumber = -1;
int action;

  if ( useFocus ) {

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

  if ( useFocus ) {

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

  if ( !blank( displayFileName ) )
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
