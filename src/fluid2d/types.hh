#ifndef __types_h
#define __types_h

#if HAVE_CONFIG_H
#include <config.h>
#endif

typedef unsigned char uint8;
typedef signed char int8;

typedef unsigned short uint16;
typedef short int16;

typedef unsigned int uint32;
typedef int int32;


#ifdef WORDS_BIGENDIAN
inline void ENDIANDAPT(int16& x) { x = ((x & 0xFF) << 8)| ((x >> 8) & 0xFF);}
inline void ENDIANADAPT(uint16& x) { x = ((x & 0xFF) << 8)| ((x >> 8) & 0xFF);}
#else
#define ENDIANADAPT(x)
#endif


#endif

/* CVS LOG

   $Log: types.hh,v $
   Revision 1.2  2002/06/20 09:59:47  gerddie
   added cvs-log entry


*/

