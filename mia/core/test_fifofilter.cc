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


#define BOOST_TEST_DYN_LINK

#define EXPORT_HANDLER
#include <cassert>
#include <iostream>
#include <cmath>
#include <numeric>
#include <climits>

#include <boost/test/test_tools.hpp>
#include <boost/test/unit_test_suite.hpp>
#include <boost/test/floating_point_comparison.hpp>


#include <mia/core/cmdlineparser.hh>
#include <mia/core/fifofilter.hh>

#include <mia/core/fifofilter.cxx>

using namespace std;
using namespace mia;


typedef TFifoFilter<char> CCharFifoFilter;
typedef std::shared_ptr<TFifoFilter<char> >  PCharFifoFilter;

class CCharFifoFilterSink : public CCharFifoFilter {
public:
	CCharFifoFilterSink();
	const string result();
private:
	virtual void do_push(char c);

	string m_result;

};

CCharFifoFilterSink::CCharFifoFilterSink():CCharFifoFilter(0, 1, 0)
{
}

const string CCharFifoFilterSink::result()
{
	return m_result;
}

void CCharFifoFilterSink::do_push(char c)
{
	m_result.append(1, c);
}



BOOST_AUTO_TEST_CASE( test_sink_mechanics )
{
	CCharFifoFilterSink sink;


	for ( size_t i = 0; i < 10; ++i)
		sink.push('a');

	sink.finalize();

	BOOST_CHECK_EQUAL(sink.result(), string(10,'a'));

}

typedef TFifoFilter<int> CIntFifoFilter;
typedef std::shared_ptr<TFifoFilter<int> >  PIntFifoFilter;
typedef TFifoFilterSink<int> CIntFifoFilterSink;

class CAddSomeFifoFilter : public CIntFifoFilter {
public:
	CAddSomeFifoFilter(int value);
private:
	virtual int do_filter();
	virtual void do_push(int c);
	virtual void evaluate(size_t slice);

	int m_value;
	int m_buf;
};

CAddSomeFifoFilter::CAddSomeFifoFilter(int value):
	CIntFifoFilter(0,1,0),
	m_value(value)
{
}

int CAddSomeFifoFilter::do_filter()
{
	return m_buf;
}


void CAddSomeFifoFilter::do_push(int c)
{
	m_buf = c;
}


void CAddSomeFifoFilter::evaluate(size_t /*slice*/)
{
	m_buf += m_value;
}

class CMeanIntFifoFilter : public CIntFifoFilter {
public:
	CMeanIntFifoFilter(size_t width);
private:
	virtual int do_filter();
	virtual void shift_buffer();
	virtual void do_push(int c);
	vector<int> m_buf;
};

CMeanIntFifoFilter::CMeanIntFifoFilter(size_t hwidth):
	CIntFifoFilter(2 * hwidth + 1, hwidth + 1, 0),
	m_buf(2 * hwidth + 1)
{
}

int CMeanIntFifoFilter::do_filter()
{
	int result;
	result = accumulate(m_buf.begin() + get_start(), m_buf.begin() + get_end(), 0);
	result /= get_end() - get_start();
	return result;
}

void CMeanIntFifoFilter::shift_buffer()
{
	copy_backward(m_buf.begin(), m_buf.end() - 1, m_buf.end());
}

void CMeanIntFifoFilter::do_push(int c)
{
	m_buf[0] = c;
}


BOOST_AUTO_TEST_CASE( test_mean )
{
	int test_result[10] = {
		2, 2, 3, 4, 5,
		6, 7, 8, 8, 9
	};

	CMeanIntFifoFilter filter(2);
	CIntFifoFilterSink *sink = new CIntFifoFilterSink();
	CIntFifoFilter::Pointer psink(sink);

	filter.append_filter(psink);

	for ( size_t i = 1; i < 11; ++i)
		filter.push(i);

	filter.finalize();

	const vector<int>& result = sink->result();
	BOOST_CHECK_EQUAL(result.size(), 10u);
	if (!equal(result.begin(), result.end(), test_result)) {
		BOOST_MESSAGE("int_ptr_equal() failed");
		for (size_t i = 0; i < 10; ++i)
			cvfail() << result[i] << " should be " << test_result[i] << "\n";
	}
}


class CMeanAddIntFifoFilter : public CIntFifoFilter {
public:
	CMeanAddIntFifoFilter(size_t width);
private:
	virtual int do_filter();
	virtual void shift_buffer();
	virtual void evaluate(size_t slice);
	virtual void do_push(int c);

	vector<int> m_buf;
};

