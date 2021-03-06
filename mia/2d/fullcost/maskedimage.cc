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

#include <mia/2d/fullcost/maskedimage.hh>
#include <mia/2d/filter.hh>

NS_MIA_BEGIN

using namespace std;

C2DMaskedImageFullCost::C2DMaskedImageFullCost(const std::string& src,
              const std::string& ref,
              const std::string& src_mask,
              const std::string& ref_mask,
              P2DMaskedImageCost cost,
              double weight):
       C2DFullCost(weight),
       m_src_key(C2DImageIOPluginHandler::instance().load_to_pool(src)),
       m_ref_key(C2DImageIOPluginHandler::instance().load_to_pool(ref)),
       m_ref_mask_bit(nullptr),
       m_ref_mask_scaled_bit(nullptr),
       m_cost_kernel(cost)
{
       assert(m_cost_kernel);

       if (!src_mask.empty())
              m_src_mask_key = C2DImageIOPluginHandler::instance().load_to_pool(src_mask);

       if (!ref_mask.empty())
              m_ref_mask_key = C2DImageIOPluginHandler::instance().load_to_pool(ref_mask);

       if (src_mask.empty() && ref_mask.empty()) {
              throw invalid_argument("C2DMaskedImageFullCost: No masks give, you should used "
                                     "the plain image cost instead");
       }
}

bool C2DMaskedImageFullCost::do_has(const char *property) const
{
       return m_cost_kernel->has(property);
}

/*
   This functions combines the moving and the fixed masks to a singular mask for cost function evaluation.
   - if no transformation is given then the original masks will be or-combined. In this case
     the masks must be of the same size.
   - if a transformation is given then the moving mask is transformed and then combined with
     the reference image mask
*/
P2DImage C2DMaskedImageFullCost::get_combined_mask(const C2DTransformation *t, C2DBitImage **combined_mask) const
{
       P2DImage temp_mask;
       *combined_mask = m_ref_mask_scaled_bit;

       if (m_src_mask_scaled) {
              cvdebug() << "Maskedimage: Start with moving mask\n";

              if (t)
                     temp_mask = (*t)(*m_src_mask_scaled);
              else
                     temp_mask.reset(m_src_mask_scaled->clone());

              *combined_mask = static_cast<C2DBitImage *>(temp_mask.get());
              assert(*combined_mask);

              if (m_ref_mask_scaled_bit) {
                     cvdebug() << "Maskedimage: Combine with reference mask\n";

                     if ( (*combined_mask)->get_size() != m_ref_mask_scaled_bit->get_size()) {
                            assert(!t && "Bug: The transformation should have created a mask of "
                                   "the same size like the reference image mask");
                            throw create_exception<invalid_argument>
                            ("C2DMaskedImageFullCost: the masks to be combined are "
                             "of different size: moving=[", (*combined_mask)->get_size(),
                             "], reference=[", m_ref_mask_scaled_bit->get_size(), "]");
                     }

                     // A parameter should define how the masks are combined
                     transform((*combined_mask)->begin(), (*combined_mask)->end(),
                               m_ref_mask_scaled_bit->begin(), (*combined_mask)->begin(),
                     [](bool a, bool b) {
                            return a || b;
                     });
                     // here a penalty could be added e.g. to ensure that the moving mask is
                     // always inside the fixed mask. However, it is difficult to evaluate a
                     // force for this.
              }
       } else
              cvdebug() << "Maskedimage: Only have reference mask\n";

       return temp_mask;
}

double C2DMaskedImageFullCost::do_value(const C2DTransformation& t) const
{
       TRACE_FUNCTION;
       assert(m_src_scaled  && "Bug: you must call 'reinit()' before calling value(transform)");
       P2DImage temp  = t(*m_src_scaled);
       C2DBitImage *temp_mask_bit = nullptr;
       P2DImage temp_mask  = get_combined_mask(&t, &temp_mask_bit);
       assert(temp_mask_bit);
       const double result = m_cost_kernel->value(*temp, *temp_mask_bit);
       cvdebug() << "C2DMaskedImageFullCost::value = " << result << "\n";
       return result;
}

