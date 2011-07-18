/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
 *
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
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

/* 
  LatexBeginPluginDescription{2D Transformations}
   
   \subsection{Vectorfield}
   \label{transform2d:vf}
   
   \begin{description}
   
   \item [Plugin:] vf
   \item [Description:] This plug-in implements a transformation that defines a translation for 
                        each point of the grid defining the domain of the transformation. 
   \item [Degrees of Freedom:] with the grid size $(n_x,n_y)$: $2* nx * ny$
  
   \end{description}
   This plug-in doesn't  take parameters. 

   LatexEnd  
 */

#include <limits>
#include <mia/core/msgstream.hh>
#include <mia/2d/transform/vectorfield.hh>
#include <mia/2d/transformfactory.hh>
#include <mia/2d/2dvfio.hh>

NS_MIA_BEGIN
using namespace std;

C2DGridTransformation::C2DGridTransformation(const C2DBounds& size):
	m_field(size), 
	m_upscale_interpolator_factory("bspline:d=1", "zero")
{
}

P2DTransformation C2DGridTransformation::do_upscale(const C2DBounds& size) const
{
	/* This implementation could be improved by using something like the spline interpolator 
	   that applies the scaling in a separable way. */

	TRACE("C2DGridTransformation::upscale");
	DEBUG_ASSERT_RELEASE_THROW(m_field.get_size().x != 0 && m_field.get_size().y != 0, 
				   "C2DGridTransformation::do_upscale: input field has a zero dimension"); 


	C2DGridTransformation *result = new C2DGridTransformation(size);

	unique_ptr<T2DInterpolator<C2DFVector> >
		interp(m_upscale_interpolator_factory.create(m_field)); 
	

	float x_mult = float(size.x) / (float)m_field.get_size().x;
	float y_mult = float(size.y) / (float)m_field.get_size().y;
	float ix_mult = 1.0f / x_mult;
	float iy_mult = 1.0f / y_mult;
	
	auto i = result->m_field.begin();
	
	for (unsigned int y = 0; y < size.y; y++){
		for (unsigned int x = 0; x < size.x; x++,++i){
			C2DFVector help(ix_mult * x, iy_mult * y);
			C2DFVector val = (*interp)(help);
			*i = C2DFVector(val.x * x_mult,val.y * y_mult);
		}
	}
	
	return P2DTransformation(result);
}

const C2DBounds& C2DGridTransformation::get_size() const
{
	return m_field.get_size();
}

bool C2DGridTransformation::save(const std::string& filename) const
{
	C2DIOVectorfield outfield(m_field);
	return C2DVFIOPluginHandler::instance().save(filename, outfield);
}

void C2DGridTransformation::add(const C2DTransformation& a)
{
	const C2DGridTransformation& other = dynamic_cast<const C2DGridTransformation&>(a);
	m_field += other.m_field;
}

void C2DGridTransformation::update(float step, const C2DFVectorfield& a)
{
	C2DFVectorfield::const_iterator inf = a.begin();
	C2DFVectorfield::const_iterator enf = a.end();
	C2DFVectorfield::iterator onf = m_field.begin();

	while (inf != enf)
		*onf++ += step * *inf++;

}

C2DTransformation *C2DGridTransformation::do_clone() const
{
	return new C2DGridTransformation(*this);
}

C2DTransformation *C2DGridTransformation::invert() const
{
	assert(0 && "not implemented"); 
	return new C2DGridTransformation(*this);
}


size_t C2DGridTransformation::degrees_of_freedom() const
{
	return 2 * m_field.size();
}


C2DFMatrix C2DGridTransformation::derivative_at(int x, int y) const
{
	C2DFMatrix result(C2DFVector(1.0, 0), C2DFVector(0, 1.0));
	result -= field_derivative_at(x,y);
	return result;
}

C2DFMatrix C2DGridTransformation::field_derivative_at(int x, int y) const
{
	C2DFMatrix result;

	const int sx = m_field.get_size().x;
	const int sy = m_field.get_size().y;

	if (y >= 0 && y < sy ) {
		if (x >= 0 && x < sx ) {
			C2DFVectorfield::const_pointer center = &m_field[sx * y + x];
			if (x > 0 && x < sx - 1) {
				result.x = (center[ 1] - center[-1]) * 0.5f;
			}
			if (y > 0 && y < sy - 1 ) {
				result.y = (center[ sx] - center[-sx]) * 0.5f;
			}
		}
	}
	return result;
}

