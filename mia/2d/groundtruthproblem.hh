#ifndef mia_2d_ground_truth_problem_hh
#define mia_2d_ground_truth_problem_hh

#include <gsl++/multimin.hh>
#include <mia/2d/correlation_weight.hh>


NS_MIA_BEGIN


class GroundTruthProblem : public gsl::CFDFMinimizer::Problem {
public:
	GroundTruthProblem(double a, double b, 
			   const C2DBounds& slice_size, 
			   size_t nframes, 
			   const gsl::DoubleVector& left_side, 
			   const  CCorrelationEvaluator::result_type& corr); 

protected: 
	const std::vector<double>& get_spacial_gradient() const; 
	const std::vector<double>& get_time_derivative() const; 
private: 
	virtual double  do_f(const gsl::DoubleVector& x); 
	virtual void    do_df(const gsl::DoubleVector&  x, gsl::DoubleVector&  g); 
	virtual double  do_fdf(const gsl::DoubleVector&  x, gsl::DoubleVector&  g); 
	
	void evaluate_spacial_gradients(const gsl::DoubleVector& x); 
	void evaluate_time_gradients(const gsl::DoubleVector& x); 
	void evaluate_slice_gradient(gsl::DoubleVector::const_iterator ii,  std::vector<double>::iterator iout); 

	std::vector<double> m_spacial_gradient; 
	std::vector<double> m_time_derivative;
	
	double m_a;
	double m_b;
	const C2DBounds& m_slice_size; 
	size_t m_nframes; 
	size_t m_frame_size; 
	const gsl::DoubleVector& m_left_side; 
	const  CCorrelationEvaluator::result_type& m_corr; 
}; 

NS_MIA_END

#endif
