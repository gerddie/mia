/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2015 Gert Wollny
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iostream>
#include <fstream>
#include <stdexcept>

#include <mia/core/probmap.hh>

NS_MIA_BEGIN
using namespace std;

static const string pipe_console("-");
static const string map_signature("#probability-map");
static const string label_map_signature("#label-map");


CProbabilityVector::CProbabilityVector()
{
}

CProbabilityVector::CProbabilityVector(size_t nClass, size_t nElm):
	std::vector<double_vector>(nClass, double_vector(nElm))
{
}

CProbabilityVector::CProbabilityVector(const std::string& in_fname)
{
	if (in_fname == pipe_console) {
		do_load(cin);
	}else {
		ifstream instr(in_fname.c_str());
		do_load(instr);
	}
}

bool CProbabilityVector::save(const std::string& out_fname) const
{
	if (out_fname == pipe_console) {
		return do_save(cout);
	}else {
		ofstream outstr(out_fname.c_str());
		return do_save(outstr);
	}
}

bool CProbabilityVector::do_save(ostream& os) const
{
	os << map_signature <<"\n";
	os << size() << " " << (*this)[0].size() << '\n';

	for (size_t i = 0; i < (*this)[0].size(); ++i) {
		for (size_t k = 0; k < size(); ++k) {
			os << (*this)[k][i] << " ";
		}
		os <<'\n';
	}
	return os.good();
}

void CProbabilityVector::do_load(istream& is)
{

	string buf;
	is >> buf;
	size_t hsize, nclasses;

	if (buf != map_signature) {
		throw invalid_argument(string("input file is not a probability map: signature:'") + buf + string("'"));
	}

	is >> nclasses >> hsize;

	if (!is.good())
		throw runtime_error("error reading from input file");

	resize(nclasses);
	for (size_t k = 0; k < size(); ++k) {
		(*this)[k] = double_vector(hsize);
	}


	/* hsize is first used to allocate a vector. If it is too big
	   a std.:bad_alloc exception will be thrown. This is not worse 
	   than a check for size that would restrict hsize to an abitrary 
	   value */
	// coverity[tainted_scalar]
	for (size_t i = 0; i < hsize; ++i) {
		for (size_t k = 0; k < size(); ++k) {
			is >> (*this)[k][i];
		}
	}
	if (!is.good())
		throw runtime_error("error reading from input file");

}

EXPORT_CORE bool operator == (const CProbabilityVector& a, const CProbabilityVector& b)
{
	if ( a.size() != b.size() )
		return false;

	auto ai = a.begin();
	auto ae = a.end();
	auto bi = b.begin();

	while (ai != ae)  {
		if (*ai++ != *bi++)
			return false;
	}
	return true;
}


CLabelMap::CLabelMap()
{
}

CLabelMap::CLabelMap(const std::string& in_fname)
{
	if (in_fname == pipe_console) {
		do_load(cin);
	}else {
		ifstream instr(in_fname.c_str());
		do_load(instr);
	}
}

bool CLabelMap::save(const std::string& out_fname) const
{
	if (out_fname == pipe_console) {
		return do_save(cout);
	}else {
		ofstream outstr(out_fname.c_str());
		return do_save(outstr);
	}
}


bool CLabelMap::do_save(ostream& os) const
{
	os << label_map_signature << "\n";
	os << size()<<'\n';

	for (auto im = begin(), em = end();
	     im != em; ++im) {
		os << im->first << ' ' << im->second << '\n';
	}
	return os.good();
}

void CLabelMap::do_load(istream& is)
{
	string buf;
	is >> buf;
	size_t hsize;

	if (buf != label_map_signature) {
		throw invalid_argument(string("input file is not a label map: signature:'") + buf + string("'"));
	}

	is >> hsize;

	if (!is.good())
		throw runtime_error("error reading from input file");


	size_t src, trgt;

	/* hsize is first used to allocate a vector. If it is too big
	   a std.:bad_alloc exception will be thrown. This is not worse 
	   than a check for size that would restrict hsize to an abitrary 
	   value */
	// coverity[tainted_scalar]
	for (size_t i = 0; i < hsize; ++i) {
		is >> src >> trgt;
		(*this)[src] = trgt;
	}
	if (!is.good())
		throw runtime_error("error reading from input file");
}

EXPORT_CORE bool operator == (const CLabelMap& a, const CLabelMap& b)
{
	if (a.size() != b.size())
		return false;

	auto ai = a.begin();
	auto bi = b.begin();
	auto ae = a.end();

	while (ai != ae) {
		if (ai->first != bi->first ||
		    ai->second != bi->second)
			return false;
		++ai;
		++bi;
	}
	return true;
}


NS_MIA_END
