/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004 - 2010
 *
 * Max-Planck-Institute for Human Cognitive and Brain Science	
 * Max-Planck-Institute for Evolutionary Anthropology 
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PUcRPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>


#include <mia/core.hh>
#include <mia/core/errormacro.hh>
#include <mia/3d/3dvfio.hh>


NS_MIA_USE; 

typedef unsigned int uint32; 

void FillField0(C3DFVectorfield& Field, const C3DBounds& Size, float /*fun*/)
{
	auto ri = Field.begin();
	float xdef = Size.x / 32.0;
	float ydef = Size.y / 32.0;
	float zdef = Size.z / 32.0;
	float xHelp = M_PI/(Size.x-1);
	float yHelp = M_PI/(Size.y-1);
	float zHelp = M_PI/(Size.z-1);
	
	for (uint32 z = 0; z < Size.z; z++){
		float zfactor = sin(zHelp * z);
		for (uint32 y = 0; y < Size.y; y++){
			float yfactor=sin(yHelp * y) * zfactor;
			for (uint32 x = 0; x < Size.x; x++,ri++){
				float factor = sin(xHelp * x) * yfactor;
				(*ri).x = factor * xdef;     ;
				(*ri).y = factor * ydef;
				(*ri).z = factor * zdef;
			}
		}
	}
}

void FillField1(C3DFVectorfield& Field,const C3DBounds& Size, float fun)
{
	float xfun = fun * Size.x / 32.0; 
	float yfun = fun * Size.y / 32.0; 
	float zfun = fun * Size.z / 32.0; 

	auto  ri = Field.begin();
	float xHelp = M_PI/(Size.x-1);
	float yHelp = M_PI/(Size.y-1);
	float zHelp = M_PI/(Size.z-1);
	
	for (uint32 z = 0; z < Size.z; z++){
		float zfactor = sin(zHelp * z) * sin(fun * zHelp * z);
		for (uint32 y = 0; y < Size.y; y++){
			float yfactor= sin (yHelp * y) * sin(fun * yHelp * y ) * zfactor ;
			for (uint32 x = 0; x < Size.x; x++,ri++){
				float factor = (sin(xHelp * x) * sin(fun * xHelp * x)) * yfactor;
				ri->x = factor * xfun;     
				ri->y = factor * yfun;
				ri->z = factor * zfun;
			}
		}
	}
}

void FillField2(C3DFVectorfield& Field,const C3DBounds& Size,float fun)
{
	auto  ri = Field.begin();
	float xdef = Size.x / 4.0;
	float ydef = Size.y / 4.0;
	float zdef = Size.z / 4.0;
	float xHelp = 2 * M_PI/(Size.x-1);
	float yHelp = 2 * M_PI/(Size.y-1);
	float zHelp = 2 * M_PI/(Size.z-1);
	
	for (uint32 z = 0; z < Size.z; z++){
		float zfactor = sin(zHelp * z) * cos(fun * zHelp *z);
		for (uint32 y = 0; y < Size.y; y++){
			float yfactor=(sin( yHelp *y)* cos(fun* yHelp *y) )* zfactor ;
			for (uint32 x = 0; x < Size.x; x++,ri++){
				float factor = (sin( xHelp *x) * cos(fun * xHelp * x)) * yfactor;
				(*ri).x = factor * xdef;     ;
				(*ri).y = factor * ydef;
				(*ri).z = factor * zdef;
			}
		}
	}
}

const char program_info[] = "This is a program to create 3D vector fields."; 

int do_main(int argc, const char *args[])
{

	C3DBounds Size(64,64,64);
	
	short fieldtype = 0;
	float funfactor = 1.0;
	
	string out_filename;

	
	CCmdOptionList options(program_info);

	options.push_back(make_opt( out_filename, "out-file", 'o', "output file for the vector field", 
				    CCmdOption::required));
	options.push_back(make_opt( Size, "size", 's', "size of the vector field"));
	options.push_back(make_opt( fieldtype, "ftype", 'y', "vector field type")); 
	options.push_back(make_opt( funfactor, "fun", 'f', "just some parameter to vary the field ;-)")); 
	
	options.parse(argc, args, false);

	C3DFVectorfield Field(Size);
	switch (fieldtype){
	case 0:FillField0(Field,Size,funfactor);break;
	default:FillField1(Field,Size,funfactor);break;
	}
	
	if (!C3DVFIOPluginHandler::instance().save("", out_filename, Field)){
		THROW(runtime_error, "Unable to save result vector field to " << out_filename << "\n"); 
	}
	return EXIT_SUCCESS;	

}

int main(int argc, const char *argv[])
{
	try {
		return do_main(argc, argv);
	}
	catch (const runtime_error &e){
		cerr << argv[0] << " runtime: " << e.what() << endl;
	}
	catch (const invalid_argument &e){
		cerr << argv[0] << " error: " << e.what() << endl;
	}
	catch (const std::exception& e){
		cerr << argv[0] << " error: " << e.what() << endl;
	}
	catch (...){
		cerr << argv[0] << " unknown exception" << endl;
	}

	return EXIT_FAILURE;
}