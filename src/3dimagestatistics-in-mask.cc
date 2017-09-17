/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * 
 * Copyright (c) Leipzig, Madrid 1999-2017 Gert Wollny
 *
 *
 * pez-tools is free software; you can redistribute it and/or modify
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
 * along with pez-tools; if not, see <http://www.gnu.org/licenses/>.
 *
 */


#include <mia/internal/main.hh>
#include <mia/core/cmdlineparser.hh>
#include <mia/core/meanvar.hh>
#include <mia/core/filter.hh>
#include <mia/core/histogram.hh>
#include <mia/core/errormacro.hh>
#include <mia/3d/imageio.hh>





using namespace mia; 
using namespace std; 

const SProgramDescription g_description = {
        {pdi_group, "Analysis, filtering, combining, and segmentation of 3D images"}, 
	{pdi_short, "Evaluate statistics within a masked region of the input data."}, 
	{pdi_description, "This program evaluates the statistics of an image within a masked "
         "region of the image. Possible outputs are a summary statistics and a histogram of "
         "the values."
        }, 
	{pdi_example_descr, "evaluate the summary statistics on the input and a histogram with 10 bins and "
         "automtically evaluated boundaries. Save summary in summary.txt and the histogram in hist.txt"}, 
	{pdi_example_code, "--image image.v.gz --mask mask.v.gz --summary summary.txt --histogram hist.txt"}
}; 

struct SHistogramParams {
	string struct_name; 
        size_t bins; 
        float range_min; 
        float range_max; 
        
}; 

struct FPixelAccumulator : public TFilter<size_t> {
public: 
        FPixelAccumulator(const C3DBitImage& mask):m_mask(mask) {
                size_t count = 0; 
                for(auto m : mask) 
                        if (m) 
                                ++count; 
                m_pixels.reserve(count); 
        }
        
        template <typename T>
        size_t operator () (const T3DImage<T>& image) {
                assert(image.get_size() == m_mask.get_size()); 
                
                auto im = m_mask.begin();
                auto ii = image.begin(); 
                auto ie = image.end(); 
                
                while (ii != ie) {
                        if (*im)
                                m_pixels.push_back(*ii); 
			++ii; ++im; 
                }
                sort(m_pixels.begin(), m_pixels.end()); 
                return m_pixels.size(); 
        }
        
        void get_summary(ostream& os) const {
                double min_val = m_pixels[0]; 
                double max_val = m_pixels[m_pixels.size() - 1];                
                double median = m_pixels[m_pixels.size() / 2];
                auto meanvar =  mean_var(m_pixels.begin(), m_pixels.end()); 
                os << "#   mean   |  variation |   median   |    min    |    max\n"; 
                os << meanvar.first << " " << meanvar.second << " " 
                   << median << " " << min_val << " " << max_val; 
        }
        
        void get_histogram(SHistogramParams params, ostream& os) const {
                if (params.range_min >= params.range_max)  {
                        params.range_min = m_pixels[0];
                        params.range_max = m_pixels[m_pixels.size() - 1];
                }
                THistogram<THistogramFeeder<double > > histo(THistogramFeeder<double >(params.range_min, params.range_max, params.bins)); 
                histo.push_range(m_pixels.begin(), m_pixels.end());
                os << "#histogram\n"; 
                os << "struct=" << params.struct_name << '\n'; 
                os << "min=" << params.range_min << '\n'; 
                os << "max=" << params.range_max << '\n'; 
                os << "step=" << ( params.range_max - params.range_min ) / double(params.bins - 1) << '\n';
                os << "values="; 
                for (auto ih = histo.begin(); ih != histo.end(); ++ih) 
                        os << *ih << " "; 
                os << "\n"; 
        }

private: 
        const C3DBitImage& m_mask; 
        vector<double> m_pixels; 
}; 

