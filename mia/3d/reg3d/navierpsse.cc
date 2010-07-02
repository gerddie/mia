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
  This plug-in implements the navier-stokes operator like registration model
  that accounts for linear elastic and fluid dynamic registration.
  Which model is used depends on the selected time step.

  To solve the PDE a full SOR is implemented.
*/

#ifdef __POWERPC__
#include <altivec.h>
#undef bool
#include <unistd.h>
#elif __SSE__
#include <xmmintrin.h>
#else
#error SSE required
#endif


#include <iomanip>
#include <set>
#include <limits>
#include <cstdlib>

//#include <mia/3d/3DDatafield.cxx>

#include <mia/3d/model.hh>








NS_MIA_USE
using namespace boost;

using namespace std;

NS_BEGIN(navierpsse_regmodel)

class C3DNavierRegModel: public C3DRegModel {
public:
	C3DNavierRegModel(float mu, float lambda, size_t maxiter, float epsilon);

	bool  test_kernel();
private:




	virtual void do_solve(const C3DFVectorfield& b, C3DFVectorfield& x) const;
	virtual float do_force_scale() const;

//	void set_flags(C3DUBImage::iterator loc)const;
//	float  solve_at(const C3DFVector& b, v4sf *x, const v4sf *param)const;
	float _M_mu;
	float _M_lambda;
	float _M_a1,_M_a, _M_b, _M_c, _M_a_b, _M_b_4;
//	float _M_omega;
	float _M_epsilon;
	size_t _M_max_iter;
};

class C3DNavierRegModelPlugin: public C3DRegModelPlugin {
public:
	C3DNavierRegModelPlugin();
	C3DNavierRegModelPlugin::ProductPtr do_create()const;

private:
	virtual bool do_test() const;
	const string do_get_descr()const;

	float _M_mu;
	float _M_lambda;
	float _M_epsilon;
	int _M_maxiter;
};


typedef struct {float x,y,z,a; } float4;

#ifdef __POWERPC__
typedef vector float v4sf;
#else

typedef __m128 v4sf;
#define sse_addps   _mm_add_ps
#define sse_mulps   _mm_mul_ps
#define sse_shufps  _mm_shuffle_ps
#define sse_subps   _mm_sub_ps
#define sse_addss   _mm_add_ss
#define sse_sqrtss  _mm_sqrt_ss
#define sse_prefetch(ADR,DUMMY,HINT) // _mm_prefetch((const char*)ADR, HINT)

#define SSE_HINT_T0 _MM_HINT_T0
#define SSE_HINT_T1 _MM_HINT_T1
#define SSE_HINT_T2 _MM_HINT_T2
#define SSE_HINT_NTA _MM_HINT_NTA

#endif //__POWERPC__
typedef union {
	float4 mem;
	v4sf   xmm;
} xchg;

struct C3DVectorToVF4 {
	xchg  operator ()(const C3DFVector& x) const {
		xchg result;
		const float4 f = {x.x, x.y, x.z, 0.0f};
		result.mem = f;
		return result;
	}
};

struct VF4ToC3DVector {
	C3DFVector  operator ()(const xchg& x) const {
		return C3DFVector(x.mem.x, x.mem.y, x.mem.z);
	}
};

C3DNavierRegModel::C3DNavierRegModel(float mu, float lambda, size_t maxiter,  float epsilon):
	_M_mu(mu),
	_M_lambda(lambda),
	_M_epsilon(epsilon),
	_M_max_iter(maxiter)
{
	float a = mu;
	float b = lambda + mu;
	_M_c = 1 / (6.0f * a + 2.0f * b);
	_M_b_4 = 0.25f * b * _M_c;
	_M_a_b = ( a + b ) * _M_c;
	_M_a = a * _M_c;

	cvdebug() << "initialise model with mu=" << mu << " lambda=" << lambda << "\n";
}


float C3DNavierRegModel::do_force_scale() const
{
	cvdebug() << "Force scale = " <<_M_c << "\n";
	return _M_c;
}

