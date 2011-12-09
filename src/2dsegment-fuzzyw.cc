/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004 - 2011
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


#include <mia/core/histogram.hh>
#include <mia/core/cmdlineparser.hh>
#include <mia/core/factorycmdlineoption.hh>
#include <mia/2d.hh>

#include <memory>
#include <vector>

using namespace mia;
using namespace std; 

const SProgramDescrption g_description = {
	"2D image processing", 
	
	"This program is a  implementation of a fuzzy c-means segmentation algorithm",
	
	"Run a 5-class segmentation over inpt image input.v and store the class "
	"probability images in cls.v.", 
	
	"-i input.v -a 5 -o cls.v"
}; 

class CSegment2DFuzzy: public TFilter<P2DImage> {
public: 
	struct Params {
		Params();
		vector<double> class_centres; 
		P2DFilter neighbourhood_filter; 
		float p; 
		float alpha; 
		float epsilon; 
	}; 

	CSegment2DFuzzy(const Params& p); 

	template <typename T> 
	P2DImage operator() (const T2DImage<T>& image); 
private:
	Params m_params; 
	vector<P2DImage> m_probs;
	P2DImage m_gain; 
}; 

CSegment2DFuzzy::Params::Params():
	neighbourhood_filter(produce_2dimage_filter("shmean:shape=8n")),
	p(2), 
	alpha(0.7), 
	epsilon(0.01)
{
}

CSegment2DFuzzy::CSegment2DFuzzy(const Params& p):
	m_params(p)
{
}

class MyFCM {
public: 
	template <typename T> 
	MyFCM(const T2DImage<T>& image, const CSegment2DFuzzy::Params& params);

	void run();

private: 
	friend class CSegment2DFuzzy; 
	void filter_neighborhood(C2DFImage& deltasq); 
	void estimate_prob(const vector<C2DFImage>& deltasq); 
	P2DImage get_gain()const; 
	vector<P2DImage> get_probs() const; 

	C2DBounds m_size; 
	C2DFImage m_image;
	C2DFImage m_gain; 
	P2DFilter m_neighbourhood; 
	vector<double> m_class_centres; 
	vector<C2DFImage> m_probs; 
	float m_p; 
	float m_alpha; 
	float m_epsilon; 
}; 

/*
  Initialization of needed fields and transformation to the logarithmic space 
*/
template <typename T> 
MyFCM::MyFCM(const T2DImage<T>& image, const CSegment2DFuzzy::Params& params):
	m_size(image.get_size()), 
	m_image(m_size), 
	m_gain(m_size), 
	m_neighbourhood(params.neighbourhood_filter), 
	m_class_centres(params.class_centres), 
	m_probs(m_class_centres.size(), C2DFImage(m_size)), 
	m_p(params.p), 
	m_alpha(params.alpha),
	m_epsilon(params.epsilon)
{
	fill(m_gain.begin(), m_gain.end(), 1.0); 
	copy(image.begin(), image.end(), m_image.begin()); 
}

P2DImage MyFCM::get_gain()const
{
	return P2DImage(m_gain.clone()); 
	
}

vector<P2DImage> MyFCM::get_probs() const
{
	vector<P2DImage> result(m_probs.size()); 
	transform(m_probs.begin(), m_probs.end(), result.begin(), 
		  [](const C2DFImage& image){ return P2DImage(image.clone());}); 
	return result; 
}

void MyFCM::filter_neighborhood(C2DFImage& deltasq) 
{
	auto h = m_neighbourhood->filter(deltasq); 
	const C2DFImage& h2 = dynamic_cast<const C2DFImage&>(*h); 
	
	transform(h2.begin(), h2.end(), deltasq.begin(), deltasq.begin(),
		  [&m_alpha](float D, float m){return 1.0 / (D + m_alpha * m);});
}

void MyFCM::estimate_prob(const vector<C2DFImage>& deltasq)
{
	const int nclasses = m_probs.size(); 

	vector<C2DFImage::const_iterator> idsq(nclasses); 
	vector<C2DFImage::iterator> iprob(nclasses); 
	
	transform(deltasq.begin(), deltasq.end(), idsq.begin(), [](const C2DFImage& img){return img.begin();}); 
	transform(m_probs.begin(), m_probs.end(), iprob.begin(), [](C2DFImage& img){return img.begin();}); 

	while (idsq[0] != deltasq[0].end()) {
		float sum = 0.0; 
		for (int i = 0; i < nclasses; ++i)
			sum += *idsq[i];
		
		for (int i = 0; i < nclasses; ++i) {
			*iprob[i] =  *idsq[i] / sum; 
			++iprob[i]; 
			++idsq[i]; 
		}
	}
}

