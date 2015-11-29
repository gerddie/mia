/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2015 Gert Wollny
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


#include <mia/core/cmeansinit/kmeans.hh>

#include <iomanip>

NS_MIA_BEGIN

CKMeansInitializer::CKMeansInitializer(size_t nclasses):m_nclasses(nclasses)
{
}

bool kmeans_step(const CMeans::NormalizedHistogram& nh, std::vector<int>& classification,
		 CMeans::DVector& classes, size_t l, int& biggest_class )
{
	cvdebug()<<  "kmeans enter: ";
	for (size_t i = 0; i <= l; ++i )
		cverb << std::setw(8) << classes[i]<< " ";  
	cverb << "\n"; 
	
	biggest_class = -1; 
	const double convLimit = 0.005;	// currently fixed
	
	std::vector<double> sums(classes.size()); 
	std::vector<double> count(classes.size()); 
	
	bool conv = false;
	int iter = 50; 
	
	while( iter-- && !conv) {

		sort(classes.begin(), classes.end()); 
		
		// assign closest cluster center
		auto ob = classification.begin(); 
		for (auto b = nh.begin(); b != nh.end(); ++b, ++ob) {
			const double val = b->first;
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
			
			count[c] += b->second;
			sums[c] += val * b->second;
		};

		
		// recompute cluster centers
		conv = true;
		double max_count = 0; 
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
					classes[i] = (classes[i] + classes[i - 1]) / 2.0; 
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
	

CMeans::DVector CKMeansInitializer::run(const CMeans::NormalizedHistogram& nh) const
{
	if (m_nclasses < 2)
		throw create_exception<std::invalid_argument>("cmeans: requested ", m_nclasses, 
						    "class(es), required are at least two");
	
	if ( nh.size() < m_nclasses ) 
		throw create_exception<std::invalid_argument>("kmeans: insufficient input: want ", m_nclasses , 
						    " classes, but git only ",  nh.size(), " distinct input values"); 

	CMeans::DVector classes(m_nclasses);

	std::vector<int> classification(nh.size(), 0);
	
	double sum = 0.0;
	for(auto h : nh) {
		sum += h.first * h.second; 
	}; 

	
	// simple initialization splitting at the mean 
	classes[0] = sum / 1.99;  
	classes[1] = sum / 2.01; 
	
	// first run calles directly 
	int biggest_class = 0; 
	kmeans_step(nh, classification, classes, 1, biggest_class);
	
	// further clustering always splits biggest class 
	for (size_t  l = 2; l < m_nclasses; l++) {
		const size_t pos = biggest_class > 0 ? biggest_class - 1 : biggest_class + 1; 
		classes[l] = 0.5 * (classes[biggest_class] + classes[pos]);
		kmeans_step(nh, classification, classes, l, biggest_class); 
	};		
	
	// some post iteration until centers no longer change 
	for (size_t  l = 1; l < 3; l++) {
		if (kmeans_step(nh, classification, classes, m_nclasses - 1, biggest_class)) 
			break; 
	}
	return classes; 
        
}

CKMeansInitializerPlugin::CKMeansInitializerPlugin():
	CMeansInitializerSizedPlugin("kmeans")
{
}


CMeansInitializerPlugin::Product * CKMeansInitializerPlugin::do_create() const
{
        return new CKMeansInitializer(get_size_param());
}

const std::string CKMeansInitializerPlugin::do_get_descr() const
{
        return "C-Means initializer that sets the initial class centers by using a k-means classification";
}

extern "C" EXPORT CPluginBase  *get_plugin_interface()
{

	return new CKMeansInitializerPlugin(); 
}


NS_MIA_END
