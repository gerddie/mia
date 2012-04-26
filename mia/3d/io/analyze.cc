/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2012 Gert Wollny
 *
 * This program is free software; you can redistribute it and/or modify
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <limits>
#include <errno.h>

#include <boost/algorithm/minmax_element.hpp>
#include <cstring>
#include <string>
#include <sstream>

#include <mia/core/file.hh>
#include <mia/core/filter.hh>
#include <mia/core/msgstream.hh>
#include <mia/3d/io/analyze.hh>


NS_BEGIN(IMAGEIO_3D_ANALYZE)

NS_MIA_USE
using namespace std;
using namespace boost;

#pragma pack(1)
struct analyze_header_key        /* header key     */
{                                /* off + size     */
	int sizeof_hdr;            /*  0 + 4         */
	char data_type[10];      /*  4 + 10        */
	char db_name[18];        /* 14 + 18        */
	int extents;             /* 32 + 4         */
	short int session_error; /* 36 + 2         */
	char regular;            /* 38 + 1         */
	char hkey_un0;           /* 39 + 1         */
};                               /* total=40 bytes */

struct analyze_image_dimension
{                                /* off + size     */
	short int dim[8];        /* 0 + 16         */
	unsigned char vox_units[4];       /* 16 + 2         */
	unsigned char cal_units[8];       /* 18 + 2         */
	short int unused1;      /* 28 + 2         */
	short int datatype;      /* 30 + 2         */
	short int bitpix;        /* 32 + 2         */
	short int dim_un0;       /* 34 + 2         */
	float pixdim[8];         /* 36 + 32        */
	/*
	  pixdim[] specifies the voxel dimensitons:
	  pixdim[1] - voxel width
	  pixdim[2] - voxel height
	  pixdim[3] - interslice distance
	  ...etc
	*/
	float vox_offset;        /* 68 + 4          */
	float funused1;          /* 72 + 4          */
	float funused2;          /* 76 + 4          */
	float funused3;          /* 80 + 4          */
	float cal_max;           /* 84 + 4          */
	float cal_min;           /* 88 + 4          */
	float compressed;        /* 92 + 4          */
	float verified;          /* 96 + 4          */
	int glmax,glmin;         /* 100 + 8         */
};                               /* total=108 bytes */

struct analyze_data_history
{                                /* off + size     */
	char descrip[80];        /* 0 + 80         */
	char aux_file[24];       /* 80 + 24        */
	char orient;             /* 104 + 1        */
	char originator[10];     /* 105 + 10       */
	char generated[10];      /* 115 + 10       */
	char scannum[10];        /* 125 + 10       */
	char patient_id[10];     /* 135 + 10       */
	char exp_date[10];       /* 145 + 10       */
	char exp_time[10];       /* 155 + 10       */
	char hist_un0[3];        /* 165 + 3        */
	int views;                /* 168 + 4        */
	int vols_added;          /* 172 + 4        */
	int start_field;         /* 176 + 4        */
	int field_skip;          /* 180 + 4        */
	int omax, omin;          /* 184 + 8        */
	int smax, smin;          /* 192 + 8        */
};
struct analyze_dsr
{
	struct analyze_header_key hk;          /* 0 + 40           */
	struct analyze_image_dimension dime;   /* 40 + 108         */
	struct analyze_data_history hist;      /* 148 + 200        */
};                                     /* total= 348 bytes */
#pragma pack()

/* Acceptable values for datatype */
#define DT_NONE                      0
#define DT_UNKNOWN                   0
#define DT_BINARY                    1
#define DT_UNSIGNED_CHAR             2
#define DT_SIGNED_SHORT              4
#define DT_SIGNED_INT                8
#define DT_FLOAT                     16
#define DT_COMPLEX                   32
#define DT_DOUBLE                    64
#define DT_RGB                       128
#define DT_ALL                       255

enum EAnaOrientation {
	ao_transverse_unflipped = 0,
	ao_coronal_unflipped,
	ao_saggital_unflipped,
	ao_transverse_flipped,
	ao_coronal_flipped,
	ao_saggital_flipped,
	ao_unknown
};

