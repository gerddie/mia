/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2016 Gert Wollny
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

/**
   some DICOM tags that may be used 
   \cond DICOM_TAGS 
 */
extern EXPORT_CORE const char * IDModality;
extern EXPORT_CORE const char * IDPatientOrientation;
extern EXPORT_CORE const char * IDPatientPosition;
extern EXPORT_CORE const char * IDAcquisitionDate;
extern EXPORT_CORE const char * IDAcquisitionNumber;
extern EXPORT_CORE const char * IDImageType;
extern EXPORT_CORE const char * IDInstanceNumber;

extern EXPORT_CORE const char * IDSeriesNumber;
extern EXPORT_CORE const char * IDSliceLocation;
extern EXPORT_CORE const char * IDStudyID;
extern EXPORT_CORE const char * IDSmallestImagePixelValue;
extern EXPORT_CORE const char * IDLargestImagePixelValue;
extern EXPORT_CORE const char * IDProtocolName; 


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
extern EXPORT_CORE const char * IDSpacingBetweenSlices; 
extern EXPORT_CORE const char * IDPhotometricInterpretation; 




extern EXPORT_CORE const char * IDAttrPixelSizeIsImager; 
extern EXPORT_CORE const char * IDRescaleIntercept; 
extern EXPORT_CORE const char * IDRescaleSlope; 

// \endcond DICOM_TAGS

NS_MIA_END

#endif 
