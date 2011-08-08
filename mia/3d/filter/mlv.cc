/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
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
   LatexBeginPluginDescription{3D image filters}
   
   \subsection{Mean Least Variance filter}
   \label{filter3d:mlv}
   
   \begin{description}
   
   \item [Plugin:] mlv
   \item [Description:] Apply the mean least variance filter. 
   \item [Input:] Abitrary gray scale or binary image 
   \item [Output:] The filtered image of the same pixel type and dimension 
   
   \plugtabstart
   w & int & filter width parameter, the actual filter captures a neighborhood of 
                 $(4 * w + 1) \times (4 * w + 1) \times (4 * w + 1)$ voxels & 1 \\ 
   \plugtabend
   \end{description}

   LatexEnd  
 */


#include <mia/core/filter.hh>
#include <mia/core/msgstream.hh>
#include <mia/3d/filter/mlv.hh>
#include <limits>

NS_BEGIN(mlv_3dimage_filter)
NS_MIA_USE;
using namespace std;

struct FSquare {
	float operator() (float x) {
		return x * x;
	}
};

template <typename InputIterator>
void C3DMLVImageFilter::run_sub(InputIterator begin, InputIterator end, int cy, int cz, C3DFImage& mu, C3DFImage& sigma,
			 C3DFImage& n, vector<float>& buffer, const vector<float>& eins)const
{
	transform(begin, end, buffer.begin(), FSquare());

	for (int z = 0; z < m_kh; ++z)
		for (int y = 0; y < m_kh; ++y)
			for (int x = 0; x < m_kh; ++x) {
				transform(begin, end, mu.begin_at(x, cy + y, cz + z),
					  mu.begin_at(x, cy + y, cz + z), plus<float>());

				transform(buffer.begin(), buffer.end(), sigma.begin_at(x, cy + y, cz + z),
					  sigma.begin_at(x, cy + y, cz + z), plus<float>());

				transform(eins.begin(), eins.end(), n.begin_at(x, cy + y, cz + z),
					  n.begin_at(x, cy + y, cz + z), plus<float>());
			}
}


template <typename T>
T C3DMLVImageFilter::get(const C3DFImage& mu, const C3DFImage& sigma, int x, int y, int z, T ref)const
{

	const int zmax = z +  m_kh;
	const int ymax = y +  m_kh;
	const int xmax = x +  m_kh;


	float best_sigma = numeric_limits<float>::max();
	float best_mu = 0;

	int i = 0;

	for (int iz = z; iz < zmax; ++iz)
		for (int iy = y; iy < ymax; ++iy)
			for (int ix = x; ix < xmax; ++ix, ++i) {
				float const s = sigma(ix, iy, iz);
				float const m = mu(ix, iy, iz);


				if (best_sigma > s ) {
					best_sigma = s;
					best_mu = m;
				}else if (best_sigma == s && best_mu != m
					  && ::fabs(ref - m) < ::fabs(ref - best_mu)) {
					best_mu = m;
				}
			}
	return  static_cast<T>(best_mu);
}

template <typename T>
typename C3DMLVImageFilter::result_type C3DMLVImageFilter::operator () (const T3DImage<T>& data) const
{
	cvdebug() << "C3DMLV::operator () begin\n";

	C3DBounds border(2*m_l, 2*m_l, 2*m_l);
	vector<float> buffer(data.get_size().x);
	vector<float> eins(data.get_size().x);
	fill(eins.begin(), eins.end(), 1.0f);

	C3DFImage mu(data.get_size() + border);
	fill(mu.begin(), mu.end(), 0);
	C3DFImage N(data.get_size() + border);
	fill(N.begin(), N.end(), 0);
	C3DFImage sigma(data.get_size() + border);
	fill(sigma.begin(), sigma.end(), 0);

	cvdebug() << "temporary image have size (" << mu.get_size() << ")\n";

	for (size_t z = 0; z < data.get_size().z; ++z) {
		for (size_t y = 0; y < data.get_size().y; ++y) {
			run_sub(data.begin_at(0, y, z), data.begin_at(0, y, z) + data.get_size().x,
				y, z, mu, sigma, N, buffer, eins);
		}
	}

	for(C3DFImage::iterator mu_i = mu.begin(), mu_e = mu.end(),
		    sigma_i = sigma.begin(), n_i = N.begin();
	    mu_i != mu_e; ++mu_i, ++sigma_i, ++n_i) {

		assert(*n_i > 0.1);
		*mu_i /= *n_i;
		*sigma_i =*n_i > 1 ?  (*sigma_i - *n_i * *mu_i * *mu_i) / ( *n_i -1 ) : 0.0f;

	}
	cvdebug() << "evaluated mu sigma\n";


	T3DImage<T> *result  = new T3DImage<T>(data.get_size(), data.get_attribute_list());
	typename T3DImage<T>::iterator i = result->begin();

	for (int z = 0; z < (int)data.get_size().z; ++z) {
		for (int y = 0; y < (int)data.get_size().y; ++y) {
			for (int x = 0; x < (int)data.get_size().x; ++x, ++i)
				*i = get<T>(mu, sigma, x, y, z, *i);
		}
	}

	cvdebug() << "C3DMLV::operator () end\n";
	return P3DImage(result);
}

C3DMLVImageFilter::C3DMLVImageFilter(int hw):
	m_l(hw),
	m_kh(2 * hw + 1)
{
}

P3DImage C3DMLVImageFilter::do_filter(const C3DImage& image) const
{
	return mia::filter(*this, image);
}

C3DMLVImageFilterFactory::C3DMLVImageFilterFactory():
	C3DFilterPlugin("mlv"),
	m_hw(1)
{
	add_parameter("w", new CIntParameter(m_hw, 0, numeric_limits<int>::max(), false, "filter width parameter"));

}

C3DFilter *C3DMLVImageFilterFactory::do_create()const
{
	return new C3DMLVImageFilter(m_hw);
}

const string C3DMLVImageFilterFactory::do_get_descr()const
{
	return "Mean Least Variance 3D image filter";
}

bool  C3DMLVImageFilterFactory::do_test() const
{
	return false;
}


extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C3DMLVImageFilterFactory();
}

NS_END
