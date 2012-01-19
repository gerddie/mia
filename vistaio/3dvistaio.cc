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
#include <cerrno>
#include <cstring>

#include <mia/core/file.hh>
#include <mia/core/filter.hh>
#include <mia/core/msgstream.hh>
#include <mia/3d/3dimageio.hh>

#include <vistaio/vista4mia.hh>

NS_BEGIN(vista_3d_io)

NS_MIA_USE

using namespace std;
using namespace boost;

class CVista3DImageIOPlugin : public C3DImageIOPlugin {
public:
	CVista3DImageIOPlugin();
private:
	void do_add_suffixes(multimap<string, string>& map) const;
	PData do_load(const string& fname) const;
	bool do_save(const string& fname, const Data& data) const;
	const string do_get_descr() const;
	std::string do_get_preferred_suffix() const; 
};

CVista3DImageIOPlugin::CVista3DImageIOPlugin():
	C3DImageIOPlugin("vista")
{
	add_supported_type(it_bit);
	add_supported_type(it_ubyte);
	add_supported_type(it_sbyte);
	add_supported_type(it_sshort);
	add_supported_type(it_sint);
	add_supported_type(it_float);
	add_supported_type(it_double);
	add_property(io_plugin_property_multi_record);
	add_property(io_plugin_property_has_attributes);
}

void CVista3DImageIOPlugin::do_add_suffixes(multimap<string, string>& map) const
{
	map.insert(pair<string,string>(".v", get_name()));
	map.insert(pair<string,string>(".V", get_name()));
	map.insert(pair<string,string>(".vista", get_name()));
	map.insert(pair<string,string>(".VISTA", get_name()));

}


template <typename T>
P3DImage read_image(VImage image)
{
	typedef typename vista_repnkind<T>::type O;
	T3DImage<T> *result = new T3DImage<T>(C3DBounds(VImageNColumns(image), VImageNRows(image), VImageNBands(image)));
	P3DImage presult(result);

	O *begin = (O*)VPixelPtr(image,0,0,0);
	O *end = begin + result->size();
	std::copy(begin, end, result->begin());

	copy_attr_list(*result, VImageAttrList(image));

	return presult;
}

P3DImage copy_from_vista(VImage image)
{
	// this could be changed to add a bunch of images
	// however, then one would also have to write these as such ...
	switch (VPixelRepn(image)) {
	case VBitRepn : return read_image<bool>(image);
	case VUByteRepn : return read_image<unsigned char>(image);
	case VSByteRepn : return read_image<signed char>(image);
	case VShortRepn : return read_image<signed short>(image);
	case VLongRepn : return read_image<signed int>(image);
	case VFloatRepn : return read_image<float>(image);
	case VDoubleRepn : return read_image<double>(image);
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

	VImage *images;
	VAttrList attr_list;
	int nimages = VReadImages(f, &attr_list, &images);
	// a vista file?
	if (!nimages)
		return CVista3DImageIOPlugin::PData();


	CVista3DImageIOPlugin::PData result(new CVista3DImageIOPlugin::Data());

	for (int i = 0; i < nimages; ++i) {

		P3DImage r = copy_from_vista(images[i]);
		result->push_back(r);

		VDestroyImage(images[i]);
	}

	VFree(images);

	VDestroyAttrList (attr_list);

	return result;
}


struct CVImageCreator: public TFilter <VImage> {
	template <typename T>
	VImage operator ()( const T3DImage<T>& image) const;
};

template <typename T>
VImage CVImageCreator::operator ()( const T3DImage<T>& image) const
{
	typedef dispatch_creat_vimage<typename T3DImage<T>::const_iterator,
		typename vista_repnkind<T>::type> dispatcher;
	VImage result =  dispatcher::apply(image.begin(), image.end(),
					   image.get_size().x, image.get_size().y, image.get_size().z);
	copy_attr_list(VImageAttrList(result), image);
	return result;
}

bool CVista3DImageIOPlugin::do_save(const string& fname, const C3DImageVector& data) const
{

	COutputFile f(fname);

	vector<VImage> images(data.size());

	CVImageCreator creator;
	vector<VImage>::iterator img = images.begin();
	for (C3DImageVector::const_iterator i = data.begin();
	     i != data.end(); ++i, ++img) {
		*img = filter(creator, **i);
	}

	bool result = VWriteImages(f, NULL, data.size(), &images[0]) == TRUE;

	for (vector<VImage>::iterator i = images.begin(); i != images.end(); ++i)
		VDestroyImage(*i);

	return result;
}

const string CVista3DImageIOPlugin::do_get_descr() const
{
	return "a 3dimage io plugin for vista images";
}

std::string CVista3DImageIOPlugin::do_get_preferred_suffix() const
{
	return "v"; 
}


extern "C" EXPORT  CPluginBase *get_plugin_interface()
{
	CDoubleTranslator::register_for("double");
	CFloatTranslator::register_for("float");
	CUBTranslator::register_for("ubyte");
	CSBTranslator::register_for("sbyte");
	CSSTranslator::register_for("short");
	CSITranslator::register_for("int");
	CBitTranslator::register_for("bit");
	CVoxelAttributeTranslator::register_for("voxel");
	C3DIntAttributeTranslator::register_for("ca");
	C3DIntAttributeTranslator::register_for("cp");
	return new CVista3DImageIOPlugin();
}

NS_END
