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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif



#include <mia/core/histogram.hh>
#include <mia/core/cmdlineparser.hh>
#include <mia/2d.hh>


#include <memory>
#include <vector>

using namespace mia;
using namespace std; 

typedef vector<C2DFImage> C2DFImageVec;

const SProgramDescription g_description = {
        {pdi_group, "Analysis, filtering, combining, and segmentation of 2D images"}, 
	{pdi_short, "Run a segmentation of a noisy  2D image."}, 
	{pdi_description, "This program is experimental"}, 
	{pdi_example_descr, "No example yet" }, 
	{pdi_example_code, "-i input.png -o label.png"}
}; 


class  FInitialLabelImage: public TFilter<C2DUBImage> {
public: 
        FInitialLabelImage(float thresh, P2DImage orig):
                m_thresh(thresh),
                m_orig(orig)
        {
        }
        template< typename T, typename S>
        C2DUBImage operator() (const T2DImage<T>& median, const T2DImage<S>& mad) const; 
private:
        float m_thresh;
        P2DImage m_orig; 
}; 

template< typename T, typename S>
C2DUBImage FInitialLabelImage::operator() (const T2DImage<T>& median, const T2DImage<S>& mad) const
{
        // median filteredd image is of same pixel type like original
        const T2DImage<T>& porig = dynamic_cast<const T2DImage<T>&>(*m_orig); 
        
        C2DUBImage out_image(median.get_size(), median);
        
        auto imedian = median.begin();
        auto imad = mad.begin();
        auto iorig = porig.begin(); 
        auto iout = out_image.begin();
        auto eout = out_image.end(); 

        while (iout != eout) {
                if (*imedian != 0) {
                        cvdebug() << "*imad = " << *imad << "\n"; 
                        if (*imad < m_thresh)
                                *iout = 3;
                        else {
                                *iout = (*iorig < *imedian) ? 1 : 2; 
                        }
                } else
                        *iout = 0; 
                
                ++iout; ++imad; ++imedian; ++iorig; 
        }
        return out_image; 
}

        

int do_main(int argc, char *argv[])
{

	string in_filename; 
	string out_filename;

        float bg_thresh = 0.1f;
        unsigned median_filterwidth = 8;
        float mad_thresh = 22.0f;
        
        
	
	const auto& image2dio = C2DImageIOPluginHandler::instance();

	CCmdOptionList opts(g_description);
        opts.set_group("File-IO"); 
	opts.add(make_opt( in_filename, "in-file", 'i', "image to be segmented", CCmdOptionFlags::required_input, &image2dio )); 
	opts.add(make_opt( out_filename, "out-file", 'o', "class label image", CCmdOptionFlags::required_output, &image2dio ));

        opts.set_group("Parameters"); 
	opts.add(make_opt( bg_thresh, "bg-thresh", 't', "background intensity threshhold. if this value is in (0,1), "
                           "then it is assumed to be a fraction of the intensity range, if it is set to a value <0 then "
                           "an automatic estimation based on the histogram will be tried."));
	opts.add(make_opt( median_filterwidth, "median-filterwidth", 'w', "width of the median-MAD filter to be applied"));
	opts.add(make_opt( mad_thresh, "mad-thresh", 'm', "Threshold value for the median absolute distance (MAD) value. "
                           "pixels with MAD values below this threshhold are considered to belong to a homogenous image area."));
	
	if (opts.parse(argc, argv) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 
	
	auto in_image = load_image2d(in_filename);

        if (bg_thresh < 0.0f) {
                cvwarn() << "threshhold auto-estimation not yet implemented\n";
                return EXIT_FAILURE; 
        }else if (bg_thresh < 1.0){
                auto range = in_image->get_minmax_intensity();
		bg_thresh = (range.second  - range.first)  * bg_thresh + range.first;
        }
	cvinfo() << "Using intensity threshhold: " << bg_thresh << "\n";
	
        stringstream filter;
        filter << "medianmad:w=" << median_filterwidth << ",thresh=" << bg_thresh << ",madfile=mad.png";

        auto median = run_filter(in_image, filter.str().c_str());
        auto mad = load_image2d("mad.png");

        FInitialLabelImage ilm(mad_thresh, in_image);
        auto out_image = mia::filter(ilm, *median, *mad);

        save_image(out_filename, out_image); 
        
	return EXIT_SUCCESS; 
	
}

#include <mia/internal/main.hh>
MIA_MAIN(do_main); 

