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

extern VistaIOTypeMethods VistaIOEdgesMethods;	/* in EdgesType.c */
extern VistaIOTypeMethods VistaIOImageMethods;	/* in ImageType.c */
extern VistaIOTypeMethods VistaIOGraphMethods;	/* in GraphType.c */
extern VistaIOTypeMethods VolumesMethods;	/* in VolumesType.c */
extern VistaIOTypeMethods VCPEListMethods;

static VistaIORepnInfoRec builtin_repn_info[] = {
	{"unknown"},

	/* Integer and floating-point numbers: */
	{"bit", sizeof (VistaIOBit), 1, 0.0, 1.0, NULL}
	,
	{"ubyte", sizeof (VUByte), 8, 0.0, 255.0, NULL}
	,
	{"sbyte", sizeof (VSByte), 8, -128.0, 127.0, NULL}
	,
	{"short", sizeof (VistaIOShort), 16, -32768.0, 32767.0, NULL}
	,
	{"long", sizeof (VistaIOLong), 32, -2147483648.0, 2147483647.0, NULL}
	,
	{"float", sizeof (VistaIOFloat), 32,
	 -3.40282346638528860e+38, 3.40282346638528860e+38, NULL}
	,
	{"double", sizeof (VistaIODouble), 64,
	 -1.797693134862315708e+308, 1.797693134862315708e+308, NULL}
	,
	
	/* Miscellaneous representations: */
	{"attr-list", sizeof (VistaIOAttrList), 0, 0.0, 0.0, NULL}
	,
	{"boolean", sizeof (VistaIOBoolean), 1, 0.0, 0.0, NULL}
	,
	{"bundle", sizeof (VistaIOPointer), 0, 0.0, 0.0, NULL}
	,
	{"list", sizeof (VistaIOList), 0, 0.0, 0.0, NULL}
	,
	{"pointer", sizeof (VistaIOPointer), 0, 0.0, 0.0, NULL}
	,
	{"string", sizeof (VistaIOString), 0, 0.0, 0.0, NULL}
	,

	/* Standard object types: */
	{"edges", sizeof (VistaIOPointer), 0, 0.0, 0.0, &VistaIOEdgesMethods}
	,
	{"image", sizeof (VistaIOPointer), 0, 0.0, 0.0, &VistaIOImageMethods}
	,

	/* new object types */
	{"graph", sizeof (VistaIOPointer), 0, 0.0, 0.0, &VistaIOGraphMethods},
	{"volumes", sizeof (VistaIOPointer), 0, 0.0, 0.0, &VolumesMethods},
	{"cpelist", sizeof (VistaIOPointer), 0, 0.0, 0.0, &VCPEListMethods}, 
	{"field3d", sizeof (VistaIOPointer), 0, 0.0, 0.0, &VistaIOField3DMethods}, 
	{"field2d", sizeof (VistaIOPointer), 0, 0.0, 0.0, &VistaIOField2DMethods},
	{NULL, sizeof (VistaIOPointer), 0, 0.0, 0.0, NULL}
};


EXPORT_VISTA VistaIORepnInfoRec *VistaIORepnInfo = builtin_repn_info;

static VistaIORepnKind nRepnKinds = VNRepnKinds;


/*! \brief Register some handlers for dealing with objects of a particular type.
 *  
 *  \param  name
 *  \param  methods
 *  \return Returns the VistaIORepnKind code assigned the new type.
 */

VistaIORepnKind VistaIORegisterType (VistaIOStringConst name, VistaIOTypeMethods * methods)
{
	VistaIORepnInfoRec *p;

	/* Move the existing type information into a bigger table: */
	if (VistaIORepnInfo == builtin_repn_info) {
		VistaIORepnInfo =
			VistaIOMalloc ((VNRepnKinds + 1) * sizeof (VistaIORepnInfoRec));
		VistaIOCopy (builtin_repn_info, VistaIORepnInfo, VNRepnKinds);
	} else
		VistaIORepnInfo =
			VistaIORealloc (VistaIORepnInfo,
				  (nRepnKinds + 1) * sizeof (VistaIORepnInfoRec));

	/* Write the new type's info into the last table entry: */
	p = VistaIORepnInfo + nRepnKinds;
	p->name = VistaIONewString (name);
	p->precision = p->min_value = p->max_value = 0.0; 
	p->size =0;
	p->methods = methods;

	return nRepnKinds++;
}


/*! \brief Locate information about a named type.
 *
 *  \param  name
 *  \return VistaIORepnKind
 */

VistaIORepnKind VistaIOLookupType (VistaIOStringConst name)
{
	VistaIORepnKind repn;

	for (repn = VistaIOUnknownRepn; repn < nRepnKinds; repn++)
		if (strcmp (VistaIORepnInfo[repn].name, name) == 0)
			return repn;
	return VistaIOUnknownRepn;
}
