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

#define __x_text_gen_cc 1

#include "x_text_gen.h"
#include "app_pkg.h"
#include "act_win.h"

#include "thread.h"

static void axtoMonitorAlarmPvConnectState (
  pvClass *classPtr,
  void *clientData,
  void *args )
{

activeXTextClass *axto = 
  (activeXTextClass *) clientData;

  axto->actWin->appCtx->proc->lock();

  if ( !axto->activeMode ) {
    axto->actWin->appCtx->proc->unlock();
    return;
  }

  if ( classPtr->getOp( args ) == classPtr->pvkOpConnUp() ) {

    axto->needAlarmConnectInit = 1;

  }
  else { // lost connection

    axto->alarmPvConnected = 0;
    axto->active = 0;
    axto->fgColor.setDisconnected();
    axto->bgColor.setDisconnected();
    axto->bufInvalidate();
    axto->needDraw = 1;

  }

  axto->actWin->addDefExeNode( axto->aglPtr );

  axto->actWin->appCtx->proc->unlock();

}

static void xTextAlarmUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args )
{

class activeXTextClass *axto = (activeXTextClass *) clientData;

  axto->actWin->appCtx->proc->lock();

  if ( !axto->activeMode ) {
    axto->actWin->appCtx->proc->unlock();
    return;
  }

  axto->fgColor.setStatus( classPtr->getStatus( args ), 
    classPtr->getSeverity( args ) );
  axto->bgColor.setStatus( classPtr->getStatus( args ), 
    classPtr->getSeverity( args ) );

  if ( axto->active ) {
    axto->bufInvalidate();
    axto->needRefresh = 1;
    axto->actWin->addDefExeNode( axto->aglPtr );
  }

  axto->actWin->appCtx->proc->unlock();

}

static void axtoMonitorVisPvConnectState (
  pvClass *classPtr,
  void *clientData,
  void *args )
{

activeXTextClass *axto = 
  (activeXTextClass *) clientData;

  axto->actWin->appCtx->proc->lock();

  if ( !axto->activeMode ) {
    axto->actWin->appCtx->proc->unlock();
    return;
  }

  if ( classPtr->getOp( args ) == classPtr->pvkOpConnUp() ) {

    axto->needVisConnectInit = 1;

  }
  else { // lost connection

    axto->visPvConnected = 0;
    axto->active = 0;
    axto->fgColor.setDisconnected();
    axto->bgColor.setDisconnected();
    axto->bufInvalidate();
    axto->needDraw = 1;

  }

  axto->actWin->addDefExeNode( axto->aglPtr );

  axto->actWin->appCtx->proc->unlock();

}

static void xTextVisUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args )
{

pvValType pvV;
class activeXTextClass *axto = (activeXTextClass *) clientData;

  axto->actWin->appCtx->proc->lock();

  if ( !axto->activeMode ) {
    axto->actWin->appCtx->proc->unlock();
    return;
  }

  pvV.d = *( (double *) classPtr->getValue( args ) );
  if ( ( pvV.d >= axto->minVis.d ) && ( pvV.d < axto->maxVis.d ) )
    axto->visibility = 1 ^ axto->visInverted;
  else
    axto->visibility = 0 ^ axto->visInverted;

  if ( axto->active ) {

    if ( axto->visibility ) {

      axto->needRefresh = 1;

    }
    else {

      axto->needErase = 1;
      axto->needRefresh = 1;

    }

    axto->actWin->addDefExeNode( axto->aglPtr );

  }

  axto->actWin->appCtx->proc->unlock();

}

static void axtc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeXTextClass *axto = (activeXTextClass *) client;

  axto->actWin->setChanged();

  axto->eraseSelectBoxCorners();
  axto->erase();

  strncpy( axto->id, axto->bufId, 31 );

  axto->fgColorMode = axto->bufFgColorMode;
  if ( axto->fgColorMode == AXTC_K_COLORMODE_ALARM )
    axto->fgColor.setAlarmSensitive();
  else
    axto->fgColor.setAlarmInsensitive();
  axto->fgColor.setColor( axto->bufFgColor, axto->actWin->ci );

  axto->bgColorMode = axto->bufBgColorMode;
  if ( axto->bgColorMode == AXTC_K_COLORMODE_ALARM )
    axto->bgColor.setAlarmSensitive();
  else
    axto->bgColor.setAlarmInsensitive();
  axto->bgColor.setColor( axto->bufBgColor, axto->actWin->ci );

  axto->alarmPvExpStr.setRaw( axto->bufAlarmPvName );

  axto->visPvExpStr.setRaw( axto->bufVisPvName );

  strncpy( axto->pvUserClassName, 
    axto->actWin->pvObj.getPvName(axto->pvNameIndex), 15);
  strncpy( axto->pvClassName, 
    axto->actWin->pvObj.getPvClassName(axto->pvNameIndex), 15);

  if ( axto->bufVisInverted )
    axto->visInverted = 0;
  else
    axto->visInverted = 1;

  strncpy( axto->minVisString, axto->bufMinVisString, 39 );
  strncpy( axto->maxVisString, axto->bufMaxVisString, 39 );

  axto->value.setRaw( axto->bufValue );

  strncpy( axto->fontTag, axto->fm.currentFontTag(), 63 );
  axto->actWin->fi->loadFontTag( axto->fontTag );
  axto->actWin->drawGc.setFontTag( axto->fontTag, axto->actWin->fi );

  axto->stringLength = strlen( axto->value.getRaw() );

  axto->fs = axto->actWin->fi->getXFontStruct( axto->fontTag );

  axto->updateFont( axto->value.getRaw(), axto->fontTag, &axto->fs,
   &axto->fontAscent, &axto->fontDescent, &axto->fontHeight,
   &axto->stringWidth );

  axto->stringY = axto->y + axto->fontAscent;

  axto->alignment = axto->fm.currentFontAlignment();

  if ( axto->alignment == XmALIGNMENT_BEGINNING )
    axto->stringX = axto->x;
  else if ( axto->alignment == XmALIGNMENT_CENTER )
    axto->stringX = axto->x + axto->w/2 - axto->stringWidth/2;
  else if ( axto->alignment == XmALIGNMENT_END )
    axto->stringX = axto->x + axto->w - axto->stringWidth;

  axto->useDisplayBg = axto->bufUseDisplayBg;

  axto->autoSize = axto->bufAutoSize;

  axto->x = axto->bufX;
  axto->sboxX = axto->bufX;

  axto->y = axto->bufY;
  axto->sboxY = axto->bufY;

  axto->w = axto->bufW;
  axto->sboxW = axto->bufW;

  axto->h = axto->bufH;
  axto->sboxH = axto->bufH;

  axto->updateDimensions();

  if ( axto->autoSize && axto->fs ) {
    axto->w = XTextWidth( axto->fs, axto->value.getRaw(), axto->stringLength );
    axto->sboxW = axto->w;
    axto->h = axto->fs->ascent + axto->fs->descent;
    axto->sboxH = axto->h;
  }

}

