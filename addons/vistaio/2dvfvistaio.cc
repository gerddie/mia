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

#include <sstream>
#include <cassert>
#include <algorithm>
#include <mia/core/file.hh>
#include <mia/core/filter.hh>
#include <mia/core/msgstream.hh>
#include <mia/2d/vfio.hh>

#include <vistaio/vista4mia.hh>

NS_BEGIN(vista_2dvf_io)

NS_MIA_USE

using namespace std;
using namespace boost;

class CVista2DVFIOPlugin : public C2DVFIOPlugin {
public:
	CVista2DVFIOPlugin();
private:
	PData do_load(const string& fname) const;
	bool do_save(const string& fname, const Data& data) const;
	const string do_get_descr() const;
};

CVista2DVFIOPlugin::CVista2DVFIOPlugin():
	C2DVFIOPlugin("vista")
{
	add_supported_type(it_float);
	add_suffix(".v");
	add_suffix(".vf");
	add_suffix(".V");
	add_suffix(".VF");
	
	add_standard_vistaio_properties(*this); 

}

CVista2DVFIOPlugin::PData  CVista2DVFIOPlugin::do_load(const string& fname) const
{
	CInputFile f(fname);

	VistaIOAttrList vlist = VistaIOReadFile(f,NULL);

	VistaIOResetProgressIndicator();

	if (!vlist)
		return CVista2DVFIOPlugin::PData();

	CVista2DVFIOPlugin::PData result;
	VistaIOAttrListPosn posn;

	for (VistaIOFirstAttr(vlist, &posn); VistaIOAttrExists(&posn) && !result; VistaIONextAttr(&posn)) {
		if (VistaIOGetAttrRepn(&posn) != VistaIOField2DRepn)
			continue;

		VistaIOField2D field = NULL;
		VistaIOGetAttrValue(&posn, 0, VistaIOField2DRepn, &field);

		if (!field)
			throw runtime_error(fname + "looked like a vector field, but is none");

		if (field->nsize_element != 2)
			throw runtime_error(fname + "is not a field of 2D vectors");

		if (field->repn != VistaIOFloatRepn) {
			cvdebug() << "Skipping input field, which is not of type float\n";
			continue;
		}


		result = CVista2DVFIOPlugin::PData(new C2DIOVectorfield(C2DBounds(field->x_dim, field->y_dim)));
		T2DVector<VistaIOFloat> * input  = (T2DVector<VistaIOFloat> *)field->p.data;
		copy(input, input + result->size(), result->begin());
		copy_attr_list(*result, field->attr);
	}


	VistaIODestroyAttrList(vlist);
	return result;
}


bool CVista2DVFIOPlugin::do_save(const string& fname, const C2DIOVectorfield& data) const
{

	COutputFile f(fname);

	VistaIOAttrList vlist = VistaIOCreateAttrList();
	VistaIOField2D out_field = VistaIOCreateField2D(data.get_size().x,
					    data.get_size().y,
					    2,
					    VistaIOFloatRepn);
	T2DVector<VistaIOFloat> * output  = (T2DVector<VistaIOFloat> *)out_field->p.data;
	copy(data.begin(), data.end(), output);

	copy_attr_list(out_field->attr, data);
	VistaIOSetAttr(vlist, "2DFVectorfield", NULL, VistaIOField2DRepn, out_field);

	bool result = VistaIOWriteFile(f,vlist);
	VistaIODestroyAttrList(vlist);

	return result;
}

const string CVista2DVFIOPlugin::do_get_descr() const
{
	return "a 2d vector field io plugin for vista";
}

extern "C" EXPORT  CPluginBase *get_plugin_interface()
{
	C2DFVectorTranslator::register_for("pixel");
	return new CVista2DVFIOPlugin();
}

NS_END
