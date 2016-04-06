/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2016 Gert Wollny
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

#include <stdexcept>
#include <boost/filesystem.hpp>
#include <mia/core/errormacro.hh>
#include <mia/2d/segsetwithimages.hh>
#include <mia/2d/imageio.hh>
#include <mia/2d/filter.hh>

#include <mia/core/export_handler.hh>
#include <mia/core/ioplugin.cxx>
#include <mia/core/iohandler.cxx>


namespace bfs=boost::filesystem;

NS_MIA_BEGIN
using namespace std;

const char *CSegSetWithImages::data_descr = "2dmyocardsegset"; 

CSegSetWithImages::CSegSetWithImages()
{
}

CSegSetWithImages::CSegSetWithImages(int version):CSegSet(version)
{
}

CSegSetWithImages::CSegSetWithImages(const xmlpp::Document& node, const string& fileroot):
	CSegSet(node)
{
	
	auto iframe = get_frames().begin();
	auto eframe = get_frames().end();

	while (iframe != eframe) {
		string input_image = iframe->get_imagename();
		string iimage = bfs::path(input_image).string();
		input_image = (fileroot / bfs::path(iimage) ).string();
		iframe->set_imagename(iimage);
		
		P2DImage image = load_image2d(input_image); 
		m_images.push_back(image);
		iframe->set_image(image); 
		++iframe;
	}	
}

CSegSetWithImages::CSegSetWithImages(const string& filename, bool ignore_path):
	CSegSet(filename)
{
	string src_path;
	if (ignore_path) {
		bfs::path src_path_(filename);
		src_path_.remove_filename();
		src_path = src_path_.string();
		cvdebug() << "Segmentation path" << src_path << "\n";
	}


	auto iframe = get_frames().begin();
	auto eframe = get_frames().end();

	while (iframe != eframe) {
		string input_image = iframe->get_imagename();
		string iimage = bfs::path(input_image).string();
		if (ignore_path) {
			input_image = (src_path / bfs::path(iimage) ).string();
			iframe->set_imagename(iimage);
		}
		P2DImage image = load_image2d(input_image); 
		m_images.push_back(image);
		iframe->set_image(image); 
		++iframe;
	}
}

CSegSetWithImages *CSegSetWithImages::clone() const
{
	return new CSegSetWithImages(*this);
}

// sets the image series 
void CSegSetWithImages::set_images(const C2DImageSeries& series)
{
	if (series.size() != get_frames().size()) 
		throw create_exception<invalid_argument>("image set size (", series.size(), 
					       ") and number of segmentation frames ",
					       get_frames().size(), "must have same number of images"); 
	m_images = series; 
}

void CSegSetWithImages::save_images(const string& filename) const
{
	bfs::path src_path(filename);
	src_path.remove_filename();
	
	auto iframe = get_frames().begin();
	auto eframe = get_frames().end();
	auto iimage = m_images.begin();

	while (iframe != eframe) {
		string image_name = iframe->get_imagename();
		string filename = (image_name[0] == '/') ? 
			image_name : (src_path / bfs::path(image_name)).string(); 
                        
		if (!save_image(filename, *iimage))
			throw create_exception<runtime_error>("CSegSetWithImages:unable to save image to '",image_name, "'" ); 
		++iframe; 
		++iimage; 
	}
}

void CSegSetWithImages::add_frame(const CSegFrame& frame, P2DImage image)
{
	assert(	m_images.size() == get_frames().size()); 
	add_frame(frame); 
	m_images.push_back(image); 
}

const C2DImageSeries& CSegSetWithImages::get_images()const
{
	return m_images;
}

struct 	CSegFrameCropper {
	CSegFrameCropper(const C2DIVector& shift,
			 P2DFilter filter,
			 const string& image_name);

	CSegFrame operator()(const CSegFrame& frame, const C2DImage& image) const;

private:
	C2DIVector m_shift;
	P2DFilter m_filter;
	bfs::path m_image_outpath;
};


CSegSetWithImages CSegSetWithImages::crop(const C2DIVector&  start, const C2DIVector&  end,
	const string& crop_filename_base)
{
	CSegSetWithImages result;

	stringstream mask_lv;
	mask_lv << "crop:start=[" << start
		<< "],end=[" << end << "]";
	cvinfo() << "crop region = '" << mask_lv.str() << "'\n";
	auto image_cropper = C2DFilterPluginHandler::instance().produce(mask_lv.str().c_str());

	CSegFrameCropper frame_cropper(start, image_cropper, crop_filename_base);

	Frames::const_iterator iframe = get_frames().begin();
	Frames::const_iterator eframe = get_frames().end();
	C2DImageSeries::const_iterator iimages = get_images().begin();

	size_t i = 0;
	while (iframe != eframe) {
		cvinfo() << "Crop Frame " << i++ << "\n";

		result.add_frame(frame_cropper(*iframe, **iimages));
		++iframe;
		++iimages;
	}
	return result;
}


CSegFrameCropper::CSegFrameCropper(const C2DIVector& shift,
				   P2DFilter filter,
				   const string& image_name):
	m_shift(shift),
	m_filter(filter),
	m_image_outpath(image_name)
{
	m_image_outpath.remove_filename();

}


CSegFrame CSegFrameCropper::operator()(const CSegFrame& frame, const C2DImage& image) const
{
	P2DImage cropped = m_filter->filter(image);
	const string out_filename = (m_image_outpath.string() / bfs::path(frame.get_imagename())).string();

	if (!save_image(out_filename, cropped))
		cvwarn() << "Could not write cropped file '" << out_filename << "'\n";

	CSegFrame result = frame;
	result.shift(m_shift, frame.get_imagename());
	return result;

}

template <> const char *  const
TPluginHandler<CSegSetWithImagesIOPlugin>::m_help =  
	"Input/output of 2D image series with segmentations.";


template class TIOPlugin<CSegSetWithImages>;
template class TIOPluginHandler<CSegSetWithImagesIOPlugin>;
template class TPluginHandler<CSegSetWithImagesIOPlugin>;
template class THandlerSingleton< TIOPluginHandler<CSegSetWithImagesIOPlugin >>;



NS_MIA_END
