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

#include <mia/core/cmdlineparser.hh>
#include <mia/core/msgstream.hh>
#include <mia/core/testplugin.hh>

#include <config.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#include <fcntl.h> 
#endif

#ifdef HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif


/*
  Wenn we ron coverage, then the child process sends a signal to the parent 
  process to let the parent program terminate before the child that actually 
  runs the interesting code is terminated. 
*/
#ifdef MIA_COVERAGE	
#include<signal.h>
#endif 

#include <sys/types.h>
#include <sys/wait.h>

NS_MIA_USE
using namespace std;

#ifdef MIA_COVERAGE	
bool wait_for_child = true;
void sig_usr(int signo)
{
	if (signo == SIGINT)
		wait_for_child = false; 
}
#endif 	

bool fork_and_run_check(const char *me, vector<const char*  >& options, const string& expect)
{
        int aStdoutPipe[2];

#ifdef MIA_COVERAGE	
	struct sigaction sig;
	sigemptyset(&sig.sa_mask);          
	sig.sa_flags = 0;
	sig.sa_handler = sig_usr;
	wait_for_child = true;
#endif 	
        if (pipe2(aStdoutPipe, O_NONBLOCK) < 0) {
                perror ("Allocatin stdout pipe");
                return false; 
        }
        
        cvdebug() << "Parent: About to fork" << endl; 
	pid_t pid = fork();
	if (!pid) { // child process

                if (dup2(aStdoutPipe[1], STDOUT_FILENO) == -1) {
                        perror("redirecting stdout");
                        return false;
                }
                
		execvp(me, (char* const* )&options[0]);
		cvfail() << "unable to start myself" << endl;
		return false;
	}else {
                cvdebug() << "Parent: Start reading" << endl; 

                string child_output; 
                char c;
#ifdef MIA_COVERAGE
		sigaction(SIGINT,&sig,NULL); 
		while (wait_for_child) 
#else 		
		int result = -1;
		while (!waitpid(pid, &result, WNOHANG)) 
#endif
		{
                        while(read(aStdoutPipe[0], &c, 1) == 1) {
                                child_output.push_back(c);
                        }
                }
		bool test_result = child_output == expect;
		if (!test_result) {
			cvfail() << "Option '"<< options[1] << "' failed:\n";
			cvfail() << "got "<< child_output.size() << " '"<< child_output << "'\n";
			cvfail() << "expected "<< expect.size() << " '" << expect << "'\n\n";

			for (unsigned i = 0; i < min(  expect.size(), child_output.size()); ++i){
				if (child_output[i] != expect[i]) {
					cvfail() << "First character error: " << i << " "
						 << "got '" << child_output.substr(i)  << "', expect '" << expect.substr(i) << "'\n";
					break; 
				}
			}
		}
		return test_result;
	}
}

const SProgramDescription general_help {
	{pdi_group, "Test"}, 
	{pdi_short, "program tests"}, 
	{pdi_description, "This program tests the command line parser output."}, 
	{pdi_example_descr, "Example text"}, 
	{pdi_example_code, "Example command"}
};

extern string expect_xml_help_start; 
extern string expect_xml_help_end;

int do_main(int argc, char **args)
{
	string required_option;
	string other_required_option;  
        CCmdOptionList options(general_help);
        options.add("A", make_opt(required_option, "required", 'r',
				   "some required option",
				   CCmdOptionFlags::required_input));
        options.add("A", make_opt(required_option, "other", 0,
			     "other required option",
			     CCmdOptionFlags::required_output));

	options.set_stdout_is_result(); 
	options.set_group("empty"); 

	CPluginSearchpath sp(true);
	sp.add("testplug"); 
	CTestPluginHandler::set_search_path(sp);
	
        if (options.parse(argc, args, "plugin",
			  &CTestPluginHandler::instance()) != CCmdOptionList::hr_no) {
#ifdef MIA_COVERAGE
		// strange, the output to std::cerr makes sure that the signal is received, 
		// sleep(n) doesn't.
		sleep(1); 
		std::cerr << "\n"; 
		kill(getppid(), SIGINT);
#endif 
		return 0;
	}

	string expect_xml_help = expect_xml_help_start + string(get_revision()) + expect_xml_help_end;
	vector<const char*> arg{args[0],"--help-xml", "-", NULL};
	

	bool failed = false;
	
	// args[0] is well defined, i.e. it is the name of the current program
	// coverity[TAINTED_STRING]
	if (!fork_and_run_check(args[0], arg, expect_xml_help)) {
		failed = true; 
	}

	return failed ? EXIT_FAILURE : EXIT_SUCCESS; 
}; 

#include <mia/internal/main.hh>
MIA_MAIN(do_main); 

string expect_xml_help_start = "<?xml version=\"1.0\"?>\n"
	"<program>\n"
	"  <name>test-cmdxmlhelp</name>\n"
	"  <version>"; 

string expect_xml_help_end="</version>\n"
	"  <section>Test</section>\n"
"  <description>This program tests the command line parser output.</description>\n"
"  <basic_usage> test-cmdxmlhelp -r &lt;required&gt; --other &lt;value&gt; [options] &lt;PLUGINS:none/test&gt;</basic_usage>\n"
"  <whatis>program tests</whatis>\n"
"  <group name=\"A\">\n"
"    <option default=\"\" long=\"required\" short=\"r\" type=\"string\">some required option<flags>input required </flags></option>\n"
"    <option default=\"\" long=\"other\" short=\"\" type=\"string\">other required option<flags>output required </flags></option>\n"
"  </group>\n"
"  <group name=\"Help &amp; Info\">\n"
"    <option default=\"warning\" long=\"verbose\" short=\"V\" type=\"dict\">verbosity of output, print messages of given level and higher priorities. Supported priorities starting at lowest level are:<dict><value name=\"trace\">Function call trace</value><value name=\"debug\">Debug output</value><value name=\"info\">Low level messages</value><value name=\"message\">Normal messages</value><value name=\"warning\">Warnings</value><value name=\"fail\">Report test failures</value><value name=\"error\">Report errors</value><value name=\"fatal\">Report only fatal errors</value></dict></option>\n"
"    <option default=\"false\" long=\"copyright\" short=\"\" type=\"bool\">print copyright information<flags>nonipype </flags></option>\n"
"    <option default=\"false\" long=\"help\" short=\"h\" type=\"bool\">print this help<flags>nonipype </flags></option>\n"
"    <option default=\"false\" long=\"usage\" short=\"?\" type=\"bool\">print a short help<flags>nonipype </flags></option>\n"
"    <option default=\"false\" long=\"version\" short=\"\" type=\"bool\">print the version number and exit<flags>nonipype </flags></option>\n"
"  </group>\n"
"  <group name=\"Processing\">\n"
"    <option default=\"-1\" long=\"threads\" short=\"\" type=\"int\">Maxiumum number of threads to use for processing,This number should be lower or equal to the number of logical processor cores in the machine. (-1: automatic estimation).</option>\n"
"  </group>\n"
"  <freeparams name=\"none/test\" type=\"factory\"/>\n"
"  <stdout-is-result/>\n"
"  <handler name=\"none/test\">This is a handler for the test plug-ins<plugin name=\"dummy1\">test_dummy_symbol from dummy1</plugin><plugin name=\"dummy2\">test module with no data (2)</plugin><plugin name=\"dummy3\">test_dummy_symbol from dummy3</plugin></handler>\n"
"  <Example>Example text<Code>Example command</Code></Example>\n"
"  <Author>Gert Wollny</Author>\n"
"</program>\n"; 
