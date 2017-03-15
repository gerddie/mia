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

#define _XOPEN_SOURCE 500
#include <stdlib.h>
#include <stdio.h>


#include <stdexcept>
#include <climits>

#include <mia/core/cmdlineparser.hh>
#include <mia/core/msgstream.hh>

#include <config.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#include <fcntl.h> 
#endif

#include<signal.h>

#ifdef HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif


#include <sys/types.h>
#include <sys/wait.h>


NS_MIA_USE
using namespace std;

struct CmdlineParserFixture {
	CmdlineParserFixture():m_level(cverb.get_level()) {
	}
	~CmdlineParserFixture() {
		cverb.set_verbosity(m_level);
	}
private:
	vstream::Level m_level;

};


bool fork_and_run_check(const char *me, const char *console_width, string& child_output)
{
	int master = posix_openpt (O_RDWR | O_NOCTTY | O_NONBLOCK);
	if (master < 0)
	  {
	    cvfail() << "unable to get a ptty" << endl;
	    return false;
	  }

	if (grantpt (master) < 0 || unlockpt (master) < 0)
	  {
	    cvfail() << "unable to grant access to terminal" << endl;
	    return false;
	  }
	const char *name = ptsname (master);
	int slave = open (name, O_RDWR);
	if (slave < 0)
	  {
	    cvfail() << "unable to open slave tty" << endl;
	    return false;
	  }

        cvdebug() << "Parent: About to fork" << endl; 
	pid_t pid = fork();
	if (!pid) { // child process

                if (dup2(slave, STDOUT_FILENO) == -1) {
                        perror("redirecting stdout");
                        return false;
                }

                if (dup2(slave, STDIN_FILENO) == -1) {
                        perror("redirecting stdin");
                        return false;
                }
		cvdebug() << "Slave: start self" << endl; 
		execlp(me, me,
		       "-w", console_width,
		       "--internal",
		       NULL);
		cvfail() << "unable to start myself" << endl;
		return false;
	}else {
                cvdebug() << "Parent: Start reading" << endl; 
                child_output.clear(); 
                char c;
		int wstatus = 0;
                while (0 == waitpid (pid, &wstatus, WNOHANG)) {
                        while(read (master, &c, 1) == 1) {
                                child_output.push_back(c);
                        }
                }

		// now get the exit code

		if (WIFEXITED(wstatus)) {
			WEXITSTATUS(wstatus);
			if (wstatus != 0) {
				cverr() << "Child process exited with status " << wstatus << "\n";
				return false;
			}
			return true; 
		}
		
                return false;
	}
}

const SProgramDescription general_help {
	{pdi_group, "Test"}, 
	{pdi_short, "program tests"}, 
	{pdi_description, "This program tests the command line parser output."}, 
	{pdi_example_descr, "Example text"}, 
	{pdi_example_code, "Example command"}
};

#ifdef MIA_COVERAGE
extern "C" void __gcov_dump(void );
#endif 

int main(int argc, const char **args)
{
        int console_width = 60;
        bool internal = false;
        
        CCmdOptionList options(general_help);
        options.add(make_opt(console_width, "width", 'w', "set console width"));
        options.add(make_opt(internal, "internal", 'i', "was forked" ));

	try {
		if (options.parse(argc, args) != CCmdOptionList::hr_no)
			return -1;
	}
	catch (const std::logic_error& x) {
		cvfail() << x.what() << "\n"; 
		return -1;
	}
	
	if (internal) {
		cvdebug() << "Start internal\n"; 
		int retvalue = 0; 
                struct winsize ws;
                int old_width = 100; 
                if (ioctl(0,TIOCGWINSZ,&ws)==0) {
                        old_width = ws.ws_col;
			ws.ws_col = console_width;
			
			if (ioctl(0,TIOCSWINSZ,&ws)==0) {
				CCmdOptionList olist(general_help);
				int test = 0; 
				vector<const char *> options;
				options.push_back("self");
				options.push_back("-h");
				
				olist.add(make_opt(test, "lala", 'i', "a string option"));
				try {
					if (olist.parse(options.size(), &options[0]) == CCmdOptionList::hr_no) {
						ws.ws_col = old_width;
						if (ioctl(0,TIOCSWINSZ,&ws) !=0) {
							cverr() << "Resetting console width failed\n"; 
							retvalue = -2;
						}
					}else
						retvalue = 0;
				}
				catch (const std::logic_error& x) {
					cvfail() << x.what() << "\n"; 
					retvalue = -4;
				}
			}else
				retvalue = -5;
                }else
			retvalue = -6;

		ws.ws_col = old_width;
		ioctl(0,TIOCSWINSZ,&ws);

		if (retvalue) 
			cverr() << "Retval " << retvalue << endl;
		
		return retvalue;
		
        } else {
		cvdebug() << "Start Master\n"; 
		
                int retval = 0; 
                // fork the tests
                string child_output_100;

		// args[0] is well defined, i.e. it is the name of the current program
		// coverity[TAINTED_STRING]
                if (fork_and_run_check(args[0], "100", child_output_100)) {
                        if (child_output_100.size() != 1843) {
			  cvfail () << child_output_100 << endl;
                                cvfail() << "Output 100: expected 1843 bytes, got "
                                         << child_output_100.size() <<"\n"; 
                                retval = -7;
                        }
                }else 
                        retval = -2;
		
		
                string child_output_50;

		// args[0] is well defined, i.e. it is the name of the current program
		// coverity[TAINTED_STRING]
                if (fork_and_run_check(args[0], "50", child_output_50)) {
                        if (child_output_50.size() != 2311) {
			  cvfail () << child_output_50 << endl;
                                cvfail() << "Output 50: expected 2311 bytes, got "
                                         << child_output_50.size() <<"\n"; 
                                retval = -8;
                        }
                }else 
                        retval = -9;

		if (retval) 
			cverr() << "Retval " << retval << endl;
                return retval; 
        }
}; 