typedef struct
{
	float real;
	float imag;
} COMPLEX;

const TDictMap<EPixelType>::Table analyze_type_table[] = {
	{"UNKNOWN",  it_unknown, "unkown pixel type"},
	{"BINARY",   it_bit,     "binary pixel type"},
	{"CHAR",     it_ubyte,   "unsigned 8 bit"},
	{"SHORT",    it_sshort,  "signed 16 bit"},
	{"INT",      it_sint,    "signed 32 bit"},
	{"FLOAT",    it_float,   "single floating point"},
	{"COMPLEX",  it_unknown, "complex, not supported"},
	{"DOUBLE",   it_double,  "double floating point"},
	{"RGB",      it_unknown, "RGB, not supported"},
        {0, it_unknown, ""}
};



extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new CAnalyze3DImageIOPlugin();
}

CAnalyze3DImageIOPlugin::CAnalyze3DImageIOPlugin():
	C3DImageIOPlugin("analyze"),
	m_type_table(analyze_type_table)
{
//	add_supported_type(it_bit);
	add_supported_type(it_ubyte);
	add_supported_type(it_sshort);
	add_supported_type(it_sint);
	add_supported_type(it_float);
	add_supported_type(it_double);
}

template <typename T>
inline void swap_32(T& v)
{

	char outp[4];
	const char *inp = reinterpret_cast<const char *>(&v);
	outp[3] = inp[0];
	outp[2] = inp[1];
	outp[1] = inp[2];
	outp[0] = inp[3];
        memcpy(&v, outp, 4);
}

template <typename T>
inline void swap_64(T& v)
{

        char outp[8];
        const char *inp = reinterpret_cast<const char *>(&v);
        outp[7] = inp[0];
        outp[6] = inp[1];
        outp[5] = inp[2];
        outp[4] = inp[3];
        outp[3] = inp[4];
        outp[2] = inp[5];
        outp[1] = inp[6];
        outp[0] = inp[7];
        memcpy(&v, outp, 8);
}

inline void swap_int16(short& v)
{
	v = (v << 8 ) | ( ( v >> 8 ) & 0xFF);
}

void  CAnalyze3DImageIOPlugin::swap_hdr(analyze_dsr& hdr) const
{
	swap_32(hdr.hk.sizeof_hdr) ;
	swap_32(hdr.hk.extents) ;
	swap_int16(hdr.hk.session_error) ;
	swap_int16(hdr.dime.dim[0]) ;
	swap_int16(hdr.dime.dim[1]) ;
	swap_int16(hdr.dime.dim[2]) ;
	swap_int16(hdr.dime.dim[3]) ;
	swap_int16(hdr.dime.dim[4]) ;
	swap_int16(hdr.dime.dim[5]) ;
	swap_int16(hdr.dime.dim[6]) ;
	swap_int16(hdr.dime.dim[7]) ;
	swap_int16(hdr.dime.unused1) ;
	swap_int16(hdr.dime.datatype) ;
	swap_int16(hdr.dime.bitpix) ;
	swap_32(hdr.dime.pixdim[0]) ;
	swap_32(hdr.dime.pixdim[1]) ;
	swap_32(hdr.dime.pixdim[2]) ;
	swap_32(hdr.dime.pixdim[3]) ;
	swap_32(hdr.dime.pixdim[4]) ;
	swap_32(hdr.dime.pixdim[5]) ;
	swap_32(hdr.dime.pixdim[6]) ;
	swap_32(hdr.dime.pixdim[7]) ;
	swap_32(hdr.dime.vox_offset) ;
	swap_32(hdr.dime.funused1) ;
	swap_32(hdr.dime.funused2) ;
	swap_32(hdr.dime.cal_max) ;
	swap_32(hdr.dime.cal_min) ;
	swap_32(hdr.dime.compressed) ;
	swap_32(hdr.dime.verified) ;
	swap_int16(hdr.dime.dim_un0) ;
	swap_32(hdr.dime.glmax) ;
	swap_32(hdr.dime.glmin) ;
}


