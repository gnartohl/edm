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

#include "app_pkg.h"

#include "thread.h"
#include "edm.version"
#include <unistd.h>

typedef struct libRecTag {
  struct libRecTag *flink;
  char *className;
  char *fileName;
  char *typeName;
  char *text;
} libRecType, *libRecPtr;

static void manageComponents (
  char *op,
  char *libFile )
{

typedef int (*REGFUNC)( char **, char **, char ** );
REGFUNC func;

int stat, index, comment, fileExists, fileEmpty;
char *classNamePtr, *typeNamePtr, *textPtr, *error;
void *dllHandle;
char fileName[255+1], prefix[255+1], line[255+1], buf[255+1];
int numComponents = 0;
int numToAdd = 0;
char *envPtr, *tk, *more;
FILE *f;
libRecPtr head, tail, cur;

  head = new libRecType;
  tail = head;
  tail->flink = NULL;

  envPtr = getenv(environment_str3);
  if ( envPtr ) {
    strncpy( prefix, envPtr, 255 );
    if ( prefix[strlen(prefix)-1] != '/' ) strncat( prefix, "/", 255 );
  }
  else {
    strcpy( prefix, "/etc/" );
  }

  strncpy( fileName, prefix, 255 );
  strncat( fileName, "edmObjects", 255 );

  f = fopen( fileName, "r" );
  if ( f ) {

    printf( appContextClass_str1, fileName );

    fileExists = 1;
    fileEmpty = 0;

    // read in existing components

    more = fgets( line, 255, f );
    if ( more ) {
      tk = strtok( line, "\n" );
      numComponents = atol( tk );
      if ( numComponents <= 0 ) {
	printf( "3\n" );
        printf( appContextClass_str2, fileName );
        return;
      }
    }
    else {
      printf( "4\n" );
      printf( appContextClass_str2, fileName );
      fileEmpty = 1;
      fclose( f );
    }

    if ( !fileEmpty ) {

      index = 0;
      do {

        more = fgets( line, 255, f );
        if ( more ) {

          cur = new libRecType;

          strncpy( buf, line, 255 );

          comment = 0;
          tk = strtok( buf, " \t\n" );

          if ( !tk ) {
            comment = 1;
          }
          else if ( tk[0] == '#' ) {
            comment = 1;
          }

          if ( !comment ) { /* not an empty line or comment */

            tail->flink = cur;
            tail = cur;
            tail->flink = NULL;

            tk = strtok( line, " \t\n" );
            if ( !tk ) {
              printf( appContextClass_str3 );
              return;
            }
            cur->className = new char[strlen(tk)+1];
            strcpy( cur->className, tk );

            tk = strtok( NULL, " \t\n" );
            if ( !tk ) {
              printf( appContextClass_str3 );
              return;
            }
            cur->fileName = new char[strlen(tk)+1];
            strcpy( cur->fileName, tk );

            tk = strtok( NULL, " \t\n" );
            if ( !tk ) {
              printf( appContextClass_str3 );
              return;
            }
            cur->typeName = new char[strlen(tk)+1];
            strcpy( cur->typeName, tk );

            tk = strtok( NULL, "\n" );
            if ( !tk ) {
              printf( appContextClass_str3 );
              return;
            }
            cur->text = new char[strlen(tk)+1];
            strcpy( cur->text, tk );

            index++;

          }

        }

      } while ( more );

      fclose( f );

      if ( index != numComponents ) {
        printf( appContextClass_str4, fileName );
        return;
      }

    }
    else {

      fileExists = 0; // file was empty so behave as if file does not exist

    }

  }
  else {
    fileExists = 0;
  }

  if ( libFile[0] != '/' ) {
    printf( appContextClass_str5 );
    printf( appContextClass_str6 );
    return;
  }

  dllHandle = dlopen( libFile, RTLD_LAZY );
  if ((error = dlerror()) != NULL)  {
    fputs(error, stderr);
    fputs( "\n", stderr );
    return;
  }

  if ( strcmp( op, global_str8 ) == 0 ) {

    printf( "\n" );

    strcpy( line, "firstRegRecord" );
    func = (REGFUNC) dlsym( dllHandle, line );
    if ((error = dlerror()) != NULL)  {
      fputs( appContextClass_str8, stderr );
      return;
    }

    stat = (*func)( &classNamePtr, &typeNamePtr, &textPtr );

    strcpy( line, "nextRegRecord" );
    func = (REGFUNC) dlsym( dllHandle, line );
    if ((error = dlerror()) != NULL)  {
      fputs( appContextClass_str9, stderr );
      return;
    }

    if ( !stat ) {
      strncpy( line, appContextClass_str10, 255 );
      strncat( line, appContextClass_str11, 255 );
      printf( "%s\n\n", line );
    }

    while ( !stat ) {

      strncpy( line, classNamePtr, 255 );

      if ( strlen(line) < 37 )
        index = 42;
      else
        index = strlen(line) + 5;
      strncat( line, "                                        ", 255 );
      strncpy( &line[index], typeNamePtr, 255 );

      if ( strlen(line) < 50 )
        index = 55;
      else
        index = strlen(line) + 5;
      strncat( line, "                                        ", 255 );
      strncpy( &line[index], textPtr, 255 );

      printf( "%s\n", line );

      stat = (*func)( &classNamePtr, &typeNamePtr, &textPtr );

    }

    printf( "\n\n" );

  }
  else if ( strcmp( op, global_str6 ) == 0 ) {

    if ( !fileExists ) {
      printf( appContextClass_str13, fileName );
    }

    printf( "\n" );

    strcpy( line, "firstRegRecord" );
    func = (REGFUNC) dlsym( dllHandle, line );
    if ((error = dlerror()) != NULL)  {
      fputs( appContextClass_str14, stderr );
      return;
    }

    stat = (*func)( &classNamePtr, &typeNamePtr, &textPtr );

    strcpy( line, "nextRegRecord" );
    func = (REGFUNC) dlsym( dllHandle, line );
    if ((error = dlerror()) != NULL)  {
      fputs( appContextClass_str9, stderr );
      return;
    }

    numToAdd = 0;
    while ( !stat ) {

      cur = head->flink;
      while ( cur ) {
        if ( strcmp( classNamePtr, cur->className ) == 0 ) {
          printf( appContextClass_str15, classNamePtr );
          return;
	}
        cur = cur->flink;
      }

      numToAdd++;

      printf( appContextClass_str16,
       classNamePtr, typeNamePtr, textPtr );

      cur = new libRecType;
      tail->flink = cur;
      tail = cur;
      tail->flink = NULL;

      cur->className = new (char)[strlen(classNamePtr)+1];
      strcpy( cur->className, classNamePtr );
      cur->fileName = new (char)[strlen(libFile)+1];
      strcpy( cur->fileName, libFile );
      cur->typeName = new (char)[strlen(typeNamePtr)+1];
      strcpy( cur->typeName, typeNamePtr );
      cur->text = new (char)[strlen(textPtr)+1];
      strcpy( cur->text, textPtr );

      numComponents++;

      stat = (*func)( &classNamePtr, &typeNamePtr, &textPtr );

    }

    if ( numToAdd == 0 ) {
      printf(
       appContextClass_str17, fileName );
      return;
    }

    printf( "\n" );

    strncpy( line, fileName, 255 );
    strncat( line, "~", 255 );

    if ( fileExists ) {

      stat = unlink( line );

      printf( appContextClass_str18, line );
      stat = rename( fileName, line );
      if ( stat ) {
        perror( appContextClass_str19 );
        return;
      }

    }

    f = fopen( fileName, "w" );
    if ( f ) {

      fprintf( f, "%-d\n", numComponents );
      cur = head->flink;
      while ( cur ) {
        fprintf( f, "%s %s %s %s\n", cur->className, cur->fileName,
         cur->typeName, cur->text );
        cur = cur->flink;
      }

    }
    else {
      perror( fileName );
      return;
    }

    if ( numToAdd == 1 )
      printf( appContextClass_str20 );
    else if ( numToAdd > 1 )
      printf( appContextClass_str21 );

  }
  else if ( strcmp( op, global_str7 ) == 0 ) {

    printf( "\n" );

    printf( appContextClass_str23 );

  }

}

