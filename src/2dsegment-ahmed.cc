/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2012 Gert Wollny
 *
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

/*
   This program implements and enhances the paper:
   Mohamed N. Ahmed et. al, "A Modified Fuzzy C-Means Algorithm for Bias Field 
   estimation and Segmentation of MRI Data", IEEE Trans. on Medical Imaging, 
   Vol. 21, No. 3, March 2002
   
   changes are: 
   - p = 2
   - exp
   - 
*/


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


#include <mia/core/histogram.hh>
#include <mia/core/cmdlineparser.hh>
#include <mia/2d.hh>

#include <memory>
#include <vector>

using namespace mia;
using namespace std; 

typedef vector<C2DFImage> C2DFImageVec;

const SProgramDescription g_description = {
	"Analysis, filtering, combining, and segmentation of 2D images", 
	
	"This program implements a variation of the paper:"
	"Mohamed N. Ahmed et. al, \"A Modified Fuzzy C-Means Algorithm for Bias Field "
	"estimation and Segmentation of MRI Data\", IEEE Trans. on Medical Imaging, "
	"Vol. 21, No. 3, March 2002, changes are: p=2, and exp", 
	
	"Run a 5-class segmentation over inpt image input.v and store the class "
	"probability images in cls.v.", 
	
	"-i input.v -a 5 -o cls.v"
}; 


class CSegment2d  {
public: 
	typedef C2DImageVector result_type; 

	CSegment2d(bool bg_correct, int ncc,
		   const vector<float>& icc, float k); 
	
	template <class T> 
	CSegment2d::result_type operator()(const T2DImage<T>& image); 

private: 
	void process(const C2DFImage& image, 
		     vector<float>& class_centers, 
		     C2DFImageVec& prob)const; 

	float update_class_centers(C2DFImage& image, C2DFImageVec& prob, vector<float>& class_centers)const; 
	
	void evaluate_probabilities(C2DFImageVec& prob) const; 

	unsigned int _M_nClasses;
	bool         _M_bg_correct; 
	vector<float> _M_class_centers; 
	const float _M_k; 
	mutable float _M_minh; 
	mutable float _M_maxh; 
	
	
};



CSegment2d::CSegment2d(bool bg_correct, int ncc,
				 const vector<float>& icc, float k):
	_M_nClasses(ncc), 
	_M_bg_correct(bg_correct), 
	_M_class_centers(icc),
	_M_k(k)
{
}	

struct CLogTransform {
	CLogTransform(float minh, float maxh):
		_M_minh(minh),  
		_M_div(maxh - minh + 1.0)
	{
	}

	float operator() (float x) const 
	{
		return (x - _M_minh) / _M_div;
	}
private: 
	float _M_minh;
	float _M_div; 

};

struct CExpTransform {
	CExpTransform(float minh, float maxh):
		_M_minh(minh), 
		_M_div(maxh - minh + 1.0)
	{
	}
	float operator() (float x) const 
	{
		return x *  _M_div + _M_minh;
	}
private: 
	float _M_minh;
	float _M_div; 

};

template <class T>
struct CThresholder {
	CThresholder(T thresh):_M_thresh(thresh){
	}
	
	T operator ()(T x) {
		return x > _M_thresh ? x : 0; 
	}
private: 
	T _M_thresh; 
};

template <typename CI>
void get_min_max(CI begin, CI end, typename CI::value_type& minh, typename CI::value_type& maxh)
{
	auto minmax = minmax_element(begin, end); 
	minh = *minmax.first; 
	maxh = *minmax.second; 
}


