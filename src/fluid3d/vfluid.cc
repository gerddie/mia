/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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


/*
  removed a bug in perturbationAt thanks to zhangzhijun <zjzhang@ee.cuhk.edu.hk>
*/


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <mia/3d/deformer.hh>

#include <mia/core/threadedmsg.hh>
#include <tbb/parallel_reduce.h>
#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>


//#include "3Dinterpolator.hh"
#include "vfluid.hh"
#include "eqn_solver.hh"

NS_MIA_USE

CWatch Clock;
int  STARTSIZE=16;

using namespace std;
using namespace mia;

template <typename T, typename S>
struct __dispatch_convert {
	static T3DImage<T> apply(const T3DImage<S>& image) {
		T3DImage<T> result(image.get_size(), image);
		copy(image.begin(), image.end(), result.begin());
		return result;
	}
};

template <typename T>
struct __dispatch_convert<T,T> {
	static T3DImage<T> apply(const T3DImage<T>& image) {
		return image;
	}
};

template <typename T>
struct FC2DImage2Type: public TFilter< T3DImage<T> > {
	template <typename S>
	T3DImage<T> operator ()(const T3DImage<S>& image) const {
		return __dispatch_convert<T,S>::apply(image);
	}
};


TFluidReg::TFluidReg(const TFluidRegParams& params,TLinEqnSolver *_solver, const C3DInterpolatorFactory& _ipf):
	lambda(params.Lambda),
	my(params.My),
	Start(0,0,0),
	End(params.source->get_size()),
	matter_threshold(params.matter_threshold),
	solver(_solver),
	initialStepsize(params.InitialStepsize),
	ipf(_ipf)
{
	FC2DImage2Type<float> Image2Float;
	src = mia::filter(Image2Float, *params.source);
	ref =  mia::filter(Image2Float, *params.reference);
	tmp = src;

	memset(&Measurement,0,sizeof(Measurement));
	a = my;
	b = lambda + my;
	c = 1/(6*a+2*b);

	assert(params.reference);

#ifdef OPTIMIZE_REGION

	C3DFImage diff(src->get_size());
	transform(src->begin(), src->end(), ref.begin(), diff.begin(), 
		  [](float x, float y){return x-y;}); 

	diff->get_region_of_interest(&Start,&End,matter_threshold);
	Start.x -= 3;
	Start.y -= 3;
	Start.z -= 3;

	End.x += 3;
	End.y += 3;
	End.z += 3;
#endif
	if (Start.x-1 >= End.x) Start.x =1;
	if (Start.y-1 >= End.y) Start.y =1;
	if (Start.z-1 >= End.z) Start.z =1;

	if (End.x >= src.get_size().x) End.x = src.get_size().x-1;
	if (End.y >= src.get_size().y) End.y = src.get_size().y-1;
	if (End.z >= src.get_size().z) End.z = src.get_size().z-1;


	ROI = End - Start;

	V = new C3DFVectorfield(ROI);
}

TFluidReg::~TFluidReg()
{
	delete V;
}

inline unsigned int TFluidReg::GetLinCoord(unsigned int x,
					       unsigned int y,unsigned int z)const
{
	x -= Start.x;
	y -= Start.y;
	z -= Start.z;
	assert(x < ROI.x);
	assert(y < ROI.y);
	assert(z < ROI.z);

	return (x + ROI.x * ( y + ROI.y * z ) );
}


inline C3DFVector TFluidReg::forceAt(int x, int y ,int z,float *misma)const
{
	float mis = tmp(x,y,z) - ref(x,y,z);
	*misma = fabs(mis);
	int idx = x + (y + z * tmp.get_size().y) * tmp.get_size().x;
	cvdebug() << idx << "\n";
	return mis * tmp.get_gradient(idx);
}


inline C3DFVector TFluidReg::forceAt(int hardcode,float *misma)const
{
	float mis = tmp[hardcode] - ref[hardcode];
	*misma = fabs(mis);
	return mis * tmp.get_gradient(hardcode);
}

float  TFluidReg::calculatePerturbation()
{
	auto callback = [this](const tbb::blocked_range<size_t>& range, float gamma) -> float{
		CThreadMsgStream thread_stream;
		C3DBounds max_p; 
		for (auto z = range.begin(); z != range.end();++z)
			for (size_t y = Start.y; y < End.y; y++)  {
				for (size_t x = Start.x; x < End.x; x++)  {
					float g = perturbationAt(x, y, z);
					if (g > gamma){
						gamma = g;
						max_p.x = x; 
						max_p.y = y; 
						max_p.z = z; 
					}
				}
			}	
		return gamma; 
	}; 

	float gamma = tbb::parallel_reduce( tbb::blocked_range<size_t>(Start.z, End.z, 1), 
					   0.0f, callback, 
					   [](float x, float y){return max(x,y);}); 

	cvinfo() << "pert = " << gamma << "\n";
	return gamma;
}