static void managePvComponents (
  char *op,
  char *libFile )
{

typedef int (*PVREGFUNC)( char **, char ** );
PVREGFUNC func;

int stat, index, comment, fileExists, fileEmpty;
char *classNamePtr, *textPtr, *error;
void *dllHandle;
char fileName[255+1], prefix[255+1], line[255+1], buf[255+1];
int numComponents = 0;
int numToAdd = 0;
char *envPtr, *tk, *more;
FILE *f;
libRecPtr head, tail, cur;

  head = new libRecType;
  tail = head;
  tail->flink = NULL;

  envPtr = getenv(environment_str3);
  if ( envPtr ) {
    strncpy( prefix, envPtr, 255 );
    if ( prefix[strlen(prefix)-1] != '/' ) strncat( prefix, "/", 255 );
  }
  else {
    strcpy( prefix, "/etc/" );
  }

  strncpy( fileName, prefix, 255 );
  strncat( fileName, "edmPvObjects", 255 );

  _edmDebug();

  f = fopen( fileName, "r" );
  if ( f ) {

    printf( appContextClass_str1, fileName );

    fileExists = 1;
    fileEmpty = 0;

    // read in existing components

    more = fgets( line, 255, f );
    if ( more ) {
      tk = strtok( line, "\n" );
      numComponents = atol( tk );
      if ( numComponents <= 0 ) {
	printf( "3\n" );
        printf( appContextClass_str2, fileName );
        return;
      }
    }
    else {
      printf( "4\n" );
      printf( appContextClass_str2, fileName );
      fileEmpty = 1;
      fclose( f );
    }

    if ( !fileEmpty ) {

      index = 0;
      do {

        more = fgets( line, 255, f );
        if ( more ) {

          cur = new libRecType;

          strncpy( buf, line, 255 );

          comment = 0;
          tk = strtok( buf, " \t\n" );

          if ( !tk ) {
            comment = 1;
          }
          else if ( tk[0] == '#' ) {
            comment = 1;
          }

          if ( !comment ) { /* not an empty line or comment */

            tail->flink = cur;
            tail = cur;
            tail->flink = NULL;

            tk = strtok( line, " \t\n" );
            if ( !tk ) {
              printf( appContextClass_str3 );
              return;
            }
            cur->className = new char[strlen(tk)+1];
            strcpy( cur->className, tk );

            tk = strtok( NULL, " \t\n" );
            if ( !tk ) {
              printf( appContextClass_str3 );
              return;
            }
            cur->fileName = new char[strlen(tk)+1];
            strcpy( cur->fileName, tk );

            tk = strtok( NULL, "\n" );
            if ( !tk ) {
              printf( appContextClass_str3 );
              return;
            }
            cur->text = new char[strlen(tk)+1];
            strcpy( cur->text, tk );

            index++;

          }

        }

      } while ( more );

      fclose( f );

      if ( index != numComponents ) {
        printf( appContextClass_str4, fileName );
        return;
      }

    }
    else {

      fileExists = 0; // file was empty so behave as if file does not exist

    }

  }
  else {
    fileExists = 0;
  }

  if ( libFile[0] != '/' ) {
    printf( appContextClass_str5 );
    printf( appContextClass_str6 );
    return;
  }

  dllHandle = dlopen( libFile, RTLD_LAZY );
  if ((error = dlerror()) != NULL)  {
    fputs(error, stderr);
    fputs( "\n", stderr );
    return;
  }

  if ( strcmp( op, global_str65 ) == 0 ) {

    printf( "\n" );

    strcpy( line, "firstPvRegRecord" );
    func = (PVREGFUNC) dlsym( dllHandle, line );
    if ((error = dlerror()) != NULL)  {
      fputs( appContextClass_str8, stderr );
      return;
    }

    stat = (*func)( &classNamePtr, &textPtr );

    strcpy( line, "nextPvRegRecord" );
    func = (PVREGFUNC) dlsym( dllHandle, line );
    if ((error = dlerror()) != NULL)  {
      fputs( appContextClass_str9, stderr );
      return;
    }

    if ( !stat ) {
      strncpy( line, appContextClass_str110, 255 );
      strncat( line, appContextClass_str111, 255 );
      printf( "%s\n\n", line );
    }

    while ( !stat ) {

      strncpy( line, classNamePtr, 255 );

      if ( strlen(line) < 45 )
        index = 45;
      else
        index = strlen(line) + 5;
      strncat( line, "                                        ", 255 );
      strncpy( &line[index], textPtr, 255 );

      printf( "%s\n", line );

      // get next
      stat = (*func)( &classNamePtr, &textPtr );

    }

    printf( "\n\n" );

  }
  else if ( strcmp( op, global_str63 ) == 0 ) {

    if ( !fileExists ) {
      printf( appContextClass_str13, fileName );
    }

    printf( "\n" );

    strcpy( line, "firstPvRegRecord" );
    func = (PVREGFUNC) dlsym( dllHandle, line );
    if ((error = dlerror()) != NULL)  {
      fputs( appContextClass_str14, stderr );
      return;
    }

    stat = (*func)( &classNamePtr, &textPtr );

    strcpy( line, "nextPvRegRecord" );
    func = (PVREGFUNC) dlsym( dllHandle, line );
    if ((error = dlerror()) != NULL)  {
      fputs( appContextClass_str9, stderr );
      return;
    }

    numToAdd = 0;
    while ( !stat ) {

      cur = head->flink;
      while ( cur ) {
        if ( strcmp( classNamePtr, cur->className ) == 0 ) {
          printf( appContextClass_str15, classNamePtr );
          return;
	}
        cur = cur->flink;
      }

      numToAdd++;

      printf( appContextClass_str109, classNamePtr, textPtr );

      cur = new libRecType;
      tail->flink = cur;
      tail = cur;
      tail->flink = NULL;

      cur->className = new (char)[strlen(classNamePtr)+1];
      strcpy( cur->className, classNamePtr );
      cur->fileName = new (char)[strlen(libFile)+1];
      strcpy( cur->fileName, libFile );
      cur->text = new (char)[strlen(textPtr)+1];
      strcpy( cur->text, textPtr );

      numComponents++;

      stat = (*func)( &classNamePtr, &textPtr );

    }

    if ( numToAdd == 0 ) {
      printf(
       appContextClass_str17, fileName );
      return;
    }

    printf( "\n" );

    strncpy( line, fileName, 255 );
    strncat( line, "~", 255 );

    if ( fileExists ) {

      stat = unlink( line );

      printf( appContextClass_str18, line );
      stat = rename( fileName, line );
      if ( stat ) {
        perror( appContextClass_str19 );
        return;
      }

    }

    f = fopen( fileName, "w" );
    if ( f ) {

      fprintf( f, "%-d\n", numComponents );
      cur = head->flink;
      while ( cur ) {
        fprintf( f, "%s %s %s\n", cur->className, cur->fileName,
         cur->text );
        cur = cur->flink;
      }

    }
    else {
      perror( fileName );
      return;
    }

    if ( numToAdd == 1 )
      printf( appContextClass_str20 );
    else if ( numToAdd > 1 )
      printf( appContextClass_str21 );

  }
  else if ( strcmp( op, global_str64 ) == 0 ) {

    printf( "\n" );

    printf( appContextClass_str23 );

  }

}

#ifdef __epics__

void ctlPvUpdate (
  struct event_handler_args ast_args )
{

appContextClass *apco = (appContextClass *) ast_args.usr;
char *fName, str[41], name[127+1];
int stat;
activeWindowListPtr cur;

  fName = (char *) ast_args.dbr;

  if ( strcmp( fName, "" ) == 0 ) {
    strcpy( str, "" );
    stat = ca_put( DBR_STRING, apco->ctlPvId, &str );
    return;
  }

  getFileName( name, fName, 127 );

  cur = apco->head->flink;
  while ( cur != apco->head ) {
    if ( strcmp( name, cur->node.name ) == 0 ) {
      // deiconify
      XMapWindow( cur->node.d, XtWindow(cur->node.topWidgetId()) );
      // raise
      XRaiseWindow( cur->node.d, XtWindow(cur->node.topWidgetId()) );
      strcpy( str, "" );
      stat = ca_put( DBR_STRING, apco->ctlPvId, &str );
      return;  // display is already open; don't open another instance
    }
    cur = cur->flink;
  }

  cur = new activeWindowListType;
  cur->requestDelete = 0;
  cur->requestActivate = 0;
  cur->requestReactivate = 0;
  cur->requestOpen = 0;
  cur->requestPosition = 0;
  cur->requestCascade = 0;
  cur->requestImport = 0;
  cur->requestRefresh = 0;

  cur->node.create( apco, NULL, 0, 0, 0, 0, apco->numMacros, apco->macros,
   apco->expansions );
  cur->node.realize();
  cur->node.setGraphicEnvironment( &apco->ci, &apco->fi );

  cur->blink = apco->head->blink;
  apco->head->blink->flink = cur;
  apco->head->blink = cur;
  cur->flink = apco->head;

  cur->node.storeFileName( fName );

  cur->requestOpen = 1;
  (apco->requestFlag)++;

  cur->requestActivate = 1;
  (apco->requestFlag)++;

  strcpy( str, "" );
  stat = ca_put( DBR_STRING, apco->ctlPvId, &str );

}

#else

#ifdef GENERIC_PV

void ctlPvUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args
) {

appContextClass *apco = (appContextClass *) clientData;
char *fName, str[41], name[127+1];
int stat;
activeWindowListPtr cur;

  apco->proc->lock();

  fName = (char *) apco->ctlPvId->getValue( args );

  if ( strcmp( fName, "" ) == 0 ) {
    strcpy( str, "" );
    stat = apco->ctlPvId->put( apco->ctlPvId->pvrString(), str );
    apco->proc->unlock();
    return;
  }

  getFileName( name, fName, 127 );

  cur = apco->head->flink;
  while ( cur != apco->head ) {
    if ( strcmp( name, cur->node.name ) == 0 ) {
      // deiconify
      XMapWindow( cur->node.d, XtWindow(cur->node.topWidgetId()) );
      // raise
      XRaiseWindow( cur->node.d, XtWindow(cur->node.topWidgetId()) );
      strcpy( str, "" );
      stat = apco->ctlPvId->put( apco->ctlPvId->pvrString(), str );
      apco->proc->unlock();
      return;  // display is already open; don't open another instance
    }
    cur = cur->flink;
  }

  cur = new activeWindowListType;
  cur->requestDelete = 0;
  cur->requestActivate = 0;
  cur->requestReactivate = 0;
  cur->requestOpen = 0;
  cur->requestPosition = 0;
  cur->requestCascade = 0;
  cur->requestImport = 0;
  cur->requestRefresh = 0;

  cur->node.create( apco, NULL, 0, 0, 0, 0, apco->numMacros, apco->macros,
   apco->expansions );
  cur->node.realize();
  cur->node.setGraphicEnvironment( &apco->ci, &apco->fi );

  cur->blink = apco->head->blink;
  apco->head->blink->flink = cur;
  apco->head->blink = cur;
  cur->flink = apco->head;

  cur->node.storeFileName( fName );

  cur->requestOpen = 1;
  (apco->requestFlag)++;

  cur->requestActivate = 1;
  (apco->requestFlag)++;

  strcpy( str, "" );
  stat = apco->ctlPvId->put( apco->ctlPvId->pvrString(), str );

  apco->proc->unlock();

}

#endif

#endif

void app_fileSelectOk_cb (
  Widget w,
  XtPointer client,
  XtPointer call )
{

XmFileSelectionBoxCallbackStruct *cbs =
 (XmFileSelectionBoxCallbackStruct *) call;
appContextClass *apco = (appContextClass *) client;
activeWindowListPtr cur;
char *fName;

  if ( !XmStringGetLtoR( cbs->value, XmFONTLIST_DEFAULT_TAG, &fName ) ) {
    goto done;
  }

  if ( !*fName ) {
    XtFree( fName );
    goto done;
  }

  cur = new activeWindowListType;

  cur->requestDelete = 0;
  cur->requestActivate = 0;
  cur->requestReactivate = 0;
  cur->requestOpen = 0;
  cur->requestPosition = 0;
  cur->requestCascade = 0;
  cur->requestImport = 0;
  cur->requestRefresh = 0;

  cur->node.create( apco, NULL, 0, 0, 0, 0, apco->numMacros, apco->macros,
   apco->expansions );
  cur->node.realize();
  cur->node.setGraphicEnvironment( &apco->ci, &apco->fi );

  cur->blink = apco->head->blink;
  apco->head->blink->flink = cur;
  apco->head->blink = cur;
  cur->flink = apco->head;

  cur->node.storeFileName( fName );

  XtFree( fName );

  cur->requestOpen = 1;
  (apco->requestFlag)++;

  if ( apco->executeOnOpen ) {
    cur->requestActivate = 1;
    (apco->requestFlag)++;
  }

done:

  XtUnmanageChild( w );

}

void app_fileSelectCancel_cb (
  Widget w,
  XtPointer client,
  XtPointer call )
{

  XtUnmanageChild( w );

}

void app_importSelectOk_cb (
  Widget w,
  XtPointer client,
  XtPointer call )
{

XmFileSelectionBoxCallbackStruct *cbs =
 (XmFileSelectionBoxCallbackStruct *) call;
appContextClass *apco = (appContextClass *) client;
activeWindowListPtr cur;
char *fName;

  if ( !XmStringGetLtoR( cbs->value, XmFONTLIST_DEFAULT_TAG, &fName ) ) {
    goto done;
  }

  if ( !*fName ) {
    XtFree( fName );
    goto done;
  }

  cur = new activeWindowListType;

  cur->requestDelete = 0;
  cur->requestActivate = 0;
  cur->requestReactivate = 0;
  cur->requestOpen = 0;
  cur->requestPosition = 0;
  cur->requestCascade = 0;
  cur->requestImport = 0;
  cur->requestRefresh = 0;

  cur->node.create( apco, NULL, 0, 0, 0, 0, apco->numMacros, apco->macros,
   apco->expansions );
  cur->node.realize();
  cur->node.setGraphicEnvironment( &apco->ci, &apco->fi );

  cur->blink = apco->head->blink;
  apco->head->blink->flink = cur;
  apco->head->blink = cur;
  cur->flink = apco->head;

  cur->node.storeFileName( fName );

  XtFree( fName );

  cur->requestOpen = 1;
  cur->requestImport = 1;
  (apco->requestFlag)++;

done:

  XtUnmanageChild( w );

}

void app_importSelectCancel_cb (
  Widget w,
  XtPointer client,
  XtPointer call )
{

  XtUnmanageChild( w );

}

void new_cb (
  Widget w,
  XtPointer client,
  XtPointer call )
{

appContextClass *apco = (appContextClass *) client;
activeWindowListPtr cur, next;

  // traverse list and delete nodes so marked
  cur = apco->head->flink;
  while ( cur != apco->head ) {
    next = cur->flink;
    if ( cur->requestDelete ) {
      cur->blink->flink = cur->flink;
      cur->flink->blink = cur->blink;
      delete cur;
    }
    cur = next;
  }

  cur = new activeWindowListType;
  cur->requestDelete = 0;
  cur->requestActivate = 0;
  cur->requestReactivate = 0;
  cur->requestOpen = 0;
  cur->requestPosition = 0;
  cur->requestCascade = 0;
  cur->requestImport = 0;
  cur->requestRefresh = 0;
  cur->node.create( apco, NULL, 100, 100, 500, 600, apco->numMacros,
   apco->macros, apco->expansions );
  cur->node.realize();
  cur->node.setGraphicEnvironment( &apco->ci, &apco->fi );
  cur->node.setDisplayScheme( &apco->displayScheme );
  XtMapWidget( XtParent( cur->node.drawWidgetId() ) );

  cur->blink = apco->head->blink;
  apco->head->blink->flink = cur;
  apco->head->blink = cur;
  cur->flink = apco->head;

}

void refreshUserLib_cb (
  Widget w,
  XtPointer client,
  XtPointer call )
{

appContextClass *apco = (appContextClass *) client;

  apco->reopenUserLib();

}

void open_cb (
  Widget w,
  XtPointer client,
  XtPointer call )
{

appContextClass *apco = (appContextClass *) client;
int n;
Arg args[10];
XmString xmStr1, xmStr2;
char *envPtr, buf[127+1], prefix[127+1], *tk;

  envPtr = getenv( environment_str1 );
  if ( envPtr ) {
    strncpy( buf, envPtr, 127 );
    tk = strtok( buf, ":" );
    if ( tk ) {
      strncpy( prefix, tk, 127 );
      if ( prefix[strlen(prefix)-1] != '/' )
       strncat( prefix, "/", 127 );
    }
    else {
      strcpy( prefix, "./" );
    }
  }
  else {
    strcpy( prefix, "./" );
  }

  n = 0;

  xmStr1 = XmStringCreateLocalized( "*.edl" );
  XtSetArg( args[n], XmNpattern, xmStr1 ); n++;

  if ( strcmp( prefix, "" ) != 0 ) {
    xmStr2 = XmStringCreateLocalized( prefix );
    XtSetArg( args[n], XmNdirectory, xmStr2 ); n++;
  }

  XtSetValues( apco->fileSelectBox, args, n );

  XmStringFree( xmStr1 );
  if ( strcmp( prefix, "" ) != 0 ) XmStringFree( xmStr2 );

  XtManageChild( apco->fileSelectBox );

  XSetWindowColormap( apco->display, XtWindow(XtParent(apco->fileSelectBox)),
   apco->ci.getColorMap() );

}

void open_user_cb (
  Widget w,
  XtPointer client,
  XtPointer call )
{

appContextClass *apco = (appContextClass *) client;
int n;
Arg args[10];
XmString xmStr1, xmStr2;
char *envPtr, buf[127+1], prefix[127+1], *tk;

  envPtr = getenv( environment_str1 );
  if ( envPtr ) {
    strncpy( buf, envPtr, 127 );
    tk = strtok( buf, ":" );
    if ( tk ) {
      tk = strtok( NULL, ":" );
      if ( tk ) {
        strncpy( prefix, tk, 127 );
        if ( prefix[strlen(prefix)-1] != '/' )
         strncat( prefix, "/", 127 );
      }
      else {
        strcpy( prefix, "./" );
      }
    }
    else {
      strcpy( prefix, "./" );
    }
  }
  else {
    strcpy( prefix, "./" );
  }

  n = 0;

  xmStr1 = XmStringCreateLocalized( "*.edl" );
  XtSetArg( args[n], XmNpattern, xmStr1 ); n++;

  if ( strcmp( prefix, "" ) != 0 ) {
    xmStr2 = XmStringCreateLocalized( prefix );
    XtSetArg( args[n], XmNdirectory, xmStr2 ); n++;
  }

  XtSetValues( apco->fileSelectBox, args, n );

  XmStringFree( xmStr1 );
  if ( strcmp( prefix, "" ) != 0 ) XmStringFree( xmStr2 );

  XtManageChild( apco->fileSelectBox );

  XSetWindowColormap( apco->display, XtWindow(XtParent(apco->fileSelectBox)),
   apco->ci.getColorMap() );

}

