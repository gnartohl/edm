#include <stdio.h>
#include <string.h>

#include "act_grf.h"
#include "act_win.h"
#include "app_pkg.h"

#ifdef __cplusplus
extern "C" {
#endif

// static int state = 0;

void win1Activate (
  activeWindowClass *ptr )
{

  printf( "this is win1Activate 1\n" );

}

void win1Deactivate (
  activeWindowClass *ptr )
{

  printf( "this is win1Deactivate... 1\n" );

}

void dtext1Change (
  activeGraphicClass *ptr )
{

int stat;
char value[31+1];

  stat = ptr->getProperty( ptr->idName(), "widgetValue", 31, value );
  printf( "dtext1Change, value = [%s]\n", value );

  stat = ptr->setProperty( "text1", "value", value );

}

void sl1Change (
  activeGraphicClass *ptr )
{

int stat;
double value;

  stat = ptr->getProperty( ptr->idName(), "controlValue", &value );
  printf( "sl1Change, value = %-g\n", value );

}

void b1Activate (
  activeGraphicClass *ptr )
{

  printf( "b1Activate...\n" );

}

void b1Deactivate (
  activeGraphicClass *ptr )
{

  printf( "b1Deactivate\n" );

}

void b1Down (
  activeGraphicClass *ptr )
{

int i;

printf( "b1Down\n" );

  ptr->setProperty( "dsym1", "gate", "up" );
  i = 2; ptr->setProperty( "sym1", "value", &i );
  i = 1; ptr->setProperty( ptr->idName(), "controlValue", &i );
  i = 1; ptr->setProperty( ptr->idName(), "readValue", &i );
  ptr->setProperty( "text1", "value", "down" );

//    ptr->openDisplay( "5.edl", 0 );

}

void b1Up (
  activeGraphicClass *ptr )
{

int i;

printf( "b1Up\n" );

  ptr->setProperty( "dsym1", "gate", "down" );
  i = 1; ptr->setProperty( "sym1", "value", &i );
  i = 0; ptr->setProperty( ptr->idName(), "controlValue", &i );
  i = 0; ptr->setProperty( ptr->idName(), "readValue", &i );
  ptr->setProperty( "text1", "value", "up" );

}

void yes (
  activeGraphicClass *ptr )
{

int i;

  i = 1; ptr->setProperty( ptr->idName(), "controlValue", &i );
  i = 1; ptr->setProperty( ptr->idName(), "readValue", &i );
  ptr->setProperty( "window1", "dsym1", "continuous", "yes" );

}

void no (
  activeGraphicClass *ptr )
{

int i;

  i = 0; ptr->setProperty( ptr->idName(), "controlValue", &i );
  i = 0; ptr->setProperty( ptr->idName(), "readValue", &i );
  ptr->setProperty( "window1", "dsym1", "continuous", "no" );

}

void button1Init (
  activeGraphicClass *ptr )
{

int i;

  i = 1; ptr->setProperty( ptr->idName(), "controlValue", &i );
  i = 0; ptr->setProperty( ptr->idName(), "readValue", &i );

}

void b2Init (
  activeGraphicClass *ptr )
{

int i;

  i = 1; ptr->setProperty( ptr->idName(), "controlValue", &i );
  i = 0; ptr->setProperty( ptr->idName(), "readValue", &i );

}

void b3Init (
  activeGraphicClass *ptr )
{

int i;

  i = 1; ptr->setProperty( ptr->idName(), "controlValue", &i );
  i = 0; ptr->setProperty( ptr->idName(), "readValue", &i );

}

void b4Init (
  activeGraphicClass *ptr )
{

int i;

  i = 1; ptr->setProperty( ptr->idName(), "controlValue", &i );
  i = 0; ptr->setProperty( ptr->idName(), "readValue", &i );

}

void b5Init (
  activeGraphicClass *ptr )
{

int i;

  i = 1; ptr->setProperty( ptr->idName(), "controlValue", &i );
  i = 0; ptr->setProperty( ptr->idName(), "readValue", &i );

}

#ifdef __cplusplus
}
#endif