void C2DGridTransformation::set_identity()
{
	fill(m_field.begin(),m_field.end(), C2DFVector(0,0));
}

float C2DGridTransformation::get_max_transform() const
{
	C2DFVectorfield::const_iterator i = m_field.begin();
	C2DFVectorfield::const_iterator e = m_field.end();

	assert(i != e);
	float value = i->norm2();

	++i;
	while (i != e) {
		float v = i->norm2();
		if (value < v)
			value = v;
		++i;
	}
	return sqrt(value);
}


CDoubleVector C2DGridTransformation::get_parameters() const
{
	CDoubleVector result(m_field.size() * 2);
	auto r = result.begin();
	for(auto f = m_field.begin(); f != m_field.end(); ++f) {
		*r++ = f->x;
		*r++ = f->y;
	}
	return result;
}

void C2DGridTransformation::set_parameters(const CDoubleVector& params)
{
	assert(2 * m_field.size() == params.size());
	auto r = params.begin();
	for(auto f = m_field.begin(); f != m_field.end(); ++f) {
		f->x = *r++;
		f->y = *r++;
	}
}


C2DGridTransformation::iterator_impl::iterator_impl(const C2DBounds& pos, const C2DBounds& size, 
						    C2DFVectorfield::const_iterator start):
	C2DTransformation::iterator_impl(pos, size), 
	m_current(start)
{
	if (pos.y < size.y && pos.x < size.x ) 
		m_value = C2DFVector(get_pos()) - *m_current; 
}

C2DTransformation::iterator_impl * C2DGridTransformation::iterator_impl::clone() const
{
	return new C2DGridTransformation::iterator_impl(get_pos(), get_size(), m_current); 
}

const C2DFVector&  C2DGridTransformation::iterator_impl::do_get_value()const
{
	return m_value; 
}

void C2DGridTransformation::iterator_impl::do_x_increment()
{
	++m_current; 
	m_value = C2DFVector(get_pos()) - *m_current; 
}

void C2DGridTransformation::iterator_impl::do_y_increment()
{
	++m_current; 
	m_value = C2DFVector(get_pos()) - *m_current; 
}


P2DImage C2DGridTransformation::apply(const C2DImage& image, const C2DInterpolatorFactory& ipf) const
{
	assert(image.get_size() == m_field.get_size());
	return transform2d(image, ipf, *this);
}

C2DFVector C2DGridTransformation::operator ()(const  C2DFVector& x) const
{
	return x - apply(x);
}

C2DGridTransformation::const_iterator C2DGridTransformation::begin() const
{
	return const_iterator(new iterator_impl(C2DBounds(0,0), 
						m_field.get_size(), m_field.begin()));
}

C2DGridTransformation::const_iterator C2DGridTransformation::end() const
{
	return const_iterator(new iterator_impl( m_field.get_size(), m_field.get_size(), 
						 m_field.end()));
}


C2DGridTransformation::field_iterator C2DGridTransformation::field_begin()
{
	return m_field.begin();
}

C2DGridTransformation::field_iterator C2DGridTransformation::field_end()
{
	return m_field.end();
}

C2DGridTransformation::const_field_iterator C2DGridTransformation::field_begin() const
{
	return m_field.begin();
}

C2DGridTransformation::const_field_iterator C2DGridTransformation::field_end()const
{
	return m_field.end();
}

void C2DGridTransformation::translate(const C2DFVectorfield& gradient, CDoubleVector& params) const
{
	assert(2 * params.size() != gradient.size());

	// translating the gradient also means multiplication with -1 
	// because the transformation is I-u(x)
	auto r = params.begin();
	for(auto f = gradient.begin(); f != gradient.end(); ++f, r+=2) {
		r[0] = -f->x;
		r[1] = -f->y;
	}
}

