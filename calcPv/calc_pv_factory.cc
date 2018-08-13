// -*- c++ -*-
//
// calc_pv_factory.cc
//
// kasemir@lanl.gov

// new version 8/23/2011 sinclairjw@ornl.gov

#include<sys/time.h>
#include<unistd.h>   
#include<stdio.h>
#include<stdlib.h>
#include<float.h>
#include<math.h>
#include "epicsAlarmLike.h"
#include "environment.str"
#include"calc_pv_factory.h"

static PV_Factory *calc_pv_factory = new CALC_PV_Factory();

static const int NO_EXPR_FOUND = 0;
static const int IS_EXPR = 1;
static const int SYNTAX_ERROR = -1;

static const char *whitespace = " \t\n\r";
#define CALC_PV_HUGE_VAL 1e50

#undef CALC_DEBUG
//#define CALC_DEBUG 1

#define DONE -1
#define SIGN_OR_NUM 1
#define NUM 2
#define SIGN_OR_NUM2 3
#define NUM1 4
#define SIGN_OR_POINT_OR_NUM 5
#define POINT_OR_NUM 6
#define EXP_OR_POINT_OR_NUM 7
#define EXP_OR_NUM 8
#define NUM2 9

static void trim (
  char *str )
{

// trim white space

int first, last, i, ii, l;

  l = strlen(str);

  i = ii = 0;

  while ( ( i < l ) && isspace( str[i] ) ) {
    i++;
  }

  first = i;

  i = l-1;
  while ( ( i >= first ) && isspace( str[i] ) ) {
    i--;
  }

  last = i;

  if ( first > 0 ) {

    for ( i=first; i<=last; i++ ) {
      str[ii] = str[i];
      ii++;
    }

    str[ii] = 0;

  }
  else if ( last < l-1 ) {

    str[last+1] = 0;

  }

}

static int legalFloat (
  const char *str )
{

char buf[127+1];
int i, l, legal, state;

  strncpy( buf, str, 127 );
  buf[127] = 0;
  trim( buf );
  l = strlen(buf);
  if ( l < 1 ) return 0;

  state = SIGN_OR_POINT_OR_NUM;
  i = 0;
  legal = 1;
  while ( state != DONE ) {

    if ( i >= l ) state = DONE;

    switch ( state ) {

    case SIGN_OR_POINT_OR_NUM:

      if ( buf[i] == '-' ) {
        i++;
        state = POINT_OR_NUM;
        continue;
      }
        
      if ( buf[i] == '+' ) {
        i++;
        state = POINT_OR_NUM;
        continue;
      }
        
      if ( buf[i] == '.' ) {
        i++;
        state = NUM1;
        continue;
      }
        
      if ( isdigit(buf[i]) ) {
        i++;
        state = EXP_OR_POINT_OR_NUM;
        continue;
      }

      legal = 0;
      state = DONE;

      break;        

    case NUM1:

      if ( isdigit(buf[i]) ) {
        i++;
        state = EXP_OR_NUM;
        continue;
      }

      legal = 0;
      state = DONE;

      break;        

    case EXP_OR_POINT_OR_NUM:

      if ( buf[i] == 'E' ) {
        i++;
        state = SIGN_OR_NUM2;
        continue;
      }
        
      if ( buf[i] == 'e' ) {
        i++;
        state = SIGN_OR_NUM2;
        continue;
      }
        
      if ( buf[i] == '.' ) {
        i++;
        state = EXP_OR_NUM;
        continue;
      }
        
      if ( isdigit(buf[i]) ) {
        i++;
        continue;
      }

      legal = 0;
      state = DONE;

      break;        

    case POINT_OR_NUM:

      if ( buf[i] == '.' ) {
        i++;
        state = EXP_OR_NUM;
        continue;
      }
        
      if ( isdigit(buf[i]) ) {
        i++;
        state = EXP_OR_POINT_OR_NUM;
        continue;
      }

      legal = 0;
      state = DONE;

      break;        

    case EXP_OR_NUM:

      if ( buf[i] == 'E' ) {
        i++;
        state = SIGN_OR_NUM2;
        continue;
      }
        
      if ( buf[i] == 'e' ) {
        i++;
        state = SIGN_OR_NUM2;
        continue;
      }
        
      if ( isdigit(buf[i]) ) {
        i++;
        continue;
      }

      legal = 0;
      state = DONE;

      break;        

    case SIGN_OR_NUM2:

      if ( buf[i] == '-' ) {
        i++;
        state = NUM2;
        continue;
      }
        
      if ( buf[i] == '+' ) {
        i++;
        state = NUM2;
        continue;
      }
        
      if ( isdigit(buf[i]) ) {
        i++;
        state = NUM2;
        continue;
      }

      legal = 0;
      state = DONE;

      break;        

    case NUM2:

      if ( isdigit(buf[i]) ) {
        i++;
        continue;
      }

      legal = 0;
      state = DONE;

      break;        

    }

  }

  return legal;

}

