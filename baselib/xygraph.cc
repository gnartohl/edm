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

#define __xygraph_cc 1

#include "xygraph.h"

// This is the EPICS specific line right now:
static PV_Factory *pv_factory = new EPICS_PV_Factory();

static void updateTimerAction (
  XtPointer client,
  XtIntervalId *id )
{

xyGraphClass *xyo = (xyGraphClass *) client;

  if ( !xyo->updateTimerActive ) {
    xyo->updateTimer = 0;
    return;
  }

  xyo->updateTimer = appAddTimeOut(
   xyo->actWin->appCtx->appContext(),
   xyo->updateTimerValue, updateTimerAction, client );

  xyo->actWin->appCtx->proc->lock();
  xyo->needRealUpdate = 1;
  xyo->actWin->addDefExeNode( xyo->aglPtr );
  xyo->actWin->appCtx->proc->unlock();

}

static void setKpXMinDoubleValue (
  Widget w,
  XtPointer client,
  XtPointer call ) {

xyGraphClass *xyo = (xyGraphClass *) client;

  //printf( "setKpXMinDoubleValue\n" );

  xyo->actWin->appCtx->proc->lock();
  xyo->kpXMinEfDouble.setValue( xyo->kpXMin );
  xyo->needXRescale = 1;

  if ( ( xyo->xAxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) ||
       ( xyo->xAxisStyle == XYGC_K_AXIS_STYLE_TIME_LOG10 ) ) {
    if ( xyo->kpXMin > 0 ) {
      xyo->xRescaleValue = log10(xyo->kpXMin);
    }
    else {
      xyo->xRescaleValue = 0;
    }
  }
  else {
    xyo->xRescaleValue = xyo->kpXMin;
  }

  xyo->actWin->addDefExeNode( xyo->aglPtr );
  xyo->actWin->appCtx->proc->unlock();

}

static void setKpXMaxDoubleValue (
  Widget w,
  XtPointer client,
  XtPointer call ) {

xyGraphClass *xyo = (xyGraphClass *) client;

  //printf( "setKpXMaxDoubleValue\n" );

  xyo->actWin->appCtx->proc->lock();
  xyo->kpXMaxEfDouble.setValue( xyo->kpXMax );
  xyo->needXRescale = 1;

  if ( ( xyo->xAxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) ||
       ( xyo->xAxisStyle == XYGC_K_AXIS_STYLE_TIME_LOG10 ) ) {
    if ( xyo->kpXMax > 0 ) {
      xyo->xRescaleValue = log10(xyo->kpXMax);
    }
    else {
      xyo->xRescaleValue = 0;
    }
  }
  else {
    xyo->xRescaleValue = xyo->kpXMax;
  }

  xyo->actWin->addDefExeNode( xyo->aglPtr );
  xyo->actWin->appCtx->proc->unlock();

}

static void cancelKpXMin (
  Widget w,
  XtPointer client,
  XtPointer call ) {

xyGraphClass *xyo = (xyGraphClass *) client;
int i, ii, first;
double dxValue, minValue;

  //printf( "cancelKpXMin\n" );

  xyo->actWin->appCtx->proc->lock();

  xyo->kpXMinEfDouble.setNull(1);

  if ( xyo->xAxisSource == XYGC_K_USER_SPECIFIED ) {

    minValue = xyo->xMin.value();

    if ( xyo->xAxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) {
      if ( minValue <= 0 ) minValue = 1;
      minValue = log10( minValue );
    }
    else if ( xyo->xAxisStyle == XYGC_K_AXIS_STYLE_TIME_LOG10 ) {
      if ( minValue <= 0 ) minValue = 1;
      minValue = log10( minValue );
    }

  }
  else if ( xyo->xAxisSource == XYGC_K_FROM_PV ) {

    minValue = xyo->dbXMin[0];
    for ( i=1; i<xyo->numTraces; i++ ) {
      if ( xyo->dbXMin[i] < minValue ) minValue = xyo->dbXMin[i];
    }

    if ( xyo->xAxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) {
      if ( minValue <= 0 ) minValue = 1;
      minValue = log10( minValue );
    }
    else if ( xyo->xAxisStyle == XYGC_K_AXIS_STYLE_TIME_LOG10 ) {
      if ( minValue <= 0 ) minValue = 1;
      minValue = log10( minValue );
    }

  }
  else { // auto scale

    // find min x value

    minValue = 0.9 * xyo->curXMax;
    if ( minValue > xyo->curXMax ) minValue = 1.1 * xyo->curXMax;
    first = 1;
    for ( i=0; i<xyo->numTraces; i++ ) {

      ii = xyo->arrayHead[i];
      while ( ii != xyo->arrayTail[i] ) {

        switch ( xyo->xPvType[i] ) {
        case DBR_FLOAT:
          dxValue = (double) ( (float *) xyo->xPvData[i] )[ii];
          break;
        case DBR_DOUBLE: 
          dxValue = ( (double *) xyo->xPvData[i] )[ii];
          break;
        case DBR_SHORT:
          dxValue = (double) ( (unsigned short *) xyo->xPvData[i] )[ii];
          break;
        case DBR_CHAR:
          dxValue = (double) ( (unsigned char *) xyo->xPvData[i] )[ii];
          break;
        case DBR_LONG:
          dxValue = (double) ( (int *) xyo->xPvData[i] )[ii];
          break;
        case DBR_ENUM:
          dxValue = (double) ( (unsigned short *) xyo->xPvData[i] )[ii];
          break;
        default:
          dxValue = ( (double *) xyo->xPvData[i] )[ii];
          break;
        }

        if ( first ) {
          first = 0;
          minValue = dxValue;
        }
        else {
          if ( dxValue < minValue ) minValue = dxValue;
        }

        ii++;
        if ( ii > xyo->plotBufSize[i] ) {
          ii = 0;
        }

      }

    }

    if ( xyo->xAxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) {
      if ( minValue <= 0 ) minValue = 1;
      minValue = log10( minValue );
    }
    else if ( xyo->xAxisStyle == XYGC_K_AXIS_STYLE_TIME_LOG10 ) {
      if ( minValue <= 0 ) minValue = 1;
      minValue = log10( minValue );
    }

  }

  //printf( "min value = %-g\n", minValue );

  xyo->needXRescale = 1;
  xyo->kpCancelMinX = 1;
  xyo->xRescaleValue = minValue;
  xyo->actWin->addDefExeNode( xyo->aglPtr );

  xyo->actWin->appCtx->proc->unlock();

}

static void cancelKpXMax (
  Widget w,
  XtPointer client,
  XtPointer call ) {

xyGraphClass *xyo = (xyGraphClass *) client;
int i, ii, first;
double dxValue, maxValue;

  //printf( "cancelKpXMax\n" );

  xyo->actWin->appCtx->proc->lock();

  xyo->kpXMaxEfDouble.setNull(1);

  if ( xyo->xAxisSource == XYGC_K_USER_SPECIFIED ) {

    maxValue = xyo->xMax.value();

    if ( xyo->xAxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) {
      if ( maxValue <= 0 ) maxValue = 1;
      maxValue = log10( maxValue );
    }
    else if ( xyo->xAxisStyle == XYGC_K_AXIS_STYLE_TIME_LOG10 ) {
      if ( maxValue <= 0 ) maxValue = 1;
      maxValue = log10( maxValue );
    }

  }
  else if ( xyo->xAxisSource == XYGC_K_FROM_PV ) {

    maxValue = xyo->dbXMax[0];
    for ( i=1; i<xyo->numTraces; i++ ) {
      if ( xyo->dbXMax[i] > maxValue ) maxValue = xyo->dbXMax[i];
    }

    if ( xyo->xAxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) {
      if ( maxValue <= 0 ) maxValue = 1;
      maxValue = log10( maxValue );
    }
    else if ( xyo->xAxisStyle == XYGC_K_AXIS_STYLE_TIME_LOG10 ) {
      if ( maxValue <= 0 ) maxValue = 1;
      maxValue = log10( maxValue );
    }

  }
  else { // auto scale

    // find max x value

    maxValue = 1.1 * xyo->curXMin;
    if ( maxValue < xyo->curXMin ) maxValue = 0.9 * xyo->curXMin;
    first = 1;
    for ( i=0; i<xyo->numTraces; i++ ) {

      ii = xyo->arrayHead[i];
      while ( ii != xyo->arrayTail[i] ) {

        switch ( xyo->xPvType[i] ) {
        case DBR_FLOAT:
          dxValue = (double) ( (float *) xyo->xPvData[i] )[ii];
          break;
        case DBR_DOUBLE: 
          dxValue = ( (double *) xyo->xPvData[i] )[ii];
          break;
        case DBR_SHORT:
          dxValue = (double) ( (unsigned short *) xyo->xPvData[i] )[ii];
          break;
        case DBR_CHAR:
          dxValue = (double) ( (unsigned char *) xyo->xPvData[i] )[ii];
          break;
        case DBR_LONG:
          dxValue = (double) ( (int *) xyo->xPvData[i] )[ii];
          break;
        case DBR_ENUM:
          dxValue = (double) ( (unsigned short *) xyo->xPvData[i] )[ii];
          break;
        default:
          dxValue = ( (double *) xyo->xPvData[i] )[ii];
          break;
        }

        if ( first ) {
          first = 0;
          maxValue = dxValue;
        }
        else {
          if ( dxValue > maxValue ) maxValue = dxValue;
        }

        ii++;
        if ( ii > xyo->plotBufSize[i] ) {
          ii = 0;
        }

      }

    }

    if ( xyo->xAxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) {
      if ( maxValue <= 0 ) maxValue = 1;
      maxValue = log10( maxValue );
    }
    else if ( xyo->xAxisStyle == XYGC_K_AXIS_STYLE_TIME_LOG10 ) {
      if ( maxValue <= 0 ) maxValue = 1;
      maxValue = log10( maxValue );
    }

  }

  //printf( "max value = %-g\n", maxValue );

  xyo->needXRescale = 1;
  xyo->kpCancelMaxX = 1;
  xyo->xRescaleValue = maxValue;
  xyo->actWin->addDefExeNode( xyo->aglPtr );

  xyo->actWin->appCtx->proc->unlock();

}

static void setKpY1MinDoubleValue (
  Widget w,
  XtPointer client,
  XtPointer call ) {

xyGraphClass *xyo = (xyGraphClass *) client;

  //printf( "setKpY1MinDoubleValue\n" );

  xyo->actWin->appCtx->proc->lock();
  xyo->kpY1MinEfDouble.setValue( xyo->kpY1Min );
  xyo->needY1Rescale = 1;

  if ( xyo->y1AxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) {
    if ( xyo->kpY1Min > 0 ) {
      xyo->y1RescaleValue = log10(xyo->kpY1Min);
    }
    else {
      xyo->y1RescaleValue = 0;
    }
  }
  else {
    xyo->y1RescaleValue = xyo->kpY1Min;
  }

  xyo->actWin->addDefExeNode( xyo->aglPtr );
  xyo->actWin->appCtx->proc->unlock();

}

static void setKpY1MaxDoubleValue (
  Widget w,
  XtPointer client,
  XtPointer call ) {

xyGraphClass *xyo = (xyGraphClass *) client;

  //printf( "setKpY1MaxDoubleValue\n" );

  xyo->actWin->appCtx->proc->lock();
  xyo->kpY1MaxEfDouble.setValue( xyo->kpY1Max );
  xyo->needY1Rescale = 1;

  if ( xyo->y1AxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) {
    if ( xyo->kpY1Max > 0 ) {
      xyo->y1RescaleValue = log10(xyo->kpY1Max);
    }
    else {
      xyo->y1RescaleValue = 0;
    }
  }
  else {
    xyo->y1RescaleValue = xyo->kpY1Max;
  }

  xyo->actWin->addDefExeNode( xyo->aglPtr );
  xyo->actWin->appCtx->proc->unlock();

}

static void cancelKpY1Min (
  Widget w,
  XtPointer client,
  XtPointer call ) {

xyGraphClass *xyo = (xyGraphClass *) client;
int i, ii, first;
double dy1Value, minValue;

  //printf( "cancelKpY1Min\n" );

  xyo->actWin->appCtx->proc->lock();

  xyo->kpY1MinEfDouble.setNull(1);

  if ( xyo->y1AxisSource == XYGC_K_USER_SPECIFIED ) {

    minValue = xyo->y1Min.value();

    if ( xyo->y1AxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) {
      if ( minValue <= 0 ) minValue = 1;
      minValue = log10( minValue );
    }

  }
  else if ( xyo->y1AxisSource == XYGC_K_FROM_PV ) {

    minValue = xyo->dbYMin[0];
    for ( i=1; i<xyo->numTraces; i++ ) {
      if ( xyo->dbYMin[i] < minValue ) minValue = xyo->dbYMin[i];
    }

    if ( xyo->y1AxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) {
      if ( minValue <= 0 ) minValue = 1;
      minValue = log10( minValue );
    }

  }
  else { // auto scale

    // find min y1 value

    minValue = 0.9 * xyo->curY1Max;
    if ( minValue > xyo->curY1Max ) minValue = 1.1 * xyo->curY1Max;
    first = 1;
    for ( i=0; i<xyo->numTraces; i++ ) {

      ii = xyo->arrayHead[i];
      while ( ii != xyo->arrayTail[i] ) {

        switch ( xyo->yPvType[i] ) {
        case DBR_FLOAT:
          dy1Value = (double) ( (float *) xyo->yPvData[i] )[ii];
          break;
        case DBR_DOUBLE: 
          dy1Value = ( (double *) xyo->yPvData[i] )[ii];
          break;
        case DBR_SHORT:
          dy1Value = (double) ( (unsigned short *) xyo->yPvData[i] )[ii];
          break;
        case DBR_CHAR:
          dy1Value = (double) ( (unsigned char *) xyo->yPvData[i] )[ii];
          break;
        case DBR_LONG:
          dy1Value = (double) ( (int *) xyo->yPvData[i] )[ii];
          break;
        case DBR_ENUM:
          dy1Value = (double) ( (unsigned short *) xyo->yPvData[i] )[ii];
          break;
        default:
          dy1Value = ( (double *) xyo->yPvData[i] )[ii];
          break;
        }

        if ( first ) {
          first = 0;
          minValue = dy1Value;
        }
        else {
          if ( dy1Value < minValue ) minValue = dy1Value;
        }

        ii++;
        if ( ii > xyo->plotBufSize[i] ) {
          ii = 0;
        }

      }

    }

    if ( xyo->y1AxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) {
      if ( minValue <= 0 ) minValue = 1;
      minValue = log10( minValue );
    }

  }

  //printf( "min value = %-g\n", minValue );

  xyo->needY1Rescale = 1;
  xyo->kpCancelMinY1 = 1;
  xyo->y1RescaleValue = minValue;
  xyo->actWin->addDefExeNode( xyo->aglPtr );

  xyo->actWin->appCtx->proc->unlock();

}

static void cancelKpY1Max (
  Widget w,
  XtPointer client,
  XtPointer call ) {

xyGraphClass *xyo = (xyGraphClass *) client;
int i, ii, first;
double dy1Value, maxValue;

  //printf( "cancelKpY1Max\n" );

  xyo->actWin->appCtx->proc->lock();

  xyo->kpY1MaxEfDouble.setNull(1);

  if ( xyo->y1AxisSource == XYGC_K_USER_SPECIFIED ) {

    maxValue = xyo->y1Max.value();

    if ( xyo->y1AxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) {
      if ( maxValue <= 0 ) maxValue = 1;
      maxValue = log10( maxValue );
    }

  }
  else if ( xyo->y1AxisSource == XYGC_K_FROM_PV ) {

    maxValue = xyo->dbYMax[0];
    for ( i=1; i<xyo->numTraces; i++ ) {
      if ( xyo->dbYMax[i] > maxValue ) maxValue = xyo->dbYMax[i];
    }

    if ( xyo->y1AxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) {
      if ( maxValue <= 0 ) maxValue = 1;
      maxValue = log10( maxValue );
    }

  }
  else { // auto scale

    // find max y1 value

    maxValue = 1.1 * xyo->curY1Min;
    if ( maxValue < xyo->curY1Min ) maxValue = 0.9 * xyo->curY1Min;
    first = 1;
    for ( i=0; i<xyo->numTraces; i++ ) {

      ii = xyo->arrayHead[i];
      while ( ii != xyo->arrayTail[i] ) {

        switch ( xyo->yPvType[i] ) {
        case DBR_FLOAT:
          dy1Value = (double) ( (float *) xyo->yPvData[i] )[ii];
          break;
        case DBR_DOUBLE: 
          dy1Value = ( (double *) xyo->yPvData[i] )[ii];
          break;
        case DBR_SHORT:
          dy1Value = (double) ( (unsigned short *) xyo->yPvData[i] )[ii];
          break;
        case DBR_CHAR:
          dy1Value = (double) ( (unsigned char *) xyo->yPvData[i] )[ii];
          break;
        case DBR_LONG:
          dy1Value = (double) ( (int *) xyo->yPvData[i] )[ii];
          break;
        case DBR_ENUM:
          dy1Value = (double) ( (unsigned short *) xyo->yPvData[i] )[ii];
          break;
        default:
          dy1Value = ( (double *) xyo->yPvData[i] )[ii];
          break;
        }

        if ( first ) {
          first = 0;
          maxValue = dy1Value;
        }
        else {
          if ( dy1Value > maxValue ) maxValue = dy1Value;
        }

        ii++;
        if ( ii > xyo->plotBufSize[i] ) {
          ii = 0;
        }

      }

    }

    if ( xyo->y1AxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) {
      if ( maxValue <= 0 ) maxValue = 1;
      maxValue = log10( maxValue );
    }

  }

  //printf( "max value = %-g\n", maxValue );

  xyo->needY1Rescale = 1;
  xyo->kpCancelMaxY1 = 1;
  xyo->y1RescaleValue = maxValue;
  xyo->actWin->addDefExeNode( xyo->aglPtr );

  xyo->actWin->appCtx->proc->unlock();

}

static void resetMonitorConnection (
  struct connection_handler_args arg )
{

xyGraphClass *xyo = (xyGraphClass *) ca_puser(arg.chid);

  if ( arg.op == CA_OP_CONN_UP ) {

    xyo->actWin->appCtx->proc->lock();
    xyo->needResetConnect = 1;
    xyo->actWin->addDefExeNode( xyo->aglPtr );
    xyo->actWin->appCtx->proc->unlock();

  }

}

static void resetValueUpdate (
  struct event_handler_args arg )
{

xyGraphClass *xyo = (xyGraphClass *) ca_puser(arg.chid);
short value;

  value = *( (short *) arg.dbr );
  if ( (  value && ( xyo->resetMode == XYGC_K_RESET_MODE_IF_NOT_ZERO ) ) ||
       ( !value && ( xyo->resetMode == XYGC_K_RESET_MODE_IF_ZERO ) ) ) {

    xyo->actWin->appCtx->proc->lock();
    xyo->needReset = 1;
    xyo->actWin->addDefExeNode( xyo->aglPtr );
    xyo->actWin->appCtx->proc->unlock();

  }

}

static void xMonitorConnection (
  struct connection_handler_args arg )
{

objPlusIndexPtr ptr = (objPlusIndexPtr) ca_puser(arg.chid);
xyGraphClass *xyo = (xyGraphClass *) ptr->objPtr;

  if ( arg.op == CA_OP_CONN_UP ) {

    if ( !xyo->connection.pvsConnected() ) {

      xyo->connection.setPvConnected( (void *) ptr->index );
      if ( xyo->connection.pvsConnected() ) {

        xyo->actWin->appCtx->proc->lock();
        xyo->needConnect = 1;
        xyo->actWin->addDefExeNode( xyo->aglPtr );
        xyo->actWin->appCtx->proc->unlock();

      }

    }

  }
  else {

    xyo->connection.setPvDisconnected( (void *) ptr->index );
    xyo->actWin->appCtx->proc->lock();
    xyo->active = 0;
    xyo->bufInvalidate();
    xyo->needErase = 1;
    xyo->needDraw = 1;
    xyo->actWin->addDefExeNode( xyo->aglPtr );
    xyo->actWin->appCtx->proc->unlock();


  }

}

static void xInfoUpdate (
  struct event_handler_args arg )
{

objPlusIndexPtr ptr = (objPlusIndexPtr) ca_puser(arg.chid);
xyGraphClass *xyo = (xyGraphClass *) ptr->objPtr;
struct dbr_gr_double grRec = *( (dbr_gr_double *) arg.dbr );
int i =  ptr->index;

  xyo->dbXMin[i] = grRec.lower_disp_limit;
  xyo->dbXMax[i] = grRec.upper_disp_limit;
  xyo->dbXPrec[i] = grRec.precision;

  xyo->actWin->appCtx->proc->lock();
  xyo->needInit = 1;
  xyo->xArrayNeedInit[i] = 1;
  xyo->actWin->addDefExeNode( xyo->aglPtr );
  xyo->actWin->appCtx->proc->unlock();

}

static void xValueUpdate (
  struct event_handler_args arg )
{

objPlusIndexPtr ptr = (objPlusIndexPtr) ca_puser(arg.chid);
xyGraphClass *xyo = (xyGraphClass *) ptr->objPtr;
int i =  ptr->index;
int ii;
char *xArray, *yArray;
double dxValue, dyValue;
short scaledX, scaledY;

  //printf( "xValueUpdate, i=%-d, size=%-d\n", i, xyo->xPvSize[i] );

  if ( !xyo->activeMode ) return;

  xyo->actWin->appCtx->proc->lock();

  switch ( xyo->traceType[i] ) {

  case XYGC_K_TRACE_XY:

    if ( xyo->xPvCount[i] > 1 ) { // vector

      for ( ii=0; ii<xyo->xPvCount[i]; ii++ ) {

        switch ( xyo->xPvType[i] ) {

        case DBR_FLOAT:
          ( (float *) xyo->xPvData[i] )[ii] = ( (float *) arg.dbr )[ii];
          break;

        case DBR_DOUBLE: 
          ( (double *) xyo->xPvData[i] )[ii] = ( (double *) arg.dbr )[ii];
          break;

        case DBR_SHORT:
          ( (short *) xyo->xPvData[i] )[ii] = ( (short *) arg.dbr )[ii];
          break;

        case DBR_CHAR:
          ( (char *) xyo->xPvData[i] )[ii] = ( (char *) arg.dbr )[ii];
          break;

        case DBR_LONG:
          ( (long *) xyo->xPvData[i] )[ii] = ( (long *) arg.dbr )[ii];
          break;

        case DBR_ENUM:
          ( (short *) xyo->xPvData[i] )[ii] = ( (short *) arg.dbr )[ii];
          break;

        default:
          ( (double *) xyo->xPvData[i] )[ii] = ( (double *) arg.dbr )[ii];
          break;

        }

      }

      xyo->xArrayGotValue[i] = 1;
      xyo->xArrayNeedUpdate[i] = 1;
      xyo->needVectorUpdate = 1;
      xyo->actWin->addDefExeNode( xyo->aglPtr );

    }
    else { // scalar

      //printf( "scalar\n" );
      //printf( "head = %-d, tail = %-d\n", xyo->arrayHead[i],
      // xyo->arrayTail[i] );

      memcpy( (void *) &xyo->xPvCurValue[i], (void *) arg.dbr,
       xyo->xPvSize[i] ); // save cur value for y event

      if ( ( xyo->arrayNumPoints[i] >= xyo->count ) &&
           ( xyo->plotMode == XYGC_K_PLOT_MODE_PLOT_N_STOP ) ) {
        xyo->actWin->appCtx->proc->unlock();
        return;
      }

      // x
      ii = xyo->arrayTail[i] * xyo->xPvSize[i];
      xArray = (char *) xyo->xPvData[i];
      memcpy( (void *) &xArray[ii], (void *) arg.dbr, xyo->xPvSize[i] );

      if ( ( xyo->plotUpdateMode[i] == XYGC_K_UPDATE_ON_X_OR_Y ) ||
           ( xyo->plotUpdateMode[i] == XYGC_K_UPDATE_ON_X ) ) {

        // y
        ii = xyo->arrayTail[i] * xyo->yPvSize[i];
        yArray = (char *) xyo->yPvData[i];
        memcpy( (void *) &yArray[ii], (void *) &xyo->yPvCurValue[i],
         xyo->yPvSize[i] ); // get cur value of y
        xyo->yArrayGotValue[i] = 1;

      }

      if ( ( xyo->plotUpdateMode[i] != XYGC_K_UPDATE_ON_Y ) &&
             xyo->yArrayGotValue[i] ) {

        ii = xyo->arrayTail[i];

        switch ( xyo->yPvType[i] ) {
        case DBR_FLOAT:
          dyValue = (double) ( (float *) xyo->yPvData[i] )[ii];
          break;
        case DBR_DOUBLE: 
          dyValue = ( (double *) xyo->yPvData[i] )[ii];
          break;
        case DBR_SHORT:
          dyValue = (double) ( (unsigned short *) xyo->yPvData[i] )[ii];
          break;
        case DBR_CHAR:
          dyValue = (double) ( (unsigned char *) xyo->yPvData[i] )[ii];
          break;
        case DBR_LONG:
          dyValue = (double) ( (int *) xyo->yPvData[i] )[ii];
          break;
        case DBR_ENUM:
          dyValue = (double) ( (unsigned short *) xyo->yPvData[i] )[ii];
          break;
        default:
          dyValue = ( (double *) xyo->yPvData[i] )[ii];
          break;
        }

        if ( xyo->y1AxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) {
          if ( dyValue > 0 ) dyValue = log10( dyValue );
        }

        if ( xyo->y1AxisSource == XYGC_K_AUTOSCALE ) {
          if ( xyo->kpY1MinEfDouble.isNull() ) {
            if ( dyValue < xyo->curY1Min ) {
              xyo->needY1Rescale = 1;
              xyo->y1RescaleValue = dyValue;
              xyo->actWin->addDefExeNode( xyo->aglPtr );
            }
          }
          if ( xyo->kpY1MaxEfDouble.isNull() ) {
            if ( dyValue > xyo->curY1Max ) {
              xyo->needY1Rescale = 1;
              xyo->y1RescaleValue = dyValue;
              xyo->actWin->addDefExeNode( xyo->aglPtr );
            }
          }
        }

        scaledY = (short) xyo->plotAreaH -
         (short) rint( ( dyValue - xyo->curY1Min ) *
         xyo->y1Factor[i] - xyo->y1Offset[i] );

        switch ( xyo->xPvType[i] ) {
        case DBR_FLOAT:
          dxValue = (double) ( (float *) xyo->xPvData[i] )[ii];
          break;
        case DBR_DOUBLE: 
          dxValue = ( (double *) xyo->xPvData[i] )[ii];
          break;
        case DBR_SHORT:
          dxValue = (double) ( (unsigned short *) xyo->xPvData[i] )[ii];
          break;
        case DBR_CHAR:
          dxValue = (double) ( (unsigned char *) xyo->xPvData[i] )[ii];
          break;
        case DBR_LONG:
          dxValue = (double) ( (int *) xyo->xPvData[i] )[ii];
          break;
        case DBR_ENUM:
          dxValue = (double) ( (unsigned short *) xyo->xPvData[i] )[ii];
          break;
        default:
          dxValue = ( (double *) xyo->xPvData[i] )[ii];
          break;
        }

        if ( xyo->xAxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) {
          if ( dxValue > 0 ) dxValue = log10( dxValue );
        }
        else if ( xyo->xAxisStyle == XYGC_K_AXIS_STYLE_TIME_LOG10 ) {
          if ( dxValue > 0 ) dxValue  = log10( dxValue );
        }

        if ( xyo->xAxisSource == XYGC_K_AUTOSCALE ) {
          if ( xyo->kpXMinEfDouble.isNull() ) {
            if ( dxValue < xyo->curXMin ) {
              xyo->needXRescale = 1;
              xyo->xRescaleValue = dxValue;
              xyo->actWin->addDefExeNode( xyo->aglPtr );
	    }
	  }
          if ( xyo->kpXMaxEfDouble.isNull() ) {
            if ( dxValue > xyo->curXMax ) {
              xyo->needXRescale = 1;
              xyo->xRescaleValue = dxValue;
              xyo->actWin->addDefExeNode( xyo->aglPtr );
	    }
	  }
	}

        scaledX = (short) rint( ( dxValue - xyo->curXMin ) *
         xyo->xFactor[i] + xyo->xOffset[i] );

        xyo->addPoint( dxValue, scaledX, scaledY, i );

        xyo->yArrayGotValue[i] = xyo->xArrayGotValue[i] = 0;

        xyo->arrayTail[i]++;
        if ( xyo->arrayTail[i] > xyo->plotBufSize[i] ) {
          xyo->arrayTail[i] = 0;
        }
        if ( xyo->arrayTail[i] == xyo->arrayHead[i] ) {
          xyo->arrayHead[i]++;
          if ( xyo->arrayHead[i] > xyo->plotBufSize[i] ) {
            xyo->arrayHead[i] = 0;
          }
        }

        if ( xyo->arrayNumPoints[i] > xyo->count ) {
          xyo->needBufferScroll = 1;
          xyo->needThisbufScroll[i] = 1;
	}

        xyo->needUpdate = 1;
        xyo->xArrayNeedUpdate[i] = 1;
        xyo->yArrayNeedUpdate[i] = 1;
        xyo->actWin->addDefExeNode( xyo->aglPtr );

      }
      else {

        xyo->xArrayGotValue[i] = 1;

      }

    }

    break;

  case XYGC_K_TRACE_CHRONOLOGICAL:

    printf( "error XYGC_K_TRACE_CHRONOLOGICAL in xValueUpdate\n" );

    break;

  }