template <typename Iterator, int size>
struct __swap_endian
{
	static void apply(Iterator /*begin*/, Iterator /*end*/) {
		// standard is do nothing
	}
};

template <typename Iterator>
struct __swap_endian<Iterator, 2>
{
	static void apply(Iterator begin, Iterator end) {
		while (begin != end)  {
			*begin =  (*begin << 8 ) | ( ( *begin >> 8 ) & 0xFF);
			++begin;
		}
	}
};

template <typename Iterator>
struct __swap_endian<Iterator, 4>
{
	static void apply(Iterator begin, Iterator end) {
		while (begin != end)  {
			swap_32(*begin);
			++begin;
		}
	}
};

template <typename Iterator>
struct __swap_endian<Iterator, 8>
{
	static void apply(Iterator begin, Iterator end) {
		while (begin != end)  {
                        swap_64(*begin);
			++begin;
		}
	}
};

template <typename T>
void swap_endian(T3DImage<T>& image)
{
	__swap_endian<typename T3DImage<T>::iterator , sizeof(T)>::apply(image.begin(), image.end());
}


template <typename T, bool flipped>
struct do_read_image {
	static C3DImage * apply(const C3DBounds& size, CInputFile& data_file, bool do_swap_endian) {
		T3DImage<T> *result = new T3DImage<T>(size);
		if (fread(&(*result)(0,0,0), sizeof(T), result->size(), data_file) != result->size())
			throw runtime_error("Analyze: unable to read data");
		if (do_swap_endian)
			swap_endian(*result);
		return result;
	}
};

template <typename T>
struct do_read_image<T, true> {
	static C3DImage * apply(const C3DBounds& size, CInputFile& data_file, bool do_swap_endian) {
		T3DImage<T> *result = new T3DImage<T>(size);
		for (size_t z = size.z; z > 0; --z)
			if (fread(&(*result)(0,0,z - 1), sizeof(T), result->size(), data_file) != result->size())
				throw runtime_error("Analyze: unable to read data");
		if (do_swap_endian)
			swap_endian(*result);
		return result;
	}
};


#if 0
template <>
struct do_read_image<bool> {
	static C3DImage * apply(const C3DBounds& size, CInputFile& data_file, bool swap_endian) const {


		T3DImage<bool> *result = new T3DImage<bool>(size);
		if (fread(&(*result)(0,0,0), sizeof(T), result->size(), data_file) != result->size())
			throw runtime_error("Analyze: unable to read data");
		if (swap_endian)
			swap_endian(*result);
		return result;
	}
};

#endif

template <bool flipped>
C3DImage *CAnalyze3DImageIOPlugin::read_image(const C3DBounds& size, short datatype, CInputFile& data_file) const
{
	if (datatype & 128)
		cvwarn() << "Got an RGB indicator but I will ignore it\n";

	switch (datatype & 0xFF) {
//	case DT_BINARY       :return do_read_image<bool>::apply(size, data_file, m_swap_endian);
	case DT_UNSIGNED_CHAR:return do_read_image<unsigned char, flipped>::apply(size, data_file, m_swap_endian);
	case DT_SIGNED_SHORT :return do_read_image<signed short, flipped>::apply(size, data_file, m_swap_endian);
	case DT_SIGNED_INT   :return do_read_image<signed int, flipped>::apply(size, data_file, m_swap_endian);
	case DT_FLOAT        :return do_read_image<float, flipped>::apply(size, data_file, m_swap_endian);
	case DT_DOUBLE       :return do_read_image<double, flipped>::apply(size, data_file, m_swap_endian);
	default:
		stringstream msg;
		msg << "Analyze: unsupported image type:" << datatype;
		throw invalid_argument(msg.str());
	}
}


void set_typeinfo(analyze_image_dimension& dime, EPixelType pixel_type)
{
	switch (pixel_type) {
	case it_ubyte:
		dime.datatype = DT_UNSIGNED_CHAR;
		dime.bitpix   = 8;
		break;

	case it_sshort:
		dime.datatype = DT_SIGNED_SHORT;
		dime.bitpix   = 16;
		break;

	case it_sint:
		dime.datatype = DT_SIGNED_INT;
		dime.bitpix   = 32;
		break;

	case it_float:
		dime.datatype = DT_FLOAT;
		dime.bitpix   = 32;
		break;

	case it_double:
		dime.datatype = DT_DOUBLE;
		dime.bitpix   = 64;
		break;

	default:
		throw invalid_argument("Analyze: input pixel type not suporrted");
	}
}

