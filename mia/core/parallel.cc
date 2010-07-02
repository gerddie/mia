/* -*- mia-c++  -*-
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

#include <mia/core/parallel.hh>
#include <mia/core/msgstream.hh>

NS_MIA_BEGIN


CMessenger::CMessenger(size_t nslaves):
	_M_command(-1),
	_M_param(NULL),
	_M_precmd_barrier(nslaves),
	_M_finished(true)
{

}

void CMessenger::send_command_and_wait(int command, boost::any *param)
{
	{
		cvdebug() << "MASTER: send command "<< command << "\n";
		boost::mutex::scoped_lock lock(_M_send_mutex);
		_M_finished = false;
		_M_command = command;
		_M_param = param;
		_M_send_condition.notify_all();
	}
	cvdebug() << "MASTER: send command "<< command << " wait for finish\n";
	{
		boost::mutex::scoped_lock lock(_M_receive_mutex);
		while (!_M_finished)
			_M_receive_condition.wait(lock);
	}
	cvdebug() << "MASTER: send command "<< command << " done\n";
}

void CMessenger::send_command_finished()
{
	cvdebug() << "SLAVE: send 'finished' \n";
	boost::mutex::scoped_lock lock(_M_receive_mutex);
	_M_finished = true;
	_M_command = -1;
	_M_receive_condition.notify_one();
	cvdebug() <<  "SLAVE: send 'finished' done \n";
}

int CMessenger::wait_for_command()
{
	cvdebug() << "SLAVE: wait for command \n";
	// first gather all slaves
	_M_precmd_barrier.wait();
	boost::mutex::scoped_lock lock(_M_send_mutex);
	while (_M_command < 0)
		_M_send_condition.wait(lock);
	cvdebug() << "SLAVE: got command " <<  _M_command << " \n";
	int command = _M_command;
	return command;
}

CSlave::CSlave(PMessenger messanger, PBarrier final_barrier, size_t nr):
	_M_messanger(messanger),
	_M_final_barrier(final_barrier),
	_M_nr(nr)

{
}

CSlave::~CSlave()
{
}

void CSlave::operator ()()
{
	bool run = true;

	cvdebug() << "SLAVE:" << _M_nr << " started  \n";
	while ( run ) {

		int command = _M_messanger->wait_for_command();

		if (command != CMessenger::cmd_finish)
			run_command(command, _M_messanger->get_param());
		else {
			run = false;
			cvdebug() << "SLAVE:" << _M_nr << " got terminate command; run = " << run << "\n";
		}

		cvdebug() << "SLAVE:" << _M_nr << " wait at barrier  \n";
		if (_M_final_barrier->wait()) {
			cvdebug() << "SLAVE:" << _M_nr << " send finish  \n";
			_M_messanger->send_command_finished();
			cvdebug() << "SLAVE:" << _M_nr << " send finish  done \n";
		}
		cvdebug() << "SLAVE:" << _M_nr << " proceed after barrier with run = " << run <<'\n';
	}
	cvdebug() << "SLAVE:" << _M_nr << " terminated \n";
}

NS_MIA_END

