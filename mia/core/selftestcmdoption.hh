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

#ifndef mia_core_selftestcmdoption_hh
#define mia_core_selftestcmdoption_hh

#include <mia/core/cmdoption.hh>
#include <memory>

NS_MIA_BEGIN

/**
   The base class for the selftest callback. 

   The self test class provides the interface to add a selftest option 
   that invokes the tests for normal programs. 
   
   For a working implementation the abstract method 
 
   int do_run(int argc, char **argv) const; 
 
   must be overridden. It must return zero if the tests pass and non-zero if they don't. 
*/
struct EXPORT_CORE CSelftestCallback {
        /**
           Constructor of the callback function. Usually you can just inherit it 
           by the C++11 "using Callback::Callback" directive. 
           
           \param argc number of following arguments 
           \param argv an array of string arguments
        */
        CSelftestCallback(int argc, char **argv); 
        
 private: 
        friend class CSelftestOption; 
        
        /** runs the test suite 
            \returns 0 if all tests were successfull and non-zero otherwise
        */
        int run() const; 
        
        /**
           Interface for the callback function to be overridden.
        */
        virtual int do_run(int argc, char **argv) const = 0; 
        
        int m_argc; 
        char **m_argv; 
}; 

/**
   This define creates an derived selftest class that runs a BOOST test suite. 
   As usual, the boost tests have to be defined by using BOOST_*_TEST_CASE. 
   
   In order to use this test case an instance of this class created with \a new 
   must be passed to the command line parser by using the add_selftest method.
   \param NAME name of the selftest class. 
   
 */
#define SELFTEST_CALLBACK(NAME)  class NAME: public CSelftestCallback { \
        public:                                                         \
        using CSelftestCallback::CSelftestCallback;                     \
                                                                        \
        private:                                                        \
        int do_run(int argc, char **argv)const {                        \
                return ::boost::unit_test::unit_test_main( &init_unit_test, argc, argv); \
        }                                                               \
}

NS_MIA_END

#endif 
