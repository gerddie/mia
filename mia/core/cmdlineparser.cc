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

#include <config.h>
#include <miaconfig.h>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <map>
#include <ctype.h>
#include <stdexcept>

#ifdef HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif

#ifdef HAVE_TBB 
#include <tbb/task_scheduler_init.h>
#else
#include <mia/core/parallelcxx11.hh>
#endif

#include <mia/core/xmlinterface.hh>
#include <mia/core/tools.hh>
#include <mia/core/msgstream.hh>
#include <mia/core/cmdstringoption.hh>
#include <mia/core/cmdbooloption.hh>
#include <mia/core/cmdlineparser.hh>
#include <mia/core/fixedwidthoutput.hh>

#if defined(__PPC__) && ( TBB_INTERFACE_VERSION  < 6101 )
#define TBB_PREFERE_ONE_THREAD 1
#endif 

extern void print_full_copyright(const char *name, const char *author);


NS_MIA_BEGIN
extern char const *get_revision(); 

using std::ostream;
using std::ofstream;
using std::ostringstream;
using std::string;
using std::invalid_argument; 
using std::logic_error; 
using std::vector; 
using std::map; 
using std::unique_ptr; 


#define ENTRY(X) {X, #X }
const std::map<EProgramDescriptionEntry, const char *> g_DescriptionEntryNames = {
	ENTRY(pdi_group), 
	ENTRY(pdi_short), 
	ENTRY(pdi_description), 
	ENTRY(pdi_example_descr),
	ENTRY(pdi_example_code),
	ENTRY(pdi_author)
}; 

struct CCmdOptionListData {
	CShortoptionMap short_map;
	CLongoptionMap  long_map;
	typedef vector<PCmdOption> COptionsGroup;
	typedef map<string, COptionsGroup > COptionsMap;
	COptionsMap options;
	COptionsMap::iterator current_group;
	vector<string> remaining;

	bool help;
	string help_xml; 
	bool usage;
	bool version; 
	bool copyright;
	vstream::Level verbose;
	int max_threads;
	bool m_selftest_run; 
	bool m_stdout_is_result; 

	CCmdOptionListData(const SProgramDescription& description); 

	void add(PCmdOption opt);
	void add(const string& group, PCmdOption opt);
	
	CHistoryRecord get_values() const;

	void set_current_group(const string& name);

	CCmdOption *find_option(const char *key) const;
	CCmdOption *find_option(char key) const;

	void post_set(); 
	void print_help_xml(const char *progname, const CPluginHandlerBase *additional_help) const; 
	void print_help(const char *name_help, bool has_additional) const;
	void print_usage(const char *name_help) const;
	void print_version(const char *name_help) const;

	vector<const char *> has_unset_required_options() const; 
	void set_logstream(std::ostream& os); 
	string set_description_value(EProgramDescriptionEntry entry, 
				     const SProgramDescription& description, const char *default_value = ""); 

	const char *get_author() const; 
	
	string m_program_group; 
	string m_short_descr; 
	string m_general_help; 
	string m_program_example_descr;
	string m_program_example_code; 
	string m_free_parametertype; 
	string m_author; 
	ostream *m_log; 
};


/**
   This option is used to run a selftest. 

   A program can only have one selftest option and all other options that are not 
   in the Help & Info section are ignored when the selftest option is set. 
   The selftest is done within a class derived from the CSelftestOption::Callback 
   class that overrides the Callback::do_run() method. 
*/
class EXPORT_CORE CSelftestOption: public CCmdOption {
public: 
        /**
           Test callback base class. 

           The tests to be run must be implemented in the "int do_run() const"  method, and 
           it must return 0 if the test run successfull. 
         */



private: 
        friend class CCmdOptionList; 
        /**
           Constructor of the selftest function 
        */
        CSelftestOption(bool& run, int& test_result, CSelftestCallback *callback);
        

