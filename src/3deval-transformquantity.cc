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

#define VSTREAM_DOMAIN "3deval-transformquantity"

#include <fstream>
#include <ostream>
#include <cstring>
#include <cerrno>
#include <iterator>

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
#include <mia/3d/imageio.hh>
#include <mia/3d/datafield.cxx>
#include <miaconfig.h>


using namespace std;
using namespace mia;

const SProgramDescription g_description = {
	{pdi_group,"Tools for the Analysis of 3D image series"},
	{pdi_short, "Evaluate derivative of a transformation"}, 
	{pdi_description,"Get derivative a transformation obtained by by using image registration for "
	 "any given positions in 3D. "
	 "The position data is given in CSV format:\n"
	 "   id;time;x;y;z;reserved\n"
	 "The output data will be stored in a binary file with an ascii header describing the data. "
	 "An example header is given below:\n\n"
	 "MIA\n" 
	 "tensorfield {\n"
	 "  dim=3          #number of dimensions of the data\n"
	 "  components=13  #number of components per element\n"
	 "  component_description=vector3,scalar,matrix3x3 #interpretation of components \n"
	 "  elements=20    #number of elements in file\n"
	 "  interpretation=strain #interpretation of elements (strain|derivative)\n"
	 "  style=sparse   #storage style (sparse|grid) \n"
	 "  repn=float32   #representation of values \n"
	 "  size=1000 1000 200 #grid size of the transformation\n"
	 "  endian=low     #endianess of binary data (low|big) \n" 
	 "}\n\n"
	 
	 "This example header has to be interpreted like follows: three-dimensional data, each entry consists of 13 values "
	 "the values etry consists of a 3D vector, a scalar, and a 3x3 matrix (saved in row-major format)."
	 "The data records represent strain tensors, and only a sparse set of points is given. "
	 "The values are given as single floating point (32 bit). "
	 "The original transformation field corresponds to images of "
	 "1000x1000x200 voxels and the binary data is stored in low endian format." 
	}, 
	{pdi_example_descr,"Derivative of the transformation change.v3df for each  point given in input.csv are "
	 "evaluated and written to output.s."}, 
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
	FQuantityEvaluator(const C3DTransformation& t):
		m_t(t) {}; 
	virtual C3DFMatrix operator ()(const C3DFVector& p) const = 0; 
	virtual C3DFMatrix operator ()(const C3DBounds& p) const = 0; 
protected: 
	const C3DTransformation& m_t; 
}; 

struct FDerivativeEvaluator : public FQuantityEvaluator {
	FDerivativeEvaluator(const C3DTransformation& t):
		FQuantityEvaluator(t){}
	
	virtual C3DFMatrix operator ()(const C3DFVector& p) const {
		return m_t.derivative_at(p);
	}
	virtual C3DFMatrix operator ()(const C3DBounds& p) const {
		return m_t.derivative_at(p.x, p.y, p.z);
	}
}; 

struct FStrainEvaluator : public FQuantityEvaluator {
	FStrainEvaluator(const C3DTransformation& t):
		FQuantityEvaluator(t){}
	
	virtual C3DFMatrix  operator ()(const C3DFVector& p) const {
		auto m = m_t.derivative_at(p);
		return m * m.transposed() - C3DFMatrix::_1;
	}
	virtual C3DFMatrix operator ()(const C3DBounds& p) const {
		auto m = m_t.derivative_at(p.x, p.y, p.z);
		return m * m.transposed() - C3DFMatrix::_1;	
	}
}; 

// ensure the data type is packed tightly  
#pragma pack(4)
struct SSparseStracPoint {
	C3DFVector pos; 
	float time; 
	C3DFMatrix tensor; 
	SSparseStracPoint(const C3DFVector& _pos, float _time, const C3DFMatrix& _tensor):
		pos(_pos), time(_time), tensor(_tensor){
	}
}; 
#pragma pack()

struct FQuantityEvaluatorSparse {
	FQuantityEvaluatorSparse(const FQuantityEvaluator& qe):
		m_qe(qe) {
	}
	SSparseStracPoint operator ()(const C3DTrackPoint& p) const {
		return SSparseStracPoint(p.get_pos(), p.get_time(), m_qe(p.get_pos())); 
	}					
protected: 
	const FQuantityEvaluator& m_qe; 
}; 

