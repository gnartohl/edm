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

#ifndef __mp_h
#define __mp_h 1

#include "act_grf.h"
#include "entry_form.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "mtvp/mtvp.h"

#ifdef __cplusplus
}
#endif

#include "cadef.h"

#define MPC_MAJOR_VERSION 1
#define MPC_MINOR_VERSION 1
#define MPC_RELEASE 0

#define MPC_NUM_STATES 10

// this define is per the mtvp sample code
#define MSG_PTR (&Msg)

#ifdef __mp_cc

static void mpc_sendMsg (
  XtPointer client,
  XtIntervalId *id );

static void mpc_readMsg (
  XtPointer client,
  int *fd,
  XtInputId *id );

static void mpc_putValue (
  Widget w,
  XtPointer client,
  XtPointer call );

static void mpc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

static void mpc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

static void mpc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

static void mpc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

static void mpc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

static void mpc_controlUpdate (
  struct event_handler_args ast_args );

static void mpc_monitor_control_connect_state (
  struct connection_handler_args arg );

static void mpc_monitor_framesPlayed_connect_state (
  struct connection_handler_args arg );

#endif

class activeMpClass : public activeGraphicClass {

private:

friend void mpc_sendMsg (
  XtPointer client,
  XtIntervalId *id );

friend void mpc_readMsg (
  XtPointer client,
  int *fd,
  XtInputId *id );

friend void mpc_putValue (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void mpc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void mpc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void mpc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void mpc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void mpc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void mpc_controlUpdate (
  struct event_handler_args ast_args );

friend void mpc_monitor_control_connect_state (
  struct connection_handler_args arg );

friend void mpc_monitor_framesPlayed_connect_state (
  struct connection_handler_args arg );

/* to communicate with player */
PMP_STRUCT Player;

XtInputId msgInputId;
XtIntervalId msgPollTimer;

int opComplete, fileOpen, playerHasQuit, inputDisabled, timerActive;

int bufX, bufY, bufW, bufH;

char curControlV[39+1], controlV[39+1];

unsigned int topShadowColor, bufTopShadowColor;
unsigned int botShadowColor, bufBotShadowColor;
unsigned int bufFgColor, bufBgColor;
pvColorClass fgColor, bgColor;
colorButtonClass fgCb, bgCb, topShadowCb, botShadowCb;
int fgColorMode, bgColorMode, bufFgColorMode, bufBgColorMode;

fontMenuClass fm;
char fontTag[63+1], bufFontTag[63+1];
XmFontList fontList;
XFontStruct *fs;
int fontAscent, fontDescent, fontHeight;

chid controlPvId;
evid alarmEventId, controlEventId;
chid framesPlayedPvId;

char bufControlPvName[39+1];
expStringClass controlPvExpStr;

char bufframesPlayedPvName[39+1];
expStringClass framesPlayedPvExpStr;

int numStates;

int controlExists, widgetsCreated, controlPvConnected, 
 framesPlayedExists, framesPlayedPvConnected, notActive, activeMode;

Widget optionMenu, pulldownMenu, curHistoryWidget, pb[MPC_NUM_STATES],
 frame;

int needCtlConnectInit, needFrPlConnectInit, needDraw;

public:

activeMpClass::activeMpClass ( void );

activeMpClass::activeMpClass
 ( const activeMpClass *source );

activeMpClass::~activeMpClass ( void );

char *activeMpClass::objName ( void ) {

  return name;

}

int activeMpClass::createInteractive (
  activeWindowClass *aw_obj,
  int x,
  int y,
  int w,
  int h );

int activeMpClass::save (
  FILE *f );

int activeMpClass::createFromFile (
  FILE *fptr,
  char *name,
  activeWindowClass *actWin );

int activeMpClass::genericEdit ( void );

int activeMpClass::edit ( void );

int activeMpClass::editCreate ( void );

int activeMpClass::draw ( void );

int activeMpClass::erase ( void );

int activeMpClass::drawActive ( void );

int activeMpClass::eraseActive ( void );

int activeMpClass::expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] );

int activeMpClass::expand2nd (
  int numMacros,
  char *macros[],
  char *expansions[] );

int activeMpClass::containsMacros ( void );

int activeMpClass::createWidgets ( void );

int activeMpClass::activate ( int pass, void *ptr );

int activeMpClass::deactivate ( int pass );

void activeMpClass::updateDimensions ( void );

void activeMpClass::executeFromDeferredQueue ( void );

void activeMpClass::executeDeferred ( void );

};

#ifdef __cplusplus
extern "C" {
#endif

void *create_activeMpClassPtr ( void );
void *clone_activeMpClassPtr ( void * );

#ifdef __cplusplus
}
#endif

#endif
