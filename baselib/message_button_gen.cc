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

#define __message_button_gen_cc 1

#include "message_button_gen.h"
#include "app_pkg.h"
#include "act_win.h"

#include "thread.h"

#include <unistd.h> // for sleep

static void msgbtc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeMessageButtonClass *msgbto = (activeMessageButtonClass *) client;

  msgbto->actWin->setChanged();

  msgbto->eraseSelectBoxCorners();
  msgbto->erase();

  msgbto->fgColor.setColor( msgbto->bufFgColor, msgbto->actWin->ci );

  msgbto->onColor.setColor( msgbto->bufOnColor, msgbto->actWin->ci );

  msgbto->offColor.setColor( msgbto->bufOffColor, msgbto->actWin->ci );

  msgbto->topShadowColor = msgbto->bufTopShadowColor;
  msgbto->botShadowColor = msgbto->bufBotShadowColor;

  msgbto->destPvExpString.setRaw( msgbto->bufDestPvName );
  msgbto->sourcePressPvExpString.setRaw( msgbto->bufSourcePressPvName );
  msgbto->sourceReleasePvExpString.setRaw( msgbto->bufSourceReleasePvName );

//    strncpy( msgbto->sourcePressPvName, msgbto->bufSourcePressPvName, 39 );
//    strncpy( msgbto->sourceReleasePvName, msgbto->bufSourceReleasePvName, 39 );

  strncpy( msgbto->onLabel, msgbto->bufOnLabel, MAX_ENUM_STRING_SIZE );
  strncpy( msgbto->offLabel, msgbto->bufOffLabel, MAX_ENUM_STRING_SIZE );

  // copy pvClassName from pvObj.  Is this the proper pv object name?
  strncpy( msgbto->pvUserClassName, msgbto->actWin->pvObj.getPvName(msgbto->pvNameIndex), 15);
  strncpy( msgbto->pvClassName, msgbto->actWin->pvObj.getPvClassName(msgbto->pvNameIndex), 15);

  strncpy( msgbto->fontTag, msgbto->fm.currentFontTag(), 63 );
  msgbto->actWin->fi->loadFontTag( msgbto->fontTag );
  msgbto->fs = msgbto->actWin->fi->getXFontStruct( msgbto->fontTag );

  msgbto->toggle = msgbto->bufToggle;

  msgbto->pressAction = msgbto->bufPressAction;
  msgbto->releaseAction = msgbto->bufReleaseAction;

  msgbto->_3D = msgbto->buf3D;

  msgbto->invisible = msgbto->bufInvisible;

  msgbto->x = msgbto->bufX;
  msgbto->sboxX = msgbto->bufX;

  msgbto->y = msgbto->bufY;
  msgbto->sboxY = msgbto->bufY;

  msgbto->w = msgbto->bufW;
  msgbto->sboxW = msgbto->bufW;

  msgbto->h = msgbto->bufH;
  msgbto->sboxH = msgbto->bufH;

  msgbto->updateDimensions();

}

static void msgbtc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeMessageButtonClass *msgbto = (activeMessageButtonClass *) client;

  msgbtc_edit_update( w, client, call );
  msgbto->refresh( msgbto );

}

static void msgbtc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeMessageButtonClass *msgbto = (activeMessageButtonClass *) client;

  msgbtc_edit_apply ( w, client, call );
  msgbto->ef.popdown();
  msgbto->operationComplete();

}

static void msgbtc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeMessageButtonClass *msgbto = (activeMessageButtonClass *) client;

  msgbto->ef.popdown();
  msgbto->operationCancel();

}

static void msgbtc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeMessageButtonClass *msgbto = (activeMessageButtonClass *) client;

  msgbto->ef.popdown();
  msgbto->operationCancel();
  msgbto->erase();
  msgbto->deleteRequest = 1;
  msgbto->drawAll();

}

static void msgbt_monitor_dest_connect_state (
  pvClass *classPtr,
  void *clientData,
  void *args )
{

activeMessageButtonClass *msgbto = (activeMessageButtonClass *) clientData;

  msgbto->actWin->appCtx->proc->lock();

  if ( !msgbto->activeMode ) {
    msgbto->actWin->appCtx->proc->unlock();
    return;
  }

  if ( classPtr->getOp( args ) == classPtr->pvkOpConnUp() ) {

    // Gets pvType, so other functions can use it directly.
    msgbto->destType = msgbto->destPvId->getType();

    msgbto->needConnectInit = 1;

  }
  else {

    msgbto->destPvConnected = 0;
    msgbto->active = 0;
    msgbto->onColor.setDisconnected();
    msgbto->offColor.setDisconnected();
    msgbto->needDraw = 1;

  }

  msgbto->actWin->addDefExeNode( msgbto->aglPtr );
  msgbto->actWin->appCtx->proc->unlock();

}