float C2DGridTransformation::pertuberate(C2DFVectorfield& v) const
{
	C2DFVectorfield::iterator iv = v.begin();
	float max_gamma = 0.0f;
	for (size_t y = 0; y < v.get_size().y; ++y)
		for (size_t x = 0; x < v.get_size().x; ++x, ++iv){
			const C2DFMatrix j = field_derivative_at(x,y);
			const C2DFVector ue = j * *iv;
			*iv -= ue;
			float gamma = iv->norm2();
			if (gamma > max_gamma)
				max_gamma = gamma;
		}
	return sqrt(max_gamma);
}

double C2DGridTransformation::dddgx_xxx(int x, int y) const 
{
	return 0.5 * ((m_field(x+2,y).x - 2 * m_field(x+1,y).x) - 
		      ( m_field(x-2,y).x - 2 * m_field(x-1,y).x)); 
}

double C2DGridTransformation::dddgy_yyy(int x, int y) const 
{
	return 0.5 * ((m_field(x,y+2).y - 2 * m_field(x,y+1).y) - 
		      ( m_field(x,y-2).y - 2 * m_field(x,y-1).y)); 
}

double C2DGridTransformation::dddgy_xxx(int x, int y) const 
{
	return 0.5 * ((m_field(x+2,y).y - 2 * m_field(x+1,y).y) - 
		      ( m_field(x-2,y).y - 2 * m_field(x-1,y).y)); 
}

double C2DGridTransformation::dddgx_yyy(int x, int y) const 
{
	return 0.5 * ((m_field(x,y+2).x - 2 * m_field(x,y+1).x) - 
		      ( m_field(x,y-2).x - 2 * m_field(x,y-1).x)); 
}


C2DFVector C2DGridTransformation::ddg_xx(int x, int y) const
{
	return m_field(x+1,y) + m_field(x-1,y) - 2 * m_field(x,y); 
}

C2DFVector C2DGridTransformation::ddg_xy(int x, int y) const
{
	return 0.25 * ((m_field(x+1,y+1) + m_field(x-1,y-1)) - 
		       (m_field(x+1,y-1) + m_field(x-1,y+1))); 
}


C2DFVector C2DGridTransformation::ddg_yy(int x, int y) const
{
	return m_field(x,y+1) + m_field(x,y-1) - 2 * m_field(x,y); 
}


C2DFVector C2DGridTransformation::dddg_xxy(int x, int y) const
{
	return 0.5 * (ddg_xx(x, y+1) - ddg_xx(x, y-1)); 	
}

C2DFVector C2DGridTransformation::dddg_yyx(int x, int y) const
{
	return 0.5 * (ddg_yy(x+1, y) - ddg_yy(x-1, y)); 	
}


C2DFVector C2DGridTransformation::get_graddiv_at(int x, int y) const
{

	const double dfx_xx =  (m_field(x+1,y).x + m_field(x-1,y).x - 2 * m_field(x,y).x);
	const double dfy_yy =  (m_field(x,y+1).y + m_field(x,y-1).y - 2 * m_field(x,y).y);
	
	const C2DFVector df_xxy = dddg_xxy(x, y); 
	const C2DFVector df_yyx = dddg_yyx(x,y); 
	const double dfx_xxx =  dddgx_xxx(x, y); 
	const double dfy_yyy =  dddgy_yyy(x, y); 

	const C2DFVector df_xy = ddg_xy(x,y); 

	const double dhx = (df_yyx.y + df_xxy.x) * (dfy_yy + df_xy.x) + 
		(df_xy.y + dfx_xx) * (df_xxy.y + dfx_xxx ); 
	
	const double dhy = (dfy_yyy + df_yyx.x) * (dfy_yy + df_xy.x) + 
		(df_xy.y + dfx_xx) * (df_yyx.y + df_xxy.x); 
	
	C2DFVector vv(2*dhx, 2*dhy); 
	cvdebug() << x << ", " << y <<  vv << "\n"; 
	return vv; 
}

