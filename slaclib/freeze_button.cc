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

#define __freeze_button_cc 1

#include "freeze_button.h"
#include "app_pkg.h"
#include "act_win.h"

#include "thread.h"

static void doBlink (
  void *ptr
) {

    activeFreezeButtonClass *fb = (activeFreezeButtonClass *) ptr;
    fb->bufInvalidate();
    fb->drawActive();

}

static void fbtc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeFreezeButtonClass *fbto = (activeFreezeButtonClass *) client;

  fbto->actWin->setChanged();

  fbto->eraseSelectBoxCorners();
  fbto->erase();

  fbto->fgColor = fbto->bufFgColor;
  fbto->bgColor = fbto->bufBgColor;
  fbto->frozenBgColor = fbto->bufFrozenBgColor;
  fbto->topShadowColor = fbto->bufTopShadowColor;
  fbto->botShadowColor = fbto->bufBotShadowColor;

  strncpy( fbto->fontTag, fbto->fm.currentFontTag(), 63 );
  fbto->actWin->fi->loadFontTag( fbto->fontTag );
  fbto->fs = fbto->actWin->fi->getXFontStruct( fbto->fontTag );

  fbto->_3D = fbto->buf3D;

  strncpy( fbto->label, fbto->bufLabel, 31 );
  strncpy( fbto->frozenLabel, fbto->bufFrozenLabel, 31 );

  fbto->x = fbto->bufX;
  fbto->sboxX = fbto->bufX;

  fbto->y = fbto->bufY;
  fbto->sboxY = fbto->bufY;

  fbto->w = fbto->bufW;
  fbto->sboxW = fbto->bufW;

  fbto->h = fbto->bufH;
  fbto->sboxH = fbto->bufH;

  fbto->updateDimensions();

}

static void fbtc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeFreezeButtonClass *fbto = (activeFreezeButtonClass *) client;

  fbtc_edit_update( w, client, call );
  fbto->refresh( fbto );

}

static void fbtc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeFreezeButtonClass *fbto = (activeFreezeButtonClass *) client;

  fbtc_edit_update( w, client, call );
  fbto->ef.popdown();
  fbto->operationComplete();

}

static void fbtc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeFreezeButtonClass *fbto = (activeFreezeButtonClass *) client;

  fbto->ef.popdown();
  fbto->operationCancel();

}

static void fbtc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeFreezeButtonClass *fbto = (activeFreezeButtonClass *) client;

  fbto->ef.popdown();
  fbto->operationCancel();
  fbto->erase();
  fbto->deleteRequest = 1;
  fbto->drawAll();

}

activeFreezeButtonClass::activeFreezeButtonClass ( void ) {

  name = new char[strlen("activeFreezeButtonClass")+1];
  strcpy( name, "activeFreezeButtonClass" );
  checkBaseClassVersion( activeGraphicClass::MAJOR_VERSION, name );
  deleteRequest = 0;
  selected = 0;
  setBlinkFunction((void *)doBlink);
}

// copy constructor
activeFreezeButtonClass::activeFreezeButtonClass
 ( const activeFreezeButtonClass *source ) {

activeGraphicClass *fbto = (activeGraphicClass *) this;

  fbto->clone( (activeGraphicClass *) source );

  name = new char[strlen("activeFreezeButtonClass")+1];
  strcpy( name, "activeFreezeButtonClass" );

  deleteRequest = 0;

  fgCb = source->fgCb;
  bgCb = source->bgCb;
  frozenBgCb = source->frozenBgCb;
  topShadowCb = source->topShadowCb;
  botShadowCb = source->botShadowCb;

  strncpy( fontTag, source->fontTag, 63 );

  fs = actWin->fi->getXFontStruct( fontTag );

  fgColor = source->fgColor;
  bgColor = source->bgColor;
  frozenBgColor = source->frozenBgColor;
  topShadowColor = source->topShadowColor;
  botShadowColor = source->botShadowColor;

  _3D = source->_3D;
  strncpy( label, source->label, 31 );
  strncpy( frozenLabel, source->frozenLabel, 31 );

  doAccSubs( label, 31 );
  doAccSubs( frozenLabel, 31 );

  updateDimensions();
    
  setBlinkFunction((void *)doBlink);
}