void import_cb (
  Widget w,
  XtPointer client,
  XtPointer call )
{

appContextClass *apco = (appContextClass *) client;
int n;
Arg args[10];
XmString xmStr1, xmStr2;
char *envPtr, buf[127+1], prefix[127+1], *tk;

  envPtr = getenv( environment_str1 );
  if ( envPtr ) {
    strncpy( buf, envPtr, 127 );
    tk = strtok( buf, ":" );
    if ( tk ) {
      strncpy( prefix, tk, 127 );
      if ( prefix[strlen(prefix)-1] != '/' )
       strncat( prefix, "/", 127 );
    }
    else {
      strcpy( prefix, "./" );
    }
  }
  else {
    strcpy( prefix, "./" );
  }

  n = 0;

  xmStr1 = XmStringCreateLocalized( "*.xch" );
  XtSetArg( args[n], XmNpattern, xmStr1 ); n++;

  if ( strcmp( prefix, "" ) != 0 ) {
    xmStr2 = XmStringCreateLocalized( prefix );
    XtSetArg( args[n], XmNdirectory, xmStr2 ); n++;
  }

  XtSetValues( apco->importSelectBox, args, n );

  XmStringFree( xmStr1 );
  if ( strcmp( prefix, "" ) != 0 ) XmStringFree( xmStr2 );

  XtManageChild( apco->importSelectBox );

  XSetWindowColormap( apco->display, XtWindow(XtParent(apco->importSelectBox)),
   apco->ci.getColorMap() );

}

void continue_cb (
  Widget w,
  XtPointer client,
  XtPointer call )
{

appContextClass *apco = (appContextClass *) client;

  apco->confirm.popdown();

}

void abort_cb (
  Widget w,
  XtPointer client,
  XtPointer call )
{

appContextClass *apco = (appContextClass *) client;

  apco->confirm.popdown();
  apco->exitFlag = 1;

}

void exit_cb (
  Widget w,
  XtPointer client,
  XtPointer call )
{

appContextClass *apco = (appContextClass *) client;
activeWindowListPtr cur;
int changes = 0;

Window root, child;
int rootX, rootY, winX, winY;
unsigned int mask;

  cur = apco->head->flink;
  while ( cur != apco->head ) {
    if ( cur->node.changed() ) changes = 1;
    cur = cur->flink;
  }

  if ( changes ) {
    XQueryPointer( apco->display, XtWindow(apco->appTop), &root, &child,
     &rootX, &rootY, &winX, &winY, &mask );
    apco->confirm.create( apco->appTop, rootX, rootY, 2,
     appContextClass_str24, NULL, NULL );
    apco->confirm.addButton( appContextClass_str25, abort_cb, (void *) apco );
    apco->confirm.addButton( appContextClass_str26, continue_cb, (void *) apco );
    apco->confirm.finished();
    apco->confirm.popup();
    XSetWindowColormap( apco->display, XtWindow(apco->confirm.top()),
     apco->ci.getColorMap() );
    return;
  }

  apco->exitFlag = 1;

}

void view_pvList_cb (
  Widget w,
  XtPointer client,
  XtPointer call )
{

appContextClass *apco = (appContextClass *) client;

  apco->pvList.popup();

}

void view_screens_cb (
  Widget w,
  XtPointer client,
  XtPointer call )
{

appContextClass *apco = (appContextClass *) client;
char string[79+1];
int i, nodeCount = 0;
activeWindowListPtr cur;

  cur = apco->head->flink;
  while ( cur != apco->head ) {
    if ( blank( cur->node.name ) ) {
      apco->postMessage( appContextClass_str27 );
    }
    else {
      apco->postMessage( cur->node.name );
    }
    nodeCount++;
    cur = cur->flink;
  }

  if ( nodeCount ) {

    sprintf( string, appContextClass_str28, nodeCount );
    for( i=0; string[i]; i++ ) string[i] = '-';
    apco->postMessage( string );

    sprintf( string, appContextClass_str28, nodeCount );
    apco->postMessage( string );

    strcpy( string, " " );
    apco->postMessage( string );

  }
  else {

    strcpy( string, appContextClass_str29 );
    apco->postMessage( string );

    strcpy( string, " " );
    apco->postMessage( string );

  }

}

void view_msgBox_cb (
  Widget w,
  XtPointer client,
  XtPointer call )
{

appContextClass *apco = (appContextClass *) client;

  apco->msgBox.popup();

  XSetWindowColormap( apco->display, XtWindow(apco->msgBox.top()),
   apco->ci.getColorMap() );

}

appContextClass::appContextClass (
  void )
{

  executeCount = 0;
  isActive = 0;
  exitFlag = 0;
  executeOnOpen = 0;
  noEdit = 0;
  requestFlag = 0;
  iconified = 0;
  usingControlPV = 0;

  entryFormX = 0;
  entryFormY = 0;
  entryFormW = 0;
  entryFormH = 600;
  largestH = 600;

  head = new activeWindowListType;
  head->flink = head;
  head->blink = head;

  numMacros = 0;
  macroHead = new macroListType;
  macroHead->flink = macroHead;
  macroHead->blink = macroHead;

  numFiles = 0;
  fileHead = new fileListType;
  fileHead->flink = fileHead;
  fileHead->blink = fileHead;

  strcpy( ctlPV, "" );
  strcpy( userLib, "" );

  cutHead1 = new activeGraphicListType;
  cutHead1->flink = cutHead1;
  cutHead1->blink = cutHead1;

}

appContextClass::~appContextClass (
  void )
{

activeWindowListPtr cur, next;
activeGraphicListPtr curCut, nextCut;
macroListPtr curMacro, nextMacro;
fileListPtr curFile, nextFile;

  ci.closeColorWindow();

  // empty cut list
  curCut = cutHead1->flink;
  while ( curCut != cutHead1 ) {
    nextCut = curCut->flink;
    delete curCut->node;
    delete curCut;
    curCut = nextCut;
  }
  delete cutHead1;

  // walk macroList and delete
  curMacro = macroHead->flink;
  while ( curMacro != macroHead ) {
    nextMacro = curMacro->flink;
    if ( curMacro->macro ) delete curMacro->macro;
    if ( curMacro->expansion ) delete curMacro->expansion;
    delete curMacro;
    curMacro = nextMacro;
  }
  delete macroHead;

  // walk fileList and delete
  curFile = fileHead->flink;
  while ( curFile != fileHead ) {
    nextFile = curFile->flink;
    if ( curFile->file ) delete curFile->file;
    delete curFile;
    curFile = nextFile;
  }
  delete fileHead;

  // walk activeWindowList and delete
  cur = head->flink;
  while ( cur != head ) {
    next = cur->flink;
    if ( cur->node.mode == AWC_EXECUTE ) {
      cur->node.returnToEdit( 0 );
    }
    cur->blink->flink = cur->flink; // maintain list structure!
    cur->flink->blink = cur->blink;
    delete cur;
    cur = next;
  }
  processAllEventsWithSync( app, display );
  delete head;

  // delete widgets

  XtRemoveCallback( importSelectBox, XmNcancelCallback,
   app_importSelectCancel_cb, (void *) this );
  XtRemoveCallback( importSelectBox, XmNcancelCallback,
   app_importSelectCancel_cb, (void *) this );
  XtUnmanageChild( importSelectBox );
  XtDestroyWidget( importSelectBox );

  XtRemoveCallback( fileSelectBox, XmNcancelCallback,
   app_fileSelectCancel_cb, (void *) this );
  XtRemoveCallback( fileSelectBox, XmNcancelCallback,
   app_fileSelectCancel_cb, (void *) this );
  XtUnmanageChild( fileSelectBox );
  XtDestroyWidget( fileSelectBox );

  pvList.destroy();

  msgBox.destroy();

  XtUnmapWidget( appTop );

  XtDestroyWidget( mainWin );

  processAllEventsWithSync( app, display );

  XtDestroyWidget( appTop );

  processAllEventsWithSync( app, display );

}

int appContextClass::initDeferredExecutionQueue ( void )
{

int i, stat;

  stat = sys_iniq( &appDefExeFreeQueue );
  if ( !( stat & 1 ) ) {
    printf( appContextClass_str30 );
    return 2;
  }
  stat = sys_iniq( &appDefExeActiveQueue );
  if ( !( stat & 1 ) ) {
    printf( appContextClass_str30 );
    return 2;
  }

  appDefExeFreeQueue.flink = NULL;
  appDefExeFreeQueue.blink = NULL;
  appDefExeActiveQueue.flink = NULL;
  appDefExeActiveQueue.blink = NULL;

  for ( i=0; i<APPDEFEXE_QUEUE_SIZE; i++ ) {

    stat = INSQTI( (void *) &appDefExeNodes[i], (void *) &appDefExeFreeQueue,
     0 );
    if ( !( stat & 1 ) ) {
      printf( appContextClass_str30 );
      return 2;
    }

  }

  return 1;

}

