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

#define AGIFC_MAJOR_VERSION 4
#define AGIFC_MINOR_VERSION 0
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
int noErase, bufNoErase;

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

activeGifClass ( void );

activeGifClass
 ( const activeGifClass *source );

~activeGifClass ( void );

int genericEdit ( void );

int edit ( void );

int editCreate ( void );

void checkGifFileTime ( void );

int readGifFile ( void );

int createInteractive (
  activeWindowClass *aw_obj,
  int x,
  int y,
  int w,
  int h );

int save (
  FILE *f );

int old_save (
  FILE *f );

int createFromFile (
  FILE *fptr,
  char *name,
  activeWindowClass *actWin );

int old_createFromFile (
  FILE *fptr,
  char *name,
  activeWindowClass *actWin );

int draw ( void );

int draw (
  int _x,
  int _y,
  int _w,
  int _h );

int erase ( void );

int drawActive ( void );

int drawActive (
  int _x,
  int _y,
  int _w,
  int _h );

int eraseActive ( void );

int activate (
  int pass,
  void *ptr );

int deactivate (
  int pass );

int checkResizeSelectBox (
  int _x,
  int _y,
  int _w,
  int _h );

int checkResizeSelectBoxAbs (
  int _x,
  int _y,
  int _w,
  int _h );

void executeDeferred ( void );

char *getSearchString (
  int index );

void replaceString (
  int i,
  int max,
  char *string
);

};

#ifdef __cplusplus
extern "C" {
#endif

void *create_cfcf6c8a_dbeb_11d2_8a97_00104b8742dfPtr( void );
void *clone_cfcf6c8a_dbeb_11d2_8a97_00104b8742dfPtr ( void * );

#ifdef __cplusplus
}
#endif

#endif
