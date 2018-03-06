/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis
 * Copyright (c) Leipzig, Madrid 1999-2017 Gert Wollny
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

#include <mia/2d/filter/sepconv.hh>

NS_BEGIN(SeparableConvolute_2dimage_filter)

NS_MIA_USE;
using namespace std;
using namespace boost;
namespace bfs =::boost::filesystem;

CSeparableConvolute::CSeparableConvolute(P1DSpacialKernel kx,
              P1DSpacialKernel ky):
       m_kx(kx),
       m_ky(ky)
{
}

// this should be repaced by some more sophisticated code if ranges get smaller
template <typename A, typename B>
struct SConvert {
       B operator ()( A x) const
       {
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
       typedef std::vector<T> invec_t;
       T2DImage<T> *data = new T2DImage<T>(image);
       CSeparableConvolute::result_type result(data);
       int cachXSize = data->get_size().x;
       int cachYSize = data->get_size().y;

       if (m_kx.get()) {
              invec_t buffer(cachXSize);

              for (int y = 0; y < cachYSize; y++) {
                     data->get_data_line_x(y, buffer);
                     fold(buffer, *m_kx);
                     data->put_data_line_x(y, buffer);
              }
       }

       if (m_ky.get()) {
              invec_t buffer(cachYSize);

              for (int x = 0; x < cachXSize; x++) {
                     data->get_data_line_y(x, buffer);
                     fold(buffer, *m_ky);
                     data->put_data_line_y(x, buffer);
              }
       }

       return result;
}

CSeparableConvolute::result_type CSeparableConvolute::do_filter(const C2DImage& image) const
{
       return mia::filter(*this, image);
}


C2DSeparableConvoluteFilterPlugin::C2DSeparableConvoluteFilterPlugin():
       C2DFilterPlugin("sepconv")
{
       add_parameter("kx", make_param(m_kx, "gauss:w=1", false, "filter kernel in x-direction"));
       add_parameter("ky", make_param(m_ky, "gauss:w=1", false, "filter kernel in y-direction"));
}

C2DFilter *C2DSeparableConvoluteFilterPlugin::do_create()const
{
       return new CSeparableConvolute(m_kx, m_ky);
}

const string C2DSeparableConvoluteFilterPlugin::do_get_descr()const
{
       return "2D image intensity separaple convolution filter";
}

C2DGaussFilterPlugin::C2DGaussFilterPlugin():
       C2DFilterPlugin("gauss"),
       m_w(1)
{
       add_parameter("w", make_lc_param(m_w, 0, false, "filter width parameter"));
}

C2DFilter *C2DGaussFilterPlugin::do_create()const
{
       const auto&  skp = C1DSpacialKernelPluginHandler::instance();
       stringstream fdescr;
       fdescr << "gauss:w=" << m_w;
       auto k = skp.produce(fdescr.str().c_str());
       return new CSeparableConvolute(k, k);
}

const string C2DGaussFilterPlugin::do_get_descr()const
{
       return "isotropic 2D gauss filter";
}

const TDictMap<EGradientDirection>::Table dir_dict[] = {
       {"x", gd_x, "gradient in x-direction "},
       {"y", gd_y, "gradient in y-direction "},
       {NULL, gd_undefined, ""}
};

const TDictMap<EGradientDirection> Ddirection(dir_dict);

C2DSobelFilterPlugin::C2DSobelFilterPlugin():
       C2DFilterPlugin("sobel"),
       m_direction(gd_x)
{
       add_parameter("dir", new CDictParameter<EGradientDirection>(m_direction, Ddirection, "Gradient direction"));
}

mia::C2DFilter *C2DSobelFilterPlugin::do_create()const
{
       const auto&  skp = C1DSpacialKernelPluginHandler::instance();
       auto gauss = skp.produce("gauss:w=1");
       auto cdiff = skp.produce("cdiff");

       switch (m_direction) {
       case gd_x:
              return new CSeparableConvolute(cdiff, gauss);

       case gd_y:
              return new CSeparableConvolute(gauss, cdiff);

       default:
              throw invalid_argument("C2DSobelFilterPlugin: unknown gradient direction specified");
       }
}

const std::string C2DSobelFilterPlugin::do_get_descr()const
{
       return "The 2D Sobel filter for gradient evaluation. Note that the output pixel type "
              "of the filtered image is the same as the input pixel type, so converting the input "
              "beforehand to a floating point valued image is recommendable.";
}


C2DScharrFilterPlugin::C2DScharrFilterPlugin():
       C2DFilterPlugin("scharr"),
       m_direction(gd_x)
{
       add_parameter("dir", new CDictParameter<EGradientDirection>(m_direction, Ddirection, "Gradient direction"));
}

mia::C2DFilter *C2DScharrFilterPlugin::do_create()const
{
       const auto&  skp = C1DSpacialKernelPluginHandler::instance();
       auto scharr = skp.produce("scharr");
       auto cdiff = skp.produce("cdiff");

       switch (m_direction) {
       case gd_x:
              return new CSeparableConvolute(cdiff, scharr);

       case gd_y:
              return new CSeparableConvolute(scharr, cdiff);

       default:
              throw invalid_argument("C2DScharrFilterPlugin: unknown gradient direction specified");
       }
}

const std::string C2DScharrFilterPlugin::do_get_descr()const
{
       return "The 2D Scharr filter for gradient evaluation. Note that the output pixel type "
              "of the filtered image is the same as the input pixel type, so converting the input "
              "beforehand to a floating point valued image is recommendable.";
}


extern "C" EXPORT CPluginBase *get_plugin_interface()
{
       CPluginBase *gauss = new C2DGaussFilterPlugin();
       gauss->append_interface(new C2DSeparableConvoluteFilterPlugin());
       gauss->append_interface(new C2DSobelFilterPlugin());
       return gauss;
}

NS_END