void appContextClass::processDeferredExecutionQueue ( void )
{

int q_stat_r, q_stat_i;
APPDEFEXE_NODE_PTR node;

  do {

    q_stat_r = REMQHI( (void *) &appDefExeActiveQueue, (void **) &node, 0 );

    if ( q_stat_r & 1 ) {

      if ( node->obj ) node->obj->executeFromDeferredQueue();
      if ( node->awObj ) node->awObj->executeFromDeferredQueue();

      q_stat_i = INSQTI( (void *) node, (void *) &appDefExeFreeQueue, 0 );
      if ( !( q_stat_i & 1 ) ) {
        printf( appContextClass_str31 );
      }

    }
    else if ( q_stat_r != QUEWASEMP ) {
      printf( appContextClass_str32 );
    }

  } while ( q_stat_r & 1 );

}

void appContextClass::postDeferredExecutionQueue (
  class activeGraphicClass *ptr )
{

int q_stat_r, q_stat_i;
APPDEFEXE_NODE_PTR node;

    q_stat_r = REMQHI( (void *) &appDefExeFreeQueue, (void **) &node, 0 );

    if ( q_stat_r & 1 ) {

      node->awObj = NULL;
      node->obj = ptr;

      q_stat_i = INSQTI( (void *) node, (void *) &appDefExeActiveQueue, 0 );
      if ( !( q_stat_i & 1 ) ) {
        printf( appContextClass_str33 );
      }

    }
    else {
      printf( appContextClass_str34 );
    }

}

void appContextClass::postDeferredExecutionQueue (
  class activeWindowClass *ptr )
{

int q_stat_r, q_stat_i;
APPDEFEXE_NODE_PTR node;

    q_stat_r = REMQHI( (void *) &appDefExeFreeQueue, (void **) &node, 0 );

    if ( q_stat_r & 1 ) {

      node->awObj = ptr;
      node->obj = NULL;

      q_stat_i = INSQTI( (void *) node, (void *) &appDefExeActiveQueue, 0 );
      if ( !( q_stat_i & 1 ) ) {
        printf( appContextClass_str33 );
      }

    }
    else {
      printf( appContextClass_str34 );
    }

}

void appContextClass::createMainWindow ( void )
{

XmString menuStr, str;

  mainWin = XtVaCreateManagedWidget( "", xmMainWindowWidgetClass,
   appTop,
   XmNscrollBarDisplayPolicy, XmAS_NEEDED,
   XmNscrollingPolicy, XmAUTOMATIC,
   NULL );

  // create menubar
  menuBar = XmCreateMenuBar( mainWin, "", NULL, 0 );

  filePullDown = XmCreatePulldownMenu( menuBar, "", NULL, 0 );

  menuStr = XmStringCreateLocalized( appContextClass_str35 );
  fileCascade = XtVaCreateManagedWidget( "", xmCascadeButtonWidgetClass,
   menuBar,
   XmNlabelString, menuStr,
   XmNmnemonic, 'f',
   XmNsubMenuId, filePullDown,
   NULL );
  XmStringFree( menuStr );

  if ( !noEdit ) {

    str = XmStringCreateLocalized( appContextClass_str36 );
    newB = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
     filePullDown,
     XmNlabelString, str,
     NULL );
    XmStringFree( str );
    XtAddCallback( newB, XmNactivateCallback, new_cb,
     (XtPointer) this );

  }

  str = XmStringCreateLocalized( appContextClass_str37 );
  newB = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
   filePullDown,
   XmNlabelString, str,
   NULL );
  XmStringFree( str );
  XtAddCallback( newB, XmNactivateCallback, open_cb,
   (XtPointer) this );

  str = XmStringCreateLocalized( appContextClass_str38 );
  newB = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
   filePullDown,
   XmNlabelString, str,
   NULL );
  XmStringFree( str );
  XtAddCallback( newB, XmNactivateCallback, open_user_cb,
   (XtPointer) this );

  str = XmStringCreateLocalized( appContextClass_str39 );
  newB = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
   filePullDown,
   XmNlabelString, str,
   NULL );
  XmStringFree( str );
  XtAddCallback( newB, XmNactivateCallback, import_cb,
   (XtPointer) this );

  str = XmStringCreateLocalized( appContextClass_str40 );
  newB = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
   filePullDown,
   XmNlabelString, str,
   NULL );
  XmStringFree( str );
  XtAddCallback( newB, XmNactivateCallback, refreshUserLib_cb,
   (XtPointer) this );

  str = XmStringCreateLocalized( appContextClass_str41 );
  newB = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
   filePullDown,
   XmNlabelString, str,
   NULL );
  XmStringFree( str );
  XtAddCallback( newB, XmNactivateCallback, exit_cb,
   (XtPointer) this );


  viewPullDown = XmCreatePulldownMenu( menuBar, "", NULL, 0 );

  menuStr = XmStringCreateLocalized( appContextClass_str42 );
  viewCascade = XtVaCreateManagedWidget( "", xmCascadeButtonWidgetClass,
   menuBar,
   XmNlabelString, menuStr,
   XmNmnemonic, 'v',
   XmNsubMenuId, viewPullDown,
   NULL );
  XmStringFree( menuStr );

  str = XmStringCreateLocalized( appContextClass_str43 );
  msgB = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
   viewPullDown,
   XmNlabelString, str,
   NULL );
  XmStringFree( str );
  XtAddCallback( msgB, XmNactivateCallback, view_msgBox_cb,
   (XtPointer) this );

  str = XmStringCreateLocalized( appContextClass_str44 );
  pvB = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
   viewPullDown,
   XmNlabelString, str,
   NULL );
  XmStringFree( str );
  XtAddCallback( pvB, XmNactivateCallback, view_pvList_cb,
   (XtPointer) this );

  str = XmStringCreateLocalized( appContextClass_str45 );
  pvB = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
   viewPullDown,
   XmNlabelString, str,
   NULL );
  XmStringFree( str );
  XtAddCallback( pvB, XmNactivateCallback, view_screens_cb,
   (XtPointer) this );

  XtManageChild( menuBar );

}

void appContextClass::addActiveWindow (
  activeWindowListPtr node )
{

  // link in

  node->requestDelete = 0;
  node->requestActivate = 0;
  node->requestReactivate = 0;
  node->requestOpen = 0;
  node->requestPosition = 0;
  node->requestCascade = 0;
  node->requestImport = 0;
  node->requestRefresh = 0;

  node->blink = head->blink;
  head->blink->flink = node;
  head->blink = node;
  node->flink = head;

}

int appContextClass::refreshActiveWindow (
  activeWindowClass *activeWindowNode )
{

  // simply mark for refresh and increment request flag

activeWindowListPtr cur;

  cur = head->flink;
  while ( cur != head ) {
    if ( &(cur->node) == activeWindowNode ) {
      if ( !cur->requestRefresh ) {
        cur->requestRefresh = 1;
        requestFlag++;
      }
    }
    cur = cur->flink;
  }

  return 1;

}

int appContextClass::removeActiveWindow (
  activeWindowClass *activeWindowNode )
{

  // simply mark for delete and increment request flag

activeWindowListPtr cur;

  cur = head->flink;
  while ( cur != head ) {
    if ( &(cur->node) == activeWindowNode ) {
      cur->requestDelete = 1;
      requestFlag++;
    }
    cur = cur->flink;
  }

  return 1;

}

int appContextClass::openEditActiveWindow (
  activeWindowClass *activeWindowNode )
{

  // simply mark for open and increment request flag

activeWindowListPtr cur;

  cur = head->flink;
  while ( cur != head ) {
    if ( &(cur->node) == activeWindowNode ) {
      cur->requestOpen = 1;
      cur->requestPosition = 0;
      cur->requestCascade = 0;
      cur->requestImport = 0;
      requestFlag++;
    }
    cur = cur->flink;
  }

  return 1;

}

int appContextClass::openActivateActiveWindow (
  activeWindowClass *activeWindowNode )
{

  // simply mark for open and activation and increment request flag

activeWindowListPtr cur;

  cur = head->flink;
  while ( cur != head ) {
    if ( &(cur->node) == activeWindowNode ) {
      cur->requestOpen = 1;
      cur->requestPosition = 0;
      cur->requestCascade = 0;
      cur->requestImport = 0;
      requestFlag++;
      cur->requestActivate = 1;
      requestFlag++;
    }
    cur = cur->flink;
  }

  return 1;

}