static void axtc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeXTextClass *axto = (activeXTextClass *) client;

  axtc_edit_update( w, client, call );
  axto->refresh( axto );

}

static void axtc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeXTextClass *axto = (activeXTextClass *) client;

  axtc_edit_apply ( w, client, call );
  axto->ef.popdown();
  axto->operationComplete();

}

static void axtc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeXTextClass *axto = (activeXTextClass *) client;

  axto->ef.popdown();
  axto->operationCancel();

}

static void axtc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeXTextClass *axto = (activeXTextClass *) client;

  axto->ef.popdown();
  axto->operationCancel();
  axto->erase();
  axto->deleteRequest = 1;
  axto->drawAll();

}

activeXTextClass::activeXTextClass ( void ) {

  name = new char[strlen("activeXTextClass")+1];
  strcpy( name, "activeXTextClass" );

  visibility = 0;
  prevVisibility = -1;
  visInverted = 0;
  visPvConnected = alarmPvConnected = 0;
  visPvExists = alarmPvExists = 0;
  active = 0;
  activeMode = 0;
  fgColorMode = AXTC_K_COLORMODE_STATIC;
  bgColorMode = AXTC_K_COLORMODE_STATIC;
  strcpy( minVisString, "" );
  strcpy( maxVisString, "" );
  strcpy( id, "" );
  strcpy( pvClassName, "" );
  strcpy( pvUserClassName, "" );

}

// copy constructor
activeXTextClass::activeXTextClass
 ( const activeXTextClass *source ) {

activeGraphicClass *ago = (activeGraphicClass *) this;

  ago->clone( (activeGraphicClass *) source );

  name = new char[strlen("activeXTextClass")+1];
  strcpy( name, "activeXTextClass" );

  fgColor.copy(source->fgColor);
  bgColor.copy(source->bgColor);
  fgCb = source->fgCb;
  bgCb = source->bgCb;
  fgColorMode = source->fgColorMode;
  bgColorMode = source->bgColorMode;
  visInverted = source->visInverted;

  alarmPvExpStr.setRaw( source->alarmPvExpStr.rawString );
  visPvExpStr.setRaw( source->visPvExpStr.rawString );

  visibility = 0;
  prevVisibility = -1;
  visPvConnected = alarmPvConnected = 0;
  visPvExists = alarmPvExists = 0;
  active = 0;
  activeMode = 0;

  strncpy( minVisString, source->minVisString, 39 );
  strncpy( maxVisString, source->maxVisString, 39 );

  strncpy( pvClassName, source->pvClassName, 15 );
  strncpy( pvUserClassName, source->pvUserClassName, 15 );

  strncpy( id, source->id, 31 );

  useDisplayBg = source->useDisplayBg;

  autoSize = source->autoSize;

  strncpy( fontTag, source->fontTag, 63 );
  strncpy( bufFontTag, source->bufFontTag, 63 );

  fs = actWin->fi->getXFontStruct( fontTag );

  value.copy( source->value );

  alignment = source->alignment;

  stringLength = source->stringLength;
  fontAscent = source->fontAscent;
  fontDescent = source->fontDescent;
  fontHeight = source->fontHeight;
  stringWidth = source->stringWidth;
  stringY = source->stringY;
  stringX = source->stringX;

}

int activeXTextClass::createInteractive (
  activeWindowClass *aw_obj,
  int _x,
  int _y,
  int _w,
  int _h ) {

int stat = 1;

  actWin = (activeWindowClass *) aw_obj;
  xOrigin = 0;
  yOrigin = 0;
  x = _x;
  y = _y;
  w = _w;
  h = _h;

  fgColor.setColor( actWin->defaultTextFgColor, actWin->ci );
  bgColor.setColor( actWin->defaultBgColor, actWin->ci );

  useDisplayBg = 1;
  autoSize = 1;

  strcpy( fontTag, actWin->defaultFontTag );

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

  alignment = actWin->defaultAlignment;

  strncpy( pvUserClassName, actWin->defaultPvType, 15 );

  this->draw();

  this->editCreate();

  return stat;

}

