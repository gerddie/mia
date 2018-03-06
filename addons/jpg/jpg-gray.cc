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

#include <cstdio>
#include <cstdlib>

#include <sstream>
#include <cassert>
#include <mia/core/file.hh>
#include <mia/core/filter.hh>
#include <mia/core/msgstream.hh>
#include <mia/2d/imageio.hh>

#include "jpeg-common.hh"




NS_BEGIN(IMAGEIO_2D_JPG)

using namespace mia;
using namespace std;
using namespace boost;

class CJpeg2DImageIOPlugin : public C2DImageIOPlugin
{
public:
       CJpeg2DImageIOPlugin();
private:
       PData do_load(const string& fname) const;
       bool do_save(const string& fname, const Data& data) const;
       const string do_get_descr() const;

       bool do_save_jpeg(FILE *f, const C2DUBImage& image) const;
};

CJpeg2DImageIOPlugin::CJpeg2DImageIOPlugin():
       C2DImageIOPlugin("jpg")
{
       add_supported_type(it_ubyte);
       add_suffix(".jpg");
       add_suffix(".JPG");
       add_suffix(".jpeg");
       add_suffix(".JPEG");
}

METHODDEF(void) mia_jpeg_load_error_exit (j_common_ptr /*cinfo*/)
{
       throw mia::create_exception<runtime_error>("JpegGray::load: error reading from input file");
}

METHODDEF(void) mia_jpeg_save_error_exit (j_common_ptr /*cinfo*/)
{
       throw create_exception<runtime_error>("JpegGray::save: error reading from input file");
}


C2DImageIOPlugin::PData CJpeg2DImageIOPlugin::do_load(const string& fname) const
{
       CInputFile f(fname);

       if (!f)
              throw create_exception<runtime_error>("C2DImageIOPlugin::load :unable to open input file '", fname, "'");

       miajpeg::JpegDecompress decompress;
       decompress.err.error_exit = mia_jpeg_load_error_exit;
       jpeg_stdio_src(&decompress.info, f);

       // is it a jpeg image?
       if (jpeg_read_header(&decompress.info, TRUE) != JPEG_HEADER_OK)
              return C2DImageIOPlugin::PData();

       jpeg_start_decompress(&decompress.info);

       // only one component?
       if (decompress.info.output_components != 1) {
              throw create_exception<invalid_argument>("The plugin '", get_descr(),
                            "' only supports gray scale images, but got an image with ",
                            decompress.info.output_components, " color components.");
       }

       if (decompress.info.data_precision != 8) {
              throw create_exception<invalid_argument>("The plugin '", get_descr(), "'only supports 8-bit per pixel images, "
                            "but got an image with ",
                            decompress.info.data_precision, " bits data precision.");
       }

       int row_stride = decompress.info.output_width * decompress.info.output_components;
       vector<JSAMPLE> buf(row_stride);
       JSAMPROW buffer[1];
       buffer[0] = &buf[0];
       C2DUBImage *result = new C2DUBImage(C2DBounds(decompress.info.output_width, decompress.info.output_height));
       P2DImage presult(result);

       while (decompress.info.output_scanline < decompress.info.output_height) {
              (void) jpeg_read_scanlines(&decompress.info, buffer, 1);
              copy(buf.begin(), buf.end(),
                   result->begin_at(0, decompress.info.output_scanline - 1));
       }

       C2DImageIOPlugin::PData result_vector(new C2DImageIOPlugin::Data);
       result_vector->push_back(presult);
       return result_vector;
}

bool CJpeg2DImageIOPlugin::do_save_jpeg(FILE *f, const C2DUBImage& image) const
{
       miajpeg::JpegCompress compress;
       compress.err.error_exit = mia_jpeg_save_error_exit;
       compress.info.image_width = image.get_size().x;
       compress.info.image_height = image.get_size().y;
       vector<JSAMPLE> samples(image.get_size().x);
       JSAMPROW scanline[1];
       scanline[0] = &samples[0];
       compress.info.input_components = 1;
       compress.info.in_color_space = JCS_GRAYSCALE;
       jpeg_set_defaults(&compress.info);
       jpeg_set_quality(&compress.info, 100, 1);
       jpeg_stdio_dest(&compress.info, f);
       jpeg_start_compress(&compress.info, TRUE);

       for (unsigned int y = 0; y < image.get_size().y; ++y) {
              copy(image.begin_at(0, y), image.begin_at(0, y) + compress.info.image_width, scanline[0]);
              jpeg_write_scanlines(&compress.info, scanline, 1);
       }

       jpeg_finish_compress(&compress.info);
       jpeg_destroy_compress(&compress.info);
       return true;
}

bool CJpeg2DImageIOPlugin::do_save(const string& fname, const C2DImageVector& data) const
{
       TRACE_FUNCTION;

       if (data.size() != 1)
              throw invalid_argument("Jpeg support exactly 1 image per file");

       const C2DUBImage *image = dynamic_cast<const C2DUBImage *>(data[0].get());

       if (!image)
              throw invalid_argument("JPEG save only supports unsigned byte data");

       COutputFile f(fname);

       if (!f)
              throw create_exception<runtime_error>("CPNG2DImageIO::save:unable to open output file '", fname, "'");

       return do_save_jpeg(f, *image);
}

const string CJpeg2DImageIOPlugin::do_get_descr() const
{
       return "a 2dimage io plugin for jpeg gray scale images";
}

extern "C" EXPORT  CPluginBase *get_plugin_interface()
{
       return new CJpeg2DImageIOPlugin();
}



NS_END
