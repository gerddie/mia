/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2015 Gert Wollny
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

#include <sstream>
#include <ostream>
#include <fstream>
#include <mia/core.hh>
#include <mia/2d.hh>

NS_MIA_USE;
using namespace std;

const SProgramDescription g_description = {
	
	{pdi_group, "Processing of series of 2D images in a 3D fashion (out-of-core)"}, 
	{pdi_short, "Pre-classify the input image series by using a c-means estimator"}, 
	
	
	{pdi_description, "This program first evaluates a sparse histogram of an input image "
         "series, then runs a c-means classification over the histogram, and then estimates a "
         "per image seeds for later segmentation based on class probabilities. "
         "This program accepts only images of eight or 16 bit integer pixels."
        }, 
	
	{pdi_example_descr,"Run the program over images imageXXXX.png with the sparse histogram, "
         "threshold the lower 30% bins (if available), run cmeans with two classes on the non-zero "
         "pixels and then create the seeds mask as seedXXXX.png."}, 
	
	{pdi_example_code, "-i image.png -o seed -t png --histogram-tresh=30 --classes 2"}
}; 

class CFullHistogram : public TFilter<size_t> {
        
public: 
        CFullHistogram();

        template <typename T>
        size_t operator ()(const T2DImage<T>& image); 

        vector<pair<int, unsigned long>> get_compressed_histogram()const; 
        
private: 
        
        vector<unsigned long> m_histogram; 

        int m_shift; 
        EPixelType m_pixeltype; 

        
        
}; 

template <typename T> 
struct dispatch_by_pixeltype {
        static void apply(const T2DImage<T>& MIA_PARAM_UNUSED(image),
                          int MIA_PARAM_UNUSED(shift), vector<long>& MIA_PARAM_UNUSED(histogram)){
                throw invalid_argument("Input pixel type not supported"); 
        }
}; 

template <> 
struct dispatch_by_pixeltype<unsigned char> {
        static void apply(const C2DUBImage& image, int MIA_PARAM_UNUSED(shift), vector<long>& histogram){
                for (auto p: image) {
                        ++histogram[p]; 
                }
        }
}; 

template <> 
struct dispatch_by_pixeltype<unsigned short> {
        static void apply(const C2DUSImage& image, int MIA_PARAM_UNUSED(shift), vector<long>& histogram){
                for (auto p: image) {
                        ++histogram[p]; 
                }
        }
}; 

template <> 
struct dispatch_by_pixeltype<signed char> {
        static void apply(const C2DSBImage& image, int shift, vector<long>& histogram){
                for (auto p: image) {
                        ++histogram[shift + p]; 
                }
        }
}; 

template <> 
struct dispatch_by_pixeltype<unsigned short> {
        static void apply(const C2DSSImage& image, int shift, vector<long>& histogram){
                int shift = -numeric_limits<signed short>::min(); 
                for (auto p: image) {
                        ++histogram[shift + p]; 
                }
        }
}; 

CFullHistogram::CFullHistogram():
        m_shift(0), 
        m_pixeltype(it_none)
{
}

template <typename T>
size_t CFullHistogram::operator ()(const T2DImage<T>& image)
{
        if (m_pixeltype ==it_none) {
                m_pixeltype = image.get_pixel_type();
                m_shift = numeric_limits<T>::min();
                switch (m_pixeltype) {
                case it_sbyte:
                case it_ubyte:
                        m_histogram.resize(256);
                        break; 
                case it_sshort:
                case it_ushort:
                        m_histogram.resize(65536);
                        break; 
                default:
                        throw create_exception<invalid_argument>("Input pixel type '", CPixelTypeDict.get_name(m_pixeltype),
                                                                 "' not supported."); 
                }
                        
        } else if (m_pixeltype != image.get_pixel_type()){
                throw create_exception<invalid_argument>("Input pixels not of consisted type, started with ",
                                                         CPixelTypeDict.get_name(m_pixeltype), ", but got now ",
                                                         CPixelTypeDict.get_name(image.get_pixel_type())); 
        }

        dispatch_by_pixeltype<T>::apply(image, m_shift, m_histogram);

        return image.size(); 
}

vector<pair<int, unsigned long>> CFullHistogram::get_compressed_histogram()const
{

        int nonzero_bins = 0;
        for (auto b: m_histogram) {
                if (b > 0)
                        ++nonzero_bins; 
        }

        vector<pair<int, unsigned long>> result;
        result.reserve(nonzero_bins);
        for (int i = 0; i < m_histogram.size(); ++i) {
                if (m_histogram[i] != 0)
                        result.push_back(make_pair(i - m_shift, m_histogram[i])); 
        }
        return result; 
}

typedef map<double, unsigned char> CLabelSeedMapper;
class FSetLabels: public TFilter<P2DImage> {
	FSetLabels(const CLabelSeedMapper& map);