inline static void set_flags(C3DUBImage::iterator loc, const int dx, const int dxy)
{

	C3DUBImage::iterator lmz = loc - dxy;
	C3DUBImage::iterator lpz = loc + dxy;
	C3DUBImage::iterator lmy = loc - dx;
	C3DUBImage::iterator lpy = loc + dx;

	lmz[     -dx] = 1;
	lmz[ -    1 ] = 1;
	lmz[      0 ] = 1;
	lmz[ +    1 ] = 1;
	lmz[      dx] = 1;

	lmy[ - 1 ] = 1;
	lmy[   0 ] = 1;
	lmy[   1 ] = 1;

	loc[ - 1 ] = 1;
	loc[   0 ] = 1;
	loc[ + 1 ] = 1;

	lpy[ - 1 ] = 1;
	lpy[   0 ] = 1;
	lpy[ + 1 ] = 1;

	lpz[   -dx ] = 1;
	lpz[ -   1 ] = 1;
	lpz[     0 ] = 1;
	lpz[     1 ] = 1;
	lpz[    dx ] = 1;
}

#ifdef __POWERPC__
inline static float  solve_at(const xchg& b, xchg *v, const xchg& param, const int dx, const int dxy)
{
	const vector float zero = {0,0,0,0};

	const vector unsigned char S_XY = { 4, 5, 6, 7, 0, 1, 2, 3,  0xF, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF };
	const vector unsigned char S_XZ = { 8, 9, 10, 11, 0xF, 0xF, 0xF, 0xF, 0, 1, 2, 3, 0xF, 0xF, 0xF, 0xF };
        const vector unsigned char S_YZ = { 0xF, 0xF, 0xF, 0xF, 8, 9, 10, 11, 4, 5, 6, 7, 0xF, 0xF, 0xF, 0xF };
        const vector unsigned char S_10 = { 4, 5, 6, 7, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF };
        const vector unsigned char S_20 = { 8, 9, 10, 11, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF };
        const vector unsigned char S_AB = { 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0xF, 0xF, 0xF, 0xF};
        const vector unsigned char S_A =  { 4, 5, 6, 7, 4, 5, 6, 7, 4, 5, 6, 7, 0xF, 0xF, 0xF, 0xF};
        const vector unsigned char S_B4 = { 8, 9, 10, 11, 8, 9, 10, 11, 8, 9, 10, 11,  0xF, 0xF, 0xF, 0xF};



	xchg old_v = *v;

	register xchg& vv = *v;

	const  xchg *vm = v - dxy;
	const  xchg *vp = v + dxy;
        const  xchg *vpm = vp - dx;
	const  xchg *vmp = vm + dx;
	const  xchg *vpp = vp + dx;
	const  xchg *vmm = vm - dx;




	const  xchg *vdm = v - dx;
	const  xchg *vdp = v + dx;


	register v4sf a, m, h;
        m = vec_perm( param.xmm, param.xmm, S_AB);
	register v4sf r   = vec_add (v[-1].xmm, v[ 1].xmm);
	register v4sf vs2 = vec_add (vdm[ 0 ].xmm, vdp[ 0 ].xmm);
	register v4sf vs3 = vec_add (vm[ 0 ].xmm, vp[ 0 ].xmm);

	r = vec_madd(r, m, b.xmm);
        h = vec_perm( param.xmm, param.xmm, S_A);


	vs2 = vec_add (vs2, vs3);

	a = vec_add(vdm[-1].xmm, vdp[ 1].xmm);
	m = vec_add(vdp[-1].xmm, vdm[ 1].xmm);

	r = vec_madd(vs2, h, r);


	vs2 = vec_add(vm[-1].xmm, vp[ 1 ].xmm);
	a = vec_perm( a, a, S_XY);



	register v4sf vs4 = vec_add(vm[1].xmm, vp[ -1 ].xmm);
	vs3 = vec_add(vmm->xmm, vpp->xmm);
	vs2 = vec_perm( vs2, vs2, S_XZ);

        h = vec_perm( param.xmm, param.xmm, S_B4);

	register v4sf vs5 = vec_add(vmp->xmm, vpm->xmm);
	vs3 = vec_perm( vs3, vs3, S_YZ);
	m = vec_perm( m, m, S_XY);
	a = vec_add(a, vs2);
	vs4 = vec_perm( vs4, vs4, S_XZ);
	a = vec_add(a, vs3);
	m = vec_add(m, vs4);
	vs5 = vec_perm( vs5, vs5, S_YZ);
	m = vec_add(m, vs5);

	a = vec_sub(a, m);

	vv.xmm = r = vec_madd(a, h, r);

	r = vec_sub(r, old_v.xmm);

	r = vec_madd(r, r,zero);

	vs2 = vec_perm(r, r, S_10);
	m = vec_perm(r, r, S_20);

	r = vec_add(r, vs2);
	r = vec_add(r, m);
	r = vec_rsqrte(r);
        r = vec_re(r);

	return ((float *)&r)[0];


}
#elif __SSE__


