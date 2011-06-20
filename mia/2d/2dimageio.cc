/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
 * Max-Planck-Institute for Human Cognitive and Brain Science
 * Max-Planck-Institute for Evolutionary Anthropology
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
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

#include <mia/core/export_handler.hh>

#include <mia/2d/2dimageio.hh>
#include <mia/core/ioplugin.cxx>
#include <mia/core/iohandler.cxx>
#include <mia/core/errormacro.hh>

NS_MIA_BEGIN

const char *io_2dimage_type::type_descr = "2dimage";

C2DImageVector *C2DImageVector::clone() const
{
	return new C2DImageVector(*this);
}

C2DImageIOPPH::C2DImageIOPPH(const std::list< ::boost::filesystem::path>& searchpath):
	TIOPluginHandler<C2DImageIOPlugin>(searchpath)
{
	C2DFVectorTranslator::register_for("pixel");
}

template class TIOPlugin<io_2dimage_type>;
template class THandlerSingleton<C2DImageIOPPH>;
template class TIOHandlerSingleton<C2DImageIOPPH>;
template class TIOPluginHandler<C2DImageIOPlugin>;
template class TPluginHandler<C2DImageIOPlugin>;


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
		THROW(runtime_error, filename << " no image  found");
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

C2DImageGroupedSeries  EXPORT_2D load_image_series(const std::vector<std::string>& filenames)
{
	C2DImageGroupedSeries result; 
	const static string unknown_protocol("Unknown"); 
	int instance_nr = 0; 
	
	for (auto f = filenames.begin(); f != filenames.end(); ++f) {
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
			if (result.find(key) == result.end())
				result[key] = C2DImageSeriesGroup(); 
			C2DImageSeriesGroup& group = result[key]; 

			// look for acquisition number 
			auto attr = (*i)->get_attribute(IDAcquisitionNumber); 
			const CIntAttribute *paq = dynamic_cast<const CIntAttribute *>(attr.get());
			int aqnr = -1; 
			if (paq) 
				aqnr = *paq; 
			if (group.find(aqnr) == group.end()) 
				group[aqnr] = C2DImageSeries(); 

			// look for slice location number, if non exists fake one
			attr = (*i)->get_attribute(IDSliceLocation);
			const CFloatAttribute *pinst = dynamic_cast<const CFloatAttribute *>(attr.get());
			float location; 
			if (!pinst) {
				location = 0.0; 
				(*i)->set_attribute(IDInstanceNumber, PAttribute(new CIntAttribute(location))); 
			} else 
				location = floor(1000.0 * *pinst) / 1000.0; 
			
			group[aqnr].push_back(*i); 
			cvdebug() << "Add '" <<* f  
				  << "' to Protocol group '" << protocol
				  << "' with acquisition no. " << aqnr 
				  << "' and location no. " << location << "\n"; 
		}
	}
	for (auto g = result.begin(); g != result.end(); ++g) {
		for (auto aq = g->second.begin(); aq != g->second.end(); ++aq) {
			sort(aq->second.begin(), aq->second.end(), 
			     [](const P2DImage& lhs, const P2DImage& rhs) {
				     const auto lhs_attr = lhs->get_attribute(IDSliceLocation);
				     const auto rhs_attr = lhs->get_attribute(IDSliceLocation);
				     const int lhs_inr = dynamic_cast<const CFloatAttribute&>(*lhs_attr); 
				     const int rhs_inr = dynamic_cast<const CFloatAttribute&>(*rhs_attr); 
				     return lhs_inr < rhs_inr; 
			     }); 
		}
	}
	return result; 
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
NS_MIA_END