  xyo->actWin->appCtx->proc->unlock();

}

static void yMonitorConnection (
  struct connection_handler_args arg )
{

objPlusIndexPtr ptr = (objPlusIndexPtr) ca_puser(arg.chid);
xyGraphClass *xyo = (xyGraphClass *) ptr->objPtr;

  if ( arg.op == CA_OP_CONN_UP ) {

    if ( !xyo->connection.pvsConnected() ) {

      xyo->connection.setPvConnected(
       (void *) ( ptr->index + XYGC_K_MAX_TRACES ) );

      if ( xyo->connection.pvsConnected() ) {

        xyo->actWin->appCtx->proc->lock();
        xyo->needConnect = 1;
        xyo->actWin->addDefExeNode( xyo->aglPtr );
        xyo->actWin->appCtx->proc->unlock();

      }

    }

  }
  else {

    xyo->connection.setPvDisconnected( (void *) ptr->index );
    xyo->actWin->appCtx->proc->lock();
    xyo->active = 0;
    xyo->bufInvalidate();
    xyo->needErase = 1;
    xyo->needDraw = 1;
    xyo->actWin->addDefExeNode( xyo->aglPtr );
    xyo->actWin->appCtx->proc->unlock();


  }

}

static void yInfoUpdate (
  struct event_handler_args arg )
{

objPlusIndexPtr ptr = (objPlusIndexPtr) ca_puser(arg.chid);
xyGraphClass *xyo = (xyGraphClass *) ptr->objPtr;
struct dbr_gr_double grRec = *( (dbr_gr_double *) arg.dbr );
int i =  ptr->index;

  //printf( "yInfoUpdate, i=%-d\n", i );

  xyo->dbYMin[i] = grRec.lower_disp_limit;
  xyo->dbYMax[i] = grRec.upper_disp_limit;
  xyo->dbYPrec[i] = grRec.precision;

  xyo->actWin->appCtx->proc->lock();
  xyo->needInit = 1;
  xyo->yArrayNeedInit[i] = 1;
  xyo->actWin->addDefExeNode( xyo->aglPtr );
  xyo->actWin->appCtx->proc->unlock();

}

static void yValueUpdate (
  struct event_handler_args arg )
{

objPlusIndexPtr ptr = (objPlusIndexPtr) ca_puser(arg.chid);
xyGraphClass *xyo = (xyGraphClass *) ptr->objPtr;
int ii, i =  ptr->index;
char *xArray, *yArray;
double dxValue, dyValue;
short scaledX, scaledY;

  //printf( "yValueUpdate, i=%-d, size=%-d\n", i, xyo->yPvSize[i] );

  if ( !xyo->activeMode ) return;

  xyo->actWin->appCtx->proc->lock();

  switch ( xyo->traceType[i] ) {

  case XYGC_K_TRACE_XY:

    if ( xyo->yPvCount[i] > 1 ) { // vector

      //memcpy( xyo->yPvData[i], arg.dbr, xyo->yPvSize[i] );
      //xyo->arrayNumPoints[i] = xyo->yPvCount[i];

      for ( ii=0; ii<xyo->yPvCount[i]; ii++ ) {

        switch ( xyo->yPvType[i] ) {

        case DBR_FLOAT:
          ( (float *) xyo->yPvData[i] )[ii] = ( (float *) arg.dbr )[ii];
          break;

        case DBR_DOUBLE: 
          ( (double *) xyo->yPvData[i] )[ii] = ( (double *) arg.dbr )[ii];
          break;

        case DBR_SHORT:
          ( (short *) xyo->yPvData[i] )[ii] = ( (short *) arg.dbr )[ii];
          break;

        case DBR_CHAR:
          ( (char *) xyo->yPvData[i] )[ii] = ( (char *) arg.dbr )[ii];
          break;

        case DBR_LONG:
          ( (long *) xyo->yPvData[i] )[ii] = ( (long *) arg.dbr )[ii];
          break;

        case DBR_ENUM:
          ( (short *) xyo->yPvData[i] )[ii] = ( (short *) arg.dbr )[ii];
          break;

        default:
          ( (double *) xyo->yPvData[i] )[ii] = ( (double *) arg.dbr )[ii];
          break;

        }

      }

      xyo->yArrayGotValue[i] = 1;
      xyo->yArrayNeedUpdate[i] = 1;
      xyo->needVectorUpdate = 1;
      xyo->actWin->addDefExeNode( xyo->aglPtr );

    }
    else { // scalar

      //printf( "scalar\n" );
      //printf( "head = %-d, tail = %-d\n", xyo->arrayHead[i],
      // xyo->arrayTail[i] );

      memcpy( (void *) &xyo->yPvCurValue[i], (void *) arg.dbr,
       xyo->yPvSize[i] ); // save cur value for x event

      if ( ( xyo->arrayNumPoints[i] >= xyo->count ) &&
           ( xyo->plotMode == XYGC_K_PLOT_MODE_PLOT_N_STOP ) ) {
        xyo->actWin->appCtx->proc->unlock();
        return;
      }

      // y
      ii = xyo->arrayTail[i] * xyo->yPvSize[i];
      yArray = (char *) xyo->yPvData[i];
      memcpy( (void *) &yArray[ii], (void *) arg.dbr, xyo->yPvSize[i] );

      if ( ( xyo->plotUpdateMode[i] == XYGC_K_UPDATE_ON_X_OR_Y ) ||
           ( xyo->plotUpdateMode[i] == XYGC_K_UPDATE_ON_Y ) ) {

        // x
        ii = xyo->arrayTail[i] * xyo->xPvSize[i];
        xArray = (char *) xyo->xPvData[i];
        memcpy( (void *) &xArray[ii], (void *) &xyo->xPvCurValue[i],
         xyo->xPvSize[i] ); // get cur value of x
        xyo->xArrayGotValue[i] = 1;

      }

      if ( ( xyo->plotUpdateMode[i] != XYGC_K_UPDATE_ON_X ) &&
             xyo->xArrayGotValue[i] ) {

        ii = xyo->arrayTail[i];

        switch ( xyo->yPvType[i] ) {
        case DBR_FLOAT:
          dyValue = (double) ( (float *) xyo->yPvData[i] )[ii];
          break;
        case DBR_DOUBLE: 
          dyValue = ( (double *) xyo->yPvData[i] )[ii];
          break;
        case DBR_SHORT:
          dyValue = (double) ( (unsigned short *) xyo->yPvData[i] )[ii];
          break;
        case DBR_CHAR:
          dyValue = (double) ( (unsigned char *) xyo->yPvData[i] )[ii];
          break;
        case DBR_LONG:
          dyValue = (double) ( (int *) xyo->yPvData[i] )[ii];
          break;
        case DBR_ENUM:
          dyValue = (double) ( (unsigned short *) xyo->yPvData[i] )[ii];
          break;
        default:
          dyValue = ( (double *) xyo->yPvData[i] )[ii];
          break;
        }

        if ( xyo->y1AxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) {
          if ( dyValue > 0 ) dyValue = log10( dyValue );
        }

        if ( xyo->y1AxisSource == XYGC_K_AUTOSCALE ) {
          if ( xyo->kpY1MinEfDouble.isNull() ) {
            if ( dyValue < xyo->curY1Min ) {
              xyo->needY1Rescale = 1;
              xyo->y1RescaleValue = dyValue;
              xyo->actWin->addDefExeNode( xyo->aglPtr );
            }
          }
          if ( xyo->kpY1MaxEfDouble.isNull() ) {
            if ( dyValue > xyo->curY1Max ) {
              xyo->needY1Rescale = 1;
              xyo->y1RescaleValue = dyValue;
              xyo->actWin->addDefExeNode( xyo->aglPtr );
            }
          }
        }

        scaledY = (short) xyo->plotAreaH -
         (short) rint( ( dyValue - xyo->curY1Min ) *
         xyo->y1Factor[i] - xyo->y1Offset[i] );

        switch ( xyo->xPvType[i] ) {
        case DBR_FLOAT:
          dxValue = (double) ( (float *) xyo->xPvData[i] )[ii];
          break;
        case DBR_DOUBLE: 
          dxValue = ( (double *) xyo->xPvData[i] )[ii];
          break;
        case DBR_SHORT:
          dxValue = (double) ( (unsigned short *) xyo->xPvData[i] )[ii];
          break;
        case DBR_CHAR:
          dxValue = (double) ( (unsigned char *) xyo->xPvData[i] )[ii];
          break;
        case DBR_LONG:
          dxValue = (double) ( (int *) xyo->xPvData[i] )[ii];
          break;
        case DBR_ENUM:
          dxValue = (double) ( (unsigned short *) xyo->xPvData[i] )[ii];
          break;
        default:
          dxValue = ( (double *) xyo->xPvData[i] )[ii];
          break;
        }

        if ( xyo->xAxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) {
          if ( dxValue > 0 ) dxValue = log10( dxValue );
        }
        else if ( xyo->xAxisStyle == XYGC_K_AXIS_STYLE_TIME_LOG10 ) {
          if ( dxValue > 0 ) dxValue  = log10( dxValue );
        }

        if ( xyo->xAxisSource == XYGC_K_AUTOSCALE ) {
          if ( xyo->kpXMinEfDouble.isNull() ) {
            if ( dxValue < xyo->curXMin ) {
              xyo->needXRescale = 1;
              xyo->xRescaleValue = dxValue;
              xyo->actWin->addDefExeNode( xyo->aglPtr );
	    }
	  }
          if ( xyo->kpXMaxEfDouble.isNull() ) {
            if ( dxValue > xyo->curXMax ) {
              xyo->needXRescale = 1;
              xyo->xRescaleValue = dxValue;
              xyo->actWin->addDefExeNode( xyo->aglPtr );
	    }
	  }
	}

        scaledX = (short) rint( ( dxValue - xyo->curXMin ) *
         xyo->xFactor[i] + xyo->xOffset[i] );

        xyo->addPoint( dxValue, scaledX, scaledY, i );

        xyo->yArrayGotValue[i] = xyo->xArrayGotValue[i] = 0;

        xyo->arrayTail[i]++;
        if ( xyo->arrayTail[i] > xyo->plotBufSize[i] ) {
          xyo->arrayTail[i] = 0;
        }
        if ( xyo->arrayTail[i] == xyo->arrayHead[i] ) {
          xyo->arrayHead[i]++;
          if ( xyo->arrayHead[i] > xyo->plotBufSize[i] ) {
            xyo->arrayHead[i] = 0;
          }
        }

        if ( xyo->arrayNumPoints[i] > xyo->count ) {
          xyo->needBufferScroll = 1;
          xyo->needThisbufScroll[i] = 1;
	}

        xyo->needUpdate = 1;
        xyo->xArrayNeedUpdate[i] = 1;
        xyo->yArrayNeedUpdate[i] = 1;
        xyo->actWin->addDefExeNode( xyo->aglPtr );

      }
      else {

        xyo->yArrayGotValue[i] = 1;

      }

    }

    break;

  case XYGC_K_TRACE_CHRONOLOGICAL:

    printf( "XYGC_K_TRACE_CHRONOLOGICAL not implemented in yValueUpdate\n" );

    break;

  }

  xyo->actWin->appCtx->proc->unlock();

}

static void yValueWithTimeUpdate (
  struct event_handler_args arg )
{

objPlusIndexPtr ptr = (objPlusIndexPtr) ca_puser(arg.chid);
xyGraphClass *xyo = (xyGraphClass *) ptr->objPtr;
int ii, i =  ptr->index;
int sec, nsec;
double dxValue, dyValue;
short scaledX, scaledY;
//struct timespec ts;
struct dbr_time_float *dbrFltPtr;
struct dbr_time_double *dbrDblPtr;
struct dbr_time_short *dbrShrtPtr;
struct dbr_time_char *dbrChrPtr;
struct dbr_time_long *dbrLngPtr;
struct dbr_time_enum *dbrEnumPtr;

  //printf( "yValueWithTimeUpdate, i=%-d, size=%-d\n", i, xyo->yPvSize[i] );

  if ( !xyo->activeMode ) return;

  xyo->actWin->appCtx->proc->lock();

  switch ( xyo->traceType[i] ) {

  case XYGC_K_TRACE_XY:

    printf( "xy illegal\n" );
    break;

  case XYGC_K_TRACE_CHRONOLOGICAL:

    if ( xyo->yPvCount[i] > 1 ) { // vector

      for ( ii=0; ii<xyo->yPvCount[i]; ii++ ) {

        switch ( xyo->yPvType[i] ) {

        case DBR_FLOAT:
          ( (float *) xyo->yPvData[i] )[ii] = ( (float *) arg.dbr )[ii];
          break;

        case DBR_DOUBLE: 
          ( (double *) xyo->yPvData[i] )[ii] = ( (double *) arg.dbr )[ii];
          break;

        case DBR_SHORT:
          ( (short *) xyo->yPvData[i] )[ii] = ( (short *) arg.dbr )[ii];
          break;

        case DBR_CHAR:
          ( (char *) xyo->yPvData[i] )[ii] = ( (char *) arg.dbr )[ii];
          break;

        case DBR_LONG:
          ( (long *) xyo->yPvData[i] )[ii] = ( (long *) arg.dbr )[ii];
          break;

        case DBR_ENUM:
          ( (short *) xyo->yPvData[i] )[ii] = ( (short *) arg.dbr )[ii];
          break;

        default:
          ( (double *) xyo->yPvData[i] )[ii] = ( (double *) arg.dbr )[ii];
          break;

        }

        dxValue = (double) ii;
        ( (double *) xyo->xPvData[i] )[ii] = dxValue;

      }

      xyo->yArrayNeedUpdate[i] = 1;
      xyo->needVectorUpdate = 1;
      xyo->actWin->addDefExeNode( xyo->aglPtr );

    }
    else { // scalar

      //printf( "scalar\n" );
      //printf( "head = %-d, tail = %-d\n", xyo->arrayHead[i],
      // xyo->arrayTail[i] );

      if ( ( xyo->arrayNumPoints[i] >= xyo->count ) &&
           ( xyo->plotMode == XYGC_K_PLOT_MODE_PLOT_N_STOP ) ) {
        xyo->actWin->appCtx->proc->unlock();
        return;
      }

      ii = xyo->arrayTail[i];

      switch ( xyo->yPvType[i] ) {

      case DBR_FLOAT:
        dbrFltPtr = (struct dbr_time_float *) arg.dbr;
        ( (float *) xyo->yPvData[i] )[ii] = dbrFltPtr->value;
        dyValue = (double) dbrFltPtr->value;
        sec = dbrFltPtr->stamp.secPastEpoch;
        nsec = dbrFltPtr->stamp.nsec;
        break;

      case DBR_DOUBLE: 
        dbrDblPtr = (struct dbr_time_double *) arg.dbr;
        ( (double *) xyo->yPvData[i] )[ii] = dbrDblPtr->value;
        dyValue = dbrDblPtr->value;
        sec = dbrDblPtr->stamp.secPastEpoch;
        nsec = dbrDblPtr->stamp.nsec;
        break;

      case DBR_SHORT:
        dbrShrtPtr = (struct dbr_time_short *) arg.dbr;
        ( (short *) xyo->yPvData[i] )[ii] = dbrShrtPtr->value;
        dyValue = (double) dbrShrtPtr->value;
        sec = dbrShrtPtr->stamp.secPastEpoch;
        nsec = dbrShrtPtr->stamp.nsec;
        break;

      case DBR_CHAR:
        dbrChrPtr = (struct dbr_time_char *) arg.dbr;
        ( (char *) xyo->yPvData[i] )[ii] = dbrChrPtr->value;
        dyValue = (double) dbrChrPtr->value;
        sec = dbrChrPtr->stamp.secPastEpoch;
        nsec = dbrChrPtr->stamp.nsec;
        break;

      case DBR_LONG:
        dbrLngPtr = (struct dbr_time_long *) arg.dbr;
        ( (long *) xyo->yPvData[i] )[ii] = dbrLngPtr->value;
        dyValue = (double) dbrLngPtr->value;
        sec = dbrLngPtr->stamp.secPastEpoch;
        nsec = dbrLngPtr->stamp.nsec;
        break;

      case DBR_ENUM:
        dbrEnumPtr = (struct dbr_time_enum *) arg.dbr;
        ( (short *) xyo->yPvData[i] )[ii] = dbrEnumPtr->value;
        dyValue = (double) dbrEnumPtr->value;
        sec = dbrEnumPtr->stamp.secPastEpoch;
        nsec = dbrEnumPtr->stamp.nsec;
        break;

      default:
        dbrDblPtr = (struct dbr_time_double *) arg.dbr;
        ( (double *) xyo->yPvData[i] )[ii] = dbrDblPtr->value;
        dyValue = dbrDblPtr->value;
        sec = dbrDblPtr->stamp.secPastEpoch;
        nsec = dbrDblPtr->stamp.nsec;
        break;

      }

      if ( xyo->y1AxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) {
        if ( dyValue > 0 ) dyValue = log10( dyValue );
      }

      //printf( "pv: sec = %-d, nsec = %-d\n", sec, nsec );

      if ( xyo->firstTimeSample ) {
        //time( &ts.tv_sec );
        //sec = ts.tv_sec - 631152000;
        //nsec = 0;
	//printf( "cur: sec = %-d, nsec = %-d\n", sec, nsec );
        xyo->firstTimeSample = 0;
        xyo->curSec = sec;
        xyo->curNsec = nsec;
        sec = nsec = 0;
      }
      else {
        sec -= xyo->curSec;
        nsec -= xyo->curNsec;
      }

      if ( ( xyo->xAxisStyle == XYGC_K_AXIS_STYLE_TIME ) ||
           ( xyo->xAxisStyle == XYGC_K_AXIS_STYLE_STRIPCHART ) ) {

        dxValue = (double) sec + (double) nsec * 0.000000001;
        ( (double *) xyo->xPvData[i] )[ii] = dxValue;

      }
      else if ( xyo->xAxisStyle == XYGC_K_AXIS_STYLE_TIME_LOG10 ) {

        dxValue = (double) sec + (double) nsec * 0.000000001;
        ( (double *) xyo->xPvData[i] )[ii] = dxValue;
        if ( dxValue > 0 ) dxValue = log10( dxValue );

      }
      else if ( xyo->xAxisStyle == XYGC_K_AXIS_STYLE_LINEAR ) {

        dxValue = (double) ++(xyo->totalCount[i]);
        ( (double *) xyo->xPvData[i] )[ii] = dxValue;

      }
      else if ( xyo->xAxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) {

        dxValue = (double) ++(xyo->totalCount[i]);
        ( (double *) xyo->xPvData[i] )[ii] = dxValue;
        if ( dxValue > 0 ) dxValue = log10( dxValue );

      }

      //printf( "dxValue = %-g, dyValue = %-g\n", dxValue, dyValue );

      if ( xyo->xAxisSource == XYGC_K_AUTOSCALE ) {
        if ( xyo->kpXMinEfDouble.isNull() ) {
          if ( dxValue < xyo->curXMin ) {
            xyo->needXRescale = 1;
            xyo->xRescaleValue = dxValue;
            xyo->actWin->addDefExeNode( xyo->aglPtr );
          }
        }
        if ( xyo->kpXMaxEfDouble.isNull() ) {
          if ( dxValue > xyo->curXMax ) {
            xyo->needXRescale = 1;
            xyo->xRescaleValue = dxValue;
            xyo->actWin->addDefExeNode( xyo->aglPtr );
          }
	}
      }

      if ( xyo->y1AxisSource == XYGC_K_AUTOSCALE ) {
        if ( xyo->kpY1MinEfDouble.isNull() ) {
          if ( dyValue < xyo->curY1Min ) {
            xyo->needY1Rescale = 1;
            xyo->y1RescaleValue = dyValue;
            xyo->actWin->addDefExeNode( xyo->aglPtr );
          }
        }
        if ( xyo->kpY1MaxEfDouble.isNull() ) {
          if ( dyValue > xyo->curY1Max ) {
            xyo->needY1Rescale = 1;
            xyo->y1RescaleValue = dyValue;
            xyo->actWin->addDefExeNode( xyo->aglPtr );
          }
        }
      }

      scaledY = (short) xyo->plotAreaH -
       (short) rint( ( dyValue - xyo->curY1Min ) *
       xyo->y1Factor[i] - xyo->y1Offset[i] );

      scaledX = (short) rint( ( dxValue - xyo->curXMin ) *
       xyo->xFactor[i] + xyo->xOffset[i] );

      xyo->addPoint( dxValue, scaledX, scaledY, i );

      xyo->yArrayGotValue[i] = xyo->xArrayGotValue[i] = 0;

      xyo->arrayTail[i]++;
      if ( xyo->arrayTail[i] > xyo->plotBufSize[i] ) {
        xyo->arrayTail[i] = 0;
      }
      if ( xyo->arrayTail[i] == xyo->arrayHead[i] ) {
        xyo->arrayHead[i]++;
        if ( xyo->arrayHead[i] > xyo->plotBufSize[i] ) {
          xyo->arrayHead[i] = 0;
        }
      }

      if ( xyo->arrayNumPoints[i] > xyo->count ) {
        xyo->needBufferScroll = 1;
        xyo->needThisbufScroll[i] = 1;
      }

      xyo->needUpdate = 1;
      xyo->xArrayNeedUpdate[i] = 1;
      xyo->yArrayNeedUpdate[i] = 1;
      xyo->actWin->addDefExeNode( xyo->aglPtr );

    }

    break;

  }

  xyo->actWin->appCtx->proc->unlock();

}

static void axygc_edit_ok_trace (
  Widget w,
  XtPointer client,
  XtPointer call )
{

xyGraphClass *axygo = (xyGraphClass *) client;

  axygo->efTrace->popdownNoDestroy();

}

//-------------------------------------------------------------------------

static void axygc_edit_ok_axis (
  Widget w,
  XtPointer client,
  XtPointer call )
{

xyGraphClass *axygo = (xyGraphClass *) client;

  axygo->efAxis->popdownNoDestroy();

}

//-------------------------------------------------------------------------

