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

// utility functions

#include "utility.h"
#include "environment.str"

static int g_serverSocketFd = -1;
static int g_accumulator = 0;
static int g_useAccumulator = 0;

int doReSearchReplace (
  int caseInsensivite,
  char *expression,
  char *newText,
  int max,
  char *oldString,
  char *newString
) {

regex_t compiled_re;
int res, status=0;
int flags;

  strcpy( newString, "" );

  //fprintf( stderr, "expression = [%s]\n", expression );

  flags = REG_EXTENDED;
  if ( caseInsensivite ) flags |= REG_ICASE;

  res = regcomp( &compiled_re, expression, flags );
  if ( res ) {

    // error (consider it no match)
    status = -1;

  }
  else {

    int arrayi, l, start, size, so, eo, index,
     i, ii, newi, oldi, newTextLen;
    regmatch_t pmatch[10];
    bool more, first;

    newi = 0;
    oldi = 0;
    arrayi = 0;
    l = strlen( oldString );
    newTextLen = strlen( newText );
    more = true;
    index = 0;
    first = true;
    do {

      //fprintf( stderr, "index = %-d\n", index );
      //fprintf( stderr, "search string = [%s]\n", &oldString[index] );
      res = regexec( &compiled_re, &oldString[index],
       10, pmatch, REG_EXTENDED );
      if ( !res ) {

	//for ( i=9; i>=0; i-- ) {
        //  fprintf( stderr, "so[%-d] = %-d, eo[%-d] = %-d\n",
        //   i, pmatch[i].rm_so,
        //   i, pmatch[i].rm_eo );
        //}

	// find last match
	size = 0;
	for ( i=9; i>=0; i-- ) {
          //fprintf( stderr, "so[%-d] = %-d, eo[%-d] = %-d\n",
          // i, pmatch[i].rm_so,
          // i, pmatch[i].rm_eo );
          if ( pmatch[i].rm_eo != pmatch[i].rm_so ) {
            so = pmatch[i].rm_so + index;
            eo = pmatch[i].rm_eo + index;
	    size = eo - so;
	    break;
	  }
	}

        if ( size > 0 ) {

          start = oldi;
          for ( ii=start; ii<so; ii++ ) {
            newString[newi] = oldString[oldi];
            newi++; if ( newi >= max ) newi = max;
            oldi++;
          }
          oldi = eo;
          for ( ii=0; ii<newTextLen; ii++ ) {
            newString[newi] = newText[ii];
            newi++; if ( newi >= max ) newi = max;
          }
          newString[newi] = 0;
	  //printf( "newString is [%s]\n", newString );

        }
	else {

	  if ( first ) {
            status = -1;
	    more = false;
	  }

	}

        if ( more ) {
          first = false;
          eo = pmatch[0].rm_eo + index;
          index = eo;
          if ( index >= l ) more = false;
        }

      }
      else {

        if ( first ) status = -1;

	more = false;

	// copy rest of string
	//fprintf( stderr, "oldi = %-d\n", oldi );
	//fprintf( stderr, "newi = %-d\n", newi );
        for ( ii=oldi; ii<l; ii++ ) {
          newString[newi] = oldString[ii];
          newi++; if ( newi >= max ) newi = max;
          //newString[newi] = 0;
          //printf( "newString is [%s]\n", newString );
        }
        newString[newi] = 0;
        //printf( "final newString is [%s]\n", newString );

      }

    } while ( more );

  }

  regfree(&compiled_re);

  return status;

}

int doSearchReplace (
  int caseInsensivite,
  int useRegExpr,
  char *expression,
  char *newText,
  int max,
  char *oldString,
  char *newString
) {

  unsigned int i, ii, newi, start,
   lenOldString, lenExpression, lenNewText;
  int result, lastPos;
  bool more, anyMatch;

  if ( useRegExpr ) {

    return doReSearchReplace( caseInsensivite, expression, newText,
     max, oldString, newString );

  }

  lenOldString = strlen( oldString );
  lenExpression = strlen( expression );
  lenNewText = strlen( newText );
  lastPos = lenOldString - lenExpression;
  if ( lastPos < 0 ) return -1; // no match

  start = newi = i = 0;
  more = true;
  anyMatch = false;

  while ( more ) {

    if ( caseInsensivite ) {
      result = strncasecmp( expression, &oldString[i], lenExpression );
    }
    else {
      result = strncmp( expression, &oldString[i], lenExpression );
    }

    if ( result == 0 ) { // match
      anyMatch = true;
      // skip past found expression in old string
      i += lenExpression;
      if ( i >= lenOldString ) more = false;
      // insert expression into newString
      for ( ii=0; ii<lenNewText; ii++ ) {
        if ( newi < (unsigned int) max ) newString[newi] = newText[ii];
        newi++;
      }
    }
    else {
      // copy single char into newString
      if ( newi < (unsigned int) max ) newString[newi] = oldString[i];
      newi++;
      i++;
      if ( i >= lenOldString ) more = false;
    }

  }

  if ( newi < (unsigned int) max ) newString[newi] = 0;
  newString[max] = 0;

  if ( anyMatch ) {
    return 0;
  }
  else {
    return -1;
  }

}

void enableAccumulator ( void ) {

static int noAccumulator = -1;

  if ( noAccumulator == -1 ) {
    char *envPtr = getenv( environment_str36 );
    if ( envPtr ) {
      noAccumulator = 1;
    }
    else {
      noAccumulator = 0;
    }
  }

  if ( noAccumulator == 1 ) {
    g_useAccumulator = 0;
  }
  else {
    g_useAccumulator = 1;
  }

}

void disableAccumulator ( void ) {
  g_useAccumulator = 0;
}

int useAccumulator ( void ) {
  return g_useAccumulator;
}

void setAccumulator (
  int v
) {
  g_accumulator = v;
}

int getAccumulator ( void ) {
  return g_accumulator;
}

void incAccumulator ( void ) {
  g_accumulator++;
}

int buildSymbols (
  char *string,
  int *n,
  char symbols[10][31+1],
  int increment[10],
  int multiplier[10]
) {

regex_t compiled_re;
int status=0, res, i;

char *re = "\\$\\(([-]?[0-9]+\\*)*(#)(([+-])([0-9]+))*\\)";

  //fprintf( stderr, "re = [%s]\n", re );
  //fprintf( stderr, "string = [%s]\n", string );

  for ( i=0; i<10; i++ ) {
    strcpy( symbols[i], "" );
    increment[i] = 0;
    multiplier[i] = 1;
  }
  *n = 0;

  res = regcomp( &compiled_re, re, REG_EXTENDED );
  if ( res ) {

    // error
    status = -1;

  }
  else {

    int arrayi, l, start, size, so, eo, index, mult, inc, sign, offset, len;
    char text[1023+1];
    regmatch_t pmatch[10];
    bool more;

    arrayi = 0;
    l = strlen( string );
    more = true;
    index = 0;
    do {

      //fprintf( stderr, "index = %-d\n", index );
      res = regexec( &compiled_re, &string[index], 6, pmatch, REG_EXTENDED );
      if ( !res ) {

        //for ( i=0; i<6; i++ ) {
        //  fprintf( stderr, "so = %-d, eo = %-d\n", pmatch[i].rm_so + index,
        //  pmatch[i].rm_eo + index );
        //}

        // copy matched substring into display string
        // match 0 is always the full match,
        // match 1 is the first selected substring
        so = pmatch[0].rm_so + index;
        eo = pmatch[0].rm_eo + index;
        start = so;
        size = eo - so;
        if ( size > 1023 ) size = 1023;

        if ( size <= 0 ) {
          //fprintf( stderr, "no match\n" );
          more = false;
          continue;
        }

        so = pmatch[1].rm_so + index;
        eo = pmatch[1].rm_eo + index;
        start = so;
        size = eo - so;
        if ( size > 1023 ) size = 1023;

        if ( size <= 0 ) {
          mult = 1;
        }
        else {
          memcpy( text, string+start, size );
          text[size] = 0;
          mult = strtol( text, NULL, 0 );
        }

        multiplier[arrayi] = mult;

        so = pmatch[4].rm_so + index;
        eo = pmatch[4].rm_eo + index;
        start = so;
        size = eo - so;
        if ( size > 1023 ) size = 1023;

        if ( size <= 0 ) {
          sign = 1;
        }
        else {
          memcpy( text, string+start, size );
          text[size] = 0;
          if ( strcmp( text, "-" ) == 0 ) {
            sign = -1;
          }
          else {
            sign = 1;
          }
        }

        so = pmatch[5].rm_so + index;
        eo = pmatch[5].rm_eo + index;
        start = so;
        size = eo - so;
        if ( size > 1023 ) size = 1023;

        if ( size <= 0 ) {
          inc = 0;
        }
        else {
          memcpy( text, string+start, size );
          text[size] = 0;
          inc = strtol( text, NULL, 0 ) * sign;
        }

        increment[arrayi] = inc;

        so = pmatch[0].rm_so + index;
        eo = pmatch[0].rm_eo + index;
        start = so;
        size = eo - so;
        if ( size > 1023 ) size = 1023;
        memcpy( text, string+start, size );
        text[size] = 0;
        if ( size > 3 ) {
          offset = 2;
          text[size-1] = 0;
          len = size - 3;
          //fprintf( stderr, "1 - text = [%s]\n", &text[2] );
        }
        else {
          offset = 0;
          len = size;
          //fprintf( stderr, "2 - text = [%s]\n", text );
        }

        if ( len > 31 ) len = 31;
        strncpy( symbols[arrayi], &text[offset], len+1 );
        symbols[arrayi][31] = 0;

        eo = pmatch[0].rm_eo + index;
        index = eo;
        if ( index >= l ) more = false;

      }
      else {

        more = false;
        //regerror( res, &compiled_re, buf, sizeof buf );
        //fprintf( stderr,"2 - Error in regular expression match: %s\n", buf );

      }

      arrayi++;
      if ( arrayi < 10 ) {
        (*n)++;
      }
      else {
        more = false;
      }

    } while ( more );

  }

  regfree(&compiled_re);

  return status;

}

void doAccSubs (
  expStringClass &s
) {

  if ( useAccumulator() ) {

    if ( s.getRaw() && !blank(s.getRaw()) ) {

      expStringClass tmp;
      int n, inc[10], mult[10];
      char syms[10][31+1];
      char vals[10][31+1];
      char *syms1[10];
      char *vals1[10];

      int i;
      int status = buildSymbols( s.getRaw(), &n, syms, inc, mult );

      if ( status ) return;

      for ( i=0; i<n; i++ ) {
        snprintf( vals[i], 31, "%-d", mult[i] * getAccumulator() + inc[i] );
        syms1[i] = syms[i];
        vals1[i] = vals[i];
        //fprintf( stderr, "syms1[%-d] = %s, vals1[%-d] = %s\n",
        // i, syms1[i], i, vals1[i] );
      }

      //fprintf( stderr, "1 - es = [%s]\n", s.getRaw() );
      tmp.setRaw( s.getRaw() );
      tmp.expand1st( n, (char **) syms1, (char **) vals1 );
      s.setRaw( tmp.getExpanded() );
      //fprintf( stderr, "2 - es = [%s]\n", s.getRaw() );

    }

  }

}

void doAccSubs (
  char *s,
  int maxlen
) {

  if ( useAccumulator() && ( maxlen > 0 ) ) {

    if ( s && !blank(s) ) {
      expStringClass tmp;
      int n, inc[10], mult[10];
      char syms[10][31+1];
      char vals[10][31+1];
      char *syms1[10];
      char *vals1[10];

      int i;
      int status = buildSymbols( s, &n, syms, inc, mult );

      if ( status ) return;

      for ( i=0; i<n; i++ ) {
        snprintf( vals[i], 31, "%-d", mult[i] * getAccumulator() + inc[i] );
        syms1[i] = syms[i];
        vals1[i] = vals[i];
      }

      tmp.setRaw( s );
      tmp.expand1st( n, (char **) syms1, (char **) vals1 );

      strncpy( s, tmp.getExpanded(), maxlen-1 );
      s[maxlen-1] = 0;

    }

  }

}

int useAppTopParent ( void ) {

static int useAppTop = -1;
char *envPtr;

  if ( useAppTop == -1 ) {
    envPtr = getenv( environment_str34 );
    if ( envPtr ) {
      useAppTop = 1;
    }
    else {
      useAppTop = 0;
    }
  }

  return useAppTop;

}

int debugMode ( void ) {

int val;
char *envPtr;

  envPtr = getenv( environment_str29 );
  if ( envPtr ) {
    val = atol(envPtr);
    if ( !val ) val = 1; // if value is non-numeric make it 1
    return val;
  }
  else {
    return 0;
  }

}

static int g_needDiagModeInit = 1;
static int g_diagMode = 0;

int diagnosticMode ( void ) {

int val;
char *envPtr;

  if ( g_needDiagModeInit ) {

    g_needDiagModeInit = 0;

    envPtr = getenv( environment_str30 );
    if ( envPtr ) {
      g_diagMode = atol(envPtr);
      if ( !g_diagMode ) val = 1; // if value is non-numeric make it 1
    }
    else {
      g_diagMode = 0;
    }

  }

  return g_diagMode;

}

static int g_needDiagInit = 1;
static char g_diagFileName[255+1];

int logDiagnostic (
  char *text
) {

char *envPtr;
char time_string[34+1];
static FILE *diagFile = NULL;
static pid_t procPid;
static char hostName[63+1], buf[255+1];
int i, ii, l, fd;
mode_t curMode;

  if ( g_needDiagInit ) {

    g_needDiagInit = 0;

    procPid = getpid();
    gethostname( hostName, 63 );

    sys_get_datetime_string( 31, time_string );
    time_string[31] = 0;
    Strncat( time_string, " : ", 34 );

    envPtr = getenv( environment_str8 );
    if ( envPtr ) {
      strncpy( g_diagFileName, envPtr, 255 );
      if ( envPtr[strlen(envPtr)] != '/' ) {
        Strncat( g_diagFileName, "/", 255 );
      }
    }
    else {
      strncpy( g_diagFileName, "/tmp/", 255 );
    }

    Strncat( g_diagFileName, "edmStdErr", 255 );

    Strncat( g_diagFileName, "_XXXXXX", 255 );

    mkstemp( g_diagFileName );

    l = strlen( g_diagFileName );
    for ( i=l-7, ii=0; i<l; i++, ii++ ) {
      buf[ii] = g_diagFileName[i];
    }
    buf[ii] = 0;

    close( 2 );
    curMode = umask( 0 );
    fd = open( g_diagFileName, O_CREAT|O_WRONLY, 0777 );
    umask( curMode );
    fprintf( stderr, time_string );
    fprintf( stderr, "host %s, pid %-d - ", hostName, procPid );
    fprintf( stderr, "first diagnostic message\n" );
    fprintf( stderr, time_string );
    fprintf( stderr, text );

    envPtr = getenv( environment_str8 );
    if ( envPtr ) {
      strncpy( g_diagFileName, envPtr, 255 );
      if ( envPtr[strlen(envPtr)] != '/' ) {
        Strncat( g_diagFileName, "/", 255 );
      }
    }
    else {
      strncpy( g_diagFileName, "/tmp/", 255 );
    }

    Strncat( g_diagFileName, "edmDiag", 255 );

    Strncat( g_diagFileName, buf, 255 );

    curMode = umask( 0 );
    diagFile = fopen( g_diagFileName, "a" );
    umask( curMode );

    if ( diagFile ) {

      fprintf( diagFile, time_string );
      fprintf( diagFile, "host %s, pid %-d - ", hostName, procPid );
      fprintf( diagFile, "first diagnostic message\n" );
      fprintf( diagFile, time_string );
      fprintf( diagFile, text );

    }
    else {

      return 2;

    }

  }
  else {

    curMode = umask( 0 );
    diagFile = fopen( g_diagFileName, "a" );
    umask( curMode );

    if ( diagFile ) {

      sys_get_datetime_string( 31, time_string );
      time_string[31] = 0;
      Strncat( time_string, " : ", 34 );
      fprintf( diagFile, time_string );
      fprintf( diagFile, text );

    }
    else {

      return 2;

    }

  }

  fclose( diagFile );

  return 1;

}

static int g_badWindowErrorsAreDisabled = 0;

void disableBadWindowErrors ( int arg ) {

  g_badWindowErrorsAreDisabled = arg;

}

int badWindowErrorsDisabled ( void ) {

  return g_badWindowErrorsAreDisabled;

}

char *getEnvironmentVar (
  char *name
) {

FILE *f;
char *tk, *context, buf[32000+1];
static char *var = NULL;

  if ( !getenv(name) ) return NULL;

  strncpy( buf, getenv(name), 255 );
  buf[255] = 0;

  //fprintf( stderr, "name = [%s]\n", buf );

  context = NULL;
  tk = strtok_r( buf, " \t\n", &context );

  if ( !tk ) return NULL;
  if ( tk[0] != '@' ) return getenv( name );

  // else name translates to file from which values are read
  strncpy( buf, getenv(name), 255 );
  buf[255] = 0;

  context = NULL;
  tk = strtok_r( buf, "@ \t\n", &context );

  if ( !tk ) return NULL;

  //fprintf( stderr, "read env var values from file: [%s]\n", tk );

  f = fileOpen( tk, "r" );

  tk = fgets( buf, 32000, f );
  buf[32000] = 0;
  if ( !tk ) {
    fclose( f );
    return NULL;
  }

  context = NULL;
  tk = strtok_r( buf, "\n", &context );
  if ( !tk ) return NULL;

  if ( var ) delete var;

  var = new char[strlen(tk)+1];
  strcpy( var, tk );

  return var;

}

void setServerSocketFd (
  int fd
) {

  g_serverSocketFd = fd;

}

// From Stevens' book

#ifdef OPEN_MAX
static int g_openMax = OPEN_MAX;
#else
static int g_openMax = 0;
#endif

#define OPEN_MAX_GUESS 256

static int open_max ( void ) {

  if ( !g_openMax ) {
    errno = 0;
    if ( ( g_openMax = sysconf( _SC_OPEN_MAX ) ) < 0 ) {
      g_openMax = OPEN_MAX_GUESS;
    }
  }

  return g_openMax;

}

// From Stevens' book

int executeCmd (
  const char *cmdString
) {

pid_t pid;
int status, i, l;
struct sigaction ignore, saveintr, savequit;
sigset_t chldmask, savemask;
char buf[2048+1];

  if ( !cmdString ) return 1;

  l = strlen( cmdString);
  if ( l > 2048 ) return 1;

  strcpy( buf, cmdString );

  ignore.sa_handler = SIG_IGN;
  sigemptyset( &ignore.sa_mask );
  ignore.sa_flags = 0;
  if ( sigaction( SIGINT, &ignore, &saveintr ) < 0 ) return -1;
  if ( sigaction( SIGQUIT, &ignore, &savequit ) < 0 ) return -1;

  sigemptyset( &chldmask );
  sigaddset( &chldmask, SIGCHLD );
  if ( sigprocmask( SIG_BLOCK, &chldmask, &savemask ) < 0 ) return -1;

  if ( ( pid = fork() ) < 0 ) {

    status = -1;

  }
  else if ( pid == 0 ) {    // child

    // restore previous signal actions & reset signal mask
    sigaction( SIGINT, &saveintr, NULL );
    sigaction( SIGQUIT, &savequit, NULL );
    sigprocmask( SIG_SETMASK, &savemask, NULL );

    if ( g_serverSocketFd != -1 ) {
      close( g_serverSocketFd );
    }

    // close all open files
    for ( i=3; i<open_max(); i++ ) {
      close( i );
    }

    // create new process group session
    setsid();

    execl( "/bin/sh", "sh", "-c", buf, (char *) NULL );
    _exit(127);

  }
  else {    // parent

    while ( waitpid( pid, &status, 0 ) < 0 ) {
      if ( errno != EINTR ) {
	status = -1;
	break;
      }
    }

  }

  // restore previous signal actions & reset signal mask
  if ( sigaction( SIGINT, &saveintr, NULL ) < 0 ) return -1;
  if ( sigaction( SIGQUIT, &savequit, NULL ) < 0 ) return -1;
  if ( sigprocmask( SIG_SETMASK, &savemask, NULL ) < 0 ) return -1;

  return status;

}