activeMessageButtonClass::activeMessageButtonClass ( void ) {

  name = new char[strlen("activeMessageButtonClass")+1];
  strcpy( name, "activeMessageButtonClass" );
  buttonPressed = 0;

//    strcpy( sourcePressPvName, "" );
//    strcpy( sourceReleasePvName, "" );

  strcpy( pvClassName, "" );
  strcpy( pvUserClassName, "" );

  strcpy( onLabel, "" );
  strcpy( offLabel, "" );
  toggle = 0;
  pressAction = 0;
  releaseAction = 0;
  _3D = 1;
  invisible = 0;

}

// copy constructor
activeMessageButtonClass::activeMessageButtonClass
 ( const activeMessageButtonClass *source ) {

activeGraphicClass *msgbto = (activeGraphicClass *) this;

  msgbto->clone( (activeGraphicClass *) source );

  name = new char[strlen("activeMessageButtonClass")+1];
  strcpy( name, "activeMessageButtonClass" );

  buttonPressed = 0;

  fgCb = source->fgCb;
  onCb = source->onCb;
  offCb = source->offCb;
  topShadowCb = source->topShadowCb;
  botShadowCb = source->botShadowCb;

  strncpy( fontTag, source->fontTag, 63 );

  fs = actWin->fi->getXFontStruct( fontTag );

  fgColor.copy( source->fgColor );
  offColor.copy( source->offColor );
  onColor.copy( source->onColor );

  topShadowColor = source->topShadowColor;
  botShadowColor = source->botShadowColor;

  destPvExpString.copy( source->destPvExpString );
  sourcePressPvExpString.copy( source->sourcePressPvExpString );
  sourceReleasePvExpString.copy( source->sourceReleasePvExpString );

  strncpy( pvClassName, source->pvClassName, 15 );
  strncpy( pvUserClassName, source->pvUserClassName, 15 );

//    strncpy( sourcePressPvName, source->sourcePressPvName, 39 );
//    strncpy( sourceReleasePvName, source->sourceReleasePvName, 39 );

  strncpy( onLabel, source->onLabel, MAX_ENUM_STRING_SIZE );
  strncpy( offLabel, source->offLabel, MAX_ENUM_STRING_SIZE );

  toggle = source->toggle;
  pressAction = source->pressAction;
  releaseAction = source->releaseAction;
  _3D = source->_3D;
  invisible = source->invisible;

  updateDimensions();

}

int activeMessageButtonClass::createInteractive (
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

  fgColor.setColor( actWin->defaultTextFgColor, actWin->ci );
  onColor.setColor( actWin->defaultBgColor, actWin->ci );
  offColor.setColor( actWin->defaultBgColor, actWin->ci );
  topShadowColor = actWin->defaultTopShadowColor;
  botShadowColor = actWin->defaultBotShadowColor;

//    strcpy( sourcePressPvName, "" );
//    strcpy( sourceReleasePvName, "" );

  strcpy( fontTag, actWin->defaultFontTag );
  actWin->fi->loadFontTag( fontTag );
  fs = actWin->fi->getXFontStruct( fontTag );

  updateDimensions();

  strcpy( onLabel, "" );
  strcpy( offLabel, "" );

  toggle = 0;
  pressAction = 0;
  releaseAction = 0;
  _3D = 1;
  invisible = 0;

  strncpy( pvUserClassName, actWin->defaultPvType, 15 );

  this->draw();

  this->editCreate();

  return 1;

}

