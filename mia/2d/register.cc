/* -*- mona-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
 * Max-Planck-Institute for Human Cognitive and Brain Science	
 * Max-Planck-Institute for Evolutionary Anthropology 
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

/*
  This program implements 2D gray scale image registration. 
  Depending on the available plug-ins it can accomodate various models and cost-functions. 
  So far input images can be given as PNG, TIF, BMP and OpenEXR (depending on the installed plug-ins)
  The input images must be of the same dimensions and gray scale (whatever bit-depth). 
  The vector field will be saved as a EXR image with two frames of float values, one for the X 
  and one for the Y values of the vectors. 
  Other vector field output plug-ins might be supported depending on the installed plug-ins. 
*/

#include <mia/core.hh>
#include <mia/2d.hh>
#include <sstream>
#include <iomanip>
#include <mia/2d/register.hh>
#include <boost/algorithm/minmax_element.hpp>

NS_MIA_USE
using namespace boost; 
using namespace std; 


#ifdef WIN32
#define REGMOVE_IMGTYPE "bmp"
#else
#define REGMOVE_IMGTYPE "png"
#endif

// an image copy operator that keeps the type of the internal images 
struct FImageCopy: public TFilter<P2DImage> {
	template <typename T>
	P2DImage operator ()( const T2DImage<T>& image) const {
		return P2DImage(new T2DImage<T>(image)); 
	}
}; 

// concat two vector fields: a := a + b 
// to accumulate a deformation in 'sum' in the used reference frame you have
// do the following: 
// a := a + sum; sum := a
// b := b + sum; sum := b  
//  
// C = B(A(x)) -> A
// A(x) = x- a(x), B(x) = x - b(x)
// B(A(x)) = x - a(x) - b(x - a(x))
// x - c(x) = x - a(x) - b(x-a(x)) 
// a(x) - c(x) = - b(x-a(x))
// c(x) - a(x) = b(x-a(x))
// a_new(x) = c(x) = a(x) + b(x-a(x))
// a(x) +=  b(x-a(x))
// 

void cat (C2DFVectorfield& a, const C2DFVectorfield& b)
{
	

	assert(a.get_size() == b.get_size());
	
	C2DFVectorfield::iterator i = a.begin();
	for (size_t y = 0; y < a.get_size().y; ++y)  {
		for (size_t x = 0; x < a.get_size().x; ++x,++i)  {
			C2DFVector xi = C2DFVector(x, y) - *i;
			*i += b.get_interpol_val_at(xi);
		}
	}
}

/*
  a := A(B(x)) 

  B(x) = x - b(x)
  A(x) = x - a(x)
  C(x) = x - c(x)
  C(x) = A(B(x)) = A(x-b(x)) = x - b(x) - a(x - b(x))
  x - c(x) = x - b(x) - a(x - b(x))
  c(x) = b(x) + a(x-b(x))
*/


// space saving method of cat2 with overwriting both fields
void cat3 (const C2DFVectorfield& a, const C2DFVectorfield& b, C2DFVectorfield& r)
{
	assert(a.get_size() == b.get_size());
	assert(a.get_size() == r.get_size());
	
	C2DFVectorfield::iterator ir = r.begin();
	C2DFVectorfield::const_iterator ib = b.begin(); 

	for (size_t y = 0; y < a.get_size().y; ++y)  {
		for (size_t x = 0; x < a.get_size().x; ++x, ++ir, ++ib)  {
			C2DFVector xi = C2DFVector(x, y) - *ib;
			*ir = a.get_interpol_val_at(xi) +  *ib;
		}
	}
}


// 
// upscale a vector field by using tri-linear interpolation 
// and scaling of the vectors 
P2DFVectorfield upscale( const C2DFVectorfield& vf, C2DBounds size)
{
	P2DFVectorfield Result(new C2DFVectorfield(size));
	float x_mult = float(size.x) / (float)vf.get_size().x; 
	float y_mult = float(size.y) / (float)vf.get_size().y;
	float ix_mult = 1.0f / x_mult; 
	float iy_mult = 1.0f / y_mult; 
	
	C2DFVectorfield::iterator i = Result->begin();
	
	for (unsigned int y = 0; y < size.y; y++){
		for (unsigned int x = 0; x < size.x; x++,++i){
			C2DFVector help(ix_mult * x, iy_mult * y);
			C2DFVector val = vf.get_interpol_val_at(help);
			*i = C2DFVector(val.x * x_mult,val.y * y_mult);
		}
	}
	return Result;
}