static int checkFormat (
  char *expression,
  int *name0,
  int *name1,
  int *expr0,
  int *expr1
) {

static const int NO_EXPR_FOUND = 0;
static const int IS_EXPR = 1;
static const int SYNTAX_ERROR = -1;

static const int FINDING_FIRST_NON_WHITE = 1;
static const int FINDING_NAME_OR_EMBEDDED_EXP = 2;
static const int FINDING_EMBEDDED_EXP_START = 3;
static const int FINDING_EMBEDDED_EXP_END = 4;
static const int FINDING_NAME_END = 5;
static const int FINDING_NAME_AND_EMBEDDED_EXP_END = 6;
static const int ALL_DONE = -1;

int l = strlen(expression);
int i = 0;
int state = FINDING_FIRST_NON_WHITE;
int status = NO_EXPR_FOUND;

  *name0 = 0;
  *name1 = 0;
  *expr0 = 0;
  *expr1 = 0;

  if ( l == 0 ) return NO_EXPR_FOUND;

  while ( state != ALL_DONE ) {

    //fprintf( stderr, "i=%-d c=%c s=%-d\n", i,
    // (expression[i]?expression[i]:'?'), state );

    switch ( state ) {

    case FINDING_FIRST_NON_WHITE:

      if ( i >= l ) {
        status = NO_EXPR_FOUND;
	state = ALL_DONE;
      }
      else if ( ( expression[i] != ' ' ) && ( expression[i] != '\t' ) ) {
        i--;
        state = FINDING_NAME_OR_EMBEDDED_EXP;
      }

      break;

    case FINDING_NAME_OR_EMBEDDED_EXP:

      if ( i >= l ) {
        status = NO_EXPR_FOUND;
	state = ALL_DONE;
      }
      else if ( expression[i] == '{' ) {
        *name0 = i;
        *expr0 = i + 1;
        state = FINDING_NAME_AND_EMBEDDED_EXP_END;
      }
      else if (
                ( expression[i] == '_' ) ||
                ( ( expression[i] >= 'A' ) && ( expression[i] <= 'Z' ) ) ||
                ( ( expression[i] >= 'a' ) && ( expression[i] <= 'z' ) )
	      ) {
        *name0 = i;
        state = FINDING_NAME_END;
      }

      break;

    case FINDING_EMBEDDED_EXP_START:

      if ( i >= l ) {
        status = SYNTAX_ERROR;
	state = ALL_DONE;
      }
      else if ( expression[i] == '{' ) {
        *expr0 = i + 1;
        state = FINDING_EMBEDDED_EXP_END;
      }

      break;

    case FINDING_EMBEDDED_EXP_END:

      if ( i >= l ) {
        status = SYNTAX_ERROR;
	state = ALL_DONE;
      }
      else if ( expression[i] == '}' ) {
        *expr1 = i - 1;
        if ( expression[i-1] == '{' ) {
          status = SYNTAX_ERROR;
          state = ALL_DONE;
        }
	else {
          status = IS_EXPR;
          state = ALL_DONE;
        }
      }

      break;

    case FINDING_NAME_END:

      if ( i >= l ) {
        *name1 = l - 1;
        status = NO_EXPR_FOUND;
	state = ALL_DONE;
      }
      else if ( expression[i] == '=' ) {
        *name1 = i - 1;
        state = FINDING_EMBEDDED_EXP_START;
      }
      else if ( expression[i] == '}' ) {
        status = SYNTAX_ERROR;
	state = ALL_DONE;
      }
      else if ( expression[i] == '{' ) {
        status = SYNTAX_ERROR;
	state = ALL_DONE;
      }
      else if (
                ( expression[i] != '_' ) &&
                ( expression[i] != '-' ) &&
                ( expression[i] != ':' ) &&
                ( expression[i] != '(' ) &&
                ( expression[i] != ')' ) &&
                ( expression[i] != '[' ) &&
                ( expression[i] != ']' ) &&
                ( ( expression[i] < 'A' ) || ( expression[i] > 'Z' ) ) &&
                ( ( expression[i] < 'a' ) || ( expression[i] > 'z' ) ) &&
                ( ( expression[i] < '0' ) || ( expression[i] > '9' ) )
	      ) {
        status = SYNTAX_ERROR;
	state = ALL_DONE;
      }

      break;

    case FINDING_NAME_AND_EMBEDDED_EXP_END:

      if ( i >= l ) {
        status = SYNTAX_ERROR;
	state = ALL_DONE;
      }
      else if ( expression[i] == '}' ) {
        *name1 = i;
        *expr1 = i - 1;
        if ( expression[i-1] == '{' ) {
          status = SYNTAX_ERROR;
          state = ALL_DONE;
        }
	else {
          status = IS_EXPR;
          state = ALL_DONE;
        }
      }

      break;
    }

    i++;

  }

  if ( *name1 < *name0 ) status = SYNTAX_ERROR;
  if ( *expr1 < *expr0 ) status = SYNTAX_ERROR;

  //fprintf( stderr, "status = %-d\n", status );

  return status;

}

