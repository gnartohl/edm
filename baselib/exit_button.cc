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

#define __exit_button_cc 1

#include "exit_button.h"
#include "app_pkg.h"
#include "act_win.h"

#include "thread.h"

static void ebtc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeExitButtonClass *ebto = (activeExitButtonClass *) client;

  ebto->actWin->setChanged();

  ebto->eraseSelectBoxCorners();
  ebto->erase();

  ebto->fgColor = ebto->bufFgColor;
  ebto->bgColor = ebto->bufBgColor;
  ebto->topShadowColor = ebto->bufTopShadowColor;
  ebto->botShadowColor = ebto->bufBotShadowColor;

  strncpy( ebto->fontTag, ebto->fm.currentFontTag(), 63 );
  ebto->actWin->fi->loadFontTag( ebto->fontTag );
  ebto->fs = ebto->actWin->fi->getXFontStruct( ebto->fontTag );

  ebto->_3D = ebto->buf3D;

  ebto->iconify = ebto->bufIconify;

  ebto->exitProgram = ebto->bufExitProgram;

  ebto->invisible = ebto->bufInvisible;

  strncpy( ebto->label, ebto->bufLabel, 31 );

  ebto->x = ebto->bufX;
  ebto->sboxX = ebto->bufX;

  ebto->y = ebto->bufY;
  ebto->sboxY = ebto->bufY;

  ebto->w = ebto->bufW;
  ebto->sboxW = ebto->bufW;

  ebto->h = ebto->bufH;
  ebto->sboxH = ebto->bufH;

  ebto->updateDimensions();

}

static void ebtc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeExitButtonClass *ebto = (activeExitButtonClass *) client;

  ebtc_edit_update( w, client, call );
  ebto->refresh( ebto );

}

static void ebtc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeExitButtonClass *ebto = (activeExitButtonClass *) client;

  ebtc_edit_update( w, client, call );
  ebto->ef.popdown();
  ebto->operationComplete();

}

static void ebtc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeExitButtonClass *ebto = (activeExitButtonClass *) client;

  ebto->ef.popdown();
  ebto->operationCancel();

}

static void ebtc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeExitButtonClass *ebto = (activeExitButtonClass *) client;

  ebto->ef.popdown();
  ebto->operationCancel();
  ebto->erase();
  ebto->deleteRequest = 1;
  ebto->drawAll();

}

activeExitButtonClass::activeExitButtonClass ( void ) {

  name = new char[strlen("activeExitButtonClass")+1];
  strcpy( name, "activeExitButtonClass" );
  deleteRequest = 0;
  selected = 0;
  iconify = 0;
  exitProgram = 0;

}

// copy constructor
activeExitButtonClass::activeExitButtonClass
 ( const activeExitButtonClass *source ) {

activeGraphicClass *ebto = (activeGraphicClass *) this;

  ebto->clone( (activeGraphicClass *) source );

  name = new char[strlen("activeExitButtonClass")+1];
  strcpy( name, "activeExitButtonClass" );

  deleteRequest = 0;

  fgCb = source->fgCb;
  bgCb = source->bgCb;
  topShadowCb = source->topShadowCb;
  botShadowCb = source->botShadowCb;

  strncpy( fontTag, source->fontTag, 63 );

  fs = actWin->fi->getXFontStruct( fontTag );

  fgColor = source->fgColor;
  bgColor = source->bgColor;
  topShadowColor = source->topShadowColor;
  botShadowColor = source->botShadowColor;

  _3D = source->_3D;
  iconify = source->iconify;
  exitProgram = source->exitProgram;
  invisible = source->invisible;
  strncpy( label, source->label, 31 );

  updateDimensions();

}

int activeExitButtonClass::createInteractive (
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
  iconify = 0;
  exitProgram = 0;
  invisible = 0;
  strncpy( label, activeExitButtonClass_str1, 31 );

  this->draw();

  this->editCreate();

  return 1;

}