	template <typename T> 
	P2DImage operator() (const T2DImage<T>& image) const;   
private:
	const CLabelSeedMapper& m_map; 

}; 

FSetLabels::FSetLabels(const CLabelSeedMapper& map):m_map(map)
{
}
						    

template <typename T> 
P2DImage FSetLabels::operator() (const T2DImage<T>& image) const
{
	C2DUBImage *labels = new C2DUBImage(image.get_size(), image);

	transform(image.begin(), image.end(), labels.begin(), [&m_map](T x) {
			return m_map[x]; 
		}); 
	return labels; 
}

int do_main( int argc, char *argv[] )
{
        string out_filename;
	string in_filename;
        string out_filetype("png");

	float seed_threshold = 0.9; 
        float histogram_thresh = 30;
	PInitializer class_center_initializer; 
		
	const C2DImageIOPluginHandler::Instance& imageio = C2DImageIOPluginHandler::instance();

	CCmdOptionList options(g_description);

        options.set_group("File-IO"); 
	options.add(make_opt( in_filename, "in-file", 'i', "input image(s) to be filtered", 
			      CCmdOptionFlags::required_input, &imageio));
	options.add(make_opt( out_filename, "out-seeds", 'o', "output file name base", 
			      CCmdOptionFlags::required_output));
        options.add(make_opt( out_type, "type", 't', "output file name type", 
			      CCmdOptionFlags::required_output));

        options.set_group("Classes");
        options.add(make_opt( histogram_thresh, bf_closed_interval, {0,50}, "histogram-thresh", 'T',
                              "Percent of the extrem parts of the histogram to be collapsed into the respective last histogram bin."));
        
        options.add(make_opt( nclasses, bf_min_open, {0}, "classes", 'C', "Number of classes the non-zero pixels "
                              "should be classified in"));

	
        
	if (options.parse(argc, argv) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 

	size_t start_filenum = 0;
	size_t end_filenum  = 0;
	size_t format_width = 0;

	string src_basename = get_filename_pattern_and_range(in_filename, start_filenum, end_filenum, format_width);
		
	if (start_filenum >= end_filenum)
		throw invalid_argument(string("no files match pattern ") + src_basename);


        CFullHistogram histo;
        size_t n_pixels = 0; 
        for (size_t i = start_filenum; i < end_filenum; ++i) {
                string src_name = create_filename(src_basename.c_str(), i);
                C2DImageIOPluginHandler::Instance::PData  in_image_list = imageio.load(src_name);
                cvmsg() << "Read:" << src_name << "\r";
                if (in_image_list.get() && in_image_list->size()) {
                        for (auto k = in_image_list->begin(); k != in_image_list->end(); ++k)
                                n_pixels += accumulate(histo, **k);
                }
        }

        auto chistogram = histo.get_compressed_histogram();

        size_t n_cut_off = static_cast<size_t>(floor(n_pixels / 100.0 * histogram_thresh));



	auto ii = chistogram.begin(); 
        while ( ni < n_cut_off && ii != chistogram.end()) {
		ni += ii->second;
                ++ii; 
        }
	
	auto ie = chistogram.end() - 1;
        while ( ne < n_cut_off && ie !=  ii) {
		ne += ie->second;
                --ie;
        }

        vector<pair<int, unsigned long>> threshed_histo(ii, ie);

	DVector class_centers; 
	
	CMeans cmeans(0.01, 0.00001, class_center_initializer);
	SparseProbmap pv = cmeans.run(threshed_histo,  class_centers);

	// now create the label map.

	for (int i = 0; i < pv.size(); ++i) {
		int result = 0; 
		for (auto k = 0; k < class_centers.size(); ++k) {
			if (pv[i].second[k] > seed_threshold)
				result = k + 1; 
		}
		value_map[pv[i].first] = result; 
	}

	// created the labeled images
	FSetLabels  set_labels(value_map);

	P2DFilter seeded_ws = produce_2dimage_filter("sws:seed=labelseed.@"); 
	
        for (size_t i = start_filenum; i < end_filenum; ++i) {
                string src_name = create_filename(src_basename.c_str(), i);
                C2DImageIOPluginHandler::Instance::PData  in_image_list = imageio.load(src_name);
                cvmsg() << "Read:" << src_name << "\r";
                if (in_image_list.get() && in_image_list->size()) {
                        for (auto k = in_image_list->begin(); k != in_image_list->end(); ++k) {
				// create label image
				auto label_seed = mia::filter (set_labels, **k);
				save_image("labelseed.@");

				auto label = seeded_ws->filter(**k);
				*k = label; 
			}
                }
		string trgt_name = create_filename(out_filename.c_str(), i);
		stringstream ss;
		ss << out_filename << setw(format_width) << setfill('0') << i << "." << out_filetype;

		imageio.save(ss.str(), in_image_list);
        }
	return EXIT_SUCCESS;
}

MIA_MAIN(do_main); 

        