C2DImageRegister::C2DImageRegister(size_t start_size, const C2DImageCost& cost, size_t max_iter, 
				   C2DRegModel& model, C2DRegTimeStep& time_step, 
				   const C2DInterpolatorFactory&  ipf, float outer_epsilon, bool save_steps, P2DImage movie
				):
	_M_start_size(start_size), 
	_M_cost(cost), 
	_M_max_iter(max_iter), 
	_M_model(model), 
	_M_time_step(time_step), 
	_M_ipf(ipf),
	_M_outer_epsilon(outer_epsilon), 
	_M_save_steps(save_steps), 
	_M_mnum(0),
	_M_movie(movie)

{
	if ( _M_start_size <= 0 ) 
		throw invalid_argument("C2DImageRegister: start size must be greater than 0"); 
		
}

static size_t log2(size_t x)
{
	size_t result = 0; 
	while (x >>= 1)
		++result; 
	return result; 
}

// This operator takes care of the multi-resolution loop of the registration 
P2DFVectorfield C2DImageRegister::operator () (const C2DImage& source, const C2DImage& reference)
{
	const C2DFilterPluginHandler::Instance& fph = C2DFilterPluginHandler::instance(); 

	if (source.get_size() != reference.get_size())
		throw invalid_argument("only registration of images of the same size is allowed"); 


	size_t x_shift = log2(source.get_size().x / _M_start_size); 
	size_t y_shift = log2(source.get_size().y / _M_start_size);

	if (_M_movie.get()) {
		stringstream regmov;
		C2DImageVector vimg;
		vimg.push_back(_M_movie); 
		regmov << "regmov" << setw(5) << setfill('0') << _M_mnum++ << "."REGMOVE_IMGTYPE;  
		C2DImageIOPluginHandler::instance().save(REGMOVE_IMGTYPE, regmov.str(), vimg); 
	}
	
	P2DFVectorfield result; 

	while (x_shift || y_shift) {
		C2DBounds block_size(1 << x_shift, 1 << y_shift); 
		stringstream downscalerstr; 
		downscalerstr << "downscale:bx=" << (1 << x_shift) << ",by=" << (1 << y_shift); 
		SHARED_PTR(C2DFilter> downscaler )= fph.produce(downscalerstr.str().c_str()); 
		
		P2DImage src_scaled = downscaler->filter(source); 
		P2DImage ref_scaled = downscaler->filter(reference);

		if (!result)
			result = P2DFVectorfield(new C2DFVectorfield(src_scaled->get_size()));
		else 
			result = upscale( *result, src_scaled->get_size() );

		if (_M_time_step.has_regrid()) {		
			reg_level_regrid(*src_scaled, *ref_scaled, *result);
		}else {
			reg_level(*src_scaled, *ref_scaled, *result);
		}
		
		if (x_shift)
			--x_shift; 

		if (y_shift)
			--y_shift; 
	}
	
	
	if (!result) {
		result = P2DFVectorfield(new C2DFVectorfield(source.get_size()));
	}else {
		result = upscale( *result, source.get_size());
	}
	if (_M_time_step.has_regrid()) {		
		reg_level_regrid(source, reference, *result); 
	}else
		reg_level(source, reference, *result); 

		
	return result; 
}


// this functions does the registration on one multi-resolution level
void C2DImageRegister::reg_level_regrid(const C2DImage& source, const C2DImage& reference, C2DFVectorfield& result)