int activeXTextClass::genericEdit ( void ) {

char title[32], *ptr;

  ptr = actWin->obj.getNameFromClass( "activeXTextClass" );
  if ( ptr )
    strncpy( title, ptr, 31 );
  else
    strncpy( title, "Unknown object", 31 );

  Strncat( title, " Properties", 31 );

  strncpy( bufId, id, 31 );

  bufX = x;
  bufY = y;
  bufW = w;
  bufH = h;

  bufFgColor = fgColor.pixelColor();
  bufFgColorMode = fgColorMode;

  bufBgColor = bgColor.pixelColor();
  bufBgColorMode = bgColorMode;

  if ( alarmPvExpStr.getRaw() )
    strncpy( bufAlarmPvName, alarmPvExpStr.getRaw(), 127 );
  else
    strncpy( bufAlarmPvName, "", 127 );

  if ( visPvExpStr.getRaw() )
    strncpy( bufVisPvName, visPvExpStr.getRaw(), 127 );
  else
    strncpy( bufVisPvName, "", 127 );

  if ( visInverted )
    bufVisInverted = 0;
  else
    bufVisInverted = 1;

  strncpy( bufMinVisString, minVisString, 39 );
  strncpy( bufMaxVisString, maxVisString, 39 );

  strncpy( bufFontTag, fontTag, 63 );
  bufUseDisplayBg = useDisplayBg;
  bufAutoSize = autoSize;

  if ( value.getRaw() )
    strncpy( bufValue, value.getRaw(), 255 );
  else
    strncpy( bufValue, "", 255 );

  ef.create( actWin->top, &actWin->appCtx->entryFormX,
   &actWin->appCtx->entryFormY, &actWin->appCtx->entryFormW,
   &actWin->appCtx->entryFormH, &actWin->appCtx->largestH,
   title, NULL, NULL, NULL );

  ef.addTextField( "ID", 27, bufId, 31 );

  ef.addTextField( "X", 27, &bufX );
  ef.addTextField( "Y", 27, &bufY );
  ef.addTextField( "Width", 27, &bufW );
  ef.addTextField( "Height", 27, &bufH );
  ef.addToggle( "Auto Size", &bufAutoSize );
  ef.addFontMenu( "Font", actWin->fi, &fm, fontTag );
  fm.setFontAlignment( alignment );
  ef.addColorButton( "Fg Color", actWin->ci, &fgCb, &bufFgColor );
  ef.addToggle( "Alarm Sensitive", &bufFgColorMode );
  ef.addToggle( "Use Display Bg", &bufUseDisplayBg );
  ef.addColorButton( "Bg Color", actWin->ci, &bgCb, &bufBgColor );
  ef.addToggle( "Alarm Sensitive", &bufBgColorMode );
  ef.addTextField( "Alarm PV", 27, bufAlarmPvName, 127 );
  ef.addTextField( "Visability PV", 27, bufVisPvName, 127 );
  ef.addOption( " ", "Not Visible if|Visible if", &bufVisInverted );
  ef.addTextField( ">=", 27, bufMinVisString, 39 );
  ef.addTextField( "and <", 27, bufMaxVisString, 39 );
  ef.addTextField( "Value", 27, bufValue, 255 );

  actWin->pvObj.getOptionMenuList( pvOptionList, 255, &numPvTypes );
  if ( numPvTypes == 1 ) {
    pvNameIndex= 0;
  }
  else {
    // printf( "pvUserClassName = [%s]\n", pvUserClassName );
    pvNameIndex = actWin->pvObj.getNameNum( pvUserClassName );
    if ( pvNameIndex < 0 ) pvNameIndex = 0;
    // printf( "pvOptionList = [%s]\n", pvOptionList );
    // printf( "pvNameIndex = %-d\n", pvNameIndex );
    ef.addOption( "PV Type", pvOptionList, &pvNameIndex );
  }

  return 1;

}

int activeXTextClass::editCreate ( void ) {

  this->genericEdit();
  ef.finished( axtc_edit_ok, axtc_edit_apply, axtc_edit_cancel_delete, this );
  actWin->currentEf = NULL;
  ef.popup();

  return 1;

}

int activeXTextClass::edit ( void ) {

  this->genericEdit();
  ef.finished( axtc_edit_ok, axtc_edit_apply, axtc_edit_cancel, this );
  fm.setFontAlignment( alignment );
  actWin->currentEf = &ef;
  ef.popup();

  return 1;

}