double C2DMaskedImageFullCost::do_value() const
{
       TRACE_FUNCTION;
       assert(m_src_scaled  && "Bug: you must call 'reinit()' before calling value()");
       C2DBitImage *temp_mask_bit = nullptr;
       P2DImage temp_mask  = get_combined_mask(nullptr, &temp_mask_bit);
       assert(temp_mask_bit);
       const double result = m_cost_kernel->value(*m_src_scaled, *temp_mask_bit);
       cvdebug() << "C2DMaskedImageFullCost::value = " << result << "\n";
       return result;
}


double C2DMaskedImageFullCost::do_evaluate(const C2DTransformation& t, CDoubleVector& gradient) const
{
       TRACE_FUNCTION;
       assert(m_src_scaled  && "Bug: you must call 'reinit()' before calling evauate(...)");
       P2DImage temp  = t(*m_src_scaled);
       C2DBitImage *temp_mask_bit = nullptr;
       P2DImage temp_mask  = get_combined_mask(&t, &temp_mask_bit);
       assert(temp_mask_bit);
       C2DFVectorfield force(get_current_size());
       double result = m_cost_kernel->evaluate_force(*temp, *temp_mask_bit, force);
       t.translate(force, gradient);
       return result;
}

void C2DMaskedImageFullCost::do_set_size()
{
       TRACE_FUNCTION;
       assert(m_src);
       assert(m_ref);

       if (m_src_scaled->get_size() != get_current_size() ||
           m_ref_scaled->get_size() != get_current_size()) {
              stringstream filter_descr;
              filter_descr << "scale:s=[" << get_current_size() << "]";
              auto scaler = C2DFilterPluginHandler::instance().produce(filter_descr.str());
              assert(scaler);
              cvdebug() << "C2DMaskedImageFullCost:scale images to " << get_current_size() <<
                        " using '" << filter_descr.str() << "'\n";
              m_src_scaled = scaler->filter(*m_src);
              m_ref_scaled = scaler->filter(*m_ref);
              m_cost_kernel->set_reference(*m_ref_scaled);

              if (m_src_mask)  {
                     m_src_mask_scaled  = scaler->filter(*m_src_mask);
              }

              if (m_ref_mask)  {
                     m_ref_mask_scaled  = scaler->filter(*m_ref_mask);
                     m_ref_mask_scaled_bit = static_cast<C2DBitImage *>(m_ref_mask_scaled.get());
              }

              assert ((m_src_mask_scaled || m_ref_mask_scaled) &&
                      "Bug: At this point at least one mask should be available");
       }
}

bool C2DMaskedImageFullCost::do_get_full_size(C2DBounds& size) const
{
       TRACE_FUNCTION;
       assert(m_src && "Hint: call 'reinit()' before calling get_full_size()");

       if (size == C2DBounds::_0) {
              size = m_src->get_size();
              return true;
       } else
              return 	size == m_src->get_size();
}

void C2DMaskedImageFullCost::do_reinit()
{
       TRACE_FUNCTION;
       //cvmsg() << "C2DMaskedImageFullCost: read " << m_src_key << " and " << m_ref_key << "\n";
       m_src_scaled = m_src = get_from_pool(m_src_key);
       m_ref_scaled = m_ref = get_from_pool(m_ref_key);

       if (m_src_mask_key.key_is_valid()) {
              m_src_mask = get_from_pool(m_src_mask_key);

              if (m_src->get_size() != m_src_mask->get_size()) {
                     throw create_exception<runtime_error>("C2DMaskedImageFullCost: moving image has size [",
                                                           m_src->get_size(), "], but corresponding mask is of size [",
                                                           m_src_mask->get_size(), "]");
              }

              if (m_src_mask->get_pixel_type() != it_bit) {
                     // one could also add a binarize filter here, but  it's better to force
                     // the user to set the pixel type correctly
                     throw create_exception<invalid_argument>("C2DMaskedImageFullCost: moving mask image ",
                                   m_src_mask_key.get_key(),
                                   " must be binary");
              }

              m_src_mask_scaled = m_src_mask;
       }

       if (m_ref_mask_key.key_is_valid()) {
              m_ref_mask = get_from_pool(m_ref_mask_key);

              if (m_ref->get_size() != m_ref_mask->get_size()) {
                     throw create_exception<runtime_error>("C2DMaskedImageFullCost: reference image has size [",
                                                           m_src->get_size(), "], but corresponding mask is of size [",
                                                           m_src_mask->get_size(), "]");
              }

              m_ref_mask_scaled_bit = m_ref_mask_bit = dynamic_cast<C2DBitImage *>(m_ref_mask.get());

              if (!m_ref_mask_scaled_bit)  {
                     throw create_exception<invalid_argument>("C2DMaskedImageFullCost: reference mask image "
                                   "must be binary");
              }
       }

       if (m_src->get_size() != m_ref->get_size()) {
              cvinfo() << "C2DMaskedImageFullCost: registering moving image of size [" << m_src->get_size() << "] "
                       << "to reference image of size [" << m_ref->get_size() << "]\n";
       }

       if (m_src->get_pixel_size() != m_ref->get_pixel_size()) {
              cvwarn() << "C2DMaskedImageFullCost: moving and reference image are of different pixel dimensions."
                       << "unless you optimize a transformation that supports global scaling this might "
                       << "not be what you want to do\n";
       }

       m_cost_kernel->set_reference(*m_ref_scaled);
}

