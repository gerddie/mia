/***/

namespace std {

template <typename T> 
swap(T* &a, T* &b) {
        return; 
}

}

void BOOST_REQUIRE(bool x) 
{
        if (!x)
		__coverity_panic__();  
  
}


void BOOST_REQUIRE(const void *x) 
{
        if (!x)
		__coverity_panic__();  
  
}

