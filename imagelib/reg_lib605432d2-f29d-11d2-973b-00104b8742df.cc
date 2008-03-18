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

#include "reg_lib605432d2-f29d-11d2-973b-00104b8742df.str"
#include "environment.str"
#include "edm.version"

typedef struct libRecTag {
  char *className;
  char *typeName;
  char *text;
} libRecType, *libRecPtr;

static int libRecIndex = 0;

static libRecType libRec[] = {
  { "cfcf6c8a_dbeb_11d2_8a97_00104b8742df", global_str3, reg_str1 },
  { "activePngClass", global_str3, reg_str2 }
};

#ifdef __cplusplus
extern "C" {
#endif

char *version ( void ) {

static char *v = VERSION;

  return v;

}

char *author ( void ) {

static char *a = "John Sinclair (sinclairjw@ornl.gov)";

  return a;

}

int firstRegRecord (
  char **className,
  char **typeName,
  char **text )
{

  libRecIndex = 0;

  *className = libRec[libRecIndex].className;
  *typeName = libRec[libRecIndex].typeName;
  *text = libRec[libRecIndex].text;

  return 0;

}

int nextRegRecord (
  char **className,
  char **typeName,
  char **text )
{

int max = sizeof(libRec) / sizeof(libRecType) - 1;

  if ( libRecIndex >= max ) return -1; //no more 
  libRecIndex++;

  *className = libRec[libRecIndex].className;
  *typeName = libRec[libRecIndex].typeName;
  *text = libRec[libRecIndex].text;

  return 0;

}

#ifdef __cplusplus
}
#endif