int activeMessageButtonClass::save (
  FILE *f )
{

int r, g, b;

  fprintf( f, "%-d %-d %-d\n", MSGBTC_MAJOR_VERSION, MSGBTC_MINOR_VERSION,
   MSGBTC_RELEASE );

  fprintf( f, "%-d\n", x );
  fprintf( f, "%-d\n", y );
  fprintf( f, "%-d\n", w );
  fprintf( f, "%-d\n", h );

  actWin->ci->getRGB( fgColor.pixelColor(), &r, &g, &b );
  fprintf( f, "%-d %-d %-d\n", r, g, b );

  actWin->ci->getRGB( onColor.pixelColor(), &r, &g, &b );
  fprintf( f, "%-d %-d %-d\n", r, g, b );

  actWin->ci->getRGB( offColor.pixelColor(), &r, &g, &b );
  fprintf( f, "%-d %-d %-d\n", r, g, b );

  actWin->ci->getRGB( topShadowColor, &r, &g, &b );
  fprintf( f, "%-d %-d %-d\n", r, g, b );

  actWin->ci->getRGB( botShadowColor, &r, &g, &b );
  fprintf( f, "%-d %-d %-d\n", r, g, b );

  if ( destPvExpString.getRaw() )
    writeStringToFile( f, destPvExpString.getRaw() );
  else
    writeStringToFile( f, "" );

  if ( sourcePressPvExpString.getRaw() )
    writeStringToFile( f, sourcePressPvExpString.getRaw() );
  else
    writeStringToFile( f, "" );

  if ( sourceReleasePvExpString.getRaw() )
    writeStringToFile( f, sourceReleasePvExpString.getRaw() );
  else
    writeStringToFile( f, "" );

//    writeStringToFile( f, sourcePressPvName );
//    writeStringToFile( f, sourceReleasePvName );

  writeStringToFile( f, onLabel );

  writeStringToFile( f, offLabel );

  fprintf( f, "%-d\n", toggle );

  fprintf( f, "%-d\n", pressAction );

  fprintf( f, "%-d\n", releaseAction );

  fprintf( f, "%-d\n", _3D );

  fprintf( f, "%-d\n", invisible );

  writeStringToFile( f, fontTag );

  // version 1.5.0
  writeStringToFile( f, pvClassName );

  return 1;

}

int activeMessageButtonClass::createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int r, g, b;
int major, minor, release;
unsigned int pixel;
char oneName[39+1];

  this->actWin = _actWin;

  fscanf( f, "%d %d %d\n", &major, &minor, &release ); actWin->incLine();

  fscanf( f, "%d\n", &x ); actWin->incLine();
  fscanf( f, "%d\n", &y ); actWin->incLine();
  fscanf( f, "%d\n", &w ); actWin->incLine();
  fscanf( f, "%d\n", &h ); actWin->incLine();

  this->initSelectBox(); // call after getting x,y,w,h

  fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
  if ( ( major < 2 ) && ( minor < 2 ) ) {
    r *= 256;
    g *= 256;
    b *= 256;
  }
  actWin->ci->setRGB( r, g, b, &pixel );
  fgColor.setColor( pixel, actWin->ci );

  fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
  if ( ( major < 2 ) && ( minor < 2 ) ) {
    r *= 256;
    g *= 256;
    b *= 256;
  }
  actWin->ci->setRGB( r, g, b, &pixel );
  onColor.setColor( pixel, actWin->ci );

  fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
  if ( ( major < 2 ) && ( minor < 2 ) ) {
    r *= 256;
    g *= 256;
    b *= 256;
  }
  actWin->ci->setRGB( r, g, b, &pixel );
  offColor.setColor( pixel, actWin->ci );

  fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
  if ( ( major < 2 ) && ( minor < 2 ) ) {
    r *= 256;
    g *= 256;
    b *= 256;
  }
  actWin->ci->setRGB( r, g, b, &topShadowColor );

  fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
  if ( ( major < 2 ) && ( minor < 2 ) ) {
    r *= 256;
    g *= 256;
    b *= 256;
  }
  actWin->ci->setRGB( r, g, b, &botShadowColor );

  readStringFromFile( oneName, 39, f ); actWin->incLine();
  destPvExpString.setRaw( oneName );

  readStringFromFile( oneName, 39, f ); actWin->incLine();
  sourcePressPvExpString.setRaw( oneName );

  readStringFromFile( oneName, 39, f ); actWin->incLine();
  sourceReleasePvExpString.setRaw( oneName );

  readStringFromFile( onLabel, MAX_ENUM_STRING_SIZE, f ); actWin->incLine();

  readStringFromFile( offLabel, MAX_ENUM_STRING_SIZE, f ); actWin->incLine();

  fscanf( f, "%d\n", &toggle ); actWin->incLine();

  if ( ( major > 1 ) || ( minor > 2 ) ) {
    fscanf( f, "%d\n", &pressAction ); actWin->incLine();
    fscanf( f, "%d\n", &releaseAction ); actWin->incLine();
  }
  else {
    pressAction = 0;
    releaseAction = 0;
  }

  fscanf( f, "%d\n", &_3D ); actWin->incLine();

  fscanf( f, "%d\n", &invisible ); actWin->incLine();

  readStringFromFile( fontTag, 63, f ); actWin->incLine();

  actWin->fi->loadFontTag( fontTag );
  fs = actWin->fi->getXFontStruct( fontTag );
  
  if ( ( major > 1 ) || ( minor > 4 ) ) {
    readStringFromFile( pvClassName, 15, f ); actWin->incLine();
    strncpy( pvUserClassName, actWin->pvObj.getNameFromClass( pvClassName ),
      15 );
  }


  updateDimensions();

  return 1;

}