        virtual bool do_set_value(const char *str_value);
	virtual void do_write_value(std::ostream& os) const;
        virtual size_t do_get_needed_args() const; 
        
        std::unique_ptr<CSelftestCallback> m_callback;
        int& m_test_result;
        bool& m_run; 
        
}; 

void CCmdOptionListData::set_logstream(ostream& os)
{
	m_log  = &os; 
}

string CCmdOptionListData::set_description_value(EProgramDescriptionEntry entry, 
						 const SProgramDescription& description, const char *default_value)
{
	auto id = description.find(entry); 
	if (id != description.end()) 
		return string( id->second);
	else {
		auto ed = g_DescriptionEntryNames.find(entry); 
		assert(ed != g_DescriptionEntryNames.end()); 
		if (!default_value) {
			cvwarn() << "Description value '" <<  ed->second << "' not set\n"; 
			return string(""); 
		} else {
			cvdebug() << "Description value '" <<  ed->second 
				  << "' not set, using default '" << default_value << "'\n"; 
			return string(default_value); 
		}
	}
}

const char *g_help_optiongroup="Help & Info"; 
const char *g_default_author = "Gert Wollny"; 
const char *g_basic_copyright1 = "This software is Copyright (c) "; 
const char *g_basic_copyright2 = " 1999-2016 Leipzig, Germany and Madrid, Spain. "
	      "It comes with ABSOLUTELY NO WARRANTY and you may redistribute it "
	      "under the terms of the GNU GENERAL PUBLIC LICENSE Version 3 (or later). "
	      "For more information run the program with the option '--copyright'.\n"; 

CCmdOptionListData::CCmdOptionListData(const SProgramDescription& description):
	help(false),
	usage(false),
	version(false), 
	copyright(false),
	verbose(vstream::ml_warning),
#if HAVE_TBB 
#if TBB_PREFERE_ONE_THREAD
	max_threads(1), 
#else
	max_threads(tbb::task_scheduler_init::automatic), 
#endif
#else
	max_threads(-1),
#endif 	
	m_selftest_run(false), 
	m_stdout_is_result(false), 
	m_log(&std::cout)
{

	m_program_group = set_description_value(pdi_group, description); 
	m_short_descr = set_description_value(pdi_short, description); 
	m_general_help = set_description_value(pdi_description, description);  
	m_program_example_descr = set_description_value(pdi_example_descr, description); 
	m_program_example_code = set_description_value(pdi_example_code, description); 
	m_author = set_description_value(pdi_author, description, g_default_author); 
	
	options[""] = vector<PCmdOption>();

	set_current_group(g_help_optiongroup);
	add(make_opt(verbose, g_verbose_dict, "verbose",  'V', 
		     "verbosity of output, print messages of given level and higher priorities."
		     " Supported priorities starting at lowest level are:"));
	add(make_opt(copyright,  "copyright", 0, "print copyright information", CCmdOptionFlags::nonipype));
	add(make_opt(help,  "help", 'h', "print this help", CCmdOptionFlags::nonipype));
	add(make_opt(help_xml,  "help-xml", 0, "print help formatted as XML", CCmdOptionFlags::nonipype));
	add(make_opt(usage,  "usage", '?', "print a short help", CCmdOptionFlags::nonipype));
	add(make_opt(version,  "version", 0, "print the version number and exit", CCmdOptionFlags::nonipype));

	set_current_group("Processing"); 
	add(make_opt(max_threads, "threads", 0, "Maxiumum number of threads to use for processing," 
			     "This number should be lower or equal to the number of logical processor cores in the machine. "
			     "(-1: automatic estimation).")); 
	
	set_current_group("");
}

