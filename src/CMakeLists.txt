#
# This file is part of MIA - a toolbox for medical image analysis 
# Copyright (c) Leipzig, Madrid 1999-2015 Gert Wollny
#
# MIA is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, see <http://www.gnu.org/licenses/>.
#
#

ADD_CUSTOM_TARGET(xmldoc)

INCLUDE (${CMAKE_SOURCE_DIR}/doc/MiaDoctools.cmake)

SET(NIPYPE_INTERFACE_INIT_FILE ${CMAKE_CURRENT_BINARY_DIR}/__init__.py)
MIA_PREPARE_AUTODOC(mia)

ADD_CUSTOM_TARGET(mandir COMMAND mkdir -p ${CMAKE_BINARY_DIR}/doc/man)

LINK_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR}/../mia/core)
LINK_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR}/../mia/2d)
LINK_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR}/../mia/3d)

#basic helpers 
DEFEXE(filenumberpattern  miacore  )
DEFEXE(cmeans             miacore  )
DEFEXE(labelsort          miacore  )
DEFEXE(wavelettrans       miacore  )
DEFEXE(myowavelettest     miacore  )

#2D programs 
DEFEXE(2dimagefilter      mia2d )
DEFEXE(2dcost             mia2d )
DEFEXE(2ddistance         mia2d )
DEFEXE(multihist          mia2d )
DEFEXE(2dimagestats       mia2d )
DEFEXE(2dimagefilterstack mia2d )
DEFEXE(2dimagecreator     mia2d )
DEFEXE(2dimagecombiner    mia2d )
DEFEXE(raw2image          mia2d )
DEFEXE(2ddeform           mia2d )
DEFEXE(2dtransform        mia2d )
DEFEXE(2dforce            mia2d )
DEFEXE(2davgmasked        mia2d )
DEFEXE(2dimageregistration mia2d )
DEFEXE(2dimageselect      mia2d )
DEFEXE(2dimagecombine-dice mia2d )
DEFEXE(2dfuzzysegment     mia2d )
DEFEXE(2dsegment-ahmed    mia2d )
DEFEXE(2dsegment-fuzzyw   mia2d )
DEFEXE(2dmultiimagevar    mia2d )
DEFEXE(2dmultiimageregistration    mia2d )
DEFEXE(2dmulti-force      mia2d )
DEFEXE(2dimagefullstats   mia2d )
DEFEXE(2dbinarycombine    mia2d )
DEFEXE(2dtransformation-to-strain mia2d)
DEFEXE(2dimageseries-maximum-intensity-projection mia2d) 

DEFCHKEXE(2dlerp mia2d )


#Programs related to 2D myocardic perfusion 

DEFEXE(2dsegshift             mia2dmyocardperf )
DEFEXE(2dsegshiftperslice     mia2dmyocardperf )
DEFEXE(2dsegseriesstats       mia2dmyocardperf )
DEFEXE(2dseries2dordermedian  mia2dmyocardperf )
DEFEXE(2dmyoserial-nonrigid   mia2dmyocardperf )
DEFEXE(2dmyoperiodic-nonrigid mia2dmyocardperf )
DEFEXE(2dseriescorr           mia2dmyocardperf )
DEFEXE(2dseriesgradvariation  mia2dmyocardperf )
DEFEXE(2dseriesgradMAD        mia2dmyocardperf )
DEFEXE(2dseriessmoothgradMAD  mia2dmyocardperf )
DEFEXE(2dsegmentcropbox       mia2dmyocardperf )
DEFEXE(2dseghausdorff         mia2dmyocardperf )
DEFEXE(2dserieshausdorff      mia2dmyocardperf )
DEFEXE(2dsegcompare           mia2dmyocardperf )
DEFEXE(2dgroundtruthreg       mia2dmyocardperf )
DEFEXE(2dseries2sets          mia2dmyocardperf )
DEFEXE(2dseries-segdistance   mia2dmyocardperf )
DEFEXE(2dseries-mincorr       mia2dmyocardperf )

DEFEXE(2dseries-sectionmask   mia2dmyocardperf )
DEFEXE(2dmyoseries-dice       mia2dmyocardperf )
DEFEXE(2dmyoseries-compdice   mia2dmyocardperf )
DEFEXE(2dmyocard-segment      mia2dmyocardperf )
DEFEXE(2dmyopgt-nonrigid      mia2dmyocardperf )
DEFEXE(2dmany2one-nonrigid    mia2dmyocardperf )
DEFEXE(2dmyoset-all2one-nonrigid    mia2dmyocardperf )

