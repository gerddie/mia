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


#include <queue>
#include <stdexcept>

#include <boost/type_traits.hpp>

#include <mia/2d/filter.hh>
#include <libmia/filter.hh>

#include <libmia/probmapio.hh>

namespace ws_2dimage_filter
{
NS_MIA_USE;
using namespace std;



static char const *plugin_name = "ws";
static const CStringOption param_map("map", "seed class map", "");
static const CIntOption param_width("w", "window width for variation evaluation", 1, 1, 1000);
static const CFloatOption param_thresh("thresh", "threshold for seed probability", 0.9, 0.5, 1.0);
static const CFloatOption param_tol("tol", "tolerance for watershed", 0.8, 0.1, 1.2);
static const CFloatOption param_steep("p", "decision parameter whether fuzzy segmentation or neighborhood should be used", 1.5, 1, 5.0);

static const C2DUBImage::value_type undefined = numeric_limits<C2DUBImage::value_type>::max();

class C2DWatershedFilter: public C2DFilter
{
       int m_width;
       float m_thresh;
       float m_tol;
       float m_steep;
       CProbabilityVector m_pv;
public:
       C2DWatershedFilter(int hwidth, float thresh, float steep, float tol, const CProbabilityVector& pv):
              m_width(hwidth),
              m_thresh(thresh),
              m_tol(tol),
              m_steep(steep),
              m_pv(pv)
       {
       }

       template <class Data2D>
       typename C2DWatershedFilter::result_type operator () (const Data2D& data) const ;

private:
       template <class Data2D>
       C2DFImage get_var_image(const Data2D& data, float& max_val) const;
};


class C2DWatershedFilterImageFilter: public C2DImageFilterBase
{
       C2DWatershedFilter m_filter;
public:
       C2DWatershedFilterImageFilter(int hwidth, float thresh, float steep, float tol, const CProbabilityVector& pv);

