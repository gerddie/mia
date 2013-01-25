/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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
#include <mia/core/bfsv23dispatch.hh>
#include <mia/2d/imageio.hh>

NS_BEGIN(IMAGEIO_3D_DICOM)

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
	add_suffix(".dcm");
	add_suffix(".DCM");

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
	C3DImageCreator(size_t nz): m_nz(nz),
				    m_delta_z(0.0) {
	};

	template <typename T>
	bool operator() ( const T2DImage<T>& image);

	P3DImage get_image() const;
private:
	P3DImage m_result;
	size_t m_nz;
	size_t m_z;
	float m_slice_pos;
	float m_delta_z;
	C2DBounds m_size2d;
	C2DFVector m_pixel_size;
};

template <typename T>
bool C3DImageCreator::operator() ( const T2DImage<T>& image)
{
	T3DImage<T> *target = NULL;
	if (!m_result) {
		m_size2d = image.get_size();
		target = new T3DImage<T>(C3DBounds(m_size2d.x, m_size2d.y, m_nz), image);
		m_result.reset(target);
		m_z = 0;
		m_pixel_size = image.get_pixel_size();
		m_slice_pos = image.template get_attribute_as<float>(IDSliceLocation);
		m_result->delete_attribute("pixel");
	}else {
		target = dynamic_cast<T3DImage<T> *>(m_result.get());
		if (!target) {
			throw invalid_argument("Series input images have different pixel type");
		}
		if (m_size2d != image.get_size()) {
			throw invalid_argument("Series input images have different slice size");
		}
		float new_slice_pos = image.template get_attribute_as<float>(IDSliceLocation);
		m_delta_z = new_slice_pos - m_slice_pos;
		m_slice_pos = new_slice_pos;
	}
	assert(m_z < m_nz);
	target->put_data_plane_xy(m_delta_z < 0 ? m_nz - 1 - m_z: m_z, image);
	++m_z;
	return true;
}

P3DImage C3DImageCreator::get_image() const
{
	m_result->set_voxel_size(C3DFVector(m_pixel_size.x, m_pixel_size.y, 
					    m_delta_z > 0 ? m_delta_z : - m_delta_z));
	m_result->delete_attribute(IDSliceLocation);
	m_result->delete_attribute(IDInstanceNumber);

	return m_result;
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

std::string CDicom3DImageIOPlugin::do_get_preferred_suffix() const
{
	return "dcm"; 
}


C3DImageIOPlugin::PData CDicom3DImageIOPlugin::get_images(const vector<P2DImage>& candidates) const
{
	TRACE_FUNCTION;
	assert(!candidates.empty());

	PData result(new Data);

	CAquisitions acc;

	// read all the images into a map
	for(auto i =  candidates.begin();   i != candidates.end(); ++i) {
		if ( (*i)->has_attribute(IDAcquisitionNumber) &&
		     (*i)->has_attribute(IDInstanceNumber) &&
		     (*i)->has_attribute(IDSeriesNumber)) {
			acc[(*i)->get_attribute(IDAcquisitionNumber)]
				[(*i)->get_attribute(IDSeriesNumber)].push(*i);
		}else{
			cvwarn() << "Discard image because of no "
				 << ((*i)->has_attribute(IDAcquisitionNumber) ? "" : "aquisition") 
				 << ((*i)->has_attribute(IDInstanceNumber) ? "" : "instance") 
				 << ((*i)->has_attribute(IDSeriesNumber) ? "" : "series")
				 << " number\n"; 
				
		}
	}

	for (auto a = acc.begin(); a != acc.end(); ++a) {
		for (auto s = a->second.begin(); s != a->second.end(); ++s) {
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
	string ext = __bfs_get_extension(dir);
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
		if (boost::regex_match(__bfs_get_filename(di->path()), pat_expr) &&
		    __bfs_get_filename(di->path()) != fname) {
			bfs::path f =  di->path();
			cvdebug() << "read file '" << f << "'\n";
			CDicomReader reader(f.string().c_str());
			if (reader.good() && reader.get_attribute(IDStudyID, false) == study_id)
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

	// this is not very nice 
	string study_id;
	if (prototype->has_attribute(IDStudyID))
		study_id = prototype->get_attribute(IDStudyID)->as_string();
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

	string m_fnamebase;
	string m_extension;
	size_t m_series;
	size_t m_slice;
	mutable float m_location;
};

CSliceSaver::CSliceSaver(const string& fname):
	m_location(0)
{
	// filename split the
	bfs::path fullname(fname);
	m_extension = __bfs_get_extension(fullname);
	m_fnamebase = __bfs_get_stem(fullname);
}

void CSliceSaver::set_instance(size_t series, size_t slice)
{
	m_series = series;
	m_slice = slice;
}

template <typename T>
bool CSliceSaver::operator () ( const T3DImage<T>& image) const
{
	assert(m_slice < image.get_size().z);
	stringstream fname;
	fname << m_fnamebase << setw(4) << setfill('0') << m_series + 1 << "_" <<
		setw(4) << setfill('0') << m_slice + 1 << m_extension;

	C3DFVector voxel = image.get_voxel_size();
	C2DFVector pixel(voxel.x, voxel.y);


	T2DImage<T> slice = image.get_data_plane_xy(m_slice);
	slice.set_pixel_size(pixel);

	slice.set_attribute(IDInstanceNumber, PAttribute(new CIntAttribute(m_slice + 1)));
	slice.set_attribute(IDSliceLocation,  PAttribute(new CFloatAttribute(m_location)));
	m_location += voxel.z;

	if (!slice.has_attribute(IDSeriesNumber))
		slice.set_attribute(IDSeriesNumber, PAttribute(new CIntAttribute(m_series + 1)));

	CDicomWriter weiter(slice);
	return weiter.write(fname.str().c_str());
}

bool CDicom3DImageIOPlugin::do_save(const string& fname, const Data& data) const
{
	if (data.empty())
		throw create_exception<runtime_error>( "CDicom3DImageIOPlugin: '", fname, "', no images to save");


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
	return "Dicom image series as 3D";
}


extern "C" EXPORT  CPluginBase *get_plugin_interface()
{
	return new CDicom3DImageIOPlugin();
}

NS_END
