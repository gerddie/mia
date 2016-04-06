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

#include <stdexcept>
#include <iostream>
#include <cstdlib>
#include <mia/core/msgstream.hh>


#define MIA_MAIN(callback)						\
	int main( int argc, char *argv[] )				\
	{								\
	        try {							\
			auto verb = getenv("MIA_INITIAL_VERBOSITY");	\
		      if (verb) {                                       \
			      auto level = mia::g_verbose_dict.get_value(verb); \
			      mia::vstream::instance().set_verbosity(level); \
                      }                                                 \
  	              return callback(argc, argv);			\
	        }                                                       \
		catch (const std::runtime_error &e){			\
			std::cerr << "\n" << argv[0] << " runtime error: " << e.what() << std::endl; \
 	        }							\
 	        catch (const std::invalid_argument &e){			\
		        std::cerr << "\n"  << argv[0] << " invalid argument: " << e.what() << std::endl;	\
	        }							\
 	        catch (const std::logic_error &e){			\
		        std::cerr << "\n"  << argv[0] << " logic error: " << e.what() << std::endl; \
	        }							\
	        catch (const std::exception& e){			\
		        std::cerr << "\n"  << argv[0] << " error: " << e.what() << std::endl; \
	        }							\
	        catch (...){						\
			std::cerr << "\n"  << argv[0] << " unknown exception" << std::endl; \
		}							\
		return EXIT_FAILURE;					\
	}
