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

#include <limits>
#include <stdexcept>

// this is for the definition of the 3D image plugin base classes
#include <mia/3d/filter/reorient.hh>


NS_BEGIN(reorient_3dimage_filter)

// don't need to write all the names ...
NS_MIA_USE;
using namespace std;

C3DReorient::C3DReorient(EOrientations strategy): m_strategy(strategy)
{
}

/* This is the work horse operator of the filter. */
template <typename T>
C3DReorient::result_type C3DReorient::operator () (const mia::T3DImage<T>& data) const
{
       TRACE(__FUNCTION__);
       T3DImage<T> *result = NULL;
       EOrientations strategy = m_strategy;
       cvdebug() << "strategy = " << strategy << "(" << m_strategy << ")\n";

       switch (strategy) {
       case xyz: {
              result = new T3DImage<T>(data.get_size(), data);
              copy(data.begin(), data.end(), result->begin());
              break;
       }

       case flip_yz: {
              C3DBounds out_size(data.get_size().xzy() );
              result = new T3DImage<T>(out_size, data);
              result->set_voxel_size(data.get_voxel_size().xzy());

              for (size_t z = 0; z < data.get_size().z; ++z) {
                     for (size_t y = 0; y < data.get_size().y; ++y) {
                            copy(data.begin_at(0, y, z), data.begin_at(0, y, z) + data.get_size().x,
                                 result->begin_at(0, z, y));
                     }
              }

              break;
       }

       case flip_xy: {
              C3DBounds out_size(data.get_size().yxz() );
              result = new T3DImage<T>(out_size, data);
              result->set_voxel_size(data.get_voxel_size().yxz());

              for (size_t z = 0; z < data.get_size().z; ++z) {
                     for (size_t y = 0; y < data.get_size().y; ++y) {
                            for (size_t x = 0; x < data.get_size().x; ++x) {
                                   (*result)(y, x, z) = data(x, y, z);
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
                                   (*result)(y, z, x) = data(x, y, z);
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
                                   (*result)(z, x, y) = data(x, y, z);
                            }
                     }
              }

              break;
       }

       case flip_xz: {
              C3DBounds out_size(data.get_size().zyx() );
              result = new T3DImage<T>(out_size, data);
              result->set_voxel_size(data.get_voxel_size().zyx());

              for (size_t z = 0; z < data.get_size().z; ++z) {
                     for (size_t y = 0; y < data.get_size().y; ++y) {
                            for (size_t x = 0; x < data.get_size().x; ++x) {
                                   (*result)(z, y, x) = data(x, y, z);
                            }
                     }
              }

              break;
       }

       case rotate_x90: {
              // rotate around x-axis by 90 degree
              C3DBounds out_size(data.get_size().yxz() );
              result = new T3DImage<T>(out_size, data);
              result->set_voxel_size(data.get_voxel_size().yxz());

              for (size_t z = 0; z < data.get_size().z; ++z) {
                     for (size_t y = 0; y < data.get_size().y; ++y) {
                            copy(data.begin_at(0, y, z), data.begin_at(0, y, z) + data.get_size().x,
                                 result->begin_at(0, result->get_size().z - z - 1, y));
                     }
              }

              break;
       }

       case rotate_x180: {
              // rotate around x-axis by 180 degree
              C3DBounds out_size(data.get_size());
              result = new T3DImage<T>(out_size, data);
              result->set_voxel_size(data.get_voxel_size());

              for (size_t z = 0; z < data.get_size().z; ++z) {
                     for (size_t y = 0; y < data.get_size().y; ++y) {
                            copy(data.begin_at(0, y, z), data.begin_at(0, y, z) + data.get_size().x,
                                 result->begin_at(0, result->get_size().y - 1 - y,
                                                  result->get_size().z - 1 - z));
                     }
              }

              break;
       }

       case rotate_x270: {
              // rotate around x-axis by 270 degree
              C3DBounds out_size(data.get_size().yxz() );
              result = new T3DImage<T>(out_size, data);
              result->set_voxel_size(data.get_voxel_size().yxz());

              for (size_t z = 0; z < data.get_size().z; ++z) {
                     for (size_t y = 0; y < data.get_size().y; ++y) {
                            copy(data.begin_at(0, y, z), data.begin_at(0, y, z) + data.get_size().x,
                                 result->begin_at(0, z, result->get_size().y - 1 - y));
                     }
              }

              break;
       }

       case rotate_y90: {
              // rotate around y-axis by 90 degree clock wise
              C3DBounds out_size(data.get_size().zyx());
              result = new T3DImage<T>(out_size, data);
              result->set_voxel_size(data.get_voxel_size().zyx());
              unsigned  oz = data.get_size().z - 1;

              for (size_t z = 0; z < data.get_size().z; ++z, --oz) {
                     for (size_t y = 0; y < data.get_size().y; ++y) {
                            for (size_t x = 0; x < data.get_size().x; ++x) {
                                   (*result)(oz, y, x) = data(x, y, z);
                            }
                     }
              }

              break;
       }

       case rotate_y180: {
              // rotate around y-axis by 180 degree
              C3DBounds out_size(data.get_size());
              result = new T3DImage<T>(out_size, data);
              result->set_voxel_size(data.get_voxel_size());
              unsigned  oz = data.get_size().z - 1;

              for (size_t z = 0; z < data.get_size().z; ++z, --oz) {
                     for (size_t y = 0; y < data.get_size().y; ++y) {
                            unsigned ox = data.get_size().x - 1;

                            for (size_t x = 0; x < data.get_size().x; ++x, --ox) {
                                   (*result)(oz, y, ox) = data(x, y, z);
                            }
                     }
              }

              break;
       }

       case rotate_y270: {
              // rotate around y-axis by 90 degree clock wise
              C3DBounds out_size(data.get_size().zyx());
              result = new T3DImage<T>(out_size, data);
              result->set_voxel_size(data.get_voxel_size().zyx());

              for (size_t z = 0; z < data.get_size().z; ++z) {
                     for (size_t y = 0; y < data.get_size().y; ++y) {
                            unsigned  ox = data.get_size().x - 1;

                            for (size_t x = 0; x < data.get_size().x; ++x, --ox) {
                                   (*result)(z, y, ox) = data(x, y, z);
                            }
                     }
              }

              break;
       }

       case rotate_z90: {
              // rotate around z-axis by 90 degree clock wise
              C3DBounds out_size(data.get_size().yxz());
              result = new T3DImage<T>(out_size, data);
              result->set_voxel_size(data.get_voxel_size().yxz());

              for (size_t z = 0; z < data.get_size().z; ++z) {
                     unsigned  oy = data.get_size().y - 1;

                     for (size_t y = 0; y < data.get_size().y; ++y, --oy) {
                            for (size_t x = 0; x < data.get_size().x; ++x) {
                                   (*result)(oy, x, z) = data(x, y, z);
                            }
                     }
              }

              break;
       }

       case rotate_z180: {
              // rotate around y-axis by 180 degree
              C3DBounds out_size(data.get_size());
              result = new T3DImage<T>(out_size, data);
              result->set_voxel_size(data.get_voxel_size());

              for (size_t z = 0; z < data.get_size().z; ++z) {
                     unsigned  oy = data.get_size().y - 1;

                     for (size_t y = 0; y < data.get_size().y; ++y, --oy) {
                            unsigned ox = data.get_size().x - 1;

                            for (size_t x = 0; x < data.get_size().x; ++x, --ox) {
                                   (*result)(oy, ox, z) = data(x, y, z);
                            }
                     }
              }

              break;
       }

       case rotate_z270: {
              // rotate around z-axis by 270 degree clock wise
              C3DBounds out_size(data.get_size().yxz());
              result = new T3DImage<T>(out_size, data);
              result->set_voxel_size(data.get_voxel_size().yxz());

              for (size_t z = 0; z < data.get_size().z; ++z) {
                     for (size_t y = 0; y < data.get_size().y; ++y) {
                            unsigned  ox = data.get_size().x - 1;

                            for (size_t x = 0; x < data.get_size().x; ++x, --ox) {
                                   (*result)(y, ox, z) = data(x, y, z);
                            }
                     }
              }

              break;
       }

       default:
              throw invalid_argument("Unknown reorientation requested");
       }

       E3DImageOrientation new_orient = get_new_orientation(strategy, data.get_orientation());
       cvdebug() << "new_orient = " << new_orient << "\n";
       result->set_orientation(new_orient);
       return P3DImage(result);
}

