/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
 *
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
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

#include <fstream>
#include <mia/core/msgstream.hh>
#include <mia/core/utils.hh>
#include <mia/3d/transformfactory.hh>

#include <mia/3d/transform/rigid.hh>

NS_MIA_BEGIN
using namespace boost::lambda;
using namespace std;

P3DImage C3DRigidTransformation::apply(const C3DImage& image,
					const C3DInterpolatorFactory& ipf) const
{
	if (image.get_size() != get_size()) {
		cvwarn() << "C3DRigidTransformation::apply: size of input differs from transformation target size\n";
	}
	return transform3d(image, ipf, *this);
}

C3DFVector C3DRigidTransformation::apply(const C3DFVector& x) const
{
	return transform(x);
}



C3DFVector C3DRigidTransformation::transform(const C3DFVector& x)const
{
	if (!_M_matrix_valid)
		evaluate_matrix();

	return C3DFVector(
		_M_t[0] * x.x + _M_t[1] * x.y + _M_t[2] * x.z + _M_t[3],
		_M_t[4] * x.x + _M_t[5] * x.y + _M_t[6] * x.z + _M_t[7],
		_M_t[8] * x.x + _M_t[9] * x.y + _M_t[10] * x.z + _M_t[11]);
}

C3DRigidTransformation::C3DRigidTransformation(const C3DBounds& size):
	_M_t(12),
	_M_size(size),
	_M_translation(0.0, 0.0, 0.0),
	_M_rotation(0.0,0.0,0.0),
	_M_matrix_valid(false)
{
}

C3DRigidTransformation::C3DRigidTransformation(const C3DRigidTransformation& other):
	_M_t(other._M_t),
	_M_size(other._M_size),
	_M_translation(other._M_translation),
	_M_rotation(other._M_rotation),
	_M_matrix_valid(_M_matrix_valid)
{
}

C3DTransformation *C3DRigidTransformation::do_clone()const
{
	return new C3DRigidTransformation(*this);
}

C3DTransformation *C3DRigidTransformation::invert()const
{
	
	C3DRigidTransformation *result = new C3DRigidTransformation(*this); 
	cverr() << "to be implemented\n"; 
	result->_M_matrix_valid = false; 
	return result;
}


C3DRigidTransformation::C3DRigidTransformation(const C3DBounds& size,const C3DFVector& translation,
					       const C3DFVector& rotation):
	_M_t(12),
	_M_size(size),
	_M_translation(translation),
	_M_rotation(rotation),
	_M_matrix_valid(false)
{
}

bool C3DRigidTransformation::save(const std::string& filename, const std::string& /*type*/) const
{
	ofstream file(filename.c_str());
	file << "Transformation: 3D\n"
	     << "Matrix: ";
	for (size_t i = 0; i < 12 ; ++i)
		file << _M_t[i] << " ";
	file << "\n";
	return file.good();
}

size_t C3DRigidTransformation::degrees_of_freedom() const
{
	return 6;
}

void C3DRigidTransformation::update(float /*step*/, const C3DFVectorfield& /*a*/)
{
	assert(!"not implemented");
}

void C3DRigidTransformation::translate(float x, float y, float z)
{
	_M_matrix_valid = false;
	_M_translation.x +=  x;
	_M_translation.y +=  y;
	_M_translation.z +=  z;
}

void C3DRigidTransformation::rotate(float xy, float xz, float yz)
{
	_M_matrix_valid = false;
	_M_rotation.z += xy;
	_M_rotation.y += xz;
	_M_rotation.x += yz;
	float sx,cx,sy,cy,sz,cz; 
	sincosf(_M_rotation.z, &sz, &cz); 
	sincosf(_M_rotation.y, &sy, &cy);
	sincosf(_M_rotation.x, &sx, &cx);

	C3DFVector t(cy*cz * _M_translation.x 
		     - (cx*sz-cz*sx*sy) * _M_translation.y 
		     + (sx*sz-cx*cz*sy) * _M_translation.z, 
		     cy*sz * _M_translation.x 
		     + (cx*cz-sx*sy*sz) * _M_translation.y 
		     - (cx*sy*sz+cz*sx) * _M_translation.z, 
		     sy* _M_translation.x  
		     + cy*sx  * _M_translation.y + cx*cy * _M_translation.z); 
	
	_M_translation = t;
}

