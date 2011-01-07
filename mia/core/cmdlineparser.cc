/* -*- mona-c++  -*-
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

#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <map>
#include <ctype.h>
#include <stdexcept>
#include <mia/core/msgstream.hh>
#include <mia/core/cmdlineparser.hh>

extern void print_full_copyright(const char *name);


NS_MIA_BEGIN
using namespace std;

CCmdOption::CCmdOption(const char *long_opt, const char *long_help, Flags flags):
	_M_long_opt(long_opt),
	_M_long_help(long_help), 
	_M_flags(flags)
{
        assert(long_opt);
        assert(long_help);
}

CCmdOption::~CCmdOption()
{
}

size_t CCmdOption::get_needed_args() const
{
	return do_get_needed_args();
}


void CCmdOption::add_option(CShortoptionMap& sm, CLongoptionMap& lm)
{
	do_add_option(sm, lm);
}

void CCmdOption::print_short_help(std::ostream& os) const
{
	do_print_short_help(os);
}

void CCmdOption::get_long_help(std::ostream& os) const
{
	do_get_long_help(os);
}

const char *CCmdOption::long_help() const
{
	return _M_long_help;
}

const char *CCmdOption::get_long_option() const
{
	return _M_long_opt;
}

void CCmdOption::clear_required()
{
	_M_flags = static_cast<CCmdOption::Flags>(_M_flags & (~required)); 
}

bool CCmdOption::is_required() const
{
	return (_M_flags & required ) == required; 
}

void CCmdOption::do_get_long_help(std::ostream& /* os */) const
{
}

void CCmdOption::get_opt_help(std::ostream& os) const
{
	do_get_opt_help(os);
}

void CCmdOption::do_get_opt_help(std::ostream& os) const
{
	os << _M_long_help;
}

void   CCmdOption::write_value(std::ostream& os) const
{
	do_write_value(os);
}

void CCmdOption::set_value(const char *str_value)
{
	do_set_value(str_value);
}

const std::string CCmdOption::get_value_as_string() const
{
	return do_get_value_as_string();
}

const std::string CCmdOption::do_get_value_as_string() const
{
	return "";
}

struct CCmdOptionData {
	CCmdOptionData(char short_opt, const char *short_help);
	char _M_short_opt;
	const char *_M_long_opt;
	const char *_M_short_help;
};

CCmdOptionData::CCmdOptionData(char short_opt, const char *short_help):
	_M_short_opt(short_opt),
	_M_short_help(short_help)
{
}

CCmdOptionValue::CCmdOptionValue(char short_opt, const char *long_opt, 
				 const char *long_help, 
				 const char *short_help, Flags flags):
	CCmdOption(long_opt, long_help, flags),
	_M_impl(new CCmdOptionData(short_opt,  short_help))
{
}


char CCmdOptionValue::get_short_option() const
{
	return _M_impl->_M_short_opt;
}

const char *CCmdOptionValue::get_short_help() const
{
	return _M_impl->_M_short_help;
}

void CCmdOptionValue::do_print_short_help(std::ostream&   os  ) const
{
	if ( get_short_option() ) {
		os << '-' << get_short_option(); 
		if (  get_short_help() )
			os << " " << get_short_help();
		os  << " ";
	}
}

void CCmdOptionValue::do_get_long_help(std::ostream& os) const
{
	os << long_help();
	do_get_long_help_really(os);
}

void CCmdOptionValue::do_get_long_help_really(std::ostream& /*os*/) const
{

}


void CCmdOptionValue::do_get_opt_help(std::ostream& os) const
{
	if ( get_short_option() )
		os  << "  -" << get_short_option() << " ";
	else
		os  << "     ";
	os << "--" << get_long_option();
	write_value(os);
	os << " ";
}