CAnalyze3DImageIOPlugin::PData CAnalyze3DImageIOPlugin::do_load(const string&  filename) const
{
	CInputFile f(filename);
	if (!f){
		cvdebug() << filename << ":" << strerror(errno) << "\n";
		return PData();
	}

	analyze_dsr hdr;
	if (fread(&hdr, 1, sizeof(analyze_dsr), f) != sizeof(analyze_dsr)) {
		cvdebug() << filename.c_str() << ":" << "unable to read analyze header\n";
		return PData();
	}

	if (hdr.dime.dim[0] < 0 || hdr.dime.dim[0] > 15) {
		swap_hdr(hdr);
		m_swap_endian = true;
	}else
		m_swap_endian = false;

	if (hdr.dime.dim[0] < 0 || hdr.dime.dim[0] > 15) {
		cvdebug() << filename.c_str() << ":" << "not an analyze  header\n";
		return PData();
	}

	if ((unsigned int)hdr.hk.sizeof_hdr < sizeof(hdr)) {
		cvdebug() << filename.c_str() << ":" << "not an analyze  header\n";
		return PData();
	}

	if (hdr.dime.dim[0] < 3) {
		cvdebug() << filename.c_str() << ":" << "not a supported analyze  header\n";
		return PData();
	}

	// get the size
	C3DBounds size(hdr.dime.dim[1], hdr.dime.dim[2], hdr.dime.dim[3]);
	C3DFVector voxel(hdr.dime.pixdim[1], hdr.dime.pixdim[2], hdr.dime.pixdim[3]);
	cvdebug() << "Analyze: got voxel size " << voxel << "\n";

	size_t num_img = 1;

	for(short int i = 4; i<hdr.dime.dim[0] ; ++i ) {
		num_img *= hdr.dime.dim[i];
	}
	// open data fiele
	const string data_file_name = filename.substr(0, filename.length() - 3) + string("img");
	CInputFile data_file(data_file_name);
	if (!data_file)
		throw runtime_error(string("Analyze: unable to find data file:") + data_file_name );

	// create output list
	PData result(new C3DImageVector());

        // read data
	while (num_img > 0) {
		--num_img;
		E3DImageOrientation orientation = ior_unknown;
		bool unflipped = false;
		switch ( hdr.hist.orient ) {
		case ao_transverse_unflipped: unflipped = true;
		case ao_transverse_flipped: orientation = ior_axial;
			break;
		case ao_coronal_unflipped: unflipped = true;
		case ao_coronal_flipped:    orientation = ior_coronal;
			break;
		case ao_saggital_unflipped: unflipped = true;
		case ao_saggital_flipped:    orientation = ior_saggital;
			break;
		default:
			unflipped = true;
			orientation = ior_unknown;
		}
		C3DImage *img = unflipped ?
			read_image<false>(size, hdr.dime.datatype , data_file)
			:
			read_image<true>(size, hdr.dime.datatype , data_file);

		P3DImage image(img);
		image->set_voxel_size(voxel);
		image->set_orientation(orientation);
		result->push_back(image);
	}

	// clean up
	return result;

}

template <typename T>
struct __do_fwrite {
	static size_t apply(const T3DImage<T>& image, size_t a, size_t b, FILE *f) {
		return fwrite(&image(0,0,0), a,  b, f);
	}
};

template <>
struct __do_fwrite<bool> {
	static size_t apply(const T3DImage<bool>& /*image*/, size_t /*a*/, size_t /*b*/, FILE */*f*/) {
		throw invalid_argument("Analyze:Saving boolen not yet supported");
	}
};

class CSavefilter: public TFilter<bool> {
public:
	CSavefilter(COutputFile& f, analyze_image_dimension& dime):
		m_f(f),
		m_dime(dime)
		{
		}

