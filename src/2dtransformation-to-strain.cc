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

#include <mia/core/cmdlineparser.hh>
#include <mia/core/msgstream.hh>
#include <mia/core/file.hh>
#include <mia/2d/transformio.hh>
#include <mia/2d/imageio.hh>
#include <mia/2d/datafield.cxx>
#include <miaconfig.h>

#include <cstring>
#include <cerrno>



NS_MIA_USE
using namespace std;

const SProgramDescription g_description = {
        {pdi_group, "Registration, Comparison, and Transformation of 2D images"}, 
	{pdi_short, "Green strain tensor."}, 
	{pdi_description, "Evaluate the Green strain tensor corresponding to "
	 "a given 2D transformation for each grid point."}, 
	{pdi_example_descr, "Evaluate the Green strain tensor from the transformation stored in trans.v "
	 "and save it to output.v"}, 
	{pdi_example_code, "-i  trans.v  -o output.mt"}
}; 

int do_main(int argc, char **argv)
{
	CCmdOptionList options(g_description);

	string trans_filename;
	string out_filename;

	options.add(make_opt( trans_filename, "in-file", 'i', "input transformation", 
			      CCmdOptionFlags::required_input, &C2DTransformationIOPluginHandler::instance()));
	options.add(make_opt( out_filename, "out-file", 'o', "output Green's strain tensor", 
			      CCmdOptionFlags::required_output));

	if (options.parse(argc, argv) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 

	auto transformation = C2DTransformationIOPluginHandler::instance().load(trans_filename);

	const auto size = transformation->get_size(); 

	vector<float> tensorfield(size.product() * 4);
	auto o = tensorfield.begin(); 

	for (unsigned int y = 0; y < size.y; ++y) 
		for (unsigned int x = 0; x < size.x; ++x, o += 4) {
			auto d = transformation->derivative_at(x,y); 
			auto m = d * d.transposed() - C2DFMatrix::_1;
			
				o[0] = m.x.x; 
				o[1] = m.x.y; 
				o[2] = m.y.x; 
				o[3] = m.y.y; 
			}
	
	cvmsg() << "\ndone\n";

	// save 
	COutputFile output(out_filename); 
	if (!output) 
		throw create_exception<runtime_error>( "Unable to open '", out_filename, "' for writing:", strerror(errno)); 
	

	fprintf(output, "MIA\n"); 
	fprintf(output, "tensorfield {\n"); 
	fprintf(output, "  dim=2\n"); 
	fprintf(output, "  components=4\n");
	fprintf(output, "  repn=float32\n");
	fprintf(output, "  size=%d %d\n", size.x, size.y); 

#ifdef WORDS_BIGENDIAN
	fprintf(output, "  endian=big\n" ); 
#else
	fprintf(output, "  endian=low\n" ); 
#endif
	fprintf(output, "}\n\xC" );

	if (fwrite(&tensorfield[0], sizeof(float),  tensorfield.size(), output) != tensorfield.size()) {
		throw create_exception<runtime_error>( "Unable to write data to '", out_filename, "':", strerror(errno)); 
	}
	
	return EXIT_SUCCESS;
}

#include <mia/internal/main.hh>
MIA_MAIN(do_main)
