/* -*- mia-c++ -*-
 * Copyright (c) 2007 Gert Wollny <gert at die.upm.es>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <stdio.h>
#include <idl_export.h>

const char *program[] = {
        "tests_good = 0",
        "tests = 2", 

	"debug_level = 'message'", 
	"dummy = MIA_DEBUG_LEVEL(debug_level)",
	"result = MIA_ECHO(12)",
	"MIA_SET_TEST",

	"PRINT, 'TEST ECHO'",
	"IF ( result NE 12 ) THEN PRINT, 'ECHO failed:', result, '!= 12' ELSE tests_good = tests_good + 1 ",

	"filter = 'gauss:w=1'", 
	"in_type = 'float'", 
	"out_type = 'float'",

	"xsize = 3S", 
	"ysize = 3S", 
	"imag_in = fltarr(xsize,ysize)", 

	"imag_in[0,0] = 0.0", 
	"imag_in[1,0] = 0.0", 
	"imag_in[2,0] = 0.0", 
	"imag_in[0,1] = 0.0", 
	"imag_in[1,1] = 1.0", 
	"imag_in[2,1] = 0.0",
	"imag_in[0,2] = 0.0", 
	"imag_in[1,2] = 0.0", 
	"imag_in[2,2] = 0.0", 

	"imag_out = fltarr(xsize,ysize)",
	"imag_out[0,0] = 0.0625", 
	"imag_out[1,0] = 0.125", 
	"imag_out[2,0] = 0.0625", 
	"imag_out[0,1] = 0.125", 
	"imag_out[1,1] = 0.25", 
	"imag_out[2,1] = 0.125", 
	"imag_out[0,2] = 0.0625", 
	"imag_out[1,2] = 0.125", 
	"imag_out[2,2] = 0.0625", 

	"PRINT, 'CALL MIA_FILTER2D'",
	"imag_flt = MIA_FILTER2D(filter, imag_in)", 
	"PRINT, 'CALL MIA_FILTER2D DONE'",

	"filter2d_failed = 0", 
	"FOR i = 0, 2  DO "
	"	FOR j = 0, 2 DO "
	"             IF ( imag_out[i,j] NE imag_flt[i,j] ) THEN filter2d_failed = 1", 
	" IF ( filter2d_failed EQ 0 ) THEN tests_good = tests_good + 1 ELSE PRINT, 'MIA_FILTER2D failed'"

}; 
	
int main(int argc, char **argv) 
{
	if (IDL_Init(0, &argc, argv)) {
		IDL_Execute(sizeof(program)/sizeof(char*), program); 
		IDL_Cleanup(IDL_FALSE); 
	}
	return -1; 
}