C2DFVector C2DGridTransformation::get_gradcurl_at(int x, int y) const
{
	const double dfy_xx =  (m_field(x+1,y).y + m_field(x-1,y).y - 2 * m_field(x,y).y);
	const double dfx_yy =  (m_field(x,y+1).x + m_field(x,y-1).x - 2 * m_field(x,y).x);
	const double dfy_xxx =  dddgy_xxx(x, y); 
	const double dfx_yyy =  dddgx_yyy(x, y); 
	
	const C2DFVector df_xy = ddg_xy(x,y); 
	const C2DFVector df_xxy = dddg_xxy(x, y); 	
	const C2DFVector df_yyx = dddg_yyx(x,y); 


	const double p1 = dfy_xx - df_xy.x; 
	const double p2 = df_xy.y - dfx_yy;

	const double dhx = p1 * (dfy_xxx - df_xxy.x) + p2 * (df_xxy.y - df_yyx.x);
	
	const double dhy = p1 * (df_xxy.y - df_yyx.x) + p2 * (df_yyx.y - dfx_yyy);

	C2DFVector vv(2*dhx, 2*dhy); 
	cvdebug() << x << ", " << y <<  vv << "\n"; 
	return vv; 

}

float C2DGridTransformation::grad_divergence(double weight, CDoubleVector& gradient) const
{
	const int dx =  m_field.get_size().x;
	auto iv = m_field.begin() + dx + 1; 
	
	double result = 0.0;
	auto ig = gradient.begin() + 2*(2*dx + 2); 
	for(size_t y = 2; y < m_field.get_size().y - 2; ++y, iv += 4, ig += 8 )
		for(size_t x = 2; x < m_field.get_size().x - 2; ++x, ++iv, ig += 2){

			const double dfx_xx =  (iv[ 1].x + iv[- 1].x - 2 * iv[0].x);
			const double dfy_yy =  (iv[dx].y + iv[-dx].y - 2 * iv[0].y);
			const double dfy_xy = 0.25 * (iv[dx + 1].y + iv[-dx-1].y - iv[1-dx].y - iv[dx-1].y);
			const double dfx_xy = 0.25 * (iv[dx + 1].x + iv[-dx-1].x - iv[1-dx].x - iv[dx-1].x); 

			const double p1 = dfy_yy + dfx_xy; 
			const double p2 = dfx_xx + dfy_xy; 

			C2DFVector grd = get_graddiv_at(x, y); 
			// this needs to be tested 
			ig[0] +=  weight * grd.x; 
			ig[1] +=  weight * grd.y; 


			const double v = p1 * p1 + p2 * p2; 
			result += v; 
		}
	return weight * result;
}

double C2DGridTransformation::grad_curl(double weight, CDoubleVector& gradient) const
{
	const int dx =  m_field.get_size().x;
	auto iv = m_field.begin() + dx + 1; 
	
	double result = 0.0;
	auto ig = gradient.begin() + dx + 1; 	
	for(size_t y = 1; y < m_field.get_size().y - 1; ++y, iv += 2, ig += 4 )
		for(size_t x = 1; x < m_field.get_size().x - 1; ++x, ++iv,  ig += 2) {
			const double dfy_xx = iv[ 1].y + iv[- 1].y - 2 * iv[0].y;
			const double dfx_yy = iv[dx].x + iv[-dx].x - 2 * iv[0].x;
			const double dfy_xy = iv[dx].y + iv[-1].y -  iv[0].y - iv[dx-1].y;
			const double dfx_xy = iv[1].x - iv[0].x - iv[1-dx].x + iv[-dx].x; 
			
			const double dhx = dfy_xx - dfx_xy;
			const double dhy = dfx_yy - dfy_xy;
			

			C2DFVector grd = get_gradcurl_at(x, y); 
			ig[0] +=  weight * grd.x; 
			ig[1] +=  weight * grd.y; 

			result += (dhx * dhx + dhy * dhy); 
		}
	return weight * result;
}

double C2DGridTransformation::get_divcurl_cost(double wd, double wr, CDoubleVector& gradient) const
{
	double result = 0.0; 
	// todo: if wd == wr run special case 

	if (wd > 0) 
		result += grad_divergence(wd, gradient); 
	if (wr > 0) 
		result += grad_curl(wr, gradient); 
	
	return result; 
}

