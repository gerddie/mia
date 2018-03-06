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

/*
  Todo:
  - adaptive filter sizes?
  - Consider different tresholds for larger filter width
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <mia/core/histogram.hh>
#include <mia/core/cmdlineparser.hh>
#include <mia/core/cmeans.hh>
#include <mia/core/parallel.hh>
#include <mia/core/threadedmsg.hh>
#include <mia/3d.hh>

#include <memory>
#include <vector>
#include <numeric>

using namespace mia;
using namespace std;

typedef vector<C3DFImage> C3DFImageVec;

const SProgramDescription g_description = {
       {pdi_group, "Analysis, filtering, combining, and segmentation of 3D images"},
       {pdi_short, "Run a c-means segmentation of a 3D image."},
       {
              pdi_description, "This program runs the segmentation of a 3D image by applying "
              "a localized c-means approach that helps to overcome intensity inhomogeneities "
              "in the image. The approach evaluates a global c-means clustering, and then "
              "separates the image into overlapping regions where more c-means iterations "
              "are run only including the locally present classes, i.e. the classes that "
              "relatively contain more pixels than a given threshold. This program implements "
              "algorithm described in: "
              "[Dunmore CJ, Wollny G, Skinner MM. (2018) MIA-Clustering: a novel method "
              "for segmentation of paleontological material. PeerJ 6:e4374.](https://doi.org/10.7717/peerj.4374)"
       },
       {
              pdi_example_descr, "Run the segmentation on image test.png using three classes, "
              "local regions of 40 pixels (grid width 20 pixels), and a class ignore threshold of 0.01."
       },
       {pdi_example_code, "-i test.png -o label.png -n 3 -g 20 -t 0.01"}
};

class FRunHistogram : public TFilter<void>
{
public:
       template <typename T>
       void operator()(const T3DImage<T>& image);

       CSparseHistogram::Compressed get_histogram() const;

private:
       CSparseHistogram m_sparse_histogram;
};

struct ProtectedProbBuffer {
       CMutex mutex;
       vector<C3DFDatafield> probmap;

       ProtectedProbBuffer(int n_classes, const C3DBounds& size);

       ProtectedProbBuffer(const ProtectedProbBuffer& orig);
};


class FLocalCMeans: public TFilter<void>
{
public:
       typedef map<int, CMeans::DVector> Probmap;


       FLocalCMeans(float epsilon, const vector<double>& global_class_centers,
                    const C3DBounds& start, const C3DBounds& end,
                    const Probmap& global_probmap,
                    float rel_cluster_threshold,
                    const map<int, unsigned>& segmap,
                    ProtectedProbBuffer& prob_buffer,
                    bool partition_with_background);

       template <typename T>
       void operator()(const T3DImage<T>& image);
private:

       const float m_epsilon;
       const vector<double>& m_global_class_centers;
       const C3DBounds m_start;
       const C3DBounds m_end;
       const Probmap& m_global_probmap;
       const float m_rel_cluster_threshold;
       const map<int, unsigned>& m_segmap;

       ProtectedProbBuffer& m_prob_buffer;
       size_t m_count;
       bool m_partition_with_background;
};

class FCrispSeg: public TFilter<P3DImage>
{
public:
       FCrispSeg(const map<int, unsigned>& segmap):
              m_segmap(segmap)
       {
       }

       template <typename T>
       P3DImage operator()(const T3DImage<T>& image) const
       {
              P3DImage out_image = make_shared<C3DUBImage>(image.get_size());
              C3DUBImage *help = static_cast<C3DUBImage *>(out_image.get());
              transform(image.begin(), image.end(), help->begin(),
              [this](T x) {
                     return m_segmap.at(x);
              });
              return out_image;
       }
private:
       const map<int, unsigned>& m_segmap;
};



int do_main(int argc, char *argv[])
{
       string in_filename;
       string out_filename;
       string out_filename2;
       string cls_filename;
       string debug_filename;
       int blocksize = 15;
       double rel_cluster_threshold = 0.02;
       float cmeans_epsilon = 0.0001;
       float class_label_thresh = 0.0f;
       bool ignore_partition_with_background = false;
       CMeans::PInitializer class_center_initializer;
       const auto& image3dio = C3DImageIOPluginHandler::instance();
       CCmdOptionList opts(g_description);
       opts.set_group("File-IO");
       opts.add(make_opt( in_filename, "in-file", 'i', "image to be segmented",
                          CCmdOptionFlags::required_input, &image3dio ));
       opts.add(make_opt( out_filename, "out-file", 'o', "class label image based on "
                          "merging local labels", CCmdOptionFlags::output, &image3dio ));
       opts.add(make_opt( out_filename2, "out-global-crisp", 'G', "class label image based on "
                          "global segmentation", CCmdOptionFlags::output, &image3dio ));
       opts.add(make_opt( cls_filename, "class-prob", 'C', "class probability image file, filetype "
                          "must support floating point multi-frame images", CCmdOptionFlags::output, &image3dio ));
       opts.set_group("Parameters");
       opts.add(make_opt( blocksize, EParameterBounds::bf_min_closed, {3},
                          "grid-spacing", 'g', "Spacing of the grid used to modulate "
                          "the intensity inhomogeneities"));
       opts.add(make_opt( class_center_initializer, "kmeans:nc=3",
                          "cmeans", 'c', "c-means initializer"));
       opts.add(make_opt( cmeans_epsilon, EParameterBounds::bf_min_open,
       {0.0}, "c-means-epsilon", 'e', "c-means breaking condition for update tolerance"));
       opts.add(make_opt( rel_cluster_threshold, EParameterBounds::bf_min_closed | EParameterBounds::bf_max_open,
       {0.0, 1.0}, "relative-cluster-threshold", 't', "threshhold to ignore classes when initializing"
       " the local cmeans from the global one."));
       opts.add(make_opt(ignore_partition_with_background, "ignore-background", 'B',
                         "Don't take background probablities into account when desiding whether classes are to be ignored"));
       opts.add(make_opt(class_label_thresh,
                         EParameterBounds::bf_min_closed | EParameterBounds::bf_max_closed,
       {0.0f, 1.0f},
       "label-threshold", 'L',
       "for values <= 0.5: create segmentation based on highest class probability, "
       "labels start at 0. For values >0.5: create labels only for voxels with a "
       "class probability higher than the given value, labels start at 1 and voxels "
       "without an according class probability are set to 0; this output is suitable "
       "for the seeded watershed filter."));

       if (opts.parse(argc, argv) != CCmdOptionList::hr_no)
              return EXIT_SUCCESS;

       if (out_filename.empty() && out_filename2.empty()) {
              throw invalid_argument("You must specify at least one output file");
       }

       auto in_image = load_image3d(in_filename);
       FRunHistogram global_histogram;
       mia::accumulate(global_histogram, *in_image);
       CMeans globale_cmeans(cmeans_epsilon, class_center_initializer);
       auto gh = global_histogram.get_histogram();
       CMeans::DVector global_class_centers;
       auto global_sparse_probmap = globale_cmeans.run(gh, global_class_centers, false);
       cvinfo() << "Histogram range: [" << gh[0].first << ", " << gh[gh.size() - 1].first << "]\n";
       cvmsg() << "Global class centers: " << global_class_centers << "\n";
       cvinfo() << "Probmap size = " << global_sparse_probmap.size()
                << " weight number " << global_sparse_probmap[0].second.size() << "\n";
       const unsigned n_classes = global_class_centers.size();
       // need the normalized class centers
       map<int, unsigned> segmap;
       for_each(global_sparse_probmap.begin(), global_sparse_probmap.end(),
       [&segmap](const CMeans::SparseProbmap::value_type & x) {
              int c = 0;
              float max_prob = 0.0f;

              for (unsigned i = 0; i < x.second.size(); ++i) {
                     auto f = x.second[i];

                     if (f > max_prob) {
                            max_prob = f;
                            c = i;
                     };
              }

              segmap[x.first] = c;
       });
       FLocalCMeans::Probmap global_probmap;

       for (auto k : global_sparse_probmap) {
              global_probmap[k.first] = k.second;
       };

       if (!out_filename2.empty()) {
              FCrispSeg cs(segmap);
              auto crisp_global_seg = mia::filter(cs, *in_image);

              if (!save_image (out_filename2, crisp_global_seg)) {
                     cverr() << "Unable to save to '" << out_filename2 << "'\n";
              }
       }

       int  nx = (in_image->get_size().x + blocksize - 1) / blocksize + 1;
       int  ny = (in_image->get_size().y + blocksize - 1) / blocksize + 1;
       int  nz = (in_image->get_size().z + blocksize - 1) / blocksize + 1;
       int  start_x = - static_cast<int>(nx * blocksize - in_image->get_size().x) / 2;
       int  start_y = - static_cast<int>(ny * blocksize - in_image->get_size().y) / 2;
       int  start_z = - static_cast<int>(nz * blocksize - in_image->get_size().z) / 2;
       cvdebug() << "Start at " << start_x << ", " << start_y << ", " << start_z << "\n";
       int max_threads = CMaxTasks::get_max_tasks();
       assert(max_threads > 0);
       CMutex current_probbuffer_mutex;
       int current_probbuffer = 0;
       vector<ProtectedProbBuffer> prob_buffers(max_threads,
                     ProtectedProbBuffer(global_class_centers.size(),
                                         in_image->get_size()));
       auto block_runner = [&](const C1DParallelRange & z_range) -> void {
              CThreadMsgStream msg_stream;
              current_probbuffer_mutex.lock();
              int my_prob_buffer = current_probbuffer++;

              if (current_probbuffer >= max_threads)
                     current_probbuffer = 0;

              current_probbuffer_mutex.unlock();

              for (int  i = z_range.begin(); i < z_range.end(); ++i)
              {
                     int iz_base = start_z + i * blocksize;
                     unsigned iz = iz_base < 0 ? 0 : iz_base;

                     /* Work around the case that the image size (k*blocksize+1) */
                     if (iz >= in_image->get_size().z)
                            break;

                     unsigned iz_end = iz_base +  2 * blocksize;

                     if (iz_end > in_image->get_size().z)
                            iz_end = in_image->get_size().z;

                     cvmsg() << "Run slices " << iz_base << " - " <<  iz_end
                             << " with buffer " << my_prob_buffer << "\n";

                     for (int  iy_base = start_y; iy_base < (int)in_image->get_size().y; iy_base +=  blocksize) {
                            unsigned iy = iy_base < 0 ? 0 : iy_base;
                            unsigned iy_end = iy_base +  2 * blocksize;

                            if (iy_end > in_image->get_size().y)
                                   iy_end = in_image->get_size().y;

                            for (int ix_base = start_x; ix_base < (int)in_image->get_size().x; ix_base +=  blocksize) {
                                   unsigned ix = ix_base < 0 ? 0 : ix_base;
                                   unsigned ix_end = ix_base +  2 * blocksize;

                                   if (ix_end > in_image->get_size().x)
                                          ix_end = in_image->get_size().x;

                                   FLocalCMeans lcm(cmeans_epsilon, global_class_centers,
                                                    C3DBounds(ix, iy, iz),
                                                    C3DBounds(ix_end, iy_end, iz_end),
                                                    global_probmap,
                                                    rel_cluster_threshold,
                                                    segmap,
                                                    prob_buffers[my_prob_buffer],
                                                    !ignore_partition_with_background);
                                   mia::accumulate(lcm, *in_image);
                            }
                     }
              }

       };
       pfor(C1DParallelRange(0, nz, 1), block_runner);

       // sum the probabilities
       for (unsigned i = 1; i < prob_buffers.size(); ++i) {
              for (unsigned c = 0; c < n_classes; ++c) {
                     transform(prob_buffers[i].probmap[c].begin(), prob_buffers[i].probmap[c].end(),
                               prob_buffers[0].probmap[c].begin(), prob_buffers[0].probmap[c].begin(),
                     [](float x, float y) {
                            return x + y;
                     });
              }
       }

       // normalize probability images
       vector<C3DFDatafield>& prob_buffer = prob_buffers[0].probmap;
       C3DFImage sum(prob_buffer[0]);

       for (unsigned c = 1; c < n_classes; ++c) {
              transform(sum.begin(), sum.end(), prob_buffer[c].begin(), sum.begin(),
              [](float x, float y) {
                     return x + y;
              });
       }

       for (unsigned c = 0; c < n_classes; ++c) {
              transform(sum.begin(), sum.end(), prob_buffer[c].begin(), prob_buffer[c].begin(),
              [](float s, float p) {
                     return p / s;
              });
       }

       if (!cls_filename.empty()) {
              C3DImageIOPluginHandler::Instance::Data classes;

              for (unsigned c = 0; c < n_classes; ++c)
                     classes.push_back(make_shared<C3DFImage>(prob_buffer[c]));

              if (!C3DImageIOPluginHandler::instance().save(cls_filename, classes))
                     cverr() << "Error writing class images to '" << cls_filename << "'\n";
       }

       // now for each pixel we have a probability sum that should take inhomogeinities
       // into account
       C3DUBImage out_image(in_image->get_size(), *in_image);
       fill(out_image.begin(), out_image.end(), 0);

       if (class_label_thresh <= 0.5f) {
              for (unsigned c = 1; c < n_classes; ++c) {
                     auto iout = out_image.begin();
                     auto eout = out_image.end();
                     auto itest = prob_buffer[0].begin();
                     auto iprob = prob_buffer[c].begin();

                     while ( iout != eout ) {
                            if (*itest < *iprob) {
                                   *itest = *iprob;
                                   *iout = c;
                            }

                            ++iout;
                            ++itest;
                            ++iprob;
                     }
              }
       } else {
              for (unsigned c = 0; c < n_classes; ++c) {
                     auto iout = out_image.begin();
                     auto eout = out_image.end();
                     auto iprob = prob_buffer[c].begin();

                     while ( iout != eout ) {
                            if (class_label_thresh < *iprob) {
                                   *iout = c + 1;
                            }

                            ++iout;
                            ++iprob;
                     }
              }
       }

       return save_image(out_filename, out_image) ? EXIT_SUCCESS : EXIT_FAILURE;
}


