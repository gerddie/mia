/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2015 Gert Wollny
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
#include <mia/core/file.hh>
#include <mia/core/filter.hh>
#include <mia/core/msgstream.hh>
#include <mia/core/attribute_names.hh>
#include <mia/2d/imageio.hh>

#include <vistaio/vista4mia.hh>

NS_BEGIN(vista_2d_io)

NS_MIA_USE

using namespace std;
using namespace boost;

class CVista2DImageIOPlugin : public C2DImageIOPlugin {
public:
	CVista2DImageIOPlugin();
private:
	PData do_load(const string& fname) const;
	bool do_save(const string& fname, const Data& data) const;
	const string do_get_descr() const;
	const std::string do_get_preferred_suffix() const; 
};

CVista2DImageIOPlugin::CVista2DImageIOPlugin():
	C2DImageIOPlugin("vista")
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

	add_standard_vistaio_properties(*this); 

	add_suffix(".v");
	add_suffix(".V");
	add_suffix(".vista");
	add_suffix(".VISTA");

}


template <typename T>
void read_image(VistaIOImage image, CVista2DImageIOPlugin::Data& result_list, int aqnr)
{
	size_t n = VistaIOImageNBands(image);
	size_t slice_size = VistaIOImageNColumns(image)* VistaIOImageNRows(image);
	typedef typename vista_repnkind<T>::type O;
	O *begin = (O*)VistaIOPixelPtr(image,0,0,0);
	O *end = begin + slice_size;

	size_t idx = 0; 
	while (idx < n) {
		T2DImage<T> *result = new T2DImage<T>(C2DBounds(VistaIOImageNColumns(image), VistaIOImageNRows(image)));
		P2DImage presult(result);
		std::copy(begin, end, result->begin());
		copy_attr_list(*result, VistaIOImageAttrList(image));

		presult->set_attribute(IDInstanceNumber, PAttribute(new CIntAttribute(idx))); 
		if (!presult->has_attribute(IDAcquisitionNumber))
			presult->set_attribute(IDAcquisitionNumber, PAttribute(new CIntAttribute(aqnr)));
		result_list.push_back(presult);
		begin += slice_size;
		end   += slice_size;
		++idx; 
	}
}

void copy_from_vista(VistaIOImage image, CVista2DImageIOPlugin::Data& result, int i)
{
	VistaIOBoolean is_unsigned = 0; 
	VistaIOExtractAttr (VistaIOImageAttrList(image), "repn-unsigned",NULL, VistaIOBitRepn, 
			    &is_unsigned, 0);

	switch (VistaIOPixelRepn(image)) {
	case VistaIOBitRepn :  read_image<bool>(image, result, i);break;
	case VistaIOUByteRepn :  read_image<unsigned char>(image, result, i);break;
	case VistaIOSByteRepn :  read_image<signed char>(image, result, i);break;
	case VistaIOShortRepn : 
		if (is_unsigned) 
			read_image<unsigned short>(image, result, i);
		else 
			 read_image<signed short>(image, result, i);
		break;
	case VistaIOLongRepn : 
		if (is_unsigned) 
			 read_image<unsigned int>(image, result, i);
		else
			 read_image<signed int>(image, result, i);
		break;
	case VistaIOFloatRepn :  read_image<float>(image, result, i);break;
	case VistaIODoubleRepn : read_image<double>(image, result, i);break;
	default:
		throw invalid_argument("2d vista load: Unknown pixel format");
	}
}

CVista2DImageIOPlugin::PData  CVista2DImageIOPlugin::do_load(const string& fname) const
{
	CInputFile f(fname);

	VistaIOImage *images = NULL;
	VistaIOAttrList attr_list = NULL;
	int nimages = VistaIOReadImages(f, &attr_list, &images);
	if (!nimages)
		return 	CVista2DImageIOPlugin::PData();

	CVista2DImageIOPlugin::PData result(new CVista2DImageIOPlugin::Data());

	for (int i = 0; i < nimages; ++i) {
		copy_from_vista(images[i], *result, i);
		VistaIODestroyImage(images[i]);
	}

	VistaIOFree(images);
	VistaIODestroyAttrList (attr_list);

	return result;
}






struct CVImageCreator: public TFilter <VistaIOImage> {
	template <typename T>
	VistaIOImage operator ()( const T2DImage<T>& image) const;
};

template <typename T>
VistaIOImage CVImageCreator::operator ()( const T2DImage<T>& image) const
{
	typedef dispatch_creat_vimage<typename T2DImage<T>::const_iterator, T> dispatcher;
	VistaIOImage result = dispatcher::apply(image.begin(), image.end(), image.get_size().x, image.get_size().y, 1);
	copy_attr_list(VistaIOImageAttrList(result), image);
	return result;
}

bool CVista2DImageIOPlugin::do_save(const string& fname, const C2DImageVector& data) const
{

	COutputFile f(fname);

	vector<VistaIOImage> images(data.size());

	CVImageCreator creator;
	vector<VistaIOImage>::iterator img = images.begin();
	for (C2DImageVector::const_iterator i = data.begin();
	     i != data.end(); ++i, ++img) {
		*img = filter(creator, **i);
	}

	bool result = VistaIOWriteImages(f, NULL, data.size(), &images[0]) == TRUE;

	// clean up
	for (vector<VistaIOImage>::iterator i = images.begin(); i != images.end(); ++i)
		VistaIODestroyImage(*i);

	return result;
}

const string CVista2DImageIOPlugin::do_get_descr() const
{
	return "a 2dimage io plugin for vista images";
}

const std::string CVista2DImageIOPlugin::do_get_preferred_suffix() const
{
	return "v"; 
}


extern "C" EXPORT  CPluginBase *get_plugin_interface()
{
	C2DFVectorTranslator::register_for("pixel");
	return new CVista2DImageIOPlugin();
}

NS_END