float  TFluidReg::calculateJacobian()const
{
	auto callback = [this](const tbb::blocked_range<size_t>& range, float jmin) -> float{
		CThreadMsgStream thread_stream;
		for (auto z = range.begin(); z != range.end();++z)
			for (size_t y = Start.y+1; y < End.y-1; y++)
				for (size_t x = Start.x+1; x < End.x-1; x++) {
					float j = jacobianAt(x, y, z);
					if (j < jmin) jmin = j;
				}
		return jmin; 
	}; 
	
	
	float jmin = tbb::parallel_reduce( tbb::blocked_range<size_t>(Start.z + 1, End.z-1, 1), 
					   numeric_limits<float>::max(), callback, 
					   [](float x, float y){return min(x,y);}); 
	cvinfo() << "jacobian = " << jmin << endl;
        return jmin;
}

float TFluidReg::work(P3DImage NewSource, C3DFVectorfield& uIn )
{
	FC2DImage2Type<float> Image2Float;
	src = ::mia::filter(Image2Float, *NewSource);
	InitTemps();
	memset(&Measurement,0,sizeof(Measurement));
        bool regrid = true;
	int trys = 0;
	u = &uIn;
	double gamma = 0;
	float stepSize = initialStepsize;
	float dmin; // = src->get_mismatch(ref);


	//	C3DFVectorfield Backup(u->get_size());
	//	copy(u->begin(), u->end(), Backup.begin());

	int max_iter = 7680/(ROI.x + ROI.y +ROI.z);

	if (max_iter > 100)
		max_iter = 100;


	dmin = calculateForces();

	for (int i = 0; (i < max_iter) && (trys < 5); i++)  {

		cvmsg() << setw(3) << i << ": " << dmin << "\r";
                // Algorithm 2, step 2+3:

		// Algorithm 2, step 4:
                solvePDE();
		// Algorithm 2, step 5+6:

		gamma = calculatePerturbation();
step5:
		delta = stepSize / gamma;
                // Algorithm 2, step 7:
                double j = calculateJacobian();

                if (j < 0.5)  {
                        if (regrid == true)  {
				if (stepSize > 0.2) {
					stepSize -= 0.1;
				}else {
					break;
				}
				goto step5;
                        };

                        // return deformation field to merge ist with global field
			break;
                } else {
                        // apply weighted perturbation to displacement field
                        // see algorithm 2, step 7, second part
                        regrid = false;
			ApplyShift();

			mismatch = calculateForces();

			while (mismatch >= dmin) {
				if (stepSize > 0.2) {
					stepSize -= 0.1;
					//delta = -0.1 / gamma;
					//ApplyShift(); // revert deformation
					//mismatch = calculateForces();
				} else {
					trys++;
					break;
				}
			}
			if (mismatch < dmin)  {
				if (mismatch < 0.8 * dmin && stepSize < 0.8)
					stepSize += 0.1;

				if (mismatch < 0.99 * dmin)
					trys =0;
				else
					trys++;

				dmin = mismatch;

				//				copy(u->begin(), u->end(), Backup.begin());

				/*
				 Backup=*u; //backup better fit
				Backup.make_single_ref();
				*/
			}
		}
        }
	//        if (verbose) fprintf(stderr, "\n");
	//	*u = Backup; // restore better fit
	DeleteTemps();

	cvmsg() << "\n";
	return dmin;
}

void TFluidReg::InitTemps()
{
    	B = new C3DFVectorfield(ROI);
}

void TFluidReg::DeleteTemps()
{
    	delete B;
}

float	TFluidReg::calculateForces()
{
	auto callback = [this](const tbb::blocked_range<size_t>& range, float mismatch) -> float{
		CThreadMsgStream thread_stream;
		C3DBounds max_p; 
		for (auto z = range.begin(); z != range.end();++z) {
			int iz = z - Start.z;
			int iy = 0;
			for (size_t y = Start.y; y < End.y; y++, iy++)  {
				int ix =0;
				for (size_t x = Start.x; x < End.x; x++,ix++)  {
					float mis;
					(*B)(ix,iy,iz) = forceAt(x,y,z,&mis);
					mismatch += mis;
				}
			}
                }
		return mismatch; 
        }; 
	
	float mismatch = tbb::parallel_reduce( tbb::blocked_range<size_t>(Start.z, End.z, 1), 
					       0.0f, callback, 
					       [](float x, float y){return x + y;}); 
	

	return mismatch/ref.size();
}

