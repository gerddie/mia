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
	{pdi_short, "Run a k-means segmentation of a 2D image."}, 
	{pdi_description, "This program runs the segmentation of a 2D image by applying "
	 "a localized k-means approach that helps to overcome intensity inhomogeneities "
	 "in the image. The approach evaluates a global k-means clustering, and then "
	 "separates the image into overlapping regions where more k-means iterations "
	 "are run only including the locally present classes, i.e. the classes that "
	 "relatively contain more pixels than a given threshhold."}, 
	{pdi_example_descr, "Run the segmentation on image test.png using three classes, "
	 "local regions of 40 pixels (grid width 20 pixels), and a class ignore threshhold of 0.01." }, 
	{pdi_example_code, "-i test.png -o label.png -n 3 -g 20 -t 0.01"}
}; 


class FKmeans: public TFilter<vector<double>> {
public: 
	FKmeans(const vector<double>& in_classes, vector<C2DSBImage>& class_output, double rel_cluster_threshold);
	void set_range(const C2DBounds& start, const C2DBounds& end, int xtarget, int ytarget);

	template <typename T>
	vector<double> operator () (const T2DImage<T>& image);

private:
	vector<double> m_in_classes;
	vector<C2DSBImage>& m_class_output;
	double m_rel_cluster_threshold; 
	C2DBounds m_start; 
	C2DBounds m_end;
	int m_xtarget;
	int m_ytarget; 
}; 

class FKMeansLocal: public TFilter<P2DImage> {
public:
	FKMeansLocal(const vector<C2DDImage>& class_centers); 

	template <typename T>
	P2DImage operator () (const T2DImage<T>& image) const;
private:
	const vector<C2DDImage>& m_class_centers; 
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

		
	const auto& image2dio = C2DImageIOPluginHandler::instance();

