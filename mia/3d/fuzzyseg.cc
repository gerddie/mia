/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2016 Gert Wollny
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <fstream>
#include <cstdlib>
#include <cmath>
#include <string>
//#include <dlfcn.h>
#include <algorithm>

#include <mia/core/cmdlineparser.hh>
#include <mia/core/histogram.hh>
#include <mia/core/kmeans.hh>
#include <mia/3d/fuzzyclustersolver_cg.hh>
#include <mia/3d/fuzzyseg.hh>

NS_MIA_BEGIN
using namespace std;


/* maximum number of iterations for EM-Algorithm */
#define _MAXIT 5


typedef vector<C3DFImage*> C3DFImageVec;


class CSegment3d : public TFilter<C3DFImageVec> {
public:
	CSegment3d(const unsigned int& nClasses, const float& res);
	~CSegment3d ();
	P3DImage get_out_image () const;

	template <typename T>
	CSegment3d::result_type operator () (const T3DImage<T>& data);
private:
	unsigned int m_nClasses;
	float 	     m_res;
	P3DImage     m_out;

};


/// regularization for the clustring PDE
#define _LAMBDA1	2e4
#define _LAMBDA2	2e5
/// maximum number of iterations for the solver
#define _MAX_ITER_PDE 	1000


using namespace std;

template <class Data3D>
vector<double> Isodata3d (const Data3D& src_image, unsigned int nClasses, unsigned int  maxPixVal)
{
	vector<double> clCenter(nClasses);

	/** classmembership of each possible pixelvalue 0-maxPixVal */
	vector<unsigned int> classOfGreyval( maxPixVal + 1 );

	// discard all intensities below
	unsigned int ignore = 0;

	if (nClasses <= 1 || nClasses > maxPixVal) {
		string out_of_range = ("Isodata3d: parameter --no-of-classes out of range");
		throw invalid_argument(out_of_range);

	};

	// let initial cluster centers be evenly distributed between 0...maxPixelVal
	double dCenters = (double)(maxPixVal)/(nClasses+1);  		 // get distance between clCenters
	for (unsigned int i = 0; i < nClasses; i++)
		clCenter[i] = (i+1) *dCenters; //save clCenters

	// first pass: build a histogram
	THistogram<THistogramFeeder<int> > histo(THistogramFeeder<int>(0, maxPixVal + 1, maxPixVal + 1));
	histo.push_range (src_image.begin(), src_image.end ());

	// now find cluster centers
	double diff = HUGE;
	for (unsigned int t = 0; diff > 1.0 && t < 15; t++) {

		// for every grey value
		for (unsigned int gV = ignore+1; gV < maxPixVal+1; gV++) {

			double dmin = maxPixVal * maxPixVal;
			// for each class
			for (unsigned int nc = 0; nc < nClasses; nc++) {

				//distance: shade of grey to class center
				double dx = gV - clCenter[nc];
				dx *= dx;

				if (dx < dmin) {
					//match nearest class
					classOfGreyval[gV] = nc;
					dmin = dx;
				};

			};

		};

		diff = 0;

		for (unsigned int c = 0; c < nClasses; c++) {

			//sum up #pixels belonging to class c
			//multiply #pixel with grey value (g) & sum up
			double nPixels = 0;
			double sumPixelVal = 0;

			for (unsigned int g = ignore+1; g < maxPixVal+1; g++) {
				if (classOfGreyval[g] == c) {

					nPixels 	+= histo[g];
					sumPixelVal += histo[g] * g;
				};
			};

			//if pixels in class -> get average grey value of cls
			sumPixelVal = nPixels? sumPixelVal/nPixels : 0;
			double dx   = sumPixelVal - clCenter[c];
			clCenter[c] = sumPixelVal;
			diff += dx*dx;

		};
	};

	// compute classOfGreyvalue for classification
	for (unsigned int i = 0; i < ignore+1; i++)
		classOfGreyval[i] = 0;

	for (unsigned int gV = ignore+1; gV < maxPixVal+1; gV++) {

		double dmin = HUGE;
		int newClass = 0;

		for (unsigned int c = 0; c < nClasses; c++) {

			// distance: grey value to current cluster center
			double dValCtr = abs((double)gV - clCenter[c]);
			// save class-1 with shortest distance
			if (dValCtr < dmin) {
				dmin = dValCtr;
				newClass = c;
			};
		};
		// set classOfGreyvalue
		classOfGreyval[gV] = newClass + 1;
	};

	return clCenter;
};



// solves the PDE  (W + lambda1 * H1 + lambda2 * H2) = f
void solvePDE (C3DFImage& weight_image, C3DFImage& force_image, C3DFImage& gain_image, double lambda1, double lambda2, double *firstnormr0, double relres, double min_res)
{

	solve_sCG *solver = new  solve_sCG(weight_image, force_image, gain_image, lambda1, lambda2, relres, min_res);
	solver->solve ( _MAX_ITER_PDE, firstnormr0);
	solver->get_solution (gain_image);
	delete solver;

	return;

}

