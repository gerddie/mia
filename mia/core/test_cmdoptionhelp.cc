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

bool fork_and_run_check(const char *me, const string& option, const string& expect)
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
                
		execlp(me, me,
		       option.c_str(), 
		       NULL);
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
			cvfail() << "Option '"<< option << "' failed:\n";
			cvfail() << "got "<< child_output.size() << " '"<< child_output << "'\n";
			cvfail() << "expected "<< expect.size() << " '" << expect << "'\n\n";
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


int main(int argc, const char **args)
{
	int required_option;  
	
        CCmdOptionList options(general_help);
        options.add(make_opt(required_option, "required", 'r',
			     "some required option",
			     CCmdOptionFlags::required_input));

	vector <const char *> args_help(argc + 2);
	for (int i = 0; i< argc; ++i) {
		args_help[i] = args[i]; 
	}
	args_help[argc] = "-r";
	args_help[argc+1] = "1"; 
	
        if (options.parse(args_help.size(), &args_help[0], "spline",
		      &CSplineKernelPluginHandler::instance()) != CCmdOptionList::hr_no)
		return 0;


	map<string, string> option_results;

	option_results["--usage"] = "Usage:\n  test-cmdoptionhelp -r required -V verbose -h help -? usage \n"; 

	bool failed = false; 
	for (auto pairs: option_results) {
		if (!fork_and_run_check(args[0], pairs.first, pairs.second)) {
			failed = true; 
		}
	}
	return failed ? EXIT_FAILURE : EXIT_SUCCESS; 
}; 