static void axygc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call )
{

xyGraphClass *axygo = (xyGraphClass *) client;
int i;

  axygo->actWin->setChanged();

  axygo->eraseSelectBoxCorners();
  axygo->erase();

  axygo->x = axygo->eBuf->bufX;
  axygo->sboxX = axygo->eBuf->bufX;

  axygo->y = axygo->eBuf->bufY;
  axygo->sboxY = axygo->eBuf->bufY;

  axygo->w = axygo->eBuf->bufW;
  axygo->sboxW = axygo->eBuf->bufW;

  axygo->h = axygo->eBuf->bufH;
  axygo->sboxH = axygo->eBuf->bufH;

  axygo->graphTitle.setRaw( axygo->eBuf->bufGraphTitle );

  axygo->xLabel.setRaw( axygo->eBuf->bufXLabel );

  axygo->yLabel.setRaw( axygo->eBuf->bufYLabel );

  axygo->fgColor = axygo->eBuf->bufFgColor;

  axygo->bgColor = axygo->eBuf->bufBgColor;

  axygo->gridColor = axygo->eBuf->bufGridColor;

  axygo->plotMode = axygo->eBuf->bufPlotMode;

  axygo->count = axygo->eBuf->bufCount;

  axygo->updateTimerValue = axygo->eBuf->bufUpdateTimerValue;
  if ( axygo->updateTimerValue < 0 ) axygo->updateTimerValue = 0;

  axygo->numTraces = 0;
  for ( i=0; i<XYGC_K_MAX_TRACES; i++ ) {

    axygo->plotStyle[i] = axygo->eBuf->bufPlotStyle[i];

    axygo->plotSymbolType[i] = axygo->eBuf->bufPlotSymbolType[i];

    axygo->plotUpdateMode[i] = axygo->eBuf->bufPlotUpdateMode[i];

    axygo->plotColor[i] = axygo->eBuf->bufPlotColor[i];

    axygo->lineThk[i] = axygo->eBuf->bufLineThk[i]+1;

    axygo->opMode[i] = axygo->eBuf->bufOpMode[i];

    axygo->y2Scale[i] = axygo->eBuf->bufY2Scale[i];

    if ( axygo->eBuf->bufLineStyle[i] == 0 ) {
      axygo->lineStyle[i] = LineSolid;
    }
    else {
      axygo->lineStyle[i] = LineOnOffDash;
    }

    if ( ( !blank( axygo->eBuf->bufXPvName[i] ) ) &&
         ( !blank( axygo->eBuf->bufYPvName[i] ) ) ) {

      (axygo->numTraces)++;
      axygo->xPvExpStr[i].setRaw( axygo->eBuf->bufXPvName[i] );
      axygo->yPvExpStr[i].setRaw( axygo->eBuf->bufYPvName[i] );
      axygo->traceType[i] = XYGC_K_TRACE_XY;

    }
    else if ( !blank( axygo->eBuf->bufYPvName[i] ) ) {

      (axygo->numTraces)++;
      axygo->xPvExpStr[i].setRaw( "" );
      axygo->yPvExpStr[i].setRaw( axygo->eBuf->bufYPvName[i] );
      axygo->traceType[i] = XYGC_K_TRACE_CHRONOLOGICAL;

    }
    else {

      axygo->xPvExpStr[i].setRaw( "" );
      axygo->yPvExpStr[i].setRaw( "" );
      axygo->traceType[i] = XYGC_K_TRACE_INVALID;

    }

  }

  axygo->xAxis = axygo->eBuf->bufXAxis;
  axygo->xAxisStyle = axygo->eBuf->bufXAxisStyle;
  axygo->xAxisSource = axygo->eBuf->bufXAxisSource;
  axygo->xMin = axygo->eBuf->bufXMin;
  axygo->xMax = axygo->eBuf->bufXMax;
  axygo->xAxisTimeFormat = axygo->eBuf->bufXAxisTimeFormat;

  axygo->y1Axis = axygo->eBuf->bufY1Axis;
  axygo->y1AxisStyle = axygo->eBuf->bufY1AxisStyle;
  axygo->y1AxisSource = axygo->eBuf->bufY1AxisSource;
  axygo->y1Min = axygo->eBuf->bufY1Min;
  axygo->y1Max = axygo->eBuf->bufY1Max;

  axygo->y2Axis = axygo->eBuf->bufY2Axis;
  axygo->y2AxisStyle = axygo->eBuf->bufY2AxisStyle;
  axygo->y2AxisSource = axygo->eBuf->bufY2AxisSource;
  axygo->y2Min = axygo->eBuf->bufY2Min;
  axygo->y2Max = axygo->eBuf->bufY2Max;

  axygo->border = axygo->eBuf->bufBorder;

  axygo->trigPvExpStr.setRaw( axygo->eBuf->bufTrigPvName );
  axygo->resetPvExpStr.setRaw( axygo->eBuf->bufResetPvName );
  axygo->resetMode = axygo->eBuf->bufResetMode;

  strncpy( axygo->fontTag, axygo->fm.currentFontTag(), 63 );
  axygo->actWin->fi->loadFontTag( axygo->fontTag );
  axygo->actWin->drawGc.setFontTag( axygo->fontTag, axygo->actWin->fi );

  axygo->xNumLabelIntervals = axygo->eBuf->bufXNumLabelIntervals;
  axygo->xLabelGrid = axygo->eBuf->bufXLabelGrid;
  axygo->xNumMajorPerLabel = axygo->eBuf->bufXNumMajorPerLabel;
  axygo->xMajorGrid = axygo->eBuf->bufXMajorGrid;
  axygo->xNumMinorPerMajor = axygo->eBuf->bufXNumMinorPerMajor;
  axygo->xMinorGrid = axygo->eBuf->bufXMinorGrid;
  axygo->xAnnotationFormat = axygo->eBuf->bufXAnnotationFormat;
  axygo->xAnnotationPrecision = axygo->eBuf->bufXAnnotationPrecision;

  axygo->y1NumLabelIntervals = axygo->eBuf->bufY1NumLabelIntervals;
  axygo->y1LabelGrid = axygo->eBuf->bufY1LabelGrid;
  axygo->y1NumMajorPerLabel = axygo->eBuf->bufY1NumMajorPerLabel;
  axygo->y1MajorGrid = axygo->eBuf->bufY1MajorGrid;
  axygo->y1NumMinorPerMajor = axygo->eBuf->bufY1NumMinorPerMajor;
  axygo->y1MinorGrid = axygo->eBuf->bufY1MinorGrid;
  axygo->y1AnnotationFormat = axygo->eBuf->bufY1AnnotationFormat;
  axygo->y1AnnotationPrecision = axygo->eBuf->bufY1AnnotationPrecision;

  axygo->y2NumLabelIntervals = axygo->eBuf->bufY2NumLabelIntervals;
  axygo->y2LabelGrid = axygo->eBuf->bufY2LabelGrid;
  axygo->y2NumMajorPerLabel = axygo->eBuf->bufY2NumMajorPerLabel;
  axygo->y2MajorGrid = axygo->eBuf->bufY2MajorGrid;
  axygo->y2NumMinorPerMajor = axygo->eBuf->bufY2NumMinorPerMajor;
  axygo->y2MinorGrid = axygo->eBuf->bufY2MinorGrid;
  axygo->y2AnnotationFormat = axygo->eBuf->bufY2AnnotationFormat;
  axygo->y2AnnotationPrecision = axygo->eBuf->bufY2AnnotationPrecision;

  // check for conflicts

  for ( i=0; i<axygo->numTraces; i++ ) {

#if 0
    if ( ( axygo->traceType[i] == XYGC_K_TRACE_XY ) &&
         ( ( axygo->xAxisStyle == XYGC_K_AXIS_STYLE_TIME ) ||
           ( axygo->xAxisStyle == XYGC_K_AXIS_STYLE_STRIPCHART ) ) ) {
      axygo->traceType[i] = XYGC_K_TRACE_INVALID;
    }
#endif

    if ( axygo->xAxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) {
      if ( axygo->xMin.value() <= 0.0 ) axygo->xMin.setValue( 1.0 );
    }
    else if ( axygo->xAxisStyle == XYGC_K_AXIS_STYLE_TIME_LOG10 ) {
      if ( axygo->xMin.value() <= 0.0 ) axygo->xMin.setValue( 1.0 );
    }

    if ( axygo->xMin.value() >= axygo->xMax.value() ) {
      axygo->xMax.setValue( axygo->xMin.value() * 2.0 );
    }

    if ( axygo->xMin.value() >= axygo->xMax.value() ) { // in case xMin is 0
      axygo->xMax.setValue( axygo->xMin.value() + 1.0 );
    }

    if ( axygo->y1AxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) {
      if ( axygo->y1Min.value() <= 0.0 ) axygo->y1Min.setValue( 1.0 );
    }

    if ( axygo->y1Min.value() >= axygo->y1Max.value() ) {
      axygo->y1Max.setValue( axygo->y1Min.value() * 2.0 );
    }

    if ( axygo->y1Min.value() >= axygo->y1Max.value() ) { // in case y1Min is 0
      axygo->y1Max.setValue( axygo->y1Min.value() + 1.0 );
    }

    if ( axygo->y2AxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) {
      if ( axygo->y2Min.value() <= 0.0 ) axygo->y2Min.setValue( 1.0 );
    }

    if ( axygo->y2Min.value() >= axygo->y2Max.value() ) {
      axygo->y2Max.setValue( axygo->y2Min.value() * 2.0 );
    }

    if ( axygo->y2Min.value() >= axygo->y2Max.value() ) { // in case y2Min is 0
      axygo->y2Max.setValue( axygo->y2Min.value() + 1.0 );
    }

  }

  axygo->updateDimensions();

}

static void axygc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call )
{

xyGraphClass *axygo = (xyGraphClass *) client;

  axygc_edit_update( w, client, call );
  axygo->refresh( axygo );

}

static void axygc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call )
{

xyGraphClass *axygo = (xyGraphClass *) client;

  axygc_edit_update( w, client, call );
  axygo->ef.popdown();
  axygo->operationComplete();

}

static void axygc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call )
{

xyGraphClass *axygo = (xyGraphClass *) client;

  axygo->ef.popdown();
  axygo->operationCancel();

}

static void axygc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call )
{

xyGraphClass *axygo = (xyGraphClass *) client;

  axygo->ef.popdown();
  axygo->operationCancel();
  axygo->erase();
  axygo->deleteRequest = 1;
  axygo->drawAll();

}

//-------------------------------------------------------------------------

xyGraphClass::xyGraphClass ( void ) {

int i;

  name = new char[strlen("xyGraphClass")+1];
  strcpy( name, "xyGraphClass" );

  plotMode = XYGC_K_PLOT_MODE_PLOT_N_STOP;
  count = 2;

  numTraces = 0;

  for ( i=0; i<XYGC_K_MAX_TRACES; i++ ) {
    xPv[i] = NULL;
    yPv[i] = NULL;
    xEv[i] = NULL;
    yEv[i] = NULL;
    xPvData[i] = NULL;
    yPvData[i] = NULL;
    plotBuf[i] = NULL;
    plotBufSize[i] = 0;
    plotInfo[i] = NULL;
    plotInfoSize[i] = 0;
  }
  trigPv = NULL;
  resetPv = NULL;
  resetEv = NULL;
  trigEv = NULL;

  pixmap = (Pixmap) NULL;

  resetMode = 0;
  xAxis = 1;
  xAxisStyle = XYGC_K_AXIS_STYLE_LINEAR;
  xAxisSource = XYGC_K_AUTOSCALE;
  xAxisTimeFormat = 0;

  y1Axis = 1;
  y1AxisStyle = XYGC_K_AXIS_STYLE_LINEAR;
  y1AxisSource = XYGC_K_AUTOSCALE;

  y2Axis = 1;
  y2AxisStyle = XYGC_K_AXIS_STYLE_LINEAR;
  y2AxisSource = XYGC_K_AUTOSCALE;

  xFormatType = 0;
  strcpy( xFormat, "f" );

  y1FormatType = 0;
  strcpy( y1Format, "f" );

  y2FormatType = 0;
  strcpy( y2Format, "f" );

  border = 1;

  activeMode = 0;

  connection.setMaxPvs( 2 * XYGC_K_MAX_TRACES + 2 );

  xNumLabelIntervals.setNull(1);
  xLabelGrid = 0;
  xNumMajorPerLabel.setNull(1);
  xMajorGrid = 0;
  xNumMinorPerMajor.setNull(1);
  xMinorGrid = 0;
  xAnnotationPrecision.setNull(1);
  xAnnotationFormat = 0;

  y1NumLabelIntervals.setNull(1);
  y1LabelGrid = 0;
  y1NumMajorPerLabel.setNull(1);
  y1MajorGrid = 0;
  y1NumMinorPerMajor.setNull(1);
  y1MinorGrid = 0;
  y1AnnotationPrecision.setNull(1);
  y1AnnotationFormat = 0;

  y2NumLabelIntervals.setNull(1);
  y2LabelGrid = 0;
  y2NumMajorPerLabel.setNull(1);
  y2MajorGrid = 0;
  y2NumMinorPerMajor.setNull(1);
  y2MinorGrid = 0;
  y2AnnotationPrecision.setNull(1);
  y2AnnotationFormat = 0;

  updateTimerValue = 0;

  eBuf = NULL;

  msgDialogPopedUp = 0;

}

// copy constructor
xyGraphClass::xyGraphClass
 ( const xyGraphClass *source ) {

activeGraphicClass *ago = (activeGraphicClass *) this;
int i;

  ago->clone( (activeGraphicClass *) source );

  name = new char[strlen("xyGraphClass")+1];
  strcpy( name, "xyGraphClass" );

  graphTitle.copy( source->graphTitle );
  xLabel.copy( source->xLabel );
  yLabel.copy( source->yLabel );

  fgCb = source->fgCb;
  bgCb = source->bgCb;
  gridCb = source->gridCb;

  fgColor = source->fgColor;
  bgColor = source->bgColor;
  gridColor = source->gridColor;

  plotMode = source->plotMode;
  count = source->count;
  border = source->border;

  numTraces = source->numTraces;

  for ( i=0; i<XYGC_K_MAX_TRACES; i++ ) {
    plotStyle[i] = source->plotStyle[i];
    plotSymbolType[i] = source->plotSymbolType[i];
    plotUpdateMode[i] = source->plotUpdateMode[i];
    plotColor[i] = source->plotColor[i];
    traceType[i] = source->traceType[i];
    lineThk[i] = source->lineThk[i];
    opMode[i] = source->opMode[i];
    y2Scale[i] = source->y2Scale[i];
    lineStyle[i] = source->lineStyle[i];
    xPvExpStr[i].copy( source->xPvExpStr[i] );
    yPvExpStr[i].copy( source->yPvExpStr[i] );
    xPv[i] = NULL;
    yPv[i] = NULL;
    xEv[i] = NULL;
    yEv[i] = NULL;
    xPvData[i] = NULL;
    yPvData[i] = NULL;
    plotBuf[i] = NULL;
    plotBufSize[i] = 0;
    plotInfo[i] = NULL;
    plotInfoSize[i] = 0;
  }

  trigPv = NULL;
  trigEv = NULL;
  trigPvExpStr.copy( source->trigPvExpStr );

  pixmap = (Pixmap) NULL;

  resetPv = NULL;
  resetEv = NULL;
  resetPvExpStr.copy( source->resetPvExpStr );
  resetMode = source->resetMode;

  xAxis = source->xAxis;
  xAxisStyle = source->xAxisStyle;
  xAxisSource = source->xAxisSource;
  xMin = source->xMin;
  xMax = source->xMax;
  xAxisTimeFormat = source->xAxisTimeFormat;

  y1Axis = source->y1Axis;
  y1AxisStyle = source->y1AxisStyle;
  y1AxisSource = source->y1AxisSource;
  y1Min = source->y1Min;
  y1Max = source->y1Max;

  y2Axis = source->y2Axis;
  y2AxisStyle = source->y2AxisStyle;
  y2AxisSource = source->y2AxisSource;
  y2Min = source->y2Min;
  y2Max = source->y2Max;

  xFormatType = source->xFormatType;
  strncpy( xFormat, source->xFormat, 15 );

  y1FormatType = source->y1FormatType;
  strncpy( y1Format, source->y1Format, 15 );

  y2FormatType = source->y2FormatType;
  strncpy( y2Format, source->y2Format, 15 );

  activeMode = 0;

  strncpy( fontTag, source->fontTag, 63 );
  fs = actWin->fi->getXFontStruct( fontTag );
  fontAscent = source->fontAscent;
  fontDescent = source->fontDescent;
  fontHeight = source->fontHeight;

  xNumLabelIntervals = source->xNumLabelIntervals;
  xLabelGrid = source->xLabelGrid;
  xNumMajorPerLabel = source->xNumMajorPerLabel;
  xMajorGrid = source->xMajorGrid;
  xNumMinorPerMajor = source->xNumMinorPerMajor;
  xMinorGrid = source->xMinorGrid;
  xAnnotationPrecision = source->xAnnotationPrecision;
  xAnnotationFormat = source->xAnnotationFormat;

  y1NumLabelIntervals = source->y1NumLabelIntervals;
  y1LabelGrid = source->y1LabelGrid;
  y1NumMajorPerLabel = source->y1NumMajorPerLabel;
  y1MajorGrid = source->y1MajorGrid;
  y1NumMinorPerMajor = source->y1NumMinorPerMajor;
  y1MinorGrid = source->y1MinorGrid;
  y1AnnotationPrecision = source->y1AnnotationPrecision;
  y1AnnotationFormat = source->y1AnnotationFormat;

  y2NumLabelIntervals = source->y2NumLabelIntervals;
  y2LabelGrid = source->y2LabelGrid;
  y2NumMajorPerLabel = source->y2NumMajorPerLabel;
  y2MajorGrid = source->y2MajorGrid;
  y2NumMinorPerMajor = source->y2NumMinorPerMajor;
  y2MinorGrid = source->y2MinorGrid;
  y2AnnotationPrecision = source->y2AnnotationPrecision;
  y2AnnotationFormat = source->y2AnnotationFormat;

  connection.setMaxPvs( 2 * XYGC_K_MAX_TRACES + 2 );

  updateTimerValue = source->updateTimerValue;

  eBuf = NULL;

  msgDialogPopedUp = 0;

  actWin->fi->loadFontTag( fontTag );
  actWin->drawGc.setFontTag( fontTag, actWin->fi );

  updateDimensions();

}

xyGraphClass::~xyGraphClass ( void ) {

  if ( name ) delete name;
  if ( eBuf ) delete eBuf;

}

void xyGraphClass::plotPvConnectStateCallback (
  ProcessVariable *pv,
  void *userarg
) {

objPlusIndexPtr ptr = (objPlusIndexPtr) userarg;
xyGraphClass *axygo = (xyGraphClass *) ptr->objPtr;

  if ( pv->is_valid() ) {

  }
  else { // lost connection

    axygo->connection.setPvDisconnected( (void *) ptr->index );

    axygo->actWin->appCtx->proc->lock();
    axygo->needRefresh = 1;
    axygo->actWin->addDefExeNode( axygo->aglPtr );
    axygo->actWin->appCtx->proc->unlock();

  }

}

void xyGraphClass::plotUpdate (
  ProcessVariable *pv,
  void *userarg
) {

objPlusIndexPtr ptr = (objPlusIndexPtr) userarg;
xyGraphClass *axygo = (xyGraphClass *) ptr->objPtr;

  if ( !axygo->connection.pvsConnected() ) {

    axygo->connection.setPvConnected( (void *) ptr->index );

    if ( axygo->connection.pvsConnected() ) {
      axygo->actWin->appCtx->proc->lock();
      axygo->needConnect = 1;
      axygo->actWin->addDefExeNode( axygo->aglPtr );
      axygo->actWin->appCtx->proc->unlock();
    }

  }
  else {

    axygo->actWin->appCtx->proc->lock();
    axygo->needUpdate = 1;
    axygo->actWin->addDefExeNode( axygo->aglPtr );
    axygo->actWin->appCtx->proc->unlock();

  }

}

int xyGraphClass::createInteractive (
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

int i;
char traceColor[15+1];

  fgColor = actWin->defaultTextFgColor;
  bgColor = actWin->defaultBgColor;
  gridColor = actWin->defaultTextFgColor;

  for ( i=0; i<XYGC_K_MAX_TRACES; i++ ) {
    sprintf( traceColor, "trace%-d", i );
    plotStyle[i] = XYGC_K_PLOT_STYLE_LINE;
    opMode[i] = XYGC_K_SCOPE_MODE;
    y2Scale[i] = 0;
    plotUpdateMode[i] = XYGC_K_UPDATE_ON_X_AND_Y;
    plotColor[i] = actWin->ci->colorIndexByAlias( traceColor );
    lineThk[i] = 1;
    lineStyle[i] = LineSolid;
  }

  strcpy( fontTag, actWin->defaultCtlFontTag );
  actWin->fi->loadFontTag( fontTag );

  updateDimensions();

  this->draw();

  this->editCreate();

  return 1;

}

int xyGraphClass::save (
  FILE *f )
{

int i, stat = 1;

  fprintf( f, "%-d %-d %-d\n", XYGC_MAJOR_VERSION, XYGC_MINOR_VERSION,
   XYGC_RELEASE );

  fprintf( f, "%-d\n", x );
  fprintf( f, "%-d\n", y );
  fprintf( f, "%-d\n", w );
  fprintf( f, "%-d\n", h );

  if ( graphTitle.getRaw() )
    writeStringToFile( f, graphTitle.getRaw() );
  else
    writeStringToFile( f, "" );

  if ( xLabel.getRaw() )
    writeStringToFile( f, xLabel.getRaw() );
  else
    writeStringToFile( f, "" );

  if ( yLabel.getRaw() )
    writeStringToFile( f, yLabel.getRaw() );
  else
    writeStringToFile( f, "" );

  actWin->ci->writeColorIndex( f, fgColor );

  actWin->ci->writeColorIndex( f, bgColor );

  fprintf( f, "%-d\n", plotMode );

  fprintf( f, "%-d\n", border );

  fprintf( f, "%-d\n", count );

  fprintf( f, "%-d\n", updateTimerValue );

  fprintf( f, "%-d\n", xAxis );
  fprintf( f, "%-d\n", xAxisStyle );
  fprintf( f, "%-d\n", xAxisSource );
  stat = xMin.write( f );
  stat = xMax.write( f );
  fprintf( f, "%-d\n", xAxisTimeFormat );

  fprintf( f, "%-d\n", y1Axis );
  fprintf( f, "%-d\n", y1AxisStyle );
  fprintf( f, "%-d\n", y1AxisSource );
  stat = y1Min.write( f );
  stat = y1Max.write( f );

  fprintf( f, "%-d\n", y2Axis );
  fprintf( f, "%-d\n", y2AxisStyle );
  fprintf( f, "%-d\n", y2AxisSource );
  stat = y2Min.write( f );
  stat = y2Max.write( f );

  if ( trigPvExpStr.getRaw() )
    writeStringToFile( f, trigPvExpStr.getRaw() );
  else
    writeStringToFile( f, "" );

  if ( resetPvExpStr.getRaw() )
    writeStringToFile( f, resetPvExpStr.getRaw() );
  else
    writeStringToFile( f, "" );

  fprintf( f, "%-d\n", resetMode );

  writeStringToFile( f, fontTag );

  xNumLabelIntervals.write( f );
  fprintf( f, "%-d\n", xLabelGrid );
  xNumMajorPerLabel.write( f );
  fprintf( f, "%-d\n", xMajorGrid );
  xNumMinorPerMajor.write( f );
  fprintf( f, "%-d\n", xMinorGrid );
  fprintf( f, "%-d\n", xAnnotationFormat );
  xAnnotationPrecision.write( f );

  y1NumLabelIntervals.write( f );
  fprintf( f, "%-d\n", y1LabelGrid );
  y1NumMajorPerLabel.write( f );
  fprintf( f, "%-d\n", y1MajorGrid );
  y1NumMinorPerMajor.write( f );
  fprintf( f, "%-d\n", y1MinorGrid );
  fprintf( f, "%-d\n", y1AnnotationFormat );
  y1AnnotationPrecision.write( f );

  y2NumLabelIntervals.write( f );
  fprintf( f, "%-d\n", y2LabelGrid );
  y2NumMajorPerLabel.write( f );
  fprintf( f, "%-d\n", y2MajorGrid );
  y2NumMinorPerMajor.write( f );
  fprintf( f, "%-d\n", y2MinorGrid );
  fprintf( f, "%-d\n", y2AnnotationFormat );
  y2AnnotationPrecision.write( f );

  actWin->ci->writeColorIndex( f, gridColor );

  fprintf( f, "%-d\n", numTraces );

  for ( i=0; i<numTraces; i++ ) {

    if ( xPvExpStr[i].getRaw() )
      writeStringToFile( f, xPvExpStr[i].getRaw() );
    else
      writeStringToFile( f, "" );

    if ( yPvExpStr[i].getRaw() )
      writeStringToFile( f, yPvExpStr[i].getRaw() );
    else
      writeStringToFile( f, "" );

    actWin->ci->writeColorIndex( f, plotColor[i] );

    fprintf( f, "%-d\n", plotStyle[i] );
    fprintf( f, "%-d\n", lineThk[i] );
    fprintf( f, "%-d\n", lineStyle[i] );
    fprintf( f, "%-d\n", plotUpdateMode[i] );
    fprintf( f, "%-d\n", plotSymbolType[i] );
    fprintf( f, "%-d\n", opMode[i] );
    fprintf( f, "%-d\n", y2Scale[i] );

  }

  return stat;

}