// HashedCalcPvList:
// Used to delay connection to calc pv until corresponding formula
// is loaded (at least one calc pv must include the formula or it
// must have been defined in the calculation list file.

// HashedExpression:
// One formula, hashed by name, read from the config file
// and converted into postfix notation.
// Can calc. result when fed arguments.
//
// CALC_PV_Factor:
// Called by main PV_Factory for "CALC\..." ProcessVariables.
// Reads the config file.
// When create("sum(fred, freddy)") is called,
// it looks for the formula "sum"
// and creates a CALC_ProcessVariable for sum with fred & freddy
// as arguments
//
// CALC_ProcessVariable
// Subscribes to arguments, recalculates value of formula
// whenever arguments change.
// If there are no arguments (-> no incoming events),
// timer is used to generate value events.

// ------------------------------------------------------------------
// HashedCalcPvList -------------------------------------------------
// ------------------------------------------------------------------
HashedCalcPvList::HashedCalcPvList()
{ pv = NULL; }

HashedCalcPvList::HashedCalcPvList(
  CALC_ProcessVariable *onePv,
  char *oneName )
{
  pv = onePv;
  name = strdup( oneName );
  needComplete = true;
}

HashedCalcPvList::~HashedCalcPvList()
{
  pv = NULL;
  free( name );
  name = NULL;
}

// Required for Hashtable<>:
size_t hash(const HashedCalcPvList *item, size_t N)
{
  return generic_string_hash( (const char *) item->pv, N );
}

int equals(const HashedCalcPvList *lhs, const HashedCalcPvList *rhs)
{
  if ( (unsigned long ) lhs->pv < (unsigned long ) rhs->pv ) {
    return 1;
  }
  else if ( (unsigned long ) lhs->pv > (unsigned long ) rhs->pv ) {
    return -1;
  }
  return 0;
}

// ------------------------------------------------------------------
// HashedExpression -------------------------------------------------
// ------------------------------------------------------------------
HashedExpression::HashedExpression () {

  name = NULL;
  formula = NULL;

}

HashedExpression::HashedExpression(const char *_name, char *_formula,
 char *rewriteString=NULL )
{

int st;
short error;

  name = strdup(_name);
  if ( rewriteString ) {
    expStr.setRaw( rewriteString );
  }
  formula = strdup(_formula);
  st = edm_postfix(_formula, this->compiled, &error);
  if ( st != 0) {
    fprintf(stderr, "CALC '%s': error in expression '%s'\n",
     name, _formula);
  }

}

int HashedExpression::setFormula (
  char *oneFormula
) {

int st;
short error;

  if ( oneFormula ) {
    if ( strcmp( oneFormula, this->formula ) == 0 ) {
      return 0;
    }
  }

  st = edm_postfix( oneFormula, this->compiled, &error );
  if ( st != 0) {
    fprintf(stderr, "CALC '%s': error in expression '%s'\n  Formula not updated\n",
     this->name, oneFormula);
    // restore previous
    if ( this->formula ) {
      st = edm_postfix( this->formula, this->compiled, &error );
    }
  }
  else {
#ifdef CALC_DEBUG
    fprintf( stderr, "CALC, formula updated for %s\n  Old: [%s]\n  New: [%s]\n",
     this->name, this->formula, oneFormula );
#endif
    free( this->formula );
    formula = strdup(oneFormula);
  }

  return st;

}

HashedExpression::~HashedExpression()
{

    if (name)
    {
        free(name);
        name = NULL;
    }
    if (formula)
    {
        free(formula);
        formula = NULL;
    }

}

bool HashedExpression::calc(const double args[], double &result)
{   
  return edm_calcPerform((double *) args, &result, compiled) == 0;
}

// Required for Hashtable<>:
size_t hash(const HashedExpression *item, size_t N)
{
  return generic_string_hash(item->name, N);
}

bool equals(const HashedExpression *lhs, const HashedExpression *rhs)
{
  return strcmp(lhs->name, rhs->name)==0;
}

// ------------------------------------------------------------------
// CALC_PV_Factory --------------------------------------------------
// ------------------------------------------------------------------
//enum { HashTableSize = 43 };
enum { HashTableSize = 503 };
typedef Hashtable<HashedExpression,
                  offsetof(HashedExpression, node),
                  HashTableSize> ExpressionHash;
static ExpressionHash *expressions = NULL;
typedef Hashtable<HashedCalcPvList,
                  offsetof(HashedCalcPvList, node),
                  HashTableSize> CalcPvListHash;
static CalcPvListHash *calcPvList = NULL;