{
	double cost_value; 
	size_t iter = 0; 
	size_t inertia = 5; 
	bool better = false; 

	double initial_cost; 

	C2DGridTransformation local_shift(source.get_size()); 
	C2DGridTransformation best_local_shift(source.get_size()); 

	local_shift.set_identity(); 
	
	P2DImage temp = filter(FDeformer2D(result, _M_ipf), source);
	double new_cost_value = initial_cost = _M_cost.value(*temp, reference); 
	
	C2DFVectorfield v(source.get_size()); 
	C2DFVectorfield force(source.get_size());
	

	const char lend = cverb.get_level() == vstream::ml_debug ? '\n' : '\r'; 
	cvmsg() << reference.get_size() << "@" << iter << "|cost:" << new_cost_value << lend;

	float best_cost_value = new_cost_value; 

	do {
		++iter; 
		cost_value = new_cost_value; 

		force.clear(); 
		_M_cost.evaluate_force(*temp, reference, _M_model.get_force_scale(), force);
				
		// solve for the force to get a velocity or deformation field
		v.clear(); 
		_M_model.solve(force, v); 

		// obtain the maximum shift. v might be updated according to the time step model
		float maxshift = _M_time_step.calculate_pertuberation(v, local_shift); 
		if (maxshift <= 0) {
			cvmsg() << "zero time step, perfect registration?\n"; 
			break; 
		}

		// get the time step based on the maximum shift
		float delta = _M_time_step.get_delta(maxshift); 
		cvdebug() << "maxshift = " << maxshift << " Timestep = " << delta << "\n"; 
		
		// now check, whether the new deformation would result in a 
		// non-topologic distortion of the image. if so, regrid
		// some time step models might always return false 
		if (_M_time_step.regrid_requested(local_shift, v, delta)) {
			result.add(local_shift); 
			local_shift.set_identity();
			better = false;
		}

		local_shift.update(delta, v); 
		temp = filter(FDeformer2D(result + local_shift, _M_ipf), source);
		
		
		if (_M_movie.get()) {
			stringstream regmov; 
			C2DImageVector vimg;
			P2DFVectorfield scaled(upscale(force, _M_movie->get_size()));
			vimg.push_back(filter(FDeformer2D(*scaled, _M_ipf), *_M_movie)); 
			regmov << "regmov" << setw(5) << setfill('0') << _M_mnum++ << "."REGMOVE_IMGTYPE;  
			C2DImageIOPluginHandler::instance().save(REGMOVE_IMGTYPE, regmov.str(), vimg); 
		}
		

		new_cost_value = _M_cost.value(*temp, reference); 
		
		// if the cost is reduced, we allow for some search steps 
		// with increasing cost again, and we save the current best deformation 
		if (new_cost_value < best_cost_value) {
			better = true; 
			best_local_shift = local_shift; 
			best_cost_value = new_cost_value; 
			inertia = 5; 
		}

		// if the cost reduction was big, increase the possible time step 
		if (new_cost_value < cost_value * 0.9)
			_M_time_step.increase(); 
		
		// if the cost increased, reduce the time step if possible, 
		// or count down the allowed search in sub-optimal areas
		if (new_cost_value > best_cost_value)
			if (!_M_time_step.decrease())
				--inertia; 
		
		cvmsg() << iter << "@" << reference.get_size() 
			<< ": best ratio=" << best_cost_value / initial_cost 
			<< ", absolute cost=" << new_cost_value << "                     "
			<< lend;
		
	} while ( ((inertia > 0) || (new_cost_value < best_cost_value)) && 
		  new_cost_value / initial_cost > _M_outer_epsilon && iter < _M_max_iter);

	cvmsg() << '\n'; 
	
	// if re-gridding is possible, we need to accumulate the deformation one last time
	
	if (better) {
		cvdebug() << "reg_level: sum the final result\n";  
		result += best_local_shift; 
	}
}

struct FMinMax: public TFilter <pair<float, float> > {
	template <typename T>
	pair<float, float> operator ()(const T2DImage<T>& image) const {
		pair<typename T2DImage<T>::const_iterator, typename T2DImage<T>::const_iterator> 
			minmax = ::boost::minmax_element(image.begin(), image.end()); 
		return pair<float, float>(*minmax.first, *minmax.second); 
	}
}; 


// this functions does the registration on one multi-resolution level
void C2DImageRegister::reg_level(const C2DImage& source, const C2DImage& reference, C2DFVectorfield& result)