void MyFCM::run()
{
	// helper for differences
	vector<C2DFImage> deltasq(m_probs.size(), C2DFImage(m_size)); 
	vector<C2DFImage> corrected(m_probs.size(), C2DFImage(m_size)); 
	const int nclasses = m_class_centres.size(); 

	auto old_class_centres = m_class_centres; 
	float delta_cls = 0.0; 
	do  {
		
		// evaluate helper 
		for (int i = 0; i < nclasses; ++i) {
			float vi = m_class_centres[i]; 
			for (auto ids = deltasq[i].begin(), ig = m_gain.begin(), ii = m_image.begin(), 
				     id = corrected[i].begin(); 
			     ids != deltasq[i].end(); ++ids, ++ig, ++ii, ++id) {
				*id = *ii / *ig; 
				float value = *id - vi; 
				*ids = exp(value * value);
			}
			filter_neighborhood(deltasq[i]); 
			filter_neighborhood(corrected[i]); 
			// if p = 2, the exponent becomes 1.0 and we can save this step 
			if (m_p != 2.0) {
				const float p_help = 1 / (1.0 - m_p); 
				transform(deltasq[i].begin(), deltasq[i].end(), deltasq[i].begin(), 
					  [&p_help](float x) { return pow(x, p_help); }); 
			}
		}
		
		estimate_prob(deltasq);
		
		// now update the cluster centers 
		if (m_p != 2.0) {
			for (int i = 0; i < nclasses; ++i) {
				transform(m_probs[i].begin(),  m_probs[i].end(), deltasq[i].begin(), 
					  [&m_p](float x) { return pow(x, m_p);}); 
			}
		} else {
			for (int i = 0; i < nclasses; ++i) {
				copy(m_probs[i].begin(),  m_probs[i].end(), deltasq[i].begin());
			}
		}
		
		for (int i = 0; i < nclasses; ++i) {
			float sum = 0.0f; 
			float n = 0.0f; 
			auto ic = corrected[i].begin(); 
			for (auto up = deltasq[i].begin(); up != deltasq[i].end(); ++ic, ++up) {
				sum += *up * *ic; 
				n += *up; 
			}
			m_class_centres[i] = sum / (( 1.0 + m_alpha) * n); 
		}
		
		// now update the gain field 
		
		vector<C2DFImage::const_iterator> iprobs(nclasses); 
		transform(deltasq.begin(), deltasq.end(), iprobs.begin(), [](const C2DFImage& img){return img.begin();});

		auto ii = m_image.begin(); 
		for (auto ig = m_gain.begin(); ig != m_gain.end(); ++ig, ++ii) {
			float sum = 0.0; 
			float n = 0.0; 
			for (int i = 0; i < nclasses; ++i) {
				n += m_class_centres[i] * *iprobs[i]; 
				sum += *iprobs[i]; 
				++iprobs[i]; 
			}
			*ig = sum / n * *ii; 
		}
		delta_cls = inner_product(old_class_centres.begin(), old_class_centres.end(), 
					  m_class_centres.begin(), 0.0f, 
					  [](float x, float y) {return x + y;}, 
					  [](float x, float y) {float d = x-y; return d * d; });
		cvmsg() << "Class centers = " << m_class_centres << ", delta = " << delta_cls << "\n"; 

	} while (delta_cls > m_epsilon); 
}


template <typename T> 
P2DImage CSegment2DFuzzy::operator() (const T2DImage<T>& image)
{
	MyFCM myfmc(image,  m_params); 
	myfmc.run();
	m_probs = myfmc.get_probs();
	m_gain = myfmc.get_gain();
	
	const C2DFImage& work_image = myfmc.m_image; 
	T2DImage<T> *result = new T2DImage<T>(image.get_size(), image); 
	transform(work_image.begin(), work_image.end(), result->begin(), 
		  [](double x){ return mia_round_clamped<T>(x);}); 
	return P2DImage(result); 
}


int do_main(int argc, char *argv[])
{

	string in_filename; 
	string cls_filename; 
	string out_filename; 
	string gain_log_field; 

	CSegment2DFuzzy::Params params; 
	unsigned int n_classes = 3; 

	
	CCmdOptionList opts(g_description);
	opts.set_group("File I/O"); 
	opts.add(make_opt( in_filename, "in-file", 'i', "image to be segmented", CCmdOption::required )); 
	opts.add(make_opt( cls_filename, "cls-file", 'c', "class probability images", CCmdOption::required )); 
	opts.add(make_opt( out_filename, "out-file", 'o', "B-field corrected image")); 
	opts.add(make_opt( gain_log_field, "gain-log-file", 'g', "Logarithmic gain field")); 
	
	opts.set_group("Segmentation");
	opts.add(make_opt( n_classes, "no-of-classes", 'n', "number of classes to segment"));
	opts.add(make_opt( params.class_centres, "class-centres", 'C', "initial class centers"));
	opts.add(make_opt( params.neighbourhood_filter, "neighborhood", 'N', "neighborhood filter for B-field correction"));
	opts.add(make_opt( params.alpha, "alpha", 'a', "weight of neighborhood filter for B-field correction"));
	opts.add(make_opt( params.p, "fuzziness", 'p', 
			   " parameter describing the fuzzyness of mattar distinction")); 
	opts.add(make_opt( params.epsilon, "epsilon", 'e', 
			   "Stopping criterion for class center estimation.")); 

	
	if (opts.parse(argc, argv) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 


	auto in_image = load_image2d(in_filename);	
	
	if (params.class_centres.empty()) {
		cvdebug() << "no class centers given at command line, estimate using k-means\n"; 

		ostringstream kmeans_filter; 
		kmeans_filter << "kmeans:c=" << n_classes; 
		
		P2DImage kmeans_image = run_filter(*in_image, kmeans_filter.str().c_str()); 
		params.class_centres = 
			kmeans_image->get_attribute_as<vector<double> >(ATTR_IMAGE_KMEANS_CLASSES); 
	}else{
		if (params.class_centres.size() != n_classes) {
			cvwarn() << "Specified initial class centres but also number of classes. "
				 << "The number of classes resulting from the given centres takes precedence\n"; 
		}
	}


	
	
	// initialize the functor
	CSegment2DFuzzy segment(params);
	auto b0_corrected = mia::accumulate (segment, *in_image);
	
	if (!out_filename.empty()) {
		if (!save_image(out_filename, b0_corrected)) {
			cvwarn() << "Unable to save B0 corrected image to '" << out_filename << "'\n"; 
		}
	}

	return EXIT_SUCCESS; 
}

#include <mia/internal/main.hh>
MIA_MAIN(do_main); 


