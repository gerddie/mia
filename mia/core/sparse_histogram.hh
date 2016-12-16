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


#include <mia/core/filter.hh>
#include <vector> 
#include <type_traits>
#include <limits> 

NS_MIA_BEGIN

/**
   \ingroup utils 
    
   \brief A sparse histogram 

   This class implements a sparse histogram. As input only 
   8 and 16 bit (un)signed data is allowed. 
*/

class EXPORT_CORE CSparseHistogram : public TFilter<size_t> {
        
public: 
	typedef std::vector<std::pair<int, unsigned long>> Compressed; 

	CSparseHistogram();

        /**
           Fees pixels to the histogram
           \tparam InIterator input iterator, must be a forward iterator 
           \param begin start of range 
           \param end end of range 
           \returns number of added pixels
        */
        template <typename InIterator>
        size_t operator ()(InIterator begin, InIterator end); 

	template <typename Image>
	size_t operator ()(const Image& image) {
		return (*this)(image.begin(), image.end()); 
	}
	
        /**
           \returns the histogram as a vector of <value, count> pairs 
         */
        Compressed get_compressed_histogram()const; 
 private: 
        std::vector<uint64_t> m_histogram; 
        int64_t m_shift; 
        EPixelType m_pixeltype; 
}; 

EXPORT_CORE std::ostream& operator << (std::ostream& os, const std::pair<short, uint64_t>& pair); 

//   Implementation

///  @cond INTERNAL

template <typename InIterator, bool sig> 
struct dispatch_by_pixeltype {
        static size_t  apply(InIterator MIA_PARAM_UNUSED(begin), InIterator MIA_PARAM_UNUSED(end),
                             std::vector<uint64_t>& MIA_PARAM_UNUSED(histogram)){
                throw std::invalid_argument("Input pixel type not supported"); 
        }
}; 

template <typename InIterator> 
struct dispatch_by_pixeltype<InIterator, false> {
        static size_t apply(InIterator begin, InIterator end, std::vector<uint64_t>& histogram){
                size_t n = 0; 
                while ( begin != end) {
                        ++histogram[*begin];
                        ++begin;
                        ++n; 
                }
                return n; 
        }
}; 

template <typename InIterator> 
struct dispatch_by_pixeltype<InIterator, true> {
        static size_t apply(InIterator begin, InIterator end, std::vector<uint64_t>& histogram){
                typedef typename InIterator::value_type in_pixels;
		int64_t shift = -std::numeric_limits<in_pixels>::min();
                size_t n = 0; 
                while ( begin != end) {
                        ++histogram[*begin + shift];
                        ++begin;
                        ++n; 
                }
                return n; 
        }
}; 


template <typename InIterator>
size_t CSparseHistogram::operator ()(InIterator begin, InIterator end)
{
        typedef typename InIterator::value_type in_pixeltype; 
        if (m_pixeltype ==it_none) {
                m_pixeltype = pixel_type<in_pixeltype>::value;
		m_shift = -std::numeric_limits<in_pixeltype>::min(); 
                switch (m_pixeltype) {
                case it_sbyte:
                case it_ubyte:
                        m_histogram.resize(256);
                        break; 
                case it_sshort:
                case it_ushort:
                        m_histogram.resize(65536);
                        break; 
                default:
                        throw create_exception<std::invalid_argument>("Input pixel type '",
                                                                 CPixelTypeDict.get_name(m_pixeltype),
                                                                 "' not supported."); 
                }
                
        } else if (m_pixeltype != pixel_type<in_pixeltype>::value){
                throw create_exception<std::invalid_argument>("Input pixels not of consisted type, started with ",
                                                         CPixelTypeDict.get_name(m_pixeltype), ", but got now ",
                                                         CPixelTypeDict.get_name(pixel_type<in_pixeltype>::value)); 
        }

        const bool is_signed = std::is_signed<in_pixeltype>::value; 

        size_t n = 0; 
        n += dispatch_by_pixeltype<InIterator, is_signed>::apply(begin, end, m_histogram);
        return n; 
}

/// @endcond

NS_MIA_END
