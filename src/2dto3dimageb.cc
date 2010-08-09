/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
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

#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <stdexcept>

#include <mia/core.hh>
#include <mia/2d/2dfilter.hh>
#include <mia/2d/2dimageio.hh>
#include <mia/3d/3dimageio.hh>

static const char *program_info = 
	"This program is used to combine a series of 2D images of equal \n"
	"size and type images to a 3D image.\n"
	"Basic usage:\n"
	"  mia-2dto3dimageb  -o <output image> <slice1> <slice2> ...\n"; 


NS_MIA_USE
using namespace std;
using namespace boost;

struct C3DImageCollector : public TFilter<bool> {

	C3DImageCollector(size_t slices):
		_M_slices(slices),
		_M_cur_slice(0)
	{
	}

	template <typename T>
	bool operator ()(const T2DImage<T>& image) {

		if (_M_image == 0)
			_M_image = SHARED_PTR(C3DImage) (new T3DImage<T>(C3DBounds(image.get_size().x,
										  image.get_size().y,
										  _M_slices)));

		T3DImage<T> *out_image = dynamic_cast<T3DImage<T> *>(_M_image.get());
		if (!out_image)
			throw invalid_argument("input images are not all of the same type");

		if (_M_cur_slice < _M_slices) {
			if (out_image->get_size().x != image.get_size().x ||
			    out_image->get_size().y != image.get_size().y)
				throw invalid_argument("input images are not all of the same size");

			typename T3DImage<T>::iterator out = out_image->begin() +
				image.get_size().x * image.get_size().y * _M_cur_slice;

			copy(image.begin(), image.end(), out);
		}
		++ _M_cur_slice;
		return true;
	}

	SHARED_PTR(C3DImage) result() const {
		return _M_image;
	}

private:
	size_t _M_slices;
	size_t _M_cur_slice;

	SHARED_PTR(C3DImage) _M_image;
};

/* Revision string */
const char revision[] = "not specified";

int main( int argc, const char *argv[] )
{

	string in_filename;
	string out_filename("3");
	string out_type;

	const C2DImageIOPluginHandler::Instance& image2dio = C2DImageIOPluginHandler::instance();
	const C3DImageIOPluginHandler::Instance& image3dio = C3DImageIOPluginHandler::instance();

	CCmdOptionList options(program_info);
	options.push_back(make_opt( out_filename, "out-file", 'o', "output file name", "3d", true));
	options.push_back(make_opt( out_type, image3dio.get_set(), "type", 't',"output file type" , "filetype"));

	try {

		options.parse(argc, argv);

		if (options.get_remaining().empty())
			throw runtime_error("no slices given ...");


		CHistory::instance().append(argv[0], revision, options);

		//size_t start_filenum = 0;
		//size_t end_filenum  = 0;
		//size_t format_width = 0;

		char new_line = cverb.show_debug() ? '\n' : '\r';

		vector<const char *> input_images = options.get_remaining();

		C3DImageCollector ic(input_images.size());

		for (vector<const char *>::const_iterator  i = input_images.begin(); i != input_images.end(); ++i) {

			cvmsg() << "Load " << *i << new_line;
			C2DImageIOPluginHandler::Instance::PData  in_image_list = image2dio.load(*i);

			if (in_image_list.get() && in_image_list->size()) {
				accumulate(ic, **in_image_list->begin());
			}
		}
		cvmsg() << "\n";

		C3DImageVector result;
		result.push_back(ic.result());

		if (image3dio.save(out_type, out_filename, result))
			return EXIT_SUCCESS;
		else
			cerr << argv[0] << " fatal: unable to output image to " <<  out_filename << endl;
	}
	catch (const runtime_error &e){
		cerr << argv[0] << " runtime: " << e.what() << endl;
	}
	catch (const invalid_argument &e){
		cerr << argv[0] << " error: " << e.what() << endl;
	}
	catch (const std::exception& e){
		cerr << argv[0] << " error: " << e.what() << endl;
	}
	catch (...){
		cerr << argv[0] << " unknown exception" << endl;
	}

	return EXIT_FAILURE;
}
