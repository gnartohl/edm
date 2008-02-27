/* This is postfix.c and calcPerform.c from R3.13.3 EPICS/base/src/libCom
 * modified to work without vxWorks.h, dbDefs.h, EPICS Sharelib stuff
 * kasemir@lanl.gov
 */

/* postfix.c,v 1.32.6.2 2000/02/02 22:25:27 jba Exp
 * Subroutines used to convert an infix expression to a postfix expression
 *
 *      Author:          Bob Dalesio
 *      Date:            12-12-86
 *
 *      Experimental Physics and Industrial Control System (EPICS)
 *
 *      Copyright 1991, the Regents of the University of California,
 *      and the University of Chicago Board of Governors.
 *
 *      This software was produced under  U.S. Government contracts:
 *      (W-7405-ENG-36) at the Los Alamos National Laboratory,
 *      and (W-31-109-ENG-38) at Argonne National Laboratory.
 *
 *      Initial development by:
 *              The Controls and Automation Group (AT-8)
 *              Ground Test Accelerator
 *              Accelerator Technology Division
 *              Los Alamos National Laboratory
 *
 *      Co-developed with
 *              The Controls and Computing Group
 *              Accelerator Systems Division
 *              Advanced Photon Source
 *              Argonne National Laboratory
 *
 * Modification Log:
 * -----------------
 * .01  01-11-89        lrd     added right shift and left shift operations
 * .02  01-13-89        lrd     modified to load into IOCs
 * .03  02-01-89        lrd     added trigonometric functions
 * .04  04-05-89        lrd     fixed the order of some operations in the
 *                              element table and added a warning label
 * .05  11-26-90        lrd     fix SINH, COSH, TANH
 * .06	02-20-92	rcz	fixed for vxWorks build
 * .07  02-24-92        jba     add EXP and fixed trailing blanks in expression
 * .08  03-03-92        jba     added MAX and MIN and comma(like close paren)
 * .09  03-06-92        jba     added multiple conditional expressions ?
 * .10  04-01-92        jba     allowed floating pt constants in expression
 * .11  05-01-92        jba     flt pt constant string replaced with double in postfix
 * .12  08-21-92        jba     ANSI c changes
 * .13  08-21-92        jba     initialized *ppostfix: needed when calc expr not defined
 * .14  12-11-92	mrk	Removed include for stdioLib.h
 * .15  11-03-93		jba		Added test for extra close paren at end of expression
 * .16  01-24-94		jba		Changed seperator test to catch invalid commas
 * .17  05-11-94		jba		Added support for CONST_PI, CONST_R2D, and CONST_D2R
 * 								and conversion of infix expression to uppercase
*/

/* 
 * Subroutines
 *
 *	Public
 *
 * postfix		convert an algebraic expression to symbolic postfix
 *	args
 *		pinfix		the algebraic expression
 *		ppostfix	the symbolic postfix expression
 *	returns
 *		0		successful
 *		-1		not successful
 * Private routines for postfix
 *
 * find_element		finds a symbolic element in the expression element tbl
 *	args
 *		pbuffer		pointer to the infox expression element
 *		pelement	pointer to the expression element table entry
 *		pno_bytes	pointer to the size of this element
 *	returns
 *		TRUE		element found
 *		FALSE		element not found
 * get_element		finds the next expression element in the infix expr
 *	args
 *		pinfix		pointer into the infix expression
 *		pelement	pointer to the expression element table
 *		pno_bytes	size of the element in the infix expression
 *		plink		pointer to a resolved database reference (N/A)
 *	returns
 *		FINE		found an expression element
 *		VARIABLE	found a database reference
 *		UNKNOWN_ELEMENT	unknown element found in the infix expression
 * match_element	finds an alpha element in the expression table
 *	args
 *		pbuffer		pointer to an alpha expression element
 *		pelement	pointer to the expression element table
 *	returns
 *		TRUE		found the element in the element table
 *		FLASE		expression element not found
 */

#include	<stdlib.h>
#include	<stdio.h>
#include	<string.h>
#include	<ctype.h>
#include	<math.h>

#define epicsExportSharedSymbols
#include	"postfix.h"
#include	"postfixPvt.h"


/* declarations for postfix */
/* element types */
#define	OPERAND		0
#define UNARY_OPERATOR	1
#define	BINARY_OPERATOR	2
#define	EXPR_TERM	3
#define	COND		4
#define	CLOSE_PAREN	5
#define	CONDITIONAL	6
#define	ELSE		7
#define	SEPERATOR	8
#define	TRASH		9
#define	FLOAT_PT_CONST	10
#define	MINUS_OPERATOR	11

#define UNARY_MINUS_I_S_P  7
#define UNARY_MINUS_I_C_P  8
#define UNARY_MINUS_CODE   UNARY_NEG
#define BINARY_MINUS_I_S_P 4
#define BINARY_MINUS_I_C_P 4
#define BINARY_MINUS_CODE  SUB

