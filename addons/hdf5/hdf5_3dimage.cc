/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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

#include <mia/core/filter.hh>

#include <addons/hdf5/hdf5a_mia.hh>
#include <addons/hdf5/hdf5_3dimage.hh>

NS_BEGIN(hdf5_3dimage)
using namespace mia; 

using std::string; 
using std::vector; 
using std::stringstream; 




CHDF53DImageIOPlugin::CHDF53DImageIOPlugin():
        C3DImageIOPlugin("hdf5")
{
        add_supported_type(it_bit);
        add_supported_type(it_sbyte);
        add_supported_type(it_ubyte);
	add_supported_type(it_sshort);
	add_supported_type(it_ushort);
	add_supported_type(it_sint);
	add_supported_type(it_uint);
#ifdef LONG_64BIT
	add_supported_type(it_slong);
	add_supported_type(it_ulong);
#endif 
	add_supported_type(it_float);
	add_supported_type(it_double);

	add_suffix(".h5");
	add_suffix(".H5");

        // enable some translators for post-conversion
	CVoxelAttributeTranslator::register_for("voxel");
	C3DIntAttributeTranslator::register_for("ca");
	C3DIntAttributeTranslator::register_for("cp");

}

struct HDF5ReadCallbackdata { 
	CHDF53DImageIOPlugin::Data& result; 
	string path;
        H5Base& id; 
}; 

herr_t hdf5_walk (hid_t loc_id, const char *name, const H5L_info_t *info,
                  void *operator_data)
{
        herr_t          status;
        H5O_info_t      infobuf;

        status = H5Oget_info_by_name (loc_id, name, &infobuf, H5P_DEFAULT);
        HDF5ReadCallbackdata *cbd = reinterpret_cast<HDF5ReadCallbackdata *>(operator_data); 
        string path = cbd->path + string("/") + string(name); 
        cvdebug() << "HDF5 read: " << path << "\n"; 

        switch (infobuf.type) {
        case H5O_TYPE_GROUP: { // recursion 
		cvdebug() << "Dive into group '" << name << "'\n"; 
		string sname(name);
		H5Base group =  H5Group::open(cbd->id, sname);
                HDF5ReadCallbackdata rec_cbd = {cbd->result, path, group};
                status = H5Literate (group, H5_INDEX_NAME, H5_ITER_NATIVE, NULL, hdf5_walk, &rec_cbd);
        }break; 
                
        case H5O_TYPE_DATASET: 
        case H5O_TYPE_NAMED_DATATYPE: {
		cvdebug() << "Found data set\n"; 
		auto dataset = H5Dataset::open(cbd->id, name);
		auto size = dataset.get_size(); 
		if (size.size() != 3) {
			cvdebug() << "HDF5 (3dimage): found " << size.size() << " data set, ignoring\n"; 
			break; 
		}
		C3DBounds bsize(size[2], size[1], size[0]); 
		H5Type file_type(H5Dget_type(dataset)); 
		H5Type mem_type = file_type.get_native_type(); 
		int type_id = mem_type.get_mia_type_id(); 
		switch (type_id) {
		case EAttributeType::attr_bool: 
			cbd->result.push_back(read_image<C3DBitImage>(bsize, dataset)); 
			break; 
		case EAttributeType::attr_uchar: 
			cbd->result.push_back(read_image<C3DUBImage>(bsize, dataset)); 
			break; 
		case EAttributeType::attr_schar:
			cbd->result.push_back(read_image<C3DSBImage>(bsize, dataset)); 
			break; 
		case EAttributeType::attr_ushort:
			cbd->result.push_back(read_image<C3DUSImage>(bsize, dataset)); 
			break; 
		case EAttributeType::attr_sshort:
			cbd->result.push_back(read_image<C3DSSImage>(bsize, dataset)); 
			break; 
		case EAttributeType::attr_uint:
			cbd->result.push_back(read_image<C3DUIImage>(bsize, dataset)); 
			break; 
		case EAttributeType::attr_sint:
			cbd->result.push_back(read_image<C3DSIImage>(bsize, dataset)); 
			break; 
#ifdef LONG_64BIT
		case EAttributeType::attr_ulong:
			cbd->result.push_back(read_image<C3DULImage>(bsize, dataset)); 
			break; 
		case EAttributeType::attr_slong:
			cbd->result.push_back(read_image<C3DSLImage>(bsize, dataset)); 
			break; 
#endif
		case EAttributeType::attr_float:
			cbd->result.push_back(read_image<C3DFImage>(bsize, dataset)); 
			break; 
		case EAttributeType::attr_double:
			cbd->result.push_back(read_image<C3DDImage>(bsize, dataset)); 
			break; 
		default: 
			cverr() << "HDF5 (3dimage): Found unsupported image pixel type " << type_id << "\n"; 
		}
        }break; 
        default: 
                cvdebug() << "HDF5 read: ignoring unknown '" << name << "'\n"; 
        }; 
        return 0; 
}


CHDF53DImageIOPlugin::PData CHDF53DImageIOPlugin::do_load(const string&  filename) const
{
        PData result(new Data); 
        
        H5File file = H5File::open(filename.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT); 
	if (file < 0) {
		// either the file doesn't exist or it is not a HDF5 file
		return PData(); 
	}
        
        HDF5ReadCallbackdata cbd = {*result, "", file}; 

        auto status = H5Literate (file, H5_INDEX_NAME, H5_ITER_NATIVE, NULL, hdf5_walk, &cbd);
	
	return result; 
}

class FHDF5Saver:public TFilter<void> {
public: 
        FHDF5Saver(H5File& file):m_file(file), m_id(0){}
        
        template <typename T> 
        void operator ()( const T3DImage<T>& image); 
private: 
        H5File& m_file; 
        int m_id; 

}; 

template <typename T> 
void FHDF5Saver::operator ()( const T3DImage<T>& image)
{
        vector<hsize_t> dims = {image.get_size().z, image.get_size().y, image.get_size().x}; 
        
        auto space = H5Space::create(dims); 
        
        auto file_type = Mia_to_h5_types<T>::file_datatype(); 
        
        stringstream path_str; 
        path_str << "/mia/" << m_id++; 
        auto path = path_str.str(); 
        if (image.has_attribute("hdf5-path")) {
                auto stored_path = image.get_attribute("hdf5-path");
                if (stored_path->type_id() == EAttributeType::attr_string) {
                        string s = stored_path->as_string(); 
                        if (s[0] == '/') {
                                path = s; 
                        }else{
                                cvwarn() << "HDF5 save: image has path " << s << " without leading '/', ignoring"; 
                        }
                }
        }
	cvdebug() << "Add image to '" << path << "'\n"; 
        auto dataset = H5Dataset::create(m_file, path.c_str(), file_type, space);
        dataset.write(image.begin(), image.end()); 
        translate_to_hdf5_attributes(dataset, image); 
}


std::string CHDF53DImageIOPlugin::do_get_preferred_suffix() const
{
	return "h5"; 
}

bool CHDF53DImageIOPlugin::do_save(const string& fname, const Data& data) const
{
        H5File file = H5File::create(fname.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
        FHDF5Saver saver(file); 
        for (auto i = data.begin(); i != data.end(); ++i) 
                mia::accumulate(saver, **i); 
        return true; 
}

const std::string CHDF53DImageIOPlugin::do_get_descr() const
{
        return "HDF5 3D image IO"; 
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{

	return new CHDF53DImageIOPlugin; 
}


NS_END
