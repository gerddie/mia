
/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
 *
 * Max-Planck-Institute for Human Cognitive and Brain Science
 * Max-Planck-Institute for Evolutionary Anthropology
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PUcRPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <stdexcept>
#include <boost/filesystem.hpp>
#include <mia/core/errormacro.hh>
#include <mia/2d/SegSetWithImages.hh>
#include <mia/2d/2dimageio.hh>
#include <mia/2d/2dfilter.hh>



namespace bfs=boost::filesystem;

NS_MIA_BEGIN
using namespace std;

CSegSetWithImages::CSegSetWithImages()
{
}

CSegSetWithImages::CSegSetWithImages(const string& filename, bool ignore_path):
	CSegSet(filename)
{
	string src_path;
	if (ignore_path) {
		bfs::path src_path_(filename);
		src_path_.remove_filename();
		src_path = src_path_.directory_string();
		cvdebug() << "Segmentation path" << src_path << "\n";
	}


	CSegSet::Frames::iterator iframe = get_frames().begin();
	CSegSet::Frames::iterator eframe = get_frames().end();

	while (iframe != eframe) {
		string input_image = iframe->get_imagename();
		string iimage = bfs::path(input_image).filename();
		if (ignore_path) {
			input_image = (src_path / bfs::path(iimage) ).directory_string();
			iframe->set_imagename(iimage);
		}
		_M_images.push_back(load_image2d(input_image));
		++iframe;
	}
}

// sets the image series 
void CSegSetWithImages::set_images(const C2DImageSeries& series)
{
	if (series.size() != get_frames().size()) 
		THROW(invalid_argument, "image set and number of segmentation frames must have same number of images"); 
	_M_images = series; 
}

const C2DImageSeries& CSegSetWithImages::get_images()const
{
	return _M_images;
}

struct 	CSegFrameCropper {
	CSegFrameCropper(const C2DIVector& shift,
			 C2DFilterPlugin::ProductPtr filter,
			 const string& image_name);

	CSegFrame operator()(const CSegFrame& frame, const C2DImage& image) const;

private:
	C2DIVector _M_shift;
	C2DFilterPlugin::ProductPtr _M_filter;
	bfs::path _M_image_outpath;
};


CSegSetWithImages CSegSetWithImages::crop(const C2DIVector&  start, const C2DIVector&  end,
	const string& crop_filename_base)
{
	CSegSetWithImages result;

	stringstream mask_lv;
	mask_lv << "crop:start=[" << start
		<< "],end=[" << end << "]";
	cvinfo() << "crop region = '" << mask_lv.str() << "'\n";
	C2DFilterPlugin::ProductPtr image_cropper =
		C2DFilterPluginHandler::instance().produce(mask_lv.str().c_str());

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
				   C2DFilterPlugin::ProductPtr filter,
				   const string& image_name):
	_M_shift(shift),
	_M_filter(filter),
	_M_image_outpath(image_name)
{
	_M_image_outpath.remove_filename();

}


CSegFrame CSegFrameCropper::operator()(const CSegFrame& frame, const C2DImage& image) const
{
	P2DImage cropped = _M_filter->filter(image);
	const string out_filename = (_M_image_outpath.file_string() / bfs::path(frame.get_imagename())).file_string();

	if (!save_image2d(out_filename, cropped))
		cvwarn() << "Could not write cropped file '" << out_filename << "'\n";

	CSegFrame result = frame;
	result.shift(_M_shift, frame.get_imagename());
	return result;

}


NS_MIA_END
