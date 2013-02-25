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

#include <boost/algorithm/minmax_element.hpp>
#include <sstream>
#include <iomanip>

#include <mia/core/sqmin.hh>
#include <mia/3d/register.hh>
#include <mia/3d/imageio.hh>

NS_MIA_BEGIN
using namespace std;


// an image copy operator that keeps the type of the internal images
struct FImageCopy: public TFilter<P3DImage> {
	template <typename T>
	P3DImage operator ()( const T3DImage<T>& image) const {
		return P3DImage(new T3DImage<T>(image));
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

void cat (C3DFVectorfield& a, const C3DFVectorfield& b)
{
	assert(a.get_size() == b.get_size());

	C3DFVectorfield::iterator i = a.begin();


	for (size_t z = 0; z < a.get_size().z; ++z)
		for (size_t y = 0; y < a.get_size().y; ++y)  {
			for (size_t x = 0; x < a.get_size().x; ++x,++i)  {
				C3DFVector xi = C3DFVector(x, y, z)- *i;
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
void cat3 (const C3DFVectorfield& a, const C3DFVectorfield& b, C3DFVectorfield& r)
{
	assert(a.get_size() == b.get_size());
	assert(a.get_size() == r.get_size());

	C3DFVectorfield::iterator ir = r.begin();
	C3DFVectorfield::const_iterator ib = b.begin();

	for (size_t z = 0; z < a.get_size().z; ++z)
		for (size_t y = 0; y < a.get_size().y; ++y)  {
			for (size_t x = 0; x < a.get_size().x; ++x, ++ir, ++ib)  {
				C3DFVector xi = C3DFVector(x, y, z) - *ib;
				*ir = a.get_interpol_val_at(xi) +  *ib;
			}
		}
}


void cat3 (const C3DFVectorfield& a, const C3DFVectorfield& b, C3DFVectorfield& r, float scale)
{
	assert(a.get_size() == b.get_size());
	assert(a.get_size() == r.get_size());

	C3DFVectorfield::iterator ir = r.begin();
	C3DFVectorfield::const_iterator ib = b.begin();

	for (size_t z = 0; z < a.get_size().z; ++z)
		for (size_t y = 0; y < a.get_size().y; ++y)  {
			for (size_t x = 0; x < a.get_size().x; ++x, ++ir, ++ib)  {
				C3DFVector shift = scale * *ib;
				C3DFVector xi = C3DFVector(x, y, z) - shift;
				*ir = a.get_interpol_val_at(xi) +  shift;
			}
		}
}


//
// upscale a vector field by using tri-linear interpolation
// and scaling of the vectors
P3DFVectorfield upscale( const C3DFVectorfield& vf, C3DBounds size)
{
	P3DFVectorfield Result(new C3DFVectorfield(size));
	float x_mult = float(size.x) / (float)vf.get_size().x;
	float y_mult = float(size.y) / (float)vf.get_size().y;
	float z_mult = float(size.z) / (float)vf.get_size().z;
	float ix_mult = 1.0f / x_mult;
	float iy_mult = 1.0f / y_mult;
	float iz_mult = 1.0f / z_mult;

	C3DFVectorfield::iterator i = Result->begin();

	for (unsigned int z = 0; z < size.z; z++)
		for (unsigned int y = 0; y < size.y; y++)
			for (unsigned int x = 0; x < size.x; x++,++i){
				C3DFVector help(ix_mult * x, iy_mult * y, iz_mult * z);
				C3DFVector val = vf.get_interpol_val_at(help);
				*i = C3DFVector(val.x * x_mult,val.y * y_mult, val.z * z_mult);
			}
	return Result;
}


C3DImageRegister::C3DImageRegister(size_t start_size, C3DImageCost& cost, size_t max_iter,
				   C3DRegModel& model, C3DRegTimeStep& time_step,
				   const C3DInterpolatorFactory&  ipf, float outer_epsilon, bool save_steps):
	m_start_size(start_size),
	m_cost(cost),
	m_max_iter(max_iter),
	m_model(model),
	m_time_step(time_step),
	m_ipf(ipf),
	m_outer_epsilon(outer_epsilon),
	m_save_steps(save_steps),
	m_mnum(0)
{
}

size_t log2(size_t x)
{
	size_t result = 0;
	while (x >>= 1)
		++result;
	return result;
}

// This operator takes care of the multi-resolution loop of the registration
P3DFVectorfield C3DImageRegister::operator () (const C3DImage& source, const C3DImage& reference)
{
	const C3DFilterPluginHandler::Instance& fph = C3DFilterPluginHandler::instance();

	if (source.get_size() != reference.get_size())
		throw invalid_argument("only registration of images of the same size is allowed");

	size_t x_shift = log2(source.get_size().x / m_start_size);
	size_t y_shift = log2(source.get_size().y / m_start_size);
	size_t z_shift = log2(source.get_size().z / m_start_size);

	P3DFVectorfield result;

	while (x_shift || y_shift|| z_shift) {
		C3DBounds block_size(1 << x_shift, 1 << y_shift, 1 << z_shift);
		stringstream downscalerstr;
		downscalerstr << "downscale:bx=" << (1 << x_shift) << ",by=" << (1 << x_shift) << ",bz=" << (1 << z_shift);
		std::shared_ptr<C3DFilter > downscaler = fph.produce(downscalerstr.str().c_str());

		P3DImage src_scaled = downscaler->filter(source);
		P3DImage ref_scaled = downscaler->filter(reference);

		if (!result)
			result = P3DFVectorfield(new C3DFVectorfield(src_scaled->get_size()));
		else
			result = upscale( *result, src_scaled->get_size() );

		if (m_time_step.has_regrid()) {
			reg_level_regrid_opt(*src_scaled, *ref_scaled, *result);
		}else {
			reg_level(*src_scaled, *ref_scaled, *result);
		}

		if (x_shift)
			--x_shift;

		if (y_shift)
			--y_shift;

		if (z_shift)
			--z_shift;

	}


	if (!result) {
		result = P3DFVectorfield(new C3DFVectorfield(source.get_size()));
	}else {
		result = upscale( *result, source.get_size());
	}
	if (m_time_step.has_regrid()) {
	reg_level_regrid_opt(source, reference, *result);
	}else
		reg_level(source, reference, *result);


	return result;
}


// this functions does the registration on one multi-resolution level
void C3DImageRegister::reg_level_regrid_opt(const C3DImage& source, const C3DImage& reference, C3DFVectorfield& result)

{
	size_t iter = 0;
	size_t inertia = 5;
	bool better = false;

	const C3DImageIOPluginHandler::Instance& imageio =  C3DImageIOPluginHandler::instance();

	double initial_cost;

	C3DFVectorfield local_shift(source.get_size());
	C3DFVectorfield best_local_shift(source.get_size());

	local_shift.clear();

	P3DImage temp = filter(FDeformer3D(result, m_ipf), source);
	m_cost.set_reference(reference); 
	double new_cost_value = initial_cost = m_cost.value(*temp);
	double C0 = new_cost_value;

	C3DFVectorfield v(source.get_size());
	C3DFVectorfield force(source.get_size());


	const char lend = cverb.get_level() == vstream::ml_debug ? '\n' : '\r';
	cvmsg() << reference.get_size() << "@" << iter << "|cost:" << new_cost_value << lend;

	float best_cost_value = new_cost_value;

	do {
		++iter;
		const float force_scale = m_model.get_force_scale(); 

		force.clear();
		m_cost.evaluate_force(*temp, force);
		transform(force.begin(), force.end(), force.begin(), 
			  [&force_scale](const C3DFVector& x){return force_scale * x;}); 

		C3DBounds l(0,0,0);

		// solve for the force to get a velocity or deformation field

		m_model.solve(force, v);

		// obtain the maximum shift. v might be updated according to the time step model
		float maxshift = m_time_step.calculate_pertuberation(v, local_shift);
		if (maxshift <= 0) {
			cvmsg() << "zero time step, perfect registration?\n";
			break;
		}

		// get the time step based on the maximum shift
		float delta = m_time_step.get_delta(maxshift);
		cvdebug() << "Timestep = " << delta << "\n";

		// now check, whether the new deformation would result in a
		// non-topologic distortion of the image. if so, regrid
		// some time step models might always return false
		if (m_time_step.regrid_requested(local_shift, v, delta)) {
			cat (local_shift, result);
			result = local_shift;
			local_shift.clear();
			better = false;
			if (m_save_steps) {
				stringstream regmov;
				C3DImageVector vimg;
				temp = filter(FDeformer3D(result, m_ipf), source);
				vimg.push_back(temp);
				regmov << "regmov" << setw(5) << setfill('0') << m_mnum - 1 << "regrid.vff";
				imageio.save(regmov.str(), vimg);
			}

		}
		double best_fdelta;

		{
			C3DFVectorfield local_shift_tmp(local_shift);

			m_time_step.apply(v, local_shift_tmp, delta);
			cat3(result, local_shift_tmp, force);
			temp = filter(FDeformer3D(force, m_ipf), source);
			double C1 = m_cost.value(*temp);
			double fdelta_c2 = (C1 < best_cost_value ? 2.0 : 0.5);

			std::copy(local_shift.begin(), local_shift.end(), local_shift_tmp.begin());

			m_time_step.apply(v, local_shift_tmp, fdelta_c2 * delta);
			cat3(result, local_shift_tmp, force);
			temp = filter(FDeformer3D(force, m_ipf), source);
			double C2 = m_cost.value(*temp);

			best_fdelta = min_ax2_bx_c(fdelta_c2, C0, C1, C2);

			// this is just hardcoded crap ...
			if ( best_fdelta < 0.1)
				best_fdelta = 0.1;
			m_time_step.apply(v, local_shift, best_fdelta * delta);
		}

		cat3(result, local_shift, force);
		temp = filter(FDeformer3D(force, m_ipf), source);
		new_cost_value = C0 = m_cost.value(*temp);


		// todo: add some logic to use the best step width as an estimate in the next step



		if (m_save_steps) {
			stringstream regmov;
			C3DImageVector vimg;
			vimg.push_back(temp);
			regmov << "regmov" << setw(5) << setfill('0') << m_mnum++ << ".vff";
			imageio.save(regmov.str(), vimg);
		}




		// if the cost is reduced, we allow for some search steps
		// with increasing cost again, and we save the current best deformation
		if (new_cost_value < best_cost_value) {
			better = true;
			best_local_shift = local_shift;
			best_cost_value = new_cost_value;
			if (new_cost_value < best_cost_value * 0.9)
				inertia = 5;
		}

		// if the cost reduction was big, increase the possible time step
		if (best_fdelta > 1.5) {
			m_time_step.increase();
		}

		// if the cost increased, reduce the time step if possible,
		// or count down the allowed search in sub-optimal areas
		if (new_cost_value > best_cost_value || best_fdelta < 0.5)
			if (!m_time_step.decrease())
				--inertia;

		cvmsg() << iter << "@" << reference.get_size()
			<< ": best ratio=" << best_cost_value / initial_cost
			<< ", absolute cost=" << new_cost_value
			<< ", fdelta =" << best_fdelta << "                     "
			<< lend;

	} while ( ((inertia > 0) || (new_cost_value < best_cost_value)) &&
		  new_cost_value / initial_cost > m_outer_epsilon && iter < m_max_iter);

	cvmsg() << '\n';

	// if re-gridding is possible, we need to accumulate teh deformation one last time

	if (better) {
		cvdebug() << "reg_level: sum the final result\n";
		cat (best_local_shift, result);
		result = best_local_shift;
	}

	if (m_save_steps) {
		stringstream regmov;
		C3DImageVector vimg;
		vimg.push_back(filter(FDeformer3D(best_local_shift, m_ipf), source));
		regmov << "regmov" << setw(5) << setfill('0') << --m_mnum << "final.vff";
		m_mnum++;
		imageio.save(regmov.str(), vimg);
	}
}


// this functions does the registration on one multi-resolution level
void C3DImageRegister::reg_level_regrid(const C3DImage& source, const C3DImage& reference, C3DFVectorfield& result)

{
	size_t iter = 0;
	size_t inertia = 5;
	bool better = false;

	const C3DImageIOPluginHandler::Instance& imageio =  C3DImageIOPluginHandler::instance();

	double initial_cost;

	C3DFVectorfield local_shift(source.get_size());
	C3DFVectorfield best_local_shift(source.get_size());

	local_shift.clear();

	P3DImage temp = filter(FDeformer3D(result, m_ipf), source);
	m_cost.set_reference(reference); 
	double new_cost_value = initial_cost = m_cost.value(*temp);

	C3DFVectorfield v(source.get_size());
	C3DFVectorfield force(source.get_size());


	const char lend = cverb.get_level() == vstream::ml_debug ? '\n' : '\r';
	cvmsg() << reference.get_size() << "@" << iter << "|cost:" << new_cost_value << lend;

	float best_cost_value = new_cost_value;

	do {
		++iter;

		const float force_scale = m_model.get_force_scale(); 
		
		float cost_value = new_cost_value;
		force.clear();
		m_cost.evaluate_force(*temp,  force);
		transform(force.begin(), force.end(), force.begin(), 
			  [&force_scale](const C3DFVector& x){return force_scale * x;}); 

		C3DBounds l(0,0,0);

#if 0
		C3DFVectorfield::const_iterator iforce = force.begin();
		for (l.z = 0; l.z < force.get_size().z; ++l.z)
			for (l.y = 0; l.y < force.get_size().y; ++l.y)
				for (l.x = 0; l.x < force.get_size().x; ++l.x, ++iforce) {
					if (iforce->norm2() > 0)
						cvdebug() << l << ":" << *iforce << "\n";
				}
#endif
		// solve for the force to get a velocity or deformation field

		m_model.solve(force, v);

		// obtain the maximum shift. v might be updated according to the time step model
		float maxshift = m_time_step.calculate_pertuberation(v, local_shift);
		if (maxshift <= 0) {
			cvmsg() << "zero time step, perfect registration?\n";
			break;
		}

		// get the time step based on the maximum shift
		float delta = m_time_step.get_delta(maxshift);
		cvdebug() << "Timestep = " << delta << "\n";

		// now check, whether the new deformation would result in a
		// non-topologic distortion of the image. if so, regrid
		// some time step models might always return false
		if (m_time_step.regrid_requested(local_shift, v, delta)) {
			cat (local_shift, result);
			result = local_shift;
			local_shift.clear();
			better = false;
			if (m_save_steps) {
				stringstream regmov;
				C3DImageVector vimg;
				temp = filter(FDeformer3D(result, m_ipf), source);
				vimg.push_back(temp);
				regmov << "regmov" << setw(5) << setfill('0') << m_mnum - 1 << "regrid.vff";
				imageio.save(regmov.str(), vimg);
			}

		}

		m_time_step.apply(v, local_shift, delta);

		cat3(result, local_shift, force);
		temp = filter(FDeformer3D(force, m_ipf), source);

		if (m_save_steps) {
			stringstream regmov;
			C3DImageVector vimg;
			vimg.push_back(temp);
			regmov << "regmov" << setw(5) << setfill('0') << m_mnum++ << ".vff";
			imageio.save(regmov.str(), vimg);
		}

		new_cost_value = m_cost.value(*temp);


		// if the cost is reduced, we allow for some search steps
		// with increasing cost again, and we save the current best deformation
		if (new_cost_value < best_cost_value) {
			better = true;
			best_local_shift = local_shift;
			best_cost_value = new_cost_value;
			inertia = 5;
		}

		// if the cost reduction was big, increase the possible time step
		if (new_cost_value < cost_value * 0.9) {
			m_time_step.increase();
		}

		// if the cost increased, reduce the time step if possible,
		// or count down the allowed search in sub-optimal areas
		if (new_cost_value > best_cost_value)
			if (!m_time_step.decrease())
				--inertia;

		cvmsg() << iter << "@" << reference.get_size()
			<< ": best ratio=" << best_cost_value / initial_cost
			<< ", absolute cost=" << new_cost_value
			<< ", delta =" << delta << "                     "
			<< lend;

	} while ( ((inertia > 0) || (new_cost_value < best_cost_value)) &&
		  new_cost_value / initial_cost > m_outer_epsilon && iter < m_max_iter);

	cvmsg() << '\n';

	// if re-gridding is possible, we need to accumulate teh deformation one last time

	if (better) {
		cvdebug() << "reg_level: sum the final result\n";
		cat (best_local_shift, result);
		result = best_local_shift;
	}

	if (m_save_steps) {
		stringstream regmov;
		C3DImageVector vimg;
		vimg.push_back(filter(FDeformer3D(best_local_shift, m_ipf), source));
		regmov << "regmov" << setw(5) << setfill('0') << --m_mnum << "final.vff";
		m_mnum++;
		imageio.save(regmov.str(), vimg);
	}
}

struct FMinMax: public TFilter <pair<float, float> > {
	template <typename T>
	pair<float, float> operator ()(const T3DImage<T>& image) const {
		pair<typename T3DImage<T>::const_iterator, typename T3DImage<T>::const_iterator>
			minmax = ::boost::minmax_element(image.begin(), image.end());
		return pair<float, float>(*minmax.first, *minmax.second);
	}
};


// this functions does the registration on one multi-resolution level
void C3DImageRegister::reg_level(const C3DImage& source, const C3DImage& reference, C3DFVectorfield& result)

{
	double cost_value;
	size_t iter = 0;
	size_t inertia = 5;

	pair<float, float> mm = filter( FMinMax(), source);
	float force_scale =m_model.get_force_scale() / ((mm.second - mm.first)* (mm.second - mm.first));

	const C3DImageIOPluginHandler::Instance& imageio = C3DImageIOPluginHandler::instance() ;

	double initial_cost;

	P3DImage temp = filter(FDeformer3D(result, m_ipf), source);
	m_cost.set_reference(reference); 
	double new_cost_value = initial_cost = m_cost.value(*temp);

	C3DFVectorfield force(source.get_size());
	C3DFVectorfield u(source.get_size());
	C3DFVectorfield best_result(result);

	const char lend = cverb.get_level() == vstream::ml_debug ? '\n' : '\r';
	cvmsg() << reference.get_size() << "@" << iter << "|cost:" << new_cost_value << lend;

	float best_cost_value = new_cost_value;

	float maxshift = m_time_step.calculate_pertuberation(result, result);
	float delta = m_time_step.get_delta(1.0 + maxshift);
	do {
		++iter;
		cost_value = new_cost_value;

		float help_scale = delta * force_scale; 
		force.clear();
		m_cost.evaluate_force(*temp,  force);
		transform(force.begin(), force.end(), force.begin(), 
			  [&help_scale](const C3DFVector& x){return help_scale * x;}); 


		m_model.solve(force, result);

		maxshift = m_time_step.calculate_pertuberation(result, result);
		if (maxshift <= 0) {
			cvmsg() << "zero time step, perfect registration?\n";
			break;
		}

		// get the time step based on the maximum shift
		delta = m_time_step.get_delta(1.0/iter + maxshift);
		cvdebug() << "Timestep = " << delta << "\n";


		temp = filter(FDeformer3D(result, m_ipf), source);

		if (m_save_steps) {
			stringstream regmov;
			C3DImageVector vimg;
			vimg.push_back(temp);
			regmov << "regmov" << setw(5) << setfill('0') << m_mnum++ << ".vff";
			imageio.save(regmov.str(), vimg);
		}

		new_cost_value = m_cost.value(*temp);

		if (new_cost_value < best_cost_value) {
			best_cost_value = new_cost_value;
			best_result = result;
			inertia = 5;
		}

		// if the cost reduction was big, increase the possible time step
		if (new_cost_value < cost_value * 0.9) {
			m_time_step.increase();
		}

		// if the cost increased, reduce the time step if possible,
		// or count down the allowed search in sub-optimal areas
		if (new_cost_value >= cost_value)
			if (!m_time_step.decrease()) {
				--inertia;
			}

		cvmsg() << iter << "@" << reference.get_size()
			<< ": best ratio=" << best_cost_value / initial_cost
			<< ", current =" << new_cost_value / cost_value
			<< ", delta =" << delta << "                     "
			<< lend;

	} while ( inertia > 0 && new_cost_value / initial_cost > m_outer_epsilon && iter < m_max_iter);

	cvmsg() << '\n';

	result = best_result;
	if (m_save_steps) {
		stringstream regmov;
		C3DImageVector vimg;
		vimg.push_back(filter(FDeformer3D(result, m_ipf), source));
		regmov << "regmov" << setw(5) << setfill('0') << --m_mnum << "final.vff";
		m_mnum++;
		imageio.save(regmov.str(), vimg);
	}
}

NS_MIA_END