int activeXTextClass::createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int r, g, b;
int major, minor, release;
unsigned int pixel;
char oneValue[255+1];
int stat = 1;

  this->actWin = _actWin;

  fscanf( f, "%d %d %d\n", &major, &minor, &release ); actWin->incLine();

  fscanf( f, "%d\n", &x ); actWin->incLine();
  fscanf( f, "%d\n", &y ); actWin->incLine();
  fscanf( f, "%d\n", &w ); actWin->incLine();
  fscanf( f, "%d\n", &h ); actWin->incLine();

  this->initSelectBox(); // call after getting x,y,w,h

  fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
  if ( ( major < 2 ) && ( minor < 3 ) ) {
    r *= 256;
    g *= 256;
    b *= 256;
  }
  actWin->ci->setRGB( r, g, b, &pixel );
  fgColor.setColor( pixel, actWin->ci );

  fscanf( f, "%d\n", &fgColorMode ); actWin->incLine();

  if ( fgColorMode == AXTC_K_COLORMODE_ALARM )
    fgColor.setAlarmSensitive();
  else
    fgColor.setAlarmInsensitive();

  fscanf( f, "%d\n", &useDisplayBg ); actWin->incLine();

  fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
  if ( ( major < 2 ) && ( minor < 3 ) ) {
    r *= 256;
    g *= 256;
    b *= 256;
  }
  actWin->ci->setRGB( r, g, b, &pixel );
  bgColor.setColor( pixel, actWin->ci );

  fscanf( f, "%d\n", &bgColorMode ); actWin->incLine();

  if ( bgColorMode == AXTC_K_COLORMODE_ALARM )
    bgColor.setAlarmSensitive();
  else
    bgColor.setAlarmInsensitive();

  readStringFromFile( oneValue, 127, f ); actWin->incLine();
  alarmPvExpStr.setRaw( oneValue );

  readStringFromFile( oneValue, 127, f ); actWin->incLine();
  visPvExpStr.setRaw( oneValue );

  fscanf( f, "%d\n", &visInverted ); actWin->incLine();

  if ( ( major > 1 ) || ( minor > 0 ) ) {
    readStringFromFile( minVisString, 39, f ); actWin->incLine();
    readStringFromFile( maxVisString, 39, f ); actWin->incLine();
  }
  else {
    strcpy( minVisString, "1" );
    strcpy( maxVisString, "1" );
  }

  readStringFromFile( oneValue, 255, f ); actWin->incLine();
  value.setRaw( oneValue );

  readStringFromFile( fontTag, 63, f ); actWin->incLine();

  fscanf( f, "%d\n", &alignment ); actWin->incLine();

  if ( ( major > 1 ) || ( minor > 1 ) ) {
    fscanf( f, "%d\n", &autoSize ); actWin->incLine();
  }
  else {
    autoSize = 0;
  }

  if ( ( major > 1 ) || ( minor > 3 ) ) {
    readStringFromFile( this->id, 31, f ); actWin->incLine();
  }
  else {
    strcpy( this->id, "" );
  }

  actWin->fi->loadFontTag( fontTag );
  actWin->drawGc.setFontTag( fontTag, actWin->fi );

  if ( value.getRaw() )
    stringLength = strlen( value.getRaw() );
  else
    stringLength = 0;

  fs = actWin->fi->getXFontStruct( fontTag );

  if ( value.getRaw() )
    updateFont( value.getRaw(), fontTag, &fs, &fontAscent, &fontDescent,
     &fontHeight, &stringWidth );
  else
    updateFont( " ", fontTag, &fs, &fontAscent, &fontDescent,
     &fontHeight, &stringWidth );

  if ( ( major > 1 ) || ( minor > 4 ) ) {

    readStringFromFile( pvClassName, 15, f ); actWin->incLine();

     strncpy( pvUserClassName, actWin->pvObj.getNameFromClass( pvClassName ),
     15 );

  }

  stringY = y + fontAscent;

  if ( alignment == XmALIGNMENT_BEGINNING )
    stringX = x;
  else if ( alignment == XmALIGNMENT_CENTER )
    stringX = x + w/2 - stringWidth/2;
  else if ( alignment == XmALIGNMENT_END )
    stringX = x + w - stringWidth;

  return stat;

}