const char *CCmdOptionListData::get_author() const
{
	return m_author.c_str(); 
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

void CCmdOptionListData::add(const string& group, PCmdOption opt)
{
	if (options.find(group) == options.end()) 
		options[group] = vector<PCmdOption>();
	options[group].push_back(opt); 
	opt->add_option(short_map, long_map);
}

CHistoryRecord CCmdOptionListData::get_values() const
{
	CHistoryRecord result;
	for(auto o_i = options.begin(); o_i != options.end(); ++o_i)
		for(auto g_i = o_i->second.begin(); g_i != o_i->second.end(); ++g_i) 
			result[(*g_i)->get_long_option()] = (*g_i)->get_value_as_string();
	return result;
}

void CCmdOptionListData::post_set()
{
	for(auto o_i = options.begin(); o_i != options.end(); ++o_i)
		for(auto g_i = o_i->second.begin(); g_i != o_i->second.end(); ++g_i) 
			(*g_i)->post_set(); 
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


void CCmdOptionListData::print_help_xml(const char *name_help, const CPluginHandlerBase *additional_help) const
{
	HandlerHelpMap handler_help_map; 
	if (additional_help) 
		additional_help->add_dependend_handlers(handler_help_map); 
		
	unique_ptr<CXMLDocument> doc(new CXMLDocument);
	
	auto nodeRoot = doc->create_root_node("program");
	auto program_name = nodeRoot->add_child("name"); 
	program_name->set_child_text(name_help); 
	auto  version_string = nodeRoot->add_child("version"); 
	version_string->set_child_text(get_revision()); 
	auto program_group = nodeRoot->add_child("section"); 
	program_group->set_child_text(m_program_group); 
	auto description = nodeRoot->add_child("description"); 
	description->set_child_text(m_general_help); 
	auto basic_usage = nodeRoot->add_child("basic_usage"); 
	auto  short_descr = nodeRoot->add_child("whatis"); 
	short_descr->set_child_text(m_short_descr); 


	ostringstream usage_text; 
	usage_text << " " << name_help << " "; 

	for (auto g = options.begin(); g != options.end(); ++g) {
		// no need to store empty groups 
		if (g->second.empty()) 
			continue; 
		
		auto group = nodeRoot->add_child("group"); 
		group->set_attribute("name", g->first); 
		
		for (auto iopt= g->second.begin(); iopt != g->second.end(); ++iopt) {
			const CCmdOption& opt = **iopt; 
			if (opt.get_long_option() == string("help-xml")) 
				continue; 
			
			opt.add_option_xml(*group, handler_help_map); 
			
			if (opt.is_required()) {
				if (opt.get_short_option())
					usage_text << "-" << opt.get_short_option() << " <" 
						   << opt.get_long_option() << "> "; 
				else
					usage_text << "--" << opt.get_long_option() 
						   << " <value> ";
			}
		}
	}
	if (additional_help) {
		auto free_parameters = nodeRoot->add_child("freeparams"); 
		free_parameters->set_attribute("name", additional_help->get_descriptor()); 
		free_parameters->set_attribute("type", "factory"); 
	}
	
	if (m_stdout_is_result) {
		nodeRoot->add_child("stdout-is-result");
	}
		
	usage_text << "[options]"; 
	if (additional_help) 
		usage_text << " <PLUGINS:" << additional_help->get_descriptor() <<">"; 
	basic_usage->set_child_text(usage_text.str()); 

	for (auto h = handler_help_map.begin(); h != handler_help_map.end(); ++h)
		h->second->get_xml_help(*nodeRoot);
	
	auto example = nodeRoot->add_child("Example");
	example->set_child_text(m_program_example_descr); 
	auto example_code = example->add_child("Code"); 
	example_code->set_child_text(m_program_example_code); 
	
	auto cr = nodeRoot->add_child("Author");
	cr->set_child_text(m_author); 

	ofstream xmlfile(help_xml.c_str());  
	
	xmlfile << doc->write_to_string_formatted();
	xmlfile << "\n"; 
}

/**
   This help printing is a mess ...
 */
void CCmdOptionListData::print_help(const char *name_help, bool has_additional) const
{
	const size_t max_opt_width = 23;
	
	size_t max_width = 70;
#ifdef HAVE_SYS_IOCTL_H
	// the test cases require a fixed width handling and write to a ostringstream 
	auto log_to_string = dynamic_cast<ostringstream*>(m_log); 
	if (!log_to_string) {
		struct winsize ws; 
		if (ioctl(0,TIOCGWINSZ,&ws)==0) {
			max_width = ws.ws_col;
			if (max_width < max_opt_width + 20) 
				max_width = max_opt_width + 20; 
			
		} 
		if (max_width > 100) 
			max_width = 100; 
	}
#endif
	CFixedWidthOutput console(*m_log, max_width); 

	vector<string> opt_table;
	vector<string> help_table;
	
	console.write("\nProgram group:  "); 
	console.write(m_program_group); 

	console.push_offset(4); 
	console.write("\n\n"); 
	console.write(m_general_help);
	console.pop_offset();
	console.newline(); 
	
	ostringstream usage_text; 
	usage_text <<name_help << " "; 
	
	
	size_t opt_size = 0;
	*m_log << setiosflags(std::ios_base::left);
	for (auto i = options.begin(); i != options.end(); ++i) {

		if (i->second.empty()) 
			continue;
		ostringstream group;
		group << "\n" << i->first; 
		opt_table.push_back(group.str());
		help_table.push_back("  ");
		
		for (auto g_i = i->second.begin(); g_i != i->second.end(); ++g_i) {
			ostringstream opt;
			ostringstream shelp;

			const PCmdOption& k = *g_i;
			k->get_opt_help(opt);

			opt_table.push_back(opt.str());
			size_t size = opt.str().size();
			if (opt_size < size)
				opt_size = size;

			k->get_long_help(shelp);
			help_table.push_back(shelp.str());

			if (k->is_required()) {
				if (k->get_short_option())
					usage_text << "-" << k->get_short_option() << " <" << k->get_long_option() << "> "; 
				else
					usage_text << "--" << k->get_long_option() << " <value> ";
			}
		}
	}
	if (opt_size > max_opt_width)
		opt_size = max_opt_width;

	usage_text << "[options]"; 
	if (has_additional) {
		usage_text << " [<" << m_free_parametertype <<">]"
			   << " [<" << m_free_parametertype <<">]"
			   << " ..."; 
	}

	console.write("\nBasic usage:"); 
	console.push_offset(4); 
	console.newline(); 
	console.push_offset(4); 
	console.set_linecontinue(true); 
	console.write(usage_text.str()); 
	console.set_linecontinue(false); 
	console.pop_offset(); 
	console.pop_offset(); 
	console.newline(); 

	console.write("\nThe program supports the following command line options:\n");

		
	auto t  = opt_table.begin();
	for (auto i = help_table.begin(); i != help_table.end(); ++i, ++t) {
		console.write(*t); 
		console.push_offset(opt_size+1); 
		if (t->length() > opt_size) 
			console.newline(); 
		else 
			for (size_t i = t->length(); i <= opt_size; ++i) 
				console.write(" ");
		console.write(*i);
		console.pop_offset(); 
		console.newline(); 
	}

	if (!m_program_example_descr.empty() && !m_program_example_code.empty()) { 
		console.newline();
		console.push_offset(2);
		console.write("Example usage:\n"); 
		console.write(m_program_example_descr); 
		console.push_offset(2);
		console.newline(); 
		console.newline(); 
		console.write(name_help); 
		console.write(" "); 
		console.set_linecontinue(true); 
		console.push_offset(4);
		console.write(m_program_example_code);
		console.set_linecontinue(false); 
		console.reset_offset(); 
		console.newline();
	}
	
	console.write("\n"); 
	console.push_offset(2);
	console.write("Copyright:\n");
	console.write(g_basic_copyright1);
	console.write(get_author());
	console.write(g_basic_copyright2);
	console.pop_offset(); 
	console.write("\n");
	*m_log << setiosflags(std::ios_base::right);

}

void CCmdOptionListData::print_usage(const char *name) const
{
	*m_log << "Usage:\n";
	*m_log << "  " << name << " ";
	for (COptionsMap::const_iterator i = options.begin();
	     i != options.end(); ++i) {
		COptionsGroup::const_iterator g_i = i->second.begin();
		COptionsGroup::const_iterator g_e = i->second.end();

		while (g_i != g_e) {

			const PCmdOption& k = *g_i;
			k->print_short_help(*m_log);
			++g_i;
		}
	}
	*m_log << '\n';
}

void CCmdOptionListData::print_version(const char *name_help) const
{
	*m_log << name_help << " version: " << get_revision() << "\n\n"; 
	*m_log << g_basic_copyright1; 
	*m_log << get_author(); 
	*m_log << g_basic_copyright2 << "\n"; 
}

CCmdOptionList::CCmdOptionList(const SProgramDescription& description):
	m_impl(new CCmdOptionListData(description))
{
}	

void CCmdOptionList::add(PCmdOption opt)
{
	m_impl->add(opt);
}

void CCmdOptionList::add(const std::string& table, PCmdOption opt)
{
	m_impl->add(table, opt);
}

void CCmdOptionList::add_selftest(int& test_result, CSelftestCallback *callback)
{
	m_impl->add("Test", PCmdOption(new CSelftestOption(m_impl->m_selftest_run, test_result, callback))); 
}

void CCmdOptionList::set_group(const std::string& group)
{
	m_impl->set_current_group(group); 
}

int CCmdOptionList::handle_shortargs(const char *arg, size_t remaining_args, const char *args[])
{
	cvdebug() << "entering with argument " << *arg << " and " << remaining_args << " remaining arguments\n"; 
	bool bool_options_only = false;
	do {
		CCmdOption *opt = m_impl->find_option(*arg);
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
                                // this handles when the option value is attacted to the flag 
				// like in -bsomevalue
				opt->set_value(arg);
				bool_options_only = false;
				break;
			default:
				throw invalid_argument("need a space between option and "
						       "parameter for multiple parameters");
			}
		}else {
			cvdebug() << "remaining_args = " << remaining_args << ", and " << nargs << " needed\n"; 
			if (remaining_args < nargs ) {
				throw create_exception<invalid_argument>("Option -", opt->get_short_option(), 
							       ": requires ", nargs, " arguments, but only ", 
							       remaining_args, " remaining.");
			}

			switch (nargs) {
			case 0:
				opt->set_value(NULL);
				bool_options_only = true;
				break;
			case 1:
                                // this handles when the option value is not attacted to the flag 
				// like in -b somevalue
				opt->set_value(args[0]);
				bool_options_only = false;
				break;
			default:// actually currently multiple 
				throw logic_error("Command line parameters that take more then"
						  " one parameter are not yet implemented");
			}
			return nargs;
		}
	} while (*arg && bool_options_only);

	return 0;
}