int appContextClass::openActivateActiveWindow (
  activeWindowClass *activeWindowNode,
  int x,
  int y )
{

  // simply mark for open and activation and increment request flag

activeWindowListPtr cur;

  cur = head->flink;
  while ( cur != head ) {
    if ( &(cur->node) == activeWindowNode ) {
      cur->requestOpen = 1;
      cur->requestPosition = 1;
      cur->requestCascade = 0;
      cur->requestImport = 0;
      cur->x = x;
      cur->y = y;
      requestFlag++;
      cur->requestActivate = 1;
      requestFlag++;
    }
    cur = cur->flink;
  }

  return 1;

}

int appContextClass::openActivateCascadeActiveWindow (
  activeWindowClass *activeWindowNode )
{

  // simply mark for open and activation and increment request flag

activeWindowListPtr cur;

  cur = head->flink;
  while ( cur != head ) {
    if ( &(cur->node) == activeWindowNode ) {
      cur->requestOpen = 1;
      cur->requestPosition = 0;
      cur->requestCascade = 1;
      cur->requestImport = 0;
      requestFlag++;
      cur->requestActivate = 1;
      requestFlag++;
    }
    cur = cur->flink;
  }

  return 1;

}

int appContextClass::openActivateCascadeActiveWindow (
  activeWindowClass *activeWindowNode,
  int x,
  int y )
{

  // simply mark for open and activation and increment request flag

activeWindowListPtr cur;

  cur = head->flink;
  while ( cur != head ) {
    if ( &(cur->node) == activeWindowNode ) {
      cur->requestOpen = 1;
      cur->requestPosition = 1;
      cur->requestCascade = 1;
      cur->requestImport = 0;
      requestFlag++;
      cur->requestActivate = 1;
      requestFlag++;
    }
    cur = cur->flink;
  }

  return 1;

}

int appContextClass::activateActiveWindow (
  activeWindowClass *activeWindowNode )
{

  // simply mark for activation and increment request flag

activeWindowListPtr cur;

  cur = head->flink;
  while ( cur != head ) {
    if ( &(cur->node) == activeWindowNode ) {
      cur->requestActivate = 1;
      requestFlag++;
    }
    cur = cur->flink;
  }

  return 1;

}

int appContextClass::reactivateActiveWindow (
  activeWindowClass *activeWindowNode )
{

  // simply mark for reactivation and increment request flag

activeWindowListPtr cur;

  cur = head->flink;
  while ( cur != head ) {
    if ( &(cur->node) == activeWindowNode ) {
      cur->requestReactivate = 1;
      requestFlag++;
    }
    cur = cur->flink;
  }

  return 1;

}

static void displayVersion ( void ) {

  printf( "  edm version %s Copyright (C) 1999 John W. Sinclair\n", VERSION );
  printf( "\n" );

  printf( "  This program is free software; you can redistribute it and/or modify\n" );
  printf( "  it under the terms of the GNU General Public License as published by\n" );
  printf( "  the Free Software Foundation; either version 2 of the License, or\n" );
  printf( "  (at your option) any later version.\n" );
  printf( "\n" );

printf( "  This program is distributed in the hope that it will be useful,\n" );
  printf( "  but WITHOUT ANY WARRANTY; without even the implied warranty of\n" );
  printf( "  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n" );
  printf( "  GNU General Public License for more details.\n" );
  printf( "\n" );
 
printf( "  You should have received a copy of the GNU General Public License\n" );
  printf( "  along with this program; if not, write to the Free Software\n" );
  printf( "  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.\n" );

}

static void displayParamInfo ( void ) {

  printf( "\n" );
  displayVersion();

  printf( "\n" );
   printf( global_str24 );
  printf( "\n" );

  printf( global_str25 );

#ifdef GENERIC_PV

  printf( global_str26 );

#else

  printf( global_str27 );

#endif

  printf( global_str28 );

  printf( global_str29 );

  printf( global_str30 );
  printf( global_str31 );

  printf( global_str32 );
  printf( global_str33 );

#ifdef GENERIC_PV
  printf( global_str34 );
#endif

  printf( global_str35 );
  printf( global_str36 );
  printf( global_str37 );

  printf( global_str57 );
  printf( global_str58 );
  printf( global_str59 );

  printf( global_str60 );
  printf( global_str61 );

  printf( global_str38 );

  printf( global_str39 );

  printf( global_str40 );

  printf( global_str41 );
  printf( global_str42 );
  printf( global_str43 );
  printf( global_str44 );
  printf( global_str45 );

  printf( global_str46 );
  printf( global_str47 );
  printf( "\n" );
  printf( global_str48 );
  printf( "\n" );
  printf( global_str49 );
  printf( "\n" );
  printf( global_str50 );
  printf( "\n" );
  printf( global_str51 );
  printf( global_str52 );
  printf( "\n" );
  printf( global_str53 );
  printf( "\n" );
  printf( global_str54 );
  printf( "\n" );

}

#define DONE -1
#define SWITCHES 1
#define FILES 2

int appContextClass::getParams(
  int argc,
  char **argv )
{

char buf[1023+1], mac[1023+1], exp[1023+1];
int state = SWITCHES;
int i, n = 1;
char *tk;
macroListPtr curMacro;
fileListPtr curFile;

#ifdef GENERIC_PV
  strcpy( pvClassName, "" );
#endif

  strcpy( displayName, "" );
  local = 0;

  // check first for component management commands
  if ( argc > 1 ) {

    if ( ( strcmp( argv[1], global_str6 ) == 0 ) ||
         ( strcmp( argv[1], global_str7 ) == 0 ) ||
         ( strcmp( argv[1], global_str8 ) == 0 ) ) {
      if ( argc < 3 ) {
        printf( appContextClass_str77 );
        displayParamInfo();
        exit(0);
      }
      manageComponents( argv[1], argv[2] );
      printf( "\n\n" );
      exit(0);
    }

  }

  if ( argc > 1 ) {

    if ( ( strcmp( argv[1], global_str63 ) == 0 ) ||
         ( strcmp( argv[1], global_str64 ) == 0 ) ||
         ( strcmp( argv[1], global_str65 ) == 0 ) ) {
      if ( argc < 3 ) {
        printf( appContextClass_str77 );
        displayParamInfo();
        exit(0);
      }
      managePvComponents( argv[1], argv[2] );
      printf( "\n\n" );
      exit(0);
    }

  }

  while ( n < argc ) {

    switch ( state ) {

    case SWITCHES:

      if ( argv[n][0] == '-' ) {

        if ( strcmp( argv[n], global_str11 ) == 0 ) {
          displayParamInfo();
          exit(0);
        }
        else if ( strcmp( argv[n], global_str12 ) == 0 ) {
          displayParamInfo();
          exit(0);
        }
        else if ( strcmp( argv[n], global_str13 ) == 0 ) {
          displayParamInfo();
          exit(0);
        }
        else if ( strcmp( argv[n], global_str14 ) == 0 ) {
          printf( "\n" );
          displayVersion();
          printf( "\n" );
          exit(0);
        }
        else if ( strcmp( argv[n], global_str15 ) == 0 ) {
          printf( "\n" );
          displayVersion();
          printf( "\n" );
          exit(0);
        }
        else if ( strcmp( argv[n], global_str16 ) == 0 ) {
          printf( "\n" );
          displayVersion();
          printf( "\n" );
          exit(0);
        }
        else if ( strcmp( argv[n], global_str17 ) == 0 ) {
          executeOnOpen = 1;
        }
        else if ( strcmp( argv[n], global_str9 ) == 0 ) {
          local = 1;
        }
        else if ( strcmp( argv[n], global_str10 ) == 0 ) {
        }
        else if ( strcmp( argv[n], global_str18 ) == 0 ) {
          noEdit = 1;
        }
        else if ( strcmp( argv[n], global_str19 ) == 0 ) {
          n++;
          if ( n >= argc ) return 2; // missing macro arg
          strncpy( buf, argv[n], 1023 );
          tk = strtok( buf, "=," );
          while ( tk ) {
            strncpy( mac, tk, 1023 );
            tk = strtok( NULL, "=," );
            if ( tk ) {
              strncpy( exp, tk, 1023 );
            }
            else {
              return 4; // macro, but no value
            }
            numMacros++;
            curMacro = new macroListType;
            if ( !curMacro ) return 6;
            curMacro->macro = new char[strlen(mac)+1];
            if ( !curMacro->macro ) return 6;
            strcpy( curMacro->macro, mac );
            curMacro->expansion = new char[strlen(exp)+1];
            if ( !curMacro->expansion ) return 6;
            strcpy( curMacro->expansion, exp );
            macroHead->blink->flink = curMacro;
            curMacro->flink = macroHead;
            curMacro->blink = macroHead->blink;
            macroHead->blink = curMacro;
            tk = strtok( NULL, "=," );
          }

          if ( numMacros == 0 ) return 4;

        }
        else if ( strcmp( argv[n], global_str20 ) == 0 ) {
          n++;
          if ( n >= argc ) return 2;
          strncpy( ctlPV, argv[n], 127 );
        }
        else if ( strcmp( argv[n], global_str22 ) == 0 ) {
          n++;
          if ( n >= argc ) return 2;
          strncpy( userLib, argv[n], 127 );
          userLibObject.openUserLibrary( userLib );
        }
        else if ( strcmp( argv[n], global_str21 ) == 0 ) {
          n++;
          if ( n >= argc ) return 2;
          strncpy( displayName, argv[n], 127 );
        }

#ifdef GENERIC_PV
        else if ( strcmp( argv[n], global_str23 ) == 0 ) {
          n++;
          if ( n >= argc ) return 2;
          strncpy( pvClassName, argv[n], 15 );
          pvClassName[15] = 0;
        }
#endif

        else {
          return -2;
        }

        n++;

      }
      else {

        state = FILES;

      }

      break;

    case FILES:

      if ( argv[n][0] == '-' ) {

        return 8;

      }
      else {

        numFiles++;
        curFile = new fileListType;
        if ( !curFile ) return 6;
        curFile->file = new char[strlen(argv[n])+1];
        if ( !curFile->file ) return 6;
        strcpy( curFile->file, argv[n] );
        fileHead->blink->flink = curFile;
        curFile->flink = fileHead;
        curFile->blink = fileHead->blink;
        fileHead->blink = curFile;

        n++;

      }

      break;

    }

  }

  if ( numMacros > 0 ) {
    macros = new (char *)[numMacros];
    expansions = new (char *)[numMacros];
    i = 0;
    curMacro = macroHead->flink;
    while ( curMacro != macroHead ) {
      macros[i] = curMacro->macro;
      expansions[i] = curMacro->expansion;
      i++;
      curMacro = curMacro->flink;
    }
  }

  return 1;

}

