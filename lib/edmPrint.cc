#define __edmPrint_cc 1

#include "edmPrint.str"
#include "environment.str"
#include "edmPrint.h"
#include "expString.h"

static char * const g_nullString = "";

static int posInt (
  char *tk
) {

unsigned int i;

  for ( i=0; i<strlen(tk); i++ ) {

    if ( !isdigit( tk[i] ) ) return 0;

  }

  return 1;

}

#ifdef __linux__
static void *printThread (
  THREAD_HANDLE h )
{
#endif

#ifdef __solaris__
static void *printThread (
  THREAD_HANDLE h )
{
#endif

#ifdef __osf__
static void printThread (
  THREAD_HANDLE h )
{
#endif

#ifdef HP_UX
static void *printThread (
  THREAD_HANDLE h )
{
#endif

#ifdef darwin
static void *printThread (
  THREAD_HANDLE h )
{
#endif

int stat;
edmPrintThreadParamBlockPtr threadParamBlock =
 (edmPrintThreadParamBlockPtr) thread_get_app_data( h );

  //fprintf( stderr, "this is printThread [%s]\n", threadParamBlock->cmd );

  stat = system( threadParamBlock->cmd );

  threadParamBlock->epo->printInProgress = 0;
  threadParamBlock->epo->finished = 1;
  (threadParamBlock->epo->event)++;

  stat = thread_detached_exit( h, NULL ); // this call deallocates h

#ifdef __linux__
  return (void *) NULL;
#endif

#ifdef __solaris__
  return (void *) NULL;
#endif

#ifdef darwin
  return (void *) NULL;
#endif

}


static void ok (
  Widget w,
  XtPointer client,
  XtPointer call )
{

edmPrintClass *epo = (edmPrintClass *) client;

int i, ii, opt;

  for ( i=0; i<epo->numOptions; i++ ) {
    strncpy( epo->option[i], epo->optionDefault[i],
     edmPrintClass::MAX_OPTION_CHARS );
    epo->option[i][edmPrintClass::MAX_OPTION_CHARS] = 0;
  }

  for ( i=0; i<epo->numFields; i++ ) {

    opt = epo->optionIndex[i];

    switch ( epo->optionType[i] ) {

    case edmPrintClass::STRING_TYPE:
      if ( epo->actionOperator[i][0] == edmPrintClass::OP_PLUS_EQUAL ) {
        Strncat( epo->option[opt], epo->action[i][0],
         edmPrintClass::MAX_OPTION_CHARS );
      }
      else {
        strncpy( epo->option[opt], epo->action[i][0],
         edmPrintClass::MAX_OPTION_CHARS );
      }
      Strncat( epo->option[opt], epo->optionStringValue[i],
       edmPrintClass::MAX_OPTION_CHARS );
      Strncat( epo->option[opt], " ",
       edmPrintClass::MAX_OPTION_CHARS );
      break;

    case edmPrintClass::INT_TYPE:
      if ( epo->numActions[i] ) {
        ii = epo->optionIntValue[i];
        if ( epo->actionOperator[i][ii] == edmPrintClass::OP_PLUS_EQUAL ) {
          Strncat( epo->option[opt], epo->action[i][ii],
           edmPrintClass::MAX_OPTION_CHARS );
	}
	else {
          strncpy( epo->option[opt], epo->action[i][ii],
           edmPrintClass::MAX_OPTION_CHARS );
	}
        Strncat( epo->option[opt], " ", edmPrintClass::MAX_OPTION_CHARS );
      }
      break;

    }

  }

  i = epo->numFields - 2;
  epo->printToFile = epo->optionIntValue[i]; // this is always "print to file"

  epo->cmdReady = 1;
  (epo->event)++;
  epo->printInProgress = 2;
  epo->ef.popdown();

}

static void apply (
  Widget w,
  XtPointer client,
  XtPointer call )
{

  //edmPrintClass *epo = (edmPrintClass *) client;

}

static void cancel (
  Widget w,
  XtPointer client,
  XtPointer call )
{

edmPrintClass *epo = (edmPrintClass *) client;

  epo->cmdReady = 0;
  epo->printInProgress = 0;
  epo->ef.popdown();

}

edmPrintClass::edmPrintClass ( void ) {

int i, ii;

  numOptions = 0;
  for ( i=0; i<MAX_OPTIONS; i++ ) {
    option[i] = NULL;
    optionDefault[i] = NULL;
  }

  for ( i=0; i<MAX_FIELDS; i++ ) {
    optionType[i] = 0;
    optionIntValue[i] = 0;
    strcpy( optionStringValue[i], "" );
    numActions[i] = 0;
    label[i] = NULL;
    menu[i] = NULL;
    for ( ii=0; ii<MAX_ACTIONS; ii++ ) {
      action[i][ii] = NULL;
      actionOperator[i][ii] = 0;
    }
  }

  printToFile = 0;

  threadBlock.cmd = NULL;
  threadBlock.epo = NULL;

  strcpy( xwinIdBuf, "0" );
  status = SUCCESS;
  errMsg = NULL;
  printCmd = NULL;
  printToFileCmd = NULL;
  newCmd = NULL;
  lineBuf = NULL;
  tokenInBuffer = 0;
  ctx = NULL;
  ctx2 = NULL;
  needFileRead = 1;
  printInProgress = 0;
  cmdReady = 0;
  event = 0;
  finished = 0;
  printFailureFlag = 0;
  fileDefError = 0;
  lineNo = 0;

  efW = 300;
  efH = 400;
  efMaxH = 400;

  parsePrintDefinition();

}

edmPrintClass::~edmPrintClass ( void ) {

int i, ii;

  if ( errMsg ) {
    delete[] errMsg;
    errMsg = NULL;
  }

  for ( i=0; i<MAX_OPTIONS; i++ ) {
    if ( option[i] ) {
      delete[] option[i];
      option[i] = NULL;
    }
    if ( optionDefault[i] ) {
      free( optionDefault[i] );
      optionDefault[i] = NULL;
    }
  }

  for ( i=0; i<MAX_FIELDS; i++ ) {

    if ( label[i] ) {
      free( label[i] );
      label[i] = NULL;
    }

    if ( menu[i] ) {
      free( menu[i] );
      menu[i] = NULL;
    }

    for ( ii=0; ii<MAX_ACTIONS; ii++ ) {
      if ( action[i][ii] ) {
        free( action[i][ii] );
        action[i][ii] = NULL;
      }
    }

  }

  if ( printCmd ) {
    free( printCmd );
    printCmd = NULL;
  }

  if ( printToFileCmd ) {
    free( printToFileCmd );
    printToFileCmd = NULL;
  }

  if ( newCmd ) {
    free( newCmd );
    newCmd = NULL;
  }

}

char *edmPrintClass::errorMsg ( void ) {

  return errMsg;

}

void edmPrintClass::setErrorMsg (
  char *msg
) {

  if ( !errMsg ) {
    errMsg = new char[ERR_MSG_SIZE+1];
  }

  strncpy( errMsg, msg, ERR_MSG_SIZE );
  errMsg[ERR_MSG_SIZE] = 0;

}

void edmPrintClass::setErrorMsg (
  char *msg,
  char *arg
) {

  if ( !errMsg ) {
    errMsg = new char[ERR_MSG_SIZE+1];
  }

  snprintf( errMsg, ERR_MSG_SIZE, msg, arg );

}

void edmPrintClass::setErrorMsg (
  char *msg,
  int arg
) {

  if ( !errMsg ) {
    errMsg = new char[ERR_MSG_SIZE+1];
  }

  snprintf( errMsg, ERR_MSG_SIZE, msg, arg );

}

void edmPrintClass::setErrorMsg (
  char *msg,
  char *sarg,
  int iarg
) {

  if ( !errMsg ) {
    errMsg = new char[ERR_MSG_SIZE+1];
  }

  snprintf( errMsg, ERR_MSG_SIZE, msg, sarg, iarg );

}

int edmPrintClass::printStatus ( void ) {

  return status;

}

int edmPrintClass::printStatusOK ( void ) {

  return ( status & 1 );

}

int edmPrintClass::printFailure ( void ) {

int flag = printFailureFlag;

  if ( printFailureFlag ) {
    printFailureFlag = 0;
    if ( event > 0 ) event--;
  }

  return flag;

}

int edmPrintClass::printEvent ( void ) {

  // i.e. send a print event to caller

  return event;

}

int edmPrintClass::printDefFileError ( void )  {

int flag = fileDefError;

  if ( fileDefError ) {
    fileDefError = 0;
    if ( event > 0 ) event--;
  }

  return flag;

}

int edmPrintClass::printFinished ( void ) {

int flag = finished;

  if ( finished ) {
    finished = 0;
    if ( event > 0 ) event--;
  }

  return flag;

}

int edmPrintClass::printCmdReady ( void ) {

  return cmdReady;

}

int edmPrintClass::printDialog (
  char *_displayName,
  Widget top,
  Colormap cmap,
  int x,
  int y
) {

int i;

  if ( printInProgress ) return IN_PROGRESS;

  if ( !( status & 1 ) ) return status;

  sprintf( xwinIdBuf, "%u", (unsigned int) XtWindow(top) );

  strncpy( displayName, XDisplayName(_displayName), 63 );

  // this is alway the "print to file" file name
  strcpy( option[MAX_OPTIONS-1], "" );

  efX = x;
  efY = y;

  ef.create( top, cmap, &efX, &efY, &efW, &efH, &efMaxH, "Print", NULL, NULL, NULL );

  for ( i=0; i<numFields; i++ ) {

    switch ( fieldType[i] ) {

    case FIELD_TYPE_MENU:

      ef.addOption( label[i], menu[i], &optionIntValue[i] );
      break;

    case FIELD_TYPE_TOGGLE:
      ef.addToggle( label[i], &optionIntValue[i] );
      break;

    case FIELD_TYPE_TEXT:
      ef.addTextField( label[i], 31, optionStringValue[i], 31 );
      break;

    }

  }

  ef.finished( ok, apply, cancel, this );

  ef.popup();

  printInProgress = 1;

  return SUCCESS;

}

int edmPrintClass::doPrint ( void ) {

char buf[1023+1], *tk, *ctx;
THREAD_HANDLE thread;

  if ( printInProgress != 2 ) return NOT_READY;
  if ( !cmdReady ) return NOT_READY;

  if ( !( status & 1 ) ) return status;

  if ( printToFile ) {

    if ( blank( option[MAX_OPTIONS-1] ) ) { //last option is the file name

      printInProgress = cmdReady = 0;
      if ( event > 0 ) event--;

      setErrorMsg( edmPrint_str9 );
      printFailureFlag = 1;
      event++;

      return FAILURE;

    }

    if ( !printToFileCmd ) {

      printInProgress = cmdReady = 0;
      if ( event > 0 ) event--;

      setErrorMsg( edmPrint_str10 );
      printFailureFlag = 1;
      event++;

      return FAILURE;

    }

    strncpy( buf, printToFileCmd, 1023 );
    buf[1023] = 0;

  }
  else {

    if ( !printCmd ) {

      printInProgress = cmdReady = 0;
      if ( event > 0 ) event--;

      setErrorMsg( edmPrint_str10 );
      printFailureFlag = 1;
      event++;

      return FAILURE;

    }

    strncpy( buf, printCmd, 1023 );
    buf[1023] = 0;

  }

  if ( !newCmd ) {
    newCmd = new char[1024];
  }

  strcpy( newCmd, "" );

  ctx = NULL;
  tk = strtok_r( buf, " \t\n", &ctx );
  if ( !tk ) {
 
    printInProgress = cmdReady = 0;
    if ( event > 0 ) event--;

    setErrorMsg( edmPrint_str10 );
    printFailureFlag = 1;
    event++;

    return NO_PRINT_CMD;

  }

  while ( tk ) {

    if ( strcmp( tk, "<WINID>" ) == 0 ) {
      Strncat( newCmd, xwinIdBuf, 1023 );
    }
    else if ( strcmp( tk, "<opt1>" ) == 0 ) {
      Strncat( newCmd, option[0], 1023 );
    }
    else if ( strcmp( tk, "<DSPNAME>" ) == 0 ) {
      Strncat( newCmd, displayName, 1023 );
    }
    else if ( strcmp( tk, "<opt2>" ) == 0 ) {
      Strncat( newCmd, option[1], 1023 );
    }
    else if ( strcmp( tk, "<opt3>" ) == 0 ) {
      Strncat( newCmd, option[2], 1023 );
    }
    else if ( strcmp( tk, "<opt4>" ) == 0 ) {
      Strncat( newCmd, option[3], 1023 );
    }
    else if ( strcmp( tk, "<opt5>" ) == 0 ) {
      Strncat( newCmd, option[4], 1023 );
    }
    else if ( strcmp( tk, "<opt6>" ) == 0 ) {
      Strncat( newCmd, option[5], 1023 );
    }
    else if ( strcmp( tk, "<opt7>" ) == 0 ) {
      Strncat( newCmd, option[6], 1023 );
    }
    else if ( strcmp( tk, "<opt8>" ) == 0 ) {
      Strncat( newCmd, option[7], 1023 );
    }
    else if ( strcmp( tk, "<opt9>" ) == 0 ) {
      Strncat( newCmd, option[8], 1023 );
    }
    else if ( strcmp( tk, "<opt10>" ) == 0 ) {
      Strncat( newCmd, option[9], 1023 );
    }
    else if ( strcmp( tk, "<file>" ) == 0 ) {
      Strncat( newCmd, option[10], 1023 );
    }
    else {
      Strncat( newCmd, tk,  1023 );
    }

    Strncat( newCmd, " ", 1023 );

    tk = strtok_r( NULL, " \t\n", &ctx );

  }

  // one more pass to translate <WINID> & <DSPNAME> which may have been
  // in an option

  strncpy( buf, newCmd, 1023 );
  buf[1023] = 0;

  strcpy( newCmd, "" );

  ctx = NULL;
  tk = strtok_r( buf, " \t\n", &ctx );
  if ( !tk ) {

    printInProgress = cmdReady = 0;
    if ( event > 0 ) event--;

    setErrorMsg( edmPrint_str10 );
    printFailureFlag = 1;
    event++;

    return NO_PRINT_CMD;

  }

  while ( tk ) {

    if ( strcmp( tk, "<WINID>" ) == 0 ) {
      Strncat( newCmd, xwinIdBuf, 1023 );
    }
    else if ( strcmp( tk, "<DSPNAME>" ) == 0 ) {
      Strncat( newCmd, displayName, 1023 );
    }
    else {
      Strncat( newCmd, tk,  1023 );
    }

    Strncat( newCmd, " ", 1023 );

    tk = strtok_r( NULL, " \t\n", &ctx );

  }

  if ( debugMode() ) {

    // dummy error to make the print command display
    setErrorMsg( "print command = [%s]", newCmd );
    fileDefError = 1;
    event++;

    fprintf( stderr, "%s\n", errMsg );

  }

  threadBlock.cmd = newCmd;
  threadBlock.epo = this;

  thread_create_handle( &thread, &threadBlock );
  thread_create_proc( thread, printThread ); // thread resets printInProgress
   
  cmdReady = 0;
  if ( event > 0 ) event--;

  return SUCCESS;

}

char *edmPrintClass::getTok(
  char *buf,
  char **ctx
) {

#define WHITESPACE 1
#define QUOTES 2

char *ptr, *tk;

  if ( buf ) {
    scanState = WHITESPACE;
    *ctx = buf;
  }

  ptr = *ctx;
  tk = NULL;

  if ( !( *ptr ) ) {
    return NULL;
  }

  // find start

  while ( *ptr ) {

    if ( scanState == WHITESPACE ) {

      if ( *ptr == '"' ) {
        scanState = QUOTES;
        ptr++;
        tk = ptr;
        if ( *ptr ) ptr++;
	break;
      }
      else if ( ( *ptr != ' ' ) && ( *ptr != 9 ) && ( *ptr != 10 ) ) {
        tk = ptr;
        ptr++;
	break;
      }

    }
    else { // ""

      if ( ( *ptr != '"' ) && ( *ptr != 10 ) ) {
        ptr++;
        tk = ptr;
        if ( *ptr ) ptr++;
	break;
      }

    }

    ptr++;

  }

  if ( !( *ptr ) ) {
    *ctx = ptr;
    return tk;
  }

  // find end

  while ( *ptr ) {

    if ( scanState == WHITESPACE ) {

      if ( *ptr == '"' ) {
        scanState = QUOTES;
        *ptr = 0;
        ptr++;
        *ctx = ptr;
	break;
      }
      else if ( ( *ptr == ' ' ) || ( *ptr == 9 ) || ( *ptr == 10 ) ) {
        *ptr = 0;
        ptr++;
        *ctx = ptr;
	break;
      }

    }
    else { // ""

      if ( ( *ptr == '"' ) || ( *ptr == 10 ) ) {
        scanState = WHITESPACE;
        *ptr = 0;
        ptr++;
        *ctx = ptr;
	break;
      }

    }

    ptr++;

  }

  if ( !*tk ) {
    tk = g_nullString;
  }
  if ( strcmp( tk, "\"" ) == 0 ) {
    tk = g_nullString;
  }

  return tk;

}

void edmPrintClass::putTkBack (
  char *tk
) {

  strcpy( lineBuf2, tk );
  tokenInBuffer = 1;

}

char *edmPrintClass::nextTk ( void ) {

char *tk;
int ignore;

  if ( tokenInBuffer ) {
    tokenInBuffer = 0;
    tk = lineBuf2;
    return tk;
  }

  do {

    if ( needFileRead ) {

      do {

        ignore = 0; // ignore blank lines and comment lines
        tk = fgets( lineBuf, MAX_LINE_SIZE, printDefFile );
        if ( !tk ) return NULL;

        needFileRead = 0;
        lineNo++;

        ctx = NULL;
        tk = getTok( lineBuf, &ctx );
        if ( !tk ) {
          ignore = 1;
        }
        else if ( tk[0] == '#' ) {
          ignore = 1;
        }

      } while ( ignore );

      return tk;

    }
    else {

      tk = getTok( NULL, &ctx );
      if ( !tk ) needFileRead = 1;

    }

  } while ( !tk );  

  return tk;

}

int edmPrintClass::parsePrintDefinition ( void ) {

#define DONE -1

#define GET_KEYWORD 1

#define GET_PRINT_DIALOG 5
#define GET_PRINT_DIALOG_DIMS 6

#define GET_PRINT_CMD 10

#define GET_PRINT_TO_FILE_CMD 20

#define GET_OPTION_DEFAULTS 30
#define GET_OPTION_DEFAULT_VALUE 31

#define GET_OPTION_NAME 40
#define GET_ADVANCED_OPTION_NAME 41

#define GET_MENU_LABEL 50
#define GET_MENU_DEFAULT 51
#define GET_MENU_OPTION 52

#define GET_TOGGLE_LABEL 60
#define GET_TOGGLE_DEFAULT 61
#define GET_TOGGLE_OPTION 62

#define GET_TEXT_LABEL 70
#define GET_TEXT_DEFAULT 71
#define GET_TEXT_VALUE 72

int state, index, caseIndex, op, i, ii;
char *tk, tmp[31+1];

  if ( !( status & 1 ) ) return status;

  lineBuf = new char[MAX_LINE_SIZE+1];
  lineBuf2 = new char[MAX_LINE_SIZE+1];
  tokenInBuffer = 0;
  fileDefError = 0;

  status = openPrintDefFile();
  if ( !( status & 1 ) ) {
    fileDefError = 1;
    event++;
    return status;
  }

  state = GET_KEYWORD;
  numFields = 0;

  while ( state != DONE ) {

    tk = nextTk();
    if ( !tk ) state = DONE;

    // fprintf( stderr, "(%-d) tk = [%s]\n", state, tk );

    switch ( state ) {

    case GET_KEYWORD:

      if ( strcmp( tk, "printDialog" ) == 0 ) {
        state = GET_PRINT_DIALOG;
      }
      else if ( strcmp( tk, "printCommand" ) == 0 ) {
        state = GET_PRINT_CMD;
      }
      else if ( strcmp( tk, "printToFileCommand" ) ==0 ) {
	state = GET_PRINT_TO_FILE_CMD;
      }
      else if ( strcmp( tk, "optionDefaults" ) ==0 ) {
	state = GET_OPTION_DEFAULTS;
      }
      else if ( strcmp( tk, "option" ) ==0 ) {
	state = GET_OPTION_NAME;
      }
      else if ( strcmp( tk, "advancedOption" ) ==0 ) {
	state = GET_ADVANCED_OPTION_NAME;
      }
      else {
        setErrorMsg( edmPrint_str2, tk, lineNo );
        goto syntaxError;
      }

      break;

    case GET_PRINT_DIALOG:

      if ( strcmp( tk, "{" ) != 0 ) {
	setErrorMsg( edmPrint_str5, lineNo );
	goto syntaxError;
      }

      state = GET_PRINT_DIALOG_DIMS;
      break;

    case GET_PRINT_DIALOG_DIMS:

      if ( strcmp( tk, "}" ) == 0 ) {

        state = GET_KEYWORD;
	break;

      }
      else if ( strcmp( tk, "w" ) == 0 ) {

        tk = nextTk();
        if ( !tk ) {
          setErrorMsg( edmPrint_str6 );
          goto missingData;
        }

        if ( strcmp( tk, "=" ) != 0 ) {
          setErrorMsg( edmPrint_str5, lineNo );
          goto syntaxError;
        }

        tk = nextTk();
        if ( !tk ) {
          setErrorMsg( edmPrint_str6 );
          goto missingData;
        }

        if ( !posInt(tk) ) {
          setErrorMsg( edmPrint_str5, lineNo );
          goto syntaxError;
        }
        efW = atol(tk);

      }
      else if ( strcmp( tk, "h" ) == 0 ) {

        tk = nextTk();
        if ( !tk ) {
          setErrorMsg( edmPrint_str6 );
          goto missingData;
        }

        if ( strcmp( tk, "=" ) != 0 ) {
          setErrorMsg( edmPrint_str5, lineNo );
          goto syntaxError;
        }

        tk = nextTk();
        if ( !tk ) {
          setErrorMsg( edmPrint_str6 );
          goto missingData;
        }

        if ( !posInt(tk) ) {
          setErrorMsg( edmPrint_str5, lineNo );
          goto syntaxError;
        }
        efH = efMaxH = atol(tk);

      }
      else {

        setErrorMsg( edmPrint_str5, lineNo );
        goto syntaxError;

      }

      break;


    case GET_PRINT_CMD:

      if ( strcmp( tk, "=" ) != 0 ) {
	setErrorMsg( edmPrint_str5, lineNo );
	goto syntaxError;
      }

      tk = nextTk();
      if ( !tk ) {
        setErrorMsg( edmPrint_str6 );
        goto missingData;
      }

      if ( printCmd ) {
        setErrorMsg( edmPrint_str3, lineNo );
        setErrorMsg( edmPrint_str5, lineNo );
        goto syntaxError;
      }
      printCmd = strdup( tk );

      //fprintf( stderr, "printCmd = [%s]\n", printCmd );

      state = GET_KEYWORD;

      break;

    case GET_PRINT_TO_FILE_CMD:

      if ( strcmp( tk, "=" ) != 0 ) {
	setErrorMsg( edmPrint_str5, lineNo );
	goto syntaxError;
      }

      tk = nextTk();
      if ( !tk ) {
        setErrorMsg( edmPrint_str6 );
        goto missingData;
      }

      if ( printToFileCmd ) {
        setErrorMsg( edmPrint_str4, lineNo );
        goto syntaxError;
      }
      printToFileCmd = strdup( tk );

      //fprintf( stderr, "printToFileCmd = [%s]\n", printToFileCmd );
      state = GET_KEYWORD;

      break;

    case GET_OPTION_DEFAULTS:
      if ( strcmp( tk, "{" ) == 0 ) {
        state = GET_OPTION_DEFAULT_VALUE;
      }
      else {
        setErrorMsg( edmPrint_str5, lineNo );
        goto syntaxError;
      }
      break;

    case GET_OPTION_DEFAULT_VALUE:

      if ( strcmp( tk, "}" ) == 0 ) {
	state = GET_KEYWORD;
	break;
      }

      if ( strncmp( tk, "opt", 3 ) != 0 ) {
        setErrorMsg( edmPrint_str5, lineNo );
        goto syntaxError;
      }
      if ( strlen( tk ) < 4 ) {
        setErrorMsg( edmPrint_str5, lineNo );
        goto syntaxError;
      }
      strncpy( tmp, &tk[3], 10 );
      tmp[10] = 0;
      if ( !posInt( tmp ) ) {
        setErrorMsg( edmPrint_str5, lineNo );
        goto syntaxError;
      }
      index = atol( tmp );
      if ( index < 1 ) {
        setErrorMsg( edmPrint_str7, lineNo );
        goto limitError;
      }
      if ( index > MAX_OPTIONS-1 ) {
        setErrorMsg( edmPrint_str7, lineNo );
        goto limitError; // one option reserved
      }
      index -= 1;

      tk = nextTk();
      if ( !tk ) {
        setErrorMsg( edmPrint_str6 );
        goto missingData;
      }

      if ( strcmp( tk, "=" ) != 0 ) {
	setErrorMsg( edmPrint_str5, lineNo );
	goto syntaxError;
      }

      tk = nextTk();
      if ( !tk ) {
        setErrorMsg( edmPrint_str6 );
        goto missingData;
      }

      //fprintf( stderr, "opt %-d default = %s\n", index, tk );

      if ( index+1 > numOptions ) numOptions = index+1;

      if ( !option[index] ) {
        option[index] = new char[MAX_OPTION_CHARS+1];
        strcpy( option[index], "" );
        optionDefault[index] = strdup( tk );
      }

      break;

    case GET_OPTION_NAME:

      if ( !posInt( tk ) ) {
        setErrorMsg( edmPrint_str5, lineNo );
        goto syntaxError;
      }
      index = atol( tk );
      if ( index < 1 ) {
        setErrorMsg( edmPrint_str7, lineNo );
        goto limitError;
      }
      if ( index > MAX_OPTIONS-1 ) {
        setErrorMsg( edmPrint_str7, lineNo );
        goto limitError; // one option reserved
      }
      index -= 1;

      if ( index+1 > numOptions ) numOptions = index+1;
      if ( !option[index] ) {
        option[index] = new char[MAX_OPTION_CHARS+1];
        strcpy( option[index], "" );
        optionDefault[index] = strdup( "" );
      }

      tk = nextTk(); // option name
      if ( !tk ) {
        setErrorMsg( edmPrint_str6 );
        goto missingData;
      }

      //fprintf( stderr, "\noption %-d %s", index+1, tk );
      optionIndex[numFields] = index;

      tk = nextTk();
      if ( !tk ) {
        setErrorMsg( edmPrint_str6 );
        goto missingData;
      }

      if ( strcmp( tk, "=" ) != 0 ) {
	setErrorMsg( edmPrint_str5, lineNo );
	goto syntaxError;
      }

      tk = nextTk();
      if ( !tk ) {
        setErrorMsg( edmPrint_str6 );
        goto missingData;
      }

      if ( strcmp( tk, "menu" ) == 0 ) {

        tk = nextTk();
        if ( !tk ) {
          setErrorMsg( edmPrint_str6 );
          goto missingData;
        }

        //fprintf( stderr, ": menu choices = [%s]\n", tk );
        optionType[numFields] = INT_TYPE;
        fieldType[numFields] = FIELD_TYPE_MENU;
        menu[numFields] = strdup( tk );

        tk = nextTk();
        if ( !tk ) {
          setErrorMsg( edmPrint_str6 );
          goto missingData;
        }

        if ( strcmp( tk, "{" ) != 0 ) {
	  setErrorMsg( edmPrint_str5, lineNo );
	  goto syntaxError;
        }

        state = GET_MENU_LABEL;

      }
      else if ( strcmp( tk, "toggle" ) == 0 ) {

        //fprintf( stderr, ": toggle\n" );
        optionType[numFields] = INT_TYPE;
        fieldType[numFields] = FIELD_TYPE_TOGGLE;

        tk = nextTk();
        if ( !tk ) {
          setErrorMsg( edmPrint_str6 );
          goto missingData;
        }

        if ( strcmp( tk, "{" ) != 0 ) {
	  setErrorMsg( edmPrint_str5, lineNo );
	  goto syntaxError;
        }

        state = GET_TOGGLE_LABEL;

      }
      else if ( strcmp( tk, "text" ) == 0 ) {

        //fprintf( stderr, ": text\n" );
        optionType[numFields] = STRING_TYPE;
        fieldType[numFields] = FIELD_TYPE_TEXT;

        tk = nextTk();
        if ( !tk ) {
          setErrorMsg( edmPrint_str6 );
          goto missingData;
        }

        if ( strcmp( tk, "{" ) != 0 ) {
	  setErrorMsg( edmPrint_str5, lineNo );
	  goto syntaxError;
        }

        state = GET_TEXT_LABEL;

      }
      else {

        setErrorMsg( edmPrint_str5, lineNo );
        goto syntaxError;

      }

      break;


    // menu

    case GET_MENU_LABEL:

      if ( strcmp( tk, "label" ) != 0 ) {
        setErrorMsg( edmPrint_str5, lineNo );
        goto syntaxError;
      }

      tk = nextTk();
      if ( !tk ) {
        setErrorMsg( edmPrint_str6 );
        goto missingData;
      }

      if ( strcmp( tk, "=" ) != 0 ) {
        setErrorMsg( edmPrint_str5, lineNo );
        goto syntaxError;
      }

      tk = nextTk();
      if ( !tk ) {
        setErrorMsg( edmPrint_str6 );
        goto missingData;
      }

      //fprintf( stderr, "label = [%s]\n", tk );
      label[numFields] = strdup( tk );

      state = GET_MENU_DEFAULT;

      break;

    case GET_MENU_DEFAULT:

      if ( strcmp( tk, "default" ) != 0 ) {
        setErrorMsg( edmPrint_str5, lineNo );
        goto syntaxError;
      }

      tk = nextTk();
      if ( !tk ) {
        setErrorMsg( edmPrint_str6 );
        goto missingData;
      }

      if ( strcmp( tk, "=" ) != 0 ) {
        setErrorMsg( edmPrint_str5, lineNo );
        goto syntaxError;
      }

      tk = nextTk();
      if ( !tk ) {
        setErrorMsg( edmPrint_str6 );
        goto missingData;
      }

      //fprintf( stderr, "default = [%s]\n", tk );
      optionIntValue[numFields] = atol( tk );

      state = GET_MENU_OPTION;
      numActions[numFields] = 0;

      break;

    case GET_MENU_OPTION:

      if ( strcmp( tk, "}" ) == 0 ) {
        if ( numFields < MAX_OPTIONS-2 ) numFields++; // one option reserved
	state = GET_KEYWORD;
	break;
      }

      if ( !posInt(tk) ) {
        setErrorMsg( edmPrint_str5, lineNo );
        goto syntaxError;
      }
      caseIndex = atol( tk );
      if ( caseIndex < 0 ) {
	setErrorMsg( edmPrint_str7, lineNo );
	goto limitError;
      }
      if ( caseIndex > MAX_ACTIONS-1 ) {
	setErrorMsg( edmPrint_str7, lineNo );
	goto limitError;
      }

      tk = nextTk();
      if ( !tk ) {
        setErrorMsg( edmPrint_str6 );
        goto missingData;
      }

      if ( strcmp( tk, "option" ) != 0 ) {
        setErrorMsg( edmPrint_str5, lineNo );
        goto syntaxError;
      }

      tk = nextTk();
      if ( !tk ) {
        setErrorMsg( edmPrint_str6 );
        goto missingData;
      }

      if ( strcmp( tk, "=" ) == 0 ) {
        op = OP_EQUAL;
      }
      else if ( strcmp( tk, "+=" ) == 0 ) {
        op = OP_PLUS_EQUAL;
      }
      else {
        setErrorMsg( edmPrint_str5, lineNo );
        goto syntaxError;
      }

      tk = nextTk();
      if ( !tk ) {
        setErrorMsg( edmPrint_str6 );
        goto missingData;
      }

      //fprintf( stderr, "case %-d = [%s], option=%-d, operator=%-d\n",
      // caseIndex, tk, index, op );
      action[numFields][caseIndex] = strdup( tk );
      actionOperator[numFields][caseIndex] = op;
      if ( numActions[numFields] < caseIndex+1 )
       numActions[numFields] = caseIndex+1;

      break;


    // toggle

    case GET_TOGGLE_LABEL:

      if ( strcmp( tk, "label" ) != 0 ) {
        setErrorMsg( edmPrint_str5, lineNo );
        goto syntaxError;
      }

      tk = nextTk();
      if ( !tk ) {
        setErrorMsg( edmPrint_str6 );
        goto missingData;
      }

      if ( strcmp( tk, "=" ) != 0 ) {
        setErrorMsg( edmPrint_str5, lineNo );
        goto syntaxError;
      }

      tk = nextTk();
      if ( !tk ) {
        setErrorMsg( edmPrint_str6 );
        goto missingData;
      }

      //fprintf( stderr, "label = [%s]\n", tk );
      label[numFields] = strdup( tk );

      state = GET_TOGGLE_DEFAULT;

      break;

    case GET_TOGGLE_DEFAULT:

      if ( strcmp( tk, "default" ) != 0 ) {
        setErrorMsg( edmPrint_str5, lineNo );
        goto syntaxError;
      }

      tk = nextTk();
      if ( !tk ) {
        setErrorMsg( edmPrint_str6 );
        goto missingData;
      }

      if ( strcmp( tk, "=" ) != 0 ) {
        setErrorMsg( edmPrint_str5, lineNo );
        goto syntaxError;
      }

      tk = nextTk();
      if ( !tk ) {
        setErrorMsg( edmPrint_str6 );
        goto missingData;
      }

      //fprintf( stderr, "default = [%s]\n", tk );
      optionIntValue[numFields] = atol( tk );

      state = GET_TOGGLE_OPTION;
      numActions[numFields] = 0;

      break;

    case GET_TOGGLE_OPTION:

      if ( strcmp( tk, "}" ) == 0 ) {
        if ( numFields < MAX_OPTIONS-2 ) numFields++; // one option reserved
	state = GET_KEYWORD;
	break;
      }

      if ( !posInt(tk) ) {
        setErrorMsg( edmPrint_str5, lineNo );
        goto syntaxError;
      }
      caseIndex = atol( tk );
      if ( caseIndex < 0 ) {
	setErrorMsg( edmPrint_str7, lineNo );
	goto limitError;
      }
      if ( caseIndex > MAX_ACTIONS-1 ) {
	setErrorMsg( edmPrint_str7, lineNo );
	goto limitError;
      }

      tk = nextTk();
      if ( !tk ) {
        setErrorMsg( edmPrint_str6 );
        goto missingData;
      }

      if ( strcmp( tk, "option" ) != 0 ) {
        setErrorMsg( edmPrint_str5, lineNo );
        goto syntaxError;
      }

      tk = nextTk();
      if ( !tk ) {
        setErrorMsg( edmPrint_str6 );
        goto missingData;
      }

      if ( strcmp( tk, "=" ) == 0 ) {
        op = OP_EQUAL;
      }
      else if ( strcmp( tk, "+=" ) == 0 ) {
        op = OP_PLUS_EQUAL;
      }
      else {
        setErrorMsg( edmPrint_str5, lineNo );
        goto syntaxError;
      }

      tk = nextTk();
      if ( !tk ) {
        setErrorMsg( edmPrint_str6 );
        goto missingData;
      }

      //fprintf( stderr, "case %-d = [%s], option=%-d, operator=%-d\n",
      // caseIndex, tk, index, op );
      action[numFields][caseIndex] = strdup( tk );
      actionOperator[numFields][caseIndex] = op;
      if ( numActions[numFields] < caseIndex+1 )
       numActions[numFields] = caseIndex+1;

      break;


    // text

    case GET_TEXT_LABEL:

      if ( strcmp( tk, "label" ) != 0 ) {
        setErrorMsg( edmPrint_str5, lineNo );
        goto syntaxError;
      }

      tk = nextTk();
      if ( !tk ) {
        setErrorMsg( edmPrint_str6 );
        goto missingData;
      }

      if ( strcmp( tk, "=" ) != 0 ) {
        setErrorMsg( edmPrint_str5, lineNo );
        goto syntaxError;
      }

      tk = nextTk();
      if ( !tk ) {
        setErrorMsg( edmPrint_str6 );
        goto missingData;
      }

      //fprintf( stderr, "label = [%s]\n", tk );
      label[numFields] = strdup( tk );

      state = GET_TEXT_DEFAULT;

      break;

    case GET_TEXT_DEFAULT:

      if ( strcmp( tk, "default" ) != 0 ) {
        setErrorMsg( edmPrint_str5, lineNo );
        goto syntaxError;
      }

      tk = nextTk();
      if ( !tk ) {
        setErrorMsg( edmPrint_str6 );
        goto missingData;
      }

      if ( strcmp( tk, "=" ) != 0 ) {
        setErrorMsg( edmPrint_str5, lineNo );
        goto syntaxError;
      }

      tk = nextTk();
      if ( !tk ) {
        setErrorMsg( edmPrint_str6 );
        goto missingData;
      }

      //fprintf( stderr, "default = [%s]\n", tk );
      strncpy( optionStringValue[numFields], tk, 31 );
      optionStringValue[numFields][31] = 0;

      state = GET_TEXT_VALUE;

      break;

    case GET_TEXT_VALUE:

      if ( strcmp( tk, "option" ) != 0 ) {
        setErrorMsg( edmPrint_str5, lineNo );
        goto syntaxError;
      }

      tk = nextTk();
      if ( !tk ) {
        setErrorMsg( edmPrint_str6 );
        goto missingData;
      }

      if ( strcmp( tk, "=" ) == 0 ) {
        op = OP_EQUAL;
      }
      else if ( strcmp( tk, "+=" ) == 0 ) {
        op = OP_PLUS_EQUAL;
      }
      else {
        setErrorMsg( edmPrint_str5, lineNo );
        goto syntaxError;
      }

      tk = nextTk();
      if ( !tk ) {
        setErrorMsg( edmPrint_str6 );
        goto missingData;
      }

      //fprintf( stderr, "value = [%s], option=%-d, operator=%-d\n", tk, index, op );
      numActions[numFields] = 1;
      actionOperator[numFields][0] = op;
      action[numFields][0] = strdup( tk );

      tk = nextTk();
      if ( !tk ) {
        setErrorMsg( edmPrint_str6 );
        goto missingData;
      }

      if ( strcmp( tk, "}" ) != 0 ) {
        setErrorMsg( edmPrint_str5, lineNo );
        goto syntaxError;
      }

      if ( numFields < MAX_OPTIONS-2 ) numFields++; // one option reserved
      state = GET_KEYWORD;

      break;


    case GET_ADVANCED_OPTION_NAME:
      break;

    }

  }

  delete[] lineBuf;
  lineBuf = NULL;
  delete[] lineBuf2;
  lineBuf2 = NULL;
  status = closePrintDefFile();
  if ( !( status & 1 ) ) {
    fileDefError = 1;
    event++;
    return status;
  }

  // add print to file info

  option[MAX_OPTIONS-1] = new char[MAX_OPTION_CHARS+1];
  strcpy( option[MAX_OPTIONS-1], "" );
  optionDefault[MAX_OPTIONS-1] = strdup( "" );

  optionIndex[numFields] = MAX_OPTIONS-1;
  optionType[numFields] = INT_TYPE;
  fieldType[numFields] = FIELD_TYPE_TOGGLE;
  label[numFields] = strdup( "Print To File" );
  optionIntValue[numFields] = 0;
  numActions[numFields] = 0;
  numFields++;

  optionIndex[numFields] = MAX_OPTIONS-1;
  optionType[numFields] = STRING_TYPE;
  fieldType[numFields] = FIELD_TYPE_TEXT;
  label[numFields] = strdup( "File Name" );
  strncpy( optionStringValue[numFields], "", 31 );
  optionStringValue[numFields][31] = 0;
  numActions[numFields] = 1;
  action[numFields][0] = strdup( "" );
  actionOperator[numFields][0] = OP_EQUAL;
  numFields++;

  if ( !printCmd || !printToFileCmd ) {
    setErrorMsg( edmPrint_str11 );
    fprintf( stderr, "%s\n", errMsg );
    fileDefError = 1;
    event++;
    status = FAILURE;
    return status;
  }

  // -------------------------------------------------------------------
  // Allow the environment variable EDMPRINTER to appear in all action values
  // and also in menu choices

  {

    expStringClass es;
    int numMacros = 1;
    char *macros[1];
    char *expansions[1];

    char *ep = getenv( "EDMPRINTER" );

    if ( ep ) {

      macros[0] = new char[strlen("EDMPRINTER")+1];
      strcpy( macros[0], "EDMPRINTER" );
      expansions[0] = new char[strlen(ep)+1];
      strcpy( expansions[0], ep );

      for ( i=0; i<numFields; i++ ) {

	// menu choices
	if ( fieldType[i] == FIELD_TYPE_MENU ) {
	  es.setRaw( menu[i] );
          if ( es.containsPrimaryMacros() ) {
	    es.expand1st( numMacros, macros, expansions );
	    if ( !blank( es.getExpanded() ) ) {
              delete menu[i];
	      menu[i] = new char[strlen(es.getExpanded())+1];
	      strcpy( menu[i], es.getExpanded() );
	    }
	  }
	}

	// action values
        for ( ii=0; ii<numActions[i]; ii++ ) {

	  es.setRaw( action[i][ii] );
          if ( es.containsPrimaryMacros() ) {
	    es.expand1st( numMacros, macros, expansions );
	    if ( !blank( es.getExpanded() ) ) {
              delete action[i][ii];
	      action[i][ii] = new char[strlen(es.getExpanded())+1];
	      strcpy( action[i][ii], es.getExpanded() );
	    }
	  }

        }

      }

      delete macros[0];
      delete expansions[0];

    }

  }

  // -------------------------------------------------------------------

  return SUCCESS;

missingData:

  fprintf( stderr, "%s\n", errMsg );
  status = FAILURE;
  fileDefError = 1;
  event++;
  return status;

syntaxError:

  fprintf( stderr, "%s\n", errMsg );
  status = FAILURE;
  fileDefError = 1;
  event++;
  return status;

limitError:

  fprintf( stderr, "%s\n", errMsg );
  status = FAILURE;
  fileDefError = 1;
  event++;
  return status;

}

int edmPrintClass::openPrintDefFile ( void ) {

char *envPtr;
char buf[127+1];

  if ( !( status & 1 ) ) return status;

  needFileRead = 1;
  lineNo = 0;

  // str7="EDMPRINTDEF"
  envPtr = getenv( environment_str7 );
  if ( envPtr ) {

    strncpy( buf, envPtr, 127 );
    buf[127] = 0;

  }
  else {

    // build path, str2="EDMFILES"
    envPtr = getenv( environment_str2 );
    if ( envPtr ) {

      strncpy( buf, envPtr, 127 );
      buf[127] = 0;

      if ( buf[strlen(buf)-1] != '/' ) {
        Strncat( buf, "/", 127 );
      }

    }
    else {

      strcpy( buf, "/etc/edm/" );

    }

    Strncat( buf, "edmPrintDef", 127 );

  }

  printDefFile = fopen( buf, "r" );
  if ( !printDefFile ) {
    setErrorMsg( edmPrint_str1, buf );
    return FAILURE;
  }

  return SUCCESS;

}

int edmPrintClass::closePrintDefFile ( void ) {

int stat;

  if ( !( status & 1 ) ) return status;

  stat = fclose( printDefFile );
  if ( stat ) {
    setErrorMsg( edmPrint_str8 );
    return FAILURE;
  }

  return SUCCESS;

}
