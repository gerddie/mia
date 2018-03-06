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

#ifndef mia_template_combiner_filter_hh
#define mia_template_combiner_filter_hh


#include <mia/template/combiner.hh>
#include <mia/core/iohandler.hh>



NS_MIA_BEGIN

template <typename Image>
class TImageCombinerFilter: public TDataFilter<Image>
{
public:
       TImageCombinerFilter(std::shared_ptr<TImageCombiner<Image>> combiner,
                            const std::string& other_image_file,  bool reverse);

       typename Image::Pointer do_filter(const Image& image) const;

       std::shared_ptr<TImageCombiner<Image>> m_combiner;
       std::string m_other_image;
       bool m_reverse;

};


template <class Image>
class TImageCombinerFilterPlugin: public TDataFilterPlugin<Image>
{
public:
       TImageCombinerFilterPlugin();
       virtual TDataFilter<Image> *do_create()const;
       virtual const std::string do_get_descr()const;
private:
       std::shared_ptr<TImageCombiner<Image>> m_combiner;
       std::string m_other_image;
       bool m_reverse;

};

template <typename Image>
TImageCombinerFilter<Image>::TImageCombinerFilter(std::shared_ptr<TImageCombiner<Image>> combiner,
              const std::string& other_image_file,  bool reverse):
       m_combiner(combiner),
       m_other_image(other_image_file),
       m_reverse(reverse)
{
}

template <typename Image>
typename Image::Pointer TImageCombinerFilter<Image>::do_filter(const Image& image) const
{
       auto other_image = load_image<typename Image::Pointer>(m_other_image);

       if (m_reverse)
              return m_combiner->combine(*other_image, image);
       else
              return m_combiner->combine(image, *other_image);
}


template <typename Image>
TImageCombinerFilterPlugin<Image>::TImageCombinerFilterPlugin():
       TDataFilterPlugin<Image>("combiner"),
       m_reverse(false)
{
       typedef typename IOHandler_of<Image>::type IOHandler;
       this->add_parameter("op", make_param(m_combiner, "",  true, "Image combiner to be applied to the images"));
       this->add_parameter("image", new CStringParameter(m_other_image, CCmdOptionFlags::required_input, "second image that is needed in the combiner",
                           &IOHandler::instance()));
       this->add_parameter("reverse", new CBoolParameter(m_reverse, false, "reverse the order in which the images passed to the combiner"));
}

template <typename Image>
TDataFilter<Image> *TImageCombinerFilterPlugin<Image>::do_create()const
{
       return new TImageCombinerFilter<Image>(m_combiner, m_other_image, m_reverse);
}

template <typename Image>
const std::string TImageCombinerFilterPlugin<Image>::do_get_descr()const
{
       return "Combine two images with the given combiner operator. if 'reverse' is set to false, the first "
              "operator is the image passed through the filter pipeline, and the second image is loaded "
              "from the file given with the 'image' parameter the moment the filter is run.";
}

NS_MIA_END

#endif