int do_main( int argc, char *argv[] )
{
	// Parameters 
	string in_filename;
	string out_filename;
	string trans_filename; 
	
        EQuantity quantity = tq_strain;

	// Option setup 
	CCmdOptionList options(g_description);
	
	options.set_group("\nFile-IO"); 
	options.add(make_opt( in_filename, "in-file", 'i', 
			      "input point set, if this parameter is given a sparse evaluation "
			      "of the quantity will be done, otherwise the quantity is evalutated "
			      "for each grid point of the transformation range.", CCmdOptionFlags::input));
	options.add(make_opt( out_filename, "out-file", 'o', 
			      "output strains file, for a format description see above.", CCmdOptionFlags::required_output)); 
	
	options.add(make_opt( trans_filename, "transformation", 't', "transformation of which the quantity will be evaluated.", 
			      CCmdOptionFlags::required_input, &C3DTransformationIOPluginHandler::instance())); 

	options.set_group("\nParameters"); 
	options.add(make_opt( quantity, tqmap, "quantity", 'q', 
			      "Specify the quantity to be evaluated at the given points")); 

	if (options.parse(argc, argv) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 


	// read the transformation 
	auto t = C3DTransformationIOPluginHandler::instance().load(trans_filename);

	
	unique_ptr<FQuantityEvaluator> qe;
	switch (quantity) {
	case tq_derivative:
		qe.reset(new FDerivativeEvaluator(*t)); 
		break;
	case tq_strain: 
		qe.reset(new FStrainEvaluator(*t)); 
		break; 
	default: 
		throw invalid_argument("Unknown quantity evaluator specified"); 
	}


	// this splitting off of the saving is ugly, could be solved by template specialization 
	COutputFile output(out_filename); 
	if (!output) 
		throw create_exception<runtime_error>("Unable to open '", out_filename, "' for writing:",  strerror(errno)); 
	
	
	fprintf(output, "MIA\n"); 
	fprintf(output, "tensorfield {\n"); 
	fprintf(output, "  dim=3\n"); 
        fprintf(output, "  interpretation=%s\n", tqmap.get_name(quantity)); 
	fprintf(output, "  repn=float32\n");
	const auto size = t->get_size(); 
	fprintf(output, "  size=%d %d %d\n", size.x, size.y, size.z); 

#ifdef WORDS_BIGENDIAN
	fprintf(output, "  endian=big\n" ); 
#else
	fprintf(output, "  endian=low\n" ); 
#endif

	int element_size = 0; 
	string components; 
	string style;  
	
	if (in_filename.empty()) {
		auto size = t->get_size(); 
		vector <C3DFMatrix> tensorfield(size.product());
		auto iout = tensorfield.begin(); 
		
		C3DBounds pos; 
		for (pos.z = 0; pos.z < size.z; ++pos.z)
			for (pos.y = 0; pos.y < size.y; ++pos.y)
				for (pos.x = 0; pos.x < size.x; ++pos.x, ++iout)
					*iout = (*qe)(pos);
		
		element_size = sizeof(C3DFMatrix)/sizeof(float);
		components.assign("matrix3x3"); 
		style.assign("grid"); 

		unsigned int nelements = static_cast<unsigned int>(tensorfield.size()); 
		
		fprintf(output, "  components=%d\n", element_size); 
		fprintf(output, "  component_description=%s\n", components.c_str()); 
		fprintf(output, "  elements=%d\n", nelements); 
		fprintf(output, "  style=%s\n", style.c_str()); 
		fprintf(output, "}\n" );
		
		if (fwrite(&tensorfield[0], sizeof(C3DFMatrix),  tensorfield.size(), output) != tensorfield.size())
			throw create_exception<runtime_error>("Unable to write data to '", out_filename, "':", 
							      strerror(errno));

	}else{
		vector< C3DTrackPoint > trackpoints = C3DTrackPoint::load_trackpoints(in_filename); 
		vector<SSparseStracPoint> tensorfield; 
		tensorfield.reserve(trackpoints.size()); 
		
		FQuantityEvaluatorSparse translater(*qe); 
		transform(trackpoints.begin(), trackpoints.end(), back_inserter(tensorfield), translater); 
		
		element_size = sizeof(SSparseStracPoint)/sizeof(float); 
		components.assign("vector3,scalar,matrix3x3"); 
		style.assign("sparse"); 

		unsigned int nelements = static_cast<unsigned int>(tensorfield.size()); 
	
		fprintf(output, "  components=%d\n", element_size); 
		fprintf(output, "  component_description=%s\n", components.c_str()); 
		fprintf(output, "  elements=%d\n", nelements); 
		fprintf(output, "  style=%s\n", style.c_str()); 
		fprintf(output, "}\n" );
		
		if (fwrite(&tensorfield[0], sizeof(SSparseStracPoint),  tensorfield.size(), output) != tensorfield.size())
			throw create_exception<runtime_error>("Unable to write data to '", out_filename, "':", 
							      strerror(errno));
						      
	}

	return EXIT_SUCCESS;	
}

MIA_MAIN(do_main);
	

