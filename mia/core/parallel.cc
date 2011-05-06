/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
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

#include <mia/core/parallel.hh>
#include <mia/core/msgstream.hh>

NS_MIA_BEGIN


CMessenger::CMessenger(size_t nslaves):
	m_command(-1),
	m_param(NULL),
	m_precmd_barrier(nslaves),
	m_finished(true)
{

}

void CMessenger::send_command_and_wait(int command, boost::any *param)
{
	{
		cvdebug() << "MASTER: send command "<< command << "\n";
		boost::mutex::scoped_lock lock(m_send_mutex);
		m_finished = false;
		m_command = command;
		m_param = param;
		m_send_condition.notify_all();
	}
	cvdebug() << "MASTER: send command "<< command << " wait for finish\n";
	{
		boost::mutex::scoped_lock lock(m_receive_mutex);
		while (!m_finished)
			m_receive_condition.wait(lock);
	}
	cvdebug() << "MASTER: send command "<< command << " done\n";
}

void CMessenger::send_command_finished()
{
	cvdebug() << "SLAVE: send 'finished' \n";
	boost::mutex::scoped_lock lock(m_receive_mutex);
	m_finished = true;
	m_command = -1;
	m_receive_condition.notify_one();
	cvdebug() <<  "SLAVE: send 'finished' done \n";
}

int CMessenger::wait_for_command()
{
	cvdebug() << "SLAVE: wait for command \n";
	// first gather all slaves
	m_precmd_barrier.wait();
	boost::mutex::scoped_lock lock(m_send_mutex);
	while (m_command < 0)
		m_send_condition.wait(lock);
	cvdebug() << "SLAVE: got command " <<  m_command << " \n";
	int command = m_command;
	return command;
}

CSlave::CSlave(PMessenger messanger, PBarrier final_barrier, size_t nr):
	m_messanger(messanger),
	m_final_barrier(final_barrier),
	m_nr(nr)

{
}

CSlave::~CSlave()
{
}

void CSlave::operator ()()
{
	bool run = true;

	cvdebug() << "SLAVE:" << m_nr << " started  \n";
	while ( run ) {

		int command = m_messanger->wait_for_command();

		if (command != CMessenger::cmd_finish)
			run_command(command, m_messanger->get_param());
		else {
			run = false;
			cvdebug() << "SLAVE:" << m_nr << " got terminate command; run = " << run << "\n";
		}

		cvdebug() << "SLAVE:" << m_nr << " wait at barrier  \n";
		if (m_final_barrier->wait()) {
			cvdebug() << "SLAVE:" << m_nr << " send finish  \n";
			m_messanger->send_command_finished();
			cvdebug() << "SLAVE:" << m_nr << " send finish  done \n";
		}
		cvdebug() << "SLAVE:" << m_nr << " proceed after barrier with run = " << run <<'\n';
	}
	cvdebug() << "SLAVE:" << m_nr << " terminated \n";
}

NS_MIA_END

