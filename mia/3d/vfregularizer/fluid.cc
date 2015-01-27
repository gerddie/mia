

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

double C3DFluidVectorfieldRegularizer::do_run(C3DFVectorfield& output, C3DFVectorfield& input) const
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

        // first iteration runs over the whole field 
        fill(update_flags.begin(), update_flags.end(), 1);

        m_fluid_row_kernel->set_data_fields(output, input, residua);
        
        do {
                iter++; 
                residuum == 0.0f; 


                m_fluid_row_kernel->set_update_fields(update_flags, dset_flags); 
                
                for (unsigned z = 1; z < work_size.z; ++z) {
                        for (unsigned y = 1; y < work_size.y; ++y) {
                                residuum += m_fluid_row_kernel->evaluate_row(y, z);
                        }
                }
                        
                
        } while (iter < m_maxiter && residuum > m_epsilon); 
        
        

        
}


        

float C3DFluidVectorfieldRegularizer::solve_at(C3DFVector_sse *v, const C3DFVector_sse& b) const 
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
        
        //
        //             V(-1,-1,0).xy - V(1,-1,0).xy + V(1,1,0).xy - V(-1,1,0).xy
        // 
        __m128 vdxy = Vm1m1p0 - Vp1m1p0 + Vp1p1p0 - Vm1p1p0; // only 1 and 2 of interest

        //
        //             V(-1,0,-1).xz - V(1,0,-1).xz + V(1,0,1).xz - V(-1,0,1).xz
        //             
	__m128 vdxz = Vm1p0m1 - Vp1p0m1 + Vp1p0p1 - Vm1p0p1; // only 1 and 3 of interest

        //
        //             V(0,-1,-1).yz - V(0,1,-1).yz + V(0,1,1).yz - V(0,-1,1).yz
        //             
        __m128 vdyz = Vp0m1m1 - Vp0p1m1 + Vp0p1p1  - Vp0m1p1; // only 2 and 3 of interest 
        
        // 
        //            V(1,0,0) + V(-1,0,0)
        // 
        __m128 sxx = Vp1p0p0 + Vm1p0p0; 

        // 
        //            V(0,1,0) + V(0,-1,0)
        // 
        __m128 syy = Vp0p1p0 + Vp0m1p0;
        
        // 
        //            V(0,0,1) + V(0,0,-1)
        // 
	__m128 szz = Vp0p0p1 + Vp0p0m1;

        // reorder to obtain

        // a_b * sxx.x + a * (syy.x + szz.x) 
        // a_b * syy.y + a * (sxx.y + szz.y) 
        // a_b * szz.z + a * (sxx.z + syy.z)

        __m128 p1 = sxx; 
        _mm_shuffle_ps(p1, szz, _MM_SHUFFLE(3,2,3,0)); // [0, szz.z, 0, sxx.x] 
        
        __m128 p2 = syy; 
        _mm_shuffle_ps(p2, p2, _MM_SHUFFLE(3,3,1,3)); // [0, 0, syy.y, 0] 
        _mm_add_ps(p1, p2);                            // [0, szz.z, syy.y, sxx.x] 

        __m128 p2 = sxx;
        _mm_shuffle_ps(szz, sxx, _MM_SHUFFLE(3,2,1,0)); // [0, sxx.z, szz.y, szz.x] 
        
        _mm_shuffle_ps(p2, syy, _MM_SHUFFLE(2,0,2,1));  // [syy.z, syy.x, sxx.z, sxx.y]
        _mm_shuffle_ps(p2, p2, _MM_SHUFFLE(0,1,0,2));         // [dc  , syy.z, sxx.y, syy.x]


        auto p = p1 * m_a_b_sse +  m_a_sse * (szz + p2); 
        
        // now q
        //   vdxy[ 0, 0, y, x ],  vdxz[ 0, z, 0, x], vdyz [0, z, 0, x] 
        // 
        // dxy.y + dxz.z   //  
        // dyz.z + dxy.x   
        // dxz.x + dyz.y


        _mm_shuffle_ps(vdxy, vdyz, _MM_SHUFFLE(0, 1, 0, 1)); // [* , vdyz.y, vdxy.x, vdxy.y]
        _mm_shuffle_ps(vdxz, vdyz, _MM_SHUFFLE(2, 0, 2, 1)); // [vdxz.z, vdxz.x, vdyz.z, vdyz.y]
        _mm_shuffle_ps(vdxz, vdxz  _MM_SHUFFLE(0, 2, 1, 3));// [*,vdxz.x, vdyz.z, vdxz.z]
        auto q = vdxz + vdxy; 
        
        auto b_sse = _mm_set_pd(b.x, b.y. b.z, 0.0f); 
        
        auto s = m_sor_sse * (b_sse + p + m_b_4_sse * q);
        auto delta = s - *v; 
        
        *v = s; 
        
        return sse_norm2(delta);

}

void C3DFluidVectorfieldRegularizer::on_size_changed()
{
        m_dx = get_size().x; 
        m_dxy = m_dx * get_size().y; 
        cvinfo() << "Temporary memory requirement will be " << 
                (4 + 2 + 32) * get_size().product() / (1024 * 1024 * 1024) << " GiB\n"; 
}



