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
#include "math.h"

void _edmDebug ( void );

static double loc_log10 ( double d ) {

  if ( d == 0 ) return 0;
  return log10( fabs( d ) );

}

static void setVal (
  int pvtype,
  void *dest,
  int i,
  double src
) {

  switch ( pvtype ) {

  case ProcessVariable::specificType::flt:
    ( (float *) dest )[i] = (float) src;
    break;

  case ProcessVariable::specificType::real: 
    ( (double *) dest )[i] = (double) src;
    break;

  case ProcessVariable::specificType::shrt:
    ( (short *) dest )[i] = (short) src;
    break;

  case ProcessVariable::specificType::chr:
    ( (char *) dest )[i] = (char) src;
    break;

  case ProcessVariable::specificType::integer:
    ( (int *) dest )[i] = (int) src;
    break;

  case ProcessVariable::specificType::enumerated:
    ( (short *) dest )[i] = (short) src;
    break;

  default:
    ( (double *) dest )[i] = (double) src;
    break;

  }

}

static double dclamp (
  double val
) {

  if ( val < -16000 ) return -16000;
  if ( val >  16000 ) return  16000;
  return val;

}

static short sclamp (
  short val
) {

  if ( val < -16000 ) return -16000;
  if ( val >  16000 ) return  16000;
  return val;

}

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

static void updateAutoScaleTimerAction (
  XtPointer client,
  XtIntervalId *id )
{

xyGraphClass *xyo = (xyGraphClass *) client;

  if ( !xyo->updateAutoScaleTimerActive ) {
    xyo->updateAutoScaleTimer = 0;
    return;
  }

  xyo->updateAutoScaleTimer = appAddTimeOut(
   xyo->actWin->appCtx->appContext(),
   xyo->updateAutoScaleTimerValue, updateAutoScaleTimerAction, client );

  xyo->actWin->appCtx->proc->lock();
  xyo->needAutoScaleUpdate = 1;
  xyo->actWin->addDefExeNode( xyo->aglPtr );
  xyo->actWin->appCtx->proc->unlock();

}

static void adjp_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call )
{

xyGraphClass *xyo = (xyGraphClass *) client;

  // x
  if ( !xyo->eBuf->bufXMin.isNull() ) {
    xyo->kpXMin = xyo->eBuf->bufXMin.value();
    setKpXMinDoubleValue( w, client, call );
  }

  if ( !xyo->eBuf->bufXMax.isNull() ) {
    xyo->kpXMax = xyo->eBuf->bufXMax.value();
    setKpXMaxDoubleValue( w, client, call );
  }

  // y1
  if ( !xyo->eBuf->bufY1Min[0].isNull() ) {
    xyo->kpY1Min[0] = xyo->eBuf->bufY1Min[0].value();
    setKpY1MinDoubleValue( w, client, call );
  }

  if ( !xyo->eBuf->bufY1Max[0].isNull() ) {
    xyo->kpY1Max[0] = xyo->eBuf->bufY1Max[0].value();
    setKpY1MaxDoubleValue( w, client, call );
  }

  // y2
  if ( !xyo->eBuf->bufY1Min[1].isNull() ) {
    xyo->kpY1Min[1] = xyo->eBuf->bufY1Min[1].value();
    setKpY2MinDoubleValue( w, client, call );
  }

  if ( !xyo->eBuf->bufY1Max[1].isNull() ) {
    xyo->kpY1Max[1] = xyo->eBuf->bufY1Max[1].value();
    setKpY2MaxDoubleValue( w, client, call );
  }

}

static void adjp_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call )
{

xyGraphClass *xyo = (xyGraphClass *) client;

  adjp_edit_apply( w, client, call );
  xyo->ef.popdown();

}

static void adjp_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call )
{

xyGraphClass *xyo = (xyGraphClass *) client;

  xyo->ef.popdown();

}

static void dump_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call )
{

xyGraphClass *xyo = (xyGraphClass *) client;
int i, index, n, size, h, t;
double dxValue, dyValue;
char fname[255+1], *envPtr;
FILE *tmp;

time_t curSeconds;
edmTime base( (const unsigned long) ( xyo->curSec ),
 (const unsigned long) xyo->curNsec );

  curSeconds = base.getSec() + xyo->timeOffset;

  // try EDMDUMPFILES, then EDMTMPFILES, else use /tmp
  envPtr = getenv( environment_str14 );
  if ( envPtr ) {

    strncpy( fname, envPtr, 255 );
    if ( envPtr[strlen(envPtr)] != '/' ) {
      Strncat( fname, "/", 255 );
    }

  }
  else {

    envPtr = getenv( environment_str8 );
    if ( envPtr ) {

      strncpy( fname, envPtr, 255 );
      if ( envPtr[strlen(envPtr)] != '/' ) {
        Strncat( fname, "/", 255 );
      }

    }
    else {

      strncpy( fname, "/tmp/", 255 );

    }

  }

  Strncat( fname, xyo->dumpFileName, 255 );

  if ( xyo->actWin->fileExists( fname ) ) {
    xyo->actWin->appCtx->postMessage( "File exists - not overwritten" );
    xyo->actWin->appCtx->raiseMessageWindow();
    return;
  }

  tmp = fopen( fname, "w" );

  if ( debugMode() ) {
    fprintf( stderr, "dumpFileName = [%s]\n", fname );
  }

  if ( !tmp ) {

    xyo->actWin->appCtx->postMessage( "File open failure" );
    xyo->actWin->appCtx->raiseMessageWindow();
    return;

  }

  fprintf( tmp, "number of traces = %-d\n", xyo->numTraces );

  for ( i=0; i<xyo->numTraces; i++ ) {

    if ( xyo->traceType[i] == XYGC_K_TRACE_CHRONOLOGICAL ) {

      // vector
      if ( (int) xyo->yPv[i]->get_dimension() > 1 ) {

        fprintf( tmp, "trace %-d (chronological) size = %-d\n",
                 i, (int) xyo->yPvEffCount(i) );

        fprintf( tmp, "index, %s\n", xyo->yPvExpStr[i].getExpanded() );

        for ( n=0; n<xyo->yPvEffCount(i); n++ ) {

          switch ( xyo->yPvType[i] ) {
          case ProcessVariable::specificType::flt:
            dyValue = (double) ( (float *) xyo->yPvData[i] )[n];
            break;
          case ProcessVariable::specificType::real: 
            dyValue = ( (double *) xyo->yPvData[i] )[n];
            break;
          case ProcessVariable::specificType::shrt:
            if (  xyo->ySigned[i] ) {
              dyValue = (double) ( (short *) xyo->yPvData[i] )[n];
            }
            else {
              dyValue = (double) ( (unsigned short *) xyo->yPvData[i] )[n];
            }
            break;
          case ProcessVariable::specificType::chr:
            if (  xyo->ySigned[i] ) {
              dyValue = (double) ( (char *) xyo->yPvData[i] )[n];
            }
            else {
              dyValue = (double) ( (unsigned char *) xyo->yPvData[i] )[n];
            }
            break;
          case ProcessVariable::specificType::integer:
            if (  xyo->ySigned[i] ) {
              dyValue = (double) ( (int *) xyo->yPvData[i] )[n];
            }
            else {
              dyValue = (double) ( (unsigned int *) xyo->yPvData[i] )[n];
            }
            break;
          case ProcessVariable::specificType::enumerated:
            if (  xyo->ySigned[i] ) {
              dyValue = (double) ( (short *) xyo->yPvData[i] )[n];
            }
            else {
              dyValue = (double) ( (unsigned short *) xyo->yPvData[i] )[n];
            }
            break;
          default:
            dyValue = ( (double *) xyo->yPvData[i] )[n];
            break;
          }

          fprintf( tmp, "%-d, %-g\n", n, dyValue );

        }

      }
      else {

	// scalar
	h = xyo->arrayHead[i];
        t = xyo->arrayTail[i];
        n = h;
	index = 0;

	if ( t >= h ) {
	  size = t - h;
	}
	else {
	  size = xyo->plotBufSize[i] - h + t;
	}

        fprintf( tmp, "trace %-d (chronological) size = %-d\n",
         i, size );

        fprintf( tmp, "index, %s\n", xyo->yPvExpStr[i].getExpanded() );

	while ( n != t ) {

          if ( ( xyo->xAxisStyle == XYGC_K_AXIS_STYLE_TIME ) ||
               ( xyo->xAxisStyle == XYGC_K_AXIS_STYLE_TIME_LOG10 ) ) {

            if ( xyo->xAxisTimeFormat == XYGC_K_AXIS_TIME_FMT_SEC ) {
              // for chronological plot, x type is double
              dxValue = ( (double *) xyo->xPvData[i] )[n];
	    }
	    else {
              dxValue = ( (double *) xyo->xPvData[i] )[n] + (double) curSeconds;
	    }

	  }
	  else if ( xyo->xAxisStyle == XYGC_K_AXIS_STYLE_LINEAR ) {

            dxValue = (double) index;

	  }
	  else {

            dxValue = (double) index;

	  }

          switch ( xyo->yPvType[i] ) {
          case ProcessVariable::specificType::flt:
            dyValue = (double) ( (float *) xyo->yPvData[i] )[n];
            break;
          case ProcessVariable::specificType::real: 
            dyValue = ( (double *) xyo->yPvData[i] )[n];
            break;
          case ProcessVariable::specificType::shrt:
            if (  xyo->ySigned[i] ) {
              dyValue = (double) ( (short *) xyo->yPvData[i] )[n];
            }
            else {
              dyValue = (double) ( (unsigned short *) xyo->yPvData[i] )[n];
            }
            break;
          case ProcessVariable::specificType::chr:
            if (  xyo->ySigned[i] ) {
              dyValue = (double) ( (char *) xyo->yPvData[i] )[n];
            }
            else {
              dyValue = (double) ( (unsigned char *) xyo->yPvData[i] )[n];
            }
            break;
          case ProcessVariable::specificType::integer:
            if (  xyo->ySigned[i] ) {
              dyValue = (double) ( (int *) xyo->yPvData[i] )[n];
            }
            else {
              dyValue = (double) ( (unsigned int *) xyo->yPvData[i] )[n];
            }
            break;
          case ProcessVariable::specificType::enumerated:
            if (  xyo->ySigned[i] ) {
              dyValue = (double) ( (short *) xyo->yPvData[i] )[n];
            }
            else {
              dyValue = (double) ( (unsigned short *) xyo->yPvData[i] )[n];
            }
            break;
          default:
            dyValue = ( (double *) xyo->yPvData[i] )[n];
            break;
          }

          fprintf( tmp, "%-19.13g, %-g\n", dxValue, dyValue );

	  index++;
	  n++;
          if ( n > xyo->plotBufSize[i] ) {
	    n = 0;
	  }

	}

      }

    }
    else {

      // vector
      if ( ( (int) xyo->yPv[i]->get_dimension() > 1 ) &&
           ( (int) xyo->xPv[i]->get_dimension() > 1 ) ) {

        fprintf( tmp, "trace %-d (x/y) x size = %-d, y size = %-d\n",
                 i, xyo->xPvEffCount(i), xyo->yPvEffCount(i) );

        fprintf( tmp, "%s, %s\n", xyo->xPvExpStr[i].getExpanded(),
         xyo->yPvExpStr[i].getExpanded() );

        size = xyo->yPvEffCount(i);
        if ( size > xyo->xPvEffCount(i) ) {
          size = xyo->xPvEffCount(i);
        }

        for ( n=0; n<size; n++ ) {

          // x
          switch ( xyo->xPvType[i] ) {
          case ProcessVariable::specificType::flt:
            dxValue = (double) ( (float *) xyo->xPvData[i] )[n];
            break;
          case ProcessVariable::specificType::real: 
            dxValue = ( (double *) xyo->xPvData[i] )[n];
            break;
          case ProcessVariable::specificType::shrt:
            if (  xyo->xSigned[i] ) {
              dxValue = (double) ( (short *) xyo->xPvData[i] )[n];
            }
            else {
              dxValue = (double) ( (unsigned short *) xyo->xPvData[i] )[n];
            }
            break;
          case ProcessVariable::specificType::chr:
            if (  xyo->xSigned[i] ) {
              dxValue = (double) ( (char *) xyo->xPvData[i] )[n];
            }
            else {
              dxValue = (double) ( (unsigned char *) xyo->xPvData[i] )[n];
            }
            break;
          case ProcessVariable::specificType::integer:
            if (  xyo->xSigned[i] ) {
              dxValue = (double) ( (int *) xyo->xPvData[i] )[n];
            }
            else {
              dxValue = (double) ( (unsigned int *) xyo->xPvData[i] )[n];
            }
            break;
          case ProcessVariable::specificType::enumerated:
            if (  xyo->xSigned[i] ) {
              dxValue = (double) ( (short *) xyo->xPvData[i] )[n];
            }
            else {
              dxValue = (double) ( (unsigned short *) xyo->xPvData[i] )[n];
            }
            break;
          default:
            dxValue = ( (double *) xyo->xPvData[i] )[n];
            break;
         }

          // y
          switch ( xyo->yPvType[i] ) {
          case ProcessVariable::specificType::flt:
            dyValue = (double) ( (float *) xyo->yPvData[i] )[n];
            break;
          case ProcessVariable::specificType::real: 
            dyValue = ( (double *) xyo->yPvData[i] )[n];
            break;
          case ProcessVariable::specificType::shrt:
            if (  xyo->ySigned[i] ) {
              dyValue = (double) ( (short *) xyo->yPvData[i] )[n];
            }
            else {
              dyValue = (double) ( (unsigned short *) xyo->yPvData[i] )[n];
            }
            break;
          case ProcessVariable::specificType::chr:
            if (  xyo->ySigned[i] ) {
              dyValue = (double) ( (char *) xyo->yPvData[i] )[n];
            }
            else {
              dyValue = (double) ( (unsigned char *) xyo->yPvData[i] )[n];
            }
            break;
          case ProcessVariable::specificType::integer:
            if (  xyo->ySigned[i] ) {
              dyValue = (double) ( (int *) xyo->yPvData[i] )[n];
            }
            else {
              dyValue = (double) ( (unsigned int *) xyo->yPvData[i] )[n];
            }
            break;
          case ProcessVariable::specificType::enumerated:
            if (  xyo->ySigned[i] ) {
              dyValue = (double) ( (short *) xyo->yPvData[i] )[n];
            }
            else {
              dyValue = (double) ( (unsigned short *) xyo->yPvData[i] )[n];
            }
            break;
          default:
            dyValue = ( (double *) xyo->yPvData[i] )[n];
            break;
          }

          fprintf( tmp, "%-g, %-g\n", dxValue, dyValue );

        }

      }
      else if ( ( (int) xyo->yPv[i]->get_dimension() == 1 ) &&
                ( (int) xyo->xPv[i]->get_dimension() == 1 ) ) {

	// scalar
	h = xyo->arrayHead[i];
        t = xyo->arrayTail[i];
        n = h;

	if ( t >= h ) {
	  size = t - h;
	}
	else {
	  size = xyo->plotBufSize[i] - h + t;
	}

        fprintf( tmp, "trace %-d (x/y) x size = %-d, y size = %-d\n",
	 i, size, size );

        fprintf( tmp, "%s, %s\n", xyo->xPvExpStr[i].getExpanded(),
         xyo->yPvExpStr[i].getExpanded() );

	while ( n != t ) {

          // x
          switch ( xyo->xPvType[i] ) {
          case ProcessVariable::specificType::flt:
            dxValue = (double) ( (float *) xyo->xPvData[i] )[n];
            break;
          case ProcessVariable::specificType::real: 
            dxValue = ( (double *) xyo->xPvData[i] )[n];
            break;
          case ProcessVariable::specificType::shrt:
            if (  xyo->xSigned[i] ) {
              dxValue = (double) ( (short *) xyo->xPvData[i] )[n];
            }
            else {
              dxValue = (double) ( (unsigned short *) xyo->xPvData[i] )[n];
            }
            break;
          case ProcessVariable::specificType::chr:
            if (  xyo->xSigned[i] ) {
              dxValue = (double) ( (char *) xyo->xPvData[i] )[n];
            }
            else {
              dxValue = (double) ( (unsigned char *) xyo->xPvData[i] )[n];
            }
            break;
          case ProcessVariable::specificType::integer:
            if (  xyo->xSigned[i] ) {
              dxValue = (double) ( (int *) xyo->xPvData[i] )[n];
            }
            else {
              dxValue = (double) ( (unsigned int *) xyo->xPvData[i] )[n];
            }
            break;
          case ProcessVariable::specificType::enumerated:
            if (  xyo->xSigned[i] ) {
              dxValue = (double) ( (short *) xyo->xPvData[i] )[n];
            }
            else {
              dxValue = (double) ( (unsigned short *) xyo->xPvData[i] )[n];
            }
            break;
          default:
            dxValue = ( (double *) xyo->xPvData[i] )[n];
            break;
         }

          // y
          switch ( xyo->yPvType[i] ) {
          case ProcessVariable::specificType::flt:
            dyValue = (double) ( (float *) xyo->yPvData[i] )[n];
            break;
          case ProcessVariable::specificType::real: 
            dyValue = ( (double *) xyo->yPvData[i] )[n];
            break;
          case ProcessVariable::specificType::shrt:
            if (  xyo->ySigned[i] ) {
              dyValue = (double) ( (short *) xyo->yPvData[i] )[n];
            }
            else {
              dyValue = (double) ( (unsigned short *) xyo->yPvData[i] )[n];
            }
            break;
          case ProcessVariable::specificType::chr:
            if (  xyo->ySigned[i] ) {
              dyValue = (double) ( (char *) xyo->yPvData[i] )[n];
            }
            else {
              dyValue = (double) ( (unsigned char *) xyo->yPvData[i] )[n];
            }
            break;
          case ProcessVariable::specificType::integer:
            if (  xyo->ySigned[i] ) {
              dyValue = (double) ( (int *) xyo->yPvData[i] )[n];
            }
            else {
              dyValue = (double) ( (unsigned int *) xyo->yPvData[i] )[n];
            }
            break;
          case ProcessVariable::specificType::enumerated:
            if (  xyo->ySigned[i] ) {
              dyValue = (double) ( (short *) xyo->yPvData[i] )[n];
            }
            else {
              dyValue = (double) ( (unsigned short *) xyo->yPvData[i] )[n];
            }
            break;
          default:
            dyValue = ( (double *) xyo->yPvData[i] )[n];
            break;
          }

          fprintf( tmp, "%-g, %-g\n", dxValue, dyValue );

	  n++;
          if ( n > xyo->plotBufSize[i] ) {
	    n = 0;
	  }

	}

      }
      else {

        fprintf( tmp, "Cannot dump x/y trace - mixed pv dimensions\n" );

      }

    }

  }

  fclose( tmp );

}

static void dump_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call )
{

xyGraphClass *xyo = (xyGraphClass *) client;

  dump_edit_apply( w, client, call );
  xyo->efDump.popdown();

}

static void dump_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call )
{

xyGraphClass *xyo = (xyGraphClass *) client;

  xyo->efDump.popdown();

}

static void menu_cb (
  Widget w,
  XtPointer client,
  XtPointer call )
{

xyGraphClass *xyo = (xyGraphClass *) client;
int yi;

  if ( w == xyo->pbAutoScale ) {

    for ( yi=0; yi<xyGraphClass::NUM_Y_AXES; yi++ ) {
      if ( xyo->numYTraces[yi] > 0 ) {
        xyo->kpY1MinEfDouble[yi].setNull(1);
        xyo->kpY1MaxEfDouble[yi].setNull(1);
      }
    }
    xyo->kpXMinEfDouble.setNull(1);
    xyo->kpXMaxEfDouble.setNull(1);

    xyo->actWin->appCtx->proc->lock();
    xyo->needNewLimits = 1;
    xyo->actWin->addDefExeNode( xyo->aglPtr );
    xyo->actWin->appCtx->proc->unlock();

  }
  else if ( w == xyo->pbOrigScale ) {

    for ( yi=0; yi<xyGraphClass::NUM_Y_AXES; yi++ ) {
      if ( xyo->numYTraces[yi] > 0 ) {
        xyo->kpY1MinEfDouble[yi].setNull(1);
        xyo->kpY1MaxEfDouble[yi].setNull(1);
      }
    }
    xyo->kpXMinEfDouble.setNull(1);
    xyo->kpXMaxEfDouble.setNull(1);

    xyo->actWin->appCtx->proc->lock();
    xyo->needOriginalLimits = 1;
    xyo->actWin->addDefExeNode( xyo->aglPtr );
    xyo->actWin->appCtx->proc->unlock();

  }
  else if ( w == xyo->pbAdjustParams ) {

    if ( !xyo->eBuf ) {
      xyo->eBuf = new xyGraphClass::editBufType;
    }

    if ( !xyo->ef.formIsPoppedUp() ) {

      xyo->adjpFormX = xyo->popupMenuX;
      xyo->adjpFormY = xyo->popupMenuY;
      xyo->adjpFormW = 0;
      xyo->adjpFormH = 0;
      xyo->adjpFormMaxH = 600;

      xyo->ef.create( xyo->actWin->top,
       xyo->actWin->appCtx->ci.getColorMap(),
       &xyo->adjpFormX, &xyo->adjpFormY,
       &xyo->adjpFormW, &xyo->adjpFormH, &xyo->adjpFormMaxH,
       "Adjust Params", NULL, NULL, NULL );

      xyo->eBuf->bufXMin.setNull(1);
      xyo->eBuf->bufXMax.setNull(1);
      xyo->eBuf->bufY1Min[0].setNull(1);
      xyo->eBuf->bufY1Max[0].setNull(1);
      xyo->eBuf->bufY1Min[1].setNull(1);
      xyo->eBuf->bufY1Max[1].setNull(1);

      if ( ( xyo->xAxisStyle != XYGC_K_AXIS_STYLE_TIME ) ||
           ( xyo->xAxisTimeFormat == XYGC_K_AXIS_TIME_FMT_SEC ) ) {

        xyo->ef.addTextField( "X Min", 10, &xyo->eBuf->bufXMin );
        xyo->ef.addTextField( "X Max", 10, &xyo->eBuf->bufXMax );

      }

      xyo->ef.addTextField( "Y1 Min", 10, &xyo->eBuf->bufY1Min[0] );

      xyo->ef.addTextField( "Y1 Max", 10, &xyo->eBuf->bufY1Max[0] );

      xyo->ef.addTextField( "Y2 Min", 10, &xyo->eBuf->bufY1Min[1] );

      xyo->ef.addTextField( "Y2 Max", 10, &xyo->eBuf->bufY1Max[1] );

      xyo->ef.finished( adjp_edit_ok, adjp_edit_apply, adjp_edit_cancel,
       xyo );

      xyo->ef.popup();

    }

  }
  else if ( w == xyo->pbClearPlot ) {

    xyo->actWin->appCtx->proc->lock();
    xyo->needReset = 1;
    xyo->actWin->addDefExeNode( xyo->aglPtr );
    xyo->actWin->appCtx->proc->unlock();

  }
  else if ( w == xyo->pbDumpData ) {

    if ( !xyo->eBuf ) {
      xyo->eBuf = new xyGraphClass::editBufType;
    }

    xyo->dumpFormX = xyo->popupMenuX;
    xyo->dumpFormY = xyo->popupMenuY;
    xyo->dumpFormW = 0;
    xyo->dumpFormH = 0;
    xyo->dumpFormMaxH = 600;

    xyo->efDump.create( xyo->actWin->top,
     xyo->actWin->appCtx->ci.getColorMap(),
     &xyo->dumpFormX, &xyo->dumpFormY,
     &xyo->dumpFormW, &xyo->dumpFormH, &xyo->dumpFormMaxH,
     "Dump Data", NULL, NULL, NULL );

    xyo->efDump.addTextField( "File", 35, xyo->dumpFileName, 255 );

    xyo->efDump.finished( dump_edit_ok, dump_edit_apply, dump_edit_cancel,
     xyo );

    xyo->efDump.popup();

  }

}

static void setKpXMinDoubleValue (
  Widget w,
  XtPointer client,
  XtPointer call ) {

xyGraphClass *xyo = (xyGraphClass *) client;

  xyo->actWin->appCtx->proc->lock();

  if ( ( xyo->xAxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) ||
       ( xyo->xAxisStyle == XYGC_K_AXIS_STYLE_TIME_LOG10 ) ) {

    if ( xyo->kpXMin > 0 ) {

      xyo->xRescaleValue = loc_log10( xyo->kpXMin );

    }
    else {

      xyo->xRescaleValue = 0;

    }

  }
  else {

    if ( ( xyo->xAxisStyle == XYGC_K_AXIS_STYLE_TIME ) &&
       ( xyo->xAxisTimeFormat != XYGC_K_AXIS_TIME_FMT_SEC ) ) {

      xyo->xRescaleValue = xyo->curXMin + xyo->kpXMin;

    }
    else {

      xyo->xRescaleValue = xyo->kpXMin;

    }

  }

  xyo->kpXMinEfDouble.setValue( xyo->xRescaleValue );
  xyo->needXRescale = 1;
  xyo->actWin->addDefExeNode( xyo->aglPtr );
  xyo->actWin->appCtx->proc->unlock();

}

static void setKpXMaxDoubleValue (
  Widget w,
  XtPointer client,
  XtPointer call ) {

xyGraphClass *xyo = (xyGraphClass *) client;

  xyo->actWin->appCtx->proc->lock();

  if ( ( xyo->xAxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) ||
       ( xyo->xAxisStyle == XYGC_K_AXIS_STYLE_TIME_LOG10 ) ) {

    if ( xyo->kpXMax > 0 ) {

      xyo->xRescaleValue = loc_log10( xyo->kpXMax );

    }
    else {

      xyo->xRescaleValue = 0;

    }

  }
  else {

    if ( ( xyo->xAxisStyle == XYGC_K_AXIS_STYLE_TIME ) &&
       ( xyo->xAxisTimeFormat != XYGC_K_AXIS_TIME_FMT_SEC ) ) {

      xyo->xRescaleValue = xyo->curXMax + xyo->kpXMax;

    }
    else {

      xyo->xRescaleValue = xyo->kpXMax;

    }

  }

  xyo->kpXMaxEfDouble.setValue( xyo->xRescaleValue );
  xyo->needXRescale = 1;
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
int ctl;

  xyo->actWin->appCtx->proc->lock();

  xyo->kpXMinEfDouble.setNull(1);

  if ( xyo->xAxisSource == XYGC_K_USER_SPECIFIED ) {

    minValue = xyo->xMin.value();

    if ( xyo->xAxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) {
      if ( minValue <= 0 ) minValue = 1;
      minValue = loc_log10(  minValue  );
    }
    else if ( xyo->xAxisStyle == XYGC_K_AXIS_STYLE_TIME_LOG10 ) {
      if ( minValue <= 0 ) minValue = 1;
      minValue = loc_log10(  minValue  );
    }

  }
  else if ( xyo->xAxisSource == XYGC_K_FROM_PV ) {

    minValue = xyo->dbXMin[0];
    for ( i=1; i<xyo->numTraces; i++ ) {
      if ( xyo->dbXMin[i] < minValue ) minValue = xyo->dbXMin[i];
    }

    if ( xyo->xAxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) {
      if ( minValue <= 0 ) minValue = 1;
      minValue = loc_log10(  minValue  );
    }
    else if ( xyo->xAxisStyle == XYGC_K_AXIS_STYLE_TIME_LOG10 ) {
      if ( minValue <= 0 ) minValue = 1;
      minValue = loc_log10(  minValue  );
    }

  }
  else { // auto scale

    // find min x value

    minValue = 0.9 * xyo->curXMax;
    if ( minValue > xyo->curXMax ) minValue = 1.1 * xyo->curXMax;
    first = 1;
    for ( i=0; i<xyo->numTraces; i++ ) {

      ctl = (int) xyo->traceCtl & ( 1 << i );

      if ( !ctl ) {

      ii = xyo->arrayHead[i];
      while ( ii != xyo->arrayTail[i] ) {

        // There are two views of pv types, Type and specificType; this uses
        // specificType
        switch ( xyo->xPvType[i] ) {
        case ProcessVariable::specificType::flt:
          dxValue = (double) ( (float *) xyo->xPvData[i] )[ii];
          break;
        case ProcessVariable::specificType::real: 
          dxValue = ( (double *) xyo->xPvData[i] )[ii];
          break;
        case ProcessVariable::specificType::shrt:
          if ( xyo->xSigned[i] ) {
            dxValue = (double) ( (short *) xyo->xPvData[i] )[ii];
          }
          else {
            dxValue = (double) ( (unsigned short *) xyo->xPvData[i] )[ii];
          }
          break;
        case ProcessVariable::specificType::chr:
          if ( xyo->xSigned[i] ) {
            dxValue = (double) ( (char *) xyo->xPvData[i] )[ii];
          }
          else {
            dxValue = (double) ( (unsigned char *) xyo->xPvData[i] )[ii];
          }
          break;
        case ProcessVariable::specificType::integer:
          if ( xyo->xSigned[i] ) {
            dxValue = (double) ( (int *) xyo->xPvData[i] )[ii];
          }
          else {
            dxValue = (double) ( (unsigned int *) xyo->xPvData[i] )[ii];
          }
          break;
        case ProcessVariable::specificType::enumerated:
          if ( xyo->xSigned[i] ) {
            dxValue = (double) ( (short *) xyo->xPvData[i] )[ii];
          }
          else {
            dxValue = (double) ( (unsigned short *) xyo->xPvData[i] )[ii];
          }
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

    }

    if ( xyo->xAxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) {
      if ( minValue <= 0 ) minValue = 1;
      minValue = loc_log10(  minValue  );
    }
    else if ( xyo->xAxisStyle == XYGC_K_AXIS_STYLE_TIME_LOG10 ) {
      if ( minValue <= 0 ) minValue = 1;
      minValue = loc_log10(  minValue  );
    }

  }

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
int ctl;

  xyo->actWin->appCtx->proc->lock();

  xyo->kpXMaxEfDouble.setNull(1);

  if ( xyo->xAxisSource == XYGC_K_USER_SPECIFIED ) {

    maxValue = xyo->xMax.value();

    if ( xyo->xAxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) {
      if ( maxValue <= 0 ) maxValue = 1;
      maxValue = loc_log10(  maxValue  );
    }
    else if ( xyo->xAxisStyle == XYGC_K_AXIS_STYLE_TIME_LOG10 ) {
      if ( maxValue <= 0 ) maxValue = 1;
      maxValue = loc_log10(  maxValue  );
    }

  }
  else if ( xyo->xAxisSource == XYGC_K_FROM_PV ) {

    maxValue = xyo->dbXMax[0];
    for ( i=1; i<xyo->numTraces; i++ ) {
      if ( xyo->dbXMax[i] > maxValue ) maxValue = xyo->dbXMax[i];
    }

    if ( xyo->xAxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) {
      if ( maxValue <= 0 ) maxValue = 1;
      maxValue = loc_log10(  maxValue  );
    }
    else if ( xyo->xAxisStyle == XYGC_K_AXIS_STYLE_TIME_LOG10 ) {
      if ( maxValue <= 0 ) maxValue = 1;
      maxValue = loc_log10(  maxValue  );
    }

  }
  else { // auto scale

    // find max x value

    maxValue = 1.1 * xyo->curXMin;
    if ( maxValue < xyo->curXMin ) maxValue = 0.9 * xyo->curXMin;
    first = 1;
    for ( i=0; i<xyo->numTraces; i++ ) {

      ctl = (int) xyo->traceCtl & ( 1 << i );

      if ( !ctl ) {

      ii = xyo->arrayHead[i];
      while ( ii != xyo->arrayTail[i] ) {

        // There are two views of pv types, Type and specificType; this uses
        // specificType
        switch ( xyo->xPvType[i] ) {
        case ProcessVariable::specificType::flt:
          dxValue = (double) ( (float *) xyo->xPvData[i] )[ii];
          break;
        case ProcessVariable::specificType::real: 
          dxValue = ( (double *) xyo->xPvData[i] )[ii];
          break;
        case ProcessVariable::specificType::shrt:
          if ( xyo->xSigned[i] ) {
            dxValue = (double) ( (short *) xyo->xPvData[i] )[ii];
          }
          else {
            dxValue = (double) ( (unsigned short *) xyo->xPvData[i] )[ii];
          }
          break;
        case ProcessVariable::specificType::chr:
          if ( xyo->xSigned[i] ) {
            dxValue = (double) ( (char *) xyo->xPvData[i] )[ii];
          }
          else {
            dxValue = (double) ( (unsigned char *) xyo->xPvData[i] )[ii];
          }
          break;
        case ProcessVariable::specificType::integer:
          if ( xyo->xSigned[i] ) {
            dxValue = (double) ( (int *) xyo->xPvData[i] )[ii];
          }
          else {
            dxValue = (double) ( (unsigned int *) xyo->xPvData[i] )[ii];
          }
          break;
        case ProcessVariable::specificType::enumerated:
          if ( xyo->xSigned[i] ) {
            dxValue = (double) ( (short *) xyo->xPvData[i] )[ii];
          }
          else {
            dxValue = (double) ( (unsigned short *) xyo->xPvData[i] )[ii];
          }
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

    }

    if ( xyo->xAxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) {
      if ( maxValue <= 0 ) maxValue = 1;
      maxValue = loc_log10(  maxValue  );
    }
    else if ( xyo->xAxisStyle == XYGC_K_AXIS_STYLE_TIME_LOG10 ) {
      if ( maxValue <= 0 ) maxValue = 1;
      maxValue = loc_log10(  maxValue  );
    }

  }

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

  setKpYMinDoubleValue( w, client, call, 0 );

}

static void setKpY2MinDoubleValue (
  Widget w,
  XtPointer client,
  XtPointer call ) {

  setKpYMinDoubleValue( w, client, call, 1 );

}

static void setKpYMinDoubleValue (
  Widget w,
  XtPointer client,
  XtPointer call,
  int yIndex ) {

xyGraphClass *xyo = (xyGraphClass *) client;
int yi = yIndex;

  xyo->actWin->appCtx->proc->lock();
  xyo->kpY1MinEfDouble[yi].setValue( xyo->kpY1Min[yi] );
  xyo->needY1Rescale[yi] = 1;

  if ( xyo->y1AxisStyle[yi] == XYGC_K_AXIS_STYLE_LOG10 ) {
    if ( xyo->kpY1Min[yi] > 0 ) {
      xyo->y1RescaleValue[yi] = loc_log10( xyo->kpY1Min[yi] );
    }
    else {
      xyo->y1RescaleValue[yi] = 0;
    }
  }
  else {
    xyo->y1RescaleValue[yi] = xyo->kpY1Min[yi];
  }

  xyo->actWin->addDefExeNode( xyo->aglPtr );
  xyo->actWin->appCtx->proc->unlock();

}

static void setKpY1MaxDoubleValue (
  Widget w,
  XtPointer client,
  XtPointer call ) {

  setKpYMaxDoubleValue( w, client, call, 0 );

}

static void setKpY2MaxDoubleValue (
  Widget w,
  XtPointer client,
  XtPointer call ) {

  setKpYMaxDoubleValue( w, client, call, 1 );

}

static void setKpYMaxDoubleValue (
  Widget w,
  XtPointer client,
  XtPointer call,
  int yIndex ) {

xyGraphClass *xyo = (xyGraphClass *) client;
int yi = yIndex;

  xyo->actWin->appCtx->proc->lock();
  xyo->kpY1MaxEfDouble[yi].setValue( xyo->kpY1Max[yi] );
  xyo->needY1Rescale[yi] = 1;

  if ( xyo->y1AxisStyle[yi] == XYGC_K_AXIS_STYLE_LOG10 ) {
    if ( xyo->kpY1Max[yi] > 0 ) {
      xyo->y1RescaleValue[yi] = loc_log10( xyo->kpY1Max[yi] );
    }
    else {
      xyo->y1RescaleValue[yi] = 0;
    }
  }
  else {
    xyo->y1RescaleValue[yi] = xyo->kpY1Max[yi];
  }

  xyo->actWin->addDefExeNode( xyo->aglPtr );
  xyo->actWin->appCtx->proc->unlock();

}

static void cancelKpY1Min (
  Widget w,
  XtPointer client,
  XtPointer call
) {

  cancelKpYMin( w, client, call, 0 );

}

static void cancelKpY2Min (
  Widget w,
  XtPointer client,
  XtPointer call
) {

  cancelKpYMin( w, client, call, 1 );

}

static void cancelKpYMin (
  Widget w,
  XtPointer client,
  XtPointer call,
  int yIndex ) {

xyGraphClass *xyo = (xyGraphClass *) client;
int i, ii, first, yScaleIndex;
double dy1Value, minValue;
int yi = yIndex;
int ctl;

  xyo->actWin->appCtx->proc->lock();

  xyo->kpY1MinEfDouble[yi].setNull(1);

  if ( xyo->y1AxisSource[yi] == XYGC_K_USER_SPECIFIED ) {

    minValue = xyo->y1Min[yi].value();

    if ( xyo->y1AxisStyle[yi] == XYGC_K_AXIS_STYLE_LOG10 ) {
      if ( minValue <= 0 ) minValue = 1;
      minValue = loc_log10(  minValue  );
    }

  }
  else if ( xyo->y1AxisSource[yi] == XYGC_K_FROM_PV ) {

    minValue = xyo->dbYMin[xyo->lowestYScaleIndex[yi]];
    for ( i=1; i<xyo->numTraces; i++ ) {

      yScaleIndex = 0;
      if ( xyo->y2Scale[i] ) yScaleIndex = 1;

      if ( yScaleIndex == yi ) {
        if ( xyo->dbYMin[i] < minValue ) minValue = xyo->dbYMin[i];
      }

    }

    if ( xyo->y1AxisStyle[yi] == XYGC_K_AXIS_STYLE_LOG10 ) {
      if ( minValue <= 0 ) minValue = 1;
      minValue = loc_log10(  minValue  );
    }

  }
  else { // auto scale

    // find min y1 value

    minValue = 0.9 * xyo->curY1Max[yi];
    if ( minValue > xyo->curY1Max[yi] ) minValue = 1.1 * xyo->curY1Max[yi];
    first = 1;
    for ( i=0; i<xyo->numTraces; i++ ) {

      ctl = (int) xyo->traceCtl & ( 1 << i );

      if ( !ctl ) {

      yScaleIndex = 0;
      if ( xyo->y2Scale[i] ) yScaleIndex = 1;

      if ( yScaleIndex == yi ) {

        ii = xyo->arrayHead[i];
        while ( ii != xyo->arrayTail[i] ) {

        // There are two views of pv types, Type and specificType; this uses
        // specificType
          switch ( xyo->yPvType[i] ) {
          case ProcessVariable::specificType::flt:
            dy1Value = (double) ( (float *) xyo->yPvData[i] )[ii];
            break;
          case ProcessVariable::specificType::real: 
            dy1Value = ( (double *) xyo->yPvData[i] )[ii];
            break;
          case ProcessVariable::specificType::shrt:
            if ( xyo->ySigned[i] ) {
              dy1Value = (double) ( (short *) xyo->yPvData[i] )[ii];
            }
            else {
              dy1Value = (double) ( (unsigned short *) xyo->yPvData[i] )[ii];
            }
            break;
          case ProcessVariable::specificType::chr:
            if ( xyo->ySigned[i] ) {
              dy1Value = (double) ( (char *) xyo->yPvData[i] )[ii];
            }
            else {
              dy1Value = (double) ( (unsigned char *) xyo->yPvData[i] )[ii];
            }
            break;
          case ProcessVariable::specificType::integer:
            if ( xyo->ySigned[i] ) {
              dy1Value = (double) ( (int *) xyo->yPvData[i] )[ii];
            }
            else {
              dy1Value = (double) ( (unsigned int *) xyo->yPvData[i] )[ii];
            }
            break;
          case ProcessVariable::specificType::enumerated:
            if ( xyo->ySigned[i] ) {
              dy1Value = (double) ( (short *) xyo->yPvData[i] )[ii];
            }
            else {
              dy1Value = (double) ( (unsigned short *) xyo->yPvData[i] )[ii];
            }
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

      }

    }

    if ( xyo->y1AxisStyle[yi] == XYGC_K_AXIS_STYLE_LOG10 ) {
      if ( minValue <= 0 ) minValue = 1;
      minValue = loc_log10(  minValue  );
    }

  }

  xyo->needY1Rescale[yi] = 1;
  xyo->kpCancelMinY1[yi] = 1;
  xyo->y1RescaleValue[yi] = minValue;
  xyo->actWin->addDefExeNode( xyo->aglPtr );

  xyo->actWin->appCtx->proc->unlock();

}

static void cancelKpY1Max (
  Widget w,
  XtPointer client,
  XtPointer call
) {

  cancelKpYMax( w, client, call, 0 );

}

static void cancelKpY2Max (
  Widget w,
  XtPointer client,
  XtPointer call
) {

  cancelKpYMax( w, client, call, 1 );

}

static void cancelKpYMax (
  Widget w,
  XtPointer client,
  XtPointer call,
  int yIndex ) {

xyGraphClass *xyo = (xyGraphClass *) client;
int i, ii, first, yScaleIndex;
double dy1Value, maxValue;
int yi = yIndex;
int ctl;

  xyo->actWin->appCtx->proc->lock();

  xyo->kpY1MaxEfDouble[yi].setNull(1);

  if ( xyo->y1AxisSource[yi] == XYGC_K_USER_SPECIFIED ) {

    maxValue = xyo->y1Max[yi].value();

    if ( xyo->y1AxisStyle[yi] == XYGC_K_AXIS_STYLE_LOG10 ) {
      if ( maxValue <= 0 ) maxValue = 1;
      maxValue = loc_log10( maxValue );
    }

  }
  else if ( xyo->y1AxisSource[yi] == XYGC_K_FROM_PV ) {

    maxValue = xyo->dbYMax[xyo->lowestYScaleIndex[yi]];
    for ( i=1; i<xyo->numTraces; i++ ) {

      yScaleIndex = 0;
      if ( xyo->y2Scale[i] ) yScaleIndex = 1;

      if ( yScaleIndex == yi ) {
        if ( xyo->dbYMax[i] > maxValue ) maxValue = xyo->dbYMax[i];
      }

    }

    if ( xyo->y1AxisStyle[yi] == XYGC_K_AXIS_STYLE_LOG10 ) {
      if ( maxValue <= 0 ) maxValue = 1;
      maxValue = loc_log10(  maxValue  );
    }

  }
  else { // auto scale

    // find max y1 value

    maxValue = 1.1 * xyo->curY1Min[yi];
    if ( maxValue < xyo->curY1Min[yi] ) maxValue = 0.9 * xyo->curY1Min[yi];
    first = 1;
    for ( i=0; i<xyo->numTraces; i++ ) {

      ctl = (int) xyo->traceCtl & ( 1 << i );

      if ( !ctl ) {

      yScaleIndex = 0;
      if ( xyo->y2Scale[i] ) yScaleIndex = 1;

      if ( yScaleIndex == yi ) {

        ii = xyo->arrayHead[i];
        while ( ii != xyo->arrayTail[i] ) {

          // There are two views of pv types, Type and specificType; this uses
          // specificType
          switch ( xyo->yPvType[i] ) {
          case ProcessVariable::specificType::flt:
            dy1Value = (double) ( (float *) xyo->yPvData[i] )[ii];
            break;
          case ProcessVariable::specificType::real: 
            dy1Value = ( (double *) xyo->yPvData[i] )[ii];
            break;
          case ProcessVariable::specificType::shrt:
            if ( xyo->ySigned[i] ) {
              dy1Value = (double) ( (short *) xyo->yPvData[i] )[ii];
            }
            else {
              dy1Value = (double) ( (unsigned short *) xyo->yPvData[i] )[ii];
            }
            break;
          case ProcessVariable::specificType::chr:
            if ( xyo->ySigned[i] ) {
              dy1Value = (double) ( (char *) xyo->yPvData[i] )[ii];
            }
            else {
              dy1Value = (double) ( (unsigned char *) xyo->yPvData[i] )[ii];
            }
            break;
          case ProcessVariable::specificType::integer:
            if ( xyo->ySigned[i] ) {
              dy1Value = (double) ( (int *) xyo->yPvData[i] )[ii];
            }
            else {
              dy1Value = (double) ( (unsigned int *) xyo->yPvData[i] )[ii];
            }
            break;
          case ProcessVariable::specificType::enumerated:
            if ( xyo->ySigned[i] ) {
              dy1Value = (double) ( (short *) xyo->yPvData[i] )[ii];
            }
            else {
              dy1Value = (double) ( (unsigned short *) xyo->yPvData[i] )[ii];
            }
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

      }

    }

    if ( xyo->y1AxisStyle[yi] == XYGC_K_AXIS_STYLE_LOG10 ) {
      if ( maxValue <= 0 ) maxValue = 1;
      maxValue = loc_log10(  maxValue  );
    }

  }

  xyo->needY1Rescale[yi] = 1;
  xyo->kpCancelMaxY1[yi] = 1;
  xyo->y1RescaleValue[yi] = maxValue;
  xyo->actWin->addDefExeNode( xyo->aglPtr );

  xyo->actWin->appCtx->proc->unlock();

}

static void traceCtlMonitorConnection (
  ProcessVariable *pv,
  void *userarg )
{

xyGraphClass *xyo = (xyGraphClass *) userarg;

  if ( pv->is_valid() ) {

    xyo->actWin->appCtx->proc->lock();
    xyo->needTraceCtlConnect = 1;
    xyo->actWin->addDefExeNode( xyo->aglPtr );
    xyo->actWin->appCtx->proc->unlock();

  }

}

static void traceCtlValueUpdate (
  ProcessVariable *pv,
  void *userarg )
{

xyGraphClass *xyo = (xyGraphClass *) userarg;

  xyo->traceCtl = pv->get_int();

  xyo->actWin->appCtx->proc->lock();
  xyo->needTraceUpdate = 1;
  xyo->actWin->addDefExeNode( xyo->aglPtr );
  xyo->actWin->appCtx->proc->unlock();

}

static void resetMonitorConnection (
  ProcessVariable *pv,
  void *userarg )
{

xyGraphClass *xyo = (xyGraphClass *) userarg;

  if ( pv->is_valid() ) {

    xyo->actWin->appCtx->proc->lock();
    xyo->needResetConnect = 1;
    xyo->actWin->addDefExeNode( xyo->aglPtr );
    xyo->actWin->appCtx->proc->unlock();

  }

}

static void resetValueUpdate (
  ProcessVariable *pv,
  void *userarg )
{

xyGraphClass *xyo = (xyGraphClass *) userarg;
short value;

  value = pv->get_int();
  if ( (  value && ( xyo->resetMode == XYGC_K_RESET_MODE_IF_NOT_ZERO ) ) ||
       ( !value && ( xyo->resetMode == XYGC_K_RESET_MODE_IF_ZERO ) ) ) {

    xyo->actWin->appCtx->proc->lock();
    xyo->needReset = 1;
    xyo->actWin->addDefExeNode( xyo->aglPtr );
    xyo->actWin->appCtx->proc->unlock();

  }

}

static void trigMonitorConnection (
  ProcessVariable *pv,
  void *userarg )
{

xyGraphClass *xyo = (xyGraphClass *) userarg;

  if ( pv->is_valid() ) {

    xyo->actWin->appCtx->proc->lock();
    xyo->needTrigConnect = 1;
    xyo->actWin->addDefExeNode( xyo->aglPtr );
    xyo->actWin->appCtx->proc->unlock();

  }

}

static void trigValueUpdate (
  ProcessVariable *pv,
  void *userarg )
{

xyGraphClass *xyo = (xyGraphClass *) userarg;
int i, ii, yi;
double dxValue, dyValue;
double scaledX, scaledY;
double yZero;
int ctl;

  xyo->actWin->appCtx->proc->lock();

  for ( i=0; i<xyo->numTraces; i++ ) {

    ctl = (int) xyo->traceCtl & ( 1 << i );

    // make sure arrays have been allocated
    if ( !xyo->xPvData[i] || !xyo->yPvData[i] ) {
      xyo->actWin->appCtx->proc->unlock();
      return;
    }

    if ( xyo->plotUpdateMode[i] == XYGC_K_UPDATE_ON_TRIG ) {

      yi = 0;
      if ( xyo->y2Scale[i] ) yi = 1;

      switch ( xyo->traceType[i] ) {

      case XYGC_K_TRACE_XY:

        if ( xyo->forceVector[i] || ( xyo->xPv[i]->get_dimension() > 1 ) ) { // vector

          xyo->yArrayNeedUpdate[i] = xyo->xArrayNeedUpdate[i] = 1;
          xyo->needVectorUpdate = 1;
          xyo->actWin->addDefExeNode( xyo->aglPtr );

        }
        else { // scalar

          //if ( xyo->yArrayGotValue[i] && xyo->xArrayGotValue[i] ) {

          ii = xyo->arrayTail[i];

          // x
          setVal( xyo->xPvType[i], (void *) xyo->xPvData[i], ii, xyo->xPvCurValue[i] );

          // y
          setVal( xyo->yPvType[i], (void *) xyo->yPvData[i], ii, xyo->yPvCurValue[i] );

          // There are two views of pv types, Type and specificType; this uses
          // specificType
          switch ( xyo->yPvType[i] ) {
          case ProcessVariable::specificType::flt:
            dyValue = (double) ( (float *) xyo->yPvData[i] )[ii];
            break;
          case ProcessVariable::specificType::real: 
            dyValue = ( (double *) xyo->yPvData[i] )[ii];
            break;
          case ProcessVariable::specificType::shrt:
            if ( xyo->ySigned[i] ) {
              dyValue = (double) ( (short *) xyo->yPvData[i] )[ii];
            }
            else {
              dyValue = (double) ( (unsigned short *) xyo->yPvData[i] )[ii];
            }
            break;
          case ProcessVariable::specificType::chr:
            if ( xyo->ySigned[i] ) {
              dyValue = (double) ( (char *) xyo->yPvData[i] )[ii];
            }
            else {
              dyValue = (double) ( (unsigned char *) xyo->yPvData[i] )[ii];
            }
            break;
          case ProcessVariable::specificType::integer:
            if ( xyo->ySigned[i] ) {
              dyValue = (double) ( (int *) xyo->yPvData[i] )[ii];
            }
            else {
              dyValue = (double) ( (unsigned int *) xyo->yPvData[i] )[ii];
            }
            break;
          case ProcessVariable::specificType::enumerated:
            if ( xyo->ySigned[i] ) {
              dyValue = (double) ( (short *) xyo->yPvData[i] )[ii];
            }
            else {
              dyValue = (double) ( (unsigned short *) xyo->yPvData[i] )[ii];
            }
            break;
          default:
            dyValue = ( (double *) xyo->yPvData[i] )[ii];
            break;
          }

          if ( xyo->y1AxisStyle[yi] == XYGC_K_AXIS_STYLE_LOG10 ) {
            dyValue = loc_log10(  dyValue  );
          }

          if ( !ctl ) {
          if ( xyo->y1AxisSource[yi] == XYGC_K_AUTOSCALE ) {
            if ( xyo->kpY1MinEfDouble[yi].isNull() ) {
              if ( dyValue < xyo->curY1Min[yi] ) {
                xyo->needY1Rescale[yi] = 1;
                xyo->y1RescaleValue[yi] = dyValue;
                xyo->actWin->addDefExeNode( xyo->aglPtr );
              }
            }
            if ( xyo->kpY1MaxEfDouble[yi].isNull() ) {
              if ( dyValue > xyo->curY1Max[yi] ) {
                xyo->needY1Rescale[yi] = 1;
                xyo->y1RescaleValue[yi] = dyValue;
                xyo->actWin->addDefExeNode( xyo->aglPtr );
              }
            }
          }
	  }

          scaledY = xyo->plotAreaH -
           rint( ( dyValue - xyo->curY1Min[yi] ) *
           xyo->y1Factor[yi][i] - xyo->y1Offset[yi][i] );
          scaledY = dclamp( scaledY );


          if ( xyo->y1AxisStyle[yi] == XYGC_K_AXIS_STYLE_LOG10 ) {
            yZero = xyo->plotAreaH + xyo->y1Offset[yi][i];
          }
          else {
            yZero = xyo->plotAreaH -
             rint( ( 0.0 - xyo->curY1Min[yi] ) *
             xyo->y1Factor[yi][i] - xyo->y1Offset[yi][i] );
            yZero = dclamp( yZero );
          }

          // There are two views of pv types, Type and specificType; this uses
          // specificType
          switch ( xyo->xPvType[i] ) {
          case ProcessVariable::specificType::flt:
            dxValue = (double) ( (float *) xyo->xPvData[i] )[ii];
            break;
          case ProcessVariable::specificType::real: 
            dxValue = ( (double *) xyo->xPvData[i] )[ii];
            break;
          case ProcessVariable::specificType::shrt:
            if ( xyo->xSigned[i] ) {
              dxValue = (double) ( (short *) xyo->xPvData[i] )[ii];
            }
            else {
              dxValue = (double) ( (unsigned short *) xyo->xPvData[i] )[ii];
            }
            break;
          case ProcessVariable::specificType::chr:
            if ( xyo->xSigned[i] ) {
              dxValue = (double) ( (char *) xyo->xPvData[i] )[ii];
            }
            else {
              dxValue = (double) ( (unsigned char *) xyo->xPvData[i] )[ii];
            }
            break;
          case ProcessVariable::specificType::integer:
            if ( xyo->xSigned[i] ) {
              dxValue = (double) ( (int *) xyo->xPvData[i] )[ii];
            }
            else {
              dxValue = (double) ( (unsigned int *) xyo->xPvData[i] )[ii];
            }
            break;
          case ProcessVariable::specificType::enumerated:
            if ( xyo->xSigned[i] ) {
              dxValue = (double) ( (short *) xyo->xPvData[i] )[ii];
            }
            else {
              dxValue = (double) ( (unsigned short *) xyo->xPvData[i] )[ii];
            }
            break;
          default:
            dxValue = ( (double *) xyo->xPvData[i] )[ii];
            break;
          }

          if ( xyo->xAxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) {
            dxValue = loc_log10(  dxValue  );
          }
          else if ( xyo->xAxisStyle == XYGC_K_AXIS_STYLE_TIME_LOG10 ) {
            dxValue  = loc_log10(  dxValue  );
          }

          if ( !ctl ) {
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
	  }

          scaledX = rint( ( dxValue - xyo->curXMin ) *
           xyo->xFactor[i] + xyo->xOffset[i] );
          scaledX = dclamp( scaledX );

          xyo->addPoint( dxValue, scaledX, scaledY, yZero, i );

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
            xyo->arrayNumPoints[i] = xyo->count;
            xyo->needBufferScroll = 1;
            xyo->needThisbufScroll[i] = 1;
          }

          xyo->needUpdate = 1;
          xyo->xArrayNeedUpdate[i] = 1;
          xyo->yArrayNeedUpdate[i] = 1;
          xyo->actWin->addDefExeNode( xyo->aglPtr );

	  //}

        }

        break;

      case XYGC_K_TRACE_CHRONOLOGICAL:

        if ( xyo->forceVector[i] || ( xyo->yPv[i]->get_dimension() > 1 ) ) { // vector

          xyo->yArrayNeedUpdate[i] = xyo->xArrayNeedUpdate[i] = 1;
          xyo->needVectorUpdate = 1;
          xyo->actWin->addDefExeNode( xyo->aglPtr );

        }
        else { // scalar

          //if ( xyo->yArrayGotValue[i] && xyo->xArrayGotValue[i] ) {

            dyValue = (double) xyo->yPvCurValue[i];
            dxValue = (double) xyo->xPvCurValue[i];

            if ( xyo->y1AxisStyle[yi] == XYGC_K_AXIS_STYLE_LOG10 ) {
              dyValue = loc_log10(  dyValue  );
            }

            if ( !ctl ) {

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

            if ( xyo->y1AxisSource[yi] == XYGC_K_AUTOSCALE ) {
              if ( xyo->kpY1MinEfDouble[yi].isNull() ) {
                if ( dyValue < xyo->curY1Min[yi] ) {
                  xyo->needY1Rescale[yi] = 1;
                  xyo->y1RescaleValue[yi] = dyValue;
                  xyo->actWin->addDefExeNode( xyo->aglPtr );
                }
              }
              if ( xyo->kpY1MaxEfDouble[yi].isNull() ) {
                if ( dyValue > xyo->curY1Max[yi] ) {
                  xyo->needY1Rescale[yi] = 1;
                  xyo->y1RescaleValue[yi] = dyValue;
                  xyo->actWin->addDefExeNode( xyo->aglPtr );
                }
              }
            }

	    }

            scaledY = xyo->plotAreaH -
             rint( ( dyValue - xyo->curY1Min[yi] ) *
             xyo->y1Factor[yi][i] - xyo->y1Offset[yi][i] );
            scaledY = dclamp( scaledY );

            if ( xyo->y1AxisStyle[yi] == XYGC_K_AXIS_STYLE_LOG10 ) {
              yZero = xyo->plotAreaH + xyo->y1Offset[yi][i];
            }
            else {
              yZero = xyo->plotAreaH -
               rint( ( 0.0 - xyo->curY1Min[yi] ) *
               xyo->y1Factor[yi][i] - xyo->y1Offset[yi][i] );
              yZero = dclamp( yZero );
            }

            scaledX = rint( ( dxValue - xyo->curXMin ) *
             xyo->xFactor[i] + xyo->xOffset[i] );
            scaledX = dclamp( scaledX );

            xyo->addPoint( dxValue, scaledX, scaledY, yZero, i );

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
              xyo->arrayNumPoints[i] = xyo->count;
              xyo->needBufferScroll = 1;
              xyo->needThisbufScroll[i] = 1;
            }

            xyo->needUpdate = 1;
            xyo->xArrayNeedUpdate[i] = 1;
            xyo->yArrayNeedUpdate[i] = 1;
            xyo->actWin->addDefExeNode( xyo->aglPtr );

	  //}

	}

        break;

      }

    }

  }

  xyo->actWin->appCtx->proc->unlock();

}

static void xMonitorConnection (
  ProcessVariable *pv,
  void *userarg )
{

objPlusIndexPtr ptr = (objPlusIndexPtr) userarg;
xyGraphClass *xyo = (xyGraphClass *) ptr->objPtr;

  if ( pv->is_valid() ) {

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

static void xValueUpdate (
  ProcessVariable *pv,
  void *userarg )
{

objPlusIndexPtr ptr = (objPlusIndexPtr) userarg;
xyGraphClass *xyo = (xyGraphClass *) ptr->objPtr;
int i =  ptr->index;
int ii, yi;
double dxValue, dyValue;
double scaledX, scaledY;
double yZero;
int ctl;

  if ( !xyo->activeMode ) return;

  ctl = (int) xyo->traceCtl & ( 1 << i );

  xyo->actWin->appCtx->proc->lock();

  if ( !xyo->xArrayGotValueCallback[i] ) xyo->xArrayGotValueCallback[i] = 1;

  yi = 0;
  if ( xyo->y2Scale[i] ) yi = 1;

  switch ( xyo->traceType[i] ) {

  case XYGC_K_TRACE_XY:

    if ( xyo->forceVector[i] || ( xyo->xPv[i]->get_dimension() > 1 ) ) { // vector

      for ( ii=0; ii<xyo->xPvEffCount( i ); ii++ ) {

        if ( xyo->xPv[i]->get_dimension() == 1 ) {

          // There are two views of pv types, Type and specificType; this uses
          // specificType
          switch ( xyo->xPvType[i] ) {

          case ProcessVariable::specificType::flt:
            ( (float *) xyo->xPvData[i] )[ii] =
             (float) pv->get_double();
            break;

          case ProcessVariable::specificType::real:
            ( (double *) xyo->xPvData[i] )[ii] = pv->get_double();
            break;

          case ProcessVariable::specificType::shrt:
            if ( xyo->xSigned[i] ) {
              ( (short *) xyo->xPvData[i] )[ii] = (short) pv->get_int();
            }
            else {
              ( (unsigned short *) xyo->xPvData[i] )[ii] =
               (unsigned short) pv->get_short_array()[ii];
            }
            break;

          case ProcessVariable::specificType::chr:
            if ( xyo->xSigned[i] ) {
              ( (char *) xyo->xPvData[i] )[ii] = (char) pv->get_int();
            }
            else {
              ( (unsigned char *) xyo->xPvData[i] )[ii] =
               (unsigned char) pv->get_int();
            }
            break;

          case ProcessVariable::specificType::integer:
            if ( xyo->xSigned[i] ) {
              ( (int *) xyo->xPvData[i] )[ii] = pv->get_int();
            }
            else {
              ( (unsigned int *) xyo->xPvData[i] )[ii] =
               (unsigned int) pv->get_int();
            }
            break;

          case ProcessVariable::specificType::enumerated:
            if ( xyo->xSigned[i] ) {
              ( (short *) xyo->xPvData[i] )[ii] = (short) pv->get_int();
            }
            else {
              ( (unsigned short *) xyo->xPvData[i] )[ii] =
               (unsigned short) pv->get_int();
            }
            break;

          default:
            ( (double *) xyo->xPvData[i] )[ii] = pv->get_double();
            break;

          }

	}
	else {

          // There are two views of pv types, Type and specificType; this uses
          // specificType
          switch ( xyo->xPvType[i] ) {

          case ProcessVariable::specificType::flt:
            ( (float *) xyo->xPvData[i] )[ii] =
             (float) pv->get_double_array()[ii];
            break;

          case ProcessVariable::specificType::real: 
            ( (double *) xyo->xPvData[i] )[ii] = pv->get_double_array()[ii];
            break;

          case ProcessVariable::specificType::shrt:
            if ( xyo->xSigned[i] ) {
              ( (short *) xyo->xPvData[i] )[ii] = (short) pv->get_short_array()[ii];
            }
            else {
              ( (unsigned short *) xyo->xPvData[i] )[ii] =
               (unsigned short) pv->get_short_array()[ii];
            }
            break;

          case ProcessVariable::specificType::chr:
            if ( xyo->xSigned[i] ) {
              ( (char *) xyo->xPvData[i] )[ii] = (char) pv->get_char_array()[ii];
            }
            else {
              ( (unsigned char *) xyo->xPvData[i] )[ii] =
               (unsigned char) pv->get_char_array()[ii];
            }
            break;

          case ProcessVariable::specificType::integer:
            if ( xyo->xSigned[i] ) {
              ( (int *) xyo->xPvData[i] )[ii] = pv->get_int_array()[ii];
            }
            else {
              ( (unsigned int *) xyo->xPvData[i] )[ii] =
               (unsigned int) pv->get_int_array()[ii];
            }
            break;

          case ProcessVariable::specificType::enumerated:
            if ( xyo->xSigned[i] ) {
              ( (short *) xyo->xPvData[i] )[ii] = (short) pv->get_int_array()[ii];
            }
            else {
              ( (unsigned short *) xyo->xPvData[i] )[ii] =
               (unsigned short) pv->get_int_array()[ii];
            }
            break;

          default:
            ( (double *) xyo->xPvData[i] )[ii] = pv->get_double_array()[ii];
            break;

          }

	}

      }

      xyo->xArrayGotValue[i] = 1;

      if ( xyo->plotUpdateMode[i] != XYGC_K_UPDATE_ON_TRIG ) {

        xyo->xArrayNeedUpdate[i] = 1;
        xyo->needVectorUpdate = 1;
        xyo->actWin->addDefExeNode( xyo->aglPtr );

      }

    }
    else { // scalar

      xyo->xPvCurValue[i] = pv->get_double();

      if ( ( xyo->arrayNumPoints[i] >= xyo->count ) &&
           ( xyo->plotMode == XYGC_K_PLOT_MODE_PLOT_N_STOP ) ) {
        xyo->arrayNumPoints[i] = xyo->count;
        xyo->actWin->appCtx->proc->unlock();
        return;
      }

      // x
      ii = xyo->arrayTail[i];

      // There are two views of pv types, Type and specificType; this uses
      // specificType
      switch ( xyo->xPvType[i] ) {

      case ProcessVariable::specificType::flt:
        ( (float *) xyo->xPvData[i] )[ii] = (float) pv->get_double();
        break;

      case ProcessVariable::specificType::real: 
        ( (double *) xyo->xPvData[i] )[ii] = pv->get_double();
        break;

      case ProcessVariable::specificType::shrt:
        if ( xyo->xSigned[i] ) {
          ( (short *) xyo->xPvData[i] )[ii] = (short) pv->get_int();
        }
        else {
          ( (unsigned short *) xyo->xPvData[i] )[ii] =
           (unsigned short) pv->get_int();
        }
        break;

      case ProcessVariable::specificType::chr:
        if ( xyo->xSigned[i] ) {
          ( (char *) xyo->xPvData[i] )[ii] = (char) pv->get_int();
        }
        else {
          ( (unsigned char *) xyo->xPvData[i] )[ii] =
	   (unsigned char) pv->get_int();
        }
        break;

      case ProcessVariable::specificType::integer:
        if ( xyo->xSigned[i] ) {
          ( (int *) xyo->xPvData[i] )[ii] = pv->get_int();
        }
        else {
          ( (unsigned int *) xyo->xPvData[i] )[ii] =
           (unsigned int) pv->get_int();
        }
        break;

      case ProcessVariable::specificType::enumerated:
        if ( xyo->xSigned[i] ) {
          ( (short *) xyo->xPvData[i] )[ii] = (short) pv->get_int();
        }
        else {
          ( (unsigned short *) xyo->xPvData[i] )[ii] =
           (unsigned short) pv->get_int();
        }
        break;

      default:
        ( (double *) xyo->xPvData[i] )[ii] = pv->get_double();
        break;

      }

      if ( ( xyo->plotUpdateMode[i] == XYGC_K_UPDATE_ON_X_OR_Y ) ||
           ( xyo->plotUpdateMode[i] == XYGC_K_UPDATE_ON_X ) ) {

        // y
        ii = xyo->arrayTail[i];
        setVal( xyo->yPvType[i], (void *) xyo->yPvData[i], ii, xyo->yPvCurValue[i] );

        xyo->yArrayGotValue[i] = 1;

      }

      if ( ( xyo->plotUpdateMode[i] != XYGC_K_UPDATE_ON_TRIG ) &&
           ( xyo->plotUpdateMode[i] != XYGC_K_UPDATE_ON_Y ) &&
             xyo->yArrayGotValue[i] ) {

        ii = xyo->arrayTail[i];

        // There are two views of pv types, Type and specificType; this uses
        // specificType
        switch ( xyo->yPvType[i] ) {
        case ProcessVariable::specificType::flt:
          dyValue = (double) ( (float *) xyo->yPvData[i] )[ii];
          break;
        case ProcessVariable::specificType::real: 
          dyValue = ( (double *) xyo->yPvData[i] )[ii];
          break;
        case ProcessVariable::specificType::shrt:
          if ( xyo->ySigned[i] ) {
            dyValue = (double) ( (short *) xyo->yPvData[i] )[ii];
          }
          else {
            dyValue = (double) ( (unsigned short *) xyo->yPvData[i] )[ii];
          }
          break;
        case ProcessVariable::specificType::chr:
          if ( xyo->ySigned[i] ) {
            dyValue = (double) ( (char *) xyo->yPvData[i] )[ii];
          }
          else {
            dyValue = (double) ( (unsigned char *) xyo->yPvData[i] )[ii];
          }
          break;
        case ProcessVariable::specificType::integer:
          if ( xyo->ySigned[i] ) {
            dyValue = (double) ( (int *) xyo->yPvData[i] )[ii];
          }
          else {
            dyValue = (double) ( (unsigned int *) xyo->yPvData[i] )[ii];
          }
          break;
        case ProcessVariable::specificType::enumerated:
          if ( xyo->ySigned[i] ) {
            dyValue = (double) ( (short *) xyo->yPvData[i] )[ii];
          }
          else {
            dyValue = (double) ( (unsigned short *) xyo->yPvData[i] )[ii];
          }
          break;
        default:
          dyValue = ( (double *) xyo->yPvData[i] )[ii];
          break;
        }

        if ( xyo->y1AxisStyle[yi] == XYGC_K_AXIS_STYLE_LOG10 ) {
          dyValue = loc_log10(  dyValue  );
        }

	if ( !ctl ) {

        if ( xyo->y1AxisSource[yi] == XYGC_K_AUTOSCALE ) {
          if ( xyo->kpY1MinEfDouble[yi].isNull() ) {
            if ( dyValue < xyo->curY1Min[yi] ) {
              xyo->needY1Rescale[yi] = 1;
              xyo->y1RescaleValue[yi] = dyValue;
              xyo->actWin->addDefExeNode( xyo->aglPtr );
            }
          }
          if ( xyo->kpY1MaxEfDouble[yi].isNull() ) {
            if ( dyValue > xyo->curY1Max[yi] ) {
              xyo->needY1Rescale[yi] = 1;
              xyo->y1RescaleValue[yi] = dyValue;
              xyo->actWin->addDefExeNode( xyo->aglPtr );
            }
          }
        }

	}

        scaledY = xyo->plotAreaH -
         rint( ( dyValue - xyo->curY1Min[yi] ) *
         xyo->y1Factor[yi][i] - xyo->y1Offset[yi][i] );
        scaledY = dclamp( scaledY );

        if ( xyo->y1AxisStyle[yi] == XYGC_K_AXIS_STYLE_LOG10 ) {
          yZero = xyo->plotAreaH + xyo->y1Offset[yi][i];
        }
        else {
          yZero = xyo->plotAreaH -
           rint( ( 0.0 - xyo->curY1Min[yi] ) *
           xyo->y1Factor[yi][i] - xyo->y1Offset[yi][i] );
          yZero = dclamp( yZero );
        }

        // There are two views of pv types, Type and specificType; this uses
        // specificType
        switch ( xyo->xPvType[i] ) {
        case ProcessVariable::specificType::flt:
          dxValue = (double) ( (float *) xyo->xPvData[i] )[ii];
          break;
        case ProcessVariable::specificType::real: 
          dxValue = ( (double *) xyo->xPvData[i] )[ii];
          break;
        case ProcessVariable::specificType::shrt:
          if ( xyo->xSigned[i] ) {
            dxValue = (double) ( (short *) xyo->xPvData[i] )[ii];
          }
          else {
            dxValue = (double) ( (unsigned short *) xyo->xPvData[i] )[ii];
          }
          break;
        case ProcessVariable::specificType::chr:
          if ( xyo->xSigned[i] ) {
            dxValue = (double) ( (char *) xyo->xPvData[i] )[ii];
          }
          else {
            dxValue = (double) ( (unsigned char *) xyo->xPvData[i] )[ii];
          }
          break;
        case ProcessVariable::specificType::integer:
          if ( xyo->xSigned[i] ) {
            dxValue = (double) ( (int *) xyo->xPvData[i] )[ii];
          }
          else {
            dxValue = (double) ( (unsigned int *) xyo->xPvData[i] )[ii];
          }
          break;
        case ProcessVariable::specificType::enumerated:
          if ( xyo->xSigned[i] ) {
            dxValue = (double) ( (short *) xyo->xPvData[i] )[ii];
          }
          else {
            dxValue = (double) ( (unsigned short *) xyo->xPvData[i] )[ii];
          }
          break;
        default:
          dxValue = ( (double *) xyo->xPvData[i] )[ii];
          break;
        }

        if ( xyo->xAxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) {
          dxValue = loc_log10(  dxValue  );
        }
        else if ( xyo->xAxisStyle == XYGC_K_AXIS_STYLE_TIME_LOG10 ) {
          dxValue  = loc_log10(  dxValue  );
        }


	if ( !ctl ) {

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

	}

        scaledX = rint( ( dxValue - xyo->curXMin ) *
         xyo->xFactor[i] + xyo->xOffset[i] );
        scaledX = dclamp( scaledX );

        xyo->addPoint( dxValue, scaledX, scaledY, yZero, i );

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
          xyo->arrayNumPoints[i] = xyo->count;
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

    fprintf( stderr, "error XYGC_K_TRACE_CHRONOLOGICAL in xValueUpdate\n" );

    break;

  }

  xyo->actWin->appCtx->proc->unlock();

}

static void nMonitorConnection (
  ProcessVariable *pv,
  void *userarg )
{

objPlusIndexPtr ptr = (objPlusIndexPtr) userarg;
xyGraphClass *xyo = (xyGraphClass *) ptr->objPtr;
int i = ptr->index;

  if ( pv->is_valid() ) {

    if ( xyo->needNPvConnect[i] == 0 ) {

      xyo->actWin->appCtx->proc->lock();
      xyo->needNConnect = 1;
      xyo->needNPvConnect[i] = 1;
      xyo->actWin->addDefExeNode( xyo->aglPtr );
      xyo->actWin->appCtx->proc->unlock();

    }

  }

}

static void nValueUpdate (
  ProcessVariable *pv,
  void *userarg )
{

objPlusIndexPtr ptr = (objPlusIndexPtr) userarg;
xyGraphClass *xyo = (xyGraphClass *) ptr->objPtr;
int i =  ptr->index;

  if ( pv->is_valid() ) {

    xyo->actWin->appCtx->proc->lock();
    xyo->traceSize[i] = (int) pv->get_int();
    xyo->needArraySizeChange = 1;
    xyo->actWin->addDefExeNode( xyo->aglPtr );
    xyo->actWin->appCtx->proc->unlock();

  }

}

static void yMonitorConnection (
  ProcessVariable *pv,
  void *userarg )
{

objPlusIndexPtr ptr = (objPlusIndexPtr) userarg;
xyGraphClass *xyo = (xyGraphClass *) ptr->objPtr;

  if ( pv->is_valid() ) {

    if ( !xyo->connection.pvsConnected() ) {

      xyo->connection.setPvConnected( (void *) ( ptr->index ) );

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

static void yValueUpdate (
  ProcessVariable *pv,
  void *userarg )
{

objPlusIndexPtr ptr = (objPlusIndexPtr) userarg;
xyGraphClass *xyo = (xyGraphClass *) ptr->objPtr;
int ii, i =  ptr->index;
double dxValue, dyValue;
double scaledX, scaledY;
double yZero;
int yi;
int ctl;

  if ( !xyo->activeMode ) return;

  ctl = (int) xyo->traceCtl & ( 1 << i );

  xyo->actWin->appCtx->proc->lock();

  if ( !xyo->yArrayGotValueCallback[i] ) xyo->yArrayGotValueCallback[i] = 1;

  yi = 0;
  if ( xyo->y2Scale[i] ) yi = 1;

  switch ( xyo->traceType[i] ) {

  case XYGC_K_TRACE_XY:

    if ( xyo->forceVector[i] || ( xyo->yPv[i]->get_dimension() > 1 ) ) { // vector

      for ( ii=0; ii<xyo->yPvEffCount( i ); ii++ ) {

        if ( xyo->yPv[i]->get_dimension() == 1 ) {

          // There are two views of pv types, Type and specificType; this uses
          // specificType
          switch ( xyo->yPvType[i] ) {

          case ProcessVariable::specificType::flt:
            ( (float *) xyo->yPvData[i] )[ii] =
             (float) pv->get_double();
            break;

          case ProcessVariable::specificType::real: 
            ( (double *) xyo->yPvData[i] )[ii] = pv->get_double();
            break;

          case ProcessVariable::specificType::shrt:
            if ( xyo->ySigned[i] ) {
              ( (short *) xyo->yPvData[i] )[ii] = (short) pv->get_int();
            }
            else {
              ( (unsigned short *) xyo->yPvData[i] )[ii] =
               (unsigned short) pv->get_short_array()[ii];
            }
            break;

          case ProcessVariable::specificType::chr:
            if ( xyo->ySigned[i] ) {
              ( (char *) xyo->yPvData[i] )[ii] = (char) pv->get_int();
            }
            else {
              ( (unsigned char *) xyo->yPvData[i] )[ii] =
               (unsigned char) pv->get_int();
            }
            break;

          case ProcessVariable::specificType::integer:
            if ( xyo->ySigned[i] ) {
              ( (int *) xyo->yPvData[i] )[ii] = pv->get_int();
            }
            else {
              ( (unsigned int *) xyo->yPvData[i] )[ii] =
               (unsigned int) pv->get_int();
            }
            break;

          case ProcessVariable::specificType::enumerated:
            if ( xyo->ySigned[i] ) {
              ( (short *) xyo->yPvData[i] )[ii] = (short) pv->get_int();
            }
            else {
              ( (unsigned short *) xyo->yPvData[i] )[ii] =
               (unsigned short) pv->get_int();
            }
            break;

          default:
            ( (double *) xyo->yPvData[i] )[ii] = pv->get_double();
            break;

          }

	}
	else {

          // There are two views of pv types, Type and specificType; this uses
          // specificType
          switch ( xyo->yPvType[i] ) {

          case ProcessVariable::specificType::flt:
            ( (float *) xyo->yPvData[i] )[ii] =
             (float) pv->get_double_array()[ii];
            break;

          case ProcessVariable::specificType::real: 
            ( (double *) xyo->yPvData[i] )[ii] = pv->get_double_array()[ii];
            break;

          case ProcessVariable::specificType::shrt:
            if ( xyo->ySigned[i] ) {
              ( (short *) xyo->yPvData[i] )[ii] = (short) pv->get_short_array()[ii];
            }
            else {
              ( (unsigned short *) xyo->yPvData[i] )[ii] =
               (unsigned short) pv->get_short_array()[ii];
            }
            break;

          case ProcessVariable::specificType::chr:
            if ( xyo->ySigned[i] ) {
              ( (char *) xyo->yPvData[i] )[ii] = (char) pv->get_char_array()[ii];
            }
            else {
              ( (unsigned char *) xyo->yPvData[i] )[ii] =
               (unsigned char) pv->get_char_array()[ii];
            }
            break;

          case ProcessVariable::specificType::integer:
            if ( xyo->ySigned[i] ) {
              ( (int *) xyo->yPvData[i] )[ii] = pv->get_int_array()[ii];
            }
            else {
              ( (unsigned int *) xyo->yPvData[i] )[ii] =
               (unsigned int) pv->get_int_array()[ii];
            }
            break;

          case ProcessVariable::specificType::enumerated:
            if ( xyo->ySigned[i] ) {
              ( (short *) xyo->yPvData[i] )[ii] = (short) pv->get_int_array()[ii];
            }
            else {
              ( (unsigned short *) xyo->yPvData[i] )[ii] =
               (unsigned short) pv->get_int_array()[ii];
            }
            break;

          default:
            ( (double *) xyo->yPvData[i] )[ii] = pv->get_double_array()[ii];
            break;

          }

	}

      }

      xyo->yArrayGotValue[i] = 1;

      if ( xyo->plotUpdateMode[i] != XYGC_K_UPDATE_ON_TRIG ) {

        xyo->yArrayNeedUpdate[i] = 1;
        xyo->needVectorUpdate = 1;
        xyo->actWin->addDefExeNode( xyo->aglPtr );

      }

    }
    else { // scalar

      xyo->yPvCurValue[i] = pv->get_double();

      if ( ( xyo->arrayNumPoints[i] >= xyo->count ) &&
           ( xyo->plotMode == XYGC_K_PLOT_MODE_PLOT_N_STOP ) ) {
        xyo->arrayNumPoints[i] = xyo->count;
        xyo->actWin->appCtx->proc->unlock();
        return;
      }

      // y
      ii = xyo->arrayTail[i];

      // There are two views of pv types, Type and specificType; this uses
      // specificType
      switch ( xyo->yPvType[i] ) {

      case ProcessVariable::specificType::flt:
        ( (float *) xyo->yPvData[i] )[ii] = (float) pv->get_double();
        break;

      case ProcessVariable::specificType::real: 
        ( (double *) xyo->yPvData[i] )[ii] = pv->get_double();
        break;

      case ProcessVariable::specificType::shrt:
        if ( xyo->ySigned[i] ) {
          ( (short *) xyo->yPvData[i] )[ii] = (short) pv->get_int();
        }
        else {
          ( (unsigned short *) xyo->yPvData[i] )[ii] =
           (unsigned short) pv->get_int();
        }
        break;

      case ProcessVariable::specificType::chr:
        if ( xyo->ySigned[i] ) {
          ( (char *) xyo->yPvData[i] )[ii] = (char) pv->get_int();
        }
        else {
          ( (unsigned char *) xyo->yPvData[i] )[ii] =
	   (unsigned char) pv->get_int();
        }
        break;

      case ProcessVariable::specificType::integer:
        if ( xyo->ySigned[i] ) {
          ( (int *) xyo->yPvData[i] )[ii] = pv->get_int();
        }
        else {
          ( (unsigned int *) xyo->yPvData[i] )[ii] =
           (unsigned int) pv->get_int();
        }
        break;

      case ProcessVariable::specificType::enumerated:
        if ( xyo->ySigned[i] ) {
          ( (short *) xyo->yPvData[i] )[ii] = (short) pv->get_int();
        }
        else {
          ( (unsigned short *) xyo->yPvData[i] )[ii] =
           (unsigned short) pv->get_int();
        }
        break;

      default:
        ( (double *) xyo->yPvData[i] )[ii] = pv->get_double();
        break;

      }

      if ( ( xyo->plotUpdateMode[i] == XYGC_K_UPDATE_ON_X_OR_Y ) ||
           ( xyo->plotUpdateMode[i] == XYGC_K_UPDATE_ON_Y ) ) {

        // x
        ii = xyo->arrayTail[i];
        setVal( xyo->xPvType[i], (void *) xyo->xPvData[i], ii, xyo->xPvCurValue[i] );

        xyo->xArrayGotValue[i] = 1;

      }

      if ( ( xyo->plotUpdateMode[i] != XYGC_K_UPDATE_ON_TRIG ) &&
           ( xyo->plotUpdateMode[i] != XYGC_K_UPDATE_ON_X ) &&
             xyo->xArrayGotValue[i] ) {

        ii = xyo->arrayTail[i];

        // There are two views of pv types, Type and specificType; this uses
        // specificType
        switch ( xyo->yPvType[i] ) {
        case ProcessVariable::specificType::flt:
          dyValue = (double) ( (float *) xyo->yPvData[i] )[ii];
          break;
        case ProcessVariable::specificType::real: 
          dyValue = ( (double *) xyo->yPvData[i] )[ii];
          break;
        case ProcessVariable::specificType::shrt:
          if ( xyo->ySigned[i] ) {
            dyValue = (double) ( (short *) xyo->yPvData[i] )[ii];
          }
          else {
            dyValue = (double) ( (unsigned short *) xyo->yPvData[i] )[ii];
          }
          break;
        case ProcessVariable::specificType::chr:
          if ( xyo->ySigned[i] ) {
            dyValue = (double) ( (char *) xyo->yPvData[i] )[ii];
          }
          else {
            dyValue = (double) ( (unsigned char *) xyo->yPvData[i] )[ii];
          }
          break;
        case ProcessVariable::specificType::integer:
          if ( xyo->ySigned[i] ) {
            dyValue = (double) ( (int *) xyo->yPvData[i] )[ii];
	  }
	  else {
            dyValue = (double) ( (unsigned int *) xyo->yPvData[i] )[ii];
	  }
          break;
        case ProcessVariable::specificType::enumerated:
          if ( xyo->ySigned[i] ) {
            dyValue = (double) ( (short *) xyo->yPvData[i] )[ii];
	  }
	  else {
            dyValue = (double) ( (unsigned short *) xyo->yPvData[i] )[ii];
	  }
          break;
        default:
          dyValue = ( (double *) xyo->yPvData[i] )[ii];
          break;
        }

        if ( xyo->y1AxisStyle[yi] == XYGC_K_AXIS_STYLE_LOG10 ) {
          dyValue = loc_log10(  dyValue  );
        }

	if ( !ctl ) {

        if ( xyo->y1AxisSource[yi] == XYGC_K_AUTOSCALE ) {
          if ( xyo->kpY1MinEfDouble[yi].isNull() ) {
            if ( dyValue < xyo->curY1Min[yi] ) {
              xyo->needY1Rescale[yi] = 1;
              xyo->y1RescaleValue[yi] = dyValue;
              xyo->actWin->addDefExeNode( xyo->aglPtr );
            }
          }
          if ( xyo->kpY1MaxEfDouble[yi].isNull() ) {
            if ( dyValue > xyo->curY1Max[yi] ) {
              xyo->needY1Rescale[yi] = 1;
              xyo->y1RescaleValue[yi] = dyValue;
              xyo->actWin->addDefExeNode( xyo->aglPtr );
            }
          }
        }

	}

        scaledY = xyo->plotAreaH -
         rint( ( dyValue - xyo->curY1Min[yi] ) *
         xyo->y1Factor[yi][i] - xyo->y1Offset[yi][i] );
        scaledY = dclamp( scaledY );

        if ( xyo->y1AxisStyle[yi] == XYGC_K_AXIS_STYLE_LOG10 ) {
          yZero = xyo->plotAreaH + xyo->y1Offset[yi][i];
        }
        else {
          yZero = xyo->plotAreaH -
           rint( ( 0.0 - xyo->curY1Min[yi] ) *
           xyo->y1Factor[yi][i] - xyo->y1Offset[yi][i] );
          yZero = dclamp( yZero );
        }

        // There are two views of pv types, Type and specificType; this uses
        // specificType
        switch ( xyo->xPvType[i] ) {
        case ProcessVariable::specificType::flt:
          dxValue = (double) ( (float *) xyo->xPvData[i] )[ii];
          break;
        case ProcessVariable::specificType::real: 
          dxValue = ( (double *) xyo->xPvData[i] )[ii];
          break;
        case ProcessVariable::specificType::shrt:
          if ( xyo->xSigned[i] ) {
            dxValue = (double) ( (short *) xyo->xPvData[i] )[ii];
	  }
	  else {
            dxValue = (double) ( (unsigned short *) xyo->xPvData[i] )[ii];
	  }
          break;
        case ProcessVariable::specificType::chr:
          if ( xyo->xSigned[i] ) {
            dxValue = (double) ( (char *) xyo->xPvData[i] )[ii];
	  }
	  else {
            dxValue = (double) ( (unsigned char *) xyo->xPvData[i] )[ii];
	  }
          break;
        case ProcessVariable::specificType::integer:
          if ( xyo->xSigned[i] ) {
            dxValue = (double) ( (int *) xyo->xPvData[i] )[ii];
	  }
	  else {
            dxValue = (double) ( (unsigned int *) xyo->xPvData[i] )[ii];
	  }
          break;
        case ProcessVariable::specificType::enumerated:
          if ( xyo->xSigned[i] ) {
            dxValue = (double) ( (short *) xyo->xPvData[i] )[ii];
	  }
	  else {
            dxValue = (double) ( (unsigned short *) xyo->xPvData[i] )[ii];
	  }
          break;
        default:
          dxValue = ( (double *) xyo->xPvData[i] )[ii];
          break;
        }

        if ( xyo->xAxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) {
          dxValue = loc_log10(  dxValue  );
        }
        else if ( xyo->xAxisStyle == XYGC_K_AXIS_STYLE_TIME_LOG10 ) {
          dxValue  = loc_log10(  dxValue  );
        }

	if ( !ctl ) {

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

	}

        scaledX = rint( ( dxValue - xyo->curXMin ) *
         xyo->xFactor[i] + xyo->xOffset[i] );
        scaledX = dclamp( scaledX );

        xyo->addPoint( dxValue, scaledX, scaledY, yZero, i );

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
          xyo->arrayNumPoints[i] = xyo->count;
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

    fprintf( stderr, "XYGC_K_TRACE_CHRONOLOGICAL not implemented in yValueUpdate\n" );

    break;

  }

  xyo->actWin->appCtx->proc->unlock();

}

static void yValueWithTimeUpdate (
  ProcessVariable *pv,
  void *userarg )
{

objPlusIndexPtr ptr = (objPlusIndexPtr) userarg;
xyGraphClass *xyo = (xyGraphClass *) ptr->objPtr;
int ii, yi, i =  ptr->index;
int sec, nsec;
double dxValue=0., dyValue;
double scaledX, scaledY;
double yZero;
int ctl;

  if ( !xyo->activeMode ) return;

  ctl = (int) xyo->traceCtl & ( 1 << i );

  xyo->actWin->appCtx->proc->lock();

  if ( !xyo->yArrayGotValueCallback[i] ) xyo->yArrayGotValueCallback[i] = 1;

  yi = 0;
  if ( xyo->y2Scale[i] ) yi = 1;

  switch ( xyo->traceType[i] ) {

  case XYGC_K_TRACE_XY:

    fprintf( stderr, "xy illegal\n" );
    break;

  case XYGC_K_TRACE_CHRONOLOGICAL:

    if ( xyo->forceVector[i] || ( xyo->yPv[i]->get_dimension() > 1 ) ) { // vector

      for ( ii=0; ii<xyo->yPvEffCount( i ); ii++ ) {

        if ( xyo->yPv[i]->get_dimension() == 1 ) {

          // There are two views of pv types, Type and specificType; this uses
          // specificType
          switch ( xyo->yPvType[i] ) {

          case ProcessVariable::specificType::flt:
            ( (float *) xyo->yPvData[i] )[ii] =
             (float) pv->get_double();
            break;

          case ProcessVariable::specificType::real: 
            ( (double *) xyo->yPvData[i] )[ii] = pv->get_double();
            break;

          case ProcessVariable::specificType::shrt:
            if ( xyo->ySigned[i] ) {
              ( (short *) xyo->yPvData[i] )[ii] = (short) pv->get_int();
            }
            else {
              ( (unsigned short *) xyo->yPvData[i] )[ii] =
               (unsigned short) pv->get_short_array()[ii];
            }
            break;

          case ProcessVariable::specificType::chr:
            if ( xyo->ySigned[i] ) {
              ( (char *) xyo->yPvData[i] )[ii] = (char) pv->get_int();
            }
            else {
              ( (unsigned char *) xyo->yPvData[i] )[ii] =
               (unsigned char) pv->get_int();
            }
            break;

          case ProcessVariable::specificType::integer:
            if ( xyo->ySigned[i] ) {
              ( (int *) xyo->yPvData[i] )[ii] = pv->get_int();
            }
            else {
              ( (unsigned int *) xyo->yPvData[i] )[ii] =
               (unsigned int) pv->get_int();
            }
            break;

          case ProcessVariable::specificType::enumerated:
            if ( xyo->ySigned[i] ) {
              ( (short *) xyo->yPvData[i] )[ii] = (short) pv->get_int();
            }
            else {
              ( (unsigned short *) xyo->yPvData[i] )[ii] =
               (unsigned short) pv->get_int();
            }
            break;

          default:
            ( (double *) xyo->yPvData[i] )[ii] = pv->get_double();
            break;

          }

	}
	else {

          // There are two views of pv types, Type and specificType; this uses
          // specificType
          switch ( xyo->yPvType[i] ) {

          case ProcessVariable::specificType::flt:
            ( (float *) xyo->yPvData[i] )[ii] =
             (float) pv->get_double_array()[ii];
            break;

          case ProcessVariable::specificType::real: 
            ( (double *) xyo->yPvData[i] )[ii] = pv->get_double_array()[ii];
            break;

          case ProcessVariable::specificType::shrt:
            if ( xyo->ySigned[i] ) {
              ( (short *) xyo->yPvData[i] )[ii] = (short) pv->get_short_array()[ii];
            }
            else {
              ( (unsigned short *) xyo->yPvData[i] )[ii] =
               (unsigned short) pv->get_short_array()[ii];
            }
            break;

          case ProcessVariable::specificType::chr:
            if ( xyo->ySigned[i] ) {
              ( (char *) xyo->yPvData[i] )[ii] = (char) pv->get_char_array()[ii];
            }
            else {
              ( (unsigned char *) xyo->yPvData[i] )[ii] =
               (unsigned char) pv->get_char_array()[ii];
            }
            break;

          case ProcessVariable::specificType::integer:
            if ( xyo->ySigned[i] ) {
              ( (int *) xyo->yPvData[i] )[ii] = pv->get_int_array()[ii];
            }
            else {
              ( (unsigned int *) xyo->yPvData[i] )[ii] =
               (unsigned int) pv->get_int_array()[ii];
            }
            break;

          case ProcessVariable::specificType::enumerated:
            if ( xyo->ySigned[i] ) {
              ( (short *) xyo->yPvData[i] )[ii] = (short) pv->get_int_array()[ii];
            }
            else {
              ( (unsigned short *) xyo->yPvData[i] )[ii] =
               (unsigned short) pv->get_int_array()[ii];
            }
            break;

          default:
            ( (double *) xyo->yPvData[i] )[ii] = pv->get_double_array()[ii];
            break;

          }

	}

        dxValue = (double) ii;
        ( (double *) xyo->xPvData[i] )[ii] = dxValue;

      }

      if ( xyo->plotUpdateMode[i] != XYGC_K_UPDATE_ON_TRIG ) {

        xyo->yArrayNeedUpdate[i] = 1;
        xyo->needVectorUpdate = 1;
        xyo->actWin->addDefExeNode( xyo->aglPtr );

      }

    }
    else { // scalar

      if ( ( xyo->arrayNumPoints[i] >= xyo->count ) &&
           ( xyo->plotMode == XYGC_K_PLOT_MODE_PLOT_N_STOP ) ) {
        xyo->arrayNumPoints[i] = xyo->count;
        xyo->actWin->appCtx->proc->unlock();
        return;
      }

      ii = xyo->arrayTail[i];

      // There are two views of pv types, Type and specificType; this uses
      // specificType
      switch ( xyo->yPvType[i] ) {

      case ProcessVariable::specificType::flt:
        ( (float *) xyo->yPvData[i] )[ii] = dyValue = (float) pv->get_double();
        sec = pv->get_time_t();
        nsec = pv->get_nano();
        break;

      case ProcessVariable::specificType::real: 
        ( (double *) xyo->yPvData[i] )[ii] = dyValue = pv->get_double();
        sec = pv->get_time_t();
        nsec = pv->get_nano();
        break;

      case ProcessVariable::specificType::shrt:
        if ( xyo->ySigned[i] ) {
          ( (short *) xyo->yPvData[i] )[ii] = (short) pv->get_int();
          dyValue = (short) pv->get_int();
        }
        else {
          ( (unsigned short *) xyo->yPvData[i] )[ii] =
           (unsigned short) pv->get_int();
	  dyValue = (unsigned short) pv->get_int();
        }
        sec = pv->get_time_t();
        nsec = pv->get_nano();
        break;

      case ProcessVariable::specificType::chr:
        if ( xyo->ySigned[i] ) {
          ( (char *) xyo->yPvData[i] )[ii] = (char) pv->get_int();
	  dyValue = (char) pv->get_int();
        }
        else {
          ( (unsigned char *) xyo->yPvData[i] )[ii] =
	   (unsigned char) pv->get_int();
	  dyValue = (unsigned char) pv->get_int();
        }
        sec = pv->get_time_t();
        nsec = pv->get_nano();
        break;

      case ProcessVariable::specificType::integer:
        if ( xyo->ySigned[i] ) {
          ( (int *) xyo->yPvData[i] )[ii] = pv->get_int();
	  dyValue = pv->get_int();
        }
        else {
          ( (unsigned int *) xyo->yPvData[i] )[ii] =
           (unsigned int) pv->get_int();
	  dyValue = (unsigned int) pv->get_int();
        }
        sec = pv->get_time_t();
        nsec = pv->get_nano();
        break;

      case ProcessVariable::specificType::enumerated:
        if ( xyo->ySigned[i] ) {
          ( (short *) xyo->yPvData[i] )[ii] = (short) pv->get_int();
	  dyValue = (short) pv->get_int();
        }
        else {
          ( (unsigned short *) xyo->yPvData[i] )[ii] =
           (unsigned short) pv->get_int();
	  dyValue = (unsigned short) pv->get_int();
        }
        sec = pv->get_time_t();
        nsec = pv->get_nano();
        break;

      default:
        ( (double *) xyo->yPvData[i] )[ii] = pv->get_double();
	dyValue = pv->get_double();
        sec = pv->get_time_t();
        nsec = pv->get_nano();
        break;

      }

      if ( xyo->firstTimeSample ) {
        //xyo->firstTimeSample = 0;
        xyo->curSec = sec;
        xyo->curNsec = 0;
        sec = 0;
      }
      else {
        sec -= xyo->curSec;
        nsec -= xyo->curNsec;
      }

      if ( xyo->xAxisStyle == XYGC_K_AXIS_STYLE_TIME ) {

        dxValue = (double) sec + (double) nsec * 0.000000001;
        ( (double *) xyo->xPvData[i] )[ii] = dxValue;

      }
      else if ( xyo->xAxisStyle == XYGC_K_AXIS_STYLE_TIME_LOG10 ) {

        dxValue = (double) sec + (double) nsec * 0.000000001;
        ( (double *) xyo->xPvData[i] )[ii] = dxValue;
        dxValue = loc_log10(  dxValue  );

      }
      else if ( xyo->xAxisStyle == XYGC_K_AXIS_STYLE_LINEAR ) {

        if ( xyo->special[i] ) {
          dxValue = (double) ( xyo->totalCount[i] % xyo->count );
	}
	else {
          dxValue = (double) xyo->totalCount[i];
	}
        ( (double *) xyo->xPvData[i] )[ii] = dxValue;

        if ( !(xyo->firstTimeSample) ) {
          ++(xyo->totalCount[i]);
	}

      }
      else if ( xyo->xAxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) {

        if ( xyo->special[i] ) {
          dxValue = (double) ( xyo->totalCount[i] % xyo->count );
	}
	else {
          dxValue = (double) xyo->totalCount[i];
	}
        ( (double *) xyo->xPvData[i] )[ii] = dxValue;
        dxValue = loc_log10(  dxValue  );

        if ( !xyo->firstTimeSample ) {
          ++(xyo->totalCount[i]);
	}

      }

      xyo->firstTimeSample = 0;

      if ( xyo->plotUpdateMode[i] != XYGC_K_UPDATE_ON_TRIG ) {

        if ( xyo->y1AxisStyle[yi] == XYGC_K_AXIS_STYLE_LOG10 ) {
          dyValue = loc_log10(  dyValue  );
        }

	if ( !ctl ) {

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

        if ( xyo->y1AxisSource[yi] == XYGC_K_AUTOSCALE ) {
          if ( xyo->kpY1MinEfDouble[yi].isNull() ) {
            if ( dyValue < xyo->curY1Min[yi] ) {
              xyo->needY1Rescale[yi] = 1;
              xyo->y1RescaleValue[yi] = dyValue;
              xyo->actWin->addDefExeNode( xyo->aglPtr );
            }
          }
          if ( xyo->kpY1MaxEfDouble[yi].isNull() ) {
            if ( dyValue > xyo->curY1Max[yi] ) {
              xyo->needY1Rescale[yi] = 1;
              xyo->y1RescaleValue[yi] = dyValue;
              xyo->actWin->addDefExeNode( xyo->aglPtr );
            }
          }
        }

	}

        scaledY = xyo->plotAreaH -
         rint( ( dyValue - xyo->curY1Min[yi] ) *
         xyo->y1Factor[yi][i] - xyo->y1Offset[yi][i] );
        scaledY = dclamp( scaledY );

        if ( xyo->y1AxisStyle[yi] == XYGC_K_AXIS_STYLE_LOG10 ) {
          yZero = xyo->plotAreaH + xyo->y1Offset[yi][i];
        }
        else {
          yZero = xyo->plotAreaH -
           rint( ( 0.0 - xyo->curY1Min[yi] ) *
           xyo->y1Factor[yi][i] - xyo->y1Offset[yi][i] );
          yZero = dclamp( yZero );
        }

        scaledX = rint( ( dxValue - xyo->curXMin ) *
         xyo->xFactor[i] + xyo->xOffset[i] );
        scaledX = dclamp( scaledX );

        xyo->addPoint( dxValue, scaledX, scaledY, yZero, i );

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
          xyo->arrayNumPoints[i] = xyo->count;
          xyo->needBufferScroll = 1;
          xyo->needThisbufScroll[i] = 1;
        }

        xyo->needUpdate = 1;
        xyo->xArrayNeedUpdate[i] = 1;
        xyo->yArrayNeedUpdate[i] = 1;
        xyo->actWin->addDefExeNode( xyo->aglPtr );

      }
      else {

        xyo->xPvCurValue[i] = dxValue;
        xyo->yPvCurValue[i] = dyValue;
        xyo->yArrayGotValue[i] = xyo->xArrayGotValue[i] = 1;

      }

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
int i, yi;

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

  axygo->y2Label.setRaw( axygo->eBuf->bufY2Label );

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
    if ( axygo->plotStyle[i] == XYGC_K_PLOT_STYLE_SINGLE_POINT ) {
      axygo->forceVector[i] = 1;
    }
    else {
      axygo->forceVector[i] = 0;
    }

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

    if ( ( !blankOrComment( axygo->eBuf->bufXPvName[i] ) ) &&
         ( !blankOrComment( axygo->eBuf->bufYPvName[i] ) ) ) {

      (axygo->numTraces)++;
      axygo->xPvExpStr[i].setRaw( axygo->eBuf->bufXPvName[i] );
      axygo->yPvExpStr[i].setRaw( axygo->eBuf->bufYPvName[i] );
      axygo->traceType[i] = XYGC_K_TRACE_XY;

    }
    else if ( !blankOrComment( axygo->eBuf->bufYPvName[i] ) ) {

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

    axygo->xSigned[i] = axygo->eBuf->bufXSigned[i];
    axygo->ySigned[i] = axygo->eBuf->bufYSigned[i];

    if ( !blankOrComment( axygo->eBuf->bufNPvName[i] ) ) {
      axygo->nPvExpStr[i].setRaw( axygo->eBuf->bufNPvName[i] );
    }
    else {
      axygo->nPvExpStr[i].setRaw( "" );
    }

  }

  axygo->xAxis = axygo->eBuf->bufXAxis;
  axygo->xAxisStyle = axygo->eBuf->bufXAxisStyle;
  axygo->xAxisSource = axygo->eBuf->bufXAxisSource;
  axygo->xMin = axygo->eBuf->bufXMin;
  axygo->xMax = axygo->eBuf->bufXMax;
  axygo->xAxisTimeFormat = axygo->eBuf->bufXAxisTimeFormat;

  for ( yi=0; yi<xyGraphClass::NUM_Y_AXES; yi++ ) {
    axygo->y1Axis[yi] = axygo->eBuf->bufY1Axis[yi];
    axygo->y1AxisStyle[yi] = axygo->eBuf->bufY1AxisStyle[yi];
    axygo->y1AxisSource[yi] = axygo->eBuf->bufY1AxisSource[yi];
    axygo->y1Min[yi] = axygo->eBuf->bufY1Min[yi];
    axygo->y1Max[yi] = axygo->eBuf->bufY1Max[yi];
  }

  axygo->border = axygo->eBuf->bufBorder;
  axygo->plotAreaBorder = axygo->eBuf->bufPlotAreaBorder;
  axygo->autoScaleBothDirections = axygo->eBuf->bufAutoScaleBothDirections;
  axygo->autoScaleTimerMs = axygo->eBuf->bufAutoScaleTimerMs;
  axygo->autoScaleThreshPct = axygo->eBuf->bufAutoScaleThreshPct;
  if ( axygo->autoScaleThreshPct.isNull() ) {
    axygo->autoScaleThreshFrac = 0.5;
  }
  else {
    axygo->autoScaleThreshFrac = 0.01 * axygo->autoScaleThreshPct.value();
  }

  axygo->traceCtlPvExpStr.setRaw( axygo->eBuf->bufTraceCtlPvName );
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
  axygo->xGridMode = axygo->eBuf->bufXGridMode;
  axygo->xAxisSmoothing = axygo->eBuf->bufXAxisSmoothing;

  for ( yi=0; yi<xyGraphClass::NUM_Y_AXES; yi++ ) {
    axygo->y1NumLabelIntervals[yi] = axygo->eBuf->bufY1NumLabelIntervals[yi];
    axygo->y1LabelGrid[yi] = axygo->eBuf->bufY1LabelGrid[yi];
    axygo->y1NumMajorPerLabel[yi] = axygo->eBuf->bufY1NumMajorPerLabel[yi];
    axygo->y1MajorGrid[yi] = axygo->eBuf->bufY1MajorGrid[yi];
    axygo->y1NumMinorPerMajor[yi] = axygo->eBuf->bufY1NumMinorPerMajor[yi];
    axygo->y1MinorGrid[yi] = axygo->eBuf->bufY1MinorGrid[yi];
    axygo->y1AnnotationFormat[yi] = axygo->eBuf->bufY1AnnotationFormat[yi];
    axygo->y1AnnotationPrecision[yi] =
     axygo->eBuf->bufY1AnnotationPrecision[yi];
    axygo->y1GridMode[yi] = axygo->eBuf->bufY1GridMode[yi];
    axygo->y1AxisSmoothing[yi] = axygo->eBuf->bufY1AxisSmoothing[yi];
  }

  // check for conflicts

  for ( i=0; i<axygo->numTraces; i++ ) {

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

    for ( yi=0; yi<xyGraphClass::NUM_Y_AXES; yi++ ) {

      if ( axygo->y1AxisStyle[yi] == XYGC_K_AXIS_STYLE_LOG10 ) {
        if ( axygo->y1Min[yi].value() <= 0.0 )
          axygo->y1Min[yi].setValue( 1.0 );
      }

      if ( axygo->y1Min[yi].value() >= axygo->y1Max[yi].value() ) {
        axygo->y1Max[yi].setValue( axygo->y1Min[yi].value() * 2.0 );
      }

      // in case y Min is 0
      if ( axygo->y1Min[yi].value() >= axygo->y1Max[yi].value() ) {
        axygo->y1Max[yi].setValue( axygo->y1Min[yi].value() + 1.0 );
      }

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

int i, yi;
struct tm ts;
time_t t1, t2;

  ts.tm_sec = ts.tm_min = ts.tm_hour = ts.tm_mday = ts.tm_mon = ts.tm_year = 
    ts.tm_wday = ts.tm_yday = ts.tm_isdst = 0;
  ts.tm_mday = 1;
  ts.tm_year = 70;
  t1 = mktime( &ts );
  ts.tm_year = 90;
  t2 = mktime( &ts );

  timeOffset = t2 - t1;

  name = new char[strlen("xyGraphClass")+1];
  strcpy( name, "xyGraphClass" );
  checkBaseClassVersion( activeGraphicClass::MAJOR_VERSION, name );

  plotMode = XYGC_K_PLOT_MODE_PLOT_N_STOP;
  count = 2;

  numTraces = 0;

  for ( i=0; i<XYGC_K_MAX_TRACES; i++ ) {
    xPv[i] = NULL;
    yPv[i] = NULL;
    nPv[i] = NULL;
    xPvData[i] = NULL;
    yPvData[i] = NULL;
    plotBuf[i] = NULL;
    plotBufSize[i] = 0;
    plotInfo[i] = NULL;
    plotInfoSize[i] = 0;
    forceVector[i] = 0;
    xSigned[i] = 0;
    ySigned[i] = 0;
    traceType[i] = XYGC_K_TRACE_XY;
    plotStyle[i] = XYGC_K_PLOT_STYLE_LINE;
    plotSymbolType[i] = XYGC_K_SYMBOL_TYPE_NONE;
    plotUpdateMode[i] = XYGC_K_UPDATE_ON_X_AND_Y;
    plotColor[i] = 0;
    lineThk[i] = 1;
    opMode[i] = XYGC_K_SCOPE_MODE;
    y2Scale[i] = 0;
    lineStyle[i] = LineSolid;
    needNPvConnect[i] = 0;
  }
  trigPv = NULL;
  resetPv = NULL;
  traceCtlPv = NULL;
  traceCtl = 0;

  pixmap = (Pixmap) NULL;

  resetMode = 0;
  xAxis = 1;
  xAxisStyle = XYGC_K_AXIS_STYLE_LINEAR;
  xAxisSource = XYGC_K_AUTOSCALE;
  xAxisTimeFormat = 0;
  curXMin = 1;
  curXMax = 2;
  curXNumLabelTicks = 2;

  for ( yi=0; yi<xyGraphClass::NUM_Y_AXES; yi++ ) {
    y1Axis[yi] = 1;
    y1AxisStyle[yi] = XYGC_K_AXIS_STYLE_LINEAR;
    y1AxisSource[yi] = XYGC_K_AUTOSCALE;
    y1FormatType[yi] = 0;
    strcpy( y1Format[yi], "f" );
    curY1Min[yi] = 1;
    curY1Max[yi] = 2;
    curY1NumLabelTicks[yi] = 2;
  }

  xFormatType = 0;
  strcpy( xFormat, "f" );

  border = 1;
  plotAreaBorder = 0;
  autoScaleBothDirections = 0;
  autoScaleTimerMs.setNull(1);
  autoScaleThreshPct.setNull(1);
  autoScaleThreshFrac = 1;

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
  xGridMode = 0; // not user specified
  xAxisSmoothing = 0; // XYGC_K_SMOOTHING

  for ( yi=0; yi<xyGraphClass::NUM_Y_AXES; yi++ ) {
    y1NumLabelIntervals[yi].setNull(1);
    y1LabelGrid[yi] = 0;
    y1NumMajorPerLabel[yi].setNull(1);
    y1MajorGrid[yi] = 0;
    y1NumMinorPerMajor[yi].setNull(1);
    y1MinorGrid[yi] = 0;
    y1AnnotationPrecision[yi].setNull(1);
    y1AnnotationFormat[yi] = 0;
    y1GridMode[yi] = 0; // not user specified
    y1AxisSmoothing[yi] = 0; // XYGC_K_SMOOTHING
  }

  updateTimerValue = 0;

  eBuf = NULL;

  msgDialogPopedUp = 0;

}

// copy constructor
xyGraphClass::xyGraphClass
 ( const xyGraphClass *source ) {

activeGraphicClass *ago = (activeGraphicClass *) this;
int i, yi;

  ago->clone( (activeGraphicClass *) source );

  name = new char[strlen("xyGraphClass")+1];
  strcpy( name, "xyGraphClass" );

  timeOffset = source->timeOffset;

  graphTitle.copy( source->graphTitle );
  xLabel.copy( source->xLabel );
  yLabel.copy( source->yLabel );
  y2Label.copy( source->y2Label );

  fgCb = source->fgCb;
  bgCb = source->bgCb;
  gridCb = source->gridCb;

  fgColor = source->fgColor;
  bgColor = source->bgColor;
  gridColor = source->gridColor;

  plotMode = source->plotMode;
  count = source->count;
  border = source->border;
  plotAreaBorder = source->plotAreaBorder;
  autoScaleBothDirections = source->autoScaleBothDirections;
  autoScaleTimerMs = source->autoScaleTimerMs;
  autoScaleThreshPct = source->autoScaleThreshPct;

  numTraces = source->numTraces;

  for ( i=0; i<XYGC_K_MAX_TRACES; i++ ) {
    xSigned[i] = source->xSigned[i];
    ySigned[i] = source->ySigned[i];
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
    nPvExpStr[i].copy( source->nPvExpStr[i] );
    xPv[i] = NULL;
    yPv[i] = NULL;
    nPv[i] = NULL;
    xPvData[i] = NULL;
    yPvData[i] = NULL;
    plotBuf[i] = NULL;
    plotBufSize[i] = 0;
    plotInfo[i] = NULL;
    plotInfoSize[i] = 0;
    forceVector[i] = 0;
    needNPvConnect[i] = 0;
  }

  trigPv = NULL;
  trigPvExpStr.copy( source->trigPvExpStr );

  pixmap = (Pixmap) NULL;

  traceCtlPv = NULL;
  traceCtlPvExpStr.copy( source->traceCtlPvExpStr );
  traceCtl = 0;

  resetPv = NULL;
  resetPvExpStr.copy( source->resetPvExpStr );
  resetMode = source->resetMode;

  xAxis = source->xAxis;
  xAxisStyle = source->xAxisStyle;
  xAxisSource = source->xAxisSource;
  xMin = source->xMin;
  xMax = source->xMax;
  xAxisTimeFormat = source->xAxisTimeFormat;
  curXMin = 1;
  curXMax = 2;
  curXNumLabelTicks = 2;

  for ( yi=0; yi<xyGraphClass::NUM_Y_AXES; yi++ ) {
    y1Axis[yi] = source->y1Axis[yi];
    y1AxisStyle[yi] = source->y1AxisStyle[yi];
    y1AxisSource[yi] = source->y1AxisSource[yi];
    y1Min[yi] = source->y1Min[yi];
    y1Max[yi] = source->y1Max[yi];
    y1FormatType[yi] = source->y1FormatType[yi];
    strncpy( y1Format[yi], source->y1Format[yi], 15 );
    curY1Min[yi] = 1;
    curY1Max[yi] = 2;
    curY1NumLabelTicks[yi] = 2;
  }

  xFormatType = source->xFormatType;
  strncpy( xFormat, source->xFormat, 15 );

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
  xGridMode = source->xGridMode;
  xAxisSmoothing = source->xAxisSmoothing;

  for ( yi=0; yi<xyGraphClass::NUM_Y_AXES; yi++ ) {
    y1NumLabelIntervals[yi] = source->y1NumLabelIntervals[yi];
    y1LabelGrid[yi] = source->y1LabelGrid[yi];
    y1NumMajorPerLabel[yi] = source->y1NumMajorPerLabel[yi];
    y1MajorGrid[yi] = source->y1MajorGrid[yi];
    y1NumMinorPerMajor[yi] = source->y1NumMinorPerMajor[yi];
    y1MinorGrid[yi] = source->y1MinorGrid[yi];
    y1AnnotationPrecision[yi] = source->y1AnnotationPrecision[yi];
    y1AnnotationFormat[yi] = source->y1AnnotationFormat[yi];
    y1GridMode[yi] = source->y1GridMode[yi];
    y1AxisSmoothing[yi] = source->y1AxisSmoothing[yi];
  }

  connection.setMaxPvs( 2 * XYGC_K_MAX_TRACES + 2 );

  updateTimerValue = source->updateTimerValue;

  eBuf = NULL;

  msgDialogPopedUp = 0;

  actWin->fi->loadFontTag( fontTag );
  actWin->drawGc.setFontTag( fontTag, actWin->fi );

  doAccSubs( graphTitle );
  doAccSubs( xLabel );
  doAccSubs( yLabel );
  doAccSubs( y2Label );
  doAccSubs( traceCtlPvExpStr );
  doAccSubs( trigPvExpStr );
  doAccSubs( resetPvExpStr );
  for ( i=0; i<XYGC_K_MAX_TRACES; i++ ) {
    doAccSubs( xPvExpStr[i] );
    doAccSubs( yPvExpStr[i] );
    doAccSubs( nPvExpStr[i] );
  }

  updateDimensions();

}

xyGraphClass::~xyGraphClass ( void ) {

  if ( name ) delete[] name;
  if ( eBuf ) delete eBuf;

}

int xyGraphClass::yPvEffCount (
 int i
) {

size_t cnt;
  
  cnt = yPv[i]->get_count();
  
  if ( nPv[i] ) {
    if ( nPv[i]->is_valid() ) {
      cnt = traceSize[i];
      if ( cnt <= 0 ) cnt = yPv[i]->get_count();
      if ( cnt > yPv[i]->get_count() ) cnt = yPv[i]->get_count();
    }
  }

  return (int) cnt;
  
}

int xyGraphClass::xPvEffCount (
 int i
) {

size_t cnt;

  cnt = xPv[i]->get_count();
  
  if ( nPv[i] ) {
    if ( nPv[i]->is_valid() ) {
      cnt = traceSize[i];
      if ( cnt <= 0 ) cnt = xPv[i]->get_count();
      if ( cnt > xPv[i]->get_count() ) cnt = xPv[i]->get_count();
    }
  }

  return (int) cnt;
  
}

int xyGraphClass::getDbXMinXMax (
  double *min,
  double *max
) {

int i, start, allChronological;
int ctl;

  *min = 0;
  *max = 1;
  allChronological = 1;

  i = 0;
  start = numTraces;
  while ( i<numTraces ) {

    ctl = (int) traceCtl & ( 1 << i );

    if ( traceType[i] != XYGC_K_TRACE_CHRONOLOGICAL ) {
      allChronological = 0;
      if ( !ctl ) {
        *min = dbXMin[i];
        *max = dbXMax[i];
      }
      start = i+1;
      break;
    }

    i++;

  }

  for ( i=start; i<numTraces; i++ ) {
    ctl = (int) traceCtl & ( 1 << i );
    if ( traceType[i] != XYGC_K_TRACE_CHRONOLOGICAL ) {
      allChronological = 0;
      if ( !ctl ) {
        if ( dbXMin[i] < *min ) *min = dbXMin[i];
        if ( dbXMax[i] > *max ) *max = dbXMax[i];
      }
    }
  }

  return allChronological;

}

void xyGraphClass::getDbYMinYMax (
  double *min,
  double *max,
  int yi
) {

int i, start;
int ctl;

  *min = 0;
  *max = 1;

  if ( yi == 0 ) {

    i = 0;
    start = numTraces;
    while ( i<numTraces ) {

      ctl = (int) traceCtl & ( 1 << i );

      if ( !y2Scale[i] ) {
	if ( !ctl ) {
          *min = dbYMin[i];
          *max = dbYMax[i];
	}
        start = i+1;
        break;
      }

      i++;

    }

    for ( i=start; i<numTraces; i++ ) {
      ctl = (int) traceCtl & ( 1 << i );
      if ( !y2Scale[i] ) {
        if ( !ctl ) {
          if ( dbYMin[i] < *min ) *min = dbYMin[i];
          if ( dbYMax[i] > *max ) *max = dbYMax[i];
	}
      }
    }

  }
  else {

    i = 0;
    start = numTraces;
    while ( i<numTraces ) {

      ctl = (int) traceCtl & ( 1 << i );

      if ( y2Scale[i] ) {
	if ( !ctl ) {
          *min = dbYMin[i];
          *max = dbYMax[i];
	}
        start = i+1;
        break;
      }

      i++;

    }

    for ( i=start; i<numTraces; i++ ) {
      ctl = (int) traceCtl & ( 1 << i );
      if ( y2Scale[i] ) {
	if ( !ctl ) {
          if ( dbYMin[i] < *min ) *min = dbYMin[i];
          if ( dbYMax[i] > *max ) *max = dbYMax[i];
	}
      }
    }

  }

}

void xyGraphClass::getXMinMax (
  double *min,
  double *max
) {

int i, ii, first;
double dxValue;
int ctl;

  *min = *max = 0;

  first = 1;
  for ( i=0; i<numTraces; i++ ) {

    ctl = (int) traceCtl & ( 1 << i );

    if ( !ctl ) {

      ii = arrayHead[i];
      while ( ii != arrayTail[i] ) {

        if ( traceType[i] == XYGC_K_TRACE_CHRONOLOGICAL ) {

          dxValue = ( (double *) xPvData[i] )[ii];

        }
        else {
  
          // There are two views of pv types, Type and specificType; this uses
          // specificType
          switch ( xPvType[i] ) {
          case ProcessVariable::specificType::flt:
            dxValue = (double) ( (float *) xPvData[i] )[ii];
            break;
          case ProcessVariable::specificType::real: 
            dxValue = ( (double *) xPvData[i] )[ii];
            break;
          case ProcessVariable::specificType::shrt:
            if ( xSigned[i] ) {
              dxValue = (double) ( (short *) xPvData[i] )[ii];
            }
            else {
              dxValue = (double) ( (unsigned short *) xPvData[i] )[ii];
            }
            break;
          case ProcessVariable::specificType::chr:
            if ( xSigned[i] ) {
              dxValue = (double) ( (char *) xPvData[i] )[ii];
            }
            else {
              dxValue = (double) ( (unsigned char *) xPvData[i] )[ii];
            }
            break;
          case ProcessVariable::specificType::integer:
            if ( xSigned[i] ) {
              dxValue = (double) ( (int *) xPvData[i] )[ii];
            }
            else {
              dxValue = (double) ( (unsigned int *) xPvData[i] )[ii];
            }
            break;
          case ProcessVariable::specificType::enumerated:
            if ( xSigned[i] ) {
              dxValue = (double) ( (short *) xPvData[i] )[ii];
            }
            else {
              dxValue = (double) ( (unsigned short *) xPvData[i] )[ii];
            }
            break;
          default:
            dxValue = ( (double *) xPvData[i] )[ii];
            break;
          }

        }

        if ( first ) {
          first = 0;
          *min = *max = dxValue;
        }
        else {
          if ( dxValue < *min ) *min = dxValue;
          if ( dxValue > *max ) *max = dxValue;
        }

        ii++;
        if ( ii > plotBufSize[i] ) {
          ii = 0;
        }

      }

    }

  }

}

void xyGraphClass::getYMinMax (
  int yi,
  double min[],
  double max[]
) {

int i, ii, first[NUM_Y_AXES];
double dyValue[NUM_Y_AXES];
int ctl;

  for ( i=0; i<NUM_Y_AXES; i++ ) {
    first[i] = 1;
    min[i] = max[i] = 0;
  }

  for ( i=0; i<numTraces; i++ ) {

    ctl = (int) traceCtl & ( 1 << i );

    if ( !ctl ) {

    if ( ( ( yi == 0 ) && !y2Scale[i] ) ||
         ( ( yi > 0 ) && y2Scale[i] ) ) {

      ii = arrayHead[i];
      while ( ii != arrayTail[i] ) {

        // There are two views of pv types, Type and specificType; this uses
        // specificType
        switch ( yPvType[i] ) {
        case ProcessVariable::specificType::flt:
          dyValue[yi] = (double) ( (float *) yPvData[i] )[ii];
          break;
        case ProcessVariable::specificType::real: 
          dyValue[yi] = ( (double *) yPvData[i] )[ii];
          break;
        case ProcessVariable::specificType::shrt:
          if ( ySigned[i] ) {
            dyValue[yi] = (double) ( (short *) yPvData[i] )[ii];
          }
          else {
            dyValue[yi] = (double) ( (unsigned short *) yPvData[i] )[ii];
          }
          break;
        case ProcessVariable::specificType::chr:
          if ( ySigned[i] ) {
            dyValue[yi] = (double) ( (char *) yPvData[i] )[ii];
          }
          else {
            dyValue[yi] = (double) ( (unsigned char *) yPvData[i] )[ii];
          }
          break;
        case ProcessVariable::specificType::integer:
          if ( ySigned[i] ) {
            dyValue[yi] = (double) ( (int *) yPvData[i] )[ii];
          }
          else {
            dyValue[yi] = (double) ( (unsigned int *) yPvData[i] )[ii];
          }
          break;
        case ProcessVariable::specificType::enumerated:
          if ( ySigned[i] ) {
            dyValue[yi] = (double) ( (short *) yPvData[i] )[ii];
          }
          else {
            dyValue[yi] = (double) ( (unsigned short *) yPvData[i] )[ii];
          }
          break;
        default:
          dyValue[yi] = ( (double *) yPvData[i] )[ii];
          break;
        }

        if ( first[yi] ) {
          first[yi] = 0;
          min[yi] = max[yi] = dyValue[yi];
        }
        else {
          if ( dyValue[yi] < min[yi] ) min[yi] = dyValue[yi];
          if ( dyValue[yi] > max[yi] ) max[yi] = dyValue[yi];
        }

        ii++;
        if ( ii > plotBufSize[i] ) {
          ii = 0;
        }

      }

    }

    }

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
    forceVector[i] = 0;
    opMode[i] = XYGC_K_SCOPE_MODE;
    y2Scale[i] = 0;
    plotUpdateMode[i] = XYGC_K_UPDATE_ON_X_AND_Y;
    plotColor[i] = actWin->ci->colorIndexByAlias( traceColor );
    lineThk[i] = 1;
    lineStyle[i] = LineSolid;
    xSigned[i] = 0;
    ySigned[i] = 0;
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

int stat, major, minor, release;

tagClass tag;

int zero = 0;
int one = 1;
char *emptyStr = "";

int plotModePlotNPtsAndStop = 0;
static char *plotModeEnumStr[2] = {
  "plotNPtsAndStop",
  "plotLastNPts"
};
static int plotModeEnum[2] = {
  0,
  1
};

int opModeScope = 0;
static char *opModeEnumStr[2] = {
  "scope",
  "plot"
};
static int opModeEnum[2] = {
  0,
  1
};

int plotStyleLine = 0;
static char *plotStyleEnumStr[4] = {
  "line",
  "point",
  "needle",
  "single point"
};
static int plotStyleEnum[4] = {
  0,
  1,
  2,
  3
};

int updateModexAndY = 0;
static char *updateModeEnumStr[5] = {
  "xAndY",
  "xOrY",
  "x",
  "y",
  "trigger"
};
static int updateModeEnum[5] = {
  0,
  1,
  2,
  3,
  4
};

int styleSolid = LineSolid;
static char *styleEnumStr[2] = {
  "solid",
  "dash"
};
static int styleEnum[2] = {
  LineSolid,
  LineOnOffDash
};

int symbolNone = 0;
static char *symbolEnumStr[4] = {
  "none",
  "circle",
  "square",
  "diamond"
};
static int symbolEnum[4] = {
  0,
  1,
  2,
  3
};

int xAxisStyleLinear = 0;
static char *xAxisStyleEnumStr[4] = {
  "linear",
  "log10",
  "time",
  "loc_log10(time)"
};
static int xAxisStyleEnum[4] = {
  0,
  1,
  2,
  3
};

int yAxisStyleLinear = 0;
static char *yAxisStyleEnumStr[2] = {
  "linear",
  "log10"
};
static int yAxisStyleEnum[2] = {
  0,
  1
};

int axisSrcFromPv = 0;
static char *axisSrcEnumStr[3] = {
  "fromPv",
  "fromUser",
  "AutoScale"
};
static int axisSrcEnum[3] = {
  0,
  1,
  2
};

int timeFormatSec = 0;
static char *timeFormatEnumStr[2] = {
  "seconds",
  "dateTime"
};
static int timeFormatEnum[2] = {
  0,
  1
};

int annoFormatF = 0;
static char *annoFormatEnumStr[2] = {
  "f",
  "g"
};
static int annoFormatEnum[2] = {
  0,
  1
};

int resetModeIfNotZero = 0;
static char *resetModeEnumStr[2] = {
  "ifNotZero",
  "ifZero"
};
static int resetModeEnum[2] = {
  0,
  1
};

  major = XYGC_MAJOR_VERSION;
  minor = XYGC_MINOR_VERSION;
  release = XYGC_RELEASE;

  tag.init();
  tag.loadW( "beginObjectProperties" );
  tag.loadW( "major", &major );
  tag.loadW( "minor", &minor );
  tag.loadW( "release", &release );

  tag.loadW( "# Geometry" );
  tag.loadW( "x", &x );
  tag.loadW( "y", &y );
  tag.loadW( "w", &w );
  tag.loadW( "h", &h );

  tag.loadW( "# Appearance" );
  tag.loadBoolW( "border", &border, &zero );
  tag.loadBoolW( "plotAreaBorder", &plotAreaBorder, &zero );
  tag.loadBoolW( "autoScaleBothDirections", &autoScaleBothDirections, &zero );
  tag.loadW( "autoScaleUpdateMs", &autoScaleTimerMs );
  tag.loadW( "autoScaleThreshPct", &autoScaleThreshPct );
  tag.loadW( "graphTitle", &graphTitle, emptyStr );
  tag.loadW( "xLabel", &xLabel, emptyStr );
  tag.loadW( "yLabel", &yLabel, emptyStr );
  tag.loadW( "y2Label", &y2Label, emptyStr );
  tag.loadW( "fgColor", actWin->ci, &fgColor );
  tag.loadW( "bgColor", actWin->ci, &bgColor );
  tag.loadW( "gridColor", actWin->ci, &gridColor );
  tag.loadW( "font", fontTag );

  tag.loadW( "# Operating Modes" );
  tag.loadW( "plotMode", 2, plotModeEnumStr, plotModeEnum, &plotMode,
   &plotModePlotNPtsAndStop );
  tag.loadW( "nPts", &count );
  tag.loadW( "updateTimerMs", &updateTimerValue, &zero );
  tag.loadW( "traceCtlPv", &traceCtlPvExpStr, emptyStr );
  tag.loadW( "triggerPv", &trigPvExpStr, emptyStr );
  tag.loadW( "resetPv", &resetPvExpStr, emptyStr );
  tag.loadW( "resetMode", 2, resetModeEnumStr, resetModeEnum, &resetMode,
   &resetModeIfNotZero );

  tag.loadW( "# X axis properties" );
  tag.loadBoolW( "showXAxis", &xAxis, &zero );
  tag.loadW( "xAxisStyle", 4, xAxisStyleEnumStr, xAxisStyleEnum, &xAxisStyle,
   &xAxisStyleLinear );
  tag.loadW( "xAxisSrc", 3, axisSrcEnumStr, axisSrcEnum, &xAxisSource,
   &axisSrcFromPv );
  tag.loadW( "xMin", &xMin );
  tag.loadW( "xMax", &xMax );
  tag.loadW( "xAxisTimeFormat", 2, timeFormatEnumStr, timeFormatEnum,
   &xAxisTimeFormat, &timeFormatSec );
  tag.loadW( "xLabelIntervals", &xNumLabelIntervals );
  tag.loadW( "xMajorsPerLabel", &xNumMajorPerLabel );
  tag.loadW( "xMinorsPerMajor", &xNumMinorPerMajor );
  tag.loadBoolW( "xShowLabelGrid", &xLabelGrid, &zero );
  tag.loadBoolW( "xShowMajorGrid", &xMajorGrid, &zero );
  tag.loadBoolW( "xShowMinorGrid", &xMinorGrid, &zero );
  tag.loadW( "xLableFormat", 2, annoFormatEnumStr, annoFormatEnum,
   &xAnnotationFormat, &annoFormatF );
  tag.loadW( "xLablePrecision", &xAnnotationPrecision );
  tag.loadW( "xUserSpecScaleDiv", &xGridMode, &zero );
  tag.loadW( "xAxisSmoothing", &xAxisSmoothing, &zero );

  tag.loadW( "# Y axis properties" );
  tag.loadBoolW( "showYAxis", &y1Axis[0], &zero );
  tag.loadW( "yAxisStyle", 2, yAxisStyleEnumStr, yAxisStyleEnum,
   &y1AxisStyle[0], &yAxisStyleLinear );
  tag.loadW( "yAxisSrc", 3, axisSrcEnumStr, axisSrcEnum, &y1AxisSource[0],
   &axisSrcFromPv );
  tag.loadW( "yMin", &y1Min[0] );
  tag.loadW( "yMax", &y1Max[0] );
  tag.loadW( "yLabelIntervals", &y1NumLabelIntervals[0] );
  tag.loadW( "yMajorsPerLabel", &y1NumMajorPerLabel[0] );
  tag.loadW( "yMinorsPerMajor", &y1NumMinorPerMajor[0] );
  tag.loadBoolW( "yShowLabelGrid", &y1LabelGrid[0], &zero );
  tag.loadBoolW( "yShowMajorGrid", &y1MajorGrid[0], &zero );
  tag.loadBoolW( "yShowMinorGrid", &y1MinorGrid[0], &zero );
  tag.loadW( "yAxisFormat", 2, annoFormatEnumStr, annoFormatEnum,
   &y1AnnotationFormat[0], &annoFormatF );
  tag.loadW( "yAxisPrecision", &y1AnnotationPrecision[0] );
  tag.loadW( "yUserSpecScaleDiv", &y1GridMode[0], &zero );
  tag.loadW( "yAxisSmoothing", &y1AxisSmoothing[0], &zero );

  tag.loadW( "# Y2 axis properties" );
  tag.loadBoolW( "showY2Axis", &y1Axis[1], &zero );
  tag.loadW( "y2AxisStyle", 2, yAxisStyleEnumStr, yAxisStyleEnum,
   &y1AxisStyle[1], &yAxisStyleLinear );
  tag.loadW( "y2AxisSrc", 3, axisSrcEnumStr, axisSrcEnum, &y1AxisSource[1],
   &axisSrcFromPv );
  tag.loadW( "y2Min", &y1Min[1] );
  tag.loadW( "y2Max", &y1Max[1] );
  tag.loadW( "y2LabelIntervals", &y1NumLabelIntervals[1] );
  tag.loadW( "y2MajorsPerLabel", &y1NumMajorPerLabel[1] );
  tag.loadW( "y2MinorsPerMajor", &y1NumMinorPerMajor[1] );
  tag.loadBoolW( "y2ShowLabelGrid", &y1LabelGrid[1], &zero );
  tag.loadBoolW( "y2ShowMajorGrid", &y1MajorGrid[1], &zero );
  tag.loadBoolW( "y2ShowMinorGrid", &y1MinorGrid[1], &zero );
  tag.loadW( "y2AxisFormat", 2, annoFormatEnumStr, annoFormatEnum,
   &y1AnnotationFormat[1], &annoFormatF );
  tag.loadW( "y2AxisPrecision", &y1AnnotationPrecision[1] );
  tag.loadW( "y2UserSpecScaleDiv", &y1GridMode[1], &zero );
  tag.loadW( "y2AxisSmoothing", &y1AxisSmoothing[1], &zero );

  // trace properties (arrays)
  tag.loadW( "# Trace Properties" );
  tag.loadW( "numTraces", &numTraces );
  tag.loadW( "xPv", xPvExpStr, numTraces, emptyStr );
  tag.loadW( "yPv", yPvExpStr, numTraces, emptyStr );
  tag.loadW( "nPv", nPvExpStr, numTraces, emptyStr );
  tag.loadW( "plotStyle", 4, plotStyleEnumStr, plotStyleEnum, plotStyle,
   numTraces, &plotStyleLine );
  tag.loadW( "lineThickness", lineThk, numTraces, &one );
  tag.loadW( "lineStyle", 2, styleEnumStr, styleEnum, lineStyle, numTraces,
   &styleSolid );
  tag.loadW( "plotUpdateMode", 5, updateModeEnumStr, updateModeEnum,
   plotUpdateMode, numTraces, &updateModexAndY );
  tag.loadW( "plotSymbolType", 4, symbolEnumStr, symbolEnum, plotSymbolType,
   numTraces, &symbolNone );
  tag.loadW( "opMode", 2, opModeEnumStr, opModeEnum, opMode, numTraces,
   &opModeScope );
  tag.loadW( "useY2Axis", y2Scale, numTraces, &zero );
  tag.loadW( "xSigned", xSigned, numTraces, &zero );
  tag.loadW( "ySigned", ySigned, numTraces, &zero );
  tag.loadW( "plotColor", actWin->ci, plotColor, numTraces );

  tag.loadW( unknownTags );
  tag.loadW( "endObjectProperties" );
  tag.loadW( "" );

  stat = tag.writeTags( f );

  return stat;

}

int xyGraphClass::old_save (
  FILE *f )
{

int i, yi, stat = 1;
efDouble dummy;

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

  for ( yi=0; yi<2; yi++ ) {
    fprintf( f, "%-d\n", y1Axis[yi] );
    fprintf( f, "%-d\n", y1AxisStyle[yi] );
    fprintf( f, "%-d\n", y1AxisSource[yi] );
    stat = y1Min[yi].write( f );
    stat = y1Max[yi].write( f );
  }

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

  for ( yi=0; yi<2; yi++ ) {
    y1NumLabelIntervals[yi].write( f );
    fprintf( f, "%-d\n", y1LabelGrid[yi] );
    y1NumMajorPerLabel[yi].write( f );
    fprintf( f, "%-d\n", y1MajorGrid[yi] );
    y1NumMinorPerMajor[yi].write( f );
    fprintf( f, "%-d\n", y1MinorGrid[yi] );
    fprintf( f, "%-d\n", y1AnnotationFormat[yi] );
    y1AnnotationPrecision[yi].write( f );
  }

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
    fprintf( f, "%-d\n", xSigned[i] );
    fprintf( f, "%-d\n", ySigned[i] );

  }

  // at version 1.2 added scroll magnitude then in version 1.3
  // all strip chart functionality was removed so next write
  // will be dummy for backward compatibility

  dummy.write( f );

  // version 1.4.0
  if ( y2Label.getRaw() )
    writeStringToFile( f, y2Label.getRaw() );
  else
    writeStringToFile( f, "" );

  return stat;

}

int xyGraphClass::createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int major, minor, release, i, n, yi, stat;
char traceColor[15+1];

tagClass tag;

int zero = 0;
int one = 1;
char *emptyStr = "";

int plotModePlotNPtsAndStop = 0;
static char *plotModeEnumStr[2] = {
  "plotNPtsAndStop",
  "plotLastNPts"
};
static int plotModeEnum[2] = {
  0,
  1
};

int opModeScope = 0;
static char *opModeEnumStr[2] = {
  "scope",
  "plot"
};
static int opModeEnum[2] = {
  0,
  1
};

int plotStyleLine = 0;
static char *plotStyleEnumStr[4] = {
  "line",
  "point",
  "needle",
  "single point"
};
static int plotStyleEnum[4] = {
  0,
  1,
  2,
  3
};

int updateModexAndY = 0;
static char *updateModeEnumStr[5] = {
  "xAndY",
  "xOrY",
  "x",
  "y",
  "trigger"
};
static int updateModeEnum[5] = {
  0,
  1,
  2,
  3,
  4
};

int styleSolid = LineSolid;
static char *styleEnumStr[2] = {
  "solid",
  "dash"
};
static int styleEnum[2] = {
  LineSolid,
  LineOnOffDash
};

int symbolNone = 0;
static char *symbolEnumStr[4] = {
  "none",
  "circle",
  "square",
  "diamond"
};
static int symbolEnum[4] = {
  0,
  1,
  2,
  3
};

int xAxisStyleLinear = 0;
static char *xAxisStyleEnumStr[4] = {
  "linear",
  "log10",
  "time",
  "loc_log10(time)"
};
static int xAxisStyleEnum[4] = {
  0,
  1,
  2,
  3
};

int yAxisStyleLinear = 0;
static char *yAxisStyleEnumStr[2] = {
  "linear",
  "log10"
};
static int yAxisStyleEnum[2] = {
  0,
  1
};

int axisSrcFromPv = 0;
static char *axisSrcEnumStr[3] = {
  "fromPv",
  "fromUser",
  "AutoScale"
};
static int axisSrcEnum[3] = {
  0,
  1,
  2
};

int timeFormatSec = 0;
static char *timeFormatEnumStr[2] = {
  "seconds",
  "dateTime"
};
static int timeFormatEnum[2] = {
  0,
  1
};

int annoFormatF = 0;
static char *annoFormatEnumStr[2] = {
  "f",
  "g"
};
static int annoFormatEnum[2] = {
  0,
  1
};

int resetModeIfNotZero = 0;
static char *resetModeEnumStr[2] = {
  "ifNotZero",
  "ifZero"
};
static int resetModeEnum[2] = {
  0,
  1
};


  this->actWin = _actWin;

  tag.init();
  tag.loadR( "beginObjectProperties" );
  tag.loadR( unknownTags );
  tag.loadR( "major", &major );
  tag.loadR( "minor", &minor );
  tag.loadR( "release", &release );

  //tag.loadR( "# Geometry" );
  tag.loadR( "x", &x );
  tag.loadR( "y", &y );
  tag.loadR( "w", &w );
  tag.loadR( "h", &h );

  //tag.loadR( "# Appearance" );
  tag.loadR( "border", &border, &zero );
  tag.loadR( "plotAreaBorder", &plotAreaBorder, &zero );
  tag.loadR( "autoScaleBothDirections", &autoScaleBothDirections, &zero );
  tag.loadR( "autoScaleUpdateMs", &autoScaleTimerMs );
  tag.loadR( "autoScaleThreshPct", &autoScaleThreshPct );
  tag.loadR( "graphTitle", &graphTitle, emptyStr );
  tag.loadR( "xLabel", &xLabel, emptyStr );
  tag.loadR( "yLabel", &yLabel, emptyStr );
  tag.loadR( "y2Label", &y2Label, emptyStr );
  tag.loadR( "fgColor", actWin->ci, &fgColor );
  tag.loadR( "bgColor", actWin->ci, &bgColor );
  tag.loadR( "gridColor", actWin->ci, &gridColor );
  tag.loadR( "font", 63, fontTag );

  //tag.loadR( "# Operating Modes" );
  tag.loadR( "plotMode", 2, plotModeEnumStr, plotModeEnum, &plotMode,
   &plotModePlotNPtsAndStop );
  tag.loadR( "nPts", &count );
  tag.loadR( "updateTimerMs", &updateTimerValue, &zero );
  tag.loadR( "traceCtlPv", &traceCtlPvExpStr, emptyStr );
  tag.loadR( "triggerPv", &trigPvExpStr, emptyStr );
  tag.loadR( "resetPv", &resetPvExpStr, emptyStr );
  tag.loadR( "resetMode", 2, resetModeEnumStr, resetModeEnum, &resetMode,
   &resetModeIfNotZero );

  //tag.loadR( "# X axis properties" );
  tag.loadR( "showXAxis", &xAxis, &zero );
  tag.loadR( "xAxisStyle", 4, xAxisStyleEnumStr, xAxisStyleEnum, &xAxisStyle,
   &xAxisStyleLinear );
  tag.loadR( "xAxisSrc", 3, axisSrcEnumStr, axisSrcEnum, &xAxisSource,
   &axisSrcFromPv );
  tag.loadR( "xMin", &xMin );
  tag.loadR( "xMax", &xMax );
  tag.loadR( "xAxisTimeFormat", 2, timeFormatEnumStr, timeFormatEnum,
   &xAxisTimeFormat, &timeFormatSec );
  tag.loadR( "xLabelIntervals", &xNumLabelIntervals );
  tag.loadR( "xMajorsPerLabel", &xNumMajorPerLabel );
  tag.loadR( "xMinorsPerMajor", &xNumMinorPerMajor );
  tag.loadR( "xShowLabelGrid", &xLabelGrid, &zero );
  tag.loadR( "xShowMajorGrid", &xMajorGrid, &zero );
  tag.loadR( "xShowMinorGrid", &xMinorGrid, &zero );
  tag.loadR( "xLableFormat", 2, annoFormatEnumStr, annoFormatEnum,
   &xAnnotationFormat, &annoFormatF );
  tag.loadR( "xLablePrecision", &xAnnotationPrecision );
  tag.loadR( "xUserSpecScaleDiv", &xGridMode, &zero );
  tag.loadR( "xAxisSmoothing", &xAxisSmoothing, &zero );

  //tag.loadR( "# Y axis properties" );
  tag.loadR( "showYAxis", &y1Axis[0], &zero );
  tag.loadR( "yAxisStyle", 2, yAxisStyleEnumStr, yAxisStyleEnum,
   &y1AxisStyle[0], &yAxisStyleLinear );
  tag.loadR( "yAxisSrc", 3, axisSrcEnumStr, axisSrcEnum, &y1AxisSource[0],
   &axisSrcFromPv );
  tag.loadR( "yMin", &y1Min[0] );
  tag.loadR( "yMax", &y1Max[0] );
  tag.loadR( "yLabelIntervals", &y1NumLabelIntervals[0] );
  tag.loadR( "yMajorsPerLabel", &y1NumMajorPerLabel[0] );
  tag.loadR( "yMinorsPerMajor", &y1NumMinorPerMajor[0] );
  tag.loadR( "yShowLabelGrid", &y1LabelGrid[0], &zero );
  tag.loadR( "yShowMajorGrid", &y1MajorGrid[0], &zero );
  tag.loadR( "yShowMinorGrid", &y1MinorGrid[0], &zero );
  tag.loadR( "yAxisFormat", 2, annoFormatEnumStr, annoFormatEnum,
   &y1AnnotationFormat[0], &annoFormatF );
  tag.loadR( "yAxisPrecision", &y1AnnotationPrecision[0] );
  tag.loadR( "yUserSpecScaleDiv", &y1GridMode[0], &zero );
  tag.loadR( "yAxisSmoothing", &y1AxisSmoothing[0], &zero );

  //tag.loadR( "# Y2 axis properties" );
  tag.loadR( "showY2Axis", &y1Axis[1], &zero );
  tag.loadR( "y2AxisStyle", 2, yAxisStyleEnumStr, yAxisStyleEnum,
   &y1AxisStyle[1], &yAxisStyleLinear );
  tag.loadR( "y2AxisSrc", 3, axisSrcEnumStr, axisSrcEnum, &y1AxisSource[1],
   &axisSrcFromPv );
  tag.loadR( "y2Min", &y1Min[1] );
  tag.loadR( "y2Max", &y1Max[1] );
  tag.loadR( "y2LabelIntervals", &y1NumLabelIntervals[1] );
  tag.loadR( "y2MajorsPerLabel", &y1NumMajorPerLabel[1] );
  tag.loadR( "y2MinorsPerMajor", &y1NumMinorPerMajor[1] );
  tag.loadR( "y2ShowLabelGrid", &y1LabelGrid[1], &zero );
  tag.loadR( "y2ShowMajorGrid", &y1MajorGrid[1], &zero );
  tag.loadR( "y2ShowMinorGrid", &y1MinorGrid[1], &zero );
  tag.loadR( "y2AxisFormat", 2, annoFormatEnumStr, annoFormatEnum,
   &y1AnnotationFormat[1], &annoFormatF );
  tag.loadR( "y2AxisPrecision", &y1AnnotationPrecision[1] );
  tag.loadR( "y2UserSpecScaleDiv", &y1GridMode[1], &zero );
  tag.loadR( "y2AxisSmoothing", &y1AxisSmoothing[1], &zero );

  // trace properties (arrays)
  //tag.loadR( "# Trace Properties" );
  tag.loadR( "numTraces", &numTraces, &zero );
  tag.loadR( "xPv", XYGC_K_MAX_TRACES, xPvExpStr, &n, emptyStr );
  tag.loadR( "yPv", XYGC_K_MAX_TRACES, yPvExpStr, &n, emptyStr );
  tag.loadR( "nPv", XYGC_K_MAX_TRACES, nPvExpStr, &n, emptyStr );
  tag.loadR( "plotStyle", 4, plotStyleEnumStr, plotStyleEnum,
   XYGC_K_MAX_TRACES, plotStyle, &n, &plotStyleLine );
  tag.loadR( "lineThickness", XYGC_K_MAX_TRACES, lineThk, &n, &one );
  tag.loadR( "lineStyle", 2, styleEnumStr, styleEnum, XYGC_K_MAX_TRACES,
   lineStyle, &n, &styleSolid );
  tag.loadR( "plotUpdateMode", 5, updateModeEnumStr, updateModeEnum,
   XYGC_K_MAX_TRACES, plotUpdateMode, &n, &updateModexAndY );
  tag.loadR( "plotSymbolType", 4, symbolEnumStr, symbolEnum, XYGC_K_MAX_TRACES,
   plotSymbolType, &n, &symbolNone );
  tag.loadR( "opMode", 2, opModeEnumStr, opModeEnum, XYGC_K_MAX_TRACES,
   opMode, &n, &opModeScope );
  tag.loadR( "useY2Axis", XYGC_K_MAX_TRACES, y2Scale, &n, &zero );
  tag.loadR( "xSigned", XYGC_K_MAX_TRACES, xSigned, &n, &zero );
  tag.loadR( "ySigned", XYGC_K_MAX_TRACES, ySigned, &n, &zero );
  tag.loadR( "plotColor", actWin->ci, XYGC_K_MAX_TRACES, plotColor,
   &n );

  tag.loadR( "endObjectProperties" );

  stat = tag.readTags( f, "endObjectProperties" );

  if ( !( stat & 1 ) ) {
    actWin->appCtx->postMessage( tag.errMsg() );
  }

  if ( major > XYGC_MAJOR_VERSION ) {
    postIncompatable();
    return 0;
  }

  if ( major < 4 ) {
    postIncompatable();
    return 0;
  }

  this->initSelectBox(); // call after getting x,y,w,h

  if ( autoScaleThreshPct.isNull() ) {
    autoScaleThreshFrac = 0.5;
  }
  else {
    autoScaleThreshFrac = 0.01 * autoScaleThreshPct.value();
  }

  for ( i=0; i<numTraces; i++ ) {

    if ( plotStyle[i] == XYGC_K_PLOT_STYLE_SINGLE_POINT ) {
      forceVector[i] = 1;
    }
    else {
      forceVector[i] = 0;
    }

    if ( ( !blankOrComment( xPvExpStr[i].getRaw() ) ) &&
         ( !blankOrComment( yPvExpStr[i].getRaw() ) ) ) {
      traceType[i] = XYGC_K_TRACE_XY;
    }
    else if ( !blankOrComment( yPvExpStr[i].getRaw() ) ) {
      traceType[i] = XYGC_K_TRACE_CHRONOLOGICAL;
    }
    else {
      traceType[i] = XYGC_K_TRACE_INVALID;
    }

  }

  for ( i=numTraces; i<XYGC_K_MAX_TRACES; i++ ) {
    sprintf( traceColor, "trace%-d", i );
    plotColor[i] = actWin->ci->colorIndexByAlias( traceColor );
    lineThk[i] = 1;
    lineStyle[i] = LineSolid;
    plotUpdateMode[i] = XYGC_K_UPDATE_ON_X_AND_Y;
    plotSymbolType[i] = XYGC_K_SYMBOL_TYPE_NONE;
    opMode[i] = XYGC_K_SCOPE_MODE;
    y2Scale[i] = 0;
  }

  actWin->fi->loadFontTag( fontTag );
  actWin->drawGc.setFontTag( fontTag, actWin->fi );

  // check for conflicts

  for ( i=0; i<numTraces; i++ ) {

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

    for ( yi=0; yi<xyGraphClass::NUM_Y_AXES; yi++ ) {

      if ( y1AxisStyle[yi] == XYGC_K_AXIS_STYLE_LOG10 ) {
        if ( y1Min[yi].value() <= 0.0 ) y1Min[yi].setValue( 1.0 );
      }

      if ( y1Min[yi].value() >= y1Max[yi].value() ) {
        y1Max[yi].setValue( y1Min[yi].value() * 2.0 );
      }

      if ( y1Min[yi].value() >= y1Max[yi].value() ) { // in case y Min is 0
        y1Max[yi].setValue( y1Min[yi].value() + 1.0 );
      }

    }

  }

  updateDimensions();

  return stat;

}

int xyGraphClass::old_createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int i, yi;
int major, minor, release;
int stat = 1;
char str[127+1], traceColor[15+1], onePv[PV_Factory::MAX_PV_NAME+1];
efDouble dummy;

  this->actWin = _actWin;

  fscanf( f, "%d %d %d\n", &major, &minor, &release ); actWin->incLine();

  if ( major > XYGC_MAJOR_VERSION ) {
    postIncompatable();
    return 0;
  }

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

  for ( yi=0; yi<2; yi++ ) {
    fscanf( f, "%d\n", &y1Axis[yi] ); actWin->incLine();
    fscanf( f, "%d\n", &y1AxisStyle[yi] ); actWin->incLine();
    fscanf( f, "%d\n", &y1AxisSource[yi] ); actWin->incLine();
    stat = y1Min[yi].read( f ); actWin->incLine();
    stat = y1Max[yi].read( f ); actWin->incLine();
  }

  readStringFromFile( onePv, PV_Factory::MAX_PV_NAME+1, f );
   actWin->incLine();
  trigPvExpStr.setRaw( onePv );

  readStringFromFile( onePv, PV_Factory::MAX_PV_NAME+1, f );
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

  for ( yi=0; yi<2; yi++ ) {
    y1NumLabelIntervals[yi].read( f ); actWin->incLine();
    fscanf( f, "%d\n", &y1LabelGrid[yi] ); actWin->incLine();
    y1NumMajorPerLabel[yi].read( f ); actWin->incLine();
    fscanf( f, "%d\n", &y1MajorGrid[yi] ); actWin->incLine();
    y1NumMinorPerMajor[yi].read( f ); actWin->incLine();
    fscanf( f, "%d\n", &y1MinorGrid[yi] ); actWin->incLine();
    fscanf( f, "%d\n", &y1AnnotationFormat[yi] ); actWin->incLine();
    y1AnnotationPrecision[yi].read( f ); actWin->incLine();
  }

  actWin->ci->readColorIndex( f, &gridColor );
  actWin->incLine(); actWin->incLine();

  fscanf( f, "%d\n", &numTraces ); actWin->incLine();

  for ( i=0; i<numTraces; i++ ) {

    readStringFromFile( onePv, PV_Factory::MAX_PV_NAME+1, f );
     actWin->incLine();
    xPvExpStr[i].setRaw( onePv );

    readStringFromFile( onePv, PV_Factory::MAX_PV_NAME+1, f );
     actWin->incLine();
    yPvExpStr[i].setRaw( onePv );

    if ( ( !blankOrComment( xPvExpStr[i].getRaw() ) ) &&
         ( !blankOrComment( yPvExpStr[i].getRaw() ) ) ) {
      traceType[i] = XYGC_K_TRACE_XY;
    }
    else if ( !blankOrComment( yPvExpStr[i].getRaw() ) ) {
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

    if ( ( ( major == 1 ) && ( minor > 0 ) ) || ( major > 1 ) ) {
      fscanf( f, "%d\n", &xSigned[i] );
      fscanf( f, "%d\n", &ySigned[i] );
    }
    else {
      xSigned[i] = 0;
      ySigned[i] = 0;
    }

#if 0
    if ( lineStyle[i] == 0 ) {
      lineStyle[i] = LineSolid;
    }
    else {
      lineStyle[i] = LineOnOffDash;
    }
#endif

  }

  // at version 1.2 added scroll magnitude then in version 1.3
  // all strip chart functionality was removed so next read
  // will be dummy

  if ( ( ( major == 1 ) && ( minor > 1 ) ) || ( major > 1 ) ) {
    stat = dummy.read( f ); actWin->incLine();
  }

  if ( ( ( major == 1 ) && ( minor > 3 ) ) || ( major > 1 ) ) {
    readStringFromFile( str, 127+1, f ); actWin->incLine();
    y2Label.setRaw( str );
  }

  for ( i=numTraces; i<XYGC_K_MAX_TRACES; i++ ) {
    sprintf( traceColor, "trace%-d", i );
    plotColor[i] = actWin->ci->colorIndexByAlias( traceColor );
    lineThk[i] = 1;
    lineStyle[i] = LineSolid;
    plotUpdateMode[i] = XYGC_K_UPDATE_ON_X_AND_Y;
    plotSymbolType[i] = XYGC_K_SYMBOL_TYPE_NONE;
    opMode[i] = XYGC_K_SCOPE_MODE;
    y2Scale[i] = 0;
    forceVector[i] = 0;
  }

  actWin->fi->loadFontTag( fontTag );
  actWin->drawGc.setFontTag( fontTag, actWin->fi );

  // check for conflicts

  for ( i=0; i<numTraces; i++ ) {

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

    for ( yi=0; yi<xyGraphClass::NUM_Y_AXES; yi++ ) {

      if ( y1AxisStyle[yi] == XYGC_K_AXIS_STYLE_LOG10 ) {
        if ( y1Min[yi].value() <= 0.0 ) y1Min[yi].setValue( 1.0 );
      }

      if ( y1Min[yi].value() >= y1Max[yi].value() ) {
        y1Max[yi].setValue( y1Min[yi].value() * 2.0 );
      }

      if ( y1Min[yi].value() >= y1Max[yi].value() ) { // in case y Min is 0
        y1Max[yi].setValue( y1Min[yi].value() + 1.0 );
      }

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
int i, yi;

  ptr = actWin->obj.getNameFromClass( "xyGraphClass" );
  if ( ptr )
    strncpy( title, ptr, 31 );
  else
    strncpy( title, "Unknown object", 31 );

  Strncat( title, " Properties", 31 );

  if ( !eBuf ) {
    eBuf = new editBufType;
  }

  eBuf->bufX = x;
  eBuf->bufY = y;
  eBuf->bufW = w;
  eBuf->bufH = h;

  eBuf->bufPlotMode = plotMode;

  eBuf->bufBorder = border;

  eBuf->bufPlotAreaBorder = plotAreaBorder;
  eBuf->bufAutoScaleBothDirections = autoScaleBothDirections;
  eBuf->bufAutoScaleTimerMs = autoScaleTimerMs;
  eBuf->bufAutoScaleThreshPct = autoScaleThreshPct;

  eBuf->bufUpdateTimerValue = updateTimerValue;

  eBuf->bufCount = count;

  strncpy( eBuf->bufGraphTitle, graphTitle.getRaw(), 127 );
  eBuf->bufGraphTitle[127] = 0;
  strncpy( eBuf->bufXLabel, xLabel.getRaw(), 127 );
  eBuf->bufXLabel[127] = 0;
  strncpy( eBuf->bufYLabel, yLabel.getRaw(), 127 );
  eBuf->bufYLabel[127] = 0;
  strncpy( eBuf->bufY2Label, y2Label.getRaw(), 127 );
  eBuf->bufY2Label[127] = 0;
  eBuf->bufFgColor = fgColor;
  eBuf->bufBgColor = bgColor;
  eBuf->bufGridColor = gridColor;
  strncpy( eBuf->bufTraceCtlPvName, traceCtlPvExpStr.getRaw(),
   PV_Factory::MAX_PV_NAME );
  eBuf->bufTraceCtlPvName[PV_Factory::MAX_PV_NAME] = 0;
  strncpy( eBuf->bufTrigPvName, trigPvExpStr.getRaw(),
   PV_Factory::MAX_PV_NAME );
  eBuf->bufTrigPvName[PV_Factory::MAX_PV_NAME] = 0;
  strncpy( eBuf->bufResetPvName, resetPvExpStr.getRaw(),
   PV_Factory::MAX_PV_NAME );
  eBuf->bufResetPvName[PV_Factory::MAX_PV_NAME] = 0;
  eBuf->bufResetMode = resetMode;

  eBuf->bufXNumLabelIntervals = xNumLabelIntervals;
  eBuf->bufXLabelGrid = xLabelGrid;
  eBuf->bufXNumMajorPerLabel = xNumMajorPerLabel;
  eBuf->bufXMajorGrid = xMajorGrid;
  eBuf->bufXNumMinorPerMajor = xNumMinorPerMajor;
  eBuf->bufXMinorGrid = xMinorGrid;
  eBuf->bufXAnnotationFormat = xAnnotationFormat;
  eBuf->bufXAnnotationPrecision = xAnnotationPrecision;
  eBuf->bufXGridMode = xGridMode;
  eBuf->bufXAxisSmoothing = xAxisSmoothing;

  for ( yi=0; yi<xyGraphClass::NUM_Y_AXES; yi++ ) {
    eBuf->bufY1NumLabelIntervals[yi] = y1NumLabelIntervals[yi];
    eBuf->bufY1LabelGrid[yi] = y1LabelGrid[yi];
    eBuf->bufY1NumMajorPerLabel[yi] = y1NumMajorPerLabel[yi];
    eBuf->bufY1MajorGrid[yi] = y1MajorGrid[yi];
    eBuf->bufY1NumMinorPerMajor[yi] = y1NumMinorPerMajor[yi];
    eBuf->bufY1MinorGrid[yi] = y1MinorGrid[yi];
    eBuf->bufY1AnnotationFormat[yi] = y1AnnotationFormat[yi];
    eBuf->bufY1AnnotationPrecision[yi] = y1AnnotationPrecision[yi];
    eBuf->bufY1GridMode[yi] = y1GridMode[yi];
    eBuf->bufY1AxisSmoothing[yi] = y1AxisSmoothing[yi];
  }

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
  ef.addTextField( "Y2 Label", 35, eBuf->bufY2Label, 127 );
  ef.addColorButton( "Foreground", actWin->ci, &fgCb, &eBuf->bufFgColor );
  ef.addColorButton( "Background", actWin->ci, &bgCb, &eBuf->bufBgColor );
  ef.addColorButton( "Grid", actWin->ci, &gridCb, &eBuf->bufGridColor );
  ef.addOption( "Plot Mode", "plot n pts & stop|plot last n pts", &eBuf->bufPlotMode );
  ef.addTextField( "Count", 35, &eBuf->bufCount );
  ef.addTextField( "Update Delay (ms)", 35, &eBuf->bufUpdateTimerValue );
  ef.addToggle( "Border", &eBuf->bufBorder );
  ef.addToggle( "Plot Area Border", &eBuf->bufPlotAreaBorder );

  ef.addToggle( "Auto Scale Inward", &eBuf->bufAutoScaleBothDirections );
  scaleInwardEntry = ef.getCurItem();
  ef.addTextField( "Auto Scale Rate (ms)", 35, &eBuf->bufAutoScaleTimerMs );
  scaleInwardTimerEntry = ef.getCurItem();
  scaleInwardEntry->addDependency( scaleInwardTimerEntry );
  ef.addTextField( "Auto Scale Thresh (%)", 35, &eBuf->bufAutoScaleThreshPct );
  scaleInwardThreshEntry = ef.getCurItem();
  scaleInwardEntry->addDependency( scaleInwardThreshEntry );
  scaleInwardEntry->addDependencyCallbacks();

  ef.addEmbeddedEf( "X/Y/Trace Data", "... ", &efTrace );

    efTrace->create( actWin->top, actWin->appCtx->ci.getColorMap(),
     &actWin->appCtx->entryFormX,
     &actWin->appCtx->entryFormY, &actWin->appCtx->entryFormW,
     &actWin->appCtx->entryFormH, &actWin->appCtx->largestH,
     "Trace Properties", NULL, NULL, NULL );

    for ( i=0; i<numTraces; i++ ) {
      strncpy( eBuf->bufXPvName[i], xPvExpStr[i].getRaw(),
       PV_Factory::MAX_PV_NAME );
      eBuf->bufXPvName[i][PV_Factory::MAX_PV_NAME] = 0;
      strncpy( eBuf->bufYPvName[i], yPvExpStr[i].getRaw(),
       PV_Factory::MAX_PV_NAME );
      eBuf->bufYPvName[i][PV_Factory::MAX_PV_NAME] = 0;
      strncpy( eBuf->bufNPvName[i], nPvExpStr[i].getRaw(),
       PV_Factory::MAX_PV_NAME );
      eBuf->bufNPvName[i][PV_Factory::MAX_PV_NAME] = 0;
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
      eBuf->bufXSigned[i] = xSigned[i];
      eBuf->bufYSigned[i] = ySigned[i];
    }
    for ( i=numTraces; i<XYGC_K_MAX_TRACES; i++ ) {
      strcpy( eBuf->bufXPvName[i], "" );
      strcpy( eBuf->bufYPvName[i], "" );
      strcpy( eBuf->bufNPvName[i], "" );
      eBuf->bufPlotStyle[i] = XYGC_K_PLOT_STYLE_LINE;
      eBuf->bufPlotSymbolType[i] = XYGC_K_SYMBOL_TYPE_NONE;
      eBuf->bufPlotUpdateMode[i] = XYGC_K_UPDATE_ON_X_AND_Y;
      eBuf->bufPlotColor[i] = plotColor[i];
      eBuf->bufLineThk[i] = 0;
      eBuf->bufLineStyle[i] = LineSolid;
      eBuf->bufOpMode[i] = XYGC_K_SCOPE_MODE;
      eBuf->bufY2Scale[i] = 0;
      eBuf->bufXSigned[i] = 0;
      eBuf->bufYSigned[i] = 0;
    }

    i = 0;
    efTrace->beginSubForm();
    efTrace->addTextField( "X ", 20, eBuf->bufXPvName[i],
     PV_Factory::MAX_PV_NAME );
    efTrace->addLabel( "  S " );
    efTrace->addToggle( " ", &eBuf->bufXSigned[i] );
    efTrace->addLabel( "Y " );
    efTrace->addTextField( "", 20, eBuf->bufYPvName[i],
     PV_Factory::MAX_PV_NAME );
    efTrace->addLabel( "  S " );
    efTrace->addToggle( " ", &eBuf->bufYSigned[i] );
    efTrace->addLabel( "N " );
    efTrace->addTextField( "", 15, eBuf->bufNPvName[i],
     PV_Factory::MAX_PV_NAME );
    efTrace->addOption( "", "scope|plot", &eBuf->bufOpMode[i] );
    efTrace->addLabel( "  Y2" );
    efTrace->addToggle( " ", &eBuf->bufY2Scale[i] );
    //efTrace->addLabel( "  Style" );
    efTrace->addOption( "", "line|point|needle|single point", &eBuf->bufPlotStyle[i] );
    efTrace->addLabel( "  Update" );
    efTrace->addOption( "", "X and Y|X or Y|X|Y|Trigger",
     &eBuf->bufPlotUpdateMode[i] );
    efTrace->addLabel( "  Thk" );
    efTrace->addOption( "", "1|2|3|4|5|6|7|8|9", &eBuf->bufLineThk[i] );
    //efTrace->addLabel( "  Line" );
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
       PV_Factory::MAX_PV_NAME );
      efTrace->addLabel( "  S " );
      efTrace->addToggle( " ", &eBuf->bufXSigned[i] );
      efTrace->addLabel( "Y " );
      efTrace->addTextField( "", 20, eBuf->bufYPvName[i],
       PV_Factory::MAX_PV_NAME );
      efTrace->addLabel( "  S " );
      efTrace->addToggle( " ", &eBuf->bufYSigned[i] );
      efTrace->addLabel( "N " );
      efTrace->addTextField( "", 15, eBuf->bufNPvName[i],
       PV_Factory::MAX_PV_NAME );
      efTrace->addOption( "", "scope|plot", &eBuf->bufOpMode[i] );
      efTrace->addLabel( "  Y2" );
      efTrace->addToggle( " ", &eBuf->bufY2Scale[i] );
      //efTrace->addLabel( "  Style" );
      efTrace->addOption( "", "line|point|needle|single point", &eBuf->bufPlotStyle[i] );
      efTrace->addLabel( "  Update" );
      efTrace->addOption( "", "X and Y|X or Y|X|Y|Trigger",
       &eBuf->bufPlotUpdateMode[i] );
      efTrace->addLabel( "  Thk" );
      efTrace->addOption( "", "1|2|3|4|5|6|7|8|9", &eBuf->bufLineThk[i] );
      //efTrace->addLabel( "  Line" );
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

  for ( yi=0; yi<xyGraphClass::NUM_Y_AXES; yi++ ) {
    eBuf->bufY1Axis[yi] = y1Axis[yi];
    eBuf->bufY1AxisStyle[yi] = y1AxisStyle[yi];
    eBuf->bufY1AxisSource[yi] = y1AxisSource[yi];
    eBuf->bufY1Min[yi] = y1Min[yi];
    eBuf->bufY1Max[yi] = y1Max[yi];
  }

  ef.addEmbeddedEf( "Axis Data", "... ", &efAxis );

    efAxis->create( actWin->top, actWin->appCtx->ci.getColorMap(),
     &actWin->appCtx->entryFormX,
     &actWin->appCtx->entryFormY, &actWin->appCtx->entryFormW,
     &actWin->appCtx->entryFormH, &actWin->appCtx->largestH,
     "Axis Properties", NULL, NULL, NULL );

    efAxis->beginSubForm();
    efAxis->addLabel( "X   " );
    efAxis->addLabel( " Show" );
    efAxis->addToggle( " ", &eBuf->bufXAxis );
    efAxis->addLabel( " Style" );
    efAxis->addOption( "", "linear|log10|time|loc_log10(time)", &eBuf->bufXAxisStyle );
    efAxis->addLabel( " Range" );
    efAxis->addOption( "", "from pv|user-specified|auto-scale",
     &eBuf->bufXAxisSource );
    efAxis->addLabel( " Minimum " );
    efAxis->addTextField( "", 10, &eBuf->bufXMin );
    efAxis->addLabel( " Maximum " );
    efAxis->addTextField( "", 10, &eBuf->bufXMax );
    efAxis->addLabel( " Time Format" );
    efAxis->addOption( "",
     "Seconds|mm-dd-yy hh:mm:ss",
     &eBuf->bufXAxisTimeFormat );
    efAxis->addLabel( "No Scale Adjustment" );
    efAxis->addToggle( " ", &eBuf->bufXAxisSmoothing );
    efAxis->endSubForm();

    efAxis->beginLeftSubForm();
    efAxis->addLabel( "    " );
    efAxis->addLabel( " Label Tick Intervals " );
    efAxis->addTextField( "", 3, &eBuf->bufXNumLabelIntervals );
    efAxis->addLabel( " Grid" );
    efAxis->addToggle( " ", &eBuf->bufXLabelGrid );
    efAxis->addLabel( " Majors/Label " );
    efAxis->addTextField( "", 3, &eBuf->bufXNumMajorPerLabel );
    efAxis->addLabel( " Grid" );
    efAxis->addToggle( " ", &eBuf->bufXMajorGrid );
    efAxis->addLabel( " Minors/Major " );
    efAxis->addTextField( "", 3, &eBuf->bufXNumMinorPerMajor );
    efAxis->addLabel( " Grid" );
    efAxis->addToggle( " ", &eBuf->bufXMinorGrid );
    efAxis->addLabel( "User Specified Scale Divisions" );
    efAxis->addToggle( " ", &eBuf->bufXGridMode );
    //efAxis->addLabel( " Format" );
    //efAxis->addOption( "", "f|g", &eBuf->bufXAnnotationFormat );
    //efAxis->addLabel( " Precision " );
    //efAxis->addTextField( "", 3, &eBuf->bufXAnnotationPrecision );
    efAxis->endSubForm();
   
    efAxis->addSeparator();

    yi = 0;
    efAxis->beginLeftSubForm();
    efAxis->addLabel( "Y1  " );
    efAxis->addLabel( " Show" );
    efAxis->addToggle( " ", &eBuf->bufY1Axis[yi] );
    efAxis->addLabel( " Style" );
    efAxis->addOption( "", "linear|log10", &eBuf->bufY1AxisStyle[yi] );
    efAxis->addLabel( " Range" );
    efAxis->addOption( "", "from pv|user-specified|auto-scale",
     &eBuf->bufY1AxisSource[yi] );
    efAxis->addLabel( " Minimum " );
    efAxis->addTextField( "", 10, &eBuf->bufY1Min[yi] );
    efAxis->addLabel( " Maximum " );
    efAxis->addTextField( "", 10, &eBuf->bufY1Max[yi] );
    efAxis->addLabel( "No Scale Adjustment" );
    efAxis->addToggle( " ", &eBuf->bufY1AxisSmoothing[yi] );
    efAxis->endSubForm();

    efAxis->beginLeftSubForm();
    efAxis->addLabel( "    " );
    efAxis->addLabel( " Label Tick Intervals " );
    efAxis->addTextField( "", 3, &eBuf->bufY1NumLabelIntervals[yi] );
    efAxis->addLabel( " Grid" );
    efAxis->addToggle( " ", &eBuf->bufY1LabelGrid[yi] );
    efAxis->addLabel( " Majors/Label " );
    efAxis->addTextField( "", 3, &eBuf->bufY1NumMajorPerLabel[yi] );
    efAxis->addLabel( " Grid" );
    efAxis->addToggle( " ", &eBuf->bufY1MajorGrid[yi] );
    efAxis->addLabel( " Minors/Major " );
    efAxis->addTextField( "", 3, &eBuf->bufY1NumMinorPerMajor[yi] );
    efAxis->addLabel( " Grid" );
    efAxis->addToggle( " ", &eBuf->bufY1MinorGrid[yi] );
    efAxis->addLabel( "User Specified Scale Divisions" );
    efAxis->addToggle( " ", &eBuf->bufY1GridMode[yi] );
    //efAxis->addLabel( " Format" );
    //efAxis->addOption( "", "f|g", &eBuf->bufY1AnnotationFormat[yi] );
    //efAxis->addLabel( " Precision " );
    //efAxis->addTextField( "", 3, &eBuf->bufY1AnnotationPrecision[yi] );
    efAxis->endSubForm();
   
    efAxis->addSeparator();

    yi = 1;
    efAxis->beginLeftSubForm();
    efAxis->addLabel( "Y2  " );
    efAxis->addLabel( " Show" );
    efAxis->addToggle( " ", &eBuf->bufY1Axis[yi] );
    efAxis->addLabel( " Style" );
    efAxis->addOption( "", "linear|log10",
     &eBuf->bufY1AxisStyle[yi] );
    efAxis->addLabel( " Range" );
    efAxis->addOption( "", "from pv|user-specified|auto-scale",
     &eBuf->bufY1AxisSource[yi] );
    efAxis->addLabel( " Minimum " );
    efAxis->addTextField( "", 10, &eBuf->bufY1Min[yi] );
    efAxis->addLabel( " Maximum " );
    efAxis->addTextField( "", 10, &eBuf->bufY1Max[yi] );
    efAxis->addLabel( "No Scale Adjustment" );
    efAxis->addToggle( " ", &eBuf->bufY1AxisSmoothing[yi] );
    efAxis->endSubForm();

    efAxis->beginLeftSubForm();
    efAxis->addLabel( "    " );
    efAxis->addLabel( " Label Tick Intervals " );
    efAxis->addTextField( "", 3, &eBuf->bufY1NumLabelIntervals[yi] );
    efAxis->addLabel( " Grid" );
    efAxis->addToggle( " ", &eBuf->bufY1LabelGrid[yi] );
    efAxis->addLabel( " Majors/Label " );
    efAxis->addTextField( "", 3, &eBuf->bufY1NumMajorPerLabel[yi] );
    efAxis->addLabel( " Grid" );
    efAxis->addToggle( " ", &eBuf->bufY1MajorGrid[yi] );
    efAxis->addLabel( " Minors/Major " );
    efAxis->addTextField( "", 3, &eBuf->bufY1NumMinorPerMajor[yi] );
    efAxis->addLabel( " Grid" );
    efAxis->addToggle( " ", &eBuf->bufY1MinorGrid[yi] );
    efAxis->addLabel( "User Specified Scale Divisions" );
    efAxis->addToggle( " ", &eBuf->bufY1GridMode[yi] );
    //efAxis->addLabel( " Format" );
    //efAxis->addOption( "", "f|g", &eBuf->bufY1AnnotationFormat[yi] );
    //efAxis->addLabel( " Precision " );
    //efAxis->addTextField( "", 3, &eBuf->bufY1AnnotationPrecision[yi] );
    efAxis->endSubForm();

    efAxis->finished( axygc_edit_ok_axis, this );

  ef.addTextField( "Trace Ctl PV", 35, eBuf->bufTraceCtlPvName,
   PV_Factory::MAX_PV_NAME );
  ef.addTextField( "Trigger PV", 35, eBuf->bufTrigPvName,
   PV_Factory::MAX_PV_NAME );

  ef.addTextField( "Reset PV", 35, eBuf->bufResetPvName,
   PV_Factory::MAX_PV_NAME );
  resetPvEntry = ef.getCurItem();
  ef.addOption( "Reset Mode", "if not zero|if zero", &eBuf->bufResetMode );
  resetModeEntry = ef.getCurItem();
  resetPvEntry->addDependency( resetModeEntry );
  resetPvEntry->addDependencyCallbacks();

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

int i, ii, yi, count;
double dxValue, dyValue;
double scaledX, scaledY;
double yZero;
int ctl;

  count = 0;
  for ( i=0; i<numTraces; i++ ) {

    ctl = (int) traceCtl & ( 1 << i );

    if ( !ctl ) {

    xFactor[i] =
     (double) ( plotAreaW ) / ( curXMax - curXMin );
    xOffset[i] = plotAreaX;

    yi = 0;
    if ( y2Scale[i] ) yi = 1;

    y1Factor[yi][i] =
     (double) ( plotAreaH ) / ( curY1Max[yi] - curY1Min[yi] );
    y1Offset[yi][i] = plotAreaY;

    initPlotInfo( i );
    arrayNumPoints[i] = curNpts[i] = 0;

    ii = arrayHead[i];
    while ( ii != arrayTail[i] ) {

      // There are two views of pv types, Type and specificType; this uses
      // specificType
      switch ( yPvType[i] ) {
      case ProcessVariable::specificType::flt:
        dyValue = (double) ( (float *) yPvData[i] )[ii];
        break;
      case ProcessVariable::specificType::real: 
        dyValue = ( (double *) yPvData[i] )[ii];
        break;
      case ProcessVariable::specificType::shrt:
        if ( ySigned[i] ) {
          dyValue = (double) ( (short *) yPvData[i] )[ii];
        }
        else {
          dyValue = (double) ( (unsigned short *) yPvData[i] )[ii];
        }
        break;
      case ProcessVariable::specificType::chr:
        if ( ySigned[i] ) {
          dyValue = (double) ( (char *) yPvData[i] )[ii];
        }
        else {
          dyValue = (double) ( (unsigned char *) yPvData[i] )[ii];
        }
        break;
      case ProcessVariable::specificType::integer:
          if ( ySigned[i] ) {
            dyValue = (double) ( (int *) yPvData[i] )[ii];
          }
          else {
            dyValue = (double) ( (unsigned int *) yPvData[i] )[ii];
          }
        break;
      case ProcessVariable::specificType::enumerated:
        if ( ySigned[i] ) {
          dyValue = (double) ( (short *) yPvData[i] )[ii];
        }
        else {
          dyValue = (double) ( (unsigned short *) yPvData[i] )[ii];
        }
        break;
      default:
        dyValue = ( (double *) yPvData[i] )[ii];
        break;
      }

      if ( y1AxisStyle[yi] == XYGC_K_AXIS_STYLE_LOG10 ) {
        dyValue = loc_log10(  dyValue  );
      }

      scaledY = plotAreaH -
       rint( ( dyValue - curY1Min[yi] ) *
       y1Factor[yi][i] - y1Offset[yi][i] );
      scaledY = dclamp( scaledY );

      if ( y1AxisStyle[yi] == XYGC_K_AXIS_STYLE_LOG10 ) {
        yZero = plotAreaH + y1Offset[yi][i];
      }
      else {
        yZero = plotAreaH -
         rint( ( 0.0 - curY1Min[yi] ) *
         y1Factor[yi][i] - y1Offset[yi][i] );
        yZero = dclamp( yZero );
      }

      if ( traceType[i] == XYGC_K_TRACE_CHRONOLOGICAL ) {

        dxValue = ( (double *) xPvData[i] )[ii];

      }
      else {
  
        // There are two views of pv types, Type and specificType; this uses
        // specificType
        switch ( xPvType[i] ) {
        case ProcessVariable::specificType::flt:
          dxValue = (double) ( (float *) xPvData[i] )[ii];
          break;
        case ProcessVariable::specificType::real: 
          dxValue = ( (double *) xPvData[i] )[ii];
          break;
        case ProcessVariable::specificType::shrt:
          if ( xSigned[i] ) {
            dxValue = (double) ( (short *) xPvData[i] )[ii];
	  }
	  else {
            dxValue = (double) ( (unsigned short *) xPvData[i] )[ii];
	  }
          break;
        case ProcessVariable::specificType::chr:
          if ( xSigned[i] ) {
            dxValue = (double) ( (char *) xPvData[i] )[ii];
	  }
	  else {
            dxValue = (double) ( (unsigned char *) xPvData[i] )[ii];
	  }
          break;
        case ProcessVariable::specificType::integer:
          if ( xSigned[i] ) {
            dxValue = (double) ( (int *) xPvData[i] )[ii];
	  }
	  else {
            dxValue = (double) ( (unsigned int *) xPvData[i] )[ii];
	  }
          break;
        case ProcessVariable::specificType::enumerated:
          if ( xSigned[i] ) {
            dxValue = (double) ( (short *) xPvData[i] )[ii];
	  }
	  else {
            dxValue = (double) ( (unsigned short *) xPvData[i] )[ii];
	  }
          break;
        default:
          dxValue = ( (double *) xPvData[i] )[ii];
          break;
        }

      }

      if ( xAxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) {
        dxValue = loc_log10(  dxValue  );
      }
      else if ( xAxisStyle == XYGC_K_AXIS_STYLE_TIME_LOG10 ) {
        dxValue = loc_log10(  dxValue  );
      }

      scaledX = rint( ( dxValue - curXMin ) *
       xFactor[i] + xOffset[i] );
      scaledX = dclamp( scaledX );

      addPoint( dxValue, scaledX, scaledY, yZero, i );

      //yArrayGotValue[i] = xArrayGotValue[i] = 0;
      //printf( "addPoint 6 - reset flags\n" );

      ii++;
      if ( ii > plotBufSize[i] ) {
        ii = 0;
      }

    }

    }

  }

}

void xyGraphClass::genChronoVector (
  int i, // trace
  int *rescale
) {

int ii, iii, yi, needRescale;
double dxValue, dyValue;
double scaledX, scaledY;
double yZero;
char format[31+1];
int ctl;

  *rescale = needRescale = 0;

  ctl = (int) traceCtl & ( 1 << i );

  if ( ctl ) return;

  yi = 0;
  if ( y2Scale[i] ) yi = 1;

  initPlotInfo( i );
  arrayNumPoints[i] = 0;

  for ( ii=0; ii<yPvEffCount( i ); ii++ ) {

    // There are two views of pv types, Type and specificType; this uses
    // specificType
    switch ( yPvType[i] ) {
    case ProcessVariable::specificType::flt:
      dyValue = (double) ( (float *) yPvData[i] )[ii];
      break;
    case ProcessVariable::specificType::real: 
      dyValue = ( (double *) yPvData[i] )[ii];
      break;
    case ProcessVariable::specificType::shrt:
      if ( ySigned[i] ) {
        dyValue = (double) ( (short *) yPvData[i] )[ii];
      }
      else {
        dyValue = (double) ( (unsigned short *) yPvData[i] )[ii];
      }
      break;
    case ProcessVariable::specificType::chr:
      if ( ySigned[i] ) {
        dyValue = (double) ( (char *) yPvData[i] )[ii];
      }
      else {
        dyValue = (double) ( (unsigned char *) yPvData[i] )[ii];
      }
      break;
    case ProcessVariable::specificType::integer:
      if ( ySigned[i] ) {
        dyValue = (double) ( (int *) yPvData[i] )[ii];
      }
      else {
        dyValue = (double) ( (unsigned int *) yPvData[i] )[ii];
      }
      break;
    case ProcessVariable::specificType::enumerated:
      if ( ySigned[i] ) {
        dyValue = (double) ( (short *) yPvData[i] )[ii];
      }
      else {
        dyValue = (double) ( (unsigned short *) yPvData[i] )[ii];
      }
      break;
    default:
      dyValue = ( (double *) yPvData[i] )[ii];
      break;
    }

    if ( y1AxisStyle[yi] == XYGC_K_AXIS_STYLE_LOG10 ) {
      dyValue = loc_log10(  dyValue  );
    }

    scaledY = plotAreaH -
     rint( ( dyValue - curY1Min[yi] ) *
     y1Factor[yi][i] - y1Offset[yi][i] );
    scaledY = dclamp( scaledY );

    if ( y1AxisStyle[yi] == XYGC_K_AXIS_STYLE_LOG10 ) {
      yZero = plotAreaH + y1Offset[yi][i];
    }
    else {
      yZero = plotAreaH -
       rint( ( 0.0 - curY1Min[yi] ) *
       y1Factor[yi][i] - y1Offset[yi][i] );
      yZero = dclamp( yZero );
    }

#if 0
    // There are two views of pv types, Type and specificType; this uses
    // specificType
    switch ( xPvType[i] ) {
    case ProcessVariable::specificType::flt:
      dxValue = (double) ( (float *) xPvData[i] )[ii];
      break;
    case ProcessVariable::specificType::real: 
      dxValue = ( (double *) xPvData[i] )[ii];
      break;
    case ProcessVariable::specificType::shrt:
      if ( xSigned[i] ) {
        dxValue = (double) ( (short *) xPvData[i] )[ii];
      }
      else {
        dxValue = (double) ( (unsigned short *) xPvData[i] )[ii];
      }
      break;
    case ProcessVariable::specificType::chr:
      if ( xSigned[i] ) {
        dxValue = (double) ( (char *) xPvData[i] )[ii];
      }
      else {
        dxValue = (double) ( (unsigned char *) xPvData[i] )[ii];
      }
      break;
    case ProcessVariable::specificType::integer:
      if ( xSigned[i] ) {
        dxValue = (double) ( (int *) xPvData[i] )[ii];
      }
      else {
        dxValue = (double) ( (unsigned int *) xPvData[i] )[ii];
      }
      break;
    case ProcessVariable::specificType::enumerated:
      if ( xSigned[i] ) {
        dxValue = (double) ( (short *) xPvData[i] )[ii];
      }
      else {
        dxValue = (double) ( (unsigned short *) xPvData[i] )[ii];
      }
      break;
    default:
      dxValue = ( (double *) xPvData[i] )[ii];
      break;
    }

#endif

    dxValue = ( (double *) xPvData[i] )[ii];

    if ( xAxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) {
      dxValue = loc_log10(  dxValue  );
    }
    else if ( xAxisStyle == XYGC_K_AXIS_STYLE_TIME_LOG10 ) {
      dxValue = loc_log10(  dxValue  );
    }

    scaledX = rint( ( dxValue - curXMin ) *
     xFactor[i] + xOffset[i] );
    scaledX = dclamp( scaledX );

    addPoint( dxValue, scaledX, scaledY, yZero, i );

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

    if ( y1AxisSource[yi] == XYGC_K_AUTOSCALE ) {
      if ( kpY1MinEfDouble[yi].isNull() ) {
        if ( dyValue < curY1Min[yi] ) {
          needRescale = 1;
          curY1Min[yi] = dyValue;
        }
      }
      if ( kpY1MaxEfDouble[yi].isNull() ) {
        if ( dyValue > curY1Max[yi] ) {
          needRescale = 1;
          curY1Max[yi] = dyValue;
        }
      }
    }

  }

  if ( needRescale ) {

    needNewLimits = 1;
    //needAutoScaleUpdate = 1;
    actWin->addDefExeNode( aglPtr );

    if ( xAxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) {
      get_log10_scale_params1( curXMin, curXMax, &adjCurXMin, &adjCurXMax,
       &curXNumLabelTicks, &curXMajorsPerLabel, &curXMinorsPerMajor, format );
      if ( ( xAxisSource == XYGC_K_AUTOSCALE ) && kpXMinEfDouble.isNull() ) {
        curXMin = adjCurXMin;
      }
      if ( ( xAxisSource == XYGC_K_AUTOSCALE ) && kpXMaxEfDouble.isNull() ) {
        curXMax = adjCurXMax;
      }
    }
    else if ( xAxisStyle == XYGC_K_AXIS_STYLE_TIME_LOG10 ) {
      get_log10_scale_params1( curXMin, curXMax, &adjCurXMin, &adjCurXMax,
       &curXNumLabelTicks, &curXMajorsPerLabel, &curXMinorsPerMajor, format );
      if ( ( xAxisSource == XYGC_K_AUTOSCALE ) && kpXMinEfDouble.isNull() ) {
        curXMin = adjCurXMin;
      }
      if ( ( xAxisSource == XYGC_K_AUTOSCALE ) && kpXMaxEfDouble.isNull() ) {
        curXMax = adjCurXMax;
      }
    }
    else {
      get_scale_params1( curXMin, curXMax,
       &adjCurXMin, &adjCurXMax,
       &curXNumLabelTicks, &curXMajorsPerLabel, &curXMinorsPerMajor, format );
      if ( xAxisSmoothing == XYGC_K_NO_SMOOTHING ) {
        adjCurXMin = curXMin;
        adjCurXMax = curXMax;
      }
      if ( ( xAxisSource == XYGC_K_AUTOSCALE ) && kpXMinEfDouble.isNull() ) {
        curXMin = adjCurXMin;
      }
      if ( ( xAxisSource == XYGC_K_AUTOSCALE ) && kpXMaxEfDouble.isNull() ) {
        curXMax = adjCurXMax;
      }
    }

    if ( y1AxisStyle[yi] == XYGC_K_AXIS_STYLE_LOG10 ) {
      get_log10_scale_params1( curY1Min[yi], curY1Max[yi], &adjCurY1Min[yi],
       &adjCurY1Max[yi], &curY1NumLabelTicks[yi], &curY1MajorsPerLabel[yi],
       &curY1MinorsPerMajor[yi], format );
      if ( ( y1AxisSource[yi] == XYGC_K_AUTOSCALE ) && kpY1MinEfDouble[yi].isNull() ) {
        curY1Min[yi] = adjCurY1Min[yi];
      }
      if ( ( y1AxisSource[yi] == XYGC_K_AUTOSCALE ) && kpY1MaxEfDouble[yi].isNull() ) {
        curY1Max[yi] = adjCurY1Max[yi];
      }
    }
    else {
      get_scale_params1( curY1Min[yi], curY1Max[yi], &adjCurY1Min[yi],
       &adjCurY1Max[yi], &curY1NumLabelTicks[yi], &curY1MajorsPerLabel[yi],
       &curY1MinorsPerMajor[yi], format );
      if ( y1AxisSmoothing[yi] == XYGC_K_NO_SMOOTHING ) {
        adjCurY1Min[yi] = curY1Min[yi];
        adjCurY1Max[yi] = curY1Max[yi];
      }
      if ( ( y1AxisSource[yi] == XYGC_K_AUTOSCALE ) && kpY1MinEfDouble[yi].isNull() ) {
        curY1Min[yi] = adjCurY1Min[yi];
      }
      if ( ( y1AxisSource[yi] == XYGC_K_AUTOSCALE ) && kpY1MaxEfDouble[yi].isNull() ) {
        curY1Max[yi] = adjCurY1Max[yi];
      }
    }

    updateDimensions();

    for ( iii=0; iii<numTraces; iii++ ) {
      xFactor[iii] =
       (double) ( plotAreaW ) / ( curXMax - curXMin );
      xOffset[iii] = plotAreaX;
    }

    for ( iii=0; iii<numTraces; iii++ ) {
      y1Factor[yi][iii] =
       (double) ( plotAreaH ) / ( curY1Max[yi] - curY1Min[yi] );
      y1Offset[yi][iii] = plotAreaY;
    }

    *rescale = 1;

  }

  arrayHead[i] = 0;
  arrayTail[i] = yPvEffCount( i );
  yArrayGotValue[i] = xArrayGotValue[i] = 0;

}

void xyGraphClass::genXyVector (
  int i, // trace
  int *rescale
) {

int ii, iii, yi, needRescale, n;
double dxValue, dyValue;
double scaledX, scaledY;
double yZero;
// Ron Chestnut changes 3/2/2007
 double squash;
double new_min_y = -1., new_max_y = 1.;
double new_min_x = -1., new_max_x = 1.;
// End of changes
char format[31+1];
int ctl;

  *rescale = needRescale = 0;

  ctl = (int) traceCtl & ( 1 << i );

  if ( ctl ) return;

  yi = 0;
  if ( y2Scale[i] ) yi = 1;

  initPlotInfo( i );
  arrayNumPoints[i] = 0;

  n = yPvEffCount( i );
  if ( xPvEffCount( i ) < n ) n = xPvEffCount( i );

  for ( ii=0; ii<n; ii++ ) {

    // There are two views of pv types, Type and specificType; this uses
    // specificType
    switch ( yPvType[i] ) {
    case ProcessVariable::specificType::flt:
      dyValue = (double) ( (float *) yPvData[i] )[ii];
      break;
    case ProcessVariable::specificType::real: 
      dyValue = ( (double *) yPvData[i] )[ii];
      break;
    case ProcessVariable::specificType::shrt:
      if ( ySigned[i] ) {
        dyValue = (double) ( (short *) yPvData[i] )[ii];
      }
      else {
        dyValue = (double) ( (unsigned short *) yPvData[i] )[ii];
      }
      break;
    case ProcessVariable::specificType::chr:
      if ( ySigned[i] ) {
        dyValue = (double) ( (char *) yPvData[i] )[ii];
      }
      else {
        dyValue = (double) ( (unsigned char *) yPvData[i] )[ii];
      }
      break;
    case ProcessVariable::specificType::integer:
      if ( ySigned[i] ) {
        dyValue = (double) ( (int *) yPvData[i] )[ii];
      }
      else {
        dyValue = (double) ( (unsigned int *) yPvData[i] )[ii];
      }
      break;
    case ProcessVariable::specificType::enumerated:
      if ( ySigned[i] ) {
        dyValue = (double) ( (short *) yPvData[i] )[ii];
      }
      else {
        dyValue = (double) ( (unsigned short *) yPvData[i] )[ii];
      }
      break;
    default:
      dyValue = ( (double *) yPvData[i] )[ii];
      break;
    }

    if ( y1AxisStyle[yi] == XYGC_K_AXIS_STYLE_LOG10 ) {
      dyValue = loc_log10(  dyValue  );
    }
// Ron Chestnut changes 3/2/2007
    if ( ii == 0) new_min_y = new_max_y = dyValue;
    else {
      if(dyValue > new_max_y) new_max_y=dyValue;
      if(dyValue < new_min_y) new_min_y=dyValue;
    }
// end of changes
    scaledY = plotAreaH -
     rint( ( dyValue - curY1Min[yi] ) *
     y1Factor[yi][i] - y1Offset[yi][i] );
    scaledY = dclamp( scaledY );

    if ( y1AxisStyle[yi] == XYGC_K_AXIS_STYLE_LOG10 ) {
      yZero = plotAreaH + y1Offset[yi][i];
    }
    else {
      yZero = plotAreaH -
       rint( ( 0.0 - curY1Min[yi] ) *
       y1Factor[yi][i] - y1Offset[yi][i] );
      yZero = dclamp( yZero );
    }

    // There are two views of pv types, Type and specificType; this uses
    // specificType
    switch ( xPvType[i] ) {
    case ProcessVariable::specificType::flt:
      dxValue = (double) ( (float *) xPvData[i] )[ii];
      break;
    case ProcessVariable::specificType::real: 
      dxValue = ( (double *) xPvData[i] )[ii];
      break;
    case ProcessVariable::specificType::shrt:
      if ( xSigned[i] ) {
        dxValue = (double) ( (short *) xPvData[i] )[ii];
      }
      else {
        dxValue = (double) ( (unsigned short *) xPvData[i] )[ii];
      }
      break;
    case ProcessVariable::specificType::chr:
      if ( xSigned[i] ) {
        dxValue = (double) ( (char *) xPvData[i] )[ii];
      }
      else {
        dxValue = (double) ( (unsigned char *) xPvData[i] )[ii];
      }
      break;
    case ProcessVariable::specificType::integer:
      if ( xSigned[i] ) {
        dxValue = (double) ( (int *) xPvData[i] )[ii];
      }
      else {
        dxValue = (double) ( (unsigned int *) xPvData[i] )[ii];
      }
      break;
    case ProcessVariable::specificType::enumerated:
      if ( xSigned[i] ) {
        dxValue = (double) ( (short *) xPvData[i] )[ii];
      }
      else {
        dxValue = (double) ( (unsigned short *) xPvData[i] )[ii];
      }
      break;
    default:
      dxValue = ( (double *) xPvData[i] )[ii];
      break;
    }

    if ( xAxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) {
      dxValue = loc_log10(  dxValue  );
    }
    else if ( xAxisStyle == XYGC_K_AXIS_STYLE_TIME_LOG10 ) {
      dxValue = loc_log10(  dxValue  );
    }

    scaledX = rint( ( dxValue - curXMin ) *
     xFactor[i] + xOffset[i] );
    scaledX = dclamp( scaledX );

    addPoint( dxValue, scaledX, scaledY, yZero, i );

    // =================================================
    // DO NOT AUTO SCALE BOTH DIRECTIONS
    if ( !autoScaleBothDirections || 1 ) {

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

      if ( y1AxisSource[yi] == XYGC_K_AUTOSCALE ) {
        if ( kpY1MinEfDouble[yi].isNull() ) {
          if ( dyValue < curY1Min[yi] ) {
            needRescale = 1;
            curY1Min[yi] = dyValue;
          }
        }
        if ( kpY1MaxEfDouble[yi].isNull() ) {
          if ( dyValue > curY1Max[yi] ) {
            needRescale = 1;
            curY1Max[yi] = dyValue;
            //actWin->addDefExeNode( aglPtr );
          }
        }
      }

    }
    // =================================================

    // =================================================
    // DO AUTO SCALE BOTH DIRECTIONS
    if ( autoScaleBothDirections && 0 ) {
// Ron Chestnut changes 3/2/2007
      if ( ii == 0) {
        new_min_x = new_max_x = dxValue;
      }
      else {
        if(dxValue > new_max_x) new_max_x=dxValue;
        if(dxValue < new_min_x) new_min_x=dxValue;
      }
// end of changes
    }
    // =================================================

  }

  // =================================================
  // DO AUTO SCALE BOTH DIRECTIONS
  if ( autoScaleBothDirections && 0 ) {

// Ron Chestnut changes 3/2/2007

    if ( xAxisSource == XYGC_K_AUTOSCALE ) {
      squash = (curXMax-curXMin)*.20;
      if ( kpXMinEfDouble.isNull() ) {
        if ( new_min_x < curXMin ||
             new_min_x > curXMin+squash) {
          needRescale = 1;
        //fprintf( stderr, "min X rescale %g %g\n", new_min_x,curXMin );
          curXMin = new_min_x;
        }
      }
      if ( kpXMaxEfDouble.isNull() ) {
        if ( new_max_x > curXMax ||
             new_max_x < curXMax - squash) {
          needRescale = 1;
        //fprintf( stderr, "max X rescale %g %g\n",new_max_x,curXMax );
          curXMax = new_max_x;
        }
      }
    }
 
    if ( y1AxisSource[yi] == XYGC_K_AUTOSCALE ) {
      squash = (curY1Max[yi]-curY1Min[yi])*.20;
      if ( kpY1MinEfDouble[yi].isNull() ) {
        if ( new_min_y < curY1Min[yi] ||
             new_min_y > curY1Min[yi]+squash) {
          needRescale = 1;
          //fprintf( stderr, "min Y rescale %g %g\n", new_min_y,curY1Min[yi] );
          curY1Min[yi] = new_min_y;
        }
      }
      if ( kpY1MaxEfDouble[yi].isNull() ) {
        if ( new_max_y > curY1Max[yi] ||
             new_max_y < curY1Max[yi]-squash) {
          needRescale = 1;
          //fprintf( stderr, "max Y rescale %g %g\n",new_max_y,curY1Max[yi] );
          curY1Max[yi] = new_max_y;
        }
      }
    }
// end of changes

  }
  // =================================================

  if ( needRescale ) {

    needNewLimits = 1;
    //needAutoScaleUpdate = 1;
    actWin->addDefExeNode( aglPtr );

    if ( xAxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) {
      get_log10_scale_params1( curXMin, curXMax, &adjCurXMin, &adjCurXMax,
       &curXNumLabelTicks, &curXMajorsPerLabel, &curXMinorsPerMajor, format );
      if ( ( xAxisSource == XYGC_K_AUTOSCALE ) && kpXMinEfDouble.isNull() ) {
        curXMin = adjCurXMin;
      }
      if ( ( xAxisSource == XYGC_K_AUTOSCALE ) && kpXMaxEfDouble.isNull() ) {
        curXMax = adjCurXMax;
      }
    }
    else if ( xAxisStyle == XYGC_K_AXIS_STYLE_TIME_LOG10 ) {
      get_log10_scale_params1( curXMin, curXMax, &adjCurXMin, &adjCurXMax,
       &curXNumLabelTicks, &curXMajorsPerLabel, &curXMinorsPerMajor, format );
      if ( ( xAxisSource == XYGC_K_AUTOSCALE ) && kpXMinEfDouble.isNull() ) {
        curXMin = adjCurXMin;
      }
      if ( ( xAxisSource == XYGC_K_AUTOSCALE ) && kpXMaxEfDouble.isNull() ) {
        curXMax = adjCurXMax;
      }
    }
    else {
      get_scale_params1( curXMin, curXMax,
       &adjCurXMin, &adjCurXMax,
       &curXNumLabelTicks, &curXMajorsPerLabel, &curXMinorsPerMajor, format );
      if ( ( xAxisSource == XYGC_K_AUTOSCALE ) && kpXMinEfDouble.isNull() ) {
        curXMin = adjCurXMin;
      }
      if ( xAxisSmoothing == XYGC_K_NO_SMOOTHING ) {
        adjCurXMin = curXMin;
        adjCurXMax = curXMax;
      }
      if ( ( xAxisSource == XYGC_K_AUTOSCALE ) && kpXMaxEfDouble.isNull() ) {
        curXMax = adjCurXMax;
      }
    }

    if ( y1AxisStyle[yi] == XYGC_K_AXIS_STYLE_LOG10 ) {
      get_log10_scale_params1( curY1Min[yi], curY1Max[yi], &adjCurY1Min[yi], &adjCurY1Max[yi],
      &curY1NumLabelTicks[yi], &curY1MajorsPerLabel[yi],
       &curY1MinorsPerMajor[yi], format );
      if ( ( y1AxisSource[yi] == XYGC_K_AUTOSCALE ) && kpY1MinEfDouble[yi].isNull() ) {
        curY1Min[yi] = adjCurY1Min[yi];
      }
      if ( ( y1AxisSource[yi] == XYGC_K_AUTOSCALE ) && kpY1MaxEfDouble[yi].isNull() ) {
        curY1Max[yi] = adjCurY1Max[yi];
      }
    }
    else {
      get_scale_params1( curY1Min[yi], curY1Max[yi], &adjCurY1Min[yi], &adjCurY1Max[yi],
       &curY1NumLabelTicks[yi], &curY1MajorsPerLabel[yi],
       &curY1MinorsPerMajor[yi], format );
      if ( y1AxisSmoothing[yi] == XYGC_K_NO_SMOOTHING ) {
        adjCurY1Min[yi] = curY1Min[yi];
        adjCurY1Max[yi] = curY1Max[yi];
      }
      if ( ( y1AxisSource[yi] == XYGC_K_AUTOSCALE ) && kpY1MinEfDouble[yi].isNull() ) {
        curY1Min[yi] = adjCurY1Min[yi];
      }
      if ( ( y1AxisSource[yi] == XYGC_K_AUTOSCALE ) && kpY1MaxEfDouble[yi].isNull() ) {
        curY1Max[yi] = adjCurY1Max[yi];
      }
    }

    updateDimensions();

    for ( iii=0; iii<numTraces; iii++ ) {
      xFactor[iii] =
       (double) ( plotAreaW ) / ( curXMax - curXMin );
      xOffset[iii] = plotAreaX;
    }

    for ( iii=0; iii<numTraces; iii++ ) {
      y1Factor[yi][iii] =
       (double) ( plotAreaH ) / ( curY1Max[yi] - curY1Min[yi] );
      y1Offset[yi][iii] = plotAreaY;
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
int ctl;

  if ( !enabled || !activeMode || !init ) return 1;

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
  drawY2Scale();
  drawTitle();
  drawXlabel();
  drawYlabel();
  drawY2label();

  actWin->executeGc.setLineWidth(1);
  actWin->executeGc.setLineStyle( LineSolid );
  actWin->executeGc.restoreFg();

  bufInvalid = 0;
  for ( i=0; i<numTraces; i++ ) {
    ctl = (int) traceCtl & ( 1 << i );
    if ( !ctl ) {
      traceIsDrawn[i] = 0;
      yArrayNeedUpdate[i] = 1;
      xArrayNeedUpdate[i] = 1;
    }
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

void xyGraphClass::drawCircles (
  int index,
  XPoint *xp,
  int n
) {

XArc arc[100];
int numFullDraws, remainder, i, ii, j, symHW, symHH;

  symHH = symHalfHeight + lineThk[index];
  symHW = symHalfWidth + lineThk[index];

  numFullDraws = n / 100;
  for ( i=0, j=0; i<numFullDraws; i++, j+=100 ) {

    for ( ii=0; ii<100; ii++ ) {
      arc[ii].x = xp[j+ii].x - symHW;
      arc[ii].y = xp[j+ii].y - symHH;
      arc[ii].width = symHW + symHW;
      arc[ii].height = symHH + symHH;
      arc[ii].angle1 = 0;
      arc[ii].angle2 = 360*64;
    }

    XDrawArcs( actWin->d, pixmap,
     actWin->executeGc.normGC(), arc, 100 );

  }

  remainder = n % 100;
  for ( ii=0; ii<remainder; ii++ ) {
    arc[ii].x = xp[j+ii].x - symHW;
    arc[ii].y = xp[j+ii].y - symHH;
    arc[ii].width = symHW + symHW;
    arc[ii].height = symHH + symHH;
    arc[ii].angle1 = 0;
    arc[ii].angle2 = 360*64;
  }

  XDrawArcs( actWin->d, pixmap,
   actWin->executeGc.normGC(), arc, remainder );

}

void xyGraphClass::drawSquares (
  int index,
  XPoint *xp,
  int n
) {

XRectangle rec[100];
int numFullDraws, remainder, i, ii, j, symHW, symHH;

  symHH = symHalfHeight + lineThk[index];
  symHW = symHalfWidth + lineThk[index];

  numFullDraws = n / 100;
  for ( i=0, j=0; i<numFullDraws; i++, j+=100 ) {

    for ( ii=0; ii<100; ii++ ) {
      rec[ii].x = xp[j+ii].x - symHW;
      rec[ii].y = xp[j+ii].y - symHH;
      rec[ii].width = symHW + symHW;
      rec[ii].height = symHH + symHH;
    }

    XDrawRectangles( actWin->d, pixmap,
     actWin->executeGc.normGC(), rec, 100 );

  }

  remainder = n % 100;
  for ( ii=0; ii<remainder; ii++ ) {
    rec[ii].x = xp[j+ii].x - symHW;
    rec[ii].y = xp[j+ii].y - symHH;
    rec[ii].width = symHW + symHW;
    rec[ii].height = symHH + symHH;
  }

  XDrawRectangles( actWin->d, pixmap,
   actWin->executeGc.normGC(), rec, remainder );

}

void xyGraphClass::drawDiamonds (
  int index,
  XPoint *xp,
  int n
) {

XSegment seg[400];
int numFullDraws, remainder, i, ii, j, jj, symHW, symHH;

  symHH = (int) ( 1.4 * ( (double) symHalfHeight + (double) lineThk[index] ) );
  symHW = (int) ( 1.4 * ( (double) symHalfWidth + (double) lineThk[index] ) );

  numFullDraws = n / 100;
  for ( i=0, j=0; i<numFullDraws; i++, j+=100 ) {

    for ( ii=0, jj=0; ii<100; ii++, jj+=4 ) {

      seg[jj].x1 = xp[j+ii].x;
      seg[jj].y1 = xp[j+ii].y + symHH;
      seg[jj].x2 = xp[j+ii].x + symHW;
      seg[jj].y2 = xp[j+ii].y;

      seg[jj+1].x1 = xp[j+ii].x + symHW;
      seg[jj+1].y1 = xp[j+ii].y;
      seg[jj+1].x2 = xp[j+ii].x;
      seg[jj+1].y2 = xp[j+ii].y - symHH;

      seg[jj+2].x1 = xp[j+ii].x;
      seg[jj+2].y1 = xp[j+ii].y - symHH;
      seg[jj+2].x2 = xp[j+ii].x - symHW;
      seg[jj+2].y2 = xp[j+ii].y;

      seg[jj+3].x1 = xp[j+ii].x - symHW;
      seg[jj+3].y1 = xp[j+ii].y;
      seg[jj+3].x2 = xp[j+ii].x;
      seg[jj+3].y2 = xp[j+ii].y + symHH;

    }

    XDrawSegments( actWin->d, pixmap,
     actWin->executeGc.normGC(), seg, 400 );

  }

  remainder = n % 100;
  for ( ii=0, jj=0; ii<remainder; ii++, jj+=4 ) {

    seg[jj].x1 = xp[j+ii].x;
    seg[jj].y1 = xp[j+ii].y + symHH;
    seg[jj].x2 = xp[j+ii].x + symHW;
    seg[jj].y2 = xp[j+ii].y;

    seg[jj+1].x1 = xp[j+ii].x + symHW;
    seg[jj+1].y1 = xp[j+ii].y;
    seg[jj+1].x2 = xp[j+ii].x;
    seg[jj+1].y2 = xp[j+ii].y - symHH;

    seg[jj+2].x1 = xp[j+ii].x;
    seg[jj+2].y1 = xp[j+ii].y - symHH;
    seg[jj+2].x2 = xp[j+ii].x - symHW;
    seg[jj+2].y2 = xp[j+ii].y;

    seg[jj+3].x1 = xp[j+ii].x - symHW;
    seg[jj+3].y1 = xp[j+ii].y;
    seg[jj+3].x2 = xp[j+ii].x;
    seg[jj+3].y2 = xp[j+ii].y + symHH;

  }

  XDrawSegments( actWin->d, pixmap,
   actWin->executeGc.normGC(), seg, remainder*4 );

}

int xyGraphClass::eraseActive ( void ) {

int i;
XRectangle xR = { plotAreaX+1, plotAreaY, plotAreaW-2, plotAreaH };
//XRectangle xR = { plotAreaX+1, plotAreaY+1, plotAreaW-2, plotAreaH-2 };

  if ( !enabled || !activeMode || !init ) return 1;

  if ( bufInvalid ) {
    return 1;
  }

  actWin->executeGc.saveFg();
  actWin->executeGc.setFG( actWin->ci->pix(bgColor) );

  XSetClipRectangles( actWin->display(), actWin->executeGc.normGC(), 0, 0,
   &xR, 1, Unsorted );

  for ( i=0; i<numTraces; i++ ) {

    if ( yArrayNeedUpdate[i] && xArrayNeedUpdate[i] && traceIsDrawn[i] ) {

      actWin->executeGc.setLineWidth(1);
      actWin->executeGc.setLineStyle( LineSolid );

      //actWin->executeGc.setFGforGivenBG( actWin->ci->pix(plotColor[i]),
      // actWin->ci->pix(bgColor) );

      traceIsDrawn[i] = 0;

      if ( ( plotStyle[i] == XYGC_K_PLOT_STYLE_POINT ) ||
           ( plotStyle[i] == XYGC_K_PLOT_STYLE_SINGLE_POINT ) ) {

        if ( curNpts[i] > 0 ) {

          if ( plotSymbolType[i] == XYGC_K_SYMBOL_TYPE_NONE ) {

            XDrawPoints( actWin->d, pixmap,
             actWin->executeGc.normGC(), plotBuf[i], curNpts[i],
             CoordModeOrigin );

          }
          else if ( plotSymbolType[i] == XYGC_K_SYMBOL_TYPE_CIRCLE ) {

            drawCircles( i, plotBuf[i], curNpts[i] );

          }
          else if ( plotSymbolType[i] == XYGC_K_SYMBOL_TYPE_SQUARE ) {

            drawSquares( i, plotBuf[i], curNpts[i] );

          }
          else if ( plotSymbolType[i] == XYGC_K_SYMBOL_TYPE_DIAMOND ) {

            drawDiamonds( i, plotBuf[i], curNpts[i] );

          }

        }

      }
      else{

        if ( curNpts[i] > 0 ) {

          if ( plotSymbolType[i] == XYGC_K_SYMBOL_TYPE_CIRCLE ) {
            drawCircles( i, plotBuf[i], curNpts[i] );
          }
          else if ( plotSymbolType[i] == XYGC_K_SYMBOL_TYPE_SQUARE ) {
            drawSquares( i, plotBuf[i], curNpts[i] );
          }
          else if ( plotSymbolType[i] == XYGC_K_SYMBOL_TYPE_DIAMOND ) {
            drawDiamonds( i, plotBuf[i], curNpts[i] );
          }

          actWin->executeGc.setLineWidth( lineThk[i] );
          actWin->executeGc.setLineStyle( lineStyle[i] );

          if ( curNpts[i] > 1 ) {

            XDrawLines( actWin->d, pixmap,
             actWin->executeGc.normGC(), plotBuf[i], curNpts[i],
             CoordModeOrigin );

          }

        }

      }

    }

  }

  XSetClipMask( actWin->display(), actWin->executeGc.normGC(), None );

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
int ctl;

  ctl = (int) traceCtl & ( 1 << i );

  if ( ctl ) return 1;

  actWin->executeGc.setLineWidth(1);
  actWin->executeGc.setLineStyle( LineSolid );

  yArrayNeedUpdate[i] = xArrayNeedUpdate[i] = 1;
  if ( yArrayNeedUpdate[i] && xArrayNeedUpdate[i] ) {

    actWin->executeGc.setFGforGivenBG( actWin->ci->pix(plotColor[i]),
     actWin->ci->pix(bgColor) );

    traceIsDrawn[i] = 1;

    yArrayNeedUpdate[i] = xArrayNeedUpdate[i] = 0;

    if ( forceVector[i] || ( yPv[i]->get_dimension() > 1 ) ) { // vector

      npts = fillVectorPlotArray( i );

      if ( npts > 0 ) {

        if ( plotStyle[i] == XYGC_K_PLOT_STYLE_POINT ||
           ( plotStyle[i] == XYGC_K_PLOT_STYLE_SINGLE_POINT ) ) {

          if ( plotSymbolType[i] == XYGC_K_SYMBOL_TYPE_NONE ) {

            XDrawPoints( actWin->d, pixmap,
             actWin->executeGc.normGC(), plotBuf[i], npts,
             CoordModeOrigin );

	  }
	  else if ( plotSymbolType[i] == XYGC_K_SYMBOL_TYPE_CIRCLE ) {

            drawCircles( i, plotBuf[i], npts );

	  }
	  else if ( plotSymbolType[i] == XYGC_K_SYMBOL_TYPE_SQUARE ) {

            drawSquares( i, plotBuf[i], npts );

	  }
	  else if ( plotSymbolType[i] == XYGC_K_SYMBOL_TYPE_DIAMOND ) {

            drawDiamonds( i, plotBuf[i], npts );

	  }

          curNpts[i] = npts;

        }
        else {

	  if ( plotSymbolType[i] == XYGC_K_SYMBOL_TYPE_CIRCLE ) {
            drawCircles( i, plotBuf[i], npts );
	  }
	  else if ( plotSymbolType[i] == XYGC_K_SYMBOL_TYPE_SQUARE ) {
            drawSquares( i, plotBuf[i], npts );
	  }
	  else if ( plotSymbolType[i] == XYGC_K_SYMBOL_TYPE_DIAMOND ) {
            drawDiamonds( i, plotBuf[i], npts );
	  }

          if ( npts > 1 ) {

            actWin->executeGc.setLineWidth( lineThk[i] );
            actWin->executeGc.setLineStyle( lineStyle[i] );

            XDrawLines( actWin->d, pixmap,
             actWin->executeGc.normGC(), plotBuf[i], npts,
             CoordModeOrigin );

	  }

          curNpts[i] = npts;

        }

      }

    }
    else { // scalar

      npts = fillScalarPlotArray( i );

      if ( npts > 0 ) {

        if ( plotStyle[i] == XYGC_K_PLOT_STYLE_POINT ||
           ( plotStyle[i] == XYGC_K_PLOT_STYLE_SINGLE_POINT ) ) {

          if ( plotSymbolType[i] == XYGC_K_SYMBOL_TYPE_NONE ) {

            XDrawPoints( actWin->d, pixmap,
             actWin->executeGc.normGC(), plotBuf[i], npts,
             CoordModeOrigin );

	  }
	  else if ( plotSymbolType[i] == XYGC_K_SYMBOL_TYPE_CIRCLE ) {

            drawCircles( i, plotBuf[i], npts );

	  }
	  else if ( plotSymbolType[i] == XYGC_K_SYMBOL_TYPE_SQUARE ) {

            drawSquares( i, plotBuf[i], npts );

	  }
	  else if ( plotSymbolType[i] == XYGC_K_SYMBOL_TYPE_DIAMOND ) {

            drawDiamonds( i, plotBuf[i], npts );

	  }

          curNpts[i] = npts;

        }
	else {

	  if ( plotSymbolType[i] == XYGC_K_SYMBOL_TYPE_CIRCLE ) {
            drawCircles( i, plotBuf[i], npts );
	  }
	  else if ( plotSymbolType[i] == XYGC_K_SYMBOL_TYPE_SQUARE ) {
            drawSquares( i, plotBuf[i], npts );
	  }
	  else if ( plotSymbolType[i] == XYGC_K_SYMBOL_TYPE_DIAMOND ) {
            drawDiamonds( i, plotBuf[i], npts );
	  }

          if ( npts > 1 ) {

            actWin->executeGc.setLineWidth( lineThk[i] );
            actWin->executeGc.setLineStyle( lineStyle[i] );

            XDrawLines( actWin->d, pixmap,
             actWin->executeGc.normGC(), plotBuf[i], npts,
             CoordModeOrigin );

	  }

          curNpts[i] = npts;

        }

      }

    }

  }

  return 1;

}

int xyGraphClass::drawActive ( void ) {

int i;
XRectangle xR = { plotAreaX+1, plotAreaY, plotAreaW-2, plotAreaH };
//XRectangle xR = { plotAreaX+1, plotAreaY+1, plotAreaW-2, plotAreaH-2 };

  if ( !enabled || !activeMode || !init ) return 1;

  if ( bufInvalid ) {
    actWin->appCtx->proc->lock();
    needRefresh = 1;
    actWin->addDefExeNode( aglPtr );
    actWin->appCtx->proc->unlock();
    return 1;
  }

  if ( drawGridFlag ) {
    drawGridFlag = 0;
    drawGrid();
  }

  actWin->executeGc.saveFg();

  XSetClipRectangles( actWin->display(), actWin->executeGc.normGC(), 0, 0,
   &xR, 1, Unsorted );

  for ( i=0; i<numTraces; i++ ) {

    drawActiveOne( i );

  }

  XSetClipMask( actWin->display(), actWin->executeGc.normGC(), None );

  if ( plotAreaBorder ) {

    actWin->executeGc.setLineWidth(1);
    actWin->executeGc.setLineStyle( LineSolid );
    actWin->executeGc.setFG( actWin->ci->pix(fgColor) );

    XDrawLine( actWin->d, pixmap, actWin->executeGc.normGC(),
     plotAreaX, plotAreaY, plotAreaX+plotAreaW, plotAreaY );

    if ( !xAxis ) {
      XDrawLine( actWin->d, pixmap, actWin->executeGc.normGC(),
       plotAreaX, plotAreaY+plotAreaH, plotAreaX+plotAreaW,
       plotAreaY+plotAreaH );
    }

    if ( !y1Axis[0] ) {
      XDrawLine( actWin->d, pixmap, actWin->executeGc.normGC(),
       plotAreaX, plotAreaY, plotAreaX, plotAreaY+plotAreaH );
    }

    if ( !y1Axis[1] ) {
      XDrawLine( actWin->d, pixmap, actWin->executeGc.normGC(),
       plotAreaX+plotAreaW, plotAreaY, plotAreaX+plotAreaW,
       plotAreaY+plotAreaH );
    }

  }

  // Restore defaults
  actWin->executeGc.setLineWidth(1);
  actWin->executeGc.setLineStyle( LineSolid );
  actWin->executeGc.restoreFg();

  // Output buffer to window
  XCopyArea( actWin->display(), pixmap,
   drawable(actWin->executeWidget), actWin->executeGc.normGC(),
   0, 0, w+1, h+1, x, y );

  return 1;

}

void xyGraphClass::bufInvalidate ( void ) {

  bufInvalid = 1;
  updateDimensions();

}

int xyGraphClass::expandTemplate (
  int numMacros,
  char *macros[],
  char *expansions[] ) {

int i;
expStringClass tmpStr;

  tmpStr.setRaw( graphTitle.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  graphTitle.setRaw( tmpStr.getExpanded() );

  tmpStr.setRaw( xLabel.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  xLabel.setRaw( tmpStr.getExpanded() );

  tmpStr.setRaw( yLabel.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  yLabel.setRaw( tmpStr.getExpanded() );

  tmpStr.setRaw( y2Label.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  y2Label.setRaw( tmpStr.getExpanded() );

  tmpStr.setRaw( traceCtlPvExpStr.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  traceCtlPvExpStr.setRaw( tmpStr.getExpanded() );

  tmpStr.setRaw( trigPvExpStr.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  trigPvExpStr.setRaw( tmpStr.getExpanded() );

  tmpStr.setRaw( resetPvExpStr.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  resetPvExpStr.setRaw( tmpStr.getExpanded() );

  for ( i=0; i<numTraces; i++ ) {

    tmpStr.setRaw( xPvExpStr[i].getRaw() );
    tmpStr.expand1st( numMacros, macros, expansions );
    xPvExpStr[i].setRaw( tmpStr.getExpanded() );

    tmpStr.setRaw( yPvExpStr[i].getRaw() );
    tmpStr.expand1st( numMacros, macros, expansions );
    yPvExpStr[i].setRaw( tmpStr.getExpanded() );

    tmpStr.setRaw( nPvExpStr[i].getRaw() );
    tmpStr.expand1st( numMacros, macros, expansions );
    nPvExpStr[i].setRaw( tmpStr.getExpanded() );

  }

  return 1;

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

  stat = y2Label.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;

  stat = traceCtlPvExpStr.expand1st( numMacros, macros, expansions );
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
    stat = nPvExpStr[i].expand1st( numMacros, macros, expansions );
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

  stat = y2Label.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;

  stat = traceCtlPvExpStr.expand2nd( numMacros, macros, expansions );
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
    stat = nPvExpStr[i].expand2nd( numMacros, macros, expansions );
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

  result = y2Label.containsPrimaryMacros();
  if ( result ) return result;

  result = traceCtlPvExpStr.containsPrimaryMacros();
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
    result = nPvExpStr[i].containsPrimaryMacros();
    if ( result ) return result;
  }

  return 0;

}

char *xyGraphClass::firstDragName ( void ) {

  if ( !enabled ) return NULL;

  dragIndex = 0;
  return dragName[dragIndex];

}

char *xyGraphClass::nextDragName ( void ) {

  if ( !enabled ) return NULL;

  if ( dragIndex < (int) ( sizeof(dragName) / sizeof(char *) ) - 1 ) {
    dragIndex++;
    return dragName[dragIndex];
  }
  else {
    return NULL;
  }

}

char *xyGraphClass::dragValue (
  int i
) {

int ii;

  if ( !enabled ) return NULL;

  if ( actWin->mode == AWC_EXECUTE ) {

    if ( ( i >= 0 ) && ( i < XYGC_K_MAX_TRACES ) ) {

      return yPvExpStr[i].getExpanded();

    }
    else if ( ( i >= XYGC_K_MAX_TRACES ) &&
            ( i < ( XYGC_K_MAX_TRACES * 2 ) ) ) {

      ii = i - XYGC_K_MAX_TRACES;
      return xPvExpStr[ii].getExpanded();

    }
    else if ( i == ( XYGC_K_MAX_TRACES * 2 ) ) {

      return trigPvExpStr.getExpanded();

    }
    else if ( i == ( XYGC_K_MAX_TRACES * 2 + 1 ) ) {

      return resetPvExpStr.getExpanded();

    }
    else if ( i == ( XYGC_K_MAX_TRACES * 2 + 2 ) ) {

      return traceCtlPvExpStr.getExpanded();

    }

  }
  else {

    if ( ( i >= 0 ) && ( i < XYGC_K_MAX_TRACES ) ) {

      return yPvExpStr[i].getRaw();

    }
    else if ( ( i >= XYGC_K_MAX_TRACES ) &&
            ( i < ( XYGC_K_MAX_TRACES * 2 ) ) ) {

      ii = i - XYGC_K_MAX_TRACES;
      return xPvExpStr[ii].getRaw();

    }
    else if ( i == ( XYGC_K_MAX_TRACES * 2 ) ) {

      return trigPvExpStr.getRaw();

    }
    else if ( i == ( XYGC_K_MAX_TRACES * 2 + 1 ) ) {

      return resetPvExpStr.getRaw();

    }
    else if ( i == ( XYGC_K_MAX_TRACES * 2 + 2 ) ) {

      return traceCtlPvExpStr.getRaw();

    }

  }

  return NULL;

}

int xyGraphClass::activate (
  int pass,
  void *ptr )
{

int i, yScaleIndex, yi, n;
int screen_num, depth;
Arg args[5];
XmString str;

  switch ( pass ) {

  case 1:

    opComplete = 0;
    break;

  case 2:

    if ( !opComplete ) {

      opComplete = 1;

      widgetsCreated = 0;

      initEnable();

      // for popup menu
      if ( !widgetsCreated ) {

        n = 0;
        XtSetArg( args[n], XmNpopupEnabled, (XtArgVal) False ); n++;
        popUpMenu = XmCreatePopupMenu( actWin->topWidgetId(), "", args, n );

        pullDownMenu = XmCreatePulldownMenu( popUpMenu, "", NULL, 0 );

        str = XmStringCreateLocalized( "Perform auto-scale" );
        pbAutoScale = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
         popUpMenu,
         XmNlabelString, str,
         NULL );
        XmStringFree( str );

        XtAddCallback( pbAutoScale, XmNactivateCallback, menu_cb,
         (XtPointer) this );

        str = XmStringCreateLocalized( "Restore original scale" );
        pbOrigScale = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
         popUpMenu,
         XmNlabelString, str,
         NULL );
        XmStringFree( str );

        XtAddCallback( pbOrigScale, XmNactivateCallback, menu_cb,
         (XtPointer) this );

        str = XmStringCreateLocalized( "Adjust scale params" );
        pbAdjustParams = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
         popUpMenu,
         XmNlabelString, str,
         NULL );
        XmStringFree( str );

        XtAddCallback( pbAdjustParams, XmNactivateCallback, menu_cb,
         (XtPointer) this );

        str = XmStringCreateLocalized( "Clear Plot" );
        pbClearPlot = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
         popUpMenu,
         XmNlabelString, str,
         NULL );
        XmStringFree( str );

        XtAddCallback( pbClearPlot, XmNactivateCallback, menu_cb,
         (XtPointer) this );

        str = XmStringCreateLocalized( "Dump to file" );
        pbDumpData = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
         popUpMenu,
         XmNlabelString, str,
         NULL );
        XmStringFree( str );

        XtAddCallback( pbDumpData, XmNactivateCallback, menu_cb,
         (XtPointer) this );

	widgetsCreated = 1;

      }

      // for keypad functions

      xMinX0 = xMinX1 = xMinY0 = xMinY1 = -1;
      xMaxX0 = xMaxX1 = xMaxY0 = xMaxY1 = -1;
      kpXMinEfDouble.setNull(1);
      kpXMaxEfDouble.setNull(1);
      kpCancelMinX = kpCancelMaxX = 0;

      for ( yi=0; yi<xyGraphClass::NUM_Y_AXES; yi++ ) {
        lowestYScaleIndex[yi] = numTraces-1;
        y1MinX0[yi] = y1MinX1[yi] = y1MinY0[yi] = y1MinY1[yi] = -1;
        y1MaxX0[yi] = y1MaxX1[yi] = y1MaxY0[yi] = y1MaxY1[yi] = -1;
        kpY1MinEfDouble[yi].setNull(1);
        kpY1MaxEfDouble[yi].setNull(1);
        kpCancelMinY1[yi] = kpCancelMaxY1[yi] = 0;
      }

      // for timer
      updateTimer = 0;
      updateTimerActive = 0;
      updateAutoScaleTimer = 0;
      updateAutoScaleTimerActive = 0;

      // for message dialog
      msgDialog.create( actWin->topWidgetId() );
      msgDialogPopedUp = 0;

      firstBoxRescale = 1;
      doingBoxRescale = 0;

      screen_num = DefaultScreen( actWin->display() );
      depth = DefaultDepth( actWin->display(), screen_num );
      pixmap = XCreatePixmap( actWin->display(),
       XtWindow(actWin->executeWidget), w+2, h+2, depth );

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
        curXMin = loc_log10(  curXMin  );
	curXMax = loc_log10(  curXMax  );
      }
      else if ( xAxisStyle == XYGC_K_AXIS_STYLE_TIME_LOG10 ) {
        curXMin = loc_log10(  curXMin  );
	curXMax = loc_log10(  curXMax  );
      }

      adjCurXMin = curXMin;
      adjCurXMax = curXMax;

      for ( yi=0; yi<xyGraphClass::NUM_Y_AXES; yi++ ) {
        curY1Min[yi] = y1Min[yi].value();
        curY1Max[yi] = y1Max[yi].value();
        if ( y1AxisStyle[yi] == XYGC_K_AXIS_STYLE_LOG10 ) {
          curY1Min[yi] = loc_log10(  curY1Min[yi]  );
          curY1Max[yi] = loc_log10(  curY1Max[yi]  );
        }
        adjCurY1Min[yi] = curY1Min[yi];
        adjCurY1Max[yi] = curY1Max[yi];
      }

      curXNumLabelTicks = xNumLabelIntervals.value();
      if ( curXNumLabelTicks < 1 ) curXNumLabelTicks = 1;
      curXMajorsPerLabel = xNumMajorPerLabel.value();
      curXMinorsPerMajor = xNumMinorPerMajor.value();

      for ( yi=0; yi<xyGraphClass::NUM_Y_AXES; yi++ ) {
        curY1NumLabelTicks[yi] = y1NumLabelIntervals[yi].value();
        if ( curY1NumLabelTicks[yi] < 1 ) curY1NumLabelTicks[yi] = 1;
        curY1MajorsPerLabel[yi] = y1NumMajorPerLabel[yi].value();
        curY1MinorsPerMajor[yi] = y1NumMinorPerMajor[yi].value();
      }

      updateDimensions();

      aglPtr = ptr;
      connection.init();
      init = 0;
      bufInvalid = 1;
      activeMode = 1;
      firstTimeSample = 1;
      numBufferScrolls = 0;
      needConnect = needInit = needRefresh = needErase = needDraw = 
       needUpdate = needResetConnect = needReset = needTrigConnect =
       needTrig = needTraceCtlConnect = needTraceUpdate =
       needXRescale = needBufferScroll = needVectorUpdate =
       needRealUpdate = needBoxRescale = needNewLimits =
       needOriginalLimits = needAutoScaleUpdate = needArraySizeChange = 0;
      drawGridFlag = 0;

      for ( yi=0; yi<xyGraphClass::NUM_Y_AXES; yi++ ) {
        needY1Rescale[yi] = 0;
        numYTraces[yi] = 0;
      }

      traceCtlPv = NULL;
      initialTraceCtlConnection = 1;

      if ( !blankOrComment( traceCtlPvExpStr.getExpanded() ) ) {
        traceCtlPvExists = 1;
        traceCtlPv = the_PV_Factory->create( traceCtlPvExpStr.getExpanded() );
	if ( traceCtlPv ) {
	  traceCtlPv->add_conn_state_callback( traceCtlMonitorConnection, this );
	}
	else {
          fprintf( stderr, "pv create failed for [%s]\n",
           traceCtlPvExpStr.getExpanded() );
        }
      }
      else {
        traceCtlPvExists = 0;
      }

      resetPv = NULL;
      initialResetConnection = 1;

      if ( !blankOrComment( resetPvExpStr.getExpanded() ) ) {
        resetPvExists = 1;
        resetPv = the_PV_Factory->create( resetPvExpStr.getExpanded() );
	if ( resetPv ) {
	  resetPv->add_conn_state_callback( resetMonitorConnection, this );
	}
	else {
          fprintf( stderr, "pv create failed for [%s]\n",
           resetPvExpStr.getExpanded() );
        }
      }
      else {
        resetPvExists = 0;
      }

      trigPv = NULL;
      initialTrigConnection = 1;

      if ( !blankOrComment( trigPvExpStr.getExpanded() ) ) {
        trigPvExists = 1;
        trigPv = the_PV_Factory->create( trigPvExpStr.getExpanded() );
	if ( trigPv ) {
	  trigPv->add_conn_state_callback( trigMonitorConnection, this );
	}
	else {
          fprintf( stderr, "pv create failed for [%s]\n",
           trigPvExpStr.getExpanded() );
        }
      }
      else {
        trigPvExists = 0;
      }

      for ( i=0; i<numTraces; i++ ) {

        xPvCurValue[i] = 0;
        yPvCurValue[i] = 0;

        if ( y2Scale[i] ) {
          yScaleIndex = 1;
          numYTraces[1]++;
	}
	else {
          yScaleIndex = 0;
          numYTraces[0]++;
	}

        if ( i < lowestYScaleIndex[yScaleIndex] ) {
          lowestYScaleIndex[yScaleIndex] = i;
	}

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
        yArrayGotValueCallback[i] = 0;
        xArrayGotValueCallback[i] = 0;
        xPvData[i] = NULL;
        yPvData[i] = NULL;
        xPv[i] = NULL;
        initialXConnection[i] = 1;
        yPv[i] = NULL;
        initialYConnection[i] = 1;
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
        needNPvConnect[i] = 0;
        traceSize[i] = 0;
        xPvCount[i] = 0;
        yPvCount[i] = 0;
        xPvSize[i] = 0;
        yPvSize[i] = 0;
	xPvDim[i] = 10;
	yPvDim[i] = 10;
      }

      for ( i=0; i<numTraces; i++ ) {

        if ( !blankOrComment( yPvExpStr[i].getExpanded() ) ) {

          if ( traceType[i] == XYGC_K_TRACE_XY ) {

            ycArgRec[i].objPtr = (void *) this;
            ycArgRec[i].index = i + XYGC_K_MAX_TRACES;

            connection.addPv();

	    yPv[i] = the_PV_Factory->create( yPvExpStr[i].getExpanded() );
	    if ( yPv[i] ) {
	      yPv[i]->add_conn_state_callback( yMonitorConnection,
               &ycArgRec[i] );
	    }
	    else {
              fprintf( stderr, "pv create failed for [%s]\n",
               yPvExpStr[i].getExpanded() );
            }

            if ( !blankOrComment( xPvExpStr[i].getExpanded() ) ) {

              xcArgRec[i].objPtr = (void *) this;
              xcArgRec[i].index = i;

              connection.addPv();

	      xPv[i] = the_PV_Factory->create( xPvExpStr[i].getExpanded() );
	      if ( xPv[i] ) {
	        xPv[i]->add_conn_state_callback( xMonitorConnection,
                 &xcArgRec[i] );
	      }
	      else {
                fprintf( stderr, "pv create failed for [%s]\n",
                 xPvExpStr[i].getExpanded() );
              }

            }

          }
          else if ( traceType[i] == XYGC_K_TRACE_CHRONOLOGICAL ) {

            ycArgRec[i].objPtr = (void *) this;
            ycArgRec[i].index = i + XYGC_K_MAX_TRACES;

            connection.addPv();

	    yPv[i] = the_PV_Factory->create( yPvExpStr[i].getExpanded() );
	    if ( yPv[i] ) {
	      yPv[i]->add_conn_state_callback( yMonitorConnection,
               &ycArgRec[i] );
	    }
	    else {
              fprintf( stderr, "pv create failed for [%s]\n",
               yPvExpStr[i].getExpanded() );
            }

          }

        }

        if ( !blankOrComment( nPvExpStr[i].getExpanded() ) ) {

          ncArgRec[i].objPtr = (void *) this;
          ncArgRec[i].index = i;

          nPv[i] = the_PV_Factory->create( nPvExpStr[i].getExpanded() );
	  if ( nPv[i] ) {
	    nPv[i]->add_conn_state_callback( nMonitorConnection,
             &ncArgRec[i] );
	  }
	  else {
            fprintf( stderr, "pv create failed for [%s]\n",
             nPvExpStr[i].getExpanded() );
          }

	}

      }

    }

    break;

  case 3:

    if ( autoScaleBothDirections ) {

      if ( autoScaleTimerMs.isNull() ) {
        updateAutoScaleTimerValue = 5000;
      }
      else {
        updateAutoScaleTimerValue = autoScaleTimerMs.value();
      }

      updateAutoScaleTimerValue = autoScaleTimerMs.value();
      if ( updateAutoScaleTimerValue < 1000 ) {
        updateAutoScaleTimerValue = 1000;
      }

      if ( !updateAutoScaleTimerActive ) {
        updateAutoScaleTimer = appAddTimeOut( actWin->appCtx->appContext(),
         updateAutoScaleTimerValue, updateAutoScaleTimerAction, this );
        updateAutoScaleTimerActive = 1;
      }

    }

    break;

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

int i;

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

    if ( updateAutoScaleTimerActive ) {
      if ( updateAutoScaleTimer ) {
        XtRemoveTimeOut( updateAutoScaleTimer );
        updateAutoScaleTimer = 0;
      }
      updateAutoScaleTimerActive = 0;
    }

    if ( ef.formIsPoppedUp() ) {
      ef.popdown();
    }

    if ( efDump.formIsPoppedUp() ) {
      efDump.popdown();
    }

    if ( widgetsCreated ) {
      XtDestroyWidget( popUpMenu );
      widgetsCreated = 0;
    }

    msgDialog.destroy(); 

    if ( traceCtlPv ) {
      traceCtlPv->remove_conn_state_callback( traceCtlMonitorConnection, this );
      traceCtlPv->remove_value_callback( traceCtlValueUpdate, this );
      traceCtlPv->release();
      traceCtlPv = NULL;
    }

    if ( resetPv ) {
      resetPv->remove_conn_state_callback( resetMonitorConnection, this );
      resetPv->remove_value_callback( resetValueUpdate, this );
      resetPv->release();
      resetPv = NULL;
    }

    if ( trigPv ) {
      trigPv->remove_conn_state_callback( trigMonitorConnection, this );
      trigPv->remove_value_callback( trigValueUpdate, this );
      trigPv->release();
      trigPv = NULL;
    }

    for ( i=0; i<numTraces; i++ ) {

      if ( yPv[i] ) {

        yPv[i]->remove_conn_state_callback( yMonitorConnection, &ycArgRec[i] );

        if ( traceType[i] == XYGC_K_TRACE_XY ) {
          yPv[i]->remove_value_callback( yValueUpdate, &yvArgRec[i] );
	}
        else if ( traceType[i] == XYGC_K_TRACE_CHRONOLOGICAL ) {
          yPv[i]->remove_value_callback( yValueWithTimeUpdate, &yvArgRec[i] );
	}

        yPv[i]->release();
        yPv[i] = NULL;

      }

      if ( xPv[i] ) {

        xPv[i]->remove_conn_state_callback( xMonitorConnection, &xcArgRec[i] );

        if ( traceType[i] == XYGC_K_TRACE_XY ) {
          xPv[i]->remove_value_callback( xValueUpdate, &xvArgRec[i] );
	}

        xPv[i]->release();
        xPv[i] = NULL;

      }

      if ( nPv[i] ) {

        nPv[i]->remove_conn_state_callback( nMonitorConnection, &ncArgRec[i] );

        nPv[i]->remove_value_callback( nValueUpdate, &nvArgRec[i] );

        nPv[i]->release();
        nPv[i] = NULL;

      }

      if ( xPvData[i] ) {
        delete[] (char *) xPvData[i];
        xPvData[i] = NULL;
      }

      if ( yPvData[i] ) {
        delete[] (char *) yPvData[i];
        yPvData[i] = NULL;
      }

      if ( plotBuf[i] ) {
        delete[] plotBuf[i];
        plotBuf[i] = NULL;
        plotBufSize[i] = 0;
      }

      if ( plotInfo[i] ) {
        delete[] plotInfo[i];
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

int lx, hx, ly1, ly2, bInc, tInc, xlInc, ylInc, y2lInc, yi;

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

  ly1 = 0;
  yi = 0;
  if ( y1Axis[yi] ) {
    if ( y1AxisStyle[yi] == XYGC_K_AXIS_STYLE_LOG10 ) {
      ly1 = yLog10ScaleWidth( fontTag, fs, curY1Min[yi],
       curY1Max[yi], curY1NumLabelTicks[yi] ) + 4;
    }
    else {
      //ly1 = yScaleWidth( fontTag, fs, curY1Min[yi], curY1Max[yi],
      // curY1NumLabelTicks[yi] ) + 4;
      ly1 = yScaleWidth( fontTag, fs, adjCurY1Min[yi], adjCurY1Max[yi],
       curY1NumLabelTicks[yi] ) + 4;
    }
  }

  ly2 = 0;
  yi = 1;
  if ( y1Axis[yi] ) {
    if ( y1AxisStyle[yi] == XYGC_K_AXIS_STYLE_LOG10 ) {
      ly2 = yLog10ScaleWidth( fontTag, fs, curY1Min[yi],
       curY1Max[yi], curY1NumLabelTicks[yi] ) + 2;
    }
    else {
      //ly2 = yScaleWidth( fontTag, fs, curY1Min[yi], curY1Max[yi],
      // curY1NumLabelTicks[yi] ) + 2;
      ly2 = yScaleWidth( fontTag, fs, adjCurY1Min[yi], adjCurY1Max[yi],
       curY1NumLabelTicks[yi] ) + 2;
    }
  }

  hx = 0;
  lx = 0;
  if ( xAxis ) {
    hx = (int) ( (double) fontHeight * 1.4 );
    if ( xAxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) {
      lx = xScaleMargin( fontTag, fs, pow(10,curXMin), pow(10,curXMax) ) + 1;
    }
    else if ( xAxisStyle == XYGC_K_AXIS_STYLE_TIME_LOG10 ) {
      lx = xScaleMargin( fontTag, fs, curXMin, curXMax ) + 1;
    }
    else if ( ( xAxisStyle == XYGC_K_AXIS_STYLE_TIME ) &&
              ( xAxisTimeFormat == XYGC_K_AXIS_TIME_FMT_MMDDYY_HHMMSS ) ) {
      lx = xTimeScaleMargin( fontTag, fs, curXMin, curXMax ) + 1;
      hx += fontHeight;
    }
    else {
      lx = xScaleMargin( fontTag, fs, curXMin, curXMax ) + 1;
    }
  }

  if ( y1Axis[0] && !y1Axis[1] ) {
    ly2 = 10;
  }

  if ( !y1Axis[0] && y1Axis[1] ) {
    ly1 = 10;
  }

  if ( ly1 < lx ) ly1 = lx;
  if ( ly2 < lx ) ly2 = lx;

  if ( border )
    bInc = 1;
  else
    bInc = 0;

  if ( blank( graphTitle.getExpanded() ) ) {
    if ( y1Axis[0] || y1Axis[1] ) {
      tInc = (int) ( fontHeight / 2 ) + 1;
    }
    else {
      tInc = 0;
    }
  }
  else
    tInc = fontHeight + 1;

  if ( blank( xLabel.getExpanded() ) ) {
    if ( !xAxis && ( y1Axis[0] || y1Axis[1] ) ) {
      xlInc = (int) ( fontHeight / 2 ) + 1;
    }
    else {
      xlInc = 0;
    }
  }
  else
    xlInc = fontHeight + 1;

  if ( !y1Axis[0] || blank( yLabel.getExpanded() ) )
    ylInc = 0;
  else
    ylInc = fontHeight + 1;

  if ( !y1Axis[1] || blank( y2Label.getExpanded() ) )
    y2lInc = 0;
  else
    y2lInc = fontHeight + 1;

  plotAreaX = ly1 + bInc + ylInc;
  plotAreaW = w - bInc - bInc - ylInc - ly1 - ly2 - y2lInc;

  plotAreaY = 0 + bInc + (int) ( 1.5 * tInc );
  plotAreaH = h - bInc - bInc - (int) ( 1.5 * tInc ) -
   (int) ( 1.5 * xlInc ) - (int) ( 1.5 * hx );

}

void xyGraphClass::btnDrag (
  XMotionEvent *me,
  int _x,
  int _y,
  int buttonState,
  int buttonNumber )
{

int rsw, rsh;
int pmX, pmY;
int tmpX, tmpY;

  if ( !enabled ) return;

  pmX = me->x - this->x;
  pmY = me->y - this->y;

  if ( doingBoxRescale ) {

    actWin->executeGc.saveFg();
    actWin->executeGc.setLineWidth(1);
    actWin->executeGc.setLineStyle( LineSolid );
    actWin->executeGc.setFGforGivenBG (
     actWin->ci->pix(fgColor), actWin->ci->pix(bgColor) );

    if ( oldRescaleBoxW >= 0 ) {
      tmpX = rescaleBoxX0;
    }
    else {
      tmpX = rescaleBoxX0 + oldRescaleBoxW;
    }

    if ( oldRescaleBoxH >= 0 ) {
      tmpY = rescaleBoxY0;
    }
    else {
      tmpY = rescaleBoxY0 + oldRescaleBoxH;
    }

    XDrawRectangle( actWin->d, pixmap,
     actWin->executeGc.xorGC(), tmpX, tmpY,
     abs(oldRescaleBoxW), abs(oldRescaleBoxH) );

    rescaleBoxX1 = pmX;
    rescaleBoxY1 = pmY;
    rsw = rescaleBoxX1 - rescaleBoxX0;
    rsh = rescaleBoxY1 - rescaleBoxY0;
    oldRescaleBoxW = rsw;
    oldRescaleBoxH = rsh;

    if ( rsw >= 0 ) {
      tmpX = rescaleBoxX0;
    }
    else {
      tmpX = rescaleBoxX0 + rsw;
    }

    if ( rsh >= 0 ) {
      tmpY = rescaleBoxY0;
    }
    else {
      tmpY = rescaleBoxY0 + rsh;
    }

    XDrawRectangle( actWin->d, pixmap,
     actWin->executeGc.xorGC(), tmpX, tmpY,
     abs(rsw), abs(rsh) );

    actWin->executeGc.restoreFg();

    actWin->appCtx->proc->lock();
    needRealUpdate = 1;
    actWin->addDefExeNode( aglPtr );
    actWin->appCtx->proc->unlock();

  }

}

void xyGraphClass::btnUp (
  XButtonEvent *be,
  int _x,
  int _y,
  int buttonState,
  int buttonNumber,
  int *action )
{

int pmX, pmY;
double dx0, dy0, dx1, dy1;
int yi = 0;
int tmpX, tmpY;

  *action = 0;

  if ( !enabled ) return;

  pmX = be->x - this->x;
  pmY = be->y - this->y;

  if ( doingBoxRescale ) {

    actWin->executeGc.saveFg();
    actWin->executeGc.setLineWidth(1);
    actWin->executeGc.setLineStyle( LineSolid );
    actWin->executeGc.setFGforGivenBG (
     actWin->ci->pix(fgColor), actWin->ci->pix(bgColor) );

    if ( oldRescaleBoxW >= 0 ) {
      tmpX = rescaleBoxX0;
    }
    else {
      tmpX = rescaleBoxX0 + oldRescaleBoxW;
    }

    if ( oldRescaleBoxH >= 0 ) {
      tmpY = rescaleBoxY0;
    }
    else {
      tmpY = rescaleBoxY0 + oldRescaleBoxH;
    }

    XDrawRectangle( actWin->d, pixmap,
     actWin->executeGc.xorGC(), tmpX, tmpY,
     abs(oldRescaleBoxW), abs(oldRescaleBoxH) );

    actWin->executeGc.restoreFg();

    doingBoxRescale = 0;

    if ( ( abs(oldRescaleBoxW) < 5 ) || ( abs(oldRescaleBoxH) < 5 ) ) return;

    rescaleBoxX1 = pmX;

    dx0 = ( rescaleBoxX0 - xOffset[0] ) / xFactor[0] + curXMin;
    dx1 = ( rescaleBoxX1 - xOffset[0] ) / xFactor[0] + curXMin;

    if ( dx0 < dx1 ) {
      boxXMin = dx0;
      boxXMax = dx1;
    }
    else {
      boxXMin = dx1;
      boxXMax = dx0;
    }

    kpXMinEfDouble.setNull(0);
    kpXMaxEfDouble.setNull(0);

    for ( yi=0; yi<xyGraphClass::NUM_Y_AXES; yi++ ) {

      if ( numYTraces[yi] > 0 ) {

        rescaleBoxY1 = pmY;

        dy0 = ( plotAreaH - rescaleBoxY0 +
         y1Offset[yi][lowestYScaleIndex[yi]] ) /
         y1Factor[yi][lowestYScaleIndex[yi]] + curY1Min[yi];
        dy1 = ( plotAreaH - rescaleBoxY1 +
         y1Offset[yi][lowestYScaleIndex[yi]] ) /
         y1Factor[yi][lowestYScaleIndex[yi]] + curY1Min[yi];

        if ( dy0 < dy1 ) {
          boxYMin[yi] = dy0;
          boxYMax[yi] = dy1;
        }
        else {
          boxYMin[yi] = dy1;
          boxYMax[yi] = dy0;
        }

        kpY1MinEfDouble[yi].setNull(0);
        kpY1MaxEfDouble[yi].setNull(0);

      }

    }

    actWin->appCtx->proc->lock();
    needBoxRescale = 1;
    actWin->addDefExeNode( aglPtr );
    actWin->appCtx->proc->unlock();

  }

  if ( ( buttonNumber == 3 ) &&
      !( buttonState & ShiftMask ) &&
      !( buttonState & ControlMask ) ) {

    popupMenuX = be->x_root;
    popupMenuY = be->y_root;
    XmMenuPosition( popUpMenu, be );
    XtManageChild( popUpMenu );
    return;

#if 0
    if ( !firstBoxRescale ) {

      firstBoxRescale = 1;
      boxXMin = savedXMin;
      boxXMax = savedXMax;
      for ( yi=0; yi<xyGraphClass::NUM_Y_AXES; yi++ ) {
        if ( numYTraces[yi] > 0 ) {
          boxYMin[yi] = savedYMin[yi];
          boxYMax[yi] = savedYMax[yi];
          kpY1MinEfDouble[yi].setNull( savedYMinNullState[yi] );
          kpY1MaxEfDouble[yi].setNull( savedYMaxNullState[yi] );
	}
      }
      kpXMinEfDouble.setNull( savedXMinNullState );
      kpXMaxEfDouble.setNull( savedXMaxNullState );

      actWin->appCtx->proc->lock();
      needBoxRescale = 1;
      actWin->addDefExeNode( aglPtr );
      actWin->appCtx->proc->unlock();

    }
    else {

      actWin->appCtx->proc->lock();
      needNewLimits = 1;
      actWin->addDefExeNode( aglPtr );
      actWin->appCtx->proc->unlock();

    }
#endif


  }
  else if ( ( buttonNumber == 3 ) &&
      ( buttonState & ShiftMask ) &&
      !( buttonState & ControlMask ) ) {

#if 0
    if ( !firstBoxRescale ) {

      firstBoxRescale = 1;
      boxXMin = savedXMin;
      boxXMax = savedXMax;
      for ( yi=0; yi<xyGraphClass::NUM_Y_AXES; yi++ ) {
        if ( numYTraces[yi] > 0 ) {
          boxYMin[yi] = savedYMin[yi];
          boxYMax[yi] = savedYMax[yi];
	}
      }

    }
#endif

    for ( yi=0; yi<xyGraphClass::NUM_Y_AXES; yi++ ) {
      if ( numYTraces[yi] > 0 ) {
        kpY1MinEfDouble[yi].setNull(1);
        kpY1MaxEfDouble[yi].setNull(1);
      }
    }
    kpXMinEfDouble.setNull(1);
    kpXMaxEfDouble.setNull(1);

    actWin->appCtx->proc->lock();
    needOriginalLimits = 1;
    actWin->addDefExeNode( aglPtr );
    actWin->appCtx->proc->unlock();

  }

  if ( msgDialogPopedUp ) {
    msgDialog.popdown();
    msgDialogPopedUp = 0;
    return;
  }

}

void xyGraphClass::btnDown (
  XButtonEvent *be,
  int _x,
  int _y,
  int buttonState,
  int buttonNumber,
  int *action )
{

double dxValue, dyValue, dyValue2;
char buf[63+1], xBuf[63+1], xBuf2[63+1], y1Buf[63+1], y2Buf[63+1];
int pmX, pmY, ifrac;
int yi, usebuf2;
time_t t;
struct tm ts;
Widget parent;

  if ( useAppTopParent() ) {
    parent = actWin->appCtx->apptop();
  }
  else {
    parent = actWin->top;
  }

  *action = 0;

  if ( !enabled ) return;

  pmX = be->x - this->x;
  pmY = be->y - this->y;

  if ( ( buttonNumber == 1 ) &&
       ( buttonState & ShiftMask ) ) {

    usebuf2 = 0;

    if ( ( pmX > plotAreaX ) && ( pmX < plotAreaX+plotAreaW ) &&
         ( pmY > plotAreaY ) && ( pmY < plotAreaY+plotAreaH ) ) {

      if ( ( xAxisStyle == XYGC_K_AXIS_STYLE_TIME ) &&
         ( xAxisTimeFormat != XYGC_K_AXIS_TIME_FMT_SEC ) ) {

        dxValue = ( pmX - xOffset[0] ) / xFactor[0] + curXMin;

        {

          edmTime base( (const unsigned long) ( curSec ),
           (const unsigned long) curNsec );
          edmTime cur( (double) dxValue );
          edmTime total = base + cur;

          t = total.getSec() + timeOffset;

	}

        localtime_r( &t, &ts );
        ifrac = (int) rint( ( dxValue - floor( dxValue ) ) * 100.0 );
        if ( ifrac == 100 ) ifrac = 99;
        if ( ifrac > 0 ) {
          sprintf( xBuf, "%02d:%02d:%02d.%02d", ts.tm_hour, ts.tm_min,
           ts.tm_sec, ifrac );
        }
        else {
          sprintf( xBuf, "%02d:%02d:%02d", ts.tm_hour, ts.tm_min,
           ts.tm_sec );
        }

        sprintf( xBuf2, "%02d-%02d-%04d", ts.tm_mon+1, ts.tm_mday,
         ts.tm_year+1900 );
        usebuf2 = 1;

      }
      else {

        dxValue = ( pmX - xOffset[0] ) / xFactor[0] + curXMin;

        if ( ( xAxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) ||
             ( xAxisStyle == XYGC_K_AXIS_STYLE_TIME_LOG10 ) ) {
          dxValue = pow(10,dxValue);
        }

        sprintf( xBuf, "%-.6g", dxValue );

      }

      yi = 0;
      strcpy( y1Buf, "" );
      //if ( y1Axis[yi] && ( numYTraces[yi] > 0 ) ) {
      if ( numYTraces[yi] > 0 ) {
        dyValue = ( plotAreaH - pmY + y1Offset[yi][lowestYScaleIndex[yi]] )
         / y1Factor[yi][lowestYScaleIndex[yi]] + curY1Min[yi];
        if ( y1AxisStyle[yi] == XYGC_K_AXIS_STYLE_LOG10 ) {
          dyValue = pow(10,dyValue);
        }
        sprintf( y1Buf, " %-.6g", dyValue );
      }

      yi = 1;
      strcpy( y2Buf, "" );
      //if ( y1Axis[yi] && ( numYTraces[yi] > 0 ) ) {
      if ( numYTraces[yi] > 0 ) {
        dyValue2 = ( plotAreaH - pmY + y1Offset[yi][lowestYScaleIndex[yi]] )
         / y1Factor[yi][lowestYScaleIndex[yi]] + curY1Min[yi];
        if ( y1AxisStyle[yi] == XYGC_K_AXIS_STYLE_LOG10 ) {
          dyValue2 = pow(10,dyValue2);
        }
        if ( strcmp( y1Buf, "" ) != 0 ) {
          sprintf( y2Buf, ", %-.6g", dyValue2 );
	}
	else {
          sprintf( y2Buf, " %-.6g (y2)", dyValue2 );
	}
      }
 
      if ( msgDialogPopedUp ) {
        msgDialog.popdown();
      }
      if ( usebuf2 ) {
        sprintf( buf, "[ %s %s,%s%s ]", xBuf2, xBuf, y1Buf, y2Buf );
      }
      else {
        sprintf( buf, "[ %s,%s%s ]", xBuf, y1Buf, y2Buf );
      }
      msgDialog.popup( buf, this->x+_x-be->x+actWin->xPos(),
       this->y+_y-be->y+actWin->yPos() );
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
        for ( yi=0; yi<xyGraphClass::NUM_Y_AXES; yi++ ) {
          if ( numYTraces[yi] > 0 ) {
            savedYMin[yi] = curY1Min[yi];
            savedYMax[yi] = curY1Max[yi];
            savedYMinNullState[yi] = kpY1MinEfDouble[yi].isNull();
            savedYMaxNullState[yi] = kpY1MaxEfDouble[yi].isNull();
	  }
	}
        savedXMinNullState = kpXMinEfDouble.isNull();
        savedXMaxNullState = kpXMaxEfDouble.isNull();
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
          kp.create( parent, be->x_root, be->y_root, "",
           &kpXMin, (void *) this,
           (XtCallbackProc) setKpXMinDoubleValue,
           (XtCallbackProc) cancelKpXMin );
        }
      }

      if ( ( xMaxX0 <= pmX ) && ( xMaxX1 >= pmX ) &&
           ( xMaxY0 <= pmY ) && ( xMaxY1 >= pmY ) ) {
        if ( !kp.isPoppedUp() ) {
          kp.create( parent, be->x_root, be->y_root, "",
           &kpXMax, (void *) this,
           (XtCallbackProc) setKpXMaxDoubleValue,
           (XtCallbackProc) cancelKpXMax );
        }
      }



      yi = 0;
      if ( ( y1MinX0[yi] <= pmX ) && ( y1MinX1[yi] >= pmX ) &&
           ( y1MinY0[yi] <= pmY ) && ( y1MinY1[yi] >= pmY ) ) {
        if ( !kp.isPoppedUp() ) {
          kp.create( parent, be->x_root, be->y_root, "",
           &kpY1Min[yi], (void *) this,
           (XtCallbackProc) setKpY1MinDoubleValue,
           (XtCallbackProc) cancelKpY1Min );
        }
      }

      if ( ( y1MaxX0[yi] <= pmX ) && ( y1MaxX1[yi] >= pmX ) &&
           ( y1MaxY0[yi] <= pmY ) && ( y1MaxY1[yi] >= pmY ) ) {
        if ( !kp.isPoppedUp() ) {
          kp.create( parent, be->x_root, be->y_root, "",
           &kpY1Max[yi], (void *) this,
           (XtCallbackProc) setKpY1MaxDoubleValue,
           (XtCallbackProc) cancelKpY1Max );
        }
      }



      yi = 1;
      if ( ( y1MinX0[yi] <= pmX ) && ( y1MinX1[yi] >= pmX ) &&
           ( y1MinY0[yi] <= pmY ) && ( y1MinY1[yi] >= pmY ) ) {
        if ( !kp.isPoppedUp() ) {
          kp.create( parent, be->x_root, be->y_root, "",
           &kpY1Min[yi], (void *) this,
           (XtCallbackProc) setKpY2MinDoubleValue,
           (XtCallbackProc) cancelKpY2Min );
        }
      }

      if ( ( y1MaxX0[yi] <= pmX ) && ( y1MaxX1[yi] >= pmX ) &&
           ( y1MaxY0[yi] <= pmY ) && ( y1MaxY1[yi] >= pmY ) ) {
        if ( !kp.isPoppedUp() ) {
          kp.create( parent, be->x_root, be->y_root, "",
           &kpY1Max[yi], (void *) this,
           (XtCallbackProc) setKpY2MaxDoubleValue,
           (XtCallbackProc) cancelKpY2Max );
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

int i, ii, nc, ni, nu, nvu, nru, nr, ne, nd, ntcc, ntu, nrstc, nrst, ntrgc,
 tmpC, ntrg, nxrescl, nbs, nbrescl, nnl, nol, nasu, nnc, nni, nasc,
 doRescale, anyRescale, size,
 ny1rescl[NUM_Y_AXES], num, maxDim;
double dyValue, dxValue, range, oneMax, oldXMin, xmin, xmax, ymin[2], ymax[2],
 scaledX, scaledY;
double yZero;
char format[31+1];
int yi, yScaleIndex, allChronological;

double checkXMin, checkXMax, checkY1Min[NUM_Y_AXES], checkY1Max[NUM_Y_AXES],
 diff, maxDiff;
int autoScaleX=0, autoScaleY[NUM_Y_AXES];

  if ( actWin->isIconified ) return;

  xmin = 1.0e30;
  xmax = -1.0e30;
  ymin[0] = 1.0e30;
  ymin[1] = 1.0e30;
  ymax[0] = -1.0e30;
  ymax[1] = -1.0e30;
  allChronological = 0;

  actWin->appCtx->proc->lock();

  nc = needConnect; needConnect = 0;
  ni = needInit; needInit = 0;
  nu = needUpdate; needUpdate = 0;
  nvu = needVectorUpdate; needVectorUpdate = 0;
  nru = needRealUpdate; needRealUpdate = 0;
  nr = needRefresh; needRefresh = 0;
  ne = needErase; needErase = 0;
  nd = needDraw; needDraw = 0;
  ntcc = needTraceCtlConnect; needTraceCtlConnect = 0;
  ntu = needTraceUpdate; needTraceUpdate = 0;
  nrstc = needResetConnect; needResetConnect = 0;
  nrst = needReset; needReset = 0;
  ntrgc = needTrigConnect; needTrigConnect = 0;
  ntrg = needTrig; needTrig = 0;
  nxrescl = needXRescale; needXRescale = 0;
  nbs = needBufferScroll; needBufferScroll = 0;
  nbrescl = needBoxRescale; needBoxRescale = 0;
  nnl = needNewLimits; needNewLimits = 0;
  nol = needOriginalLimits; needOriginalLimits = 0;
  nasu = needAutoScaleUpdate; needAutoScaleUpdate = 0;
  nnc = needNConnect; needNConnect = 0;
  nni = needNInit; needNInit = 0;
  nasc = needArraySizeChange; needArraySizeChange = 0;
  actWin->remDefExeNode( aglPtr );

  for ( yi=0; yi<xyGraphClass::NUM_Y_AXES; yi++ ) {
    ny1rescl[yi] = needY1Rescale[yi]; needY1Rescale[yi] = 0;
  }

  actWin->appCtx->proc->unlock();

  doRescale = 0;

  if ( !activeMode ) return;

  if ( nnc ) {

    for ( i=0; i<numTraces; i++ ) {

      if ( needNPvConnect[i] == 1 ) {
        needNPvConnect[i] = 2;
        nni = 1;
      }

    }

  }

  if ( nni ) {

    for ( i=0; i<numTraces; i++ ) {

      if ( needNPvConnect[i] == 2 ) {
        nvArgRec[i].objPtr = (void *) this;
        nvArgRec[i].index = i;
        nPv[i]->add_value_callback( nValueUpdate, &nvArgRec[i] );
        needNPvConnect[i] = 3;
      }

    }

  }

  if ( nc ) {

    for ( i=0; i<numTraces; i++ ) {

      if ( yPv[i] ) {

        if ( yPv[i]->is_valid() ) {

          yPvType[i] = (int) yPv[i]->get_specific_type().type;
          //yPvCount[i] = (int) yPv[i]->get_dimension();
          yPvCount[i] = (int) yPv[i]->get_count();
          yPvDim[i] = (int) yPv[i]->get_dimension();

          if ( traceSize[i] < 0 ) traceSize[i] = 0;

          if ( traceSize[i] > yPvDim[i] ) {
            yPvCount[i] = yPvDim[i];
          }
          else if ( traceSize[i] > 0 ) {
            yPvCount[i] = traceSize[i];
          }

          yPvSize[i] = yPvDim[i] * (int) yPv[i]->get_specific_type().size / 8;
          dbYMin[i] = yPv[i]->get_lower_disp_limit();
          dbYMax[i] = yPv[i]->get_upper_disp_limit();
          dbYPrec[i] = yPv[i]->get_precision();

	  //printf( "y pv ele size = %-d\n", (int) yPv[i]->get_specific_type().size / 8 );
	  //printf( "y pv dim = %-d\n", yPvDim[i] );
	  //printf( "y pv size = %-d\n\n", yPvSize[i] );

          ni = 1;
          yArrayNeedInit[i] = 1;

        }

      }

      if ( xPv[i] ) {

        if ( xPv[i]->is_valid() ) {

          xPvType[i] = (int) xPv[i]->get_specific_type().type;
          //xPvCount[i] = (int) xPv[i]->get_dimension();
          xPvCount[i] = (int) xPv[i]->get_count();
          xPvDim[i] = (int) xPv[i]->get_dimension();

          if ( traceSize[i] < 0 ) traceSize[i] = 0;

          if ( traceSize[i] > xPvDim[i] ) {
            xPvCount[i] = xPvDim[i];
          }
          else if ( traceSize[i] > 0 ) {
            xPvCount[i] = traceSize[i];
          }

          xPvSize[i] = xPvDim[i] * (int) xPv[i]->get_specific_type().size / 8;
          dbXMin[i] = xPv[i]->get_lower_disp_limit();
          dbXMax[i] = xPv[i]->get_upper_disp_limit();
          dbXPrec[i] = xPv[i]->get_precision();

	  //printf( "x pv ele size = %-d\n", (int) xPv[i]->get_specific_type().size / 8 );
	  //printf( "x pv dim = %-d\n", xPvDim[i] );
	  //printf( "x pv size = %-d\n\n", xPvSize[i] );

          ni = 1;
          xArrayNeedInit[i] = 1;

        }

      }

    }

  }

  if ( ntcc ) {

    if ( initialTraceCtlConnection ) {

      initialTraceCtlConnection = 0;

      traceCtlPv->add_value_callback( traceCtlValueUpdate, this );

    }

  }

  if ( ntu ) {

    regenBuffer();
    fullRefresh();
    nru = 1;

  }

  if ( nrstc ) {

    if ( initialResetConnection ) {

      initialResetConnection = 0;

      resetPv->add_value_callback( resetValueUpdate, this );

    }

  }

  if ( ntrgc ) {

    if ( initialTrigConnection ) {

      initialTrigConnection = 0;

      trigPv->add_value_callback( trigValueUpdate, this );

    }

  }

  if ( ni ) {

    for ( i=0; i<numTraces; i++ ) {

      if (
           ( ( plotStyle[i] == XYGC_K_PLOT_STYLE_LINE ) ||
             ( plotStyle[i] == XYGC_K_PLOT_STYLE_POINT ) ) &&
           ( traceType[i] == XYGC_K_TRACE_CHRONOLOGICAL ) &&
           ( !forceVector[i] && ( yPv[i]->get_dimension() == 1 ) ) && // must be scalar; use y here,
                                   // x is not used for chonological
           ( ( xAxisStyle == XYGC_K_AXIS_STYLE_LINEAR ) ||
             ( xAxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) )
      ) {

        special[i] = 1;

      }
      else {

        special[i] = 0;

      }

      yi = 0;
      if ( y2Scale[i] ) yi = 1;

      // ------------------------------------------------------------------
      // pass one - init, pass two - add events

      // pass one
      if ( yArrayNeedInit[i] ) {

        if ( i == lowestYScaleIndex[yi] ) {
          if ( y1AxisSource[yi] == XYGC_K_FROM_PV ) {
            curY1Min[yi] = dbYMin[i];
            curY1Max[yi] = dbYMax[i];
            curY1Prec[yi] = dbYPrec[i];
            if ( y1AxisStyle[yi] == XYGC_K_AXIS_STYLE_LOG10 ) {
              if ( curY1Min[yi] <= 0 ) curY1Min[yi] = 1e-2;
              if ( curY1Max[yi] <= 0 ) curY1Max[yi] = 1e-1;
              curY1Min[yi] = loc_log10(  curY1Min[yi]  );
              curY1Max[yi] = loc_log10(  curY1Max[yi]  );
            }
          }
        }

        //yArrayNeedInit[i] = 0;

        xFactor[i] =
         (double) ( plotAreaW ) / ( curXMax - curXMin );
        xOffset[i] = plotAreaX;

        y1Factor[yi][i] =
         (double) ( plotAreaH ) / ( curY1Max[yi] - curY1Min[yi] );
        y1Offset[yi][i] = plotAreaY;

        yvArgRec[i].objPtr = (void *) this;
        yvArgRec[i].index = i;

        if ( !yPvData[i] ) {

          if ( forceVector[i] || ( yPv[i]->get_dimension() > 1 ) ) { // vector

            maxDim = yPvDim[i];
            if ( xPvDim[i] > maxDim ) maxDim = xPvDim[i];

            yPvData[i] = (void *) new char[yPvSize[i]+80];
            for ( ii=0; ii<yPvSize[i]; ii++ ) ( (char *) yPvData[i] )[ii] = 0;

            size = (plotAreaX+plotAreaW)*4+10;
            if ( 3 * maxDim + 10 > size ) {
              size = 3 * maxDim + 10;
	    }

            plotBuf[i] = (XPoint *) new XPoint[size];

            plotBufSize[i] = yPvDim[i]+1; // used with plotInfo in scope mode

            size = plotAreaX+plotAreaW+10;

            if ( 2 * maxDim + 10 > size ) {
              size = 2 * maxDim + 10;
	    }

            plotInfo[i] =
             (plotInfoPtr) new plotInfoType[size];
            plotInfoSize[i] = plotAreaX + plotAreaW;

            initPlotInfo( i );

          }
          else { // scalar

            if ( count < 2 )
	      tmpC = 2;
	    else
	      tmpC = count;

            bufferScrollSize = (int) ( (double) tmpC * 0.1 );
            if ( bufferScrollSize < 1 ) bufferScrollSize = 1;

            yPvData[i] = (void *) new char[yPvSize[i]*(tmpC+10)];
            for ( ii=0; ii<yPvSize[i]*(tmpC+10); ii++ ) ( (char *) yPvData[i] )[ii] = 0;

            size = (plotAreaX+plotAreaW)*4+10;
            if ( 2*tmpC+10 > size ) size = 2*tmpC+10;
            plotBuf[i] = (XPoint *) new XPoint[size];

            plotBufSize[i] = tmpC+1; // used with plotInfo in scope mode

            size = plotAreaX+plotAreaW+10;
            if ( 2*tmpC+10 > size ) size = 2*tmpC+10;
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
                if ( curXMin <= 0 ) curXMin = 1e-2;
                if ( curXMax <= 0 ) curXMax = 1e-1;
                curXMin = loc_log10(  curXMin  );
                curXMax = loc_log10(  curXMax  );
              }
              else if ( xAxisStyle == XYGC_K_AXIS_STYLE_TIME_LOG10 ) {
                if ( curXMin <= 0 ) curXMin = 1e-2;
                if ( curXMax <= 0 ) curXMax = 1e-1;
                curXMin = loc_log10(  curXMin  );
                curXMax = loc_log10(  curXMax  );
              }
            }
          }

          //if ( initialYConnection[i] ) {

	  //  initialYConnection[i] = 0;

	  //  yPv[i]->add_value_callback( yValueUpdate, &yvArgRec[i] );

	  //}

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

          //if ( forceVector[i] || ( yPv[i]->get_dimension() > 1 ) ) { // vector

          //  if ( initialYConnection[i] ) {

	  //    initialYConnection[i] = 0;

	  //    yPv[i]->add_value_callback( yValueWithTimeUpdate,
          //     &yvArgRec[i] );

	  //  }

          //}
          //else {

          //  if ( initialYConnection[i] ) {

	  //    initialYConnection[i] = 0;

	  //    yPv[i]->add_value_callback( yValueWithTimeUpdate,
          //     &yvArgRec[i] );

	  //  }

          //}

        }

      }

      if ( xArrayNeedInit[i] ) {

        //xArrayNeedInit[i] = 0;

        xvArgRec[i].objPtr = (void *) this;
        xvArgRec[i].index = i;

        if ( !xPvData[i] ) {

          if ( forceVector[i] || ( xPv[i]->get_dimension() > 1 ) ) { // vector

            xPvData[i] = (void *) new char[xPvSize[i]+80];

          }
          else { // scalar

            if ( count < 2 )
	      tmpC = 2;
	    else
	      tmpC = count;

            xPvData[i] = (void *) new char[xPvSize[i]*(tmpC+10)];

          }

        }

        //if ( traceType[i] == XYGC_K_TRACE_XY ) { // sanity check

        //  if ( initialXConnection[i] ) {

	//    initialXConnection[i] = 0;

	//    xPv[i]->add_value_callback( xValueUpdate, &xvArgRec[i] );

	//  }

        //}

      }

      // end of pass one
      // ------------------------------------------------------------------

      // pass two
      if ( yArrayNeedInit[i] ) {

        yArrayNeedInit[i] = 0;

        if ( traceType[i] == XYGC_K_TRACE_XY ) {

          if ( initialYConnection[i] ) {

	    initialYConnection[i] = 0;

	    yPv[i]->add_value_callback( yValueUpdate, &yvArgRec[i] );

	  }

        }
        else if ( traceType[i] == XYGC_K_TRACE_CHRONOLOGICAL ) {

          if ( forceVector[i] || ( yPv[i]->get_dimension() > 1 ) ) { // vector

            if ( initialYConnection[i] ) {

	      initialYConnection[i] = 0;

	      yPv[i]->add_value_callback( yValueWithTimeUpdate,
               &yvArgRec[i] );

	    }

          }
          else {

            if ( initialYConnection[i] ) {

	      initialYConnection[i] = 0;

	      yPv[i]->add_value_callback( yValueWithTimeUpdate,
               &yvArgRec[i] );

	    }

          }

        }

      }

      if ( xArrayNeedInit[i] ) {

        xArrayNeedInit[i] = 0;

        if ( traceType[i] == XYGC_K_TRACE_XY ) { // sanity check

          if ( initialXConnection[i] ) {

	    initialXConnection[i] = 0;

	    xPv[i]->add_value_callback( xValueUpdate, &xvArgRec[i] );

	  }

        }

      }

      // end of pass two
      // ------------------------------------------------------------------

      if ( count == 1 ) {
        arrayNumPoints[i] = 0;
        curNpts[i] = 0;
      }
      else {
        arrayHead[i] = 0;
        arrayTail[i] = 0;
        arrayNumPoints[i] = 0;
        curNpts[i] = 0;
      }

    }

    init = 1;

    fullRefresh();

    nol = 1;

  }

  // this needs to come before nbs, nxresc, ny1resc
  if ( nvu ) {

    drawGridFlag = 1;

    anyRescale = 0;
    for ( i=0; i<numTraces; i++ ) {

      if ( yArrayGotValueCallback[i] == 1 ) {
        yArrayGotValueCallback[i] = 2;
        yArrayNeedUpdate[i] = 1;
      }

      if ( xArrayGotValueCallback[i] == 1 ) {
        xArrayGotValueCallback[i] = 2;
        xArrayNeedUpdate[i] = 1;
      }

      if ( forceVector[i] || ( yPv[i]->get_dimension() > 1 ) ) {

        if ( traceType[i] == XYGC_K_TRACE_CHRONOLOGICAL ) {

          if ( yArrayNeedUpdate[i] ) {

            xArrayNeedUpdate[i] = 1;

            genChronoVector( i, &doRescale );
            if ( doRescale ) anyRescale = 1;

          }

        }
        else {

          if ( yArrayNeedUpdate[i] || xArrayNeedUpdate[i] ) {

            switch ( plotUpdateMode[i] ) {

            case XYGC_K_UPDATE_ON_TRIG:
              genXyVector( i, &doRescale );
              if ( doRescale ) anyRescale = 1;
              break;

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

    curXMin = boxXMin;
    curXMax = boxXMax;

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
      get_log10_scale_params1( curXMin, curXMax, &adjCurXMin, &adjCurXMax,
       &curXNumLabelTicks, &curXMajorsPerLabel, &curXMinorsPerMajor,
       format );
      if ( ( xAxisSource == XYGC_K_AUTOSCALE ) && kpXMinEfDouble.isNull() ) {
        curXMin = adjCurXMin;
      }
      if ( ( xAxisSource == XYGC_K_AUTOSCALE ) && kpXMaxEfDouble.isNull() ) {
        curXMax = adjCurXMax;
      }
    }
    else if ( xAxisStyle == XYGC_K_AXIS_STYLE_TIME_LOG10 ) {
      get_log10_scale_params1( curXMin, curXMax, &adjCurXMin, &adjCurXMax,
       &curXNumLabelTicks, &curXMajorsPerLabel, &curXMinorsPerMajor,
       format );
      if ( ( xAxisSource == XYGC_K_AUTOSCALE ) && kpXMinEfDouble.isNull() ) {
        curXMin = adjCurXMin;
      }
      if ( ( xAxisSource == XYGC_K_AUTOSCALE ) && kpXMaxEfDouble.isNull() ) {
        curXMax = adjCurXMax;
      }
    }
    else {
      get_scale_params1( curXMin, curXMax,
       &adjCurXMin, &adjCurXMax,
       &curXNumLabelTicks, &curXMajorsPerLabel, &curXMinorsPerMajor,
       format );
      if ( xAxisSmoothing == XYGC_K_NO_SMOOTHING ) {
        adjCurXMin = curXMin;
        adjCurXMax = curXMax;
      }
      if ( ( xAxisSource == XYGC_K_AUTOSCALE ) && kpXMinEfDouble.isNull() ) {
        curXMin = adjCurXMin;
      }
      if ( ( xAxisSource == XYGC_K_AUTOSCALE ) && kpXMaxEfDouble.isNull() ) {
        curXMax = adjCurXMax;
      }
    }
  
    for ( yi=0; yi<xyGraphClass::NUM_Y_AXES; yi++ ) {

      if ( numYTraces[yi] > 0 ) {

        curY1Min[yi] = boxYMin[yi];
        curY1Max[yi] = boxYMax[yi];

        if ( curY1Min[yi] >= curY1Max[yi] ) {
          if ( y1AxisStyle[yi] == XYGC_K_AXIS_STYLE_LOG10 ) {
            curY1Max[yi] = curY1Min[yi] * 10.0;
          }
          else {
            curY1Max[yi] = curY1Min[yi] * 2.0;
          }
        }
        if ( curY1Min[yi] >= curY1Max[yi] ) { // in case y Min is 0
          curY1Max[yi] = curY1Min[yi] + 1.0;
        }

        if ( y1AxisStyle[yi] == XYGC_K_AXIS_STYLE_LOG10 ) {
          get_log10_scale_params1( curY1Min[yi], curY1Max[yi], &adjCurY1Min[yi], &adjCurY1Max[yi],
           &curY1NumLabelTicks[yi], &curY1MajorsPerLabel[yi],
           &curY1MinorsPerMajor[yi], format );
          if ( ( y1AxisSource[yi] == XYGC_K_AUTOSCALE ) && kpY1MinEfDouble[yi].isNull() ) {
            curY1Min[yi] = adjCurY1Min[yi];
          }
          if ( ( y1AxisSource[yi] == XYGC_K_AUTOSCALE ) && kpY1MaxEfDouble[yi].isNull() ) {
            curY1Max[yi] = adjCurY1Max[yi];
          }
        }
        else {
          get_scale_params1( curY1Min[yi], curY1Max[yi], &adjCurY1Min[yi],
           &adjCurY1Max[yi], &curY1NumLabelTicks[yi], &curY1MajorsPerLabel[yi],
           &curY1MinorsPerMajor[yi], format );
          if ( y1AxisSmoothing[yi] == XYGC_K_NO_SMOOTHING ) {
            adjCurY1Min[yi] = curY1Min[yi];
            adjCurY1Max[yi] = curY1Max[yi];
          }
          if ( ( y1AxisSource[yi] == XYGC_K_AUTOSCALE ) && kpY1MinEfDouble[yi].isNull() ) {
            curY1Min[yi] = adjCurY1Min[yi];
          }
          if ( ( y1AxisSource[yi] == XYGC_K_AUTOSCALE ) && kpY1MaxEfDouble[yi].isNull() ) {
            curY1Max[yi] = adjCurY1Max[yi];
          }
        }

      }

    }

    updateDimensions();

    for ( i=0; i<numTraces; i++ ) {

      yi = 0;
      if ( y2Scale[i] ) yi = 1;

      xFactor[i] =
       (double) ( plotAreaW ) / ( curXMax - curXMin );
      xOffset[i] = plotAreaX;

      y1Factor[yi][i] =
       (double) ( plotAreaH ) / ( curY1Max[yi] - curY1Min[yi] );
      y1Offset[yi][i] = plotAreaY;

    }

    kpXMinEfDouble.setValue( curXMin );
    kpXMaxEfDouble.setValue( curXMax );

    for ( yi=0; yi<xyGraphClass::NUM_Y_AXES; yi++ ) {
      kpY1MinEfDouble[yi].setValue( curY1Min[yi] );
      kpY1MaxEfDouble[yi].setValue( curY1Max[yi] );
    }

    regenBuffer();
    fullRefresh();

  }

  if ( nbs ) {

    drawGridFlag = 1;

    for ( i=0; i<numTraces; i++ ) {

      if ( !special[i] ) {

        yi = 0;
        if ( y2Scale[i] ) yi = 1;

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

              // There are two views of pv types, Type and specificType; this
              // uses specificType
              switch ( yPvType[i] ) {
              case ProcessVariable::specificType::flt:
                dyValue = (double) ( (float *) yPvData[i] )[ii];
                break;
              case ProcessVariable::specificType::real: 
                dyValue = ( (double *) yPvData[i] )[ii];
                break;
              case ProcessVariable::specificType::shrt:
                if ( ySigned[i] ) {
                  dyValue = (double) ( (short *) yPvData[i] )[ii];
                }
                else {
                  dyValue = (double) ( (unsigned short *) yPvData[i] )[ii];
                }
                break;
              case ProcessVariable::specificType::chr:
                if ( ySigned[i] ) {
                  dyValue = (double) ( (char *) yPvData[i] )[ii];
                }
                else {
                  dyValue = (double) ( (unsigned char *) yPvData[i] )[ii];
                }
                break;
              case ProcessVariable::specificType::integer:
                if ( ySigned[i] ) {
                  dyValue = (double) ( (int *) yPvData[i] )[ii];
                }
                else {
                  dyValue = (double) ( (unsigned int *) yPvData[i] )[ii];
                }
                break;
              case ProcessVariable::specificType::enumerated:
                if ( ySigned[i] ) {
                  dyValue = (double) ( (short *) yPvData[i] )[ii];
                }
                else {
                  dyValue = (double) ( (unsigned short *) yPvData[i] )[ii];
                }
                break;
              default:
                dyValue = ( (double *) yPvData[i] )[ii];
                break;
              }

              if ( y1AxisStyle[yi] == XYGC_K_AXIS_STYLE_LOG10 ) {
                dyValue = loc_log10(  dyValue  );
              }

              scaledY = plotAreaH -
               rint( ( dyValue - curY1Min[yi] ) *
               y1Factor[yi][i] - y1Offset[yi][i] );
              scaledY = dclamp( scaledY );

              if ( y1AxisStyle[yi] == XYGC_K_AXIS_STYLE_LOG10 ) {
                yZero = plotAreaH + y1Offset[yi][i];
              }
              else {
                yZero = plotAreaH -
                 rint( ( 0.0 - curY1Min[yi] ) *
                 y1Factor[yi][i] - y1Offset[yi][i] );
                yZero = dclamp( yZero );
              }

              if ( traceType[i] == XYGC_K_TRACE_CHRONOLOGICAL ) {

                dxValue = ( (double *) xPvData[i] )[ii];

              }
              else {
  
                // There are two views of pv types, Type and specificType; this
                // uses specificType
                switch ( xPvType[i] ) {
                case ProcessVariable::specificType::flt:
                  dxValue = (double) ( (float *) xPvData[i] )[ii];
                  break;
                case ProcessVariable::specificType::real: 
                  dxValue = ( (double *) xPvData[i] )[ii];
                  break;
                case ProcessVariable::specificType::shrt:
                  if ( xSigned[i] ) {
                    dxValue = (double) ( (short *) xPvData[i] )[ii];
                  }
                  else {
                    dxValue = (double) ( (unsigned short *) xPvData[i] )[ii];
                  }
                  break;
                case ProcessVariable::specificType::chr:
                  if ( xSigned[i] ) {
                    dxValue = (double) ( (char *) xPvData[i] )[ii];
                  }
                  else {
                    dxValue = (double) ( (unsigned char *) xPvData[i] )[ii];
                  }
                  break;
                case ProcessVariable::specificType::integer:
                  if ( xSigned[i] ) {
                    dxValue = (double) ( (int *) xPvData[i] )[ii];
                  }
                  else {
                    dxValue = (double) ( (unsigned int *) xPvData[i] )[ii];
                  }
                  break;
                case ProcessVariable::specificType::enumerated:
                  if ( xSigned[i] ) {
                    dxValue = (double) ( (short *) xPvData[i] )[ii];
                  }
                  else {
                    dxValue = (double) ( (unsigned short *) xPvData[i] )[ii];
                  }
                  break;
                default:
                  dxValue = ( (double *) xPvData[i] )[ii];
                  break;
                }

              }

              if ( xAxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) {
                dxValue = loc_log10(  dxValue  );
              }
              else if ( xAxisStyle == XYGC_K_AXIS_STYLE_TIME_LOG10 ) {
                dxValue = loc_log10(  dxValue  );
              }

              scaledX = rint( ( dxValue - curXMin ) *
               xFactor[i] + xOffset[i] );
              scaledX = dclamp( scaledX );

              addPoint( dxValue, scaledX, scaledY, yZero, i );

              ii++;
              if ( ii > plotBufSize[i] ) {
                ii = 0;
              }

            } while ( ii != arrayTail[i] );

          }

        }

      }

    }

    nu = 1;

  }

  if ( nxrescl ) {

    oldXMin = curXMin;

    if ( xRescaleValue < curXMin ) {
      range = curXMax - xRescaleValue;
      curXMin = xRescaleValue - 0.1 * range;
    }
    else if ( xRescaleValue > curXMax ) {

      if ( ( xAxisStyle == XYGC_K_AXIS_STYLE_TIME ) ||
           ( xAxisStyle == XYGC_K_AXIS_STYLE_TIME_LOG10 ) ) {

        getXMinMax( &curXMin, &oneMax );

        range = xRescaleValue - curXMin;
        curXMax = xRescaleValue;
        //curXMax = xRescaleValue + 0.33 * range;

      }
      else {

        range = xRescaleValue - curXMin;
        curXMax = xRescaleValue;
        //curXMax = xRescaleValue + 0.1 * range;

      }

    }

    if ( !kpXMinEfDouble.isNull() ) {
      curXMin = kpXMinEfDouble.value();
      if ( ( xAxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) ||
           ( xAxisStyle == XYGC_K_AXIS_STYLE_TIME_LOG10 ) ) {
        if ( curXMin <= 0 ) curXMin = 1e-2;
        curXMin = loc_log10( curXMin );
      }
    }

    if ( !kpXMaxEfDouble.isNull() ) {
      //fprintf( stderr, "user xmax = %-g\n", kpXMaxEfDouble.value() );
      curXMax = kpXMaxEfDouble.value();
      if ( ( xAxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) ||
           ( xAxisStyle == XYGC_K_AXIS_STYLE_TIME_LOG10 ) ) {
        if ( curXMax <= 0 ) curXMax = 1e-1;
        curXMax = loc_log10( curXMax );
      }
    }

    if ( kpCancelMinX ) {
      kpCancelMinX = 0;
      if ( xAxisSource == XYGC_K_FROM_PV ) {
        curXMin = dbXMin[0];
        if ( xAxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) {
          if ( curXMin <= 0 ) curXMin = 1e-2;
          curXMin = loc_log10(  curXMin  );
        }
        else if ( xAxisStyle == XYGC_K_AXIS_STYLE_TIME_LOG10 ) {
          if ( curXMin <= 0 ) curXMin = 1e-2;
          curXMin = loc_log10(  curXMin  );
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
          if ( curXMax <= 0 ) curXMax = 1e-1;
          curXMax = loc_log10(  curXMax  );
        }
        else if ( xAxisStyle == XYGC_K_AXIS_STYLE_TIME_LOG10 ) {
          if ( curXMax <= 0 ) curXMax = 1e-1;
          curXMax = loc_log10(  curXMax  );
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
      get_log10_scale_params1( curXMin, curXMax, &adjCurXMin, &adjCurXMax,
       &curXNumLabelTicks, &curXMajorsPerLabel, &curXMinorsPerMajor,
       format );
      if ( ( xAxisSource == XYGC_K_AUTOSCALE ) && kpXMinEfDouble.isNull() ) {
        curXMin = adjCurXMin;
      }
      if ( ( xAxisSource == XYGC_K_AUTOSCALE ) && kpXMaxEfDouble.isNull() ) {
        curXMax = adjCurXMax;
      }
    }
    else if ( xAxisStyle == XYGC_K_AXIS_STYLE_TIME_LOG10 ) {
      get_log10_scale_params1( curXMin, curXMax, &adjCurXMin, &adjCurXMax,
       &curXNumLabelTicks, &curXMajorsPerLabel, &curXMinorsPerMajor,
       format );
      if ( ( xAxisSource == XYGC_K_AUTOSCALE ) && kpXMinEfDouble.isNull() ) {
        curXMin = adjCurXMin;
      }
      if ( ( xAxisSource == XYGC_K_AUTOSCALE ) && kpXMaxEfDouble.isNull() ) {
        curXMax = adjCurXMax;
      }
    }
    else {
      get_scale_params1( curXMin, curXMax,
       &adjCurXMin, &adjCurXMax,
       &curXNumLabelTicks, &curXMajorsPerLabel, &curXMinorsPerMajor,
       format );
      if ( xAxisSmoothing == XYGC_K_NO_SMOOTHING ) {
        adjCurXMin = curXMin;
        adjCurXMax = curXMax;
      }
      if ( ( xAxisSource == XYGC_K_AUTOSCALE ) && kpXMinEfDouble.isNull() ) {
        curXMin = adjCurXMin;
      }
      if ( ( xAxisSource == XYGC_K_AUTOSCALE ) && kpXMaxEfDouble.isNull() ) {
        curXMax = adjCurXMax;
      }
    }

    updateDimensions();

    for ( i=0; i<numTraces; i++ ) {
      xFactor[i] =
       (double) ( plotAreaW ) / ( curXMax - curXMin );
      xOffset[i] = plotAreaX;
    }

    doRescale = 1;

  }

  for ( yi=0; yi<xyGraphClass::NUM_Y_AXES; yi++ ) {

    if ( ny1rescl[yi] ) {

      if ( y1RescaleValue[yi] < curY1Min[yi] ) {
        range = curY1Max[yi] - y1RescaleValue[yi];
        curY1Min[yi] = y1RescaleValue[yi] - 0.1 * range;
      }
      else if ( y1RescaleValue[yi] > curY1Max[yi] ) {
        range = y1RescaleValue[yi] - curY1Min[yi];
        curY1Max[yi] = y1RescaleValue[yi] + 0.1 * range;
      }

      if ( !kpY1MinEfDouble[yi].isNull() ) {
        curY1Min[yi] = kpY1MinEfDouble[yi].value();
        if ( y1AxisStyle[yi] == XYGC_K_AXIS_STYLE_LOG10 ) {
          if ( curY1Min[yi] <= 0 ) curY1Min[yi] = 1e-2;
          curY1Min[yi] = loc_log10( curY1Min[yi] );
        }
      }

      if ( !kpY1MaxEfDouble[yi].isNull() ) {
        curY1Max[yi] = kpY1MaxEfDouble[yi].value();
        if ( y1AxisStyle[yi] == XYGC_K_AXIS_STYLE_LOG10 ) {
          if ( curY1Max[yi] <= 0 ) curY1Max[yi] = 1e-1;
          curY1Max[yi] = loc_log10( curY1Max[yi] );
        }
      }

      if ( kpCancelMinY1[yi] ) {
        kpCancelMinY1[yi] = 0;
        if ( y1AxisSource[yi] == XYGC_K_FROM_PV ) {
          curY1Min[yi] = dbYMin[lowestYScaleIndex[yi]];
          if ( y1AxisStyle[yi] == XYGC_K_AXIS_STYLE_LOG10 ) {
            if ( curY1Min[yi] <= 0 ) curY1Min[yi] = 1e-2;
            curY1Min[yi] = loc_log10(  curY1Min[yi]  );
          }
        }
        else {
          curY1Min[yi] = y1RescaleValue[yi];
        }
      }

      if ( kpCancelMaxY1[yi] ) {
        kpCancelMaxY1[yi] = 0;
        if ( y1AxisSource[yi] == XYGC_K_FROM_PV ) {
          curY1Max[yi] = dbYMax[lowestYScaleIndex[yi]];
          if ( y1AxisStyle[yi] == XYGC_K_AXIS_STYLE_LOG10 ) {
            if ( curY1Max[yi] <= 0 ) curY1Max[yi] = 1e-1;
            curY1Max[yi] = loc_log10(  curY1Max[yi]  );
          }
        }
        else {
          curY1Max[yi] = y1RescaleValue[yi];
        }
      }

      if ( curY1Min[yi] >= curY1Max[yi] ) {
        if ( y1AxisStyle[yi] == XYGC_K_AXIS_STYLE_LOG10 ) {
          curY1Max[yi] = curY1Min[yi] * 10.0;
        }
        else {
          curY1Max[yi] = curY1Min[yi] * 2.0;
        }
      }
      if ( curY1Min[yi] >= curY1Max[yi] ) { // in case y Min is 0
        curY1Max[yi] = curY1Min[yi] + 1.0;
      }

      if ( y1AxisStyle[yi] == XYGC_K_AXIS_STYLE_LOG10 ) {
        get_log10_scale_params1( curY1Min[yi], curY1Max[yi], &adjCurY1Min[yi], &adjCurY1Max[yi],
         &curY1NumLabelTicks[yi], &curY1MajorsPerLabel[yi],
         &curY1MinorsPerMajor[yi], format );
        if ( ( y1AxisSource[yi] == XYGC_K_AUTOSCALE ) && kpY1MinEfDouble[yi].isNull() ) {
          curY1Min[yi] = adjCurY1Min[yi];
        }
        if ( ( y1AxisSource[yi] == XYGC_K_AUTOSCALE ) && kpY1MaxEfDouble[yi].isNull() ) {
          curY1Max[yi] = adjCurY1Max[yi];
        }
      }
      else {
        get_scale_params1( curY1Min[yi], curY1Max[yi], &adjCurY1Min[yi],
         &adjCurY1Max[yi], &curY1NumLabelTicks[yi], &curY1MajorsPerLabel[yi],
         &curY1MinorsPerMajor[yi], format );
        if ( y1AxisSmoothing[yi] == XYGC_K_NO_SMOOTHING ) {
          adjCurY1Min[yi] = curY1Min[yi];
          adjCurY1Max[yi] = curY1Max[yi];
        }
        if ( ( y1AxisSource[yi] == XYGC_K_AUTOSCALE ) && kpY1MinEfDouble[yi].isNull() ) {
          curY1Min[yi] = adjCurY1Min[yi];
        }
        if ( ( y1AxisSource[yi] == XYGC_K_AUTOSCALE ) && kpY1MaxEfDouble[yi].isNull() ) {
          curY1Max[yi] = adjCurY1Max[yi];
        }
      }

      updateDimensions();

      for ( i=0; i<numTraces; i++ ) {

        yScaleIndex = 0;
        if ( y2Scale[i] ) yScaleIndex = 1;

        if ( yScaleIndex == yi ) {

          y1Factor[yi][i] =
           (double) ( plotAreaH ) / ( curY1Max[yi] - curY1Min[yi] );
          y1Offset[yi][i] = plotAreaY;

        }

      }

      doRescale = 1;

    }

  }

  if ( nol ) {

    getXMinMax( &xmin, &xmax );
    getYMinMax( 0, ymin, ymax );
    getYMinMax( 1, ymin, ymax );

    for ( num=0; num<2; num++ ) {

      curXNumLabelTicks = xNumLabelIntervals.value();
      if ( curXNumLabelTicks < 1 ) curXNumLabelTicks = 1;
      curXMajorsPerLabel = xNumMajorPerLabel.value();
      curXMinorsPerMajor = xNumMinorPerMajor.value();

      if ( xAxisSource == XYGC_K_FROM_PV ) {
        allChronological = getDbXMinXMax( &curXMin, &curXMax );
      }
      else if ( xAxisSource == XYGC_K_USER_SPECIFIED ) {
        curXMin = xMin.value();
        curXMax = xMax.value();
        get_scale_params1( curXMin, curXMax,
         &adjCurXMin, &adjCurXMax,
         &curXNumLabelTicks, &curXMajorsPerLabel, &curXMinorsPerMajor,
         format );
        if ( xAxisSmoothing == XYGC_K_NO_SMOOTHING ) {
          adjCurXMin = curXMin;
          adjCurXMax = curXMax;
        }
      }
      else {
        curXMin = xMin.value();
        curXMax = xMax.value();
        if ( xmin < curXMin ) curXMin = xmin;
        if ( xmax > curXMax ) curXMax = xmax;
        if ( xAxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) {
          get_log10_scale_params1( curXMin, curXMax, &adjCurXMin, &adjCurXMax,
           &curXNumLabelTicks, &curXMajorsPerLabel, &curXMinorsPerMajor,
           format );
          if ( ( xAxisSource == XYGC_K_AUTOSCALE ) && kpXMinEfDouble.isNull() ) {
            curXMin = adjCurXMin;
          }
          if ( ( xAxisSource == XYGC_K_AUTOSCALE ) && kpXMaxEfDouble.isNull() ) {
            curXMax = adjCurXMax;
          }
	}
	else {
          get_scale_params1( curXMin, curXMax,
           &adjCurXMin, &adjCurXMax,
           &curXNumLabelTicks, &curXMajorsPerLabel, &curXMinorsPerMajor,
           format );
          if ( xAxisSmoothing == XYGC_K_NO_SMOOTHING ) {
            adjCurXMin = curXMin;
            adjCurXMax = curXMax;
          }
          if ( ( xAxisSource == XYGC_K_AUTOSCALE ) && kpXMinEfDouble.isNull() ) {
            curXMin = adjCurXMin;
          }
          if ( ( xAxisSource == XYGC_K_AUTOSCALE ) && kpXMaxEfDouble.isNull() ) {
            curXMax = adjCurXMax;
          }
	}
      }
      if ( xAxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) {
        if ( curXMin <= 0 ) curXMin = 1e-2;
        if ( curXMax <= 0 ) curXMax = 1e-1;
        curXMin = loc_log10(  curXMin  );
        curXMax = loc_log10(  curXMax  );
      }
      else if ( xAxisStyle == XYGC_K_AXIS_STYLE_TIME_LOG10 ) {
        if ( curXMin <= 0 ) curXMin = 1e-2;
        if ( curXMax <= 0 ) curXMax = 1e-1;
        curXMin = loc_log10(  curXMin  );
        curXMax = loc_log10(  curXMax  );
      }

      if ( allChronological ) { // then autoscale X
        if ( xmin < curXMin ) curXMin = xmin;
        if ( xmax > curXMax ) curXMax = xmax;
        if ( xAxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) {
          get_log10_scale_params1( curXMin, curXMax, &adjCurXMin, &adjCurXMax,
           &curXNumLabelTicks, &curXMajorsPerLabel, &curXMinorsPerMajor,
           format );
          if ( ( xAxisSource == XYGC_K_AUTOSCALE ) && kpXMinEfDouble.isNull() ) {
            curXMin = adjCurXMin;
          }
          if ( ( xAxisSource == XYGC_K_AUTOSCALE ) && kpXMaxEfDouble.isNull() ) {
            curXMax = adjCurXMax;
          }
	}
	else {
          get_scale_params1( curXMin, curXMax,
           &adjCurXMin, &adjCurXMax,
           &curXNumLabelTicks, &curXMajorsPerLabel, &curXMinorsPerMajor,
           format );
          if ( xAxisSmoothing == XYGC_K_NO_SMOOTHING ) {
            adjCurXMin = curXMin;
            adjCurXMax = curXMax;
          }
          if ( ( xAxisSource == XYGC_K_AUTOSCALE ) && kpXMinEfDouble.isNull() ) {
            curXMin = adjCurXMin;
          }
          if ( ( xAxisSource == XYGC_K_AUTOSCALE ) && kpXMaxEfDouble.isNull() ) {
            curXMax = adjCurXMax;
          }
	}
      }
      // from pv limits but not all chronological
      else {
        if ( xAxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) {
          get_log10_scale_params1( curXMin, curXMax, &adjCurXMin, &adjCurXMax,
           &curXNumLabelTicks, &curXMajorsPerLabel, &curXMinorsPerMajor,
           format );
        }
        else if ( xAxisStyle == XYGC_K_AXIS_STYLE_TIME_LOG10 ) {
          get_log10_scale_params1( curXMin, curXMax, &adjCurXMin, &adjCurXMax,
           &curXNumLabelTicks, &curXMajorsPerLabel, &curXMinorsPerMajor,
           format );
        }
        else {
          get_scale_params1( curXMin, curXMax,
           &adjCurXMin, &adjCurXMax,
           &curXNumLabelTicks, &curXMajorsPerLabel, &curXMinorsPerMajor,
           format );
          if ( xAxisSmoothing == XYGC_K_NO_SMOOTHING ) {
            adjCurXMin = curXMin;
            adjCurXMax = curXMax;
          }
        }
      }

      for ( yi=0; yi<xyGraphClass::NUM_Y_AXES; yi++ ) {
        curY1NumLabelTicks[yi] = y1NumLabelIntervals[yi].value();
        if ( curY1NumLabelTicks[yi] < 1 ) curY1NumLabelTicks[yi] = 1;
        curY1MajorsPerLabel[yi] = y1NumMajorPerLabel[yi].value();
        curY1MinorsPerMajor[yi] = y1NumMinorPerMajor[yi].value();
      }

#if 0
      for ( i=0; i<numTraces; i++ ) {
        xFactor[i] =
         (double) ( plotAreaW ) / ( curXMax - curXMin );
        xOffset[i] = plotAreaX;
      }
#endif

      for ( yi=0; yi<xyGraphClass::NUM_Y_AXES; yi++ ) {

        if ( y1AxisSource[yi] == XYGC_K_FROM_PV ) {
          getDbYMinYMax( &curY1Min[yi], &curY1Max[yi], yi );
	}
	else if ( y1AxisSource[yi] == XYGC_K_USER_SPECIFIED ) {
          curY1Min[yi] = y1Min[yi].value();
          curY1Max[yi] = y1Max[yi].value();
          get_scale_params1( curY1Min[yi], curY1Max[yi], &adjCurY1Min[yi],
           &adjCurY1Max[yi], &curY1NumLabelTicks[yi], &curY1MajorsPerLabel[yi],
           &curY1MinorsPerMajor[yi], format );
          if ( y1AxisSmoothing[yi] == XYGC_K_NO_SMOOTHING ) {
            adjCurY1Min[yi] = curY1Min[yi];
            adjCurY1Max[yi] = curY1Max[yi];
          }
	}
	else {
          curY1Min[yi] = y1Min[yi].value();
          if ( ymin[yi] < curY1Min[yi] ) curY1Min[yi] = ymin[yi];
          curY1Max[yi] = y1Max[yi].value();
          if ( ymax[yi] > curY1Max[yi] ) curY1Max[yi] = ymax[yi];
	}
        if ( y1AxisStyle[yi] == XYGC_K_AXIS_STYLE_LOG10 ) {
          if ( curY1Min[yi] <= 0 ) curY1Min[yi] = 1e-2;
          if ( curY1Max[yi] <= 0 ) curY1Max[yi] = 1e-1;
          curY1Min[yi] = loc_log10(  curY1Min[yi]  );
          curY1Max[yi] = loc_log10(  curY1Max[yi]  );
	}
        if ( y1AxisStyle[yi] == XYGC_K_AXIS_STYLE_LOG10 ) {
          get_log10_scale_params1( curY1Min[yi], curY1Max[yi], &adjCurY1Min[yi], &adjCurY1Max[yi],
           &curY1NumLabelTicks[yi], &curY1MajorsPerLabel[yi],
           &curY1MinorsPerMajor[yi], format );
          if ( ( y1AxisSource[yi] == XYGC_K_AUTOSCALE ) && kpY1MinEfDouble[yi].isNull() ) {
            curY1Min[yi] = adjCurY1Min[yi];
          }
          if ( ( y1AxisSource[yi] == XYGC_K_AUTOSCALE ) && kpY1MaxEfDouble[yi].isNull() ) {
            curY1Max[yi] = adjCurY1Max[yi];
          }
        }
        else {
          get_scale_params1( curY1Min[yi], curY1Max[yi], &adjCurY1Min[yi],
           &adjCurY1Max[yi], &curY1NumLabelTicks[yi], &curY1MajorsPerLabel[yi],
           &curY1MinorsPerMajor[yi], format );
          if ( y1AxisSmoothing[yi] == XYGC_K_NO_SMOOTHING ) {
            adjCurY1Min[yi] = curY1Min[yi];
            adjCurY1Max[yi] = curY1Max[yi];
          }
          if ( ( y1AxisSource[yi] == XYGC_K_AUTOSCALE ) && kpY1MinEfDouble[yi].isNull() ) {
            curY1Min[yi] = adjCurY1Min[yi];
          }
          if ( ( y1AxisSource[yi] == XYGC_K_AUTOSCALE ) && kpY1MaxEfDouble[yi].isNull() ) {
            curY1Max[yi] = adjCurY1Max[yi];
          }
        }

#if 0
        for ( i=0; i<numTraces; i++ ) {
          y1Factor[yi][i] =
           (double) ( plotAreaH ) / ( curY1Max[yi] - curY1Min[yi] );
          y1Offset[yi][i] = plotAreaY;
        }
#endif

      }
    
      updateDimensions();

      for ( i=0; i<numTraces; i++ ) {
        xFactor[i] =
         (double) ( plotAreaW ) / ( curXMax - curXMin );
        xOffset[i] = plotAreaX;
      }

      for ( yi=0; yi<xyGraphClass::NUM_Y_AXES; yi++ ) {

        for ( i=0; i<numTraces; i++ ) {
          y1Factor[yi][i] =
           (double) ( plotAreaH ) / ( curY1Max[yi] - curY1Min[yi] );
          y1Offset[yi][i] = plotAreaY;
        }

      }

    }

    for ( yi=0; yi<xyGraphClass::NUM_Y_AXES; yi++ ) {
      if ( numYTraces[yi] > 0 ) {
        kpY1MinEfDouble[yi].setNull(1);
        kpY1MaxEfDouble[yi].setNull(1);
      }
    }
    kpXMinEfDouble.setNull(1);
    kpXMaxEfDouble.setNull(1);

    regenBuffer();
    fullRefresh();

  }

  if ( nasu && !doingBoxRescale ) {

    if ( xAxisSource == XYGC_K_AUTOSCALE ) {

      maxDiff = 0;
      autoScaleX = 0;

      getXMinMax( &checkXMin, &checkXMax );

      if ( checkXMin != checkXMax ) {

        if ( kpXMinEfDouble.isNull() || kpXMaxEfDouble.isNull() ) {
          if ( ( curXMax - curXMin ) != 0 ) {
            diff = ( fabs( curXMax - curXMin ) - fabs( checkXMax - checkXMin ) ) /
             fabs( curXMax - curXMin );
            if ( diff > maxDiff ) maxDiff = diff;
          }
        }

        if ( maxDiff > autoScaleThreshFrac ) {
          autoScaleX = 1;
        }

      }

    }

    for ( yi=0; yi<xyGraphClass::NUM_Y_AXES; yi++ ) {

      maxDiff = 0;
      autoScaleY[yi] = 0;

      if ( y1AxisSource[yi] == XYGC_K_AUTOSCALE ) {

        getYMinMax( yi, checkY1Min, checkY1Max );

        if ( y1AxisStyle[yi] == XYGC_K_AXIS_STYLE_LOG10 ) {
          if ( checkY1Min[yi] <= 0 ) checkY1Min[yi] = 1e-2;
          if ( checkY1Max[yi] <= 0 ) checkY1Max[yi] = 1e-1;
          checkY1Min[yi] = loc_log10(  checkY1Min[yi]  );
          checkY1Max[yi] = loc_log10(  checkY1Max[yi]  );
	}

        if ( checkY1Min[yi] != checkY1Max[yi] ) {

          if ( kpY1MinEfDouble[yi].isNull() || kpY1MaxEfDouble[yi].isNull() ) {
            if ( ( curY1Max[yi] - curY1Min[yi] ) != 0 ) {
              diff = ( fabs( curY1Max[yi] - curY1Min[yi] ) -
                       fabs( checkY1Max[yi] - checkY1Min[yi] ) ) /
               fabs( curY1Max[yi] - curY1Min[yi] );
              if ( diff > maxDiff ) maxDiff = diff;
            }
          }

          if ( maxDiff > autoScaleThreshFrac ) {
            autoScaleY[yi] = 1;
          }

	}

      }

    }

    anyRescale = 0;

    if ( autoScaleX ) {

      if ( xAxisSource == XYGC_K_AUTOSCALE ) {

        anyRescale = 1;

        getXMinMax( &checkXMin, &checkXMax );

        if ( kpXMinEfDouble.isNull() ) {
          curXMin = checkXMin - 0.02 * fabs( checkXMax - curXMin );
	}
        if ( kpXMaxEfDouble.isNull() ) {
	  curXMax = checkXMax + 0.02 * fabs( checkXMax - curXMin );
	}

        if ( xAxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) {
          if ( curXMin <= 0 ) curXMin = 1e-2;
          if ( curXMax <= 0 ) curXMax = 1e-1;
          curXMin = loc_log10(  curXMin  );
          curXMax = loc_log10(  curXMax  );
          get_log10_scale_params1( curXMin, curXMax, &adjCurXMin, &adjCurXMax,
           &curXNumLabelTicks, &curXMajorsPerLabel, &curXMinorsPerMajor,
           format );
          if ( ( xAxisSource == XYGC_K_AUTOSCALE ) && kpXMinEfDouble.isNull() ) {
            curXMin = adjCurXMin;
          }
          if ( ( xAxisSource == XYGC_K_AUTOSCALE ) && kpXMaxEfDouble.isNull() ) {
            curXMax = adjCurXMax;
          }
        }
        else if ( xAxisStyle == XYGC_K_AXIS_STYLE_TIME_LOG10 ) {
          if ( curXMin <= 0 ) curXMin = 1e-2;
          if ( curXMax <= 0 ) curXMax = 1e-1;
          curXMin = loc_log10(  curXMin  );
          curXMax = loc_log10(  curXMax  );
          get_log10_scale_params1( curXMin, curXMax, &adjCurXMin, &adjCurXMax,
           &curXNumLabelTicks, &curXMajorsPerLabel, &curXMinorsPerMajor,
           format );
          if ( ( xAxisSource == XYGC_K_AUTOSCALE ) && kpXMinEfDouble.isNull() ) {
            curXMin = adjCurXMin;
          }
          if ( ( xAxisSource == XYGC_K_AUTOSCALE ) && kpXMaxEfDouble.isNull() ) {
            curXMax = adjCurXMax;
          }
        }
        else {
          get_scale_params1( curXMin, curXMax,
           &adjCurXMin, &adjCurXMax,
           &curXNumLabelTicks, &curXMajorsPerLabel, &curXMinorsPerMajor,
           format );
          if ( xAxisSmoothing == XYGC_K_NO_SMOOTHING ) {
            adjCurXMin = curXMin;
            adjCurXMax = curXMax;
          }
          if ( ( xAxisSource == XYGC_K_AUTOSCALE ) && kpXMinEfDouble.isNull() ) {
            curXMin = adjCurXMin;
          }
          if ( ( xAxisSource == XYGC_K_AUTOSCALE ) && kpXMaxEfDouble.isNull() ) {
            curXMax = adjCurXMax;
          }
        }

        for ( i=0; i<numTraces; i++ ) {
          xFactor[i] =
           (double) ( plotAreaW ) / ( curXMax - curXMin );
          xOffset[i] = plotAreaX;
        }

      }

    }

    for ( yi=0; yi<xyGraphClass::NUM_Y_AXES; yi++ ) {

      if ( autoScaleY[yi] ) {

        if ( y1AxisSource[yi] == XYGC_K_AUTOSCALE ) {

          getYMinMax( yi, checkY1Min, checkY1Max );

	  if ( kpY1MinEfDouble[yi].isNull() ) {
	    curY1Min[yi] = checkY1Min[yi] - 0.02 * fabs( checkY1Max[yi] - curY1Min[yi] );
	  }

	  if ( kpY1MaxEfDouble[yi].isNull() ) {
	    curY1Max[yi] = checkY1Max[yi] + 0.02 * fabs( checkY1Max[yi] - curY1Min[yi] );
	  }

          anyRescale = 1;

          if ( y1AxisStyle[yi] == XYGC_K_AXIS_STYLE_LOG10 ) {
            if ( curY1Min[yi] <= 0 ) curY1Min[yi] = 1e-2;
            if ( curY1Max[yi] <= 0 ) curY1Max[yi] = 1e-1;
            curY1Min[yi] = loc_log10(  curY1Min[yi]  );
            curY1Max[yi] = loc_log10(  curY1Max[yi]  );
            get_log10_scale_params1( curY1Min[yi], curY1Max[yi], &adjCurY1Min[yi], &adjCurY1Max[yi],
             &curY1NumLabelTicks[yi], &curY1MajorsPerLabel[yi],
             &curY1MinorsPerMajor[yi], format );
            if ( ( y1AxisSource[yi] == XYGC_K_AUTOSCALE ) && kpY1MinEfDouble[yi].isNull() ) {
              curY1Min[yi] = adjCurY1Min[yi];
            }
            if ( ( y1AxisSource[yi] == XYGC_K_AUTOSCALE ) && kpY1MaxEfDouble[yi].isNull() ) {
              curY1Max[yi] = adjCurY1Max[yi];
            }
          }
          else {
            get_scale_params1( curY1Min[yi], curY1Max[yi], &adjCurY1Min[yi],
             &adjCurY1Max[yi], &curY1NumLabelTicks[yi], &curY1MajorsPerLabel[yi],
             &curY1MinorsPerMajor[yi], format );
            if ( y1AxisSmoothing[yi] == XYGC_K_NO_SMOOTHING ) {
              adjCurY1Min[yi] = curY1Min[yi];
              adjCurY1Max[yi] = curY1Max[yi];
            }
            if ( ( y1AxisSource[yi] == XYGC_K_AUTOSCALE ) && kpY1MinEfDouble[yi].isNull() ) {
              curY1Min[yi] = adjCurY1Min[yi];
            }
            if ( ( y1AxisSource[yi] == XYGC_K_AUTOSCALE ) && kpY1MaxEfDouble[yi].isNull() ) {
              curY1Max[yi] = adjCurY1Max[yi];
            }
          }

          for ( i=0; i<numTraces; i++ ) {

            yScaleIndex = 0;
            if ( y2Scale[i] ) yScaleIndex = 1;

            y1Factor[yScaleIndex][i] =
             (double) ( plotAreaH ) / ( curY1Max[yScaleIndex] - curY1Min[yScaleIndex] );
            y1Offset[yScaleIndex][i] = plotAreaY;

          }

        }

      }

    }

    if ( anyRescale ) {
      updateDimensions();
      doRescale = 1;
    }

  }

  if ( nnl ) {

    anyRescale = 0;

    if ( xAxisSource == XYGC_K_AUTOSCALE ) {

      anyRescale = 1;

      getXMinMax( &checkXMin, &checkXMax );

      if ( kpXMinEfDouble.isNull() ) {
        curXMin = checkXMin - 0.02 * fabs( checkXMax - curXMin );
      }
      if ( kpXMaxEfDouble.isNull() ) {
        curXMax = checkXMax + 0.02 * fabs( checkXMax - curXMin );
      }

      if ( xAxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) {
        if ( curXMin <= 0 ) curXMin = 1e-2;
        if ( curXMax <= 0 ) curXMax = 1e-1;
        curXMin = loc_log10(  curXMin  );
        curXMax = loc_log10(  curXMax  );
        get_log10_scale_params1( curXMin, curXMax, &adjCurXMin, &adjCurXMax,
         &curXNumLabelTicks, &curXMajorsPerLabel, &curXMinorsPerMajor,
         format );
        if ( ( xAxisSource == XYGC_K_AUTOSCALE ) && kpXMinEfDouble.isNull() ) {
          curXMin = adjCurXMin;
        }
        if ( ( xAxisSource == XYGC_K_AUTOSCALE ) && kpXMaxEfDouble.isNull() ) {
          curXMax = adjCurXMax;
        }
      }
      else if ( xAxisStyle == XYGC_K_AXIS_STYLE_TIME_LOG10 ) {
        if ( curXMin <= 0 ) curXMin = 1e-2;
        if ( curXMax <= 0 ) curXMax = 1e-1;
        curXMin = loc_log10(  curXMin  );
        curXMax = loc_log10(  curXMax  );
        get_log10_scale_params1( curXMin, curXMax, &adjCurXMin, &adjCurXMax,
         &curXNumLabelTicks, &curXMajorsPerLabel, &curXMinorsPerMajor,
         format );
        if ( ( xAxisSource == XYGC_K_AUTOSCALE ) && kpXMinEfDouble.isNull() ) {
          curXMin = adjCurXMin;
        }
        if ( ( xAxisSource == XYGC_K_AUTOSCALE ) && kpXMaxEfDouble.isNull() ) {
          curXMax = adjCurXMax;
        }
      }
      else {
        get_scale_params1( curXMin, curXMax,
         &adjCurXMin, &adjCurXMax,
         &curXNumLabelTicks, &curXMajorsPerLabel, &curXMinorsPerMajor,
         format );
        if ( xAxisSmoothing == XYGC_K_NO_SMOOTHING ) {
          adjCurXMin = curXMin;
          adjCurXMax = curXMax;
        }
        if ( ( xAxisSource == XYGC_K_AUTOSCALE ) && kpXMinEfDouble.isNull() ) {
          curXMin = adjCurXMin;
        }
        if ( ( xAxisSource == XYGC_K_AUTOSCALE ) && kpXMaxEfDouble.isNull() ) {
          curXMax = adjCurXMax;
        }
      }

      for ( i=0; i<numTraces; i++ ) {
        xFactor[i] =
         (double) ( plotAreaW ) / ( curXMax - curXMin );
        xOffset[i] = plotAreaX;
      }

    }

    for ( yi=0; yi<xyGraphClass::NUM_Y_AXES; yi++ ) {

      if ( y1AxisSource[yi] == XYGC_K_AUTOSCALE ) {

        getYMinMax( yi, checkY1Min, checkY1Max );

        if ( ( y1AxisStyle[yi] != XYGC_K_AXIS_STYLE_LOG10 ) &&
             ( y1AxisStyle[yi] != XYGC_K_AXIS_STYLE_TIME_LOG10 ) ) {

          if ( kpY1MinEfDouble[yi].isNull() ) {
            curY1Min[yi] = checkY1Min[yi] - 0.02 * fabs( checkY1Max[yi] - checkY1Min[yi] );
          }
          if ( kpY1MaxEfDouble[yi].isNull() ) {
            curY1Max[yi] = checkY1Max[yi] + 0.02 * fabs( checkY1Max[yi] - checkY1Min[yi] );
          }

	}
	else {

          if ( kpY1MinEfDouble[yi].isNull() ) {
            curY1Min[yi] = checkY1Min[yi];
          }
          if ( kpY1MaxEfDouble[yi].isNull() ) {
            curY1Max[yi] = checkY1Max[yi] + 0.02 * fabs( checkY1Max[yi] - checkY1Min[yi] );
          }

	}

        anyRescale = 1;

        if ( y1AxisStyle[yi] == XYGC_K_AXIS_STYLE_LOG10 ) {
          if ( curY1Min[yi] <= 0 ) curY1Min[yi] = 1e-2;
          if ( curY1Max[yi] <= 0 ) curY1Max[yi] = 1e-1;
          curY1Min[yi] = loc_log10(  curY1Min[yi]  );
          curY1Max[yi] = loc_log10(  curY1Max[yi]  );
          get_log10_scale_params1( curY1Min[yi], curY1Max[yi], &adjCurY1Min[yi], &adjCurY1Max[yi],
           &curY1NumLabelTicks[yi], &curY1MajorsPerLabel[yi],
           &curY1MinorsPerMajor[yi], format );
          if ( ( y1AxisSource[yi] == XYGC_K_AUTOSCALE ) && kpY1MinEfDouble[yi].isNull() ) {
            curY1Min[yi] = adjCurY1Min[yi];
          }
          if ( ( y1AxisSource[yi] == XYGC_K_AUTOSCALE ) && kpY1MaxEfDouble[yi].isNull() ) {
            curY1Max[yi] = adjCurY1Max[yi];
          }
        }
        else {
          get_scale_params1( curY1Min[yi], curY1Max[yi], &adjCurY1Min[yi],
           &adjCurY1Max[yi], &curY1NumLabelTicks[yi], &curY1MajorsPerLabel[yi],
           &curY1MinorsPerMajor[yi], format );
          if ( y1AxisSmoothing[yi] == XYGC_K_NO_SMOOTHING ) {
            adjCurY1Min[yi] = curY1Min[yi];
            adjCurY1Max[yi] = curY1Max[yi];
          }
          if ( ( y1AxisSource[yi] == XYGC_K_AUTOSCALE ) && kpY1MinEfDouble[yi].isNull() ) {
            curY1Min[yi] = adjCurY1Min[yi];
          }
          if ( ( y1AxisSource[yi] == XYGC_K_AUTOSCALE ) && kpY1MaxEfDouble[yi].isNull() ) {
            curY1Max[yi] = adjCurY1Max[yi];
          }
        }

        for ( i=0; i<numTraces; i++ ) {
          y1Factor[yi][i] =
           (double) ( plotAreaH ) / ( curY1Max[yi] - curY1Min[yi] );
          y1Offset[yi][i] = plotAreaY;
        }

      }

    }

    if ( anyRescale ) {
      updateDimensions();
      doRescale = 1;
    }

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

    for ( i=0; i<numTraces; i++ ) {

      if ( yArrayGotValueCallback[i] == 1 ) {
        yArrayGotValueCallback[i] = 2;
        yArrayNeedUpdate[i] = 1;
      }

      if ( xArrayGotValueCallback[i] == 1 ) {
        xArrayGotValueCallback[i] = 2;
        xArrayNeedUpdate[i] = 1;
      }

    }

    eraseActive();
    drawActive();

  }

  if ( nr ) {
    regenBuffer();
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

    // --------------------------

    for ( num=0; num<2; num++ ) {

      curXNumLabelTicks = xNumLabelIntervals.value();
      if ( curXNumLabelTicks < 1 ) curXNumLabelTicks = 1;
      curXMajorsPerLabel = xNumMajorPerLabel.value();
      curXMinorsPerMajor = xNumMinorPerMajor.value();

      if ( xAxisSource == XYGC_K_FROM_PV ) {
        allChronological = getDbXMinXMax( &curXMin, &curXMax );
      }
      else if ( xAxisSource == XYGC_K_USER_SPECIFIED ) {
        curXMin = xMin.value();
        curXMax = xMax.value();
      }
      else {
        curXMin = xMin.value();
        curXMax = xMax.value();
        if ( xAxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) {
          get_log10_scale_params1( curXMin, curXMax, &adjCurXMin, &adjCurXMax,
           &curXNumLabelTicks, &curXMajorsPerLabel, &curXMinorsPerMajor,
           format );
          if ( ( xAxisSource == XYGC_K_AUTOSCALE ) && kpXMinEfDouble.isNull() ) {
            curXMin = adjCurXMin;
          }
          if ( ( xAxisSource == XYGC_K_AUTOSCALE ) && kpXMaxEfDouble.isNull() ) {
            curXMax = adjCurXMax;
          }
	}
	else {
          get_scale_params1( curXMin, curXMax,
           &adjCurXMin, &adjCurXMax,
           &curXNumLabelTicks, &curXMajorsPerLabel, &curXMinorsPerMajor,
           format );
          if ( xAxisSmoothing == XYGC_K_NO_SMOOTHING ) {
            adjCurXMin = curXMin;
            adjCurXMax = curXMax;
          }
          if ( ( xAxisSource == XYGC_K_AUTOSCALE ) && kpXMinEfDouble.isNull() ) {
            curXMin = adjCurXMin;
          }
          if ( ( xAxisSource == XYGC_K_AUTOSCALE ) && kpXMaxEfDouble.isNull() ) {
            curXMax = adjCurXMax;
          }
	}
      }
      if ( xAxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) {
        if ( curXMin <= 0 ) curXMin = 1e-2;
        if ( curXMax <= 0 ) curXMax = 1e-1;
        curXMin = loc_log10(  curXMin  );
        curXMax = loc_log10(  curXMax  );
      }
      else if ( xAxisStyle == XYGC_K_AXIS_STYLE_TIME_LOG10 ) {
        if ( curXMin <= 0 ) curXMin = 1e-2;
        if ( curXMax <= 0 ) curXMax = 1e-1;
        curXMin = loc_log10(  curXMin  );
        curXMax = loc_log10(  curXMax  );
      }

      if ( allChronological ) { // then autoscale X
        if ( xAxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) {
          get_log10_scale_params1( curXMin, curXMax, &adjCurXMin, &adjCurXMax,
           &curXNumLabelTicks, &curXMajorsPerLabel, &curXMinorsPerMajor,
           format );
          if ( ( xAxisSource == XYGC_K_AUTOSCALE ) && kpXMinEfDouble.isNull() ) {
            curXMin = adjCurXMin;
          }
          if ( ( xAxisSource == XYGC_K_AUTOSCALE ) && kpXMaxEfDouble.isNull() ) {
            curXMax = adjCurXMax;
          }
	}
	else {
          get_scale_params1( curXMin, curXMax,
           &adjCurXMin, &adjCurXMax,
           &curXNumLabelTicks, &curXMajorsPerLabel, &curXMinorsPerMajor,
           format );
          if ( xAxisSmoothing == XYGC_K_NO_SMOOTHING ) {
            adjCurXMin = curXMin;
            adjCurXMax = curXMax;
          }
          if ( ( xAxisSource == XYGC_K_AUTOSCALE ) && kpXMinEfDouble.isNull() ) {
            curXMin = adjCurXMin;
          }
          if ( ( xAxisSource == XYGC_K_AUTOSCALE ) && kpXMaxEfDouble.isNull() ) {
            curXMax = adjCurXMax;
          }
	}
      }
      // from pv limits but not all chronological
      else {
        if ( xAxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) {
          get_log10_scale_params1( curXMin, curXMax, &adjCurXMin, &adjCurXMax,
           &curXNumLabelTicks, &curXMajorsPerLabel, &curXMinorsPerMajor,
           format );
        }
        else if ( xAxisStyle == XYGC_K_AXIS_STYLE_TIME_LOG10 ) {
          get_log10_scale_params1( curXMin, curXMax, &adjCurXMin, &adjCurXMax,
           &curXNumLabelTicks, &curXMajorsPerLabel, &curXMinorsPerMajor,
           format );
        }
        else {
          get_scale_params1( curXMin, curXMax,
           &adjCurXMin, &adjCurXMax,
           &curXNumLabelTicks, &curXMajorsPerLabel, &curXMinorsPerMajor,
           format );
          if ( xAxisSmoothing == XYGC_K_NO_SMOOTHING ) {
            adjCurXMin = curXMin;
            adjCurXMax = curXMax;
          }
        }
      }

      for ( yi=0; yi<xyGraphClass::NUM_Y_AXES; yi++ ) {
        curY1NumLabelTicks[yi] = y1NumLabelIntervals[yi].value();
        if ( curY1NumLabelTicks[yi] < 1 ) curY1NumLabelTicks[yi] = 1;
        curY1MajorsPerLabel[yi] = y1NumMajorPerLabel[yi].value();
        curY1MinorsPerMajor[yi] = y1NumMinorPerMajor[yi].value();
      }

      for ( i=0; i<numTraces; i++ ) {
        xFactor[i] =
         (double) ( plotAreaW ) / ( curXMax - curXMin );
        xOffset[i] = plotAreaX;
      }

      for ( yi=0; yi<xyGraphClass::NUM_Y_AXES; yi++ ) {

        if ( y1AxisSource[yi] == XYGC_K_FROM_PV ) {
          getDbYMinYMax( &curY1Min[yi], &curY1Max[yi], yi );
	}
	else if ( y1AxisSource[yi] == XYGC_K_USER_SPECIFIED ) {
          curY1Min[yi] = y1Min[yi].value();
          curY1Max[yi] = y1Max[yi].value();
	}
	else {
          curY1Min[yi] = y1Min[yi].value();
          curY1Max[yi] = y1Max[yi].value();
	}
        if ( y1AxisStyle[yi] == XYGC_K_AXIS_STYLE_LOG10 ) {
          if ( curY1Min[yi] <= 0 ) curY1Min[yi] = 1e-2;
          if ( curY1Max[yi] <= 0 ) curY1Max[yi] = 1e-1;
          curY1Min[yi] = loc_log10(  curY1Min[yi]  );
          curY1Max[yi] = loc_log10(  curY1Max[yi]  );
	}
        if ( y1AxisStyle[yi] == XYGC_K_AXIS_STYLE_LOG10 ) {
          get_log10_scale_params1( curY1Min[yi], curY1Max[yi], &adjCurY1Min[yi], &adjCurY1Max[yi],
           &curY1NumLabelTicks[yi], &curY1MajorsPerLabel[yi],
           &curY1MinorsPerMajor[yi], format );
          if ( ( y1AxisSource[yi] == XYGC_K_AUTOSCALE ) && kpY1MinEfDouble[yi].isNull() ) {
            curY1Min[yi] = adjCurY1Min[yi];
          }
          if ( ( y1AxisSource[yi] == XYGC_K_AUTOSCALE ) && kpY1MaxEfDouble[yi].isNull() ) {
            curY1Max[yi] = adjCurY1Max[yi];
          }
        }
        else {
          get_scale_params1( curY1Min[yi], curY1Max[yi], &adjCurY1Min[yi],
           &adjCurY1Max[yi], &curY1NumLabelTicks[yi], &curY1MajorsPerLabel[yi],
           &curY1MinorsPerMajor[yi], format );
          if ( y1AxisSmoothing[yi] == XYGC_K_NO_SMOOTHING ) {
            adjCurY1Min[yi] = curY1Min[yi];
            adjCurY1Max[yi] = curY1Max[yi];
          }
          if ( ( y1AxisSource[yi] == XYGC_K_AUTOSCALE ) && kpY1MinEfDouble[yi].isNull() ) {
            curY1Min[yi] = adjCurY1Min[yi];
          }
          if ( ( y1AxisSource[yi] == XYGC_K_AUTOSCALE ) && kpY1MaxEfDouble[yi].isNull() ) {
            curY1Max[yi] = adjCurY1Max[yi];
          }
        }

        for ( i=0; i<numTraces; i++ ) {
          y1Factor[yi][i] =
           (double) ( plotAreaH ) / ( curY1Max[yi] - curY1Min[yi] );
          y1Offset[yi][i] = plotAreaY;
        }

      }
    
      updateDimensions();

    }

    // --------------------------

#if 0

    curXMin = xMin.value();
    curXMax = xMax.value();
    if ( xAxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) {
      if ( curXMin <= 0 ) curXMin = 1e-2;
      if ( curXMax <= 0 ) curXMax = 1e-1;
      curXMin = loc_log10(  curXMin  );
      curXMax = loc_log10(  curXMax  );
    }
    else if ( xAxisStyle == XYGC_K_AXIS_STYLE_TIME_LOG10 ) {
      if ( curXMin <= 0 ) curXMin = 1e-2;
      if ( curXMax <= 0 ) curXMax = 1e-1;
      curXMin = loc_log10(  curXMin  );
      curXMax = loc_log10(  curXMax  );
    }

    for ( i=0; i<numTraces; i++ ) {
      xFactor[i] =
       (double) ( plotAreaW ) / ( curXMax - curXMin );
      xOffset[i] = plotAreaX;
    }

    for ( yi=0; yi<xyGraphClass::NUM_Y_AXES; yi++ ) {

      curY1Min[yi] = y1Min[yi].value();
      curY1Max[yi] = y1Max[yi].value();
      if ( y1AxisStyle[yi] == XYGC_K_AXIS_STYLE_LOG10 ) {
        if ( curY1Min[yi] <= 0 ) curY1Min[yi] = 1e-2;
        if ( curY1Max[yi] <= 0 ) curY1Max[yi] = 1e-1;
        curY1Min[yi] = loc_log10(  curY1Min[yi]  );
        curY1Max[yi] = loc_log10(  curY1Max[yi]  );
      }

      for ( i=0; i<numTraces; i++ ) {
        y1Factor[yi][i] =
         (double) ( plotAreaH ) / ( curY1Max[yi] - curY1Min[yi] );
        y1Offset[yi][i] = plotAreaY;
      }

    }
    
    curXNumLabelTicks = xNumLabelIntervals.value();
    if ( curXNumLabelTicks < 1 ) curXNumLabelTicks = 1;
    curXMajorsPerLabel = xNumMajorPerLabel.value();
    curXMinorsPerMajor = xNumMinorPerMajor.value();

    for ( yi=0; yi<xyGraphClass::NUM_Y_AXES; yi++ ) {
      curY1NumLabelTicks[yi] = y1NumLabelIntervals[yi].value();
      if ( curY1NumLabelTicks[yi] < 1 ) curY1NumLabelTicks[yi] = 1;
      curY1MajorsPerLabel[yi] = y1NumMajorPerLabel[yi].value();
      curY1MinorsPerMajor[yi] = y1NumMinorPerMajor[yi].value();
    }

#endif

    // --------------------------

    if ( !firstBoxRescale ) {

      firstBoxRescale = 1;
      boxXMin = savedXMin;
      boxXMax = savedXMax;
      for ( yi=0; yi<xyGraphClass::NUM_Y_AXES; yi++ ) {
        if ( numYTraces[yi] > 0 ) {
          boxYMin[yi] = savedYMin[yi];
          boxYMax[yi] = savedYMax[yi];
        }
      }

    }

    for ( yi=0; yi<xyGraphClass::NUM_Y_AXES; yi++ ) {
      if ( numYTraces[yi] > 0 ) {
        kpY1MinEfDouble[yi].setNull(1);
        kpY1MaxEfDouble[yi].setNull(1);
      }
    }
    kpXMinEfDouble.setNull(1);
    kpXMaxEfDouble.setNull(1);

    regenBuffer();
    fullRefresh();

  }

  if ( nasc ) { // this needs to be at end

    for ( i=0; i<numTraces; i++ ) {

      if ( traceSize[i] < 0 ) traceSize[i] = 0;

      if ( traceSize[i] > yPvDim[i] ) {
        yPvCount[i] = yPvDim[i];
      }
      else if ( traceSize[i] > 0 ) {
        yPvCount[i] = traceSize[i];
      }

      if ( traceSize[i] > xPvDim[i] ) {
        xPvCount[i] = xPvDim[i];
      }
      else if ( traceSize[i] > 0 ) {
        xPvCount[i] = traceSize[i];
      }

    }

    actWin->appCtx->proc->lock();

    for ( i=0; i<numTraces; i++ ) {
      yArrayNeedUpdate[i] = 1;
      xArrayNeedUpdate[i] = 1;
    }

    needVectorUpdate = 1;

    actWin->addDefExeNode( aglPtr );
    actWin->appCtx->proc->unlock();

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
  double oneX,
  double dScaledX,
  double dScaledY,
  double scaledYZero,
  int trace
) {

int i;
short scaledX, scaledY;

  if ( !plotInfo[trace] ) return;

  if ( ( dScaledX > 32767 ) ||
       ( dScaledX < -32768 ) ||
       ( dScaledY > 32767 ) ||
       ( dScaledY < -32767 ) ) return;

  scaledX = (short) dScaledX;
  scaledY = (short) dScaledY;

  if ( opMode[trace] == XYGC_K_SCOPE_MODE ) {

    i = plotInfoTail[trace];

    plotInfo[trace][i].firstX = scaledX;
    plotInfo[trace][i].firstY = scaledY;
    plotInfo[trace][i].yZero = scaledYZero;

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

    if ( ( scaledX < plotAreaX ) || ( scaledX > plotInfoSize[trace] ) ) {
      return;
    }

    plotInfo[trace][scaledX].yZero = scaledYZero;

    if ( plotInfo[trace][scaledX].n == 0 ) {

      plotInfo[trace][scaledX].firstDX = oneX;
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

      if ( oneX < plotInfo[trace][scaledX].firstDX ) {

        plotInfo[trace][scaledX].lastDX = plotInfo[trace][scaledX].firstDX;
        plotInfo[trace][scaledX].lastX = plotInfo[trace][scaledX].firstX;
        plotInfo[trace][scaledX].lastY = plotInfo[trace][scaledX].firstY;

        plotInfo[trace][scaledX].firstDX = oneX;
        plotInfo[trace][scaledX].firstX = scaledX;
        plotInfo[trace][scaledX].firstY = scaledY;

      }
      else {

        plotInfo[trace][scaledX].lastDX = oneX;
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

      if ( oneX < plotInfo[trace][scaledX].firstDX ) {
        plotInfo[trace][scaledX].firstDX = oneX;
        plotInfo[trace][scaledX].firstX = scaledX;
        plotInfo[trace][scaledX].firstY = scaledY;
      }
      else if ( oneX >= plotInfo[trace][scaledX].lastDX ) {
        plotInfo[trace][scaledX].lastDX = oneX;
        plotInfo[trace][scaledX].lastX = scaledX;
        plotInfo[trace][scaledX].lastY = scaledY;
      }

      (plotInfo[trace][scaledX].n)++;

    }

    arrayNumPoints[trace]++;

  }

}

int xyGraphClass::fillPlotArray (
  int trace,
  int isVector
) {

int i, npts, curCount;
short curX, curY, prevX=0, prevY=0;
double n;

  curCount = npts = 0;

  if ( opMode[trace] == XYGC_K_SCOPE_MODE ) {

    if ( plotInfoHead[trace] == plotInfoTail[trace] ) return npts;

    if ( plotStyle[trace] == XYGC_K_PLOT_STYLE_NEEDLE ) {

      i = plotInfoHead[trace];
      if ( i != plotInfoTail[trace] ) {

        prevX = plotInfo[trace][i].firstX;
        prevY = plotInfo[trace][i].firstY;

        plotBuf[trace][npts].x = prevX;
        plotBuf[trace][npts].y = plotInfo[trace][i].yZero;
        //plotBuf[trace][npts].y = plotAreaY+plotAreaH+11;
        npts++;

        plotBuf[trace][npts].x = prevX;
        plotBuf[trace][npts].y = prevY;
        npts++;

        plotBuf[trace][npts].x = prevX;
        plotBuf[trace][npts].y = plotInfo[trace][i].yZero;
        //plotBuf[trace][npts].y = plotAreaY+plotAreaH+11;
        npts++;

      }

      if ( !isVector ) {
        curCount++;
        if ( curCount >= count ) return npts;
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
          plotBuf[trace][npts].y = plotInfo[trace][i].yZero;
          //plotBuf[trace][npts].y = plotAreaY+plotAreaH+11;
          npts++;

          plotBuf[trace][npts].x = curX;
          plotBuf[trace][npts].y = curY;
          npts++;

          plotBuf[trace][npts].x = curX;
          plotBuf[trace][npts].y = plotInfo[trace][i].yZero;
          //plotBuf[trace][npts].y = plotAreaY+plotAreaH+11;
          npts++;

        }

        if ( !isVector ) {
          curCount++;
          if ( curCount >= count ) return npts;
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
        if ( special[trace] ) {
          n = (double) npts;
          if ( ( xAxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) ||
               ( xAxisStyle == XYGC_K_AXIS_STYLE_TIME_LOG10 ) ) {
            n = loc_log10(  n  );
          }
          prevX = (short) rint( ( n - curXMin ) *
           xFactor[trace] + xOffset[trace] );
          prevX = sclamp( prevX );
        }
        else {
          prevX = plotInfo[trace][i].firstX;
        }
        prevY = plotInfo[trace][i].firstY;
        plotBuf[trace][npts].x = prevX;
        plotBuf[trace][npts].y = prevY;
        npts++;
      }

      if ( !isVector ) {
        curCount++;
        if ( curCount >= count ) return npts;
      }

      i++;
      if ( i >= plotBufSize[trace] ) { // use plotBufSize here
        i = 0;
      }

      while ( i != plotInfoTail[trace] ) {

        if ( special[trace] ) {
          n = (double) npts;
          if ( ( xAxisStyle == XYGC_K_AXIS_STYLE_LOG10 ) ||
               ( xAxisStyle == XYGC_K_AXIS_STYLE_TIME_LOG10 ) ) {
            n = loc_log10(  n  );
          }
          curX = (short) rint( ( n - curXMin ) *
           xFactor[trace] + xOffset[trace] );
          curX = sclamp( curX );
        }
        else {
          curX = plotInfo[trace][i].firstX;
        }
        curY = plotInfo[trace][i].firstY;

        if ( ( curX != prevX ) || ( curY != prevY ) ) {
          prevX = curX;
          plotBuf[trace][npts].x = curX;
          prevY = curY;
          plotBuf[trace][npts].y = curY;
          npts++;
        }

        if ( !isVector ) {
          curCount++;
          if ( curCount >= count ) return npts;
	}

        i++;
        if ( i >= plotBufSize[trace] ) { // use plotBufSize here
          i = 0;
        }

      }

    }

  }
  else { // plot sorted by x

    if ( plotStyle[trace] == XYGC_K_PLOT_STYLE_NEEDLE ) {

      // because we are dealing with scaled values, top of needle is minY
      // (y increases downward)

      npts = 0;
      for ( i=plotAreaX; i<=plotInfoSize[trace]; i++ ) {

        if ( plotInfo[trace][i].n == 1 ) {

          plotBuf[trace][npts].x = plotInfo[trace][i].firstX;
          plotBuf[trace][npts].y = plotInfo[trace][i].yZero;
          //plotBuf[trace][npts].y = plotAreaY+plotAreaH+11;
          npts++;

          plotBuf[trace][npts].x = plotInfo[trace][i].firstX;
          plotBuf[trace][npts].y = plotInfo[trace][i].firstY;
          npts++;

          plotBuf[trace][npts].x = plotInfo[trace][i].firstX;
          plotBuf[trace][npts].y = plotInfo[trace][i].yZero;
          //plotBuf[trace][npts].y = plotAreaY+plotAreaH+11;
          npts++;

        }
        else if ( plotInfo[trace][i].n == 2 ) {

          plotBuf[trace][npts].x = plotInfo[trace][i].firstX;
          plotBuf[trace][npts].y = plotInfo[trace][i].yZero;
          //plotBuf[trace][npts].y = plotAreaY+plotAreaH+11;
          npts++;

          plotBuf[trace][npts].x = plotInfo[trace][i].firstX;
          plotBuf[trace][npts].y = plotInfo[trace][i].firstY;
          npts++;

          plotBuf[trace][npts].x = plotInfo[trace][i].firstX;
          plotBuf[trace][npts].y = plotInfo[trace][i].lastY;
          npts++;

          plotBuf[trace][npts].x = plotInfo[trace][i].firstX;
          plotBuf[trace][npts].y = plotInfo[trace][i].yZero;
          //plotBuf[trace][npts].y = plotAreaY+plotAreaH+11;
          npts++;

        }
        else if ( plotInfo[trace][i].n > 2 ) {

          plotBuf[trace][npts].x = plotInfo[trace][i].firstX;
          plotBuf[trace][npts].y = plotInfo[trace][i].yZero;
          //plotBuf[trace][npts].y = plotAreaY+plotAreaH+11;
          npts++;

          plotBuf[trace][npts].x = plotInfo[trace][i].firstX;
          plotBuf[trace][npts].y = plotInfo[trace][i].minY;
          npts++;

          plotBuf[trace][npts].x = plotInfo[trace][i].firstX;
          plotBuf[trace][npts].y = plotInfo[trace][i].yZero;
          //plotBuf[trace][npts].y = plotAreaY+plotAreaH+11;
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

  }

  return npts;

}

int xyGraphClass::fillScalarPlotArray (
  int trace
) {

int stat;

  stat = fillPlotArray ( trace, 0 );
  return stat;

}

int xyGraphClass::fillVectorPlotArray (
  int trace
) {

int stat;

  stat = fillPlotArray ( trace, 1 );
  return stat;

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

    if ( ( xAxisStyle == XYGC_K_AXIS_STYLE_TIME ) &&
         ( xAxisTimeFormat != XYGC_K_AXIS_TIME_FMT_SEC ) ) {

      {

        time_t t;

        edmTime base( (const unsigned long) ( curSec ),
         (const unsigned long) curNsec );

        t = base.getSec() + timeOffset;

        drawXLinearTimeScale ( actWin->d, pixmap, &actWin->executeGc, xAxis,
         plotAreaX, plotAreaY+plotAreaH, plotAreaW,
         t, curXMin, curXMax, xAxisTimeFormat,
         curXNumLabelTicks, curXMajorsPerLabel, curXMinorsPerMajor,
         actWin->ci->pix(fgColor), actWin->executeGc.getBaseBG(), xLabelGrid,
         xMajorGrid, xMinorGrid, plotAreaH, actWin->ci->pix(gridColor),
         actWin->fi, fontTag, fs, 1,
         !kpXMinEfDouble.isNull(), !kpXMaxEfDouble.isNull(),
         0 );

      }

    }
    else {

      if ( xGridMode == XYGC_K_USER_SPECIFIED ) {
        curXNumLabelTicks = xNumLabelIntervals.value();
        if ( curXNumLabelTicks < 1 ) curXNumLabelTicks = 1;
        curXMajorsPerLabel = xNumMajorPerLabel.value();
        curXMinorsPerMajor = xNumMinorPerMajor.value();
      }

      drawXLinearScale2 ( actWin->d, pixmap, &actWin->executeGc, xAxis,
       plotAreaX, plotAreaY+plotAreaH, plotAreaW,
       curXMin, curXMax, adjCurXMin, adjCurXMax,
       curXNumLabelTicks, curXMajorsPerLabel, curXMinorsPerMajor,
       actWin->ci->pix(fgColor), actWin->executeGc.getBaseBG(), xLabelGrid,
       xMajorGrid, xMinorGrid, plotAreaH, actWin->ci->pix(gridColor),
       actWin->fi, fontTag, fs, 1,
       !kpXMinEfDouble.isNull(), !kpXMaxEfDouble.isNull(),
       0 );

    }

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

int yi = 0;

  if ( y1AxisStyle[yi] == XYGC_K_AXIS_STYLE_LOG10 ) {

    drawYLog10Scale ( actWin->d, pixmap, &actWin->executeGc, y1Axis[yi],
     plotAreaX, plotAreaY+plotAreaH, plotAreaH,
     curY1Min[yi], curY1Max[yi],
     curY1NumLabelTicks[yi], curY1MajorsPerLabel[yi], curY1MinorsPerMajor[yi],
     actWin->ci->pix(fgColor), actWin->executeGc.getBaseBG(), y1LabelGrid[yi],
     y1MajorGrid[yi], y1MinorGrid[yi], plotAreaW, actWin->ci->pix(gridColor),
     actWin->fi, fontTag, fs, 1,
     !kpY1MinEfDouble[yi].isNull(), !kpY1MaxEfDouble[yi].isNull(),
     0 );

    if ( y1Axis[yi] ) {
      getYLog10LimitCoords( plotAreaX, plotAreaY+plotAreaH, plotAreaH,
       curY1Min[yi], curY1Max[yi],
       curY1NumLabelTicks[yi], fontTag, fs,
       &y1MinX0[yi], &y1MinX1[yi], &y1MinY0[yi], &y1MinY1[yi],
       &y1MaxX0[yi], &y1MaxX1[yi], &y1MaxY0[yi], &y1MaxY1[yi] );
    }
    else {
      y1MinX0[yi] = y1MinX1[yi] = y1MinY0[yi] = y1MinY1[yi] = 0;
      y1MaxX0[yi] = y1MaxX1[yi] = y1MaxY0[yi] = y1MaxY1[yi] = -1;
    }

  }
  else {

    if ( y1GridMode[yi] == XYGC_K_USER_SPECIFIED ) {
      curY1NumLabelTicks[yi] = y1NumLabelIntervals[yi].value();
      if ( curY1NumLabelTicks[yi] < 1 ) curY1NumLabelTicks[yi] = 1;
      curY1MajorsPerLabel[yi] = y1NumMajorPerLabel[yi].value();
      curY1MinorsPerMajor[yi] = y1NumMinorPerMajor[yi].value();
    }

    drawYLinearScale2 ( actWin->d, pixmap, &actWin->executeGc, y1Axis[yi],
     plotAreaX, plotAreaY+plotAreaH, plotAreaH,
     curY1Min[yi], curY1Max[yi],
     adjCurY1Min[yi], adjCurY1Max[yi],
     curY1NumLabelTicks[yi], curY1MajorsPerLabel[yi], curY1MinorsPerMajor[yi],
     actWin->ci->pix(fgColor), actWin->executeGc.getBaseBG(), y1LabelGrid[yi],
     y1MajorGrid[yi], y1MinorGrid[yi], plotAreaW, actWin->ci->pix(gridColor),
     actWin->fi, fontTag, fs, 1,
     !kpY1MinEfDouble[yi].isNull(), !kpY1MaxEfDouble[yi].isNull(),
     0 );

    if ( y1Axis[yi] ) {
      getYLimitCoords( plotAreaX, plotAreaY+plotAreaH, plotAreaH,
       curY1Min[yi], curY1Max[yi],
       curY1NumLabelTicks[yi], fontTag, fs,
       &y1MinX0[yi], &y1MinX1[yi], &y1MinY0[yi], &y1MinY1[yi],
       &y1MaxX0[yi], &y1MaxX1[yi], &y1MaxY0[yi], &y1MaxY1[yi] );
    }
    else {
      y1MinX0[yi] = y1MinX1[yi] = y1MinY0[yi] = y1MinY1[yi] = 0;
      y1MaxX0[yi] = y1MaxX1[yi] = y1MaxY0[yi] = y1MaxY1[yi] = -1;
    }
  }

}

void xyGraphClass::drawY2Scale ( void ) {

int yi = 1;

  if ( y1AxisStyle[yi] == XYGC_K_AXIS_STYLE_LOG10 ) {

    drawY2Log10Scale ( actWin->d, pixmap, &actWin->executeGc, y1Axis[yi],
     plotAreaX+plotAreaW, plotAreaY+plotAreaH, plotAreaH,
     curY1Min[yi], curY1Max[yi],
     curY1NumLabelTicks[yi], curY1MajorsPerLabel[yi], curY1MinorsPerMajor[yi],
     actWin->ci->pix(fgColor), actWin->executeGc.getBaseBG(), y1LabelGrid[yi],
     y1MajorGrid[yi], y1MinorGrid[yi], plotAreaW, actWin->ci->pix(gridColor),
     actWin->fi, fontTag, fs, 1,
     !kpY1MinEfDouble[yi].isNull(), !kpY1MaxEfDouble[yi].isNull(),
     0 );

    if ( y1Axis[yi] ) {
      getY2Log10LimitCoords( plotAreaX+plotAreaW, plotAreaY+plotAreaH,
       plotAreaH,
       curY1Min[yi], curY1Max[yi],
       curY1NumLabelTicks[yi], fontTag, fs,
       &y1MinX0[yi], &y1MinX1[yi], &y1MinY0[yi], &y1MinY1[yi],
       &y1MaxX0[yi], &y1MaxX1[yi], &y1MaxY0[yi], &y1MaxY1[yi] );
    }
    else {
      y1MinX0[yi] = y1MinX1[yi] = y1MinY0[yi] = y1MinY1[yi] = 0;
      y1MaxX0[yi] = y1MaxX1[yi] = y1MaxY0[yi] = y1MaxY1[yi] = -1;
    }

  }
  else {

    if ( y1GridMode[yi] == XYGC_K_USER_SPECIFIED ) {
      curY1NumLabelTicks[yi] = y1NumLabelIntervals[yi].value();
      if ( curY1NumLabelTicks[yi] < 1 ) curY1NumLabelTicks[yi] = 1;
      curY1MajorsPerLabel[yi] = y1NumMajorPerLabel[yi].value();
      curY1MinorsPerMajor[yi] = y1NumMinorPerMajor[yi].value();
    }

    drawY2LinearScale2 ( actWin->d, pixmap, &actWin->executeGc, y1Axis[yi],
     plotAreaX+plotAreaW, plotAreaY+plotAreaH, plotAreaH,
     curY1Min[yi], curY1Max[yi],
     adjCurY1Min[yi], adjCurY1Max[yi],
     curY1NumLabelTicks[yi], curY1MajorsPerLabel[yi], curY1MinorsPerMajor[yi],
     actWin->ci->pix(fgColor), actWin->executeGc.getBaseBG(), y1LabelGrid[yi],
     y1MajorGrid[yi], y1MinorGrid[yi], plotAreaW, actWin->ci->pix(gridColor),
     actWin->fi, fontTag, fs, 1,
     !kpY1MinEfDouble[yi].isNull(), !kpY1MaxEfDouble[yi].isNull(),
     0 );

    if ( y1Axis[yi] ) {
      getY2LimitCoords( plotAreaX+plotAreaW, plotAreaY+plotAreaH, plotAreaH,
       curY1Min[yi], curY1Max[yi],
       curY1NumLabelTicks[yi], fontTag, fs,
       &y1MinX0[yi], &y1MinX1[yi], &y1MinY0[yi], &y1MinY1[yi],
       &y1MaxX0[yi], &y1MaxX1[yi], &y1MaxY0[yi], &y1MaxY1[yi] );
    }
    else {
      y1MinX0[yi] = y1MinX1[yi] = y1MinY0[yi] = y1MinY1[yi] = 0;
      y1MaxX0[yi] = y1MaxX1[yi] = y1MaxY0[yi] = y1MaxY1[yi] = -1;
    }
  }

}

void xyGraphClass::drawGrid ( void ) {

int yi;

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

      if ( ( xAxisStyle == XYGC_K_AXIS_STYLE_TIME ) &&
           ( xAxisTimeFormat != XYGC_K_AXIS_TIME_FMT_SEC ) ) {

        {

          time_t t;

          edmTime base( (const unsigned long) ( curSec ),
           (const unsigned long) curNsec );

          t = base.getSec() + timeOffset;

          drawXLinearTimeScale ( actWin->d, pixmap, &actWin->executeGc, xAxis,
           plotAreaX, plotAreaY+plotAreaH, plotAreaW,
           t, curXMin, curXMax, xAxisTimeFormat,
           curXNumLabelTicks, curXMajorsPerLabel, curXMinorsPerMajor,
           actWin->ci->pix(fgColor), actWin->executeGc.getBaseBG(), xLabelGrid,
           xMajorGrid, xMinorGrid, plotAreaH, actWin->ci->pix(gridColor),
           actWin->fi, fontTag, fs, 1,
           !kpXMinEfDouble.isNull(), !kpXMaxEfDouble.isNull(),
           0 );

        }

      }
      else {

        if ( xGridMode == XYGC_K_USER_SPECIFIED ) {
          curXNumLabelTicks = xNumLabelIntervals.value();
          if ( curXNumLabelTicks < 1 ) curXNumLabelTicks = 1;
          curXMajorsPerLabel = xNumMajorPerLabel.value();
          curXMinorsPerMajor = xNumMinorPerMajor.value();
        }

        drawXLinearScale2 ( actWin->d, pixmap, &actWin->executeGc, xAxis,
         plotAreaX, plotAreaY+plotAreaH, plotAreaW,
         curXMin, curXMax, adjCurXMin, adjCurXMax,
         curXNumLabelTicks, curXMajorsPerLabel, curXMinorsPerMajor,
         actWin->ci->pix(fgColor), actWin->executeGc.getBaseBG(), xLabelGrid,
         xMajorGrid, xMinorGrid, plotAreaH, actWin->ci->pix(gridColor),
         actWin->fi, fontTag, fs, 1,
         !kpXMinEfDouble.isNull(), !kpXMaxEfDouble.isNull(),
         0 );

      }

    }

  }

  for ( yi=0; yi<xyGraphClass::NUM_Y_AXES; yi++ ) {

    if ( y1LabelGrid[yi] || y1MajorGrid[yi] || y1MinorGrid[yi] ) {

      if ( yi == 0 ) {

        if ( y1AxisStyle[yi] == XYGC_K_AXIS_STYLE_LOG10 ) {

          drawYLog10Scale ( actWin->d, pixmap, &actWin->executeGc, y1Axis[yi],
           plotAreaX, plotAreaY+plotAreaH, plotAreaH,
           curY1Min[yi], curY1Max[yi], curY1NumLabelTicks[yi],
           curY1MajorsPerLabel[yi], curY1MinorsPerMajor[yi],
           actWin->ci->pix(fgColor), actWin->executeGc.getBaseBG(),
           y1LabelGrid[yi], y1MajorGrid[yi], y1MinorGrid[yi], plotAreaW,
           actWin->ci->pix(gridColor), actWin->fi, fontTag, fs, 1,
           !kpY1MinEfDouble[yi].isNull(), !kpY1MaxEfDouble[yi].isNull(),
           0 );

        }
        else {

          if ( y1GridMode[yi] == XYGC_K_USER_SPECIFIED ) {
            curY1NumLabelTicks[yi] = y1NumLabelIntervals[yi].value();
            if ( curY1NumLabelTicks[yi] < 1 ) curY1NumLabelTicks[yi] = 1;
            curY1MajorsPerLabel[yi] = y1NumMajorPerLabel[yi].value();
            curY1MinorsPerMajor[yi] = y1NumMinorPerMajor[yi].value();
          }

          drawYLinearScale2 ( actWin->d, pixmap, &actWin->executeGc, y1Axis[yi],
           plotAreaX, plotAreaY+plotAreaH, plotAreaH,
           curY1Min[yi], curY1Max[yi],
           adjCurY1Min[yi], adjCurY1Max[yi],
           curY1NumLabelTicks[yi], curY1MajorsPerLabel[yi],
           curY1MinorsPerMajor[yi], actWin->ci->pix(fgColor),
           actWin->executeGc.getBaseBG(), y1LabelGrid[yi],
           y1MajorGrid[yi], y1MinorGrid[yi], plotAreaW,
           actWin->ci->pix(gridColor), actWin->fi, fontTag, fs, 1,
           !kpY1MinEfDouble[yi].isNull(), !kpY1MaxEfDouble[yi].isNull(),
           0 );

        }

      }
      else {

        if ( y1AxisStyle[yi] == XYGC_K_AXIS_STYLE_LOG10 ) {

          drawY2Log10Scale ( actWin->d, pixmap, &actWin->executeGc, y1Axis[yi],
           plotAreaX+plotAreaW, plotAreaY+plotAreaH, plotAreaH,
           curY1Min[yi], curY1Max[yi],
           curY1NumLabelTicks[yi], curY1MajorsPerLabel[yi], curY1MinorsPerMajor[yi],
           actWin->ci->pix(fgColor), actWin->executeGc.getBaseBG(), y1LabelGrid[yi],
           y1MajorGrid[yi], y1MinorGrid[yi], plotAreaW, actWin->ci->pix(gridColor),
           actWin->fi, fontTag, fs, 1,
           !kpY1MinEfDouble[yi].isNull(), !kpY1MaxEfDouble[yi].isNull(),
           0 );

        }
        else {

          if ( y1GridMode[yi] == XYGC_K_USER_SPECIFIED ) {
            curY1NumLabelTicks[yi] = y1NumLabelIntervals[yi].value();
            if ( curY1NumLabelTicks[yi] < 1 ) curY1NumLabelTicks[yi] = 1;
            curY1MajorsPerLabel[yi] = y1NumMajorPerLabel[yi].value();
            curY1MinorsPerMajor[yi] = y1NumMinorPerMajor[yi].value();
          }

          drawY2LinearScale2 ( actWin->d, pixmap, &actWin->executeGc, y1Axis[yi],
           plotAreaX+plotAreaW, plotAreaY+plotAreaH, plotAreaH,
           curY1Min[yi], curY1Max[yi],
           adjCurY1Min[yi], adjCurY1Max[yi],
           curY1NumLabelTicks[yi], curY1MajorsPerLabel[yi], curY1MinorsPerMajor[yi],
           actWin->ci->pix(fgColor), actWin->executeGc.getBaseBG(), y1LabelGrid[yi],
           y1MajorGrid[yi], y1MinorGrid[yi], plotAreaW, actWin->ci->pix(gridColor),
           actWin->fi, fontTag, fs, 1,
           !kpY1MinEfDouble[yi].isNull(), !kpY1MaxEfDouble[yi].isNull(),
           0 );

        }

      }

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
int lX=0, lY=0, lW=0, inc, stat, useRotated, cW, maxW=0;
char fullName[127+1], label[127+1];

  if ( y1Axis[0] && !blank( yLabel.getExpanded() ) ) {

    strncpy( label, yLabel.getExpanded(), 127 );
    label[127] = 0;

    actWin->executeGc.saveFg();
    actWin->executeGc.setFG( actWin->ci->pix(fgColor) );

    useRotated = 1;

    stat = actWin->fi->getFontName( fontTag, 90.0, fullName, 127 );
    if ( !( stat & 1 ) ) {
      useRotated = 0;
    }

    stat = actWin->executeGc.setNativeFont( fullName, actWin->fi );
    if ( !( stat & 1 ) ) {
      useRotated = 0;
    }

    if ( useRotated ) {
      lX = fontHeight;
      lW = 2 * XTextWidth( fs, label, strlen(label) );
      lY = plotAreaY + ( plotAreaH + lW ) / 2;
    }
    else {
      maxW = XTextWidth( fs, &label[0], 1 );
      for ( i=0; i<strlen(label); i++ ) {
        cW = XTextWidth( fs, &label[i], 1 );
        if ( cW > maxW ) maxW = cW;
      }
      lW = fontHeight * strlen(label);
      actWin->fi->loadFontTag( fontTag );
      actWin->executeGc.setFontTag( fontTag, actWin->fi );
      lY = fontHeight + plotAreaY + ( plotAreaH - lW ) / 2;
    }

    for ( i=0; i<strlen(label); i++ ) {

      if ( !useRotated ) {
        lX = maxW - XTextWidth( fs, &label[i], 1 ) / 2;
      }

      XDrawString( actWin->d, pixmap,
       actWin->executeGc.normGC(), lX, lY, &label[i], 1 );

      if ( useRotated ) {
        inc = 2*XTextWidth( fs, &label[i], 1 );
        lY -= inc;
      }
      else {
        inc = fontHeight;
        lY += inc;
      }

    }

    actWin->executeGc.restoreFg();

  }

}

void xyGraphClass::drawY2label ( void ) {

unsigned int i;
int lX=0, lY=0, lW=0, inc, stat, useRotated, cW, maxW=0;
char fullName[127+1], label[127+1];

  if ( y1Axis[1] && !blank( y2Label.getExpanded() ) ) {

    strncpy( label, y2Label.getExpanded(), 127 );
    label[127] = 0;

    actWin->executeGc.saveFg();
    actWin->executeGc.setFG( actWin->ci->pix(fgColor) );

    useRotated = 1;

    stat = actWin->fi->getFontName( fontTag, 270.0, fullName, 127 );
    if ( !( stat & 1 ) ) {
      useRotated = 0;
    }

    stat = actWin->executeGc.setNativeFont( fullName, actWin->fi );
    if ( !( stat & 1 ) ) {
      useRotated = 0;
    }

    if ( useRotated ) {
      lX = w - fontHeight;
      lW = 2 * XTextWidth( fs, label, strlen(label) );
      lY = plotAreaY + ( plotAreaH - lW ) / 2;
    }
    else {
      maxW = XTextWidth( fs, &label[0], 1 );
      for ( i=0; i<strlen(label); i++ ) {
        cW = XTextWidth( fs, &label[i], 1 );
        if ( cW > maxW ) maxW = cW;
      }
      lW = fontHeight * strlen(label);
      actWin->fi->loadFontTag( fontTag );
      actWin->executeGc.setFontTag( fontTag, actWin->fi );
      lY = fontHeight + plotAreaY + ( plotAreaH - lW ) / 2;
    }

    for ( i=0; i<strlen(label); i++ ) {

      if ( !useRotated ) {
        lX = w - maxW - XTextWidth( fs, &label[i], 1 ) / 2;
      }

      XDrawString( actWin->d, pixmap,
       actWin->executeGc.normGC(), lX, lY, &label[i], 1 );

      if ( useRotated ) {
        inc = 2*XTextWidth( fs, &label[i], 1 );
        lY += inc;
      }
      else {
        inc = fontHeight;
        lY += inc;
      }

    }

    actWin->executeGc.restoreFg();

  }

}

void xyGraphClass::getPvs (
  int max,
  ProcessVariable *pvs[],
  int *n ) {

int i, ii, num;

  num = XYGC_K_MAX_TRACES + XYGC_K_MAX_TRACES + XYGC_K_MAX_TRACES + 3;

  if ( max < num ) {
    *n = 0;
    return;
  }

  *n = num;
  ii = 0;
  for ( i=0; i<XYGC_K_MAX_TRACES; i++ ) {
    pvs[ii++] = xPv[i];
    pvs[ii++] = yPv[i];
    pvs[ii++] = nPv[i];
  }
  pvs[ii++] = resetPv;
  pvs[ii++] = trigPv;
  pvs[ii++] = traceCtlPv;

}

char *xyGraphClass::getSearchString (
  int i
) {

int num = XYGC_K_MAX_TRACES + XYGC_K_MAX_TRACES + XYGC_K_MAX_TRACES + 6;
int ii, selector, index;

  if ( i == 0 ) {
    return graphTitle.getRaw();
  }
  else if ( i == 1 ) {
    return xLabel.getRaw();
  }
  else if ( i == 2 ) {
    return yLabel.getRaw();
  }
  else if ( i == 3 ) {
    return y2Label.getRaw();
  }
  else if ( i == 4 ) {
    return traceCtlPvExpStr.getRaw();
  }
  else if ( i == 5 ) {
    return trigPvExpStr.getRaw();
  }
  else if ( i == 6 ) {
    return resetPvExpStr.getRaw();
  }
  else if ( ( i > 6 ) && ( i < num ) ) {
    ii = i - 7;
    selector = ii % 3;
    index = ii / 3;
    if ( selector == 0 ) {
      return xPvExpStr[index].getRaw();
    }
    else if ( selector == 1 ) {
      return yPvExpStr[index].getRaw();
    }
    else if ( selector == 2 ) {
      return nPvExpStr[index].getRaw();
    }
  }

  return NULL;

}

void xyGraphClass::replaceString (
  int i,
  int max,
  char *string
) {

int num = XYGC_K_MAX_TRACES + XYGC_K_MAX_TRACES + XYGC_K_MAX_TRACES + 6;
int ii, selector, index;

  if ( i == 0 ) {
    graphTitle.setRaw( string );
  }
  else if ( i == 1 ) {
    xLabel.setRaw( string );
  }
  else if ( i == 2 ) {
    yLabel.setRaw( string );
  }
  else if ( i == 3 ) {
    y2Label.setRaw( string );
  }
  else if ( i == 4 ) {
    traceCtlPvExpStr.setRaw( string );
  }
  else if ( i == 5 ) {
    trigPvExpStr.setRaw( string );
  }
  else if ( i == 6 ) {
    resetPvExpStr.setRaw( string );
  }
  else if ( ( i > 6 ) && ( i < num ) ) {
    ii = i - 7;
    selector = ii % 3;
    index = ii / 3;
    if ( selector == 0 ) {
      xPvExpStr[index].setRaw( string );
    }
    else if ( selector == 1 ) {
      yPvExpStr[index].setRaw( string );
    }
    else if ( selector == 2 ) {
      nPvExpStr[index].setRaw( string );
    }
  }

}

// crawler functions may return blank pv names
char *xyGraphClass::crawlerGetFirstPv ( void ) {

  crawlerPvIndex = 0;
  return trigPvExpStr.getExpanded();

}

char *xyGraphClass::crawlerGetNextPv ( void ) {

int i, max;

  max = numTraces*2 + 2;

  if ( crawlerPvIndex >= max ) return NULL;

  crawlerPvIndex++;

  if ( crawlerPvIndex == 1 ) {
    return traceCtlPvExpStr.getExpanded();
  }
  else if ( crawlerPvIndex == 2 ) {
    return resetPvExpStr.getExpanded();
  }
  else {

    // index starts here from 3; x is odd, y is even

    i = ( crawlerPvIndex - 3 ) / 2;

    if ( crawlerPvIndex % 2 ) {

      // odd - x

      return xPvExpStr[i].getExpanded();

    }
    else {

      // even - y

      return yPvExpStr[i].getExpanded();

    }

  }

  return NULL;

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