void CCmdOptionValue::do_set_value(const char *str_value)
{
	bool result;

	try {
		result = do_set_value_really(str_value);
	}

	catch (std::invalid_argument& x) {
		stringstream msg;
		msg << "Error parsing --" << get_long_option() << ":" << x.what();
		throw std::invalid_argument(msg.str());
	}

	if (!result) {
		throw invalid_argument(string("Error parsing option '") +
				       string(get_long_option()) +
				       string("' from input '") +
				       string(str_value) + string("'"));
	}
	clear_required();
}

void CCmdOptionValue::do_add_option(CShortoptionMap& sm, CLongoptionMap& lm)
{
	TRACE_FUNCTION;
	if (get_short_option() != 0) {
		assert(sm.find(get_short_option()) == sm.end() &&
		       "don't add the same short option twice" );
		sm[get_short_option()] = this;
	}

	if (get_long_option() != 0) {
		assert(lm.find(get_long_option()) == lm.end() &&
		       "don't add the same long option twice");
		lm[get_long_option()] = this;
	}
}

CCmdOptionValue::~CCmdOptionValue()
{
	delete _M_impl;
}

struct CCmdOptionListData {
	CShortoptionMap short_map;
	CLongoptionMap  long_map;
	typedef vector<PCmdOption> COptionsGroup;
	typedef map<string, COptionsGroup > COptionsMap;
	COptionsMap options;
	COptionsMap::iterator current_group;
	vector<const char *> remaining;

	bool help;
	bool usage;
	bool copyright;
	vstream::Level verbose;

	CCmdOptionListData(const string& general_help);

	void add(PCmdOption opt);
	CHistoryRecord get_values() const;

	void set_current_group(const string& name);

	CCmdOption *find_option(const char *key) const;
	CCmdOption *find_option(char key) const;

	void print_help() const;
	void print_usage() const;

	vector<const char *> has_unset_required_options() const; 
	void write(size_t tab1, size_t width, const string& s)const; 

	string _M_general_help; 
};


CCmdSetOption::CCmdSetOption(std::string& val, const std::set<std::string>& set, 
			     char short_opt, const char *long_opt, const char *long_help,
			     const char *short_help, Flags flags):
	CCmdOptionValue(short_opt, long_opt, long_help, short_help,  flags),
	_M_value(val),
	_M_set(set)
{
}


bool CCmdSetOption::do_set_value_really(const char *str_value)
{
	if (_M_set.find(str_value) == _M_set.end())
		return false;
	_M_value = str_value;
	return true;
}

size_t CCmdSetOption::do_get_needed_args() const
{
	return 1;
}

void CCmdSetOption::do_write_value(std::ostream& os) const
{
	os << "=" << _M_value;
}

void CCmdSetOption::do_get_long_help_really(std::ostream& os) const
{
	if (_M_set.size() > 0) {
		os << "\n(" ;
		std::set<std::string>::const_iterator i = _M_set.begin();
		os << *i;
		++i;
		while ( i != _M_set.end() )
			os << '|' << *i++;
		os  << ")";
	}
}

const std::string CCmdSetOption::do_get_value_as_string() const
{
	return _M_value;
}

const char *g_help_optiongroup="\nHelp & Info"; 
const char *g_basic_copyright = 
        "\n"
	" This software is copyright (c) Gert Wollny et al.\n"
	" It comes with  ABSOLUTELY NO WARRANTY and you may redistribute it\n"
	" under the terms of the GNU GENERAL PUBLIC LICENSE Version 3 (or later).\n"
	" For more information run the program with the option '--copyright'\n"; 

CCmdOptionListData::CCmdOptionListData(const string& general_help):
	help(false),
	usage(false),
	copyright(false),
	verbose(vstream::ml_warning), 
	_M_general_help(general_help)
{
	options[""] = vector<PCmdOption>();

	set_current_group(g_help_optiongroup);
	add(make_opt(verbose, g_verbose_dict, "verbose",  'V', "verbosity of output", 
		     CCmdOption::not_required));
	add(make_opt(copyright,  "copyright", 0, "print copyright information", 
		     CCmdOption::not_required));
	add(make_opt(help,  "help", 'h', "print this help", 
		     CCmdOption::not_required));
	add(make_opt(usage,  "usage", '?', "print a short help", 
		     CCmdOption::not_required));
	set_current_group("");
}

