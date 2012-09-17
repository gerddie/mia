/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2012 Gert Wollny, David Pastor 
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

#define VSTREAM_DOMAIN "3deval-transformquantity"

#include <fstream>
#include <ostream>

#include <mia/core/filter.hh>
#include <mia/core/msgstream.hh>
#include <mia/core/cmdlineparser.hh>
#include <mia/core/errormacro.hh>
#include <mia/internal/main.hh>
#include <mia/3d/transformio.hh>
#include <mia/3d/trackpoint.hh>
#include <mia/core/cmdlineparser.hh>
#include <mia/core/msgstream.hh>
#include <mia/core/file.hh>
#include <mia/3d/transformio.hh>
#include <mia/3d/3dimageio.hh>
#include <mia/3d/3DDatafield.cxx>
#include <miaconfig.h>

#include <cstring>
#include <cerrno>

using namespace std;
using namespace mia;

const SProgramDescription g_description = {
	{pdi_group,"Tools for the Analysis of 3D image series"},
	{pdi_short, "Evaluate derivative of a transformation"}, 
	{pdi_description,"Get derivative a transformation obtained by by using image registration for any given positions in 3D. "
		 "The position data is given in CSV format:\n"
		 "   id;time;x;y;z;reserved\n\n"}, 
	{pdi_example_descr,"Derivative of the transformation change.v3df for each selected point are "
		 "evaluated and written to output.s "}, 
	{pdi_example_code,"-i input.csv -o output.s --transformation change.v3df"}
}; 

enum EQuantity { tq_derivative, tq_strain, tq_unknown }; 

static const TDictMap<EQuantity>::Table table[] = {
	{"derivative", tq_derivative, "Evaluate the transformation derivative at the given points"},
	{"strain", tq_strain, "Evaluate the strain tensor at the given points"},
	{NULL, tq_unknown, ""}
};
const TDictMap<EQuantity> tqmap(table);


struct FQuantityEvaluator {
	virtual void apply(C3DFMatrix& m) const = 0; 
}; 

struct FDerivativeEvaluator : public FQuantityEvaluator {
	virtual void apply(C3DFMatrix& MIA_PARAM_UNUSED(m)) const {
	}
}; 

struct FStrainEvaluator : public FQuantityEvaluator {
	virtual void  apply(C3DFMatrix& m) const {
		m = m * m.transposed() - C3DFMatrix::_1;
	}
}; 
 

int do_main( int argc, char *argv[] )
{
	// Parameters 
	string in_filename;
	string out_filename;
	string trans_filename; 
	float time_step = 1.0; 
	
        EQuantity quantity = tq_strain;

	// Option setup 
	CCmdOptionList options(g_description);
	
	options.set_group("\nFile-IO"); 
	options.add(make_opt( in_filename, "in-file", 'i', 
				    "input point set", CCmdOption::required));
	options.add(make_opt( out_filename, "out-file", 'o', 
				    "output strains file", CCmdOption::required)); 

	options.add(make_opt( trans_filename, "transformation", 't', 
				    "transformation describing the monitored change", CCmdOption::required)); 

	options.set_group("\nParameters"); 
	options.add(make_opt( quantity, tqmap, "quantity", 'q', 
			      "Specify the quantity to be evaluated at the given points")); 
	
	options.add(make_opt( time_step, "time-step", 'T', 
			      "time step to use for the position update")); 
	
	if (options.parse(argc, argv) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 


	// read the transformation 
	auto t = C3DTransformationIOPluginHandler::instance().load(trans_filename);

	// read the trackpoints 
	vector< C3DTrackPoint > trackpoints = load_trackpoints(in_filename); 
	
	unique_ptr<FQuantityEvaluator> qe;
	switch (quantity) {
	case tq_derivative:
		qe.reset(new FDerivativeEvaluator()); 
		break;
	case tq_strain: 
		qe.reset(new FStrainEvaluator()); 
		break; 
	default: 
		throw invalid_argument("Unknown quantity evaluator specified"); 
	}
	
	vector<float> tensorfield(trackpoints.size() * 9);
	auto o = tensorfield.begin(); 
	
	for(auto p = trackpoints.begin(); p!=trackpoints.end(); ++p, o+=9){
		
		auto m = t->derivative_at(p->get_pos());
	        qe->apply(m); 
		
		o[0] = m.x.x; 
		o[1] = m.x.y; 
		o[2] = m.x.z; 
		o[3] = m.y.x; 
		o[4] = m.y.y; 
		o[5] = m.y.z; 
		o[6] = m.z.x; 
		o[7] = m.z.y; 
		o[8] = m.z.z;
	}

	//Write the tensors as the given format...
	COutputFile output(out_filename); 
	if (!output) 
		throw create_exception<runtime_error>("Unable to open '", out_filename, "' for writing:", 
							      strerror(errno));
	

	fprintf(output, "MIA\n"); 
	fprintf(output, "tensorfield {\n"); 
	fprintf(output, "  dim=3\n"); 
	fprintf(output, "  components=9\n");
	fprintf(output, "  elements=%ld\n", trackpoints.size()); 
        fprintf(output, "  interpretation=%s", tqmap.get_name(quantity)); 
        fprintf(output, "  pointsfile=%s", in_filename.c_str());
	fprintf(output, "  repn=float32\n");
	
	// do we really need this? 
	const auto size = t->get_size(); 
	fprintf(output, "  size=%d %d %d\n", size.x, size.y, size.z); 

	#ifdef WORDS_BIGENDIAN
	fprintf(output, "  endian=big\n" ); 
	#else
	fprintf(output, "  endian=low\n" ); 
	#endif
	fprintf(output, "}\n\xC" );

	if (fwrite(&tensorfield[0], sizeof(float),  tensorfield.size(), output) != tensorfield.size()) {
		throw create_exception<runtime_error>("Unable to write data to '", out_filename, "':", 
						      strerror(errno));
	}
	
	return EXIT_SUCCESS;	

}

MIA_MAIN(do_main);
	