double C2DGridTransformation::get_divcurl_cost(double wd, double wr) const
{
	double result = 0.0; 
	// todo: if wd == wr run special case 
	const int dx =  m_field.get_size().x;
	auto iv = m_field.begin() + dx + 1; 
	
	for(size_t y = 1; y < m_field.get_size().y - 1; ++y, iv += 2 )
		for(size_t x = 1; x < m_field.get_size().x - 1; ++x, ++iv) {
			const double dfx_xx = iv[ 1].x + iv[- 1].x - 2 * iv[0].x;
			const double dfy_yy = iv[dx].y + iv[-dx].y - 2 * iv[0].y;
			const double dfy_xy = iv[dx].y + iv[-1].y -  iv[0].y - iv[dx-1].y;
			const double dfx_xy = iv[1].x - iv[0].x - iv[1-dx].x + iv[-dx].x; 
			const double dfy_xx = iv[ 1].y + iv[- 1].y - 2 * iv[0].y;
			const double dfx_yy = iv[dx].x + iv[-dx].x - 2 * iv[0].x;
			
			const double dhx = dfy_xx - dfx_xy;
			const double dhy = dfx_yy - dfy_xy;


			const double dgx = dfx_xx + dfy_xy;
			const double dgy = dfy_yy + dfx_xy;
			

			result += wr * (dhx * dhx + dhy * dhy) + 
				wd * (dgx * dgx + dgy * dgy); 
		}
	return result; 
}


float C2DGridTransformation::get_jacobian(const C2DFVectorfield& v, float delta) const
{
	assert(v.get_size() == get_size());
	const int dx = v.get_size().x;
	float j_min = numeric_limits<float>::max();

	for(size_t y = 1; y < get_size().y - 1; ++y) {
		C2DFVectorfield::const_iterator iu = m_field.begin_at(1,y);
		C2DFVectorfield::const_iterator iv = v.begin_at(1,y);
		for(size_t x = 1; x < get_size().x - 1; ++x, ++iu, ++iv) {
			C2DFVector jx((iu[1] - iu[-1]) + (delta * (iv[1] - iv[-1])));
			C2DFVector jy((iu[dx] - iu[-dx]) + (delta * (iv[dx] - iv[-dx])));

			const float j = (2.0 - jx.x) * (2.0 - jy.y) - jx.y * jy.x;
			if ( j_min > j) {
				j_min = j;
			}

		}
	}
	return 0.25 * j_min;
}

EXPORT_2D C2DGridTransformation operator + (const C2DGridTransformation& a, const C2DGridTransformation& b)
{
	assert( a.get_size() == b.get_size());

	C2DGridTransformation result(a.get_size());

	C2DFVectorfield::iterator ri = result.field_begin();
	C2DFVectorfield::const_iterator bi = b.field_begin();

	for (size_t y = 0; y < a.get_size().y; ++y)  {
		for (size_t x = 0; x < a.get_size().x; ++x, ++ri, ++bi)  {
			const C2DFVector xi = C2DFVector(x,y) - *bi;
			*ri = a.apply(xi) +  *bi;
		}
	}
	return result;
}

/**
   Transformation creator 
 */
class C2DGridTransformCreator: public C2DTransformCreator {
	virtual P2DTransformation do_create(const C2DBounds& size) const;
};

P2DTransformation C2DGridTransformCreator::do_create(const C2DBounds& size) const
{
	return P2DTransformation(new C2DGridTransformation(size));
}



/**
   Plugin class to create the creater.  
 */
class C2DGridTransformCreatorPlugin: public C2DTransformCreatorPlugin {
public:
	C2DGridTransformCreatorPlugin();
	virtual C2DTransformCreator *do_create() const;
	virtual bool do_test() const;
	const std::string do_get_descr() const;
};

C2DGridTransformCreatorPlugin::C2DGridTransformCreatorPlugin():
	C2DTransformCreatorPlugin("vf")
{
}

C2DTransformCreator *C2DGridTransformCreatorPlugin::do_create() const
{
	return new C2DGridTransformCreator();
}

bool C2DGridTransformCreatorPlugin::do_test() const
{
	return true;
}

const std::string C2DGridTransformCreatorPlugin::do_get_descr() const
{
	return "plugin to create vectorfield transformations";
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C2DGridTransformCreatorPlugin();
}



NS_MIA_END