#ifdef __linux__
static void *executeCmdThread (
  THREAD_HANDLE h )
{
#endif

#ifdef darwin
static void *executeCmdThread (
  THREAD_HANDLE h )
{
#endif

#ifdef __solaris__
static void *executeCmdThread (
  THREAD_HANDLE h )
{
#endif

#ifdef __osf__
static void executeCmdThread (
  THREAD_HANDLE h )
{
#endif

#ifdef HP_UX
static void *executeCmdThread (
  THREAD_HANDLE h )
{
#endif

int stat;
char *cmd = (char *) thread_get_app_data( h );

  stat = executeCmd( cmd );

  stat = thread_request_free_ptr( (void *) cmd );
  stat = thread_detached_exit( h, NULL ); // this call deallocates h

#ifdef __linux__
  return (void *) NULL;
#endif

#ifdef darwin
  return (void *) NULL;
#endif
  
#ifdef __solaris__
  return (void *) NULL;
#endif

}

void executeCommandInThread (
  char *_cmd
) {

char *cmd;
THREAD_HANDLE thread;
int stat;

  if ( !_cmd ) return;
  if ( blankOrComment(_cmd) ) return;

  cmd = new char[strlen(_cmd)+1];
  strcpy( cmd, _cmd );

  stat = thread_create_handle( &thread, (void *) cmd );
  stat = thread_create_proc( thread, executeCmdThread );
  stat = thread_detach( thread );

}

char *expandEnvVars (
  char *inStr,
  int maxOut,
  char *outStr
) {

  // expands all environment vars of the form $(envVar) found in inStr
  // and sends the expanded string to outStr

int i, ii, iii=0, inL, state, bufOnHeap;
char *ptr, stackBuf[255+1];
char *buf;

static const int DONE = -1;
static const int FINDING_DOLLAR = 1;
static const int FINDING_LEFT_PAREN = 2;
static const int FINDING_RIGHT_PAREN = 3;

  if ( !inStr || !outStr || ( maxOut < 1 ) ) return NULL;

  inL = strlen( inStr );
  if ( inL < 1 ) return NULL;

  if ( maxOut > 255 ) {
    buf = new char[maxOut+1];
    bufOnHeap = 1;
  }
  else {
    buf = stackBuf;
    bufOnHeap = 0;
  }

  state = FINDING_DOLLAR;
  strcpy( outStr, "" );
  i = ii = 0;
  while ( state != DONE ) {

    switch ( state ) {

    case FINDING_DOLLAR:

      if ( i >= inL ) {
        state = DONE;
        break;
      }

      if ( inStr[i] == '\n' ) {
        state = DONE;
      }
      else if ( inStr[i] == '$' ) {
        state = FINDING_LEFT_PAREN;
      }
      else {
        if ( ii >= maxOut ) goto limitErr; // out string too big
        outStr[ii] = inStr[i];
        ii++;
      }

      break;

    case FINDING_LEFT_PAREN:

      if ( i >= inL ) goto syntaxErr; // never found '(' after '$'
      if ( inStr[i] == '\n' ) goto syntaxErr; // never found '(' after '$'
      if ( inStr[i] != '(' ) goto syntaxErr; // char after '$' was not '('
      strcpy( buf, "" );
      iii = 0;
      state = FINDING_RIGHT_PAREN;

      break;

    case FINDING_RIGHT_PAREN:

      if ( i >= inL ) goto syntaxErr; // never found ')'
      if ( inStr[i] == '\n' ) goto syntaxErr; // never found ')'

      if ( inStr[i] == ')' ) {

	// add terminating 0
        if ( iii >= maxOut ) goto syntaxErr; // env var value too big
        buf[iii] = 0;

        // translate and output env var value using buf

        if ( !blank( buf ) ) {

          ptr = getenv( buf );
          if ( ptr ) {

            for ( iii=0; iii<(int) strlen(ptr); iii++ ) {
              if ( ii > maxOut ) goto limitErr;
              outStr[ii] = ptr[iii];
              ii++;
	    }

	  }

	}

        state = FINDING_DOLLAR;

      }
      else {

        if ( iii >= maxOut ) goto syntaxErr; // env var value too big
        buf[iii] = inStr[i];
        iii++;

      }

      break;

    }

    i++;

  }

// normalReturn

  // add terminating 0
  if ( ii > maxOut ) goto limitErr;
  outStr[ii] = 0;

  if ( bufOnHeap ) delete [] buf;

  return outStr;

syntaxErr:

  fprintf( stderr, "Syntax error in env var reference\n" );
  if ( bufOnHeap ) delete [] buf;
  return NULL;

limitErr:

  fprintf( stderr, "Parameter size limit exceeded in env var reference\n" );
  if ( bufOnHeap ) delete [] buf;
  return NULL;

}

int blank (
  char *string )
{

unsigned int i, l;

  l = strlen(string);
  if ( !l ) return 1;

  for ( i=0; i<l; i++ ) {
    if ( !isspace( (int) string[i] ) ) return 0;
  }

  return 1;

}

int blankOrComment (
  char *string )
{

// return 1 for blank string or if first char is comment character

unsigned int i, l;

  l = strlen(string);
  if ( !l ) return 1;

  for ( i=0; i<l; i++ ) {
    if ( !isspace( (int) string[i] ) ) {
      if ( string[i] == '#' ) return 1;
      return 0;
    }
  }

  return 1;

}

XtIntervalId appAddTimeOut (
  XtAppContext app,
  unsigned long interval,
  XtTimerCallbackProc proc,
  XtPointer client_data )
{

  if ( interval < 10 ) {
    interval = 10; // ms
  }

  return XtAppAddTimeOut( app, interval, proc, client_data );

}

void genericProcessAllEvents (
  int sync,
  XtAppContext app,
  Display *d )
{

XEvent Xev;
int result, isXEvent, count;

  count = 1000;

  if ( sync ) {
    XFlush( d );
    XSync( d, False ); /* wait for all X transactions to complete */
  }

  do {
    result = XtAppPending( app );
    if ( result ) {
      isXEvent = XtAppPeekEvent( app, &Xev );
      if ( isXEvent ) {
        if ( Xev.type != Expose ) {
          XtAppProcessEvent( app, result );
	}
        else {
	  // XtAppNextEvent( app, &Xev ); // discard
          XtAppProcessEvent( app, result );
	}
      }
      else { // process all timer or alternate events
        XtAppProcessEvent( app, result );
      }
    }
    count--;
  } while ( result && count );

}

void processAllEventsWithSync (
  XtAppContext app,
  Display *d )
{

  genericProcessAllEvents( 1, app, d );

}

void processAllEvents (
  XtAppContext app,
  Display *d )
{

  genericProcessAllEvents( 0, app, d );

}

void trimWhiteSpace (
  char *str )
{

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

int isLegalInteger (
  char *str )
{

char buf[127+1];
int i, l, legal, state, hex;

  strncpy( buf, str, 127 );
  trimWhiteSpace( buf );
  l = strlen(buf);
  if ( l < 1 ) return 0;

  hex = 0;
  i = 0;
  if ( l > 2 ) {
    if ( ( buf[0] == '0' ) && ( ( buf[1] == 'x' ) || ( buf[1] == 'X' ) ) ) {
      hex = 1;
      i = 2;
    }
  }

  state = SIGN_OR_NUM;
  legal = 1;
  while ( state != DONE ) {

    if ( i >= l ) state = DONE;

    switch ( state ) {

    case SIGN_OR_NUM:

      if ( buf[i] == '-' ) {
        i++;
        state = NUM;
        continue;
      }
        
      if ( buf[i] == '+' ) {
        i++;
        state = NUM;
        continue;
      }
        
      if ( hex ) {
        if ( isxdigit(buf[i]) ) {
          i++;
          state = NUM;
          continue;
        }
      }
      else {
        if ( isdigit(buf[i]) ) {
          i++;
          state = NUM;
          continue;
        }
      }

      legal = 0;
      state = DONE;

      break;        

    case NUM:

      if ( hex ) {
        if ( isxdigit(buf[i]) ) {
          i++;
          continue;
        }
      }
      else {
        if ( isdigit(buf[i]) ) {
          i++;
          continue;
        }
      }

      legal = 0;
      state = DONE;

      break;        

    }

  }

  return legal;

}

int isLegalFloat (
  char *str )
{

char buf[127+1];
int i, l, legal, state;

  strncpy( buf, str, 127 );
  trimWhiteSpace( buf );
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

int writeStringToFile (
  FILE *f,
  char *str )
{

int stat, i, j;
char tmp[255+1];

  if ( strcmp( str, "" ) == 0 ) {

    stat = fprintf( f, "<<<empty>>>\n" );
    if ( stat == -1 ) return 0;

  }
  else if ( blank(str) ) {

    {
      char buf[300+1];

      strcpy( buf, "<<<blank>>>" );
      Strncat( buf, str, 300 );
      for ( i=0, j=0; i<(int)strlen(buf); i++ ) {
        if ( buf[i] == 10 ) {
	  tmp[j] = 0;
          stat = fprintf( f, "%s\\n", tmp );
          if ( stat == -1 ) return 0;
          j = 0;
	}
        else if ( buf[i] == '\\' ) {
	  tmp[j] = 0;
          stat = fprintf( f, "%s\\\\", tmp );
          if ( stat == -1 ) return 0;
          j = 0;
	}
        else {
          tmp[j] = buf[i];
          j++;
          if ( j == 255 ) {
            tmp[j] = 0;
            stat = fprintf( f, "%s", tmp );
            if ( stat == -1 ) return 0;
            j = 0;
	  }
	}
      }
      if ( j ) {
        tmp[j] = 0;
        stat = fprintf( f, "%s\n", tmp );
        if ( stat == -1 ) return 0;
        j = 0;
      }
      else {
        stat = fprintf( f, "\n" );
        if ( stat == -1 ) return 0;
      }

    }

  }
  else {

    for ( i=0, j=0; i<(int)strlen(str); i++ ) {
      if ( str[i] == 10 ) {
        tmp[j] = 0;
        stat = fprintf( f, "%s\\n", tmp );
        if ( stat == -1 ) return 0;
        j = 0;
      }
      else if ( str[i] == '\\' ) {
        tmp[j] = 0;
        stat = fprintf( f, "%s\\\\", tmp );
        if ( stat == -1 ) return 0;
        j = 0;
      }
      else {
        tmp[j] = str[i];
        j++;
        if ( j == 255 ) {
          tmp[j] = 0;
          stat = fprintf( f, "%s", tmp );
          if ( stat == -1 ) return 0;
          j = 0;
	}
      }
    }
    if ( j ) {
      tmp[j] = 0;
      stat = fprintf( f, "%s\n", tmp );
      if ( stat == -1 ) return 0;
      j = 0;
    }
    else {
      stat = fprintf( f, "\n" );
      if ( stat == -1 ) return 0;
    }

  }

  return 1;

}

#if 0
void readStringFromFile (
  char *str,
  int maxChars,
  FILE *f )
{

char *ptr;
int i, l;
unsigned int ii;

  ptr = fgets( str, maxChars, f );
  if ( !ptr ) {
    strcpy( str, "" );
    return;
  }

  l = strlen(str);

  if ( l > maxChars ) l = maxChars;
  if ( l < 1 ) l = 1;
  str[l-1] = 0;

  if ( strcmp( str, "<<<empty>>>" ) == 0 ) {

    strcpy( str, "" );

  }
  else if ( strncmp( str, "<<<blank>>>", 11 ) == 0 ) {

    {
      char buf[300+1];

      strncpy( buf, str, 300 );
      for ( i=0, ii=11; ii<strlen(str); i++, ii++ ) {
        str[i] = buf[ii];
      }

      if ( i > maxChars ) i = maxChars;
      str[i] = 0;

    }

  }

}
#endif

#if 1
void readStringFromFile (
  char *str,
  int maxChars,
  FILE *f )
{

char *ptr;
int i, j, l, max, first, escape;
char buf[10000+1];

  if ( maxChars < 1 ) return;

  if ( maxChars > 10000 )
    max = 10000;
  else
    max = maxChars-1;

  ptr = fgets( buf, 10000, f );
  if ( !ptr ) {
    strcpy( str, "4 0 0" );
    return;
  }
  buf[10000] = 0;

  l = strlen(buf);

  buf[l-1] = 0;
  if ( l > max ) l = max;

  if ( strcmp( buf, "<<<empty>>>" ) == 0 ) {

    strcpy( str, "" );
    return;

  }
  else if ( strncmp( buf, "<<<blank>>>", 11 ) == 0 ) {

    first = 11;

  }
  else {

    first = 0;

  }

  escape = 0;
  for ( i=first, j=0; i<l; i++ ) {

    if ( escape ) {

      if ( buf[i] == '\\' ) {
        str[j] = buf[i];
        if ( j < max ) j++;
      }
      else if ( buf[i] == 'n' ) {
        str[j] = 10;
        if ( j < max ) j++;
      }
      else {
        str[j] = buf[i];
        if ( j < max ) j++;
      }

      escape = 0;

    }
    else {

      if ( buf[i] == '\\' ) {
        escape = 1;
      }
      else if ( buf[i] == 1 ) {
        str[j] = 10;
        if ( j < max ) j++;
      }
      else {
        str[j] = buf[i];
        if ( j < max ) j++;
      }

    }

  }

  str[j] = 0;

}
#endif

int xDrawText (
  Display *d,
  Window win,
  gcClass *gc,
  XFontStruct *fs,
  int _x,
  int _y,
  int _alignment,
  char *value ) {

int stringLength, stringWidth, stringX, stringY;

  stringLength = strlen( value );

  if ( fs ) {
    stringWidth = XTextWidth( fs, value, stringLength );
    stringY = _y + fs->ascent;
  }
  else {
    stringWidth = 0;
    stringY = _y;
  }

  stringX = _x;

  if ( _alignment == XmALIGNMENT_BEGINNING ) {
    // no change
  }
  else if ( _alignment == XmALIGNMENT_CENTER )
    stringX = _x - stringWidth/2;
  else if ( _alignment == XmALIGNMENT_END )
    stringX = _x - stringWidth;

  XDrawString( d, win,
   gc->normGC(), stringX, stringY, value, stringLength );

  return 1;

}

int xEraseText (
  Display *d,
  Window win,
  gcClass *gc,
  XFontStruct *fs,
  int _x,
  int _y,
  int _alignment,
  char *value ) {

int stringLength, stringWidth, stringX, stringY;

  stringLength = strlen( value );

  if ( fs ) {
    stringWidth = XTextWidth( fs, value, stringLength );
    stringY = _y + fs->ascent;
  }
  else {
    stringWidth = 0;
    stringY = _y;
  }

  stringX = _x;

  if ( _alignment == XmALIGNMENT_BEGINNING ) {
    // no change
  }
  else if ( _alignment == XmALIGNMENT_CENTER )
    stringX = _x - stringWidth/2;
  else if ( _alignment == XmALIGNMENT_END )
    stringX = _x - stringWidth;

  XDrawString( d, win,
   gc->eraseGC(), stringX, stringY, value, stringLength );

  return 1;

}

int xDrawImageText (
  Display *d,
  Window win,
  gcClass *gc,
  XFontStruct *fs,
  int _x,
  int _y,
  int _alignment,
  char *value ) {

int stringLength, stringWidth, stringX, stringY;

  stringLength = strlen( value );

  if ( fs ) {
    stringWidth = XTextWidth( fs, value, stringLength );
    stringY = _y + fs->ascent;
  }
  else {
    stringWidth = 0;
    stringY = _y;
  }

  stringX = _x;

  if ( _alignment == XmALIGNMENT_BEGINNING ) {
    // no change
  }
  else if ( _alignment == XmALIGNMENT_CENTER )
    stringX = _x - stringWidth/2;
  else if ( _alignment == XmALIGNMENT_END )
    stringX = _x - stringWidth;

  XDrawImageString( d, win,
   gc->normGC(), stringX, stringY, value, stringLength );

  return 1;

}

int xEraseImageText (
  Display *d,
  Window win,
  gcClass *gc,
  XFontStruct *fs,
  int _x,
  int _y,
  int _alignment,
  char *value ) {

int stringLength, stringWidth, stringX, stringY;

  stringLength = strlen( value );

  if ( fs ) {
    stringWidth = XTextWidth( fs, value, stringLength );
    stringY = _y + fs->ascent;
  }
  else {
    stringWidth = 0;
    stringY = _y;
  }

  stringX = _x;

  if ( _alignment == XmALIGNMENT_BEGINNING ) {
    // no change
  }
  else if ( _alignment == XmALIGNMENT_CENTER )
    stringX = _x - stringWidth/2;
  else if ( _alignment == XmALIGNMENT_END )
    stringX = _x - stringWidth;

  XDrawImageString( d, win,
   gc->eraseGC(), stringX, stringY, value, stringLength );

  return 1;

}

int drawImageText (
  Widget widget,
  Drawable dr,
  gcClass *gc,
  XFontStruct *fs,
  int _x,
  int _y,
  int _alignment,
  char *value ) {

int stringLength, stringWidth, stringX, stringY;

  stringLength = strlen( value );

  if ( fs ) {
    stringWidth = XTextWidth( fs, value, stringLength );
    stringY = _y + fs->ascent;
  }
  else {
    stringWidth = 0;
    stringY = _y;
  }

  stringX = _x;

  if ( _alignment == XmALIGNMENT_BEGINNING ) {
    // no change
  }
  else if ( _alignment == XmALIGNMENT_CENTER )
    stringX = _x - stringWidth/2;
  else if ( _alignment == XmALIGNMENT_END )
    stringX = _x - stringWidth;

  XDrawImageString( XtDisplay(widget), dr,
   gc->normGC(), stringX, stringY, value, stringLength );

  return 1;

}

int drawImageText (
  Widget widget,
  gcClass *gc,
  XFontStruct *fs,
  int _x,
  int _y,
  int _alignment,
  char *value ) {

  Drawable dr = XtWindow(widget);
  return drawImageText( widget, dr, gc, fs, _x, _y, _alignment, value );

}

int eraseImageText (
  Widget widget,
  Drawable dr,
  gcClass *gc,
  XFontStruct *fs,
  int _x,
  int _y,
  int _alignment,
  char *value ) {

int stringLength, stringWidth, stringX, stringY;

  stringLength = strlen( value );

  if ( fs ) {
    stringWidth = XTextWidth( fs, value, stringLength );
    stringY = _y + fs->ascent;
  }
  else {
    stringWidth = 0;
    stringY = _y;
  }

  stringX = _x;

  if ( _alignment == XmALIGNMENT_BEGINNING ) {
    // no change
  }
  else if ( _alignment == XmALIGNMENT_CENTER )
    stringX = _x - stringWidth/2;
  else if ( _alignment == XmALIGNMENT_END )
    stringX = _x - stringWidth;

  XDrawImageString( XtDisplay(widget), dr,
   gc->eraseGC(), stringX, stringY, value, stringLength );

  return 1;

}

int eraseImageText (
  Widget widget,
  gcClass *gc,
  XFontStruct *fs,
  int _x,
  int _y,
  int _alignment,
  char *value ) {

  Drawable dr = XtWindow(widget);
  return eraseImageText( widget, dr, gc, fs, _x, _y, _alignment, value );

}

int drawText (
  Widget widget,
  Drawable dr,
  gcClass *gc,
  XFontStruct *fs,
  int _x,
  int _y,
  int _alignment,
  char *value ) {

int stat;

  stat = xDrawText( XtDisplay(widget), dr,
   gc, fs, _x, _y, _alignment, value );

  return stat;

}

int drawText (
  Widget widget,
  gcClass *gc,
  XFontStruct *fs,
  int _x,
  int _y,
  int _alignment,
  char *value ) {

  Drawable dr = XtWindow(widget);
  return drawText( widget, dr, gc, fs, _x, _y, _alignment, value );

}

int eraseText (
  Widget widget,
  Drawable dr,
  gcClass *gc,
  XFontStruct *fs,
  int _x,
  int _y,
  int _alignment,
  char *value ) {

int stat;

  stat = xEraseText( XtDisplay(widget), dr,
   gc, fs, _x, _y, _alignment, value );

  return stat;

}

int eraseText (
  Widget widget,
  gcClass *gc,
  XFontStruct *fs,
  int _x,
  int _y,
  int _alignment,
  char *value ) {

  Drawable dr = XtWindow(widget);
  return eraseText( widget, dr, gc, fs, _x, _y, _alignment, value );


}

int textBoundaries (
  XFontStruct *fs,
  int _x,
  int _y,
  int _alignment,
  char *value,
  int *x0,
  int *y0,
  int *x1,
  int *y1 ) {

int stringLength, stringWidth, stringX, asc, dsc;

  stringLength = strlen( value );

  if ( fs ) {
    asc = fs->ascent;
    dsc = fs->descent;
    stringWidth = XTextWidth( fs, value, stringLength );
  }
  else {
    asc = 10;
    dsc = 5;
    stringWidth = 0;
  }

  stringX = _x;

  if ( _alignment == XmALIGNMENT_BEGINNING ) {
    // no change
  }
  else if ( _alignment == XmALIGNMENT_CENTER )
    stringX = _x - stringWidth/2;
  else if ( _alignment == XmALIGNMENT_END )
    stringX = _x - stringWidth;

  *y0 = _y;
  *y1 = _y + asc + dsc;
  *x0 = stringX;
  *x1 = stringX + stringWidth;

  return 1;

}

int fileIsLocked (
  FILE *f )
{

int stat;
int fd = fileno(f);
struct flock l;

  l.l_type = F_WRLCK;
  l.l_start = 0;
  l.l_whence = SEEK_SET;
  l.l_len = 1;
  l.l_pid = 0;

  stat = fcntl( fd, F_GETLK, &l );

  if ( stat < 0 ) {
    return 1; /* failure, assume file is locked */
  }

  if ( l.l_pid == 0 )
    return 0; /* not locked */
  else
    return 1; /* locked */

}

int lockFile (
  FILE *f )
{

int stat;
int fd = fileno(f);
struct flock l;

  l.l_type = F_WRLCK;
  l.l_start = 0;
  l.l_whence = SEEK_SET;
  l.l_len = 1;

  stat = fcntl( fd, F_SETLK, &l );

  if ( stat < 0 )
    return 0; /* even, failure */
  else
    return 1; /* odd, success */

}

int unlockFile (
  FILE *f )
{

int stat;
int fd = fileno(f);
struct flock l;

  l.l_type = F_UNLCK;
  l.l_start = 0;
  l.l_whence = SEEK_SET;
  l.l_len = 1;

  stat = fcntl( fd, F_SETLK, &l );

  if ( stat < 0 )
    return 0; /* even, failure */
  else
    return 1; /* odd, success */

  return 1;

}

void buildFileName (
  char *inName,
  char *prefix,
  char *postfix,
  char *expandedName,
  int maxSize )
{

char *gotOne;

    gotOne = strstr( inName, "/" );

  if ( gotOne ) {
    strncpy( expandedName, inName, maxSize );
  }
  else {
    strncpy( expandedName, prefix, maxSize );
    Strncat( expandedName, inName, maxSize );
  }

  if ( strlen(expandedName) > strlen(postfix) ) {
    if ( strcmp( &expandedName[strlen(expandedName)-strlen(postfix)], postfix )
     != 0 ) {
      Strncat( expandedName, postfix, maxSize );
    }
  }
  else {
    Strncat( expandedName, postfix, maxSize );
  }

}

int getFileName (
  char *name,
  char *fullName,
  int maxSize )
{

int start, end, i, ii, l, ret_stat;

 if ( !fullName || !name ) {
   ret_stat = 0;
   goto err_return;
 }

  l = strlen(fullName);

  start = 0;

  for ( i=l-1; i>=0; i-- ) {

    if ( fullName[i] == '/' ) {
      start = i+1;
      break;

    }

  }

  end = l-1;

  for ( i=l-1; i>=start; i-- ) {

    if ( fullName[i] == '.' ) {
      end = i-1;
      break;

    }

  }

  strcpy( name, "" );
  for ( i=start, ii=0; (i<=end) && (ii<maxSize); i++, ii++ ) {
    name[ii] = fullName[i];
  }

  if ( ii >= maxSize ) ii = maxSize-1;
  name[ii] = 0;

  return 1;

err_return:

  if ( name ) strcpy( name, "" );

  return ret_stat;

}

int getFilePrefix (
  char *prefix,
  char *fullName,
  int maxSize )
{

int start, end, i, ii, l, ret_stat;

 if ( !fullName || !prefix ) {
   ret_stat = 0;
   goto err_return;
 }

  l = strlen(fullName);

  start = 0;
  end = -1;

  for ( i=l-1; i>=0; i-- ) {

    if ( fullName[i] == '/' ) {
      end = i;
      break;

    }

  }

  strcpy( prefix, "" );
  for ( i=start, ii=0; (i<=end) && (ii<maxSize); i++, ii++ ) {
    prefix[ii] = fullName[i];
  }

  if ( ii >= maxSize ) ii = maxSize-1;
  prefix[ii] = 0;

  return 1;

err_return:

  if ( prefix ) strcpy( prefix, "" );

  return ret_stat;

}

int getFilePostfix (
  char *postfix,
  char *fullName,
  int maxSize )
{

int start, end, i, ii, l, ret_stat;

 if ( !fullName || !postfix ) {
   ret_stat = 0;
   goto err_return;
 }

  l = strlen(fullName);

  start = l;
  end = l-1;

  for ( i=l-1; i>=0; i-- ) {

    if ( fullName[i] == '/' ) break;

    if ( fullName[i] == '.' ) {
      start = i;
      break;

    }

  }

  strcpy( postfix, "" );
  for ( i=start, ii=0; (i<=end) && (ii<maxSize); i++, ii++ ) {
    postfix[ii] = fullName[i];
  }

  if ( ii >= maxSize ) ii = maxSize-1;
  postfix[ii] = 0;

  return 1;

err_return:

  if ( postfix ) strcpy( postfix, "" );

  return ret_stat;

}

char *getNextDataString (
  char *str,
  int max,
  FILE *f )
{

int blankOrComment;
char *gotOne, *tk, *context, buf[255+1];

  do {

    gotOne = fgets( str, max, f );
    if ( !gotOne ) return NULL;

    strncpy( buf, str, 255 );

    context = NULL;
    tk = strtok_r( buf, " \t\n", &context );

    blankOrComment = 0;
    if ( tk ) {
      if ( tk[0] == '#' ) blankOrComment = 1;
    }
    else {
      blankOrComment = 1;
    }

  } while ( blankOrComment );

  return str;

}

// these routines consider an ascii 1 to mean new line

void XDrawStrings (
  Display *d,
  Window w,
  GC gc,
  int x,
  int y,
  int h,
  char *str,
  int len )
{

int i, start, l, strL;

  i = start = l = 0;
  strL = strlen( str );

  while ( i < strL ) {

    if ( ( str[i] == 1 ) || ( str[i] == 10 ) ) {

      if ( l ) {
        XDrawString( d, w, gc, x, y, &str[start], l );
        l = 0;
      }

      start = i+1;
      y += h;

    }
    else {

      l++;

    }

    i++;

  }

  if ( l ) {
    XDrawString( d, w, gc, x, y, &str[start], l );
  }

}

void getStringBoxSize (
  char *str,
  int len,
  XFontStruct **fs,
  int alignment,
  int *width,
  int *height )
{

int i, start, l, strL, stringWidth, maxWidth, maxHeight, charHeight=0;

  maxWidth = 2;
  maxHeight = 0;

  i = start = l = 0;
  strL = strlen( str );

  while ( i < strL ) {

    if ( ( str[i] == 1 ) || ( str[i] == 10 ) ) {

      if ( l ) {

        if ( fs && *fs ) {
          stringWidth = XTextWidth( *fs, &str[start], l );
          charHeight = (*fs)->ascent + (*fs)->descent;
	}
	else {
	  stringWidth = 10;
          charHeight = 2;
	}

        if ( stringWidth > maxWidth ) maxWidth = stringWidth;

        l = 0;

      }

      start = i+1;
      maxHeight += charHeight;

    }
    else {

      l++;

    }

    i++;

  }

  if ( l ) {

    if ( fs && *fs ) {
      stringWidth = XTextWidth( *fs, &str[start], l );
      charHeight = (*fs)->ascent + (*fs)->descent;
    }
    else {
      stringWidth = 10;
      charHeight = 2;
    }

    if ( stringWidth > maxWidth ) maxWidth = stringWidth;

    maxHeight += charHeight;

  }

  *width = maxWidth;
  *height = maxHeight;

}

void XDrawStringsAligned (
  Display *d,
  Window w,
  GC gc,
  int x,
  int y,
  int fieldWidth,
  char *str,
  int len,
  XFontStruct **fs,
  int alignment )
{

int charHeight=0, i, start, l, strL, stringWidth, stringX=0;

  i = start = l = 0;
  strL = strlen( str );

  while ( i < strL ) {

    if ( ( str[i] == 1 ) || ( str[i] == 10 ) ) {

      if ( l ) {

        if ( fs && *fs ) {
          stringWidth = XTextWidth( *fs, &str[start], l );
          charHeight = (*fs)->ascent + (*fs)->descent;
	}
	else {
	  stringWidth = 10;
          charHeight = 2;
	}

        if ( alignment == XmALIGNMENT_BEGINNING )
          stringX = x;
        else if ( alignment == XmALIGNMENT_CENTER )
          stringX = x + fieldWidth/2 - stringWidth/2;
        else if ( alignment == XmALIGNMENT_END )
          stringX = x + fieldWidth - stringWidth;

        XDrawString( d, w, gc, stringX, y, &str[start], l );

        l = 0;

      }

      start = i+1;
      y += charHeight;

    }
    else {

      l++;

    }

    i++;

  }

  if ( l ) {

    if ( fs && *fs ) {
      stringWidth = XTextWidth( *fs, &str[start], l );
      charHeight = (*fs)->ascent + (*fs)->descent;
    }
    else {
      stringWidth = 10;
      charHeight = 2;
    }

    if ( alignment == XmALIGNMENT_BEGINNING )
      stringX = x;
    else if ( alignment == XmALIGNMENT_CENTER )
      stringX = x + fieldWidth/2 - stringWidth/2;
    else if ( alignment == XmALIGNMENT_END )
      stringX = x + fieldWidth - stringWidth;

    XDrawString( d, w, gc, stringX, y, &str[start], l );

  }

}

void XDrawImageStringsAligned (
  Display *d,
  Window w,
  GC gc,
  int x,
  int y,
  int fieldWidth,
  char *str,
  int len,
  XFontStruct **fs,
  int alignment )
{

int charHeight=0, i, start, l, strL, stringWidth, stringX=0;

  i = start = l = 0;
  strL = strlen( str );

  while ( i < strL ) {

    if ( ( str[i] == 1 ) || ( str[i] == 10 ) ) {

      if ( l ) {

        if ( fs && *fs ) {
          stringWidth = XTextWidth( *fs, &str[start], l );
          charHeight = (*fs)->ascent + (*fs)->descent;
	}
	else {
	  stringWidth = 10;
          charHeight = 2;
	}

        if ( alignment == XmALIGNMENT_BEGINNING )
          stringX = x;
        else if ( alignment == XmALIGNMENT_CENTER )
          stringX = x + fieldWidth/2 - stringWidth/2;
        else if ( alignment == XmALIGNMENT_END )
          stringX = x + fieldWidth - stringWidth;

        XDrawImageString( d, w, gc, stringX, y, &str[start], l );

        l = 0;

      }

      start = i+1;
      y += charHeight;

    }
    else {

      l++;

    }

    i++;

  }

  if ( l ) {

    if ( fs && *fs ) {
      stringWidth = XTextWidth( *fs, &str[start], l );
      charHeight = (*fs)->ascent + (*fs)->descent;
    }
    else {
      stringWidth = 10;
      charHeight = 2;
    }

    if ( alignment == XmALIGNMENT_BEGINNING )
      stringX = x;
    else if ( alignment == XmALIGNMENT_CENTER )
      stringX = x + fieldWidth/2 - stringWidth/2;
    else if ( alignment == XmALIGNMENT_END )
      stringX = x + fieldWidth - stringWidth;

    XDrawImageString( d, w, gc, stringX, y, &str[start], l );

  }

}

void XDrawImageStrings (
  Display *d,
  Window w,
  GC gc,
  int x,
  int y,
  int h,
  char *str,
  int len )
{

int i, start, l, strL;

  i = start = l = 0;
  strL = strlen( str );

  while ( i < strL ) {

    if ( ( str[i] == 1 ) || ( str[i] == 10 ) ) {

      if ( l ) {
        XDrawImageString( d, w, gc, x, y, &str[start], l );
        l = 0;
      }

      start = i+1;
      y += h;

    }
    else {

      l++;

    }

    i++;

  }

  if ( l ) {
    XDrawImageString( d, w, gc, x, y, &str[start], l );
  }

}

#if 0

// these routines consider an ascii 1 to mean new line

void XDrawStrings (
  Display *d,
  Window w,
  GC gc,
  int x,
  int y,
  int h,
  char *str,
  int len )
{

char buf[255+1], *tk, *context;

  strncpy( buf, str, 255 );

  context = NULL;
  tk = strtok_r( buf, "\001", &context );

  while ( tk ) {

    XDrawString( d, w, gc, x, y, tk, strlen(tk) );
    
    tk = strtok_r( NULL, "\001", &context );
    y += h;

  }

}

void XDrawImageStrings (
  Display *d,
  Window w,
  GC gc,
  int x,
  int y,
  int h,
  char *str,
  int len )
{

char buf[255+1], *tk, *context;

  strncpy( buf, str, 255 );

  context = NULL;
  tk = strtok_r( buf, "\001", &context );

  while ( tk ) {

    XDrawImageString( d, w, gc, x, y, tk, strlen(tk) );
    
    tk = strtok_r( NULL, "\001", &context );
    y += h;

  }

}

#endif

int countSymbolsAndValues (
  char *string,
  int *total,
  int *maxLen
) {

  // string contains s1=v1,s2=v2,s3=v3,...
  // this routine counts the number of symbol,value pairs
  // maxLen equals the longest string found

char *context, *tk, buf[511+1];

  if ( !string ) return 100; // fail

  strncpy( buf, string, 511 );
  buf[511] = 0;

  *maxLen = 0;
  *total = 0;
  context = NULL;
  tk = strtok_r( buf, ",=", &context );

  while ( tk ) {

    tk = strtok_r( NULL, ",=", &context );
    if ( !tk ) return 101; // missing value

    if ( strlen(tk) > (unsigned int) *maxLen ) *maxLen = (int) strlen(tk);
    (*total)++;

    tk = strtok_r( NULL, ",=", &context );

  }

  return 1;

}

int parseSymbolsAndValues (
  char *string,
  int max,
  char *symbols[],
  char *values[],
  int *numFound
) {

  // string contains s1=v1,s2=v2,s3=v3,...
  // this routine puts s1, s2, s3, ... into symbols
  // and v1, v2, v3, ... into values

int l;
char *context, *tk, buf[10000+1];

  if ( !string ) return 100; // fail

  l = strlen(string);
  if ( l > 10000 ) l = 10000;
  strncpy( buf, string, l );
  buf[l] = 0;

  *numFound = 0;
  context = NULL;
  tk = strtok_r( buf, ",=", &context );

  while ( tk && ( *numFound < max ) ) {

    l = strlen(tk) + 1;
    symbols[*numFound] = new char[l];
    strcpy( symbols[*numFound], tk );
    trimWhiteSpace( symbols[*numFound] );

    tk = strtok_r( NULL, ",=", &context );
    if ( !tk ) {
      return 101; // missing value
    }

    if ( strcmp( tk, "''" ) == 0 ) {
      l = 1;
      values[*numFound] = new char[l];
      strcpy( values[*numFound], "" );
    }
    else if ( strcmp( tk, "\\'\\'" ) == 0 ) {
      l = 1;
      values[*numFound] = new char[l];
      strcpy( values[*numFound], "" );
    }
    else if ( strcmp( tk, "\\\"\\\"" ) == 0 ) {
      l = 1;
      values[*numFound] = new char[l];
      strcpy( values[*numFound], "" );
    }
    else {
      l = strlen(tk) + 1;
      values[*numFound] = new char[l];
      strcpy( values[*numFound], tk );
    }

    (*numFound)++;

    tk = strtok_r( NULL, ",=", &context );

  }

  return 1;

}

int parseLocalSymbolsAndValues (
  char *string,
  int max,
  int maxLen,
  char *symbols[],
  char *values[],
  int *numFound
) {

  // string contains s1=v1,s2=v2,s3=v3,...
  // this routine puts s1, s2, s3, ... into symbols
  // and v1, v2, v3, ... into values

int l;
char *context, *tk, buf[10000+1];

  if ( !string ) return 100; // fail

  l = strlen(string);
  if ( l > 10000 ) l = 10000;
  strncpy( buf, string, l );
  buf[l] = 0;

  *numFound = 0;
  context = NULL;
  tk = strtok_r( buf, ",=", &context );

  while ( tk && ( *numFound < max ) ) {

    l = strlen(tk) + 1;
    strncpy( symbols[*numFound], tk, maxLen );
    symbols[*numFound][maxLen] = 0;
    trimWhiteSpace( symbols[*numFound] );

    tk = strtok_r( NULL, ",=", &context );
    if ( !tk ) return 101; // missing value

    if ( strcmp( tk, "''" ) == 0 ) {
      l = 1;
      values[*numFound] = new char[l];
      strcpy( values[*numFound], "" );
    }
    else if ( strcmp( tk, "\\'\\'" ) == 0 ) {
      l = 1;
      values[*numFound] = new char[l];
      strcpy( values[*numFound], "" );
    }
    else if ( strcmp( tk, "\\\"\\\"" ) == 0 ) {
      l = 1;
      values[*numFound] = new char[l];
      strcpy( values[*numFound], "" );
    }
    else {
      l = strlen(tk) + 1;
      values[*numFound] = new char[l];
      strcpy( values[*numFound], tk );
    }

    (*numFound)++;

    tk = strtok_r( NULL, ",=", &context );

  }

  return 1;

}

int get_scale_params1 (
  double min,
  double max,
  double *adj_min,
  double *adj_max,
  int *num_label_ticks,
  int *majors_per_label,
  int *minors_per_major,
  char *format
) {

double dmin, dmax, diff, mag, norm;

int imag, inorm, imin, imax, inc1, inc2, inc5, imin1, imax1,
 imin2, imax2, imin5, imax5, best, bestInc, bestMin, bestMax, idiff, idiv,
 choice, ok;

  dmin = min;
  dmax = max;

  if ( dmax <= dmin ) dmax = dmin + 1.0;

  diff = dmax - dmin;

  /* fprintf( stderr, "dmin = %-g, dmax = %-g, diff =  %-g\n", dmin, dmax, diff ); */

  mag = log10( diff );
  imag = (int ) floor( mag ) - 1;

  norm = diff * pow(10.0,-1.0*imag);
  inorm = (int) ceil( norm );

  //fprintf( stderr, "mag = %-g, imag = %-d\n", mag, imag );
  //fprintf( stderr, "norm = %-d\n", inorm );


  /* normalize min & max */
  imin = (int) floor( dmin * pow(10.0,-1.0*imag) );
  imax = (int) ceil( dmax * pow(10.0,-1.0*imag) );

  ok = 0;

  inc1 = imax - imin;
  imin1 = imin;
  imax1 = imax;
  if ( inc1 < 8 ) ok = 1;

  //fprintf( stderr, "1st adj min 1 = %-d, 1st adj max 1 = %-d\n", imin1, imax1 );

  if ( imin < 0 ) {
    if ( imin % 2 )
      imin2 = imin - 2 - ( imin % 2 );
    else
      imin2 = imin;
  }
  else {
    imin2 = imin - ( imin % 2 );
  }

  if ( imax < 0 ) {
    imax2 = imax + 2 + ( imax % 2 );
  }
  else {
    if ( imax % 2 )
      imax2 = imax + 2 - ( imax % 2 );
    else
      imax2 = imax;
  }

  inc2 = ( imax2 - imin2 ) / 2;
  if ( inc2 < 8 ) ok = 1;

  //fprintf( stderr, "1st adj min 2 = %-d, 1st adj max 2 = %-d\n", imin2, imax2 );

  if ( imin < 0 ) {
    if ( imin % 5 )
      imin5 = imin - 5 - ( imin % 5 );
    else
      imin5 = imin;
  }
  else {
    imin5 = imin - ( imin % 5 );
  }

  if ( imax < 0 ) {
    imax5 = imax + 5 + ( imax % 5 );
  }
  else {
    if ( imax % 5 )
      imax5 = imax + 5 - ( imax % 5 );
    else
      imax5 = imax;
  }

  inc5 = ( imax5 - imin5 ) / 5;
  if ( inc5 < 8 ) ok = 1;

  //fprintf( stderr, "1st adj min 5 = %-d, 1st adj max 5 = %-d\n", imin5, imax5 );

  //fprintf( stderr, "1 inc1 = %-d\n", inc1 );
  //fprintf( stderr, "1 inc2 = %-d\n", inc2 );
  //fprintf( stderr, "1 inc5 = %-d\n", inc5 );

  if ( ! ok ) {

    imag++;

    /* normalize min & max */
    imin = (int) floor( dmin * pow(10.0,-1.0*imag) );
    imax = (int) ceil( dmax * pow(10.0,-1.0*imag) );

    inc1 = imax - imin;
    imin1 = imin;
    imax1 = imax;
    if ( inc1 < 8 ) ok = 1;

    //fprintf( stderr, "1st adj min 1 = %-d, 1st adj max 1 = %-d\n", imin1, imax1 );

    if ( imin < 0 ) {
      if ( imin % 2 )
        imin2 = imin - 2 - ( imin % 2 );
      else
        imin2 = imin;
    }
    else {
      imin2 = imin - ( imin % 2 );
    }

    if ( imax < 0 ) {
      imax2 = imax + 2 + ( imax % 2 );
    }
    else {
      if ( imax % 2 )
        imax2 = imax + 2 - ( imax % 2 );
      else
        imax2 = imax;
    }

    inc2 = ( imax2 - imin2 ) / 2;
    if ( inc2 < 8 ) ok = 1;

    //fprintf( stderr, "1st adj min 2 = %-d, 1st adj max 2 = %-d\n", imin2, imax2 );

    if ( imin < 0 ) {
      if ( imin % 5 )
        imin5 = imin - 5 - ( imin % 5 );
      else
        imin5 = imin;
    }
    else {
      imin5 = imin - ( imin % 5 );
    }

    if ( imax < 0 ) {
      imax5 = imax + 5 + ( imax % 5 );
    }
    else {
      if ( imax % 5 )
        imax5 = imax + 5 - ( imax % 5 );
      else
        imax5 = imax;
    }

    inc5 = ( imax5 - imin5 ) / 5;
    if ( inc5 < 8 ) ok = 1;

    //fprintf( stderr, "1st adj min 5 = %-d, 1st adj max 5 = %-d\n", imin5, imax5 );

    //fprintf( stderr, "2 inc1 = %-d\n", inc1 );
    //fprintf( stderr, "2 inc2 = %-d\n", inc2 );
    //fprintf( stderr, "2 inc5 = %-d\n", inc5 );

  }

  // find best

  best = abs( inc1 - 6 );
  bestInc = inc1;
  bestMin = imin1;
  bestMax = imax1;
  idiv = inc1;
  choice = 1;

  if ( abs( inc2 - 6 ) < best ) {
    best = abs( inc2 - 6 );
    bestInc = inc2;
    bestMin = imin2;
    bestMax = imax2;
    idiv = inc2;
    choice = 2;
  }

  if ( abs( inc5 - 6 ) < best ) {
    best = abs( inc5 - 6 );
    bestInc = inc5;
    bestMin = imin5;
    bestMax = imax5;
    idiv = inc5;
    choice = 5;
  }

  idiff = ( bestMax - bestMin );
  *adj_min = (double) bestMin * pow(10.0,imag);
  *adj_max = (double) bestMax * pow(10.0,imag);

  *num_label_ticks = idiv;

  if ( choice == 1 ) {
    *majors_per_label = 5;
    *minors_per_major = 2;
  }
  else if ( choice == 2 ) {
    *majors_per_label = 2;
    *minors_per_major = 2;
  }
  else { // 5
    *majors_per_label = 5;
    *minors_per_major = 2;
  }

  strcpy( format, "-g" );

  if ( !ok ) return 0;

  return 1;

}

int get_log10_scale_params1 (
  double min,
  double max,
  double *adj_min,
  double *adj_max,
  int *num_label_ticks,
  int *majors_per_label,
  int *minors_per_major,
  char *format
) {

//double dmin, dmax, diff, mag, norm;

//int imag, inorm, imin, imax, inc1, inc2, inc5, imin1, imax1,
// imin2, imax2, imin5, imax5, best, bestInc, bestMin, bestMax, idiff, idiv,
// choice, ok, div;

int imin, imax, inc1, imin1, imax1,
 bestInc, bestMin, bestMax, idiff, idiv,
 choice, ok, div;

//fprintf( stderr, "\n\n=========================================================\n" );
//fprintf( stderr, "get_log10_scale_params1, min=%-g, max=%-g\n", min, max );

  div = 1;
  ok = 0;

  imin = (int) floor( min );
  imax = (int) ceil( max );

  do {

    if ( imin < 0 ) {
      if ( imin % div )
        imin1 = imin - div - ( imin % div );
      else
        imin1 = imin;
    }
    else {
      imin1 = imin - ( imin % div );
    }

    if ( imax < 0 ) {
      if ( imax % div )
        imax1 = imax + div + ( imax % div );
      else
        imax1 = imax;
    }
    else {
      if ( imax % div )
        imax1 = imax + div - ( imax % div );
      else
        imax1 = imax;
    }

    //fprintf( stderr, "imin = %-d, imax = %-d\n", imin, imax );
    //fprintf( stderr, "imin1 = %-d, imax1 = %-d\n", imin1, imax1 );
    //fprintf( stderr, "div = %-d\n", div );
    //fprintf( stderr, "imin %% div = %-d\n", imin % div );

    inc1 = ( imax1 - imin1 ) / div;
    if ( inc1 < 1 ) inc1 = 1;
    //if ( inc1 < 8 ) ok = 1;
    if ( inc1 <= 20 ) ok = 1;

    //fprintf( stderr, "1st adj min 1 = %-d, 1st adj max 1 = %-d\n", imin1, imax1 );
    //fprintf( stderr, "inc1 = %-d\n", inc1 );

    if ( !ok ) {
      div += 10;
      //printf( "not ok - div = %-d\n", div );
    }

  } while ( !ok );

  //fprintf( stderr, "2 inc1 = %-d\n", inc1 );

  bestInc = inc1;
  bestMin = imin1;
  bestMax = imax1;
  idiv = inc1;
  choice = 1;

  idiff = ( bestMax - bestMin );
  *adj_min = floor( (double) bestMin );
  *adj_max = ceil( (double) bestMax );

  *num_label_ticks = idiv;

  *majors_per_label = div;

  *minors_per_major = 9;

  strcpy( format, "-g" );

  return 1;

}

int get_scale_params (
  double min,
  double max,
  double *adj_min,
  double *adj_max,
  double *label_tick,
  int *majors_per_label,
  int *minors_per_major,
  char *format
) {

int num_label_ticks, stat;

  stat = get_scale_params1 (
   min, max, adj_min, adj_max,
   &num_label_ticks, majors_per_label,
   minors_per_major, format );

  if ( num_label_ticks < 1 ) num_label_ticks = 1;

  *label_tick = ( *adj_max - *adj_min ) /
   (double) num_label_ticks;

  return stat;

}

int formatString (
  double value,
  char *string,
  int len
) {

char buf[128];

  if ( !string ) return 0;
  if ( len < 1 ) return 0;

  snprintf( buf, 127, "%-g", value );
  buf[127] = 0;

  if ( strlen(buf) > 8 ) {
    snprintf( buf, 127, "%-3g", value );
    buf[127] = 0;
  }

  strncpy( string, buf, len );

  return 1;

}

int formatString (
  double value,
  char *string,
  int len,
  char *fmt
) {

char buf[128];

  if ( !string ) return 0;
  if ( len < 1 ) return 0;

  if ( !fmt ) {
    return formatString( value, string, len );
  }

  snprintf( buf, 127, fmt, value );
  buf[127] = 0;

  strncpy( string, buf, len );

  return 1;

}

static void updateFontInfo (
  char *string,
  char *fontTag,
  XFontStruct **fs,
  int *ascent,
  int *descent,
  int *height,
  int *width )
{

int l;

  if ( string )
    l = strlen(string);
  else
    l = 0;

  if ( fs && *fs ) {

    *ascent = (*fs)->ascent;
    *descent = (*fs)->descent;
    *height = *ascent + *descent;
    *width = XTextWidth( *fs, string, l );

  }
  else {

    *ascent = 10;
    *descent = 5;
    *height = *ascent + *descent;
    *width = 10;

  }

}

int xScaleHeight (
  char *fontTag,
  XFontStruct *fs
) {

int label_tick_height, fontAscent, fontDescent, fontHeight,
 stringWidth;

  updateFontInfo( " ", fontTag, &fs,
   &fontAscent, &fontDescent, &fontHeight,
   &stringWidth );

  label_tick_height = (int) ( 0.8 * (double) ( fontHeight - 2 ) );

  return fontHeight + label_tick_height * 2;

}

int xScaleMargin (
  char *fontTag,
  XFontStruct *fs,
  double adj_min,
  double adj_max
) {

int stat, scaleOfs, l;
char buf[31+1];
int stringWidth;

  stat = formatString( adj_min, buf, 31 );

  if ( fs ) {
    stringWidth = XTextWidth( fs, buf, strlen(buf) );
  }
  else {
    stringWidth = 0;
  }

  scaleOfs = stringWidth;

  stat = formatString( adj_max, buf, 31 );

  if ( fs ) {
    stringWidth = XTextWidth( fs, buf, strlen(buf) );
  }
  else {
    stringWidth = 0;
  }

  l = stringWidth;
  if ( l > scaleOfs ) scaleOfs = l;

  scaleOfs = scaleOfs / 2 + 6;

  return scaleOfs;

}

int xTimeScaleHeight (
  char *fontTag,
  XFontStruct *fs
) {

int label_tick_height, fontAscent, fontDescent, fontHeight,
 stringWidth;

  updateFontInfo( " ", fontTag, &fs,
   &fontAscent, &fontDescent, &fontHeight,
   &stringWidth );

  label_tick_height = (int) ( 0.8 * (double) ( fontHeight - 2 ) );

  return (int) ( fontHeight * 2.5 + label_tick_height * 2 );

}

int xTimeScaleMargin (
  char *fontTag,
  XFontStruct *fs,
  double adj_min,
  double adj_max
) {

int scaleOfs;
char buf[31+1];
int stringWidth;

  strcpy( buf, "00-00-0000" );

  if ( fs ) {
    stringWidth = XTextWidth( fs, buf, strlen(buf) );
  }
  else {
    stringWidth = 0;
  }

  scaleOfs = stringWidth;

  scaleOfs = scaleOfs / 2 + 6;

  return scaleOfs;

}

void drawXLinearTimeScale (
  Display *d,
  Window win,
  gcClass *gc,
  int drawScale,
  int x,
  int y,
  int scaleLen,
  time_t absolute_time,
  double adj_min,
  double adj_max,
  int time_format,
  int num_label_ticks,
  int majors_per_label,
  int minors_per_major,
  unsigned int scaleColor,
  unsigned int bgColor,
  int labelGrid,
  int majorGrid,
  int minorGrid,
  int gridHeight,
  unsigned int gridColor,
  fontInfoClass *fi,
  char *fontTag,
  XFontStruct *fs,
  int annotateScale,
  int minConstrained,
  int maxConstrained,
  int erase
) {

int count, firstLabel, ii, iii, x0, y0, x1, y1;
int label_tick_height, major_tick_height, minor_tick_height, first=0, ifrac;
double xFactor, xOffset, labelVal, majorInc, majorVal,
 minorInc, minorVal, lastInc, labelInc, seconds;
int fontAscent, fontDescent, fontHeight,
 stringWidth;
char buf1[31+1], buf2[31+1];
unsigned int white, black;

struct tm *t;
time_t theTime;

  if ( scaleLen < 1 ) return;
  if ( num_label_ticks < 1 ) return;
  if ( adj_max <= adj_min ) return;

  firstLabel = 1;

  white = WhitePixel( d, DefaultScreen(d) );
  black = BlackPixel( d, DefaultScreen(d) );

  gc->saveFg();
  gc->saveBg();

  gc->setLineWidth(1);
  gc->setLineStyle( LineSolid );
  gc->setFG( scaleColor );
  gc->setBG( bgColor );

  labelInc = ( adj_max - adj_min ) / num_label_ticks;
  if ( labelInc <= 0 ) labelInc = 1;

  xFactor = (double) ( scaleLen ) / ( adj_max - adj_min );
  xOffset = x;

  labelVal = adj_min;

  if ( drawScale ) {

  first = 1;

  updateFontInfo( " ", fontTag, &fs,
   &fontAscent, &fontDescent, &fontHeight,
   &stringWidth );

  /* draw axis */
  x0 = x;
  y0 = y;
  x1 = x0 + scaleLen;
  y1 = y0;
  if ( erase )
    XDrawLine( d, win, gc->eraseGC(), x0, y0, x1, y1 );
  else
    XDrawLine( d, win, gc->normGC(), x0, y0, x1, y1 );

  }
  else {

    fontHeight = 1;

  }

  label_tick_height = (int) ( 0.8 * (double) abs( fontHeight - 2 ) );
  major_tick_height = (int) ( 0.7 * ( double) label_tick_height );
  minor_tick_height = (int) ( ( double) label_tick_height * 0.4 );

  lastInc = labelInc * 0.5;

  /* draw label ticks */

  seconds = adj_min;

  count = 0;
  while ( labelVal < ( adj_max - lastInc ) ) {

    x0 = (int) rint( ( labelVal - adj_min ) * xFactor + xOffset );
    x1 = x0;
    y0 = y;
    y1 = y0 + label_tick_height;

    if ( labelGrid && count ) {
      if ( erase ) {
        XDrawLine( d, win, gc->eraseGC(), x0, y0, x1, y0-gridHeight );
      }
      else {
        gc->setFG( gridColor );
        XDrawLine( d, win, gc->normGC(), x0, y0, x1, y0-gridHeight );
        gc->setFG( scaleColor );
      }
    }
    count++;

    if ( drawScale ) {

    if ( erase )
      XDrawLine( d, win, gc->eraseGC(), x0, y0, x1, y1 );
    else
      XDrawLine( d, win, gc->normGC(), x0, y0, x1, y1 );

    if ( annotateScale ) {
      gc->setFontTag( fontTag, fi );
      y1 = y0 + (int) ( 1.2 * label_tick_height );

      theTime = absolute_time + (time_t) seconds;
      t = localtime( &theTime );

      ifrac = (int) rint( ( seconds - floor( seconds ) ) * 100.0 );

      if ( ifrac > 0.0 ) {
        sprintf( buf1, "%02d:%02d:%02d.%02d", t->tm_hour, t->tm_min,
         t->tm_sec, ifrac );
      }
      else {
        sprintf( buf1, "%02d:%02d:%02d", t->tm_hour, t->tm_min, t->tm_sec );
      }

      sprintf( buf2, "%02d-%02d-%04d", t->tm_mon+1, t->tm_mday,
       t->tm_year+1900 );

      seconds += labelInc;

      if ( minConstrained ) {
        if ( first ) {
          gc->setFG( black );
          gc->setBG( white );
        }
      }
      if ( erase ) {
        xEraseImageText( d, win, gc, fs, x0, y1,
         XmALIGNMENT_CENTER, buf1 );
        if ( firstLabel ) {
          firstLabel = 10; // ????????????????
          xEraseImageText( d, win, gc, fs, x0, y1+(int)(fontHeight),
           XmALIGNMENT_CENTER, buf2 );
	}
      }
      else {
        xDrawImageText( d, win, gc, fs, x0, y1,
         XmALIGNMENT_CENTER, buf1 );
        if ( firstLabel ) {
          firstLabel = 10; // ????????????????
          xDrawImageText( d, win, gc, fs, x0, y1+(int)(fontHeight),
           XmALIGNMENT_CENTER, buf2 );
	}
      }
      if ( minConstrained ) {
        if ( first ) {
          first = 0;
          gc->setFG( scaleColor );
          gc->setBG( bgColor );
        }
      }
    }

    }

    if ( majors_per_label > 0 ) {

      majorInc = labelInc / majors_per_label;
      if ( majorInc <= 0 ) majorInc = 1;
      majorVal = labelVal;
      for ( ii=0; ii<majors_per_label; ii++ ) {

        if ( ii > 0 ) {

          x0 = (int) rint( ( majorVal - adj_min ) * xFactor + xOffset );
          x1 = x0;
          y0 = y;
          y1 = y0 + major_tick_height;

          if ( majorGrid ) {
            if ( erase ) {
              XDrawLine( d, win, gc->eraseGC(), x0, y0, x1, y0-gridHeight );
            }
            else {
              gc->setFG( gridColor );
              XDrawLine( d, win, gc->normGC(), x0, y0, x1, y0-gridHeight );
              gc->setFG( scaleColor );
            }
          }

          if ( drawScale ) {

          if ( erase )
            XDrawLine( d, win, gc->eraseGC(), x0, y0, x1, y1 );
          else
            XDrawLine( d, win, gc->normGC(), x0, y0, x1, y1 );

	  }

        }

        if ( minors_per_major  > 0 ) {

          minorInc = majorInc / minors_per_major;
          if ( minorInc <= 0 ) minorInc = 1;
          minorVal = majorVal + minorInc;

          for ( iii=1; iii<minors_per_major; iii++ ) {

            x0 = (int) rint( ( minorVal - adj_min ) *
             xFactor + xOffset );
            x1 = x0;
            y0 = y;
            y1 = y0 + minor_tick_height;

            if ( minorGrid ) {
              gc->setLineStyle( LineOnOffDash );
              if ( erase ) {
                XDrawLine( d, win, gc->eraseGC(), x0, y0, x1, y0-gridHeight );
              }
              else {
                gc->setFG( gridColor );
                XDrawLine( d, win, gc->normGC(), x0, y0, x1, y0-gridHeight );
                gc->setFG( scaleColor );
              }
              gc->setLineStyle( LineSolid );
            }

            if ( drawScale ) {

            if ( erase )
              XDrawLine( d, win, gc->eraseGC(), x0, y0, x1, y1 );
            else
              XDrawLine( d, win, gc->normGC(), x0, y0, x1, y1 );

	    }

            minorVal += minorInc;

	  }

	}

        majorVal += majorInc;

      }

    }

    labelVal += labelInc;

  }

  // draw last label tick

  x0 = (int) rint( ( labelVal - adj_min ) * xFactor + xOffset );
  x1 = x0;
  y0 = y;
  y1 = y0 + label_tick_height;

  if ( drawScale ) {

  if ( erase )
    XDrawLine( d, win, gc->eraseGC(), x0, y0, x1, y1 );
  else
    XDrawLine( d, win, gc->normGC(), x0, y0, x1, y1 );

  if ( annotateScale ) {
    gc->setFontTag( fontTag, fi );
    y1 = y0 + (int) ( 1.2 * label_tick_height );

    theTime = absolute_time + (time_t) seconds;
    t = localtime( &theTime );

    ifrac = (int) rint( ( seconds - floor( seconds ) ) * 100.0 );

    if ( ifrac > 0.0 ) {
      sprintf( buf1, "%02d:%02d:%02d.%02d", t->tm_hour, t->tm_min,
       t->tm_sec, ifrac );
    }
    else {
      sprintf( buf1, "%02d:%02d:%02d", t->tm_hour, t->tm_min, t->tm_sec );
    }

    sprintf( buf2, "%02d-%02d-%04d", t->tm_mon+1, t->tm_mday,
     t->tm_year+1900 );

    absolute_time += (time_t) labelInc;

    if ( maxConstrained ) {
      gc->setFG( black );
      gc->setBG( white );
    }
    if ( erase ) {
      xEraseImageText( d, win, gc, fs, x0, y1,
       XmALIGNMENT_CENTER, buf1 );
      xEraseImageText( d, win, gc, fs, x0, y1+(int)(fontHeight),
       XmALIGNMENT_CENTER, buf2 );
    }
    else {
      xDrawImageText( d, win, gc, fs, x0, y1,
       XmALIGNMENT_CENTER, buf1 );
      xDrawImageText( d, win, gc, fs, x0, y1+(int)(fontHeight),
       XmALIGNMENT_CENTER, buf2 );
    }
    if ( maxConstrained ) {
     gc->setFG( scaleColor );
      gc->setBG( bgColor );
    }
  }

  }

  gc->restoreFg();
  gc->restoreBg();

}

void drawXLinearScale (
  Display *d,
  Window win,
  gcClass *gc,
  int drawScale,
  int x,
  int y,
  int scaleLen,
  double adj_min,
  double adj_max,
  int num_label_ticks,
  int majors_per_label,
  int minors_per_major,
  unsigned int scaleColor,
  unsigned int bgColor,
  int labelGrid,
  int majorGrid,
  int minorGrid,
  int gridHeight,
  unsigned int gridColor,
  fontInfoClass *fi,
  char *fontTag,
  XFontStruct *fs,
  int annotateScale,
  int minConstrained,
  int maxConstrained,
  int erase,
  char *fmt
) {

int count, ii, iii, x0, y0, x1, y1;
int label_tick_height, major_tick_height, minor_tick_height, first=0;
double xFactor, xOffset, labelVal, majorInc, majorVal,
 minorInc, minorVal, lastInc, labelInc, z;
int fontAscent, fontDescent, fontHeight,
 stringWidth;
char buf[31+1];
unsigned int white, black;
int reverse = 0;

  if ( adj_min > adj_max ) {
    adj_min *= -1;
    adj_max *= -1;
    reverse = 1;
  }

  if ( scaleLen < 1 ) return;
  if ( num_label_ticks < 1 ) return;
  if ( adj_max == adj_min ) return;

  white = WhitePixel( d, DefaultScreen(d) );
  black = BlackPixel( d, DefaultScreen(d) );

  gc->saveFg();
  gc->saveBg();

  gc->setLineWidth(1);
  gc->setLineStyle( LineSolid );
  gc->setFG( scaleColor );
  gc->setBG( bgColor );

  labelInc = ( adj_max - adj_min ) / num_label_ticks;
  if ( labelInc <= 0 ) labelInc = 1;

  xFactor = (double) ( scaleLen ) / ( adj_max - adj_min );
  xOffset = x;

  labelVal = adj_min;

  if ( drawScale ) {

  first = 1;

  updateFontInfo( " ", fontTag, &fs,
   &fontAscent, &fontDescent, &fontHeight,
   &stringWidth );

  /* draw axis */
  x0 = x;
  y0 = y;
  x1 = x0 + scaleLen;
  y1 = y0;
  if ( erase )
    XDrawLine( d, win, gc->eraseGC(), x0, y0, x1, y1 );
  else
    XDrawLine( d, win, gc->normGC(), x0, y0, x1, y1 );

  }
  else {

    fontHeight = 1;

  }

  label_tick_height = (int) ( 0.8 * (double) abs( fontHeight - 2 ) );
  major_tick_height = (int) ( 0.7 * ( double) label_tick_height );
  minor_tick_height = (int) ( ( double) label_tick_height * 0.4 );

  lastInc = labelInc * 0.5;

  /* draw label ticks */

  count = 0;
  while ( labelVal < ( adj_max - lastInc ) ) {

    x0 = (int) rint( ( labelVal - adj_min ) * xFactor + xOffset );
    x1 = x0;
    y0 = y;
    y1 = y0 + label_tick_height;

    if ( labelGrid && count ) {
      if ( erase ) {
        XDrawLine( d, win, gc->eraseGC(), x0, y0, x1, y0-gridHeight );
      }
      else {
        gc->setFG( gridColor );
        XDrawLine( d, win, gc->normGC(), x0, y0, x1, y0-gridHeight );
        gc->setFG( scaleColor );
      }
    }
    count++;

    if ( drawScale ) {

    if ( erase )
      XDrawLine( d, win, gc->eraseGC(), x0, y0, x1, y1 );
    else
      XDrawLine( d, win, gc->normGC(), x0, y0, x1, y1 );

    if ( annotateScale ) {
      gc->setFontTag( fontTag, fi );
      y1 = y0 + (int) ( 1.2 * label_tick_height );
      z = fabs( labelVal - 0.0 ) / labelInc;
      if ( z < 1e-5 ) {
        if ( fmt ) {
          formatString( 0.0, buf, 31, fmt );
	}
	else {
          formatString( 0.0, buf, 31 );
	}
      }
      else {
	if ( reverse ) {
          if ( fmt ) {
            formatString( -1*labelVal, buf, 31, fmt );
	  }
	  else {
            formatString( -1*labelVal, buf, 31 );
	  }
	}
	else {
          if ( fmt ) {
            formatString( labelVal, buf, 31, fmt );
	  }
	  else {
            formatString( labelVal, buf, 31 );
	  }
	}
      }
      if ( minConstrained ) {
        if ( first ) {
          gc->setFG( black );
          gc->setBG( white );
        }
      }
      if ( erase )
        xEraseImageText( d, win, gc, fs, x0, y1,
         XmALIGNMENT_CENTER, buf );
      else
        xDrawImageText( d, win, gc, fs, x0, y1,
         XmALIGNMENT_CENTER, buf );
      if ( minConstrained ) {
        if ( first ) {
          first = 0;
          gc->setFG( scaleColor );
          gc->setBG( bgColor );
        }
      }
    }

    }

    if ( majors_per_label > 0 ) {

      majorInc = labelInc / majors_per_label;
      if ( majorInc <= 0 ) majorInc = 1;
      majorVal = labelVal;
      for ( ii=0; ii<majors_per_label; ii++ ) {

        if ( ii > 0 ) {

          x0 = (int) rint( ( majorVal - adj_min ) * xFactor + xOffset );
          x1 = x0;
          y0 = y;
          y1 = y0 + major_tick_height;

          if ( majorGrid ) {
            if ( erase ) {
              XDrawLine( d, win, gc->eraseGC(), x0, y0, x1, y0-gridHeight );
            }
            else {
              gc->setFG( gridColor );
              XDrawLine( d, win, gc->normGC(), x0, y0, x1, y0-gridHeight );
              gc->setFG( scaleColor );
            }
          }

          if ( drawScale ) {

          if ( erase )
            XDrawLine( d, win, gc->eraseGC(), x0, y0, x1, y1 );
          else
            XDrawLine( d, win, gc->normGC(), x0, y0, x1, y1 );

	  }

        }

        if ( minors_per_major  > 0 ) {

          minorInc = majorInc / minors_per_major;
          if ( minorInc <= 0 ) minorInc = 1;
          minorVal = majorVal + minorInc;

          for ( iii=1; iii<minors_per_major; iii++ ) {

            x0 = (int) rint( ( minorVal - adj_min ) *
             xFactor + xOffset );
            x1 = x0;
            y0 = y;
            y1 = y0 + minor_tick_height;

            if ( minorGrid ) {
              gc->setLineStyle( LineOnOffDash );
              if ( erase ) {
                XDrawLine( d, win, gc->eraseGC(), x0, y0, x1, y0-gridHeight );
              }
              else {
                gc->setFG( gridColor );
                XDrawLine( d, win, gc->normGC(), x0, y0, x1, y0-gridHeight );
                gc->setFG( scaleColor );
              }
              gc->setLineStyle( LineSolid );
            }

            if ( drawScale ) {

            if ( erase )
              XDrawLine( d, win, gc->eraseGC(), x0, y0, x1, y1 );
            else
              XDrawLine( d, win, gc->normGC(), x0, y0, x1, y1 );

	    }

            minorVal += minorInc;

	  }

	}

        majorVal += majorInc;

      }

    }

    labelVal += labelInc;

  }

  // draw last label tick

  x0 = (int) rint( ( labelVal - adj_min ) * xFactor + xOffset );
  x1 = x0;
  y0 = y;
  y1 = y0 + label_tick_height;

#if 0
  if ( labelGrid ) {
    if ( erase ) {
      XDrawLine( d, win, gc->eraseGC(), x0, y0, x1, y0-gridHeight );
    }
    else {
      gc->setFG( gridColor );
      XDrawLine( d, win, gc->normGC(), x0, y0, x1, y0-gridHeight );
      gc->setFG( scaleColor );
    }
  }
#endif

  if ( drawScale ) {

  if ( erase )
    XDrawLine( d, win, gc->eraseGC(), x0, y0, x1, y1 );
  else
    XDrawLine( d, win, gc->normGC(), x0, y0, x1, y1 );

  if ( annotateScale ) {
    gc->setFontTag( fontTag, fi );
    y1 = y0 + (int) ( 1.2 * label_tick_height );
    z = fabs( labelVal - 0.0 ) / labelInc;
    if ( z < 1e-5 ) {
      if ( fmt ) {
        formatString( 0.0, buf, 31, fmt );
      }
      else {
        formatString( 0.0, buf, 31 );
      }
    }
    else {
      if ( reverse ) {
        if ( fmt ) {
          formatString( -1*labelVal, buf, 31, fmt );
	}
	else {
          formatString( -1*labelVal, buf, 31 );
	}
      }
      else {
        if ( fmt ) {
          formatString( labelVal, buf, 31, fmt );
	}
	else {
          formatString( labelVal, buf, 31 );
	}
      }
    }
    if ( maxConstrained ) {
      gc->setFG( black );
      gc->setBG( white );
    }
    if ( erase )
      xEraseImageText( d, win, gc, fs, x0, y1,
       XmALIGNMENT_CENTER, buf );
    else
      xDrawImageText( d, win, gc, fs, x0, y1,
       XmALIGNMENT_CENTER, buf );
    if ( maxConstrained ) {
     gc->setFG( scaleColor );
      gc->setBG( bgColor );
    }
  }

  }

  gc->restoreFg();
  gc->restoreBg();

}

void drawXLinearScale (
  Display *d,
  Window win,
  gcClass *gc,
  int drawScale,
  int x,
  int y,
  int scaleLen,
  double adj_min,
  double adj_max,
  int num_label_ticks,
  int majors_per_label,
  int minors_per_major,
  unsigned int scaleColor,
  unsigned int bgColor,
  int labelGrid,
  int majorGrid,
  int minorGrid,
  int gridHeight,
  unsigned int gridColor,
  fontInfoClass *fi,
  char *fontTag,
  XFontStruct *fs,
  int annotateScale,
  int minConstrained,
  int maxConstrained,
  int erase
) {

  drawXLinearScale (
   d,
   win,
   gc,
   drawScale,
   x,
   y,
   scaleLen,
   adj_min,
   adj_max,
   num_label_ticks,
   majors_per_label,
   minors_per_major,
   scaleColor,
   bgColor,
   labelGrid,
   majorGrid,
   minorGrid,
   gridHeight,
   gridColor,
   fi,
   fontTag,
   fs,
   annotateScale,
   minConstrained,
   maxConstrained,
   erase,
   NULL );

}

void drawXLinearScale2 (
  Display *d,
  Window win,
  gcClass *gc,
  int drawScale,
  int x,
  int y,
  int scaleLen,
  double cur_min,
  double cur_max,
  double adj_min,
  double adj_max,
  int num_label_ticks,
  int majors_per_label,
  int minors_per_major,
  unsigned int scaleColor,
  unsigned int bgColor,
  int labelGrid,
  int majorGrid,
  int minorGrid,
  int gridHeight,
  unsigned int gridColor,
  fontInfoClass *fi,
  char *fontTag,
  XFontStruct *fs,
  int annotateScale,
  int minConstrained,
  int maxConstrained,
  int erase
) {

int count, ii, iii, x0, y0, x1, y1;
int label_tick_height, major_tick_height, minor_tick_height, first=0;
double xFactor, xOffset, adjXOffset, labelVal, lastLabelVal, majorInc,
 majorVal, minorInc, minorVal, lastInc, labelInc, z;
int fontAscent, fontDescent, fontHeight,
 stringWidth;
char buf[31+1];
unsigned int white, black;
int reverse = 0;

  if ( adj_min > adj_max ) {
    adj_min *= -1;
    adj_max *= -1;
    reverse = 1;
  }

  if ( scaleLen < 1 ) return;
  if ( num_label_ticks < 1 ) return;
  if ( adj_max == adj_min ) return;

  white = WhitePixel( d, DefaultScreen(d) );
  black = BlackPixel( d, DefaultScreen(d) );

  gc->saveFg();
  gc->saveBg();

  gc->setLineWidth(1);
  gc->setLineStyle( LineSolid );
  gc->setFG( scaleColor );
  gc->setBG( bgColor );

  xFactor = (double) ( scaleLen ) / ( cur_max - cur_min );
  xOffset = x;

  adjXOffset = (int) rint( ( adj_min - cur_min ) * xFactor + xOffset );
  xOffset = adjXOffset;

  labelInc = ( adj_max - adj_min ) / num_label_ticks;
  if ( labelInc <= 0 ) labelInc = 1;

  lastLabelVal = labelVal = adj_min;

  if ( drawScale ) {

  first = 1;

  updateFontInfo( " ", fontTag, &fs,
   &fontAscent, &fontDescent, &fontHeight,
   &stringWidth );

  /* draw axis */
  x0 = x;
  y0 = y;
  x1 = x0 + scaleLen;
  y1 = y0;
  if ( erase )
    XDrawLine( d, win, gc->eraseGC(), x0, y0, x1, y1 );
  else
    XDrawLine( d, win, gc->normGC(), x0, y0, x1, y1 );

  }
  else {

    fontHeight = 1;

  }

  label_tick_height = (int) ( 0.8 * (double) abs( fontHeight - 2 ) );
  major_tick_height = (int) ( 0.7 * ( double) label_tick_height );
  minor_tick_height = (int) ( ( double) label_tick_height * 0.4 );

  lastInc = labelInc * 0.5;

  /* draw label ticks */

  count = 0;
  while ( labelVal < ( adj_max + lastInc ) ) {

    x0 = (int) rint( ( labelVal - adj_min ) * xFactor + xOffset );
    x1 = x0;
    y0 = y;
    y1 = y0 + label_tick_height;

    if ( ( x0 >= x ) && ( x0 <= ( x + scaleLen ) ) ) {

      if ( labelGrid && count ) {
        if ( erase ) {
          XDrawLine( d, win, gc->eraseGC(), x0, y0, x1, y0-gridHeight );
        }
        else {
          gc->setFG( gridColor );
          XDrawLine( d, win, gc->normGC(), x0, y0, x1, y0-gridHeight );
          gc->setFG( scaleColor );
        }
      }

      if ( drawScale ) {

        lastLabelVal = labelVal;

        if ( erase ) {
          XDrawLine( d, win, gc->eraseGC(), x0, y0, x1, y1 );
	}
        else {
          XDrawLine( d, win, gc->normGC(), x0, y0, x1, y1 );
	}
  
        if ( annotateScale ) {
          gc->setFontTag( fontTag, fi );
          y1 = y0 + (int) ( 1.2 * label_tick_height );
          z = fabs( labelVal - 0.0 ) / labelInc;
          if ( z < 1e-5 ) {
            formatString( 0.0, buf, 31 );
          }
          else {
    	    if ( reverse ) {
              formatString( -1*labelVal, buf, 31 );
    	    }
    	    else {
              formatString( labelVal, buf, 31 );
    	    }
          }
          if ( minConstrained ) {
            if ( first ) {
              gc->setFG( black );
              gc->setBG( white );
            }
          }
          if ( erase ) {
            xEraseImageText( d, win, gc, fs, x0, y1,
             XmALIGNMENT_CENTER, buf );
	  }
          else {
            xDrawImageText( d, win, gc, fs, x0, y1,
             XmALIGNMENT_CENTER, buf );
	  }
          if ( minConstrained ) {
            if ( first ) {
              first = 0;
              gc->setFG( scaleColor );
              gc->setBG( bgColor );
            }
          }
        }

      }

    }
    count++;

    if ( majors_per_label > 0 ) {

      majorInc = labelInc / majors_per_label;
      if ( majorInc <= 0 ) majorInc = 1;
      majorVal = labelVal;
      for ( ii=0; ii<majors_per_label; ii++ ) {

        if ( ii > 0 ) {

          x0 = (int) rint( ( majorVal - adj_min ) * xFactor + xOffset );
          x1 = x0;
          y0 = y;
          y1 = y0 + major_tick_height;

          if ( ( x0 >= x ) && ( x0 <= ( x + scaleLen ) ) ) {

            if ( majorGrid ) {
              if ( erase ) {
                XDrawLine( d, win, gc->eraseGC(), x0, y0, x1, y0-gridHeight );
              }
              else {
                gc->setFG( gridColor );
                XDrawLine( d, win, gc->normGC(), x0, y0, x1, y0-gridHeight );
                gc->setFG( scaleColor );
              }
            }

            if ( drawScale ) {

              if ( erase ) {
                XDrawLine( d, win, gc->eraseGC(), x0, y0, x1, y1 );
              }
              else {
                XDrawLine( d, win, gc->normGC(), x0, y0, x1, y1 );
	      }

            }

	  }

	}

        if ( minors_per_major  > 0 ) {

          minorInc = majorInc / minors_per_major;
          if ( minorInc <= 0 ) minorInc = 1;
          minorVal = majorVal + minorInc;

          for ( iii=1; iii<minors_per_major; iii++ ) {

            x0 = (int) rint( ( minorVal - adj_min ) *
             xFactor + xOffset );
            x1 = x0;
            y0 = y;
            y1 = y0 + minor_tick_height;

            if ( ( x0 >= x ) && ( x0 <= ( x + scaleLen ) ) ) {

              if ( minorGrid ) {
                gc->setLineStyle( LineOnOffDash );
                if ( erase ) {
                  XDrawLine( d, win, gc->eraseGC(), x0, y0, x1, y0-gridHeight );
                }
                else {
                  gc->setFG( gridColor );
                  XDrawLine( d, win, gc->normGC(), x0, y0, x1, y0-gridHeight );
                  gc->setFG( scaleColor );
                }
                gc->setLineStyle( LineSolid );
              }

              if ( drawScale ) {

		if ( erase ) {
		  XDrawLine( d, win, gc->eraseGC(), x0, y0, x1, y1 );
		}
                else {
                  XDrawLine( d, win, gc->normGC(), x0, y0, x1, y1 );
		}

              }

	    }

            minorVal += minorInc;

	  }

	}

        majorVal += majorInc;

      }

    }

    labelVal += labelInc;

  }

  labelVal = lastLabelVal;

  // draw last label tick

  x0 = (int) rint( ( labelVal - adj_min ) * xFactor + xOffset );
  x1 = x0;
  y0 = y;
  y1 = y0 + label_tick_height;

  if ( ( x0 >= x ) && ( x0 <= ( x + scaleLen ) ) ) {

#if 0
    if ( labelGrid ) {
      if ( erase ) {
        XDrawLine( d, win, gc->eraseGC(), x0, y0, x1, y0-gridHeight );
      }
      else {
        gc->setFG( gridColor );
        XDrawLine( d, win, gc->normGC(), x0, y0, x1, y0-gridHeight );
        gc->setFG( scaleColor );
      }
    }
#endif

    if ( drawScale ) {

      if ( erase )
        XDrawLine( d, win, gc->eraseGC(), x0, y0, x1, y1 );
      else
        XDrawLine( d, win, gc->normGC(), x0, y0, x1, y1 );

      if ( annotateScale ) {
        gc->setFontTag( fontTag, fi );
        y1 = y0 + (int) ( 1.2 * label_tick_height );
        z = fabs( labelVal - 0.0 ) / labelInc;
        if ( z < 1e-5 ) {
          formatString( 0.0, buf, 31 );
        }
        else {
          if ( reverse ) {
            formatString( -1*labelVal, buf, 31 );
          }
          else {
            formatString( labelVal, buf, 31 );
          }
        }
        if ( maxConstrained ) {
          gc->setFG( black );
          gc->setBG( white );
        }
        if ( erase )
          xEraseImageText( d, win, gc, fs, x0, y1,
           XmALIGNMENT_CENTER, buf );
        else
          xDrawImageText( d, win, gc, fs, x0, y1,
           XmALIGNMENT_CENTER, buf );
        if ( maxConstrained ) {
         gc->setFG( scaleColor );
          gc->setBG( bgColor );
        }
      }

    }

  }

  gc->restoreFg();
  gc->restoreBg();

}

void drawXLog10Scale (
  Display *d,
  Window win,
  gcClass *gc,
  int drawScale,
  int x,
  int y,
  int scaleLen,
  double adj_min,
  double adj_max,
  int num_label_ticks,
  int majors_per_label,
  int minors_per_major,
  unsigned int scaleColor,
  unsigned int bgColor,
  int labelGrid,
  int majorGrid,
  int minorGrid,
  int gridHeight,
  unsigned int gridColor,
  fontInfoClass *fi,
  char *fontTag,
  XFontStruct *fs,
  int annotateScale,
  int minConstrained,
  int maxConstrained,
  int erase
) {

int count, ii, iii, x0, y0, x1, y1;
int label_tick_height, major_tick_height, minor_tick_height, first=0;
double xFactor, xOffset, labelVal, majorInc, majorVal,
 minorInc, minorVal, lastInc, labelInc, val, val0, val1;
int fontAscent, fontDescent, fontHeight,
 stringWidth;
char buf[31+1];
unsigned int white, black;

  if ( scaleLen < 1 ) return;
  if ( num_label_ticks < 1 ) return;
  if ( adj_max <= adj_min ) return;

  white = WhitePixel( d, DefaultScreen(d) );
  black = BlackPixel( d, DefaultScreen(d) );

  gc->saveFg();
  gc->saveBg();

  gc->setLineWidth(1);
  gc->setLineStyle( LineSolid );
  gc->setFG( scaleColor );
  gc->setBG( bgColor );

  labelInc = ( adj_max - adj_min ) / num_label_ticks;
  if ( labelInc <= 0 ) labelInc = 1;

  xFactor = (double) ( scaleLen ) / ( adj_max - adj_min );
  xOffset = x;

  labelVal = adj_min;

  if ( drawScale ) {

  first = 1;

  updateFontInfo( " ", fontTag, &fs,
   &fontAscent, &fontDescent, &fontHeight,
   &stringWidth );

  /* draw axis */
  x0 = x;
  y0 = y;
  x1 = x0 + scaleLen;
  y1 = y0;
  if ( erase )
    XDrawLine( d, win, gc->eraseGC(), x0, y0, x1, y1 );
  else
    XDrawLine( d, win, gc->normGC(), x0, y0, x1, y1 );

  }
  else {

    fontHeight = 1;

  }

  label_tick_height = (int) ( 0.8 * (double) abs( fontHeight - 2 ) );
  major_tick_height = (int) ( 0.7 * ( double) label_tick_height );
  minor_tick_height = (int) ( ( double) label_tick_height * 0.4 );

  lastInc = labelInc * 0.5;

  /* draw label ticks */

  count = 0;
  while ( labelVal < ( adj_max - lastInc ) ) {

    x0 = (int) rint( ( labelVal - adj_min ) * xFactor + xOffset );
    x1 = x0;
    y0 = y;
    y1 = y0 + label_tick_height;

    if ( labelGrid && count ) {
      if ( erase ) {
        XDrawLine( d, win, gc->eraseGC(), x0, y0, x1, y0-gridHeight );
      }
      else {
        gc->setFG( gridColor );
        XDrawLine( d, win, gc->normGC(), x0, y0, x1, y0-gridHeight );
        gc->setFG( scaleColor );
      }
    }
    count++;

    if ( drawScale ) {

    if ( erase )
      XDrawLine( d, win, gc->eraseGC(), x0, y0, x1, y1 );
    else
      XDrawLine( d, win, gc->normGC(), x0, y0, x1, y1 );

    if ( annotateScale ) {
      gc->setFontTag( fontTag, fi );
      y1 = y0 + (int) ( 1.2 * label_tick_height );
      formatString( pow(10,labelVal), buf, 31 );
      if ( minConstrained ) {
        if ( first ) {
          gc->setFG( black );
          gc->setBG( white );
        }
      }
      if ( erase )
        xEraseImageText( d, win, gc, fs, x0, y1,
         XmALIGNMENT_CENTER, buf );
      else
        xDrawImageText( d, win, gc, fs, x0, y1,
         XmALIGNMENT_CENTER, buf );
      if ( minConstrained ) {
        if ( first ) {
          first = 0;
          gc->setFG( scaleColor );
          gc->setBG( bgColor );
        }
      }
    }

    }

    if ( majors_per_label > 0 ) {

      majorInc = labelInc / majors_per_label;
      if ( majorInc <= 0 ) majorInc = 1;
      majorVal = labelVal;
      for ( ii=0; ii<majors_per_label; ii++ ) {

        if ( ii > 0 ) {

          x0 = (int) rint( ( majorVal - adj_min ) * xFactor + xOffset );
          x1 = x0;
          y0 = y;
          y1 = y0 + major_tick_height;

          if ( majorGrid ) {
            if ( erase ) {
              XDrawLine( d, win, gc->eraseGC(), x0, y0, x1, y0-gridHeight );
            }
            else {
              gc->setFG( gridColor );
              XDrawLine( d, win, gc->normGC(), x0, y0, x1, y0-gridHeight );
              gc->setFG( scaleColor );
            }
          }

          if ( drawScale ) {

          if ( erase )
            XDrawLine( d, win, gc->eraseGC(), x0, y0, x1, y1 );
          else
            XDrawLine( d, win, gc->normGC(), x0, y0, x1, y1 );

	  }

        }

        if ( minors_per_major  > 0 ) {

          val0 = pow( 10, majorVal );
          val1 = val0 * 10;
          minorInc = ( val1 - val0 ) / minors_per_major;
          if ( minorInc <= 0 ) minorInc = 1;
          val = val0 + minorInc;

          for ( iii=1; iii<minors_per_major; iii++ ) {

            minorVal = log10( val );

            x0 = (int) rint( ( minorVal - adj_min ) *
             xFactor + xOffset );
            x1 = x0;
            y0 = y;
            y1 = y0 + minor_tick_height;

            if ( minorGrid ) {
              gc->setLineStyle( LineOnOffDash );
              if ( erase ) {
                XDrawLine( d, win, gc->eraseGC(), x0, y0, x1, y0-gridHeight );
              }
              else {
                gc->setFG( gridColor );
                XDrawLine( d, win, gc->normGC(), x0, y0, x1, y0-gridHeight );
                gc->setFG( scaleColor );
              }
              gc->setLineStyle( LineSolid );
            }

            if ( drawScale ) {

            if ( erase )
              XDrawLine( d, win, gc->eraseGC(), x0, y0, x1, y1 );
            else
              XDrawLine( d, win, gc->normGC(), x0, y0, x1, y1 );

	    }

            val += minorInc;

	  }

	}

        majorVal += majorInc;

      }

    }

    labelVal += labelInc;

  }

  // draw last label tick

  x0 = (int) rint( ( labelVal - adj_min ) * xFactor + xOffset );
  x1 = x0;
  y0 = y;
  y1 = y0 + label_tick_height;

#if 0
  if ( labelGrid ) {
    if ( erase ) {
      XDrawLine( d, win, gc->eraseGC(), x0, y0, x1, y0-gridHeight );
    }
    else {
      gc->setFG( gridColor );
      XDrawLine( d, win, gc->normGC(), x0, y0, x1, y0-gridHeight );
      gc->setFG( scaleColor );
    }
  }
#endif

  if ( drawScale ) {

  if ( erase )
    XDrawLine( d, win, gc->eraseGC(), x0, y0, x1, y1 );
  else
    XDrawLine( d, win, gc->normGC(), x0, y0, x1, y1 );

  if ( annotateScale ) {
    gc->setFontTag( fontTag, fi );
    y1 = y0 + (int) ( 1.2 * label_tick_height );
    formatString( pow(10,labelVal), buf, 31 );
    if ( maxConstrained ) {
      gc->setFG( black );
      gc->setBG( white );
    }
    if ( erase )
      xEraseImageText( d, win, gc, fs, x0, y1,
       XmALIGNMENT_CENTER, buf );
    else
      xDrawImageText( d, win, gc, fs, x0, y1,
       XmALIGNMENT_CENTER, buf );
    if ( maxConstrained ) {
     gc->setFG( scaleColor );
      gc->setBG( bgColor );
    }
  }

  }

  gc->restoreFg();
  gc->restoreBg();

}

void getXLimitCoords (
  int x,
  int y,
  int scaleLen,
  double adj_min,
  double adj_max,
  int num_label_ticks,
  char *fontTag,
  XFontStruct *fs,
  int *xMinX0,
  int *xMinX1,
  int *xMinY0,
  int *xMinY1,
  int *xMaxX0,
  int *xMaxX1,
  int *xMaxY0,
  int *xMaxY1
) {

int x0, y0, x1, y1, count;
int label_tick_height;
double xFactor, xOffset, labelVal, lastInc, labelInc, z;
int fontAscent, fontDescent, fontHeight, stringWidth;
char buf[31+1];

  if ( scaleLen < 1 ) return;
  if ( num_label_ticks < 1 ) return;
  if ( adj_max <= adj_min ) return;

  labelInc = ( adj_max - adj_min ) / num_label_ticks;
  if ( labelInc <= 0 ) labelInc = 1;

  xFactor = (double) ( scaleLen ) / ( adj_max - adj_min );
  xOffset = x;

  labelVal = adj_min;

  updateFontInfo( " ", fontTag, &fs,
   &fontAscent, &fontDescent, &fontHeight,
   &stringWidth );

  label_tick_height = (int) ( 0.8 * (double) abs( fontHeight - 2 ) );
  x0 = x;
  x1 = x0;
  y0 = y;
  y1 = y0 + (int) ( 1.2 * label_tick_height );

  lastInc = labelInc * 0.5;

  count = 0;
  while ( labelVal < ( adj_max + lastInc ) ) {

    z = fabs( labelVal - 0.0 ) / labelInc;
    if ( z < 1e-5 ) {
      formatString( 0.0, buf, 31 );
    }
    else {
      formatString( labelVal, buf, 31 );
    }

    if ( fs ) {
      stringWidth = XTextWidth( fs, buf, strlen(buf) );
    }
    else {
      stringWidth = 0;
    }

    if ( count == 0 ) {
      *xMinX0 = (int) ( x0 - stringWidth * 0.5 );
      *xMinX1 = (int) ( x0 + stringWidth * 0.5 );
      *xMinY0 = y1;
      *xMinY1 = y1 + fontHeight;
    }
    else {
      *xMaxX0 = (int) ( x0 - stringWidth * 0.5 );
      *xMaxX1 = (int) ( x0 + stringWidth * 0.5 );
      *xMaxY0 = y1;
      *xMaxY1 = y1 + fontHeight;
    }

    count++;

    labelVal += labelInc;
    x0 = x1 = (int) rint( ( labelVal - adj_min ) * xFactor + xOffset );

  }

}

void getXLog10LimitCoords (
  int x,
  int y,
  int scaleLen,
  double adj_min,
  double adj_max,
  int num_label_ticks,
  char *fontTag,
  XFontStruct *fs,
  int *xMinX0,
  int *xMinX1,
  int *xMinY0,
  int *xMinY1,
  int *xMaxX0,
  int *xMaxX1,
  int *xMaxY0,
  int *xMaxY1
) {

int x0, y0, x1, y1, count;
int label_tick_height;
double xFactor, xOffset, labelVal, lastInc, labelInc;
int fontAscent, fontDescent, fontHeight, stringWidth;
char buf[31+1];

  if ( scaleLen < 1 ) return;
  if ( num_label_ticks < 1 ) return;
  if ( adj_max <= adj_min ) return;

  labelInc = ( adj_max - adj_min ) / num_label_ticks;
  if ( labelInc <= 0 ) labelInc = 1;

  xFactor = (double) ( scaleLen ) / ( adj_max - adj_min );
  xOffset = x;

  labelVal = adj_min;

  updateFontInfo( " ", fontTag, &fs,
   &fontAscent, &fontDescent, &fontHeight,
   &stringWidth );

  label_tick_height = (int) ( 0.8 * (double) abs( fontHeight - 2 ) );
  x0 = x;
  x1 = x0;
  y0 = y;
  y1 = y0 + (int) ( 1.2 * label_tick_height );

  lastInc = labelInc * 0.5;

  count = 0;
  while ( labelVal < ( adj_max + lastInc ) ) {

    formatString( pow(10,labelVal), buf, 31 );

    if ( fs ) {
      stringWidth = XTextWidth( fs, buf, strlen(buf) );
    }
    else {
      stringWidth = 0;
    }

    if ( count == 0 ) {
      *xMinX0 = (int) ( x0 - stringWidth * 0.5 );
      *xMinX1 = (int) ( x0 + stringWidth * 0.5 );
      *xMinY0 = y1;
      *xMinY1 = y1 + fontHeight;
    }
    else {
      *xMaxX0 = (int) ( x0 - stringWidth * 0.5 );
      *xMaxX1 = (int) ( x0 + stringWidth * 0.5 );
      *xMaxY0 = y1;
      *xMaxY1 = y1 + fontHeight;
    }

    count++;

    labelVal += labelInc;
    x0 = x1 = (int) rint( ( labelVal - adj_min ) * xFactor + xOffset );

  }

}

void drawYLinearScale (
  Display *d,
  Window win,
  gcClass *gc,
  int drawScale,
  int x,
  int y,
  int scaleHeight,
  double adj_min,
  double adj_max,
  int num_label_ticks,
  int majors_per_label,
  int minors_per_major,
  unsigned int scaleColor,
  unsigned int bgColor,
  int labelGrid,
  int majorGrid,
  int minorGrid,
  int gridLen,
  unsigned int gridColor,
  fontInfoClass *fi,
  char *fontTag,
  XFontStruct *fs,
  int annotateScale,
  int minConstrained,
  int maxConstrained,
  int erase,
  char *fmt
) {

int count, ii, iii, x0, y0, x1, y1;
int label_tick_height, major_tick_height, minor_tick_height, first=0;
double yFactor, yOffset, labelVal, majorInc, majorVal,
 minorInc, minorVal, lastInc, labelInc, z;
int fontAscent, fontDescent, fontHeight,
 stringWidth;
char buf[31+1];
unsigned int white, black;
int reverse = 0;

  if ( adj_min > adj_max ) {
    adj_min *= -1;
    adj_max *= -1;
    reverse = 1;
  }

  if ( scaleHeight < 1 ) return;
  if ( num_label_ticks < 1 ) return;
  if ( adj_max == adj_min ) return;

  white = WhitePixel( d, DefaultScreen(d) );
  black = BlackPixel( d, DefaultScreen(d) );

  gc->saveFg();
  gc->saveBg();

  gc->setLineWidth(1);
  gc->setLineStyle( LineSolid );
  gc->setFG( scaleColor );
  gc->setBG( bgColor );

  labelInc = ( adj_max - adj_min ) / num_label_ticks;
  if ( labelInc <= 0 ) labelInc = 1;

  yFactor = (double) ( scaleHeight ) / ( adj_max - adj_min );
  yOffset = y;

  labelVal = adj_min;

  if ( drawScale ) {

  first = 1;

  updateFontInfo( " ", fontTag, &fs,
   &fontAscent, &fontDescent, &fontHeight,
   &stringWidth );

  /* draw axis */
  x0 = x;
  y0 = y;
  x1 = x0;
  y1 = y0 - scaleHeight;
  if ( erase )
    XDrawLine( d, win, gc->eraseGC(), x0, y0, x1, y1 );
  else
    XDrawLine( d, win, gc->normGC(), x0, y0, x1, y1 );

  }
  else {

    fontHeight = 1;

  }

  label_tick_height = (int) ( 0.8 * (double) abs( fontHeight - 2 ) );
  major_tick_height = (int) ( 0.7 * ( double) label_tick_height );
  minor_tick_height = (int) ( ( double) label_tick_height * 0.4 );

  lastInc = labelInc * 0.5;

  /* draw label ticks */

  count = 0;
  while ( labelVal < ( adj_max - lastInc ) ) {

    x0 = x;
    x1 = x0 - label_tick_height;
    y0 = (int) rint( yOffset - ( labelVal - adj_min ) * yFactor );
    y1 = y0;

    if ( labelGrid && count ) {
      if ( erase ) {
        XDrawLine( d, win, gc->eraseGC(), x0, y0, x0+gridLen, y1 );
      }
      else {
        gc->setFG( gridColor );
        XDrawLine( d, win, gc->normGC(), x0, y0, x0+gridLen, y1 );
        gc->setFG( scaleColor );
      }
    }
    count++;

    if ( drawScale ) {

    if ( erase )
      XDrawLine( d, win, gc->eraseGC(), x0, y0, x1, y1 );
    else
      XDrawLine( d, win, gc->normGC(), x0, y0, x1, y1 );

    if ( annotateScale ) {
      gc->setFontTag( fontTag, fi );
      x0 = x - (int) ( 1.2 * label_tick_height );
      y1 = y0 - (int) ( fontHeight * 0.5 );
      z = fabs( labelVal - 0.0 ) / labelInc;
      if ( z < 1e-5 ) {
        if ( fmt ) {
          formatString( 0.0, buf, 31, fmt );
	}
	else {
          formatString( 0.0, buf, 31 );
	}
      }
      else {
	if ( reverse ) {
          if ( fmt ) {
            formatString( -1*labelVal, buf, 31, fmt );
	  }
	  else {
            formatString( -1*labelVal, buf, 31 );
	  }
	}
	else {
          if ( fmt ) {
            formatString( labelVal, buf, 31, fmt );
	  }
	  else {
            formatString( labelVal, buf, 31 );
	  }
	}
      }
      if ( minConstrained ) {
        if ( first ) {
          gc->setFG( black );
          gc->setBG( white );
        }
      }
      if ( erase )
        xEraseImageText( d, win, gc, fs, x0, y1,
         XmALIGNMENT_END, buf );
      else
        xDrawImageText( d, win, gc, fs, x0, y1,
         XmALIGNMENT_END, buf );
      if ( minConstrained ) {
        if ( first ) {
          first = 0;
          gc->setFG( scaleColor );
          gc->setBG( bgColor );
        }
      }
    }

    }

    if ( majors_per_label > 0 ) {

      majorInc = labelInc / majors_per_label;
      if ( majorInc <= 0 ) majorInc = 1;
      majorVal = labelVal;
      for ( ii=0; ii<majors_per_label; ii++ ) {

        if ( ii > 0 ) {

          x0 = x;
          x1 = x0 - major_tick_height;
          y0 = (int) rint( yOffset - ( majorVal - adj_min ) * yFactor );
          y1 = y0;

          if ( majorGrid ) {
            if ( erase ) {
              XDrawLine( d, win, gc->eraseGC(), x0, y0, x0+gridLen, y1 );
            }
            else {
              gc->setFG( gridColor );
              XDrawLine( d, win, gc->normGC(), x0, y0, x0+gridLen, y1 );
              gc->setFG( scaleColor );
            }
          }

          if ( drawScale ) {

          if ( erase )
            XDrawLine( d, win, gc->eraseGC(), x0, y0, x1, y1 );
          else
            XDrawLine( d, win, gc->normGC(), x0, y0, x1, y1 );

	  }

        }

        if ( minors_per_major  > 0 ) {

          minorInc = majorInc / minors_per_major;
          if ( minorInc <= 0 ) minorInc = 1;
          minorVal = majorVal + minorInc;

          for ( iii=1; iii<minors_per_major; iii++ ) {

            x0 = x;
            x1 = x0 - minor_tick_height;
            y0 = (int) rint( yOffset - ( minorVal - adj_min ) * yFactor );
            y1 = y0;

            if ( minorGrid ) {
              gc->setLineStyle( LineOnOffDash );
              if ( erase ) {
                XDrawLine( d, win, gc->eraseGC(), x0, y0, x0+gridLen, y1 );
              }
              else {
                gc->setFG( gridColor );
                XDrawLine( d, win, gc->normGC(), x0, y0, x0+gridLen, y1 );
                gc->setFG( scaleColor );
              }
              gc->setLineStyle( LineSolid );
            }

            if ( drawScale ) {

            if ( erase )
              XDrawLine( d, win, gc->eraseGC(), x0, y0, x1, y1 );
            else
              XDrawLine( d, win, gc->normGC(), x0, y0, x1, y1 );

	    }

            minorVal += minorInc;

	  }

	}

        majorVal += majorInc;

      }

    }

    labelVal += labelInc;

  }

  // draw last label tick

  x0 = x;
  x1 = x0 - label_tick_height;
  y0 = (int) rint( yOffset - ( labelVal - adj_min ) * yFactor );
  y1 = y0;

  if ( labelGrid ) {
    if ( erase ) {
      XDrawLine( d, win, gc->eraseGC(), x0, y0, x0+gridLen, y1 );
    }
    else {
      gc->setFG( gridColor );
      XDrawLine( d, win, gc->normGC(), x0, y0, x0+gridLen, y1 );
      gc->setFG( scaleColor );
    }
  }

  if ( drawScale ) {

  if ( erase )
    XDrawLine( d, win, gc->eraseGC(), x0, y0, x1, y1 );
  else
    XDrawLine( d, win, gc->normGC(), x0, y0, x1, y1 );

  if ( annotateScale ) {
    gc->setFontTag( fontTag, fi );
    x0 = x - (int) ( 1.2 * label_tick_height );
    y1 = y0 - (int) ( fontHeight * 0.5 );
    z = fabs( labelVal - 0.0 ) / labelInc;
    if ( z < 1e-5 ) {
      if ( fmt ) {
        formatString( 0.0, buf, 31, fmt );
      }
      else {
        formatString( 0.0, buf, 31 );
      }
    }
    else {
      if ( reverse ) {
        if ( fmt ) {
          formatString( -1*labelVal, buf, 31, fmt );
	}
	else {
          formatString( -1*labelVal, buf, 31 );
	}
      }
      else {
        if ( fmt ) {
          formatString( labelVal, buf, 31, fmt );
	}
	else {
          formatString( labelVal, buf, 31 );
	}
      }
    }
    if ( maxConstrained ) {
      gc->setFG( black );
      gc->setBG( white );
    }
    if ( erase )
      xEraseImageText( d, win, gc, fs, x0, y1,
       XmALIGNMENT_END, buf );
    else
      xDrawImageText( d, win, gc, fs, x0, y1,
       XmALIGNMENT_END, buf );
    if ( maxConstrained ) {
     gc->setFG( scaleColor );
      gc->setBG( bgColor );
    }
  }

  }
 
  gc->restoreFg();
  gc->restoreBg();

}

void drawYLinearScale (
  Display *d,
  Window win,
  gcClass *gc,
  int drawScale,
  int x,
  int y,
  int scaleHeight,
  double adj_min,
  double adj_max,
  int num_label_ticks,
  int majors_per_label,
  int minors_per_major,
  unsigned int scaleColor,
  unsigned int bgColor,
  int labelGrid,
  int majorGrid,
  int minorGrid,
  int gridLen,
  unsigned int gridColor,
  fontInfoClass *fi,
  char *fontTag,
  XFontStruct *fs,
  int annotateScale,
  int minConstrained,
  int maxConstrained,
  int erase
) {

  drawYLinearScale (
   d,
   win,
   gc,
   drawScale,
   x,
   y,
   scaleHeight,
   adj_min,
   adj_max,
   num_label_ticks,
   majors_per_label,
   minors_per_major,
   scaleColor,
   bgColor,
   labelGrid,
   majorGrid,
   minorGrid,
   gridLen,
   gridColor,
   fi,
   fontTag,
   fs,
   annotateScale,
   minConstrained,
   maxConstrained,
   erase,
   NULL );

}

void drawYLinearScale2 (
  Display *d,
  Window win,
  gcClass *gc,
  int drawScale,
  int x,
  int y,
  int scaleHeight,
  double cur_min,
  double cur_max,
  double adj_min,
  double adj_max,
  int num_label_ticks,
  int majors_per_label,
  int minors_per_major,
  unsigned int scaleColor,
  unsigned int bgColor,
  int labelGrid,
  int majorGrid,
  int minorGrid,
  int gridLen,
  unsigned int gridColor,
  fontInfoClass *fi,
  char *fontTag,
  XFontStruct *fs,
  int annotateScale,
  int minConstrained,
  int maxConstrained,
  int erase
) {

int count, ii, iii, x0, y0, x1, y1;
int label_tick_height, major_tick_height, minor_tick_height, first=0;
double yFactor, yOffset, adjYOffset, labelVal, lastLabelVal, majorInc, majorVal,
 minorInc, minorVal, lastInc, labelInc, z;
int fontAscent, fontDescent, fontHeight,
 stringWidth;
char buf[31+1];
unsigned int white, black;
int reverse = 0;

  if ( adj_min > adj_max ) {
    adj_min *= -1;
    adj_max *= -1;
    reverse = 1;
  }

  if ( scaleHeight < 1 ) return;
  if ( num_label_ticks < 1 ) return;
  if ( adj_max == adj_min ) return;

  white = WhitePixel( d, DefaultScreen(d) );
  black = BlackPixel( d, DefaultScreen(d) );

  gc->saveFg();
  gc->saveBg();

  gc->setLineWidth(1);
  gc->setLineStyle( LineSolid );
  gc->setFG( scaleColor );
  gc->setBG( bgColor );

  yFactor = (double) ( scaleHeight ) / ( cur_max - cur_min );
  yOffset = y;

  adjYOffset = (int) rint( yOffset - ( adj_min - cur_min ) * yFactor );
  yOffset = adjYOffset;

  labelInc = ( adj_max - adj_min ) / num_label_ticks;
  if ( labelInc <= 0 ) labelInc = 1;

  lastLabelVal = labelVal = adj_min;

  if ( drawScale ) {

    first = 1;

    updateFontInfo( " ", fontTag, &fs,
     &fontAscent, &fontDescent, &fontHeight,
     &stringWidth );

    /* draw axis */
    x0 = x;
    y0 = y;
    x1 = x0;
    y1 = y0 - scaleHeight;
    if ( erase ) {
      XDrawLine( d, win, gc->eraseGC(), x0, y0, x1, y1 );
    }
    else {
      XDrawLine( d, win, gc->normGC(), x0, y0, x1, y1 );
    }

  }
  else {

    fontHeight = 1;

  }

  label_tick_height = (int) ( 0.8 * (double) abs( fontHeight - 2 ) );
  major_tick_height = (int) ( 0.7 * ( double) label_tick_height );
  minor_tick_height = (int) ( ( double) label_tick_height * 0.4 );

  lastInc = labelInc * 0.5;

  /* draw label ticks */

  count = 0;
  while ( labelVal < ( adj_max + lastInc ) ) {

    x0 = x;
    x1 = x0 - label_tick_height;
    y0 = (int) rint( yOffset - ( labelVal - adj_min ) * yFactor );
    y1 = y0;

    if ( ( y0 <= y ) && ( y0 >= ( y - scaleHeight ) ) ) {

      if ( labelGrid && count ) {
        if ( erase ) {
          XDrawLine( d, win, gc->eraseGC(), x0, y0, x0+gridLen, y1 );
        }
        else {
          gc->setFG( gridColor );
          XDrawLine( d, win, gc->normGC(), x0, y0, x0+gridLen, y1 );
          gc->setFG( scaleColor );
        }
      }

      if ( drawScale ) {

        lastLabelVal = labelVal;

        if ( erase ) {
          XDrawLine( d, win, gc->eraseGC(), x0, y0, x1, y1 );
        }
        else {
          XDrawLine( d, win, gc->normGC(), x0, y0, x1, y1 );
        }

        if ( annotateScale ) {
          gc->setFontTag( fontTag, fi );
          x0 = x - (int) ( 1.2 * label_tick_height );
          y1 = y0 - (int) ( fontHeight * 0.5 );
          z = fabs( labelVal - 0.0 ) / labelInc;
          if ( z < 1e-5 ) {
            formatString( 0.0, buf, 31 );
          }
          else {
            if ( reverse ) {
              formatString( -1*labelVal, buf, 31 );
            }
            else {
              formatString( labelVal, buf, 31 );
            }
          }
          if ( minConstrained ) {
            if ( first ) {
              gc->setFG( black );
              gc->setBG( white );
            }
          }
          if ( erase ) {
            xEraseImageText( d, win, gc, fs, x0, y1,
             XmALIGNMENT_END, buf );
	  }
          else {
            xDrawImageText( d, win, gc, fs, x0, y1,
             XmALIGNMENT_END, buf );
	  }
          if ( minConstrained ) {
            if ( first ) {
              first = 0;
              gc->setFG( scaleColor );
              gc->setBG( bgColor );
            }
          }

        }

      }

    }
    count++;

    if ( majors_per_label > 0 ) {

      majorInc = labelInc / majors_per_label;
      if ( majorInc <= 0 ) majorInc = 1;
      majorVal = labelVal;
      for ( ii=0; ii<majors_per_label; ii++ ) {

        if ( ii > 0 ) {

          x0 = x;
          x1 = x0 - major_tick_height;
          y0 = (int) rint( yOffset - ( majorVal - adj_min ) * yFactor );
          y1 = y0;

          if ( ( y0 <= y ) && ( y0 >= ( y - scaleHeight ) ) ) {

            if ( majorGrid ) {
              if ( erase ) {
                XDrawLine( d, win, gc->eraseGC(), x0, y0, x0+gridLen, y1 );
              }
              else {
                gc->setFG( gridColor );
                XDrawLine( d, win, gc->normGC(), x0, y0, x0+gridLen, y1 );
                gc->setFG( scaleColor );
              }
            }

            if ( drawScale ) {

	      if ( erase ) {
                XDrawLine( d, win, gc->eraseGC(), x0, y0, x1, y1 );
	      }
	      else {
                XDrawLine( d, win, gc->normGC(), x0, y0, x1, y1 );
	      }

            }

          }

        }

        if ( minors_per_major  > 0 ) {

          minorInc = majorInc / minors_per_major;
          if ( minorInc <= 0 ) minorInc = 1;
          minorVal = majorVal + minorInc;

          for ( iii=1; iii<minors_per_major; iii++ ) {

            x0 = x;
            x1 = x0 - minor_tick_height;
            y0 = (int) rint( yOffset - ( minorVal - adj_min ) * yFactor );
            y1 = y0;

            if ( ( y0 <= y ) && ( y0 >= ( y - scaleHeight ) ) ) {

              if ( minorGrid ) {
                gc->setLineStyle( LineOnOffDash );
                if ( erase ) {
                  XDrawLine( d, win, gc->eraseGC(), x0, y0, x0+gridLen, y1 );
                }
                else {
                  gc->setFG( gridColor );
                  XDrawLine( d, win, gc->normGC(), x0, y0, x0+gridLen, y1 );
                  gc->setFG( scaleColor );
                }
                gc->setLineStyle( LineSolid );
              }

              if ( drawScale ) {

                if ( erase ) {
                  XDrawLine( d, win, gc->eraseGC(), x0, y0, x1, y1 );
		}
                else {
                  XDrawLine( d, win, gc->normGC(), x0, y0, x1, y1 );
		}

              }

            }

            minorVal += minorInc;

          }

        }

        majorVal += majorInc;

      }

    }

    labelVal += labelInc;

  }

  labelVal = lastLabelVal;

  // draw last label tick

  x0 = x;
  x1 = x0 - label_tick_height;
  y0 = (int) rint( yOffset - ( labelVal - adj_min ) * yFactor );
  y1 = y0;

  if ( ( y0 <= y ) && ( y0 >= ( y - scaleHeight ) ) ) {

    if ( labelGrid ) {
      if ( erase ) {
        XDrawLine( d, win, gc->eraseGC(), x0, y0, x0+gridLen, y1 );
      }
      else {
        gc->setFG( gridColor );
        XDrawLine( d, win, gc->normGC(), x0, y0, x0+gridLen, y1 );
        gc->setFG( scaleColor );
      }
    }

    if ( drawScale ) {

      if ( erase ) {
        XDrawLine( d, win, gc->eraseGC(), x0, y0, x1, y1 );
      }
      else {
        XDrawLine( d, win, gc->normGC(), x0, y0, x1, y1 );
      }

      if ( annotateScale ) {
        gc->setFontTag( fontTag, fi );
        x0 = x - (int) ( 1.2 * label_tick_height );
        y1 = y0 - (int) ( fontHeight * 0.5 );
        z = fabs( labelVal - 0.0 ) / labelInc;
        if ( z < 1e-5 ) {
          formatString( 0.0, buf, 31 );
        }
        else {
          if ( reverse ) {
            formatString( -1*labelVal, buf, 31 );
          }
          else {
            formatString( labelVal, buf, 31 );
          }
        }
        if ( maxConstrained ) {
          gc->setFG( black );
          gc->setBG( white );
        }
        if ( erase ) {
          xEraseImageText( d, win, gc, fs, x0, y1,
           XmALIGNMENT_END, buf );
	}
        else {
          xDrawImageText( d, win, gc, fs, x0, y1,
           XmALIGNMENT_END, buf );
	}
        if ( maxConstrained ) {
         gc->setFG( scaleColor );
          gc->setBG( bgColor );
        }

      }

    }

  }
 
  gc->restoreFg();
  gc->restoreBg();

}

void drawY2LinearScale (
  Display *d,
  Window win,
  gcClass *gc,
  int drawScale,
  int x,
  int y,
  int scaleHeight,
  double adj_min,
  double adj_max,
  int num_label_ticks,
  int majors_per_label,
  int minors_per_major,
  unsigned int scaleColor,
  unsigned int bgColor,
  int labelGrid,
  int majorGrid,
  int minorGrid,
  int gridLen,
  unsigned int gridColor,
  fontInfoClass *fi,
  char *fontTag,
  XFontStruct *fs,
  int annotateScale,
  int minConstrained,
  int maxConstrained,
  int erase
) {

int count, ii, iii, x0, y0, x1, y1;
int label_tick_height, major_tick_height, minor_tick_height, first=0;
double yFactor, yOffset, labelVal, majorInc, majorVal,
 minorInc, minorVal, lastInc, labelInc, z;
int fontAscent, fontDescent, fontHeight,
 stringWidth;
char buf[31+1];
unsigned int white, black;
int reverse = 0;

  if ( adj_min > adj_max ) {
    adj_min *= -1;
    adj_max *= -1;
    reverse = 1;
  }

  if ( scaleHeight < 1 ) return;
  if ( num_label_ticks < 1 ) return;
  if ( adj_max == adj_min ) return;

  white = WhitePixel( d, DefaultScreen(d) );
  black = BlackPixel( d, DefaultScreen(d) );

  gc->saveFg();
  gc->saveBg();

  gc->setLineWidth(1);
  gc->setLineStyle( LineSolid );
  gc->setFG( scaleColor );
  gc->setBG( bgColor );

  labelInc = ( adj_max - adj_min ) / num_label_ticks;
  if ( labelInc <= 0 ) labelInc = 1;

  yFactor = (double) ( scaleHeight ) / ( adj_max - adj_min );
  yOffset = y;

  labelVal = adj_min;

  if ( drawScale ) {

  first = 1;

  updateFontInfo( " ", fontTag, &fs,
   &fontAscent, &fontDescent, &fontHeight,
   &stringWidth );

  /* draw axis */
  x0 = x;
  y0 = y;
  x1 = x0;
  y1 = y0 - scaleHeight;
  if ( erase )
    XDrawLine( d, win, gc->eraseGC(), x0, y0, x1, y1 );
  else
    XDrawLine( d, win, gc->normGC(), x0, y0, x1, y1 );

  }
  else {

    fontHeight = 1;

  }

  label_tick_height = (int) ( 0.8 * (double) abs( fontHeight - 2 ) );
  major_tick_height = (int) ( 0.7 * ( double) label_tick_height );
  minor_tick_height = (int) ( ( double) label_tick_height * 0.4 );

  lastInc = labelInc * 0.5;

  /* draw label ticks */

  count = 0;
  while ( labelVal < ( adj_max - lastInc ) ) {

    x0 = x;
    //x1 = x0 - label_tick_height;
    x1 = x0 + label_tick_height;
    y0 = (int) rint( yOffset - ( labelVal - adj_min ) * yFactor );
    y1 = y0;

    if ( labelGrid && count ) {
      if ( erase ) {
        //XDrawLine( d, win, gc->eraseGC(), x0, y0, x0+gridLen, y1 );
        XDrawLine( d, win, gc->eraseGC(), x0, y0, x0-gridLen, y1 );
      }
      else {
        gc->setFG( gridColor );
        //XDrawLine( d, win, gc->normGC(), x0, y0, x0+gridLen, y1 );
        XDrawLine( d, win, gc->normGC(), x0, y0, x0-gridLen, y1 );
        gc->setFG( scaleColor );
      }
    }
    count++;

    if ( drawScale ) {

    if ( erase )
      XDrawLine( d, win, gc->eraseGC(), x0, y0, x1, y1 );
    else
      XDrawLine( d, win, gc->normGC(), x0, y0, x1, y1 );

    if ( annotateScale ) {
      gc->setFontTag( fontTag, fi );
      //x0 = x - (int) ( 1.2 * label_tick_height );
      x0 = x + (int) ( 1.2 * label_tick_height );
      y1 = y0 - (int) ( fontHeight * 0.5 );
      z = fabs( labelVal - 0.0 ) / labelInc;
      if ( z < 1e-5 ) {
        formatString( 0.0, buf, 31 );
      }
      else {
	if ( reverse ) {
          formatString( -1*labelVal, buf, 31 );
	}
	else {
          formatString( labelVal, buf, 31 );
	}
      }
      if ( minConstrained ) {
        if ( first ) {
          gc->setFG( black );
          gc->setBG( white );
        }
      }
      if ( erase )
        //xEraseImageText( d, win, gc, fs, x0, y1,
        // XmALIGNMENT_END, buf );
        xEraseImageText( d, win, gc, fs, x0, y1,
         XmALIGNMENT_BEGINNING, buf );
      else
        //xDrawImageText( d, win, gc, fs, x0, y1,
        // XmALIGNMENT_END, buf );
        xDrawImageText( d, win, gc, fs, x0, y1,
         XmALIGNMENT_BEGINNING, buf );
      if ( minConstrained ) {
        if ( first ) {
          first = 0;
          gc->setFG( scaleColor );
          gc->setBG( bgColor );
        }
      }
    }

    }

    if ( majors_per_label > 0 ) {

      majorInc = labelInc / majors_per_label;
      if ( majorInc <= 0 ) majorInc = 1;
      majorVal = labelVal;
      for ( ii=0; ii<majors_per_label; ii++ ) {

        if ( ii > 0 ) {

          x0 = x;
          //x1 = x0 - major_tick_height;
          x1 = x0 + major_tick_height;
          y0 = (int) rint( yOffset - ( majorVal - adj_min ) * yFactor );
          y1 = y0;

          if ( majorGrid ) {
            if ( erase ) {
              //XDrawLine( d, win, gc->eraseGC(), x0, y0, x0+gridLen, y1 );
              XDrawLine( d, win, gc->eraseGC(), x0, y0, x0-gridLen, y1 );
            }
            else {
              gc->setFG( gridColor );
              //XDrawLine( d, win, gc->normGC(), x0, y0, x0+gridLen, y1 );
              XDrawLine( d, win, gc->normGC(), x0, y0, x0-gridLen, y1 );
              gc->setFG( scaleColor );
            }
          }

          if ( drawScale ) {

          if ( erase )
            XDrawLine( d, win, gc->eraseGC(), x0, y0, x1, y1 );
          else
            XDrawLine( d, win, gc->normGC(), x0, y0, x1, y1 );

	  }

        }

        if ( minors_per_major  > 0 ) {

          minorInc = majorInc / minors_per_major;
          if ( minorInc <= 0 ) minorInc = 1;
          minorVal = majorVal + minorInc;

          for ( iii=1; iii<minors_per_major; iii++ ) {

            x0 = x;
            //x1 = x0 - minor_tick_height;
            x1 = x0 + minor_tick_height;
            y0 = (int) rint( yOffset - ( minorVal - adj_min ) * yFactor );
            y1 = y0;

            if ( minorGrid ) {
              gc->setLineStyle( LineOnOffDash );
              if ( erase ) {
                //XDrawLine( d, win, gc->eraseGC(), x0, y0, x0+gridLen, y1 );
                XDrawLine( d, win, gc->eraseGC(), x0, y0, x0-gridLen, y1 );
              }
              else {
                gc->setFG( gridColor );
                //XDrawLine( d, win, gc->normGC(), x0, y0, x0+gridLen, y1 );
                XDrawLine( d, win, gc->normGC(), x0, y0, x0-gridLen, y1 );
                gc->setFG( scaleColor );
              }
              gc->setLineStyle( LineSolid );
            }

            if ( drawScale ) {

            if ( erase )
              XDrawLine( d, win, gc->eraseGC(), x0, y0, x1, y1 );
            else
              XDrawLine( d, win, gc->normGC(), x0, y0, x1, y1 );

	    }

            minorVal += minorInc;

	  }

	}

        majorVal += majorInc;

      }

    }

    labelVal += labelInc;

  }

  // draw last label tick

  x0 = x;
  //x1 = x0 - label_tick_height;
  x1 = x0 + label_tick_height;
  y0 = (int) rint( yOffset - ( labelVal - adj_min ) * yFactor );
  y1 = y0;

  if ( labelGrid ) {
    if ( erase ) {
      //XDrawLine( d, win, gc->eraseGC(), x0, y0, x0+gridLen, y1 );
      XDrawLine( d, win, gc->eraseGC(), x0, y0, x0-gridLen, y1 );
    }
    else {
      gc->setFG( gridColor );
      //XDrawLine( d, win, gc->normGC(), x0, y0, x0+gridLen, y1 );
      XDrawLine( d, win, gc->normGC(), x0, y0, x0-gridLen, y1 );
      gc->setFG( scaleColor );
    }
  }

  if ( drawScale ) {

  if ( erase )
    XDrawLine( d, win, gc->eraseGC(), x0, y0, x1, y1 );
  else
    XDrawLine( d, win, gc->normGC(), x0, y0, x1, y1 );

  if ( annotateScale ) {
    gc->setFontTag( fontTag, fi );
    //x0 = x - (int) ( 1.2 * label_tick_height );
    x0 = x + (int) ( 1.2 * label_tick_height );
    y1 = y0 - (int) ( fontHeight * 0.5 );
    z = fabs( labelVal - 0.0 ) / labelInc;
    if ( z < 1e-5 ) {
      formatString( 0.0, buf, 31 );
    }
    else {
      if ( reverse ) {
        formatString( -1*labelVal, buf, 31 );
      }
      else {
        formatString( labelVal, buf, 31 );
      }
    }
    if ( maxConstrained ) {
      gc->setFG( black );
      gc->setBG( white );
    }
    if ( erase )
      //xEraseImageText( d, win, gc, fs, x0, y1,
      // XmALIGNMENT_END, buf );
      xEraseImageText( d, win, gc, fs, x0, y1,
       XmALIGNMENT_BEGINNING, buf );
    else
      //xDrawImageText( d, win, gc, fs, x0, y1,
      // XmALIGNMENT_END, buf );
      xDrawImageText( d, win, gc, fs, x0, y1,
       XmALIGNMENT_BEGINNING, buf );
    if ( maxConstrained ) {
     gc->setFG( scaleColor );
      gc->setBG( bgColor );
    }
  }

  }
 
  gc->restoreFg();
  gc->restoreBg();

}

void drawY2LinearScale2 (
  Display *d,
  Window win,
  gcClass *gc,
  int drawScale,
  int x,
  int y,
  int scaleHeight,
  double cur_min,
  double cur_max,
  double adj_min,
  double adj_max,
  int num_label_ticks,
  int majors_per_label,
  int minors_per_major,
  unsigned int scaleColor,
  unsigned int bgColor,
  int labelGrid,
  int majorGrid,
  int minorGrid,
  int gridLen,
  unsigned int gridColor,
  fontInfoClass *fi,
  char *fontTag,
  XFontStruct *fs,
  int annotateScale,
  int minConstrained,
  int maxConstrained,
  int erase
) {

int count, ii, iii, x0, y0, x1, y1;
int label_tick_height, major_tick_height, minor_tick_height, first=0;
double yFactor, yOffset, adjYOffset, labelVal, lastLabelVal, majorInc, majorVal,
 minorInc, minorVal, lastInc, labelInc, z;
int fontAscent, fontDescent, fontHeight,
 stringWidth;
char buf[31+1];
unsigned int white, black;
int reverse = 0;

  if ( adj_min > adj_max ) {
    adj_min *= -1;
    adj_max *= -1;
    reverse = 1;
  }

  if ( scaleHeight < 1 ) return;
  if ( num_label_ticks < 1 ) return;
  if ( adj_max == adj_min ) return;

  white = WhitePixel( d, DefaultScreen(d) );
  black = BlackPixel( d, DefaultScreen(d) );

  gc->saveFg();
  gc->saveBg();

  gc->setLineWidth(1);
  gc->setLineStyle( LineSolid );
  gc->setFG( scaleColor );
  gc->setBG( bgColor );

  yFactor = (double) ( scaleHeight ) / ( cur_max - cur_min );
  yOffset = y;

  adjYOffset = (int) rint( yOffset - ( adj_min - cur_min ) * yFactor );
  yOffset = adjYOffset;

  labelInc = ( adj_max - adj_min ) / num_label_ticks;
  if ( labelInc <= 0 ) labelInc = 1;

  lastLabelVal = labelVal = adj_min;

  if ( drawScale ) {

    first = 1;

    updateFontInfo( " ", fontTag, &fs,
     &fontAscent, &fontDescent, &fontHeight,
     &stringWidth );

    /* draw axis */
    x0 = x;
    y0 = y;
    x1 = x0;
    y1 = y0 - scaleHeight;
    if ( erase ) {
      XDrawLine( d, win, gc->eraseGC(), x0, y0, x1, y1 );
    }
    else {
      XDrawLine( d, win, gc->normGC(), x0, y0, x1, y1 );
    }

  }
  else {

    fontHeight = 1;

  }

  label_tick_height = (int) ( 0.8 * (double) abs( fontHeight - 2 ) );
  major_tick_height = (int) ( 0.7 * ( double) label_tick_height );
  minor_tick_height = (int) ( ( double) label_tick_height * 0.4 );

  lastInc = labelInc * 0.5;

  /* draw label ticks */

  count = 0;
  while ( labelVal < ( adj_max + lastInc ) ) {

    x0 = x;
    x1 = x0 + label_tick_height;
    y0 = (int) rint( yOffset - ( labelVal - adj_min ) * yFactor );
    y1 = y0;

    if ( ( y0 <= y ) && ( y0 >= ( y - scaleHeight ) ) ) {

      if ( labelGrid && count ) {
        if ( erase ) {
          XDrawLine( d, win, gc->eraseGC(), x0, y0, x0-gridLen, y1 );
        }
        else {
          gc->setFG( gridColor );
          XDrawLine( d, win, gc->normGC(), x0, y0, x0-gridLen, y1 );
          gc->setFG( scaleColor );
        }
      }

      if ( drawScale ) {

        lastLabelVal = labelVal;

        if ( erase ) {
          XDrawLine( d, win, gc->eraseGC(), x0, y0, x1, y1 );
        }
        else {
          XDrawLine( d, win, gc->normGC(), x0, y0, x1, y1 );
        }

        if ( annotateScale ) {
          gc->setFontTag( fontTag, fi );
          x0 = x + (int) ( 1.2 * label_tick_height );
          y1 = y0 - (int) ( fontHeight * 0.5 );
          z = fabs( labelVal - 0.0 ) / labelInc;
          if ( z < 1e-5 ) {
            formatString( 0.0, buf, 31 );
          }
          else {
            if ( reverse ) {
              formatString( -1*labelVal, buf, 31 );
            }
            else {
              formatString( labelVal, buf, 31 );
            }
          }
          if ( minConstrained ) {
            if ( first ) {
              gc->setFG( black );
              gc->setBG( white );
            }
          }
          if ( erase ) {
            xEraseImageText( d, win, gc, fs, x0, y1,
             XmALIGNMENT_BEGINNING, buf );
          }
          else {
            xDrawImageText( d, win, gc, fs, x0, y1,
             XmALIGNMENT_BEGINNING, buf );
          }
          if ( minConstrained ) {
            if ( first ) {
              first = 0;
              gc->setFG( scaleColor );
              gc->setBG( bgColor );
            }
          }

        }

      }

    }
    count++;

    if ( majors_per_label > 0 ) {

      majorInc = labelInc / majors_per_label;
      if ( majorInc <= 0 ) majorInc = 1;
      majorVal = labelVal;
      for ( ii=0; ii<majors_per_label; ii++ ) {

        if ( ii > 0 ) {

          x0 = x;
          x1 = x0 + major_tick_height;
          y0 = (int) rint( yOffset - ( majorVal - adj_min ) * yFactor );
          y1 = y0;

          if ( ( y0 <= y ) && ( y0 >= ( y - scaleHeight ) ) ) {

            if ( majorGrid ) {
              if ( erase ) {
                XDrawLine( d, win, gc->eraseGC(), x0, y0, x0-gridLen, y1 );
              }
              else {
                gc->setFG( gridColor );
                XDrawLine( d, win, gc->normGC(), x0, y0, x0-gridLen, y1 );
                gc->setFG( scaleColor );
              }
            }

            if ( drawScale ) {

              if ( erase ) {
                XDrawLine( d, win, gc->eraseGC(), x0, y0, x1, y1 );
              }
              else {
                XDrawLine( d, win, gc->normGC(), x0, y0, x1, y1 );
              }

            }

          }

	}

        if ( minors_per_major  > 0 ) {

          minorInc = majorInc / minors_per_major;
          if ( minorInc <= 0 ) minorInc = 1;
          minorVal = majorVal + minorInc;

          for ( iii=1; iii<minors_per_major; iii++ ) {

            x0 = x;
            x1 = x0 + minor_tick_height;
            y0 = (int) rint( yOffset - ( minorVal - adj_min ) * yFactor );
            y1 = y0;

            if ( ( y0 <= y ) && ( y0 >= ( y - scaleHeight ) ) ) {

              if ( minorGrid ) {
                gc->setLineStyle( LineOnOffDash );
                if ( erase ) {
                  XDrawLine( d, win, gc->eraseGC(), x0, y0, x0-gridLen, y1 );
                }
                else {
                  gc->setFG( gridColor );
                  XDrawLine( d, win, gc->normGC(), x0, y0, x0-gridLen, y1 );
                  gc->setFG( scaleColor );
                }
                gc->setLineStyle( LineSolid );
              }

              if ( drawScale ) {

                if ( erase ) {
                  XDrawLine( d, win, gc->eraseGC(), x0, y0, x1, y1 );
                }
                else {
                  XDrawLine( d, win, gc->normGC(), x0, y0, x1, y1 );
                }

              }

            }

            minorVal += minorInc;

          }

        }

        majorVal += majorInc;

      }

    }

    labelVal += labelInc;

  }

  labelVal = lastLabelVal;

  // draw last label tick

  x0 = x;
  x1 = x0 + label_tick_height;
  y0 = (int) rint( yOffset - ( labelVal - adj_min ) * yFactor );
  y1 = y0;

  if ( ( y0 <= y ) && ( y0 >= ( y - scaleHeight ) ) ) {

    if ( labelGrid ) {
      if ( erase ) {
        XDrawLine( d, win, gc->eraseGC(), x0, y0, x0-gridLen, y1 );
      }
      else {
        gc->setFG( gridColor );
        XDrawLine( d, win, gc->normGC(), x0, y0, x0-gridLen, y1 );
        gc->setFG( scaleColor );
      }
    }

    if ( drawScale ) {

      if ( erase ) {
        XDrawLine( d, win, gc->eraseGC(), x0, y0, x1, y1 );
      }
      else {
        XDrawLine( d, win, gc->normGC(), x0, y0, x1, y1 );
      }

      if ( annotateScale ) {
        gc->setFontTag( fontTag, fi );
        x0 = x + (int) ( 1.2 * label_tick_height );
        y1 = y0 - (int) ( fontHeight * 0.5 );
        z = fabs( labelVal - 0.0 ) / labelInc;
        if ( z < 1e-5 ) {
          formatString( 0.0, buf, 31 );
        }
        else {
          if ( reverse ) {
            formatString( -1*labelVal, buf, 31 );
          }
          else {
            formatString( labelVal, buf, 31 );
          }
        }
        if ( maxConstrained ) {
          gc->setFG( black );
          gc->setBG( white );
        }
        if ( erase ) {
          xEraseImageText( d, win, gc, fs, x0, y1,
           XmALIGNMENT_BEGINNING, buf );
        }
        else {
          xDrawImageText( d, win, gc, fs, x0, y1,
           XmALIGNMENT_BEGINNING, buf );
        }
        if ( maxConstrained ) {
         gc->setFG( scaleColor );
          gc->setBG( bgColor );
        }

      }

    }

  }
 
  gc->restoreFg();
  gc->restoreBg();

}

void drawYLog10Scale (
  Display *d,
  Window win,
  gcClass *gc,
  int drawScale,
  int x,
  int y,
  int scaleHeight,
  double adj_min,
  double adj_max,
  int num_label_ticks,
  int majors_per_label,
  int minors_per_major,
  unsigned int scaleColor,
  unsigned int bgColor,
  int labelGrid,
  int majorGrid,
  int minorGrid,
  int gridLen,
  unsigned int gridColor,
  fontInfoClass *fi,
  char *fontTag,
  XFontStruct *fs,
  int annotateScale,
  int minConstrained,
  int maxConstrained,
  int erase
) {

int count, ii, iii, x0, y0, x1, y1;
int label_tick_height, major_tick_height, minor_tick_height, first=0;
double yFactor, yOffset, labelVal, majorInc, majorVal,
 minorInc, minorVal, lastInc, labelInc, val, val0, val1;
int fontAscent, fontDescent, fontHeight,
 stringWidth;
char buf[31+1];
unsigned int white, black;

//fprintf( stderr, "adj_min = %-g\n", adj_min );
//fprintf( stderr, "adj_max = %-g\n", adj_max );

  if ( scaleHeight < 1 ) return;
  if ( num_label_ticks < 1 ) return;
  if ( adj_max <= adj_min ) return;

  white = WhitePixel( d, DefaultScreen(d) );
  black = BlackPixel( d, DefaultScreen(d) );

  gc->saveFg();
  gc->saveBg();

  gc->setLineWidth(1);
  gc->setLineStyle( LineSolid );
  gc->setFG( scaleColor );
  gc->setBG( bgColor );

  labelInc = ( adj_max - adj_min ) / num_label_ticks;
  if ( labelInc <= 0 ) labelInc = 1;

  yFactor = (double) ( scaleHeight ) / ( adj_max - adj_min );
  yOffset = y;

  //fprintf( stderr, "adj_min = %-g\n", adj_min );
  //fprintf( stderr, "adj_max = %-g\n", adj_max );
  //fprintf( stderr, "yOffset = %-g\n", yOffset );
  //fprintf( stderr, "yFactor = %-g\n", yFactor );
  //fprintf( stderr, "scaleHeight = %-d\n", scaleHeight );

  labelVal = adj_min;

  if ( drawScale ) {

  first = 1;

  updateFontInfo( " ", fontTag, &fs,
   &fontAscent, &fontDescent, &fontHeight,
   &stringWidth );

  /* draw axis */
  x0 = x;
  y0 = y;
  x1 = x0;
  y1 = y0 - scaleHeight;
  if ( erase )
    XDrawLine( d, win, gc->eraseGC(), x0, y0, x1, y1 );
  else
    XDrawLine( d, win, gc->normGC(), x0, y0, x1, y1 );

  }
  else {

    fontHeight = 1;

  }

  label_tick_height = (int) ( 0.8 * (double) abs( fontHeight - 2 ) );
  major_tick_height = (int) ( 0.7 * ( double) label_tick_height );
  minor_tick_height = (int) ( ( double) label_tick_height * 0.4 );

  lastInc = labelInc * 0.5;

  /* draw label ticks */

  count = 0;
  while ( labelVal < ( adj_max - lastInc ) ) {

    x0 = x;
    x1 = x0 - label_tick_height;
    y0 = (int) rint( yOffset - ( labelVal - adj_min ) * yFactor );
    y1 = y0;

    if ( labelGrid && count ) {
      if ( erase ) {
        XDrawLine( d, win, gc->eraseGC(), x0, y0, x0+gridLen, y1 );
      }
      else {
        gc->setFG( gridColor );
        XDrawLine( d, win, gc->normGC(), x0, y0, x0+gridLen, y1 );
        gc->setFG( scaleColor );
      }
    }
    count++;

    if ( drawScale ) {

    if ( erase )
      XDrawLine( d, win, gc->eraseGC(), x0, y0, x1, y1 );
    else
      XDrawLine( d, win, gc->normGC(), x0, y0, x1, y1 );

    if ( annotateScale ) {
      gc->setFontTag( fontTag, fi );
      x0 = x - (int) ( 1.2 * label_tick_height );
      y1 = y0 - (int) ( fontHeight * 0.5 );
      formatString( pow(10,labelVal), buf, 31 );
      if ( minConstrained ) {
        if ( first ) {
          gc->setFG( black );
          gc->setBG( white );
        }
      }
      if ( erase )
        xEraseImageText( d, win, gc, fs, x0, y1,
         XmALIGNMENT_END, buf );
      else
        xDrawImageText( d, win, gc, fs, x0, y1,
         XmALIGNMENT_END, buf );
      if ( minConstrained ) {
        if ( first ) {
          first = 0;
          gc->setFG( scaleColor );
          gc->setBG( bgColor );
        }
      }
    }

    }

    if ( majors_per_label > 0 ) {

      majorInc = labelInc / majors_per_label;
      if ( majorInc <= 0 ) majorInc = 1;
      majorVal = labelVal;
      for ( ii=0; ii<majors_per_label; ii++ ) {

        if ( ii > 0 ) {

          x0 = x;
          x1 = x0 - major_tick_height;
          y0 = (int) rint( yOffset - ( majorVal - adj_min ) * yFactor );
          y1 = y0;

          if ( majorGrid ) {
            if ( erase ) {
              XDrawLine( d, win, gc->eraseGC(), x0, y0, x0+gridLen, y1 );
            }
            else {
              gc->setFG( gridColor );
              XDrawLine( d, win, gc->normGC(), x0, y0, x0+gridLen, y1 );
              gc->setFG( scaleColor );
            }
          }

          if ( drawScale ) {

          if ( erase )
            XDrawLine( d, win, gc->eraseGC(), x0, y0, x1, y1 );
          else
            XDrawLine( d, win, gc->normGC(), x0, y0, x1, y1 );

	  }

        }

        if ( minors_per_major  > 0 ) {

          val0 = pow( 10, majorVal );
          val1 = val0 * 10;
          minorInc = ( val1 - val0 ) / minors_per_major;
          if ( minorInc <= 0 ) minorInc = 1;

	  //fprintf( stderr, "val0 = %-g\n", val0 );
	  //fprintf( stderr, "val1 = %-g\n", val1 );
	  //fprintf( stderr, "minorInc = %-g\n", minorInc );
	  //fprintf( stderr, "minors_per_major = %-d\n", minors_per_major );

          val = val0 + minorInc;
	  //fprintf( stderr, "val = %-g\n", val );
	  //fprintf( stderr, "adj_min = %-g\n", adj_min );

          for ( iii=1; iii<minors_per_major; iii++ ) {

            minorVal = log10( val );
	    //fprintf( stderr, "minorVal = %-g\n", minorVal );

            x0 = x;
            x1 = x0 - minor_tick_height;
            y0 = (int) rint( yOffset - ( minorVal - adj_min ) * yFactor );
            y1 = y0;

            if ( minorGrid ) {
              gc->setLineStyle( LineOnOffDash );
              if ( erase ) {
                XDrawLine( d, win, gc->eraseGC(), x0, y0, x0+gridLen, y1 );
              }
              else {
                gc->setFG( gridColor );
                XDrawLine( d, win, gc->normGC(), x0, y0, x0+gridLen, y1 );
                gc->setFG( scaleColor );
              }
              gc->setLineStyle( LineSolid );
            }

            if ( drawScale ) {

            if ( erase )
              XDrawLine( d, win, gc->eraseGC(), x0, y0, x1, y1 );
            else
              XDrawLine( d, win, gc->normGC(), x0, y0, x1, y1 );

	    }

            val += minorInc;

	  }

	}

        majorVal += majorInc;

      }

    }

    labelVal += labelInc;

  }

  // draw last label tick

  x0 = x;
  x1 = x0 - label_tick_height;
  y0 = (int) rint( yOffset - ( labelVal - adj_min ) * yFactor );
  y1 = y0;

  if ( labelGrid ) {
    if ( erase ) {
      XDrawLine( d, win, gc->eraseGC(), x0, y0, x0+gridLen, y1 );
    }
    else {
      gc->setFG( gridColor );
      XDrawLine( d, win, gc->normGC(), x0, y0, x0+gridLen, y1 );
      gc->setFG( scaleColor );
    }
  }

  if ( drawScale ) {

  if ( erase )
    XDrawLine( d, win, gc->eraseGC(), x0, y0, x1, y1 );
  else
    XDrawLine( d, win, gc->normGC(), x0, y0, x1, y1 );

  if ( annotateScale ) {
    gc->setFontTag( fontTag, fi );
    x0 = x - (int) ( 1.2 * label_tick_height );
    y1 = y0 - (int) ( fontHeight * 0.5 );
    formatString( pow(10,labelVal), buf, 31 );
    if ( maxConstrained ) {
      gc->setFG( black );
      gc->setBG( white );
    }
    if ( erase )
      xEraseImageText( d, win, gc, fs, x0, y1,
       XmALIGNMENT_END, buf );
    else
      xDrawImageText( d, win, gc, fs, x0, y1,
       XmALIGNMENT_END, buf );
    if ( maxConstrained ) {
     gc->setFG( scaleColor );
      gc->setBG( bgColor );
    }
  }

  }

  gc->restoreFg();
  gc->restoreBg();

}

void drawY2Log10Scale (
  Display *d,
  Window win,
  gcClass *gc,
  int drawScale,
  int x,
  int y,
  int scaleHeight,
  double adj_min,
  double adj_max,
  int num_label_ticks,
  int majors_per_label,
  int minors_per_major,
  unsigned int scaleColor,
  unsigned int bgColor,
  int labelGrid,
  int majorGrid,
  int minorGrid,
  int gridLen,
  unsigned int gridColor,
  fontInfoClass *fi,
  char *fontTag,
  XFontStruct *fs,
  int annotateScale,
  int minConstrained,
  int maxConstrained,
  int erase
) {

int count, ii, iii, x0, y0, x1, y1;
int label_tick_height, major_tick_height, minor_tick_height, first=0;
double yFactor, yOffset, labelVal, majorInc, majorVal,
 minorInc, minorVal, lastInc, labelInc, val, val0, val1;
int fontAscent, fontDescent, fontHeight,
 stringWidth;
char buf[31+1];
unsigned int white, black;

  if ( scaleHeight < 1 ) return;
  if ( num_label_ticks < 1 ) return;
  if ( adj_max <= adj_min ) return;

  white = WhitePixel( d, DefaultScreen(d) );
  black = BlackPixel( d, DefaultScreen(d) );

  gc->saveFg();
  gc->saveBg();

  gc->setLineWidth(1);
  gc->setLineStyle( LineSolid );
  gc->setFG( scaleColor );
  gc->setBG( bgColor );

  labelInc = ( adj_max - adj_min ) / num_label_ticks;
  if ( labelInc <= 0 ) labelInc = 1;

  yFactor = (double) ( scaleHeight ) / ( adj_max - adj_min );
  yOffset = y;

  labelVal = adj_min;

  if ( drawScale ) {

  first = 1;

  updateFontInfo( " ", fontTag, &fs,
   &fontAscent, &fontDescent, &fontHeight,
   &stringWidth );

  /* draw axis */
  x0 = x;
  y0 = y;
  x1 = x0;
  y1 = y0 - scaleHeight;
  if ( erase )
    XDrawLine( d, win, gc->eraseGC(), x0, y0, x1, y1 );
  else
    XDrawLine( d, win, gc->normGC(), x0, y0, x1, y1 );

  }
  else {

    fontHeight = 1;

  }

  label_tick_height = (int) ( 0.8 * (double) abs( fontHeight - 2 ) );
  major_tick_height = (int) ( 0.7 * ( double) label_tick_height );
  minor_tick_height = (int) ( ( double) label_tick_height * 0.4 );

  lastInc = labelInc * 0.5;

  /* draw label ticks */

  count = 0;
  while ( labelVal < ( adj_max - lastInc ) ) {

    x0 = x;
    x1 = x0 + label_tick_height;
    y0 = (int) rint( yOffset - ( labelVal - adj_min ) * yFactor );
    y1 = y0;

    if ( labelGrid && count ) {
      if ( erase ) {
        XDrawLine( d, win, gc->eraseGC(), x0, y0, x0-gridLen, y1 );
      }
      else {
        gc->setFG( gridColor );
        XDrawLine( d, win, gc->normGC(), x0, y0, x0-gridLen, y1 );
        gc->setFG( scaleColor );
      }
    }
    count++;

    if ( drawScale ) {

    if ( erase )
      XDrawLine( d, win, gc->eraseGC(), x0, y0, x1, y1 );
    else
      XDrawLine( d, win, gc->normGC(), x0, y0, x1, y1 );

    if ( annotateScale ) {
      gc->setFontTag( fontTag, fi );
      x0 = x + (int) ( 1.2 * label_tick_height );
      y1 = y0 - (int) ( fontHeight * 0.5 );
      formatString( pow(10,labelVal), buf, 31 );
      if ( minConstrained ) {
        if ( first ) {
          gc->setFG( black );
          gc->setBG( white );
        }
      }
      if ( erase )
        xEraseImageText( d, win, gc, fs, x0, y1,
         XmALIGNMENT_BEGINNING, buf );
      else
        xDrawImageText( d, win, gc, fs, x0, y1,
         XmALIGNMENT_BEGINNING, buf );
      if ( minConstrained ) {
        if ( first ) {
          first = 0;
          gc->setFG( scaleColor );
          gc->setBG( bgColor );
        }
      }
    }

    }

    if ( majors_per_label > 0 ) {

      majorInc = labelInc / majors_per_label;
      if ( majorInc <= 0 ) majorInc = 1;
      majorVal = labelVal;
      for ( ii=0; ii<majors_per_label; ii++ ) {

        if ( ii > 0 ) {

          x0 = x;
          x1 = x0 + major_tick_height;
          y0 = (int) rint( yOffset - ( majorVal - adj_min ) * yFactor );
          y1 = y0;

          if ( majorGrid ) {
            if ( erase ) {
              XDrawLine( d, win, gc->eraseGC(), x0, y0, x0-gridLen, y1 );
            }
            else {
              gc->setFG( gridColor );
              XDrawLine( d, win, gc->normGC(), x0, y0, x0-gridLen, y1 );
              gc->setFG( scaleColor );
            }
          }

          if ( drawScale ) {

          if ( erase )
            XDrawLine( d, win, gc->eraseGC(), x0, y0, x1, y1 );
          else
            XDrawLine( d, win, gc->normGC(), x0, y0, x1, y1 );

	  }

        }

        if ( minors_per_major  > 0 ) {

          val0 = pow( 10, majorVal );
          val1 = val0 * 10;
          minorInc = ( val1 - val0 ) / minors_per_major;
          if ( minorInc <= 0 ) minorInc = 1;

          val = val0 + minorInc;

          for ( iii=1; iii<minors_per_major; iii++ ) {

            minorVal = log10( val );

            x0 = x;
            x1 = x0 + minor_tick_height;
            y0 = (int) rint( yOffset - ( minorVal - adj_min ) * yFactor );
            y1 = y0;

            if ( minorGrid ) {
              gc->setLineStyle( LineOnOffDash );
              if ( erase ) {
                XDrawLine( d, win, gc->eraseGC(), x0, y0, x0-gridLen, y1 );
              }
              else {
                gc->setFG( gridColor );
                XDrawLine( d, win, gc->normGC(), x0, y0, x0-gridLen, y1 );
                gc->setFG( scaleColor );
              }
              gc->setLineStyle( LineSolid );
            }

            if ( drawScale ) {

            if ( erase )
              XDrawLine( d, win, gc->eraseGC(), x0, y0, x1, y1 );
            else
              XDrawLine( d, win, gc->normGC(), x0, y0, x1, y1 );

	    }

            val += minorInc;

	  }

	}

        majorVal += majorInc;

      }

    }

    labelVal += labelInc;

  }

  // draw last label tick

  x0 = x;
  x1 = x0 + label_tick_height;
  y0 = (int) rint( yOffset - ( labelVal - adj_min ) * yFactor );
  y1 = y0;

  if ( labelGrid ) {
    if ( erase ) {
      XDrawLine( d, win, gc->eraseGC(), x0, y0, x0-gridLen, y1 );
    }
    else {
      gc->setFG( gridColor );
      XDrawLine( d, win, gc->normGC(), x0, y0, x0-gridLen, y1 );
      gc->setFG( scaleColor );
    }
  }

  if ( drawScale ) {

  if ( erase )
    XDrawLine( d, win, gc->eraseGC(), x0, y0, x1, y1 );
  else
    XDrawLine( d, win, gc->normGC(), x0, y0, x1, y1 );

  if ( annotateScale ) {
    gc->setFontTag( fontTag, fi );
    x0 = x + (int) ( 1.2 * label_tick_height );
    y1 = y0 - (int) ( fontHeight * 0.5 );
    formatString( pow(10,labelVal), buf, 31 );
    if ( maxConstrained ) {
      gc->setFG( black );
      gc->setBG( white );
    }
    if ( erase )
      xEraseImageText( d, win, gc, fs, x0, y1,
       XmALIGNMENT_BEGINNING, buf );
    else
      xDrawImageText( d, win, gc, fs, x0, y1,
       XmALIGNMENT_BEGINNING, buf );
    if ( maxConstrained ) {
     gc->setFG( scaleColor );
      gc->setBG( bgColor );
    }
  }

  }

  gc->restoreFg();
  gc->restoreBg();

}

int yScaleWidth (
  char *fontTag,
  XFontStruct *fs,
  double adj_min,
  double adj_max,
  int num_label_ticks
) {

int fontAscent, fontDescent, fontHeight,
 stringWidth, label_tick_length, scaleWidth, l;
char buf[31+1];
double labelInc, lastInc, labelVal, z;

  if ( num_label_ticks < 1 ) return 10; // pixels
  if ( adj_max <= adj_min ) return 10; // pixels

  updateFontInfo( " ", fontTag, &fs,
   &fontAscent, &fontDescent, &fontHeight,
   &stringWidth );

  label_tick_length = (int) ( 0.8 * (double) abs( fontHeight - 2 ) );

  labelInc = ( adj_max - adj_min ) / num_label_ticks;
  if ( labelInc <= 0 ) labelInc = 1;

  lastInc = labelInc * 0.5;
  labelVal = adj_min;
  scaleWidth = 0;
  while ( labelVal < ( adj_max + lastInc ) ) {

    z = fabs( labelVal - 0.0 ) / labelInc;
    if ( z < 1e-5 ) {
      formatString( 0.0, buf, 31 );
    }
    else {
      formatString( labelVal, buf, 31 );
    }
    if ( fs ) {
      l = XTextWidth( fs, buf, strlen(buf) );
    }
    else {
      l = 0;
    }
    if ( l > scaleWidth ) scaleWidth = l;

    labelVal += labelInc;

  }

  scaleWidth += label_tick_length + 6;

  return scaleWidth;

}

int yLog10ScaleWidth (
  char *fontTag,
  XFontStruct *fs,
  double adj_min,
  double adj_max,
  int num_label_ticks
) {

int fontAscent, fontDescent, fontHeight,
 stringWidth, label_tick_length, scaleWidth, l;
char buf[31+1];
double labelInc, lastInc, labelVal, z, log10Val;

  if ( num_label_ticks < 1 ) return 10; // pixels
  if ( adj_max <= adj_min ) return 10; // pixels

  updateFontInfo( " ", fontTag, &fs,
   &fontAscent, &fontDescent, &fontHeight,
   &stringWidth );

  label_tick_length = (int) ( 0.8 * (double) abs( fontHeight - 2 ) );

  labelInc = ( adj_max - adj_min ) / num_label_ticks;
  if ( labelInc <= 0 ) labelInc = 1;

  lastInc = labelInc * 0.5;
  labelVal = adj_min;
  scaleWidth = 0;
  while ( labelVal < ( adj_max + lastInc ) ) {

    log10Val = pow(10,labelVal);

    z = fabs( log10Val - 0.0 ) / labelInc;
    if ( z < 1e-5 ) {
      formatString( 0.0, buf, 31 );
    }
    else {
      formatString( log10Val, buf, 31 );
    }
    if ( fs ) {
      l = XTextWidth( fs, buf, strlen(buf) );
    }
    else {
      l = 0;
    }
    if ( l > scaleWidth ) scaleWidth = l;

    labelVal += labelInc;

  }

  scaleWidth += label_tick_length + 6;

  return scaleWidth;

}

int yScaleMargin (
  char *fontTag,
  XFontStruct *fs
) {

int fontAscent, fontDescent, fontHeight, stringWidth;

  updateFontInfo( " ", fontTag, &fs,
   &fontAscent, &fontDescent, &fontHeight,
   &stringWidth );

  return fontHeight;

}

void getYLimitCoords (
  int x,
  int y,
  int scaleHeight,
  double adj_min,
  double adj_max,
  int num_label_ticks,
  char *fontTag,
  XFontStruct *fs,
  int *yMinX0,
  int *yMinX1,
  int *yMinY0,
  int *yMinY1,
  int *yMaxX0,
  int *yMaxX1,
  int *yMaxY0,
  int *yMaxY1
) {

int x0, y0, x1, y1, count;
int label_tick_height;
double yFactor, yOffset, labelVal, lastInc, labelInc, z;
int fontAscent, fontDescent, fontHeight, stringWidth;
char buf[31+1];

  if ( scaleHeight < 1 ) return;
  if ( num_label_ticks < 1 ) return;
  if ( adj_max <= adj_min ) return;

  labelInc = ( adj_max - adj_min ) / num_label_ticks;
  if ( labelInc <= 0 ) labelInc = 1;

  yFactor = (double) ( scaleHeight ) / ( adj_max - adj_min );
  yOffset = y;

  labelVal = adj_min;

  updateFontInfo( " ", fontTag, &fs,
   &fontAscent, &fontDescent, &fontHeight,
   &stringWidth );

  label_tick_height = (int) ( 0.8 * (double) abs( fontHeight - 2 ) );
  x0 = x - (int) ( 1.2 * label_tick_height );
  x1 = x0;
  y0 = y - (int) ( 0.5 * fontHeight );
  y1 = y0 + fontHeight;

  lastInc = labelInc * 0.5;

  /* draw label ticks */

  count = 0;
  while ( labelVal < ( adj_max + lastInc ) ) {

    z = fabs( labelVal - 0.0 ) / labelInc;
    if ( z < 1e-5 ) {
      formatString( 0.0, buf, 31 );
    }
    else {
      formatString( labelVal, buf, 31 );
    }

    if ( fs ) {
      stringWidth = XTextWidth( fs, buf, strlen(buf) );
    }
    else {
      stringWidth = 0;
    }

    if ( count == 0 ) {
      *yMinX0 = x0 - stringWidth;
      *yMinX1 = x0;
      *yMinY0 = y0;
      *yMinY1 = y1;
    }
    else {
      *yMaxX0 = x0 - stringWidth;
      *yMaxX1 = x0;
      *yMaxY0 = y0;
      *yMaxY1 = y1;
    }

    count++;

    labelVal += labelInc;
    y0 = (int) ( rint( yOffset - ( labelVal - adj_min ) * yFactor ) -
     fontHeight * 0.5 );
    y1 = y0 + fontHeight;

  }

}

void getY2LimitCoords (
  int x,
  int y,
  int scaleHeight,
  double adj_min,
  double adj_max,
  int num_label_ticks,
  char *fontTag,
  XFontStruct *fs,
  int *yMinX0,
  int *yMinX1,
  int *yMinY0,
  int *yMinY1,
  int *yMaxX0,
  int *yMaxX1,
  int *yMaxY0,
  int *yMaxY1
) {

int x0, y0, x1, y1, count;
int label_tick_height;
double yFactor, yOffset, labelVal, lastInc, labelInc, z;
int fontAscent, fontDescent, fontHeight, stringWidth;
char buf[31+1];

  if ( scaleHeight < 1 ) return;
  if ( num_label_ticks < 1 ) return;
  if ( adj_max <= adj_min ) return;

  labelInc = ( adj_max - adj_min ) / num_label_ticks;
  if ( labelInc <= 0 ) labelInc = 1;

  yFactor = (double) ( scaleHeight ) / ( adj_max - adj_min );
  yOffset = y;

  labelVal = adj_min;

  updateFontInfo( " ", fontTag, &fs,
   &fontAscent, &fontDescent, &fontHeight,
   &stringWidth );

  label_tick_height = (int) ( 0.8 * (double) abs( fontHeight - 2 ) );
  x0 = x + (int) ( 1.2 * label_tick_height );
  x1 = x0;
  y0 = y - (int) ( 0.5 * fontHeight );
  y1 = y0 + fontHeight;

  lastInc = labelInc * 0.5;

  /* draw label ticks */

  count = 0;
  while ( labelVal < ( adj_max + lastInc ) ) {

    z = fabs( labelVal - 0.0 ) / labelInc;
    if ( z < 1e-5 ) {
      formatString( 0.0, buf, 31 );
    }
    else {
      formatString( labelVal, buf, 31 );
    }

    if ( fs ) {
      stringWidth = XTextWidth( fs, buf, strlen(buf) );
    }
    else {
      stringWidth = 0;
    }

    if ( count == 0 ) {
      *yMinX0 = x0;
      *yMinX1 = x0 + stringWidth;
      *yMinY0 = y0;
      *yMinY1 = y1;
    }
    else {
      *yMaxX0 = x0;
      *yMaxX1 = x0 + stringWidth;
      *yMaxY0 = y0;
      *yMaxY1 = y1;
    }

    count++;

    labelVal += labelInc;
    y0 = (int) ( rint( yOffset - ( labelVal - adj_min ) * yFactor ) -
     fontHeight * 0.5 );
    y1 = y0 + fontHeight;

  }

}

void getYLog10LimitCoords (
  int x,
  int y,
  int scaleHeight,
  double adj_min,
  double adj_max,
  int num_label_ticks,
  char *fontTag,
  XFontStruct *fs,
  int *yMinX0,
  int *yMinX1,
  int *yMinY0,
  int *yMinY1,
  int *yMaxX0,
  int *yMaxX1,
  int *yMaxY0,
  int *yMaxY1
) {

int x0, y0, x1, y1, count;
int label_tick_height;
double yFactor, yOffset, labelVal, lastInc, labelInc;
int fontAscent, fontDescent, fontHeight, stringWidth;
char buf[31+1];

  if ( scaleHeight < 1 ) return;
  if ( num_label_ticks < 1 ) return;
  if ( adj_max <= adj_min ) return;

  labelInc = ( adj_max - adj_min ) / num_label_ticks;
  if ( labelInc <= 0 ) labelInc = 1;

  yFactor = (double) ( scaleHeight ) / ( adj_max - adj_min );
  yOffset = y;

  labelVal = adj_min;

  updateFontInfo( " ", fontTag, &fs,
   &fontAscent, &fontDescent, &fontHeight,
   &stringWidth );

  label_tick_height = (int) ( 0.8 * (double) abs( fontHeight - 2 ) );
  x0 = x - (int) ( 1.2 * label_tick_height );
  x1 = x0;
  y0 = y - (int) ( 0.5 * fontHeight );
  y1 = y0 + fontHeight;

  lastInc = labelInc * 0.5;

  /* draw label ticks */

  count = 0;
  while ( labelVal < ( adj_max + lastInc ) ) {

    formatString( pow(10,labelVal), buf, 31 );

    if ( fs ) {
      stringWidth = XTextWidth( fs, buf, strlen(buf) );
    }
    else {
      stringWidth = 0;
    }

    if ( count == 0 ) {
      *yMinX0 = x0 - stringWidth;
      *yMinX1 = x0;
      *yMinY0 = y0;
      *yMinY1 = y1;
    }
    else {
      *yMaxX0 = x0 - stringWidth;
      *yMaxX1 = x0;
      *yMaxY0 = y0;
      *yMaxY1 = y1;
    }

    count++;

    labelVal += labelInc;
    y0 = (int) ( rint( yOffset - ( labelVal - adj_min ) * yFactor ) -
     fontHeight * 0.5 );
    y1 = y0 + fontHeight;

  }

}

void getY2Log10LimitCoords (
  int x,
  int y,
  int scaleHeight,
  double adj_min,
  double adj_max,
  int num_label_ticks,
  char *fontTag,
  XFontStruct *fs,
  int *yMinX0,
  int *yMinX1,
  int *yMinY0,
  int *yMinY1,
  int *yMaxX0,
  int *yMaxX1,
  int *yMaxY0,
  int *yMaxY1
) {

int x0, y0, x1, y1, count;
int label_tick_height;
double yFactor, yOffset, labelVal, lastInc, labelInc;
int fontAscent, fontDescent, fontHeight, stringWidth;
char buf[31+1];

  if ( scaleHeight < 1 ) return;
  if ( num_label_ticks < 1 ) return;
  if ( adj_max <= adj_min ) return;

  labelInc = ( adj_max - adj_min ) / num_label_ticks;
  if ( labelInc <= 0 ) labelInc = 1;

  yFactor = (double) ( scaleHeight ) / ( adj_max - adj_min );
  yOffset = y;

  labelVal = adj_min;

  updateFontInfo( " ", fontTag, &fs,
   &fontAscent, &fontDescent, &fontHeight,
   &stringWidth );

  label_tick_height = (int) ( 0.8 * (double) abs( fontHeight - 2 ) );
  x0 = x + (int) ( 1.2 * label_tick_height );
  x1 = x0;
  y0 = y - (int) ( 0.5 * fontHeight );
  y1 = y0 + fontHeight;

  lastInc = labelInc * 0.5;

  /* draw label ticks */

  count = 0;
  while ( labelVal < ( adj_max + lastInc ) ) {

    formatString( pow(10,labelVal), buf, 31 );

    if ( fs ) {
      stringWidth = XTextWidth( fs, buf, strlen(buf) );
    }
    else {
      stringWidth = 0;
    }

    if ( count == 0 ) {
      *yMinX0 = x0;
      *yMinX1 = x0 + stringWidth;
      *yMinY0 = y0;
      *yMinY1 = y1;
    }
    else {
      *yMaxX0 = x0;
      *yMaxX1 = x0 + stringWidth;
      *yMaxY0 = y0;
      *yMaxY1 = y1;
    }

    count++;

    labelVal += labelInc;
    y0 = (int) ( rint( yOffset - ( labelVal - adj_min ) * yFactor ) -
     fontHeight * 0.5 );
    y1 = y0 + fontHeight;

  }

}

int intersects (
  int x0,
  int y0,
  int x1,
  int y1,
  int xx0,
  int yy0,
  int xx1,
  int yy1
) {

  if ( xx0 > x1 ) return 0;
  if ( xx1 < x0 ) return 0;
  if ( yy0 > y1 ) return 0;
  if ( yy1 < y0 ) return 0;

  if ( ( x0 <= xx0 ) &&
       ( x1 >= xx1 ) &&
       ( yy0 <= y0 ) &&
       ( yy1 >= y1 ) ) {

    return 1;

  }

  if ( ( y0 <= yy0 ) &&
       ( y1 >= yy1 ) &&
       ( xx0 <= x0 ) &&
       ( xx1 >= x1 ) ) {

    return 1;

  }

  if ( ( x0 >= xx0 ) &&
       ( x0 <= xx1 ) &&
       ( y0 >= yy0 ) &&
       ( y0 <= yy1 ) ) {

    return 1;

  }

  if ( ( x0 >= xx0 ) &&
       ( x0 <= xx1 ) &&
       ( y1 >= yy0 ) &&
       ( y1 <= yy1 ) ) {

    return 1;

  }

  if ( ( x1 >= xx0 ) &&
       ( x1 <= xx1 ) &&
       ( y0 >= yy0 ) &&
       ( y0 <= yy1 ) ) {

    return 1;

  }

  if ( ( x1 >= xx0 ) &&
       ( x1 <= xx1 ) &&
       ( y1 >= yy0 ) &&
       ( y1 <= yy1 ) ) {

    return 1;

  }

  if ( ( xx0 >= x0 ) &&
       ( xx0 <= x1 ) &&
       ( yy0 >= y0 ) &&
       ( yy0 <= y1 ) ) {

    return 1;

  }

  if ( ( xx0 >= x0 ) &&
       ( xx0 <= x1 ) &&
       ( yy1 >= y0 ) &&
       ( yy1 <= y1 ) ) {

    return 1;

  }

  if ( ( xx1 >= x0 ) &&
       ( xx1 <= x1 ) &&
       ( yy0 >= y0 ) &&
       ( yy0 <= y1 ) ) {

    return 1;

  }

  if ( ( xx1 >= x0 ) &&
       ( xx1 <= x1 ) &&
       ( yy1 >= y0 ) &&
       ( yy1 <= y1 ) ) {

    return 1;

  }

  return 0;

}
