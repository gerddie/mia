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

#define VSTREAM_DOMAIN "PLUGIN TEST"

#include <memory>
#include <mia/core.hh>


NS_MIA_USE
using namespace std;

const SProgramDescription g_general_help = {
	{pdi_group, "Test"}, 
	{pdi_head,"This program runs plugin-tests."}
};

bool uninstalled = false;
bool passed = true;

static void test_plugin(const char *modname)
{
	

	cvmsg() << "Testing '" << modname << "'\n";
	auto_ptr<CPluginModule> module(new CPluginModule(modname));
	CPluginBase *plugin = module->get_interface();
	if (!plugin) {
		cverr() <<"'"<< modname << "'  doesn't provide a plugin interface\n";
		return;
	}
	do {
		cvmsg() << "  '" << plugin->get_name() << "'\n";
		if ( !plugin->test(uninstalled) ) {
			cvfail() << "'"<< plugin->get_name() << "'  tests failed\n";
			passed = false;
		}
		plugin = plugin->next_interface();
	}while (plugin);
	delete plugin;
}

int do_main(int argc, char *argv[])
{
	
	CCmdOptionList options(g_general_help);
	options.add(make_opt( uninstalled, "uninstalled", 'u', "test uninstalled plugin", NULL));
	if (options.parse(argc, argv, "plugin"))
		return EXIT_SUCCESS; 
	
	for_each(options.get_remaining().begin(),
		 options.get_remaining().end(), test_plugin);
	
	return passed ? EXIT_SUCCESS : EXIT_FAILURE;

}

#include <mia/template/main.hh>
MIA_MAIN(do_main); 