/* parsing return values */
#define	FINE		0
#define	UNKNOWN_ELEMENT	-1
#define	END		-2

/*
 * element table
 *
 * structure of an element
 */
struct	expression_element{
	char	element[10];	/* character representation of an element */
	char	in_stack_pri;	/* priority in translation stack */
	char	in_coming_pri;	/* priority when first checking */
	char	type;	/* element type */
	char	code;			/* postfix representation */
};

/*
 * NOTE: DO NOT CHANGE WITHOUT READING THIS NOTICE !!!!!!!!!!!!!!!!!!!!
 * Because the routine that looks for a match in this table takes the first 
 * match it finds, elements whose designations are contained in other elements
 * MUST come first in this list. (e.g. ABS will match A if A preceeds ABS and
 * then try to find BS therefore ABS must be first in this list
 */
static struct expression_element	elements[] = {
/* element	i_s_p	i_c_p	type_element	internal_rep */
{"ABS",		7,	8,	UNARY_OPERATOR,	ABS_VAL}, /* absolute value */
{"NOT",		7,	8,	UNARY_OPERATOR,	UNARY_NEG}, /* unary negate */
{"-",		7,	8,	MINUS_OPERATOR,	UNARY_NEG}, /* unary negate (or binary op) */
{"SQRT",		7,	8,	UNARY_OPERATOR,	SQU_RT}, /* square root */
{"SQR",		7,	8,	UNARY_OPERATOR,	SQU_RT}, /* square root */
{"EXP",		7,	8,	UNARY_OPERATOR,	EXP}, /* exponential function */
{"LOGE",		7,	8,	UNARY_OPERATOR,	LOG_E}, /* log E */
{"LN",		7,	8,	UNARY_OPERATOR,	LOG_E}, /* log E */
{"LOG",		7,	8,	UNARY_OPERATOR,	LOG_10}, /* log 10 */
{"ACOS",		7,	8,	UNARY_OPERATOR,	ACOS}, /* arc cosine */
{"ASIN",		7,	8,	UNARY_OPERATOR,	ASIN}, /* arc sine */
{"ATAN2",	7,	8,	UNARY_OPERATOR,	ATAN2}, /* arc tangent */
{"ATAN",		7,	8,	UNARY_OPERATOR,	ATAN}, /* arc tangent */
{"MAX",		7,	8,	UNARY_OPERATOR,	MAX}, /* maximum of 2 args */
{"MIN",		7,	8,	UNARY_OPERATOR,	MIN}, /* minimum of 2 args */
{"CEIL",		7,	8,	UNARY_OPERATOR,	CEIL}, /* smallest integer >= */
{"FLOOR",	7,	8,	UNARY_OPERATOR,	FLOOR}, /* largest integer <=  */
{"NINT",		7,	8,	UNARY_OPERATOR,	NINT}, /* nearest integer */
{"COSH",		7,	8,	UNARY_OPERATOR,	COSH}, /* hyperbolic cosine */
{"COS",		7,	8,	UNARY_OPERATOR,	COS}, /* cosine */
{"SINH",		7,	8,	UNARY_OPERATOR,	SINH}, /* hyperbolic sine */
{"SIN",		7,	8,	UNARY_OPERATOR,	SIN}, /* sine */
{"TANH",		7,	8,	UNARY_OPERATOR,	TANH}, /* hyperbolic tangent*/
{"TAN",		7,	8,	UNARY_OPERATOR,	TAN}, /* tangent */
{"!",		7,	8,	UNARY_OPERATOR, REL_NOT}, /* not */
{"~",		7,	8,	UNARY_OPERATOR, BIT_NOT}, /* and */
{"RNDM",    	0,	0,	OPERAND,	RANDOM}, /* Random Number */
{"OR",		1,	1,	BINARY_OPERATOR,BIT_OR}, /* or */
{"AND",		2,	2,	BINARY_OPERATOR,BIT_AND}, /* and */
{"XOR",		1,	1,	BINARY_OPERATOR,BIT_EXCL_OR}, /* exclusive or */
{"PI",		0,	0,	OPERAND,	CONST_PI}, /* pi */
{"D2R",		0,	0,	OPERAND,	CONST_D2R}, /* pi/180 */
{"R2D",		0,	0,	OPERAND,	CONST_R2D}, /* 180/pi */
{"A",		0,	0,	OPERAND,	FETCH_A}, /* fetch var A */
{"B",		0,	0,	OPERAND,	FETCH_B}, /* fetch var B */
{"C",		0,	0,	OPERAND,	FETCH_C}, /* fetch var C */
{"D",		0,	0,	OPERAND,	FETCH_D}, /* fetch var D */
{"E",		0,	0,	OPERAND,	FETCH_E}, /* fetch var E */
{"F",		0,	0,	OPERAND,	FETCH_F}, /* fetch var F */
{"G",		0,	0,	OPERAND,	FETCH_G}, /* fetch var G */
{"H",		0,	0,	OPERAND,	FETCH_H}, /* fetch var H */
{"I",		0,	0,	OPERAND,	FETCH_I}, /* fetch var I */
{"J",		0,	0,	OPERAND,	FETCH_J}, /* fetch var J */
{"K",		0,	0,	OPERAND,	FETCH_K}, /* fetch var K */
{"L",		0,	0,	OPERAND,	FETCH_L}, /* fetch var L */
{"a",		0,	0,	OPERAND,	FETCH_A}, /* fetch var A */
{"b",		0,	0,	OPERAND,	FETCH_B}, /* fetch var B */
{"c",		0,	0,	OPERAND,	FETCH_C}, /* fetch var C */
{"d",		0,	0,	OPERAND,	FETCH_D}, /* fetch var D */
{"e",		0,	0,	OPERAND,	FETCH_E}, /* fetch var E */
{"f",		0,	0,	OPERAND,	FETCH_F}, /* fetch var F */
{"g",		0,	0,	OPERAND,	FETCH_G}, /* fetch var G */
{"h",		0,	0,	OPERAND,	FETCH_H}, /* fetch var H */
{"i",		0,	0,	OPERAND,	FETCH_I}, /* fetch var I */
{"j",		0,	0,	OPERAND,	FETCH_J}, /* fetch var J */
{"k",		0,	0,	OPERAND,	FETCH_K}, /* fetch var K */
{"l",		0,	0,	OPERAND,	FETCH_L}, /* fetch var L */
{"0",		0,	0,	FLOAT_PT_CONST,	CONSTANT}, /* flt pt constant */
{"1",		0,	0,	FLOAT_PT_CONST,	CONSTANT}, /* flt pt constant */
{"2",		0,	0,	FLOAT_PT_CONST,	CONSTANT}, /* flt pt constant */
{"3",		0,	0,	FLOAT_PT_CONST,	CONSTANT}, /* flt pt constant */
{"4",		0,	0,	FLOAT_PT_CONST,	CONSTANT}, /* flt pt constant */
{"5",		0,	0,	FLOAT_PT_CONST,	CONSTANT}, /* flt pt constant */
{"6",		0,	0,	FLOAT_PT_CONST,	CONSTANT}, /* flt pt constant */
{"7",		0,	0,	FLOAT_PT_CONST,	CONSTANT}, /* flt pt constant */
{"8",		0,	0,	FLOAT_PT_CONST,	CONSTANT}, /* flt pt constant */
{"9",		0,	0,	FLOAT_PT_CONST,	CONSTANT}, /* flt pt constant */
{".",		0,	0,	FLOAT_PT_CONST,	CONSTANT}, /* flt pt constant */
{"?",		0,	0,	CONDITIONAL,	COND_IF}, /* conditional */
{":",		0,	0,	CONDITIONAL,	COND_ELSE}, /* else */
{"(",		0,	8,	UNARY_OPERATOR,	PAREN}, /* open paren */
{"^",		6,	6,	BINARY_OPERATOR,EXPON}, /* exponentiation */
{"**",		6,	6,	BINARY_OPERATOR,EXPON}, /* exponentiation */
{"+",		4,	4,	BINARY_OPERATOR,ADD}, /* addition */
#if 0
{"-",		4,	4,	BINARY_OPERATOR,SUB}, /* subtraction */
#endif
{"*",		5,	5,	BINARY_OPERATOR,MULT}, /* multiplication */
{"/",		5,	5,	BINARY_OPERATOR,DIV}, /* division */
{"%",		5,	5,	BINARY_OPERATOR,MODULO}, /* modulo */
{",",		0,	0,	SEPERATOR,	COMMA}, /* comma */
{")",		0,	0,	CLOSE_PAREN,	PAREN}, /* close paren */
{"||",		1,	1,	BINARY_OPERATOR,REL_OR}, /* or */
{"|",		1,	1,	BINARY_OPERATOR,BIT_OR}, /* or */
{"&&",		2,	2,	BINARY_OPERATOR,REL_AND}, /* and */
{"&",		2,	2,	BINARY_OPERATOR,BIT_AND}, /* and */
{">>",		2,	2,	BINARY_OPERATOR,RIGHT_SHIFT}, /* right shift */
{">=",		3,	3,	BINARY_OPERATOR,GR_OR_EQ}, /* greater or equal*/
{">",		3,	3,	BINARY_OPERATOR,GR_THAN}, /* greater than */
{"<<",		2,	2,	BINARY_OPERATOR,LEFT_SHIFT}, /* left shift */
{"<=",		3,	3,	BINARY_OPERATOR,LESS_OR_EQ},/* less or equal to*/
{"<",		3,	3,	BINARY_OPERATOR,LESS_THAN}, /* less than */
{"#",		3,	3,	BINARY_OPERATOR,NOT_EQ}, /* not equal */
{"=",		3,	3,	BINARY_OPERATOR,EQUAL}, /* equal */
{""}
};

