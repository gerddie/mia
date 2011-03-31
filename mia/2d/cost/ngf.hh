#include <numeric>
#include <boost/algorithm/minmax_element.hpp>

#include <mia/core/msgstream.hh>
#include <mia/2d/cost.hh>
#include <mia/2d/fatcost.hh>
#include <mia/2d/nfg.hh>
#include <mia/2d/2DDatafield.hh>
#include <mia/2d/2dimageio.hh>


NS_BEGIN(nfg_2dimage_cost)


class CCostEvaluator {
public:
	struct param_pass {
		mia::C2DFVectorfield::const_iterator src;
		mia::C2DFVectorfield::const_iterator ref;
		mia::C2DFVectorfield::const_iterator srcp;
		mia::C2DFVectorfield::const_iterator srcm;
	};

	virtual const  char *do_get_name() const = 0;
	virtual float get_cost(size_t x,param_pass& p) const = 0;
	virtual float get_cost_grad(size_t x,param_pass& p, mia::C2DFVectorfield::iterator g) const = 0;
};


class CCostEvaluatorCross: public CCostEvaluator {
public:
	typedef CCostEvaluator::param_pass param_pass;
	virtual const  char *do_get_name() const;
	virtual float get_cost(size_t x,param_pass& p) const;
	virtual float get_cost_grad(size_t x,param_pass& p, mia::C2DFVectorfield::iterator g)const;
};

class CCostEvaluatorScalar: public CCostEvaluator {
public:
	typedef CCostEvaluator::param_pass param_pass;
	virtual const  char *do_get_name() const ;
	virtual float get_cost(size_t x,param_pass& p) const;
	virtual float get_cost_grad(size_t x,param_pass& p, mia::C2DFVectorfield::iterator g) const ;
};


class CCostEvaluatorSQDelta: public CCostEvaluator {
public:
	typedef CCostEvaluator::param_pass param_pass;

	virtual const  char *do_get_name() const;
	virtual float get_cost(size_t x,param_pass& p) const;
	virtual float get_cost_grad(size_t x,param_pass& p, mia::C2DFVectorfield::iterator g)const;
private:
	static const float scale;
	float get_help_value(const mia::C2DFVector& ref, float dot_rs) const;
};

class CCostEvaluatorDeltaScalar: public CCostEvaluator {
public:
	typedef CCostEvaluator::param_pass param_pass;

	virtual const  char *do_get_name() const;
	virtual float get_cost(size_t x,param_pass& p) const;
	virtual float get_cost_grad(size_t x,param_pass& p, mia::C2DFVectorfield::iterator g)const;
private:
	static const float scale;
};


typedef std::shared_ptr<CCostEvaluator > PEvaluator;

class C2DNFGImageCost : public mia::C2DImageCost {
public:
	C2DNFGImageCost(PEvaluator evaluator);
	virtual void prepare_reference(const mia::C2DImage& ref)__attribute__((deprecated)); 
private:
	virtual double do_value(const mia::C2DImage& a, const mia::C2DImage& b) const ;
	virtual double do_evaluate_force(const mia::C2DImage& a, const mia::C2DImage& b, float scale, mia::C2DFVectorfield& force) const;

	virtual void post_set_reference(const mia::C2DImage& ref); 

	mia::C2DFVectorfield _M_ng_ref;
	bool _M_jump_levels_valid;
	float _M_cost_baseline;

	PEvaluator _M_evaluator;
	float _M_intensity_scale;
};

class C2DNFGImageCostPlugin: public mia::C2DImageCostPlugin {
public:
	C2DNFGImageCostPlugin();
private:
	virtual mia::C2DImageCostPlugin::ProductPtr	do_create()const;

	bool do_test() const;
	const std::string do_get_descr()const;
	std::string _M_kernel;
};

NS_END
