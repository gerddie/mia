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

/* 
   LatexBeginPluginDescription{3D image filters}
   
   \subsection{Reorient image}
   \label{filter3d:reorient}
   
   \begin{description}
   
   \item [Plugin:] reorient
   \item [Description:] Change the orientation of the 3D image 
   \item [Input:] An abitary 3D image 
   \item [Input:] The re-oriented 3D image 

   \plugtabstart
   map & string & mapping to be applied & xyz \\
   \plugtabend
   \end{description}

   LatexEnd  
 */


#include <limits>
#include <stdexcept>
// boost type trains are needed to check whether we are dealing with an integer pixel valued image
#include <boost/type_traits.hpp>

// this is for the definition of the 3D image plugin base classes
#include <mia/3d/filter/reorient.hh>


// start a namespace to avoid naming conflicts during runtime

NS_BEGIN(reorient_3dimage_filter)

// don't need to write all the names ...
NS_MIA_USE;
using namespace std;

C3DReorient::C3DReorient(EOrientations strategy):m_strategy(strategy)
{
}

/* This is the work horse operator of the filter. */
template <typename T>
C3DReorient::result_type C3DReorient::operator () (const mia::T3DImage<T>& data) const
{
	TRACE(__FUNCTION__);
	T3DImage<T> *result = NULL;

	EOrientations strategy = m_strategy;

	if (strategy == axial ||  strategy == coronal || strategy == saggital) {
		strategy = get_strategy(data.get_orientation());
	}

	cvdebug() << "strategy = " << strategy << "(" << m_strategy <<")\n";
	switch (strategy) {
	case xyz: {
		result = new T3DImage<T>(data.get_size(), data);
		copy(data.begin(), data.end(), result->begin());
		break;
	}
	case xzy: {
		C3DBounds out_size(data.get_size().xzy() );
		result = new T3DImage<T>(out_size, data);
		result->set_voxel_size(data.get_voxel_size().xzy());
		for (size_t z = 0; z < data.get_size().z; ++z) {
			for (size_t y = 0; y < data.get_size().y; ++y) {
				copy(data.begin_at(0,y,z), data.begin_at(0,y,z) + data.get_size().x,
				     result->begin_at(0,z,y));
			}
		}
		break;
	}
	case yxz: {
		C3DBounds out_size(data.get_size().yxz() );
		result = new T3DImage<T>(out_size, data);
		result->set_voxel_size(data.get_voxel_size().yxz());
		for (size_t z = 0; z < data.get_size().z; ++z) {
			for (size_t y = 0; y < data.get_size().y; ++y) {
				for (size_t x = 0; x < data.get_size().x; ++x) {
					(*result)(y,x,z) = data(x,y,z);
				}
			}
		}
		break;
	}
	case yzx: {
		C3DBounds out_size(data.get_size().yzx());
		result = new T3DImage<T>(out_size, data);
		result->set_voxel_size(data.get_voxel_size().yzx());
		for (size_t z = 0; z < data.get_size().z; ++z) {
			for (size_t y = 0; y < data.get_size().y; ++y) {
				for (size_t x = 0; x < data.get_size().x; ++x) {
					(*result)(y,z,x) = data(x,y,z);
				}
			}
		}
		break;
	}
	case zxy: {
		C3DBounds out_size(data.get_size().zxy() );
		result = new T3DImage<T>(out_size, data);
		result->set_voxel_size(data.get_voxel_size().zxy());
		for (size_t z = 0; z < data.get_size().z; ++z) {
			for (size_t y = 0; y < data.get_size().y; ++y) {
				for (size_t x = 0; x < data.get_size().x; ++x) {
					(*result)(z,x,y) = data(x,y,z);
				}
			}
		}
		break;
	}
	case zyx: {
		C3DBounds out_size(data.get_size().zyx() );
		result = new T3DImage<T>(out_size, data);
		result->set_voxel_size(data.get_voxel_size().zyx());
		for (size_t z = 0; z < data.get_size().z; ++z) {
			for (size_t y = 0; y < data.get_size().y; ++y) {
				for (size_t x = 0; x < data.get_size().x; ++x) {
					(*result)(z,y,x) = data(x,y,z);
				}
			}
		}
		break;
	}
	default:
		throw invalid_argument("Unknown reorientation requested");
	}
	E3DImageOrientation new_orient = get_new_orientation(strategy, data.get_orientation());
	cvdebug() << "new_orient = " <<new_orient <<"\n";
	result->set_orientation(new_orient);
	return P3DImage(result);
}

