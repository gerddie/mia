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
#include <mia/core/cmeans.hh>
#include <mia/2d.hh>

#include <memory>
#include <vector>

using namespace mia;
using namespace std; 

typedef vector<C2DFImage> C2DFImageVec;

const SProgramDescription g_description = {
        {pdi_group, "Analysis, filtering, combining, and segmentation of 2D images"}, 
	{pdi_short, "Run a c-means segmentation of a 2D image."}, 
	{pdi_description, "This program runs the segmentation of a 2D image by applying "
	 "a localized c-means approach that helps to overcome intensity inhomogeneities "
	 "in the image. The approach evaluates a global c-means clustering, and then "
	 "separates the image into overlapping regions where more c-means iterations "
	 "are run only including the locally present classes, i.e. the classes that "
	 "relatively contain more pixels than a given threshhold."}, 
	{pdi_example_descr, "Run the segmentation on image test.png using three classes, "
	 "local regions of 40 pixels (grid width 20 pixels), and a class ignore threshhold of 0.01." }, 
	{pdi_example_code, "-i test.png -o label.png -n 3 -g 20 -t 0.01"}
}; 

class FRunHistogram : public TFilter<void> {
public: 
	template <typename T> 
	void operator()(const T2DImage<T>& image);

	CSparseHistogram::Compressed get_histogram() const; 
	
private: 
	CSparseHistogram m_sparse_histogram; 
}; 

class FLocalCMeans: public TFilter<void> {
public: 
	
	FLocalCMeans(float k, float epsilon, const vector<double>& global_class_centers,
		     const C2DBounds& start, const C2DBounds& end,
		     const CMeans::SparseProbmap& global_probmap,
		     float rel_cluster_threshold, 
		     const map<short, int>& segmap, 
		     C2DFImageVec& prob_buffer);
	
	template <typename T> 
	void operator()(const T2DImage<T>& image);
private:

	float m_k;
	float m_epsilon;
	const vector<double>& m_global_class_centers;
	C2DBounds m_start;
	C2DBounds m_end;
	const CMeans::SparseProbmap& m_global_probmap;
	float m_rel_cluster_threshold; 
	const map<short, int>& m_segmap;
	C2DFImageVec& m_prob_buffer;
	size_t m_count; 
	
}; 