/*
 * FIND_ELEMENT
 *
 * find the pointer to an entry in the element table
 */
static int find_element(pbuffer,pelement,pno_bytes)
 register char	*pbuffer;
 register struct expression_element	**pelement;
 register short	*pno_bytes;
 {

 	/* compare the string to each element in the element table */
 	*pelement = &elements[0];
 	while ((*pelement)->element[0])
    {
 		if (strncmp(pbuffer,(*pelement)->element,
		  strlen((*pelement)->element)) == 0){
 			*pno_bytes += strlen((*pelement)->element);
 			return(TRUE);
 		}
 		*pelement += 1;
 	}
 	return(FALSE);
 }
 
/*
 * GET_ELEMENT
 *
 * get an expression element
 */
static int get_element(pinfix,pelement,pno_bytes)
register char	*pinfix;
register struct expression_element	**pelement;
register short		*pno_bytes;
{

	/* get the next expression element from the infix expression */
	if (!*pinfix) return(END);
	*pno_bytes = 0;
	while (*pinfix == 0x20){
		*pno_bytes += 1;
		pinfix++;
	}
	if (!*pinfix) return(END);
	if (!find_element(pinfix,pelement,pno_bytes))
		return(UNKNOWN_ELEMENT);
	return(FINE);

	
}