ProtectedProbBuffer::ProtectedProbBuffer(int n_classes, const C3DBounds& size):
       probmap(n_classes, C3DFDatafield(size))
{
}

ProtectedProbBuffer::ProtectedProbBuffer(const ProtectedProbBuffer& orig):
       probmap(orig.probmap)
{
}

template <typename T>
void FRunHistogram::operator()(const T3DImage<T>& image)
{
       m_sparse_histogram(image.begin(), image.end());
}

CSparseHistogram::Compressed FRunHistogram::get_histogram() const
{
       return m_sparse_histogram.get_compressed_histogram();
}


FLocalCMeans::FLocalCMeans(float epsilon, const vector<double>& global_class_centers,
                           const C3DBounds& start, const C3DBounds& end,
                           const Probmap& global_probmap,
                           float rel_cluster_threshold,
                           const map<int, unsigned>& segmap,
                           ProtectedProbBuffer& prob_buffer,
                           bool partition_with_background):
       m_epsilon(epsilon),
       m_global_class_centers(global_class_centers),
       m_start(start),
       m_end(end),
       m_global_probmap(global_probmap),
       m_rel_cluster_threshold(rel_cluster_threshold),
       m_segmap(segmap),
       m_prob_buffer(prob_buffer),
       m_count((m_end - m_start).product()),
       m_partition_with_background(partition_with_background)
{
}



