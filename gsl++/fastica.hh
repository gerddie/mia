/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2014 Gert Wollny
 *
 * MIA is free software; you can redistribute it and/or modify
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
 * along with MIA; if not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef gslpp_fastica_hh
#define gslpp_fastica_hh

namespace gsl {

/**
   
 */

class FastICA {
public: 
	enum EApproach {appr_defl, appr_symm}; 

	class FNonlinearity {
	public: 
		void set_sample(double sample_size, size_t num_samples); 
		virtual void apply(Matrix& B, double myy, const Matrix& B) const = 0; 
	protected: 
		double get_sample_size() const; 
		size_t get_num_samples() const; 
	private: 
		double sample_size; 
		size_t num_samples; 
	}; 

	
	FastICA(const Matrix&  mix);

	bool separate(); 

	void set_approach(EApproach apr); 
	
	void set_nrof_independent_components (int in_nrIC); 
	
	void set_non_linearity (FNonlinearity *in_g); 
	
	void 	set_fine_tune (bool in_finetune); 
			
	void 	set_a1 (double a1); 
		
	void 	set_a2 (double a2); 
		
 	void 	set_mu (double mu); 
	
 	void 	set_epsilon (double epsilon); 

 	void 	set_sample_size (double sampleSize);

 	void 	set_stabilization (bool in_stabilization);

 	void 	set_max_num_iterations (int in_maxNumIterations);

 	void 	set_max_fine_tune (int in_maxFineTune);

 	void 	set_first_eig (int in_firstEig);
 
	void 	set_last_eig (int in_lastEig);
 
	void 	set_pca_only (bool in_PCAonly);
 
	void 	set_init_guess (const Matrix&  ma_initGuess);

	const Matrix& 	get_mixing_matrix () const;

	const Matrix& 	get_separating_matrix () const;

	const Matrix&	get_independent_components () const;

	int  get_nrof_independent_components () const;

	const Matrix&	get_whitening_matrix () const;

	const Matrix&	get_dewhitening_matrix () const;

	
private:
	Matrix whiten(const Matrix& signal, const Matrix& evec, const DoubleVector& eval); 

	const Matrix&  m_mix;
	
	EApproach m_approach; 
	
	int m_nrIC; 
	
	std::unique_ptr<FNonlinearity> m_nonlinearity; 
	
	bool m_finetune; 
	
	double m_a1; 
	
	double m_a2; 
	
	double m_mu; 
	
 	double m_epsilon; 
	
 	double m_sampleSize;
	
 	bool m_stabilization;
	
 	int m_maxNumIterations;
	
 	int m_maxFineTune;
	
 	int m_firstEig;
	
	int m_lastEig;
	
	bool m_PCAonly;
	
	Matrix m_initGuess;
	
	Matrix m_mixing_matrix;
	
	Matrix m_separating_matrix;
	
	Matrix	m_independent_components;
	
	Matrix	m_whitening_matrix;
	
	Matrix	m_dewhitening_matrix;
	
};

class FNonlinPow3 : public FastICA::FNonlinearity {
	virtual void apply(Matrix& B, double myy, const Matrix& B) const; 
}; 


}

#endif 