int activeExitButtonClass::save (
  FILE *f )
{

int index;

  fprintf( f, "%-d %-d %-d\n", EBTC_MAJOR_VERSION, EBTC_MINOR_VERSION,
   EBTC_RELEASE );

  fprintf( f, "%-d\n", x );
  fprintf( f, "%-d\n", y );
  fprintf( f, "%-d\n", w );
  fprintf( f, "%-d\n", h );

  index = fgColor;
  fprintf( f, "%-d\n", index );

  index = bgColor;
  fprintf( f, "%-d\n", index );

  index = topShadowColor;
  fprintf( f, "%-d\n", index );

  index = botShadowColor;
  fprintf( f, "%-d\n", index );

  fprintf( f, "%-d\n", _3D );

  fprintf( f, "%-d\n", invisible );

  writeStringToFile( f, fontTag );

  writeStringToFile( f, label );

  fprintf( f, "%-d\n", iconify );

  fprintf( f, "%-d\n", exitProgram );

  return 1;

}

int activeExitButtonClass::createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int r, g, b, index;
int major, minor, release;
unsigned int pixel;

  this->actWin = _actWin;

  fscanf( f, "%d %d %d\n", &major, &minor, &release ); actWin->incLine();

  fscanf( f, "%d\n", &x ); actWin->incLine();
  fscanf( f, "%d\n", &y ); actWin->incLine();
  fscanf( f, "%d\n", &w ); actWin->incLine();
  fscanf( f, "%d\n", &h ); actWin->incLine();

  if ( major > 1 ) {

    fscanf( f, "%d\n", &index ); actWin->incLine();
    fgColor = index;

    fscanf( f, "%d\n", &index ); actWin->incLine();
    bgColor = index;

    fscanf( f, "%d\n", &index ); actWin->incLine();
    topShadowColor = index;

    fscanf( f, "%d\n", &index ); actWin->incLine();
    botShadowColor = index;

  }
  else {

    fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
    actWin->ci->setRGB( r, g, b, &pixel );
    fgColor = actWin->ci->pixIndex( pixel );

    fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
    actWin->ci->setRGB( r, g, b, &pixel );
    bgColor = actWin->ci->pixIndex( pixel );

    fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
    actWin->ci->setRGB( r, g, b, &pixel );
    topShadowColor = actWin->ci->pixIndex( pixel );

    fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
    actWin->ci->setRGB( r, g, b, &pixel );
    botShadowColor = actWin->ci->pixIndex( pixel );

  }

  fscanf( f, "%d\n", &_3D ); actWin->incLine();

  fscanf( f, "%d\n", &invisible ); actWin->incLine();

  readStringFromFile( fontTag, 63, f ); actWin->incLine();

  readStringFromFile( label, 31, f ); actWin->incLine();

  if ( ( major > 1 ) || ( minor > 0 ) ) {
    fscanf( f, "%d\n", &iconify ); actWin->incLine();
  }
  else {
    iconify = 0;
  }

  if ( ( major > 1 ) || ( minor > 1 ) ) {
    fscanf( f, "%d\n", &exitProgram ); actWin->incLine();
  }
  else {
    exitProgram = 0;
  }

  this->initSelectBox();

  actWin->fi->loadFontTag( fontTag );
  fs = actWin->fi->getXFontStruct( fontTag );

  updateDimensions();

  return 1;

}