{
	double cost_value; 
	size_t iter = 0; 
	size_t inertia = 5; 
//	bool better = false; 

	pair<float, float> mm = filter( FMinMax(), source); 
	float force_scale =_M_model.get_force_scale() / ((mm.second - mm.first)* (mm.second - mm.first)); 
	
	double initial_cost; 
	
	P2DImage temp = filter(FDeformer2D(result, _M_ipf), source);
	double new_cost_value = initial_cost = _M_cost.value(*temp, reference); 
	
	C2DFVectorfield force(source.get_size());
	C2DFVectorfield u(source.get_size()); 
	C2DFVectorfield best_result(result); 
	
	const char lend = cverb.get_level() == vstream::ml_debug ? '\n' : '\r'; 
	cvmsg() << reference.get_size() << "@" << iter << "|cost:" << new_cost_value << lend;

	float best_cost_value = new_cost_value; 

	float maxshift = _M_time_step.calculate_pertuberation(result, result); 
	float delta = _M_time_step.get_delta(1.0 + maxshift); 
	do {
		++iter; 
		cost_value = new_cost_value; 

		force.clear(); 
		_M_cost.evaluate_force(*temp, reference, delta * force_scale, force);
				
		_M_model.solve(force, u);
		
		maxshift = _M_time_step.calculate_pertuberation(u, u); 
		if (maxshift <= 0) {
			cvmsg() << "zero time step, perfect registration?\n"; 
			break; 
		}

		// get the time step based on the maximum shift
		delta = _M_time_step.get_delta(1.0/iter + maxshift); 
		cvdebug() << "Timestep = " << delta << "\n"; 
		
		_M_time_step.apply(u, result, delta);
				

		temp = filter(FDeformer2D(result, _M_ipf), source);
				
		if (_M_save_steps) {
			stringstream regmov; 
			C2DImageVector vimg; 
			vimg.push_back(temp); 
			regmov << "regmov" << setw(5) << setfill('0') << _M_mnum++ << "."REGMOVE_IMGTYPE;  
			C2DImageIOPluginHandler::instance().save(REGMOVE_IMGTYPE, regmov.str(), vimg); 
		}

		new_cost_value = _M_cost.value(*temp, reference); 
		
		if (new_cost_value < best_cost_value) {
			best_cost_value = new_cost_value; 
			best_result = result; 
			inertia = 5;
		}
			

		// if the cost reduction was big, increase the possible time step 
		if (new_cost_value < cost_value * 0.9) {
			_M_time_step.increase(); 
			
		}
		
		// if the cost increased, reduce the time step if possible, 
		// or count down the allowed search in sub-optimal areas
		if (new_cost_value >= cost_value)
			if (!_M_time_step.decrease()) {
				--inertia; 
			}
		
		cvmsg() << iter << "@" << reference.get_size() 
			<< ": best ratio=" << best_cost_value / initial_cost 
			<< ", current =" << new_cost_value / cost_value << "                     "
			<< lend;
		
	} while ( inertia > 0 && new_cost_value / initial_cost > _M_outer_epsilon && iter < _M_max_iter);

	cvmsg() << '\n'; 
	
	result = best_result; 
	if (_M_save_steps) {
		stringstream regmov; 
		C2DImageVector vimg;
		vimg.push_back(filter(FDeformer2D(result, _M_ipf), source)); 
		regmov << "regmov" << setw(5) << setfill('0') << --_M_mnum << "final."REGMOVE_IMGTYPE;
		_M_mnum++; 
		C2DImageIOPluginHandler::instance().save(REGMOVE_IMGTYPE, regmov.str(), vimg); 
	}
}

C2DMultiImageRegister::C2DMultiImageRegister(size_t start_size, size_t max_iter, 
					     C2DRegModel& model, C2DRegTimeStep& time_step, 
					     float outer_epsilon):
	_M_start_size(start_size),
	_M_max_iter(max_iter),  
	_M_model(model), 
	_M_time_step(time_step), 
	_M_outer_epsilon(outer_epsilon)
{
}

