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

#include <cmath>
#include <mia/core.hh>
#include <mia/2d.hh>
#include <mia/internal/main.hh>

NS_MIA_USE;
using namespace std;


const SProgramDescription g_description = {
       {pdi_group, "Analysis, filtering, combining, and segmentation of 2D images"},
       {pdi_short, "Evaluate the intensity variation of pixels in a series of images."},
       {
              pdi_description, "This program evaluates the pixel-wise accumulated intensity "
              "variation of a set of image given on the command line. "
              "If the input image files contain more then one image all images are used. "
              "All images must be of the same size."
       },
       {
              pdi_example_descr, "Evaluate the pixel-vise intensity variation of images i1.png, i2.png, "
              "i3.png, and i4.png and store the result to var.v."
       },
       {pdi_example_code, "-o var.v i1.png i2.png i3.png i4.png"}
};


class CVarAccumulator : public TFilter<bool>
{
public:
       CVarAccumulator():
              m_n(0)
       {
       }

       template <typename T>
       bool operator () (const T2DImage<T>& image)
       {
              if (!m_n) {
                     m_sum = C2DDImage(image.get_size(), image);
                     m_sum2 = C2DDImage(image.get_size());
              } else if (image.get_size() != m_sum.get_size()) {
                     throw create_exception<invalid_argument>( "input images differ in size");
              }

              transform(image.begin(), image.end(), m_sum.begin(), m_sum.begin(),
              [](double x, double y) {
                     return x + y;
              });
              transform(image.begin(), image.end(), m_sum2.begin(), m_sum2.begin(),
              [](double x, double y) {
                     return x * x + y;
              });
              ++m_n;
              return true;
       }

       P2DImage result()const
       {
              C2DFImage *image = new C2DFImage( m_sum.get_size());
              P2DImage result(image);
              transform(m_sum.begin(), m_sum.end(), m_sum2.begin(), image->begin(),
              [this](double sum, double sum2) {
                     return (sum2 - sum * sum / m_n) / (m_n - 1);
              });
              transform(image->begin(), image->end(), image->begin(),
              [](double x) {
                     return sqrt(x);
              });
              return result;
       }
private:
       C2DDImage m_sum;
       C2DDImage m_sum2;
       size_t m_n;
};

int do_main( int argc, char *argv[] )
{
       string out_filename;
       string out_type("vista");
       const auto& imageio = C2DImageIOPluginHandler::instance();
       CCmdOptionList options(g_description);
       options.add(make_opt( out_filename, "out-file", 'o', "output image ",
                             CCmdOptionFlags::required_output, &imageio));

       if (options.parse(argc, argv, "image", &imageio) != CCmdOptionList::hr_no)
              return EXIT_SUCCESS;

       if (options.get_remaining().empty())
              throw runtime_error("no input images given ...");

       auto input_images = options.get_remaining();
       CVarAccumulator ic;

       for (auto  i = input_images.begin(); i != input_images.end(); ++i) {
              cvmsg() << "Load " << *i << "\r";
              auto in_image_list = imageio.load(*i);

              if (in_image_list.get() && in_image_list->size()) {
                     accumulate(ic, **in_image_list->begin());
              }
       }

       cvmsg() << "\n";

       if (save_image(out_filename, ic.result()))
              return EXIT_SUCCESS;
       else
              cerr << argv[0] << " fatal: unable to output image to " <<  out_filename << endl;

       return EXIT_SUCCESS;
}

MIA_MAIN(do_main);
