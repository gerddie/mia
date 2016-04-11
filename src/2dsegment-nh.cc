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
	{pdi_short, "Run a segmentation of a noisy  2D image."}, 
	{pdi_description, "This program is experimental"}, 
	{pdi_example_descr, "No example yet" }, 
	{pdi_example_code, "-i input.png -o label.png"}
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

       

int do_main(int argc, char *argv[])
{

	string in_filename; 
	string out_filename;

       
        int blocksize = 20;
	unsigned n_classes = 3;
	double rel_cluster_threshold = 0.0001;
		
	const auto& image2dio = C2DImageIOPluginHandler::instance();

	CCmdOptionList opts(g_description);
        opts.set_group("File-IO"); 
	opts.add(make_opt( in_filename, "in-file", 'i', "image to be segmented", CCmdOptionFlags::required_input, &image2dio )); 
	opts.add(make_opt( out_filename, "out-file", 'o', "class label image", CCmdOptionFlags::required_output, &image2dio ));

        opts.set_group("Parameters"); 
	opts.add(make_opt( blocksize, EParameterBounds::bf_min_closed, {3},
			   "grid-spacing", 'g', "Spacing of the grid used to modulate the intensity inhomogeneities"));
	opts.add(make_opt( n_classes, EParameterBounds::bf_closed_interval, {2u,127u},
			   "nclasses", 'n', "Number of intensity classes to segment")); 
	opts.add(make_opt( rel_cluster_threshold, EParameterBounds::bf_open_interval, {0.0,1.0},
			   "relative-cluster-threshhold", 't', "Number of intensity classes to segment")); 
	
	
	if (opts.parse(argc, argv) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 
	
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

	unsigned nx = (in_image->get_size().x + blocksize - 1) / blocksize;
	unsigned ny = (in_image->get_size().y  + blocksize - 1)/ blocksize;

	int xtarget = 0;
	int ytarget = 0; 
	for (unsigned iy = 0; iy < ny; ++iy, ytarget = (ytarget + 1)%2) {
		for (unsigned ix = 0; ix < nx; ++ix, xtarget = (xtarget + 1)%2) {
			C2DBounds start(ix * blocksize, iy * blocksize);
			C2DBounds end((2+ix) * blocksize, (2+iy) * blocksize);

			if (end.x > in_image->get_size().x)
				end.x = in_image->get_size().x;
			
			if (end.y > in_image->get_size().y)
				end.y = in_image->get_size().y; 

			local_kmeans.set_range(start, end, xtarget, ytarget);

			auto local_class_centers = mia::accumulate(local_kmeans, *in_image); 
		}
	}

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
	cvmsg() << "Save result to " << out_filename << "\n"; 
	save_image(out_filename, result_labels); 
	
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
		auto p = i.pos();
		if (p.x > 430 && p.y > 1390)
			cvdebug() << "pos= " << i.pos() << "\n"; 
		const unsigned c = kmeans_get_closest_clustercenter(m_in_classes, l, *i); 
		++cluster_sizes[c]; 
		++i;
		++n; 
	}
	
	vector<double> rel_cluster_sizes(m_in_classes.size());
	transform(cluster_sizes.begin(), cluster_sizes.end(), rel_cluster_sizes.begin(),
		  [n](double x){return x / n;});
	
	cvmsg() << "Block [" << m_start << "],[" <<  m_end <<"] Relative cluster sizes: " << rel_cluster_sizes << "\n"; 
	
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
	
	return result;
}

#include <mia/internal/main.hh>
MIA_MAIN(do_main); 

