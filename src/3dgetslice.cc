/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2017 Gert Wollny
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

#include <iostream>
#include <string>
#include <stdexcept>
#include <sstream>
#include <iomanip>


#include <mia/3d/imageio.hh>
#include <mia/2d/imageio.hh>
#include <mia/core.hh>

using namespace mia;

const SProgramDescription g_description = {
        {pdi_group, "Image conversion"}, 
	{pdi_short, "Extract slices from a 3D image."}, 
	{pdi_description, "This program is used to extract 2D slices from a 3D data set and store them "
	 "in separate files. Output files will be numbered according to their slice index."}, 
	{pdi_example_descr, "Store 5 coronal slices starting at slice 120 coronalXXXX.png from  image.v."}, 
	{pdi_example_code, "-i image.v -o coronal -t png -n 5 -s 120"}
}; 

enum EDirection {dir_unkown, dir_xy, dir_xz, dir_yz};

const TDictMap<EDirection>::Table GDirectionmapTable[] = {
	{"xy", dir_xy, "get xy (axial) slice"},
	{"axial", dir_xy, "get xy (axial) slice"},
	{"xz", dir_xz, "get xz (coronal) slice"},
	{"coronal", dir_xz, "get xz (coronal) slice"},
	{"yz", dir_yz, "get yz (saggital) slice"},
	{"saggital", dir_yz, "get xz (saggital) slice"},
	{NULL, dir_unkown, ""}
};

const TDictMap<EDirection> GDirectionmap(GDirectionmapTable);

using namespace std;
NS_MIA_USE;

template <typename T, EDirection s_dir>
struct __dispatch {
};

template <typename T>
struct __dispatch<T, dir_xy> {
	static size_t get_end(size_t start, size_t n, const C3DBounds& size) {
		size_t end = n > 0 ? start + n : start + size.z;
		return ( end < size.z ) ? end : size.z;
	}
	static T2DImage<T> get_slice(size_t i, const T3DImage<T>& image) {
		return image.get_data_plane_xy(i);
	}
};

template <typename T>
struct __dispatch<T, dir_xz> {
	static size_t get_end(size_t start, size_t n, const C3DBounds& size) {
		size_t end = n > 0 ? start + n : start + size.y;
		return ( end < size.y ) ? end : size.y;
	}
	static T2DImage<T> get_slice(size_t i, const T3DImage<T>& image) {
		return image.get_data_plane_xz(i);
	}
};

template <typename T>
struct __dispatch<T, dir_yz> {
	static size_t get_end(size_t start, size_t n, const C3DBounds& size) {
		size_t end = n > 0 ? start + n : start + size.x;
		return ( end < size.x ) ? end : size.x;
	}
	static T2DImage<T> get_slice(size_t i, const T3DImage<T>& image) {
		return image.get_data_plane_yz(i);
	}
};


template <EDirection s_dir>
class TGetter : public TFilter<bool> {
public:
	TGetter(size_t start, size_t n, const string& fname, const string& type, int digits):
		m_start(start),
		m_n(n),
		m_fname(fname),
		m_type(type), 
		m_digits(digits)
	{
	}

	template <typename T>
	bool operator ()(const T3DImage<T>& image) const
	{
		size_t end  = __dispatch<T, s_dir>::get_end(m_start, m_n, image.get_size());

		bool retval = true;
		for(size_t i = m_start; i < end; ++i) {
			P2DImage pimage(new  T2DImage<T>(__dispatch<T, s_dir>::get_slice(i, image)));
			if (m_n != 1) {
				stringstream out_name;
				out_name << m_fname << setw(m_digits) << setfill('0') << i << "." << m_type;
				retval &= save_image(out_name.str(), pimage);
			}else{
				retval &= save_image(m_fname, pimage);
			}
		}
		return retval;
	}
private:
	size_t m_start;
	size_t m_n;
	string m_fname;
	string m_type;
	int m_digits; 
};

int do_main( int argc, char *argv[] )
{

	string in_filename;
	string out_filename;
	string out_type("png");
	size_t start_slice = 0;
	size_t slice_number = 1;
	EDirection direction = dir_xy;
	int digits = 4; 

	const auto& imageio2d = C2DImageIOPluginHandler::instance();

	CCmdOptionList options(g_description);
	options.add(make_opt( in_filename, "in-file", 'i', 
			      "input image(s) to be filtered", 
			      CCmdOptionFlags::required_input, &C3DImageIOPluginHandler::instance()));
	options.add(make_opt( out_filename, "out-file", 'o', "output image(s). If number != 1 than this is used as a base name "
			      "and should be given without extension since this will be based on the '--type' option. "
			      "If number=1 then this exact file name will be used.", 
			      CCmdOptionFlags::required_output, &imageio2d));
	options.add(make_opt( out_type, imageio2d.get_set(), "type", 't', "output file type for number != 1"));
	options.add(make_opt( start_slice, "start", 's',"start slice number"));
	options.add(make_opt( slice_number, "number", 'n', "number of slices (all=0)"));
	options.add(make_opt( digits, "ndigits", 0, "minimum number of digits of the file name numbers (if n != 1)"));

	options.add(make_opt( direction, GDirectionmap, "dir", 'd', 
			      "slice direction (xy=axial, xz=coronal, yz=saggital)"));

	if (options.parse(argc, argv) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 

	string out_suffix = imageio2d.get_preferred_suffix(out_type); 

	// read image
	auto in_image = load_image3d(in_filename);


	bool result = false;
	if (in_image) {
		switch (direction) {
		case dir_xy:
			result = mia::filter(TGetter<dir_xy>(start_slice, slice_number, 
							     out_filename, out_suffix, digits), 
					     *in_image);
			break;
		case dir_xz:
			result = mia::filter(TGetter<dir_xz>(start_slice, slice_number, 
							     out_filename, out_suffix, digits), 
					     *in_image);
			break;
		case dir_yz:
			result = mia::filter(TGetter<dir_yz>(start_slice, slice_number, 
							     out_filename, out_suffix, digits), 
					     *in_image);
			break;
		default:
			assert(!"impossible slice direction");
			throw invalid_argument( "impossible slice direction");
		}
	}

	return result ? EXIT_SUCCESS : EXIT_FAILURE;

}

#include <mia/internal/main.hh>
MIA_MAIN(do_main)
