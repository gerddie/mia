/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2011 Gert Wollny
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

/*
  LatexBeginProgramSection{Image conversion}
  \label{sec:imageconversion}
  
  These programs are used to convert raw data to images. 

  LatexEnd
*/

/*
  LatexBeginProgramDescription{Image conversion}
  
  \subsection{mia-raw2volume}
  \label{mia-raw2volume}

  \begin{description}
  \item [Description:] This program is used to convert raw data to a 3D volume file format. 

  The program is called like 
  \
  \begin{lstlisting}
mia-2dimagefilter -i <input data> -o <output image> -s <size of input> ...
  \end{lstlisting}

  \item [Options:] $\:$

  \optiontable{
  \optinfile
  \optoutfile
  \opttypethreed
  \cmdopt{repn}{r}{string}{input pixel type (Table \ref{tab:pixform})}
  \cmdopt{big-endian}{b}{}{input data is big endian (default is low endian)}
  \cmdopt{scale}{f}{3D-vector <float>}{physical voxel size given as 3D vector <sx,sy,sz>}
  \cmdopt{skip}{k}{int}{skip the given number of bytes from the beginning of the input file}
  \cmdopt{size}{s}{3D-vector <int>}{size of the input data given as 3D vector <NX,NY,NZ>}
  \cmdopt{type}{t}{string}{override output file type deducted from the output file name}
  }

  \item [Example:]Conver a data set data.raw of size <10,20,30> of short integer data with a 
          pixel size of <1.2, 2.3, 3.4>     to an Analyze file image.hdr 
   \
  \begin{lstlisting}
mia-raw2volume -i data.raw -o image.hdr -s "<10,20,30>" -k "<1.2,2.3,3.4>" -r ushort 
  \end{lstlisting}
  \item [Remark:] The output file format must support the provided voxel format. 
  \end{description}
  
  LatexEnd
*/


#include <string>

#include <mia/core.hh>
#include <mia/3d.hh>

NS_MIA_USE;
using namespace std;
using namespace boost;

inline bool am_big_endian()
{
#ifdef WORDS_BIGENDIAN
	return true;
#else
	return false;
#endif
}

const char *g_description = 
	"This program is used to convert raw data to 3D volumes\n"
	"with apropriate metadata.\n"
	"Basic usage:\n"
	"  mia-raw2volume [options] \n"; 


template <typename I>
void handle_endian(I b, I e)
{
	typedef typename iterator_traits<I>::value_type Pixel;

	typedef union  {
		char s[sizeof(Pixel)];
		Pixel v;
	} shuffle;

	switch (sizeof(Pixel)) {
	case 2:
		while (b != e) {
			shuffle s;
			s.v = *b;
			swap(s.s[0], s.s[1]);
			*b = s.v;
			++b;
		}
		break;
	case 4:
		while (b != e) {
			shuffle s;
			s.v = *b;
			swap(s.s[0], s.s[3]);
			swap(s.s[1], s.s[2]);
			*b = s.v;
			++b;
		}
		break;
	case 8:
		while (b != e) {

			shuffle s;
			s.v = *b;

			swap(s.s[0],s.s[7]);
			swap(s.s[1],s.s[6]);
			swap(s.s[2],s.s[5]);
			swap(s.s[3],s.s[4]);

			*b = s.v;

			++b;
		}
		break;
		// default:
	}
}


template <typename Image>
std::shared_ptr<C3DImage > read_image_type(CInputFile& in_file, const C3DBounds& size, const C3DFVector& scale, bool big_endian)
{
	typedef typename Image::value_type T;
	Image *image = new Image(size);
	std::shared_ptr<C3DImage > result(image);
	if (!image) {
		stringstream errmsg;
		errmsg << "Unable to allocate image of size " << size;
		throw runtime_error(errmsg.str());
	}
	if (fread(&(*image)(0,0,0), sizeof(T), image->size(),  in_file) != image->size()) {
		throw runtime_error("Unable to read full image");
	}

	image->set_voxel_size(scale);

	if ( (sizeof(T) > 1 ) && (big_endian != am_big_endian())) {
		handle_endian(image->begin(), image->end());
	}

	return result;
}

std::shared_ptr<C3DImage > read_image(CInputFile& in_file, int pixel_type, const C3DBounds& size, const C3DFVector& scale, bool high_endian)
{
	switch (pixel_type) {
	case it_ubyte: return read_image_type<C3DUBImage>(in_file, size, scale, high_endian);
	case it_sbyte: return read_image_type<C3DSBImage>(in_file, size, scale, high_endian);
	case it_sshort:return read_image_type<C3DSSImage>(in_file, size, scale, high_endian);
	case it_ushort:return read_image_type<C3DUSImage>(in_file, size, scale, high_endian);
	case it_sint:  return read_image_type<C3DSIImage>(in_file, size, scale, high_endian);
	case it_uint:  return read_image_type<C3DUIImage>(in_file, size, scale, high_endian);
	case it_float: return read_image_type<C3DFImage> (in_file, size, scale, high_endian);
	case it_double: return read_image_type<C3DDImage> (in_file, size, scale, high_endian);
	default:
		throw invalid_argument("given input pixel format not supported");

	};
}

int run(int argc, const char *argv[])
{
	EPixelType pixel_type = it_ubyte;
	bool high_endian = false;
	C3DBounds size(0,0,0);
	C3DFVector scale(1,1,1);
	string in_filename;
	string out_filename;
	size_t skip = 0;

	const C3DImageIOPluginHandler::Instance& imageio = C3DImageIOPluginHandler::instance();

	if (imageio.get_set().empty())
		throw runtime_error("Sorry, no 3D output formats supported");

	CCmdOptionList options(g_description);

	options.add(make_opt( in_filename, "in-file", 'i', "input file name", CCmdOption::required));
	options.add(make_opt( out_filename, "out-file", 'o', "output file name", CCmdOption::required));
	options.add(make_opt( pixel_type, CPixelTypeDict, "repn", 'r',"input pixel type ", CCmdOption::required));
	options.add(make_opt( high_endian, "big-endian", 'b', "input data is big endian"));
	options.add(make_opt( scale, "scale", 'f', "scale of input voxels <FX,FY,FZ>"));
	options.add(make_opt( skip, "skip", 'k', "skip number of bytes from beginning of file"));
	options.add(make_opt( size, "size", 's', "size of input <NX,NY,NZ>", CCmdOption::required));

	if (options.parse(argc, argv, false) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 


	CInputFile in_file(in_filename);
	if ( !in_file )
		throw runtime_error(string("Unable to open ")+ in_filename);

	if (size.size() != 3)
		throw invalid_argument("size takes exactly 3 parameters");

	if (scale.size() != 3)
		throw invalid_argument("scale takes exactly 3 parameters");

	C3DBounds bsize(size);
	C3DFVector fscale(scale);

	C3DImageVector out_images;
	fseek(in_file, skip, SEEK_SET);
	out_images.push_back(read_image(in_file, pixel_type, bsize, fscale, high_endian));

	return !imageio.save(out_filename, out_images);
}


int main(int argc, const char *argv[])
{
	try {
		return run(argc, argv);
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