template <class T> 
CSegment2d::result_type CSegment2d::operator()(const T2DImage<T>& image)
{
	// first evaluate the histogram borders and get the initial class centers
	T minh=0; 
	T maxh=numeric_limits<T>::max(); 
	

	get_min_max(image.begin(), image.end(), minh, maxh);  
	
	_M_minh = minh; 
	_M_maxh = maxh;
	
	cvdebug() << "Entering CSegment2d::operator()\n"; 
	
	if (_M_class_centers.empty()) {
		cvdebug() << "Estimate initial class centers from histogram\n"; 
		// estimate some classes
		_M_class_centers.resize(_M_nClasses); 
		_M_class_centers[0] = _M_minh; 
		
		float step = (_M_maxh - _M_minh) / 256.0; 
		THistogram<THistogramFeeder<T> > histogram(THistogramFeeder<T>(_M_minh, _M_maxh, 256)); 
		histogram.push_range(image.begin(), image.end());

		float thresh = (image.size() - histogram[0]) / _M_nClasses;
		float hit = 0.0; 
		size_t i = 1; 
		float val = _M_minh + step; 
		
		auto hi = histogram.begin(); 
		auto he = histogram.end(); 
		++hi; 
		
		while (i < _M_nClasses && hi != he) {
			hit += *hi; 
			if (hit > thresh) {
				_M_class_centers[i++] = val; 
				hit -= thresh; 
			}
			++hi; 
			val += step; 
		}
		
		// print out the new class centers
	}
	cvmsg() << " initial classes ["; 
	for (size_t j = 0; j < _M_nClasses; ++j)
		cvmsg() << _M_class_centers[j] << " "; 
	cvmsg() << "]" << endl;
	
	C2DFImage log_image(image.get_size()); 	                                 // 4 Bpp
	vector<float> log_class_centers(_M_nClasses); 

	CLogTransform logt(_M_minh, _M_maxh); 
	
	// fill the logarithmic image with values
	transform(image.begin(), image.end(), log_image.begin(), logt);
	transform(_M_class_centers.begin(), _M_class_centers.end(), log_class_centers.begin(), logt);
	
	// initialise teporaries
	C2DFImageVec prob; 

	for (size_t i = 0; i < _M_nClasses; ++i) {
		prob.push_back(C2DFImage(image.get_size()));                    // 4 * _M_nClasses Bpp
	}

	
	process(log_image, log_class_centers, prob); 
	
	
	CExpTransform exptrans(minh, maxh); 
	transform(log_class_centers.begin(), log_class_centers.end(), 
		       _M_class_centers.begin(), exptrans); 

	C2DImageVector result;  

	for (size_t j = 0; j < _M_nClasses; ++j) {
		C2DFImage *r = new C2DFImage(image.get_size(), image); 
		transform(prob[j].begin(), prob[j].end(), r->begin(), 
			  [](float x){return sqrt(x);}); 
		result.push_back(P2DImage(r)); 
	}
	return result; 
}
		

void CSegment2d::evaluate_probabilities(C2DFImageVec& prob)const
{
	vector<C2DFImage::iterator> pi(_M_nClasses); 
	
	C2DFImage::iterator stopper =  prob[0].end(); 
	
	for (size_t i = 0; i < _M_nClasses; ++i)
		pi[i] = prob[i].begin(); 
		
	while (pi[0] != stopper) {
		float sum = 0.0; 
		for (size_t i = 0; i < _M_nClasses; ++i)
			sum += *pi[i];
		
		if (sum != 0.0)  
			for (size_t i = 0; i < _M_nClasses; ++i) {
				*pi[i] /= sum;
				*pi[i] *= *pi[i]; 
				++pi[i];
			}
		else {// must be a bright value, otherwise, class[0] would catch it
			for (size_t i = 0; i < _M_nClasses - 1; ++i)
				*pi[i]++ = 0.0;
			*pi[_M_nClasses - 1] = 1.0; 
		}
	}
}

float CSegment2d::update_class_centers(C2DFImage& image, C2DFImageVec& prob, vector<float>& class_centers) const 
{
	float residuum = 0.0; 
	
	for (size_t i = 0; i < _M_nClasses; ++i) {
		float cc = class_centers[i]; 
		float loc_res = 100; 

		// try a fix-point iteration for the new class center
		
		while (loc_res > 0.0001) {
			float cc_old = cc; 
			
			C2DFImage::const_iterator ihelp = image.begin(); 
			C2DFImage::const_iterator iprob = prob[i].begin(); 
			C2DFImage::const_iterator eprob = prob[i].end();
			
			double sum_prob = 0.0; 
			double sum_weight = 0.0; 
			
			while (iprob != eprob) {
				if (*iprob > 0.0f) {
					const double val = *ihelp - cc;
					const double val2 = val * val; 
					const double val2mexpval2 = val2 * exp(val2)  * *iprob;

					sum_prob += val2mexpval2; 
					sum_weight += val2mexpval2 * *ihelp; 
				}
				
				++ihelp; 
				++iprob; 
			}
			// how do i deal with the (very unprobable case sum_prob==0.0?)
			if (sum_prob  != 0.0) {
				cc += 0.1 * (sum_weight / sum_prob - cc); 
			} else {
				cvwarn() << "class[" << i << "] has no probable members, keeping old value:" << sum_prob << ":" <<sum_weight <<endl; 
			}
			loc_res = cc > cc_old ? cc - cc_old :  cc_old - cc; 
			cvdebug() << "res = " << loc_res << " : cc = "<< cc <<"\n"; 
			
		}// end while (loc_res > 0.01)
		cverb << "\n"; 

		float delta = cc - class_centers[i]; 
		residuum += delta * delta; 
		class_centers[i] =  cc; 
		
	}// end update class centers
	return residuum; 
}



