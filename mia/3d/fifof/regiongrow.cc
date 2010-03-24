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

/*! \brief

A region growing filter for stacks of 2D images

*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <mia/core/type_traits.hh>
#include <mia/3d/fifof/regiongrow.hh>

NS_BEGIN(regiongrow_fifof)

NS_MIA_USE;
using namespace std;
using namespace boost;
namespace bfs=boost::filesystem;

static char const * plugin_name = "regiongrow";


C2DRegiongrowFifoFilter::C2DRegiongrowFifoFilter(const CProbabilityVector probmap,
			float low, float seed, int cls, int depth):
	C2DImageFifoFilter(1, 1, depth),
	_M_probmap(probmap),
	_M_low(low),
	_M_seed(seed),
	_M_class(cls),
	_M_depth(depth)
{
	_M_shape = C3DShapePluginHandler::instance().produce("6n");
}

template <typename T, bool is_float>
struct FTransform {
	FTransform(CProbabilityVector::value_type map):
		_M_map(map),
		_M_maxidx(map.size()-1)
	{
		assert((size_t)_M_maxidx < map.size());
	}

	double operator () (T x) const {
		if (x <= 0)
			return _M_map[0];
		if (x <= _M_maxidx)
			return _M_map[x];
		return _M_map[_M_maxidx];
	}
	CProbabilityVector::value_type _M_map;
	T _M_maxidx;
};

template <typename T>
struct FTransform<T, true> {
	FTransform(CProbabilityVector::value_type /*map*/)
	{
		throw invalid_argument("C2DRegiongrowFifoFilter doesn't support float input images");
	}

	double operator () (T /*x*/) const {
		throw invalid_argument("C2DRegiongrowFifoFilter doesn't support float input images");
	}
};


template <typename T>
int C2DRegiongrowFifoFilter::operator ()( const T2DImage<T>& image)
{
	const bool is_float = is_floating_point<T>::value; 
	transform(image.begin(), image.end(), _M_in_buffer.begin(),
		  FTransform<T, is_float>(_M_probmap[_M_class]));

	transform(_M_in_buffer.begin(), _M_in_buffer.begin() + image.size(),
		  _M_out_buffer.begin(),
		  _M_seed);
	return 0;
}

void C2DRegiongrowFifoFilter::do_push(::boost::call_traits<P2DImage>::param_type x)
{
	TRACE("C2DRegiongrowFifoFilter::do_push");
	mia::accumulate(*this, *x);
}

void C2DRegiongrowFifoFilter::do_initialize(::boost::call_traits<P2DImage>::param_type x)
{
	TRACE("C2DRegiongrowFifoFilter::do_initialize");
	_M_slice_size = x->get_size();
	_M_slice_emls = _M_slice_size.x * _M_slice_size.y;
	C3DBounds buf_size(_M_slice_size.x, _M_slice_size.y, _M_depth + 1);
	_M_in_buffer = C3DDImage(buf_size);
	_M_out_buffer = C3DBitImage(buf_size);
}

void C2DRegiongrowFifoFilter::seed_env(const C3DBounds& center, queue<C3DBounds>& seeds) const
{
	C3DShape::const_iterator sb = _M_shape->begin();
	C3DShape::const_iterator se = _M_shape->end();

	while (sb != se) {
		C3DBounds x(center.x + sb->x, center.y + sb->y, center.z + sb->z);
		++sb;
		if (x.z >= get_end())
			continue;
		if ( _M_slice_size.y <= x.y || _M_slice_size.x <= x.x)
			continue;
		if (!_M_out_buffer(x) && _M_in_buffer(x) >= _M_low)
			seeds.push(x);
	}
}

void  C2DRegiongrowFifoFilter::grow()
{
	queue<C3DBounds> seeds;
	C3DBitImage::const_iterator i = _M_out_buffer.begin();

	for (size_t z = 0; z < get_end(); ++z)
		for (size_t y = 0; y < _M_slice_size.y; ++y)
			for (size_t x = 0; x < _M_slice_size.x; ++x, ++i) {
				if (*i)
					seed_env(C3DBounds(x,y,z), seeds);
			}

	while (!seeds.empty()) {
		C3DBounds x = seeds.front();
		cvdebug() << "add " << x << "\n";
		seeds.pop();
		if (!_M_out_buffer(x)) {
			_M_out_buffer(x) = true;
			seed_env(x, seeds);
		}
	}
}

P2DImage C2DRegiongrowFifoFilter::do_filter()
{
	TRACE("C2DRegiongrowFifoFilter::do_filter");
	cvdebug() << "Range: [" << get_start() << ", " << get_end() << "]\n";
	grow();
	C2DBitImage *result = new C2DBitImage(C2DBounds(3,3));
	copy(_M_out_buffer.begin_at(0,0,get_start()),  _M_out_buffer.begin_at(0,0, get_start() + 1), result->begin());
	return P2DImage(result);
}

void C2DRegiongrowFifoFilter::shift_buffer()
{
	TRACE("C2DRegiongrowFifoFilter::shift_buffer");
	copy_backward(_M_in_buffer.begin(), _M_in_buffer.end() - _M_slice_emls, _M_in_buffer.end());
	copy_backward(_M_out_buffer.begin(), _M_out_buffer.end() - _M_slice_emls, _M_out_buffer.end());
}

class C2DRegiongrowFifoFilterPlugin : public C2DFifoFilterPlugin {
public:
	C2DRegiongrowFifoFilterPlugin();
private:
	virtual const string do_get_descr() const;
	virtual bool do_test() const;
	virtual C2DFifoFilterPlugin::ProductPtr do_create()const;

	string _M_map;
	float _M_low;
	float _M_seed;
	int _M_class;
	int _M_depth;
};


C2DRegiongrowFifoFilterPlugin::C2DRegiongrowFifoFilterPlugin():
	C2DFifoFilterPlugin(plugin_name),
	_M_low(0.5f),
	_M_seed(0.98),
	_M_class(2),
	_M_depth(10)
{
	add_parameter("map", new CStringParameter(_M_map, true, "seed class map"));
	add_parameter("low", new CFloatParameter(_M_low, .0f, 1.0f, false,
						 "low threshold for acceptance probability"));
	add_parameter("seed", new CFloatParameter(_M_seed, .0f, 1.0f, false,
						 "threshold for seed probability"));

	add_parameter("class", new CIntParameter(_M_class, 0, numeric_limits<int>::max(), false,
						 "class to be segmented"));
	add_parameter("depth", new CIntParameter(_M_depth, 5, numeric_limits<int>::max(), false,
						 "number of slices to keep during processing"));
}


const string C2DRegiongrowFifoFilterPlugin::do_get_descr() const
{
	return "2D region grow stack filter";
}


bool C2DRegiongrowFifoFilterPlugin::do_test() const
{
	return true;
}

C2DFifoFilterPlugin::ProductPtr C2DRegiongrowFifoFilterPlugin::do_create()const
{
	return C2DFifoFilterPlugin::ProductPtr(
		 new C2DRegiongrowFifoFilter(CProbabilityVector(_M_map),
					     _M_low, _M_seed, _M_class, _M_depth));
}


extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C2DRegiongrowFifoFilterPlugin();
}
NS_END