E3DImageOrientation C3DReorient::get_new_orientation(EOrientations strategy, E3DImageOrientation old_orientation) const
{
       TRACE(__FUNCTION__);
       cvdebug() << "strategy = " << strategy << ", old_orientation: " << old_orientation << "\n";

       switch (strategy) {
       case xyz:
              return old_orientation;

       case flip_yz:
              switch (old_orientation) {
              case ior_axial:
                     return ior_saggital;

              case ior_saggital:
                     return ior_axial;

              case ior_coronal:
                     return ior_axial;

              default:
                     return old_orientation;
              }

       case flip_xy:
              return old_orientation;

       case yzx:
              switch (old_orientation) {
              case ior_axial:
                     return ior_saggital;

              case ior_saggital:
                     return ior_coronal;

              case ior_coronal:
                     return ior_saggital;

              default:
                     return old_orientation;
              }

       case zxy:
              switch (old_orientation) {
              case ior_axial:
                     return ior_coronal;

              case ior_saggital:
                     return ior_axial;

              case ior_coronal:
                     return ior_axial;

              default:
                     return old_orientation;
              }

       case flip_xz:
              switch (old_orientation) {
              case ior_axial:
                     return ior_saggital;

              case ior_saggital:
                     return ior_coronal;

              case ior_coronal:
                     return ior_saggital;

              default:
                     return old_orientation;
              }

       default:
              return old_orientation;
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
       {"f-yz", C3DReorient::flip_yz, "flip y-z" },
       {"f-xy", C3DReorient::flip_xy, "flip x-y" },
       {"p-yzx", C3DReorient::yzx, "permutate x->z->y->x" },
       {"p-zxy", C3DReorient::zxy, "permutate x->y->z->x" },
       {"f-xz", C3DReorient::flip_xz, "flip x-z" },
       {"r-x90", C3DReorient::rotate_x90, "rotate around x-axis clockwise 90 degree" },
       {"r-x180", C3DReorient::rotate_x180, "rotate around x-axis clockwise 180 degree" },
       {"r-x270", C3DReorient::rotate_x270, "rotate around x-axis clockwise 270 degree" },
       {"r-y90", C3DReorient::rotate_y90, "rotate around y-axis clockwise 90 degree" },
       {"r-y180", C3DReorient::rotate_y180, "rotate around y-axis clockwise 180 degree" },
       {"r-y270", C3DReorient::rotate_y270, "rotate around y-axis clockwise 270 degree" },
       {"r-z90", C3DReorient::rotate_z90, "rotate around z-axis clockwise 90 degree" },
       {"r-z180", C3DReorient::rotate_z180, "rotate around z-axis clockwise 180 degree" },
       {"r-z270", C3DReorient::rotate_z270, "rotate around z-axis clockwise 270 degree" },
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
