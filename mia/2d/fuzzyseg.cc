/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2011 Gert Wollny
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
#include <mia/2d/fuzzyClusterSolverCG.hh>
#include <mia/2d/fuzzyClusterSolverSOR.hh>
#include <mia/2d/fuzzyseg.hh>

NS_MIA_BEGIN
using namespace std;


/* maximum number of iterations for EM-Algorithm */
#define _MAXIT 200 


typedef vector<C2DFImage*> C2DFImageVec;


class CSegment2d : public TFilter<C2DFImageVec> {
public:
	CSegment2d(const unsigned int& nClasses, const SFuzzySegParams& params);
	~CSegment2d ();
	P2DImage get_out_image () const;
	P2DImage get_gain_image () const;

	template <typename T>
	CSegment2d::result_type operator () (const T2DImage<T>& data);
private:
	unsigned int m_nClasses;
	SFuzzySegParams m_params; 
	P2DImage     m_out;
	P2DImage     m_gain_image;

};


/// maximum number of iterations for the solver
#define _MAX_ITER_PDE 	1000


using namespace std;

template <class Data2D>
vector<double> Isodata2d (const Data2D& src_image, unsigned int nClasses, unsigned int  maxPixVal)
{
	vector<double> clCenter(nClasses);

	/** classmembership of each possible pixelvalue 0-maxPixVal */
	vector<unsigned int> classOfGreyval( maxPixVal + 1 );

	// discard all intensities below
	unsigned int ignore = 0;

	if (nClasses <= 1 || nClasses > maxPixVal) {
		string out_of_range = ("Isodata2d: parameter --no-of-classes out of range");
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
void solvePDE (C2DFImage& weight_image, C2DFImage& force_image, C2DFImage& gain_image, const SFuzzySegParams& params, double *firstnormr0, double min_res)
{
#if 0 

	C2DSolveCG solver(weight_image, force_image, gain_image, lambda1, lambda2, relres, min_res);
	solver.solve ( _MAX_ITER_PDE, firstnormr0);
	solver.get_solution (gain_image);
#else 
	C2DFuzzyClusterSolver solver(weight_image,params.lambda1, params.lambda2, 1000);
	solver.solve (force_image, gain_image);
#endif 
}

template <class Data2D>
int estimateGain (C2DFImage& gain_image, const Data2D& src_image, vector<C2DFImage*>& cls_image,
		  vector<double> &clCenter, unsigned int classes, double * firstnormr0, const SFuzzySegParams& params)
{

	const unsigned int nx = src_image.get_size().x;
	const unsigned int ny = src_image.get_size().y;
	int t=0;

	// precompute force (f) and weight matrix (w)
	C2DFImage force_image (src_image.get_size());
	C2DFImage weight_image (src_image.get_size());


	for (unsigned int y = 2; y < ny-2; y++)  {
		for (unsigned int x = 2; x < nx-2; x++)  {
			double forcePixel = 0;
			double weightPixel = 0;
			for (unsigned int k = 0; k < classes; k++)  {
				double uk = (*cls_image[k])(x, y);
				double vk = clCenter[k];
				double v = uk * uk * vk;
				forcePixel += v;
				weightPixel += v * vk;
			};
			
			forcePixel *= src_image(x, y);
#if 0 			
			// korrigiere randbedingungen
			long i = y*nx + x;
			if (!border[i-nx])    forcePixel += _LAMBDA1 + 8 * _LAMBDA2;
			if (!border[i-1])     forcePixel += _LAMBDA1 + 8 * _LAMBDA2;
			if (!border[i+1])     forcePixel += _LAMBDA1 + 8 * _LAMBDA2;
			if (!border[i+nx])    forcePixel += _LAMBDA1 + 8 * _LAMBDA2;
		
			if (!border[i-nx-1])       forcePixel -= 2 * _LAMBDA2;
			if (!border[i-nx+1])       forcePixel -= 2 * _LAMBDA2;
			if (!border[i+nx-1])       forcePixel -= 2 * _LAMBDA2;
			if (!border[i+nx+1])       forcePixel -= 2 * _LAMBDA2;
			
			if (!border[i - 2*nx])    forcePixel -= _LAMBDA2;
			if (!border[i - 2])       forcePixel -= _LAMBDA2;
			if (!border[i + 2])       forcePixel -= _LAMBDA2;
			if (!border[i + 2*nx])    forcePixel -= _LAMBDA2;
#endif 			
			force_image(x, y) = (float)(forcePixel);
			weight_image(x, y) = (float)(weightPixel);
		};
	};
	// C.Wolters:
	// now solve system using scaled CG
	solvePDE(weight_image, force_image, gain_image, params, firstnormr0,1);

	return t;

};



CSegment2d::CSegment2d(const unsigned int& nClasses, const SFuzzySegParams& params):
	m_nClasses(nClasses), 
	m_params(params)
{
}

CSegment2d::~CSegment2d ()
{
}

P2DImage CSegment2d::get_out_image () const
{
	return m_out;
}

P2DImage CSegment2d::get_gain_image () const
{
	return m_gain_image;
}


/*! Functor operator
  \param data any data struture that hold an (STL- style) iterator
  \returns the corrected image, as well as the class-probability images
*/
template <typename T>
CSegment2d::result_type CSegment2d::operator () (const T2DImage<T>& data)
{

	const size_t nx = data.get_size().x;
	const size_t ny = data.get_size().y;
	const size_t noOfPixels = nx*ny;

	// number of pixels in image
	// TODO: was ist u ??
	vector<double> u(m_nClasses);

	// Field to store the border
	vector<char> border(noOfPixels);

	double firstnormr0 = 1.0;
	double nom, den, dist;

	// get type of iterator
	typedef T itype;

	// maximum pixel value in image
	auto maximum =	max_element(data.begin(), data.end());
	assert(maximum == data.end() || *maximum >= 1);
	itype iMax = *maximum;

	// increment maximum Pixel Value by one to take 0 into account
	cvmsg()  << " The Maximum pixel value is " << (double) iMax << endl;

	unsigned long k = 0;
	auto data_itr = data.begin();

	for( ; data_itr < data.end(); k++, data_itr++ ) {

		// initialize border information
		if ( fborder(k, nx, ny) )
			border[k] = 0;
		else
			border[k] = 1;

	}

	// Create and initialize gain-field
	C2DFImage gain_image( data.get_size(), data);

	fill (gain_image.begin(), gain_image.end(), 1.0);

	// class probability image, compute initial class centers
	vector<double> clCenter = Isodata2d (data, m_nClasses, (unsigned int) iMax);

	// some verbose output
	cvmsg()  << "intl. class centers:" ;
	for (unsigned int k = 0; k < m_nClasses; k++)
		cverb << " [" << k << "] " << clCenter[k];
	cverb << endl;

	// Algorithm step 1
	// create class membership volumes

	C2DFImageVec cls_image;
	for (size_t i = 0; i < m_nClasses; ++i)  {
		cls_image.push_back(new C2DFImage ( data.get_size(), data));
	}
	if (0) {
		vector<C2DFImage::iterator> cls_it( m_nClasses ); 
		transform(cls_image.begin(), cls_image.end(), cls_it.begin(), 
			  [](C2DFImage *cls) { return cls->begin(); }); 
			
		for(auto sp = data.begin(); sp != data.end(); ++sp) {
			double pixVal = *sp;
			if (pixVal == 0)
				continue;
			double sum = 0;
			for (unsigned int k = 0; k < m_nClasses; k++)  {
				dist = pixVal - clCenter[k];
				u[k] = dist? 1/(dist*dist): HUGE;
				sum += u[k];
			};
				
			for (unsigned int k = 0; k < m_nClasses; k++)  {
				double un = u[k]/sum;
				*cls_it[k] = un;
			}
		}
	}

	for (unsigned int t = 0; t < _MAXIT; t++)  {

		// Algorithm step 2:
		// estimate gain field

		estimateGain (gain_image, data, cls_image, clCenter, m_nClasses,
			      &firstnormr0, m_params);
		if (firstnormr0 < 1000)
			firstnormr0 = 1.0;

		// Algorithm step 3:
		// recompute class memberships
		double dumax = 0;
		auto sp  = data.begin();

		// Compute each pixel of the layer
		
		for (unsigned int y = 0; y < ny; y++)  {
			for (unsigned int x = 0; x < nx; x++, sp++)  {
				// get Value
				double pixVal = *sp;
				if (pixVal == 0)
					continue;
				double sum = 0;
				// get difference from gain-field
				double gainVal = gain_image(x, y);
				for (unsigned int k = 0; k < m_nClasses; k++)  {
					dist = pixVal-gainVal*clCenter[k];
					u[k] = dist? 1/(dist*dist): HUGE;
					sum += u[k];
				};
				
				for (unsigned int k = 0; k < m_nClasses; k++)  {
					double un = u[k]/sum;
					double uk = (*cls_image[k])(x, y);
					if (::fabs(un-uk) > dumax)
						dumax = ::fabs(un-uk);
					(*cls_image[k])(x, y) = un;
				}
			}
		}
		
		// Algorithm step 4:
		// recompute class clCenters
		for (unsigned int k = 0; k < m_nClasses; k++) {
			nom = 0;
			den = 0;
			typename T2DImage<T>::const_iterator sp = data.begin();
			for (unsigned int y = 0; y < ny; y++)  {
				for (unsigned int x = 0; x < nx; x++, sp++)  {
					double pixVal = *sp;
					if (pixVal == 0)
						continue;
					double gainVal = gain_image(x, y);
					double uj      = (*cls_image[k])(x, y);
					nom += uj*uj*gainVal*pixVal;
					den += uj*uj*gainVal*gainVal;
				}
			}
			clCenter[k] = den ? nom/den: 0;
			
		}
		
		cvmsg() << "\r[" << t << "]" << flush;
		cvmsg() << " class centers:";
		
		for (unsigned int k = 0; k < m_nClasses; k++)
			cverb << " [" << k << "] " << clCenter[k];

		cverb << " dumax=" << dumax << "\n"; 
		if (dumax < 0.01)
			break;
		
	};
	
	
	// compute corrected image
	T2DImage<T> *corrected_image = new T2DImage<T> (data.get_size(), data);
	C2DFImage::iterator gain_itr = gain_image.begin();
	typename T2DImage<T>::iterator corrected_itr = corrected_image->begin();
	
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
	m_gain_image.reset(new C2DFImage(gain_image)); 
	return cls_image;
}

template <>
CSegment2d::result_type CSegment2d::operator () (const C2DBitImage& /*data*/) {
	throw invalid_argument("Segemtation of binary images makes no sense");
}



EXPORT_2D P2DImage fuzzy_segment_2d(const C2DImage& src, size_t noOfClasses, const SFuzzySegParams& params, 
				    C2DImageVector& classes, P2DImage& gain)
{
	CSegment2d segment2D (noOfClasses, params);
	C2DFImageVec imagesVector = mia::accumulate (segment2D, src);

	for (size_t i=0; i < noOfClasses; i++) {
		classes.push_back( P2DImage(imagesVector[i]) );
	}
	gain = segment2D.get_gain_image(); 
	return segment2D.get_out_image();
}

NS_MIA_END