int appContextClass::startApplication (
  int argc,
  char **argv )
{

int stat, opStat;
activeWindowListPtr cur;
char *name, *envPtr;
char prefix[127+1], fname[127+1], msg[127+1];
fileListPtr curFile;
expStringClass expStr;
Atom wm_delete_window;

  name = argv[0];

  stat = getParams( argc, argv );
  if ( !( stat & 1 ) ) {
    switch ( stat ) {
    case 2:
      printf( appContextClass_str93 );
      goto err_return;
    case 4:
      printf( appContextClass_str94 );
      goto err_return;
    case 6:
      printf( appContextClass_str95 );
      goto err_return;
    case 8:
      printf( appContextClass_str96 );
      goto err_return;
    case -2:
      printf( appContextClass_str97 );
      goto err_return;
    }
err_return:
    printf( global_str55 );
    exitFlag = 1;
    return 0; // error
  }

  stat = initDeferredExecutionQueue();
  if ( !( stat & 1 ) ) {
    exitFlag = 1;
    return 0; // error
  }

#ifdef __epics__

  if ( strcmp( ctlPV, "" ) != 0 ) {

    stat = ca_search( ctlPV, &ctlPvId );
    if ( stat != ECA_NORMAL ) {

      sprintf( msg, appContextClass_str99 );
      postMessage( msg );

    }
    else {

      stat = ca_pend_io( 10.0 );
      if ( stat != ECA_NORMAL ) {

        sprintf( msg, appContextClass_str100 );
        postMessage( msg );

      }
      else {

        stat = ca_put( DBR_STRING, ctlPvId, "" );
        stat = ca_pend_io( 10.0 );

        stat = ca_add_masked_array_event( DBR_STRING, 1, ctlPvId,
         ctlPvUpdate, (void *) this, (float) 0.0, (float) 0.0, (float) 0.0,
         &ctlPvEventId, DBE_VALUE );
        if ( stat != ECA_NORMAL ) {
          sprintf( msg,
           appContextClass_str101 );
          postMessage( msg );
        }

        usingControlPV = 1;

      }

    }

  }

#else

#ifdef GENERIC_PV

  if ( strcmp( ctlPV, "" ) != 0 ) {

    ctlPvId = pvObj.createNew( pvClassName );
    if ( ctlPvId ) {

      ctlPvId->createEventId( &ctlPvEventId );

      expStr.setRaw( ctlPV );
      stat = ctlPvId->search( &expStr );
      if ( stat != PV_E_SUCCESS ) {

        sprintf( msg, appContextClass_str102 );
        postMessage( msg );

      }
      else {

        stat = ctlPvId->pendIo( 10.0 );
        if ( stat != PV_E_SUCCESS ) {

          sprintf( msg, appContextClass_str103 );
          postMessage( msg );

        }
        else {

          stat = ctlPvId->put( ctlPvId->pvrString(), (void *) "" );
          stat = ctlPvId->pendIo( 10.0 );

          stat = ctlPvId->addEvent( ctlPvId->pvrString(), 1,
	   ctlPvUpdate, (void *) this, ctlPvEventId,
	   ctlPvId->pveValue() );
          if ( stat != PV_E_SUCCESS ) {
            sprintf( msg,
             appContextClass_str104 );
            postMessage( msg );
          }

          usingControlPV = 1;

        }

      }

    }

  }

#endif

#endif

  {

  int argCount = 3;
  char *args[3] = { "edm", global_str21, displayName };

    if ( strcmp( displayName, "" ) == 0 ) argCount = 1;

    if ( executeOnOpen ) {
      appTop = XtVaAppInitialize( &app, "edm", NULL, 0, &argCount,
       args, NULL, XmNiconic, True, NULL );
      iconified = 1;
    }
    else {
      appTop = XtVaAppInitialize( &app, "edm", NULL, 0, &argCount,
       args, NULL, XmNiconic, False, NULL );
      iconified = 0;
    }

  }
 
  this->createMainWindow();

  XtRealizeWidget( appTop );

  display = XtDisplay( appTop );

  wm_delete_window = XmInternAtom( display, "WM_DELETE_WINDOW", False );
  XmAddWMProtocolCallback( appTop, wm_delete_window,
   exit_cb, (XtPointer) this );
  XtVaSetValues( appTop, XmNdeleteResponse, XmDO_NOTHING, NULL );

  largestH = XDisplayHeight( display, DefaultScreen(display) );

  msgBox.create( appTop, 0, 0, 5000, NULL, NULL );

  pvList.create( appTop, 20 );

  fileSelectBox = XmCreateFileSelectionDialog( appTop, "", NULL, 0 );

  XtAddCallback( fileSelectBox, XmNcancelCallback,
   app_fileSelectCancel_cb, (void *) this );
  XtAddCallback( fileSelectBox, XmNokCallback,
   app_fileSelectOk_cb, (void *) this );

  importSelectBox = XmCreateFileSelectionDialog( appTop, "", NULL, 0 );

  XtAddCallback( importSelectBox, XmNcancelCallback,
   app_importSelectCancel_cb, (void *) this );
  XtAddCallback( importSelectBox, XmNokCallback,
   app_importSelectOk_cb, (void *) this );

  envPtr = getenv(environment_str2);
  if ( envPtr ) {
    strncpy( prefix, envPtr, 127 );
    if ( prefix[strlen(prefix)-1] != '/' ) strncat( prefix, "/", 127 );
  }
  else {
    strcpy( prefix, "/etc/" );
  }

  strncpy( fname, prefix, 127 );
  strncat( fname, "colors.list", 127 );
  opStat = ci.initFromFile( app, display, appTop, fname );

  if ( !( opStat & 1 ) ) {
    printf( appContextClass_str106 );
    exitFlag = 1;
    return 0; // error
  }

  XSetWindowColormap( display, XtWindow(pvList.top()), ci.getColorMap() );

  strncpy( fname, prefix, 127 );
  strncat( fname, "fonts.list", 127 );
  opStat = fi.initFromFile( display, fname );

  if ( !( opStat & 1 ) ) {
    printf( appContextClass_str107 );
    exitFlag = 1;
    return 0; // error
  }

  displayScheme.loadDefault( &ci );

  curFile = fileHead->flink;
  while ( curFile != fileHead ) {

    cur = new activeWindowListType;
    cur->requestDelete = 0;
    cur->requestActivate = 0;
    cur->requestReactivate = 0;
    cur->requestOpen = 0;
    cur->requestPosition = 0;
    cur->requestCascade = 0;
    cur->requestImport = 0;
    cur->requestRefresh = 0;

    cur->blink = head->blink;
    head->blink->flink = cur;
    head->blink = cur;
    cur->flink = head;

    cur->node.create( this, NULL, 0, 0, 0, 0, numMacros, macros, expansions );
    cur->node.realize();
    cur->node.setGraphicEnvironment( &ci, &fi );

    cur->node.storeFileName( curFile->file );

    cur->requestOpen = 1;
    requestFlag++;

    if ( executeOnOpen ) {
      cur->requestActivate = 1;
      requestFlag++;
    }

    curFile = curFile->flink;

  }

  iconTestCount = 0;

  return 1;

}