int do_main(int argc, char *argv[])
{

	string in_filename; 
	string out_filename;
	string out_filename2;
	string debug_filename; 
	
        int blocksize = 20;
	unsigned n_classes = 3;
	double rel_cluster_threshold = 0.0001;
	float cmeans_k = 64.0; // should be based on input noise

	float cmeans_epsilon = 0.0001; 
	
	CMeans::PInitializer class_center_initializer; 
		
	const auto& image2dio = C2DImageIOPluginHandler::instance();

	CCmdOptionList opts(g_description);
        opts.set_group("File-IO"); 
	opts.add(make_opt( in_filename, "in-file", 'i', "image to be segmented",
			   CCmdOptionFlags::required_input, &image2dio )); 
	opts.add(make_opt( out_filename, "out-file", 'o', "class label image based on "
			   "merging local labels", CCmdOptionFlags::output, &image2dio ));
	
        opts.set_group("Parameters"); 
	opts.add(make_opt( blocksize, EParameterBounds::bf_min_closed, {3},
			   "grid-spacing", 'g', "Spacing of the grid used to modulate "
			   "the intensity inhomogeneities"));
	opts.add(make_opt( class_center_initializer, "kmeans:nc=3",
			   "cmeans", 'c', "c-means initializer"));
	opts.add(make_opt( cmeans_k, EParameterBounds::bf_min_closed,
			   {0.0}, "c-means-k", 'k', "c-means distribution variance parameter"));
	opts.add(make_opt( cmeans_epsilon, EParameterBounds::bf_min_open,
			   {0.0}, "c-means-epsilon", 'e', "c-means breaking condition for update tolerance"));
	opts.add(make_opt( rel_cluster_threshold, EParameterBounds::bf_min_closed | EParameterBounds::bf_max_open,
			   {0.0,1.0}, "relative-cluster-threshhold", 't', "Number of intensity classes to segment")); 

	
	if (opts.parse(argc, argv) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 

	if (out_filename.empty() && out_filename2.empty()) {
		throw invalid_argument("You must specify at least one output file"); 
	}
	
	auto in_image = load_image2d(in_filename);

	FRunHistogram global_histogram; 

	mia::accumulate(global_histogram, *in_image);
	
	CMeans globale_cmeans(cmeans_k, cmeans_epsilon, class_center_initializer); 
	CMeans::DVector global_class_centers; 
	auto global_sparse_probmap = globale_cmeans.run(global_histogram.get_histogram(),
							global_class_centers);
	
	map<short, int> segmap;
	for_each(global_sparse_probmap.begin(), global_sparse_probmap.end(),
		 [&segmap](const CMeans::SparseProbmap::value_type& x) {
			 int c = 0;
			 float max_prob = 0.0f;
			 for (unsigned i = 0; i< x.second.size(); ++i) {
				 auto f = x.second[i]; 
				 if (f > max_prob) {
					 max_prob = f;
					 c = i; 
				 };
			 }
			 segmap[x.first] = c; 
		 }); 
	

	int  nx = (in_image->get_size().x + blocksize - 1) / blocksize;
	int  ny = (in_image->get_size().y + blocksize - 1) / blocksize;
	int  start_x = (nx * blocksize - in_image->get_size().x) / 2; 
	int  start_y = (ny * blocksize - in_image->get_size().y) / 2; 

	vector<C2DFImage> prob_buffer(global_class_centers.size(), C2DFImage(in_image->get_size())); 
	
	for (int  iy_base = start_y; iy_base < (int)in_image->get_size().y; iy_base += 2 * blocksize) {
		unsigned iy = iy_base < 0 ? 0 : iy_base;
		unsigned iy_end = iy_base + 2 * blocksize;
		if (iy_end > in_image->get_size().y)
			iy_end = in_image->get_size().y; 
		
		for (int ix_base = start_x; ix_base < (int)in_image->get_size().x; ix_base += 2 * blocksize) {
			unsigned ix = ix_base < 0 ? 0 : ix_base;
			unsigned ix_end = ix_base + 2 * blocksize;
			if (ix_end > in_image->get_size().x)
				ix_end = in_image->get_size().x; 
			

			

			FLocalCMeans lcm(cmeans_k, cmeans_epsilon, global_class_centers,
					 C2DBounds(ix, iy), C2DBounds(ix_end, iy_end),
					 global_sparse_probmap,
					 rel_cluster_threshold,
					 segmap, 
					 prob_buffer);
			
			mia::accumulate(lcm, *in_image); 
		}
	}
	// save the prob images ? 
	
	// now for each pixel we have a probability sume that should take inhomogeinities
	// into account

	C2DUBImage out_image(in_image->get_size(), *in_image);
	fill(out_image.begin(), out_image.end(), 0);

	for (unsigned c = 1; c < n_classes; ++c) {
		auto iout = out_image.begin();
		auto eout = out_image.end();
		
		auto itest = prob_buffer[0].begin();
		auto iprob = prob_buffer[c].begin();

		while ( iout != eout ){
			if (*itest < *iprob) {
				*itest = *iprob;
				*iout = c; 
			}
			++iout; ++itest; ++iprob; 
		}
	}

	return save_image(out_filename, out_image) ? EXIT_SUCCESS : EXIT_FAILURE; 
}


template <typename T> 
void FRunHistogram::operator()(const T2DImage<T>& image)
{
	m_sparse_histogram(image.begin(), image.end()); 
}

CSparseHistogram::Compressed FRunHistogram::get_histogram() const
{
	return m_sparse_histogram.get_compressed_histogram(); 
}


FLocalCMeans::FLocalCMeans(float k, float epsilon, const vector<double>& global_class_centers,
		     const C2DBounds& start, const C2DBounds& end,
		     const CMeans::SparseProbmap& global_probmap,
		     float rel_cluster_threshold, 
		     const map<short, int>& segmap, 
		     C2DFImageVec& prob_buffer):
	m_k(k),
	m_epsilon(epsilon),
	m_global_class_centers(global_class_centers),
	m_start(start),
	m_end(end),
	m_global_probmap(global_probmap),
	m_rel_cluster_threshold(rel_cluster_threshold),
	m_segmap(segmap), 
	m_prob_buffer(prob_buffer),
	m_count((m_end - m_start).product())
{
}

template <typename T> 
void FLocalCMeans::operator()(const T2DImage<T>& image)
{
	// obtain the histogram for the current patch 
	CSparseHistogram histogram;
	histogram(image.begin_range(m_start, m_end), 
		  image.end_range(m_start, m_end));
	auto ch = histogram.get_compressed_histogram();

	// calculate the class avaliability in the patch
	vector<double> partition(m_global_class_centers.size());
	for (auto ich: ch) {
		const double n = ich.second; 
		auto v = m_global_probmap[ich.first];
		transform(partition.begin(), partition.end(), v.second.begin(),
			  partition.begin(), [n](double p, double value){return p + n * value;}); 
	}
	auto part_thresh = accumulate(partition.begin(), partition.end(), 0.0) * m_rel_cluster_threshold; 
	
	cvdebug() << "Partition = " << partition << "\n";

	// select the classes that should be used further on
	vector<double> retained_class_centers;
	vector<unsigned short> used_classed; 
	for (unsigned i = 0; i < partition.size(); ++i) {
		if (partition[i] >= part_thresh) {
			retained_class_centers.push_back(m_global_class_centers[i]);
			used_classed.push_back(i);
		}
	}
	
	stringstream cci_descr;
	cci_descr << "predefined:cc=" << retained_class_centers;
	auto cci = CMeansInitializerPluginHandler::instance().produce(cci_descr.str()); 
	
	// remove data that was globally assigned to now unused class
	
	CSparseHistogram::Compressed cleaned_histogram;
	cleaned_histogram.reserve(ch.size());
	vector<int> reindex(ch.size(), -1); 
	int ireindex = 0; 
	
	// copy used intensities 
	for (auto c: used_classed) {
		for (auto ich: ch) {
			if ( m_segmap.at(ich.first) == c) {
				cleaned_histogram.push_back(ich);
			}
		}
	}

	// evluate the local c-means classification 
	CMeans local_cmeans(m_k, m_epsilon, cci);
	auto local_map = local_cmeans.run(ch,  retained_class_centers);

	// sum the local probablities to global proability sum map
	map<unsigned short, CMeans::DVector> mapper;
	for (auto i: local_map) {
		mapper[i.first] = i.second; 
	}
	auto ii = image.begin_range(m_start, m_end);
	auto ie = image.end_range(m_start, m_end);

	while (ii != ie) {
		auto probs = mapper.find(*ii);
		if (probs != mapper.end()) {
			for (unsigned c = 0; c < used_classed.size();  ++c) {
				m_prob_buffer[used_classed[c]](ii.pos()) += probs->second[c];  
			}
		}
		++ii; 
	}
}

#include <mia/internal/main.hh>
MIA_MAIN(do_main); 

