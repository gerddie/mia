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


#include <mia/3d/imageio.hh>
#include <mia/core/pixeltype.hh>
#include <mia/core/cmdlineparser.hh>
#include <mia/3d/filter.hh>

using namespace mia;
using namespace std;

const SProgramDescription g_description = {
        {pdi_group, "Analysis, filtering, combining, and segmentation of 3D images"}, 
	{pdi_short, "Combine images."}, 
	{pdi_description, "This programs takes various input images of the same dimensions "
         "and creates a combined output images that uses the define operation for the combination." 
	}
}; 

struct pixel_combiner {
        virtual double apply (double x, double y) = 0; 
        virtual double finalize(double x, double n) = 0; 
}; 

struct pixel_mean: public pixel_combiner {
        virtual double apply (double x, double y) {
                return x + y; 
        }
        virtual double finalize(double x, double n) {
                return x / n; 
        }
};

struct pixel_max : public pixel_combiner{
        virtual double apply(double x, double y) {
                return x > y ? x : y; 
        }
        virtual double finalize(double x, double MIA_PARAM_UNUSED(n)) {
                return x; 
        }
};

struct pixel_add : public pixel_combiner{
        virtual double apply(double x, double y) {
                return x + y; 
        }
        virtual double finalize(double x, double MIA_PARAM_UNUSED(n)) {
                return x; 
        }
};

enum ECombineops {
        co_mean, 
        co_max, 
	co_add, 
        co_unknown
}; 

const TDictMap<ECombineops>::Table l_combops_table[] = {
	{"mean", co_mean, "mean image intensities"}, 
	{"max", co_max, "select max per pixel intensity"}, 
	{"add", co_add, "add pixel intensities"}, 
	{NULL, co_unknown, ""}
}; 

const TDictMap<ECombineops> g_combops_dict(l_combops_table);

class FImageSeriesCombiner : public TFilter<bool> {
public: 
        FImageSeriesCombiner(const C3DImage& prototype, pixel_combiner *op);  

        template <typename T> 
        bool operator () (const T3DImage<T>& image); 

        P3DImage get_result(unsigned n, EPixelType pixeltype) const; 
private: 
        C3DDImage m_buffer; 
        unique_ptr<pixel_combiner> m_op; 
}; 

FImageSeriesCombiner create_combiner(const C3DImage& prototype, ECombineops op)
{
        switch (op) { 
        case co_mean: return FImageSeriesCombiner(prototype, new pixel_mean());
        case co_max: return FImageSeriesCombiner(prototype, new pixel_max());
        case co_add: return FImageSeriesCombiner(prototype, new pixel_add());
        default: 
                throw invalid_argument("Unknown combiner requested"); 
        }
}

int do_main( int argc, char *argv[] )
{
	string out_filename;
        ECombineops op = co_mean; 
        EPixelType out_pixeltype = it_none; 
        


	const auto& imageio = C3DImageIOPluginHandler::instance();

	CCmdOptionList options(g_description);
	options.add(make_opt( out_filename, "out-file", 'o', "output file that contains the combined image.", 
			      CCmdOptionFlags::required_output, &imageio));

        options.add(make_opt(op, g_combops_dict, "operation", 'p', "Operation to be applied when combining the images"));
        options.add(make_opt(out_pixeltype, CPixelTypeDict, "repn", 'r', "Output pixel representation (if not given, "
                             "then the representation of the first input image will be used"));
        
	if (options.parse(argc, argv, "images", &imageio) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 

	auto in_image_files = options.get_remaining();

	if (in_image_files.empty()) {
		throw invalid_argument("Nothing to do, since no input image were given"); 
	}
	
	const auto n_images = in_image_files.size(); 
        
        // load the first image and create the combination filter
        auto first_image = load_image3d(in_image_files[0]); 
        
        auto combiner = create_combiner(*first_image, op); 
                
        accumulate(combiner, *first_image); 
        
        for (unsigned i = 1; i < n_images; ++i)  {
                auto image = load_image3d(in_image_files[i]); 
                accumulate(combiner, *image);
        }

        if (out_pixeltype == it_none)
                out_pixeltype = first_image->get_pixel_type(); 
        
        auto result = combiner.get_result(n_images, out_pixeltype); 

	return save_image(out_filename, result)? EXIT_SUCCESS : EXIT_FAILURE; 
}

#include <mia/internal/main.hh>
MIA_MAIN(do_main)

FImageSeriesCombiner::FImageSeriesCombiner(const C3DImage& prototype, pixel_combiner *op):
        m_buffer(prototype.get_size(), prototype), 
        m_op(op)
{
}
        

template <typename T> 
bool FImageSeriesCombiner::operator () (const T3DImage<T>& image)
{
        if (image.get_size() != m_buffer.get_size()) {
                throw create_exception<invalid_argument>("The input images differ in size, expect [", 
                                                         m_buffer.get_size(), "] but got [", image.get_size(), 
                                                         "], aborting");
        }
        transform(m_buffer.begin(), m_buffer.end(), image.begin(), m_buffer.begin(), 
                  [this](double x, T y){return m_op->apply(x,y);}); 
        return true; 
}

P3DImage FImageSeriesCombiner::get_result(unsigned n, EPixelType pixeltype) const
{
        C3DDImage help(m_buffer.get_size(), m_buffer); 
        
        transform(m_buffer.begin(), m_buffer.end(), help.begin(), 
                  [this, n](double x){return m_op->finalize(x, n);}); 
        
        stringstream convert_descr; 
        convert_descr << "convert:map=copy,repn=" << CPixelTypeDict.get_name(pixeltype); 
        return run_filter(help, convert_descr.str().c_str());
}
