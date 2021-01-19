/*
   rdesktop: A Remote Desktop Protocol client.

   Support functions for Extended Window Manager Hints,
   http://www.freedesktop.org/standards/wm-spec.html

   Copyright (C) Matthew Chapman 1999-2002
   
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "rdesktop.h"
#include "exception.h"

/* 
   Get window property value (32 bit format) 
   Returns zero on success, -1 on error
*/
static int
get_property_value(char *propname, long max_length,
		   unsigned long *nitems_return, unsigned char **prop_return)
{
	throw not_implemented_error();
}

/* 
   Get current desktop number
   Returns -1 on error
*/
static int
get_current_desktop(void)
{
	throw not_implemented_error();
}

/*
  Get workarea geometry
  Returns zero on success, -1 on error
 */

int
get_current_workarea(uint32 * x, uint32 * y, uint32 * width, uint32 * height)
{
	throw not_implemented_error();
}