CCmdOptionList::EHelpRequested
CCmdOptionList::parse(size_t argc, const char *args[])
{
	return do_parse(argc, args, false, NULL);
}


CCmdOptionList::EHelpRequested
CCmdOptionList::parse(size_t argc, char *args[])
{
	return do_parse(argc, (const char **)args, false, NULL);
}


CCmdOptionList::EHelpRequested
CCmdOptionList::parse(size_t argc, const char *args[], const string& additional_type, 
		      const CPluginHandlerBase *additional_help)
{
	m_impl->m_free_parametertype = additional_type; 
	return do_parse(argc, args, true, additional_help);
}

CCmdOptionList::EHelpRequested
CCmdOptionList::parse(size_t argc, char *args[], const string& additional_type, 
		      const CPluginHandlerBase *additional_help)
{
	m_impl->m_free_parametertype = additional_type; 
	return do_parse(argc, (const char **)args, true, additional_help);
}

void CCmdOptionList::set_stdout_is_result()
{
	m_impl->m_stdout_is_result = true; 
}

#ifdef HAVE_TBB 

struct TBBTaskScheduler {
	static const tbb::task_scheduler_init& initialize(int max_threads);
}; 

const tbb::task_scheduler_init& TBBTaskScheduler::initialize(int max_threads)
{
#if TBB_PREFERE_ONE_THREAD
	if (max_threads > 1)
		cvwarn() << "You use an old version (interface="
			 << TBB_INTERFACE_VERSION 
			 << ") of Intel Threading Building Blocks."
			 << " This version may hang og powerpc when using more than one thread.\n"; 
#endif 
	static tbb::task_scheduler_init init(max_threads);
	return init; 
}