	template <class T>
	bool operator ()(const T3DImage<T>& image) const {
		if (__do_fwrite<T>::apply(image, sizeof(T), image.size(), m_f)!= image.size()) {
			throw runtime_error(string("Analyze: Error writing file") + strerror(errno));
		}
		pair<typename T3DImage<T>::const_iterator, typename T3DImage<T>::const_iterator>
			image_minmax = ::boost::minmax_element(image.begin(), image.end());
		if ( (int)*image_minmax.first < m_dime.glmin)
			m_dime.glmin = (int) *image_minmax.first;
		if ( (int)*image_minmax.second > m_dime.glmax)
			m_dime.glmax = (int) *image_minmax.second;
		return true;
	}

private:
	COutputFile& m_f;
	analyze_image_dimension& m_dime;
};


bool CAnalyze3DImageIOPlugin::save_data(const string& fname, const Data& data, analyze_image_dimension& dime) const
{
	const string data_file_name = fname.substr(0, fname.length() - 3) + string("img");
	COutputFile data_file(data_file_name);

	if (!data_file)
		throw runtime_error(string("Analyze: unable to open '") + data_file_name + "' for writing");

	CSavefilter saver(data_file, dime);
	for (Data::const_iterator k = data.begin(); k != data.end(); ++k) {
		mia::filter(saver, **k);
	}

	return true;
}

bool CAnalyze3DImageIOPlugin::do_save(const string& fname, const Data& data) const
{
	analyze_dsr hdr;
	memset(&hdr, 0, sizeof(hdr));

	hdr.hk.sizeof_hdr =  sizeof(hdr);
	hdr.dime.dim[0] = 4;
	hdr.dime.dim[4] = data.size();

	if (data.empty())
		throw invalid_argument("Trying to save empty image list");

	Data::const_iterator k = data.begin();

	C3DBounds size = (*k)->get_size();
	C3DFVector voxel = (*k)->get_voxel_size();
	EPixelType pixel_type = (*k)->get_pixel_type();

	E3DImageOrientation orient = (*k)->get_orientation();

	switch (orient) {
	case ior_axial:
			hdr.hist.orient = ao_transverse_unflipped;
			break;
	case ior_coronal:
			hdr.hist.orient = ao_coronal_unflipped;
			break;
	case ior_saggital:
			hdr.hist.orient = ao_saggital_unflipped;
			break;
	default:
		hdr.hist.orient = ao_unknown;
	}

	while (k != data.end()) {
		if (size != (*k)->get_size() ||
		    pixel_type != (*k)->get_pixel_type() ||
			voxel != (*k)->get_voxel_size() ) {
			throw invalid_argument("analyze only support images series of same size and type");
		}
		++k;
	}

	k = data.begin();

	hdr.dime.dim[1] = size.x;
	hdr.dime.dim[2] = size.y;
	hdr.dime.dim[3] = size.z;

	hdr.dime.pixdim[1] = voxel.x;
	hdr.dime.pixdim[2] = voxel.y;
	hdr.dime.pixdim[3] = voxel.z;

	cvdebug() << voxel << "\n";

	hdr.hk.extents = 16384;
	hdr.hk.regular = 'r';
	set_typeinfo(hdr.dime, pixel_type);

	hdr.dime.glmin = numeric_limits<int>::max();
	save_data(fname, data, hdr.dime);

	COutputFile hdr_file(fname);

	if (!hdr_file)
		throw runtime_error(string("Analyze: unable to open '") + fname + "' for writing");

	if (fwrite(&hdr, 1, sizeof(hdr), hdr_file) != sizeof(hdr))
		throw runtime_error(string("Analyze: error writing header '") + fname);
	return true;
}

void CAnalyze3DImageIOPlugin::do_add_suffixes(multimap<string, string>& map) const
{
	map.insert(pair<string,string>(".hdr", get_name()));
}

std::string CAnalyze3DImageIOPlugin::do_get_preferred_suffix() const
{
	return "hdr"; 
}


const string CAnalyze3DImageIOPlugin::do_get_descr() const
{
	return "analyze image IO";
}

NS_END