IF(ITPP_FOUND) 
  DEFEXE(2dmyoica-full      mia2dmyocardperf )
  DEFEXE(2dmyoicapgt        mia2dmyocardperf )
  DEFEXE(2dmyomilles            mia2dmyocardperf )
  DEFEXE(2dmyocard-ica          mia2dmyocardperf )
  DEFEXE(2dmyocard-icaseries    mia2dmyocardperf )
  DEFEXE(2dmyoica-nonrigid      mia2dmyocardperf )
  DEFEXE(2dmyoica-nonrigid-parallel      mia2dmyocardperf )
  DEFEXE(2dmyoica-nonrigid2     mia2dmyocardperf )
  SET(PERF_AND_3D_LIBS mia3d mia2dmyocardperf) 
  DEFEXE(2dseriestovolume  "${PERF_AND_3D_LIBS}")
ENDIF(ITPP_FOUND)

DEFEXE(2dgrayimage-combine-to-rgb mia2d)
DEFEXE(2deval-transformquantity mia2d)
DEFEXE(2dtrackpixelmovement mia2d )

DEFCHKEXE(3dlerp mia3d )

DEFEXE(2dstack-cmeans-presegment  mia3d )

DEFEXE(2dmultiimageto3d   mia3d )
DEFEXE(3dimagestats       mia3d )
DEFEXE(3dcost             mia3d )
DEFEXE(3ddistance         mia3d )
DEFEXE(3dimagefilter      mia3d )
DEFEXE(3dimagecombine     mia3d )
DEFEXE(3dimagecreator     mia3d )
DEFEXE(3dimagefilterstack mia3d )
DEFEXE(2dimagestack-cmeans mia3d )
DEFEXE(raw2volume         mia3d )
DEFEXE(3dcrispsegment     mia3d )
DEFEXE(3ddeform           mia3d )
DEFEXE(3dforce            mia3d )
DEFEXE(3dfuzzysegment     mia3d )
DEFEXE(3dgetslice         mia3d )
DEFEXE(3dgetsize          mia3d )
DEFEXE(2dto3dimage        mia3d )
DEFEXE(2dto3dimageb       mia3d )
DEFEXE(3drigidreg         mia3d )
DEFEXE(3dnonrigidreg      mia3d )
DEFEXE(3dnonrigidreg-alt  mia3d )
DEFEXE(2dstackfilter      mia3d )
DEFEXE(3dtransform        mia3d )
DEFEXE(3dmaskseeded       mia3d )
DEFEXE(3dfield2norm       mia3d )
DEFEXE(3dimageselect      mia3d )
DEFEXE(3dbrainextractT1   mia3d )
DEFEXE(3dbinarycombine    mia3d )
DEFEXE(3ddistance-stats   mia3d )
DEFEXE(3dimageaddattributes  mia3d )
DEFEXE(3dvectorfieldcreate  mia3d )
DEFEXE(3dserial-nonrigid  mia3d )
DEFEXE(3dmany2one-nonrigid  mia3d )
DEFEXE(3dprealign-nonrigid mia3d )
DEFEXE(3dseries-track-intensity mia3d )
DEFEXE(3dtransform2vf mia3d )
DEFEXE(3dtrackpixelmovement mia3d )
DEFEXE(3dvf2transform mia3d )
DEFEXE(3dvfcompare mia3d )
DEFEXE(3deval-transformquantity mia3d)
DEFEXE(3dcost-translatedgrad mia3d)
DEFEXE(3dsegment-ahmed    mia3d)
DEFEXE(3dcombine-mr-segmentations mia3d)
DEFEXE(3dcombine-imageseries mia3d)
DEFEXE(3dpropose-boundingbox mia3d)
DEFEXE(3dimagestatistics-in-mask mia3d)

DEFEXE(3dlandmarks-transform mia3d)
DEFEXE(3dlandmarks-distances mia3d)

DEFEXE(plugin-help miamesh )
DEFEXE(meshdistance-to-stackmask miamesh )
DEFEXE(meshfilter miamesh )
DEFEXE(mesh-deformable-model miamesh )
DEFEXE(mesh-to-maskimage  miamesh)

ADD_EXECUTABLE(mia-test-plugins-as-installed test_plugins_as_installed.cc)  
TARGET_LINK_LIBRARIES(mia-test-plugins-as-installed miamesh)
TARGET_LINK_LIBRARIES(mia-test-plugins-as-installed ${BOOST_UNITTEST})
IF(WIN32)
  SET_TARGET_PROPERTIES(test-3d-${name} PROPERTIES LINKER_FLAGS "/NODEFAULTLIB:MSVCRT")
ENDIF(WIN32)


IF(ITPP_FOUND) 
  DEFEXE(3dmotioncompica-nonrigid mia3d )
ENDIF(ITPP_FOUND) 

ADD_SUBDIRECTORY(fluid2d  )
ADD_SUBDIRECTORY(fluid3d  )
ADD_SUBDIRECTORY(isosurface  )