int activeFreezeButtonClass::createInteractive (
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

  fgColor = actWin->defaultTextFgColor;
  bgColor = actWin->defaultBgColor;
  frozenBgColor = 77; //blinking
  topShadowColor = actWin->defaultTopShadowColor;
  botShadowColor = actWin->defaultBotShadowColor;

  strcpy( fontTag, actWin->defaultBtnFontTag );
  actWin->fi->loadFontTag( fontTag );
  fs = actWin->fi->getXFontStruct( fontTag );

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

  updateDimensions();

  _3D = 1;
  strncpy( label, activeFreezeButtonClass_str0, 31 );
  strncpy( frozenLabel, activeFreezeButtonClass_str1, 31 );

  this->draw();

  this->editCreate();

  return 1;

}

int activeFreezeButtonClass::save (
  FILE *f )
{

int stat, major, minor, release;

tagClass tag;

int zero = 0;
char *emptyStr = "";

  major = FBTC_MAJOR_VERSION;
  minor = FBTC_MINOR_VERSION;
  release = FBTC_RELEASE;

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
  tag.loadW( "frozenBgColor", actWin->ci, &frozenBgColor );
  tag.loadW( "topShadowColor", actWin->ci, &topShadowColor );
  tag.loadW( "botShadowColor", actWin->ci, &botShadowColor );
  tag.loadW( "label", label, emptyStr );
  tag.loadW( "frozenLabel", frozenLabel, emptyStr );
  tag.loadW( "font", fontTag );
  tag.loadBoolW( "3d", &_3D, &zero );
  tag.loadW( unknownTags );
  tag.loadW( "endObjectProperties" );
  tag.loadW( "" );

  stat = tag.writeTags( f );

  return stat;

}

int activeFreezeButtonClass::createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{
int stat, major, minor, release;

tagClass tag;

int zero = 0;
char *emptyStr = "";

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
  tag.loadR( "frozenBgColor", actWin->ci, &frozenBgColor );
  tag.loadR( "topShadowColor", actWin->ci, &topShadowColor );
  tag.loadR( "botShadowColor", actWin->ci, &botShadowColor );
  tag.loadR( "label", 31, label, emptyStr );
  tag.loadR( "frozenLabel", 31, frozenLabel, emptyStr );
  tag.loadR( "font", 63, fontTag );
  tag.loadR( "3d", &_3D, &zero );
  tag.loadR( "endObjectProperties" );

  stat = tag.readTags( f, "endObjectProperties" );

  if ( !( stat & 1 ) ) {
    actWin->appCtx->postMessage( tag.errMsg() );
  }

  if ( major > FBTC_MAJOR_VERSION ) {
    postIncompatable();
    return 0;
  }

  if ( major < 4 ) {
    postIncompatable();
    return 0;
  }

  this->initSelectBox(); // call after getting x,y,w,h

  actWin->fi->loadFontTag( fontTag );
  fs = actWin->fi->getXFontStruct( fontTag );

  updateDimensions();

  return stat;

}

int activeFreezeButtonClass::genericEdit ( void ) {

char title[32], *ptr;

  ptr = actWin->obj.getNameFromClass( "activeFreezeButtonClass" );
  if ( ptr )
    strncpy( title, ptr, 31 );
  else
    strncpy( title, activeFreezeButtonClass_str2, 31 );

  Strncat( title, activeFreezeButtonClass_str3, 31 );

  bufX = x;
  bufY = y;
  bufW = w;
  bufH = h;

  bufFgColor = fgColor;
  bufBgColor = bgColor;
  bufFrozenBgColor = frozenBgColor;
  bufTopShadowColor = topShadowColor;
  bufBotShadowColor = botShadowColor;
  strncpy( bufFontTag, fontTag, 63 );
  buf3D = _3D;
  strncpy( bufLabel, label, 31 );
  strncpy( bufFrozenLabel, frozenLabel, 31 );

  ef.create( actWin->top, actWin->appCtx->ci.getColorMap(),
   &actWin->appCtx->entryFormX,
   &actWin->appCtx->entryFormY, &actWin->appCtx->entryFormW,
   &actWin->appCtx->entryFormH, &actWin->appCtx->largestH,
   title, NULL, NULL, NULL );

  ef.addTextField( activeFreezeButtonClass_str4, 35, &bufX );
  ef.addTextField( activeFreezeButtonClass_str5, 35, &bufY );
  ef.addTextField( activeFreezeButtonClass_str6, 35, &bufW );
  ef.addTextField( activeFreezeButtonClass_str7, 35, &bufH );
  ef.addTextField( activeFreezeButtonClass_str8, 35, bufLabel, 31 );
  ef.addTextField( activeFreezeButtonClass_str9, 35, bufFrozenLabel, 31 );
  ef.addToggle( activeFreezeButtonClass_str10, &buf3D );
  ef.addFontMenu( activeFreezeButtonClass_str11, actWin->fi, &fm, fontTag );
  ef.addColorButton( activeFreezeButtonClass_str12, actWin->ci, &fgCb, &bufFgColor );
  ef.addColorButton( activeFreezeButtonClass_str13, actWin->ci, &bgCb, &bufBgColor );
  ef.addColorButton( activeFreezeButtonClass_str14, actWin->ci, &frozenBgCb, &bufFrozenBgColor );
  ef.addColorButton( activeFreezeButtonClass_str15, actWin->ci, &topShadowCb, &bufTopShadowColor );
  ef.addColorButton( activeFreezeButtonClass_str16, actWin->ci, &botShadowCb, &bufBotShadowColor );

  XtUnmanageChild( fm.alignWidget() ); // no alignment info

  return 1;

}

