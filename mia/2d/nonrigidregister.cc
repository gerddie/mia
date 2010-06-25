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

#define VSTREAM_DOMAIN "NONRIGID-REG"

#include <boost/lambda/lambda.hpp>
#include <mia/2d/nonrigidregister.hh>
NS_MIA_BEGIN

using namespace boost::lambda;

struct C2DMultiImageNonrigidRegisterImpl {
	C2DMultiImageNonrigidRegisterImpl(size_t start_size,
					  size_t max_iter,
					  P2DRegModel model,
					  P2DRegTimeStep time_step,
					  P2DTransformationFactory trans_factory,
					  float outer_epsilon);
	P2DTransformation run(C2DImageFatCostList& cost);
private:
	void register_level(C2DImageFatCostList& cost, C2DTransformation& result);
	size_t _M_start_size;
	size_t _M_max_iter;
	P2DRegModel _M_model;
	P2DRegTimeStep _M_time_step;
	P2DTransformationFactory _M_trans_factory;
	float _M_outer_epsilon;
};

C2DMultiImageNonrigidRegister::C2DMultiImageNonrigidRegister(size_t start_size, size_t max_iter,
						     P2DRegModel model,
						     P2DRegTimeStep time_step,
						     P2DTransformationFactory trans_factory,
						     float outer_epsilon):
	impl(new C2DMultiImageNonrigidRegisterImpl(start_size, max_iter,
						   model,
						   time_step,
						   trans_factory,
						   outer_epsilon))
{
}

C2DMultiImageNonrigidRegister::~C2DMultiImageNonrigidRegister()
{
	delete impl;
}

P2DTransformation C2DMultiImageNonrigidRegister::operator () (C2DImageFatCostList& cost)
{
	return impl->run(cost);
}


C2DMultiImageNonrigidRegisterImpl::C2DMultiImageNonrigidRegisterImpl(size_t start_size,
								     size_t max_iter,
								     P2DRegModel model,
								     P2DRegTimeStep time_step,
								     P2DTransformationFactory trans_factory,
								     float outer_epsilon):
	_M_start_size(start_size),
	_M_max_iter(max_iter),
	_M_model(model),
	_M_time_step(time_step),
	_M_trans_factory(trans_factory),
	_M_outer_epsilon(outer_epsilon)
{
}

P2DTransformation C2DMultiImageNonrigidRegisterImpl::run(C2DImageFatCostList& cost)
{
	TRACE_FUNCTION;

	// downscale to start size
	size_t x_shift = log2(cost.get_size().x / _M_start_size) + 1;
	size_t y_shift = log2(cost.get_size().y / _M_start_size) + 1;

	P2DTransformation result;

	while (x_shift || y_shift) {

		if (x_shift)
			--x_shift;
		if (y_shift)
			--y_shift;

		C2DBounds block_size(1 << x_shift, 1 << y_shift);
		C2DImageFatCostList scaled_cost = cost.get_downscaled(block_size);

		if (!result)
			result = _M_trans_factory->create(scaled_cost.get_size());
		else
			result = result->upscale(scaled_cost.get_size());

		register_level(scaled_cost, *result);

	} ;

	return result;
}

void C2DMultiImageNonrigidRegisterImpl::register_level(C2DImageFatCostList& cost, C2DTransformation& result)
{
	TRACE_FUNCTION;

	size_t iter = 0;
	size_t inertia = 5;

	const char lend = cverb.get_level() == vstream::ml_debug ? '\n' : '\r';
	// transform image
	cost.transform(result);

	double initial_cost;
	double best_cost;
	double new_cost = best_cost = initial_cost = cost.value();

	P2DTransformation local_shift = _M_trans_factory->create(cost.get_size());
	P2DTransformation best_local_shift;

	local_shift->set_identity();

	float force_scale = _M_model->get_force_scale();


	C2DFVectorfield gradient(cost.get_size());

	cvmsg() << cost.get_size() << "@" << iter << "|cost:" << new_cost << lend;

	do {
		++iter;
		float cost_value = new_cost;

		cost.evaluate_force(gradient);
		transform( gradient.begin(), gradient.end(), gradient.begin(), _1 * force_scale);

		C2DFVectorfield force; assert(0 && "change this");// = result.translate(gradient);
		C2DFVectorfield v(force.get_size());
		_M_model->solve(force, v);

		float  maxshift = _M_time_step->calculate_pertuberation(v, *local_shift);
		float  delta = _M_time_step->get_delta(maxshift);

		if (_M_time_step->regrid_requested(*local_shift, v, delta)) {
			result.add(*local_shift);
			local_shift->set_identity();
			best_local_shift.reset();
		}

		local_shift->update(delta, v);

		auto_ptr<C2DTransformation> result_copy(result.clone());
		result_copy->add(*local_shift);

		cost.transform(*result_copy);

		new_cost = cost.value();

		if (new_cost < cost_value) {
			inertia = 5;
			if (new_cost < cost_value * 0.9) {
				_M_time_step->increase();
			}
			best_cost = new_cost;
			best_local_shift.reset(local_shift->clone());
		} else {
			if (!_M_time_step->decrease())
				--inertia;
		}

		cvmsg() << iter << "@" << cost.get_size()
			<< ": best ratio=" << best_cost / initial_cost
			<< ", absolute cost=" << new_cost << "                     "
			<< lend;

	} while ( ((inertia > 0) || (new_cost < best_cost)) &&
		  new_cost / initial_cost > _M_outer_epsilon && iter < _M_max_iter);

	if (best_local_shift)
		local_shift = best_local_shift;
	result.add(*local_shift);
}


NS_MIA_END