/*
 * POSTFIX
 *
 * convert an infix expression to a postfix expression
 */
long postfix(char *pinfix,char *ppostfix, short *perror)
{
	short		no_bytes;
	register short	operand_needed;
	register short	new_expression;
	struct expression_element	stack[80];
	struct expression_element	*pelement;
	register struct expression_element	*pstacktop;
	double		constant;
	register char   *pposthold, *pc;	
	char in_stack_pri, in_coming_pri, code;
	char           *ppostfixStart = ppostfix;

	/* convert infix expression to upper case */
	for (pc=pinfix; *pc; pc++) {
		if (islower(*pc)) *pc = toupper(*pc);
	}

	/* place the expression elements into postfix */
	operand_needed = TRUE;
	new_expression = TRUE;
	*ppostfix = END_STACK;
	*perror = 0;
	if (* pinfix == 0 )
		return(0);
	pstacktop = stack;
	while (get_element(pinfix,&pelement,&no_bytes) != END){
	    pinfix += no_bytes;
	    switch (pelement->type){

	    case OPERAND:
		if (!operand_needed){
		    *perror = 5;
		    *ppostfixStart = BAD_EXPRESSION; return(-1);
		}

		/* add operand to the expression */
		*ppostfix++ = pelement->code;

		operand_needed = FALSE;
		new_expression = FALSE;
		break;

	    case FLOAT_PT_CONST:
		if (!operand_needed){
		    *perror = 5;
		    *ppostfixStart = BAD_EXPRESSION; return(-1);
		}

		/* add constant to the expression */
		*ppostfix++ = pelement->code;
		pposthold = ppostfix;

		pinfix-=no_bytes;
		while (*pinfix == ' ') *ppostfix++ = *pinfix++;
		while (TRUE) {
			if ( ( *pinfix >= '0' && *pinfix <= '9' ) || *pinfix == '.' ) {
				*ppostfix++ = *pinfix;
				pinfix++;
			} else if ( *pinfix == 'E' || *pinfix == 'e' ) {
				*ppostfix++ = *pinfix;
				pinfix++;
					if (*pinfix == '+' || *pinfix == '-' ) {
						*ppostfix++ = *pinfix;
						pinfix++;
					}
			} else break;
		}
		*ppostfix++ = '\0';

		ppostfix = pposthold;
		if ( sscanf(ppostfix,"%lg",&constant) != 1) {
			*ppostfix = '\0';
		} else {
			memcpy(ppostfix,(void *)&constant,8);
		}
		ppostfix+=8;

		operand_needed = FALSE;
		new_expression = FALSE;
		break;

	    case BINARY_OPERATOR:
		if (operand_needed){
		    *perror = 4;
		    *ppostfixStart = BAD_EXPRESSION; return(-1);
		}

		/* add operators of higher or equal priority to	*/
		/* postfix notation				*/
		while ((pstacktop >= stack+1) &&
		  (pstacktop->in_stack_pri >= pelement->in_coming_pri)) {
		    *ppostfix++ = pstacktop->code;
		    pstacktop--;
		}

		/* add new operator to stack */
		pstacktop++;
		*pstacktop = *pelement;

		operand_needed = TRUE;
		break;

	    case UNARY_OPERATOR:
		if (!operand_needed){
		    *perror = 5;
		    *ppostfixStart = BAD_EXPRESSION; return(-1);
		}

		/* add operators of higher or equal priority to	*/
		/* postfix notation 				*/
		while ((pstacktop >= stack+1) &&
		  (pstacktop->in_stack_pri >= pelement->in_coming_pri)) {
		      *ppostfix++ = pstacktop->code;
		      pstacktop--;
		 }

		/* add new operator to stack */
		pstacktop++;
		*pstacktop = *pelement;

		new_expression = FALSE;
		break;

	    case MINUS_OPERATOR:
		if (operand_needed){
			/* then assume minus was intended as a unary operator */
			in_coming_pri = UNARY_MINUS_I_C_P;
			in_stack_pri = UNARY_MINUS_I_S_P;
			code = UNARY_MINUS_CODE;
			new_expression = FALSE;
		}
		else {
			/* then assume minus was intended as a binary operator */
			in_coming_pri = BINARY_MINUS_I_C_P;
			in_stack_pri = BINARY_MINUS_I_S_P;
			code = BINARY_MINUS_CODE;
			operand_needed = TRUE;
		}

		/* add operators of higher or equal priority to	*/
		/* postfix notation				*/
		while ((pstacktop >= stack+1) &&
		  (pstacktop->in_stack_pri >= in_coming_pri)) {
		    *ppostfix++ = pstacktop->code;
		    pstacktop--;
		}

		/* add new operator to stack */
		pstacktop++;
		*pstacktop = *pelement;
		pstacktop->in_stack_pri = in_stack_pri;
		pstacktop->code = code;

		break;

	    case SEPERATOR:
		if (operand_needed){
		    *perror = 4;
		    *ppostfixStart = BAD_EXPRESSION; return(-1);
		}

		/* add operators to postfix until open paren */
		while (pstacktop->element[0] != '('){
		    if (pstacktop == stack+1 ||
		        pstacktop == stack){
			*perror = 6;
			*ppostfixStart = BAD_EXPRESSION; return(-1);
		    }
		    *ppostfix++ = pstacktop->code;
		    pstacktop--;
		}
		operand_needed = TRUE;
		break;

	    case CLOSE_PAREN:
		if (operand_needed){
		    *perror = 4;
		    *ppostfixStart = BAD_EXPRESSION; return(-1);
		}

		/* add operators to postfix until matching paren */
		while (pstacktop->element[0] != '('){
		    if (pstacktop == stack+1 ||
		        pstacktop == stack){
			*perror = 6;
			*ppostfixStart = BAD_EXPRESSION; return(-1);
		    }
		    *ppostfix++ = pstacktop->code;
		    pstacktop--;
		}
		pstacktop--;	/* remove ( from stack */
		break;

	    case CONDITIONAL:
		if (operand_needed){
		    *perror = 4;
		    *ppostfixStart = BAD_EXPRESSION; return(-1);
		}

		/* add operators of higher priority to	*/
		/* postfix notation 				*/
		while ((pstacktop >= stack+1) &&
		  (pstacktop->in_stack_pri > pelement->in_coming_pri)) {
		      *ppostfix++ = pstacktop->code;
		      pstacktop--;
		 }

		/* add new element to the postfix expression */
		*ppostfix++ = pelement->code;

		/* add : operator with COND_END code to stack */
		if (pelement->element[0] == ':'){
		     pstacktop++;
		     *pstacktop = *pelement;
		     pstacktop->code = COND_END;
		}

		operand_needed = TRUE;
		break;

	    case EXPR_TERM:
		if (operand_needed && !new_expression){
		    *perror = 4;
		    *ppostfixStart = BAD_EXPRESSION; return(-1);
		}

		/* add all operators on stack to postfix */
		while (pstacktop >= stack+1){
		    if (pstacktop->element[0] == '('){
			*perror = 6;
			*ppostfixStart = BAD_EXPRESSION; return(-1);
		    }
		    *ppostfix++ = pstacktop->code;
		    pstacktop--;
		}

		/* add new element to the postfix expression */
		*ppostfix++ = pelement->code;

		operand_needed = TRUE;
		new_expression = TRUE;
		break;


	    default:
		*perror = 8;
		*ppostfixStart = BAD_EXPRESSION; return(-1);
	    }
	}
	if (operand_needed){
		*perror = 4;
		*ppostfixStart = BAD_EXPRESSION; return(-1);
	}

	/* add all operators on stack to postfix */
	while (pstacktop >= stack+1){
	    if (pstacktop->element[0] == '('){
		*perror = 6;
		*ppostfixStart = BAD_EXPRESSION; return(-1);
	    }
	    *ppostfix++ = pstacktop->code;
	    pstacktop--;
	}
	*ppostfix = END_STACK;

	return(0);
}
/* calcPerform.c,v 1.31 1998/03/16 16:23:41 mrk Exp */
/*
 *	Author: Julie Sander and Bob Dalesio
 *	Date:	07-27-87
 *
 *	Experimental Physics and Industrial Control System (EPICS)
 *
 *	Copyright 1991, the Regents of the University of California,
 *	and the University of Chicago Board of Governors.
 *
 *	This software was produced under  U.S. Government contracts:
 *	(W-7405-ENG-36) at the Los Alamos National Laboratory,
 *	and (W-31-109-ENG-38) at Argonne National Laboratory.
 *
 *	Initial development by:
 *		The Controls and Automation Group (AT-8)
 *		Ground Test Accelerator
 *		Accelerator Technology Division
 *		Los Alamos National Laboratory
 *
 *	Co-developed with
 *		The Controls and Computing Group
 *		Accelerator Systems Division
 *		Advanced Photon Source
 *		Argonne National Laboratory
 *
 * Modification Log:
 * -----------------
 * .01	5-18-88		lrd	modified modulo and power to avoid math library
 * .02	5-19-88		lrd	modified absolute value to avoid math library
 *				defined unary math lib routines as doubles
 *				removed include math.h
 *				stopped loading dinglers math routines (ml)
 *				wrote a random number generator to return a
 *					double between 0 and 1
 * .03	12-09-88	lrd	fixed modulo not to perform zero division
 * .04	12-12-88	lrd	lock the record while processing
 * .05	12-13-88	lrd	made an alarm for math error
 * .06	12-15-88	lrd	Process the forward scan link
 * .07  12-23-88        lrd     Alarm on locked MAX_LOCKED times
 * .08	01-11-89	lrd	Add Right and Left Shift
 * .09	02-01-89	lrd	Add Trig functions
 * .10	03-14-89	lrd	fix true on C question mark operator
 * .11	03-29-89	lrd	make hardware errors MAJOR
 *				remove hw severity spec from database
 * .12	04-06-89	lrd	add monitor detection
 * .13	05-03-89	lrd	removed process mask from arg list
 * .14	06-05-89	lrd	check for negative square root
 * .15	08-01-89	lrd	full range of exponentiation using pow(x,y) 
 * .16	04-04-90	lrd	fix post events for read and calc alarms
 *				fix neg base raised to integer exponent
 * .17	04-06-90	lrd	change conditional to check for 0 and non-zero
 *				instead of 0 and 1 (more 'C' like)
 * .18	09-10-90	lrd	add time stamps
 * .19	11-26-90	lrd	add bit not and relational not - fix RNDM
 * .20	11-29-90	lrd	conditionally process soft channels
 * .21	12-14-90	lrd	fixed post events for the variables
 * .22  03-15-91	mrk	moved code from calcRecord to here
 * .23	08-01-91	rac	don't use FETCH_G ... for V2
 * .24	02-20-92	rcz	fixed for vxWorks build
 * .25	02-24-92	jba	add EXP and fix for EXPON when *pstacktop is 0
 * .26	02-28-92	jba	added CEIL and FLOOR
 * .27  03-06-92        jba     added MAX and MIN binary functions
 * .28  03-10-92        jba     added multiple conditional expressions ?
 * .29  04-01-92        jba     allowed floating pt constants in expression
 * .30  05-01-92        jba     flt pt constant string replaced with double in postfix
 * .31  08-21-92        jba     ANSI c changes
 * .32  12-11-92	mrk	Removed include for stdioLib.h
 * .33  08-04-93	mgb	Removed V5/V4 and EPICS_V2 conditionals
 * .34  05-11-94	jba	Added support for CONST_PI, CONST_D2R, and CONST_R2D
 * .34  08-18-94	jba	Must skip over constant when looking for COND_END,COND_ELSE
 * .35	10-07-94	mda	change local random() to local_random() to
 *				avoid colliding with math library random()

  $Log%

 */

