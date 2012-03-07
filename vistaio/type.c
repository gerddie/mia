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

#include "vistaio/vistaio.h"

/*
 *  Information about built-in types.
 */

extern VTypeMethods VEdgesMethods;	/* in EdgesType.c */
extern VTypeMethods VImageMethods;	/* in ImageType.c */
extern VTypeMethods VGraphMethods;	/* in GraphType.c */
extern VTypeMethods VolumesMethods;	/* in VolumesType.c */
extern VTypeMethods VCPEListMethods;

static VRepnInfoRec builtin_repn_info[] = {
	{"unknown"},

	/* Integer and floating-point numbers: */
	{"bit", sizeof (VBit), 1, 0.0, 1.0, NULL}
	,
	{"ubyte", sizeof (VUByte), 8, 0.0, 255.0, NULL}
	,
	{"sbyte", sizeof (VSByte), 8, -128.0, 127.0, NULL}
	,
	{"short", sizeof (VShort), 16, -32768.0, 32767.0, NULL}
	,
	{"long", sizeof (VLong), 32, -2147483648.0, 2147483647.0, NULL}
	,
	{"float", sizeof (VFloat), 32,
	 -3.40282346638528860e+38, 3.40282346638528860e+38, NULL}
	,
	{"double", sizeof (VDouble), 64,
	 -1.797693134862315708e+308, 1.797693134862315708e+308, NULL}
	,
	
	/* Miscellaneous representations: */
	{"attr-list", sizeof (VAttrList), 0, 0.0, 0.0, NULL}
	,
	{"boolean", sizeof (VBoolean), 1, 0.0, 0.0, NULL}
	,
	{"bundle", sizeof (VPointer), 0, 0.0, 0.0, NULL}
	,
	{"list", sizeof (VList), 0, 0.0, 0.0, NULL}
	,
	{"pointer", sizeof (VPointer), 0, 0.0, 0.0, NULL}
	,
	{"string", sizeof (VString), 0, 0.0, 0.0, NULL}
	,

	/* Standard object types: */
	{"edges", sizeof (VPointer), 0, 0.0, 0.0, &VEdgesMethods}
	,
	{"image", sizeof (VPointer), 0, 0.0, 0.0, &VImageMethods}
	,

	/* new object types */
	{"graph", sizeof (VPointer), 0, 0.0, 0.0, &VGraphMethods},
	{"volumes", sizeof (VPointer), 0, 0.0, 0.0, &VolumesMethods},
	{"cpelist", sizeof (VPointer), 0, 0.0, 0.0, &VCPEListMethods}, 
	{"field3d", sizeof (VPointer), 0, 0.0, 0.0, &VField3DMethods}, 
	{"field2d", sizeof (VPointer), 0, 0.0, 0.0, &VField2DMethods},
	{NULL, sizeof (VPointer), 0, 0.0, 0.0, NULL}
};


EXPORT_VISTA VRepnInfoRec *VRepnInfo = builtin_repn_info;

static VRepnKind nRepnKinds = VNRepnKinds;


/*! \brief Register some handlers for dealing with objects of a particular type.
 *  
 *  \param  name
 *  \param  methods
 *  \return Returns the VRepnKind code assigned the new type.
 */

VRepnKind VRegisterType (VStringConst name, VTypeMethods * methods)
{
	VRepnInfoRec *p;

	/* Move the existing type information into a bigger table: */
	if (VRepnInfo == builtin_repn_info) {
		VRepnInfo =
			VMalloc ((VNRepnKinds + 1) * sizeof (VRepnInfoRec));
		VCopy (builtin_repn_info, VRepnInfo, VNRepnKinds);
	} else
		VRepnInfo =
			VRealloc (VRepnInfo,
				  (nRepnKinds + 1) * sizeof (VRepnInfoRec));

	/* Write the new type's info into the last table entry: */
	p = VRepnInfo + nRepnKinds;
	p->name = VNewString (name);
	p->precision = p->min_value = p->max_value = 0.0; 
	p->size =0;
	p->methods = methods;

	return nRepnKinds++;
}


/*! \brief Locate information about a named type.
 *
 *  \param  name
 *  \return VRepnKind
 */

VRepnKind VLookupType (VStringConst name)
{
	VRepnKind repn;

	for (repn = VUnknownRepn; repn < nRepnKinds; repn++)
		if (strcmp (VRepnInfo[repn].name, name) == 0)
			return repn;
	return VUnknownRepn;
}