template <class Data3D>
int estimateGain (C3DFImage& gain_image, const Data3D& src_image, vector<C3DFImage*>& cls_image,
		  vector<double> &clCenter, unsigned int classes, double * firstnormr0, float relres, const vector<char>& border)
{

	const unsigned int nx = src_image.get_size().x;
	const unsigned int ny = src_image.get_size().y;
	const unsigned int nz = src_image.get_size().z;
	int t=0;

	// precompute force (f) and weight matrix (w)
	C3DFImage force_image (src_image.get_size());
	C3DFImage weight_image (src_image.get_size());


	for (unsigned int z = 2; z < nz-2; z++)  {
		for (unsigned int y = 2; y < ny-2; y++)  {
			for (unsigned int x = 2; x < nx-2; x++)  {
				double forcePixel = 0;
				double weightPixel = 0;
				for (unsigned int k = 0; k < classes; k++)  {
					double uk = (*cls_image[k])(x, y, z);
					double vk = clCenter[k];
					double v = uk * uk * vk;
					forcePixel += v;
					weightPixel += v * vk;
				};

				forcePixel *= src_image(x, y, z);

				// korrigiere randbedingungen
				long i = z * ny * nx + y*nx + x;
				if (!border[i-nx])    forcePixel += _LAMBDA1 + 12 * _LAMBDA2;
				if (!border[i-1])     forcePixel += _LAMBDA1 + 12 * _LAMBDA2;
				if (!border[i+1])     forcePixel += _LAMBDA1 + 12 * _LAMBDA2;
				if (!border[i+nx])    forcePixel += _LAMBDA1 + 12 * _LAMBDA2;
				if (!border[i-nx*ny]) forcePixel += _LAMBDA1 + 12 * _LAMBDA2;
				if (!border[i+nx*ny]) forcePixel += _LAMBDA1 + 12 * _LAMBDA2;

				if (!border[i-nx-1])       forcePixel -= 2 * _LAMBDA2;
				if (!border[i-nx+1])       forcePixel -= 2 * _LAMBDA2;
				if (!border[i+nx-1])       forcePixel -= 2 * _LAMBDA2;
				if (!border[i+nx+1])       forcePixel -= 2 * _LAMBDA2;
				if (!border[i-nx*ny-1])    forcePixel -= 2 * _LAMBDA2;
				if (!border[i-nx*ny+1])    forcePixel -= 2 * _LAMBDA2;
				if (!border[i-nx*ny-nx])   forcePixel -= 2 * _LAMBDA2;
				if (!border[i-nx*ny+nx])   forcePixel -= 2 * _LAMBDA2;
				if (!border[i+nx*ny-1])    forcePixel -= 2 * _LAMBDA2;
				if (!border[i+nx*ny+1])    forcePixel -= 2 * _LAMBDA2;
				if (!border[i+nx*ny-nx])   forcePixel -= 2 * _LAMBDA2;
				if (!border[i+nx*ny+nx])   forcePixel -= 2 * _LAMBDA2;

				if (!border[i - 2*nx])    forcePixel -= _LAMBDA2;
				if (!border[i - 2])       forcePixel -= _LAMBDA2;
				if (!border[i + 2])       forcePixel -= _LAMBDA2;
				if (!border[i + 2*nx])    forcePixel -= _LAMBDA2;
				if (!border[i - 2*nx*ny]) forcePixel -= _LAMBDA2;
				if (!border[i + 2*nx*ny]) forcePixel -= _LAMBDA2;

				force_image(x, y, z) = (float)(forcePixel);
				weight_image(x, y, z) = (float)(weightPixel);
			};
		};
	};
	// C.Wolters:
	// now solve system using scaled CG
	solvePDE(weight_image, force_image, gain_image, _LAMBDA1, _LAMBDA2, firstnormr0,
		 double(relres), 1);

	return t;

};



CSegment3d::CSegment3d(const unsigned int& nClasses, const float& res):
	m_nClasses(nClasses), m_res(res)
{
}

CSegment3d::~CSegment3d ()
{
}

P3DImage CSegment3d::get_out_image () const
{
	return m_out;
}


