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

#ifndef __process_h
#define __process_h 1

#ifdef __osf__
#include <sys/time.h>
#endif

#ifdef __linux__
#include <time.h>
#include <sys/time.h>
#endif

#include "thread.h"
#include "sys_types.h"

class processClass {

private:

THREAD_LOCK_HANDLE processLock;

public:

int timeCount;
SYS_TIME_TYPE tim0, tim1;
double cycleTimeFactor;
int halfSecCount;

processClass ( void );

virtual ~processClass ( void );

int lock ( void );

int unlock ( void );

};

#endif
