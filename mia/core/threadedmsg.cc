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

#include <iostream>
#include <iomanip>
#include <mia/core/threadedmsg.hh>
#include <sstream>

#include <stdexcept>
#include <mia/core/parallel.hh>
#include <mia/core/msgstream.hh>

NS_MIA_BEGIN
using std::setw;
using std::setfill;
using std::logic_error;


class thread_streamredir: public std::streambuf
{
public:
       thread_streamredir();

       static void set_master_stream(std::ostream& master);
protected:
       int sync();
       int overflow(int c);
       std::streamsize xsputn ( const char *s, std::streamsize n );
private:
       void send_to_master();

       std::ostringstream m_buffer;
       int m_id;
       static std::ostream *m_master;
       static CMutex m_master_lock;
       static int m_next_id;
};


thread_streamredir::thread_streamredir()
{
       CScopedLock lock(m_master_lock);
       m_id = m_next_id++;
}


void thread_streamredir::set_master_stream(std::ostream& master)
{
       m_master = &master;
}

int thread_streamredir::sync()
{
       if (!m_buffer.str().empty())
              send_to_master();

       return 0;
}

int thread_streamredir::overflow(int c)
{
       // this function probably never gets called, because we use a stringstream
       // to buffer the output that can grow until the memory is filled
       send_to_master();
       m_buffer << (char)c;
       return 0;
}

std::streamsize thread_streamredir::xsputn( const char *s, std::streamsize n )
{
       int i = 0;

       while (i < n) {
              // newline forces syncronization and output to the master stream
              if (*s != '\n') {
                     m_buffer << *s;

                     // since the ostringstream grows
                     // dynamically, this should never happen.
                     // Should the size be limited?
                     if (m_buffer.fail())
                            break;
              } else
                     send_to_master();

              ++s;
              ++i;
       }

       return i;
}

void thread_streamredir::send_to_master()
{
       CScopedLock lock(m_master_lock);
       char oldfill = m_master->fill();
       *m_master << "[thread " << setw(3) << setfill('0') << m_id << setfill(oldfill) << "]:"
                 << m_buffer.str() << "\n";
       m_buffer.str("");
       m_buffer.clear();
}

std::ostream *thread_streamredir::m_master = &std::cerr;
CMutex thread_streamredir::m_master_lock;
int thread_streamredir::m_next_id = 0;


CThreadMsgStream::CThreadMsgStream():
       // coverity[resource_leak] explanation below in destructor
       std::ostream(new thread_streamredir()),
       m_old(vstream::instance().set_stream(*this))
{
}

CThreadMsgStream::~CThreadMsgStream()
{
       flush();
       vstream::instance().set_stream(m_old);
       // At this point we delete the thread_streamredir object that is
       // created in the constructor
       delete rdbuf();
}

void CThreadMsgStream::do_set_master_stream(std::ostream& master)
{
       // if this is actually a CThreadMsgStream, then deadlock is certain
       // hence throw and tell the programmer to fix the program
       if (dynamic_cast<CThreadMsgStream *>(&master)) {
              throw logic_error("CThreadMsgStream::set_master_stream: trying to set the master stream to a CThreadMsgStream. "
                                "Since this would make deadlock certain I bail out now.\nPlease fix your program\n");
       }

       thread_streamredir::set_master_stream(master);
}

NS_MIA_END
