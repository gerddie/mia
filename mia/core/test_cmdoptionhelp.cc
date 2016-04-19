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
#include <climits>

#include <mia/core/cmdlineparser.hh>
#include <mia/core/msgstream.hh>
#include <mia/core/splinekernel.hh>

#include <config.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#include <fcntl.h> 
#endif

#ifdef HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif


#include <sys/types.h>
#include <sys/wait.h>

NS_MIA_USE
using namespace std;

bool fork_and_run_check(const char *me, vector<const char*  >& options, const string& expect)
{
        int aStdoutPipe[2];
        
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
		int result = -1;
                string child_output; 
                char c;

                while (!waitpid(pid, &result, WNOHANG)) {
                        while(read(aStdoutPipe[0], &c, 1) == 1) {
                                child_output.push_back(c);
                        }
                }
		bool test_result = child_output == expect;
		if (!test_result) {
			cvfail() << "Option '"<< options[2] << "' failed:\n";
			cvfail() << "got "<< child_output.size() << " '"<< child_output << "'\n";
			cvfail() << "expected "<< expect.size() << " '" << expect << "'\n\n";

			for (unsigned i = 0; i < min(  expect.size(), child_output.size()); ++i){
				if (child_output[i] != expect[i]) {
					cvfail() << "First character error: " << i << " "
						 << "got '" << child_output[i]  << "', expect '" <<expect[i] << "'\n";
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

string expect_usage("Usage:\n  test-cmdoptionhelp -r required --other <value> -V verbose -h help -? usage \n");
extern string expect_xml_help_start; 
extern string expect_xml_help_end;

int main(int argc, const char **args)
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
	
	
        if (options.parse(argc, args, "spline",
			  &CSplineKernelPluginHandler::instance()) != CCmdOptionList::hr_no)
		return 0;

	
	
	map<string, vector<const char*>> option_results;

	option_results[expect_usage] = vector<const char*>{args[0], "--usage", NULL};

	string expect_xml_help = expect_xml_help_start + string(get_revision()) + expect_xml_help_end;
	option_results[expect_xml_help] = vector<const char*>{args[0],
							      "--help-xml", "-",
							      NULL};
	

	bool failed = false; 
	for (auto pairs: option_results) {
		if (!fork_and_run_check(args[0], pairs.second, pairs.first)) {
			failed = true; 
		}
	}
	return failed ? EXIT_FAILURE : EXIT_SUCCESS; 
}; 

string expect_xml_help_start = "<?xml version=\"1.0\"?>\n"
	"<program>\n"
	"  <name>test-cmdoptionhelp</name>\n"
	"  <version>"; 

string expect_xml_help_end="</version>\n"
	"  <section>Test</section>\n"
"  <description>This program tests the command line parser output.</description>\n"
"  <basic_usage> test-cmdoptionhelp -r &lt;required&gt; --other &lt;value&gt; [options] &lt;PLUGINS:1d/splinekernel&gt;</basic_usage>\n"
"  <whatis>program tests</whatis>\n"
"  <group name=\"A\">\n"
"    <option short=\"r\" long=\"required\" default=\"\" type=\"string\"><flags>input required </flags>some required option</option>\n"
"    <option short=\"\" long=\"other\" default=\"\" type=\"string\"><flags>output required </flags>other required option</option>\n"
"  </group>\n"
"  <group name=\"Help &amp; Info\">\n"
"    <option short=\"V\" long=\"verbose\" default=\"warning\" type=\"dict\">verbosity of output, print messages of given level and higher priorities. Supported priorities starting at lowest level are:<dict><value name=\"trace\">Function call trace</value><value name=\"debug\">Debug output</value><value name=\"info\">Low level messages</value><value name=\"message\">Normal messages</value><value name=\"warning\">Warnings</value><value name=\"fail\">Report test failures</value><value name=\"error\">Report errors</value><value name=\"fatal\">Report only fatal errors</value></dict>verbosity of output, print messages of given level and higher priorities. Supported priorities starting at lowest level are:</option>\n"
"    <option short=\"\" long=\"copyright\" default=\"false\" type=\"bool\"><flags>nonipype </flags>print copyright information</option>\n"
"    <option short=\"h\" long=\"help\" default=\"false\" type=\"bool\"><flags>nonipype </flags>print this help</option>\n"
"    <option short=\"?\" long=\"usage\" default=\"false\" type=\"bool\"><flags>nonipype </flags>print a short help</option>\n"
"    <option short=\"\" long=\"version\" default=\"false\" type=\"bool\"><flags>nonipype </flags>print the version number and exit</option>\n"
"  </group>\n"
"  <group name=\"Processing\">\n"
"    <option short=\"\" long=\"threads\" default=\"-1\" type=\"int\">Maxiumum number of threads to use for processing,This number should be lower or equal to the number of logical processor cores in the machine. (-1: automatic estimation).Maxiumum number of threads to use for processing,This number should be lower or equal to the number of logical processor cores in the machine. (-1: automatic estimation).</option>\n"
"  </group>\n"
"  <freeparams name=\"1d/splinekernel\" type=\"factory\"/>\n"
"  <stdout-is-result/>\n"
"  <handler name=\"1d/splinekernel\">These plug-ins provide various kernels that evaluate the wights in spline-based interpolation.<plugin name=\"bspline\">B-spline kernel creation <param name=\"d\" type=\"int\" default=\"3\">Spline degree<bounded min=\"[0\" max=\"5]\"/></param></plugin><plugin name=\"omoms\">OMoms-spline kernel creation<param name=\"d\" type=\"int\" default=\"3\">Spline degree<bounded min=\"[3\" max=\"3]\"/></param></plugin></handler>\n"
"  <Example>Example text<Code>Example command</Code></Example>\n"
"  <Author>Gert Wollny</Author>\n"
"</program>\n"; 