gsl::DoubleVector C3DRigidTransformation::get_parameters() const
{
	gsl::DoubleVector result(degrees_of_freedom());
	result[0] = _M_translation.x;
	result[1] = _M_translation.y;
	result[2] = _M_translation.z;
	result[3] = _M_rotation.x;
	result[4] = _M_rotation.y;
	result[5] = _M_rotation.z;

	return result;
}

void C3DRigidTransformation::set_parameters(const gsl::DoubleVector& params)
{
	assert(degrees_of_freedom() == params.size());

	_M_translation.x = params[0];
	_M_translation.y = params[1];
	_M_translation.z = params[2];
	_M_rotation.x    = params[3];
	_M_rotation.y    = params[4];
	_M_rotation.z    = params[5];
	cvdebug() << "Rigid transform = (" << _M_translation << ", " << _M_rotation << ")\n"; 

	_M_matrix_valid = false;
}

float C3DRigidTransformation::divergence() const
{
	return 0.0;
}

float C3DRigidTransformation::grad_divergence() const
{
	return 0.0;
}


float C3DRigidTransformation::grad_curl() const
{
	return 0.0;
}


float C3DRigidTransformation::curl() const
{
	// this is not right
	return 0.0;
}

double C3DRigidTransformation::get_divcurl_cost(double /*wd*/, double /*wr*/, gsl::DoubleVector& /*gradient*/) const
{
	return 0.0; 
}

double C3DRigidTransformation::get_divcurl_cost(double /*wd*/, double /*wr*/) const
{
	return 0.0; 
}


const C3DBounds& C3DRigidTransformation::get_size() const
{
	return _M_size;
}

P3DTransformation C3DRigidTransformation::do_upscale(const C3DBounds& size) const
{
	C3DFVector new_trans(float(size.x) / (float)get_size().x * _M_translation.x,
			     float(size.y) / (float)get_size().y * _M_translation.y,
			     float(size.z) / (float)get_size().z * _M_translation.z);
	return P3DTransformation(new C3DRigidTransformation(size, new_trans, _M_rotation));
}

C3DFMatrix C3DRigidTransformation::derivative_at(int /*x*/, int /*y*/, int /* y */) const
{
	if (!_M_matrix_valid)
		evaluate_matrix();

	return C3DFMatrix(
		C3DFVector(_M_t[0], _M_t[4], _M_t[8]), 
		C3DFVector(_M_t[1], _M_t[5], _M_t[9]), 
		C3DFVector(_M_t[2], _M_t[6], _M_t[10]));
}

void C3DRigidTransformation::set_identity()
{
	cvdebug() << "set identity\n";
	_M_translation = C3DFVector(); 
	_M_rotation    = C3DFVector(); 
}

void C3DRigidTransformation::evaluate_matrix() const
{
	float sx,cx,sy,cy,sz,cz; 
	sincosf(_M_rotation.z, &sz, &cz); 
	sincosf(_M_rotation.y, &sy, &cy);
	sincosf(_M_rotation.x, &sx, &cx);

	_M_t[0] =    cy*cz;
	_M_t[1] =  - cx*sz-cz*sx*sy; 
	_M_t[2] =    sx*sz-cx*cz*sy;
	_M_t[3] = _M_translation.x;
	
	_M_t[4] = cy*sz;
	_M_t[5] = cx*cz-sx*sy*sz;
	_M_t[6] = - cx*sy*sz-cz*sx;
	_M_t[7] = _M_translation.y; 

	_M_t[8] = sy;
	_M_t[9] = cy*sx;
	_M_t[10]= cx*cy;
	_M_t[11]= _M_translation.z; 

	_M_matrix_valid = true;
}

float C3DRigidTransformation::get_max_transform() const
{
	if (!_M_matrix_valid) 
		evaluate_matrix(); 
	C3DFVector corners[7] = {
		C3DFVector(get_size().x, 0, 0), 
		C3DFVector(get_size().x, get_size().y,            0), 
		C3DFVector(           0, get_size().y,            0), 
		C3DFVector(           0, get_size().y, get_size().z), 
		C3DFVector(get_size().x,            0, get_size().z), 
		C3DFVector(           0,            0, get_size().z), 
		C3DFVector(get_size())
	};

	float result = apply(C3DFVector()).norm2(); 
	for(int i = 0; i < 7; ++i) {
		float h = (apply(corners[i]) - corners[i]).norm2(); 
		if (result < h) 
			result = h; 
	}

	return sqrt(result);

}

void C3DRigidTransformation::add(const C3DTransformation& /*other*/)
{
	// *this  = other * *this
	assert(0 && "not implemented");
}