inline static float  solve_at(const xchg& b, xchg *v, const xchg& param, const int dx, const int dxy)
{

 	const int S_XY = 0xF1; // 3 3 0 1
	const int S_XZ = 0xCE; // 3 0 3 2
	const int S_YZ = 0xDB; // 3 1 2 3


	const int S_AB = 0xC0; // 0 0 0 3
	const int S_A  = 0xD5; // 1 1 1 3
	const int S_B4 = 0xEA; // 2 2 2 3

	register const v4sf p = param.xmm;

	const  xchg *vm = v - dxy;
	sse_prefetch(vm, 0, SSE_HINT_T1);
	const  xchg *vp = v + dxy;
	sse_prefetch(vp, 0, SSE_HINT_T1);

	const  xchg *vdm = v - dx;
	sse_prefetch(vdm, 0, SSE_HINT_T1);
	const  xchg *vdp = v + dx;
	sse_prefetch(vdp, 0, SSE_HINT_T1);

	register v4sf a, m;
	register v4sf r   = sse_addps (v[-1].xmm, v[ 1].xmm);
	register v4sf vs2 = sse_addps (vdm[ 0 ].xmm, vdp[ 0 ].xmm);
	register v4sf vs3 = sse_addps (vm[ 0 ].xmm, vp[ 0 ].xmm);

	const  xchg *vdmm = vdm - 1;
	sse_prefetch(vdmm, 0, SSE_HINT_T1);

	const  xchg *vdpm = vdp - 1;
	sse_prefetch(vdpm, 0, SSE_HINT_T1);


	register v4sf help = sse_shufps( p, p, S_AB);
	r = sse_mulps(r, help);
	vs2 = sse_addps (vs2, vs3);

	r = sse_addps (r, b.xmm);
	help = sse_shufps( p, p, S_A);
	vs2 = sse_mulps(vs2, help);


	a = sse_addps(vdmm->xmm, vdp[ 1].xmm);
	m = sse_addps(vdpm->xmm, vdm[ 1].xmm);

	r = sse_addps (r, vs2);

	vs2 = sse_addps(vm[-1].xmm, vp[ 1 ].xmm);
	a = sse_shufps( a, a, S_XY);

	register v4sf vs4 = sse_addps(vm[1].xmm, vp[ -1 ].xmm);
	vs3 = sse_addps(vm[-dx].xmm, vp[dx].xmm);
	vs2 = sse_shufps( vs2, vs2, S_XZ);

	register v4sf vs5 = sse_addps(vm[dx].xmm, vp[-dx].xmm);

	vs3 = sse_shufps( vs3, vs3, S_YZ);
	m = sse_shufps( m, m, S_XY);
	a = sse_addps(a, vs2);
	vs4 = sse_shufps( vs4, vs4, S_XZ);

	a = sse_addps(a, vs3);
	m = sse_addps(m, vs4);

	vs5 = sse_shufps( vs5, vs5, S_YZ);
	m = sse_addps(m, vs5);

	vs3 = v->xmm;

	a = sse_subps(a, m);

	help = sse_shufps( p, p, S_B4);
	a = sse_mulps(a, help);

	v->xmm = r = sse_addps(r, a);

	r = sse_subps(r, vs3);

	r = sse_mulps(r, r);

	vs2 = sse_shufps(r, r, 0x1);
	xchg bla;
	m = sse_shufps(r, r, 0x2);

	r = sse_addss(r, vs2);
	r = sse_addss(r, m);
	bla.xmm = sse_sqrtss(r);

	return bla.mem.x;
}

