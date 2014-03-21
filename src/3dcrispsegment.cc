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
#include <mia/core/filter.hh>
#include <mia/core/cmdlineparser.hh>


using namespace std;
NS_MIA_USE;

const SProgramDescription g_description = {
        {pdi_group, "Analysis, filtering, combining, and segmentation of 3D images"}, 
	{pdi_short, "Label segmentation according to probabilities."}, 
	{pdi_description, "This program creates a label image from a fuzzy segmentation. "
	 "or each pixel the label is set to the class with the higest probability plus a given offset"}, 
	{pdi_example_descr, "Set the labels based on the input class probability file cls.v "
	 "adding the offset 2 to each label and save the result to labeled.v."}, 
	{pdi_example_code, "-i cls.v -o labeled.v -l 2" }
}; 

class C3DLabelAccumulator: public TFilter<void> {
public: 
	C3DLabelAccumulator(const C3DImage& prototype, unsigned int offset); 

	template <typename T>
	void operator () (const T3DImage<T>& image); 

	C3DImage& get_result(); 
private: 
	C3DUBImage m_labels;
	C3DDImage m_prob; 
        unsigned int m_offset; 
}; 

C3DLabelAccumulator::C3DLabelAccumulator(const C3DImage& prototype, unsigned int offset):
	m_labels(prototype.get_size(), prototype), 
	m_prob(prototype.get_size()), 
	m_offset(offset)
{
}

C3DImage& C3DLabelAccumulator::get_result()
{
	return m_labels; 
}

template <typename T>
void C3DLabelAccumulator::operator () (const T3DImage<T>& image)
{
	if (image.get_size() != m_labels.get_size())
		throw create_exception<invalid_argument>("Input images are not of the same size (got ", image.get_size(), 
								 " expected ", m_labels.get_size(), ")"); 

	auto p = m_prob.begin(); 
	auto l = m_labels.begin(); 
	auto i = image.begin(); 
	auto e = image.end(); 

	while (i != e) {
		if (*i > *p) {
			*l = m_offset; 
			*p = *i; 
		}
		++i; ++p; ++l; 
	}
	++m_offset; 
}


int do_main( int argc, char *argv[] )
{
	string in_filename;
        string out_filename;
        unsigned int label_offset = 1; 

	const auto& imageio = C3DImageIOPluginHandler::instance();

	CCmdOptionList options(g_description);
	options.add(make_opt( in_filename, "in-file", 'i', "input class file, should contain multiple images "
			      "with tissue class probabilities", 
			      CCmdOptionFlags::required_input, &imageio));
	options.add(make_opt( out_filename, "out-file", 'o', "output class label image", 
			      CCmdOptionFlags::required_output, &imageio));

	options.add(make_opt( label_offset, "label-offset", 'l', "label offset"));

	if (options.parse(argc, argv) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 


	if ( in_filename.empty() )
		throw runtime_error("'--in-image' ('i') option required");

	if ( out_filename.empty() )
		throw runtime_error("'--out-image' ('o') option required");

	// read image
	auto  in_image_list = imageio.load(in_filename);
	if (!in_image_list) 
		throw create_exception<runtime_error>("Unable to read 3D image list from '", in_filename, "'");

	if (in_image_list->empty()) 
		throw create_exception<runtime_error>("Got empty image list from '", in_filename, "'");
	
        C3DLabelAccumulator acc(*(*in_image_list)[0], label_offset); 
	for_each( in_image_list->begin(), in_image_list->end(), [&acc](P3DImage i){mia::accumulate(acc, *i);});
	
	if ( !save_image(out_filename, acc.get_result()) ){
		throw create_exception<runtime_error>("Unable to save result to '", out_filename, "'");
	}
	
	return EXIT_SUCCESS;

}

#include <mia/internal/main.hh>
MIA_MAIN(do_main)