int activeXTextClass::importFromXchFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int r, g, b, more;
unsigned int pixel;
int stat = 1;
char *tk, *gotData, *context, oneValue[255+1], buf[255+1];

  r = 0xffff;
  g = 0xffff;
  b = 0xffff;

  this->actWin = _actWin;

  fgColor.setColor( actWin->defaultTextFgColor, actWin->ci );
  bgColor.setColor( actWin->defaultBgColor, actWin->ci );

  useDisplayBg = 1;
  autoSize = 1;

  strcpy( fontTag, actWin->defaultFontTag );

  alignment = actWin->defaultAlignment;

  // continue until tag is <eod>

  do {

    gotData = getNextDataString( buf, 255, f );
    buf[255] = 0;
    if ( !gotData ) {
      actWin->appCtx->postMessage( "import file syntax error" );
      return 0;
    }

    context = NULL;

    tk = strtok_r( buf, " \t\n", &context );
    if ( !tk ) {
      actWin->appCtx->postMessage( "import file syntax error" );
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
          actWin->appCtx->postMessage( "import file syntax error" );
          return 0;
        }

        x = atol( tk );

      }
      else if ( strcmp( tk, "y" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( "import file syntax error" );
          return 0;
        }

        y = atol( tk );

      }
      else if ( strcmp( tk, "w" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( "import file syntax error" );
          return 0;
        }

        w = atol( tk );

      }
      else if ( strcmp( tk, "h" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( "import file syntax error" );
          return 0;
        }

        h = atol( tk );

      }
            
      else if ( strcmp( tk, "value" ) == 0 ) {

        tk = strtok_r( NULL, "\"", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( "import file syntax error" );
          return 0;
        }

        strncpy( oneValue, tk, 255 );
        oneValue[255] = 0;

      }
            
      else if ( strcmp( tk, "font" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( "import file syntax error" );
          return 0;
        }

        strncpy( fontTag, tk, 63 );

      }
            
      else if ( strcmp( tk, "justify" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( "import file syntax error" );
          return 0;
        }

        alignment = atol( tk );

      }
            
      else if ( strcmp( tk, "red" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( "import file syntax error" );
          return 0;
        }

        r = atol( tk );

      }
            
      else if ( strcmp( tk, "green" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( "import file syntax error" );
          return 0;
        }

        g = atol( tk );

      }
            
      else if ( strcmp( tk, "blue" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( "import file syntax error" );
          return 0;
        }

        b = atol( tk );

      }
            
    }

  } while ( more );

  actWin->ci->setRGB( r, g, b, &pixel );
  fgColor.setColor( pixel, actWin->ci );

  fgColorMode = 0; // alarm insensitive
  fgColor.setAlarmInsensitive();

  bgColorMode = 0; // alarm insensitive
  bgColor.setAlarmInsensitive();

  alarmPvExpStr.setRaw( "" );

  visPvExpStr.setRaw( "" );

  visInverted = 0;

  strcpy( minVisString, "1" );
  strcpy( maxVisString, "1" );

  value.setRaw( oneValue );

  if ( value.getRaw() )
    stringLength = strlen( value.getRaw() );
  else
    stringLength = 0;

  actWin->fi->loadFontTag( fontTag );
  actWin->drawGc.setFontTag( fontTag, actWin->fi );

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

  if ( value.getRaw() )
    updateFont( value.getRaw(), fontTag, &fs, &fontAscent, &fontDescent,
     &fontHeight, &stringWidth );
  else
    updateFont( " ", fontTag, &fs, &fontAscent, &fontDescent,
     &fontHeight, &stringWidth );

  y = y + fontDescent;

  this->initSelectBox(); // call after getting x,y,w,h

  if ( alignment == XmALIGNMENT_BEGINNING )
    stringX = x;
  else if ( alignment == XmALIGNMENT_CENTER )
    stringX = x + w/2 - stringWidth/2;
  else if ( alignment == XmALIGNMENT_END )
    stringX = x + w - stringWidth;

  stringY = y + fontAscent;

  return stat;

}

int activeXTextClass::save (
  FILE *f )
{

int r, g, b;

  fprintf( f, "%-d %-d %-d\n", AXTC_MAJOR_VERSION, AXTC_MINOR_VERSION,
   AXTC_RELEASE );
  fprintf( f, "%-d\n", x );
  fprintf( f, "%-d\n", y );
  fprintf( f, "%-d\n", w );
  fprintf( f, "%-d\n", h );

  actWin->ci->getRGB( fgColor.pixelColor(), &r, &g, &b );
  fprintf( f, "%-d %-d %-d\n", r, g, b );

  fprintf( f, "%-d\n", fgColorMode );

  fprintf( f, "%-d\n", useDisplayBg );

  actWin->ci->getRGB( bgColor.pixelColor(), &r, &g, &b );
  fprintf( f, "%-d %-d %-d\n", r, g, b );

  fprintf( f, "%-d\n", bgColorMode );

  if ( alarmPvExpStr.getRaw() )
    writeStringToFile( f, alarmPvExpStr.getRaw() );
  else
    writeStringToFile( f, "" );

  if ( visPvExpStr.getRaw() )
    writeStringToFile( f, visPvExpStr.getRaw() );
  else
    writeStringToFile( f, "" );

  fprintf( f, "%-d\n", visInverted );
  writeStringToFile( f, minVisString );
  writeStringToFile( f, maxVisString );

  if ( value.getRaw() )
    writeStringToFile( f, value.getRaw() );
  else
    writeStringToFile( f, "" );

  writeStringToFile( f, fontTag );

  fprintf( f, "%-d\n", alignment );

  fprintf( f, "%-d\n", autoSize );

  // version 1.4.0
  writeStringToFile( f, this->id );

  // version 1.5.0
  writeStringToFile( f, pvClassName );

  return 1;

}

int activeXTextClass::drawActive ( void ) {

XRectangle xR = { x, y, w, h };
int clipStat;

  if ( !activeMode || !visibility ) return 1;

  prevVisibility = visibility;

  actWin->executeGc.saveFg();

  actWin->executeGc.setFG( fgColor.getColor() );

  clipStat = actWin->executeGc.addNormXClipRectangle( xR );

  if ( strcmp( fontTag, "" ) != 0 ) {
    actWin->executeGc.setFontTag( fontTag, actWin->fi );
  }

  if ( useDisplayBg ) {

    XDrawStrings( actWin->d, XtWindow(actWin->executeWidget),
     actWin->executeGc.normGC(), stringX, stringY, fontHeight,
     value.getExpanded(), stringLength );

  }
  else {

    actWin->executeGc.saveBg();
    actWin->executeGc.setBG( bgColor.getColor() );

    XDrawImageStrings( actWin->d, XtWindow(actWin->executeWidget),
     actWin->executeGc.normGC(), stringX, stringY, fontHeight,
     value.getExpanded(), stringLength );

    actWin->executeGc.restoreBg();

  }

  if ( clipStat & 1 ) actWin->executeGc.removeNormXClipRectangle();

  actWin->executeGc.restoreFg();

  return 1;

}