#else
#error compile with SSE (>=Pentium 3) or ALTIVEC (PowerPC > G4) support
#endif

void C3DNavierRegModel::do_solve(const C3DFVectorfield& b, C3DFVectorfield& v) const
{
        // init velocity fields
        float start_residuum = 0.0;
        float residuum;
        float thresh = 0.0;

        size_t i = 0;
        assert(b.get_size() == v.get_size());

        const int  dx = b.get_size().x;
        const int  dxy = b.get_plane_size_xy();

        C3DFImage residua(b.get_size());
        fill(residua.begin(), residua.end(), 0.0f);

        C3DUBImage update_flags1(b.get_size());
        C3DUBImage update_flags2(b.get_size());

        C3DUBImage& update_flags = update_flags1;
        C3DUBImage& dset_flags = update_flags2;

        fill(update_flags.begin(), update_flags.end(), 1);

        xchg *vcache = reinterpret_cast<navierpsse_regmodel::xchg*>(valloc(b.size() * sizeof (v4sf)));
        xchg *bcache = reinterpret_cast<navierpsse_regmodel::xchg*>(valloc( b.size() * sizeof(v4sf)));

        transform(v.begin(), v.end(), vcache, C3DVectorToVF4());
        transform(b.begin(), b.end(), bcache, C3DVectorToVF4());

        xchg param = {{_M_a_b, _M_a, _M_b_4, 0.0f}};

        do {
		++i;
		residuum = 0;

                #ifdef __SSE__
		xchg *cciv = vcache;
		// assume a cache line size of 64 byte
		for (size_t x = 0; x < 2 * b.get_size().x; x += 4, cciv+=4) {
			sse_prefetch(cciv, 0, SSE_HINT_T1);
                }
                __builtin_prefetch(&param, 0, SSE_HINT_T2);
                #endif

                size_t n_pixels = 0;

                #ifdef __OPENMP
                #pragma omp parallel reduce(+:n_pixels) shared(residua, vcache, bcache, param) schedule(static)
                #endif

                for (size_t z = 1; z < b.get_size().z-1; z++) {
			size_t step = dx + dxy * z;
			double res = 0.0;

			xchg *iv = vcache + step;
			xchg *ib = bcache + step;
			C3DFImage::iterator ir = residua.begin() + step;
			C3DUBImage::iterator iu = update_flags.begin() + step;
			C3DUBImage::iterator is = dset_flags.begin() + step;


			for (size_t y = 1; y < b.get_size().y-1; y++) {

				#ifdef __POWERPC__
				xchg *cache = iv + dxy;
				vec_dst(&cache[-dx].xmm, 16 << 3 | dx << 8 | 1 << 16, 0);
				vec_dst(&cache->xmm, 16 << 3 | dx << 8 | 1 << 16, 1);
				vec_dst(&cache[dx].xmm , 16 << 3 | dx << 8 | 1 << 16, 2);
				#endif
#ifdef __SSE__
				__builtin_prefetch(iv, 0, 2);
#endif
				++ib;
				++iv;
				++ir;
				++iu;
				++is;
					for (size_t x = 1; x < b.get_size().x-1; x++, ++ib, ++iv, ++ir, ++iu, ++is){
					if (*iu) {
#ifdef __SSE__
						__builtin_prefetch(ib, 0, 1);
#endif
						++n_pixels;
						*iu = 0;
						*ir = solve_at(*ib, iv, param, dx, dxy);
					}
					if (*ir > thresh)
						set_flags(is, dx, dxy);

					res += *ir;
				}
				++ib;
				++iv;
				++ir;
				++iu;
				++is;

			}
#ifdef __OPENMP
#pragma omp critical
#endif
                        residuum += res;
                }

		if (i ==1)
			start_residuum = residuum;


		swap(update_flags, dset_flags);


		thresh = (residuum * residuum) / (start_residuum * residua.size() * (i+1));

		cvdebug() << "SOLVE " << setw(3) << i <<":" << residuum << " : " << thresh<< " : "<< n_pixels <<"\n";

		if (residuum < 1)
			break;

	} while (i < _M_max_iter && residuum / start_residuum > _M_epsilon);

	transform(vcache, vcache + v.size(), v.begin(),  VF4ToC3DVector());

	free(bcache);
	free(vcache);
}





