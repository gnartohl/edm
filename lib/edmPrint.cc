#define __edmPrint_cc 1

#include "edmPrint.str"
#include "environment.str"
#include "edmPrint.h"

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

int stat;
edmPrintThreadParamBlockPtr threadParamBlock =
 (edmPrintThreadParamBlockPtr) thread_get_app_data( h );

  //printf( "this is printThread [%s]\n", threadParamBlock->cmd );

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
  }

  for ( i=0; i<MAX_FIELDS; i++ ) {
    optionType[i] = 0;
    optionIntValue[i] = 0;
    strcpy( optionStringValue[i], "" );
    numActions[i] = 0;
    label[i] = NULL;
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

  efW = 300;
  efH = 400;
  efMaxH = 400;

  parsePrintDefinition();

}

edmPrintClass::~edmPrintClass ( void ) {

int i, ii;

  if ( errMsg ) {
    delete errMsg;
    errMsg = NULL;
  }

  for ( i=0; i<MAX_OPTIONS; i++ ) {
    if ( option[i] ) {
      delete option[i];
      option[i] = NULL;
    }
  }

  for ( i=0; i<MAX_FIELDS; i++ ) {

    if ( label[i] ) {
      delete label[i];
      label[i] = NULL;
    }

    if ( menu[i] ) {
      delete menu[i];
      menu[i] = NULL;
    }

    for ( ii=0; ii<MAX_ACTIONS; ii++ ) {
      if ( action[i][ii] ) {
        delete action[i][ii];
        action[i][ii] = NULL;
      }
    }

  }

  if ( printCmd ) {
    delete printCmd;
    printCmd = NULL;
  }

  if ( printToFileCmd ) {
    delete printToFileCmd;
    printToFileCmd = NULL;
  }

  if ( newCmd ) {
    delete newCmd;
    newCmd = NULL;
  }

}

int edmPrintClass::printStatusOK ( void ) {

  return ( status & 1 );

}

