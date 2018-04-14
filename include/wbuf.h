/*  sllist.c - source for single linked list lib
 *
 *  SLLIST - Single-Linked List Library
 *
 *  Copyright (C) 2000  Richard Heathfield
 *                      Eton Computer Systems Ltd
 *                      Macmillan Computer Publishing
 *
 *  This program is free software; you can redistribute it
 *  and/or modify it under the terms of the GNU General
 *  Public License as published by the Free Software
 *  Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will
 *  be useful, but WITHOUT ANY WARRANTY; without even the
 *  implied warranty of MERCHANTABILITY or FITNESS FOR A
 *  PARTICULAR PURPOSE.  See the GNU General Public License
 *  for more details.
 *
 *  You should have received a copy of the GNU General
 *  Public License along with this program; if not, write
 *  to the Free Software Foundation, Inc., 675 Mass Ave,
 *  Cambridge, MA 02139, USA.
 *
 *  Richard Heathfield may be contacted by email at:
 *     binary@eton.powernet.co.uk
 *
 */
#ifndef WBUF_H
#define WBUF_H
#include <avr/io.h>
#include <stdlib.h>

//#include "led.h"

#define WBF_ERROR -1
#define first 1
#define second 2
#define empty 0
#define full 1

typedef struct WBUFFER {
	char toggle;
	char status;
	int block;
	char pos;
	char* unload; 
	char* reload;
	char* stor1;
	char* stor2;
	size_t length;
} WBUFFER;

WBUFFER* wbuf_init(size_t buflen);
void wbuf_destroy(WBUFFER* buffer);
char wbuf_get_toggle(WBUFFER* buffer);
char wbuf_get_value(WBUFFER* buffer);
WBUFFER* wbuf_get_next(WBUFFER* buffer);
void wbuf_check_toggle(WBUFFER* buffer);
WBUFFER* wbuf_reload(WBUFFER* buffer, void* data, size_t datalen/*ohter params...*/);
#endif // WBUF_H