static void checkForEmbedded (
  char *expression
) {

int size, l, result, name0, name1, expr0, expr1;
char *outer, *pvname, inner[MAX_INFIX_SIZE+1];
bool needReeval;

  if ( !expression ) {
    return;
  }

  //fprintf( stderr, "checkForEmbedded - expression = [%s]\n", expression );

  l = strlen( expression );

  if ( l > MAX_INFIX_SIZE-1 ) return;

  if ( l > 2 ) {

    result = checkFormat( expression, &name0, &name1, &expr0, &expr1 );

    if ( result == SYNTAX_ERROR ) {
      fprintf(stderr, "CALC: syntax error in expression '%s'\n",
       expression );
      return;
    }

    if ( result == IS_EXPR ) {

      HashedExpression he;
      size = name1 - name0 + 1;
      outer = new char[size+1];
      strncpy( outer, &expression[name0], size );
      outer[size] = 0;
      // 'he' will delete the new'ed  expression
      he.name = outer;
      ExpressionHash::iterator entry = expressions->find(&he);
      if (entry == expressions->end()) {

        needReeval = true;

        size = name1 - name0 + 1;
        pvname = new char[size+1];
        strncpy( pvname, &expression[name0], size );
        pvname[size] = 0;

        size = expr1 - expr0 + 1;
        if ( size > MAX_INFIX_SIZE ) size = MAX_INFIX_SIZE;
        strncpy( inner, &expression[expr0], MAX_INFIX_SIZE );
        inner[size] = 0;

        strcpy( expression, pvname );

        expressions->insert( new HashedExpression( pvname, inner ) );

        delete pvname;
        pvname = 0;

      }
      else {

	// update current formula

        needReeval = true;

        size = name1 - name0 + 1;
        pvname = new char[size+1];
        strncpy( pvname, &expression[name0], size );
        pvname[size] = 0;

        size = expr1 - expr0 + 1;
        if ( size > MAX_INFIX_SIZE ) size = MAX_INFIX_SIZE;
        strncpy( inner, &expression[expr0], MAX_INFIX_SIZE );
        inner[size] = 0;

        strcpy( expression, pvname );

        HashedExpression *heToUpdate = *entry;

        heToUpdate->setFormula( inner );

        delete pvname;
        pvname = 0;

      }

    }

    if ( needReeval ) {

      CalcPvListHash::iterator ci;
      bool remove;
      ci=calcPvList->begin();
      while (ci!=calcPvList->end()) {

        HashedCalcPvList *ce = *ci;

        remove = false;

        ExpressionHash::iterator i;
        i=expressions->begin();
        while (i!=expressions->end())
        {
            HashedExpression *e = *i;

            if ( strcmp( ce->name, e->name ) == 0 ) {
              ce->pv->completeCreation( e );
              remove = true;
              break;
            }

            ++i;
        }

        if ( remove ) {
          calcPvList->erase( ci );
          delete ce;
          ci = calcPvList->begin();
        }
        else {
          ++ci;
        }

      }

    }

  }

}

CALC_PV_Factory::CALC_PV_Factory()
{
#   ifdef CALC_DEBUG
    fprintf( stderr,"CALC_PV_Factory created\n");
#   endif
    if (expressions)
    {
        fprintf(stderr, "Error: More than one CALC_PV_Factory created!\n");
        return;
    }

    processInvalid = false;
    invalidResult = 0.0;
    
    expressions = new ExpressionHash();
    
    if (calcPvList)
    {
        fprintf(stderr, "Error: More than one CALC_PV_Factory created!\n");
        return;
    }

    calcPvList = new CalcPvListHash();
    
    char *env = getenv(environment_str153);
    if (env)
    {
        env = strdup(env);
        char *start = env;
        while(start && *start)
        {
            char *end = strchr(start, ':');
            if (end)
                *end = '\0';
            parseFile(start);
            if (end)
                start = end+1;
            else
                start = 0;
        }
        free(env);
    }
    else
    {
      {
        const char *path=getenv("EDMFILES");
        if (path) {
          char *name =
           (char *) malloc(strlen(path)+strlen(CALC_FILENAME)+2);
          if (name) {
            sprintf(name, "%s/%s", path, CALC_FILENAME);
            int result = parseFile(name);
            free(name);
            if ( !result ) {
              parseFile(CALC_FILENAME);
            }
          }
        }
        else {
          parseFile(CALC_FILENAME);
        }
      }
    }
}

CALC_PV_Factory::~CALC_PV_Factory()
{
    if (expressions)
    {
        ExpressionHash::iterator i;
        i=expressions->begin();
        while (i!=expressions->end())
        {
            HashedExpression *e = *i;
            expressions->erase(i);
            delete e;
            i=expressions->begin();
        }
        delete expressions;
        expressions = 0;
    }

    if ( calcPvList ) {
      CalcPvListHash::iterator ci;
      ci = calcPvList->begin();
      while ( ci !=calcPvList->end() ) {
	HashedCalcPvList *ce = *ci;
        calcPvList->erase( ci );
        delete ce;
        ci = calcPvList->begin();
      }
      delete calcPvList;
      calcPvList = 0;
    }

#   ifdef CALC_DEBUG
    fprintf( stderr,"CALC_PV_Factory deleted\n");
#   endif
}

