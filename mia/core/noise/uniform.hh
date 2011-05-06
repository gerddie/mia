#include <cmath>

#include <limits>
#include <vector>

#include <mia/core/noisegen.hh>


NS_BEGIN(uniform_noise_generator)
class CUniformNoiseGenerator: public mia::CNoiseGenerator {
public:
	CUniformNoiseGenerator(unsigned int seed, double a, double b); 
private: 
	virtual double get() const; 
		double m_a; 
	double m_step; 
};

class CUniformNoiseGeneratorFactory: public mia::CNoiseGeneratorPlugin {
public: 
	CUniformNoiseGeneratorFactory(); 
private: 
	mia::CNoiseGeneratorPlugin::ProductPtr do_create() const; 
	virtual const std::string do_get_descr()const; 
	bool do_test()const; 
	unsigned int m_param_seed; 
	float m_param_a; 
	float m_param_b; 
}; 

NS_END
