/*
 *  Copyright 1993, 1994 University of British Columbia
 *  Copyright (c) Leipzig, Madrid 2004 - 2012
 *  Max-Planck-Institute for Human Cognitive and Brain Science	
 *  Max-Planck-Institute for Evolutionary Anthropology 
 *  BIT, ETSI Telecomunicacion, UPM
 *
 *  Permission to use, copy, modify, distribute, and sell this software and its
 *  documentation for any purpose is hereby granted without fee, provided that
 *  the above copyright notice appears in all copies and that both that
 *  copyright notice and this permission notice appear in supporting
 *  documentation. UBC makes no representations about the suitability of this
 *  software for any purpose. It is provided "as is" without express or
 *  implied warranty.
 *
 *  Author: Arthur Pope, UBC Laboratory for Computational Intelligence
 *  Maintainance and Fixes: Gert Wollny, UPM 
 */

#include <miaconfig.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <vistaio/vistaio.h>



int nx = 10; 
int ny = 11; 
int nz = 12; 

#define VTYPE double
#define TYPEREPN VistaIODoubleRepn

VistaIOBoolean RepnFilter(VistaIOBundle bundle, VistaIORepnKind repn) 
{
	return repn == VistaIOField3DRepn; 
}

VistaIOField3D VistaIORead3DVectorfield(FILE *file) 
{
	VistaIOField3D result; 
	VistaIOAttrList list; 
	
	result = NULL; 
	
	list = (VistaIOAttrList)VistaIOReadFile(file,RepnFilter);
	
	if (!list) 
		return NULL; 
	
	if (!VistaIOExtractAttr(list, "3DVector",NULL, VistaIOField3DRepn, &result,TRUE)) {
		VistaIOWarning("3DVector element of type 3dfield found");
	}
	
	VistaIODestroyAttrList(list);
	return result;	
}

int VistaIOWrite3DVectorfield(FILE *file, VistaIOField3D field)
{
	int retval; 
	VistaIOField3D help; 
	VistaIOAttrList list;
	
	retval = FALSE; 
	
	list = VistaIOCreateAttrList();
	if (list) {
		
		help = VistaIOMirrorField3D(field);
		
		VistaIOSetAttr(list,"3DVector",NULL,VistaIOField3DRepn,help);
		
		retval = VistaIOWriteFile(file,list);
		
		VistaIODestroyAttrList(list);
	}
	return retval; 
}


int main(int argc, const char *args[])
{
	FILE *file; 
	VistaIOField3D field; 
	VistaIOField3D field2;
	VistaIOAttrListPosn pos; 
	int x,y,z; 
	VTYPE *h; 
	VTYPE *k; 
	float test_float,number; 

	
	field2 = NULL; 
	test_float = 1.2; 
	
	/* Create the field */ 
	field = VistaIOCreateField3D(nx,ny,nz,3,TYPEREPN);
	
	/* put in some data */
	h = field->p.data;
	for (z = 0; z < nz; z++) 
		for (y = 0; y < ny; y++) 
			for (x = 0; x < nx; x++){

				*h++ = x; 
				*h++ = y + x;
				*h++ = z * 0.2;
			}
	
	/* add some attribute the the field */
	VistaIOSetAttr(field->attr,"test_attribute", NULL, VistaIOFloatRepn,test_float);
	
	/* open file for saving */
	if ((file = fopen("test.v","w"))==0)
		VistaIOError("unable to open file");
	
	/* Write out the data */
	VistaIOWrite3DVectorfield(file,field);
	
	/* close the file */ 
	fclose(file);
	
	
	/* open file for reading */
	if ((file = fopen("test.v","r")) == NULL)
	    VistaIOError("unable to open file for reading");
	
	/* Read field from file */     
	field2 = VistaIORead3DVectorfield(file);
	
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
	VistaIODestroyField3D(field);
	VistaIODestroyField3D(field2);

	unlink( "test.v" ); 
	return 0;
}