/*! Functor operator
  \param data any data struture that hold an (STL- style) iterator
  \returns the corrected image, as well as the class-probability images
*/
template <typename T>
CSegment3d::result_type CSegment3d::operator () (const T3DImage<T>& data)
{

	const size_t nx = data.get_size().x;
	const size_t ny = data.get_size().y;
	const size_t nz = data.get_size().z;
	const size_t noOfPixels = nx*ny*nz;

	// number of pixels in image
	// TODO: was ist u ??
	vector<double> u(m_nClasses);

	// Field to store the border
	vector<char> border(noOfPixels);

	double firstnormr0 = 1.0;
	double dumax, sum, nom, den, dist;

	unsigned long k = 0;
	typename T3DImage<T>::const_iterator data_itr = data.begin();

	for( ; data_itr < data.end(); k++, data_itr++ ) {

		// initialize border information
		if ( fborder(k, nx, ny, nz) )
			border[k] = 0;
		else
			border[k]=1;

	}

	// Create and initialize gain-field
	C3DFImage gain_image( data.get_size(), data);

	C3DFImage::iterator gainBegin = gain_image.begin();
	C3DFImage::iterator gainEnd   = gain_image.end();
	fill (gainBegin, gainEnd, 1.0);

	// class probability image, compute initial class centers
	vector<double> clCenter(m_nClasses);
	vector<unsigned short> buffer(data.size()); 
	kmeans(data.begin(), data.end(), buffer.begin(), clCenter); 


	// some verbose output
	cvmsg()  << "intl. class centers:"  << clCenter << "\n"; 

	// Algorithm step 1
	// create class membership volumes

	C3DFImageVec cls_image;
	for (size_t i = 0; i < m_nClasses; ++i)  {
		cls_image.push_back(new C3DFImage ( data.get_size(), data));
	}

	for (unsigned int t = 0; t < _MAXIT; t++)  {

		// Algorithm step 2:
		// estimate gain field

		estimateGain (gain_image, data, cls_image, clCenter, m_nClasses,
			      &firstnormr0, m_res, border);
		if (firstnormr0 < 1000)
			firstnormr0 = 1.0;

		// Algorithm step 3:
		// recompute class memberships
		dumax = 0;
		typename T3DImage<T>::const_iterator sp  = data.begin();

		// Compute each pixel of the layer
		for (unsigned int z = 0; z < nz; z++)  {
			for (unsigned int y = 0; y < ny; y++)  {
				for (unsigned int x = 0; x < nx; x++, sp++)  {
					// get Value
					double pixVal = *sp;
					if (pixVal == 0)
						continue;
					sum = 0;
					// get difference from gain-field
					double gainVal = gain_image(x, y, z);
					for (unsigned int k = 0; k < m_nClasses; k++)  {
						dist = pixVal-gainVal*clCenter[k];
						u[k] = dist? 1/(dist*dist): HUGE;
						sum += u[k];
					};

					for (unsigned int k = 0; k < m_nClasses; k++)  {
						double un = u[k]/sum;
						double uk = (*cls_image[k])(x, y, z);
						if (::fabs(un-uk) > dumax)
							dumax = ::fabs(un-uk);
						(*cls_image[k])(x, y, z) = un;
					}
				}
			}
		}

		// Algorithm step 4:
		// recompute class clCenters
		for (unsigned int k = 0; k < m_nClasses; k++) {
			nom = 0;
			den = 0;
			typename T3DImage<T>::const_iterator sp = data.begin();
			for (unsigned int z = 0; z < nz; z++)  {
				for (unsigned int y = 0; y < ny; y++)  {
					for (unsigned int x = 0; x < nx; x++, sp++)  {
						double pixVal = *sp;
						if (pixVal == 0)
							continue;
						double gainVal = gain_image(x, y, z);
						double uj      = (*cls_image[k])(x, y, z);
						nom += uj*uj*gainVal*pixVal;
						den += uj*uj*gainVal*gainVal;
					}
				}
			}
			clCenter[k] = den ? nom/den: 0;

		}

		cvmsg() << "\r[" << t << "/4]" << flush;
		cvmsg() << " class centers:" << clCenter << "\n"; 
		if (dumax < 0.01)
			break;

	};


	// compute corrected image
	T3DImage<T> *corrected_image = new T3DImage<T> (data.get_size(), data);
	C3DFImage::iterator gain_itr = gain_image.begin();
	typename T3DImage<T>::iterator corrected_itr = corrected_image->begin();

	for ( data_itr = data.begin(); data_itr < data.end();
	      gain_itr++, data_itr++, corrected_itr++) {

		double corrected_val = (double) *data_itr;
		if (*gain_itr != 0)
			corrected_val /= *gain_itr;
		*corrected_itr = (T) corrected_val;
	}

	corrected_image->set_attribute("class_centers",
				       PAttribute(new CVDoubleAttribute( clCenter)));
	m_out.reset(corrected_image);
	for (size_t i = 0; i < cls_image.size(); ++i) {
		cls_image[i]->set_attribute("class_number",
					    PAttribute(new CIntAttribute( i )));
		cls_image[i]->set_attribute("class_centers",
					    PAttribute(new CVDoubleAttribute( clCenter)));
	}
	return cls_image;
}

template <>
CSegment3d::result_type CSegment3d::operator () (const C3DBitImage& /*data*/) {
	throw invalid_argument("Segemtation of binary images makes no sense");
}




EXPORT_3D P3DImage fuzzy_segment_3d(const C3DImage& src, size_t noOfClasses, float residuum, C3DImageVector& classes)
{
	CSegment3d segment3D (noOfClasses, residuum);
	C3DFImageVec imagesVector = mia::accumulate (segment3D, src);

	for (size_t i=0; i < noOfClasses; i++) {
		classes.push_back( P3DImage(imagesVector[i]) );
	}
	return segment3D.get_out_image();
}

NS_MIA_END