void CCmdOptionListData::add(PCmdOption opt)
{
	opt->add_option(short_map, long_map);
	current_group->second.push_back(opt);
}

CCmdOption *CCmdOptionListData::find_option(const char *key) const
{
	CLongoptionMap::const_iterator i = long_map.find(key);
	return (i != long_map.end()) ? i->second : 0;
}

CCmdOption *CCmdOptionListData::find_option(char key) const
{
	CShortoptionMap::const_iterator i = short_map.find(key);
	return (i != short_map.end()) ? i->second : 0;
}

void CCmdOptionListData::set_current_group(const string& name)
{
	current_group = options.find(name);
	if (current_group == options.end()) {
		options[name] = vector<PCmdOption>();
		current_group = options.find(name);
	}
}

CHistoryRecord CCmdOptionListData::get_values() const
{
	CHistoryRecord result;
	for(auto o_i = options.begin(); o_i != options.end(); ++o_i)
		for(auto g_i = o_i->second.begin(); g_i != o_i->second.end(); ++g_i) 
			result[(*g_i)->get_long_option()] = (*g_i)->get_value_as_string();
	return result;
}


vector<const char *> CCmdOptionListData::has_unset_required_options() const  
{
	vector<const char *> result; 
	for(auto o_i = options.begin(); o_i != options.end(); ++o_i)
		for(auto g_i = o_i->second.begin(); g_i != o_i->second.end(); ++g_i)
			if ((*g_i)->is_required())
				result.push_back((*g_i)->get_long_option()); 
	return result; 
}


void CCmdOptionListData::write(size_t tab1, size_t width, const string& s) const 
{
	auto is = s.begin(); 
	auto es = s.end(); 
	bool newline = false; 
	size_t pos = 0; 
	while (is != es) {
		if (newline) {
			if (tab1) 
				clog << setw(tab1) << " "; 
			newline = false; 
			pos = tab1; 
		}
		if (*is == '\n' || 
		    (pos >= width && isspace(*is))) {
			clog << '\n'; 
			newline = true; 
		}else {
			clog << *is; 
			++pos; 
		}
		++is; 
	}
	clog << endl; 
}

/**
   This help printing is a mess ...
 */
void CCmdOptionListData::print_help() const
{
	const size_t max_opt_width = 30;
	// this should come from the terminal 
	const size_t max_width = 70;

	vector<string> opt_table;
	vector<string> help_table;
	
	write(0, max_width, _M_general_help); 
	write(0, max_width, "\nThe program supports the following command line options:"); 

	size_t opt_size = 0;
	clog << setiosflags(ios_base::left);
	for (auto i = options.begin(); i != options.end(); ++i) {

		stringstream group;
		opt_table.push_back(i->first);
		help_table.push_back("  ");
		
		for (auto g_i = i->second.begin(); g_i != i->second.end(); ++g_i) {
			stringstream opt;
			stringstream shelp;

			const PCmdOption& k = *g_i;
			k->get_opt_help(opt);

			opt_table.push_back(opt.str());
			size_t size = opt.str().size();
			if (opt_size < size)
				opt_size = size;

			k->get_long_help(shelp);
			help_table.push_back(shelp.str());

		}
	}
	if (opt_size > max_opt_width)
		opt_size = max_opt_width;

	auto t  = opt_table.begin();
	for (auto i = help_table.begin(); i != help_table.end(); ++i, ++t) {
		clog << setw(opt_size) << *t << " ";
		if (t->length() > opt_size) 
			clog << "\n " << setw(opt_size) << " "; 
		write(opt_size+1, max_width, *i); 
	}
	
	write(0, 80, g_basic_copyright); 
	clog << '\n' << setiosflags(ios_base::right);


}