int activeXTextClass::eraseUnconditional ( void ) {

XRectangle xR = { x, y, w, h };

  actWin->executeGc.addEraseXClipRectangle( xR );

  if ( strcmp( fontTag, "" ) != 0 ) {
    actWin->executeGc.setFontTag( fontTag, actWin->fi );
  }

  if ( useDisplayBg ) {

    XDrawStrings( actWin->d, XtWindow(actWin->executeWidget),
     actWin->executeGc.eraseGC(), stringX, stringY, fontHeight,
     value.getExpanded(), stringLength );

  }
  else {

    XDrawImageStrings( actWin->d, XtWindow(actWin->executeWidget),
     actWin->executeGc.eraseGC(), stringX, stringY, fontHeight,
     value.getExpanded(), stringLength );

  }

  actWin->executeGc.removeEraseXClipRectangle();

  return 1;

}

int activeXTextClass::eraseActive ( void ) {

XRectangle xR = { x, y, w, h };

  if ( !activeMode ) return 1;

  if ( prevVisibility == 0 ) {
    prevVisibility = visibility;
    return 1;
  }

  prevVisibility = visibility;

  if ( strcmp( fontTag, "" ) != 0 ) {
    actWin->executeGc.setFontTag( fontTag, actWin->fi );
  }

  if ( useDisplayBg ) {

    actWin->executeGc.addEraseXClipRectangle( xR );

    XDrawStrings( actWin->d, XtWindow(actWin->executeWidget),
     actWin->executeGc.eraseGC(), stringX, stringY, fontHeight,
     value.getExpanded(), stringLength );

    actWin->executeGc.removeEraseXClipRectangle();

  }
  else {

    actWin->executeGc.addNormXClipRectangle( xR );

    actWin->executeGc.saveFg();
    actWin->executeGc.saveBg();

    actWin->executeGc.setFG( bgColor.getColor() );
    actWin->executeGc.setBG( bgColor.getColor() );

    XDrawImageStrings( actWin->d, XtWindow(actWin->executeWidget),
     actWin->executeGc.normGC(), stringX, stringY, fontHeight,
     value.getExpanded(), stringLength );

    actWin->executeGc.restoreFg();
    actWin->executeGc.restoreBg();

    actWin->executeGc.removeNormXClipRectangle();

  }

  return 1;

}

int activeXTextClass::expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int stat;

  stat = alarmPvExpStr.expand1st( numMacros, macros, expansions );
  stat = visPvExpStr.expand1st( numMacros, macros, expansions );
  stat = value.expand1st( numMacros, macros, expansions );

  return stat;

}

int activeXTextClass::expand2nd (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int stat;

  stat = alarmPvExpStr.expand2nd( numMacros, macros, expansions );
  stat = visPvExpStr.expand2nd( numMacros, macros, expansions );
  stat = value.expand2nd( numMacros, macros, expansions );

  return stat;

}

int activeXTextClass::containsMacros ( void ) {

  if ( alarmPvExpStr.containsPrimaryMacros() ) return 1;
  if ( visPvExpStr.containsPrimaryMacros() ) return 1;
  if ( value.containsPrimaryMacros() ) return 1;

  return 0;

}

int activeXTextClass::activate (
  int pass,
  void *ptr )
{

int stat;

  switch ( pass ) {

  case 1: // initialize

    needVisConnectInit = 0;
    needAlarmConnectInit = 0;
    needErase = needDraw = needRefresh = needPropertyUpdate = 0;
    aglPtr = ptr;
    opComplete = 0;

    alarmEventId = NULL;
    visEventId = NULL;

    alarmPvConnected = visPvConnected = 0;

    actWin->appCtx->proc->lock();
    active = 0;
    actWin->appCtx->proc->unlock();

    activeMode = 1;
    prevVisibility = -1;

    stringLength = strlen( value.getExpanded() );

    updateFont( value.getExpanded(), fontTag, &fs, &fontAscent, &fontDescent,
     &fontHeight, &stringWidth );

    stringY = y + fontAscent;

    if ( alignment == XmALIGNMENT_BEGINNING )
      stringX = x;
    else if ( alignment == XmALIGNMENT_CENTER )
      stringX = x + w/2 - stringWidth/2;
    else if ( alignment == XmALIGNMENT_END )
      stringX = x + w - stringWidth;

    init = 1;
    active = 1;

    if ( !alarmPvExpStr.getExpanded() ||
         ( strcmp( alarmPvExpStr.getExpanded(), "" ) == 0 ) ) {
      alarmPvExists = 0;
    }
    else {
      alarmPvExists = 1;
      fgColor.setConnectSensitive();
      bgColor.setConnectSensitive();
      init = 0;
      active = 0;
    }

    if ( !visPvExpStr.getExpanded() ||
         ( strcmp( visPvExpStr.getExpanded(), "" ) == 0 ) ) {
      visPvExists = 0;
      visibility = 1;
    }
    else {
      visPvExists = 1;
      visibility = 0;
      fgColor.setConnectSensitive();
      bgColor.setConnectSensitive();
      init = 0;
      active = 0;
    }

    break;

  case 2: // connect to pv's

    if ( !opComplete ) {

      if ( alarmPvExists ) {

          // printf( "pvNameIndex = %-d\n", pvNameIndex );
          // printf( "pv class name = [%s]\n", pvClassName );
          // printf( "pvOptionList = [%s]\n", pvOptionList );

          alarmPvId = actWin->pvObj.createNew( pvClassName );
          if ( !alarmPvId ) {
            printf( "Cannot create %s object", pvClassName );
            // actWin->appCtx->postMessage( msg );
            opComplete = 1;
            return 1;
	  }
          alarmPvId->createEventId( &alarmEventId );
      
        stat = alarmPvId->searchAndConnect( &alarmPvExpStr,
	 axtoMonitorAlarmPvConnectState, this );
	if (stat != PV_E_SUCCESS ) {
	  printf( "error from searchAndConnect\n" );
	  return 0;
	}
      }

      if ( visPvExists ) {

          // printf( "pvNameIndex = %-d\n", pvNameIndex );
          // printf( "pv class name = [%s]\n", pvClassName );
          // printf( "pvOptionList = [%s]\n", pvOptionList );

          visPvId = actWin->pvObj.createNew( pvClassName );
          if ( !visPvId ) {
            printf( "Cannot create %s object", pvClassName );
            // actWin->appCtx->postMessage( msg );
            opComplete = 1;
            return 1;
	  }
          visPvId->createEventId( &visEventId );
      
	stat = visPvId->searchAndConnect( &visPvExpStr,
	 axtoMonitorVisPvConnectState, this );
        if ( stat != PV_E_SUCCESS ) {
          printf( "error from searchAndConect\n" );
          return 0;
        }
      }

      opComplete = 1;
      this->bufInvalidate();

    }

    break;

  case 3:
  case 4:
  case 5:
  case 6:

    break;

  }

  return 1;

}