int CALC_PV_Factory::handleProcessInvalidOptions (
  const char *line
) {

  char *tok, *ctx, buf[255+1];

  strncpy( buf, line, 255 );
  buf[255] = 0;

  ctx = NULL;
  tok = strtok_r( buf, "= \t\n", &ctx );

  if ( tok ) {

    if ( strcasecmp( tok, "processinvalid" ) == 0 ) {

      processInvalid = true;
      return 1; // yes, this was a process invalid option

    }
    else if ( strcasecmp( tok, "invalidresult" ) == 0 ) {

      tok = strtok_r( NULL, "= \t\n", &ctx );
      if ( tok ) {
        invalidResult = strtod( tok, NULL );
        return 1; // yes, this was a process invalid option
      }

    }

  }

  return 0; // no, this was not a process invalid option

}

bool CALC_PV_Factory::parseFile(const char *filename)
{
    char line[1024], name[PV_Factory::MAX_PV_NAME+1],
     newArgList[PV_Factory::MAX_PV_NAME+1];
    size_t len;
    int result;

#   ifdef CALC_DEBUG
    fprintf( stderr,"CALC_PV_Factory::parseFile '%s'\n", filename);
#   endif
    FILE *f = fopen(filename, "rt");
    if (!f)
    {
#       ifdef CALC_DEBUG
        fprintf( stderr,"CALC_PV_Factory::parseFile failed\n");
#       endif
        return false;
    }

    // Check version code
    if (!fgets(line, sizeof line, f)  ||
        strncmp(line, "CALC1", 5))
    {
        fprintf(stderr, "Invalid CALC configuration file '%s'\n",
                filename);
        fclose(f);
        return false;
    }

    // Loop over lines
    bool need_name = true;
    char *p;
    while (fgets(line, sizeof line, f))
    {
        len = strlen(line);
        if (len <= 0 || (len > 0 && line[0] == '#')) // skip comments
            continue;
        // Remove trailing white space
        while (len>0 && strchr(whitespace, line[len-1]))
        {
            line[len-1] = '\0';
            --len;
        }
        if (len <= 0)
            continue;
        // Skip leading white space
        p=line;
        while (strchr(whitespace, *p))
        {
            ++p;
            --len;
        }

        if ( *p == '#' ) { // skip comments
          continue;
        }

        result = handleProcessInvalidOptions( p );
        if ( result ) { // line was and process-invalid option
          continue;
        }
        
        if (need_name)
        {

            strncpy( name, p, PV_Factory::MAX_PV_NAME );
            name[PV_Factory::MAX_PV_NAME] = 0;
            strcpy( newArgList, "" );
            need_name = false;

        }
        else
        {

	  if ( p[0] == '@' ) {

            strncpy( newArgList, &p[1], PV_Factory::MAX_PV_NAME );
            newArgList[PV_Factory::MAX_PV_NAME] = 0;

	  }
	  else {

            need_name = true;
#           ifdef CALC_DEBUG
            fprintf( stderr,"CALC_PV_Factory::parseFile expr '%s' <-> '%s'\n",
                   name, p);
#           endif

	    if ( strcmp( newArgList, "" ) ) {

              expressions->insert(new HashedExpression(name, p,
               newArgList));

	    }
	    else {

              expressions->insert(new HashedExpression(name, p));

	    }

	  }

        }
    }
    fclose(f);
    return true;
}

// Fills *arg with copy of next argument found,
// returns 0 or updated position p
static const char *get_arg(const char *p, char **arg)
{
    // find start...
    while (*p && strchr(", \t)", *p))
        ++p;
    if (! *p)
        return 0;

    // find end, including spaces...
    const char *end = p+1;
    int braces = 0;
    while (*end)
    {
        if (*end == '(')
            ++braces;
        else if (*end == ')')
        {
            --braces;
            if (braces < 0)
                break;
        }
        else if (*end == ',')
        {
            if (braces==0)
                break;
        }
        ++end;
    }
    // end is on character NOT to copy: '\0'  ','  ')'
    // remove trailing space
    while (end > p  && strchr(whitespace, *(end-1)))
        --end;
    // copy
    int len = end - p;
    if (len <= 0)
        return 0;
    // Compiler created trash when using *arg instead of temp. narg
    char *narg = (char *)malloc(len+1);
    memcpy(narg, p, len);
    narg[len] = '\0';
    *arg = narg;

    return end;
}

