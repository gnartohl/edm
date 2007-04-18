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

#ifndef __color_pkg_h
#define __color_pkg_h 1

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <Xm/PushB.h>
#include <Xm/MainW.h>
#include <Xm/RowColumn.h>
#include <Xm/DrawingA.h>
#include <Xm/Protocols.h>

#include "sys_types.h"
#include "avl.h"
#include "thread.h"
#include "gc_pkg.h"
#include "color_pkg.str"
#include "color_list.h"
#include "msg_dialog.h"

#define COLORINFO_SUCCESS 1
#define COLORINFO_EMPTY 100
#define COLORINFO_FAIL 102
#define COLORINFO_NO_FILE 104
#define COLORINFO_NO_COLOR 106

#define NUM_SPECIAL_COLORS 7
#define NUM_BLINKING_COLORS 8
#define MAX_COLORS 88
#define NUM_COLOR_COLS 11

#define COLORINFO_K_DISCONNECTED 0
#define COLORINFO_K_INVALID 1
#define COLORINFO_K_MINOR 2
#define COLORINFO_K_MAJOR 3
#define COLORINFO_K_NOALARM 4

class colorInfoClass;

typedef struct simpleButtonTag {
  Widget wgt;
  class colorInfoClass *cio;
  int x;
  int y;
  int w;
  int h;
  int colorIndex;
  int blink;
} simpleButtonType, *simpleButtonPtr;

typedef int (*ruleFuncType)( double v1, double v2 );
typedef int (*connectingFuncType)( int v1, int v2 );

typedef struct ruleConditionTag {
    struct ruleConditionTag *flink;
    double value1;
    ruleFuncType ruleFunc1;
    double value2;
    ruleFuncType ruleFunc2;
    connectingFuncType connectingFunc;
    connectingFuncType joiningFunc; // joins one rule with another
    int result;
    char *resultName;
} ruleConditionType, *ruleConditionPtr;

typedef struct ruleTag {
    ruleConditionPtr ruleHead;
    ruleConditionPtr ruleTail;
    int needJoin;
    connectingFuncType curJoinFunc;
    int combinedOpResult;
} ruleType, *rulePtr;

typedef struct blinkNodeTag {
  AVL_FIELDS(blinkNodeTag)
  void *obj;
  void *func;
  int op; // 1=add, 2=remove
  struct blinkNodeTag *next; // for lookaside list only
} blinkNodeType, *blinkNodePtr;

typedef struct colorCacheTag {
    AVL_FIELDS(colorCacheTag)
    int rgb[3]; // [0]=r, [1]=g, [2]=b
    unsigned int pixel;
    int blinkRgb[3]; // [0]=r, [1]=g, [2]=b
    unsigned int blinkPixel;
    int index;
    int position;
    char *name;
    char *aliasValue;
    rulePtr rule;
} colorCacheType, *colorCachePtr;

typedef struct showNameBlockTag {
    int x;
    int y;
    int i;
    void *ptr;
} showNameBlockType, *showNameBlockPtr;

class colorListClass;
class colorButtonClass;

#ifdef __color_pkg_cc

static void doCiBlink (
  void *ptr
);

static void drawSimpleButton (
  simpleButtonPtr sbp
);

#endif

// colorInfoClass: handles palette and colors
//
// Definitions:
// name    -  a color string name
// menu    -  colors available to users in color dialog
// palette -  aka color map: colors available on screen
// index   -  index of color in the current palette/color map
//            (unless we talk about the menu index)
// pixel   -  index in palette, X11 pixel value, used in X calls
//
// colorInfoClass 
// * Reads color definition file
// * Translates index <-> name, index <-> pixel, pixel <-> RGB
//
// Usually accessed via actWin->ci

class colorInfoClass {

public:

    static const int SUCCESS = 1;
    static const int FAIL = 0;

    colorListClass colorList;
    colorButtonClass *curCb;

    int change;

    int colorWindowIsOpen;

    int blink;

    colorInfoClass ();
    ~colorInfoClass ();

    int colorChanged()
    {
        int i;
        i = change;
        change = 0;
        return i;
    }

    int ver3InitFromFile (FILE *f,
                          XtAppContext app,
                          Display *d,
                          Widget top,
                          char *fileName);

