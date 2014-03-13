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

#ifndef mia_core_attribute_names_hh
#define mia_core_attribute_names_hh

#include <mia/core/defines.hh>

NS_MIA_BEGIN

extern EXPORT_CORE const char * IDMediaStorageSOPClassUID;
extern EXPORT_CORE const char * IDStudyDescription;
extern EXPORT_CORE const char * IDSeriesDescription;
extern EXPORT_CORE const char * IDSamplesPerPixel;
extern EXPORT_CORE const char * IDTestValue;
extern EXPORT_CORE const char * IDSOPClassUID;
extern EXPORT_CORE const char * IDAcquisitionTime;
extern EXPORT_CORE const char * IDPositionerPrimaryAngle; 
extern EXPORT_CORE const char * IDPositionerSecondaryAngle; 
extern EXPORT_CORE const char * IDImagerPixelSpacing; 
extern EXPORT_CORE const char * IDDistanceSourceToDetector; 
extern EXPORT_CORE const char * IDDistanceSourceToPatient; 
extern EXPORT_CORE const char * IDPixelIntensityRelationship;
extern EXPORT_CORE const char * IDPositionerPrimaryAngleIncrement; 
extern EXPORT_CORE const char * IDPositionerSecondaryAngleIncrement; 
extern EXPORT_CORE const char * IDSliceThickness; 

NS_MIA_END

#endif 