int xyGraphClass::createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int i;
int major, minor, release;
int stat = 1;
char str[127+1], traceColor[15+1], onePv[activeGraphicClass::MAX_PV_NAME+1];

  this->actWin = _actWin;

  fscanf( f, "%d %d %d\n", &major, &minor, &release ); actWin->incLine();

  fscanf( f, "%d\n", &x ); actWin->incLine();
  fscanf( f, "%d\n", &y ); actWin->incLine();
  fscanf( f, "%d\n", &w ); actWin->incLine();
  fscanf( f, "%d\n", &h ); actWin->incLine();

  this->initSelectBox();

  readStringFromFile( str, 127+1, f ); actWin->incLine();
  graphTitle.setRaw( str );

  readStringFromFile( str, 127+1, f ); actWin->incLine();
  xLabel.setRaw( str );

  readStringFromFile( str, 127+1, f ); actWin->incLine();
  yLabel.setRaw( str );

  actWin->ci->readColorIndex( f, &fgColor );
  actWin->incLine(); actWin->incLine();

  actWin->ci->readColorIndex( f, &bgColor );
  actWin->incLine(); actWin->incLine();

  fscanf( f, "%d\n", &plotMode ); actWin->incLine();

  fscanf( f, "%d\n", &border ); actWin->incLine();

  fscanf( f, "%d\n", &count ); actWin->incLine();

  fscanf( f, "%d\n", &updateTimerValue ); actWin->incLine();

  fscanf( f, "%d\n", &xAxis ); actWin->incLine();
  fscanf( f, "%d\n", &xAxisStyle ); actWin->incLine();
  fscanf( f, "%d\n", &xAxisSource ); actWin->incLine();
  stat = xMin.read( f ); actWin->incLine();
  stat = xMax.read( f ); actWin->incLine();
  fscanf( f, "%d\n", &xAxisTimeFormat ); actWin->incLine();

  fscanf( f, "%d\n", &y1Axis ); actWin->incLine();
  fscanf( f, "%d\n", &y1AxisStyle ); actWin->incLine();
  fscanf( f, "%d\n", &y1AxisSource ); actWin->incLine();
  stat = y1Min.read( f ); actWin->incLine();
  stat = y1Max.read( f ); actWin->incLine();

  fscanf( f, "%d\n", &y2Axis ); actWin->incLine();
  fscanf( f, "%d\n", &y2AxisStyle ); actWin->incLine();
  fscanf( f, "%d\n", &y2AxisSource ); actWin->incLine();
  stat = y2Min.read( f ); actWin->incLine();
  stat = y2Max.read( f ); actWin->incLine();

  readStringFromFile( onePv, activeGraphicClass::MAX_PV_NAME+1, f );
   actWin->incLine();
  trigPvExpStr.setRaw( onePv );

  readStringFromFile( onePv, activeGraphicClass::MAX_PV_NAME+1, f );
   actWin->incLine();
  resetPvExpStr.setRaw( onePv );

  fscanf( f, "%d\n", &resetMode ); actWin->incLine();

  readStringFromFile( fontTag, 63+1, f ); actWin->incLine();

  xNumLabelIntervals.read( f ); actWin->incLine();
  fscanf( f, "%d\n", &xLabelGrid ); actWin->incLine();
  xNumMajorPerLabel.read( f ); actWin->incLine();
  fscanf( f, "%d\n", &xMajorGrid ); actWin->incLine();
  xNumMinorPerMajor.read( f ); actWin->incLine();
  fscanf( f, "%d\n", &xMinorGrid ); actWin->incLine();
  fscanf( f, "%d\n", &xAnnotationFormat ); actWin->incLine();
  xAnnotationPrecision.read( f ); actWin->incLine();

  y1NumLabelIntervals.read( f ); actWin->incLine();
  fscanf( f, "%d\n", &y1LabelGrid ); actWin->incLine();
  y1NumMajorPerLabel.read( f ); actWin->incLine();
  fscanf( f, "%d\n", &y1MajorGrid ); actWin->incLine();
  y1NumMinorPerMajor.read( f ); actWin->incLine();
  fscanf( f, "%d\n", &y1MinorGrid ); actWin->incLine();
  fscanf( f, "%d\n", &y1AnnotationFormat ); actWin->incLine();
  y1AnnotationPrecision.read( f ); actWin->incLine();

  y2NumLabelIntervals.read( f ); actWin->incLine();
  fscanf( f, "%d\n", &y2LabelGrid ); actWin->incLine();
  y2NumMajorPerLabel.read( f ); actWin->incLine();
  fscanf( f, "%d\n", &y2MajorGrid ); actWin->incLine();
  y2NumMinorPerMajor.read( f ); actWin->incLine();
  fscanf( f, "%d\n", &y2MinorGrid ); actWin->incLine();
  fscanf( f, "%d\n", &y2AnnotationFormat ); actWin->incLine();
  y2AnnotationPrecision.read( f ); actWin->incLine();

  actWin->ci->readColorIndex( f, &gridColor );
  actWin->incLine(); actWin->incLine();

  fscanf( f, "%d\n", &numTraces ); actWin->incLine();

  for ( i=0; i<numTraces; i++ ) {

    readStringFromFile( onePv, activeGraphicClass::MAX_PV_NAME+1, f );
     actWin->incLine();
    xPvExpStr[i].setRaw( onePv );

    readStringFromFile( onePv, activeGraphicClass::MAX_PV_NAME+1, f );
     actWin->incLine();
    yPvExpStr[i].setRaw( onePv );

    if ( ( !blank( xPvExpStr[i].getRaw() ) ) &&
         ( !blank( yPvExpStr[i].getRaw() ) ) ) {
      traceType[i] = XYGC_K_TRACE_XY;
    }
    else if ( !blank( yPvExpStr[i].getRaw() ) ) {
      traceType[i] = XYGC_K_TRACE_CHRONOLOGICAL;
    }
    else {
      traceType[i] = XYGC_K_TRACE_INVALID;
    }

    actWin->ci->readColorIndex( f, &plotColor[i] );
    actWin->incLine(); actWin->incLine();
    fscanf( f, "%d\n", &plotStyle[i] ); actWin->incLine();
    fscanf( f, "%d\n", &lineThk[i] ); actWin->incLine();
    fscanf( f, "%d\n", &lineStyle[i] ); actWin->incLine();
    fscanf( f, "%d\n", &plotUpdateMode[i] ); actWin->incLine();
    fscanf( f, "%d\n", &plotSymbolType[i] ); actWin->incLine();
    fscanf( f, "%d\n", &opMode[i] ); actWin->incLine();
    fscanf( f, "%d\n", &y2Scale[i] ); actWin->incLine();

    if ( lineStyle[i] == 0 ) {
      lineStyle[i] = LineSolid;
    }
    else {
      lineStyle[i] = LineOnOffDash;
    }

  }

  for ( i=numTraces; i<XYGC_K_MAX_TRACES; i++ ) {
    sprintf( traceColor, "trace%-d", i );
    plotColor[i] = actWin->ci->colorIndexByAlias( traceColor );
    lineThk[i] = 1;
    lineStyle[i] = 0; // solid
    plotUpdateMode[i] = XYGC_K_UPDATE_ON_X_AND_Y;
    plotSymbolType[i] = XYGC_K_SYMBOL_TYPE_NONE;
    opMode[i] = XYGC_K_SCOPE_MODE;
    y2Scale[i] = 0;
  }

  actWin->fi->loadFontTag( fontTag );
  actWin->drawGc.setFontTag( fontTag, actWin->fi );

  // check for conflicts

  for ( i=0; i<numTraces; i++ ) {

#if 0
    if ( ( traceType[i] == XYGC_K_TRACE_XY ) &&
         ( ( xAxisStyle == XYGC_K_AXIS_STYLE_TIME ) ||
           ( xAxisStyle == XYGC_K_AXIS_STYLE_TIME_LOG10 ) ||
           ( axygo->xAxisStyle == XYGC_K_AXIS_STYLE_STRIPCHART ) ) ) {
      traceType[i] = XYGC_K_TRACE_INVALID;
    }
#endif

    if ( xAxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) {
      if ( xMin.value() <= 0.0 ) xMin.setValue( 1.0 );
    }
    else if ( xAxisStyle == XYGC_K_AXIS_STYLE_TIME_LOG10 ) {
      if ( xMin.value() <= 0.0 ) xMin.setValue( 1.0 );
    }

    if ( xMin.value() >= xMax.value() ) {
      xMax.setValue( xMin.value() * 2.0 );
    }

    if ( xMin.value() >= xMax.value() ) { // in case xMin is 0
      xMax.setValue( xMin.value() + 1.0 );
    }

    if ( y1AxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) {
      if ( y1Min.value() <= 0.0 ) y1Min.setValue( 1.0 );
    }

    if ( y1Min.value() >= y1Max.value() ) {
      y1Max.setValue( y1Min.value() * 2.0 );
    }

    if ( y1Min.value() >= y1Max.value() ) { // in case y1Min is 0
      y1Max.setValue( y1Min.value() + 1.0 );
    }

    if ( y2AxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) {
      if ( y2Min.value() <= 0.0 ) y2Min.setValue( 1.0 );
    }

    if ( y2Min.value() >= y2Max.value() ) {
      y2Max.setValue( y2Min.value() * 2.0 );
    }

    if ( y2Min.value() >= y2Max.value() ) { // in case y2Min is 0
      y2Max.setValue( y2Min.value() + 1.0 );
    }

  }

  updateDimensions();

  return stat;

}

int xyGraphClass::importFromXchFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

#if 0
int r, g, b, more;
int stat = 1;
char *tk, *gotData, *context, buf[255+1];

  r = 0xffff;
  g = 0xffff;
  b = 0xffff;
#endif

  this->actWin = _actWin;

  return 0; // not implemented

}

int xyGraphClass::genericEdit ( void ) {

char title[32], *ptr;
int i;

  ptr = actWin->obj.getNameFromClass( "xyGraphClass" );
  if ( ptr )
    strncpy( title, ptr, 31 );
  else
    strncpy( title, "Unknown object", 31 );

  strncat( title, " Properties", 31 );

  if ( !eBuf ) {
    eBuf = new editBufType;
  }

  eBuf->bufX = x;
  eBuf->bufY = y;
  eBuf->bufW = w;
  eBuf->bufH = h;

  eBuf->bufPlotStyle = plotStyle;

  eBuf->bufPlotMode = plotMode;

  eBuf->bufBorder = border;

  eBuf->bufUpdateTimerValue = updateTimerValue;

  eBuf->bufCount = count;

  strncpy( eBuf->bufGraphTitle, graphTitle.getRaw(), 127 );
  strncpy( eBuf->bufXLabel, xLabel.getRaw(), 127 );
  strncpy( eBuf->bufYLabel, yLabel.getRaw(), 127 );
  eBuf->bufFgColor = fgColor;
  eBuf->bufBgColor = bgColor;
  eBuf->bufGridColor = gridColor;
  strncpy( eBuf->bufTrigPvName, trigPvExpStr.getRaw(),
   activeGraphicClass::MAX_PV_NAME );
  strncpy( eBuf->bufResetPvName, resetPvExpStr.getRaw(),
   activeGraphicClass::MAX_PV_NAME );
  eBuf->bufResetMode = resetMode;

  eBuf->bufXNumLabelIntervals = xNumLabelIntervals;
  eBuf->bufXLabelGrid = xLabelGrid;
  eBuf->bufXNumMajorPerLabel = xNumMajorPerLabel;
  eBuf->bufXMajorGrid = xMajorGrid;
  eBuf->bufXNumMinorPerMajor = xNumMinorPerMajor;
  eBuf->bufXMinorGrid = xMinorGrid;
  eBuf->bufXAnnotationFormat = xAnnotationFormat;
  eBuf->bufXAnnotationPrecision = xAnnotationPrecision;

  eBuf->bufY1NumLabelIntervals = y1NumLabelIntervals;
  eBuf->bufY1LabelGrid = y1LabelGrid;
  eBuf->bufY1NumMajorPerLabel = y1NumMajorPerLabel;
  eBuf->bufY1MajorGrid = y1MajorGrid;
  eBuf->bufY1NumMinorPerMajor = y1NumMinorPerMajor;
  eBuf->bufY1MinorGrid = y1MinorGrid;
  eBuf->bufY1AnnotationFormat = y1AnnotationFormat;
  eBuf->bufY1AnnotationPrecision = y1AnnotationPrecision;

  eBuf->bufY2NumLabelIntervals = y2NumLabelIntervals;
  eBuf->bufY2LabelGrid = y2LabelGrid;
  eBuf->bufY2NumMajorPerLabel = y2NumMajorPerLabel;
  eBuf->bufY2MajorGrid = y2MajorGrid;
  eBuf->bufY2NumMinorPerMajor = y2NumMinorPerMajor;
  eBuf->bufY2MinorGrid = y2MinorGrid;
  eBuf->bufY2AnnotationFormat = y2AnnotationFormat;
  eBuf->bufY2AnnotationPrecision = y2AnnotationPrecision;

  ef.create( actWin->top, actWin->appCtx->ci.getColorMap(),
   &actWin->appCtx->entryFormX,
   &actWin->appCtx->entryFormY, &actWin->appCtx->entryFormW,
   &actWin->appCtx->entryFormH, &actWin->appCtx->largestH,
   title, NULL, NULL, NULL );

  ef.addTextField( "X", 35, &eBuf->bufX );
  ef.addTextField( "Y", 35, &eBuf->bufY );
  ef.addTextField( "Width", 35, &eBuf->bufW );
  ef.addTextField( "Height", 35, &eBuf->bufH );
  ef.addTextField( "Title", 35, eBuf->bufGraphTitle, 127 );
  ef.addTextField( "X Label", 35, eBuf->bufXLabel, 127 );
  ef.addTextField( "Y Label", 35, eBuf->bufYLabel, 127 );
  ef.addColorButton( "Foreground", actWin->ci, &fgCb, &eBuf->bufFgColor );
  ef.addColorButton( "Background", actWin->ci, &bgCb, &eBuf->bufBgColor );
  ef.addColorButton( "Grid", actWin->ci, &gridCb, &eBuf->bufGridColor );
  ef.addOption( "Plot Mode", "plot n pts & stop|plot last n pts", &eBuf->bufPlotMode );
  ef.addTextField( "Count", 35, &eBuf->bufCount );
  ef.addTextField( "Update Delay (ms)", 35, &eBuf->bufUpdateTimerValue );
  ef.addToggle( "Border", &eBuf->bufBorder );

  ef.addEmbeddedEf( "X/Y/Trace Data", "... ", &efTrace );

    efTrace->create( actWin->top, actWin->appCtx->ci.getColorMap(),
     &actWin->appCtx->entryFormX,
     &actWin->appCtx->entryFormY, &actWin->appCtx->entryFormW,
     &actWin->appCtx->entryFormH, &actWin->appCtx->largestH,
     "Trace Properties", NULL, NULL, NULL );

    for ( i=0; i<numTraces; i++ ) {
      strncpy( eBuf->bufXPvName[i], xPvExpStr[i].getRaw(),
       activeGraphicClass::MAX_PV_NAME );
      strncpy( eBuf->bufYPvName[i], yPvExpStr[i].getRaw(),
       activeGraphicClass::MAX_PV_NAME );
      eBuf->bufPlotStyle[i] = plotStyle[i];
      eBuf->bufPlotSymbolType[i] = plotSymbolType[i];
      eBuf->bufPlotUpdateMode[i] = plotUpdateMode[i];
      eBuf->bufPlotColor[i] = plotColor[i];
      eBuf->bufLineThk[i] = lineThk[i]-1;
      if ( lineStyle[i] == LineSolid ) {
        eBuf->bufLineStyle[i] = 0;
      }
      else {
        eBuf->bufLineStyle[i] = 1;
      }
      eBuf->bufOpMode[i] = opMode[i];
      eBuf->bufY2Scale[i] = y2Scale[i];
    }
    for ( i=numTraces; i<XYGC_K_MAX_TRACES; i++ ) {
      strcpy( eBuf->bufXPvName[i], "" );
      strcpy( eBuf->bufYPvName[i], "" );
      eBuf->bufPlotStyle[i] = XYGC_K_PLOT_STYLE_LINE;
      eBuf->bufPlotSymbolType[i] = XYGC_K_SYMBOL_TYPE_NONE;
      eBuf->bufPlotUpdateMode[i] = XYGC_K_UPDATE_ON_X_AND_Y;
      eBuf->bufPlotColor[i] = plotColor[i];
      eBuf->bufLineThk[i] = 0;
      eBuf->bufLineStyle[i] = 0;
      eBuf->bufOpMode[i] = XYGC_K_SCOPE_MODE;
      eBuf->bufY2Scale[i] = 0;
    }

    i = 0;
    efTrace->beginSubForm();
    efTrace->addTextField( "X ", 20, eBuf->bufXPvName[i],
     activeGraphicClass::MAX_PV_NAME );
    efTrace->addLabel( "  Y " );
    efTrace->addTextField( "", 20, eBuf->bufYPvName[i],
     activeGraphicClass::MAX_PV_NAME );
    efTrace->addOption( "", "scope|plot", &eBuf->bufOpMode[i] );
    efTrace->addLabel( "  Y2" );
    efTrace->addToggle( "", &eBuf->bufY2Scale[i] );
    efTrace->addLabel( "  Style" );
    efTrace->addOption( "", "line|point|needle", &eBuf->bufPlotStyle[i] );
    efTrace->addLabel( "  Update" );
    efTrace->addOption( "", "X and Y|X or Y|X|Y",
     &eBuf->bufPlotUpdateMode[i] );
    efTrace->addLabel( "  Thk" );
    efTrace->addOption( "", "1|2|3|4|5|6|7|8|9", &eBuf->bufLineThk[i] );
    efTrace->addLabel( "  Line" );
    efTrace->addOption( "", "solid|dash", &eBuf->bufLineStyle[i] );
    efTrace->addLabel( "  Symbol" );
    efTrace->addOption( "", "none|circle|square|diamond",
     &eBuf->bufPlotSymbolType[i] );
    efTrace->addLabel( " " );
    efTrace->addColorButton( "", actWin->ci, &plotCb[i],
     &eBuf->bufPlotColor[i] );
    efTrace->endSubForm();

    for ( i=1; i<XYGC_K_MAX_TRACES; i++ ) {

      efTrace->beginLeftSubForm();
      efTrace->addTextField( "X ", 20, eBuf->bufXPvName[i],
       activeGraphicClass::MAX_PV_NAME );
      efTrace->addLabel( "  Y " );
      efTrace->addTextField( "", 20, eBuf->bufYPvName[i],
       activeGraphicClass::MAX_PV_NAME );
      efTrace->addOption( "", "scope|plot", &eBuf->bufOpMode[i] );
      efTrace->addLabel( "  Y2" );
      efTrace->addToggle( "", &eBuf->bufY2Scale[i] );
      efTrace->addLabel( "  Style" );
      efTrace->addOption( "", "line|point|needle", &eBuf->bufPlotStyle[i] );
      efTrace->addLabel( "  Update" );
      efTrace->addOption( "", "X and Y|X or Y|X|Y",
       &eBuf->bufPlotUpdateMode[i] );
      efTrace->addLabel( "  Thk" );
      efTrace->addOption( "", "1|2|3|4|5|6|7|8|9", &eBuf->bufLineThk[i] );
      efTrace->addLabel( "  Line" );
      efTrace->addOption( "", "solid|dash", &eBuf->bufLineStyle[i] );
      efTrace->addLabel( "  Symbol" );
      efTrace->addOption( "", "none|circle|square|diamond",
     &eBuf->bufPlotSymbolType[i] );
      efTrace->addLabel( " " );
      efTrace->addColorButton( "", actWin->ci, &plotCb[i],
       &eBuf->bufPlotColor[i] );
      efTrace->endSubForm();

    }

    efTrace->finished( axygc_edit_ok_trace, this );

  eBuf->bufXAxis = xAxis;
  eBuf->bufXAxisStyle = xAxisStyle;
  eBuf->bufXAxisSource = xAxisSource;
  eBuf->bufXMin = xMin;
  eBuf->bufXMax = xMax;
  eBuf->bufXAxisTimeFormat = xAxisTimeFormat;

  eBuf->bufY1Axis = y1Axis;
  eBuf->bufY1AxisStyle = y1AxisStyle;
  eBuf->bufY1AxisSource = y1AxisSource;
  eBuf->bufY1Min = y1Min;
  eBuf->bufY1Max = y1Max;

  eBuf->bufY2Axis = y2Axis;
  eBuf->bufY2AxisStyle = y2AxisStyle;
  eBuf->bufY2AxisSource = y2AxisSource;
  eBuf->bufY2Min = y2Min;
  eBuf->bufY2Max = y2Max;

  ef.addEmbeddedEf( "Axis Data", "... ", &efAxis );

    efAxis->create( actWin->top, actWin->appCtx->ci.getColorMap(),
     &actWin->appCtx->entryFormX,
     &actWin->appCtx->entryFormY, &actWin->appCtx->entryFormW,
     &actWin->appCtx->entryFormH, &actWin->appCtx->largestH,
     "Axis Properties", NULL, NULL, NULL );

    efAxis->beginSubForm();
    efAxis->addLabel( "X   " );
    efAxis->addLabel( " Show" );
    efAxis->addToggle( "", &eBuf->bufXAxis );
    efAxis->addLabel( " Style" );
    efAxis->addOption( "", "linear|log10|time|log10(time)|stripchart", &eBuf->bufXAxisStyle );
    efAxis->addLabel( " Range" );
    efAxis->addOption( "", "from pv|user-specified|auto-scale",
     &eBuf->bufXAxisSource );
    efAxis->addLabel( " Minimum " );
    efAxis->addTextField( "", 10, &eBuf->bufXMin );
    efAxis->addLabel( " Maximum " );
    efAxis->addTextField( "", 10, &eBuf->bufXMax );
    efAxis->addLabel( " Time Format" );
    efAxis->addOption( "",
     "hh:mm:ss|hh:mm|hh:00|MMM DD YYYY|MMM DD|MMM DD hh:00|wd hh:00",
     &eBuf->bufXAxisTimeFormat );
    efAxis->endSubForm();

    efAxis->beginLeftSubForm();
    efAxis->addLabel( "    " );
    efAxis->addLabel( " Label Tick Intervals " );
    efAxis->addTextField( "", 3, &eBuf->bufXNumLabelIntervals );
    efAxis->addLabel( " Grid" );
    efAxis->addToggle( "", &eBuf->bufXLabelGrid );
    efAxis->addLabel( " Majors/Label " );
    efAxis->addTextField( "", 3, &eBuf->bufXNumMajorPerLabel );
    efAxis->addLabel( " Grid" );
    efAxis->addToggle( "", &eBuf->bufXMajorGrid );
    efAxis->addLabel( " Minors/Major " );
    efAxis->addTextField( "", 3, &eBuf->bufXNumMinorPerMajor );
    efAxis->addLabel( " Grid" );
    efAxis->addToggle( "", &eBuf->bufXMinorGrid );
    efAxis->addLabel( " Format" );
    efAxis->addOption( "", "f|g", &eBuf->bufXAnnotationFormat );
    efAxis->addLabel( " Precision " );
    efAxis->addTextField( "", 3, &eBuf->bufXAnnotationPrecision );
    efAxis->endSubForm();
   
    efAxis->addSeparator();

    efAxis->beginLeftSubForm();
    efAxis->addLabel( "Y1  " );
    efAxis->addLabel( " Show" );
    efAxis->addToggle( "", &eBuf->bufY1Axis );
    efAxis->addLabel( " Style" );
    efAxis->addOption( "", "linear|log10", &eBuf->bufY1AxisStyle );
    efAxis->addLabel( " Range" );
    efAxis->addOption( "", "from pv|user-specified|auto-scale",
     &eBuf->bufY1AxisSource );
    efAxis->addLabel( " Minimum " );
    efAxis->addTextField( "", 10, &eBuf->bufY1Min );
    efAxis->addLabel( " Maximum " );
    efAxis->addTextField( "", 10, &eBuf->bufY1Max );
    efAxis->endSubForm();

    efAxis->beginLeftSubForm();
    efAxis->addLabel( "    " );
    efAxis->addLabel( " Label Tick Intervals " );
    efAxis->addTextField( "", 3, &eBuf->bufY1NumLabelIntervals );
    efAxis->addLabel( " Grid" );
    efAxis->addToggle( "", &eBuf->bufY1LabelGrid );
    efAxis->addLabel( " Majors/Label " );
    efAxis->addTextField( "", 3, &eBuf->bufY1NumMajorPerLabel );
    efAxis->addLabel( " Grid" );
    efAxis->addToggle( "", &eBuf->bufY1MajorGrid );
    efAxis->addLabel( " Minors/Major " );
    efAxis->addTextField( "", 3, &eBuf->bufY1NumMinorPerMajor );
    efAxis->addLabel( " Grid" );
    efAxis->addToggle( "", &eBuf->bufY1MinorGrid );
    efAxis->addLabel( " Format" );
    efAxis->addOption( "", "f|g", &eBuf->bufY1AnnotationFormat );
    efAxis->addLabel( " Precision " );
    efAxis->addTextField( "", 3, &eBuf->bufY1AnnotationPrecision );
    efAxis->endSubForm();
   
    efAxis->addSeparator();

    efAxis->beginLeftSubForm();
    efAxis->addLabel( "Y2  " );
    efAxis->addLabel( " Show" );
    efAxis->addToggle( "", &eBuf->bufY2Axis );
    efAxis->addLabel( " Style" );
    efAxis->addOption( "", "linear|log10",
     &eBuf->bufY2AxisStyle );
    efAxis->addLabel( " Range" );
    efAxis->addOption( "", "from pv|user-specified|auto-scale",
     &eBuf->bufY2AxisSource );
    efAxis->addLabel( " Minimum " );
    efAxis->addTextField( "", 10, &eBuf->bufY2Min );
    efAxis->addLabel( " Maximum " );
    efAxis->addTextField( "", 10, &eBuf->bufY2Max );
    efAxis->endSubForm();

    efAxis->beginLeftSubForm();
    efAxis->addLabel( "    " );
    efAxis->addLabel( " Label Tick Intervals " );
    efAxis->addTextField( "", 3, &eBuf->bufY2NumLabelIntervals );
    efAxis->addLabel( " Grid" );
    efAxis->addToggle( "", &eBuf->bufY2LabelGrid );
    efAxis->addLabel( " Majors/Label " );
    efAxis->addTextField( "", 3, &eBuf->bufY2NumMajorPerLabel );
    efAxis->addLabel( " Grid" );
    efAxis->addToggle( "", &eBuf->bufY2MajorGrid );
    efAxis->addLabel( " Minors/Major " );
    efAxis->addTextField( "", 3, &eBuf->bufY2NumMinorPerMajor );
    efAxis->addLabel( " Grid" );
    efAxis->addToggle( "", &eBuf->bufY2MinorGrid );
    efAxis->addLabel( " Format" );
    efAxis->addOption( "", "f|g", &eBuf->bufY2AnnotationFormat );
    efAxis->addLabel( " Precision " );
    efAxis->addTextField( "", 3, &eBuf->bufY2AnnotationPrecision );
    efAxis->endSubForm();
   
    efAxis->finished( axygc_edit_ok_axis, this );

  ef.addTextField( "Trigger PV", 35, eBuf->bufTrigPvName,
   activeGraphicClass::MAX_PV_NAME );
  ef.addTextField( "Reset PV", 35, eBuf->bufResetPvName,
   activeGraphicClass::MAX_PV_NAME );
  ef.addOption( "Reset Mode", "if not zero|if zero", &eBuf->bufResetMode );
  ef.addFontMenuNoAlignInfo( "Font", actWin->fi, &fm, fontTag );

  return 1;

}

