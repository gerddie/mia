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


bool wait_for_child = true;

void sig_usr(int signo)
{
	if (signo == SIGINT)
		wait_for_child = false; 
}


bool fork_and_run_check(const char *me, const char *console_width, string& child_output)
{
        int aStdoutPipe[2];
        
	struct sigaction sig;
	sigemptyset(&sig.sa_mask);          
	sig.sa_flags = 0;
	sig.sa_handler = sig_usr;
	
	wait_for_child = true;
	
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
		       "-w", console_width,
		       "--internal",
		       NULL);
		cvfail() << "unable to start myself" << endl;
		return false;
	}else {
		sigaction(SIGINT,&sig,NULL); 
                cvdebug() << "Parent: Start reading" << endl; 
                child_output.clear(); 
                char c;

                while (wait_for_child) {
                        while(read(aStdoutPipe[0], &c, 1) == 1) {
                                child_output.push_back(c);
                        }
                }
                return true;
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
        
        if (options.parse(argc, args) != CCmdOptionList::hr_no)
                return -1;
        
        if (internal) {

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
							retvalue = -1;
						}
					}else
						retvalue = -1;
				}
				catch (const std::logic_error& x) {
					cvfail() << x.what() << "\n"; 
					retvalue = -1;
				}
			}else
				retvalue = -1;
                }else
			retvalue = -1;

		ws.ws_col = old_width;
		ioctl(0,TIOCSWINSZ,&ws);

		cverr() << "Send signal\n";
		sleep(1); 
		kill(getppid(), SIGINT);
		sleep(1);

		
		return retvalue;
		
        } else {
		
                int retval = 0; 
                // fork the tests
                string child_output_100;

		// args[0] is well defined, i.e. it is the name of the current program
		// coverity[TAINTED_STRING]
                if (fork_and_run_check(args[0], "100", child_output_100)) {
                        if (child_output_100.size() != 1796) {
                                cvfail() << "Output 100: expected 1796 bytes, got "
                                         << child_output_100.size() <<"\n"; 
                                retval = -3;
                        }
                }else 
                        retval = -2;
		
		
                string child_output_50;

		// args[0] is well defined, i.e. it is the name of the current program
		// coverity[TAINTED_STRING]
                if (fork_and_run_check(args[0], "50", child_output_50)) {
                        if (child_output_50.size() != 2241) {
                                cvfail() << "Output 50: expected 2241 bytes, got "
                                         << child_output_50.size() <<"\n"; 
                                retval = -3;
                        }
                }else 
                        retval = -2;


                return retval; 
        }
}; 

