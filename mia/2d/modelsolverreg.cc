/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004 - 2010
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


#include <mia/2d/modelsolverreg.hh>

NS_MIA_BEGIN


class CModelSolverRegistrationImpl {
public:
	CModelSolverRegistrationImpl(P2DRegModel model,
				     P2DRegTimeStep time_step,
				     P2DTransformationFactory tf,
				     size_t start_size,
				     size_t max_iter,
				     float outer_epsilon);

	P2DTransformation apply(C2DImageFatCostList& cost) const;

private:
	void register_level(C2DImageFatCostList& cost, C2DTransformation& result)const;

	P2DTransformationFactory _M_trans_factory;
	P2DRegModel    _M_model;
	P2DRegTimeStep _M_time_step;

	size_t _M_start_size;
	size_t _M_max_iter;
	float _M_outer_epsilon;

};

CModelSolverRegistration::CModelSolverRegistration(P2DRegModel model,
						   P2DRegTimeStep time_step,
						   P2DTransformationFactory tf,
						   size_t start_size,
						   size_t max_iter,
						   float outer_epsilon):
	_M_impl(new CModelSolverRegistrationImpl(model,
						time_step,
						tf,
						start_size,
						max_iter,
						outer_epsilon))
{
}

P2DTransformation CModelSolverRegistration::operator ()(C2DImageFatCostList& cost) const
{
	return _M_impl->apply(cost);
}

CModelSolverRegistrationImpl::CModelSolverRegistrationImpl(P2DRegModel model,
							   P2DRegTimeStep time_step,
							   P2DTransformationFactory tf,
							   size_t start_size,
							   size_t max_iter,
							   float outer_epsilon):
	_M_trans_factory(tf),
	_M_model(model),
	_M_time_step(time_step),
	_M_start_size(start_size),
	_M_max_iter(max_iter),
	_M_outer_epsilon(outer_epsilon)
{
}


P2DTransformation CModelSolverRegistrationImpl::apply(C2DImageFatCostList& cost) const
{
	size_t x_shift = log2(cost.get_size().x / _M_start_size);
	size_t y_shift = log2(cost.get_size().y / _M_start_size);

	P2DTransformation result;

	while (x_shift || y_shift) {
		C2DBounds block_size(1 << x_shift, 1 << y_shift);
		C2DImageFatCostList cost_scaled = cost.get_downscaled(block_size);

		if (result)
			result = result->upscale(cost_scaled.get_size());
		else
			result = _M_trans_factory->create(cost_scaled.get_size());

		register_level(cost_scaled, *result);
		if (x_shift)
			--x_shift;

		if (y_shift)
			--y_shift;
	}

	result = result->upscale(cost.get_size());
	register_level(cost, *result);
	return result;
}

void CModelSolverRegistrationImpl::register_level(C2DImageFatCostList& cost, C2DTransformation& result) const
{
	double cost_value;
	size_t iter = 0;
	size_t inertia = 5;
	bool better = false;

	double initial_cost;

	P2DTransformation local = _M_trans_factory->create(cost.get_size());
	P2DTransformation best_local;

	cost.transform(result);
	double new_cost_value = initial_cost = cost.value();

	C2DFVectorfield gradient(cost.get_size());
	C2DFVectorfield v(cost.get_size());

	const char lend = cverb.get_level() == vstream::ml_debug ? '\n' : '\r';
	cvmsg() << cost.get_size() << "@" << iter << "|cost:" << new_cost_value << lend;

	float best_cost_value = new_cost_value;
	float force_scale = _M_model->get_force_scale();

	do {
		++iter;
		cost_value = new_cost_value;

		gradient.clear();
		cost.evaluate_force(gradient);

		for (C2DFVectorfield::iterator i = gradient.begin(); i != gradient.end(); ++i)
			*i *= force_scale;

		C2DFVectorfield force = local->translate(gradient);

		// solve for the force to get a velocity or deformation field
		//v.clear();
		_M_model->solve(force, v);

		// obtain the maximum shift. v might be updated according to the time step model
		float maxshift = _M_time_step->calculate_pertuberation(v, *local);

		if (maxshift <= 0) {
			cvmsg() << "zero time step, perfect registration?\n";
			break;
		}

		// get the time step based on the maximum shift
		float delta = _M_time_step->get_delta(maxshift);
		cvdebug() << "maxshift = " << maxshift << " Timestep = " << delta << "\n";

		// now check, whether the new deformation would result in a
		// non-topologic distortion of the image. if so, regrid
		// some time step models might always return false
		if (_M_time_step->regrid_requested(*local, v, delta)) {
			result.add(*local);
			local->set_identity();
			better = false;
		}
		local->update(delta, v);

		P2DTransformation r(result.clone());
		r->add(*local);
		cost.transform(*r);

		new_cost_value = cost.value();

		// if the cost is reduced, we allow for some search steps
		// with increasing cost again, and we save the current best deformation
		if (new_cost_value < best_cost_value) {
			better = true;
			best_local = local;
			best_cost_value = new_cost_value;
			inertia = 5;
		}

		// if the cost reduction was big, increase the possible time step
		if (new_cost_value < cost_value * 0.9)
			_M_time_step->increase();

		// if the cost increased, reduce the time step if possible,
		// or count down the allowed search in sub-optimal areas
		if (new_cost_value > best_cost_value)
			if (!_M_time_step->decrease())
				--inertia;

		cvmsg() << iter << "@" << cost.get_size()
			<< ": best ratio=" << best_cost_value / initial_cost
			<< ", absolute cost=" << new_cost_value << "                     "
			<< lend;

	} while ( ((inertia > 0) || (new_cost_value < best_cost_value)) &&
		  new_cost_value / initial_cost > _M_outer_epsilon && iter < _M_max_iter);

	cvmsg() << '\n';

	// if re-gridding is possible, we need to accumulate the deformation one last time

	if (better && best_local) {
		cvdebug() << "reg_level: sum the final result\n";

		result.add(*best_local);
	}

}

NS_MIA_END