/* This module contains the code for processing the arithmetic
 * expressions defined in calculation records. postfix must be called
 * to convert a valid infix expression to postfix. CalcPerform
 * calculates the postfix expression.
 *
 * Subroutines
 *
 *	Public
 *
 * calcPerform		perform the calculation
 *	    args
 *		double *pargs	address of arguments (12)
 *		double *presult address of result
 *		char   *rpcl	address of reverse polish buffer
 *	    returns
 *		0		fetched successfully
 *		-1		fetch failed
 *
 * Private routine for calcPerform
 *	local_random		random number generator
 *	    returns
 *		double value between 0.00 and 1.00
 */

#define	NOT_SET		0
#define	TRUE_COND	1
#define	FALSE_COND	2

#ifndef PI
#define PI 3.141592654
#endif


/*
 * RAND
 *
 * generates a random number between 0 and 1 using the
 * seed = (multy * seed) + addy         Random Number Generator by Knuth
 *                                              SemiNumerical Algorithms
 *                                              Chapter 1
 * randy = seed / 65535.0          To normalize the number between 0 - 1
 */
static unsigned short seed = 0xa3bf;
static unsigned short multy = 191 * 8 + 5;  /* 191 % 8 == 5 */
static unsigned short addy = 0x3141;
static double local_random()
{
        double  randy;

        /* random number */
        seed = (seed * multy) + addy;
        randy = (float) seed / 65535.0;

        /* between 0 - 1 */
        return(randy);
}

