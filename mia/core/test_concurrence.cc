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

#include <climits>

#define BOOST_TEST_DYN_LINK


#include <vector>
#include <boost/ref.hpp>
#include <boost/test/unit_test.hpp>


#include <mia/core/parallel.hh>
#include <mia/core/msgstream.hh>


NS_MIA_USE
using namespace boost;

class CTestSlave : public CSlave {
public:
	CTestSlave(PMessenger msg, PBarrier final_barrier, size_t& value, mutex& value_mutex, size_t nr);

private:
	virtual void run_command(int command, any* param) ;
	size_t& m_value;
	mutex& m_value_mutex;
};

CTestSlave::CTestSlave(PMessenger msg, PBarrier final_barrier, size_t& value, mutex& value_mutex, size_t nr):
	CSlave(msg, final_barrier, nr),
	m_value(value),
	m_value_mutex(value_mutex)

{

}

void CTestSlave::run_command(int command, any* /*param*/)
{
	cvdebug() << "SLAVE " << nr() << ": run command " << command << "\n";
	if (command == 2) {

		mutex::scoped_lock lock(m_value_mutex);
		++m_value;
		cvdebug() <<"SLAVE " << nr() << ": " <<  m_value << "\n";
	}
	cvdebug() << "SLAVE " << nr() << ": run command " << command << " done\n";
}



BOOST_AUTO_TEST_CASE(  test_concurence )
{

	const size_t max_slaves = 5;
	size_t value = 0;
	mutex value_mutex;

	PBarrier bar(new barrier(max_slaves));
	PMessenger msg(new CMessenger(max_slaves));

	std::vector< std::shared_ptr<CTestSlave > > slaves(max_slaves);

	thread_group threads;

	for (size_t i = 0; i < max_slaves; ++i) {
		cvdebug() << "Create slave " << i << "\n";
		slaves[i] = std::shared_ptr<CTestSlave >(new CTestSlave(msg, bar,value, value_mutex,i));
		threads.create_thread(reference_wrapper<CTestSlave>(*slaves[i]));
	}

	msg->send_command_and_wait(2);
	msg->send_command_and_wait(CMessenger::cmd_finish);

	threads.join_all();

	BOOST_CHECK(value == max_slaves);
}