int activeXTextClass::deactivate (
  int pass )
{

int stat;

  if ( pass == 1 ) {

    actWin->appCtx->proc->lock();

    activeMode = 0;

    if ( value.getRaw() )
      stringLength = strlen( value.getRaw() );
    else
      stringLength = 0;

    if ( value.getRaw() )
      updateFont( value.getRaw(), fontTag, &fs, &fontAscent, &fontDescent,
       &fontHeight, &stringWidth );
    else
      updateFont( " ", fontTag, &fs, &fontAscent, &fontDescent,
       &fontHeight, &stringWidth );

    stringY = y + fontAscent;

    if ( alignment == XmALIGNMENT_BEGINNING )
      stringX = x;
    else if ( alignment == XmALIGNMENT_CENTER )
      stringX = x + w/2 - stringWidth/2;
    else if ( alignment == XmALIGNMENT_END )
      stringX = x + w - stringWidth;

    if ( alarmPvExists ) {
      stat = alarmPvId->clearChannel();
      if ( stat != PV_E_SUCCESS )
        printf( "clearChannel failure\n" );

      stat = alarmPvId->destroyEventId( &alarmEventId );

      delete alarmPvId;

      alarmPvId = NULL;

    }

    if ( visPvExists ) {
      stat = visPvId->clearChannel();
      if ( stat != PV_E_SUCCESS )
        printf( "clearChannel failure\n" );

      stat = visPvId->destroyEventId( &alarmEventId );

      delete visPvId;

      visPvId = NULL;

    }

    actWin->appCtx->proc->unlock();

  }

  return 1;

}

int activeXTextClass::draw ( void ) {

XRectangle xR = { x, y, w, h };
int clipStat;

  if ( activeMode ) return 1;
  if ( deleteRequest ) return 1;

  actWin->drawGc.saveFg();
  actWin->drawGc.saveBg();

  actWin->drawGc.setFG( fgColor.pixelColor() );
  actWin->drawGc.setBG( bgColor.pixelColor() );

  clipStat = actWin->drawGc.addNormXClipRectangle( xR );

  if ( strcmp( fontTag, "" ) != 0 ) {
    actWin->drawGc.setFontTag( fontTag, actWin->fi );
  }

  if ( useDisplayBg ) {

    if ( value.getRaw() ) {
      XDrawStrings( actWin->d, XtWindow(actWin->drawWidget),
       actWin->drawGc.normGC(), stringX, stringY, fontHeight,
       value.getRaw(), stringLength );
    }

  }
  else {

    if ( value.getRaw() ) {
      XDrawImageStrings( actWin->d, XtWindow(actWin->drawWidget),
       actWin->drawGc.normGC(), stringX, stringY, fontHeight,
       value.getRaw(), stringLength );
    }

  }

  if ( clipStat & 1 )
    actWin->drawGc.removeNormXClipRectangle();
  else
    printf( "clipStat = %-d\n", clipStat );

  actWin->drawGc.restoreFg();
  actWin->drawGc.restoreBg();

  return 1;

}

int activeXTextClass::erase ( void ) {

XRectangle xR = { x, y, w, h };

  if ( activeMode ) return 1;
  if ( deleteRequest ) return 1;

  actWin->drawGc.addEraseXClipRectangle( xR );

  if ( strcmp( fontTag, "" ) != 0 ) {
    actWin->drawGc.setFontTag( fontTag, actWin->fi );
  }

  if ( useDisplayBg ) {

    if ( value.getRaw() ) {
      XDrawStrings( actWin->d, XtWindow(actWin->drawWidget),
       actWin->drawGc.eraseGC(), stringX, stringY, fontHeight,
       value.getRaw(), stringLength );
    }

  }
  else {

    if ( value.getRaw() ) {
      XDrawImageStrings( actWin->d, XtWindow(actWin->drawWidget),
       actWin->drawGc.eraseGC(), stringX, stringY, fontHeight,
       value.getRaw(), stringLength );
    }

  }

  actWin->drawGc.removeEraseXClipRectangle();

  return 1;

}