int activeMessageButtonClass::importFromXchFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin ){

int fgR, fgG, fgB, bgR, bgG, bgB, more;
unsigned int pixel;
char *tk, *gotData, *context, buf[255+1];

  fgR = 0xffff;
  fgG = 0xffff;
  fgB = 0xffff;

  bgR = 0xffff;
  bgG = 0xffff;
  bgB = 0xffff;

  this->actWin = _actWin;

  fgColor.setColor( actWin->defaultTextFgColor, actWin->ci );
  onColor.setColor( actWin->defaultBgColor, actWin->ci );
  offColor.setColor( actWin->defaultBgColor, actWin->ci );
  topShadowColor = actWin->defaultTopShadowColor;
  botShadowColor = actWin->defaultBotShadowColor;
  strcpy( fontTag, actWin->defaultFontTag );

  strcpy( onLabel, "" );
  strcpy( offLabel, "" );

  // continue until tag is <eod>

  do {

    gotData = getNextDataString( buf, 255, f );
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
            
      else if ( strcmp( tk, "fgred" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( "import file syntax error" );
          return 0;
        }

        fgR = atol( tk );

      }
            
      else if ( strcmp( tk, "fggreen" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( "import file syntax error" );
          return 0;
        }

        fgG = atol( tk );

      }
            
      else if ( strcmp( tk, "fgblue" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( "import file syntax error" );
          return 0;
        }

        fgB = atol( tk );

      }
            
      else if ( strcmp( tk, "bgred" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( "import file syntax error" );
          return 0;
        }

        bgR = atol( tk );

      }
            
      else if ( strcmp( tk, "bggreen" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( "import file syntax error" );
          return 0;
        }

        bgG = atol( tk );

      }
            
      else if ( strcmp( tk, "bgblue" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( "import file syntax error" );
          return 0;
        }

        bgB = atol( tk );

      }
            
      else if ( strcmp( tk, "closecurrentonpress" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( "import file syntax error" );
          return 0;
        }

        pressAction = atol( tk );

      }
            
      else if ( strcmp( tk, "invisible" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( "import file syntax error" );
          return 0;
        }

        invisible = atol( tk );

      }
            
      else if ( strcmp( tk, "font" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( "import file syntax error" );
          return 0;
        }

        strncpy( fontTag, tk, 63 );

      }

      else if ( strcmp( tk, "pressvalue" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n \t", &context );
        if ( tk ) {
          strncpy( bufSourcePressPvName, tk, 39 );
          bufSourcePressPvName[39] = 0;
          sourcePressPvExpString.setRaw( bufSourcePressPvName );
	}

      }

      else if ( strcmp( tk, "presspv" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n \t", &context );
        if ( tk ) {
          strncpy( bufDestPvName, tk, 28 );
          bufDestPvName[28] = 0;
          destPvExpString.setRaw( bufDestPvName );
	}

      }

      else if ( strcmp( tk, "onlabel" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n \t", &context );
        if ( tk ) {
          strncpy( onLabel, tk, MAX_ENUM_STRING_SIZE );
          onLabel[MAX_ENUM_STRING_SIZE] = 0;
	}

      }

      else if ( strcmp( tk, "offlabel" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n \t", &context );
        if ( tk ) {
          strncpy( offLabel, tk, MAX_ENUM_STRING_SIZE );
          offLabel[MAX_ENUM_STRING_SIZE] = 0;
	}

      }

    }

  } while ( more );

  this->initSelectBox(); // call after getting x,y,w,h

  actWin->ci->setRGB( fgR, fgG, fgB, &pixel );
  fgColor.setColor( pixel, actWin->ci );

  actWin->ci->setRGB( bgR, bgG, bgB, &pixel );
  onColor.setColor( pixel, actWin->ci );
  offColor.setColor( pixel, actWin->ci );

  actWin->fi->loadFontTag( fontTag );
  actWin->drawGc.setFontTag( fontTag, actWin->fi );
  fs = actWin->fi->getXFontStruct( fontTag );

  updateDimensions();

  return 1;

}

int activeMessageButtonClass::genericEdit ( void ) {

char title[32], *ptr;

  ptr = actWin->obj.getNameFromClass( "activeMessageButtonClass" );
  if ( ptr )
    strncpy( title, ptr, 31 );
  else
    strncpy( title, "Unknown object", 31 );

  Strncat( title, " Properties", 31 );

  bufX = x;
  bufY = y;
  bufW = w;
  bufH = h;

  bufFgColor = fgColor.pixelColor();

  bufOnColor = onColor.pixelColor();

  bufOffColor = offColor.pixelColor();

  bufTopShadowColor = topShadowColor;
  bufBotShadowColor = botShadowColor;
  strncpy( bufFontTag, fontTag, 63 );

  if ( destPvExpString.getRaw() )
    strncpy( bufDestPvName, destPvExpString.getRaw(), 39 );
  else
    strncpy( bufDestPvName, "", 39 );

  if ( sourcePressPvExpString.getRaw() )
    strncpy( bufSourcePressPvName, sourcePressPvExpString.getRaw(), 39 );
  else
    strncpy( bufSourcePressPvName, "", 39 );

  if ( sourceReleasePvExpString.getRaw() )
    strncpy( bufSourceReleasePvName, sourceReleasePvExpString.getRaw(), 39 );
  else
    strncpy( bufSourceReleasePvName, "", 39 );

//    strncpy( bufSourcePressPvName, sourcePressPvName, 39 );
//    strncpy( bufSourceReleasePvName, sourceReleasePvName, 39 );

  strncpy( bufOnLabel, onLabel, MAX_ENUM_STRING_SIZE );
  strncpy( bufOffLabel, offLabel, MAX_ENUM_STRING_SIZE );

  bufToggle = toggle;
  bufPressAction = pressAction;
  bufReleaseAction = releaseAction;
  buf3D = _3D;
  bufInvisible = invisible;

  ef.create( actWin->top, actWin->appCtx->ci.getColorMap(),
   &actWin->appCtx->entryFormX,
   &actWin->appCtx->entryFormY, &actWin->appCtx->entryFormW,
   &actWin->appCtx->entryFormH, &actWin->appCtx->largestH,
   title, NULL, NULL, NULL );

  ef.addTextField( "X", 27, &bufX );
  ef.addTextField( "Y", 27, &bufY );
  ef.addTextField( "Width", 27, &bufW );
  ef.addTextField( "Height", 27, &bufH );
  ef.addOption( "Button Type", "Push|Toggle", &bufToggle );
  ef.addToggle( "3-D Look", &buf3D );
  ef.addToggle( "Invisible", &bufInvisible );
  ef.addToggle( "Close Display on Press", &bufPressAction );
  ef.addToggle( "Close Display on Release", &bufReleaseAction );
  ef.addTextField( "Press Label", 27, bufOnLabel, MAX_ENUM_STRING_SIZE );
  ef.addTextField( "Release Label", 27, bufOffLabel, MAX_ENUM_STRING_SIZE );
  ef.addTextField( "Press Value", 27, bufSourcePressPvName, 39 );
  ef.addTextField( "Release Value", 27, bufSourceReleasePvName, 39 );
  ef.addTextField( "Destination", 27, bufDestPvName, 39 );
  ef.addFontMenu( "Label Font", actWin->fi, &fm, fontTag );
  ef.addColorButton( "Fg", actWin->ci, &fgCb, &bufFgColor );
  ef.addColorButton( "On", actWin->ci, &onCb, &bufOnColor );
  ef.addColorButton( "Off", actWin->ci, &offCb, &bufOffColor );
  ef.addColorButton( "Top Shadow", actWin->ci, &topShadowCb, &bufTopShadowColor );
  ef.addColorButton( "Bottom Shadow", actWin->ci, &botShadowCb, &bufBotShadowColor );

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

  XtUnmanageChild( fm.alignWidget() ); // no alignment info

  return 1;

}

int activeMessageButtonClass::editCreate ( void ) {

  this->genericEdit();
  ef.finished( msgbtc_edit_ok, msgbtc_edit_apply, msgbtc_edit_cancel_delete, this );
  actWin->currentEf = NULL;
  ef.popup();

  return 1;

}

int activeMessageButtonClass::edit ( void ) {

  this->genericEdit();
  ef.finished( msgbtc_edit_ok, msgbtc_edit_apply, msgbtc_edit_cancel, this );
  actWin->currentEf = &ef;
  ef.popup();

  return 1;

}

int activeMessageButtonClass::erase ( void ) {

  if ( deleteRequest ) return 1;

  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  return 1;

}

int activeMessageButtonClass::eraseActive ( void ) {

  if ( !init || !activeMode || invisible ) return 1;

  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  return 1;

}

int activeMessageButtonClass::draw ( void ) {

int tX, tY;
XRectangle xR = { x, y, w, h };

  if ( deleteRequest ) return 1;

  actWin->drawGc.saveFg();

  actWin->drawGc.setFG( onColor.pixelColor() );

  XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x, y, w, h );

  if ( !_3D ) {

    actWin->drawGc.setFG( botShadowColor );

  }

  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x, y, w, h );

  if ( _3D ) {

  actWin->drawGc.setFG( botShadowColor );

  XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x, y, x+w, y );

  XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x, y, x, y+h );

   actWin->drawGc.setFG( topShadowColor );

   XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
    actWin->drawGc.normGC(), x, y+h, x+w, y+h );

   XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
    actWin->drawGc.normGC(), x+w, y, x+w, y+h );

  actWin->drawGc.setFG( topShadowColor );

  XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x+1, y+1, x+w-1, y+1 );

  XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x+2, y+2, x+w-2, y+2 );

  XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x+1, y+1, x+1, y+h-1 );

  XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x+2, y+2, x+2, y+h-2 );

  actWin->drawGc.setFG( botShadowColor );

  XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x+1, y+h-1, x+w-1, y+h-1 );

  XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x+2, y+h-2, x+w-2, y+h-2 );

  XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x+w-1, y+1, x+w-1, y+h-1 );

  XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x+w-2, y+2, x+w-2, y+h-2 );

  }

  if ( fs ) {

    actWin->drawGc.addNormXClipRectangle( xR );

    actWin->drawGc.setFG( fgColor.pixelColor() );
    actWin->drawGc.setFontTag( fontTag, actWin->fi );

    tX = x + w/2;
    tY = y + h/2 - fontAscent/2;

    drawText( actWin->drawWidget, &actWin->drawGc, fs, tX, tY,
     XmALIGNMENT_CENTER, onLabel );

    actWin->drawGc.removeNormXClipRectangle();

  }

  actWin->drawGc.restoreFg();

  return 1;

}

