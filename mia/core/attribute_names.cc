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

#include <mia/core/attribute_names.hh>

NS_MIA_BEGIN


EXPORT_CORE const char * IDAcquisitionDate =   "AcquisitionDate";
EXPORT_CORE const char * IDImageType =         "ImageType";
EXPORT_CORE const char * IDAcquisitionNumber = "AcquisitionNumber";
EXPORT_CORE const char * IDInstanceNumber =    "InstanceNumber";
EXPORT_CORE const char * IDSliceLocation = "SliceLocation";
EXPORT_CORE const char * IDSeriesNumber = "SeriesNumber";
EXPORT_CORE const char * IDModality =          "Modality";
EXPORT_CORE const char * IDPatientOrientation ="PatientOrientation";
EXPORT_CORE const char * IDPatientPosition = "PatientPosition";
EXPORT_CORE const char * IDSmallestImagePixelValue = "SmallestImagePixelValue";
EXPORT_CORE const char * IDLargestImagePixelValue = "LargestImagePixelValue";
EXPORT_CORE const char * IDStudyID = "StudyID";
EXPORT_CORE const char * IDProtocolName = "ProtocolName"; 

EXPORT_CORE const char * IDMediaStorageSOPClassUID= "MediaStorageSOPClassUID";
EXPORT_CORE const char * IDStudyDescription = "StudyDescription";
EXPORT_CORE const char * IDSamplesPerPixel = "IDSamplesPerPixel";
EXPORT_CORE const char * IDSeriesDescription = "SeriesDescription";

EXPORT_CORE const char * IDTestValue = "TestValue";
EXPORT_CORE const char * IDTransferSyntaxUID = "TransferSyntaxUID";
EXPORT_CORE const char * IDSOPClassUID = "SOPClassUID";


EXPORT_CORE const char * IDAcquisitionTime = "AcquisitionTime";

EXPORT_CORE const char * IDPositionerPrimaryAngle = "PositionerPrimaryAngle"; 
EXPORT_CORE const char * IDPositionerSecondaryAngle = "PositionerSecondaryAngle"; 
EXPORT_CORE const char * IDImagerPixelSpacing = "ImagerPixelSpacing"; 
EXPORT_CORE const char * IDDistanceSourceToDetector = "DistanceSourceToDetector"; 
EXPORT_CORE const char * IDDistanceSourceToPatient = "DistanceSourceToPatient"; 
EXPORT_CORE const char * IDPixelIntensityRelationship = "PixelIntensityRelationship"; 

EXPORT_CORE const char * IDPositionerPrimaryAngleIncrement = "PositionerPrimaryAngleIncrement"; 
EXPORT_CORE const char * IDPositionerSecondaryAngleIncrement = "PositionerSecondaryAngleIncrement"; 
EXPORT_CORE const char * IDSliceThickness = "SliceThickness";
EXPORT_CORE const char * IDSpacingBetweenSlices = "SpacingBetweenSlices"; 
EXPORT_CORE const char * IDPhotometricInterpretation = "PhotometricInterpretation";

EXPORT_CORE const char * IDAttrPixelSizeIsImager = "PixelSizeIsImagerSize"; 
EXPORT_CORE const char * IDRescaleIntercept = "RescaleIntercept"; 
EXPORT_CORE const char * IDRescaleSlope = "RescaleSlope"; 

NS_MIA_END
