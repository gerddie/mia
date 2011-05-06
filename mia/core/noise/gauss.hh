#include <mia/core/noisegen.hh>

NS_BEGIN(gauss_noise_generator)

class CGaussNoiseGenerator: public mia::CNoiseGenerator {
public:
	CGaussNoiseGenerator(unsigned int seed, double mu, double sigma); 
private: 
	virtual double get() const; 
	double box_muller() const;
	
	double m_mu; 		
	double m_sigma; 
	mutable bool m_use_last; 
	mutable double m_y2; 
};


class CGaussNoiseGeneratorFactory: public mia::CNoiseGeneratorPlugin {
public: 
	CGaussNoiseGeneratorFactory(); 
private: 
	mia::CNoiseGeneratorPlugin::ProductPtr do_create() const; 
	virtual const std::string do_get_descr()const; 
	bool do_test()const; 
	unsigned int m_param_seed; 
	float m_param_mu; 
	float m_param_sigma;

}; 

NS_END
