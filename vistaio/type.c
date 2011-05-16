/*
*
* Copyrigh (C) 2004 Max-Planck-Institute of Cognitive Neurosience
*  
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU Lesser Public License as published by
* the Free Software Foundation; either version 3 of the License, or
* (at your option) any later version.
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU Lesser Public License for more details.
* 
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software 
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

/*$Id: type.c 976 2007-04-02 10:40:26Z wollny $ */

/*! \file   type.c
 *  \brief  Registeration of types (e.g., "image").
 *  \author Arthur Pope, UBC Laboratory for Computational Intelligentce
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