void
CSegment2d::process(const C2DFImage& image, 
			 vector<float>& class_centers, 
			 C2DFImageVec& prob)const
{
	
	int n_iterations = 0; 

	CExpTransform exptrans(_M_minh, _M_maxh); 
	
	C2DFImage tmp(image.get_size());                                        // 4Bpp

	auto_ptr<C2DFImage> bg_image(NULL);                                              

	
	if (_M_bg_correct) {
		bg_image.reset(new C2DFImage(image.get_size())); 
		fill(bg_image->begin(), bg_image->end(), 1.0); 
	}

	
	float residuum = 100; 
	// these conditions have to be exported 
	while (residuum > 0.000001 && n_iterations < 100) {
		++n_iterations; 

		// distance evaluation 
		vector<float>::const_iterator icls = class_centers.begin(); 
		for (size_t i = 0; i < _M_nClasses; ++i, ++icls) {
			
			// evaluate D for class i 
			C2DFImage::iterator di = prob[i].begin(); 
			C2DFImage::iterator de = prob[i].end(); 
			C2DFImage::const_iterator ii = image.begin(); 
			
			if (_M_bg_correct) {
				C2DFImage::const_iterator ib = bg_image->begin(); 
				while (di != de) {
					float val = *ii++ * *ib++ - *icls; 
					*di++ = expf(-val * val * _M_k); 
				}
			}else
				while (di != de) {
					float val = *ii++ - *icls; 
					*di++ = expf(-val * val * _M_k); 
				}
			

		}// end D evaluation 

		// evaluate probabilities
		evaluate_probabilities(prob);

		if (_M_bg_correct)
			transform(image.begin(), image.end(), bg_image->begin(), tmp.begin(), minus<float>()); 
		else
			copy(image.begin(), image.end(), tmp.begin()); 
		
		residuum = update_class_centers(tmp, prob, class_centers);
		
		// print out the new class centers
		cvmsg() << n_iterations << ": classes ["; 
		for (size_t j = 0; j < _M_nClasses; ++j)
			cvmsg() << exptrans(class_centers[j]) << " "; 
		cvmsg() << "]" << endl; 
		
		// update field correction 
		// this certainly needs a methodological review
		if (_M_bg_correct){
			C2DFImage::const_iterator ii = image.begin(); 
			C2DFImage::iterator ib = bg_image->begin(); 
			C2DFImage::iterator eb = bg_image->begin(); 
			
			vector<C2DFImage::const_iterator> pi(_M_nClasses); 
			
			for (size_t j = 0; j < _M_nClasses; ++j)
				pi[j] = prob[j].begin();

			
			while (ib != eb) {
				float sum_prob = 0.0; 
				float sum_weight = 0.0; 
				
				for (size_t j = 0; j < _M_nClasses; ++j) {
					sum_weight += *pi[j] * class_centers[j]; 
					sum_prob   += *pi[j]; 
					++pi[j];
				}

				assert(sum_prob != 0.0); 
				
				*ib =*ii * (sum_prob / sum_weight); 
				++ib; 
				++ii; 
			}
			
		}// end update bg field

	} // end main while loop
}

int do_main(int argc, char *argv[])
{

	string in_filename; 
	string cls_filename; 
	string out_filename; 
	int n_classes = 3; 
	bool bg_correct = false; 
	string neighborhood_filter; 
	vector<float> initial_class_centres; 
	float spread = 64; 

	
	CCmdOptionList opts(g_description);
	opts.add(make_opt( in_filename, "in-file", 'i', "image to be segmented", CCmdOption::required )); 
	opts.add(make_opt( cls_filename, "out-file", 'o', "class probability images", CCmdOption::required )); 
	opts.add(make_opt( n_classes, "no-of-classes", 'a', "number of classes"));
	opts.add(make_opt( bg_correct, "bias-correct", 'b', "apply bias field correction"));
	opts.add(make_opt( initial_class_centres, "class-centres", 'c', "initial class centers"));
	opts.add(make_opt( spread, "spread", 's', "spread parameter describing the strength of mattar distinction")); 
	
	if (opts.parse(argc, argv) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 
	
	
	if (initial_class_centres.empty()) {
		cvdebug() << "no class centers given at command line\n"; 
		if (n_classes < 2)
			throw invalid_argument("at least two classes need to be requested"); 
	}else {
		cvdebug() << "Initially classes: " << initial_class_centres.size() << "\n"; 
	}
	
	// initialize the functor
	CSegment2d Segment(bg_correct, n_classes, 
			   initial_class_centres, spread);
	
	
	auto in_image = load_image2d(in_filename); 
	
	auto seg_list = mia::accumulate (Segment, *in_image);
	C2DImageIOPluginHandler::Instance::Data out_image_list(seg_list);
	
	
		//CHistory::instance().append(argv[0], revision, opts);
	
	if ( !C2DImageIOPluginHandler::instance().save(cls_filename, seg_list) ){
		string not_save = ("unable to save result to ") + cls_filename;
		throw runtime_error(not_save);
	}
	
	return EXIT_SUCCESS; 
	
}

#include <mia/internal/main.hh>
MIA_MAIN(do_main); 


