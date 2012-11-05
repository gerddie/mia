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
#define TYPEREPN VDoubleRepn

VBoolean RepnFilter(VBundle bundle, VRepnKind repn) 
{
	return repn == VField3DRepn; 
}

VField3D VRead3DVectorfield(FILE *file) 
{
	VField3D result; 
	VAttrList list; 
	
	result = NULL; 
	
	list = (VAttrList)VReadFile(file,RepnFilter);
	
	if (!list) 
		return NULL; 
	
	if (!VExtractAttr(list, "3DVector",NULL, VField3DRepn, &result,TRUE)) {
		VWarning("3DVector element of type 3dfield found");
	}
	
	VDestroyAttrList(list);
	return result;	
}

int VWrite3DVectorfield(FILE *file, VField3D field)
{
	int retval; 
	VField3D help; 
	VAttrList list;
	
	retval = FALSE; 
	
	list = VCreateAttrList();
	if (list) {
		
		help = VMirrorField3D(field);
		
		VSetAttr(list,"3DVector",NULL,VField3DRepn,help);
		
		retval = VWriteFile(file,list);
		
		VDestroyAttrList(list);
	}
	return retval; 
}


int main(int argc, const char *args[])
{
	FILE *file; 
	VField3D field; 
	VField3D field2;
	VAttrListPosn pos; 
	int x,y,z; 
	VTYPE *h; 
	VTYPE *k; 
	float test_float,number; 

	
	field2 = NULL; 
	test_float = 1.2; 
	
	/* Create the field */ 
	field = VCreateField3D(nx,ny,nz,3,TYPEREPN);
	
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
	VSetAttr(field->attr,"test_attribute", NULL, VFloatRepn,test_float);
	
	/* open file for saving */
	if ((file = fopen("test.v","w"))==0)
		VError("unable to open file");
	
	/* Write out the data */
	VWrite3DVectorfield(file,field);
	
	/* close the file */ 
	fclose(file);
	
	
	/* open file for reading */
	if ((file = fopen("test.v","r")) == NULL)
	    VError("unable to open file for reading");
	
	/* Read field from file */     
	field2 = VRead3DVectorfield(file);
	
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
	VDestroyField3D(field);
	VDestroyField3D(field2);

	unlink( "test.v" ); 
	return 0;
}
