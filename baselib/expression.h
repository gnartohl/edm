#ifndef __expression_h
#define __expression_h 1

#define ARG_TYPE_SHORT 1
#define ARG_TYPE_LONG 2
#define ARG_TYPE_DOUBLE 3
#define ARG_TYPE_STRING 4

// base class for constants and epics PVs
class expArgClass {

private:

void *data;
int type;
int undefined;

public:

expArgClass::expArgClass ( void )
{

  data = (void *) NULL;
  type = 0;
  undefined = 1;

}

virtual expArgClass::~expArgClass ( void )
{

}

virtual int expArgClass::getType ( void )
{

  return type;

}

virtual void expArgClass::setValue (
  char *string )
{

}

virtual double expArgClass::getDouble ( void )
{

  return 0.0;

}

virtual int expArgClass::areArgsCompatable (
  expArgClass *otherArg )
{

  if ( ( this->type == ARG_TYPE_STRING ) &&
       ( otherArg->type != ARG_TYPE_STRING ) ) return 0;

  if ( ( otherArg->type == ARG_TYPE_STRING ) &&
       ( this->type != ARG_TYPE_STRING ) ) return 0;

  return 1;

};

virtual int expArgClass::isEqual (
  expArgClass *otherArg )
{

  if ( this->undefined || otherArg->undefined ) return 0;

  if ( this->type == ARG_TYPE_STRING ) {

    if ( strcmp( (char *) this->data, (char *) otherArg->data ) == 0 )
      return 1;
    else
      return 0;

  }
  else {

    if ( this->getDouble() == otherArg->getDouble() )
      return 1;
    else
      return 0;

  }

};

virtual int expArgClass::isNotEqual (
  expArgClass *otherArg )
{

  if ( this->undefined || otherArg->undefined ) return 0;

  if ( this->type == ARG_TYPE_STRING ) {

    if ( strcmp( (char *) this->data, (char *) otherArg->data ) != 0 )
      return 1;
    else
      return 0;

  }
  else {

    if ( this->getDouble() != otherArg->getDouble() )
      return 1;
    else
      return 0;

  }

};

virtual int expArgClass::isGreaterOrEqual (
  expArgClass *otherArg )
{

  if ( this->undefined || otherArg->undefined ) return 0;

  if ( this->type == ARG_TYPE_STRING ) {

    if ( strcmp( (char *) this->data, (char *) otherArg->data ) >= 0 )
      return 1;
    else
      return 0;

  }
  else {

    if ( this->getDouble() >= otherArg->getDouble() )
      return 1;
    else
      return 0;

  }

};

virtual int expArgClass::isGreater (
  expArgClass *otherArg )
{

  if ( this->undefined || otherArg->undefined ) return 0;

  if ( this->type == ARG_TYPE_STRING ) {

    if ( strcmp( (char *) this->data, (char *) otherArg->data ) > 0 )
      return 1;
    else
      return 0;

  }
  else {

    if ( this->getDouble() > otherArg->getDouble() )
      return 1;
    else
      return 0;

  }

};

virtual int expArgClass::isLessOrEqual (
  expArgClass *otherArg )
{

  if ( this->undefined || otherArg->undefined ) return 0;

  if ( this->type == ARG_TYPE_STRING ) {

    if ( strcmp( (char *) this->data, (char *) otherArg->data ) <= 0 )
      return 1;
    else
      return 0;

  }
  else {

    if ( this->getDouble() <= otherArg->getDouble() )
      return 1;
    else
      return 0;

  }

};

virtual int expArgClass::isLess (
  expArgClass *otherArg )
{

  if ( this->undefined || otherArg->undefined ) return 0;

  if ( this->type == ARG_TYPE_STRING ) {

    if ( strcmp( (char *) this->data, (char *) otherArg->data ) < 0 )
      return 1;
    else
      return 0;

  }
  else {

    if ( this->getDouble() < otherArg->getDouble() )
      return 1;
    else
      return 0;

  }

};

// ===========================================================================

class doubleConstClass : public expArgClass {

private:

double *privData;

public:

doubleConstClass::doubleConstClass ( void )
{

  privData = new double;
  type = ARG_TYPE_DOUBLE;
  undefined = 1;

  data = (void *) privData;

}

doubleConstClass::~doubleConstClass ( void )
{

  if ( privData ) delete privData;

}

double doubleConstClass::getDouble ( void )
{

  if ( this->undefined ) return 0.0;

  return *privData;

}

void doubleConstClass::setValue (
  char *string )
{

  *privData = atof( string );
  undefined = 0;

}

};

// ===========================================================================

class shortConstClass : public expArgClass {

private:

short *privData;

public:

shortConstClass::shortConstClass ( void )
{

  privData = new short;
  type = ARG_TYPE_SHORT;
  undefined = 1;

  data = (void *) privData;

}

shortConstClass::~shortConstClass ( void )
{

  if ( privData ) delete privData;

}

double shortConstClass::getDouble ( void )
{

  if ( this->undefined ) return 0.0;

  return (double) *privData;

}

void shortConstClass::setValue (
  char *string )
{

  *privData = (short) atod( string );
  undefined = 0;

}

};

// ===========================================================================

class longConstClass : public expArgClass {

private:

long *privData;

public:

longConstClass::longConstClass ( void )
{

  privData = new long;
  type = ARG_TYPE_LONG;
  undefined = 1;

  data = (void *) privData;

}

longConstClass::~longConstClass ( void )
{

  if ( privData ) delete privData;

}

double longConstClass::getDouble ( void )
{

  if ( this->undefined ) return 0.0;

  return (double) *privData;

}

void longConstClass::setValue (
  char *string )
{

  *privData = (long) atod( string );
  undefined = 0;

}

};

// ===========================================================================

class str40ConstClass : public expArgClass {

private:

str40 *privData;

public:

str40ConstClass::str40ConstClass ( void )
{

  privData = new char[40+1];
  type = ARG_TYPE_STRING;
  undefined = 1;

  data = (void *) privData;

}

str40ConstClass::~str40ConstClass ( void )
{

  if ( privData ) delete privData;

}

int str40ConstClass::getType ( void )
{

  return type;

}

void str40ConstClass::setValue (
  char *string )
{

  strncpy( privData, string, 40 );
  undefined = 0;

}

};

// ===========================================================================

#define MAXEXPLINES 8
#define MAXEXPARGS 16
#define MAXEXPSTACK 64
#define MAXEXPQUEUE 64

typedef struct expLineTag {
  char lparen[7+1];
  char left[40+1];
  char compareOp[2+1];
  char right[47+1];
  char rparen[7+1];
  char connector[15+1];
  unsigned long result;
} expLineType, *expLinePtr;

class expressionClass {

private:x

expArgClass *expArg[MAXEXPARGS];

int expListPtr;
expClass *exeQueue[MAXEXPQUEUE];

int expStackPtr;
expClass *exeStack[MAXEXPSTACK];

int resultStackPtr;
int resultStack[MAXEXPSTACK];

int numExpLines;
expLineType expLine[MAXEXPLINES];

public:

expressionClass::expressionClass ( void )
{

int i;

  expListPtr = 0;
  expStackPtr = 0;
  resultStackPtr = 0;
  numExpLines = 0;

}

int expressionClass::parse ( void )
{

  return 1;

}

int expressionClass::loadLine (
  char *lparen,
  char *left,
  char *compareOp,
  char *right,
  char *rparen,
  char *connector,
  int result )
{

  return 1;

}

int expressionClass::activate ( void )
{

  return 1;

}

int expressionClass::deactivate ( void )
{

  return 1;

}

unsigned long expressionClass::execute ( void )
{

  return 0;

}