template <typename T>
void FLocalCMeans::operator()(const T3DImage<T>& image)
{
       cvinfo() << "Run subrange [" << m_start << "]-[" << m_end << "]\n";
       // obtain the histogram for the current patch
       CSparseHistogram histogram;
       histogram(image.begin_range(m_start, m_end),
                 image.end_range(m_start, m_end));
       auto ch = histogram.get_compressed_histogram();
       // calculate the class avaliability in the patch
       vector<double> partition(m_global_class_centers.size());

       for (auto idx : ch) {
              const double n = idx.second;
              auto v = m_global_probmap.at(idx.first);
              transform(partition.begin(), partition.end(), v.begin(),
              partition.begin(), [n](double p, double value) {
                     return p + n * value;
              });
       }

       // don't count background class in partition
       int start_class = m_partition_with_background ? 0 : 1;
       auto part_thresh = std::accumulate(partition.begin() + start_class, partition.end(), 0.0) * m_rel_cluster_threshold;
       cvinfo() << "Partition=" << partition
                << ", thresh="  << part_thresh
                << "\n";
       // select the classes that should be used further on
       vector<double> retained_class_centers;
       vector<unsigned> used_classed;

       for (unsigned i = 0; i < partition.size(); ++i) {
              if (partition[i] >= part_thresh) {
                     retained_class_centers.push_back(m_global_class_centers[i]);
                     used_classed.push_back(i);
              }
       }

       // prepare linear interpolation summing
       auto center = C3DFVector((m_end + m_start)) / 2.0f;
       auto max_distance = C3DFVector((m_end - m_start)) / 2.0f;

       if (retained_class_centers.size() > 1)  {
              ostringstream cci_descr;
              cci_descr << "predefined:cc=[" << retained_class_centers << "]";
              cvinfo() << "Initializing local cmeans with '" << cci_descr.str()
                       << " for retained classes " << used_classed << "'\n";
              auto cci = CMeansInitializerPluginHandler::instance().produce(cci_descr.str());
              // remove data that was globally assigned to now unused class
              CSparseHistogram::Compressed cleaned_histogram;
              cleaned_histogram.reserve(ch.size());

              // copy used intensities
              for (auto c : used_classed) {
                     for (auto ich : ch) {
                            if ( m_segmap.at(ich.first) == c) {
                                   cleaned_histogram.push_back(ich);
                            }
                     }
              }

              // evluate the local c-means classification
              CMeans local_cmeans(m_epsilon, cci);
              auto local_map = local_cmeans.run(cleaned_histogram,  retained_class_centers);
              // create a lookup map intensity -> probability vector
              map<unsigned short, CMeans::DVector> mapper;

              for (auto i : local_map) {
                     mapper[i.first] = i.second;
              }

              // now add the new probabilities to the global maps.
              auto ii = image.begin_range(m_start, m_end);
              auto ie = image.end_range(m_start, m_end);
              CScopedLock prob_lock(m_prob_buffer.mutex);

              while (ii != ie) {
                     auto probs = mapper.find(*ii);
                     auto delta = (C3DFVector(ii.pos()) - center) / max_distance;
                     float lin_scale = (1.0 - std::fabs(delta.x)) * (1.0 - std::fabs(delta.y));

                     if (probs != mapper.end()) {
                            for (unsigned c = 0; c < used_classed.size();  ++c) {
                                   m_prob_buffer.probmap[used_classed[c]](ii.pos()) += lin_scale * probs->second[c];
                            }
                     } else { // not in local map: retain global probabilities
                            auto v = m_global_probmap.at(*ii);

                            for (unsigned c = 0; c < v.size();  ++c) {
                                   m_prob_buffer.probmap[c](ii.pos()) += lin_scale * v[c];
                            }
                     }

                     ++ii;
              }
       } else { // only one class retained, add 1.0 to probabilities, linearly smoothed
              cvinfo() << "Only one class used:" << used_classed[0] << "\n";
              auto ii = m_prob_buffer.probmap[used_classed[0]].begin_range(m_start, m_end);
              auto ie = m_prob_buffer.probmap[used_classed[0]].end_range(m_start, m_end);
              CScopedLock prob_lock(m_prob_buffer.mutex);

              while (ii != ie)  {
                     auto delta = (C3DFVector(ii.pos()) - center) / max_distance;
                     *ii += (1.0 - std::fabs(delta.x)) * (1.0 - std::fabs(delta.y)); ;
                     ++ii;
              }
       }

       cvinfo() << "Done subrange [" << m_start << "]-[" << m_end << "]\n";
}

#include <mia/internal/main.hh>
MIA_MAIN(do_main);

