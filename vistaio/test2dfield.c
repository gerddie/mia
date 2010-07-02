/*
** Copyright (c) Leipzig, Madrid 1999-2010
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
#define TYPEREPN VDoubleRepn

VBoolean RepnFilter(VBundle bundle, VRepnKind repn) 
{
	return repn == VField2DRepn; 
}

VField2D VRead2DVectorfield(FILE *file) 
{
	VField2D result; 
	VAttrList list; 
	
	result = NULL; 
	
	list = (VAttrList)VReadFile(file,RepnFilter);
	
	if (!list) 
		return NULL; 
	
	if (!VExtractAttr(list, "2DVector",NULL, VField2DRepn, &result,TRUE)) {
		VWarning("2DVector element of type 2dfield found");
	}
	
	VDestroyAttrList(list);
	return result;	
}

int VWrite2DVectorfield(FILE *file, VField2D field)
{
	int retval; 
	VField2D help; 
	VAttrList list;
	
	retval = FALSE; 
	
	list = VCreateAttrList();
	if (list) {
		
		help = VMirrorField2D(field);
		
		VSetAttr(list,"2DVector",NULL,VField2DRepn,help);
		
		retval = VWriteFile(file,list);
		
		VDestroyAttrList(list);
	}
	return retval; 
}


int main(int argc, const char *args[])
{
	FILE *file; 
	VField2D field; 
	VField2D field2;
	VAttrListPosn pos; 
	int x,y,z; 
	VTYPE *h; 
	VTYPE *k; 
	float test_float,number; 

	
	field2 = NULL; 
	test_float = 1.2; 
	
	/* Create the field */ 
	field = VCreateField2D(nx,ny,2,TYPEREPN);
	
	/* put in some data */
	h = field->p.data;
	for (y = 0; y < ny; y++) 
		for (x = 0; x < nx; x++){
			
			*h++ = x; 
			*h++ = y + x;
		}
	
	/* add some attribute the the field */
	VSetAttr(field->attr,"test_attribute", NULL, VFloatRepn,test_float);
	
	/* open file for saving */
	if ((file = fopen("test.v","w"))==0)
		VError("unable to open file");
	
	/* Write out the data */
	VWrite2DVectorfield(file,field);
	
	/* close the file */ 
	fclose(file);
	
	
	/* open file for reading */
	if ((file = fopen("test.v","r")) == NULL)
	    VError("unable to open file for reading");
	
	/* Read field from file */     
	field2 = VRead2DVectorfield(file);
	
	/* close the file */ 
	fclose(file);
	
	
	/* testing follows */ 
	
	h = field->p.data;
	k = field2->p.data;
	z = field->nsize; 
	if (z != field2->nsize) {
		VError("Error rereading test file");
	}
	
	z /= sizeof(VTYPE); 
	
	while (z--) {
		if (*h != *k) {
			VWarning("field en/decoding error: delta = %e",*h - *k);
		}
		h++; 
		k++; 
			
	}
	
	if (!VLookupAttr(field2->attr,"test_attribute",&pos)) 
		VError("not found");	
	
	
	VGetAttrValue(&pos, NULL, VFloatRepn, &number);

	if (number != test_float) {
		VError("numbers differ by %e",number - test_float);
	}
	VDestroyField2D(field);
	VDestroyField2D(field2);

	unlink( "test.v" ); 
	return 0;
}