int do_main( int argc, char *argv[] )
{

	string in_filename;
        string mask_filename;

        string summary_filename;
        string histogram_filename;

        unsigned pixel_limit = 20; 

        SHistogramParams hparams = {"undefined", 8, 0.0f, 0.0f}; 
        
	const C3DImageIOPluginHandler::Instance& imageio = C3DImageIOPluginHandler::instance();

	CCmdOptionList options(g_description);

        options.set_group("File-IO");         
	options.add(make_opt( in_filename, "image", 'i', "input image to be analyzed", 
			      CCmdOptionFlags::required_input, &imageio));
        options.add(make_opt( mask_filename, "mask", 'm', "mask image to use to restrict the analysis", 
                              CCmdOptionFlags::required_input, &imageio));

	options.add(make_opt( summary_filename, "summary", 's', "File to write the summary statistics to as plain text.", 
			      CCmdOptionFlags::output));
        options.add(make_opt( histogram_filename, "histogram", 'H', "File to write the histogram to",
                              CCmdOptionFlags::output));
        
        options.set_group("Parameters");

	options.add(make_opt(hparams.struct_name, "struct-name", 0, "Name of the structure the statistics are evaluated for")); 
        options.add(make_opt(pixel_limit, EParameterBounds::bf_min_closed, {20}, "limit", 'l', 
                             "Number of pixels the mask should at least contain.")); 
        options.add(make_opt(hparams.bins, EParameterBounds::bf_min_closed, {4}, "bins", 'b', 
                             "number of histogram bins to be used")); 
        options.add(make_opt(hparams.range_min, "hmin", 0, "lower bound for the histogram mapping. All values below this value"
                             " will be mapped into the lowest bin. If this parameter is equal or larger than hmax, the "
                             "histogram range is evaluated automatically.")); 
        options.add(make_opt(hparams.range_max, "hmax", 0, "hpper bound for the histogram mapping. All values above this value"
                             "will be mapped into the highest bin. If this parameter is equal or smaller than hmin, the "
                             "histogram range is evaluated automatically."));
        
        
	if (options.parse(argc, argv) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 
        
        if (summary_filename.empty() && histogram_filename.empty())
                throw invalid_argument("At least one option of '--summary' and '--histogram' must be given");
        
        auto image = load_image3d(in_filename); 
        auto mask_image = load_image3d(mask_filename); 
        
        if ( mask_image->get_pixel_type() != it_bit)  {
                throw create_exception<invalid_argument>("Mask image must be binary, but image in '", 
                                                          mask_filename, "' is of type '", 
                                                          CPixelTypeDict.get_name(mask_image->get_pixel_type()), "'");
        }

        if (mask_image->get_size() != image->get_size()) {
                throw create_exception<invalid_argument>("Mask image '", mask_filename, "(", mask_image->get_size(), ")'",
                                                          "and value image '", in_filename, "(", image->get_size(), ")'", 
                                                          "are of different size."); 
        }

        auto& mask = static_cast<const C3DBitImage&>(*mask_image); 
        
        FPixelAccumulator pa(mask); 
        
        auto counted_pixels = mia::accumulate(pa, *image); 
        if (counted_pixels < pixel_limit) {
                throw create_exception<runtime_error>("Resquested at least ", pixel_limit, " valid pixel, "
                                                      "but mask only contained ", counted_pixels); 
        }
        
        if (!summary_filename.empty()) { 
                // obtain summary stratistics 
                ofstream  os; 
                os.exceptions( std::ofstream::failbit | std::ofstream::badbit ); 
                os.open(summary_filename.c_str()); 

                pa.get_summary(os); 
        }
         
        if (!histogram_filename.empty()) { 
                // obtain summary stratistics 
                ofstream  os; 
                os.exceptions( std::ofstream::failbit | std::ofstream::badbit ); 
                os.open(histogram_filename.c_str()); 
                pa.get_histogram(hparams, os); 
        }

        return EXIT_SUCCESS; 
}

#include <mia/internal/main.hh>
MIA_MAIN(do_main)

