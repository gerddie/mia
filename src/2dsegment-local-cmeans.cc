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
#include <mia/core/kmeans.hh>
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
		     C2DFImageVec& prob_buffer);
	
	template <typename T> 
	void operator()(const T2DImage<T>& image);
private:

	float m_k;
	float m_epsilo;
	const vector<double>& m_global_class_centers;
	C2DBounds m_start;
	C2DBounds m_end;
	CMeans::SparseProbmap m_global_probmap; 
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
	float 

	PInitializer class_center_initializer; 
		
	const auto& image2dio = C2DImageIOPluginHandler::instance();

	CCmdOptionList opts(g_description);
        opts.set_group("File-IO"); 
	opts.add(make_opt( in_filename, "in-file", 'i', "image to be segmented",
			   CCmdOptionFlags::required_input, &image2dio )); 
	opts.add(make_opt( out_filename, "out-file", 'o', "class label image based on "
			   "merging local labels", CCmdOptionFlags::output, &image2dio ));
	opts.add(make_opt( out_filename2, "out-file2", 'O', "class label image based on "
			   "classifying by using the interpolated class centers ",
			   CCmdOptionFlags::output, &image2dio ));
	opts.add(make_opt( debug_filename, "debug-centers", 'D', "Save the class center "
			   "images to a debug file (type is double)",
			   CCmdOptionFlags::output, &image2dio ));

	
        opts.set_group("Parameters"); 
	opts.add(make_opt( blocksize, EParameterBounds::bf_min_closed, {3},
			   "grid-spacing", 'g', "Spacing of the grid used to modulate "
			   "the intensity inhomogeneities"));
	opts.add(make_opt( class_center_initializer, "kmeans:nc=3",
			   "cmeans", 'c', "c-means initializer"));
	opts.add(make_opt( cmeans_k, EParameterBounds::bf_min_closed,
			   {0.0}, "c-means-k  ",  "c-means distribution variance parameter"));
	opts.add(make_opt( cmeans_k, EParameterBounds::bf_min_closed,
			   {0.0}, "c-means-k  ", 'k', "c-mens distribution variance parameter"));
	
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
	
	CMeans globale_cmeans(k, epsilon, class_center_initializer); 
	DVector global_class_centers; 
	auto global_sparse_probmap = globale_cmeans.run(global_histogram.get_sparse_histogram(),
							global_class_centers);

	int  nx = (in_image->get_size().x + blocksize - 1) / blocksize;
	int  ny = (in_image->get_size().y + blocksize - 1) / blocksize;
	int  start_x = (nx * blocksize - in_image->get_size().x) / 2; 
	int  start_y = (ny * blocksize - in_image->get_size().y) / 2; 
	
	for (int  iy_base = start_y; iy < in_image->get_size().y; iy += 2 * blocksize) {
		unsigned iy = iy_base < 0 ? 0 : iy_base;
		unsigned iy_end = iy_base + 2 * blocksize;
		if (iy_end > in_image->get_size().y)
			iy_end = in_image->get_size().y; 
		
		for (int ix_base = start_x; ix < in_image->get_size().x; ix += 2 * blocksize) {
			unsigned ix = ix_base < 0 ? 0 : ix_base;
			unsigned ix_end = ix_base + 2 * blocksize;
			if (ix_end > in_image->get_size().x)
				ix_end = in_image->get_size().x; 
			
			FLocalCMeans lcm(k, epsilon, global_class_centers,
					 C2DBounds(ix, iy), C2DBounds(ix_end, iy_end),
					 global_sparse_probmap, 
					 prob_buffer);
			
			mia::accumulate(lcm, *in_image); 
		}
	}
	

	
	
	
	
	return EXIT_SUCCESS; 
	
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
			   C2DFImageVec& prob_buffer):
	m_k(k),
	m_epsilon(epsilon),
	m_global_class_centers(global_class_centers),
	m_start(start),
	m_end(end),
	m_global_probmap(global_probmap), 
	m_prob_buffer(prob_buffer),
	m_count((m_end - m_start).product())
{
}

template <typename T> 
void FLocalCMeans::operator()(const T2DImage<T>& image)
{
	FRunHistogram histogram;

	histogram(image.range_begin(m_start, m_end);
		  image.range_endd(m_start, m_end));

	auto ch = histogram.get_compressed_histogram();

	vector<double> partition(m_global_class_centers.size());
	for (auto ich: ch) {
		const double n = ich.second; 
		auto v = m_global_probmap[ich.first];
		transform(partition.begin(), partition.end(), v.begin(),
			  partition.begin(), [n](double p, double value){return p + n * value;}); 
	}

	cvdebug() << "Partition = " << partition "\n";
	
	vector<double> retained_class_centers;
	vector<int> used_classed; 

	for (unsigned i = 0; i < partition.size(); ++i) {
		if (partition[i] >= m_partition_thresh) {
			retained_class_centers.push_back(m_global_class_centers[i]);
			used_classed.push_back(i);
		}
	}

	stringstream cci_descr;
	cci_descr << "predefined:cc=" << retained_class_centers;
	auto cci = CMeansInitializerPluginHandler::instance().create(cci_descr.str()); 
	
	// now estimate 
	CMeans globale_cmeans(m_k, m_epsilon, cci);
	
	auto local_map = run(ch,  retained_class_centers);

	
	
	
}

#include <mia/internal/main.hh>
MIA_MAIN(do_main); 