void activeXTextClass::updateDimensions ( void )
{

  stringY = y + fontAscent;

  if ( alignment == XmALIGNMENT_BEGINNING )
    stringX = x;
  else if ( alignment == XmALIGNMENT_CENTER )
    stringX = x + w/2 - stringWidth/2;
  else if ( alignment == XmALIGNMENT_END )
    stringX = x + w - stringWidth;

  if ( activeMode ) {
    if ( value.getExpanded() )
      stringLength = strlen( value.getExpanded() );
    else
      stringLength = 0;
  }
  else {
    if ( value.getRaw() )
      stringLength = strlen( value.getRaw() );
    else
      stringLength = 0;
  }

}

void activeXTextClass::executeDeferred ( void ) {

int stat, nvc, nac, ne, nd, nr, npu;

  if ( actWin->isIconified ) return;

  actWin->appCtx->proc->lock();

  if ( !activeMode ) {
    actWin->remDefExeNode( aglPtr );
    actWin->appCtx->proc->unlock();
    return;
  }

  nvc = needVisConnectInit; needVisConnectInit = 0;
  nac = needAlarmConnectInit; needAlarmConnectInit = 0;
  ne = needErase; needErase = 0;
  nd = needDraw; needDraw = 0;
  nr = needRefresh; needRefresh = 0;
  npu = needPropertyUpdate; needPropertyUpdate = 0;
  actWin->remDefExeNode( aglPtr );

  actWin->appCtx->proc->unlock();

  if ( nvc ) {

    if ( ( visPvId->getType() == visPvId->pvrEnum() ) ||
         ( visPvId->getType() == visPvId->pvrLong() ) ||
         ( visPvId->getType() == visPvId->pvrFloat() ) ||
         ( visPvId->getType() == visPvId->pvrDouble() ) ) {

      visPvConnected = 1;

      pvType = visPvId->getType();

      minVis.d = (double) atof( minVisString );
      maxVis.d = (double) atof( maxVisString );

      if ( ( visPvConnected || !visPvExists ) &&
           ( alarmPvConnected || !alarmPvExists ) ) {

        active = 1;
        fgColor.setConnected();
        bgColor.setConnected();
        bufInvalidate();

        if ( init ) {
          eraseUnconditional();
	}

        init = 1;

        actWin->requestActiveRefresh();

      }

      if ( !visEventId->eventAdded() ) {
        stat = visPvId->addEvent( visPvId->pvrDouble(), 1,
       xTextVisUpdate, (void *) this, visEventId, visPvId->pveValue() );
        if ( stat != PV_E_SUCCESS ) {
          printf( "addEvent failed\n" );
        }
      }
    }
    else { // force a draw in the non-active state

      active = 0;
      fgColor.setDisconnected();
      bgColor.setDisconnected();
      bufInvalidate();
      drawActive();

    }

  }

  if ( nac ) {

    alarmPvConnected = 1;

    if ( ( visPvConnected || !visPvExists ) &&
         ( alarmPvConnected || !alarmPvExists ) ) {

      active = 1;
      fgColor.setConnected();
      bgColor.setConnected();
      bufInvalidate();

      if ( init ) {
        eraseUnconditional();
      }

      init = 1;

      actWin->requestActiveRefresh();

    }

    if ( !alarmEventId->eventAdded() ) {
      stat = alarmPvId->addEvent( alarmPvId->pvrDouble(), 1,
       xTextAlarmUpdate, (void *) this, alarmEventId, alarmPvId->pveValue() );
      if ( stat != PV_E_SUCCESS ) {
        printf( "addEvent failed\n" );
      }
    }
  }

  if ( ne ) {
    eraseActive();
  }

  if ( nd ) {
//      drawActive();
    stat = smartDrawAllActive();
  }

  if ( nr ) {
//      actWin->requestActiveRefresh();
    stat = smartDrawAllActive();
  }

  if ( npu ) {

    eraseActive();

    value.setRaw( bufValue );

    stringLength = strlen( bufValue );

    updateFont( value.getRaw(), fontTag, &fs, &fontAscent, &fontDescent,
     &fontHeight, &stringWidth );

    if ( autoSize && fs ) {
      w = XTextWidth( fs, value.getRaw(), stringLength );
      sboxW = w;
      h = fs->ascent + fs->descent;
      sboxH = h;
    }

    updateDimensions();

    stat = smartDrawAllActive();

  }
}

int activeXTextClass::setProperty (
  char *prop,
  char *_value )
{

  if ( strcmp( prop, "value" ) == 0 ) {

    strncpy( bufValue, _value, 255 );

    actWin->addDefExeNode( aglPtr );

    needPropertyUpdate = 1;

  }

  return 1;

}

#ifdef __cplusplus
extern "C" {
#endif

void *create_activeXTextClassPtr ( void ) {

activeXTextClass *ptr;

  ptr = new activeXTextClass;
  return (void *) ptr;

}

void *clone_activeXTextClassPtr (
  void *_srcPtr )
{

activeXTextClass *ptr, *srcPtr;

  srcPtr = (activeXTextClass *) _srcPtr;

  ptr = new activeXTextClass( srcPtr );

  return (void *) ptr;

}

#ifdef __cplusplus
}
#endif