#endif 

CCmdOptionList::EHelpRequested
CCmdOptionList::do_parse(size_t argc, const char *args[], bool has_additional, 
			 const CPluginHandlerBase *additional_help)
{

	cvdebug() << "Parse " << argc << " parameters \n"; 
	size_t idx = 1;
	while (idx < argc ) {
		const char *cur_arg = args[idx++];
		const char *ccarg = cur_arg;
		size_t remaining_args = argc - idx;

		// if we don't scan an option, we deal with a left over argument
		if (*ccarg != '-') {
			m_impl->remaining.push_back(cur_arg);
			continue;
		}

		// we found a singular dash, this is also stored as a left over argument
		++ccarg;
		if (!*ccarg) {
			m_impl->remaining.push_back(cur_arg);
			continue;
		}

		// now check if this is a long option 
		if( *ccarg == '-') { 
			cvdebug() << "handle opt '" << ccarg << "'\n"; 
			++ccarg;
			CCmdOption *opt = m_impl->find_option(ccarg);
			if (opt) {
				size_t nargs = opt->get_needed_args();
				// currently only one argument value is supported
				assert(nargs <= 1); 
				if (remaining_args < nargs ) {
					throw create_exception<invalid_argument>("Option --" , opt->get_long_option() 
					      , ": requires " 
					      , nargs , " arguments, but only " , remaining_args
					      , " remaining.");
				}
				
				opt->set_value(args[idx]);
				idx += nargs;
				continue;
			}
		} else {
			cvdebug() << "handle shortargs '" << ccarg << "'\n"; 
			int r = handle_shortargs(ccarg, argc - idx, &args[idx]);
			if (r >= 0) {
				idx +=  r;
				continue;
			}
		}
		// current option was not found
		m_impl->remaining.push_back(cur_arg);

	}

	m_impl->post_set(); 
	
	const char *name_help = strrchr(args[0], '/'); 
	name_help  = name_help ? name_help + 1 : args[0]; 

	if (m_impl->help) {
		m_impl->print_help(name_help, has_additional);
		return hr_help;
	}else if (!m_impl->help_xml.empty()) {
		m_impl->print_help_xml(name_help, additional_help);
		return hr_help_xml;
	} else if (m_impl->usage) {
		m_impl->print_usage(name_help);
		return hr_usage;
	} else if (m_impl->version) {
		m_impl->print_version(name_help);
		return hr_version;
	} else if (m_impl->copyright) {
		::print_full_copyright(name_help, m_impl->get_author());
		return hr_copyright;
	} else if (m_impl->m_selftest_run) {
		return hr_selftest; 
	}

	cverb.set_verbosity(m_impl->verbose);
	if (!has_additional && !m_impl->remaining.empty()) {
		ostringstream msg; 
		msg << "Unknown options given: "; 
		for (auto i = m_impl->remaining.begin(); m_impl->remaining.end() != i; ++i)
			msg << " '" << *i << "' "; 
		throw invalid_argument(msg.str());
	}
	
	auto unset_but_required = m_impl->has_unset_required_options(); 
	if (!unset_but_required.empty()) {
		ostringstream msg; 
		if (unset_but_required.size() > 1) 
			msg << "Some required options were not set:"; 
		else
			msg << "A required options was not set:"; 
		for (auto i = unset_but_required.begin(); unset_but_required.end() != i; ++i)
			msg << " '--" << *i << "' "; 
		msg << "\n"; 
		msg << "run '" << args[0] << " --help' for more information\n"; 
		msg << g_basic_copyright1; 
		msg << m_impl->get_author(); 
		msg << g_basic_copyright2; 
		throw invalid_argument(msg.str());
	}
	
#ifdef HAVE_TBB 
	// the return value and info output is mostly used to make sure the compiler 
	// doesn't optimize anything away. 
	const auto& ts  = TBBTaskScheduler::initialize(m_impl->max_threads); 
	cvinfo() << "Task scheduler set to " << (ts.is_active() ? "active":"inactive") << "\n";
#else
	CMaxTasks::set_max_tasks(m_impl->max_threads); 
#endif 	
	return hr_no; 
}

