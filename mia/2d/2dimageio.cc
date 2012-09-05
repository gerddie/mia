/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2012 Gert Wollny
 *
 * This program is free software; you can redistribute it and/or modify
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <mia/core/export_handler.hh>

#include <mia/2d/2dimageio.hh>
#include <mia/core/ioplugin.cxx>
#include <mia/core/iohandler.cxx>
#include <mia/core/errormacro.hh>
#include <mia/core/tools.hh>

NS_MIA_BEGIN

const char *io_2dimage_type::data_descr = "2dimage";

C2DImageVector *C2DImageVector::clone() const
{
	return new C2DImageVector(*this);
}

C2DImageIOPPH::C2DImageIOPPH(const CPathNameArray& searchpath):
	TIOPluginHandler<C2DImageIOPlugin>(searchpath)
{
	C2DFVectorTranslator::register_for("pixel");
}


template <> const char *  const 
TPluginHandler<C2DImageIOPlugin>::m_help =  
   "These plug-ins handle loading and storing of 2D images to the supported image file types.";


P2DImageVector EXPORT_2D create_image2d_vector(P2DImage image)
{
	P2DImageVector out_list_ptr(new C2DImageIOPluginHandler::Instance::Data);
	out_list_ptr->push_back(image);
	return out_list_ptr;
}

P2DImage  EXPORT_2D load_image2d(const std::string& filename)
{
	C2DImageIOPluginHandler::Instance::PData  in_image_list =
		C2DImageIOPluginHandler::instance().load(filename);

	if (!in_image_list || in_image_list->empty()) {
		throw create_exception<runtime_error>( filename, " no image  found");
	}
	if (in_image_list->size() > 1) {
		cvwarn() << "load_image2d:'" << filename
			 << "' more then one image found, loading only first one\n";
	}

	return *in_image_list->begin();
}

bool  EXPORT_2D save_image(const std::string& filename, P2DImage image)
{
	C2DImageIOPluginHandler::Instance::Data out_image_list;
	out_image_list.push_back(image);
	return C2DImageIOPluginHandler::instance().save(filename, out_image_list);
}

bool  EXPORT_2D save_image(const std::string& filename, C2DImage& image)
{
	return save_image(filename, P2DImage(&image, void_destructor<C2DImage>())); 
}

