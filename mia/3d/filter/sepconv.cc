/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2014 Gert Wollny
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

#include <mia/core/threadedmsg.hh>
#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>

#include <mia/core/filter.hh>
#include <mia/core/msgstream.hh>
#include <mia/3d/filter/sepconv.hh>


#include <limits>

NS_BEGIN(sepconv_3dimage_filter)

NS_MIA_USE;
using namespace std;
using namespace boost;
namespace bfs=::boost::filesystem;

CSeparableConvolute::CSeparableConvolute(P1DSpacialKernel kx,
					 P1DSpacialKernel ky,
					 P1DSpacialKernel kz):
	m_kx(kx),
	m_ky(ky),
	m_kz(kz)
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
	typedef std::vector<T> invec_t;

	T3DImage<T> *data = new T3DImage<T>(image.get_size(), image);
	copy(image.begin(), image.end(), data->begin()); 
	CSeparableConvolute::result_type result(data);

	int cachXSize = data->get_size().x;
	int cachYSize = data->get_size().y;
	int cachZSize = data->get_size().z;

	auto filter_x = [cachXSize, cachYSize, data, this](const tbb::blocked_range<size_t>& range) {
		invec_t buffer(cachXSize);
		for (auto z = range.begin(); z != range.end();++z) {
			for (int y = 0; y < cachYSize; y++) {
				data->get_data_line_x(y,z,buffer);
				fold(buffer, *m_kx);
				data->put_data_line_x(y,z,buffer);
			}
		}
	}; 

	auto filter_y = [cachXSize, cachYSize, data, this](const tbb::blocked_range<size_t>& range) {
		invec_t buffer(cachYSize);
		for (auto z = range.begin(); z != range.end();++z) {
			for (int x = 0; x < cachXSize; x++) {
				data->get_data_line_y(x,z,buffer);
				fold(buffer, *m_ky);
				data->put_data_line_y(x,z,buffer);
			}
		}
	}; 

	auto filter_z = [cachXSize, cachZSize, data, this](const tbb::blocked_range<size_t>& range) {
		invec_t buffer(cachZSize);
		for (auto y = range.begin(); y != range.end();++y) {
			for (int x = 0; x < cachXSize; x++) {
				data->get_data_line_z(x,y,buffer);
				fold(buffer, *m_kz);
				data->put_data_line_z(x,y,buffer);
			}
		}
	}; 

	if (m_kx.get()) {
		tbb::parallel_for(tbb::blocked_range<size_t>(0, cachZSize, 1), filter_x); 
	}


	if (m_ky.get()) {
		tbb::parallel_for(tbb::blocked_range<size_t>(0, cachZSize, 1), filter_y); 
	}

	if (m_kz.get()) {
		tbb::parallel_for(tbb::blocked_range<size_t>(0, cachYSize, 1), filter_z); 	
	}
	return result;
}

CSeparableConvolute::result_type CSeparableConvolute::do_filter(const C3DImage& image) const
{
	return mia::filter(*this, image);
}


C3DSeparableConvoluteFilterPlugin::C3DSeparableConvoluteFilterPlugin():
	C3DFilterPlugin("sepconv")
{
	add_parameter("kx", make_param(m_kx, "gauss:w=1", false, "filter kernel in x-direction"));
	add_parameter("ky", make_param(m_ky, "gauss:w=1", false, "filter kernel in y-direction"));
	add_parameter("kz", make_param(m_kz, "gauss:w=1", false, "filter kernel in z-direction"));
}

C3DFilter *C3DSeparableConvoluteFilterPlugin::do_create()const
{
	return new CSeparableConvolute(m_kx, m_ky, m_kz);
}

const string C3DSeparableConvoluteFilterPlugin::do_get_descr()const
{
	return "3D image intensity separaple convolution filter";
}


C3DGaussFilterPlugin::C3DGaussFilterPlugin():
	C3DFilterPlugin("gauss"),
	m_w(1)
{
	add_parameter("w", new CIntParameter(m_w, 0,numeric_limits<int>::max(), false, "filter width parameter"));
}

C3DFilter *C3DGaussFilterPlugin::do_create()const
{
	const C1DSpacialKernelPluginHandler::Instance&  skp = C1DSpacialKernelPluginHandler::instance();

	stringstream fdescr;
	fdescr << "gauss:w=" << m_w;
	auto k = skp.produce(fdescr.str().c_str());

	return new CSeparableConvolute(k, k, k);
}

const string C3DGaussFilterPlugin::do_get_descr()const
{
	return "isotropic 3D gauss filter";
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	CPluginBase *gauss = new C3DGaussFilterPlugin();
	gauss->append_interface(new C3DSeparableConvoluteFilterPlugin());
	return gauss;
}

NS_END

