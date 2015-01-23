

C3DFluidVectorfieldRegularizer::C3DFluidVectorfieldRegularizer(float mu, float lambda, 
                                                               size_t maxiter, float omega, float epsilon):
        m_mu(mu), 
        m_lambda(lambda),  
        m_omega(omega), 
        m_epsilon(epsilon),
        m_maxiter(maxiter), 
        m_dx(0), 
        m_dxy(0)
{
        float a = mu;
	float b = lambda + mu;
	m_c = 1 / (6.0f * a + 2.0f * b);
	m_b_4 = 0.25f * b * m_c;
	m_a_b = ( a + b ) * m_c;
	m_a = a * m_c;
}


double do_run(C3DFVectorfield& output, C3DFVectorfield& input) const
{
        // reserve space for temporaries
        T3DDatafield<float> residua(get_size());
        T3DDatafield<unsigned char> update_flags1(get_size());
        T3DDatafield<unsigned char> update_flags2(get_size());
        
        // these two will ping-pong in each update 
        T3DDatafield<unsigned char>& update_flags = update_flags1;
        T3DDatafield<unsigned char>& dset_flags = update_flags2;
        
        float start_residuum = 0.0;
        float residuum;
        float thresh = 0.0;
        
        size_t iter = 0;

        C3DBounds work_size = size - C3DBounds::_1; 
        
        // first iteration runs over the whole field 
        fill(update_flags.begin(), update_flags.end(), 1);

        do {
                iter++; 
                residuum == 0.0f; 
                
                size_t linear_index = m_dxy + m_dx + 1; 
                for (unsigned z = 1; z < work_size.z; ++z) {
                        for (unsigned y = 1; y < work_size.y; ++y) {
                                for (unsigned x = 1; x < work_size.x; ++x, ++linear_index) {
                                        float r = solve_at(&output[linear_index], input[linear_index]); 
                                }
                                linear_index += 2; 
                        }
                        linear_index += m_dx << 1; 
                }
                        
                
        } while (iter < m_maxiter && residuum > m_epsilon); 
        
        

        
}

float C3DFluidVectorfieldRegularizer::solve_at(C3DFVector *v, const C3DFVector& b) const 
{
        C3DFVector *vm = v - m_dxy; 

        __m128 	Vp0m1m1 = _mm_loadu_ps(&vm[ -m_dx]);
        __m128 	Vm1p0m1 = _mm_loadu_ps(&vm[ -1   ]);
	__m128 	Vp0p0m1 = _mm_loadu_ps(&vm[  0   ]);
	__m128 	Vp1p0m1 = _mm_loadu_ps(&vm[  1   ]);
	__m128 	Vp0p1m1 = _mm_loadu_ps(&vm[  m_dx]);

        vm = v - m_dx; 

        __m128 	Vm1m1p0 = _mm_loadu_ps(&vm[ -1 ]);
	__m128 	Vp0m1p0 = _mm_loadu_ps(&vm[  0 ]);
	__m128 	Vp1m1p0 = _mm_loadu_ps(&vm[  1 ]);

        __m128 Vp0p0m1 = _mm_loadu_ps(&v[-1]); 
        __m128 Vp0p0p1 = _mm_loadu_ps(&v[1]); 

        vm = v + m_dx; 

        __m128 	Vm1p1p0 = _mm_loadu_ps(&vm[ -1 ]);
	__m128 	Vp0p1p0 = _mm_loadu_ps(&vm[  0 ]);
	__m128 	Vp1p1p0 = _mm_loadu_ps(&vm[  1 ]);
        
        vm = v + m_dxy; 

        __m128 	Vp0m1p1 = _mm_loadu_ps(&vm[ -m_dx]);
        __m128 	Vm1p0p1 = _mm_loadu_ps(&vm[ -1   ]);
	__m128 	Vp0p0p1 = _mm_loadu_ps(&vm[  0   ]);
	__m128 	Vp1p0p1 = _mm_loadu_ps(&vm[  1   ]);
	__m128 	Vp0p1p1 = _mm_loadu_ps(&vm[  m_dx]);
        
        
        __m128 	vdxy = Vm1m1p0 - Vp1m1p0 + Vp1p1p0 - Vm1p1p0; // only 1 and 2 of interest
	__m128 	vdxz = Vm1p0m1 - Vp1p0m1 + Vp1p0p1 - Vm1p0p1; // only 1 and 3 of interest

        __m128 vdxx = Vp1p0m0 + Vm1p0p0;
        __m128 vdyy = Vp0p1p0 + Vp0m1p0;
	__m128 vdzz = Vp0p0p1 + Vp0p0m1;

        __m128 vydyz = Vp0m1m1 - Vp0p1m1 + Vp0p1p1  - Vp0m1p1; // only 2 and 3 of interest 

        
        
        __m128 p1 = _mm_shuffle_ps(vdxx, vdyy, _MM_SHUFFLE(0, 1, 1, 0));
        __m128 p2 = _mm_shuffle_ps(vdyy, vdzz, _MM_SHUFFLE(0, 1, 1, 0));
                                   
        p1 = _mm_shuffle_ps(p1, vdzz, _MM_SHUFFLE(0, 2, 2, 0));   // p1 should now contain (xx.x, yy.y, zz.z)
                            
        __m128 p2 = _mm_shuffle_ps(vdxx, vdzz, _MM_SHUFFLE(0, 2, 1, 3));
        

}

void C3DFluidVectorfieldRegularizer::on_size_changed()
{
        m_dx = get_size().x; 
        m_dxy = m_dx * get_size().y; 
        cvinfo() << "Temporary memory requirement will be " << 
                (4 + 2 + 32) * get_size().product() / (1024 * 1024 * 1024) << " GiB\n"; 
}



