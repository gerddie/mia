/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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

#include <stdexcept>
#include <cmath>
#include <cassert>
#include <numeric>
#include <fstream>

#ifdef HAVE_BLAS
extern "C" {
#include <cblas.h>
}
#endif


#include <mia/core/msgstream.hh>
#include <mia/core/cmdlineparser.hh>
#include <mia/core/probmap.hh>

NS_MIA_USE; 
using namespace std; 


const SProgramDescription g_description = {
	{pdi_group,"Miscellaneous programs"}, 
	{pdi_short,"C-means classification of a histogram."}, 
	{pdi_description, "This Program reads a histogram from stdin and evaluates a "
	 "cmeans classification of the intensity values into the given number of classes."}
}; 

typedef pair<int, int> CClassRange; 
typedef map<double, CClassRange> CClassMap; 

struct CForwardTransform {
	CForwardTransform(float minh, float maxh):
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

struct CBackTransform {
	CBackTransform(float minh, float maxh):
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

class CCMeans {
public: 
	CCMeans(double k, double epsilon, bool start_even); 
	CProbabilityVector operator()(CDoubleVector const& histogram, 
				      CDoubleVector& class_centers, bool initialise, bool auto_k) const; 
private:
	void initialise_cc(CDoubleVector& class_centers, CDoubleVector const& histogram) const; 
	void evaluate_probabilities(CDoubleVector const& classes, size_t nvalues , 
				    CProbabilityVector& pv, double k)const;
	double update_class_centers(CDoubleVector& center, CDoubleVector const& histo, CDoubleVector const& shisto, 
				    CProbabilityVector const& pv)const;

	double adjust_k(CDoubleVector const& class_centers, CDoubleVector const& histogram, CProbabilityVector const & pv)const; 
	
