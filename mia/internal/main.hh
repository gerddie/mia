/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2011 Gert Wollny
 *
 * This program is free software; you can redistribute it and/or modify
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <stdexcept>
#include <iostream>
#include <mia/core/defines.hh>


#define MIA_MAIN(callback) \
	int main( int argc, const char *argv[] ) \
	{					 \
	        try {				 \
  	              return do_main(argc, argv);\
	        }                                \   
                 catch (const std::runtime_error &e)                                 \
			 std::cerr << argv[0] << " runtime error: " << e.what() << endl; \
 	        }                                                                    \
 	        catch (const std::invalid_argument &e){                              \
		        std::cerr << argv[0] << " invalid argument: " << e.what() << endl;\
	        }                                                                    \
 	        catch (const std::logic_error &e){                                   \
		        std::cerr << argv[0] << " logic error: " << e.what() << endl;\
	        }                                                                    \
	        catch (const std::exception& e){			             \
		        std::cerr << argv[0] << " error: " << e.what() << endl;      \
	        }                                                                    \
	        catch (...){                                                         \
		       std::cerr << argv[0] << " unknown exception" << endl;         \
	}                                                                            \ 
	return EXIT_FAILURE;                                                         \ 
}