int xyGraphClass::editCreate ( void ) {

  this->genericEdit();
  ef.finished( axygc_edit_ok, axygc_edit_apply, axygc_edit_cancel_delete, this );
  actWin->currentEf = NULL;
  ef.popup();

  return 1;

}

int xyGraphClass::edit ( void ) {

  this->genericEdit();
  ef.finished( axygc_edit_ok, axygc_edit_apply, axygc_edit_cancel, this );
  actWin->currentEf = &ef;
  ef.popup();

  return 1;

}

void xyGraphClass::regenBuffer ( void ) {

int i, ii;
double dxValue, dyValue;
short scaledX, scaledY;

  //printf( "xyGraphClass::regenBuffer\n" );

  for ( i=0; i<numTraces; i++ ) {

    initPlotInfo( i );
    arrayNumPoints[i] = curNpts[i] = 0;

    ii = arrayHead[i];
    while ( ii != arrayTail[i] ) {

      switch ( yPvType[i] ) {
      case DBR_FLOAT:
        dyValue = (double) ( (float *) yPvData[i] )[ii];
        break;
      case DBR_DOUBLE: 
        dyValue = ( (double *) yPvData[i] )[ii];
        break;
      case DBR_SHORT:
        dyValue = (double) ( (unsigned short *) yPvData[i] )[ii];
        break;
      case DBR_CHAR:
        dyValue = (double) ( (unsigned char *) yPvData[i] )[ii];
        break;
      case DBR_LONG:
        dyValue = (double) ( (int *) yPvData[i] )[ii];
        break;
      case DBR_ENUM:
        dyValue = (double) ( (unsigned short *) yPvData[i] )[ii];
        break;
      default:
        dyValue = ( (double *) yPvData[i] )[ii];
        break;
      }

      if ( y1AxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) {
        if ( dyValue > 0 ) dyValue = log10( dyValue );
      }

      scaledY = (short) plotAreaH -
       (short) rint( ( dyValue - curY1Min ) *
       y1Factor[i] - y1Offset[i] );
  
      switch ( xPvType[i] ) {
      case DBR_FLOAT:
        dxValue = (double) ( (float *) xPvData[i] )[ii];
        break;
      case DBR_DOUBLE: 
        dxValue = ( (double *) xPvData[i] )[ii];
        break;
      case DBR_SHORT:
        dxValue = (double) ( (unsigned short *) xPvData[i] )[ii];
        break;
      case DBR_CHAR:
        dxValue = (double) ( (unsigned char *) xPvData[i] )[ii];
        break;
      case DBR_LONG:
        dxValue = (double) ( (int *) xPvData[i] )[ii];
        break;
      case DBR_ENUM:
        dxValue = (double) ( (unsigned short *) xPvData[i] )[ii];
        break;
      default:
        dxValue = ( (double *) xPvData[i] )[ii];
        break;
      }

      if ( xAxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) {
        if ( dxValue > 0 ) dxValue = log10( dxValue );
      }
      else if ( xAxisStyle == XYGC_K_AXIS_STYLE_TIME_LOG10 ) {
        if ( dxValue > 0 ) dxValue = log10( dxValue );
      }

      scaledX = (short) rint( ( dxValue - curXMin ) *
       xFactor[i] + xOffset[i] );

      addPoint( dxValue, scaledX, scaledY, i );

      yArrayGotValue[i] = xArrayGotValue[i] = 0;

      ii++;
      if ( ii > plotBufSize[i] ) {
        ii = 0;
      }

    }

  }

}

void xyGraphClass::genChronoVector (
  int i, // trace
  int *rescale
) {

int ii, iii, needRescale;
double dxValue, dyValue;
short scaledX, scaledY;
char format[31+1];

  *rescale = needRescale = 0;

  initPlotInfo( i );
  arrayNumPoints[i] = 0;

  for ( ii=0; ii<yPvCount[i]; ii++ ) {

    switch ( yPvType[i] ) {
    case DBR_FLOAT:
      dyValue = (double) ( (float *) yPvData[i] )[ii];
      break;
    case DBR_DOUBLE: 
      dyValue = ( (double *) yPvData[i] )[ii];
      break;
    case DBR_SHORT:
      dyValue = (double) ( (unsigned short *) yPvData[i] )[ii];
      break;
    case DBR_CHAR:
      dyValue = (double) ( (unsigned char *) yPvData[i] )[ii];
      break;
    case DBR_LONG:
      dyValue = (double) ( (int *) yPvData[i] )[ii];
      break;
    case DBR_ENUM:
      dyValue = (double) ( (unsigned short *) yPvData[i] )[ii];
      break;
    default:
      dyValue = ( (double *) yPvData[i] )[ii];
      break;
    }

    if ( y1AxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) {
      if ( dyValue > 0 ) dyValue = log10( dyValue );
    }

    scaledY = (short) plotAreaH -
     (short) rint( ( dyValue - curY1Min ) *
     y1Factor[i] - y1Offset[i] );
  
    switch ( xPvType[i] ) {
    case DBR_FLOAT:
      dxValue = (double) ( (float *) xPvData[i] )[ii];
      break;
    case DBR_DOUBLE: 
      dxValue = ( (double *) xPvData[i] )[ii];
      break;
    case DBR_SHORT:
      dxValue = (double) ( (unsigned short *) xPvData[i] )[ii];
      break;
    case DBR_CHAR:
      dxValue = (double) ( (unsigned char *) xPvData[i] )[ii];
      break;
    case DBR_LONG:
      dxValue = (double) ( (int *) xPvData[i] )[ii];
      break;
    case DBR_ENUM:
      dxValue = (double) ( (unsigned short *) xPvData[i] )[ii];
      break;
    default:
      dxValue = ( (double *) xPvData[i] )[ii];
      break;
    }

    if ( xAxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) {
      if ( dxValue > 0 ) dxValue = log10( dxValue );
    }
    else if ( xAxisStyle == XYGC_K_AXIS_STYLE_TIME_LOG10 ) {
      if ( dxValue > 0 ) dxValue = log10( dxValue );
    }

    scaledX = (short) rint( ( dxValue - curXMin ) *
     xFactor[i] + xOffset[i] );

#if 0
    printf( "[%-d]\tdxValue = %-g\n", ii, dxValue );
    printf( "\tdyValue = %-g\n", dyValue );
    printf( "\tscaledX = %-d\n", scaledX );
    printf( "\tscaledY = %-d\n\n", scaledY );
    printf( "curY1Min = %-g\n", curY1Min );
    printf( "y1Factor[i] = %-g\n", y1Factor[i] );
    printf( "y1Offset[i] = %-g\n", y1Offset[i] );
    printf( "curXMin = %-g\n", curXMin );
    printf( "xFactor[i] = %-g\n", xFactor[i] );
    printf( "xOffset[i] = %-g\n\n", xOffset[i] );
#endif

    addPoint( dxValue, scaledX, scaledY, i );

    if ( xAxisSource == XYGC_K_AUTOSCALE ) {
      if ( kpXMinEfDouble.isNull() ) {
        if ( dxValue < curXMin ) {
          curXMin = dxValue;
          needRescale = 1;
        }
      }
      if ( kpXMaxEfDouble.isNull() ) {
        if ( dxValue > curXMax ) {
          curXMax = dxValue;
          needRescale = 1;
        }
      }
    }

    if ( y1AxisSource == XYGC_K_AUTOSCALE ) {
      if ( kpY1MinEfDouble.isNull() ) {
        if ( dyValue < curY1Min ) {
          needRescale = 1;
          curY1Min = dyValue;
        }
      }
      if ( kpY1MaxEfDouble.isNull() ) {
        if ( dyValue > curY1Max ) {
          needRescale = 1;
          curY1Max = dyValue;
          actWin->addDefExeNode( aglPtr );
        }
      }
    }

  }

  if ( needRescale ) {

    if ( xAxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) {
      get_log10_scale_params1( curXMin, curXMax, &curXMin, &curXMax,
       &curXNumLabelTicks, &curXMajorsPerLabel, &curXMinorsPerMajor, format );
    }
    else if ( xAxisStyle == XYGC_K_AXIS_STYLE_TIME_LOG10 ) {
      get_log10_scale_params1( curXMin, curXMax, &curXMin, &curXMax,
       &curXNumLabelTicks, &curXMajorsPerLabel, &curXMinorsPerMajor, format );
    }
    else {
      get_scale_params1( curXMin, curXMax, &curXMin, &curXMax,
       &curXNumLabelTicks, &curXMajorsPerLabel, &curXMinorsPerMajor, format );
    }

    if ( y1AxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) {
      get_log10_scale_params1( curY1Min, curY1Max, &curY1Min, &curY1Max,
       &curY1NumLabelTicks, &curY1MajorsPerLabel, &curY1MinorsPerMajor,
       format );
    }
    else {
      get_scale_params1( curY1Min, curY1Max, &curY1Min, &curY1Max,
       &curY1NumLabelTicks, &curY1MajorsPerLabel, &curY1MinorsPerMajor,
       format );
    }

    updateDimensions();

    for ( iii=0; iii<numTraces; iii++ ) {
      xFactor[iii] =
       (double) ( plotAreaW ) / ( curXMax - curXMin );
      xOffset[iii] = plotAreaX;
    }

    for ( iii=0; iii<numTraces; iii++ ) {
      y1Factor[iii] =
       (double) ( plotAreaH ) / ( curY1Max - curY1Min );
      y1Offset[iii] = plotAreaY;
    }

    *rescale = 1;

  }

  arrayHead[i] = 0;
  arrayTail[i] = yPvCount[i];
  yArrayGotValue[i] = xArrayGotValue[i] = 0;

}

void xyGraphClass::genXyVector (
  int i, // trace
  int *rescale
) {

int ii, iii, needRescale, n;
double dxValue, dyValue;
short scaledX, scaledY;
char format[31+1];

  *rescale = needRescale = 0;

  initPlotInfo( i );
  arrayNumPoints[i] = 0;

  n = yPvCount[i];
  if ( xPvCount[i] < n ) n = xPvCount[i];

  for ( ii=0; ii<n; ii++ ) {

    switch ( yPvType[i] ) {
    case DBR_FLOAT:
      dyValue = (double) ( (float *) yPvData[i] )[ii];
      break;
    case DBR_DOUBLE: 
      dyValue = ( (double *) yPvData[i] )[ii];
      break;
    case DBR_SHORT:
      dyValue = (double) ( (unsigned short *) yPvData[i] )[ii];
      break;
    case DBR_CHAR:
      dyValue = (double) ( (unsigned char *) yPvData[i] )[ii];
      break;
    case DBR_LONG:
      dyValue = (double) ( (int *) yPvData[i] )[ii];
      break;
    case DBR_ENUM:
      dyValue = (double) ( (unsigned short *) yPvData[i] )[ii];
      break;
    default:
      dyValue = ( (double *) yPvData[i] )[ii];
      break;
    }

    if ( y1AxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) {
      if ( dyValue > 0 ) dyValue = log10( dyValue );
    }

    scaledY = (short) plotAreaH -
     (short) rint( ( dyValue - curY1Min ) *
     y1Factor[i] - y1Offset[i] );
  
    switch ( xPvType[i] ) {
    case DBR_FLOAT:
      dxValue = (double) ( (float *) xPvData[i] )[ii];
      break;
    case DBR_DOUBLE: 
      dxValue = ( (double *) xPvData[i] )[ii];
      break;
    case DBR_SHORT:
      dxValue = (double) ( (unsigned short *) xPvData[i] )[ii];
      break;
    case DBR_CHAR:
      dxValue = (double) ( (unsigned char *) xPvData[i] )[ii];
      break;
    case DBR_LONG:
      dxValue = (double) ( (int *) xPvData[i] )[ii];
      break;
    case DBR_ENUM:
      dxValue = (double) ( (unsigned short *) xPvData[i] )[ii];
      break;
    default:
      dxValue = ( (double *) xPvData[i] )[ii];
      break;
    }

    if ( xAxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) {
      if ( dxValue > 0 ) dxValue = log10( dxValue );
    }
    else if ( xAxisStyle == XYGC_K_AXIS_STYLE_TIME_LOG10 ) {
      if ( dxValue > 0 ) dxValue = log10( dxValue );
    }

    scaledX = (short) rint( ( dxValue - curXMin ) *
     xFactor[i] + xOffset[i] );

#if 0
    printf( "[%-d]\tdxValue = %-g\n", ii, dxValue );
    printf( "\tdyValue = %-g\n", dyValue );
    printf( "\tscaledX = %-d\n", scaledX );
    printf( "\tscaledY = %-d\n\n", scaledY );
    printf( "curY1Min = %-g\n", curY1Min );
    printf( "y1Factor[i] = %-g\n", y1Factor[i] );
    printf( "y1Offset[i] = %-g\n", y1Offset[i] );
    printf( "curXMin = %-g\n", curXMin );
    printf( "xFactor[i] = %-g\n", xFactor[i] );
    printf( "xOffset[i] = %-g\n\n", xOffset[i] );
#endif

    addPoint( dxValue, scaledX, scaledY, i );

    if ( xAxisSource == XYGC_K_AUTOSCALE ) {
      if ( kpXMinEfDouble.isNull() ) {
        if ( dxValue < curXMin ) {
          curXMin = dxValue;
          needRescale = 1;
        }
      }
      if ( kpXMaxEfDouble.isNull() ) {
        if ( dxValue > curXMax ) {
          curXMax = dxValue;
          needRescale = 1;
        }
      }
    }

    if ( y1AxisSource == XYGC_K_AUTOSCALE ) {
      if ( kpY1MinEfDouble.isNull() ) {
        if ( dyValue < curY1Min ) {
          needRescale = 1;
          curY1Min = dyValue;
        }
      }
      if ( kpY1MaxEfDouble.isNull() ) {
        if ( dyValue > curY1Max ) {
          needRescale = 1;
          curY1Max = dyValue;
          actWin->addDefExeNode( aglPtr );
        }
      }
    }

  }

  if ( needRescale ) {

    if ( xAxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) {
      get_log10_scale_params1( curXMin, curXMax, &curXMin, &curXMax,
       &curXNumLabelTicks, &curXMajorsPerLabel, &curXMinorsPerMajor, format );
    }
    else if ( xAxisStyle == XYGC_K_AXIS_STYLE_TIME_LOG10 ) {
      get_log10_scale_params1( curXMin, curXMax, &curXMin, &curXMax,
       &curXNumLabelTicks, &curXMajorsPerLabel, &curXMinorsPerMajor, format );
    }
    else {
      get_scale_params1( curXMin, curXMax, &curXMin, &curXMax,
       &curXNumLabelTicks, &curXMajorsPerLabel, &curXMinorsPerMajor, format );
    }

    if ( y1AxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) {
      get_log10_scale_params1( curY1Min, curY1Max, &curY1Min, &curY1Max,
       &curY1NumLabelTicks, &curY1MajorsPerLabel, &curY1MinorsPerMajor,
       format );
    }
    else {
      get_scale_params1( curY1Min, curY1Max, &curY1Min, &curY1Max,
       &curY1NumLabelTicks, &curY1MajorsPerLabel, &curY1MinorsPerMajor,
       format );
    }

    updateDimensions();

    for ( iii=0; iii<numTraces; iii++ ) {
      xFactor[iii] =
       (double) ( plotAreaW ) / ( curXMax - curXMin );
      xOffset[iii] = plotAreaX;
    }

    for ( iii=0; iii<numTraces; iii++ ) {
      y1Factor[iii] =
       (double) ( plotAreaH ) / ( curY1Max - curY1Min );
      y1Offset[iii] = plotAreaY;
    }

    *rescale = 1;

  }

  arrayHead[i] = 0;
  arrayTail[i] = n;
  yArrayGotValue[i] = xArrayGotValue[i] = 0;
  yArrayNeedUpdate[i] = xArrayNeedUpdate[i] = 1;

}

int xyGraphClass::fullRefresh ( void ) {

int i;

  if ( !activeMode || !init ) return 1;

  //printf( "full refresh\n" );

  actWin->executeGc.saveFg();

  actWin->executeGc.setFG( actWin->ci->pix(bgColor) );
  actWin->executeGc.setLineWidth(1);
  actWin->executeGc.setLineStyle( LineSolid );

  // erase all
  XDrawRectangle( actWin->d, pixmap,
   actWin->executeGc.eraseGC(), 0, 0,
   w, h );

  XFillRectangle( actWin->d, pixmap,
   actWin->executeGc.eraseGC(), 0, 0,
   w, h );

  XDrawRectangle( actWin->d, pixmap,
   actWin->executeGc.normGC(), plotAreaX, plotAreaY,
   plotAreaW, plotAreaH );

  XFillRectangle( actWin->d, pixmap,
   actWin->executeGc.normGC(), plotAreaX, plotAreaY,
   plotAreaW, plotAreaH );

  if ( border ) drawBorder();
  drawY1Scale();
  drawXScale();
  drawTitle();
  drawXlabel();
  drawYlabel();

  actWin->executeGc.setLineWidth(1);
  actWin->executeGc.setLineStyle( LineSolid );
  actWin->executeGc.restoreFg();

  bufInvalid = 0;
  for ( i=0; i<numTraces; i++ ) {
    traceIsDrawn[i] = 0;
    yArrayNeedUpdate[i] = 1;
    xArrayNeedUpdate[i] = 1;
  }

  drawActive();

  return 1;

}

int xyGraphClass::erase ( void ) {

  if ( activeMode || deleteRequest ) return 1;

  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  return 1;

}

int xyGraphClass::eraseActive ( void ) {

int i;
XRectangle xR = { plotAreaX-1, plotAreaY-1, plotAreaW+2, plotAreaH+2 };
int clipStat;

  if ( !activeMode || !init ) return 1;

  if ( bufInvalid ) {
    return 1;
  }

  actWin->executeGc.saveFg();
  actWin->executeGc.setFG( actWin->ci->pix(bgColor) );

  clipStat = actWin->executeGc.addNormXClipRectangle( xR );

  for ( i=0; i<numTraces; i++ ) {

    if ( yArrayNeedUpdate[i] && xArrayNeedUpdate[i] && traceIsDrawn[i] ) {

      actWin->executeGc.setLineWidth( lineThk[i] );
      actWin->executeGc.setLineStyle( lineStyle[i] );

      //actWin->executeGc.setFGforGivenBG( actWin->ci->pix(plotColor[i]),
      // actWin->ci->pix(bgColor) );

      traceIsDrawn[i] = 0;

      if ( plotStyle[i] == XYGC_K_PLOT_STYLE_POINT ) {

        if ( curNpts[i] > 1 ) {
          XDrawPoints( actWin->d, pixmap,
           actWin->executeGc.normGC(), plotBuf[i], curNpts[i],
           CoordModeOrigin );
        }

      }
      else{

        if ( curNpts[i] > 1 ) {
          XDrawLines( actWin->d, pixmap,
           actWin->executeGc.normGC(), plotBuf[i], curNpts[i],
           CoordModeOrigin );
        }

      }

    }

  }

  if ( clipStat & 1 ) actWin->executeGc.removeNormXClipRectangle();

  actWin->executeGc.setLineWidth(1);
  actWin->executeGc.setLineStyle( LineSolid );
  actWin->executeGc.restoreFg();

  return 1;

}

int xyGraphClass::draw ( void ) {

  if ( activeMode || deleteRequest ) return 1;

  actWin->drawGc.saveFg();

  actWin->drawGc.setFG( actWin->ci->pix(fgColor) );
  //actWin->drawGc.setBG( actWin->ci->pix(bgColor) );

  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x, y, w, h );

  actWin->drawGc.restoreFg();

  return 1;

}

int xyGraphClass::drawActiveOne (
  int i // trace
) {

int npts;

  actWin->executeGc.setLineWidth( lineThk[i] );
  actWin->executeGc.setLineStyle( lineStyle[i] );

  yArrayNeedUpdate[i] = xArrayNeedUpdate[i] = 1;
  if ( yArrayNeedUpdate[i] && xArrayNeedUpdate[i] ) {

    actWin->executeGc.setFGforGivenBG( actWin->ci->pix(plotColor[i]),
     actWin->ci->pix(bgColor) );

    traceIsDrawn[i] = 1;

    yArrayNeedUpdate[i] = xArrayNeedUpdate[i] = 0;

    if ( yPvCount[i] > 1 ) { // vector

      //printf( "draw active: vector, npts = %-d\n", arrayNumPoints[i] );

      npts = fillPlotArray( i );
      //printf( "i=%-d, npts=%-d\n", i, npts );

      if ( npts > 1 ) {

        if ( plotStyle[i] == XYGC_K_PLOT_STYLE_POINT ) {

          //printf( "draw active: draw lines, npts = %-d\n", npts );
          XDrawPoints( actWin->d, pixmap,
           actWin->executeGc.normGC(), plotBuf[i], npts,
           CoordModeOrigin );
          curNpts[i] = npts;

        }
        else {

          //printf( "draw active: draw lines, npts = %-d\n", npts );
          XDrawLines( actWin->d, pixmap,
           actWin->executeGc.normGC(), plotBuf[i], npts,
           CoordModeOrigin );
          curNpts[i] = npts;

        }

      }

    }
    else { // scalar

      //printf( "draw active: scalar, num pts = %-d\n", arrayNumPoints[i] );

      npts = fillPlotArray( i );
      //printf( "i=%-d, npts=%-d\n", i, npts );

      if ( npts > 1 ) {

        if ( plotStyle[i] == XYGC_K_PLOT_STYLE_POINT ) {

          //printf( "draw active: draw lines, npts = %-d\n", npts );
          XDrawPoints( actWin->d, pixmap,
           actWin->executeGc.normGC(), plotBuf[i], npts,
           CoordModeOrigin );
          curNpts[i] = npts;

        }
	else {

          //printf( "draw active: draw lines, npts = %-d\n", npts );
          XDrawLines( actWin->d, pixmap,
           actWin->executeGc.normGC(), plotBuf[i], npts,
           CoordModeOrigin );
          curNpts[i] = npts;

        }

      }

    }

  }

  return 1;

}

int xyGraphClass::drawActive ( void ) {

int i;
XRectangle xR = { plotAreaX-1, plotAreaY-1, plotAreaW+2, plotAreaH+2 };
int clipStat;

  if ( !activeMode || !init ) return 1;

  if ( bufInvalid ) {
    actWin->appCtx->proc->lock();
    needRefresh = 1;
    actWin->addDefExeNode( aglPtr );
    actWin->appCtx->proc->unlock();
    return 1;
  }

  drawGrid();

  actWin->executeGc.saveFg();

  clipStat = actWin->executeGc.addNormXClipRectangle( xR );

  for ( i=0; i<numTraces; i++ ) {

    drawActiveOne( i );

  }

  if ( clipStat & 1 ) actWin->executeGc.removeNormXClipRectangle();

  actWin->executeGc.setLineWidth(1);
  actWin->executeGc.setLineStyle( LineSolid );
  actWin->executeGc.restoreFg();

  XCopyArea( actWin->display(), pixmap,
   XtWindow(actWin->executeWidget), actWin->executeGc.normGC(),
   0, 0, w+1, h+1, x, y );

  return 1;

}

int xyGraphClass::altDrawActiveOne (
  int i // trace
) {

XRectangle xR = { plotAreaX-1, plotAreaY-1, plotAreaW+2, plotAreaH+2 };
int clipStat;

  if ( !activeMode || !init ) return 1;

  if ( bufInvalid ) {
    actWin->appCtx->proc->lock();
    needRefresh = 1;
    actWin->addDefExeNode( aglPtr );
    actWin->appCtx->proc->unlock();
    return 1;
  }

  drawGrid();

  actWin->executeGc.saveFg();

  clipStat = actWin->executeGc.addNormXClipRectangle( xR );

  drawActiveOne( i );

  if ( clipStat & 1 ) actWin->executeGc.removeNormXClipRectangle();

  actWin->executeGc.setLineWidth(1);
  actWin->executeGc.setLineStyle( LineSolid );
  actWin->executeGc.restoreFg();

  XCopyArea( actWin->display(), pixmap,
   XtWindow(actWin->executeWidget), actWin->executeGc.normGC(),
   0, 0, w+1, h+1, x, y );

  return 1;

}

void xyGraphClass::bufInvalidate ( void ) {

  bufInvalid = 1;

}

int xyGraphClass::expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] ) {

int i, stat, retStat = 1;

  stat = graphTitle.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;

  stat = xLabel.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;

  stat = yLabel.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;

  stat = trigPvExpStr.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;

  stat = resetPvExpStr.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;

  for ( i=0; i<numTraces; i++ ) {
    stat = xPvExpStr[i].expand1st( numMacros, macros, expansions );
    if ( !( stat & 1 ) ) retStat = stat;
    stat = yPvExpStr[i].expand1st( numMacros, macros, expansions );
    if ( !( stat & 1 ) ) retStat = stat;
  }

  return retStat;

}

