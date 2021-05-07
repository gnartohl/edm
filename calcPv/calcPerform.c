/*************************************************************************\
* Copyright (c) 2002 The University of Chicago, as Operator of Argonne
*     National Laboratory.
* Copyright (c) 2002 The Regents of the University of California, as
*     Operator of Los Alamos National Laboratory.
* EPICS BASE Versions 3.13.7
* and higher are distributed subject to a Software License Agreement found
* in file LICENSE that is included with this distribution.
\*************************************************************************/
/* calcPerform.c,v 1.37.2.5 2006/11/30 22:29:08 jhill Exp */
/*
 *	Author: Julie Sander and Bob Dalesio
 *	Date:	07-27-87
 */

#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <errlog.h>

#include "postfix.h"
#include "postfixPvt.h"

#ifdef SOLARIS
#include <ieeefp.h>

int isinf(double x) { return !finite(x) && x==x; }
#endif

static double calcRandom(void);
static int cond_search(const char **ppinst, int match);

#ifndef PI
#define PI 3.14159265358979323
#endif

typedef unsigned int epicsUInt32;
typedef int epicsInt32;

/* calcPerform
 *
 * Evalutate the postfix expression
 */
long edm_calcPerform(double *parg, double *presult, const char *pinst)
{
    double stack[CALCPERFORM_STACK+1];	/* zero'th entry not used */
    double *ptop;			/* stack pointer */
    double top; 			/* value from top of stack */
    int itop;				/* integer from top of stack */
    epicsUInt32 utop;			/* unsigned integer from top of stack */

    /* initialize */
    ptop = stack;

    if(*pinst == END_EXPRESSION) return -1;

    /* RPN evaluation loop */
    while (*pinst != END_EXPRESSION){
	switch (*pinst){

	case LITERAL:
	    ++ptop;
	    ++pinst;
	    memcpy((void *)ptop, pinst, sizeof(double));
	    pinst += sizeof(double) - 1;
	    break;

	case FETCH_A:
	case FETCH_B:
	case FETCH_C:
	case FETCH_D:
	case FETCH_E:
	case FETCH_F:
	case FETCH_G:
	case FETCH_H:
	case FETCH_I:
	case FETCH_J:
	case FETCH_K:
	case FETCH_L:
	    *++ptop = parg[*pinst - FETCH_A];
	    break;

	case STORE_A:
	case STORE_B:
	case STORE_C:
	case STORE_D:
	case STORE_E:
	case STORE_F:
	case STORE_G:
	case STORE_H:
	case STORE_I:
	case STORE_J:
	case STORE_K:
	case STORE_L:
	    parg[*pinst - STORE_A] = *ptop--;
	    break;

	case CONST_PI:
	    *++ptop = PI;
	    break;

	case CONST_D2R:
	    *++ptop = PI/180.;
	    break;

	case CONST_R2D:
	    *++ptop = 180./PI;
	    break;

	case UNARY_NEG:
	    *ptop = - *ptop;
	    break;

	case ADD:
	    top = *ptop--;
	    *ptop += top;
	    break;

	case SUB:
	    top = *ptop--;
	    *ptop -= top;
	    break;

	case MULT:
	    top = *ptop--;
	    *ptop *= top;
	    break;

	case DIV:
	    top = *ptop--;
	    *ptop /= top;
	    break;

	case MODULO:
	    top = *ptop--;
	    *ptop = fmod(*ptop, top);
	    break;

	case POWER:
	    top = *ptop--;
	    *ptop = pow(*ptop, top);
	    break;

	case ABS_VAL:
	    if (*ptop < 0.0) *ptop = - *ptop;
	    break;

	case EXP:
	    *ptop = exp(*ptop);
	    break;

	case LOG_10:
	    *ptop = log10(*ptop);
	    break;

	case LOG_E:
	    *ptop = log(*ptop);
	    break;

	case MAX:
	    top = *ptop--;
	    if (*ptop < top || isnan(top))
		*ptop = top;
	    break;

	case MIN:
	    top = *ptop--;
	    if (*ptop > top || isnan(top))
		*ptop = top;
	    break;

	case SQU_RT:
	    *ptop = sqrt(*ptop);
	    break;

	case ACOS:
	    *ptop = acos(*ptop);
	    break;

	case ASIN:
	    *ptop = asin(*ptop);
	    break;

	case ATAN:
	    *ptop = atan(*ptop);
	    break;

	case ATAN2:
	    top = *ptop--;
	    *ptop = atan2(top, *ptop);	/* Ouch!: Args backwards! */
	    break;

	case COS:
	    *ptop = cos(*ptop);
	    break;

	case SIN:
	    *ptop = sin(*ptop);
	    break;

	case TAN:
	    *ptop = tan(*ptop);
	    break;

	case COSH:
	    *ptop = cosh(*ptop);
	    break;

	case SINH:
	    *ptop = sinh(*ptop);
	    break;

	case TANH:
	    *ptop = tanh(*ptop);
	    break;

	case CEIL:
	    *ptop = ceil(*ptop);
	    break;

	case FLOOR:
	    *ptop = floor(*ptop);
	    break;

	case FINITE:
	    *ptop = finite(*ptop);
	    break;

	case ISINF:
	    *ptop = isinf(*ptop);
	    break;

	case ISNAN:
	    *ptop = isnan(*ptop);
	    break;

	case NINT:
	    top = *ptop;
	    *ptop = (double)(long)(top >= 0 ? top + 0.5 : top - 0.5);
	    break;

	case RANDOM:
	    *++ptop = calcRandom();
	    break;

	case REL_OR:
	    top = *ptop--;
	    *ptop = *ptop || top;
	    break;

	case REL_AND:
	    top = *ptop--;
	    *ptop = *ptop && top;
	    break;

	case REL_NOT:
	    *ptop = ! *ptop;
	    break;

        /* For bitwise operations on values with bit 31 set, double values
         * must first be cast to unsigned to correctly set that bit; the
         * double value must be negative in that case. The result must be
         * cast to a signed integer before converting to the double result.
         */

	case BIT_OR:
	    utop = *ptop--;
	    *ptop = (epicsInt32) ((epicsUInt32) *ptop | utop);
	    break;

	case BIT_AND:
	    utop = *ptop--;
	    *ptop = (epicsInt32) ((epicsUInt32) *ptop & utop);
	    break;

	case BIT_EXCL_OR:
	    utop = *ptop--;
	    *ptop = (epicsInt32) ((epicsUInt32) *ptop ^ utop);
	    break;

	case BIT_NOT:
	    utop = *ptop;
	    *ptop = (epicsInt32) ~utop;
	    break;

        /* The shift operators use signed integers, so a right-shift will
         * extend the sign bit into the left-hand end of the value. The
         * double-casting through unsigned here is important, see above.
         */

	case RIGHT_SHIFT:
	    utop = *ptop--;
	    *ptop = ((epicsInt32) (epicsUInt32) *ptop) >> (utop & 31);
	    break;

	case LEFT_SHIFT:
	    utop = *ptop--;
	    *ptop = ((epicsInt32) (epicsUInt32) *ptop) << (utop & 31);
	    break;

	case NOT_EQ:
	    top = *ptop--;
	    *ptop = *ptop != top;
	    break;

	case LESS_THAN:
	    top = *ptop--;
	    *ptop = *ptop < top;
	    break;

	case LESS_OR_EQ:
	    top = *ptop--;
	    *ptop = *ptop <= top;
	    break;

	case EQUAL:
	    top = *ptop--;
	    *ptop = *ptop == top;
	    break;

	case GR_OR_EQ:
	    top = *ptop--;
	    *ptop = *ptop >= top;
	    break;

	case GR_THAN:
	    top = *ptop--;
	    *ptop = *ptop > top;
	    break;

	case COND_IF:
	    if (*ptop-- == 0.0 &&
		cond_search(&pinst, COND_ELSE)) return -1;
	    break;

	case COND_ELSE:
	    if (cond_search(&pinst, COND_END)) return -1;
	    break;

	case COND_END:
	    break;

	default:
	    errlogPrintf("calcPerform: Bad Opcode %d at %p\n",*pinst, pinst);
	    return -1;
	}

	/* Advance to next opcode */
	++pinst;
    }

    /* The stack should now have one item on it, the expression value */
    if (ptop != stack + 1)
	return -1;
    *presult = *ptop;
    return 0;
}


