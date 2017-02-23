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

#include <stdexcept>
#include <climits>
#include <cstdlib>

#include <mia/internal/autotest.hh>
#include <mia/core/msgstream.hh>
#include <mia/2d/interpolator.hh>


NS_MIA_USE
using namespace std;

namespace bmpl=boost::mpl;

struct TestIntegral2DFixture {

	void check(double x0, double xF, double s1, double s2, double n, int n1, int n2);
private:
	double computeIntegralAA(double x0, double xF, double s1, double s2, double n, int q1, int q2)const;
	double computeIntegralBB(double x0, double xF, double s1, double s2, double n, int n1, int n2)const;
	double computeIntegralXX(double x0, double xF, double s1, double s2, int q1, int q2)const;
	double nChooseK(int pN, int pK)const;
};

BOOST_FIXTURE_TEST_CASE( test_integral, TestIntegral2DFixture)
{
	check(0, 30, 1, 1, 30, 2, 0);
	check(0, 30, 1, 0, 30, 1, 1);
	check(0, 30, 1, 1, 30, 1, 1);
	check(0, 30, 1, 0, 30, 2, 0);
	check(0, 30, 1, 0, 30, 0, 2);
	check(0, 30, 1, 2, 30, 0, 2);
	check(0, 30, 1, 3, 30, 0, 2);
	check(0, 30, 1, 3, 30, 2, 0);
	check(0, 30, 1, 4, 30, 2, 0);
	check(0, 30, 1, 1, 15, 2, 0);

	check(0, 30, 2, 2, 5, 2, 0);
	check(0, 30, 2, 2, 5, 1, 1);


	check(0, 30, 10, 10, 1, 2, 0);
	check(0, 30, 10, 10, 1, 1, 1);

	check(0, 30, 28, 29, 1, 2, 0);

	check(0, 9, 0, 1, 1, 0, 2);
	check(0, 9, 0, 1, 1, 2, 0);

}

void TestIntegral2DFixture::check(double x0, double xF, double s1, double s2, double n, int n1, int n2)
{
	auto kernel = CSplineKernelPluginHandler::instance().produce("bspline:d=3"); 
	double spregval = computeIntegralAA(x0, xF, s1, s2, n, n1, n2);
	double mycode = integrate2(*kernel, s1, s2, n1, n2, n, x0, xF);
	cvdebug() << "mycode = " << mycode << ", spregval = "  << spregval << "\n";
	BOOST_CHECK_CLOSE(mycode,  spregval, 1);

}

double TestIntegral2DFixture::computeIntegralAA(double x0, double xF, double s1, double s2, double n, int q1, int q2)const
{
	//TRACE("divCurlRegularizerPlugin::computeIntegralAA");

        // Computes the following integral
        //
        //           xF d^q1      3  x        d^q2    3  x
        //  integral    -----   B  (--- - s1) ----- B  (--- - s2) dx
        //           x0 dx^q1        h        dx^q2      h

        // Form the spline coefficients
        const double	C[3][3] = { {1,  0,  0 }, {1,  -1,  0 }, {1,  -2,  1 }};
        const int	d[3][3] = { {3,  0,  0 }, {2,   2,  0 }, {1,   1,  1 }};
        const double	s[3][3] = { {0,  0,  0 }, { -0.5, 0.5,  0 }, {1,   0,  -1 }};

        // Compute the integral
        double integral=0;
        for (int k=0; k<3; k++) {
                double ck=C[q1][k];
                if (ck==0) continue;
                for (int l=0; l<3; l++) {
                        double cl=C[q2][l];
                        if (cl==0) continue;
                        integral+=ck*cl*computeIntegralBB(x0,xF,s1+s[q1][k],s2+s[q2][l],n, d[q1][k],d[q2][l]);
                }
        }
	cvdebug() << "Int[" << x0 << ", " << xF << "] = "
		  << ", s1=" << s1
		  << ", q1=" << q1
		  << ", s2=" << s2
		  << ", q2=" << q2
		  << "] = " << integral << "\n";

        return integral;
}

double TestIntegral2DFixture::computeIntegralBB(double x0, double xF, double s1, double s2, double n, int n1, int n2)const
{

	//	TRACE("divCurlRegularizerPlugin::computeIntegralBB");
        // Computes the following integral
        //
        //           xF   n1  x          n2  x
        //  integral     B  (--- - s1)  B  (--- - s2) dx
        //           x0       h              h
	// in this case: h = 1

	double x0p = x0 / n;
	double xFp = xF / n;

        // Form the spline coefficients
	vector<double> c1(n1+2);

        double fact_n1=1;
        for (int k = 2; k <= n1; k++)
		fact_n1 *= k;

        double sign=1;
        for (int k=0; k <= n1+1; k++, sign *= -1)
                c1[k]=sign*nChooseK(n1+1,k)/fact_n1;

        vector<double> c2(n2+2);

	double fact_n2=1;
	for (int k = 2; k <= n2; k++)
		fact_n2 *= k;

        sign = 1;
        for (int k = 0; k <= n2+1; k++, sign *= -1)
                c2[k] = sign*nChooseK(n2+1,k)/fact_n2;

        // Compute the integral
        double n1_2 = (double)((n1+1))/2.0;
        double n2_2 = (double)((n2+1))/2.0;
        double integral = 0;
        for (int k = 0; k <= n1+1; k++) {
                for (int l = 0; l <= n2+1; l++) {
                        integral += c1[k]*c2[l]*computeIntegralXX(x0p,xFp,s1+k-n1_2,s2+l-n2_2,n1,n2);
                }
        }


        return integral * n;
}

double TestIntegral2DFixture::computeIntegralXX(double x0, double xF, double s1, double s2, int q1, int q2)const
{
	//	TRACE("divCurlRegularizerPlugin::computeIntegralXX");
        // Computation of the integral
        //             xF          q1       q2
        //    integral       (x-s1)   (x-s2)     dx
        //             x0          +        +

        // Change of variable so that s1 is 0
        // X=x-s1 => x-s2=X-(s2-s1)
        double s2p=s2-s1;
        double xFp=xF-s1;
        double x0p=x0-s1;

        // Now integrate
        if (xFp < 0) return 0;

        // Move x0 to the first point where both integrands
        // are distinct from 0
        x0p=max(x0p, max(s2p,0.0));
        if (x0p > xFp) return 0;

        // There is something to integrate
        // Evaluate the primitive at xF and x0
        double IxFp=0;
        double Ix0p=0;
        for (int k=0; k <= q2; k++) {
                double aux=nChooseK(q2,k)/(double)(q1+k+1)*pow(-s2p,q2-k);
                IxFp+=pow(xFp,q1+k+1)*aux;
                Ix0p+=pow(x0p,q1+k+1)*aux;
        }
        return IxFp-Ix0p;
}

double TestIntegral2DFixture::nChooseK(int pN, int pK)const
{
	//	TRACE("divCurlRegularizerPlugin::nChooseK");
        int n = pN;
        int k = pK;
        if (k > n) return 0;
        if (k > n/2) k = n-k;
        if (k == 0) return 1;
        if (k == 1) return n;
        double prod = 1;
        for (int i = 1; i <= k; i++)
                prod *= ((double)(n-k+i))/i;
        return prod;
}
