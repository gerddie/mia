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

#include <stdio.h>
#include <errno.h>
#include <string.h>

#ifndef WIN32
#include <signal.h>
#include <sys/time.h>
#endif

#include <mia/core/watch.hh>
#include <mia/core/msgstream.hh>

NS_MIA_BEGIN

const int TIMERSPAN=2000;

#ifndef WIN32

class CWatchUnix: public CWatch {
public: 
	CWatchUnix();
	~CWatchUnix();

private:
	static void overlap_handler(int p_sig); 
	double do_get_seconds() const; 
	
	static uint64_t overlaps; 
}; 

CWatch::CWatch()
{
}

CWatchUnix::CWatchUnix()
{
	itimerval value;
	itimerval oldvalue;

	value.it_interval.tv_sec = TIMERSPAN;
	value.it_interval.tv_usec= 0;

	value.it_value.tv_sec  = TIMERSPAN;
	value.it_value.tv_usec = 0;

	if (signal(SIGVTALRM,CWatchUnix::overlap_handler)==SIG_ERR)
		cvwarn() << "Unable to catch  signal:" << strerror(errno) << "\n"; 
	
	if (setitimer(ITIMER_VIRTUAL,&value,&oldvalue))
		cvwarn() << "setitimer failed:" << strerror(errno) << "\n"; 
}

CWatchUnix::~CWatchUnix()
{
	
}

const CWatch& CWatch::instance()
{
	static CWatchUnix me;
	return me; 
}

double CWatch::get_seconds() const
{
	return do_get_seconds(); 
}

double CWatchUnix::do_get_seconds() const
{
	itimerval value;

	if (getitimer(ITIMER_VIRTUAL,&value))
		cvwarn() << "setitimer failed:" << strerror(errno) << "\n"; 

	double result = TIMERSPAN - value.it_value.tv_sec;
	double resultlow = value.it_value.tv_usec/1e+6;
	return (result - resultlow) + TIMERSPAN*double(overlaps);
}

void CWatchUnix::overlap_handler(int p_sig)
{
	if (p_sig == SIGVTALRM) {
		overlaps++;
	}
	signal(SIGVTALRM,CWatchUnix::overlap_handler);
}

#else
CWatch::CWatch()
{
	cvwarn() << "CWatch::CWatch: fake implementation on WIN32\n";
}
double CWatch::get_seconds() const
{
	return 0.0;
}
void CWatch::overlap_handler(int p_sig)
{
}
#endif
uint64_t CWatchUnix::overlaps=0;


NS_MIA_END
