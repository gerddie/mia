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

#include <addons/nifti/niftiimage.hh>
#include <nifti1_io.h>


using namespace mia; 
using namespace std; 

NS_BEGIN(niftiimage)

static const char *AttrID_nifti_sform = "nifti-sform"; 
static const char *AttrID_nifti_sform_code = "nifti-sform-code"; 
static const char *AttrID_nifti_qform_code = "nifti-qform-code"; 
static const char *AttrID_nifti_intent_code = "nifti-intent-code"; 
static const char *AttrID_nifti_intent_name = "nifti-intent-name"; 
static const char *AttrID_nifti_intent_p1 = "nifti-intent_p1"; 
static const char *AttrID_nifti_intent_p2 = "nifti-intent_p2"; 
static const char *AttrID_nifti_intent_p3 = "nifti-intent_p3"; 

static const char *AttrID_nifti_toffset = "nifti-toffset";       // float 
static const char *AttrID_nifti_xyz_units = "nifti-xyz_units";   // int 
static const char *AttrID_nifti_time_units = "nifti-time_units"; // int 


CNifti3DImageIOPlugin::CNifti3DImageIOPlugin():
C3DImageIOPlugin("nifti")
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

	add_suffix(".nii");
	add_suffix(".NII");

        CVFloatTranslator::register_for(AttrID_nifti_sform); 
        CSSTranslator::register_for(AttrID_nifti_sform_code); 
        CSSTranslator::register_for(AttrID_nifti_qform_code);
        CSSTranslator::register_for(AttrID_nifti_intent_code); 
	
	CFloatTranslator::register_for("nifti-intent_p1"); 
	CFloatTranslator::register_for("nifti-intent_p2"); 
	CFloatTranslator::register_for("nifti-intent_p3"); 

        CFloatTranslator::register_for(AttrID_nifti_toffset); 
	CSITranslator::register_for(AttrID_nifti_xyz_units);
	CSITranslator::register_for(AttrID_nifti_time_units);

}

struct NiftiDeallocator {
	void operator () (nifti_image *image) {
		nifti_image_free(image); 
	}
}; 

void copy_attributes(C3DImage& image, const nifti_image& ni)
{
        // set position and orientation 
	image.set_attribute(AttrID_nifti_qform_code, ni.qform_code);
	
        if (ni.qform_code == 0) { // method 1
                image.set_orientation(ior_default);
                image.set_voxel_size(C3DFVector(ni.dx,  ni.dy, ni.dz)); 
        } else { // Method 2 
                image.set_orientation(ni.qfac == 1 ? ior_xyz : ior_xyz_flipped);
                image.set_voxel_size(C3DFVector(ni.dx,  ni.dy, ni.dz)); 
                image.set_origin(C3DFVector(ni.qoffset_x, ni.qoffset_y, ni.qoffset_z)); 
                double qa = sqrt(1.0 - (ni.quatern_b * ni.quatern_b + 
                                        ni.quatern_c * ni.quatern_c + 
                                        ni.quatern_d * ni.quatern_d)); 
                image.set_rotation(Quaternion(qa, ni.quatern_b, ni.quatern_c, ni.quatern_d));
        }
        
        if (ni.sform_code > 0) { // method 3
                vector<float> am = {ni.sto_xyz.m[0][0], ni.sto_xyz.m[0][1], ni.sto_xyz.m[0][2], ni.sto_xyz.m[0][3], 
                                     ni.sto_xyz.m[1][0], ni.sto_xyz.m[1][1], ni.sto_xyz.m[1][2], ni.sto_xyz.m[1][3],
                                     ni.sto_xyz.m[2][0], ni.sto_xyz.m[2][1], ni.sto_xyz.m[2][2], ni.sto_xyz.m[2][3],
                                     ni.sto_xyz.m[3][0], ni.sto_xyz.m[3][1], ni.sto_xyz.m[3][2], ni.sto_xyz.m[3][3]}; 
                
                image.set_attribute(AttrID_nifti_sform, am);
                image.set_attribute(AttrID_nifti_sform_code, ni.sform_code); 
        }
        

	image.set_attribute(AttrID_nifti_toffset, ni.toffset);
	image.set_attribute(AttrID_nifti_xyz_units, ni.xyz_units);
	image.set_attribute(AttrID_nifti_time_units, ni.time_units);
	image.set_attribute(AttrID_nifti_intent_code, ni.intent_code);
	image.set_attribute(AttrID_nifti_intent_p1, ni.intent_p1);
	image.set_attribute(AttrID_nifti_intent_p2, ni.intent_p2);
	image.set_attribute(AttrID_nifti_intent_p3, ni.intent_p3);
	image.set_attribute(AttrID_nifti_intent_name, string(ni.intent_name)); 
}