E3DImageOrientation C3DReorient::get_new_orientation(EOrientations strategy, E3DImageOrientation old_orientation) const
{
	TRACE(__FUNCTION__);
	cvdebug() << "strategy = " << strategy << ", old_orientation: " << old_orientation << "\n";
	switch (strategy) {
	case xyz: return old_orientation;
	case xzy: switch (old_orientation) {
		case ior_axial:    return ior_saggital;
		case ior_saggital: return ior_axial;
		case ior_coronal:  return ior_axial;
		default:
			return old_orientation;
		}
	case yxz:switch (old_orientation) {
		case ior_axial:    return ior_axial;
		case ior_saggital: return ior_saggital;
		case ior_coronal:  return ior_coronal;
		default:
			return old_orientation;
		}
	case yzx:switch (old_orientation) {
		case ior_axial:    return ior_saggital;
		case ior_saggital: return ior_coronal;
		case ior_coronal:  return ior_saggital;
		default:
			return old_orientation;
		}
	case zxy:switch (old_orientation) {
		case ior_axial:    return ior_coronal;
		case ior_saggital: return ior_axial;
		case ior_coronal:  return ior_axial;
		default:
			return old_orientation;
		}
	case zyx:
		switch (old_orientation) {
		case ior_axial:    return ior_saggital;
		case ior_saggital: return ior_coronal;
		case ior_coronal:  return ior_saggital;
		default:
			return old_orientation;
		}
	case axial: return ior_axial;
	case coronal: return ior_coronal;
	case saggital: return ior_saggital;
	default:
		return old_orientation;
	}
}

C3DReorient::EOrientations C3DReorient::get_strategy(E3DImageOrientation old_orientation) const
{
	TRACE(__FUNCTION__);
	switch (old_orientation)  {
	case ior_axial:
		switch (m_strategy)  {
		case axial:    return xyz;
		case saggital: return zyx; //??
		case coronal:  return xyz; //??
		default:
			return xyz;
		};
	case ior_saggital:
		switch (m_strategy)  {
		case axial:    return zyx;
		case saggital: return xyz; //??
		case coronal:  return xyz; //??
		default:
			return xyz;
		};
	case ior_coronal:
		switch (m_strategy)  {
		case axial:    return xyz;
		case saggital: return zyx; //??
		case coronal:  return xyz; //??
		default:
			return xyz;
		};
	default:
		return xyz;
	}
}

/* The actual filter dispatch function calls the filter by selecting the right pixel type through wrap_filter */
P3DImage C3DReorient::do_filter(const C3DImage& image) const
{
	return mia::filter(*this, image);
}

/* The factory constructor initialises the plugin name, and takes care that the plugin help will show its parameters */
C3DReorientImageFilterFactory::C3DReorientImageFilterFactory():
	C3DFilterPlugin("reorient"),
	m_orientation(C3DReorient::xyz)
{
    add_parameter("map", new
        CDictParameter<C3DReorient::EOrientations>(m_orientation, map, "oriantation mapping to be applied"));
}

/* The factory create function creates and returns the filter with the given options*/
C3DFilter *C3DReorientImageFilterFactory::do_create()const
{
	return new C3DReorient(m_orientation);
}

/* This function sreturns a short description of the filter */
const string C3DReorientImageFilterFactory::do_get_descr()const
{
	return "3D image reorientation filter";
}

const TDictMap<C3DReorient::EOrientations>::Table
C3DReorientImageFilterFactory::table[] = {
	{"xyz", C3DReorient::xyz, "keep orientation"},
	{"xzy", C3DReorient::xzy, "switch y-z" },
	{"yxz", C3DReorient::yzx, "switch x-y" },
	{"yzx", C3DReorient::yxz, "rotate x->z->y->x" },
	{"zxy", C3DReorient::zxy, "rotate x->y->z->x" },
	{"zyx", C3DReorient::zxy, "switch x-z" },
	{NULL,  C3DReorient::ooo, "" },
};
const TDictMap<C3DReorient::EOrientations>
C3DReorientImageFilterFactory::map(C3DReorientImageFilterFactory::table);


/*
  Finally the C-interface function that returns the filter factory. It's linking style is "C"
  to avoid the name-mangling of C++. A clean name is needed, because this function is aquired
  by its "real"(compiled) name.
*/
extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C3DReorientImageFilterFactory();
}

NS_END