float  TFluidReg::jacobianAt(int x, int y, int z)const
{
	const int usize_x = u->get_size().x;
	const int bsize_x = B->get_size().x;
	const int usize_xy = u->get_plane_size_xy();
	const int bsize_xy = B->get_plane_size_xy();
	C3DFVector *up =  &(*u)(x,y,z);

	C3DFVector ux = up[1] - up[-1];
        C3DFVector uy = up[usize_x] - up[-usize_x];
        C3DFVector uz = up[usize_xy] - up[-usize_xy];

	C3DFVector *bp =  &(*B)(x - Start.x, y - Start.y, z - Start.z);

	C3DFVector rx = bp[1] - bp[-1];
        C3DFVector ry = bp[bsize_x] - bp[- bsize_x];
        C3DFVector rz = bp[bsize_xy] - bp[- bsize_xy];

	float a = 2 - ux.x - delta * rx.x;
        float b =   - uy.x - delta * ry.x;
        float c =   - uz.x - delta * rz.x;
        float d =   - ux.y - delta * rx.y;
        float e = 2 - uy.y - delta * ry.y;
        float f =   - uz.y - delta * rz.y;
        float g =   - ux.z - delta * rx.z;
        float h =   - uy.z - delta * ry.z;
        float k = 2 - uz.z - delta * rz.z;

        return (a*(e*k-f*h)-b*(d*k-f*g)+c*(d*h-e*g)) * 0.125;
}


float  TFluidReg::perturbationAt(int x, int y, int z)
{
	int ix = x - Start.x;
	int iy = y - Start.y;
	int iz = z - Start.z;
	C3DFVector vi = (*V)(ix,iy,iz);
	C3DFVector ux = (*u)(x+1,y,z) - (*u)(x-1,y,z);
	C3DFVector uy = (*u)(x,y+1,z) - (*u)(x,y-1,z);
	C3DFVector uz = (*u)(x,y,z+1) - (*u)(x,y,z-1);
	C3DFVector _r = vi - 0.5f * (vi.x*ux + vi.y*uy + vi.z*uz);
	(*B)(ix,iy,iz) = _r;
	return _r.norm();
}

void  TFluidReg::ApplyShift()
{
	auto callback = [this](const tbb::blocked_range<size_t>& range){
		CThreadMsgStream thread_stream;

		for (auto z = range.begin(); z != range.end();++z) {
			int iz = z - Start.z; 
			int iy = 0;
			for (size_t y = Start.y; y < End.y; y++,iy++) {
				std::transform(u->begin_at(Start.x,y,z), u->begin_at(End.x,y,z), 
					       B->begin_at(0, iy, iz), u->begin_at(Start.x,y,z), 
					       [delta](const C3DFVector& uv, const C3DFVector& ub) {return uv + delta*ub;}); 
			}
		}
	}; 
	tbb::parallel_for(tbb::blocked_range<size_t>(Start.z, End.z, 1), callback); 

	tmp = C3DFImage(src.get_size());



	FDeformer3D deformer(*u, ipf);
	::mia::filter_equal_inplace<FDeformer3D,C3DImage>(deformer, src, tmp);

}

void TFluidReg::solvePDE()
{
	float t_start = Clock.get_seconds();
	Measurement.PDEEval++;
	solver->solve(*B,V);
	Measurement.niter = -1;
	Measurement.PDETime += Clock.get_seconds() - t_start;
}

double g_start;
static P3DFVectorfield do_transform(const TFluidRegParams& params,
				    P3DFVectorfield in_shift,
				    TLinEqnSolver *solver,TMeasureList *measure_list,
				    const C3DInterpolatorFactory& ipf
				    )
{
	assert(solver);

	TMeasurement Measure;
	memset(&Measure,0,sizeof(Measure));
	Measure.Size = params.source->get_size();

	P3DFVectorfield u(new C3DFVectorfield(params.source->get_size()));
	P3DFVectorfield s = in_shift ? in_shift : P3DFVectorfield(new C3DFVectorfield(params.source->get_size()));

	double mismatch = HUGE;
	bool do_continue = true;

	unique_ptr< TFluidReg> Register(new TFluidReg(params,solver, ipf));


	for (int i= 0; i < 30 && do_continue; i++){
		cvmsg() << "[" << setw(2) << i << "] @ size:" << params.source->get_size() << endl;

		FDeformer3D deformer(*s, ipf);
		P3DImage ModelDeformed = ::mia::filter(deformer, *params.source);

		double new_mismatch = Register->work(ModelDeformed, *u);

		if (new_mismatch < 0) { // returned because no better fit
			new_mismatch = -new_mismatch;
			do_continue = false;
		}

		Measure.PDEEval += Register->Measurement.PDEEval;
		Measure.PDETime += Register->Measurement.PDETime;
		Measure.niter   += Register->Measurement.niter;
		Measure.Regrids++;

		if (new_mismatch < mismatch) {
			if (new_mismatch > mismatch * 0.95) {
				do_continue = false;
			}
			*u += *s;
			u->swap(*s);
			u->clear();
			mismatch = new_mismatch;
		}else{
			break;
		}
	}
	Measure.allovertime = Clock.get_seconds() - g_start;
	cvmsg() << "time: " <<  Measure.allovertime << endl;
	measure_list->insert(measure_list->end(),Measure);

	return s;
}


