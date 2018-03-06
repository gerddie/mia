/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis
 * Copyright (c) Leipzig, Madrid 1999-2017 Gert Wollny
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
#include <mia/core/parallel.hh>
#include <mia/core/threadedmsg.hh>
#include <mia/2d.hh>

#include <memory>
#include <vector>

using namespace mia;
using namespace std;

typedef vector<C2DFImage> C2DFImageVec;

const SProgramDescription g_description = {
       {pdi_group, "Analysis, filtering, combining, and segmentation of 2D images"},
       {pdi_short, "Run a k-means segmentation of a 2D image."},
       {
              pdi_description, "This program runs the segmentation of a 2D image by applying "
              "a localized k-means approach that helps to overcome intensity inhomogeneities "
              "in the image. The approach evaluates a global k-means clustering, and then "
              "separates the image into overlapping regions where more k-means iterations "
              "are run only including the locally present classes, i.e. the classes that "
              "relatively contain more pixels than a given threshold."
       },
       {
              pdi_example_descr, "Run the segmentation on image test.png using three classes, "
              "local regions of 40 pixels (grid width 20 pixels), and a class ignore threshold of 0.01."
       },
       {pdi_example_code, "-i test.png -o label.png -n 3 -g 20 -t 0.01"}
};


class FKmeans: public TFilter<unsigned>
{
public:
       FKmeans(const vector<double>& in_classes, double rel_cluster_threshold);
       void set_position_and_range(int x, int y, const C2DBounds& start, const C2DBounds& end);

       template <typename T>
       unsigned operator () (const T2DImage<T>& image) const;

private:
       vector<double> m_in_classes;
       double m_rel_cluster_threshold;
       C2DBounds m_start;
       C2DBounds m_end;
       int m_x;
       int m_y;
};

class FKMeansLocal: public TFilter<P2DImage>
{
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
       int window = 20;
       unsigned n_classes = 3;
       double rel_cluster_threshold = 0.0001;
       const auto& image2dio = C2DImageIOPluginHandler::instance();
       CCmdOptionList opts(g_description);
       opts.set_group("File-IO");
       opts.add(make_opt( in_filename, "in-file", 'i', "image to be segmented", CCmdOptionFlags::required_input, &image2dio ));
       opts.add(make_opt( out_filename, "out-file", 'o', "class label image based on merging local labels",
                          CCmdOptionFlags::required_output, &image2dio ));
       opts.set_group("Parameters");
       opts.add(make_opt( window, EParameterBounds::bf_min_closed, {3},
                          "window", 'w', "Window size around the pixel to be analyzed"));
       opts.add(make_opt( n_classes, EParameterBounds::bf_closed_interval, {2u, 127u},
                          "nclasses", 'n', "Number of intensity classes to segment"));
       opts.add(make_opt( rel_cluster_threshold, EParameterBounds::bf_min_closed | EParameterBounds::bf_max_open,
       {0.0, 1.0}, "relative-cluster-threshold", 't', "Number of intensity classes to segment"));

       if (opts.parse(argc, argv) != CCmdOptionList::hr_no)
              return EXIT_SUCCESS;

       auto in_image = load_image2d(in_filename);
       stringstream kfilter_ss;
       kfilter_ss << "kmeans:c=" << n_classes;
       auto full_kmeans = run_filter(in_image, kfilter_ss.str().c_str());
       auto full_classes_ptr = full_kmeans->get_attribute(ATTR_IMAGE_KMEANS_CLASSES);
       const CVDoubleAttribute& full_classes = dynamic_cast<const CVDoubleAttribute&>(*full_classes_ptr);
       C2DUBImage result_labels(in_image->get_size(), *in_image);
       auto run_classification = [&result_labels, &full_classes, rel_cluster_threshold, window, &in_image]
       (const C1DParallelRange & range) {
              CThreadMsgStream thread_stream;
              FKmeans local_kmeans(full_classes, rel_cluster_threshold);

              for ( int y = range.begin(); y != range.end(); ++y ) {
                     auto ir = result_labels.begin_at(0, y);

                     for (unsigned x = 0; x < in_image->get_size().x; ++x, ++ir) {
                            C2DBounds start( x - window, y - window);
                            C2DBounds end( x + window, y + window);

                            if (start.x > in_image->get_size().x)
                                   start.x = 0;

                            if (start.y > in_image->get_size().y)
                                   start.y = 0;

                            if (end.x > in_image->get_size().x)
                                   end.x = in_image->get_size().x;

                            if (end.y > in_image->get_size().y)
                                   end.y = in_image->get_size().y;

                            local_kmeans.set_position_and_range(x, y, start, end);
                            *ir = mia::filter(local_kmeans, *in_image);
                     }
              }
       };
       pfor(C1DParallelRange( 0, in_image->get_size().y), run_classification);
       save_image(out_filename, result_labels);
       return EXIT_SUCCESS;
}


FKmeans::FKmeans(const vector<double>& in_classes,
                 double rel_cluster_threshold):
       m_in_classes(in_classes),
       m_rel_cluster_threshold(rel_cluster_threshold)
{
}

void FKmeans::set_position_and_range(int x, int y, const C2DBounds& start, const C2DBounds& end)
{
       m_start = start;
       m_end = end;
       m_x = x;
       m_y = y;
}


template <typename T>
unsigned FKmeans::operator () (const T2DImage<T>& image) const
{
       size_t n =  (m_end.x - m_start.x) * (m_end.y - m_start.y);
       vector<T> buffer(n);
       copy(image.begin_range(m_start, m_end), image.end_range(m_start, m_end), buffer.begin());
       auto i = buffer.begin();
       auto iend = buffer.end();
       vector<size_t> cluster_sizes(m_in_classes.size());
       size_t l = m_in_classes.size() - 1;
       vector<int> class_relation(n);
       auto ic = class_relation.begin();

       while ( i != iend ) {
              const unsigned c = kmeans_get_closest_clustercenter(m_in_classes, l, *i);
              ++cluster_sizes[c];
              *ic++ = c;
              ++i;
       }

       vector<double> rel_cluster_sizes(m_in_classes.size());
       transform(cluster_sizes.begin(), cluster_sizes.end(), rel_cluster_sizes.begin(),
       [n](double x) {
              return x / n;
       });
       vector<double> result(m_in_classes);
       vector<bool> fixed_centers(m_in_classes.size(), false);

       for (unsigned i = 0; i < m_in_classes.size(); ++i) {
              if (rel_cluster_sizes[i] <= m_rel_cluster_threshold) {
                     fixed_centers[i] = true;
              }
       }

       int biggest_class = -1;

       // iterate to update the class centers
       for (size_t  l = 1; l < 4; l++) {
              if (kmeans_step_with_fixed_centers(buffer.begin(), buffer.end(), class_relation.begin(),
                                                 result, fixed_centers, result.size() - 1, biggest_class))
                     break;
       }

       auto c = kmeans_get_closest_clustercenter(result, result.size() - 1, image(m_x, m_y));
       cvdebug() << "Block [" << m_x << ":" << m_y << "]@[" << m_start
                 << "],[" <<  m_end << "] " << image(m_x, m_y) << " c=" << c
                 << ", Relative cluster sizes: " << rel_cluster_sizes << "\n";
       return c;
}


#include <mia/internal/main.hh>
MIA_MAIN(do_main);