long edm_calcArgUsage(const char *pinst, unsigned long *pinputs, unsigned long *pstores)
{
    unsigned long inputs = 0;
    unsigned long stores = 0;
    char op;
    while ((op = *pinst) != END_EXPRESSION) {
	switch (op) {

	case LITERAL:
	    pinst += sizeof(double);
	    break;

	case FETCH_A:
	case FETCH_B:
	case FETCH_C:
	case FETCH_D:
	case FETCH_E:
	case FETCH_F:
	case FETCH_G:
	case FETCH_H:
	case FETCH_I:
	case FETCH_J:
	case FETCH_K:
	case FETCH_L:
	    /* Don't claim to use an arg we already stored to */
	    inputs |= (1 << (op - FETCH_A)) & ~stores;
	    break;

	case STORE_A:
	case STORE_B:
	case STORE_C:
	case STORE_D:
	case STORE_E:
	case STORE_F:
	case STORE_G:
	case STORE_H:
	case STORE_I:
	case STORE_J:
	case STORE_K:
	case STORE_L:
	    stores |= (1 << (op - STORE_A));
	    break;

	default:
	    break;
	}
	pinst++;
    }
    if (pinputs) *pinputs = inputs;
    if (pstores) *pstores = stores;
    return 0;
}

/* Generate a random number between 0 and 1 using the algorithm
 * seed = (multy * seed) + addy         Random Number Generator by Knuth
 *                                              SemiNumerical Algorithms
 *                                              Chapter 1
 * randy = seed / 65535.0          To normalize the number between 0 - 1
 */
static unsigned short seed = 0xa3bf;
static unsigned short multy = 191 * 8 + 5;  /* 191 % 8 == 5 */
static unsigned short addy = 0x3141;

static double calcRandom()
{
    seed = (seed * multy) + addy;

    /* between 0 - 1 */
    return (double) seed / 65535.0;
}

/* Search the instruction stream for a matching operator, skipping any
 * other conditional instructions found
 */
static int cond_search(const char **ppinst, int match)
{
    const char *pinst = *ppinst + 1;
    int count = 1;
    int op;

    while ((op = *pinst) != END_EXPRESSION) {
	if (op == match && --count == 0) {
	    *ppinst = pinst;
	    return 0;
	} else if (op == COND_IF)
	    count++;
	else if (op == LITERAL)
	    pinst += sizeof(double);
	pinst++;
    }
    return 1;
}

