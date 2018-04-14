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
#include "wbuf.h"

//-----------------------------------------------------------------------------
WBUFFER* wbuf_init(size_t buflen) 
{
	WBUFFER* buffer = NULL;	
	if (buflen > 0) {   
		buffer = malloc(sizeof *buffer);
		if (NULL != buffer) {
			buffer->stor1 = malloc(sizeof *buffer->stor1 * buflen);		
			buffer->stor2 = malloc(sizeof *buffer->stor2 * buflen);
			if (NULL != buffer->stor1 && NULL != buffer->stor2) {
				buffer->unload = buffer->stor1;
				buffer->reload = buffer->stor2;
				buffer->status = full;
				buffer->toggle = first;
				buffer->pos = 0;
				buffer->block = 0;//1;
				buffer->length = buflen;
			} else {
				free(buffer->stor1);
				free(buffer->stor2);
				free(buffer);
				buffer = NULL;
			} 
		} else {
			free(buffer);
			buffer = NULL;
		}
	}	
	return buffer;
}
//-----------------------------------------------------------------------------
void wbuf_destroy(WBUFFER* buffer) 
{
	if (NULL != buffer) {
		free(buffer->stor1);
		free(buffer->stor2);
		free(buffer);
		buffer = NULL;
	}
	return;
}
//-----------------------------------------------------------------------------
char wbuf_get_toggle(WBUFFER* buffer)
{
	char toggle = WBF_ERROR;	
	if (NULL != buffer) {
		toggle = buffer->toggle;
	}	
	return toggle;
}
//-----------------------------------------------------------------------------
char wbuf_get_value(WBUFFER* buffer)
{
	char value = 0;	
	if (NULL != buffer && buffer->pos >= 0) {
		value = buffer->unload[buffer->pos];
	}
	return value;	
}
//-----------------------------------------------------------------------------
WBUFFER* wbuf_get_next(WBUFFER* buffer) 
{
	if (NULL != buffer) {
		buffer->pos += 1;
		// experimental do not work :)
		//if (buffer->pos >= buffer->length) {
		//	buffer->pos = 0;
		//}
	}
	return buffer;
}
//-----------------------------------------------------------------------------
void wbuf_check_toggle(WBUFFER* buffer) 
{
	if (NULL != buffer) {
		switch (buffer->toggle) {
		case first:
			if (buffer->length == buffer->pos) {
				buffer->unload = buffer->stor2;
				buffer->reload = buffer->stor1;
				buffer->status = empty;
				buffer->toggle = second;
				buffer->pos = 0;
			}		
			break;
		case second:
			if (buffer->length == buffer->pos) {
				buffer->unload = buffer->stor1;
				buffer->reload = buffer->stor2;
				buffer->status = empty;
				buffer->toggle = first;
				buffer->pos = 0;
			}	
			break;
		}
	}
	return;
}
//-----------------------------------------------------------------------------
WBUFFER* wbuf_reload(WBUFFER* buffer, void* data, size_t datalen/*ohter params...*/) 
{
	if (NULL != buffer && NULL != data) {
		if (empty == buffer->status) {			
			int* p = data;			
			/*uint16_t*/int addr = buffer->block * buffer->length - buffer->length + 1;		
			for (int i = 0; i < buffer->length; i++) {				
				buffer->reload[i] = p[addr++];
				if (buffer->length-1 == i) {
					buffer->status = full;
				}
				// dependendent part... neet some mind force :)
				if (addr >= datalen) {
					buffer->block = 1;
				} 
			}
			buffer->block++;
		}
	}
	return buffer;
}