ProcessVariable *CALC_PV_Factory::create(const char *PV_name)
{
    char *arg_name[CALC_ProcessVariable::MaxArgs];
    char *exp_arg_name[CALC_ProcessVariable::MaxArgs];
    size_t i, ii, l, arg_count = 0;
    const char *p;
    char *p1, *start;
    int more, findingInline = 0, gettingFirst;
    CALC_ProcessVariable *pv;
    char dummy[2];
    bool haveValidExpression = false;

    strcpy( dummy, "" );
    start = dummy;

    for (i=0; i<CALC_ProcessVariable::MaxArgs; ++i)
        arg_name[i] = 0;
    
    // Locate expression: start...
    while (strchr(whitespace, *PV_name))
        ++PV_name;
    p = PV_name;
    // end...
    more = 1;
    //fprintf( stderr, "*p = [%s]\n", p );
    if ( *p == '(' ) more = 0;
    while (*p && more) {
        if ( *p == '{' ) findingInline++;
        if ( ( *p == '}' ) && findingInline ) findingInline--;
        ++p;
        if ( ( *p == '(' ) && !findingInline ) more = 0;
	//fprintf( stderr, "%c, findingInline = %-d, more = %-d\n",
        // (char) *p, findingInline, more );
    }
    // copy
    int len = p - PV_name; 
    if (len <= 0)
    {
        fprintf(stderr, "Empty expression '%s'\n", PV_name);
        return 0;
    }
    char *expression = (char *)malloc(len+1);
    memcpy(expression, PV_name, len);
    expression[len] = '\0';
    
    while (*p && strchr(whitespace, *p))
        ++p;
    // Do arguments follow?
    if (*p == '(')
    {
        ++p;
        while ((p=get_arg(p, &arg_name[arg_count])) != 0)
            ++arg_count;
    }

#   ifdef CALC_DEBUG
    fprintf( stderr,"CALC_PV_Factory::create: '%s'\n", PV_name);
    fprintf( stderr,"\texpression: '%s'\n", expression);
    for (i=0; i<arg_count; ++i)
        fprintf( stderr,"\targ %d: '%s'\n", i, arg_name[i]);
#   endif    
    checkForEmbedded( expression );
    HashedExpression he;
    he.name = expression;
    ExpressionHash::iterator entry = expressions->find(&he);
    if (entry == expressions->end())
    {
      haveValidExpression = false;
      //fprintf( stderr, "no valid expression  yet\n" );
      //fprintf( stderr, "Unknown CALC expression '%s'\n", expression );
      //return 0;
    }
    else {
      haveValidExpression = true;
    }

    if ( haveValidExpression ) {

      if ( strcmp( (*entry)->expStr.getRaw(), "" ) ) {

        char **sym = (char **) new char*[arg_count];
        char **val = (char **) new char*[arg_count];
        val = (char **) calloc( arg_count, sizeof( char* ) );
        for ( i=0; i<arg_count; ++i ) {
          sym[i] = strdup( " " );
          sym[i][0] = (char) i+65;
          val[i] = strdup( arg_name[i] );
        }
        //fprintf( stderr, "1: [%s]\n", (*entry)->expStr.getExpanded() );
        (*entry)->expStr.expand1st( arg_count, sym, val );
        //fprintf( stderr, "2: [%s]\n", (*entry)->expStr.getExpanded() );
        for ( i=0; i<arg_count; ++i ) {
          free( sym[i] );
          free( val[i] );
        }
        delete[] sym;
        delete[] val;

        for ( i=0; i<CALC_ProcessVariable::MaxArgs; ++i )
          exp_arg_name[i] = 0;

        i = ii = l = 0;
        p1 = (*entry)->expStr.getExpanded();
        gettingFirst = 1;

        while ( ii < strlen( p1 ) ) {

          if ( gettingFirst ) {

            if ( p1[ii] != ' ' ) {
              start = &p1[ii];
              l = 1;
              gettingFirst = 0;
            }

          }
          else {

            if ( p1[ii] == ',' ) {
              exp_arg_name[i] = (char *) malloc(l+1);
              strncpy( exp_arg_name[i], start, l );
              exp_arg_name[i][l] = 0;
              gettingFirst = 1;
              i++;
              l = 0;
            }
            else {
              l++;
            }

          }

          ii++;

        }

        if ( !gettingFirst ) {

              exp_arg_name[i] = (char *) malloc(l+1);
              strncpy( exp_arg_name[i], start, l );
              exp_arg_name[i][l] = 0;
              i++;

        }

#     ifdef CALC_DEBUG
        fprintf( stderr, "got %-d exp args\n", i );
        for ( ii=0; ii<i; ii++ ) {
          fprintf( stderr, "[%s]\n", exp_arg_name[ii] );
        }

        if ( strcmp( (*entry)->expStr.getRaw(), "" ) ) {
          fprintf( stderr, "raw: [%s]\n", (*entry)->expStr.getRaw() );
        }

        if ( strcmp( (*entry)->expStr.getExpanded(), "" ) ) {
          fprintf( stderr, "exp: [%s]\n", (*entry)->expStr.getExpanded() );
        }
#     endif    

        pv = new CALC_ProcessVariable(PV_name, *entry,
         i, (const char **)exp_arg_name);

        for ( ii=0; ii<i; ii++ ) {
          free( exp_arg_name[ii] );
          exp_arg_name[ii] = NULL;
        }

        pv->value = 0.0;

      }
      else {

        pv = new CALC_ProcessVariable(PV_name, *entry,
         arg_count, (const char **)arg_name);

        pv->value = 0.0;

      }

    }
    else {

      pv = new CALC_ProcessVariable(PV_name, NULL,
       arg_count, (const char **)arg_name);

      pv->value = 0.0;

      calcPvList->insert( new HashedCalcPvList( pv, expression ) );

    }

    // 'he' will delete the allocated expression
    for (i=0; i<arg_count; ++i) {
      free( arg_name[i] );
      arg_name[i] = NULL;
    }

    return pv;

}

