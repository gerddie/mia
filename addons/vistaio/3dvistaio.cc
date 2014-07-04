/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2014 Gert Wollny
 *
 * MIA is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with MIA; if not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <sstream>
#include <cassert>
#include <algorithm>
#include <cerrno>
#include <cstring>

#include <mia/core/file.hh>
#include <mia/core/filter.hh>
#include <mia/core/msgstream.hh>


#include <vistaio/3dvistaio.hh>
#include <vistaio/vista4mia.hh>

NS_BEGIN(vista_3d_io)

NS_MIA_USE

using namespace std;
using namespace boost;


CVista3DImageIOPlugin::CVista3DImageIOPlugin():
	C3DImageIOPlugin("vista")
{
	add_supported_type(it_bit);
	add_supported_type(it_ubyte);
	add_supported_type(it_sbyte);
	add_supported_type(it_sshort);
	add_supported_type(it_ushort);
	add_supported_type(it_sint);
	add_supported_type(it_uint);
	add_supported_type(it_float);
	add_supported_type(it_double);
	add_property(io_plugin_property_multi_record);
	add_property(io_plugin_property_has_attributes);
	add_suffix(".v");
	add_suffix(".V");
	add_suffix(".vista");
	add_suffix(".VISTA");
}

template <typename T>
P3DImage read_image(VistaIOImage image)
{
	cvdebug() << "Read  image of type '" << CPixelTypeDict.get_name(pixel_type<T>::value)<< "'\n"; 
	typedef typename vista_repnkind<T>::type O;
	T3DImage<T> *result = new T3DImage<T>(C3DBounds(VistaIOImageNColumns(image), VistaIOImageNRows(image), VistaIOImageNBands(image)));
	P3DImage presult(result);

	O *begin = (O*)VistaIOPixelPtr(image,0,0,0);
	O *end = begin + result->size();
	std::copy(begin, end, result->begin());

	copy_attr_list(*result, VistaIOImageAttrList(image));

	return presult;
}

P3DImage copy_from_vista(VistaIOImage image)
{

	VistaIOBoolean is_unsigned = 0; 
	VistaIOExtractAttr (VistaIOImageAttrList(image), "repn-unsigned",NULL, VistaIOBitRepn, 
			    &is_unsigned, 0);
	
	// this could be changed to add a bunch of images
	// however, then one would also have to write these as such ...
	switch (VistaIOPixelRepn(image)) {
	case VistaIOBitRepn : return read_image<bool>(image);
	case VistaIOUByteRepn : return read_image<unsigned char>(image);
	case VistaIOSByteRepn : return read_image<signed char>(image);
	case VistaIOShortRepn : 
		if (is_unsigned) 
			return read_image<unsigned short>(image);
		else 
			return read_image<signed short>(image);
	case VistaIOLongRepn : 
		if (is_unsigned) 
			return read_image<unsigned int>(image);
		else
			return read_image<signed int>(image);
	case VistaIOFloatRepn : return read_image<float>(image);
	case VistaIODoubleRepn : return read_image<double>(image);
	default:
		throw invalid_argument("3d vista load: Unknown pixel format");
	}
}

CVista3DImageIOPlugin::PData  CVista3DImageIOPlugin::do_load(const string& fname) const
{
	CInputFile f(fname);
	if (!f) {
		stringstream msg;
		msg << "Error opening '" << fname << "': " << strerror(errno);
		throw runtime_error(msg.str());
	}

	VistaIOImage *images;
	VistaIOAttrList attr_list;
	int nimages = VistaIOReadImages(f, &attr_list, &images);
	// a vista file?
	if (!nimages)
		return CVista3DImageIOPlugin::PData();

	CVista3DImageIOPlugin::PData result(new CVista3DImageIOPlugin::Data());
	for (int i = 0; i < nimages; ++i) {

		cvdebug() << "Read image no " << i << "\n"; 
		P3DImage r = copy_from_vista(images[i]);
		result->push_back(r);

		VistaIODestroyImage(images[i]);
	}

	VistaIOFree(images);
	VistaIODestroyAttrList (attr_list);

	return result;
}


struct CVImageCreator: public TFilter <VistaIOImage> {
	template <typename T>
	VistaIOImage operator ()( const T3DImage<T>& image) const;
};

template <typename T>
VistaIOImage CVImageCreator::operator ()( const T3DImage<T>& image) const
{
	typedef dispatch_creat_vimage<typename T3DImage<T>::const_iterator,
		typename vista_repnkind<T>::type> dispatcher;
	VistaIOImage result =  dispatcher::apply(image.begin(), image.end(),
					   image.get_size().x, image.get_size().y, image.get_size().z);
	copy_attr_list(VistaIOImageAttrList(result), image);
	return result;
}

bool CVista3DImageIOPlugin::do_save(const string& fname, const C3DImageVector& data) const
{

	COutputFile f(fname);

	vector<VistaIOImage> images(data.size());

	CVImageCreator creator;
	vector<VistaIOImage>::iterator img = images.begin();
	for (C3DImageVector::const_iterator i = data.begin();
	     i != data.end(); ++i, ++img) {
		*img = filter(creator, **i);
	}

	bool result = VistaIOWriteImages(f, NULL, data.size(), &images[0]) == TRUE;

	for (vector<VistaIOImage>::iterator i = images.begin(); i != images.end(); ++i)
		VistaIODestroyImage(*i);

	return result;
}

const string CVista3DImageIOPlugin::do_get_descr() const
{
	return "Vista 3D";
}

std::string CVista3DImageIOPlugin::do_get_preferred_suffix() const
{
	return "v"; 
}


extern "C" EXPORT  CPluginBase *get_plugin_interface()
{
	CVoxelAttributeTranslator::register_for("voxel");
	CVoxelAttributeTranslator::register_for("origin3d");
	C3DIntAttributeTranslator::register_for("ca");
	C3DIntAttributeTranslator::register_for("cp");
	return new CVista3DImageIOPlugin();
}

NS_END
