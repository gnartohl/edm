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

#ifndef __gif_h
#define __gif_h 1

#include "act_grf.h"
#include "entry_form.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "gif_lib.h"
#ifdef __cplusplus
}
#endif

#include <sys/stat.h>
#include <unistd.h>

// the following defines btnActionListType & btnActionListPtr
#include "btnActionListType.h"

#define AGIFC_MAJOR_VERSION 1
#define AGIFC_MINOR_VERSION 2
#define AGIFC_RELEASE 0

#ifdef __gif_cc

#include "gif.str"

static void agifc_update (
  XtPointer client,
  XtIntervalId *id );

static void agifc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

static void agifc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

static void agifc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

static void agifc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

static void agifc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

#endif

class activeGifClass : public activeGraphicClass {

private:

int init, active, activeMode, opComplete;
char gifFileName[127+1];
unsigned long *pixels;
int numPixels;
XImage *image;
char *imageData;
GifFileType *gif;
int noFile;

int bufX;
int bufY;
int bufW;
int bufH;

char bufGifFileName[127+1];
time_t fileModTime, prevFileModTime;

XtIntervalId timer;
int timerValue;
int timerActive;

int uniformSize, bufUniformSize;
int refreshRate, bufRefreshRate;
int fastErase, bufFastErase;

int needErase;

public:

friend void agifc_update (
  XtPointer client,
  XtIntervalId *id );

friend void agifc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void agifc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void agifc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void agifc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void agifc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

activeGifClass::activeGifClass ( void );

activeGifClass::activeGifClass
 ( const activeGifClass *source );

activeGifClass::~activeGifClass ( void );

int activeGifClass::genericEdit ( void );

int activeGifClass::edit ( void );

int activeGifClass::editCreate ( void );

void activeGifClass::checkGifFileTime ( void );

int activeGifClass::readGifFile ( void );

int activeGifClass::createInteractive (
  activeWindowClass *aw_obj,
  int x,
  int y,
  int w,
  int h );

int activeGifClass::save (
  FILE *f );

int activeGifClass::createFromFile (
  FILE *fptr,
  char *name,
  activeWindowClass *actWin );

int activeGifClass::draw ( void );

int activeGifClass::draw (
  int _x,
  int _y,
  int _w,
  int _h );

int activeGifClass::erase ( void );

int activeGifClass::drawActive ( void );

int activeGifClass::drawActive (
  int _x,
  int _y,
  int _w,
  int _h );

int activeGifClass::eraseActive ( void );

int activeGifClass::activate (
  int pass,
  void *ptr );

int activeGifClass::deactivate (
  int pass );

int activeGifClass::checkResizeSelectBox (
  int _x,
  int _y,
  int _w,
  int _h );

int activeGifClass::checkResizeSelectBoxAbs (
  int _x,
  int _y,
  int _w,
  int _h );

void activeGifClass::executeDeferred ( void );

};

#ifdef __cplusplus
extern "C" {
#endif

void *create_12345678901234567890123456789012GIFPtr ( void );
void *clone_12345678901234567890123456789012GIFPtr ( void * );

void *create_activeGifClassPtr ( void );
void *clone_activeGifClassPtr ( void * );

#ifdef __cplusplus
}
#endif

#endif