long calcPerform(const double *parg, double *presult, const char *post)
{
	double *pstacktop;	/* stack of values	*/
	double		stack[80];
	short		temp1;
	short	i;
	double 		*top;
	int 		itop;		/* integer top value	*/
	int 		inexttop;	/* ineteger next to top value 	*/
	short 		cond_flag;	/* conditional else flag	*/
	short 		got_if;

	/* initialize flag  */
	cond_flag = NOT_SET;
	pstacktop = &stack[0];

/* DEBUG print statements
for (i=0;i<184;i++){
printf ("%d_",post[i]);
if ( post[i] == END_STACK ) break;
if ( post[i] == 71 ) i=i+8;
}
printf ("*FINISHED*\n");
*/
        if(*post == BAD_EXPRESSION) return(-1);

	/* set post to postfix expression in calc structure */
	top = pstacktop;

	/* polish calculator loop */
	while (*post != END_STACK){

		switch (*post){
		case FETCH_A:
			++pstacktop;
			*pstacktop = parg[0];
			break;

		case FETCH_B:
			++pstacktop;
			*pstacktop = parg[1];
			break;

		case FETCH_C:
			++pstacktop;
			*pstacktop = parg[2];
			break;

		case FETCH_D:
			++pstacktop;
			*pstacktop = parg[3];
			break;

		case FETCH_E:
			++pstacktop;
			*pstacktop = parg[4];
			break;

		case FETCH_F:
			++pstacktop;
			*pstacktop = parg[5];
			break;

		case FETCH_G:
			++pstacktop;
			*pstacktop = parg[6];
			break;

		case FETCH_H:
			++pstacktop;
			*pstacktop = parg[7];
			break;

		case FETCH_I:
			++pstacktop;
			*pstacktop = parg[8];
			break;

		case FETCH_J:
			++pstacktop;
			*pstacktop = parg[9];
			break;

		case FETCH_K:
			++pstacktop;
			*pstacktop = parg[10];
			break;

		case FETCH_L:
			++pstacktop;
			*pstacktop = parg[11];
			break;

		case CONST_PI:
			++pstacktop;
			*pstacktop = PI;
			break;

		case CONST_D2R:
			++pstacktop;
			*pstacktop = PI/180.;
			break;

		case CONST_R2D:
			++pstacktop;
			*pstacktop = 180./PI;
			break;

		case ADD:
			--pstacktop;
			*pstacktop = *pstacktop + *(pstacktop+1);
			break;

		case SUB:
			--pstacktop;
			*pstacktop = *pstacktop - *(pstacktop+1);
			break;

		case MULT:
			--pstacktop;
			*pstacktop = *pstacktop * *(pstacktop+1);
			break;

		case DIV:
			--pstacktop;
			if (*(pstacktop+1) == 0) /* can't divide by zero */
				return(-1);
			*pstacktop = *pstacktop / *(pstacktop+1);
			break;

		case COND_IF:
			/* if false condition then skip true expression */
			if (*pstacktop == 0.0) {
				/* skip to matching COND_ELSE */
				for (got_if=1; got_if>0 && *(post+1) != END_STACK; ++post) {
					if (*(post+1) == CONSTANT  ) post+=8;
					else if (*(post+1) == COND_IF  ) got_if++;
					else if (*(post+1) == COND_ELSE) got_if--;
				}
			}
			/* remove condition from stack top */
			--pstacktop;
			break;
				
		case COND_ELSE:
			/* result, true condition is on stack so skip false condition  */
			/* skip to matching COND_END */
			for (got_if=1; got_if>0 && *(post+1) != END_STACK; ++post) {
				if (*(post+1) == CONSTANT  ) post+=8;
				else if (*(post+1) == COND_IF ) got_if++;
				else if (*(post+1) == COND_END) got_if--;
			}
			break;

		case COND_END:
			break;

		case ABS_VAL:
			if (*pstacktop < 0) *pstacktop = -*pstacktop;
			break;

		case UNARY_NEG:
			*pstacktop = -1* (*pstacktop);
			break;

		case SQU_RT:
			if (*pstacktop < 0) return(-1);	/* undefined */
			*pstacktop = sqrt(*pstacktop);
			break;

		case EXP:
			*pstacktop = exp(*pstacktop);
			break;

		case LOG_10:
			*pstacktop = log10(*pstacktop);
			break;

		case LOG_E:
			*pstacktop = log(*pstacktop);
			break;

		case RANDOM:
			++pstacktop;
			*pstacktop = local_random();
			break;

		case EXPON:
			--pstacktop;
			if (*pstacktop == 0) break;
			if (*pstacktop < 0){
				temp1 = (int) *(pstacktop+1);
				/* is exponent an integer */
				if ((*(pstacktop+1) - (double)temp1) != 0) return (-1);
        			*pstacktop = exp(*(pstacktop+1) * log(-*pstacktop));
				/* is value negative */
				if ((temp1 % 2) > 0) *pstacktop = -*pstacktop;
			}else{
        			*pstacktop = exp(*(pstacktop+1) * log(*pstacktop));
			}
			break;

		case MODULO:
			--pstacktop;
			itop = (int)*pstacktop;
			inexttop = (int)*(pstacktop+1);
			if (inexttop == 0)
				return(-1);
			i =  itop % inexttop;
			*pstacktop = i;
			break;

		case REL_OR:
			--pstacktop;
			*pstacktop = (*pstacktop || *(pstacktop+1));
			break;

		case REL_AND:
			--pstacktop;
			*pstacktop = (*pstacktop && *(pstacktop+1));
			break;

		case BIT_OR:
			/* force double values into integers and or them */
			itop = (int)*pstacktop;
			inexttop = (int)*(pstacktop-1);
			--pstacktop;
			*pstacktop = (inexttop | itop);
			break;

		case BIT_AND:
			/* force double values into integers and and them */
			itop = (int)*pstacktop;
			inexttop = (int)*(pstacktop-1);
			--pstacktop;
			*pstacktop = (inexttop & itop);
			break;

		case BIT_EXCL_OR:
			/*force double values to integers to exclusive or them*/
			itop = (int)*pstacktop;
			inexttop = (int)*(pstacktop-1);
			--pstacktop;
			*pstacktop = (inexttop ^ itop);
			break;

		case GR_OR_EQ:
			--pstacktop;
			*pstacktop = *pstacktop >= *(pstacktop+1);
			break;

		case GR_THAN:
			--pstacktop;
			*pstacktop = *pstacktop > *(pstacktop+1);
			break;

		case LESS_OR_EQ:
			--pstacktop;
			*pstacktop = *pstacktop <= *(pstacktop+1);
			break;

		case LESS_THAN:
			--pstacktop;
			*pstacktop = *pstacktop < *(pstacktop+1);
			break;

		case NOT_EQ:
			--pstacktop;
			*pstacktop = *pstacktop != *(pstacktop+1);
			break;

		case EQUAL:
			--pstacktop;
			*pstacktop = (*pstacktop == *(pstacktop+1));
			break;

		case RIGHT_SHIFT:
			itop = (int)*pstacktop;
			inexttop = (int)*(pstacktop-1);
			--pstacktop;
			*pstacktop = (inexttop >> itop);
			break;

		case LEFT_SHIFT:
			itop = (int)*pstacktop;
			inexttop = (int)*(pstacktop-1);
			--pstacktop;
			*pstacktop = (inexttop << itop);
			break;

                case MAX:
                        --pstacktop;
                        if (*pstacktop < *(pstacktop+1))
                                *pstacktop = *(pstacktop+1);
                        break;
 
                case MIN:
                        --pstacktop;
                        if (*pstacktop > *(pstacktop+1))
                                *pstacktop = *(pstacktop+1);
                        break;
 

		case ACOS:
			*pstacktop = acos(*pstacktop);
			break;

		case ASIN:
			*pstacktop = asin(*pstacktop);
			break;

		case ATAN:
			*pstacktop = atan(*pstacktop);
			break;

		case ATAN2:
			--pstacktop;
			*pstacktop = atan2(*(pstacktop+1), *pstacktop);
			break;

		case COS:
			*pstacktop = cos(*pstacktop);
			break;

		case SIN:
			*pstacktop = sin(*pstacktop);
			break;

		case TAN:
			*pstacktop = tan(*pstacktop);
			break;

		case COSH:
			*pstacktop = cosh(*pstacktop);
			break;

		case SINH:
			*pstacktop = sinh(*pstacktop);
			break;

		case TANH:
			*pstacktop = tanh(*pstacktop);
			break;

		case CEIL:
			*pstacktop = ceil(*pstacktop);
			break;

		case FLOOR:
			*pstacktop = floor(*pstacktop);
			break;

		case NINT:
			*pstacktop = (double)(long)((*pstacktop) >= 0 ? (*pstacktop)+0.5 : (*pstacktop)-0.5);
			break;

		case REL_NOT:
			*pstacktop = ((*pstacktop)?0:1);
			break;

		case BIT_NOT:
			itop = (int)*pstacktop;
			*pstacktop = ~itop;
			break;

		case CONSTANT:
			++pstacktop;
			++post;
			if ( post == NULL ) {
				++post;
				printf("%.7s bad constant in expression\n",post);
				break;
			}
			memcpy((void *)pstacktop,post,8);
			post+=7;
			break;
		default:
			printf("%d bad expression element\n",*post);
			break;
		}

		/* move ahead in postfix expression */
		++post;
	}

	/* if everything is peachy,the stack should end at its first position */
	if (++top == pstacktop)
		*presult = *pstacktop;
	else
		return(-1);
	return(0);
}