template <typename Image> 
CNifti3DImageIOPlugin::PData read_images(const C3DBounds& size, const nifti_image& ni)
{
        int timesteps = 1; 
	if (ni.ndim == 4) 
		timesteps = ni.nt; 

	CNifti3DImageIOPlugin::PData result(new C3DImageVector); 
	result->reserve(timesteps); 
	
	typedef typename Image::value_type TInPixel;
	const TInPixel *in_data = reinterpret_cast<const TInPixel *>(ni.data);
	
	size_t stride = size.product(); 
	for (int i = 0; i < timesteps; ++i, in_data += stride) {
		Image *img = new Image(size); 
                copy_attributes(*img, ni); 
		copy(in_data, in_data + img->size(), img->begin()); 
		result->push_back(P3DImage(img)); 
	}
	return result; 
        
}

template <typename OutImage, typename InPixel> 
CNifti3DImageIOPlugin::PData read_images(const C3DBounds& size, const nifti_image& ni)
{
        int timesteps = 1; 
	if (ni.ndim == 4) 
		timesteps = ni.nt; 

	CNifti3DImageIOPlugin::PData result(new C3DImageVector); 
	result->reserve(timesteps); 
	
	const InPixel *in_data = reinterpret_cast<const InPixel *>(ni.data);
	
	size_t stride = size.product(); 
        
        const double scale = ni.scl_slope; 
        const double shift = ni.scl_inter; 
        
	for (int i = 0; i < timesteps; ++i, in_data += stride) {
		auto *img = new OutImage(size); 
                copy_attributes(*img, ni); 
		transform(in_data, in_data + img->size(), img->begin(), 
                          [scale, shift](InPixel x){return scale * x + shift;}); 
		result->push_back(P3DImage(img)); 
	}
	return result; 
        
}

CNifti3DImageIOPlugin::PData CNifti3DImageIOPlugin::do_load(const std::string&  filename) const
{
        if (!is_nifti_file(filename.c_str()))
		return PData();

	unique_ptr<nifti_image, NiftiDeallocator> image(nifti_image_read(filename.c_str(), 1)); 
        
	if (!image) {
		cvdebug() << "File '"<<  filename << "' not a NIFTI file"; 
                return PData(); 
        }
 
        switch (image->intent_code) {
        case 0: // no specific intent, assume image data 
        case NIFTI_INTENT_DIMLESS: 
                break; 
        default: 
                throw create_exception<invalid_argument>("Found a NIFTI-1 file with intent ", 
                                                         image->intent_code, " that is not supported by MIA."); 
        }

	

	
        if (image->ndim < 3 || image->ndim > 4)
		throw create_exception<invalid_argument>("Nifti: 3D(+t) image expected but ", 
							 image->ndim, " dimensions available in '", filename); 
        
        C3DBounds size(image->nx, image->ny, image->nz);

        if (image->scl_slope != 0.0) { // data must be scaled
                switch (image->datatype) {
                case DT_UINT8:  return read_images<C3DFImage, unsigned char>(size, *image); 
                case DT_INT16:  return read_images<C3DFImage, signed short>(size, *image); 
                case DT_INT32:  return read_images<C3DDImage, signed int>(size, *image); 
                case DT_FLOAT32:return read_images<C3DFImage, float>(size,  *image); 
                case DT_FLOAT64:return read_images<C3DDImage, double>(size,  *image); 
                case DT_INT8:   return read_images<C3DFImage, signed char>(size, *image); 
                case DT_UINT16: return read_images<C3DFImage, unsigned short>(size, *image); 
                case DT_UINT32: return read_images<C3DDImage, unsigned int>(size, *image); 
#ifdef LONG_64BIT
                case DT_INT64:  return read_images<C3DDImage, signed long>(size, *image); 
                case DT_UINT64: return read_images<C3DDImage, unsigned short>(size, *image); 
#endif
                default:
                        throw create_exception<invalid_argument>("NIFTI: input format ", image->datatype, " not supported"); 
                }
        } else {  // the true pixel values are stored
                switch (image->datatype) {
                case DT_UINT8:  return read_images<C3DUBImage>(size, *image); 
                case DT_INT16:  return read_images<C3DSSImage>(size, *image); 
                case DT_INT32:  return read_images<C3DSIImage>(size, *image); 
                case DT_FLOAT32:return read_images<C3DFImage>(size,  *image); 
                case DT_FLOAT64:return read_images<C3DDImage>(size,  *image); 
                case DT_INT8:   return read_images<C3DSBImage>(size, *image); 
                case DT_UINT16: return read_images<C3DUSImage>(size, *image); 
                case DT_UINT32: return read_images<C3DUIImage>(size, *image); 
#ifdef LONG_64BIT
                case DT_INT64:  return read_images<C3DSLImage>(size, *image); 
                case DT_UINT64: return read_images<C3DULImage>(size, *image); 
#endif
                default:
                        throw create_exception<invalid_argument>("NIFTI: input format ", image->datatype, " not supported");
                }
        }
	
}