CMeanAddIntFifoFilter::CMeanAddIntFifoFilter(size_t hwidth):
	CIntFifoFilter(2 * hwidth + 1, hwidth + 1, hwidth),
	m_buf(3 * hwidth + 1)
{
	fill(m_buf.begin(), m_buf.end(), 400);
}

int CMeanAddIntFifoFilter::do_filter()
{
	cvdebug() << get_start() << ", " <<  get_end() << "\n";

	int result;
	result = accumulate(m_buf.begin() + get_start(), m_buf.begin() + get_end(), 0);
	result /= get_end() - get_start();
	return result;
}

void CMeanAddIntFifoFilter::shift_buffer()
{
	copy_backward(m_buf.begin(), m_buf.end() - 1, m_buf.end());
}

void CMeanAddIntFifoFilter::evaluate(size_t slice)
{
	m_buf[slice] += 1;
}

void CMeanAddIntFifoFilter::do_push(int c)
{
	m_buf[0] = c;
}



class CTrackCallersFilter : public CIntFifoFilter {
public:
	CTrackCallersFilter();
	const string get_starts() const;
	const string get_ends() const;
	const string get_callseries() const;
private:
	virtual void do_initialize(int x);
	virtual void post_finalize();
	virtual int do_filter();
	virtual void shift_buffer();
	virtual void evaluate(size_t slice);
	virtual void do_push(int c);
	stringstream m_starts;
	stringstream m_ends;
	stringstream m_callseries;
};


CTrackCallersFilter::CTrackCallersFilter():
	CIntFifoFilter(3, 2, 2)
{
}

void CTrackCallersFilter::do_initialize(int /*x*/)
{
	m_callseries << 'b';
}

void CTrackCallersFilter::post_finalize()
{
	m_callseries << 'x';
}

const string CTrackCallersFilter::get_starts() const
{
	return m_starts.str();
}

const string CTrackCallersFilter::get_ends() const
{
	return m_ends.str();
}

const string CTrackCallersFilter::get_callseries() const
{
	return m_callseries.str();
}


int CTrackCallersFilter::do_filter()
{
	m_callseries << 'f';
	m_starts << get_start() << " ";
	m_ends   << get_end()   << " ";
	return 0;
}

void CTrackCallersFilter::shift_buffer()
{
	m_callseries << 's';
}

void CTrackCallersFilter::evaluate(size_t /*slice*/)
{
	m_callseries << 'e';
}

void CTrackCallersFilter::do_push(int /*c*/)
{
	m_callseries << 'p';
}


BOOST_AUTO_TEST_CASE( test_callseries )
{
	CTrackCallersFilter filter;

	for ( size_t i = 0; i < 5; ++i)
		filter.push(i);

	filter.finalize();

	BOOST_CHECK_EQUAL(filter.get_starts(), "2 2 2 2 3 ");
	BOOST_CHECK_EQUAL(filter.get_ends(), "4 5 5 5 5 ");
	BOOST_CHECK_EQUAL(filter.get_callseries(), "bpspspespefspefsefsefsfx");

}


BOOST_AUTO_TEST_CASE( test_chaining)
{
	CAddSomeFifoFilter filter(1);
	CIntFifoFilter::Pointer filter2(new CAddSomeFifoFilter(2));



	CIntFifoFilterSink::Pointer sink = CIntFifoFilterSink::Pointer(new CIntFifoFilterSink());

	filter.append_filter(filter2);
	filter.append_filter(sink);

	filter.push(3);
	filter.finalize();

	const vector< int >& result = sink->result();

	BOOST_REQUIRE(result.size() == 1);
	BOOST_CHECK_EQUAL(result[0], 6);
}



BOOST_AUTO_TEST_CASE( test_mean_add )
{
	int test_result[10] = {
		3, 3, 4, 5, 6,
		7, 8, 9, 9, 10
	};

	CMeanAddIntFifoFilter filter(2);
	CIntFifoFilterSink *sink = new CIntFifoFilterSink();
	CIntFifoFilter::Pointer psink(sink);


	filter.append_filter(psink);

	for ( size_t i = 1; i < 11; ++i)
		filter.push(i);

	filter.finalize();

	const vector< int >& result = sink->result();
	BOOST_CHECK_EQUAL(result.size(), 10u);
	BOOST_CHECK(equal(result.begin(), result.end(), test_result));

	if (!equal(result.begin(), result.end(), test_result)) {
		BOOST_FAIL("int_ptr_equal()");
		for (size_t i = 0; i < 10; ++i)
			cvfail() << result[i] << " should be " << test_result[i] << "\n";
	}

}
