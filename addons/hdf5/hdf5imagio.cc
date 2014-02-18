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


#include <addons/hdf5/hdf5imageio.hh>
#include <hdf5.h>

NS_BEGIN(hdf5_3d_io)

NS_MIA_USE; 


CHDF53DImageIOPlugin::CHDF53DImageIOPlugin():
	C3DImageIOPlugin("vista")
{
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
	add_suffix(".h5");
	add_suffix(".H5");
}

CHDF53DImageIOPlugin::PData CHDF53DImageIOPlugin::do_load(const std::string& fname) const
{
        
}

struct FH5ImageSaver : public TFilter<void> {
        FH5ImageSaver(H5File& h5_file):m_h5_file(h5_file){}; 

        template <typename Image> 
        void operator ()( const Image& image); 

        void operator ()( const C3DBitImage& image); 

        H5File& h5_file; 
        int index = 0; 
}; 



template <typename T> 
void FH5ImageSaver::operator ()( const T3DImage<T>& image)
{
        auto hdf5path = image.get_attribute_as_string("descriptor");
        if (hdf5path.empty()) {
                stringstream s; 
                s << "/mia/" << index++;
                hdf5path = s.str(); 
        }


        int dims[3]; 
        dims[0] = image.get_size().z; 
        dims[1] = image.get_size().y; 
        dims[2] = image.get_size().x; 

        auto file_data_type = Mia_to_h5_types<T>::file_datatype();
        auto mem_data_type = Mia_to_h5_types<T>::internal_datatype();
	
	auto space =  H5Space::create(3, dims); 
	auto dataset = H5Dataset::create(h5_file, hdf5path.c_str(), file_data_type, *space); 
	auto status = dataset->write(space, mem_data_type, &image(0,0,0)); 
	

	
	
}

void FH5ImageSaver::operator ()( const C3DBitImage& image)
{
        // copy the image to a bitfield
        C3DBounds tmp_size((image.get_size().x + 7) / 8, image.get_size().y, image.get_size().z); 
        C3DUBImage tmp(tmp_size);

        auto o = tmp.begin(); 
        for (size_t z = 0; z < image.get_size().z; ++z) 
                for (size_t y = 0; y < image.get_size().y; ++y) {
                        char out = 0; 
                        int bit  = 1; 
                        auto iv = image.begin_at(0,y,z); 
                        for (size_t ox = 0; ox < tmp_size.x; ++ox, ++o) {
                                for (int i = 0; i < 8 && ix < image.get_size().x; 
                                     ++i, ++ix, ++iv, bit <<= 1) {
                                        if (*iv) 
                                                out |= bit;
                                }
                                *o = out; 
                        }
                }
        
        auto file_type = Mia_to_h5_types<bool>::file_datatype();
        auto internal_type = Mia_to_h5_types<bool>::internal_datatype();

	
        auto hdf5path = image.get_attribute_as_string("descriptor");
        if (hdf5path.empty()) {
                stringstream s; 
                s << "/mia/" << index++;
                hdf5path = s.str(); 
        }

        int dims[3]; 
        dims[0] = tmp_size.z; 
        dims[1] = tmp_size.y; 
        dims[2] = tmp_size.x; 

        H5S_t dataspace_id(H5Screate_simple(3, dims, NULL));

	int dim3 = 3; 
	H5S_t vector3d_space_id(H5Screate_simple(1, &dim3, NULL));

        H5D_t dataset_id(H5Dcreate(m_file_id, hdf5path.c_str(), file_type, dataspace_id, 
				   H5P_DEFAULT, H5P_DEFAULT));


	H5A_t real_size_attr(H5Acreate(dataset_id, "real-dim", Mia_to_h5_types<unsigned short>::file_datatype(), 
				       vector3d_space_id, H5P_DEFAULT, H5P_DEFAULT)); 
	
	H5Awrite(real_size_attr, Mia_to_h5_types<unsigned short>::internal_datatype(), &image.get_size().x);

        status = H5Dwrite(dataset_id, dataspace_id, internal_type, H5S_ALL, H5S_ALL, H5P_DEFAULT, 
                          &image(0,0,0));

	
}


bool CHDF53DImageIOPlugin::do_save(const std::string& fname, const Data& data) const
{
        herr_t status;

        // create the hdf5 structure 
        H5F_t file_id(H5Fopen(fname.str(), H5F_ACC_WR, H5P_DEFAULT));
        
        FH5ImageSaver saver(file_id); 
        for (auto i = data.begin(); i != data.end(); ++i) {
                mia::accumulate(saver, **data); 
        }
                
        return true; 

}

const std::string CHDF53DImageIOPlugin::do_get_descr() const
{
        return "HDF5 3D";
}

std::string CHDF53DImageIOPlugin::do_get_preferred_suffix() const
{
        return "h5"; 
}


NS_END
