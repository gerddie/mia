/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
 * Max-Planck-Institute for Human Cognitive and Brain Science
 * Max-Planck-Institute for Evolutionary Anthropology
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PUcRPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifndef mia_core_factorycmdlineoption_hh
#define mia_core_factorycmdlineoption_hh

#include <mia/core/cmdlineparser.hh>
#include <mia/core/factory.hh>

NS_MIA_BEGIN

/**
   Command line option that uses a factory singleton to create the target object from 
   the parsed command line parameter 
   \tparam F the factory type that must define a type F::ProductPtr, must provide a
   funcion \a instance() that returns an instance of the factory, and must provide a 
   method \a produce that takes a std::string or const char * as argument and returns 
   a ProductPtr object. 
   ProductPtr must be a pointer like object that holds and instance of a type T of the 
   the created object.  Type T in turn must provide a method get_init_string() that returns 
   the string that was passed to the \a produce method in order to create this object. 
   \todo use cocept checks to test these class requirements 
*/

template <typename F>
class TCmdFactoryOption: public  CCmdOptionValue{

public:
        /** Constructor of the command option
	    \retval val variable to hold the parsed option value - pass in the default value -
	    \param short_opt short option name (or 0)
	    \param long_opt long option name (must not be NULL)
	    \param long_help long help string (must not be NULL)
	    \param short_help short help string
	    \param required if this is set to true, extra checking will be done weather
	    the option is really set
	*/
	TCmdFactoryOption(typename F::ProductPtr& val, char short_opt, const char *long_opt,
			   const char *long_help, const char *short_help, CCmdOption::Flags flags);
private:
	virtual bool do_set_value_really(const char *str_value);
	virtual size_t do_get_needed_args() const;
	virtual void do_write_value(std::ostream& os) const;
	virtual void do_get_long_help_really(std::ostream& os) const;
	virtual const std::string do_get_value_as_string() const;

	typename F::ProductPtr& _M_value;
};


// implementation details 

template <typename F>
TCmdFactoryOption<F>::TCmdFactoryOption(typename F::ProductPtr& val, char short_opt, 
					const char *long_opt,
					const char *long_help, const char *short_help, 
					CCmdOption::Flags flags):
	CCmdOptionValue( short_opt, long_opt, long_help, short_help, flags ),
	_M_value( val )
{
}

template <typename F>
bool TCmdFactoryOption<F>::do_set_value_really(const char *str_value)
{
	_M_value = F::instance().produce(str_value); 
	return _M_value; 
}

template <typename F>
size_t TCmdFactoryOption<F>::do_get_needed_args() const
{
	return 1; 
}

template <typename F>
void TCmdFactoryOption<F>::do_write_value(std::ostream& os) const
{
	if (_M_value) 
		os << "=" << _M_value->get_init_string(); 
	else 
		if (is_required())
			os << "[required]"; 
		else
			os << "NULL"; 
}

template <typename F>
void TCmdFactoryOption<F>::do_get_long_help_really(std::ostream& os) const
{
	os << ", set option to 'help' for more information."; 
}

template <typename F>
const std::string TCmdFactoryOption<F>::do_get_value_as_string() const
{
	if (_M_value) {
		stringstream msg; 
		msg << "'" << _M_value->get_init_string() << "'"; 
		return msg.str();
	}
	else 
		return "''"; 
}

template <typename T>
PCmdOption make_opt(typename std::shared_ptr<T>& value, const char *long_opt, char short_opt,
		    const char *long_help, const char *short_help, 
		    CCmdOption::Flags flags= CCmdOption::not_required)
{
	typedef typename FactoryTrait<T>::type F;  
	return PCmdOption(new TCmdFactoryOption<F>(value, short_opt, long_opt,
						   long_help, short_help, flags ));
}

template <typename T>
PCmdOption make_opt(typename std::shared_ptr<T>& value, const char *long_opt, char short_opt,
		    const char *long_help, CCmdOption::Flags flags= CCmdOption::not_required)
{
	typedef typename FactoryTrait<T>::type F;  
	return PCmdOption(new TCmdFactoryOption<F>(value, short_opt, long_opt,
						   long_help, long_opt, flags ));
}


NS_MIA_END

#endif
