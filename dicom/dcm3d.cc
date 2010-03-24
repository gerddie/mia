/* -*- mia-c++  -*-
 *
 * Copyright (c) 2007 Gert Wollny <gert dot wollny at acm dot org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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

#include <dicom/dcm3d.hh>
#include <dicom/dicom4mia.hh>

#include <sstream>
#include <iomanip>
#include <map>
#include <queue>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>

#include <mia/core/errormacro.hh>
#include <mia/core/file.hh>
#include <mia/core/filter.hh>
#include <mia/core/msgstream.hh>

NS_BEGIN(dicom_3dimageio)

NS_MIA_USE;

using namespace std; 
using namespace boost; 
namespace bfs=boost::filesystem; 

CDicom3DImageIOPlugin::CDicom3DImageIOPlugin():
	C3DImageIOPlugin("dicom")
{
	add_supported_type(it_ushort);

	TTranslator<float>::register_for("SliceLocation"); 
	TTranslator<int>::register_for("SeriesNumber"); 
	TTranslator<int>::register_for("AcquisitionNumber"); 
	TTranslator<int>::register_for("InstanceNumber"); 
}

void CDicom3DImageIOPlugin::do_add_suffixes(multimap<string, string>& map) const
{
	map.insert(pair<string,string>(".dcm", get_name())); 
	map.insert(pair<string,string>(".DCM", get_name())); 
}


struct attr_less {
	bool operator()(const PAttribute& a, const PAttribute& b) {
		return a->is_less(*b); 
	}
}; 

struct image_instance_less {
	bool operator()(const P2DImage& a, const P2DImage& b) {
		return !a->get_attribute(IDInstanceNumber)->is_less(*b->get_attribute(IDInstanceNumber)); 
	}
}; 

typedef priority_queue<P2DImage, vector<P2DImage>, image_instance_less> CImageInstances; 
typedef map<PAttribute, CImageInstances, attr_less> CImageSeries; 
typedef map<PAttribute, CImageSeries, attr_less> CAquisitions; 

struct C3DImageCreator: public TFilter<bool> {
	C3DImageCreator(size_t nz): _M_nz(nz), 
				    _M_delta_z(0.0) {
	}; 
	
	template <typename T> 
	bool operator() ( const T2DImage<T>& image); 
	
	P3DImage get_image() const; 
private: 
	P3DImage _M_result; 
	size_t _M_nz; 
	size_t _M_z; 
	float _M_slice_pos; 
	float _M_delta_z; 
	C2DBounds _M_size2d; 
	C2DFVector _M_pixel_size; 
}; 

template <typename T> 
bool C3DImageCreator::operator() ( const T2DImage<T>& image)
{
	T3DImage<T> *target = NULL; 
	if (!_M_result) {
		_M_size2d = image.get_size();
		target = new T3DImage<T>(C3DBounds(_M_size2d.x, _M_size2d.y, _M_nz), image); 
		_M_result.reset(target); 
		_M_z = 0; 
		_M_pixel_size = image.get_pixel_size(); 
		_M_slice_pos = image.get_attribute_as<float>(IDSliceLocation); 
		_M_result->delete_attribute("pixel");
	}else {
		target = dynamic_cast<T3DImage<T> *>(_M_result.get()); 
		if (!target) {
			THROW(invalid_argument, "Series input images have different pixel type"); 
		}
		if (_M_size2d != image.get_size()) {
			THROW(invalid_argument, "Series input images have different slice size");
		}
		float new_slice_pos = image.get_attribute_as<float>(IDSliceLocation); 
		_M_delta_z = new_slice_pos - _M_slice_pos; 
		_M_slice_pos = new_slice_pos; 
	}
	assert(_M_z < _M_nz); 
	target->put_data_plane_xy(_M_z, image); 
	++_M_z; 
	return true; 
}

P3DImage C3DImageCreator::get_image() const
{
	_M_result->set_voxel_size(C3DFVector(_M_pixel_size.x, _M_pixel_size.y, _M_delta_z)); 
	_M_result->delete_attribute(IDSliceLocation); 
	_M_result->delete_attribute(IDInstanceNumber); 

	return _M_result; 
}

static P3DImage get_3dimage(CImageInstances& slices)
{
	TRACE_FUNCTION; 
	cvdebug() << "get_3dimage: combine " << slices.size() << " slices\n"; 
	C3DImageCreator creator(slices.size()); 
	while (!slices.empty()) {
		P2DImage slice = slices.top(); 
		slices.pop(); 
		mia::accumulate(creator, *slice); 
	}
	

	return creator.get_image(); 
}

C3DImageIOPlugin::PData CDicom3DImageIOPlugin::get_images(const vector<P2DImage>& candidates) const
{
	TRACE_FUNCTION; 
	assert(!candidates.empty()); 
	
	PData result(new Data); 

	CAquisitions acc; 
	
	// read all the images into a map
	for(vector<P2DImage>::const_iterator i =  candidates.begin(); 
	    i != candidates.end(); ++i) {
		if ( (*i)->has_attribute(IDAcquisitionNumber) && 
		     (*i)->has_attribute(IDInstanceNumber) && 
		     (*i)->has_attribute(IDSeriesNumber)) {
			acc[(*i)->get_attribute(IDAcquisitionNumber)]
				[(*i)->get_attribute(IDSeriesNumber)].push(*i); 
		}
	}
	
	for (CAquisitions::iterator a = acc.begin(); a != acc.end(); ++a) {
		for (CImageSeries::iterator s = a->second.begin(); s != a->second.end(); ++s) {
			P3DImage image = get_3dimage(s->second); 
			if (image)
				result->push_back(image); 
		}
	}
	
	return result; 
}

static void add_images(const string& fname, const string& study_id, vector<P2DImage>& candidates)
{
	TRACE_FUNCTION; 
	bfs::path dir(fname); 
	string ext = dir.extension(); 
	dir.remove_filename(); 
	
	if (dir.filename().empty())
		dir=bfs::path("."); 

	cvdebug() << "Search directory '" << dir << "' for extension '" << ext << "'\n"; 
	
	if (!bfs::exists(dir) || !bfs::is_directory(dir))
		return ; 
	
	stringstream pattern; 
	pattern << ".*\\" << ext; 
	boost::regex pat_expr(pattern.str());	

	bfs::directory_iterator di(dir); 
	bfs::directory_iterator dend;
	while (di != dend) {
		if (boost::regex_match(di->path().filename(), pat_expr) && 
			di->path().filename() != fname) {
			bfs::path f =  di->path();
			cvdebug() << "read file '" << f << "'\n"; 
			CDicomReader reader(f.directory_string().c_str()); 
			if (reader.good() && reader.get_attribute(IDStudyID, true) == study_id) 
				candidates.push_back(reader.get_image()); 
		}
		++di; 
	}
}

C3DImageIOPlugin::PData CDicom3DImageIOPlugin::do_load(const string& fname) const
{
	TRACE_FUNCTION; 
	PData result; 

	CDicomReader reader(fname.c_str()); 
	if (!reader.good())
		return result; 

	vector<P2DImage> candidates; 
	P2DImage prototype = reader.get_image(); 
	candidates.push_back(prototype); 
	
	string study_id = prototype->get_attribute(IDStudyID)->as_string(); 
	// now read all the slices in the folder that have the same study id
	
	add_images(fname, study_id, candidates); 
	

	result = get_images(candidates);
	return result; 
}

struct CSliceSaver: public TFilter<bool> 
{
	CSliceSaver(const string& fname); 
	
	template <typename T> 
	bool operator () ( const T3DImage<T>& image) const; 

	void set_instance(size_t series, size_t slice); 

	string _M_fnamebase; 
	string _M_extension; 
	size_t _M_series; 
	size_t _M_slice; 
	mutable float _M_location; 
}; 

CSliceSaver::CSliceSaver(const string& fname):
	_M_location(0)
{
	// filename split the 
	bfs::path fullname(fname); 
	_M_extension = fullname.extension(); 
	_M_fnamebase = fullname.stem(); 
}

void CSliceSaver::set_instance(size_t series, size_t slice)
{
	_M_series = series; 
	_M_slice = slice; 
}

template <typename T> 
bool CSliceSaver::operator () ( const T3DImage<T>& image) const
{
	assert(_M_slice < image.get_size().z); 
	stringstream fname; 
	fname << _M_fnamebase << setw(4) << setfill('0') << _M_series + 1 << "_" <<
		setw(4) << setfill('0') << _M_slice + 1 << _M_extension; 

	C3DFVector voxel = image.get_voxel_size(); 
	C2DFVector pixel(voxel.x, voxel.y); 
	

	T2DImage<T> slice = image.get_data_plane_xy(_M_slice); 
	slice.set_pixel_size(pixel); 
	
	slice.set_attribute(IDInstanceNumber, PAttribute(new CIntAttribute(_M_slice + 1)));
	slice.set_attribute(IDSliceLocation,  PAttribute(new CFloatAttribute(_M_location)));
	_M_location += voxel.z; 

	if (!slice.has_attribute(IDSeriesNumber))
		slice.set_attribute(IDSeriesNumber, PAttribute(new CIntAttribute(_M_series + 1)));
	
	CDicomWriter weiter(slice);
	return weiter.write(fname.str().c_str());
}

bool CDicom3DImageIOPlugin::do_save(const string& fname, const Data& data) const
{
	if (data.empty())
		THROW(runtime_error, "CDicom3DImageIOPlugin: '" << fname 
		      << "', no images to save"); 

	
	CSliceSaver saver(fname); 
	
	bool sucess = true; 
	size_t series = 0; 
	for (Data::const_iterator i = data.begin(); i != data.end() && sucess; ++i, ++series){
		for (size_t slice = 0; slice < (*i)->get_size().z && sucess; ++slice) {
			saver.set_instance(series, slice); 
			sucess = mia::filter(saver, **i); 
		}
	}
	return sucess; 
}

const string CDicom3DImageIOPlugin::do_get_descr() const
{
	return "3D image io for DICOM"; 
}


extern "C" EXPORT  CPluginBase *get_plugin_interface() 
{
	return new CDicom3DImageIOPlugin();
}

NS_END