P2DTransformation C2DMultiImageRegister::operator () (C2DImageFatCostList& cost, P2DInterpolatorFactory ipf)
{
	TRACE("C2DMultiImageRegister::operator"); 


	size_t x_shift = log2(cost.get_size().x / _M_start_size); 
	size_t y_shift = log2(cost.get_size().y / _M_start_size);


	P2DTransformation result(new C2DGridTransformation(C2DBounds(0,0))); 

	while (x_shift || y_shift) {
		C2DBounds block_size(1 << x_shift, 1 << y_shift); 
		
		C2DImageFatCostList cost_scaled = cost.get_downscaled(block_size);
		
		result = result->upscale(cost_scaled.get_size()); 
		

		C2DGridTransformation& current_result = dynamic_cast<C2DGridTransformation&>(*result); 
		if (_M_time_step.has_regrid()) {		
			reg_level_regrid(cost_scaled, current_result, *ipf);
		}else {
			reg_level(cost_scaled, current_result, *ipf);
		}
		
		if (x_shift)
			--x_shift; 

		if (y_shift)
			--y_shift; 
	}
	
	result = result->upscale(cost.get_size()); 

	{
		C2DGridTransformation& current_result = dynamic_cast<C2DGridTransformation&>(*result); 
		
		if (_M_time_step.has_regrid()) {		
			reg_level_regrid(cost, current_result, *ipf); 
		}else
			reg_level(cost, current_result, *ipf); 
	}
		
	return result; 
	
}

void C2DMultiImageRegister::reg_level_regrid(C2DImageFatCostList& cost, C2DGridTransformation& result, const C2DInterpolatorFactory& ipf)
{
	TRACE("C2DMultiImageRegister::reg_level"); 
	double cost_value; 
	size_t iter = 0; 
	size_t inertia = 5; 
	bool better = false; 

	double initial_cost; 

	C2DGridTransformation local_shift(cost.get_size()); 
	C2DGridTransformation best_local_shift(cost.get_size()); 

	// local_shift.clear(); 
	
	cost.transform(result, ipf); 
	double new_cost_value = initial_cost = cost.value(); 
	
	C2DFVectorfield v(cost.get_size()); 
	C2DFVectorfield force(cost.get_size());
	

	const char lend = cverb.get_level() == vstream::ml_debug ? '\n' : '\r'; 
	cvmsg() << cost.get_size() << "@" << iter << "|cost:" << new_cost_value << lend;


	float best_cost_value = new_cost_value; 

	float force_scale = _M_model.get_force_scale(); 
	do {
		++iter; 
		cost_value = new_cost_value; 
		
		force.clear(); 
		cost.evaluate_force(force);
		
		for (C2DFVectorfield::iterator i = force.begin(); i != force.end(); ++i)
			*i *= force_scale; 

		// solve for the force to get a velocity or deformation field
		//v.clear(); 
		_M_model.solve(force, v); 

		// obtain the maximum shift. v might be updated according to the time step model
		float maxshift = _M_time_step.calculate_pertuberation(v, local_shift); 
		if (maxshift <= 0) {
			cvmsg() << "zero time step, perfect registration?\n"; 
			break; 
		}

		// get the time step based on the maximum shift
		float delta = _M_time_step.get_delta(maxshift); 
		cvdebug() << "maxshift = " << maxshift << " Timestep = " << delta << "\n"; 
		
		// now check, whether the new deformation would result in a 
		// non-topologic distortion of the image. if so, regrid
		// some time step models might always return false 
		if (_M_time_step.regrid_requested(local_shift, v, delta)) {
			result.add(local_shift); 
			fill(local_shift.field_begin(), local_shift.field_end(), C2DFVector(0.0f, 0.0f)); 
			better = false;
		}
		local_shift.update(delta, v);

		C2DGridTransformation current_trans = result + local_shift;

		cost.transform(current_trans, ipf);
				
		new_cost_value = cost.value(); 
		
		// if the cost is reduced, we allow for some search steps 
		// with increasing cost again, and we save the current best deformation 
		if (new_cost_value < best_cost_value) {
			better = true; 
			best_local_shift = local_shift; 
			best_cost_value = new_cost_value; 
			inertia = 5; 
		}

		// if the cost reduction was big, increase the possible time step 
		if (new_cost_value < cost_value * 0.9)
			_M_time_step.increase(); 
		
		// if the cost increased, reduce the time step if possible, 
		// or count down the allowed search in sub-optimal areas
		if (new_cost_value > best_cost_value)
			if (!_M_time_step.decrease())
				--inertia; 
		
		cvmsg() << iter << "@" << cost.get_size() 
			<< ": best ratio=" << best_cost_value / initial_cost 
			<< ", absolute cost=" << new_cost_value << "                     "
			<< lend;
		
	} while ( ((inertia > 0) || (new_cost_value < best_cost_value)) && 
		  new_cost_value / initial_cost > _M_outer_epsilon && iter < _M_max_iter);

	cvmsg() << '\n'; 
	
	// if re-gridding is possible, we need to accumulate the deformation one last time
	
	if (better) {
		cvdebug() << "reg_level: sum the final result\n";  
		
		result += best_local_shift; 
	}

}

