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

#ifndef __message_button_gen_h
#define __message_button_gen_h 1

#include "act_grf.h"
#include "entry_form.h"

// #ifdef __epics__
// #include "cadef.h"
// #endif

#include "pv.h"

// #ifndef __epics__
#define MAX_ENUM_STRING_SIZE 63
// #endif

#define MSGBTC_MAJOR_VERSION 1
#define MSGBTC_MINOR_VERSION 5
#define MSGBTC_RELEASE 0

#define MSGBTC_K_PUSH 1
#define MSGBTC_K_TOGGLE 2

#ifdef __message_button_gen_cc

static void msgbtc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

static void msgbtc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

static void msgbtc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

static void msgbtc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

static void msgbtc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

#if 0
static void msgbt_sourcePressUpdate (
  struct event_handler_args ast_args );

static void msgbt_monitor_sourcePress_connect_state (
  struct connection_handler_args arg );

static void msgbt_sourceReleaseUpdate (
  struct event_handler_args ast_args );

static void msgbt_monitor_sourceRelease_connect_state (
  struct connection_handler_args arg );
#endif

static void msgbt_monitor_dest_connect_state (
  pvClass *classPtr,
  void *clientData,
  void *args );

#endif

class activeMessageButtonClass : public activeGraphicClass {

private:

friend void msgbtc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void msgbtc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void msgbtc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void msgbtc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void msgbtc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

#if 0
friend void msgbt_sourcePressUpdate (
  struct event_handler_args ast_args );

friend void msgbt_monitor_sourcePress_connect_state (
  struct connection_handler_args arg );

friend void msgbt_sourceReleaseUpdate (
  struct event_handler_args ast_args );

friend void msgbt_monitor_sourceRelease_connect_state (
  struct connection_handler_args arg );
#endif

friend void msgbt_monitor_dest_connect_state (
  pvClass *classPtr,
  void *clientData,
  void *args );

int opComplete;

int minW;
int minH;

int bufX, bufY, bufW, bufH;

int sourcePressType, sourceReleaseType, destType;

unsigned int bufFgColor, bufOnColor, bufOffColor;
pvColorClass fgColor, onColor, offColor;
unsigned int topShadowColor, bufTopShadowColor;
unsigned int botShadowColor, bufBotShadowColor;
colorButtonClass fgCb, onCb, offCb, topShadowCb, botShadowCb;
char onLabel[MAX_ENUM_STRING_SIZE+1], bufOnLabel[MAX_ENUM_STRING_SIZE+1];
char offLabel[MAX_ENUM_STRING_SIZE+1], bufOffLabel[MAX_ENUM_STRING_SIZE+1];
int buttonType, bufButtonType, _3D, buf3D, invisible, bufInvisible,
 toggle, bufToggle, pressAction, bufPressAction,
 releaseAction, bufReleaseAction;

fontMenuClass fm;
char fontTag[63+1], bufFontTag[63+1];
XFontStruct *fs;
int fontAscent, fontDescent, fontHeight;

// #ifdef __epics__
// chid sourcePressPvId, sourceReleasePvId, destPvId;
// evid sourcePressEventId, sourceReleaseEventId;
// #endif

pvClass *destPvId;
pvClass *sourcePressPvId, *sourceReleasePvId;
pvEventClass *sourcePressEventId, *sourceReleaseEventId;

expStringClass destPvExpString;
char bufDestPvName[256+1];
expStringClass sourcePressPvExpString;
char bufSourcePressPvName[256+1];
expStringClass sourceReleasePvExpString;
char bufSourceReleasePvName[256+1];

pvValType sourcePressV, sourceReleaseV, destV;

int destExists, sourcePressExists, sourceReleaseExists, buttonPressed;

int sourcePressPvConnected, sourceReleasePvConnected, destPvConnected,
 active, activeMode, init;

int needConnectInit, needErase, needDraw;

char pvClassName[15+1], pvUserClassName[15+1], pvOptionList[255+1];
int numPvTypes, pvNameIndex;

public:

activeMessageButtonClass ( void );

activeMessageButtonClass
 ( const activeMessageButtonClass *source );

~activeMessageButtonClass ( void ) {

/*   printf( "In ~activeMessageButtonClass\n" ); */

  if ( name ) delete name;

}

char *objName ( void ) {

  return name;

}

int createInteractive (
  activeWindowClass *aw_obj,
  int x,
  int y,
  int w,
  int h );

int save (
  FILE *f );

int createFromFile (
  FILE *fptr,
  char *name,
  activeWindowClass *actWin );

int importFromXchFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin );

int genericEdit ( void );

int edit ( void );

int editCreate ( void );

int draw ( void );

int erase ( void );

int drawActive ( void );

int eraseActive ( void );

int activate ( int pass, void *ptr );

int deactivate ( int pass );

void updateDimensions ( void );

void btnUp (
  int x,
  int y,
  int buttonState,
  int buttonNumber,
  int *action );

void btnDown (
  int x,
  int y,
  int buttonState,
  int buttonNumber,
  int *action );

int getButtonActionRequest (
  int *up,
  int *down,
  int *drag,
  int *focus );

int expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] );

int expand2nd (
  int numMacros,
  char *macros[],
  char *expansions[] );

int containsMacros ( void );

void executeDeferred ( void );

};

#ifdef __cplusplus
extern "C" {
#endif

void *create_activeMessageButtonClassPtr ( void );
void *clone_activeMessageButtonClassPtr ( void * );

#ifdef __cplusplus
}
#endif

#endif
