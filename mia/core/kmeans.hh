/* -*- mona-c++  -*-
 *
 * Copyright (c) Madrid 2010
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PUcRPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifndef __mia_core_kmeans_hh
#define __mia_core_kmeans_hh
#include <vector>
#include <numeric>
#include <cmath>
#include <stdexcept>
#include <iomanip>
#include <limits>
#include <mia/core/defines.hh>
#include <mia/core/errormacro.hh>
#include <boost/concept/requires.hpp>
#include <boost/concept_check.hpp>

NS_MIA_BEGIN
/// helper function called by kmeans - don't call it directly 
template <typename InputIterator, typename OutputIterator> 
bool kmeans_step(InputIterator ibegin, InputIterator iend, OutputIterator obegin, 
		 std::vector<double>& classes, size_t l, int& biggest_class ) 
{
	cvdebug()<<  "kmeans enter: ";
	for (size_t i = 0; i <= l; ++i )
		cverb << std::setw(8) << classes[i]<< " ";  
	cverb << "\n"; 
	
	biggest_class = -1; 
	const double convLimit = 0.005;	// currently fixed
	std::vector<double> sums(classes.size()); 
	std::vector<size_t> count(classes.size()); 
	
	bool conv = false;
	int iter = 50; 
	
	while( iter-- && !conv) {

		sort(classes.begin(), classes.end()); 
		
		// assign closest cluster center
		OutputIterator ob = obegin; 
		for (InputIterator b = ibegin; b != iend; ++b, ++ob) {
			const double val = *b;
			double dmin = std::numeric_limits<double>::max();
			int c = 0;
			for (size_t i = 0; i <= l; i++) {
				double d = fabs (val - classes[i]);
				if (d < dmin) {
					dmin = d;
					c = i;
				};
			};
			*ob = c; 
			
			++count[c];
			sums[c] += val;
		};
		
		// recompute cluster centers
		conv = true;
		size_t max_count = 0; 
		for (size_t i = 0; i <= l; i++) {
			if (count[i]) {
				double a = sums[i] / count[i];
				if (a  && fabs ((a - classes[i]) / a) > convLimit)
					conv = false;
				classes[i] = a;
				
				if (max_count < count[i]) {
					max_count  = count[i]; 
					biggest_class = i; 
				}
			} else { // if a class is empty move it closer to neighbour 
				if (i == 0)
					classes[i] = (classes[i] + classes[i + 1]) / 2.0; 
				else
					classes[i] = (classes[i] + classes[i - 1])  / 2.0; 
				conv = false;
			}
			sums[i] = 0;
			count[i] = 0;
		};
	};

	cvinfo()<<  "kmeans: " << l + 1 << " classes " << 50 - iter << "  iterations";
	for (size_t i = 0; i <= l; ++i )
		cverb << std::setw(8) << classes[i]<< " ";  
	cverb << "\n"; 
	
	return conv; 
}


/**
   \fn template <typename InputIterator, typename OutputIterator> 
                void kmeans(InputIterator ibegin, InputIterator iend, OutputIterator obegin, 
	       std::vector<double>& classes)
   Run a kmeans clustering on some input data and store the class centers and the 
   class membership. 
   \tparam InputIterator readable forward iterator, 
   \tparam OutputIterator writable forward iterator, 
   \param ibegin iterator indicating the start of the input data 
   \param iend iterator indicating the end of the input data, expect an STL-like handling, 
   i.e. iend points behind the last element to be accessed
   \retval obegin begin of the output range where the class membership will be stored
   it is up to the caller to ensure that this range is at least as large as the input range
   \retval classes at input the size of the vector indicates the number of clusters to be used
   at output the vector elements contain the class centers in increasing order. 
*/

template <typename InputIterator, typename OutputIterator> 
BOOST_CONCEPT_REQUIRES( ((::boost::ForwardIterator<InputIterator>))
		        ((::boost::Mutable_ForwardIterator<OutputIterator>)),
			(void)
			)
	kmeans(InputIterator ibegin, InputIterator iend, OutputIterator obegin, 
	       std::vector<double>& classes)
{
	if (classes.size() < 2)
		THROW(std::invalid_argument, "kmeans: require at least two classes");  

	const size_t nclusters = classes.size(); 
	const double size = distance(ibegin, iend); 
	if ( size < nclusters ) 
		THROW(std::invalid_argument, "kmeans: require at least as many elements as classes");

	double sum = std::accumulate(ibegin, iend, 0.0); 
	
	// simple initialization splitting at the mean 
	classes[0] = sum / (size - 1);
	classes[1] = sum / (size + 1);
	
	// first run calles directly 
	int biggest_class = 0; 
	kmeans_step(ibegin, iend, obegin, classes, 1, biggest_class); 
	
	// further clustering always splits biggest class 
	for (size_t  l = 2; l < nclusters; l++) {
		const size_t pos = biggest_class > 0 ? biggest_class - 1 : biggest_class + 1; 
		classes[l] = 0.5 * (classes[biggest_class] + classes[pos]);
		kmeans_step(ibegin, iend, obegin, classes, l, biggest_class); 
	};		
	
	// some post iteration until centers no longer change 
	for (size_t  l = 1; l < 3; l++) {
		if (kmeans_step(ibegin, iend, obegin, classes, nclusters - 1, biggest_class)) 
			break; 
	}
}

NS_MIA_END

#endif 