static int datatype_to_nifti(EPixelType ptype)
{
        switch (ptype) {
        case it_sbyte: return DT_INT8; 
        case it_ubyte: return DT_UINT8; 
        case it_sshort: return DT_INT16; 
        case it_ushort: return DT_UINT16; 
        case it_sint: return DT_INT32; 
        case it_uint: return DT_UINT32; 
#ifdef LONG_64BIT
        case it_slong: return DT_INT64; 
        case it_ulong: return DT_UINT64; 
#endif
        case it_float: return DT_FLOAT32; 
        case it_double: return DT_FLOAT64; 
        default: 
                throw create_exception<invalid_argument>("NIFTI-1 ImageIO: Trying to save pixel type '", 
                                                         CPixelTypeDict.get_name(ptype), 
                                                         "' that is not supported by nifti"); 
        }
}


class FCopyImageDataToNifti : public TFilter <void> {
public:
        FCopyImageDataToNifti(nifti_image& ni, EPixelType pt):
                m_ni(ni),
                m_pt(pt),
                m_npixels_written(0)
        {
        }

        template <typename T>
        void operator () (const T3DImage<T>& image)
        {
                // sanity type check for multi-images
                if (m_pt != image.get_pixel_type()){
                        throw create_exception<invalid_argument>("NIFTI-IO: image series containing images of varying pixel types is not supported");
                }

                T *out_data = reinterpret_cast<T *>(m_ni.data);
                copy(image.begin(), image.end(), out_data + m_npixels_written);
                m_npixels_written += image.size();
        }
private:
        nifti_image& m_ni;
        EPixelType m_pt;
        long m_npixels_written;
};