void appContextClass::applicationLoop ( void ) {

int stat, nodeCount, actionCount, iconNodeCount,
 iconActionCount, n;
activeWindowListPtr cur, next;
char msg[127+1];

  if ( requestFlag ) {

    cur = head->flink;
    while ( cur != head ) {
      if ( !cur->requestDelete ) {
        if ( cur->requestRefresh ) {
          cur->requestRefresh = 0;
          if ( requestFlag > 0 ) requestFlag--;
          cur->node.refreshActive();
        }
      }
      cur = cur->flink;
    }

    nodeCount = iconNodeCount = actionCount = iconActionCount = 0;
    // traverse list and delete nodes so marked
    cur = head->flink;
    while ( cur != head ) {
      next = cur->flink;
      nodeCount++;
      if ( cur->requestDelete ) {
        actionCount++;
        iconActionCount++;
        cur->blink->flink = cur->flink;
        cur->flink->blink = cur->blink;
        delete cur;
        if ( requestFlag > 0 ) requestFlag--;
      }
      cur = next;
    }

    /* if all windows have been removed then deiconify main window */
    if ( iconNodeCount == iconActionCount ) {
      if ( !usingControlPV ) deiconifyMainWindow();
    }

    cur = head->flink;
    while ( cur != head ) {
      if ( cur->requestOpen ) {
        cur->requestOpen = 0;
        if ( cur->requestImport ) {
          cur->requestImport = 0;
          stat = cur->node.import();
        }
        else {
          if ( cur->requestPosition ) {
            cur->requestPosition = 0;
            if ( cur->requestCascade ) {
              cur->requestCascade = 0;
              stat = cur->node.loadCascade( cur->x, cur->y );
            }
            else {
              stat = cur->node.load( cur->x, cur->y );
            }
          }
          else if ( cur->requestCascade ) {
            cur->requestPosition = 0;
            cur->requestCascade = 0;
            stat = cur->node.loadCascade();
          }
          else {
            stat = cur->node.load();
          }
        }
        XtMapWidget( XtParent( cur->node.drawWidgetId() ) );
        if ( !( stat & 1 ) ) {
          sprintf( msg, appContextClass_str108, cur->node.fileName );
          postMessage( msg );
          cur->requestDelete = 1;
          cur->requestActivate = 0;
          cur->requestReactivate = 0;
          XtUnmanageChild( cur->node.drawWidgetId() );
          XtUnmapWidget( XtParent( cur->node.drawWidgetId() ) );
          if ( requestFlag > 0 ) requestFlag--;
          if ( cur->requestActivate == 1 ) {
            cur->requestActivate = 0;
            if ( requestFlag > 0 ) requestFlag--;
          }
        }
        else {
          if ( requestFlag > 0 ) requestFlag--;
          if ( cur->requestActivate == 1 ) cur->node.setNoRefresh();
          if ( cur->requestReactivate == 1 ) cur->node.setNoRefresh();
        }
      }
      cur = cur->flink;
    }

    cur = head->flink;
    while ( cur != head ) {
      if ( cur->requestActivate == 1 ) cur->requestActivate = 2;
      if ( cur->requestReactivate == 1 ) cur->requestReactivate = 2;
      cur = cur->flink;
    }

    cur = head->flink;
    while ( cur != head ) {
      if ( ( cur->requestActivate == 2 ) && !cur->requestDelete ) {
        cur->node.createWidgets();
        cur->requestActivate = 3;
      }
      cur = cur->flink;
    }

    processAllEventsWithSync( app, display );

    nodeCount = iconNodeCount = actionCount = iconActionCount = 0;
    cur = head->flink;
    while ( cur != head ) {
      nodeCount++;
      iconNodeCount++;
      if ( cur->node.isActive() ) {
        actionCount++;
        iconActionCount++;
      }
      if ( !cur->requestDelete ) {
        if ( cur->requestActivate == 3 ) {
          cur->requestActivate = 0;
          cur->node.execute();
          actionCount++;
          iconActionCount++;
          if ( requestFlag > 0 ) requestFlag--;
          cur->node.setRefresh();
          cur->node.refreshActive();
        }
        if ( cur->requestReactivate == 2 ) {
          cur->requestReactivate = 0;
          cur->node.reexecute();
          actionCount++;
          if ( requestFlag > 0 ) requestFlag--;
          cur->node.setRefresh();
          cur->node.refreshActive();
        }
      }
      cur = cur->flink;
    }

    processAllEventsWithSync( app, display );

    /* if all windows have been activated then iconify main window */
    if ( ( iconNodeCount == iconActionCount ) && ( iconNodeCount != 0 ) ) {
      if ( !usingControlPV ) iconifyMainWindow();
    }
    else {
      if ( !usingControlPV ) deiconifyMainWindow();
    }

  }

  processDeferredExecutionQueue();

  n = 0;
  cur = head->flink;
  while ( cur != head ) {

    n++;

    // invoke the executeDeferred function for all activeGraphicClass
    // objects (display elements) for a given activeWindowClass object
    // (display screen)
    cur->node.processObjects();

    iconTestCount++;
    if ( iconTestCount > 10 ) { // periodically, check if iconified
      iconTestCount = 0;
      XtVaGetValues( cur->node.topWidgetId(),
       XmNiconic, &cur->node.isIconified,
       NULL );
    }

#ifdef __epics__
    stat = ca_pend_io( 3.0 );
#endif
    processAllEventsWithSync( app, display );

    cur = cur->flink;

  }

  processAllEventsWithSync( app, display );

}

XtAppContext appContextClass::appContext ( void )
{

  return app;

}

Widget appContextClass::fileSelectBoxWidgetId ( void )
{

  return fileSelectBox;

}

Widget appContextClass::importSelectBoxWidgetId ( void )
{

  return importSelectBox;

}

void appContextClass::postMessage (
  char *msg )
{

  msgBox.addText( msg );

}

void appContextClass::iconifyMainWindow ( void ) {

  return;

  if ( !iconified ) {
    XIconifyWindow( display, XtWindow(appTop), DefaultScreen(display) );
    iconified = 1;
  }

}

void appContextClass::deiconifyMainWindow ( void ) {

  return;

  if ( iconified ) {
    XUnmapWindow( display, XtWindow(appTop) );
    XMapWindow( display, XtWindow(appTop) );
    iconified = 0;
  }

}

int appContextClass::setProperty (
  char *winId,
  char *id,
  char *property,
  char *value )
{

int stat;
activeWindowListPtr cur;

  cur = head->flink;
  while ( cur != head ) {
    if ( strcmp( winId, cur->node.id ) == 0 ) {
      stat = cur->node.setProperty( id, property, value );
      return stat;
    }
    cur = cur->flink;
  }

  return 0;

}

int appContextClass::setProperty (
  char *winId,
  char *id,
  char *property,
  double *value )
{

int stat;
activeWindowListPtr cur;

  cur = head->flink;
  while ( cur != head ) {
    if ( strcmp( winId, cur->node.id ) == 0 ) {
      stat = cur->node.setProperty( id, property, value );
      return stat;
    }
    cur = cur->flink;
  }

  return 0;

}

int appContextClass::setProperty (
  char *winId,
  char *id,
  char *property,
  int *value )
{

int stat;
activeWindowListPtr cur;

  cur = head->flink;
  while ( cur != head ) {
    if ( strcmp( winId, cur->node.id ) == 0 ) {
      stat = cur->node.setProperty( id, property, value );
      return stat;
    }
    cur = cur->flink;
  }

  return 0;

}

int appContextClass::getProperty (
  char *winId,
  char *id,
  char *property,
  int bufSize,
  char *value )
{

int stat;
activeWindowListPtr cur;

  cur = head->flink;
  while ( cur != head ) {
    if ( strcmp( winId, cur->node.id ) == 0 ) {
      stat = cur->node.getProperty( id, property, bufSize, value );
      return stat;
    }
    cur = cur->flink;
  }

  return 0;

}

int appContextClass::getProperty (
  char *winId,
  char *id,
  char *property,
  double *value )
{

int stat;
activeWindowListPtr cur;

  cur = head->flink;
  while ( cur != head ) {
    if ( strcmp( winId, cur->node.id ) == 0 ) {
      stat = cur->node.getProperty( id, property, value );
      return stat;
    }
    cur = cur->flink;
  }

  return 0;

}

int appContextClass::getProperty (
  char *winId,
  char *id,
  char *property,
  int *value )
{

int stat;
activeWindowListPtr cur;

  cur = head->flink;
  while ( cur != head ) {
    if ( strcmp( winId, cur->node.id ) == 0 ) {
      stat = cur->node.getProperty( id, property, value );
      return stat;
    }
    cur = cur->flink;
  }

  return 0;

}

void appContextClass::reopenUserLib ( void ) {

  if ( strcmp( userLib, "" ) != 0 ) {
    userLibObject.openUserLibrary( userLib );
  }

}

void appContextClass::xSynchronize (
  int onoff
) {

  XSynchronize( display, (Bool) onoff );

}

void appContextClass::exitProgram ( void ) {

  exit_cb ( NULL, this, NULL );

}

void appContextClass::findTop ( void ) {

  XUnmapWindow( display, XtWindow(appTop) );
  XMapWindow( display, XtWindow(appTop) );
  XRaiseWindow( display, XtWindow(appTop) );

}

