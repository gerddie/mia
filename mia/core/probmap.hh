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

#ifndef __probmap_hh
#define __probmap_hh

#include <vector>
#include <string>
#include <map>
#include <iostream>

#include <mia/core/defines.hh>


typedef std::vector<double> double_vector;

NS_MIA_BEGIN

/**
   \ingroup misc
   \brief A vector of probablility values.

   A vector of probablility values that supports loading and storing
   the data.
 */
class EXPORT_CORE CProbabilityVector: public std::vector<double_vector>
{
public:

       /**
          standard constructor
       */
       CProbabilityVector();


       /**
          Constructor using describing the probability vector
          \param nClass number of probablilities per element
          \param nElm numer of elments (aka intensities)
        */

       CProbabilityVector(size_t nClass, size_t nElm);

       /**
          Loads a probability map from a file, throws an exception of the operation fails.
          \param in_fname input file name "-" reads from stdin
          \returns the probability map
       */
       CProbabilityVector(const std::string& in_fname);
       /**
          Saves a probability map to a file,
          \param out_fname output filemake, "-" uses stdout
          \returns \a true if successfull, \a false otherwise

       */
       bool save(const std::string& out_fname) const;

private:
       bool do_save(std::ostream& os) const;
       void do_load(std::istream& is);
};


/// Compare two probability vectors
EXPORT_CORE bool operator == (const CProbabilityVector& a, const CProbabilityVector& b);


/**
   \ingroup misc
   \brief A map to change labels.

   This class implements a mapping to change labels stemming from out-of-core
   region growing.
*/
class EXPORT_CORE CLabelMap: public std::map<size_t, size_t>
{
public:
       CLabelMap();

       /**
          Constructthe label map by reading from a file simple text file
          @param in_fname input file name
        */
       CLabelMap(const std::string& in_fname);

       /**
          Save the label map to a file
          @param out_fname output file name
          @returns \a true if saving was successfull
        */
       bool save(const std::string& out_fname)const ;
private:
       bool do_save(std::ostream& os) const;
       void do_load(std::istream& is);
};

/// element wise equal operator for a CLabelMap
EXPORT_CORE bool operator == (const CLabelMap& a, const CLabelMap& b);

NS_MIA_END

#endif