// ------------------------------------------------------------------
// CALC_ProcessVariable ---------------------------------------------
// ------------------------------------------------------------------

CALC_ProcessVariable::CALC_ProcessVariable(const char *name,
                                           HashedExpression *_expression,
                                           size_t _arg_count,
                                           const char *arg_name[])
        : ProcessVariable(name)
{
    size_t i;

#   ifdef CALC_DEBUG
    fprintf( stderr,"CALC_ProcessVariable '%s'\n", name);
#   endif
    
    for (i=0; i<MaxArgs; ++i)
    {
        arg[i] = 0.0;
        arg_pv[i] = 0;
    }

    precision = 4;
    upper_display = 10.0;
    lower_display = 0.0;
    upper_alarm = DBL_MAX;
    lower_alarm = -DBL_MAX;
    upper_warning = DBL_MAX;
    lower_warning = -DBL_MAX;
    upper_ctrl = 10.0;
    lower_ctrl = 0.0;

    expression = _expression;
    arg_count = _arg_count;

    // first go through and populate args and create pvs
    for (i=0; i<arg_count; ++i)
    {
        // Is this argument a number or another PV?

        if ( legalFloat( arg_name[i] ) ) {

            arg[i] = strtod(arg_name[i], 0);
            //if (arg[i] == HUGE_VAL || arg[i] == -HUGE_VAL)
            if (arg[i] == CALC_PV_HUGE_VAL || arg[i] == -CALC_PV_HUGE_VAL)
            {
                fprintf(stderr, "CALC PV %s: invalid number arg '%s'\n",
                        name, arg_name[i]);
                arg[i] = 0.0;
            }
#   ifdef CALC_DEBUG
            fprintf( stderr,"arg[%d] = CONSTANT %g\n", i, arg[i]);
#   endif
        }
        else
        {   // It's a PV
            arg_pv[i] = the_PV_Factory->create(arg_name[i]);
#   ifdef CALC_DEBUG
            fprintf( stderr,"arg[%d] = PV %s\n", i, arg_name[i]);
#   endif
        }
    }

    if ( _expression ) {

      validExpression = true;

      // then add callbacks
      for (i=0; i<arg_count; ++i)
      {
          if (arg_pv[i])
          {
              arg_pv[i]->add_conn_state_callback(status_callback, this);
              arg_pv[i]->add_value_callback(value_callback, this);
          }
      }

    }
    else {

      validExpression = false;

    }

}

void CALC_ProcessVariable::completeCreation (
  HashedExpression *_expression
) {

  int i;

    expression = _expression;

    validExpression = true;

    // add callbacks
    for (i=0; i<arg_count; ++i)
    {
        if (arg_pv[i])
        {
            arg_pv[i]->add_conn_state_callback(status_callback, this);
            arg_pv[i]->add_value_callback(value_callback, this);
        }
    }

}

CALC_ProcessVariable::~CALC_ProcessVariable()
{
#ifdef CALC_DEBUG
    fprintf( stderr,"CALC_ProcessVariable(%s) deleted\n", get_name());
#endif
    for (size_t i=0; i<arg_count; ++i)
    {
        if (arg_pv[i])
        {
            arg_pv[i]->remove_value_callback(value_callback, this);
            arg_pv[i]->remove_conn_state_callback(status_callback, this);
            arg_pv[i]->release();
            arg_pv[i] = 0;
        }
    }
}

void CALC_ProcessVariable::status_callback(ProcessVariable *pv, void *userarg)
{
    CALC_ProcessVariable *me = (CALC_ProcessVariable *)userarg;
#   ifdef CALC_DEBUG
    fprintf( stderr,"CALC %s: status callback from %s.\n",
           me->get_name(), pv->get_name());
#   endif
    me->recalc();
    me->do_conn_state_callbacks();
}

void CALC_ProcessVariable::value_callback(ProcessVariable *pv, void *userarg)
{
    CALC_ProcessVariable *me = (CALC_ProcessVariable *)userarg;
#   ifdef CALC_DEBUG
    fprintf( stderr,"CALC %s: value callback %s\n",
           me->get_name(), pv->get_name());
#   endif
    if ( me->is_valid() ) {
      me->recalc();
      me->do_value_callbacks();
    }
}

