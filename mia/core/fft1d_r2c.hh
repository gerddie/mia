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

#ifndef mia_core_fft1d_r2c_hh
#define mia_core_fft1d_r2c_hh

#include <mia/core/defines.hh>
#include <vector>
#include <complex>
#include <fftw3.h>

NS_MIA_BEGIN

/**
   \ingroup basic

   \brief a class to real-to-complex 1D FFTs

   Class to run a 1D real-to-complex FFT and its inverse. This class makes use of fftw.
   The result of a  forward transform followed directly by a backward transform is the input
   scaled by the size of the input.
*/

class EXPORT_CORE CFFT1D_R2C
{
public:
       struct Complex : public std::complex<float> {
              Complex(): std::complex<float>(0.0, 0.0) {}
              Complex(const std::complex<float>& other): std::complex<float>(other) {}
#if !defined(FFTW_NO_Complex) && defined(_Complex_I) && defined(complex) && defined(I)
              Complex(fftwf_complex& v):
                     std::complex<float>(v)
              {
              }
#else
              Complex(fftwf_complex& v):
                     std::complex<float>(v[0], v[1])
              {
              }
#endif
       };

       /// A typedef that makes switching between single precicion and double precicion easier
       typedef float          Real;

       /**
          Construtor to create the needed fftw structures for the FFT of a real input vector of
          size \a n
        */

       CFFT1D_R2C(size_t n);
       ~CFFT1D_R2C();

       /**
          Execute forward transformation. input data must be of size \a n that was given at construction time
          \returns the transformed in halfcomplex format (i.e. only the non-negative coefficints, since the
          fft is sysetric
        */
       std::vector<Complex> forward(const std::vector<Real>& data) const;

       /**
          Execute backward transformation. input data must be of size \f$\frac{n}{2} + 1\f$ with n as
          given at construction time
          \returns the backward transformed series
        */
       std::vector<Real>    backward(const std::vector<Complex>& data) const;

       /**
          Execute forward transformation. distance(in_begin, in_end) must be equal to \a n as given at construction time
          \param in_begin
          \param in_end
          \param out_begin output iterator pointing at the output range that must be at least
          of size \f$\frac{n}{2} + 1\f$ with n as given at construction time
          \sa out_size
        */
       void forward(std::vector<Real>::const_iterator in_begin,
                    std::vector<Real>::const_iterator in_end,
                    std::vector<Complex>::iterator out_begin) const;


       /**
          Execute forward transformation. distance(in_begin, in_end) must be equal to \f$\frac{n}{2} + 1\f$ with n as
          given at construction time
          \param in_begin
          \param in_end
          \param out_begin output iterator pointing at the output range that must be at least
          of size \a n as given at construction time
          \sa out_size
        */
       void backward(std::vector<Complex>::const_iterator in_begin,
                     std::vector<Complex>::const_iterator in_end,
                     std::vector<Real>::iterator out_begin) const;

       /// \returns the size of the forward transform result (i.e. \f$\frac{n}{2} + 1\f$) complex numbers.
       size_t out_size() const;

private:
       struct CFFT1D_R2CImpl *impl;

};

NS_MIA_END

#endif