int activeMessageButtonClass::drawActive ( void ) {

int tX, tY;
char string[39+1];
XRectangle xR = { x, y, w, h };

  if ( !init || !activeMode || invisible ) return 1;

  actWin->executeGc.saveFg();

  if ( !buttonPressed )
    actWin->executeGc.setFG( offColor.getColor() );
  else
    actWin->executeGc.setFG( onColor.getColor() );

  XFillRectangle( actWin->d, XtWindow(actWin->executeWidget),
   actWin->executeGc.normGC(), x, y, w, h );

  if ( !_3D ) {

    actWin->executeGc.setFG( botShadowColor );

  }

  XDrawRectangle( actWin->d, XtWindow(actWin->executeWidget),
   actWin->executeGc.normGC(), x, y, w, h );

  if ( !buttonPressed ) {

    strncpy( string, offLabel, MAX_ENUM_STRING_SIZE );

    if ( _3D ) {

    actWin->executeGc.setFG( botShadowColor );

    XDrawLine( actWin->d, XtWindow(actWin->executeWidget),
     actWin->executeGc.normGC(), x, y, x+w, y );

    XDrawLine( actWin->d, XtWindow(actWin->executeWidget),
     actWin->executeGc.normGC(), x, y, x, y+h );

    actWin->executeGc.setFG( topShadowColor );

    XDrawLine( actWin->d, XtWindow(actWin->executeWidget),
     actWin->executeGc.normGC(), x, y+h, x+w, y+h );

    XDrawLine( actWin->d, XtWindow(actWin->executeWidget),
     actWin->executeGc.normGC(), x+w, y, x+w, y+h );

    // top
    actWin->executeGc.setFG( topShadowColor );

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
    actWin->executeGc.setFG( botShadowColor );

    XDrawLine( actWin->d, XtWindow(actWin->executeWidget),
     actWin->executeGc.normGC(), x+1, y+h-1, x+w-1, y+h-1 );

    XDrawLine( actWin->d, XtWindow(actWin->executeWidget),
     actWin->executeGc.normGC(), x+2, y+h-2, x+w-2, y+h-2 );

    // right
    XDrawLine( actWin->d, XtWindow(actWin->executeWidget),
     actWin->executeGc.normGC(), x+w-1, y+1, x+w-1, y+h-1 );

    XDrawLine( actWin->d, XtWindow(actWin->executeWidget),
     actWin->executeGc.normGC(), x+w-2, y+2, x+w-2, y+h-2 );

    }

  }
  else {

    strncpy( string, onLabel, MAX_ENUM_STRING_SIZE );

    if ( _3D ) {

    actWin->executeGc.setFG( botShadowColor );

    XDrawLine( actWin->d, XtWindow(actWin->executeWidget),
     actWin->executeGc.normGC(), x, y, x+w, y );

    XDrawLine( actWin->d, XtWindow(actWin->executeWidget),
     actWin->executeGc.normGC(), x, y, x, y+h );

    // top

    actWin->executeGc.setFG( topShadowColor );

    // bottom

    actWin->executeGc.setFG( topShadowColor );

    XDrawLine( actWin->d, XtWindow(actWin->executeWidget),
     actWin->executeGc.normGC(), x, y+h, x+w, y+h );

    //right

    actWin->executeGc.setFG( topShadowColor );

    XDrawLine( actWin->d, XtWindow(actWin->executeWidget),
     actWin->executeGc.normGC(), x+w, y, x+w, y+h );

    }

  }

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

int activeMessageButtonClass::activate (
  int pass,
  void *ptr )
{

int stat, opStat;

  switch ( pass ) {

  case 1:

    needConnectInit = needErase = needDraw = 0;
    init = 0;
    aglPtr = ptr;
    opComplete = 0;

// #ifdef __epics__
//    sourcePressEventId = 0;
//    sourceReleaseEventId = 0;
// #endif

    sourcePressEventId = NULL;
    sourceReleaseEventId = NULL;

    sourcePressExists = sourceReleaseExists = 0;

    destPvConnected = sourcePressPvConnected = sourceReleasePvConnected =
     active = buttonPressed = 0;
    activeMode = 1;

    if ( !destPvExpString.getExpanded() ||
       ( strcmp( destPvExpString.getExpanded(), "" ) == 0 ) ) {
      destExists = 0;
    }
    else {
      destExists = 1;
    }

    break;

  case 2:

    if ( !opComplete ) {

      opStat = 1;

      if ( destExists ) {

        destPvId = actWin->pvObj.createNew( pvClassName );
        if ( !destPvId ) {
          printf( "Cannot create %s object", pvClassName );
          // actWin->appCtx->postMessage( msg );
          opComplete = 1;
          return 1;
	}
        // destPvId->createEventId( &sourcePressEventId );
	// destPvId->createEventId( &sourceReleaseEventId );

        stat = destPvId->searchAndConnect( &destPvExpString,
         msgbt_monitor_dest_connect_state, this );
        if ( stat != PV_E_SUCCESS ) {
          printf( "error from searchAndConnect\n" );
          return 0;
        }

      }
      else {
        init = 1;
        drawActive();
      }

      if ( opStat & 1 ) opComplete = 1;

      return opStat;

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

int activeMessageButtonClass::deactivate (
  int pass
) {

int stat;

  if ( pass == 1 ) {

    active = 0;
    activeMode = 0;

    if ( destExists ) {
      stat = destPvId->clearChannel();
      if ( stat != PV_E_SUCCESS )
        printf( "clearChannel failure\n" );
    }

  }

  return 1;

}

void activeMessageButtonClass::updateDimensions ( void )
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

void activeMessageButtonClass::btnUp (
  int x,
  int y,
  int buttonState,
  int buttonNumber,
  int *action )
{

int stat;
// Generic pvType definitions:
int maxStringSize = destPvId->maxStringSize();
int genericStringType = destPvId->pvrString();
//int genericFloatType = destPvId->pvrFloat();
int genericDoubleType = destPvId->pvrDouble();
//int genericShortType = destPvId->pvrShort();
int genericLongType = destPvId->pvrLong();
int genericEnumType = destPvId->pvrEnum();

  *action = 0;

  if ( toggle ) return;

  *action = releaseAction;

  buttonPressed = 0;
  drawActive();

  if ( strcmp( sourceReleasePvExpString.getExpanded(), "" ) == 0 ) return;

  // switch ( destType ) 

  if (destType == genericDoubleType) {
    destV.d = atof( sourceReleasePvExpString.getExpanded() );
    stat = destPvId->put( destPvId->pvrDouble( ), &destV.d );
    // stat = ca_put( DBR_DOUBLE, destPvId, &destV.d );
  }
  else if (destType == genericLongType) {
    destV.l = atol( sourceReleasePvExpString.getExpanded() );
    stat = destPvId->put( destPvId->pvrLong( ), &destV.l );
    // stat = ca_put( DBR_LONG, destPvId, &destV.l );
  }
  else if (destType == genericStringType) {
    strncpy( destV.str, sourceReleasePvExpString.getExpanded(), maxStringSize );
    stat = destPvId->put( destPvId->pvrString( ), &destV.str );
    // stat = ca_put( DBR_STRING, destPvId, &destV.str );
  }
  else if (destType == genericEnumType) {
    destV.s = (short) atol( sourceReleasePvExpString.getExpanded() );
    stat = destPvId->put( destPvId->pvrEnum( ), &destV.s );
    // stat = ca_put( DBR_ENUM, destPvId, &destV.s );
  }

}

void activeMessageButtonClass::btnDown (
  int x,
  int y,
  int buttonState,
  int buttonNumber,
  int *action )
{

int stat;
char labelValue[39+1];
// Generic pvType definitions:
int maxStringSize = destPvId->maxStringSize();
int genericStringType = destPvId->pvrString();
//int genericFloatType = destPvId->pvrFloat();
int genericDoubleType = destPvId->pvrDouble();
//int genericShortType = destPvId->pvrShort();
int genericLongType = destPvId->pvrLong();
int genericEnumType = destPvId->pvrEnum();

  actWin->appCtx->proc->lock();


  *action = pressAction;

  if ( toggle ) {
    if ( buttonPressed ) {
      buttonPressed = 0;
    }
    else {
      buttonPressed = 1;
    }
  }
  else {
    buttonPressed = 1;
  }

  if ( buttonPressed ) {
    strncpy( labelValue, sourcePressPvExpString.getExpanded(), 39 );
  }
  else {
    strncpy( labelValue, sourceReleasePvExpString.getExpanded(), 39 );
  }

  drawActive();

  if ( strcmp( labelValue, "" ) == 0 ) return;

  // switch ( destType ) 


  if (destType == genericDoubleType) {
    destV.d = atof( labelValue );
    stat = destPvId->put( destPvId->pvrDouble( ), &destV.d );
    // stat = ca_put( DBR_DOUBLE, destPvId, &destV.d );
  }
  else if (destType == genericLongType) {
    destV.l = atol( labelValue );
    stat = destPvId->put( destPvId->pvrLong( ), &destV.l );
    // stat = ca_put( DBR_LONG, destPvId, &destV.l );
  }
  else if (destType == genericStringType) {
    strncpy( destV.str, labelValue, maxStringSize );
    stat = destPvId->put( destPvId->pvrString( ), &destV.str );
    // stat = ca_put( DBR_STRING, destPvId, destV.str );
  }
  else if (destType == genericEnumType) {
    destV.s = (short) atol( labelValue );
    stat = destPvId->put( destPvId->pvrEnum( ), &destV.s );
    // stat = ca_put( DBR_ENUM, destPvId, &destV.s );
  }

  actWin->appCtx->proc->unlock();
}

int activeMessageButtonClass::getButtonActionRequest (
  int *up,
  int *down,
  int *drag,
  int *focus )
{

  *drag = 0;

  if ( destExists )
    *focus = 1;
  else
    *focus = 0;

  if ( !destExists ) {
    *up = 0;
    *down = 0;
    return 1;
  }

  *down = 1;
  *up = 1;

  return 1;

}

int activeMessageButtonClass::expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int stat;

  stat = destPvExpString.expand1st( numMacros, macros, expansions );
  stat = sourcePressPvExpString.expand1st( numMacros, macros, expansions );
  stat = sourceReleasePvExpString.expand1st( numMacros, macros, expansions );

  return stat;

}

int activeMessageButtonClass::expand2nd (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int stat;

  stat = destPvExpString.expand2nd( numMacros, macros, expansions );
  stat = sourcePressPvExpString.expand2nd( numMacros, macros, expansions );
  stat = sourceReleasePvExpString.expand2nd( numMacros, macros, expansions );

  return stat;

}

int activeMessageButtonClass::containsMacros ( void ) {

int stat;

  stat = destPvExpString.containsPrimaryMacros();
  stat = sourcePressPvExpString.containsPrimaryMacros();
  stat = sourceReleasePvExpString.containsPrimaryMacros();

  return stat;

}

void activeMessageButtonClass::executeDeferred ( void ) {

int nc, nd, ne;

  if ( actWin->isIconified ) return;

  actWin->appCtx->proc->lock();
  nc = needConnectInit; needConnectInit = 0;
  nd = needDraw; needDraw = 0;
  ne = needErase; needErase = 0;
  actWin->remDefExeNode( aglPtr );
  actWin->appCtx->proc->unlock();

//----------------------------------------------------------------------------

  if ( nc ) {

    destPvConnected = 1;
    destType = destPvId->getType();
    // destType = ca_field_type( destPvId );

    if ( !(sourcePressExists) && !(sourceReleaseExists) ) {
      active = 1;
      init = 1;
    }

    onColor.setConnected();
    offColor.setConnected();

    drawActive();

  }

//----------------------------------------------------------------------------

  if ( nd ) {

    drawActive();

  }

//----------------------------------------------------------------------------

  if ( ne ) {

    eraseActive();

  }

//----------------------------------------------------------------------------

}

#ifdef __cplusplus
extern "C" {
#endif

void *create_activeMessageButtonClassPtr ( void ) {

activeMessageButtonClass *ptr;

  ptr = new activeMessageButtonClass;
  return (void *) ptr;

}

void *clone_activeMessageButtonClassPtr (
  void *_srcPtr )
{

activeMessageButtonClass *ptr, *srcPtr;

  srcPtr = (activeMessageButtonClass *) _srcPtr;

  ptr = new activeMessageButtonClass( srcPtr );

  return (void *) ptr;

}

#ifdef __cplusplus
}
#endif