void CCmdOptionList::set_logstream(std::ostream& os)
{
	m_impl->set_logstream(os); 
}

const vector<string>& CCmdOptionList::get_remaining() const
{
	return m_impl->remaining;
}

CCmdOptionList::~CCmdOptionList()
{
	delete m_impl;
}


CHistoryRecord CCmdOptionList::get_values() const
{
	return m_impl->get_values();
}

CCmdFlagOption::CCmdFlagOption(int& val, const CFlagString& map, char short_opt, 
			       const char *long_opt, const char *long_help, 
			       const char *short_help, 
			       CCmdOptionFlags flags):
	CCmdOption(short_opt, long_opt, long_help,short_help, flags),
	m_value(val),
	m_map(map)
{
}

void CCmdFlagOption::do_write_value(std::ostream& os) const
{
	os << m_map.get(m_value);
}

void CCmdFlagOption::do_get_long_help(std::ostream& os) const
{
	os << " supported flags:(" <<m_map.get_flagnames() << ")";
}

const std::string CCmdFlagOption::do_get_value_as_string() const
{
	return m_map.get(m_value);
}


bool CCmdFlagOption::do_set_value(const char *str_value)
{
	m_value = m_map.get(str_value);
	return true;
}

size_t CCmdFlagOption::do_get_needed_args() const
{
	return 1;
}


