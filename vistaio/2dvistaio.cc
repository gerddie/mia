/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2012 Gert Wollny
 *
 * This program is free software; you can redistribute it and/or modify
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <sstream>
#include <cassert>
#include <algorithm>
#include <mia/core/file.hh>
#include <mia/core/filter.hh>
#include <mia/core/msgstream.hh>
#include <mia/2d/2dimageio.hh>

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
	std::string do_get_preferred_suffix() const; 
};

CVista2DImageIOPlugin::CVista2DImageIOPlugin():
	C2DImageIOPlugin("vista")
{
	add_supported_type(it_bit);
	add_supported_type(it_ubyte);
	add_supported_type(it_sbyte);
	add_supported_type(it_sshort);
	add_supported_type(it_sint);
	add_supported_type(it_float);
	add_supported_type(it_double);
	add_property(io_plugin_property_multi_record);

	add_suffix(".v");
	add_suffix(".V");
	add_suffix(".vista");
	add_suffix(".VISTA");

}


template <typename T>
void read_image(VImage image, CVista2DImageIOPlugin::Data& result_list, int aqnr)
{
	size_t n = VImageNBands(image);
	size_t slice_size = VImageNColumns(image)* VImageNRows(image);
	typedef typename vista_repnkind<T>::type O;
	O *begin = (O*)VPixelPtr(image,0,0,0);
	O *end = begin + slice_size;

	size_t idx = 0; 
	while (idx < n) {
		T2DImage<T> *result = new T2DImage<T>(C2DBounds(VImageNColumns(image), VImageNRows(image)));
		P2DImage presult(result);
		std::copy(begin, end, result->begin());
		copy_attr_list(*result, VImageAttrList(image));

		presult->set_attribute(IDInstanceNumber, PAttribute(new CIntAttribute(idx))); 
		if (!presult->has_attribute(IDAcquisitionNumber))
			presult->set_attribute(IDAcquisitionNumber, PAttribute(new CIntAttribute(aqnr)));
		result_list.push_back(presult);
		begin += slice_size;
		end   += slice_size;
		++idx; 
	}
}

void copy_from_vista(VImage image, CVista2DImageIOPlugin::Data& result, int i)
{
	// this could be changed to add a bunch of images
	// however, then one would also have to write these as such ...
	switch (VPixelRepn(image)) {
	case VBitRepn : return read_image<bool>(image, result, i);
	case VUByteRepn : return read_image<unsigned char>(image, result, i);
	case VSByteRepn : return read_image<signed char>(image, result, i);
	case VShortRepn : return read_image<signed short>(image, result, i);
	case VLongRepn : return read_image<signed int>(image, result, i);
	case VFloatRepn : return read_image<float>(image, result, i);
	case VDoubleRepn : return read_image<double>(image, result, i);
	default:
		throw invalid_argument("2d vista load: Unknown pixel format");
	}
}

CVista2DImageIOPlugin::PData  CVista2DImageIOPlugin::do_load(const string& fname) const
{
	CInputFile f(fname);

	VImage *images = NULL;
	VAttrList attr_list = NULL;
	int nimages = VReadImages(f, &attr_list, &images);
	if (!nimages)
		return 	CVista2DImageIOPlugin::PData();

	CVista2DImageIOPlugin::PData result(new CVista2DImageIOPlugin::Data());

	for (int i = 0; i < nimages; ++i) {
		copy_from_vista(images[i], *result, i);
		VDestroyImage(images[i]);
	}

	VFree(images);
	VDestroyAttrList (attr_list);

	return result;
}






struct CVImageCreator: public TFilter <VImage> {
	template <typename T>
	VImage operator ()( const T2DImage<T>& image) const;
};

template <typename T>
VImage CVImageCreator::operator ()( const T2DImage<T>& image) const
{
	typedef dispatch_creat_vimage<typename T2DImage<T>::const_iterator, typename vista_repnkind<T>::type> dispatcher;
	VImage result = dispatcher::apply(image.begin(), image.end(), image.get_size().x, image.get_size().y, 1);
	copy_attr_list(VImageAttrList(result), image);
	return result;
}

bool CVista2DImageIOPlugin::do_save(const string& fname, const C2DImageVector& data) const
{

	COutputFile f(fname);

	vector<VImage> images(data.size());

	CVImageCreator creator;
	vector<VImage>::iterator img = images.begin();
	for (C2DImageVector::const_iterator i = data.begin();
	     i != data.end(); ++i, ++img) {
		*img = filter(creator, **i);
	}

	bool result = VWriteImages(f, NULL, data.size(), &images[0]) == TRUE;

	// clean up
	for (vector<VImage>::iterator i = images.begin(); i != images.end(); ++i)
		VDestroyImage(*i);

	return result;
}

const string CVista2DImageIOPlugin::do_get_descr() const
{
	return "a 2dimage io plugin for vista images";
}

std::string CVista2DImageIOPlugin::do_get_preferred_suffix() const
{
	return "v"; 
}


extern "C" EXPORT  CPluginBase *get_plugin_interface()
{
	TTranslator<C2DFVector>::register_for("pixel");
	return new CVista2DImageIOPlugin();
}

NS_END