P2DImage C2DMaskedImageFullCost::get_from_pool(const C2DImageDataKey& key)
{
       C2DImageIOPlugin::PData in_image_list = key.get();

       if (!in_image_list || in_image_list->empty())
              throw invalid_argument("C2DMaskedImageFullCost: no image available in data pool");

       return (*in_image_list)[0];
}


// plugin implementation
class C2DMaskedImageFullCostPlugin: public C2DFullCostPlugin
{
public:
       C2DMaskedImageFullCostPlugin();
private:
       C2DFullCost *do_create(float weight) const;
       const std::string do_get_descr() const;
       std::string m_src_name;
       std::string m_ref_name;
       std::string m_src_mask_name;
       std::string m_ref_mask_name;

       P2DMaskedImageCost m_cost_kernel;
};

C2DMaskedImageFullCostPlugin::C2DMaskedImageFullCostPlugin():
       C2DFullCostPlugin("maskedimage"),
       m_src_name("src.@"),
       m_ref_name("ref.@")
{
       add_parameter("src", new CStringParameter(m_src_name, CCmdOptionFlags::input, "Study image", &C2DImageIOPluginHandler::instance()));
       add_parameter("ref", new CStringParameter(m_ref_name, CCmdOptionFlags::input, "Reference image", &C2DImageIOPluginHandler::instance()));
       add_parameter("src-mask", new CStringParameter(m_src_mask_name, CCmdOptionFlags::input,
                     "Study image mask (binary)", &C2DImageIOPluginHandler::instance()));
       add_parameter("ref-mask", new CStringParameter(m_ref_mask_name, CCmdOptionFlags::input,
                     "Reference image mask  (binary)", &C2DImageIOPluginHandler::instance()));
       add_parameter("cost", make_param(m_cost_kernel, "ssd", false, "Cost function kernel"));
}

C2DFullCost *C2DMaskedImageFullCostPlugin::do_create(float weight) const
{
       cvdebug() << "create C2DMaskedImageFullCostPlugin with weight= " << weight
                 << " src=" << m_src_name << " ref=" << m_ref_name
                 << " src-mask='" << m_src_mask_name << "' ref='" << m_ref_name
                 << "' cost=" << m_cost_kernel << "\n";
       return new C2DMaskedImageFullCost(m_src_name, m_ref_name,
                                         m_src_mask_name, m_ref_mask_name,
                                         m_cost_kernel, weight);
}

const std::string C2DMaskedImageFullCostPlugin::do_get_descr() const
{
       return "Generalized masked image similarity cost function that also handles multi-resolution processing. "
              "The provided masks should be densly filled regions in multi-resolution procesing because "
              "otherwise the mask information may get lost when downscaling the image. "
              "The reference mask and the transformed mask of the study image are combined by binary AND. "
              "The actual similarity measure is given es extra parameter.";
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
       return new C2DMaskedImageFullCostPlugin();
}

NS_MIA_END