CSelftestOption::CSelftestOption(bool& run, int& test_result, CSelftestCallback *callback):
        CCmdOption(0, "selftest", "run a self test of the program", 0, 
                   CCmdOptionFlags::nonipype), 
        m_callback(callback), 
        m_test_result(test_result), 
        m_run(run)
{
        
}


bool CSelftestOption::do_set_value(const char * /* str_value */)
{
        assert(m_callback); 
        m_test_result = m_callback->run();
        m_run = true;
        return true; 
}

void CSelftestOption::do_write_value(std::ostream& /* os */) const
{
}

size_t CSelftestOption::do_get_needed_args() const
{
        return 0; 
}

PCmdOption EXPORT_CORE make_opt(int& value, const CFlagString& map, const char *long_opt, 
				char short_opt,const char *long_help, 
				const char *short_help, CCmdOptionFlags flags)
{
	return PCmdOption(new CCmdFlagOption(value, map, short_opt, long_opt,
                          long_help, short_help, flags ));
}


PCmdOption EXPORT_CORE make_opt(std::string& value, const char *long_opt, char short_opt, const char *long_help, 
				CCmdOptionFlags flags, const CPluginHandlerBase *plugin_hint)
{
	return PCmdOption(new CCmdStringOption(value, short_opt, long_opt, long_help, 
					       flags, plugin_hint)); 
}

PCmdOption EXPORT_CORE make_opt(bool& value, const char *long_opt, char short_opt, const char *help, 
				CCmdOptionFlags flags)
{
	return PCmdOption(new CCmdBoolOption(value, short_opt, long_opt, help, flags ));
}



NS_MIA_END
