/*
*
* Copyrigh (C) 2004-2012 Gert Wollny 
*  
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU Lesser Public License as published by
* the Free Software Foundation; either version 3 of the License, or
* (at your option) any later version.
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU Lesser Public License for more details.
* 
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software 
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/
/*! \brief storing and loading critical points.
   Declarations for storing/loading critical points in the vista file format.
 */

#ifndef __VCPEList_h
#define __VCPEList_h

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <vistaio/vista.h>



#ifdef __cplusplus
extern "C" {
#endif

	/** all eigenvalues are zero */
#define EV_ZERO              0
	/** all eigenvalues are real */
#define EV_REAL              1
	/** all eigenvalues are real and two are equal */
#define EV_REAL_TWO_EQUAL    2
	/** all eigenvalues are real and all are equal */
#define EV_REAL_THREE_EQUAL  3
	/** two eigenvalues are conjugated complex */
#define EV_COMPLEX	     4
	
	/*! \brief VistaIOCPERec is the structute to hold a critical point. */	
	typedef struct VistaIOCPERec {
		float location[3]; /*!< 3D coordiantes of the critical point */
		float portrait[9]; /*!< Matrix A of the critical point       */
		float eval1;       /*!< first  eigenvalue of critical point this on is always real  */
		float eval2;       /*!< second eigenvalue of critical point or if two eigenvalues are conjugated complex the real part */
		float eval3;       /*!< third  eigenvalue of critical point or if two eigenvalues are conjugated complex imaginary part */ 
		float evec1[3];    /*!< first eigenvector, always real */
		float evec2[3];    /*!< second eigenvector or if two complex eigenvalues then the real part of the eigenvector */ 
		float evec3[3];    /*!< second eigenvector or if two complex eigenvalues then the imaginary part of the eigenvector */
		unsigned char typeM; /*!< type of the eigenvalues */
	}VistaIOCPERec;

	/*! \brief A pointer to a single critical point. */	
	typedef  VistaIOCPERec *VCPE;
	
	
	/*! \brief Vista data type to hold the list of critical points */	
	typedef struct VistaIOFieldCPListRec {
		VistaIOLong n_length;  /*!< number of critical points in list */
		VistaIOAttrList attr;  /*!< vista attribute list */
		VistaIOPointer data;   /*!< pointer to the actual data */
		VistaIOLong nsize;     /*!< size of data block in bytes \a data points to */
	} VistaIOCPEListRec;
	
	/*! \brief  Definition of the pointer how it is used in the vista library.*/ 
	typedef VistaIOCPEListRec *VistaIOCPEList;
	
	/** This function creates a critical point list. 
	    \param _n_element number of elements in the list
	    \retval An empty VistaIOCPEList with space for \a _n_element critical points
	*/
	VistaIOCPEList VistaIOCreateCPEList(VistaIOLong _n_element);
	
	/** name of the length attribute in the vista file */
#define CPLIST_LENGTH  "n_length"
	/** This function has to be called before using VistaIOCPEList objects with original vista functions, 
	    to make the data type available.
	    \remark as a side effect VistaIOCPEListRepn is set. 
	*/
	void VistaIORegisterCPEList();

#ifdef __cplusplus
}
#endif
			   
#endif