int activeExitButtonClass::genericEdit ( void ) {

char title[32], *ptr;

  ptr = actWin->obj.getNameFromClass( "activeExitButtonClass" );
  if ( ptr )
    strncpy( title, ptr, 31 );
  else
    strncpy( title, activeExitButtonClass_str2, 31 );

  strncat( title, activeExitButtonClass_str3, 31 );

  bufX = x;
  bufY = y;
  bufW = w;
  bufH = h;

  bufFgColor = fgColor;
  bufBgColor = bgColor;
  bufTopShadowColor = topShadowColor;
  bufBotShadowColor = botShadowColor;
  strncpy( bufFontTag, fontTag, 63 );
  buf3D = _3D;
  bufIconify = iconify;
  bufExitProgram = exitProgram;
  bufInvisible = invisible;
  strncpy( bufLabel, label, 31 );

  ef.create( actWin->top, actWin->appCtx->ci.getColorMap(),
   &actWin->appCtx->entryFormX,
   &actWin->appCtx->entryFormY, &actWin->appCtx->entryFormW,
   &actWin->appCtx->entryFormH, &actWin->appCtx->largestH,
   title, NULL, NULL, NULL );

  ef.addTextField( activeExitButtonClass_str4, 30, &bufX );
  ef.addTextField( activeExitButtonClass_str5, 30, &bufY );
  ef.addTextField( activeExitButtonClass_str6, 30, &bufW );
  ef.addTextField( activeExitButtonClass_str7, 30, &bufH );
  ef.addTextField( activeExitButtonClass_str8, 30, bufLabel, 31 );
  ef.addToggle( activeExitButtonClass_str9, &buf3D );
  ef.addToggle( activeExitButtonClass_str10, &bufInvisible );
  ef.addToggle( activeExitButtonClass_str11, &bufIconify );
  ef.addToggle( activeExitButtonClass_str12, &bufExitProgram );
  ef.addColorButton( activeExitButtonClass_str14, actWin->ci, &fgCb, &bufFgColor );
  ef.addColorButton( activeExitButtonClass_str15, actWin->ci, &bgCb, &bufBgColor );
  ef.addColorButton( activeExitButtonClass_str16, actWin->ci, &topShadowCb, &bufTopShadowColor );
  ef.addColorButton( activeExitButtonClass_str17, actWin->ci, &botShadowCb, &bufBotShadowColor );

  ef.addFontMenu( activeExitButtonClass_str13, actWin->fi, &fm, fontTag );

  XtUnmanageChild( fm.alignWidget() ); // no alignment info

  return 1;

}

int activeExitButtonClass::editCreate ( void ) {

  this->genericEdit();
  ef.finished( ebtc_edit_ok, ebtc_edit_apply, ebtc_edit_cancel_delete, this );
  actWin->currentEf = NULL;
  ef.popup();

  return 1;

}

int activeExitButtonClass::edit ( void ) {

  this->genericEdit();
  ef.finished( ebtc_edit_ok, ebtc_edit_apply, ebtc_edit_cancel, this );
  actWin->currentEf = &ef;
  ef.popup();

  return 1;

}

int activeExitButtonClass::erase ( void ) {

  if ( deleteRequest ) return 1;

  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  return 1;

}

int activeExitButtonClass::eraseActive ( void ) {

  if ( !activeMode || invisible ) return 1;

  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  return 1;

}

int activeExitButtonClass::draw ( void ) {

int tX, tY;
XRectangle xR = { x, y, w, h };

  if ( deleteRequest ) return 1;

  actWin->drawGc.saveFg();

  actWin->drawGc.setFG( actWin->ci->pix(bgColor) );

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

int activeExitButtonClass::drawActive ( void ) {

  if ( !activeMode || invisible ) return 1;

  draw();

  return 1;

}

int activeExitButtonClass::activate (
  int pass,
  void *ptr )
{

  switch ( pass ) {

  case 1:

    activeMode = 1;
    aglPtr = ptr;
    break;

  }

  return 1;

}

int activeExitButtonClass::deactivate ( void ) {

  activeMode = 0;

  return 1;

}

void activeExitButtonClass::updateDimensions ( void )
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

void activeExitButtonClass::btnUp (
  int x,
  int y,
  int buttonState,
  int buttonNumber,
  int *action )
{

  *action = 0;

}

void activeExitButtonClass::btnDown (
  int x,
  int y,
  int buttonState,
  int buttonNumber,
  int *action )
{

  if ( exitProgram ) {

    actWin->appCtx->exitProgram();

    return;

  }

  if ( iconify ) {

    *action = 0;

    XIconifyWindow( actWin->d, XtWindow(actWin->topWidgetId()),
     DefaultScreen(actWin->d) );

  }
  else {

    *action = 1; /* close window */

  }

}

int activeExitButtonClass::getButtonActionRequest (
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

void activeExitButtonClass::changeDisplayParams (
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

#ifdef __cplusplus
extern "C" {
#endif

void *create_activeExitButtonClassPtr ( void ) {

activeExitButtonClass *ptr;

  ptr = new activeExitButtonClass;
  return (void *) ptr;

}

void *clone_activeExitButtonClassPtr (
  void *_srcPtr )
{

activeExitButtonClass *ptr, *srcPtr;

  srcPtr = (activeExitButtonClass *) _srcPtr;

  ptr = new activeExitButtonClass( srcPtr );

  return (void *) ptr;

}

#ifdef __cplusplus
}
#endif