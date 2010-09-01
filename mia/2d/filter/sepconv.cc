/* -*- mia-c++  -*-
 * Copyright (c) Leipzig, Madrid 2004-2010
 * Max-Planck-Institute for Evolutionary Anthropoloy
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

#include <mia/2d/filter/sepconv.hh>

NS_BEGIN(SeparableConvolute_2dimage_filter)

NS_MIA_USE;
using namespace std;
using namespace boost;
namespace bfs=::boost::filesystem;

CSeparableConvolute::CSeparableConvolute(C1DSpacialKernelPlugin::ProductPtr kx,
					 C1DSpacialKernelPlugin::ProductPtr ky):
	_M_kx(kx),
	_M_ky(ky)
{
}

// this should be repaced by some more sophisticated code if ranges get smaller
template <typename A, typename B>
struct SConvert {
	B operator ()( A x) const {
		return B(x);
	}
};

template <typename T>
void CSeparableConvolute::fold(vector<T>& data, const C1DFilterKernel& kernel) const
{
	vector<double> tmp(data.size());
	copy(data.begin(), data.end(), tmp.begin());
	kernel.apply_inplace(tmp);
	transform(tmp.begin(), tmp.end(), data.begin(), SConvert<double, T>());
}

template <class T>
CSeparableConvolute::result_type CSeparableConvolute::operator () (const T2DImage<T>& image) const
{
	typedef typename T2DImage<T>::value_type value_type;
	typedef std::vector<value_type> invec_t;

	T2DImage<T> *data = new T2DImage<T>(image);
	CSeparableConvolute::result_type result(data);

	int cachXSize = data->get_size().x;
	int cachYSize = data->get_size().y;

	if (_M_kx.get()) {
		invec_t buffer(cachXSize);
		for (int y = 0; y < cachYSize; y++) {
			data->get_data_line_x(y,buffer);
			fold(buffer, *_M_kx);
			data->put_data_line_x(y,buffer);
		}

	}

	if (_M_ky.get()) {
		invec_t buffer(cachYSize);
		for (int x = 0; x < cachXSize; x++) {
			data->get_data_line_y(x,buffer);
			fold(buffer, *_M_ky);
			data->put_data_line_y(x,buffer);
		}
	}

	return result;
}

CSeparableConvolute::result_type CSeparableConvolute::do_filter(const C2DImage& image) const
{
	return mia::filter(*this, image);
}


C2DSeparableConvoluteFilterPlugin::C2DSeparableConvoluteFilterPlugin():
	C2DFilterPlugin("sepconv"),
	_M_kx("gauss:w=1"),
	_M_ky("gauss:w=1")
{
	add_parameter("kx", new CStringParameter(_M_kx, false, "filter kernel in x-direction"));
	add_parameter("ky", new CStringParameter(_M_ky, false, "filter kernel in y-direction"));
}

C2DSeparableConvoluteFilterPlugin::ProductPtr C2DSeparableConvoluteFilterPlugin::do_create()const
{
	C1DSpacialKernelPlugin::ProductPtr kx, ky, kz;
	const C1DSpacialKernelPluginHandler::Instance& skp = C1DSpacialKernelPluginHandler::instance();

	kx = skp.produce(_M_kx.c_str());
	ky = skp.produce(_M_ky.c_str());

	return C2DSeparableConvoluteFilterPlugin::ProductPtr(new CSeparableConvolute(kx, ky));
}

const string C2DSeparableConvoluteFilterPlugin::do_get_descr()const
{
	return "2D image intensity separaple convolution filter";
}

void C2DSeparableConvoluteFilterPlugin::prepare_path() const
{
	list< bfs::path> kernelsearchpath;
	kernelsearchpath.push_back(bfs::path("..")/bfs::path("..")/bfs::path("core")/bfs::path("spacialkernel"));
	C1DSpacialKernelPluginHandler::set_search_path(kernelsearchpath);
}

C2DGaussFilterPlugin::C2DGaussFilterPlugin():
	C2DFilterPlugin("gauss"),
	_M_w(1)
{
	add_parameter("w", new CIntParameter(_M_w, 0, numeric_limits<int>::max(),
					     false, "filter width parameter"));
}

C2DFilterPlugin::ProductPtr C2DGaussFilterPlugin::do_create()const
{
	C1DSpacialKernelPlugin::ProductPtr k;
	const C1DSpacialKernelPluginHandler::Instance&  skp = C1DSpacialKernelPluginHandler::instance();

	stringstream fdescr;
	fdescr << "gauss:w=" << _M_w;
	k = skp.produce(fdescr.str().c_str());

	return C2DSeparableConvoluteFilterPlugin::ProductPtr(new CSeparableConvolute(k, k));
}

const string C2DGaussFilterPlugin::do_get_descr()const
{
	return "isotropic 2D gauss filter";
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	CPluginBase *gauss = new C2DGaussFilterPlugin();
	gauss->append_interface(new C2DSeparableConvoluteFilterPlugin());
	return gauss;
}

NS_END

