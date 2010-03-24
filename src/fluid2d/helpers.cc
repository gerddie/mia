#include "helpers.hh"

int32 log2(uint32 in)
{
	int32 res = -1;
	while(in){
		res++;
		in >>= 1;
	}
	return res;
}

uint32 exp2(int32 in)
{
	if (in < 0)
		return 0;
	
	uint32 res =1;
	while (in--){
		res <<= 1; 
	}
	return res;
}

/* CVS LOG

   $Log: helpers.cc,v $
   Revision 1.3  2002/06/20 09:59:47  gerddie
   added cvs-log entry


*/