int edmPrintClass::printEvent ( void ) {

  return event;

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
      ef.addTextField( label[i], 20, optionStringValue[i], 31 );
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
      return FAILURE;
    }

    strncpy( buf, printToFileCmd, 1023 );
    buf[1023] = 0;

  }
  else {

    strncpy( buf, printCmd, 1023 );
    buf[1023] = 0;

  }

  if ( !newCmd ) {
    newCmd = new char[1024];
  }

  strcpy( newCmd, "" );

  ctx = NULL;
  tk = strtok_r( buf, " \t\n", &ctx );
  if ( !tk ) return NO_PRINT_CMD;

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
  if ( !tk ) return NO_PRINT_CMD;

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

  if ( debugMode() ) printf( "[%s]\n", newCmd );

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
    tk = edmPrintClass::nullString;
  }
  if ( strcmp( tk, "\"" ) == 0 ) {
    tk = edmPrintClass::nullString;
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

int state, index, caseIndex, op;
char *tk, tmp[31+1];

  if ( !( status & 1 ) ) return status;

  lineBuf = new char[MAX_LINE_SIZE+1];
  lineBuf2 = new char[MAX_LINE_SIZE+1];
  tokenInBuffer = 0;

  status = openPrintDefFile();
  if ( !( status & 1 ) ) return status;

  state = GET_KEYWORD;
  numFields = 0;

  while ( state != DONE ) {

    tk = nextTk();
    if ( !tk ) state = DONE;

    // printf( "(%-d) tk = [%s]\n", state, tk );

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
        printf( "Unknown keyword [%s]\n", tk );
        status = FAILURE;
        return FAILURE;
      }

      break;

    case GET_PRINT_DIALOG:

      if ( strcmp( tk, "{" ) != 0 ) {
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
          goto missingData;
        }

        if ( strcmp( tk, "=" ) != 0 ) {
          goto syntaxError;
        }

        tk = nextTk();
        if ( !tk ) {
          goto missingData;
        }

        if ( !posInt(tk) ) {
          goto syntaxError;
        }
        efW = atol(tk);

      }
      else if ( strcmp( tk, "h" ) == 0 ) {

        tk = nextTk();
        if ( !tk ) {
          goto missingData;
        }

        if ( strcmp( tk, "=" ) != 0 ) {
          goto syntaxError;
        }

        tk = nextTk();
        if ( !tk ) {
          goto missingData;
        }

        if ( !posInt(tk) ) {
          goto syntaxError;
        }
        efH = efMaxH = atol(tk);

      }
      else {

        goto syntaxError;

      }

      break;


    case GET_PRINT_CMD:

      if ( strcmp( tk, "=" ) != 0 ) {
	goto syntaxError;
      }

      tk = nextTk();
      if ( !tk ) {
        goto missingData;
      }

      if ( printCmd ) {
	printf( "Multiple definitions of printCmd\n" );
        goto syntaxError;
      }
      printCmd = strdup( tk );

      //printf( "printCmd = [%s]\n", printCmd );

      state = GET_KEYWORD;

      break;

    case GET_PRINT_TO_FILE_CMD:

      if ( strcmp( tk, "=" ) != 0 ) {
	goto syntaxError;
      }

      tk = nextTk();
      if ( !tk ) {
        goto missingData;
      }

      if ( printToFileCmd ) {
	printf( "Multiple definitions of printToFileCmd\n" );
        goto syntaxError;
      }
      printToFileCmd = strdup( tk );

      //printf( "printToFileCmd = [%s]\n", printToFileCmd );
      state = GET_KEYWORD;

      break;

    case GET_OPTION_DEFAULTS:
      if ( strcmp( tk, "{" ) == 0 ) {
        state = GET_OPTION_DEFAULT_VALUE;
      }
      else {
        goto syntaxError;
      }
      break;

    case GET_OPTION_DEFAULT_VALUE:

      if ( strcmp( tk, "}" ) == 0 ) {
	state = GET_KEYWORD;
	break;
      }

      if ( strncmp( tk, "opt", 3 ) != 0 ) {
        goto syntaxError;
      }
      if ( strlen( tk ) < 4 ) {
        goto syntaxError;
      }
      strncpy( tmp, &tk[3], 10 );
      tmp[10] = 0;
      if ( !posInt( tmp ) ) {
        goto syntaxError;
      }
      index = atol( tmp );
      if ( index < 1 ) goto limitError;
      if ( index > MAX_OPTIONS-1 ) goto limitError; // one option reserved
      index -= 1;

      tk = nextTk();
      if ( !tk ) {
        goto missingData;
      }

      if ( strcmp( tk, "=" ) != 0 ) {
	goto syntaxError;
      }

      tk = nextTk();
      if ( !tk ) {
        goto missingData;
      }

      //printf( "opt %-d default = %s\n", index, tk );

      if ( index+1 > numOptions ) numOptions = index+1;

      if ( !option[index] ) {
        option[index] = new char[MAX_OPTION_CHARS+1];
        strcpy( option[index], "" );
        optionDefault[index] = strdup( tk );
      }

      break;

    case GET_OPTION_NAME:

      if ( !posInt( tk ) ) {
        goto syntaxError;
      }
      index = atol( tk );
      if ( index < 1 ) goto limitError;
      if ( index > MAX_OPTIONS-1 ) goto limitError; // one option reserved
      index -= 1;

      if ( index+1 > numOptions ) numOptions = index+1;
      if ( !option[index] ) {
        option[index] = new char[MAX_OPTION_CHARS+1];
        strcpy( option[index], "" );
        optionDefault[index] = strdup( "" );
      }

      tk = nextTk(); // option name
      if ( !tk ) {
        goto missingData;
      }

      //printf( "\noption %-d %s", index+1, tk );
      optionIndex[numFields] = index;

      tk = nextTk();
      if ( !tk ) {
        goto missingData;
      }

      if ( strcmp( tk, "=" ) != 0 ) {
	goto syntaxError;
      }

      tk = nextTk();
      if ( !tk ) {
        goto missingData;
      }

      if ( strcmp( tk, "menu" ) == 0 ) {

        tk = nextTk();
        if ( !tk ) {
          goto missingData;
        }

        //printf( ": menu choices = [%s]\n", tk );
        optionType[numFields] = INT_TYPE;
        fieldType[numFields] = FIELD_TYPE_MENU;
        menu[numFields] = strdup( tk );

        tk = nextTk();
        if ( !tk ) {
          goto missingData;
        }

        if ( strcmp( tk, "{" ) != 0 ) {
	  goto syntaxError;
        }

        state = GET_MENU_LABEL;

      }
      else if ( strcmp( tk, "toggle" ) == 0 ) {

        //printf( ": toggle\n" );
        optionType[numFields] = INT_TYPE;
        fieldType[numFields] = FIELD_TYPE_TOGGLE;

        tk = nextTk();
        if ( !tk ) {
          goto missingData;
        }

        if ( strcmp( tk, "{" ) != 0 ) {
	  goto syntaxError;
        }

        state = GET_TOGGLE_LABEL;

      }
      else if ( strcmp( tk, "text" ) == 0 ) {

        //printf( ": text\n" );
        optionType[numFields] = STRING_TYPE;
        fieldType[numFields] = FIELD_TYPE_TEXT;

        tk = nextTk();
        if ( !tk ) {
          goto missingData;
        }

        if ( strcmp( tk, "{" ) != 0 ) {
	  goto syntaxError;
        }

        state = GET_TEXT_LABEL;

      }
      else {

        goto syntaxError;

      }

      break;


    // menu

    case GET_MENU_LABEL:

      if ( strcmp( tk, "label" ) != 0 ) {
        goto syntaxError;
      }

      tk = nextTk();
      if ( !tk ) {
        goto missingData;
      }

      if ( strcmp( tk, "=" ) != 0 ) {
        goto syntaxError;
      }

      tk = nextTk();
      if ( !tk ) {
        goto missingData;
      }

      //printf( "label = [%s]\n", tk );
      label[numFields] = strdup( tk );

      state = GET_MENU_DEFAULT;

      break;

    case GET_MENU_DEFAULT:

      if ( strcmp( tk, "default" ) != 0 ) {
        goto syntaxError;
      }

      tk = nextTk();
      if ( !tk ) {
        goto missingData;
      }

      if ( strcmp( tk, "=" ) != 0 ) {
        goto syntaxError;
      }

      tk = nextTk();
      if ( !tk ) {
        goto missingData;
      }

      //printf( "default = [%s]\n", tk );
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
        goto syntaxError;
      }
      caseIndex = atol( tk );
      if ( caseIndex < 0 ) goto limitError;
      if ( caseIndex > MAX_ACTIONS-1 ) goto limitError;

      tk = nextTk();
      if ( !tk ) {
        goto missingData;
      }

      if ( strcmp( tk, "option" ) != 0 ) {
        goto syntaxError;
      }

      tk = nextTk();
      if ( !tk ) {
        goto missingData;
      }

      if ( strcmp( tk, "=" ) == 0 ) {
        op = OP_EQUAL;
      }
      else if ( strcmp( tk, "+=" ) == 0 ) {
        op = OP_PLUS_EQUAL;
      }
      else {
        goto syntaxError;
      }

      tk = nextTk();
      if ( !tk ) {
        goto missingData;
      }

      //printf( "case %-d = [%s], option=%-d, operator=%-d\n",
      // caseIndex, tk, index, op );
      action[numFields][caseIndex] = strdup( tk );
      actionOperator[numFields][caseIndex] = op;
      if ( numActions[numFields] < caseIndex+1 )
       numActions[numFields] = caseIndex+1;

      break;


    // toggle

    case GET_TOGGLE_LABEL:

      if ( strcmp( tk, "label" ) != 0 ) {
        goto syntaxError;
      }

      tk = nextTk();
      if ( !tk ) {
        goto missingData;
      }

      if ( strcmp( tk, "=" ) != 0 ) {
        goto syntaxError;
      }

      tk = nextTk();
      if ( !tk ) {
        goto missingData;
      }

      //printf( "label = [%s]\n", tk );
      label[numFields] = strdup( tk );

      state = GET_TOGGLE_DEFAULT;

      break;

    case GET_TOGGLE_DEFAULT:

      if ( strcmp( tk, "default" ) != 0 ) {
        goto syntaxError;
      }

      tk = nextTk();
      if ( !tk ) {
        goto missingData;
      }

      if ( strcmp( tk, "=" ) != 0 ) {
        goto syntaxError;
      }

      tk = nextTk();
      if ( !tk ) {
        goto missingData;
      }

      //printf( "default = [%s]\n", tk );
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
        goto syntaxError;
      }
      caseIndex = atol( tk );
      if ( caseIndex < 0 ) goto limitError;
      if ( caseIndex > MAX_ACTIONS-1 ) goto limitError;

      tk = nextTk();
      if ( !tk ) {
        goto missingData;
      }

      if ( strcmp( tk, "option" ) != 0 ) {
        goto syntaxError;
      }

      tk = nextTk();
      if ( !tk ) {
        goto missingData;
      }

      if ( strcmp( tk, "=" ) == 0 ) {
        op = OP_EQUAL;
      }
      else if ( strcmp( tk, "+=" ) == 0 ) {
        op = OP_PLUS_EQUAL;
      }
      else {
        goto syntaxError;
      }

      tk = nextTk();
      if ( !tk ) {
        goto missingData;
      }

      //printf( "case %-d = [%s], option=%-d, operator=%-d\n",
      // caseIndex, tk, index, op );
      action[numFields][caseIndex] = strdup( tk );
      actionOperator[numFields][caseIndex] = op;
      if ( numActions[numFields] < caseIndex+1 )
       numActions[numFields] = caseIndex+1;

      break;


    // text

    case GET_TEXT_LABEL:

      if ( strcmp( tk, "label" ) != 0 ) {
        goto syntaxError;
      }

      tk = nextTk();
      if ( !tk ) {
        goto missingData;
      }

      if ( strcmp( tk, "=" ) != 0 ) {
        goto syntaxError;
      }

      tk = nextTk();
      if ( !tk ) {
        goto missingData;
      }

      //printf( "label = [%s]\n", tk );
      label[numFields] = strdup( tk );

      state = GET_TEXT_DEFAULT;

      break;

    case GET_TEXT_DEFAULT:

      if ( strcmp( tk, "default" ) != 0 ) {
        goto syntaxError;
      }

      tk = nextTk();
      if ( !tk ) {
        goto missingData;
      }

      if ( strcmp( tk, "=" ) != 0 ) {
        goto syntaxError;
      }

      tk = nextTk();
      if ( !tk ) {
        goto missingData;
      }

      //printf( "default = [%s]\n", tk );
      strncpy( optionStringValue[numFields], tk, 31 );
      optionStringValue[numFields][31] = 0;

      state = GET_TEXT_VALUE;

      break;

    case GET_TEXT_VALUE:

      if ( strcmp( tk, "option" ) != 0 ) {
        goto syntaxError;
      }

      tk = nextTk();
      if ( !tk ) {
        goto missingData;
      }

      if ( strcmp( tk, "=" ) == 0 ) {
        op = OP_EQUAL;
      }
      else if ( strcmp( tk, "+=" ) == 0 ) {
        op = OP_PLUS_EQUAL;
      }
      else {
        goto syntaxError;
      }

      tk = nextTk();
      if ( !tk ) {
        goto missingData;
      }

      //printf( "value = [%s], option=%-d, operator=%-d\n", tk, index, op );
      numActions[numFields] = 1;
      actionOperator[numFields][0] = op;
      action[numFields][0] = strdup( tk );

      tk = nextTk();
      if ( !tk ) {
        goto missingData;
      }

      if ( strcmp( tk, "}" ) != 0 ) {
        goto syntaxError;
      }

      if ( numFields < MAX_OPTIONS-2 ) numFields++; // one option reserved
      state = GET_KEYWORD;

      break;


    case GET_ADVANCED_OPTION_NAME:
      break;

    }

  }

  delete lineBuf;
  lineBuf = NULL;
  delete lineBuf2;
  lineBuf2 = NULL;
  status = closePrintDefFile();
  if ( !( status & 1 ) ) return status;


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

  return SUCCESS;

missingData:

  printf( "Missing information in print definition file\n" );
  status = FAILURE;
  return status;

syntaxError:

  printf( "Syntax error in print definition file\n" );
  status = FAILURE;
  return status;

limitError:

  printf( "Parameter exceeds limit in print definition file\n" );
  status = FAILURE;
  return status;

  return SUCCESS;

}

int edmPrintClass::openPrintDefFile ( void ) {

char *envPtr;
char buf[127+1];

  if ( !( status & 1 ) ) return status;

  needFileRead = 1;

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
    errMsg = strdup( edmPrint_str1 );
    status = FAILURE; // error
    return FAILURE;
  }

  return SUCCESS;

}

int edmPrintClass::closePrintDefFile ( void ) {

  if ( !( status & 1 ) ) return status;

  fclose( printDefFile );

  return SUCCESS;

}

