#ifndef mia_2d_ground_truth_problem_hh
#define mia_2d_ground_truth_problem_hh

#include <mia/core/minimizer.hh>
#include <mia/2d/correlation_weight.hh>


NS_MIA_BEGIN

/**
   \brief a class for the evaluation of a pseudo ground truth of a perfusion series  

   This class implements the optimization problem required for Ground Thruth Estimation 
   as described in 
     Chao Li and Ying Sun, Nonrigid Registration of Myocardial Perfusion MRI Using Pseudo Ground Truth,
     In Proc. Medical Image Computing and Computer-Assisted Intervention – MICCAI 2009,
     165-172, 2009
   For its use see the class C2DGroundTruthEvaluator. 
*/

class  EXPORT_2D GroundTruthProblem : public CMinimizer::Problem {
public:

	/**
	   Create the ground thruth estimator with the given parameters 
	   (see the paper for details on \f$\alpha\f$ and \f$\beta\f$) 
	   @param a parameter \f$\alpha\f$ 
	   @param b parameter \f$\beta\f$ 
	   @param slice_size 2D image size of the series images, 
	   @param nframes number of frames in the perfusion series 
	   @param left_side 
	   @param corr 
	   
	 */
	GroundTruthProblem(double a, double b, 
			   const C2DBounds& slice_size, 
			   size_t nframes, 
			   const CDoubleVector& left_side, 
			   const  CCorrelationEvaluator::result_type& corr); 

	/**
	   Set the parametes \f$\alpha\f$ and \f$\beta\f$
	 */
	void set_alpha_beta(double a, double b); 

protected: 
	
	///@returns a reference to the spacial gradient 
	const std::vector<double>& get_spacial_gradient() const; 

	///@returns a reference to the temporal gradient 
	const std::vector<double>& get_time_derivative() const; 
private: 
	virtual double  do_f(const CDoubleVector& x); 
	virtual void    do_df(const CDoubleVector&  x, CDoubleVector&  g); 
	virtual double  do_fdf(const CDoubleVector&  x, CDoubleVector&  g); 
	size_t do_size() const; 
	
	double evaluate_spacial_gradients(const CDoubleVector& x); 
	double  evaluate_time_gradients(const CDoubleVector& x); 
	double evaluate_slice_gradient(CDoubleVector::const_iterator ii,  std::vector<double>::iterator iout); 

	std::vector<double> m_spacial_gradient; 
	std::vector<double> m_time_derivative;
	
	double m_a;
	double m_b;
	const C2DBounds& m_slice_size; 
	size_t m_nframes; 
	size_t m_frame_size; 
	const CDoubleVector& m_left_side; 
	const  CCorrelationEvaluator::result_type& m_corr; 
}; 

NS_MIA_END

#endif
