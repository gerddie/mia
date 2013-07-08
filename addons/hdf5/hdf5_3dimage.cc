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

namespace hdf5_3dimage {

CHDF53DImageIOPlugin::CHDF53DImageIOPlugin():
        C3DImageIOPlugin("hdf5")
{
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

        // enable the 3DVector translator for the voxel size 
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
                HDF5ReadCallbackdata rec_cbd = {cbd->result, path, loc_id};
                status = H5Literate (loc_id, H5_INDEX_NAME, H5_ITER_NATIVE, NULL, hdf5_walk, &rec_cbd);
        }break; 
                
        case H5O_TYPE_DATASET: 
        case H5O_TYPE_NAMED_DATATYPE: {
                // read the data set 
                

        }break; 
        default: 
                cvdebug() << "HDF5 read: ignoring unknown '" << name << "'\n"; 
        }; 
        
}


CHDF53DImageIOPlugin::PData CHDF53DImageIOPlugin::do_load(const string&  filename) const
{
        PData result(new Data); 
        

        H5File file = H5File::open(fname.c_str(), H5F_RDONLY, H5P_DEFAULT); 
        
        HDF5ReadCallbackdata cbd = {*result, "", file}; 

        status = H5Literate (file, H5_INDEX_NAME, H5_ITER_NATIVE, NULL, hdf5_walk, &cbd);

}

class FHDF5Saver:public TFilter<void> {
        FHDF5Saver(H5File& file):m_file(file), m_id(0){}
        
        template <typename T> 
        void operator ()( const T3DImage<T>& image); 

        H5File& m_file; 
        int m_id; 

}; 

template <typename T> 
void FHDF5Saver::operator ()( const T3DImage<T>& image)
{
        vector<hsize_t> dims = {image.get_size().z, image.get_size().y, image.get_size().x}; 
        
        auto space = H5Space::create(dims); 
        
        auto file_type = Mia_to_h5_types<int>::file_datatype(); 
        auto mem_type = Mia_to_h5_types<int>::mem_datatype(); 
        
        stringstream path_str; 
        path_str << "/mia/" << m_id++; 
        auto path = path_str.str(); 
        if (image.has_attribute("hdf5-path")) {
                auto stored_path = image.get_attribute("hdf5-path");
                if (stored_path->type_id() == EAttributeType::attr_string) {
                        string s = stored->as_string(); 
                        if (s[0] == '/') {
                                path = s; 
                        }else{
                                cvwarn() << "HDF5 save: image has path " << s << " without leading '/', ignoring"; 
                        }
                }
        }

        auto dataset = H5Dataset::create(get_file(), path.c_str(), file_type, space);
        dataset.write(mem_type, &image(0,0,0)); 
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


}
