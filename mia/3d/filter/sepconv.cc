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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

/*! \brief basic type of a plugin handler

A bandpass filter

\file bandpass_3dimage_filter.cc
\author Gert Wollny <wollny at eva.mpg.de>

*/


#include <mia/core/filter.hh>
#include <mia/core/msgstream.hh>
#include <mia/3d/filter/sepconv.hh>


#include <limits>

NS_BEGIN(sepconv_3dimage_filter)

NS_MIA_USE;
using namespace std;
using namespace boost;
namespace bfs=::boost::filesystem;

CSeparableConvolute::CSeparableConvolute(C1DSpacialKernelPlugin::ProductPtr kx,
					 C1DSpacialKernelPlugin::ProductPtr ky,
					 C1DSpacialKernelPlugin::ProductPtr kz):
	_M_kx(kx),
	_M_ky(ky),
	_M_kz(kz)
{
}


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
CSeparableConvolute::result_type CSeparableConvolute::operator () (const T3DImage<T>& image) const
{
	typedef typename T3DImage<T>::value_type value_type;
	typedef std::vector<value_type> invec_t;

	T3DImage<T> *data = new T3DImage<T>(image);
	CSeparableConvolute::result_type result(data);

	int cachXSize = data->get_size().x;
	int cachYSize = data->get_size().y;
	int cachZSize = data->get_size().z;

	if (_M_kx.get()) {
		invec_t buffer(cachXSize);
		for (int z = 0; z < cachZSize; z++){
			for (int y = 0; y < cachYSize; y++) {
				data->get_data_line_x(y,z,buffer);
				fold(buffer, *_M_kx);
				data->put_data_line_x(y,z,buffer);
			}
		}
	}

	if (_M_ky.get()) {
		invec_t buffer(cachYSize);
		for (int z = 0; z < cachZSize; z++){
			for (int x = 0; x < cachXSize; x++) {
				data->get_data_line_y(x,z,buffer);
				fold(buffer, *_M_ky);
				data->put_data_line_y(x,z,buffer);
			}
		}
	}

	if (_M_kz.get()) {
		invec_t buffer(cachZSize);
		for (int y = 0; y < cachYSize; y++){
			for (int x = 0; x < cachXSize; x++) {
				data->get_data_line_z(x,y,buffer);
				fold(buffer, *_M_kz);
				data->put_data_line_z(x,y,buffer);
			}
		}
	}
	return result;
}

CSeparableConvolute::result_type CSeparableConvolute::do_filter(const C3DImage& image) const
{
	return mia::filter(*this, image);
}


C3DSeparableConvoluteFilterPlugin::C3DSeparableConvoluteFilterPlugin():
	C3DFilterPlugin("sepconv"),
	_M_kx("gauss:w=1"),
	_M_ky("gauss:w=1"),
	_M_kz("gauss:w=1")
{
	add_parameter("kx", new CStringParameter(_M_kx, false, "filter kernel in x-direction"));
	add_parameter("ky", new CStringParameter(_M_ky, false, "filter kernel in y-direction"));
	add_parameter("kz", new CStringParameter(_M_ky, false, "filter kernel in z-direction"));
}

C3DSeparableConvoluteFilterPlugin::ProductPtr C3DSeparableConvoluteFilterPlugin::do_create()const
{
	C1DSpacialKernelPlugin::ProductPtr kx, ky, kz;
	const C1DSpacialKernelPluginHandler::Instance&  skp = C1DSpacialKernelPluginHandler::instance();
	kx = skp.produce(_M_kx.c_str());
	ky = skp.produce(_M_ky.c_str());
	kz = skp.produce(_M_kz.c_str());


	return C3DSeparableConvoluteFilterPlugin::ProductPtr(new CSeparableConvolute(kx, ky, kz));
}

void C3DSeparableConvoluteFilterPlugin::prepare_path() const
{
}


bool C3DSeparableConvoluteFilterPlugin::do_test() const
{
	return false;
}

const string C3DSeparableConvoluteFilterPlugin::do_get_descr()const
{
	return "3D image intensity separaple convolution filter";
}


C3DGaussFilterPlugin::C3DGaussFilterPlugin():
	C3DFilterPlugin("gauss"),
	_M_w(1)
{
	add_parameter("w", new CIntParameter(_M_w, 0,numeric_limits<int>::max(), false, "filter width parameter"));
}

C3DFilterPlugin::ProductPtr C3DGaussFilterPlugin::do_create()const
{
	C1DSpacialKernelPlugin::ProductPtr k;
	const C1DSpacialKernelPluginHandler::Instance&  skp = C1DSpacialKernelPluginHandler::instance();

	stringstream fdescr;
	fdescr << "gauss:w=" << _M_w;
	k = skp.produce(fdescr.str().c_str());

	return C3DSeparableConvoluteFilterPlugin::ProductPtr(new CSeparableConvolute(k, k, k));
}

const string C3DGaussFilterPlugin::do_get_descr()const
{
	return "isotropic 3D gauss filter";
}

bool C3DGaussFilterPlugin::do_test() const
{
	cvwarn() << "no test!";
	return true;
}


extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	CPluginBase *gauss = new C3DGaussFilterPlugin();
	gauss->append_interface(new C3DSeparableConvoluteFilterPlugin());
	return gauss;
}

NS_END