int xyGraphClass::expand2nd (
  int numMacros,
  char *macros[],
  char *expansions[] ) {

int i, stat, retStat = 1;

  stat = graphTitle.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;

  stat = xLabel.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;

  stat = yLabel.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;

  stat = trigPvExpStr.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;

  stat = resetPvExpStr.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;

  for ( i=0; i<numTraces; i++ ) {
    stat = xPvExpStr[i].expand2nd( numMacros, macros, expansions );
    if ( !( stat & 1 ) ) retStat = stat;
    stat = yPvExpStr[i].expand2nd( numMacros, macros, expansions );
    if ( !( stat & 1 ) ) retStat = stat;
  }

  return retStat;

}

int xyGraphClass::containsMacros ( void ) {

int i, result;

  result = graphTitle.containsPrimaryMacros();
  if ( result ) return result;

  result = xLabel.containsPrimaryMacros();
  if ( result ) return result;

  result = yLabel.containsPrimaryMacros();
  if ( result ) return result;

  result = trigPvExpStr.containsPrimaryMacros();
  if ( result ) return result;

  result = resetPvExpStr.containsPrimaryMacros();
  if ( result ) return result;

  for ( i=0; i<numTraces; i++ ) {
    result = xPvExpStr[i].containsPrimaryMacros();
    if ( result ) return result;
    result = yPvExpStr[i].containsPrimaryMacros();
    if ( result ) return result;
  }

  return 0;

}