C3DFVector C3DRigidTransformation::operator () (const C3DFVector& x) const
{
	return apply(x); 
}

float C3DRigidTransformation::get_jacobian(const C3DFVectorfield& /*v*/, float /*delta*/) const
{
	assert(!"not implemented");
}

void C3DRigidTransformation::translate(const C3DFVectorfield& gradient, gsl::DoubleVector& params) const
{
	//

	assert(gradient.get_size() == _M_size);
	assert(params.size() == degrees_of_freedom());

	vector<double> r(params.size(), 0.0);

	auto g = gradient.begin();
	for (size_t z = 0; z < _M_size.z; ++z)
		for (size_t y = 0; y < _M_size.y; ++y)
			for (size_t x = 0; x < _M_size.x; ++x, ++g) {
				r[0] += g->x;
				r[1] += g->y;
				r[2] += g->z;
				r[3] += -z * g->y + y * g->z; 
				r[4] += -z * g->x + x * g->z; 
				r[5] += -y * g->x + x * g->y; 
			}
	copy(r.begin(), r.end(), params.begin()); 
}



C3DRigidTransformation::iterator_impl::iterator_impl(const C3DBounds& pos, const C3DBounds& size, 
						      const C3DRigidTransformation& trans):
	C3DTransformation::iterator_impl(pos, size),
	_M_trans(trans), 
	_M_value(trans.transform(C3DFVector(pos)))
{
	_M_dx = _M_trans.transform(C3DFVector(pos.x + 1.0, pos.y, pos.z)) - _M_value;
}

C3DTransformation::iterator_impl * C3DRigidTransformation::iterator_impl::clone() const
{
	return new iterator_impl(get_pos(), get_size(), _M_trans); 
}

const C3DFVector&  C3DRigidTransformation::iterator_impl::do_get_value()const
{
	return _M_value; 
}

void C3DRigidTransformation::iterator_impl::do_x_increment()
{
	_M_value += _M_dx; 
}

void C3DRigidTransformation::iterator_impl::do_y_increment()
{
	_M_value = _M_trans.transform(C3DFVector(get_pos())); 
	_M_dx = _M_trans.transform(C3DFVector(get_pos().x + 1.0, get_pos().y, get_pos().z)) - _M_value;
}

void C3DRigidTransformation::iterator_impl::do_z_increment()
{
	_M_value = _M_trans.transform(C3DFVector(get_pos())); 
	_M_dx = _M_trans.transform(C3DFVector(get_pos().x + 1.0, get_pos().y, get_pos().z)) - _M_value;
}


C3DTransformation::const_iterator C3DRigidTransformation::begin() const
{
	if (!_M_matrix_valid) 
		evaluate_matrix(); 
	return C3DTransformation::const_iterator(new iterator_impl(C3DBounds(0,0,0), get_size(), *this)); 
}

C3DTransformation::const_iterator C3DRigidTransformation::end() const
{
	return C3DTransformation::const_iterator(new iterator_impl(get_size(), get_size(), *this)); 
}

float C3DRigidTransformation::pertuberate(C3DFVectorfield& /*v*/) const
{
	assert(!"not implemented");
}

class C3DRigidTransformCreator: public C3DTransformCreator {
	virtual P3DTransformation do_create(const C3DBounds& size) const;
};

P3DTransformation C3DRigidTransformCreator::do_create(const C3DBounds& size) const
{
	return P3DTransformation(new C3DRigidTransformation(size));
}

class C3DRigidTransformCreatorPlugin: public C3DTransformCreatorPlugin {
public:
	typedef C3DTransformCreatorPlugin::ProductPtr ProductPtr;

	C3DRigidTransformCreatorPlugin();
	virtual ProductPtr do_create() const;
	virtual bool do_test() const;
	const std::string do_get_descr() const;
};

C3DRigidTransformCreatorPlugin::C3DRigidTransformCreatorPlugin():
	C3DTransformCreatorPlugin("rigid")
{
}

C3DRigidTransformCreatorPlugin::ProductPtr
C3DRigidTransformCreatorPlugin::do_create() const
{
	return ProductPtr(new C3DRigidTransformCreator());
}

bool C3DRigidTransformCreatorPlugin::do_test() const
{
	return true;
}

const std::string C3DRigidTransformCreatorPlugin::do_get_descr() const
{
	return "plugin to create rigid transformations";
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C3DRigidTransformCreatorPlugin();
}


NS_MIA_END