       virtual P2DImage do_filter(const C2DImage& image) const;
};


class C2DWatershedFilterImageFilterFactory: public C2DFilterPlugin
{
public:
       C2DWatershedFilterImageFilterFactory();
       virtual C2DFilter *create(const CParsedOptions& options) const;
       virtual const string do_get_descr()const;
};

template <class Data2D>
C2DFImage C2DWatershedFilter::get_var_image(const Data2D& data, float& max_val) const
{
       max_val = 0.0f;
       C2DFImage result(data.get_size());
       C2DFImage::iterator r = result.begin();

       for (int y = 0; y < (int)data.get_size().y; ++y)
              for (int x = 0; x < (int)data.get_size().x; ++x, ++r) {
                     *r = 0.0;
                     int n = 0;
                     float sum = 0.0f;
                     float sum2 = 0.0f;

                     for (int iy = max(0, y - m_width); iy < min (y + m_width, (int)data.get_size().y); ++iy)
                            for (int ix = max(0, x - m_width); ix < min (x + m_width, (int)data.get_size().x); ++ix) {
                                   float val = data(ix, iy);
                                   ++n;
                                   sum += val;
                                   sum2 += val * val;
                            }

                     if (n > 1)
                            *r = sqrt((sum2 - sum * sum / n) / (n - 1));

                     if (*r > max_val)
                            max_val = *r;
              }

       return result;
}

inline void fill(const C2DBounds& p, C2DUBImage& result, queue< C2DBounds >& seedpoints,  const C2DFImage& var_image,
                 float var, unsigned char cclass, queue< C2DBounds >& backlog)
{
       if (!(p.x < result.get_size().x && p.y < result.get_size().y))
              return;

       if ( result(p) == undefined) {
              if (var_image(p) >= var) {
                     result(p) = cclass;
                     seedpoints.push(p);
              } else if (cclass != 0)
                     backlog.push(p);
       }
}

template <typename T, bool is_integral>
struct  dispatch_filter {
       static P2DImage apply(const T2DImage<T>& data, const CProbabilityVector& pv, float steep, float thresh, float tol, const C2DFImage& var_image)
       {
              throw invalid_argument("Non pixel data type not supported");
              return P2DImage();
       }
};


template <typename T>
struct  dispatch_filter<T, true> {
       static P2DImage apply(const T2DImage<T>& data, const CProbabilityVector& pv, float steep, float thresh, float tol, const C2DFImage& var_image)
       {
              cvdebug() << "Evaluate seed points\n";
              // create the target image
              C2DUBImage *result = new C2DUBImage(data.get_size(), data);
              C2DUBImage::iterator r = result->begin();
              typename T2DImage<T>::const_iterator i = data.begin();
              queue< C2DBounds > seedpoints;
              queue< C2DBounds > backlog;
              const size_t n_classes = pv.size();
              vector<long> k(pv.size() + 1);
              fill(k.begin(), k.end(), 0);

              // find all points above the given probability threshold and use them as seed points
              for (size_t y = 0; y < data.get_size().y; ++y)
                     for (size_t x = 0; x < data.get_size().x; ++x, ++r, ++i) {
                            *r = undefined;
                            ++k[n_classes];
                            size_t ival = *i;

                            if ( ival < pv[0].size()) {
                                   for (size_t l = 0; l < pv.size(); ++l)
                                          if (pv[l][ival] > thresh) {
                                                 *r = l;
                                                 ++k[l];
                                                 --k[n_classes];
                                                 seedpoints.push(C2DBounds(x, y));
                                                 break;
                                          }
                            }
                     }

              // target image contains a seed segmentation

              if (cverb.show_debug()) {
                     cvdebug() << seedpoints.size() << " initial seed points\n";
                     cvdebug() << "seed:";

                     for (size_t i = 0; i < k.size(); ++i)
                            cverb << k[i] << " ";

                     cverb << "\n";
              }

// create the variation image
              vector<int> counter(pv.size());

              while (!seedpoints.empty()) {
                     // do the watershead segmentation
                     while ( !seedpoints.empty() ) {
                            C2DBounds p = seedpoints.front();
                            seedpoints.pop();
                            float var = tol * var_image(p);
                            unsigned char cclass = (*result)(p);
                            fill(C2DBounds(p.x - 1, p.y), *result, seedpoints, var_image, var, cclass, backlog);
                            fill(C2DBounds(p.x + 1, p.y), *result, seedpoints, var_image, var, cclass, backlog);
                            fill(C2DBounds(p.x, p.y - 1), *result, seedpoints, var_image, var, cclass, backlog);
                            fill(C2DBounds(p.x, p.y + 1), *result, seedpoints, var_image, var, cclass, backlog);
                     }

                     // read all pixels that failed to be assigned to a class
                     cvdebug() << "proceed with backlog\n";

                     while ( !backlog.empty() ) {
                            C2DBounds p = backlog.front();
                            backlog.pop();
                            fill(counter.begin(), counter.end(), 0);

                            if ( (*result)(p) != undefined)
                                   continue;

                            // Check which classes are already assigned in the neighborhood
                            if (p.x > 1) {
                                   unsigned char r = (*result)(p.x - 1, p.y);

                                   if ( r != undefined )
                                          ++counter[r];
                            }

                            if (p.x < data.get_size().x - 1 ) {
                                   unsigned char r = (*result)(p.x + 1, p.y);

                                   if ( r != undefined )
                                          ++counter[r];
                            }

                            if (p.y > 1) {
                                   unsigned char r = (*result)(p.x, p.y - 1);

                                   if ( r != undefined )
                                          ++counter[r];
                            }

                            if (p.y < data.get_size().y - 1 ) {
                                   unsigned char r = (*result)(p.x, p.y + 1);

                                   if ( r != undefined )
                                          ++counter[r];
                            }

                            // select the class that was most assigned around
                            int best = 0;
                            int best_val = counter[0];

                            for (size_t i = 1; i < counter.size(); ++i) {
                                   if (best_val < counter[i]) {
                                          best_val = counter[i];
                                          best = i;
                                   }
                            }

                            // find the highest probability from the fuzzy segmentation
                            typename T2DImage<T>::value_type v = data(p);
                            float best_prob = pv[0][v];
                            int best_class = 0;

                            for (size_t i = 1; i < pv.size(); ++i) {
                                   if (best_prob < pv[i][(int)v]) {
                                          best_prob = pv[i][(int)v];
                                          best_class = i;
                                   }
                            }

                            // assign the class based on the probability map if this probability
                            // is significantly larger then the probability based on the already
                            // assigned neighbors
                            if (best_prob > steep * pv[best][(int)v])
                                   (*result)(p) = best_class;
                            else
                                   (*result)(p) = best;

                            // store the seed point for the next run
                            seedpoints.push(p);
                     } // while ( !backlog.empty() )
              } // while (!seedpoints.empty())

              return P2DImage(result);
       }
};




template <class Data2D>
typename C2DWatershedFilter::result_type C2DWatershedFilter::operator () (const Data2D& data) const
{
       const bool is_integral = ::boost::is_integral<typename Data2D::value_type>::value;
       float max_val = 0.0f;
       C2DFImage var_image = get_var_image(data, max_val);
       return dispatch_filter<typename Data2D::value_type, is_integral>::apply(data, m_pv, m_steep, m_thresh, m_tol, var_image);
}

C2DWatershedFilterImageFilter::C2DWatershedFilterImageFilter(int hwidth, float thresh, float steep, float tol, const CProbabilityVector& pv):
       m_filter(hwidth, thresh, steep, tol, pv)
{
}

P2DImage C2DWatershedFilterImageFilter::do_filter(const C2DImage& image) const
{
       return wrap_filter(m_filter, image);
}

C2DWatershedFilterImageFilterFactory::C2DWatershedFilterImageFilterFactory():
       C2DFilterPlugin(plugin_name)
{
       add_help(param_width);
}

C2DFilter *C2DWatershedFilterImageFilterFactory::create(const CParsedOptions& options) const
{
       string map = param_map.get_value(options);
       float thresh = param_thresh.get_value(options);
       float steep = param_steep.get_value(options);
       float tol = param_tol.get_value(options);
       int hwidth = param_width.get_value(options);
       CProbabilityVector pv = load_probability_map(map);

       if (pv.empty())
              throw invalid_argument(string("Unable to load probability map from ") + map);

       return new C2DWatershedFilterImageFilter(hwidth, thresh, steep, tol, pv);
}

const string C2DWatershedFilterImageFilterFactory::do_get_descr()const
{
       return "2D image watershead filter with seed";
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
       return new C2DWatershedFilterImageFilterFactory();
}

} // end namespace ws_2dimage_filter
