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

#include "process.h"

processClass::processClass ( void )
{

int stat;

  stat = thread_init();
  stat = thread_create_lock_handle( &processLock );

}

processClass::~processClass ( void )
{

int stat;

  stat = thread_destroy_lock_handle( processLock );
  processLock = NULL;

}

int processClass::lock ( void )
{

int stat;

  stat = thread_lock( processLock );
  return stat;

}

int processClass::unlock ( void )
{

int stat;

  stat = thread_unlock( processLock );
  return stat;

}


