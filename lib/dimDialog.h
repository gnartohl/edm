#ifndef __dimDialog_h
#define __dimDialog_h 1

#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <Xm/BulletinB.h>
#include <Xm/DrawingA.h>
#include <Xm/PushBG.h>
#include <Xm/ToggleB.h>
#include <Xm/ArrowBG.h>
#include <Xm/Label.h>
#include <Xm/Frame.h>
#include <Xm/RowColumn.h>
#include <Xm/DialogS.h>
#include <Xm/ScrolledW.h>
#include <Xm/PanedW.h>
#include <Xm/TextF.h>
#include <Xm/Text.h>
#include <Xm/Separator.h>
#include <Xm/SashP.h>
#include <Xm/AtomMgr.h>
#include <Xm/Protocols.h>

#ifdef __dimDialog_cc

static void kill_cb (
  Widget w,
  XtPointer client,
  XtPointer call );

#endif

#include "act_win.h"

class dimDialogClass {

public:

dimDialogClass();

~dimDialogClass( void );

int destroy ( void );

int create (
  activeWindowClass *ptr
);

int popup ( void );

int popdown ( void );

int dialogIsPoppedUp ( void );

int setX (
  int x
);

int setY (
  int y
);

int setLen (
  double len
);

int setAngle (
  double angle
);

int setRelAngle (
  double angle
);

int setObjX (
  int x
);

int setObjY (
  int y
);

int setObjW (
  int w
);

int setObjH (
  int h
);

int setObjTopDist (
  int d
);

int setObjBotDist (
  int d
);

int setObjLeftDist (
  int d
);

int setObjRightDist (
  int d
);

int getDistMode ( void );

private:

activeWindowClass *awo;

int distMode;
int isPoppedUp;
int widgetsCreated;

Widget shell, topForm, sep1, sep2, sep3,
 xLabel, xValue, yLabel, yValue, lenLabel, lenValue,
 angleLabel, angleValue, negAngleValue,
 relAngleLabel, relAngleValue, negRelAngleValue,
 objXLabel, objXValue, objYLabel, objYValue,
 objWLabel, objWValue, objHLabel, objHValue,
 objTopDistLabel, objTopDistValue,
 objBotDistLabel, objBotDistValue,
 objLeftDistLabel, objLeftDistValue,
 objRightDistLabel, objRightDistValue,
 optLabel, pdm, opt, pb1, pb2, pb3, pb4;

int xval, yval;
double lval, aval, raval;

friend void kill_cb (
  Widget w,
  XtPointer client,
  XtPointer call );

};

#endif

