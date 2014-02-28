/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2014 Gert Wollny
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

#include <dicom/dcm2d.hh>
#include <dicom/dicom4mia.hh>

#include <sstream>
#include <mia/core/errormacro.hh>
#include <mia/core/file.hh>
#include <mia/core/filter.hh>
#include <mia/core/msgstream.hh>

NS_BEGIN(IMAGEIO_2D_DICOM)

NS_MIA_USE;

using namespace std;
using namespace boost;

CDicom2DImageIOPlugin::CDicom2DImageIOPlugin():
	C2DImageIOPlugin("dicom")
{
	add_supported_type(it_ushort);

	TTranslator<float>::register_for("SliceLocation");
	TTranslator<float>::register_for("AcquisitionTime"); 
	TTranslator<int>::register_for("SeriesNumber");
	TTranslator<int>::register_for("AcquisitionNumber");
	TTranslator<int>::register_for("InstanceNumber");
	CPatientPositionTranslator::register_for(IDPatientPosition);
	add_suffix(".dcm");
	add_suffix(".DCM");
}

C2DImageIOPlugin::PData CDicom2DImageIOPlugin::do_load(const string& fname) const
{
	TRACE_FUNCTION;
	PData result;

	CDicomReader reader(fname.c_str());
	if (!reader.good())
		return result;

	result.reset(new Data);
	result->push_back(reader.get_image());
	return result;
}

bool CDicom2DImageIOPlugin::do_save(const string& fname, const Data& data) const
{
	if (data.empty())
		throw create_exception<runtime_error>("CDicom2DImageIOPlugin: '", fname, 
					    "', no images to save");

	if (data.size() > 1)
		throw create_exception<runtime_error>("CDicom2DImageIOPlugin: '", fname, 
					    "' DICOM writer only supports one image per file");

	CDicomWriter writer(**data.begin());
	return writer.write(fname.c_str());
}

const string CDicom2DImageIOPlugin::do_get_descr() const
{
	return "2D image io for DICOM";
}

std::string CDicom2DImageIOPlugin::do_get_preferred_suffix() const
{
	return "dcm"; 
}


extern "C" EXPORT  CPluginBase *get_plugin_interface()
{
	return new CDicom2DImageIOPlugin();
}

NS_END
