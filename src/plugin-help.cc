/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2011 Gert Wollny
 *
 * This program is free software; you can redistribute it and/or modify
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */


#include <config.h>

#include <map>
#include <iostream>
#include <mia.hh>



using namespace std; 
NS_MIA_USE;


const char *g_program_group = "Help"; 

const char *g_general_help = 
	"This program is used to print out help about the availabel plug-ins\n"; 

const char *g_program_example_descr = 
	"Print out the help about the 2D filter plugins .\n"; 
const char *g_program_example_code = "filter/2d"; 

const SProgramDescrption description = {
	g_program_group, 
	g_general_help, 
	g_program_example_descr, 
	g_program_example_code
}; 


map<string, const CPluginHandlerBase*> collect_handlers()
{
	map<string, const CPluginHandlerBase*> result; 
	result[C1DSpacialKernelPluginHandler::instance().get_search_descriptor()] =&C1DSpacialKernelPluginHandler::instance();
	result[C2DFifoFilterPluginHandler::instance().get_search_descriptor()] =&C2DFifoFilterPluginHandler::instance();
	result[C2DFilterPluginHandler::instance().get_search_descriptor()] =&C2DFilterPluginHandler::instance();
	result[C2DFullCostPluginHandler::instance().get_search_descriptor()] =&C2DFullCostPluginHandler::instance();
	result[C2DImageCombinerPluginHandler::instance().get_search_descriptor()] =&C2DImageCombinerPluginHandler::instance();
	result[C2DImageCostPluginHandler::instance().get_search_descriptor()] =&C2DImageCostPluginHandler::instance();
	result[C2DImageCreatorPluginHandler::instance().get_search_descriptor()] =&C3DImageCreatorPluginHandler::instance();
	result[C2DImageIOPluginHandler::instance().get_search_descriptor()] =&C2DImageIOPluginHandler::instance();
	result[C2DRegModelPluginHandler::instance().get_search_descriptor()] =&C2DRegModelPluginHandler::instance();
	result[C2DShapePluginHandler::instance().get_search_descriptor()] =&C2DShapePluginHandler::instance();
	result[C2DStackDistanceTransformIOPluginHandler::instance().get_search_descriptor()] =&C2DStackDistanceTransformIOPluginHandler::instance();
	result[C2DTransformationIOPluginHandler::instance().get_search_descriptor()] =&C2DTransformationIOPluginHandler::instance();
	result[C2DVFIOPluginHandler::instance().get_search_descriptor()] =&C2DVFIOPluginHandler::instance();
	result[C3DFatImageCostPluginHandler::instance().get_search_descriptor()] =&C3DFatImageCostPluginHandler::instance();
	result[C3DFilterPluginHandler::instance().get_search_descriptor()] =&C3DFilterPluginHandler::instance();
	result[C3DFullCostPluginHandler::instance().get_search_descriptor()] =&C3DFullCostPluginHandler::instance();
	result[C3DImageCombinerPluginHandler::instance().get_search_descriptor()] =&C3DImageCombinerPluginHandler::instance();
	result[C3DImageCostPluginHandler::instance().get_search_descriptor()] =&C3DImageCostPluginHandler::instance();
	result[C3DImageCreatorPluginHandler::instance().get_search_descriptor()] =&C3DImageCreatorPluginHandler::instance();
	result[C3DImageIOPluginHandler::instance().get_search_descriptor()] =&C3DImageIOPluginHandler::instance();
	result[C3DRegModelPluginHandler::instance().get_search_descriptor()] =&C3DRegModelPluginHandler::instance();
	result[C3DRegTimeStepPluginHandler::instance().get_search_descriptor()] =&C3DRegTimeStepPluginHandler::instance();
	result[C3DShapePluginHandler::instance().get_search_descriptor()] =&C3DShapePluginHandler::instance();
	result[C3DTransformationIOPluginHandler::instance().get_search_descriptor()] =&C3DTransformationIOPluginHandler::instance();
	result[C3DTransformCreatorHandler::instance().get_search_descriptor()] =&C3DTransformCreatorHandler::instance();
	result[C3DVFIOPluginHandler::instance().get_search_descriptor()] =&C3DVFIOPluginHandler::instance();
	result[CCST2DImgKernelPluginHandler::instance().get_search_descriptor()] =&CCST2DImgKernelPluginHandler::instance();
	result[CCST2DVectorKernelPluginHandler::instance().get_search_descriptor()] =&CCST2DVectorKernelPluginHandler::instance();
	result[CFFT2DKernelPluginHandler::instance().get_search_descriptor()] =&CFFT2DKernelPluginHandler::instance();
	result[CMeshIOPluginHandler::instance().get_search_descriptor()] =&CMeshIOPluginHandler::instance();
	result[CMinimizerPluginHandler::instance().get_search_descriptor()] =&CMinimizerPluginHandler::instance();
	result[CNoiseGeneratorPluginHandler::instance().get_search_descriptor()] =&CNoiseGeneratorPluginHandler::instance();
	result[CSplineBoundaryConditionPluginHandler::instance().get_search_descriptor()] =&CSplineBoundaryConditionPluginHandler::instance();
	result[CSplineKernelPluginHandler::instance().get_search_descriptor()] =&CSplineKernelPluginHandler::instance();
	
	return result; 
}



int main( int argc, const char *argv[] )
{
	try {

		CCmdOptionList options(description);
		if (options.parse(argc, argv, "plugin-descriptor") != CCmdOptionList::hr_no)
			return EXIT_SUCCESS; 
		
		auto handlers = collect_handlers(); 
		
		if (argc == 1) {
			clog << "The following plugin-descriptors are available. To list the plug-ins of a descriptor "; 
			clog << "run: 'mia-plugin-help <descriptor>'\n\n"; 
			for (auto h = handlers.begin(); h != handlers.end(); ++h) {
				clog << "  " << h->first << "\n"; 
			}
			clog << "\n"; 
			return EXIT_SUCCESS; 
		}

		auto descriptors = options.get_remaining(); 
		for(auto i = descriptors.begin(); i != descriptors.end(); ++i) {
			auto h = handlers.find(*i);
			if (h == handlers.end()) {
				cerr << "No help available for '" << *i  << "'\n"; 
			}else {
				clog << "Available plug-ins for '" << *i << "':\n"; 
				h->second->print_short_help(clog);
				clog << "\n";
			}
		}
		return EXIT_SUCCESS; 
	}
	catch (const runtime_error &e){
		cerr << argv[0] << " runtime: " << e.what() << endl;
	}
	catch (const invalid_argument &e){
		cerr << argv[0] << " invalid argument: " << e.what() << endl;
	}
	catch (const exception& e){
		cerr << argv[0] << " error: " << e.what() << endl;
	}
	catch (...){
		cerr << argv[0] << " unknown exception" << endl;
	}

	return EXIT_FAILURE;
}