    int ver4InitFromFile (FILE *f,
                          XtAppContext app,
                          Display *d,
                          Widget top,
                          char *fileName);

    int initFromFile (XtAppContext app,
                      Display *d,
                      Widget top,
                      char *fileName);

    int openColorWindow();
    int closeColorWindow();

    unsigned int getFg();

    // RGB <-> pixel
    int getRGB (unsigned int pixel, int *r, int *g, int *b);
    int setRGB (int r, int g, int b, unsigned int *pixel);

    Colormap getColorMap ();

    // Highlights active color pallete cell
    int setCurIndex (int index);

    // index <-> pixel
    unsigned int getPixelByIndex (int index);
    unsigned int getPixelByIndexWithBlink (int index);
    unsigned int pix (int index) 
    {   return  getPixelByIndex (index); }
    unsigned int pixWblink (int index) 
    {   return  getPixelByIndexWithBlink (index); }
    int pixIndex (unsigned int pixel);
    
    // return reasonable fg for given bg
    unsigned int labelPix (int index);

    void initParseEngine (FILE *f);

    void parseError (char *msg);

    int getToken (char toke[255+1]);

    // index <-> name
    char *colorName (int index);
    int colorIndexByName (const char *name);
    int colorIndexByAlias (const char *name); // use alias list

    // Returns true/false depending on wether
    // this index is rule-based
    int isRule (int index );

    char *firstColor (colorCachePtr node);
    char *nextColor (colorCachePtr node);

    int majorVersion ();

    int menuIndex (int index);

    int menuPosition (int index);

    int menuSize ();

    // IF index refers to a rule-based color (see isRule()),
    // this routine returns the index of the
    // color to use for the given value.
    int evalRule (
      int index,
      double value );

    int isInvisible(
      int index );

    void useIndex ( void );

    void useRGB ( void );

    int colorModeIsRGB ( void );

    int writeColorIndex (
      FILE *f,
      int index
    );

    int writeColorIndex (
      FILE *f,
      char *tag,
      int index
    );

    int writeColorArrayIndex (
      FILE *f,
      int index
    );

    int writeColorArrayIndex (
      FILE *f,
      int arrayIndex,
      int index
    );

    int readColorIndex (
      FILE *f,
      int *index
    );

    void usePrivColorMap ( void );

    void warnIfBadIndex (
      int index,
      int line
    );

    int shouldShowNoAlarmState ( void );

    // The following functions are for use in color_button.cc and
    // entry_form.cc; they are not intended for general use.
    void setCurDestination(int *ptr);
    int *getCurDestination();
    void setCurCb(colorButtonClass *cb);
    colorButtonClass *getCurCb();
    int setActiveWidget(Widget w);
    Widget getActiveWidget();
    int setNameWidget(Widget w);
    Widget getNameWidget();

    // These functions are used by pvColor.cc and are not intended for
    // general use.
    int getSpecialColor (int index);
    int getSpecialIndex (int index);

    // These functions are used by gc_pkg.cc and are not intended for
    // general use.
    int blinking ( int index );
    int addToBlinkList( void *obj, void *func );
    int removeFromBlinkList( void *obj, void *func );
    int addAllToBlinkList ( void );
    int removeAllFromBlinkList ( void );

    // deprecated functions, for backward compatibility only
    int getIndex (unsigned int pixel, int *index);
    int setIndex (int index, unsigned int *pixel);
    int setCurIndexByPixel (unsigned int pixel);
    int canDiscardPixel (unsigned int pixel);

private:

    friend void doCiBlink (
      void *ptr
    );

    friend void drawSimpleButton (
      simpleButtonPtr sbp
    );

    friend void showColorName (
        XtPointer client,
        XtIntervalId *id );

    friend void doColorBlink (
        XtPointer client,
        XtIntervalId *id );

    friend void toggleColorBlink (
        XtPointer client,
        XtIntervalId *id );

    friend void colorShellEventHandler (
        Widget w,
        XtPointer client,
        XEvent *e,
        Boolean *continueToDispatch );

    friend void colorRcEventHandler (
        Widget w,
        XtPointer client,
        XEvent *e,
        Boolean *continueToDispatch );

