/*
** Copyright (c) Leipzig, Madrid 1999-2012
**                    Gert Wollny <wollny@cns.mpg.de>
**  
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 3 of the License, or
** (at your option) any later version.
** 
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
** 
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software 
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/


#include <miaconfig.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <vistaio/vistaio.h>



int nx = 10; 
int ny = 11; 

#define VTYPE double
#define TYPEREPN VistaIODoubleRepn

VistaIOBoolean RepnFilter(VistaIOBundle bundle, VistaIORepnKind repn) 
{
	return repn == VistaIOField2DRepn; 
}

VistaIOField2D VistaIORead2DVectorfield(FILE *file) 
{
	VistaIOField2D result; 
	VistaIOAttrList list; 
	
	result = NULL; 
	
	list = (VistaIOAttrList)VistaIOReadFile(file,RepnFilter);
	
	if (!list) 
		return NULL; 
	
	if (!VistaIOExtractAttr(list, "2DVector",NULL, VistaIOField2DRepn, &result,TRUE)) {
		VistaIOWarning("2DVector element of type 2dfield found");
	}
	
	VistaIODestroyAttrList(list);
	return result;	
}

int VistaIOWrite2DVectorfield(FILE *file, VistaIOField2D field)
{
	int retval; 
	VistaIOField2D help; 
	VistaIOAttrList list;
	
	retval = FALSE; 
	
	list = VistaIOCreateAttrList();
	if (list) {
		
		help = VistaIOMirrorField2D(field);
		
		VistaIOSetAttr(list,"2DVector",NULL,VistaIOField2DRepn,help);
		
		retval = VistaIOWriteFile(file,list);
		
		VistaIODestroyAttrList(list);
	}
	return retval; 
}


int main(int argc, const char *args[])
{
	FILE *file; 
	VistaIOField2D field; 
	VistaIOField2D field2;
	VistaIOAttrListPosn pos; 
	int x,y,z; 
	VTYPE *h; 
	VTYPE *k; 
	float test_float,number; 

	
	field2 = NULL; 
	test_float = 1.2; 
	
	/* Create the field */ 
	field = VistaIOCreateField2D(nx,ny,2,TYPEREPN);
	
	/* put in some data */
	h = field->p.data;
	for (y = 0; y < ny; y++) 
		for (x = 0; x < nx; x++){
			
			*h++ = x; 
			*h++ = y + x;
		}
	
	/* add some attribute the the field */
	VistaIOSetAttr(field->attr,"test_attribute", NULL, VistaIOFloatRepn,test_float);
	
	/* open file for saving */
	if ((file = fopen("test.v","w"))==0)
		VistaIOError("unable to open file");
	
	/* Write out the data */
	VistaIOWrite2DVectorfield(file,field);
	
	/* close the file */ 
	fclose(file);
	
	
	/* open file for reading */
	if ((file = fopen("test.v","r")) == NULL)
	    VistaIOError("unable to open file for reading");
	
	/* Read field from file */     
	field2 = VistaIORead2DVectorfield(file);
	
	/* close the file */ 
	fclose(file);
	
	
	/* testing follows */ 
	
	h = field->p.data;
	k = field2->p.data;
	z = field->nsize; 
	if (z != field2->nsize) {
		VistaIOError("Error rereading test file");
	}
	
	z /= sizeof(VTYPE); 
	
	while (z--) {
		if (*h != *k) {
			VistaIOWarning("field en/decoding error: delta = %e",*h - *k);
		}
		h++; 
		k++; 
			
	}
	
	if (!VistaIOLookupAttr(field2->attr,"test_attribute",&pos)) 
		VistaIOError("not found");	
	
	
	VistaIOGetAttrValue(&pos, NULL, VistaIOFloatRepn, &number);

	if (number != test_float) {
		VistaIOError("numbers differ by %e",number - test_float);
	}
	VistaIODestroyField2D(field);
	VistaIODestroyField2D(field2);

	unlink( "test.v" ); 
	return 0;
}
