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


#include <mia/core/histogram.hh>
#include <mia/core/cmdlineparser.hh>
#include <mia/2d.hh>

#include <memory>
#include <vector>

using namespace mia;
using namespace std; 

const SProgramDescription g_description = {
        {pdi_group, "Analysis, filtering, combining, and segmentation of 2D images"}, 
	{pdi_short, "Run a fuzzy c-means segmentation of a 2D image."}, 	
	{pdi_description, "This program is a  implementation of a fuzzy c-means segmentation algorithm"},
	{pdi_example_descr, "Run a 5-class segmentation over inpt image input.v and store the class "
	 "probability images in cls.v."}, 
	{pdi_example_code, "-i input.v -a 5 -o cls.v"}
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

	const C2DImageVector& get_probs() const; 
	P2DImage get_gain() const; 
private:
	Params m_params; 
	C2DImageVector m_probs;
	P2DImage m_gain; 
}; 

CSegment2DFuzzy::Params::Params():
	p(2), 
	alpha(0.7), 
	epsilon(0.01)
{
}

CSegment2DFuzzy::CSegment2DFuzzy(const Params& p):
	m_params(p)
{
}

const C2DImageVector& CSegment2DFuzzy::get_probs() const
{
	return m_probs; 
}

P2DImage CSegment2DFuzzy::get_gain() const
{
	return m_gain; 
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
	C2DImageVector get_probs() const; 


	template <typename T> 
	P2DImage get_corrected_image() const; 
	
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
	m_class_centres(params.class_centres.size()), 
	m_probs(m_class_centres.size(), C2DFImage(m_size)), 
	m_p(params.p), 
	m_alpha(params.alpha),
	m_epsilon(params.epsilon)
{
	fill(m_gain.begin(), m_gain.end(), 0.0); 
	transform(image.begin(), image.end(), m_image.begin(), [](float x){return log(x + 1);}); 
	transform(params.class_centres.begin(), params.class_centres.end(), 
		  m_class_centres.begin(), [](float x){return log(x + 1);}); 
}

P2DImage MyFCM::get_gain()const
{
	return P2DImage(m_gain.clone()); 
	
}

C2DImageVector MyFCM::get_probs() const
{
	C2DImageVector result; 
	
	transform(m_probs.begin(), m_probs.end(), back_inserter(result), 
		  [](const C2DFImage& image){ return P2DImage(image.clone());}); 
	return result; 
}

template <typename T> 
P2DImage MyFCM::get_corrected_image()const
{
	T2DImage<T> *result = new T2DImage<T>(m_image.get_size(), m_image); 
	
	transform(m_image.begin(), m_image.end(), m_gain.begin(), result->begin(), 
		  [](float y, float b){return mia_round_clamped<T>(exp(y-b)); }); 
	
	return P2DImage(result); 
}

void MyFCM::filter_neighborhood(C2DFImage& deltasq) 
{
	auto h = m_neighbourhood->filter(deltasq);
	const C2DFImage& h2 = dynamic_cast<const C2DFImage&>(*h); 
	
	transform(h2.begin(), h2.end(), deltasq.begin(), deltasq.begin(),
		  [this](float m, float D){return (D + m_alpha * m);});
}

void MyFCM::estimate_prob(const vector<C2DFImage>& deltasq)
{
	const float SMALL = 1e-8; 
	const int nclasses = m_probs.size(); 

	vector<C2DFImage::const_iterator> idsq(nclasses); 
	vector<C2DFImage::iterator> iprob(nclasses); 
	
	transform(deltasq.begin(), deltasq.end(), idsq.begin(), [](const C2DFImage& img){return img.begin();}); 
	transform(m_probs.begin(), m_probs.end(), iprob.begin(), [](C2DFImage& img){return img.begin();}); 

	auto ii = m_image.begin();  

	while (idsq[0] != deltasq[0].end()) {
		float sum = 0.0;
		bool defined_prob = false; 
		cvdebug() << "pixel=" << *ii++ << ", v = "; 
		for (int i = 0; i < nclasses && !defined_prob; ++i) {
			auto v = *idsq[i];
			cverb << v << " "; 
			// if this value is very small, then the pixel has a very high probability to be 
			// of the current class, 
			if (fabs(v) >= SMALL) 
				sum += 1.0f / v;
			else {
				for (int j = 0; j < nclasses; ++j) {
					if (j != i) 
						*iprob[j] = 0.0; 
					else 
						*iprob[j] = 1.0; 
				}
				defined_prob = true;
			}
		}
		if (!defined_prob) {
			cvdebug() << ", probs = (sum="<<sum<<") "; 
			for (int i = 0; i < nclasses; ++i) {
				*iprob[i] =  1.0f / (*idsq[i] * sum); 
				cverb << *iprob[i] << ", ";
			}
			cverb << "\n"; 
		} else {
			cvdebug() << "probs = permutation of (1 0 0 ...) \n"; 
		}
		for (int i = 0; i < nclasses; ++i) {
			++iprob[i]; 
			++idsq[i]; 
		}
	
	}
}

void MyFCM::run()
{
	// helper for differences
	vector<C2DFImage> deltasq(m_probs.size(), C2DFImage(m_size)); 
	C2DFImage corrected(m_size); 
	const int nclasses = m_class_centres.size(); 

	cvmsg() << "Initial class centers = " << m_class_centres << "\n"; 

	auto old_class_centres = m_class_centres; 
	float delta_cls = 0.0; 
	int maxiter = 30; 
	do  {
		
		// evaluate the b-field corrected image \| \y_k - \beta_k \|^2  in log-space 
		transform(m_image.begin(), m_image.end(), m_gain.begin(), corrected.begin(), 
			  [](float y, float b) {return y - b;}); 
			
		for (int i = 0; i < nclasses; ++i) {
			float vi = m_class_centres[i]; 
			
			// evaluate D_{ik} = \| \y_k - \beta_k - v_i \|^2  -> deltasq
			transform(corrected.begin(), corrected.end(), deltasq[i].begin(), 
				  [&vi](float x) -> float {
					  const float value = x - vi; 
					  return value * value; 
				  }); 
			
			// apply the neighbourhood filter
			filter_neighborhood(deltasq[i]);

			// if p = 2, the exponent becomes 1.0 and we can save this step 
			if (m_p != 2.0) {
				const float p_help = 1 / (1.0 - m_p); 
				transform(deltasq[i].begin(), deltasq[i].end(), deltasq[i].begin(), 
					  [&p_help](float x) { return pow(x, p_help); }); 
			}
		}

		
		
		// evaluate the probabilites according to eq. 13
		estimate_prob(deltasq);
		
		// now evaluate the power to p of the probabilities 
		if (m_p != 2.0) {
			for (int i = 0; i < nclasses; ++i) {
				transform(m_probs[i].begin(),  m_probs[i].end(), deltasq[i].begin(), 
					  [this](float x) { return pow(x, m_p);}); 
			}
		} else {
			for (int i = 0; i < nclasses; ++i) {
				transform(m_probs[i].begin(),  m_probs[i].end(), deltasq[i].begin(), 
					  [](float x){return x*x;});
			}
		}
		
                // apply the neighbourhood filter to the corrected image 
		filter_neighborhood(corrected);
		
		// evaluate new class centres according to eq. 15
		for (int i = 0; i < nclasses; ++i) {
			float sum = 0.0f; 
			float n = 0.0f; 
			auto ic = corrected.begin(); 
			for (auto up = deltasq[i].begin(); up != deltasq[i].end(); ++ic, ++up) {
				sum += *up * *ic; 
				n += *up; 
			}
			m_class_centres[i] = sum / (( 1.0 + m_alpha) * n); 
		}
		
		// now update the gain field according to eq. 19  
		
		vector<C2DFImage::const_iterator> iprobs(nclasses); 
		transform(deltasq.begin(), deltasq.end(), iprobs.begin(), [](const C2DFImage& img){return img.begin();});

		auto ii = m_image.begin(); 
		for (auto ig = m_gain.begin(); ig != m_gain.end(); ++ig, ++ii) {
			float sum = 0.0; 
			float n = 0.0; 
			for (int i = 0; i < nclasses; ++i) {
				sum += m_class_centres[i] * *iprobs[i]; 
				n += *iprobs[i]; 
				++iprobs[i]; 
			}
			*ig = *ii - sum / n ; 
		}
		delta_cls = inner_product(old_class_centres.begin(), old_class_centres.end(), 
					  m_class_centres.begin(), 0.0f, 
					  [](float x, float y) {return x + y;}, 
					  [](float x, float y) -> float {float d = x-y; return d * d; });
		cvmsg() << "Class centers = " << m_class_centres << ", delta = " << delta_cls << "\n"; 
		old_class_centres = m_class_centres; 
		--maxiter; 
	} while (delta_cls > m_epsilon && maxiter); 
	if (!maxiter) 
		throw runtime_error("no convergence"); 
}


template <typename T> 
P2DImage CSegment2DFuzzy::operator() (const T2DImage<T>& image)
{
	MyFCM myfmc(image,  m_params); 
	myfmc.run();
	m_probs = myfmc.get_probs();
	m_gain = myfmc.get_gain();
	
	return myfmc.get_corrected_image<T>(); 
}


int do_main(int argc, char *argv[])
{

	string in_filename; 
	string cls_filename; 
	string out_filename; 
	string gain_filename; 

	CSegment2DFuzzy::Params params; 
	unsigned int n_classes = 3; 
	const auto& image2dio = C2DImageIOPluginHandler::instance();
	
	CCmdOptionList opts(g_description);
	opts.set_group("File I/O"); 
	opts.add(make_opt( in_filename, "in-file", 'i', "image to be segmented", CCmdOptionFlags::required_input , &image2dio)); 
	opts.add(make_opt( cls_filename, "cls-file", 'c', "class probability images, the image type must "
			   "support multiple images and floating point values", CCmdOptionFlags::output, &image2dio )); 
	opts.add(make_opt( out_filename, "out-file", 'o', "B-field corrected image", CCmdOptionFlags::output , &image2dio)); 
	opts.add(make_opt( gain_filename, "gain-log-file", 'g', "Logarithmic gain field, the image type must "
			   "support floating point values",  CCmdOptionFlags::output , &image2dio)); 
	
	opts.set_group("Segmentation parameters");
	opts.add(make_opt( n_classes, "no-of-classes", 'n', "number of classes to segment"));
	opts.add(make_opt( params.class_centres, "class-centres", 'C', "initial class centers"));
	opts.add(make_opt( params.neighbourhood_filter, "shmean:shape=8n", "neighborhood", 'N', "neighborhood filter for B-field correction"));
	opts.add(make_opt( params.alpha, "alpha", 'a', "weight of neighborhood filter for B-field correction"));
	opts.add(make_opt( params.p, "fuzziness", 'p', 
			   " parameter describing the fuzzyness of mattar distinction")); 
	opts.add(make_opt( params.epsilon, "epsilon", 'e', 
			   "Stopping criterion for class center estimation.")); 

	
	if (opts.parse(argc, argv) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 
	
	if (cls_filename.empty() && out_filename.empty() && gain_filename.empty()) {
		throw invalid_argument("Not a single output given, processing makes no sense");  
	}

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

	if (!cls_filename.empty()) {
		auto probs = segment.get_probs(); 
		if (!C2DImageIOPluginHandler::instance().save(cls_filename, probs)) {
			cvwarn() << "Unable to save class probabilities to '" << cls_filename << "'\n"; 
		}
	}

	if (!gain_filename.empty()) {
		auto gain = segment.get_gain(); 
		if (!save_image(gain_filename, gain)) {
			cvwarn() << "Unable to save gain to '" << gain_filename << "'\n"; 
		}
	}

	return EXIT_SUCCESS; 
}

#include <mia/internal/main.hh>
MIA_MAIN(do_main); 