	CCmdOptionList opts(g_description);
        opts.set_group("File-IO"); 
	opts.add(make_opt( in_filename, "in-file", 'i', "image to be segmented", CCmdOptionFlags::required_input, &image2dio )); 
	opts.add(make_opt( out_filename, "out-file", 'o', "class label image based on merging local labels", CCmdOptionFlags::output, &image2dio ));
	opts.add(make_opt( out_filename2, "out-file2", 'O', "class label image based on classifying by using the interpolated class centers ",
			   CCmdOptionFlags::output, &image2dio ));
	opts.add(make_opt( debug_filename, "debug-centers", 'D', "Save the class center images to a debug file (type is double)",
			   CCmdOptionFlags::output, &image2dio ));

	
        opts.set_group("Parameters"); 
	opts.add(make_opt( blocksize, EParameterBounds::bf_min_closed, {3},
			   "grid-spacing", 'g', "Spacing of the grid used to modulate the intensity inhomogeneities"));
	opts.add(make_opt( n_classes, EParameterBounds::bf_closed_interval, {2u,127u},
			   "nclasses", 'n', "Number of intensity classes to segment")); 
	opts.add(make_opt( rel_cluster_threshold, EParameterBounds::bf_min_closed | EParameterBounds::bf_max_open,
			   {0.0,1.0}, "relative-cluster-threshhold", 't', "Number of intensity classes to segment")); 


	
	if (opts.parse(argc, argv) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 

	if (out_filename.empty() && out_filename2.empty()) {
		throw invalid_argument("You must specify at least one output file"); 
	}
	
	auto in_image = load_image2d(in_filename);
	stringstream kfilter_ss;
	kfilter_ss << "kmeans:c=" << n_classes; 

	auto full_kmeans = run_filter(in_image, kfilter_ss.str().c_str());

	vector <C2DSBImage> class_output(4, C2DSBImage(in_image->get_size()));
	for (auto image : class_output) {
		fill(image.begin(), image.end(), -1); 
	}
	
	auto full_classes_ptr = full_kmeans->get_attribute(ATTR_IMAGE_KMEANS_CLASSES);
	const CVDoubleAttribute& full_classes = dynamic_cast<const CVDoubleAttribute&>(*full_classes_ptr); 
	
	FKmeans local_kmeans(full_classes, class_output, rel_cluster_threshold);

	// true block size is selected so that the start and end pixles are on the boundaries
	unsigned nx = (in_image->get_size().x + blocksize - 1) / blocksize;
	unsigned ny = (in_image->get_size().y  + blocksize - 1)/ blocksize;
	double scalex = double(in_image->get_size().x - 1) / double (nx-1);
	double scaley = double(in_image->get_size().y - 1) / double (ny-1);   

	cvmsg() << "using a scale factor of " << scalex << " x "<< scaley << "\n"; 

	int xtarget = 0;
	int ytarget = 0;

	vector<C2DDImage> local_class_center_img(n_classes, C2DDImage(C2DBounds(nx, ny))); 

	vector<C2DDImage::iterator> ilcc;
	for (unsigned i = 0; i < n_classes; ++i) {
		ilcc.push_back(local_class_center_img[i].begin()); 
	}
	
	for (unsigned iy = 0; iy < ny; ++iy, ytarget = (ytarget + 1)%2) {
		for (unsigned ix = 0; ix < nx; ++ix, xtarget = (xtarget + 1)%2) {
			
			C2DBounds start(static_cast<unsigned>(ix * scalex),static_cast<unsigned>(iy * scaley));
			C2DBounds end(static_cast<unsigned>(2 * (ix +1) * scalex),static_cast<unsigned>( 2 * (iy +1) * scaley));

			if (start.x > in_image->get_size().x)
				start.x = 0;
			
			if (start.y > in_image->get_size().y)
				start.y = 0; 

			if (end.x > in_image->get_size().x)
				end.x = in_image->get_size().x;
			
			if (end.y > in_image->get_size().y)
				end.y = in_image->get_size().y; 

			local_kmeans.set_range(start, end, xtarget, ytarget);

			auto local_class_centers = mia::accumulate(local_kmeans, *in_image);
			for (unsigned i = 0; i < n_classes; ++i) {
				*ilcc[i]++ = local_class_centers[i];
			}
		}
	}

	if (!out_filename.empty())  {
		// merge the class labels;
		cvmsg() << "Create result image of size" << in_image->get_size() << "\n"; 
		C2DUBImage result_labels(in_image->get_size(), *in_image);
		
		auto iout = result_labels.begin();
		auto eout = result_labels.end();
		
		vector<C2DSBImage::const_iterator> ii; 
		
		for (int i = 0; i < 4; ++i) {
			ii.push_back(class_output[i].begin());
		}
		
		vector<unsigned> counts(n_classes); 
		while (iout != eout)  {
			fill(counts.begin(), counts.end(), 0);
			for (int i = 0; i < 4; ++i) {
				if (*ii[i] >= 0) {
					++counts[*ii[i]];
				}
				++ii[i]; 
			}
			unsigned max_count = 0;
			int max_class = -1;
			
			unsigned k = 0; 
			
			while ( (max_count << 1) < n_classes && k < n_classes ) {
				if (max_count < counts[k]) {
					max_count = counts[k];
					max_class = k; 
				}
				++k; 
			}
			*iout = max_class; 
			++iout; 
		}
		save_image(out_filename, result_labels);
	}

	if (!out_filename2.empty()){
		cvmsg() << "Save interpolated class center based labeling to " << out_filename2 << "\n"; 
		stringstream scale_filter_ss;
		scale_filter_ss << "scale:sx=" << in_image->get_size().x
				<< ",sy=" << in_image->get_size().y << ",interp=[bspline:d=1]"; 
		auto scale_filter = produce_2dimage_filter(scale_filter_ss.str().c_str()); 

		C2DImageIOPPH::Data scaled_images;
		transform(local_class_center_img.begin(), local_class_center_img.end(),
			  back_inserter<C2DImageIOPPH::Data>(scaled_images),
			  [ scale_filter ](const C2DDImage& img){return scale_filter->filter(img);}); 

		transform(scaled_images.begin(), scaled_images.end(), local_class_center_img.begin(),
			  [](P2DImage img){ return dynamic_cast<const C2DDImage&>(*img);}); 
		
		if (!debug_filename.empty()) {
			image2dio.save(debug_filename, scaled_images); 
		}
		
		FKMeansLocal means_local(local_class_center_img);
		auto result = mia::filter(means_local, *in_image);

		save_image(out_filename2, result);
	}
	
	return EXIT_SUCCESS; 
	
}


FKmeans::FKmeans(const vector<double>& in_classes, vector<C2DSBImage>& class_output,
		 double rel_cluster_threshold):
	m_in_classes(in_classes),
	m_class_output(class_output),
	m_rel_cluster_threshold(rel_cluster_threshold)
{
}

void FKmeans::set_range(const C2DBounds& start, const C2DBounds& end, int xtarget, int ytarget)
{
	m_start = start; 
	m_end = end;
	m_xtarget = xtarget;
	m_ytarget = ytarget; 
}
	

template <typename T>
vector<double> FKmeans::operator () (const T2DImage<T>& image)
{
	auto ibegin = image.begin_range(m_start, m_end);
	auto iend = image.end_range(m_start, m_end);
	
	auto i = ibegin;
	
	size_t n = 0; 
	vector<size_t> cluster_sizes(m_in_classes.size());
	size_t l = m_in_classes.size() - 1; 
	
	while ( i != iend ) {
		const unsigned c = kmeans_get_closest_clustercenter(m_in_classes, l, *i); 
		++cluster_sizes[c]; 
		++i;
		++n; 
	}
	
	vector<double> rel_cluster_sizes(m_in_classes.size());
	transform(cluster_sizes.begin(), cluster_sizes.end(), rel_cluster_sizes.begin(),
		  [n](double x){return x / n;});
	
	cvinfo() << "Block [" << m_start << "],[" <<  m_end <<"] Relative cluster sizes: " << rel_cluster_sizes << "\n"; 
	
	vector<short> map_idx; 
	vector<double> result; 
	for (unsigned i = 0; i < m_in_classes.size(); ++i) {
		if (rel_cluster_sizes[i] > m_rel_cluster_threshold) {
			result.push_back(m_in_classes[i]);
			map_idx.push_back(i); 
		}
	}
	
	vector<int> class_relation(n);
	
	int biggest_class = -1; 
	// iterate to update the class centers  
	for (size_t  l = 1; l < 4; l++) {
		if (kmeans_step(ibegin, iend, class_relation.begin(), result,
				result.size() - 1, biggest_class)) 
			break; 
	}
	
	transform(class_relation.begin(), class_relation.end(),
		  m_class_output[m_ytarget * 2 + m_xtarget].begin_range(m_start, m_end), 
		  [map_idx](int idx) {return map_idx[idx];});

	// put the classes back into the right size
	vector<double> remapped_result(m_in_classes); 
	for (unsigned i =0; i < result.size(); ++i) {
		remapped_result[map_idx[i]] = result[i]; 
	}
	cvinfo() << "remapped cc=" << remapped_result << "\n"; 
	return remapped_result; 
}

FKMeansLocal::FKMeansLocal(const vector<C2DDImage>& class_centers):
	m_class_centers(class_centers)
{
}

template <typename T>
P2DImage FKMeansLocal::operator () (const T2DImage<T>& image) const
{
	C2DUBImage *result_labels = new C2DUBImage(image.get_size(), image);

	auto i = image.begin();
	auto iout = result_labels->begin();
	auto eout = result_labels->end();
	
	vector<C2DDImage::const_iterator> ii;
	unsigned n_classes = m_class_centers.size(); 
	
	for (unsigned i = 0; i < n_classes; ++i) {
		ii.push_back(m_class_centers[i].begin());
	}
	vector<double> centers( n_classes ); 
	while (iout != eout)  {
		for (unsigned i = 0; i < n_classes; ++i) {
			centers[i] = *ii[i];
			++ii[i]; 
		}
		*iout = kmeans_get_closest_clustercenter(centers, n_classes-1, *i); 

		++iout; ++i; 
	}
	return P2DImage(result_labels); 
}


#include <mia/internal/main.hh>
MIA_MAIN(do_main); 