	double _M_k; 
	double _M_epsilon; 
	bool _M_even_start; 
};
	

CCMeans::CCMeans(double k, double epsilon, bool start_even):
	_M_k(k), 
	_M_epsilon(epsilon), 
	_M_even_start(start_even)
{
}
	

void CCMeans::evaluate_probabilities(CDoubleVector const& classes, size_t nvalues , 
				     CProbabilityVector& pv, double k)const
{
	for (size_t i = 0; i < nvalues; ++i) {
		double const x = double(i)/ nvalues; 
		double sum = 0.0; 
		for (size_t j = 0; j < classes.size(); ++j) {
			double  val =  x - classes[j]; 
			val = exp(- (val * val) / k);
			pv[j][i] = val; 
			sum += val;
		}
		
		assert(sum != 0.0); 

		
		for (size_t j = 0; j < classes.size(); ++j)
			pv[j][i] /= sum; 

	}
}

double CCMeans::update_class_centers(CDoubleVector& class_center, CDoubleVector const& histo, 
				     CDoubleVector const& shisto,CProbabilityVector const& pv)const
{
	double residuum = 0.0; 

	for (size_t i = 0; i < class_center.size(); ++i) {
		float cc = class_center[i]; 
		
#ifdef HAVE_BLAS
		double sum_prob   = cblas_ddot(histo.size(), &histo[0], 1, &pv[i][0], 1);
		double sum_weight = cblas_ddot(histo.size(), &shisto[0], 1, &pv[i][0], 1); 
#else		
		CDoubleVector::const_iterator ihelp = histo.begin(); 
		CDoubleVector::const_iterator shelp = shisto.begin(); 
		CDoubleVector::const_iterator iprob = pv[i].begin(); 
		CDoubleVector::const_iterator eprob = pv[i].end();
		
		double sum_prob = 0.0; 
		double sum_weight = 0.0; 
	
		for (size_t  ix = 0; ix < histo.size(); ++ix, ++iprob, ++ihelp, ++shelp) {
			if (*iprob > 0.0f) {
				sum_prob += *iprob * *ihelp; 
				sum_weight += *iprob * *shelp; 
			}
		}
		
#endif		
		if (sum_prob  != 0.0) // move slowly in the direction of new center
			cc += 0.5 * (sum_weight / sum_prob - cc); 
		else {
			cvwarn() << "class[" << i << "] has no probable members, keeping old value:" << 
				sum_prob << ":" <<sum_weight <<endl; 
			
		}
		double delta = cc - class_center[i]; 
		residuum += delta * delta; 
		class_center[i] =  cc; 
		
	}// end update class centers
	return sqrt(residuum); 

}

double CCMeans::adjust_k(CDoubleVector const& class_centers, CDoubleVector const& histogram, CProbabilityVector const & pv)const
{
	CDoubleVector cc(class_centers.size()); 
	size_t hsize = histogram.size(); 
	transform(class_centers.begin(), class_centers.end(), cc.begin(), 
		  [hsize](double x){return x * hsize;}); 
	
	// evaluate best mapping of classes based on maximum probability
	vector<int> classmap(histogram.size(), 0); 
	vector<double> classprob(pv[0]);
	for (size_t i = 1; i < cc.size(); ++i) {
		CDoubleVector::const_iterator iprob = pv[i].begin(); 
		CDoubleVector::const_iterator eprob = pv[i].end();
		vector<int>::iterator cmi = classmap.begin(); 
		vector<double>::iterator cpi = classprob.begin(); 

		while (iprob != eprob) {
			if (*cpi < *iprob) {
				*cpi = *iprob; 
				*cmi = i; 
			}
			++iprob; 
			++cpi; 
			++cmi; 
		}
	}
	
	double sum = 0.0; 
	double sum2 = 0.0; 
	double n = 0.0; 
	
	for (size_t i = 1; i < histogram.size(); ++i) {
		double delta = i - cc[classmap[i]]; 
		sum += delta * histogram[i]; 
		sum2 += delta * delta * histogram[i]; 
		n += histogram[i];
	}
	
	const double avg = sum / n; 
	float hist_size = histogram.size() - 1; 
	double new_k = 2 * ((sum2 - avg * sum) / (n-1)) /  (hist_size * hist_size); 
	
	return new_k > 0.001 ? new_k : 0.001; 
}
	
CProbabilityVector CCMeans::operator()(CDoubleVector const& histogram, CDoubleVector& class_centers, 
				       bool initialise, bool auto_k) const
{
	if (initialise)
		initialise_cc(class_centers, histogram); 
	
	CProbabilityVector pv(class_centers.size(), histogram.size()); 

	CDoubleVector scale_histo(histogram); 
	const double dx = 1.0 / histogram.size(); 
	double x = 0.0; 
	
	for (size_t i = 0; i < histogram.size(); ++i, x+=dx)
		scale_histo[i] *= x; 
	
	

	for (size_t i = 0; i < class_centers.size(); ++i)
		if (class_centers[i] > 1.0) 
			class_centers[i] /= histogram.size(); 
	
	const size_t size = histogram.size(); 
	double k = _M_k; 
	bool cont = true; 
	
	while (cont) {
		evaluate_probabilities(class_centers, size, pv, k);
		
		cvmsg() << "Class centers: ";  
		for (CDoubleVector::const_iterator i = class_centers.begin(), e = class_centers.end(); 
		     i != e; ++i) {
			cverb << *i << ", "; 
		}
		cont = update_class_centers(class_centers, histogram, scale_histo, pv) > _M_epsilon;
		if (auto_k) {
			k = adjust_k(class_centers, histogram, pv); 
			cvmsg() << "k = " << k; 
		}
		cvmsg() << '\n';
	};
	
	for (CDoubleVector::iterator i = class_centers.begin(), e = class_centers.end(); i != e; ++i)
		*i *= size; 
	
	return pv; 
}

void CCMeans::initialise_cc(CDoubleVector& class_centers, CDoubleVector const& histogram)const
{
	double const size = histogram.size(); 
	double const step = size / double(class_centers.size() + 1); 
	
	if (_M_even_start) {
		
		for (size_t i = 0; i < class_centers.size(); ++i)
			class_centers[i] = i * step; 
	}else{
		class_centers[0] = 0.0; 
		
		CDoubleVector::const_iterator hi = histogram.begin(); 
		CDoubleVector::const_iterator const he = histogram.end(); 
		++hi; 

		double const thresh = accumulate(hi, he, 0.0) / size;
		float hit = 0.0; 
		size_t i = 1; 
		float val = step; 
		
		while (i < class_centers.size() && hi != he) {
			hit += *hi; 
			if (hit > thresh) {
				class_centers[i++] = val;
				hit -= thresh; 
			}
			++hi; 
			val += step; 
		}
	}
}

void test(double k, bool auto_k)
{
	const size_t Nh = 1024; 
	const size_t Nc = 3; 
	
	cvdebug() << "k = " << k << "\n"; 
	
	CDoubleVector class_centers(Nc);
	float cstep = 1.0 / float(Nc); 

	cvdebug() << "class centers: "; 
	for (size_t i =0; i < Nc; ++i) {
		class_centers[i] = i * cstep + cstep / 4.0;
		cverb << class_centers[i] << ", "; 
	}
	cverb << "\n"; 
	
	CDoubleVector histogram(Nh);
	
	CProbabilityVector pv(class_centers.size(), histogram.size());
	
	for (size_t i = 0; i < Nh; ++i) {
		float h = 0; 
		float x = float(i)/Nh; 
		for (size_t c = 0; c < Nc; ++c) {
			float delta = class_centers[c] - x; 
			delta *= delta; 
			delta /= k;
			float val = exp(-delta);
			pv[c][i] = val; 
			h += val; 
			histogram[i] += 1024 * val; 
		}
		cout << i << ":" << histogram[i] << " "; 
		for (size_t c = 0; c < Nc; ++c) {
			pv[c][i] /= h; 
			cout << pv[c][i] << " "; 
		}
		cout << "\n"; 
	}

	CDoubleVector eval_class_centers(Nc);
	for (size_t i = 0; i < Nc; ++i)
		eval_class_centers[i] = i * cstep;
	
	
	CCMeans cmeans(k, false, 0.00001);
	CProbabilityVector eval_pv = cmeans(histogram, eval_class_centers, false, auto_k);
	for (size_t i = 0; i < Nc; ++i) {
		if (fabs(eval_class_centers[i] - Nh * class_centers[i]) > 0.5) {
			cverr() << i << ": " << eval_class_centers[i] << " vs." <<  Nh * class_centers[i] << "\n"; 
		}
	}
}


int do_main(int argc, char *argv[])
{
	int nclasses = 3; 
	int max_iter = 100; 
	bool even_start = false; 
	bool auto_k = false; 
	bool cut_histo = false; 
	double epsilon = 0.00001; 
	double k = 1.0; 
	bool self_test = false; 
	
	string in_filename; 
	string out_filename; 
	CDoubleVector class_centers; 

	CCmdOptionList options(g_description);

	options.add(make_opt( in_filename, "in-file", 'i', "input file name containing the histogram", CCmdOption::required)); 
	options.add(make_opt( out_filename, "out-file", 'o', "output file name to store probabilities", CCmdOption::required)); 
	
	options.add(make_opt( nclasses, "nclasses", 'n', "number of classes to partition into")); 
	options.add(make_opt( max_iter, "max-iter", 'm', "maximum number of iterations")); 
	options.add(make_opt( even_start, "even-start", 'e', "start with centers evenly distributed over the histogram")); 
	options.add(make_opt( class_centers, "class-centers", 'c', "initial class centers")); 
	options.add(make_opt( auto_k, "auto", 'a', "atomatic adaption of variance (experimental)")); 
	options.add(make_opt( cut_histo,"cut-histo", 't', "cut empty histogram at the end "));
	options.add(make_opt( k, "variance", 'k', "variance parameter")); 
	options.add(make_opt( self_test, "self-test", 0, "run self test"));
	

	if (options.parse(argc, argv) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 

		
	if (self_test) {
		test(0.002, false); 
		return 0; 
	}
		
	ifstream ifs( in_filename, ifstream::in ); 
			
	vector<double> histo; 
	// read input data
	size_t sig_size_c = 0; 
	size_t sig_size = 0; 
	while (ifs.good()) {
		float val, cnt;
		ifs >> val >> cnt; 
		histo.push_back(cnt); 
		++sig_size_c; 
		if (val > 0)
			sig_size = sig_size_c; 
	}
	ifs.close(); 
		
	if (sig_size < sig_size_c && cut_histo)
		histo.resize(sig_size); 

	cvmsg() << "got a histogram with " << histo.size() << " values\n"; 
	bool initialise = false; 
		
	if (class_centers.empty()) {
		class_centers.resize(nclasses); 
		initialise = true; 
	}
		
	CCMeans cmeans(k, even_start, epsilon); 
			
	CProbabilityVector pv = cmeans(histo, class_centers, initialise, auto_k); 
		
	if (!pv.save(out_filename)) {
		cverr() << "runtime: unable to save probability map\n"; 
		return EXIT_FAILURE; 
	}
	return EXIT_SUCCESS;
}

#include <mia/internal/main.hh>
MIA_MAIN(do_main)