int xyGraphClass::activate (
  int pass,
  void *ptr )
{

int i, stat;
int screen_num, depth;

  switch ( pass ) {

  case 1:

    opComplete = 0;
    break;

  case 2:

    if ( !opComplete ) {

      opComplete = 1;

      // for keypad functions
      xMinX0 = xMinX1 = xMinY0 = xMinY1 = -1;
      xMaxX0 = xMaxX1 = xMaxY0 = xMaxY1 = -1;
      kpXMinEfDouble.setNull(1);
      kpXMaxEfDouble.setNull(1);
      kpCancelMinX = kpCancelMaxX = 0;

      y1MinX0 = y1MinX1 = y1MinY0 = y1MinY1 = -1;
      y1MaxX0 = y1MaxX1 = y1MaxY0 = y1MaxY1 = -1;
      kpY1MinEfDouble.setNull(1);
      kpY1MaxEfDouble.setNull(1);
      kpCancelMinY1 = kpCancelMaxY1 = 0;

      // for timer
      updateTimer = 0;
      updateTimerActive = 0;

      // for message dialog
      msgDialog.create( actWin->topWidgetId() );
      msgDialogPopedUp = 0;

      firstBoxRescale = 1;
      doingBoxRescale = 0;

      screen_num = DefaultScreen( actWin->display() );
      depth = DefaultDepth( actWin->display(), screen_num );
      pixmap = XCreatePixmap( actWin->display(),
       XtWindow(actWin->executeWidget), w+1, h+1, depth );

      // clear pixmap
      actWin->executeGc.saveFg();

      actWin->executeGc.setFG( actWin->ci->pix(bgColor) );
      actWin->executeGc.setLineWidth(1);
      actWin->executeGc.setLineStyle( LineSolid );

      // erase all
      XDrawRectangle( actWin->d, pixmap,
       actWin->executeGc.eraseGC(), 0, 0,
       w, h );

      XFillRectangle( actWin->d, pixmap,
       actWin->executeGc.eraseGC(), 0, 0,
       w, h );

#if 0
      XDrawRectangle( actWin->d, pixmap,
       actWin->executeGc.normGC(), plotAreaX, plotAreaY,
       plotAreaW, plotAreaH );

      XFillRectangle( actWin->d, pixmap,
       actWin->executeGc.normGC(), plotAreaX, plotAreaY,
       plotAreaW, plotAreaH );
#endif

      actWin->executeGc.restoreFg();

      curXMin = xMin.value();
      curXMax = xMax.value();
      if ( xAxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) {
        curXMin = log10( curXMin );
	curXMax = log10( curXMax );
      }
      else if ( xAxisStyle == XYGC_K_AXIS_STYLE_TIME_LOG10 ) {
        curXMin = log10( curXMin );
	curXMax = log10( curXMax );
      }

      stripChartXMax = curXMax;

      curY1Min = y1Min.value();
      curY1Max = y1Max.value();
      if ( y1AxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) {
	curY1Min = log10( curY1Min );
	curY1Max = log10( curY1Max );
      }

      curY2Min = y2Min.value();
      curY2Max = y2Max.value();
      if ( y2AxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) {
	curY2Min = log10( curY2Min );
	curY2Max = log10( curY2Max );
      }

      updateDimensions();

      curXNumLabelTicks = xNumLabelIntervals.value();
      if ( curXNumLabelTicks < 2 ) curXNumLabelTicks = 2;
      curXMajorsPerLabel = xNumMajorPerLabel.value();
      curXMinorsPerMajor = xNumMinorPerMajor.value();

      curY1NumLabelTicks = y1NumLabelIntervals.value();
      if ( curY1NumLabelTicks < 2 ) curY1NumLabelTicks = 2;
      curY1MajorsPerLabel = y1NumMajorPerLabel.value();
      curY1MinorsPerMajor = y1NumMinorPerMajor.value();

      curY2NumLabelTicks = y2NumLabelIntervals.value();
      if ( curY2NumLabelTicks < 2 ) curY2NumLabelTicks = 2;
      curY2MajorsPerLabel = y2NumMajorPerLabel.value();
      curY2MinorsPerMajor = y2NumMinorPerMajor.value();

      aglPtr = ptr;
      connection.init();
      init = 0;
      bufInvalid = 1;
      activeMode = 1;
      firstTimeSample = 1;
      numBufferScrolls = 0;
      needConnect = needInit = needRefresh = needErase = needDraw = 
       needUpdate = needResetConnect = needReset = needTrigConnect =
       needTrig = needXRescale = needY1Rescale = needY2Rescale =
       needBufferScroll = needVectorUpdate = needRealUpdate =
       needBoxRescale = 0;

      resetEv = trigEv = NULL;

      if ( !blank( resetPvExpStr.getExpanded() ) ) {
        resetPvExists = 1;
	stat = ca_search_and_connect( resetPvExpStr.getExpanded(),
         &resetPv, resetMonitorConnection, this );
        if ( stat != ECA_NORMAL ) {
          printf( "ca_search_and_connect failed for [%s]\n",
           resetPvExpStr.getExpanded() );
        }
      }
      else {
        resetPvExists = 0;
      }

      for ( i=0; i<numTraces; i++ ) {
        arrayHead[i] = 0;
        arrayTail[i] = 0;
        arrayNumPoints[i] = 0;
        curNpts[i] = 0;
        xArrayNeedInit[i] = 0;
        xArrayNeedUpdate[i] = 0;
        yArrayNeedInit[i] = 0;
        yArrayNeedUpdate[i] = 0;
        yArrayGotValue[i] = 0;
        xArrayGotValue[i] = 0;
        xPvData[i] = NULL;
        yPvData[i] = NULL;
        xPv[i] = NULL;
        yPv[i] = NULL;
        xEv[i] = NULL;
        yEv[i] = NULL;
        plotBuf[i] = NULL;
        plotBufSize[i] = 0;
        plotInfoHead[i] = 0;
        plotInfoTail[i] = 0;
        plotInfo[i] = NULL;
        plotInfoSize[i] = 0;
        traceIsDrawn[i] = 0;
        plotState[i] = XYGC_K_STATE_INITIALIZING;
        needThisbufScroll[i] = 0;
        totalCount[i] = 0;
      }

      for ( i=0; i<numTraces; i++ ) {

        if ( !blank( yPvExpStr[i].getExpanded() ) ) {

          if ( traceType[i] == XYGC_K_TRACE_XY ) {

            argRec[i].objPtr = (void *) this;
            argRec[i].index = i + XYGC_K_MAX_TRACES;

            connection.addPv();

            stat = ca_search_and_connect( yPvExpStr[i].getExpanded(),
             &yPv[i], yMonitorConnection, &argRec[i] );
            if ( stat != ECA_NORMAL ) {
              printf( "ca_search_and_connect failed for [%s]\n",
               yPvExpStr[i].getExpanded() );
            }

            if ( !blank( xPvExpStr[i].getExpanded() ) ) {

              argRec[i].objPtr = (void *) this;
              argRec[i].index = i;

              connection.addPv();

              stat = ca_search_and_connect( xPvExpStr[i].getExpanded(),
               &xPv[i], xMonitorConnection, &argRec[i] );
              if ( stat != ECA_NORMAL ) {
                printf( "ca_search_and_connect failed for [%s]\n",
                 xPvExpStr[i].getExpanded() );
              }

            }

          }
	  else if ( traceType[i] == XYGC_K_TRACE_CHRONOLOGICAL ) {

            argRec[i].objPtr = (void *) this;
            argRec[i].index = i + XYGC_K_MAX_TRACES;

            connection.addPv();

            stat = ca_search_and_connect( yPvExpStr[i].getExpanded(),
             &yPv[i], yMonitorConnection, &argRec[i] );
            if ( stat != ECA_NORMAL ) {
              printf( "ca_search_and_connect failed for [%s]\n",
               yPvExpStr[i].getExpanded() );
            }

          }

	}

      }

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

int xyGraphClass::deactivate (
  int pass
) {

int i, stat;

  switch ( pass ) {

  case 1:

    activeMode = 0;

    if ( updateTimerActive ) {
      if ( updateTimer ) {
        XtRemoveTimeOut( updateTimer );
        updateTimer = 0;
      }
      updateTimerActive = 0;
    }

    msgDialog.destroy(); 

    for ( i=0; i<numTraces; i++ ) {

      if ( xEv[i] ) {
        //stat = ca_clear_event( xEv[i] );
        stat = ca_clear_channel( xPv[i] );
        xEv[i] = NULL;
      }

      if ( yEv[i] ) {
        //stat = ca_clear_event( xEv[i] );
        stat = ca_clear_channel( yPv[i] );
        yEv[i] = NULL;
      }

      if ( resetEv ) {
        //stat = ca_clear_event( resetEv );
        stat = ca_clear_channel( resetPv );
        resetEv = NULL;
      }

      if ( trigEv ) {
        //stat = ca_clear_event( trigEv );
        stat = ca_clear_channel( trigPv );
        trigEv = NULL;
      }

      if ( xPvData[i] ) {
        delete (char *) xPvData[i];
        xPvData[i] = NULL;
      }

      if ( yPvData[i] ) {
        delete (char *) yPvData[i];
        yPvData[i] = NULL;
      }

      if ( plotBuf[i] ) {
        delete plotBuf[i];
        plotBuf[i] = NULL;
        plotBufSize[i] = 0;
      }

      if ( plotInfo[i] ) {
        delete plotInfo[i];
        plotInfo[i] = NULL;
        plotInfoSize[i] = 0;
      }

    }

  }

  if ( pixmap ) {
    XFreePixmap( actWin->display(), pixmap );
    pixmap = (Pixmap) NULL;
  }

  return 1;

}

void xyGraphClass::updateDimensions ( void ) {

int l, l1, bInc, tInc, xlInc, ylInc;

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

  if ( y1AxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) {
    l = yScaleWidth( fontTag, fs, pow(10,curY1Min), pow(10,curY1Max) );
  }
  else {
    l = yScaleWidth( fontTag, fs, curY1Min, curY1Max );
  }

  if ( xAxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) {
    l1 = xScaleMargin( fontTag, fs, pow(10,curXMin), pow(10,curXMax) );
  }
  else if ( xAxisStyle == XYGC_K_AXIS_STYLE_TIME_LOG10 ) {
    l1 = xScaleMargin( fontTag, fs, pow(10,curXMin), pow(10,curXMax) );
  }
  else {
    l1 = xScaleMargin( fontTag, fs, curXMin, curXMax );
  }

  if ( l < l1 ) l = l1;

  if ( xAxis || y1Axis ) {

    plotAreaX = l + fontHeight;
    plotAreaW = w - plotAreaX - l1 - 10;

    //printf( "curXMin = %-g, curXMax = %-g\n", curXMin, curXMax );

    plotAreaH = h - xScaleHeight( fontTag, fs ) - 3 * fontHeight;
    plotAreaY = 2 * fontHeight;

  }
  else {

    if ( border )
      bInc = 1;
    else
      bInc = 0;

    if ( blank( graphTitle.getExpanded() ) )
      tInc = 0;
    else
      tInc = fontHeight;

    if ( blank( xLabel.getExpanded() ) )
      xlInc = 0;
    else
      xlInc = fontHeight;

    if ( blank( yLabel.getExpanded() ) )
      ylInc = 0;
    else
      ylInc = fontHeight;

    plotAreaX = 0 + bInc + ylInc * 3 / 2;
    plotAreaW = w - bInc - bInc - ylInc * 3;
    plotAreaY = 0 + bInc + tInc * 2;
    plotAreaH = h - bInc - bInc - ( tInc + xlInc ) * 2;

  }

}

void xyGraphClass::btnDrag (
  int _x,
  int _y,
  int buttonState,
  int buttonNumber )
{

int rsw, rsh;
int pmX, pmY;

  pmX = _x - this->x;
  pmY = _y - this->y;

  if ( doingBoxRescale ) {

    actWin->executeGc.saveFg();
    actWin->executeGc.setLineWidth(1);
    actWin->executeGc.setLineStyle( LineSolid );
    actWin->executeGc.setFGforGivenBG (
     actWin->ci->pix(fgColor), actWin->ci->pix(bgColor) );

    XDrawRectangle( actWin->d, pixmap,
     actWin->executeGc.xorGC(), rescaleBoxX0, rescaleBoxY0,
     oldRescaleBoxW, oldRescaleBoxH );

    rescaleBoxX1 = pmX;
    rescaleBoxY1 = pmY;
    rsw = abs( rescaleBoxX1 - rescaleBoxX0 );
    rsh = abs( rescaleBoxY1 - rescaleBoxY0 );
    oldRescaleBoxW = rsw;
    oldRescaleBoxH = rsh;

    XDrawRectangle( actWin->d, pixmap,
     actWin->executeGc.xorGC(), rescaleBoxX0, rescaleBoxY0,
     oldRescaleBoxW, oldRescaleBoxH );

    actWin->executeGc.restoreFg();

    actWin->appCtx->proc->lock();
    needRealUpdate = 1;
    actWin->addDefExeNode( aglPtr );
    actWin->appCtx->proc->unlock();

  }

}

void xyGraphClass::btnUp (
  int _x,
  int _y,
  int buttonState,
  int buttonNumber )
{
int pmX, pmY;
double dx0, dy0, dx1, dy1;

  pmX = _x - this->x;
  pmY = _y - this->y;

  if ( doingBoxRescale ) {

    //printf( "btnUp, curXMin=%-g, curXMax=%-g\n", curXMin, curXMax );

    actWin->executeGc.saveFg();
    actWin->executeGc.setLineWidth(1);
    actWin->executeGc.setLineStyle( LineSolid );
    actWin->executeGc.setFGforGivenBG (
     actWin->ci->pix(fgColor), actWin->ci->pix(bgColor) );

    XDrawRectangle( actWin->d, pixmap,
     actWin->executeGc.xorGC(), rescaleBoxX0, rescaleBoxY0,
     oldRescaleBoxW, oldRescaleBoxH );

    actWin->executeGc.restoreFg();

    doingBoxRescale = 0;

    rescaleBoxX1 = pmX;
    rescaleBoxY1 = pmY;

    dx0 = ( rescaleBoxX0 - xOffset[0] ) / xFactor[0] + curXMin;
    dy0 = ( plotAreaH - rescaleBoxY0 + y1Offset[0] ) /y1Factor[0] + curY1Min;
    dx1 = ( rescaleBoxX1 - xOffset[0] ) / xFactor[0] + curXMin;
    dy1 = ( plotAreaH - rescaleBoxY1 + y1Offset[0] ) /y1Factor[0] + curY1Min;

    if ( dx0 < dx1 ) {
      boxXMin = dx0;
      boxXMax = dx1;
    }
    else {
      boxXMin = dx1;
      boxXMax = dx0;
    }

    if ( dy0 < dy1 ) {
      boxYMin = dy0;
      boxYMax = dy1;
    }
    else {
      boxYMin = dy1;
      boxYMax = dy0;
    }

    kpXMinEfDouble.setNull(0);
    kpXMaxEfDouble.setNull(0);
    kpY1MinEfDouble.setNull(0);
    kpY1MaxEfDouble.setNull(0);

    //printf( "rescale to (%-g,%-g) (%-g,%-g)\n", boxXMin, boxYMin,
    // boxXMax, boxYMax );

    actWin->appCtx->proc->lock();
    needBoxRescale = 1;
    actWin->addDefExeNode( aglPtr );
    actWin->appCtx->proc->unlock();

  }

  if ( ( buttonNumber == 3 ) &&
      !( buttonState & ShiftMask ) &&
      !( buttonState & ControlMask ) ) {

    if ( !firstBoxRescale ) {

      firstBoxRescale = 1;
      boxXMin = savedXMin;
      boxXMax = savedXMax;
      boxYMin = savedYMin;
      boxYMax = savedYMax;
      kpXMinEfDouble.setNull( savedXMinNullState );
      kpXMaxEfDouble.setNull( savedXMaxNullState );
      kpY1MinEfDouble.setNull( savedYMinNullState );
      kpY1MaxEfDouble.setNull( savedYMaxNullState );

      actWin->appCtx->proc->lock();
      needBoxRescale = 1;
      actWin->addDefExeNode( aglPtr );
      actWin->appCtx->proc->unlock();

    }

  }

  if ( msgDialogPopedUp ) {
    msgDialog.popdown();
    msgDialogPopedUp = 0;
    return;
  }

}

void xyGraphClass::btnDown (
  int _x,
  int _y,
  int buttonState,
  int buttonNumber )
{

double dxValue, dyValue;
char buf[63+1];
int pmX, pmY;

  pmX = _x - this->x;
  pmY = _y - this->y;

  if ( ( buttonNumber == 1 ) &&
       ( buttonState & ShiftMask ) ) {

    if ( ( pmX > plotAreaX ) && ( pmX < plotAreaX+plotAreaW ) &&
         ( pmY > plotAreaY ) && ( pmY < plotAreaY+plotAreaH ) ) {

      //scaledX = (short) rint( ( dxValue - curXMin ) *
      // xFactor[0] + xOffset[0] );

      dxValue = ( pmX - xOffset[0] ) / xFactor[0] + curXMin;
      if ( ( xAxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) ||
           ( xAxisStyle == XYGC_K_AXIS_STYLE_TIME_LOG10 ) ) {
        dxValue = pow(10,dxValue);
      }

      //scaledY = (short) plotAreaH -
      // (short) rint( ( dyValue - curY1Min ) *
      // y1Factor[0] - y1Offset[0] );

      dyValue = ( plotAreaH - pmY + y1Offset[0] ) /y1Factor[0] + curY1Min;
      if ( y1AxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) {
        dyValue = pow(10,dyValue);
      }
 
      //printf( "pmX=%-g, pmY=%-g\n", dxValue, dyValue );

      if ( msgDialogPopedUp ) {
        msgDialog.popdown();
      }
      sprintf( buf, "(%-.4g,%-.4g)", dxValue, dyValue );
      msgDialog.popup( buf, actWin->x+this->x, actWin->y+this->y );
      msgDialogPopedUp = 1;

    }

  }
  else if ( ( buttonNumber == 1 ) &&
      !( buttonState & ShiftMask ) &&
      !( buttonState & ControlMask ) ) {

    if ( ( pmX > plotAreaX ) && ( pmX < plotAreaX+plotAreaW ) &&
         ( pmY > plotAreaY ) && ( pmY < plotAreaY+plotAreaH ) ) {

      if ( firstBoxRescale ) {
        firstBoxRescale = 0;
	savedXMin = curXMin;
	savedXMax = curXMax;
	savedYMin = curY1Min;
	savedYMax = curY1Max;
	savedXMinNullState = kpXMinEfDouble.isNull();
	savedXMaxNullState = kpXMaxEfDouble.isNull();
	savedYMinNullState = kpY1MinEfDouble.isNull();
	savedYMaxNullState = kpY1MaxEfDouble.isNull();
      }
      rescaleBoxX0 = pmX;
      rescaleBoxY0 = pmY;
      oldRescaleBoxW = 0;
      oldRescaleBoxH = 0;

      actWin->executeGc.saveFg();
      actWin->executeGc.setLineWidth(1);
      actWin->executeGc.setLineStyle( LineSolid );
      actWin->executeGc.setFGforGivenBG (
       actWin->ci->pix(fgColor), actWin->ci->pix(bgColor) );
      XDrawRectangle( actWin->d, pixmap,
       actWin->executeGc.xorGC(), rescaleBoxX0, rescaleBoxY0,
       oldRescaleBoxW, oldRescaleBoxH );
      actWin->executeGc.restoreFg();

      doingBoxRescale = 1;

    }
    else {

      if ( ( xMinX0 <= pmX ) && ( xMinX1 >= pmX ) &&
           ( xMinY0 <= pmY ) && ( xMinY1 >= pmY ) ) {
        if ( !kp.isPoppedUp() ) {
          kp.create( actWin->top, pmX+actWin->x+this->x,
           pmY+actWin->y+this->y, "",
           &kpXMin, (void *) this,
           (XtCallbackProc) setKpXMinDoubleValue,
           (XtCallbackProc) cancelKpXMin );
        }
      }

      if ( ( xMaxX0 <= pmX ) && ( xMaxX1 >= pmX ) &&
           ( xMaxY0 <= pmY ) && ( xMaxY1 >= pmY ) ) {
        if ( !kp.isPoppedUp() ) {
          kp.create( actWin->top, pmX+actWin->x+this->x,
           pmY+actWin->y+this->y, "",
           &kpXMax, (void *) this,
           (XtCallbackProc) setKpXMaxDoubleValue,
           (XtCallbackProc) cancelKpXMax );
        }
      }

      if ( ( y1MinX0 <= pmX ) && ( y1MinX1 >= pmX ) &&
           ( y1MinY0 <= pmY ) && ( y1MinY1 >= pmY ) ) {
        if ( !kp.isPoppedUp() ) {
          kp.create( actWin->top, pmX+actWin->x+this->x,
           pmY+actWin->y+this->y, "",
           &kpY1Min, (void *) this,
           (XtCallbackProc) setKpY1MinDoubleValue,
           (XtCallbackProc) cancelKpY1Min );
        }
      }

      if ( ( y1MaxX0 <= pmX ) && ( y1MaxX1 >= pmX ) &&
           ( y1MaxY0 <= pmY ) && ( y1MaxY1 >= pmY ) ) {
        if ( !kp.isPoppedUp() ) {
          kp.create( actWin->top, pmX+actWin->x+this->x,
           pmY+actWin->y+this->y, "",
           &kpY1Max, (void *) this,
           (XtCallbackProc) setKpY1MaxDoubleValue,
           (XtCallbackProc) cancelKpY1Max );
        }
      }

    }

  }

}

int xyGraphClass::getButtonActionRequest (
  int *up,
  int *down,
  int *drag )
{

  *up = 1;
  *down = 1;
  *drag = 1;

  return 1;

}

void xyGraphClass::executeDeferred ( void ) {

int i, ii, stat, nc, ni, nu, nvu, nru, nr, ne, nd, nrstc, nrst, ntrgc,
 ntrg, nxrescl, ny1rescl, ny2rescl, nbs, nbrescl,
 eleSize, scaledX, scaledY, structType, doRescale, anyRescale, size,
 firstSample;
double dyValue, dxValue, range, max, timeInc;
char format[31+1];

  if ( actWin->isIconified ) return;

  actWin->appCtx->proc->lock();
  nc = needConnect; needConnect = 0;
  ni = needInit; needInit = 0;
  nu = needUpdate; needUpdate = 0;
  nvu = needVectorUpdate; needVectorUpdate = 0;
  nru = needRealUpdate; needRealUpdate = 0;
  nr = needRefresh; needRefresh = 0;
  ne = needErase; needErase = 0;
  nd = needDraw; needDraw = 0;
  nrstc = needResetConnect; needResetConnect = 0;
  nrst = needReset; needReset = 0;
  ntrgc = needTrigConnect; needTrigConnect = 0;
  ntrg = needTrig; needTrig = 0;
  nxrescl = needXRescale; needXRescale = 0;
  ny1rescl = needY1Rescale; needY1Rescale = 0;
  ny2rescl = needY2Rescale; needY2Rescale = 0;
  nbs = needBufferScroll; needBufferScroll = 0;
  nbrescl = needBoxRescale; needBoxRescale = 0;
  actWin->remDefExeNode( aglPtr );
  actWin->appCtx->proc->unlock();

  if ( !activeMode ) return;

  if ( nc ) {

    //printf( "need connect\n" );

    for ( i=0; i<numTraces; i++ ) {

      if ( yPv[i] ) {

        yPvType[i] = ca_field_type( yPv[i] );
        yPvCount[i] = ca_element_count( yPv[i] );

        switch ( yPvType[i] ) {
        case DBR_FLOAT:
          eleSize = 4;
          break;
        case DBR_DOUBLE: 
          eleSize = 8;
          break;
        case DBR_SHORT:
          eleSize = 2;
          break;
        case DBR_CHAR:
          eleSize = 1;
          break;
        case DBR_LONG:
          eleSize = 4;
          break;
        case DBR_ENUM:
          eleSize = 2;
          break;
        default:
          eleSize = 8;
          break;
        }

        yPvSize[i] = ca_element_count( yPv[i] ) * eleSize;

        argRec[i].objPtr = (void *) this;
        argRec[i].index = i;

        stat = ca_get_callback( DBR_GR_DOUBLE, yPv[i],
         yInfoUpdate, (void *) argRec );
        if ( stat != ECA_NORMAL ) {
          printf( "error from ca_get_callback\n" );
        }

      }

      if ( xPv[i] ) {

        xPvType[i] = ca_field_type( xPv[i] );
        xPvCount[i] = ca_element_count( xPv[i] );

        switch ( xPvType[i] ) {
        case DBR_FLOAT:
          eleSize = 4;
          break;
        case DBR_DOUBLE: 
          eleSize = 8;
          break;
        case DBR_SHORT:
          eleSize = 2;
          break;
        case DBR_CHAR:
          eleSize = 1;
          break;
        case DBR_LONG:
          eleSize = 4;
          break;
        case DBR_ENUM:
          eleSize = 2;
          break;
        default:
          eleSize = 8;
          break;
        }

        xPvSize[i] = ca_element_count( xPv[i] ) * eleSize;

        argRec[i].objPtr = (void *) this;
        argRec[i].index = i;

        stat = ca_get_callback( DBR_GR_DOUBLE, xPv[i],
         xInfoUpdate, (void *) argRec );
        if ( stat != ECA_NORMAL ) {
          printf( "error from ca_get_callback\n" );
        }

      }

    }

  }

  if ( nrstc ) {

    stat = ca_add_array_event( DBR_SHORT, 1, resetPv,
     resetValueUpdate, (void *) this, 0.0, 0.0, 0.0, &resetEv );
    if ( stat != ECA_NORMAL ) {
      printf( "error from ca_add_array_event\n" );
    }

  }

  if ( ni ) {

    for ( i=0; i<numTraces; i++ ) {

      if ( yArrayNeedInit[i] ) {

        if ( i == 0 ) {
          if ( y1AxisSource == XYGC_K_FROM_PV ) {
	    curY1Min = dbYMin[i];
            curY1Max = dbYMax[i];
            curY1Prec = dbYPrec[i];
            if ( y1AxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) {
            	if ( curY1Min > 0 ) curY1Min = log10( curY1Min );
            	if ( curY1Max > 0 ) curY1Max = log10( curY1Max );
            }
	  }
	}

        if ( i == 1 ) {
          if ( y2AxisSource == XYGC_K_FROM_PV ) {
	    curY2Min = dbYMin[i];
            curY2Max = dbYMax[i];
            curY2Prec = dbYPrec[i];
            if ( y2AxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) {
            	if ( curY2Min > 0 ) curY2Min = log10( curY2Min );
            	if ( curY2Max > 0 ) curY2Max = log10( curY2Max );
            }
	  }
	}

        yArrayNeedInit[i] = 0;

        xFactor[i] =
         (double) ( plotAreaW ) / ( curXMax - curXMin );
        xOffset[i] = plotAreaX;

        y1Factor[i] =
         (double) ( plotAreaH ) / ( curY1Max - curY1Min );
        y1Offset[i] = plotAreaY;

        argRec[i].objPtr = (void *) this;
        argRec[i].index = i;

        if ( !yPvData[i] ) {

          if ( yPvCount[i] > 1 ) { // vector

            //printf( "count = %-d\n", yPvCount[i] );

            yPvData[i] = (void *) new char[yPvSize[i]*(yPvCount[i]+10)];

            size = (plotAreaX+plotAreaW)*4+10;
            if ( 3*yPvCount[i]+10 > size ) size = 3*yPvCount[i]+10;
            //printf( "plotBuf size = %-d\n", size );
            plotBuf[i] = (XPoint *) new XPoint[size];

            plotBufSize[i] = yPvCount[i]+1; // used with plotInfo in scope mode

            size = plotAreaX+plotAreaW+10;
            if ( 2*yPvCount[i]+10 > size ) size = 2*yPvCount[i]+10;
            //printf( "plotInfo size = %-d\n", size );
            plotInfo[i] =
             (plotInfoPtr) new plotInfoType[size];
            plotInfoSize[i] = plotAreaX + plotAreaW;

            initPlotInfo( i );

          }
	  else { // scalar

            if ( count < 2 ) count = 2;

            bufferScrollSize = (int) ( (double) count * 0.1 );
            if ( bufferScrollSize < 1 ) bufferScrollSize = 1;

            //printf( "count = %-d\n", count );
            yPvData[i] = (void *) new char[yPvSize[i]*(count+10)];

            size = (plotAreaX+plotAreaW)*4+10;
            if ( 2*count+10 > size ) size = 2*count+10;
            //printf( "plotBuf size = %-d\n", size );
            plotBuf[i] = (XPoint *) new XPoint[size];

            plotBufSize[i] = count+1; // used with plotInfo in scope mode

            size = plotAreaX+plotAreaW+10;
            if ( 2*count+10 > size ) size = 2*count+10;
            //printf( "plotInfo size = %-d\n", size );
            plotInfo[i] =
             (plotInfoPtr) new plotInfoType[size];
            plotInfoSize[i] = plotAreaX + plotAreaW;

            initPlotInfo( i );

          }

        }

        if ( traceType[i] == XYGC_K_TRACE_XY ) {

          if ( i == 0 ) {
            if ( xAxisSource == XYGC_K_FROM_PV ) {
	      curXMin = dbXMin[i];
              curXMax = dbXMax[i];
              curXPrec = dbXPrec[i];
              if ( xAxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) {
                curXMin = log10( curXMin );
        	curXMax = log10( curXMax );
              }
              else if ( xAxisStyle == XYGC_K_AXIS_STYLE_TIME_LOG10 ) {
                curXMin = log10( curXMin );
        	curXMax = log10( curXMax );
              }
              stripChartXMax = curXMax;
            }
          }

          stat = ca_add_array_event( ca_field_type(yPv[i]), yPvCount[i],
           yPv[i], yValueUpdate, (void *) argRec, 0.0, 0.0, 0.0, &yEv[i] );
          if ( stat != ECA_NORMAL ) {
            printf( "error from ca_add_array_event\n" );
          }

	}
	else if ( traceType[i] == XYGC_K_TRACE_CHRONOLOGICAL ) {

          if ( i == 0 ) {
            if ( xAxisSource == XYGC_K_FROM_PV ) {
              xAxisSource = XYGC_K_AUTOSCALE;
	    }
	  }

          if ( !xPvData[i] ) {
            xPvSize[i] = sizeof(double);
            xPvData[i] = (void *) new char[xPvSize[i]*(plotBufSize[i]+10)];
            for ( ii=0; ii<plotBufSize[i]; ii++ ) {
              ( (double *) xPvData[i] )[ii] = (double) ii;
	    }
	  }

          if ( yPvCount[i] > 1 ) { // vector

            stat = ca_add_array_event( yPvType[i], yPvCount[i],
             yPv[i], yValueWithTimeUpdate, (void *) argRec, 0.0, 0.0, 0.0,
              &yEv[i] );
            if ( stat != ECA_NORMAL ) {
              printf( "error from ca_add_array_event\n" );
            }

	  }
	  else {

            switch ( yPvType[i] ) {
            case DBR_FLOAT:
              structType = DBR_TIME_FLOAT;
              break;
            case DBR_DOUBLE: 
              structType = DBR_TIME_DOUBLE;
              break;
            case DBR_SHORT:
              structType = DBR_TIME_SHORT;
              break;
            case DBR_CHAR:
              structType = DBR_TIME_CHAR;
              break;
            case DBR_LONG:
              structType = DBR_TIME_LONG;
              break;
            case DBR_ENUM:
              structType = DBR_TIME_ENUM;
              break;
            default:
              structType = DBR_TIME_DOUBLE;
              break;
            }

            stat = ca_add_array_event( structType, yPvCount[i],
             yPv[i], yValueWithTimeUpdate, (void *) argRec, 0.0, 0.0, 0.0,
              &yEv[i] );
            if ( stat != ECA_NORMAL ) {
              printf( "error from ca_add_array_event\n" );
            }

	  }

	}

      }

      if ( xArrayNeedInit[i] ) {

        xArrayNeedInit[i] = 0;

        argRec[i].objPtr = (void *) this;
        argRec[i].index = i;

        if ( !xPvData[i] ) {

          if ( xPvCount[i] > 1 ) { // vector

            //printf( "count = %-d\n", xPvCount[i] );
            
            xPvData[i] = (void *) new char[xPvSize[i]*(xPvCount[i]+10)];

          }
	  else { // scalar

            if ( count < 2 ) count = 2;

            //printf( "count = %-d\n", count );
            xPvData[i] = (void *) new char[xPvSize[i]*(count+10)];

          }

        }

        if ( traceType[i] == XYGC_K_TRACE_XY ) { // sanity check

          stat = ca_add_array_event( ca_field_type(xPv[i]), xPvCount[i],
           xPv[i], xValueUpdate, (void *) argRec, 0.0, 0.0, 0.0, &xEv[i] );
          if ( stat != ECA_NORMAL ) {
            printf( "error from ca_add_array_event\n" );
          }

	}

      }

      arrayHead[i] = 0;
      arrayTail[i] = 0;
      arrayNumPoints[i] = 0;
      curNpts[i] = 0;

    }

    init = 1;

  }

  // this needs to come before nbs, nxresc, ny1resc, ny2resc
  if ( nvu ) {

    anyRescale = 0;
    for ( i=0; i<numTraces; i++ ) {

      if ( yPvCount[i] > 1 ) {

        if ( traceType[i] == XYGC_K_TRACE_CHRONOLOGICAL ) {

          if ( yArrayNeedUpdate[i] ) {

            xArrayNeedUpdate[i] = 1;

            genChronoVector( i, &doRescale );
            if ( doRescale ) anyRescale = 1;

	  }

	}
        else {

          if ( yArrayNeedUpdate[i] || xArrayNeedUpdate ) {

            switch ( plotUpdateMode[i] ) {

            case XYGC_K_UPDATE_ON_X_OR_Y:
              if ( yArrayGotValue[i] || xArrayGotValue[i] ) {
                genXyVector( i, &doRescale );
                if ( doRescale ) anyRescale = 1;
              }
	      break;

            case XYGC_K_UPDATE_ON_X_AND_Y:
              if ( yArrayGotValue[i] && xArrayGotValue[i] ) {
                genXyVector( i, &doRescale );
                if ( doRescale ) anyRescale = 1;
              }
	      break;

            case XYGC_K_UPDATE_ON_X:
              if ( xArrayGotValue[i] ) {
                genXyVector( i, &doRescale );
                if ( doRescale ) anyRescale = 1;
              }
	      break;

            case XYGC_K_UPDATE_ON_Y:
              if ( yArrayGotValue[i] ) {
                genXyVector( i, &doRescale );
                if ( doRescale ) anyRescale = 1;
              }
	      break;

	    }

	  }

	}

      }

    }

    if ( anyRescale ) {
      regenBuffer();
      fullRefresh();
    }
    else {
      nu = 1;
    }

  }

  if ( nbrescl ) {

    //printf( "box rescale\n" );
    //printf( "rescale to (%-g,%-g) (%-g,%-g)\n", boxXMin, boxYMin,
    // boxXMax, boxYMax );

    curXMin = boxXMin;
    curY1Min = boxYMin;
    curXMax = boxXMax;
    curY1Max = boxYMax;

    if ( curXMin >= curXMax ) {
      if ( xAxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) {
        curXMax = curXMin * 10.0;
      }
      else if ( xAxisStyle == XYGC_K_AXIS_STYLE_TIME_LOG10 ) {
        curXMax = curXMin * 10.0;
      }
      else {
        curXMax = curXMin * 2.0;
      }
    }
    if ( curXMin >= curXMax ) { // in case xMin is 0
      curXMax = curXMin + 1.0;
    }

    if ( curY1Min >= curY1Max ) {
      if ( y1AxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) {
        curY1Max = curY1Min * 10.0;
      }
      else {
        curY1Max = curY1Min * 2.0;
      }
    }
    if ( curY1Min >= curY1Max ) { // in case y1Min is 0
      curY1Max = curY1Min + 1.0;
    }

    if ( xAxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) {
      get_log10_scale_params1( curXMin, curXMax, &curXMin, &curXMax,
       &curXNumLabelTicks, &curXMajorsPerLabel, &curXMinorsPerMajor, format );
    }
    else if ( xAxisStyle == XYGC_K_AXIS_STYLE_TIME_LOG10 ) {
      get_log10_scale_params1( curXMin, curXMax, &curXMin, &curXMax,
       &curXNumLabelTicks, &curXMajorsPerLabel, &curXMinorsPerMajor, format );
    }
    else {
      get_scale_params1( curXMin, curXMax, &curXMin, &curXMax,
       &curXNumLabelTicks, &curXMajorsPerLabel, &curXMinorsPerMajor, format );
    }

    if ( y1AxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) {
      get_log10_scale_params1( curY1Min, curY1Max, &curY1Min, &curY1Max,
       &curY1NumLabelTicks, &curY1MajorsPerLabel, &curY1MinorsPerMajor,
       format );
    }
    else {
      get_scale_params1( curY1Min, curY1Max, &curY1Min, &curY1Max,
       &curY1NumLabelTicks, &curY1MajorsPerLabel, &curY1MinorsPerMajor,
       format );
    }

    updateDimensions();

    for ( i=0; i<numTraces; i++ ) {

      xFactor[i] =
       (double) ( plotAreaW ) / ( curXMax - curXMin );
      xOffset[i] = plotAreaX;

      y1Factor[i] =
       (double) ( plotAreaH ) / ( curY1Max - curY1Min );
      y1Offset[i] = plotAreaY;

    }

    regenBuffer();
    fullRefresh();

  }

  if ( nbs ) {

    //for ( i=0; i<numTraces; i++ ) {
    //  yArrayNeedUpdate[i] = xArrayNeedUpdate[i] = 1;
    //}

    //eraseActive();

    for ( i=0; i<numTraces; i++ ) {

      yArrayNeedUpdate[i] = xArrayNeedUpdate[i] = 1;

      if ( needThisbufScroll[i] ) {

        needThisbufScroll[i] = 0;

        initPlotInfo( i );
        yArrayNeedUpdate[i] = xArrayNeedUpdate[i] = 1;
        yArrayGotValue[i] = xArrayGotValue[i] =  0;
        //arrayNumPoints[i] = curNpts[i] = 0;
        arrayNumPoints[i] = 0;
        plotState[i] = XYGC_K_STATE_INITIALIZING;


        // we don't have to worry about head passing tail in the following
        // two blocks

        if ( arrayTail[i] != arrayHead[i] ) {

          ii = arrayHead[i] + bufferScrollSize;
          if ( ii > plotBufSize[i] ) {
            ii = ii - plotBufSize[i] - 1;
	  }

          arrayHead[i] = ii;

          // ii = arrayHead[i];

          do {

            switch ( yPvType[i] ) {
            case DBR_FLOAT:
              dyValue = (double) ( (float *) yPvData[i] )[ii];
              break;
            case DBR_DOUBLE: 
              dyValue = ( (double *) yPvData[i] )[ii];
              break;
            case DBR_SHORT:
              dyValue = (double) ( (unsigned short *) yPvData[i] )[ii];
              break;
            case DBR_CHAR:
              dyValue = (double) ( (unsigned char *) yPvData[i] )[ii];
              break;
            case DBR_LONG:
              dyValue = (double) ( (int *) yPvData[i] )[ii];
              break;
            case DBR_ENUM:
              dyValue = (double) ( (unsigned short *) yPvData[i] )[ii];
              break;
            default:
              dyValue = ( (double *) yPvData[i] )[ii];
              break;
            }

            if ( y1AxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) {
              if ( dyValue > 0 ) dyValue = log10( dyValue );
	    }

            scaledY = (short) plotAreaH -
             (short) rint( ( dyValue - curY1Min ) *
             y1Factor[i] - y1Offset[i] );

            switch ( xPvType[i] ) {
            case DBR_FLOAT:
              dxValue = (double) ( (float *) xPvData[i] )[ii];
              break;
            case DBR_DOUBLE: 
              dxValue = ( (double *) xPvData[i] )[ii];
              break;
            case DBR_SHORT:
              dxValue = (double) ( (unsigned short *) xPvData[i] )[ii];
              break;
            case DBR_CHAR:
              dxValue = (double) ( (unsigned char *) xPvData[i] )[ii];
              break;
            case DBR_LONG:
              dxValue = (double) ( (int *) xPvData[i] )[ii];
              break;
            case DBR_ENUM:
              dxValue = (double) ( (unsigned short *) xPvData[i] )[ii];
              break;
            default:
              dxValue = ( (double *) xPvData[i] )[ii];
              break;
            }

            if ( xAxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) {
              if ( dxValue > 0 ) dxValue = log10( dxValue );
	    }
            else if ( xAxisStyle == XYGC_K_AXIS_STYLE_TIME_LOG10 ) {
              if ( dxValue > 0 ) dxValue = log10( dxValue );
	    }

            scaledX = (short) rint( ( dxValue - curXMin ) *
             xFactor[i] + xOffset[i] );

            addPoint( dxValue, scaledX, scaledY, i );

            ii++;
            if ( ii > plotBufSize[i] ) {
              ii = 0;
            }

          } while ( ii != arrayTail[i] );

        }

        // altDrawActiveOne( i );

      }

    }

    //eraseActive();
    //drawActive();

    nu = 1;

  }

  doRescale = 0;
  if ( nxrescl ) {

    if ( ( xAxisStyle == XYGC_K_AXIS_STYLE_STRIPCHART ) &&
         ( kpXMaxEfDouble.isNull() ) &&
         ( kpXMinEfDouble.isNull() ) ) {

      timeInc = ( curXMax - curXMin ) / (double) curXNumLabelTicks;
      firstSample = 1;
      for ( i=0; i<numTraces; i++ ) {

      if ( traceType[i] == XYGC_K_TRACE_CHRONOLOGICAL ) {

      if ( arrayTail[i] != arrayHead[i] ) {

        ii = arrayHead[i];
        if ( firstSample ) {
          firstSample = 0;
          max = ( (double *) xPvData[i] )[ii];
	}
        do {

          dxValue = ( (double *) xPvData[i] )[ii];
          if ( dxValue > max ) max = dxValue;

          ii++;
          if ( ii > plotBufSize[i] ) {
            ii = 0;
          }

        } while ( ii != arrayTail[i] );

      }

      }

      }

      timeInc = max - curXMax + timeInc;
      if ( timeInc < 0 ) timeInc = 0;
      curSec += (int) timeInc;

      for ( i=0; i<numTraces; i++ ) {

      if ( traceType[i] == XYGC_K_TRACE_CHRONOLOGICAL ) {

      if ( arrayTail[i] != arrayHead[i] ) {

        ii = arrayHead[i];
        do {

          dxValue = ( (double *) xPvData[i] )[ii];
          dxValue -= timeInc;
          ( (double *) xPvData[i] )[ii] = dxValue;

          ii++;
          if ( ii > plotBufSize[i] ) {
            ii = 0;
          }

        } while ( ii != arrayTail[i] );

        doRescale = 1;

      }

      }

      }

    }
    else {

    if ( xRescaleValue < curXMin ) {
      range = curXMax - xRescaleValue;
      curXMin = xRescaleValue - 0.1 * range;
    }
    else if ( xRescaleValue > curXMax ) {
      range = xRescaleValue - curXMin;
      curXMax = xRescaleValue + 0.1 * range;
    }

    if ( !kpXMinEfDouble.isNull() ) {
      curXMin = kpXMinEfDouble.value();
      if ( ( xAxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) ||
           ( xAxisStyle == XYGC_K_AXIS_STYLE_TIME_LOG10 ) ) {
        if ( curXMin > 0 ) {
          curXMin = log10(curXMin);
	}
        else {
          curXMin = 0;
	}
      }
    }

    if ( !kpXMaxEfDouble.isNull() ) {
      //printf( "user xmax = %-g\n", kpXMaxEfDouble.value() );
      curXMax = kpXMaxEfDouble.value();
      if ( ( xAxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) ||
           ( xAxisStyle == XYGC_K_AXIS_STYLE_TIME_LOG10 ) ) {
        if ( curXMax > 0 ) {
          curXMax = log10(curXMax);
	}
        else {
          curXMax = 0;
	}
      }
    }

    if ( kpCancelMinX ) {
      kpCancelMinX = 0;
      if ( xAxisSource == XYGC_K_FROM_PV ) {
        curXMin = dbXMin[0];
        if ( xAxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) {
          if ( curXMin > 0 ) curXMin = log10( curXMin );
        }
        else if ( xAxisStyle == XYGC_K_AXIS_STYLE_TIME_LOG10 ) {
          if ( curXMin > 0 ) curXMin = log10( curXMin );
        }
      }
      else {
        curXMin = xRescaleValue;
      }
    }

    if ( kpCancelMaxX ) {
      kpCancelMaxX = 0;
      if ( xAxisSource == XYGC_K_FROM_PV ) {
        curXMax = dbXMax[0];
        if ( xAxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) {
          if ( curXMax > 0 ) curXMax = log10( curXMax );
        }
        else if ( xAxisStyle == XYGC_K_AXIS_STYLE_TIME_LOG10 ) {
          if ( curXMax > 0 ) curXMax = log10( curXMax );
        }
      }
      else {
        curXMax = xRescaleValue;
      }
    }

    if ( curXMin >= curXMax ) {
      if ( xAxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) {
        curXMax = curXMin * 10.0;
      }
      else if ( xAxisStyle == XYGC_K_AXIS_STYLE_TIME_LOG10 ) {
        curXMax = curXMin * 10.0;
      }
      else {
        curXMax = curXMin * 2.0;
      }
    }
    if ( curXMin >= curXMax ) { // in case xMin is 0
      curXMax = curXMin + 1.0;
    }

    if ( xAxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) {
      get_log10_scale_params1( curXMin, curXMax, &curXMin, &curXMax,
       &curXNumLabelTicks, &curXMajorsPerLabel, &curXMinorsPerMajor, format );
    }
    else if ( xAxisStyle == XYGC_K_AXIS_STYLE_TIME_LOG10 ) {
      get_log10_scale_params1( curXMin, curXMax, &curXMin, &curXMax,
       &curXNumLabelTicks, &curXMajorsPerLabel, &curXMinorsPerMajor, format );
    }
    else {
      get_scale_params1( curXMin, curXMax, &curXMin, &curXMax,
       &curXNumLabelTicks, &curXMajorsPerLabel, &curXMinorsPerMajor, format );
    }

    stripChartXMax = curXMax;

    updateDimensions();

    for ( i=0; i<numTraces; i++ ) {
      xFactor[i] =
       (double) ( plotAreaW ) / ( curXMax - curXMin );
      xOffset[i] = plotAreaX;
    }

    doRescale = 1;

    }

    if ( ( xAxisStyle == XYGC_K_AXIS_STYLE_STRIPCHART ) &&
         ( xAxisSource == XYGC_K_AUTOSCALE ) ) {

      kpXMinEfDouble.setNull(1);
      kpXMaxEfDouble.setNull(1);

    }

  }

  if ( ny1rescl ) {

    //printf( "need y1 rescale, y = %-g\n", y1RescaleValue );
    //printf( "need y1 rescale, curY1Min = %-g\n", curY1Min );
    //printf( "need y1 rescale, curY1Max = %-g\n", curY1Max );

    if ( y1RescaleValue < curY1Min ) {
      range = curY1Max - y1RescaleValue;
      curY1Min = y1RescaleValue - 0.1 * range;
    }
    else if ( y1RescaleValue > curY1Max ) {
      range = y1RescaleValue - curY1Min;
      curY1Max = y1RescaleValue + 0.1 * range;
    }

    if ( !kpY1MinEfDouble.isNull() ) {
      curY1Min = kpY1MinEfDouble.value();
      if ( y1AxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) {
        if ( curY1Min > 0 ) {
          curY1Min = log10(curY1Min);
	}
        else {
          curY1Min = 0;
	}
      }
    }

    if ( !kpY1MaxEfDouble.isNull() ) {
      curY1Max = kpY1MaxEfDouble.value();
      if ( y1AxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) {
        if ( curY1Max > 0 ) {
          curY1Max = log10(curY1Max);
	}
        else {
          curY1Max = 0;
	}
      }
    }

    if ( kpCancelMinY1 ) {
      kpCancelMinY1 = 0;
      if ( y1AxisSource == XYGC_K_FROM_PV ) {
        curY1Min = dbYMin[0];
        if ( y1AxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) {
          if ( curY1Min > 0 ) curY1Min = log10( curY1Min );
        }
      }
      else {
        curY1Min = y1RescaleValue;
      }
    }

    if ( kpCancelMaxY1 ) {
      kpCancelMaxY1 = 0;
      if ( y1AxisSource == XYGC_K_FROM_PV ) {
        curY1Max = dbYMax[0];
        if ( y1AxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) {
          if ( curY1Max > 0 ) curY1Max = log10( curY1Max );
        }
      }
      else {
        curY1Max = y1RescaleValue;
      }
    }

    if ( curY1Min >= curY1Max ) {
      if ( y1AxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) {
        curY1Max = curY1Min * 10.0;
      }
      else {
        curY1Max = curY1Min * 2.0;
      }
    }
    if ( curY1Min >= curY1Max ) { // in case y1Min is 0
      curY1Max = curY1Min + 1.0;
    }

    if ( y1AxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) {
      get_log10_scale_params1( curY1Min, curY1Max, &curY1Min, &curY1Max,
       &curY1NumLabelTicks, &curY1MajorsPerLabel, &curY1MinorsPerMajor,
       format );
    }
    else {
      get_scale_params1( curY1Min, curY1Max, &curY1Min, &curY1Max,
       &curY1NumLabelTicks, &curY1MajorsPerLabel, &curY1MinorsPerMajor,
       format );
    }

    updateDimensions();

    for ( i=0; i<numTraces; i++ ) {
      y1Factor[i] =
       (double) ( plotAreaH ) / ( curY1Max - curY1Min );
      y1Offset[i] = plotAreaY;
    }

    doRescale = 1;

  }

  if ( ny2rescl ) {

    //printf( "need y2 rescale, y = %-g\n", y2RescaleValue );

    if ( y2RescaleValue < curY2Min ) {
      range = curY2Max - y2RescaleValue;
      curY2Min = y2RescaleValue - 0.1 * range;
    }
    else if ( y2RescaleValue > curY2Max ) {
      range = y2RescaleValue - curY2Min;
      curY2Max = y2RescaleValue + 0.1 * range;
    }

#if 0
    if ( !kpYwMinEfDouble.isNull() ) {
      //printf( "user y2min = %-g\n", kpY2MinEfDouble.value() );
      curY2Min = kpY2MinEfDouble.value();
      if ( y2AxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) {
        if ( curY2Min > 0 ) {
          curY2Min = log10(curY2Min);
	}
        else {
          curY2Min = 0;
	}
      }
    }

    if ( !kpY2MaxEfDouble.isNull() ) {
      //printf( "user y2max = %-g\n", kpY2MaxEfDouble.value() );
      curY2Max = kpY2MaxEfDouble.value();
      if ( y2AxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) {
        if ( curY2Max > 0 ) {
          curY2Max = log10(curY2Max);
	}
        else {
          curY2Max = 0;
	}
      }
    }

    if ( kpCancelMinY2 ) {
      kpCancelMinY2 = 0;
      curY2Min = y2RescaleValue;
    }

    if ( kpCancelMaxY2 ) {
      kpCancelMaxY2 = 0;
      curY2Max = y2RescaleValue;
    }
#endif

    if ( curY2Min >= curY2Max ) {
      if ( y2AxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) {
        curY2Max = curY2Min * 10.0;
      }
      else {
        curY2Max = curY2Min * 2.0;
      }
    }
    if ( curY2Min >= curY2Max ) { // in case y2Min is 0
      curY2Max = curY2Min + 1.0;
    }

    if ( y2AxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) {
      get_log10_scale_params1( curY2Min, curY2Max, &curY2Min, &curY2Max,
       &curY2NumLabelTicks, &curY2MajorsPerLabel, &curY2MinorsPerMajor,
       format );
    }
    else {
      get_scale_params1( curY2Min, curY2Max, &curY2Min, &curY2Max,
       &curY2NumLabelTicks, &curY2MajorsPerLabel, &curY2MinorsPerMajor,
       format );
    }

    updateDimensions();

    for ( i=0; i<numTraces; i++ ) {
      y2Factor[i] =
       (double) ( plotAreaH ) / ( curY2Max - curY2Min );
      y2Offset[i] = plotAreaY;
    }

    doRescale = 1;

  }

  if ( doRescale ) {

    regenBuffer();
    fullRefresh();

  }

  if ( nu ) {

    if ( updateTimerValue <= 0 ) {
      nru = 1; // fall through to next if block
    }
    else {
      if ( !updateTimerActive ) {
        updateTimer = appAddTimeOut( actWin->appCtx->appContext(),
         updateTimerValue, updateTimerAction, this );
        updateTimerActive = 1;
      }
    }

  }

  if ( nru ) {

    //printf( "need real update\n" );
    eraseActive();
    drawActive();

  }

  if ( nr ) {
    fullRefresh();
  }


  if ( nrst ) {

    firstTimeSample = 1;
    for ( i=0; i<numTraces; i++ ) {
      initPlotInfo( i );
      yArrayNeedUpdate[i] = xArrayNeedUpdate[i] = 0;
      yArrayGotValue[i] = xArrayGotValue[i] =  0;
      arrayHead[i] = arrayTail[i] = arrayNumPoints[i] =
       curNpts[i] = totalCount[i] = 0;
      plotState[i] = XYGC_K_STATE_INITIALIZING;
    }
    fullRefresh();

  }

}

void xyGraphClass::initPlotInfo (
  int trace
) {

int i;

 if ( !plotInfo[trace] ) return;

  // scope mode
  plotInfoHead[trace] = 0;
  plotInfoTail[trace] = 0;

  // plot mode
  for ( i=0; i<=plotInfoSize[trace]+1; i++ ) {
    plotInfo[trace][i].n = 0;
  }

}

void xyGraphClass::addPoint (
  double x,
  short scaledX,
  short scaledY,
  int trace
) {

  //printf( "xyGraphClass::addPoint, scaledX=%-d, scaledY=%-d\n",
  // scaledX, scaledY );

  //printf( "plotInfoHead=%-d, plotInfoTail=%-d\n",
  // plotInfoHead[trace], plotInfoTail[trace] );

  //printf( "plotInfoSize[trace] = %-d\n", plotInfoSize[trace] );
  //printf( "plotBufSize[trace] = %-d\n", plotBufSize[trace] );

int i;

  if ( !plotInfo[trace] ) return;

  if ( opMode[trace] == XYGC_K_SCOPE_MODE ) {

    i = plotInfoTail[trace];

    plotInfo[trace][i].firstX = scaledX;
    plotInfo[trace][i].firstY = scaledY;

    i++;
    if ( i >= plotBufSize[trace] ) {// use plotBufSize here
      i = 0;
    }
    plotInfoTail[trace] = i;
    if ( plotInfoHead[trace] == i ) {
      plotInfoHead[trace]++;
      if ( plotInfoHead[trace] >= plotBufSize[trace] ) { // use plotBufSize
        plotInfoHead[trace] = 0;
      }
    }
  
    arrayNumPoints[trace]++;

  }
  else { // plot sorted by x

    //printf( "xyGraphClass::addPoint, plot sorted by x\n" );

    if ( ( scaledX < plotAreaX ) || ( scaledX > plotInfoSize[trace] ) ) {
      return;
    }

    //printf( "xyGraphClass::addPoint, continue\n" );

    if ( plotInfo[trace][scaledX].n == 0 ) {

      plotInfo[trace][scaledX].firstDX = x;
      plotInfo[trace][scaledX].firstX = scaledX;
      plotInfo[trace][scaledX].firstY = scaledY;
      plotInfo[trace][scaledX].n = 1;

    }
    else if ( plotInfo[trace][scaledX].n == 1 ) {

      if ( scaledY < plotInfo[trace][scaledX].firstY ) {
        plotInfo[trace][scaledX].minY = scaledY;
        plotInfo[trace][scaledX].maxY = plotInfo[trace][scaledX].firstY;
      }
      else {
        plotInfo[trace][scaledX].minY = plotInfo[trace][scaledX].firstY;
        plotInfo[trace][scaledX].maxY = scaledY;
      }

      if ( x < plotInfo[trace][scaledX].firstDX ) {

        plotInfo[trace][scaledX].lastDX = plotInfo[trace][scaledX].firstDX;
        plotInfo[trace][scaledX].lastX = plotInfo[trace][scaledX].firstX;
        plotInfo[trace][scaledX].lastY = plotInfo[trace][scaledX].firstY;

        plotInfo[trace][scaledX].firstDX = x;
        plotInfo[trace][scaledX].firstX = scaledX;
        plotInfo[trace][scaledX].firstY = scaledY;

      }
      else {

        plotInfo[trace][scaledX].lastDX = x;
        plotInfo[trace][scaledX].lastX = scaledX;
        plotInfo[trace][scaledX].lastY = scaledY;

      }

      plotInfo[trace][scaledX].n = 2;

    }
    else {

      if ( scaledY < plotInfo[trace][scaledX].minY ) {
        plotInfo[trace][scaledX].minY = scaledY;
      }
      else if ( scaledY >= plotInfo[trace][scaledX].maxY ) {
        plotInfo[trace][scaledX].maxY = scaledY;
      }

      if ( x < plotInfo[trace][scaledX].firstDX ) {
        plotInfo[trace][scaledX].firstDX = x;
        plotInfo[trace][scaledX].firstX = scaledX;
        plotInfo[trace][scaledX].firstY = scaledY;
      }
      else if ( x >= plotInfo[trace][scaledX].lastDX ) {
        plotInfo[trace][scaledX].lastDX = x;
        plotInfo[trace][scaledX].lastX = scaledX;
        plotInfo[trace][scaledX].lastY = scaledY;
      }

      (plotInfo[trace][scaledX].n)++;

    }

    arrayNumPoints[trace]++;

    //printf( "arrayNumPoints = %-d\n", arrayNumPoints[trace] );

  }

}

int xyGraphClass::fillPlotArray (
  int trace
) {

int i, npts;
short curX, curY, prevX, prevY;

  //printf( "fillPlotArray, plotInfoHead=%-d, plotInfoTail=%-d\n",
  // plotInfoHead[trace], plotInfoTail[trace] );

  npts = 0;

  if ( opMode[trace] == XYGC_K_SCOPE_MODE ) {

    if ( plotInfoHead[trace] == plotInfoTail[trace] ) return npts;

    if ( plotStyle[trace] == XYGC_K_PLOT_STYLE_NEEDLE ) {

      i = plotInfoHead[trace];
      if ( i != plotInfoTail[trace] ) {

        prevX = plotInfo[trace][i].firstX;
        prevY = plotInfo[trace][i].firstY;

        plotBuf[trace][npts].x = prevX;
        plotBuf[trace][npts].y = plotAreaY+plotAreaH;
        npts++;

        plotBuf[trace][npts].x = prevX;
        plotBuf[trace][npts].y = prevY;
        npts++;

        plotBuf[trace][npts].x = prevX;
        plotBuf[trace][npts].y = plotAreaY+plotAreaH;
        npts++;

      }

      i++;
      if ( i >= plotBufSize[trace] ) { // use plotBufSize here
        i = 0;
      }

      while ( i != plotInfoTail[trace] ) {

        curX = plotInfo[trace][i].firstX;
        curY = plotInfo[trace][i].firstY;

        if ( ( curX != prevX ) || ( curY != prevY ) ) {

          prevX = curX;
          prevY = curY;

          plotBuf[trace][npts].x = curX;
          plotBuf[trace][npts].y = plotAreaY+plotAreaH;
          npts++;

          plotBuf[trace][npts].x = curX;
          plotBuf[trace][npts].y = curY;
          npts++;

          plotBuf[trace][npts].x = curX;
          plotBuf[trace][npts].y = plotAreaY+plotAreaH;
          npts++;

        }

        i++;
        if ( i >= plotBufSize[trace] ) { // use plotBufSize here
          i = 0;
        }

      }

    }
    else {

      i = plotInfoHead[trace];
      if ( i != plotInfoTail[trace] ) {
        prevX = plotInfo[trace][i].firstX;
        prevY = plotInfo[trace][i].firstY;
        plotBuf[trace][npts].x = prevX;
        plotBuf[trace][npts].y = prevY;
        npts++;
      }

      i++;
      if ( i >= plotBufSize[trace] ) { // use plotBufSize here
        i = 0;
      }

      while ( i != plotInfoTail[trace] ) {

        curX = plotInfo[trace][i].firstX;
        curY = plotInfo[trace][i].firstY;

        if ( ( curX != prevX ) || ( curY != prevY ) ) {
          prevX = curX;
          plotBuf[trace][npts].x = curX;
          prevY = curY;
          plotBuf[trace][npts].y = curY;
          npts++;
        }

        i++;
        if ( i >= plotBufSize[trace] ) { // use plotBufSize here
          i = 0;
        }

      }

    }

  }
  else { // plot sorted by x

    //printf( "fillPlotArray, plot sorted by x\n" );

    if ( plotStyle[trace] == XYGC_K_PLOT_STYLE_NEEDLE ) {

      npts = 0;
      for ( i=plotAreaX; i<=plotInfoSize[trace]; i++ ) {

        if ( plotInfo[trace][i].n == 1 ) {

          plotBuf[trace][npts].x = plotInfo[trace][i].firstX;
          plotBuf[trace][npts].y = plotAreaY+plotAreaH;
          npts++;

          plotBuf[trace][npts].x = plotInfo[trace][i].firstX;
          plotBuf[trace][npts].y = plotInfo[trace][i].firstY;
          npts++;

          plotBuf[trace][npts].x = plotInfo[trace][i].firstX;
          plotBuf[trace][npts].y = plotAreaY+plotAreaH;
          npts++;

        }
        else if ( plotInfo[trace][i].n == 2 ) {

          plotBuf[trace][npts].x = plotInfo[trace][i].firstX;
          plotBuf[trace][npts].y = plotAreaY+plotAreaH;
          npts++;

          plotBuf[trace][npts].x = plotInfo[trace][i].firstX;
          plotBuf[trace][npts].y = plotInfo[trace][i].firstY;
          npts++;

          plotBuf[trace][npts].x = plotInfo[trace][i].lastX;
          plotBuf[trace][npts].y = plotInfo[trace][i].lastY;
          npts++;

          plotBuf[trace][npts].x = plotInfo[trace][i].lastX;
          plotBuf[trace][npts].y = plotAreaY+plotAreaH;
          npts++;

        }
        else if ( plotInfo[trace][i].n > 2 ) {

          plotBuf[trace][npts].x = plotInfo[trace][i].firstX;
          plotBuf[trace][npts].y = plotAreaY+plotAreaH;
          npts++;

          plotBuf[trace][npts].x = plotInfo[trace][i].firstX;
          plotBuf[trace][npts].y = plotInfo[trace][i].maxY;
          npts++;

          plotBuf[trace][npts].x = plotInfo[trace][i].firstX;
          plotBuf[trace][npts].y = plotAreaY+plotAreaH;
          npts++;

        }

      }

    }
    else {

      npts = 0;
      for ( i=plotAreaX; i<=plotInfoSize[trace]; i++ ) {

        if ( plotInfo[trace][i].n == 1 ) {

          plotBuf[trace][npts].x = plotInfo[trace][i].firstX;
          plotBuf[trace][npts].y = plotInfo[trace][i].firstY;
          npts++;

        }
        else if ( plotInfo[trace][i].n == 2 ) {

          plotBuf[trace][npts].x = plotInfo[trace][i].firstX;
          plotBuf[trace][npts].y = plotInfo[trace][i].firstY;
          npts++;

          plotBuf[trace][npts].x = plotInfo[trace][i].lastX;
          plotBuf[trace][npts].y = plotInfo[trace][i].lastY;
          npts++;

        }
        else if ( plotInfo[trace][i].n > 2 ) {

          plotBuf[trace][npts].x = plotInfo[trace][i].firstX;
          plotBuf[trace][npts].y = plotInfo[trace][i].firstY;
          npts++;

          plotBuf[trace][npts].x = plotInfo[trace][i].firstX;
          plotBuf[trace][npts].y = plotInfo[trace][i].maxY;
          npts++;

          plotBuf[trace][npts].x = plotInfo[trace][i].firstX;
          plotBuf[trace][npts].y = plotInfo[trace][i].minY;
          npts++;

          plotBuf[trace][npts].x = plotInfo[trace][i].lastX;
          plotBuf[trace][npts].y = plotInfo[trace][i].lastY;
          npts++;

        }

      }

    }

    //printf( "npts=%-d\n", npts );

  }

  return npts;

}

void xyGraphClass::drawBorder ( void ) {

  actWin->executeGc.saveFg();
  actWin->executeGc.setLineWidth(1);
  actWin->executeGc.setLineStyle( LineSolid );
  actWin->executeGc.setFG( actWin->ci->pix(fgColor) );

  XDrawRectangle( actWin->d, pixmap,
   actWin->executeGc.normGC(), 0, 0, w, h );

  actWin->executeGc.restoreFg();

}

void xyGraphClass::drawXScale ( void ) {

  if ( ( xAxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) ||
       ( xAxisStyle == XYGC_K_AXIS_STYLE_TIME_LOG10 ) ) {

    drawXLog10Scale ( actWin->d, pixmap, &actWin->executeGc, xAxis,
     plotAreaX, plotAreaY+plotAreaH, plotAreaW,
     curXMin, curXMax,
     curXNumLabelTicks, curXMajorsPerLabel, curXMinorsPerMajor,
     actWin->ci->pix(fgColor), actWin->executeGc.getBaseBG(), xLabelGrid,
     xMajorGrid, xMinorGrid, plotAreaH, actWin->ci->pix(gridColor),
     actWin->fi, fontTag, fs, 1,
     !kpXMinEfDouble.isNull(), !kpXMaxEfDouble.isNull(),
     0 );

    if ( xAxis ) {
      getXLog10LimitCoords( plotAreaX, plotAreaY+plotAreaH, plotAreaW,
       curXMin, curXMax,
       curXNumLabelTicks, fontTag, fs,
       &xMinX0, &xMinX1, &xMinY0, &xMinY1,
       &xMaxX0, &xMaxX1, &xMaxY0, &xMaxY1 );
    }
    else {
      xMinX0 = xMinX1 = xMinY0 = xMinY1 = 0;
      xMaxX0 = xMaxX1 = xMaxY0 = xMaxY1 = -1;
    }

  }
  else {

    drawXLinearScale ( actWin->d, pixmap, &actWin->executeGc, xAxis,
     plotAreaX, plotAreaY+plotAreaH, plotAreaW,
     curXMin, curXMax,
     curXNumLabelTicks, curXMajorsPerLabel, curXMinorsPerMajor,
     actWin->ci->pix(fgColor), actWin->executeGc.getBaseBG(), xLabelGrid,
     xMajorGrid, xMinorGrid, plotAreaH, actWin->ci->pix(gridColor),
     actWin->fi, fontTag, fs, 1,
     !kpXMinEfDouble.isNull(), !kpXMaxEfDouble.isNull(),
     0 );

    if ( xAxis ) {
      getXLimitCoords( plotAreaX, plotAreaY+plotAreaH, plotAreaW,
       curXMin, curXMax,
       curXNumLabelTicks, fontTag, fs,
       &xMinX0, &xMinX1, &xMinY0, &xMinY1,
       &xMaxX0, &xMaxX1, &xMaxY0, &xMaxY1 );
    }
    else {
      xMinX0 = xMinX1 = xMinY0 = xMinY1 = 0;
      xMaxX0 = xMaxX1 = xMaxY0 = xMaxY1 = -1;
    }

  }

}

void xyGraphClass::drawY1Scale ( void ) {

  if ( y1AxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) {

    drawYLog10Scale ( actWin->d, pixmap, &actWin->executeGc, y1Axis,
     plotAreaX, plotAreaY+plotAreaH, plotAreaH,
     curY1Min, curY1Max,
     curY1NumLabelTicks, curY1MajorsPerLabel, curY1MinorsPerMajor,
     actWin->ci->pix(fgColor), actWin->executeGc.getBaseBG(), y1LabelGrid,
     y1MajorGrid, y1MinorGrid, plotAreaW, actWin->ci->pix(gridColor),
     actWin->fi, fontTag, fs, 1,
     !kpY1MinEfDouble.isNull(), !kpY1MaxEfDouble.isNull(),
     0 );

    if ( y1Axis ) {
      getYLog10LimitCoords( plotAreaX, plotAreaY+plotAreaH, plotAreaH,
       curY1Min, curY1Max,
       curY1NumLabelTicks, fontTag, fs,
       &y1MinX0, &y1MinX1, &y1MinY0, &y1MinY1,
       &y1MaxX0, &y1MaxX1, &y1MaxY0, &y1MaxY1 );
    }
    else {
      y1MinX0 = y1MinX1 = y1MinY0 = y1MinY1 = 0;
      y1MaxX0 = y1MaxX1 = y1MaxY0 = y1MaxY1 = -1;
    }

  }
  else {

    drawYLinearScale ( actWin->d, pixmap, &actWin->executeGc, y1Axis,
     plotAreaX, plotAreaY+plotAreaH, plotAreaH,
     curY1Min, curY1Max,
     curY1NumLabelTicks, curY1MajorsPerLabel, curY1MinorsPerMajor,
     actWin->ci->pix(fgColor), actWin->executeGc.getBaseBG(), y1LabelGrid,
     y1MajorGrid, y1MinorGrid, plotAreaW, actWin->ci->pix(gridColor),
     actWin->fi, fontTag, fs, 1,
     !kpY1MinEfDouble.isNull(), !kpY1MaxEfDouble.isNull(),
     0 );

    if ( y1Axis ) {
      getYLimitCoords( plotAreaX, plotAreaY+plotAreaH, plotAreaH,
       curY1Min, curY1Max,
       curY1NumLabelTicks, fontTag, fs,
       &y1MinX0, &y1MinX1, &y1MinY0, &y1MinY1,
       &y1MaxX0, &y1MaxX1, &y1MaxY0, &y1MaxY1 );
    }
    else {
      y1MinX0 = y1MinX1 = y1MinY0 = y1MinY1 = 0;
      y1MaxX0 = y1MaxX1 = y1MaxY0 = y1MaxY1 = -1;
    }
  }

}

void xyGraphClass::drawGrid ( void ) {

  if ( xLabelGrid || xMajorGrid || xMinorGrid ) {

  if ( ( xAxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) ||
       ( xAxisStyle == XYGC_K_AXIS_STYLE_TIME_LOG10 ) ) {

      drawXLog10Scale ( actWin->d, pixmap, &actWin->executeGc, xAxis,
       plotAreaX, plotAreaY+plotAreaH, plotAreaW,
       curXMin, curXMax,
       curXNumLabelTicks, curXMajorsPerLabel, curXMinorsPerMajor,
       actWin->ci->pix(fgColor), actWin->executeGc.getBaseBG(), xLabelGrid,
       xMajorGrid, xMinorGrid, plotAreaH, actWin->ci->pix(gridColor),
       actWin->fi, fontTag, fs, 1,
       !kpXMinEfDouble.isNull(), !kpXMaxEfDouble.isNull(),
       0 );

    }
    else {

      drawXLinearScale ( actWin->d, pixmap, &actWin->executeGc, xAxis,
       plotAreaX, plotAreaY+plotAreaH, plotAreaW,
       curXMin, curXMax,
       curXNumLabelTicks, curXMajorsPerLabel, curXMinorsPerMajor,
       actWin->ci->pix(fgColor), actWin->executeGc.getBaseBG(), xLabelGrid,
       xMajorGrid, xMinorGrid, plotAreaH, actWin->ci->pix(gridColor),
       actWin->fi, fontTag, fs, 1,
       !kpXMinEfDouble.isNull(), !kpXMaxEfDouble.isNull(),
       0 );

    }

  }

  if ( y1LabelGrid || y1MajorGrid || y1MinorGrid ) {

    if ( y1AxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) {

      drawYLog10Scale ( actWin->d, pixmap, &actWin->executeGc, y1Axis,
       plotAreaX, plotAreaY+plotAreaH, plotAreaH,
       curY1Min, curY1Max,
       curY1NumLabelTicks, curY1MajorsPerLabel, curY1MinorsPerMajor,
       actWin->ci->pix(fgColor), actWin->executeGc.getBaseBG(), y1LabelGrid,
       y1MajorGrid, y1MinorGrid, plotAreaW, actWin->ci->pix(gridColor),
       actWin->fi, fontTag, fs, 1,
       !kpY1MinEfDouble.isNull(), !kpY1MaxEfDouble.isNull(),
       0 );

    }
    else {

      drawYLinearScale ( actWin->d, pixmap, &actWin->executeGc, y1Axis,
       plotAreaX, plotAreaY+plotAreaH, plotAreaH,
       curY1Min, curY1Max,
       curY1NumLabelTicks, curY1MajorsPerLabel, curY1MinorsPerMajor,
       actWin->ci->pix(fgColor), actWin->executeGc.getBaseBG(), y1LabelGrid,
       y1MajorGrid, y1MinorGrid, plotAreaW, actWin->ci->pix(gridColor),
       actWin->fi, fontTag, fs, 1,
       !kpY1MinEfDouble.isNull(), !kpY1MaxEfDouble.isNull(),
       0 );

    }

  }

}

void xyGraphClass::drawTitle ( void ) {

int tX, tY;

  if ( !blank( graphTitle.getExpanded() ) ) {

    tX = plotAreaX + plotAreaW / 2;
    tY = fontHeight / 2;

    actWin->executeGc.saveFg();
    actWin->executeGc.setFG( actWin->ci->pix(fgColor) );
    actWin->executeGc.setFontTag( fontTag, actWin->fi );

    xDrawText( actWin->d, pixmap,
     &actWin->executeGc, fs, tX, tY, XmALIGNMENT_CENTER,
     graphTitle.getExpanded() );

    actWin->executeGc.restoreFg();

  }

}

void xyGraphClass::drawXlabel ( void ) {

int lX, lY;

  if ( !blank( xLabel.getExpanded() ) ) {

    lX = plotAreaX + plotAreaW / 2;
    lY = h - fontHeight * 3 / 2;

    actWin->executeGc.saveFg();
    actWin->executeGc.setFG( actWin->ci->pix(fgColor) );
    actWin->executeGc.setFontTag( fontTag, actWin->fi );

    xDrawText( actWin->d, pixmap,
     &actWin->executeGc, fs, lX, lY, XmALIGNMENT_CENTER,
     xLabel.getExpanded() );

    actWin->executeGc.restoreFg();

  }

}

void xyGraphClass::drawYlabel ( void ) {

unsigned int i;
int lX, lY, lW, inc, stat;
char fullName[127+1], label[127+1];

  if ( !blank( yLabel.getExpanded() ) ) {

    strncpy( label, yLabel.getExpanded(), 127 );
    label[127] = 0;

    lX = fontHeight;
    lY = h - fontHeight * 3 / 2;
    lW = XTextWidth( fs, label, strlen(label) );
    lY = plotAreaY + ( plotAreaH + lW ) / 2;

    actWin->executeGc.saveFg();
    actWin->executeGc.setFG( actWin->ci->pix(fgColor) );

    stat = actWin->fi->getFontName( fontTag, 90.0, fullName, 127 );
    actWin->executeGc.setNativeFont( fullName, actWin->fi );

    for ( i=0; i<strlen(label); i++ ) {

      XDrawString( actWin->d, pixmap,
       actWin->executeGc.normGC(), lX, lY, &label[i], 1 );

      inc = XTextWidth( fs, &label[i], 1 );
      lY -= inc;

    }

    actWin->executeGc.restoreFg();

  }

}

#ifdef __cplusplus
extern "C" {
#endif

void *create_xyGraphClassPtr ( void ) {

xyGraphClass *ptr;

  ptr = new xyGraphClass;
  return (void *) ptr;

}

void *clone_xyGraphClassPtr (
  void *_srcPtr )
{

xyGraphClass *ptr, *srcPtr;

  srcPtr = (xyGraphClass *) _srcPtr;

  ptr = new xyGraphClass( srcPtr );

  return (void *) ptr;

}

#ifdef __cplusplus
}
#endif
