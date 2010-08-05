#include <mia/core/noisegen.hh>

NS_BEGIN(gauss_noise_generator)

class CGaussNoiseGenerator: public mia::CNoiseGenerator {
public:
	CGaussNoiseGenerator(unsigned int seed, double mu, double sigma); 
private: 
	virtual double get() const; 
	double box_muller() const;
	
	double _M_mu; 		
	double _M_sigma; 
	mutable bool _M_use_last; 
	mutable double _M_y2; 
};


class CGaussNoiseGeneratorFactory: public mia::CNoiseGeneratorPlugin {
public: 
	CGaussNoiseGeneratorFactory(); 
private: 
	mia::CNoiseGeneratorPlugin::ProductPtr do_create() const; 
	virtual const std::string do_get_descr()const; 
	bool do_test()const; 
	unsigned int _M_param_seed; 
	float _M_param_mu; 
	float _M_param_sigma;

}; 

NS_END
