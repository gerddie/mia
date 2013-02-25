/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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

#ifdef WIN32
#define EXPORT_DYNAMIC __declspec(dllexport)
#else
#define EXPORT_DYNAMIC
#endif


#include <mia/core.hh>
#include <mia/2d.hh>

#include "mextypemap.hh"

using namespace std; 
using namespace mia; 
namespace bfs = boost::filesystem; 

typedef void (*mexcallback)(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]);
typedef struct { 
	const char *name;  
	mexcallback callback;
} SCallbackMap; 

static void prepare_2dfilter_plugin_path()
{
	list< bfs::path> searchpath; 
	
	if (bfs::path::default_name_check_writable())
		bfs::path::default_name_check(::boost::filesystem::portable_posix_name); 
	searchpath.push_back(bfs::path("2d") / bfs::path("filter")); 
	searchpath.push_back(bfs::path("..")/ bfs::path("mia") / bfs::path("2d") / bfs::path("filter")); 
	
	C2DFilterPluginHandler::set_search_path(searchpath); 	
}

static void prepare_spacial_kernel_path()
{
	list< bfs::path> searchpath; 
	
	if (bfs::path::default_name_check_writable())
		bfs::path::default_name_check(::boost::filesystem::portable_posix_name); 
	searchpath.push_back(bfs::path("core")/bfs::path("spacialkernel"));
	searchpath.push_back(bfs::path("..")/ bfs::path("mia") / bfs::path("core")/bfs::path("spacialkernel"));

	C1DSpacialKernelPluginHandler::set_search_path(searchpath); 
}

static void mex_set_test (int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
	mexPrintf("MIAMEX - set test mode\n"); 
	
	prepare_2dfilter_plugin_path(); 
	prepare_spacial_kernel_path(); 
}	

static void mex_debuglevel (int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
	if (nrhs != 1)
		throw invalid_argument("mex_debuglevel: unknown number of input arguments"); 
	
	if (nlhs != 0)	
		throw invalid_argument("mex_debuglevel: no output argument supported "); 

	const char *level = mex_get_string(prhs[0]); 
	
	cverb.set_verbosity(g_verbose_dict.get_value(level)); 

}

static void mex_echo (int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
	if (nrhs != 1)
		throw invalid_argument("mex_echo: unknown number of input arguments"); 
	
	if (nlhs != 1)	
		throw invalid_argument("mex_echo: unknown number of output arguments"); 

	if( !mxIsInt32(prhs[0]) ) 
		throw invalid_argument("mex_echo: expect input type int32");

	int mrows = mxGetM(prhs[0]);
	int ncols = mxGetN(prhs[0]);
	
	if (mrows != 1 || ncols != 1)
		throw invalid_argument("mex_echo: only one single value expected");
	
	plhs[0] = mxCreateNumericMatrix(1,1, mxINT32_CLASS, mxREAL);
	
	int *x = (int *)mxGetData(prhs[0]);
	int *y = (int *)mxGetData(plhs[0]);
	y[0] = x[0]; 
}

void mex_filter2d(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]); 
void mex_reg2d(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]);
void mex_deform2d(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]); 
void mex_reg2d_multisource(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]); 

void mex_filter3d(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]); 
void mex_reg3d(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]); 
void mex_deform3d(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]);

const SCallbackMap callback_map[] = {
	{"echo",     mex_echo}, 
	{"set_test", mex_set_test},
	{"filter2d", mex_filter2d}, 
	{"reg2d",    mex_reg2d},
	{"mex_reg2d_multisource", mex_reg2d_multisource}, 
	{"deform2d", mex_deform2d},
	{"filter3d", mex_filter3d}, 
	{"reg3d",    mex_reg3d},
	{"deform3d", mex_deform3d},
	{"debug",    mex_debuglevel}, 
	{0, 0}
}; 

class mex_streamredir: public streamredir {
	virtual void do_put_buffer(const char *begin, const char *end); 
}; 

void mex_streamredir::do_put_buffer(const char *begin, const char *end)
{
	string buffer(begin, end); 
	mexPrintf("MIAMEX: %s\n", buffer.c_str());
}

class OutputRedirect : public ostream {
public:
	OutputRedirect(): ostream(new mex_streamredir())
	{
		vstream::instance().set_stream(*this);
	}
	~OutputRedirect()
	{
		vstream::instance().set_stream(std::cerr);
	}
};


extern "C"  EXPORT_DYNAMIC void miamex(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
	try {
		OutputRedirect oredir; 
		if (nrhs < 1) {
			mexErrMsgTxt("MIAMEX: no function given");
			return; 
		}
		
		const char *command = mex_get_string(prhs[0]); 
		const SCallbackMap *cm  = callback_map; 
		while (cm->name && strcmp(command, cm->name))
			++cm; 
		
		if (!cm->callback) {
			stringstream err; 
			err << "Function '" << command << "' not supported"; 
			throw invalid_argument(err.str());
		}
		cm->callback(nlhs, plhs, nrhs - 1, &prhs[1]);
	}
	catch (invalid_argument& x) {
		mexPrintf("MIAMEX - invalid argument:%s\n",x.what()); 
//		mexErrMsgTxt("ABORTED");
	}
	catch (runtime_error& x) {
		mexPrintf("MIAMEX - runtime error:%s\n",x.what()); 
//		mexErrMsgTxt("ABORTED");
	}
	catch (exception& x) {
		mexPrintf("MIAMEX - exception:%s\n",x.what()); 
//		mexErrMsgTxt("ABORTED");
	}
	catch (...) {
		mexErrMsgTxt("unknown exception\nABORTED");
	}
}
