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

#include <mia/core.hh>
#include <mia/mesh/filter.hh>
#include <mia/internal/main.hh>

NS_MIA_USE;
using namespace std;


const SProgramDescription g_general_help = {
        {pdi_group, "Creation, analysis, and filtering of triangular 3D meshes"}, 
	{pdi_short, "Run filters on a 3D mesh."}, 
	{pdi_description, "This program runs a series filters on a given input mesh. The filters "
	 "are given as extra parameters on the command line and are run in the order in which they "
	 "are given. To obtain a list of available filters you may run\n"
	 "   'mia-plugin-help mesh/filter'\n"
	 "from the command line"}, 
	{pdi_example_descr, "Run an isotropic scaling on input.vmesh to fit it into the box defined "
	 "by the corners <0,0,0> - <128,128,128> and save the result to output.vmesh"}, 
	{pdi_example_code, "-i input.vmesh -o output.vmesh scale:strategy=iso-fitbox,box-start=[<0,0,0>],box-end=[<128,128,128>]"}
}; 

int do_main( int argc, char *argv[] )
{

	string in_filename;
	string out_filename;

	const auto& filter_plugins = CMeshFilterPluginHandler::instance();
	const auto& meshio = CMeshIOPluginHandler::instance(); 

	CCmdOptionList options(g_general_help);
	options.add(make_opt( in_filename, "in-file", 'i', "input mesh to be filtered", CCmdOptionFlags::required_input, &meshio));
	options.add(make_opt( out_filename, "out-file", 'o', "output mesh that have been filtered", CCmdOptionFlags::required_output, &meshio));
	
	if (options.parse(argc, argv, "filter", &filter_plugins) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 


	//CHistory::instance().append(argv[0], "unknown", options);
	
	const CMeshFilterChain filter_chain(options.get_remaining()); 
	auto mesh = meshio.load(in_filename); 
	if (!mesh) 
		throw create_exception<invalid_argument>( "No mesh found in '", in_filename, "'"); 
	
	auto out_mesh = filter_chain.run(mesh);

	if ( !meshio.save(out_filename, *out_mesh) )
		throw create_exception<runtime_error>( "Unable to save result to '", out_filename, "'");

	return EXIT_SUCCESS;

}

MIA_MAIN(do_main); 