// upscale a vector field by using tri-linear interpolation
// and scaling of the vectors
P3DFVectorfield upscale( const C3DFVectorfield& vf, C3DBounds size)
{
	P3DFVectorfield Result(new C3DFVectorfield(size));
	float x_mult = float(size.x) / (float)vf.get_size().x;
	float y_mult = float(size.y) / (float)vf.get_size().y;
	float z_mult = float(size.z) / (float)vf.get_size().z;
	float ix_mult = 1.0f / x_mult;
	float iy_mult = 1.0f / y_mult;
	float iz_mult = 1.0f / z_mult;

	auto callback = [&](const tbb::blocked_range<size_t>& range){
		CThreadMsgStream thread_stream;
		
		for (auto z = range.begin(); z != range.end();++z) { 
			auto i = Result->begin_at(0,0,z);
			for (unsigned int y = 0; y < size.y; y++)
				for (unsigned int x = 0; x < size.x; x++,++i){
					C3DFVector help(ix_mult * x, iy_mult * y, iz_mult * z);
					C3DFVector val = vf.get_interpol_val_at(help);
					*i = C3DFVector(val.x * x_mult,val.y * y_mult, val.z * z_mult);
				}
		}
	}; 
	tbb::parallel_for(tbb::blocked_range<size_t>(0, size.z, 1), callback);

	return Result;
}

size_t log2i(size_t x)
{
	size_t result = 0;
	while (x > 1) {
		x >>= 1;
		++result;
	}
	return result;
}

P3DFVectorfield fluid_transform(const TFluidRegParams& params,TLinEqnSolver *solver,
				bool use_multigrid, bool use_fullres,TMeasureList *measure_list,
				const C3DInterpolatorFactory& ipf
				)
{

	P3DFVectorfield current_shift;
	bool change_res = false;
	if (use_multigrid){

		int x_shift = log2i(params.source->get_size().x / STARTSIZE);
		int y_shift = log2i(params.source->get_size().y / STARTSIZE);
		int z_shift = log2i(params.source->get_size().z / STARTSIZE);

		P3DImage RefScale;
		P3DImage ModelScale;

		while (x_shift || y_shift || z_shift){ // do while there is a downscaling

			C3DBounds BlockSize(1 << x_shift, 1 << y_shift, 1 << z_shift);
			cvmsg() << "Blocksize = " << BlockSize << "\n";

			stringstream downscale_descr;
			downscale_descr << "downscale:bx=" << BlockSize.x
					<< ",by=" << BlockSize.y
					<< ",bz=" << BlockSize.z;

			auto downscaler =
				C3DFilterPluginHandler::instance().produce(downscale_descr.str().c_str());


			if (!ModelScale) ModelScale =  downscaler->filter(*params.source);
			if (!RefScale)   RefScale =    downscaler->filter(*params.reference);

			if (current_shift) {
				// if we already have a shift
				if (current_shift->get_size() != ModelScale->get_size()) {

					current_shift = upscale(*current_shift, ModelScale->get_size());
				}
			}else{
				current_shift.reset(new C3DFVectorfield(ModelScale->get_size()));

			}


			FDeformer3D deformer(*current_shift, ipf);
			P3DImage ModelDeformed = ::mia::filter(deformer, *ModelScale);

			TFluidRegParams newparams = params;
			newparams.source = ModelDeformed;
			newparams.reference = RefScale;

			// do the registration at this level
			P3DFVectorfield shift = do_transform(newparams,P3DFVectorfield(),solver,measure_list, ipf);

			// if we already have a shift, add the newly calculated
			if (current_shift) {
				*shift += *current_shift;
			}
			current_shift = shift;

			if (change_res) { // every second time we change the res

				if (x_shift){x_shift--;}
				if (y_shift){y_shift--;}
				if (z_shift){z_shift--;}
				change_res = false;

				// delete the scaled reference
				RefScale.reset();
				ModelScale.reset();
			}else{
				change_res = true;
			}
		}
	}


	if (current_shift) {
		current_shift = upscale(*current_shift, params.reference->get_size());
	}else {
		current_shift.reset(new C3DFVectorfield(params.reference->get_size()));
	}


	// Since the upscaling will lead to a larger Vectorfield, then the real image is
	// cut away all unneccessary data

	if (use_fullres) {
		// Now for the final registration at pixel-level
		// Without multigrid the only step
		current_shift = do_transform(params, current_shift,solver,measure_list, ipf);
	}

	cvmsg() << "Registration complete" << endl;
	return current_shift;
}