C2DImageGroupedSeries  EXPORT_2D load_image_series(const std::vector<std::string>& filenames, 
						   CProgressCallback *cb)
{
	typedef map<int, C2DImageSeries> C2DImageSeriesGroupHelper; 
	typedef map<int, C2DImageSeriesGroupHelper> C2DGroupSeriesHelper; 
	typedef map<string, C2DGroupSeriesHelper> C2DImageGroupedSeriesHelper; 


	C2DImageGroupedSeriesHelper collector; 
	const static string unknown_protocol("Unknown"); 
	
	int step = 0; 
	if (cb) 
		cb->set_range(filenames.size()); 

	cvdebug() << "Load " << filenames.size() << "  images\n"; 
	for (auto f = filenames.begin(); f != filenames.end(); ++f, ++step) {
		// give some feedback 
		cvdebug() << "Load " << *f << "\n"; 
		if (cb && ! (step & 0x1f) )
			cb->update(step); 
		
		C2DImageIOPluginHandler::Instance::PData  in_image_list =
			C2DImageIOPluginHandler::instance().load(*f);
		if (!in_image_list) {
			cverr() << "load_image_series: File '" << *f 
				<< "' doesn't provide images this software can read\n"; 
			continue; 
		}
		for (auto i = in_image_list->begin(); i != in_image_list->end(); ++i) {
			// look for protocol attribute 
			auto protocol = (*i)->get_attribute_as_string(IDProtocolName); 
			std::string key = protocol.empty() ? unknown_protocol : protocol;
			if (collector.find(key) == collector.end())
				collector[key] = C2DGroupSeriesHelper(); 
			C2DGroupSeriesHelper& group = collector[key]; 

			// look for series number 
			auto seriesnr = (*i)->get_attribute(IDSeriesNumber);
			const CIntAttribute *psn = dynamic_cast<const CIntAttribute *>(seriesnr.get());
			int saqnr = -1; 
			if (psn) 
				saqnr = *psn; 

			if (group.find(saqnr) == group.end())
				group[saqnr] = C2DImageSeriesGroupHelper(); 
			C2DImageSeriesGroupHelper& subgroup = group[saqnr]; 

			// look for acquisition number 
			auto attr = (*i)->get_attribute(IDAcquisitionNumber); 
			const CIntAttribute *paq = dynamic_cast<const CIntAttribute *>(attr.get());
			int aqnr = -1; 
			if (paq) 
				aqnr = *paq; 
			if (subgroup.find(aqnr) == subgroup.end()) 
				subgroup[aqnr] = C2DImageSeries(); 
			
			// look for slice location number, if non exists fake one
			attr = (*i)->get_attribute(IDSliceLocation);
			const CFloatAttribute *pinst = dynamic_cast<const CFloatAttribute *>(attr.get());
			float location; 
			if (!pinst) {
				location = 0.0; 
				(*i)->set_attribute(IDSliceLocation, PAttribute(new CFloatAttribute(location))); 
			} else 
				location = floor(1000.0 * *pinst) / 1000.0; 
			
			subgroup[aqnr].push_back(*i); 
			cvinfo() << "Add '" <<* f  
				  << "' to Protocol group '" << protocol
				  << "' with acquisition no. " << aqnr 
				  << "' and location no. " << location << "\n"; 
		}
	}
	// now sort the slices and move the groups to a zero-index based vector 
	C2DImageGroupedSeries result; 
	for (auto g = collector.begin(); g != collector.end(); ++g) {
		cvinfo() << "Protocol '" << g->first << "'\n"; 
		C2DImageSeriesGroup group; 
		for (auto saq = g->second.begin(); saq != g->second.end(); ++saq) {
			for (auto aq = saq->second.begin(); aq != saq->second.end(); ++aq) {
				sort(aq->second.begin(), aq->second.end(), 
				     [](const P2DImage& lhs, const P2DImage& rhs) {
					     const auto lhs_attr = lhs->get_attribute(IDSliceLocation);
					     const auto rhs_attr = rhs->get_attribute(IDSliceLocation);
					     const float lhs_inr = dynamic_cast<const CFloatAttribute&>(*lhs_attr); 
					     const float rhs_inr = dynamic_cast<const CFloatAttribute&>(*rhs_attr); 
					     return lhs_inr < rhs_inr; 
				     }); 
				cvinfo() << "  Acquisition " << aq->first << " with "<< aq->second.size() <<" slices\n"; 
				group.push_back(aq->second); 
			}
			result[g->first] = group; 
		}
	}
	return result; 
}

C2DImageIOPluginHandlerTestPath::C2DImageIOPluginHandlerTestPath()
{
	CPathNameArray searchpath({bfs::path(".")});
	C2DImageIOPluginHandler::set_search_path(searchpath);
}

EXPORT_2D const char * IDAcquisitionDate =   "AcquisitionDate";
EXPORT_2D const char * IDImageType =         "ImageType";
EXPORT_2D const char * IDAcquisitionNumber = "AcquisitionNumber";
EXPORT_2D const char * IDInstanceNumber =    "InstanceNumber";
EXPORT_2D const char * IDSliceLocation = "SliceLocation";
EXPORT_2D const char * IDSeriesNumber = "SeriesNumber";
EXPORT_2D const char * IDModality =          "Modality";
EXPORT_2D const char * IDPatientOrientation ="PatientOrientation";
EXPORT_2D const char * IDPatientPosition = "PatientPosition";
EXPORT_2D const char * IDSmallestImagePixelValue = "SmallestImagePixelValue";
EXPORT_2D const char * IDLargestImagePixelValue = "LargestImagePixelValue";
EXPORT_2D const char * IDStudyID = "StudyID";
EXPORT_2D const char * IDProtocolName = "ProtocolName"; 

template class TIOPlugin<io_2dimage_type>;
template class THandlerSingleton<C2DImageIOPPH>;
template class TIOHandlerSingleton<C2DImageIOPPH>;
template class TIOPluginHandler<C2DImageIOPlugin>;
template class TPluginHandler<C2DImageIOPlugin>;

NS_MIA_END
