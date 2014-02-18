/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2014 Gert Wollny
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
#include <mia/3d/filter.hh>
#include <mia/core.hh>
#include <mia/core/labelmap.hh>

#include <map>
#include <set>

using namespace mia; 
using namespace std; 

const SProgramDescription g_description = {
        {pdi_group, "Analysis, filtering, combining, and segmentation of 3D images"}, 
	{pdi_short, "Evaluate distance statistics for two labeled images."}, 
	{pdi_description, "This program is used to evaluate the distance between equally labelled "
         "areas in two images. The output file is a csv file containing the distances for "
         "each labeled coordinate in the test image in the following form:\n"
         "\n"
         "  label,n-samples,distance,distance,...\n"}, 
	{pdi_example_descr, "Evaluate the distances for each label availabe in image.v to the "
         "corresponding labels in the image reference.v ans store the result "
         "a coma separated list of values, i.e. distances.csv."}, 
	{pdi_example_code, "-i image.v -r reference.v -o distances.csv" }
}; 


unsigned short translate_label( unsigned short l, const CLabelMap& map)
{
        if (map.empty()) 
                return l; 
        auto idx = map.find(l); 
        if (idx != map.end()) 
                return idx->second; 
        throw create_exception<runtime_error>("unmapped label '", l, "' encountered"); 
}



typedef vector<float> CDistanceResult; 


typedef map<unsigned short, CDistanceResult> CDistanceResultMap; 

class  FEvaluateDistances: public TFilter<CDistanceResultMap> {
public: 
        FEvaluateDistances(const CLabelMap& lm); 
        
        template <typename T, typename S> 
        CDistanceResultMap operator ()(const T3DImage<T>& test, const T3DImage<S>& ref) const; 
private: 
        const CLabelMap m_lm; 
}; 

template <typename T, typename S, bool, bool> 
struct __dispatch_filter {
        static CDistanceResultMap apply(const T3DImage<T>& MIA_PARAM_UNUSED(test), 
                                        const T3DImage<S>& MIA_PARAM_UNUSED(ref), 
                                        const CLabelMap& MIA_PARAM_UNUSED(lm)){
                throw create_exception<invalid_argument>("CDistanceResultMap: input data not supported,"
                                                         "got ", __type_descr<T>::value, " and ", 
                                                         __type_descr<S>::value, " but require integral scalar types");
        }
}; 

template <typename T> 
P3DImage get_mask( const T3DImage<T>& image, unsigned short label) 
{
        stringstream descr; 
        descr << "binarize:min=" << label << ",max=" << label; 
        return  run_filter(image, descr.str().c_str()); 
}

template <typename T> 
C3DFImage get_ref_distance(const T3DImage<T>& image, unsigned short label, const C3DFilter& distance)
{
        auto mask = get_mask( image, label); 
        auto d = distance.filter(*mask); 
        return dynamic_cast<C3DFImage&>(*d); 
}


template <typename T, typename S> 
struct __dispatch_filter<T,S, true, true> {
        static CDistanceResultMap apply(const T3DImage<T>& test, const T3DImage<S>& ref, const CLabelMap& lm){
                set<T> test_labels; 
                
                CDistanceResultMap result; 
                P3DFilter distance = produce_3dimage_filter("distance"); 
                
                // add all the test labels 
                for( auto t: test)
                        if (t != 0) 
                                test_labels.insert(t); 
                
                for(auto tl: test_labels) {
                        auto rl = translate_label(tl, lm);
                        auto test_maskp = get_mask(test, tl); 
                        const C3DBitImage& test_mask = dynamic_cast<const C3DBitImage&>(*test_maskp); 
                        C3DFImage ref_mask = get_ref_distance(ref, rl, *distance);
                        
                        CDistanceResult distances; 
                        auto it = test_mask.begin(); 
                        auto et = test_mask.end(); 
                        auto ir = ref_mask.begin(); 
                        
                        while (it != et) {
                                if (*it) 
                                        distances.push_back(*ir); 
                                ++it; 
                                ++ir; 
                        }
                        result[tl] = distances;
                }
                return result; 
        }
}; 

FEvaluateDistances::FEvaluateDistances(const CLabelMap& lm):
        m_lm(lm)
{
}

template <typename T, typename S> 
CDistanceResultMap FEvaluateDistances::operator ()(const T3DImage<T>& test, const T3DImage<S>& ref) const
{
        const auto T_integral =is_integral<T>::value;  
        const auto S_integral =is_integral<S>::value;  
        
        return __dispatch_filter<T, S, T_integral, S_integral>::apply(test, ref, m_lm); 
}


int do_main( int argc, char *argv[] )
{
	string in_filename;
	string ref_filename;

	string label_translate_filename;

	string out_filename;

	const auto& imageio = C3DImageIOPluginHandler::instance();

	stringstream filter_names;

	CCmdOptionList options(g_description);

	options.set_group("File-IO"); 
	options.add(make_opt( in_filename, "in-labels", 'i', "input label image", 
			      CCmdOption::required, &imageio));
	options.add(make_opt( ref_filename, "ref-labels", 'r', "reference label image",
			      CCmdOption::required, &imageio));

        options.add(make_opt( label_translate_filename, "label-map", 'l', "optional mapping of label numbers"));

        options.add(make_opt( out_filename, "out-file", 'o', "output file name to write the distances to. "
                              "The output file is a csv file, containing distances listed for each label."));
        

	if (options.parse(argc, argv) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 

        auto test_labels = load_image3d(in_filename); 
        auto ref_labels = load_image3d(ref_filename);
        
        CLabelMap lmap; 
        if (!label_translate_filename.empty()) {
                ifstream ifs(label_translate_filename); 
                lmap = CLabelMap(ifs); 
        }
        
        FEvaluateDistances evaluater(lmap); 
        auto result = mia::filter(evaluater, *test_labels, *ref_labels); 
        
        // save result 
        ofstream outf(out_filename); 
        if (outf.bad())
                throw create_exception<runtime_error>("Error opening file '", out_filename, "' for writing"); 
        
        for (auto i : result)
                outf << i.first << "," << i.second.size() << "," << i.second << "\n";

        if (outf.bad())
                throw create_exception<runtime_error>("Error opening file '", out_filename, "' for writing"); 
        outf.close(); 
	return EXIT_SUCCESS;

}

#include <mia/internal/main.hh>
MIA_MAIN(do_main)