bool CNifti3DImageIOPlugin::do_save(const std::string& fname, const Data& data) const
{
        assert(!data.empty()); 
        
	const C3DImage& image = *data[0]; 
        int dims[8] = {3,1,1,1,1,1,1,1};

        C3DBounds size = image.get_size();
        dims[1] = size.x; 
        dims[2] = size.y; 
        dims[3] = size.z; 
        auto pixel_type = image.get_pixel_type(); 
        int datatype = datatype_to_nifti(pixel_type);
        
        if (data.size() > 1) {
                dims[0] = 4; 
                dims[4] = data.size(); 
        }
	
        // create the output image

        unique_ptr<nifti_image, NiftiDeallocator> output(nifti_make_new_nim(dims, datatype, 1));
	if (!output) {
		throw create_exception<runtime_error>("NIFTI-imageio: Unable to create output image of size [", 
						      size, "]x[", data.size(), "]"); 
	}
	
	output->pixdim[0] = dims[0]; 
	auto scale = image.get_voxel_size(); 
	output->dx = output->pixdim[1] = scale.x;
	output->dy = output->pixdim[2] = scale.y;
	output->dz = output->pixdim[3] = scale.z;
	
	// it would make sense to add this next one as an attribute 
	output->dt = output->pixdim[4] = 1;

	// not supported 
	output->du = output->pixdim[5] = 1;
	output->dv = output->pixdim[6] = 1;
	output->dw = output->pixdim[7] = 1;
	
	// we don't do scaling 
	output->scl_slope = 0.0f;
	output->scl_inter = 0.0f;

	// need to see whether to support this 
	output->cal_min = 0.0f;
	output->cal_max = 0.0f ;

	if (image.has_attribute(AttrID_nifti_qform_code)) {
		output->qform_code = image.get_attribute_as<int>(AttrID_nifti_qform_code); 
	} else {
		output->qform_code = 0; // default to method 2 
	}

	// Analyze 7.5 like data 
	if (output->qform_code == 0) {
		// here starts the orientation insanity 
		// todo: re-check how this qfac is actually used

		auto org = image.get_origin(); 
		output->qoffset_x = org.x; 
		output->qoffset_y = org.y; 
		output->qoffset_z = org.z; 
		output->quatern_b = 0.0; 
		output->quatern_c = 0.0; 
		output->quatern_d = 0.0;
		
		switch (image.get_orientation()) {
		case ior_undefined:
		case ior_xyz: break; 
		default: 
			cvwarn() << __FUNCTION__ << "FIXME:manual set orientation detected, but ignored\n"; 
		}
	} else {
		output->qfac = (image.get_orientation() == ior_xyz_flipped) ? -1 : 1; 
		auto org = image.get_origin(); 
		output->qoffset_x = org.x; 
		output->qoffset_y = org.y; 
		output->qoffset_z = org.z; 
		auto rot = image.get_rotation().as_quaternion(); 
		output->quatern_b = rot.x(); 
		output->quatern_c = rot.y(); 
		output->quatern_d = rot.z();
	}
	
	// copy s-form if available 
	if (image.has_attribute(AttrID_nifti_sform_code)) {
		output->sform_code = image.get_attribute_as<int>(AttrID_nifti_sform_code); 
		auto am = image.get_attribute_as<vector<float>>(AttrID_nifti_sform);
		output->sto_xyz.m[0][0] = am[0]; 
		output->sto_xyz.m[0][1] = am[1]; 
		output->sto_xyz.m[0][2] = am[2]; 
		output->sto_xyz.m[0][3] = am[3]; 

		output->sto_xyz.m[1][0] = am[4]; 
		output->sto_xyz.m[1][1] = am[5]; 
		output->sto_xyz.m[1][2] = am[6]; 
		output->sto_xyz.m[1][3] = am[7]; 

		output->sto_xyz.m[1][0] = am[8]; 
		output->sto_xyz.m[1][1] = am[9]; 
		output->sto_xyz.m[1][2] = am[10]; 
		output->sto_xyz.m[1][3] = am[11]; 

		output->sto_xyz.m[1][0] = am[12]; 
		output->sto_xyz.m[1][1] = am[13]; 
		output->sto_xyz.m[1][2] = am[14]; 
		output->sto_xyz.m[1][3] = am[15]; 

	
	}
	output->toffset = image.get_attribute_as<float>(AttrID_nifti_toffset, 0.0f);
	output->xyz_units  = image.get_attribute_as<int>(AttrID_nifti_xyz_units, NIFTI_UNITS_MM);
	output->time_units = image.get_attribute_as<int>(AttrID_nifti_time_units, NIFTI_UNITS_SEC);

	output->intent_code = image.get_attribute_as<int>(AttrID_nifti_intent_code, 0);
	output->intent_p1 = image.get_attribute_as<float>(AttrID_nifti_intent_p1, 0);
	output->intent_p2 = image.get_attribute_as<float>(AttrID_nifti_intent_p2, 0);
	output->intent_p3 = image.get_attribute_as<float>(AttrID_nifti_intent_p3, 0);
	string intent_name = image.get_attribute_as<string>(AttrID_nifti_intent_name, ""); 
	if (!intent_name.empty()) {
		strncpy(output->intent_name, intent_name.c_str(),  14);
	}
	output->num_ext = 0; 

        // set the data
        FCopyImageDataToNifti nsave(*output, image.get_pixel_type());

        for (auto i = data.begin(); i!= data.end(); ++i){
                ::mia::accumulate(nsave, **i);

        }

	// it could be that this needs the file name without extension 
        if (nifti_set_filenames(output.get(), fname.c_str(), 1, 0) != 0) {
		throw create_exception<runtime_error>("NIFTI-imageio: Something went wrong with setting the filename to '", 
						      fname, "'"); 
	}
	auto resultfile = nifti_image_write_hdr_img(output.get(), 1, "wb"); 
        return resultfile ? false : true;
	
}

const std::string CNifti3DImageIOPlugin::do_get_descr() const
{
        return "NIFTI-1 3D image IO"; 
}

const std::string CNifti3DImageIOPlugin::do_get_preferred_suffix() const
{
        return "nii"; 
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new CNifti3DImageIOPlugin();
}

NS_END
