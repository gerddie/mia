/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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

const SProgramDescription description = {
	{pdi_group, "Help"}, 
	{pdi_short, "Print help about plug-in groups"},
	{pdi_description, "This program is used to print out help about the availabel plug-ins\n"}, 
	{pdi_example_descr, "Print out the help about the 2D filter plugins .\n"},  
	{pdi_example_code, "filter/2d"}
}; 


map<string, const CPluginHandlerBase*> collect_handlers()
{
#define ADD(Handler) \
	result[Handler ().get_descriptor()] =&Handler ();

	map<string, const CPluginHandlerBase*> result; 


	ADD(C1DSpacialKernelPluginHandler::instance); 
	ADD(C1DSpacialKernelPluginHandler::instance);
	ADD(C2DFifoFilterPluginHandler::instance);
	ADD(C2DFilterPluginHandler::instance);
	ADD(C2DFullCostPluginHandler::instance);
	ADD(C2DImageCombinerPluginHandler::instance);
	ADD(C2DImageCostPluginHandler::instance);
	ADD(C2DImageCreatorPluginHandler::instance);
	ADD(C2DImageIOPluginHandler::instance);
	ADD(C2DRegModelPluginHandler::instance);
	ADD(C2DShapePluginHandler::instance);
	ADD(C2DStackDistanceTransformIOPluginHandler::instance);
	ADD(C2DTransformCreatorHandler::instance);
	ADD(C2DTransformationIOPluginHandler::instance);
	ADD(C2DVFIOPluginHandler::instance);
	ADD(C3DFilterPluginHandler::instance);
	ADD(C3DFullCostPluginHandler::instance);
	ADD(C3DImageCombinerPluginHandler::instance);
	ADD(C3DImageCostPluginHandler::instance);
	ADD(C3DImageCreatorPluginHandler::instance);
	ADD(C3DImageIOPluginHandler::instance);
	ADD(C3DRegModelPluginHandler::instance);
	ADD(C2DRegTimeStepPluginHandler::instance);
	ADD(C3DRegTimeStepPluginHandler::instance);
	ADD(C3DShapePluginHandler::instance);
	ADD(C3DTransformationIOPluginHandler::instance);
	ADD(C3DTransformCreatorHandler::instance);
	ADD(C3DVFIOPluginHandler::instance);
	ADD(CCST2DImgKernelPluginHandler::instance);
	ADD(CCST2DVectorKernelPluginHandler::instance);
	ADD(CFFT2DKernelPluginHandler::instance);
	ADD(CMeshIOPluginHandler::instance);
	ADD(CMinimizerPluginHandler::instance);
	ADD(CNoiseGeneratorPluginHandler::instance);
	ADD(CSplineBoundaryConditionPluginHandler::instance);
	ADD(CSplineKernelPluginHandler::instance);
	ADD(CMeshFilterPluginHandler::instance);

#undef ADD
	
	return result; 
}



int do_main( int argc, char *argv[] )
{
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

#include <mia/internal/main.hh>
MIA_MAIN(do_main); 