    friend void colorFormEventHandler (
        Widget w,
        XtPointer client,
        XEvent *e,
        Boolean *continueToDispatch );

    friend class colorButtonClass;

    int major, minor, release;

    int max_colors, num_blinking_colors, num_color_cols, usingPrivateColorMap;

    AVL_HANDLE colorCacheByColorH;
    AVL_HANDLE colorCacheByPixelH;
    AVL_HANDLE colorCacheByIndexH;
    AVL_HANDLE colorCacheByNameH;
    AVL_HANDLE colorCacheByPosH;
    AVL_HANDLE colorCacheByAliasH;
    AVL_HANDLE blinkH;

    blinkNodePtr blinkLookasideHead, blinkLookasideTail;
    blinkNodePtr addBlinkHead, addBlinkTail;
    blinkNodePtr remBlinkHead, remBlinkTail;

    Display *display;
    int screen;
    int depth;
    Visual *visual;
    Colormap cmap;
    unsigned int fg;

    /*  unsigned int *colors[MAX_COLORS+NUM_BLINKING_COLORS]; */
    /*  unsigned long *blinkingColorCells[NUM_BLINKING_COLORS]; */
    /*  XColor *blinkingXColor[NUM_BLINKING_COLORS], */
    /*   *offBlinkingXColor[NUM_BLINKING_COLORS]; */

    simpleButtonPtr simpleColorButtons;
    unsigned int *colors, *blinkingColors;
    unsigned long *blinkingColorCells;
    XColor *blinkingXColor, *offBlinkingXColor;
    char **colorNames; // dynamic array of pointers to char
    colorCachePtr *colorNodes; // dynamic array

    int special[NUM_SPECIAL_COLORS];
    int specialIndex[NUM_SPECIAL_COLORS];
    int numColors;

    int curIndex, curX, curY;

    XtAppContext appCtx;
    XtIntervalId incrementTimer, showNameTimer;
    int incrementTimerValue, incrementTimerActive, showNameTimerActive;

    Widget activeWidget, nameWidget;
    int *curDestination;

    gcClass gc;

    Widget shell, rc, mbar, mb1, mb2, form, rc1, rc2, fgpb, bgpb;

    // color file processing
    static const int MAX_LINE_SIZE = 255;

    static const int GET_1ST_NONWS_CHAR = 1;
    static const int GET_TIL_END_OF_TOKEN = 2;
    static const int GET_TIL_END_OF_QUOTE = 3;
    static const int GET_TIL_END_OF_SPECIAL = 4;

    static const int GET_FIRST_TOKEN = 1;
    static const int GET_NUM_COLUMNS = 2;
    static const int GET_MAX = 3;
    static const int GET_RULE = 4;
    static const int GET_FIRST_OP_OR_ARG = 5;
    static const int GET_FIRST_ARG = 6;
    static const int GET_NEXT_OP_OR_ARG = 7;
    static const int GET_NEXT_ARG = 8;
    static const int GET_RULE_CONDITION = 9;
    static const int GET_CONNECTOR_OR_COLON = 10;
    static const int GET_COLON = 11;
    static const int GET_RESULT_NAME_OR_JOININGFUNC = 12;
    static const int GET_COLOR = 13;
    static const int GET_ALARM_PARAMS = 14;
    static const int GET_MENU_MAP = 15;
    static const int INSERT_COLOR = 16;
    static const int GET_ALIAS = 17;
    static const int GET_BLINK_PERIOD = 18;

    int readFile, tokenState, parseIndex, parseLine, tokenFirst, tokenLast,
        tokenNext, gotToken, colorIndex, colorPosition;
    char parseBuf[MAX_LINE_SIZE+1], parseToken[MAX_LINE_SIZE+1];
    FILE *parseFile;

    int maxColor, numPaletteCols;

    int maxMenuItems, menuMapSize;
    int *menuIndexMap; // dynamic array

    msgDialogClass msgDialog;
    int curPaletteRow, curPaletteCol;
    showNameBlockType showNameBlock;

    int invisibleIndex;

    int useIndexFlag;

    int usePrivColorMapFlag;

    int showNoAlarmState;

};

#endif