void CCmdOptionListData::print_usage() const
{
	clog << "Usage: ";
	for (COptionsMap::const_iterator i = options.begin();
	     i != options.end(); ++i) {
		COptionsGroup::const_iterator g_i = i->second.begin();
		COptionsGroup::const_iterator g_e = i->second.end();

		while (g_i != g_e) {

			const PCmdOption& k = *g_i;
			k->print_short_help(clog);
			++g_i;
		}
	}
	clog << '\n';
}

CCmdOptionList::CCmdOptionList(const string& general_help):
	_M_impl(new CCmdOptionListData(general_help))
{
}

CCmdOptionList::CCmdOptionList():
	_M_impl(new CCmdOptionListData("This is a MIA toolchain program."))
{
}

void CCmdOptionList::push_back(PCmdOption opt)
{
	_M_impl->add(opt);
}

void CCmdOptionList::add(const std::string& /*table*/, PCmdOption opt)
{
	_M_impl->add(opt);
}

void CCmdOptionList::set_group(const std::string& group)
{
	_M_impl->set_current_group(group); 
}

int CCmdOptionList::handle_shortargs(const char *arg, size_t /*argc*/, const char *args[])
{
	bool bool_options_only = false;
	do {
		CCmdOption *opt = _M_impl->find_option(*arg);
		if (!opt ) {
			if ( bool_options_only ) {
				throw invalid_argument(string("bad flag combination:'-") + 
						       string(arg) + string("'"));
			}
			return -1;
		}

		++arg;
		size_t nargs = opt->get_needed_args();
		if (*arg) {
			switch (nargs) {
			case 0:
				opt->set_value(NULL);
				bool_options_only = true;
				break;
			case 1:
				opt->set_value(arg);
				bool_options_only = false;
				break;
			default:
				throw invalid_argument("need a space between option and "
						       "parameter for multiple parameters");
			}
		}else {
			opt->set_value(args[0]);
			return nargs;
		}
	} while (*arg && bool_options_only);

	return 0;
}

void CCmdOptionList::parse(size_t argc, char *args[], bool has_additional)
{
	parse(argc, (const char **)args, has_additional);
}

void CCmdOptionList::parse(size_t argc, const char *args[], bool has_additional)
{

	size_t idx = 1;
	while (idx < argc ) {
		const char *cur_arg = args[idx++];
		const char *ccarg = cur_arg;
		size_t remaining_args = argc - idx;

		// if we don't scan an option, we deal with a left over argument
		if (*ccarg != '-') {
			_M_impl->remaining.push_back(cur_arg);
			continue;
		}

		// we found a singular dash, this is also stored as a left over argument
		++ccarg;
		if (!*ccarg) {
			_M_impl->remaining.push_back(cur_arg);
			continue;
		}

		// now check if this is a long option 
		if( *ccarg == '-') { 
			++ccarg;
			CCmdOption *opt = _M_impl->find_option(ccarg);
			if (opt) {
				size_t nargs = opt->get_needed_args();
				// currently onyl one argument value is supported
				assert(nargs <= 1); 
				if (remaining_args < nargs ) {
					THROW(invalid_argument, opt->get_long_option() 
					      << ": requires " 
					      << nargs << " arguments, but only " << nargs 
					      << " remaining.");
				}
				
				opt->set_value(args[idx]);
				idx += nargs;
				continue;
			}
		} else {
			int r = handle_shortargs(ccarg, argc - idx, &args[idx]);
			if (r >= 0) {
				idx +=  r;
				continue;
			}
		}
		// current option was not found
		_M_impl->remaining.push_back(cur_arg);

	}

	if (_M_impl->help) {
		_M_impl->print_help();
		exit(0);
	} else if (_M_impl->usage) {
		_M_impl->print_usage();
		exit(0);
	} else if (_M_impl->copyright) {
		::print_full_copyright(args[0]);
		exit(0);
	}

	cverb.set_verbosity(_M_impl->verbose);
	if (!has_additional && !_M_impl->remaining.empty()) {
		stringstream msg; 
		msg << "Unknown options given: "; 
		for (auto i = _M_impl->remaining.begin(); _M_impl->remaining.end() != i; ++i)
			msg << " '" << *i << "' "; 
		throw invalid_argument(msg.str());
	}
	
	auto unset_but_required = _M_impl->has_unset_required_options(); 
	if (!unset_but_required.empty()) {
		stringstream msg; 
		if (unset_but_required.size() > 1) 
			msg << "Some required options were not set:"; 
		else
			msg << "A required options was not set:"; 
		for (auto i = unset_but_required.begin(); unset_but_required.end() != i; ++i)
			msg << " '--" << *i << "' "; 
		msg << "\n"; 
		msg << "run '" << args[0] << " --help' for more information\n"; 
		msg << g_basic_copyright; 
		throw invalid_argument(msg.str());
	}

}