void C2DMultiImageRegister::reg_level(C2DImageFatCostList& cost, C2DGridTransformation& result, const C2DInterpolatorFactory& ipf)
{
	TRACE("C2DMultiImageRegister::reg_level"); 
	double cost_value; 

	size_t iter = 0; 
	size_t inertia = 5; 
	bool better = false; 

	double initial_cost; 

	C2DGridTransformation local_shift(cost.get_size()); 
	C2DGridTransformation best_local_shift(cost.get_size()); 

	cost.transform(result, ipf);
	
	double new_cost_value = initial_cost = cost.value(); 
	
	C2DFVectorfield v(cost.get_size()); 
	C2DFVectorfield force(cost.get_size());
	

	const char lend = cverb.get_level() == vstream::ml_debug ? '\n' : '\r'; 
	cvmsg() << cost.get_size() << "@" << iter << "|cost:" << new_cost_value << lend;

	float best_cost_value = new_cost_value; 

	float force_scale = _M_model.get_force_scale(); 
	cvmsg() << "Force scale = " << force_scale << "\n"; 

	do {
		++iter; 
		cost_value = new_cost_value; 

		cost.evaluate_force(force);
		for (C2DFVectorfield::iterator i = force.begin(); i != force.end(); ++i)
			*i *= force_scale; 

				
		// solve for the force to get a velocity or deformation field
		fill(v.begin(), v.end(), C2DFVector(0.0f, 0.0f)); 
		_M_model.solve(force, v); 

		// obtain the maximum shift. v might be updated according to the time step model
		float maxshift = _M_time_step.calculate_pertuberation(v, local_shift); 
		if (maxshift <= 0) {
			cvmsg() << "zero time step, perfect registration?\n"; 
			break; 
		}

		// get the time step based on the maximum shift
		float delta = _M_time_step.get_delta(maxshift); 
		cvdebug() << "maxshift = " << maxshift << " Timestep = " << delta << "\n"; 
		
		local_shift.update(delta, v); 
	
	
		cost.transform(result +  local_shift, ipf); 

		new_cost_value = cost.value(); 
		
		// if the cost is reduced, we allow for some search steps 
		// with increasing cost again, and we save the current best deformation 
		if (new_cost_value < best_cost_value) {
			better = true; 
			best_local_shift = local_shift; 
			best_cost_value = new_cost_value;
			inertia = 5; 
		}

		// if the cost reduction was big, increase the possible time step 
		if (new_cost_value < cost_value * 0.9)
			_M_time_step.increase(); 
		
		// if the cost increased, reduce the time step if possible, 
		// or count down the allowed search in sub-optimal areas
		if (new_cost_value > best_cost_value)
			if (!_M_time_step.decrease())
				--inertia; 
		
		cvmsg() << iter << "@" << cost.get_size() 
			<< ": best ratio=" << best_cost_value / initial_cost 
			<< ", absolute cost=" << new_cost_value << "                     "
			<< lend;
		
	} while ( ((inertia > 0) || (new_cost_value < best_cost_value)) && 
		  new_cost_value / initial_cost > _M_outer_epsilon && iter < _M_max_iter);

	cvmsg() << '\n'; 
	
	// if re-gridding is possible, we need to accumulate teh deformation one last time
	
	if (better) {
		cvdebug() << "reg_level: sum the final result\n";  
		
		result.add(best_local_shift); 
	}
}
