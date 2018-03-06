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

#include <string>
#include <sstream>

#include <mia/core.hh>
#include <mia/2d.hh>

NS_MIA_USE;
using namespace std;
using namespace boost;

const SProgramDescription g_description = {
       {pdi_group, "Image conversion"},
       {pdi_short,  "Convert raw data into a 2D image"},
       {pdi_description, "This program is used to convert raw data to a 2D file format."},
       {
              pdi_example_descr, "Convert a data set data.raw of size <10,20> of short integer data with a "
              "pixel size of <1.2, 2.3> to a PNG file image.png"
       },
       {pdi_example_code, "-i data.raw -o image.png -s '<10,20>' -k '<1.2,2.3>' -r ushort"}
};

inline bool am_big_endian()
{
#ifdef WORDS_BIGENDIAN
       return true;
#else
       return false;
#endif
}


template <typename Pixel, int size>
struct __swap {
       typedef union  {
              char s[size];
              Pixel v;
       } shuffle;

       static void apply(Pixel& /*p*/)
       {
              throw invalid_argument("unknown pixel size");
       }
};

template <typename Pixel>
struct __swap<Pixel, 2> {
       typedef union  {
              char s[2];
              Pixel v;
       } shuffle;

       static void apply(Pixel& p)
       {
              shuffle s;
              s.v = p;
              swap(s.s[0], s.s[1]);
              p = s.v;
       }
};

template <typename Pixel>
struct __swap<Pixel, 4> {
       typedef union  {
              char s[4];
              Pixel v;
       } shuffle;

       static void apply(Pixel& p)
       {
              shuffle s;
              s.v = p;
              swap(s.s[0], s.s[3]);
              swap(s.s[1], s.s[2]);
              p = s.v;
       }
};

template <typename Pixel>
struct __swap<Pixel, 8> {
       typedef union  {
              char s[8];
              Pixel v;
       } shuffle;

       static void apply(Pixel& p)
       {
              shuffle s;
              s.v = p;
              swap(s.s[0], s.s[7]);
              swap(s.s[1], s.s[6]);
              swap(s.s[2], s.s[5]);
              swap(s.s[3], s.s[4]);
              p = s.v;
       }
};


template <typename I>
void handle_endian(I b, I e)
{
       typedef typename iterator_traits<I>::value_type Pixel;

       while (b != e) {
              __swap<Pixel, sizeof(Pixel)>::apply(*b);
       }
}

template <typename Image>
P2DImage read_image_type(CInputFile& in_file, const C2DBounds& size, const C2DFVector& scale, bool big_endian)
{
       typedef typename Image::value_type T;
       Image *image = new Image(size);
       P2DImage result(image);

       if (!image) {
              stringstream errmsg;
              errmsg << "Unable to allocate image of size " << size;
              throw runtime_error(errmsg.str());
       }

       if (fread(&(*image)(0, 0), sizeof(T), image->size(),  in_file) != image->size()) {
              throw runtime_error("Not enough data for specified image size in input file.");
       }

       image->set_pixel_size(scale);

       if ( (sizeof(T) > 1 ) && (big_endian != am_big_endian())) {
              handle_endian(image->begin(), image->end());
       }

       return result;
}

P2DImage read_image(CInputFile& in_file, int pixel_type, const C2DBounds& size, const C2DFVector& scale, bool high_endian)
{
       switch (pixel_type) {
       case it_ubyte:
              return read_image_type<C2DUBImage>(in_file, size, scale, high_endian);

       case it_sbyte:
              return read_image_type<C2DSBImage>(in_file, size, scale, high_endian);

       case it_sshort:
              return read_image_type<C2DSSImage>(in_file, size, scale, high_endian);

       case it_ushort:
              return read_image_type<C2DUSImage>(in_file, size, scale, high_endian);

       case it_sint:
              return read_image_type<C2DSIImage>(in_file, size, scale, high_endian);

       case it_uint:
              return read_image_type<C2DUIImage>(in_file, size, scale, high_endian);

       case it_float:
              return read_image_type<C2DFImage> (in_file, size, scale, high_endian);

       case it_double:
              return read_image_type<C2DDImage> (in_file, size, scale, high_endian);

       default:
              throw invalid_argument("given input pixel format not supported");
       };
}


int do_main(int argc, char *argv[])
{
       EPixelType pixel_type = it_ubyte;
       bool high_endian = false;
       C2DBounds size(0, 0);
       C2DFVector scale(1, 1);
       string in_filename;
       string out_filename;
       string type;
       size_t skip = 0;
       const auto&  imageio = C2DImageIOPluginHandler::instance();

       if (imageio.get_set().empty())
              throw runtime_error("Sorry, no 2D output formats supported");

       CCmdOptionList options(g_description);
       options.add(make_opt( in_filename, "in-file", 'i', "input file name", CCmdOptionFlags::required_input));
       options.add(make_opt( out_filename, "out-file", 'o', "output file name", CCmdOptionFlags::required_output, &imageio));
       options.add(make_opt( pixel_type, CPixelTypeDict, "repn", 'r', "input pixel type "));
       options.add(make_opt( high_endian, "big-endian", 'b', "input data is big endian"));
       options.add(make_opt( scale, "scale", 'f', "scale of input pixels <FX,FY>"));
       options.add(make_opt( size, "size", 's', "size of input <NX,NY>", CCmdOptionFlags::required));
       options.add(make_opt( skip, "skip", 'k', "skip number of bytes from beginning of file"));

       if (options.parse(argc, argv) != CCmdOptionList::hr_no)
              return EXIT_SUCCESS;

       CInputFile in_file(in_filename);

       if ( !in_file )
              throw runtime_error(string("Unable to open ") + in_filename);

       if (size.size() != 2)
              throw invalid_argument("size takes exactly 2 parameters");

       if (scale.size() != 2)
              throw invalid_argument("scale takes exactly 2 parameters");

       C2DImageVector out_images;

       if (fseek(in_file, skip, SEEK_SET) == 0)
              out_images.push_back(read_image(in_file, pixel_type, size, scale, high_endian));
       else
              throw create_exception<invalid_argument>("Skipping ", skip, " bytes of file '",
                            in_filename, "' failed:", strerror(errno));

       return !imageio.save(out_filename, out_images);
}


#include <mia/internal/main.hh>
MIA_MAIN(do_main);
