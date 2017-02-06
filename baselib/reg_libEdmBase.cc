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

#include "reg_libEdmBase.str"
#include "environment.str"
#include "edm.version"

typedef struct libRecTag {
  char *className;
  char *typeName;
  char *text;
} libRecType, *libRecPtr;

static int libRecIndex = 0;

static libRecType libRec[] = {
  { "activeLineClass", global_str3, reg_str2 },
  { "activeRectangleClass", global_str3, reg_str3 },
  { "activeCircleClass", global_str3, reg_str4 },
  { "activeArcClass", global_str3, reg_str5 },
  { "activeXTextClass", global_str3, reg_str6 },
  { "activeXRegTextClass", global_str3, reg_str98 },
  { "activePipClass", global_str3, reg_str25 },
  { "includeWidgetClass", global_str3, reg_str33 },
  { "activeMeterClass", global_str2, reg_str8 },
  { "activeBarClass", global_str2, reg_str9 },
  { "activeMessageBoxClass", global_str2, reg_str10 },
  { "xyGraphClass", global_str2, reg_str30 },
  { "activeXTextDspClass", global_str5, reg_str24 },
  { "activeXTextDspClass:noedit", global_str2, reg_str12 },
  { "activeSliderClass", global_str5, reg_str13 },
  { "activeMotifSliderClass", global_str5, reg_str22 },
  { "activeButtonClass", global_str5, reg_str14 },
  { "activeMenuButtonClass", global_str5, reg_str15 },
  { "activeRadioButtonClass", global_str5, reg_str23 },
  { "activeMessageButtonClass", global_str5, reg_str16 },
  { "activeUpdownButtonClass", global_str5, reg_str21 },
  { "activeRampButtonClass", global_str5, reg_str28 },
  { "activeExitButtonClass", global_str5, reg_str17 },
  { "menuMuxClass", global_str5, reg_str18 },
  { "relatedDisplayClass", global_str5, reg_str19 },
  { "shellCmdClass", global_str5, reg_str20 },
  { "pvInspectorClass", global_str2, reg_str26 },
  { "activeTableClass", global_str2, reg_str27 },
  { "activeCoefTableClass", global_str2, reg_str29 },
  { "activeMpStrobeClass", global_str5, reg_str31 },
  { "activeSignalClass", global_str5, reg_str32 }
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