int activeFreezeButtonClass::editCreate ( void ) {

  this->genericEdit();
  ef.finished( fbtc_edit_ok, fbtc_edit_apply, fbtc_edit_cancel_delete, this );
  actWin->currentEf = NULL;
  ef.popup();

  return 1;

}

int activeFreezeButtonClass::edit ( void ) {

  this->genericEdit();
  ef.finished( fbtc_edit_ok, fbtc_edit_apply, fbtc_edit_cancel, this );
  actWin->currentEf = &ef;
  ef.popup();

  return 1;

}

int activeFreezeButtonClass::erase ( void ) {

  if ( deleteRequest ) return 1;

  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  return 1;

}

int activeFreezeButtonClass::eraseActive ( void ) {

  if ( !enabled || !activeMode ) return 1;

  XDrawRectangle( actWin->d, drawable(actWin->executeWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  XFillRectangle( actWin->d, drawable(actWin->executeWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  return 1;

}

int activeFreezeButtonClass::draw ( void ) {

int tX, tY;
XRectangle xR = { x, y, w, h };
  if ( deleteRequest ) return 1;

  actWin->drawGc.saveFg();

  actWin->drawGc.setFG( actWin->ci->pix(bgColor));

  XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x, y, w, h );

  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x, y, w, h );

  if ( _3D ) {

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

  }

  if ( fs ) {

    actWin->drawGc.addNormXClipRectangle( xR );

    actWin->drawGc.setFG( actWin->ci->pix(fgColor) );
    actWin->drawGc.setFontTag( fontTag, actWin->fi );

    tX = x + w/2;
    tY = y + h/2 - fontAscent/2;

    drawText( actWin->drawWidget, &actWin->drawGc, fs, tX, tY,
     XmALIGNMENT_CENTER, label );

    actWin->drawGc.removeNormXClipRectangle();

  }

  actWin->drawGc.restoreFg();
  return 1;

}

int activeFreezeButtonClass::drawActive ( void ) {

int tX, tY;
XRectangle xR = { x, y, w, h };

  if ( !enabled || !activeMode ) return 1;

  if ( deleteRequest ) return 1;

  actWin->executeGc.saveFg();

  int blink = 0;
  if(actWin->is_frozen()){
    actWin->executeGc.setFG( frozenBgColor, &blink );
  }else{
    actWin->executeGc.setFG( bgColor, &blink );
  }
  //actWin->executeGc.setFG( actWin->ci->pix(bgColor) );

  XFillRectangle( actWin->d, drawable(actWin->executeWidget),
   actWin->executeGc.normGC(), x, y, w, h );

  XDrawRectangle( actWin->d, drawable(actWin->executeWidget),
   actWin->executeGc.normGC(), x, y, w, h );

  if ( _3D ) {

  actWin->executeGc.setFG( actWin->ci->pix(botShadowColor));

  XDrawLine( actWin->d, drawable(actWin->executeWidget),
   actWin->executeGc.normGC(), x, y, x+w, y );

  XDrawLine( actWin->d, drawable(actWin->executeWidget),
   actWin->executeGc.normGC(), x, y, x, y+h );

   actWin->executeGc.setFG( actWin->ci->pix(topShadowColor) );

   XDrawLine( actWin->d, drawable(actWin->executeWidget),
    actWin->executeGc.normGC(), x, y+h, x+w, y+h );

   XDrawLine( actWin->d, drawable(actWin->executeWidget),
    actWin->executeGc.normGC(), x+w, y, x+w, y+h );

  actWin->executeGc.setFG( actWin->ci->pix(topShadowColor) );

  XDrawLine( actWin->d, drawable(actWin->executeWidget),
   actWin->executeGc.normGC(), x+1, y+1, x+w-1, y+1 );

  XDrawLine( actWin->d, drawable(actWin->executeWidget),
   actWin->executeGc.normGC(), x+2, y+2, x+w-2, y+2 );

  XDrawLine( actWin->d, drawable(actWin->executeWidget),
   actWin->executeGc.normGC(), x+1, y+1, x+1, y+h-1 );

  XDrawLine( actWin->d, drawable(actWin->executeWidget),
   actWin->executeGc.normGC(), x+2, y+2, x+2, y+h-2 );

  actWin->executeGc.setFG( actWin->ci->pix(botShadowColor) );

  XDrawLine( actWin->d, drawable(actWin->executeWidget),
   actWin->executeGc.normGC(), x+1, y+h-1, x+w-1, y+h-1 );

  XDrawLine( actWin->d, drawable(actWin->executeWidget),
   actWin->executeGc.normGC(), x+2, y+h-2, x+w-2, y+h-2 );

  XDrawLine( actWin->d, drawable(actWin->executeWidget),
   actWin->executeGc.normGC(), x+w-1, y+1, x+w-1, y+h-1 );

  XDrawLine( actWin->d, drawable(actWin->executeWidget),
   actWin->executeGc.normGC(), x+w-2, y+2, x+w-2, y+h-2 );

  }

  if ( fs ) {

    actWin->executeGc.addNormXClipRectangle( xR );

    actWin->executeGc.setFG( actWin->ci->pix(fgColor) );
    actWin->executeGc.setFontTag( fontTag, actWin->fi );

    tX = x + w/2;
    tY = y + h/2 - fontAscent/2;

    if(actWin->is_frozen()){
         drawText( actWin->executeWidget, drawable(actWin->executeWidget),
            &actWin->executeGc, fs, tX, tY, XmALIGNMENT_CENTER, frozenLabel );
    }
    else{
        drawText( actWin->executeWidget, drawable(actWin->executeWidget),
         &actWin->executeGc, fs, tX, tY, XmALIGNMENT_CENTER, label );
    }

    actWin->executeGc.removeNormXClipRectangle();

  }

  actWin->executeGc.restoreFg();
  updateBlink(blink);
  return 1;

}

int activeFreezeButtonClass::activate (
  int pass,
  void *ptr )
{

  switch ( pass ) {

  case 1:

    activeMode = 1;
    initEnable();
    aglPtr = ptr;
    break;

  }

  return 1;

}

int activeFreezeButtonClass::deactivate ( void ) {

  activeMode = 0;

  return 1;

}

void activeFreezeButtonClass::updateDimensions ( void )
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

void activeFreezeButtonClass::btnUp (
  int x,
  int y,
  int buttonState,
  int buttonNumber,
  int *action )
{

  *action = 0;
  if ( !enabled ) return;

}

void activeFreezeButtonClass::btnDown (
  int x,
  int y,
  int buttonState,
  int buttonNumber,
  int *action )
{

  if(actWin->is_frozen()){
    actWin->freeze(false);
  }
  else{
    actWin->freeze(true);
  }

}

int activeFreezeButtonClass::getButtonActionRequest (
  int *up,
  int *down,
  int *drag,
  int *focus )
{

  *drag = 0;
  *up = 0;
  *down = 1;
  *focus = 1;

  return 1;

}

void activeFreezeButtonClass::changeDisplayParams (
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
    fgColor = _textFgColor;

  if ( _flag & ACTGRF_BGCOLOR_MASK )
    bgColor = _bgColor;

  if ( _flag & ACTGRF_TOPSHADOWCOLOR_MASK )
    topShadowColor = _topShadowColor;

  if ( _flag & ACTGRF_BOTSHADOWCOLOR_MASK )
    botShadowColor = _botShadowColor;

  if ( _flag & ACTGRF_BTNFONTTAG_MASK ) {

    strcpy( fontTag, _btnFontTag );
    actWin->fi->loadFontTag( fontTag );
    fs = actWin->fi->getXFontStruct( fontTag );

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

    updateDimensions();

  }

}

char *activeFreezeButtonClass::getSearchString (
  int i
) {

  if ( i == 0 ) {
    return label;
  }
  else if ( i == 1 ) {
    return frozenLabel;
  }

  return NULL;

}

void activeFreezeButtonClass::replaceString (
  int i,
  int max,
  char *string
) {

  if ( i == 0 ) {
    int l = max;
    if ( 31 < max ) l = 31;
    strncpy( label, string, l );
    label[l] = 0;
  }
  else if ( i == 1 ) {
    int l = max;
    if ( 31 < max ) l = 31;
    strncpy( frozenLabel, string, l );
    frozenLabel[l] = 0;
  }

}