void CALC_ProcessVariable::recalc()
{
    size_t i;
    status = 0;
    severity = 0;
    time = 0;
    nano = 0;
    // Evaluate arguments, if any:
    for (i=0; i<arg_count; ++i)
    {
        if (!arg_pv[i])
        {
#   ifdef CALC_DEBUG
            fprintf( stderr,"arg[%d] = constant %g\n", i, arg[i]);
#   endif
            continue;
        }
        if (arg_pv[i]->is_valid())
        {
            arg[i] = arg_pv[i]->get_double();
            // Maximize time stamp and severity
            if (arg_pv[i]->get_time_t() > time  ||
                (arg_pv[i]->get_time_t() == time  ||
                 arg_pv[i]->get_nano() > nano))
            {
                time = arg_pv[i]->get_time_t();
                nano = arg_pv[i]->get_nano();
            }
            if (arg_pv[i]->get_severity() > severity)
            {
                severity = arg_pv[i]->get_severity();
                status   = arg_pv[i]->get_status();
            }
#           ifdef CALC_DEBUG
            fprintf( stderr,"arg[%d] = PV %s %g stat %d sevr %d\n",
                   i, arg_pv[i]->get_name(), arg[i],
                   arg_pv[i]->get_status(), arg_pv[i]->get_severity());
#           endif
        }
        else
        {
            status = UDF_ALARM;
            severity = INVALID_ALARM;
#           ifdef CALC_DEBUG
            fprintf( stderr,"arg[%d] = PV %s is invalid\n",
                   i, arg_pv[i]->get_name());
#           endif
        }
    }

    if ( severity != INVALID_ALARM ) {
      expression->calc(arg, value);
    }
    else if ( processInvalid ) {
      value = invalidResult;
    }

    if (time == 0)
    {
        struct timeval t;
        gettimeofday(&t, 0);
        time = t.tv_sec;
        nano = t.tv_usec*(unsigned long)1000;
    }

#   ifdef CALC_DEBUG
    fprintf( stderr,"Result: %g stat %d sevr %d (%s)\n",
           value, status, severity,
           (is_valid() ? "valid" : "invalid"));
#   endif
}

bool CALC_ProcessVariable::is_valid() const
{
    size_t i;
    for (i=0; i<arg_count; ++i)
    {
        // arg_pv == 0 -> have constant value,
        // otherwise the PV needs to be valid
        if (arg_pv[i]!=0 && !arg_pv[i]->is_valid())
            return false;
    }
    return validExpression;
}

static ProcessVariable::Type calc_type =
{
    ProcessVariable::Type::real,
    64,
    "real:64"
};

static ProcessVariable::specificType calc_specific_type =
{
    ProcessVariable::specificType::real,
    64
};

const ProcessVariable::Type &CALC_ProcessVariable::get_type() const
{   return calc_type; }

const ProcessVariable::specificType &CALC_ProcessVariable::get_specific_type() const
{   return calc_specific_type; }

double CALC_ProcessVariable::get_double() const
{   return value; }

const char *CALC_ProcessVariable::get_char_array() const
{   return 0; }

const short *CALC_ProcessVariable::get_short_array() const
{   return 0; }

const int *CALC_ProcessVariable::get_int_array() const
{   return 0; }

const double *CALC_ProcessVariable::get_double_array() const
{   return &value; }

time_t CALC_ProcessVariable::get_time_t() const
{   return time; }

unsigned long CALC_ProcessVariable::get_nano() const
{   return nano; }

short CALC_ProcessVariable::get_status() const
{   return status; }

short CALC_ProcessVariable::get_severity() const
{   return severity; }

short CALC_ProcessVariable:: get_precision() const
{   return precision; }

double CALC_ProcessVariable::get_upper_disp_limit() const
{   return upper_display; }

double CALC_ProcessVariable::get_lower_disp_limit() const
{   return lower_display; }

double CALC_ProcessVariable::get_upper_alarm_limit() const
{   return upper_alarm; }

double CALC_ProcessVariable::get_upper_warning_limit() const
{   return upper_warning; }

double CALC_ProcessVariable::get_lower_warning_limit() const
{   return lower_warning; }

double CALC_ProcessVariable::get_lower_alarm_limit() const
{   return lower_alarm; }

double CALC_ProcessVariable::get_upper_ctrl_limit() const
{   return upper_ctrl; }

double CALC_ProcessVariable::get_lower_ctrl_limit() const
{   return lower_ctrl; }

bool CALC_ProcessVariable::put(double value)
{   return false; }

bool CALC_ProcessVariable::put(const char *value)
{   return false; }

bool CALC_ProcessVariable::put(int value)
{   return false; }

bool CALC_ProcessVariable::putText(char *value)
{   return false; }

bool CALC_ProcessVariable::putArrayText(char *value)
{   return false; }

#ifdef __cplusplus
extern "C" {
#endif

ProcessVariable *create_CALCPtr (
  const char *PV_name
) {

ProcessVariable *ptr;

  ptr = calc_pv_factory->create( PV_name );
  return ptr;

}

#ifdef __cplusplus
}
#endif