const vector<const char *>& CCmdOptionList::get_remaining() const
{
	return _M_impl->remaining;
}

CCmdOptionList::~CCmdOptionList()
{
	delete _M_impl;
}

PCmdOption EXPORT_CORE make_opt(std::string& value, const std::set<std::string>& set,
				const char *long_opt, char short_opt,
				const char *long_help, const char *short_help, 
				CCmdOption::Flags flags)
{
	return PCmdOption(new CCmdSetOption(value, set, short_opt, long_opt, 
					    long_help, short_help, flags ));
}

PCmdOption EXPORT_CORE make_opt(std::string& value, const std::set<std::string>& set,
                                const char *long_opt, char short_opt, 
				const char *long_help, 
				CCmdOption::Flags flags)
{
	return PCmdOption(new CCmdSetOption(value, set, short_opt, long_opt, 
					    long_help, long_opt, flags ));
}


CHistoryRecord CCmdOptionList::get_values() const
{
	return _M_impl->get_values();
}

CHelpOption::CHelpOption(Callback *cb, char short_opt, const char *long_opt, const char *long_help):
	CCmdOptionValue(short_opt, long_opt, long_help, NULL, not_required), 
	_M_callback(cb)
{
}

void CHelpOption::print(std::ostream& os) const
{
	_M_callback->print(os);
}

bool CHelpOption::do_set_value_really(const char */*str_value*/)
{
	print(clog); 
	exit(0); 
}
size_t CHelpOption::do_get_needed_args() const
{
	return 0; 
}

void CHelpOption::do_write_value(std::ostream& /*os*/) const
{
}

CCmdFlagOption::CCmdFlagOption(int& val, const CFlagString& map, char short_opt, 
			       const char *long_opt, const char *long_help, 
			       const char *short_help, 
			       CCmdOption::Flags flags):
	CCmdOptionValue(short_opt, long_opt, long_help,short_help, flags),
	_M_value(val),
	_M_map(map)
{
}

void CCmdFlagOption::do_write_value(std::ostream& os) const
{
	os << _M_map.get(_M_value);
}

void CCmdFlagOption::do_get_long_help_really(std::ostream& os) const
{
	os << " supported flags:(" <<_M_map.get_flagnames() << os << ")";
}

const std::string CCmdFlagOption::do_get_value_as_string() const
{
	return _M_map.get(_M_value);
}


bool CCmdFlagOption::do_set_value_really(const char *str_value)
{
	_M_value = _M_map.get(str_value);
	return true;
}

size_t CCmdFlagOption::do_get_needed_args() const
{
	return 1;
}

PCmdOption EXPORT_CORE make_opt(int& value, const CFlagString& map, const char *long_opt, 
				char short_opt,const char *long_help, 
				const char *short_help, CCmdOption::Flags flags)
{
	return PCmdOption(new CCmdFlagOption(value, map, short_opt, long_opt,
                          long_help, short_help, flags ));
}


PCmdOption EXPORT_CORE make_help_opt(const char *long_opt, char short_opt, 
				     const char *long_help, CHelpOption::Callback *cb)
{
	return PCmdOption(new CHelpOption(cb, short_opt, long_opt, long_help));
}

NS_MIA_END
