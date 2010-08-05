/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
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

/*
  This program implements 3D gray scale image registration.
  Depending on the available plug-ins it can accomodate various models and cost-functions.
  So far input images can be given as PNG, TIF, BMP and OpenEXR (depending on the installed plug-ins)
  The input images must be of the same dimensions and gray scale (whatever bit-depth).
  The vector field will be saved as a EXR image with two frames of float values, one for the X
  and one for the Y values of the vectors.
  Other vector field output plug-ins might be supported depending on the installed plug-ins.
*/

#include <mia/core.hh>
#include <mia/3d.hh>
#include <sstream>
#include <iomanip>
#include <mia/3d/multireg.hh>
#include <boost/algorithm/minmax_element.hpp>

NS_MIA_BEGIN
using namespace boost;
using namespace std;


// an image copy operator that keeps the type of the internal images
struct FImageCopy: public TFilter<P3DImage> {
	template <typename T>
	P3DImage operator ()( const T3DImage<T>& image) const {
		return P3DImage(new T3DImage<T>(image));
	}
};


static size_t log2(size_t x)
{
	size_t result = 0;
	while (x >>= 1)
		++result;
	return result;
}

struct FMinMax: public TFilter <pair<float, float> > {
	template <typename T>
	pair<float, float> operator ()(const T3DImage<T>& image) const {
		pair<typename T3DImage<T>::const_iterator, typename T3DImage<T>::const_iterator>
			minmax = ::boost::minmax_element(image.begin(), image.end());
		return pair<float, float>(*minmax.first, *minmax.second);
	}
};

C3DMultiImageRegister::C3DMultiImageRegister(size_t start_size, size_t max_iter,
					     C3DRegModel& model, C3DRegTimeStep& time_step,
					     float outer_epsilon):
	_M_start_size(start_size),
	_M_max_iter(max_iter),
	_M_model(model),
	_M_time_step(time_step),
	_M_outer_epsilon(outer_epsilon)
{
}

P3DTransformation C3DMultiImageRegister::operator () (C3DImageFatCostList& cost, P3DInterpolatorFactory ipf)
{
	TRACE("C3DMultiImageRegister::operator");


	size_t x_shift = log2(cost.get_size().x / _M_start_size);
	size_t y_shift = log2(cost.get_size().y / _M_start_size);
	size_t z_shift = log2(cost.get_size().z / _M_start_size);


	P3DTransformation result(new C3DGridTransformation(C3DBounds(0,0,0)));

	while (x_shift || y_shift|| z_shift) {
		C3DBounds block_size(1 << x_shift, 1 << y_shift, 1 << z_shift);

		C3DImageFatCostList cost_scaled = cost.get_downscaled(block_size);

		result = result->upscale(cost_scaled.get_size());


		C3DGridTransformation& current_result = dynamic_cast<C3DGridTransformation&>(*result);
		if (_M_time_step.has_regrid()) {
			reg_level_regrid(cost_scaled, current_result, *ipf);
		}else {
			reg_level(cost_scaled, current_result, *ipf);
		}

		if (x_shift)
			--x_shift;

		if (y_shift)
			--y_shift;

		if (z_shift)
			--z_shift;
	}

	result = result->upscale(cost.get_size());

	{
		C3DGridTransformation& current_result = dynamic_cast<C3DGridTransformation&>(*result);

		if (_M_time_step.has_regrid()) {
			reg_level_regrid(cost, current_result, *ipf);
		}else
			reg_level(cost, current_result, *ipf);
	}

	return result;

}

void C3DMultiImageRegister::reg_level_regrid(C3DImageFatCostList& cost,
					     C3DGridTransformation& result,
					     const C3DInterpolatorFactory& /*ipf*/)
{
	TRACE("C3DMultiImageRegister::reg_level");
	double cost_value;
	size_t iter = 0;
	size_t inertia = 5;
	bool better = false;

	double initial_cost;

	C3DGridTransformation local_shift(cost.get_size());
	C3DGridTransformation best_local_shift(cost.get_size());

	// local_shift.clear();

	cost.transform(result);
	double new_cost_value = initial_cost = cost.value();

	C3DFVectorfield v(cost.get_size());
	C3DFVectorfield force(cost.get_size());


	const char lend = cverb.get_level() == vstream::ml_debug ? '\n' : '\n';
	cvmsg() << cost.get_size() << "@" << iter << "|cost:" << new_cost_value << lend;


	float best_cost_value = new_cost_value;

	float force_scale = _M_model.get_force_scale();
	do {
		++iter;
		cost_value = new_cost_value;

		force.clear();
		cost.evaluate_force(force);

		for (C3DFVectorfield::iterator i = force.begin(); i != force.end(); ++i)
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
			result += local_shift;
			fill(local_shift.field_begin(), local_shift.field_end(), C3DFVector(0.0f, 0.0f, 0.0f));
			better = false;
		}

		_M_time_step.apply(v, local_shift, delta);

		C3DGridTransformation current_trans = result + local_shift;

		cost.transform(current_trans);

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

	} while ( ((inertia > 0) || (new_cost_value < 0.999 * best_cost_value)) &&
		  new_cost_value / initial_cost > _M_outer_epsilon && iter < _M_max_iter);

	cvmsg() << '\n';

	// if re-gridding is possible, we need to accumulate teh deformation one last time

	if (better) {
		cvdebug() << "reg_level: sum the final result\n";

		result += best_local_shift;
	}

}

void C3DMultiImageRegister::reg_level(C3DImageFatCostList& cost,
                                      C3DGridTransformation& result,
                                      const C3DInterpolatorFactory& /*ipf*/)
{
	TRACE("C3DMultiImageRegister::reg_level");
	double cost_value;

	size_t iter = 0;
	size_t inertia = 5;
	bool better = false;

	double initial_cost;

	C3DGridTransformation local_shift(cost.get_size());
	C3DGridTransformation best_local_shift(cost.get_size());

	cost.transform(result);

	double new_cost_value = initial_cost = cost.value();

	C3DGridTransformation v(cost.get_size());
	C3DGridTransformation force(cost.get_size());


	const char lend = cverb.get_level() == vstream::ml_debug ? '\n' : '\r';
	cvmsg() << cost.get_size() << "@" << iter << "|cost:" << new_cost_value << lend;

	float best_cost_value = new_cost_value;

	float force_scale = _M_model.get_force_scale();
	cvmsg() << "Force scale = " << force_scale << "\n";

	do {
		++iter;
		cost_value = new_cost_value;

		cost.evaluate_force(force);
		for (C3DFVectorfield::iterator i = force.field_begin(); i != force.field_end(); ++i)
			*i *= force_scale;


		// solve for the force to get a velocity or deformation field
		fill(v.field_begin(), v.field_end(), C3DFVector(0.0f, 0.0f, 0.0f));
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

		_M_time_step.apply(v, local_shift, delta);

		force = result +  local_shift;

		cost.transform(force);

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

	} while ( ((inertia > 0) || (new_cost_value < 0.999 * best_cost_value)) &&
		  new_cost_value / initial_cost > _M_outer_epsilon && iter < _M_max_iter);

	cvmsg() << '\n';

	// if re-gridding is possible, we need to accumulate teh deformation one last time

	if (better) {
		cvdebug() << "reg_level: sum the final result\n";

		result += best_local_shift;
	}
}

NS_MIA_END