bool  C3DNavierRegModel::test_kernel()
{

	// assume mu = 1, lambda = 2
	//float a_b = 0.4f;
	//float a   = 0.1f;
	//float b_4 = 0.075;

	xchg param = {{_M_a_b, _M_a, _M_b_4, 0.0f}};

	C3DBounds size(3,3,3);
	size_t mid_index = 1 + size.x + size.x * size.y;

	xchg v[size.x * size.y * size.z];

	xchg *iv = v;
	C3DFVector b(1.0, 2.0, 3.0);



	C3DVectorToVF4 convertA;
	VF4ToC3DVector convertB;
	xchg vb = convertA(b);
	for (size_t z = 0; z < size.z; ++z)
		for (size_t y = 0; y < size.y; ++y)
			for (size_t x = 0; x < size.x; ++x, ++iv)
				*iv = convertA(C3DFVector(y * x, 2 * z * y, 3 * x * z));

	xchg *vi = v + mid_index;

#if 0
	const C3DFVector q_test( 0.75, 0.25, 0.5);
	const C3DFVector q_eval = get_q(vi);

	cvdebug() << q_eval << " vs. " << q_test << "\n";
	assert( (q_eval - q_test).norm() < 0.00001);

	// test p eval
	const C3DFVector p_test( 2.0, 4.0f, 6.0f);
	const C3DFVector p_eval = get_p(b, vi);


	if ((p_eval - p_test).norm() > 0.00001) {
		cvfail() << p_eval << " vs. " << p_test << "\n";
		return false;
	}

	// test q eval
#endif
	C3DFVector test0(1.75, 2.25, 3.5);

	C3DFVector test(2.75, 4.25, 6.5);

	float res = solve_at(vb, vi, param, 3, 9);
	cvdebug() << res << " vs. " << test0.norm() <<"\n";
	cvdebug() << convertB(v[mid_index]) << " vs. " << test <<"\n";


#ifdef __POWERPC__
	if (fabs(res - test0.norm()) < 0.1) {
		cvfail() << res << " .vs " << test0.norm() << "\n";
		return false;
	}

#else
	if (fabs(res - test0.norm()) > 0.01) {
		cvfail() << res << " .vs " << test0.norm() << "\n";
		return false;
	}
#endif
	return ((test - convertB(v[mid_index])).norm() < 0.0001);

}



C3DNavierRegModelPlugin::C3DNavierRegModelPlugin():
	C3DRegModelPlugin("navierpsse"),
	_M_mu(1.0),
	_M_lambda(1.0),
	_M_epsilon(0.0001),
	_M_maxiter(40)
{
	typedef CParamList::PParameter PParameter;
	add_parameter("mu", new CFloatParameter(_M_mu, 0.0, numeric_limits<float>::max(),
							   false, "isotropic compliance"));
	add_parameter("lambda", new CFloatParameter(_M_lambda, 0.0, numeric_limits<float>::max(),
							       false, "isotropic compression"));

	add_parameter("epsilon", new CFloatParameter(_M_epsilon, 0.000001, 0.1,
								false, "stopping parameter"));
	add_parameter("iter", new CIntParameter(_M_maxiter, 10, 10000,
							   false, "maximum number of iterations"));
}

C3DNavierRegModelPlugin::ProductPtr C3DNavierRegModelPlugin::do_create()const
{
	return C3DNavierRegModelPlugin::ProductPtr(new C3DNavierRegModel(_M_mu, _M_lambda,
									 _M_maxiter, _M_epsilon));
}

bool  C3DNavierRegModelPlugin::do_test() const
{
	C3DNavierRegModel model(1.0, 2.0, 10, 0.001);

	return model.test_kernel();
}

const string C3DNavierRegModelPlugin::do_get_descr()const
{
	return "navier-stokes based registration model";
}

extern "C"  EXPORT CPluginBase *get_plugin_interface()
{
	return new C3DNavierRegModelPlugin();
}

NS_END

