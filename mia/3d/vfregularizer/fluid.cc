

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
        
        size_t i = 0;
        
        // first iteration runs over the whole field 
        fill(update_flags.begin(), update_flags.end(), 1);

        
        
        

        
}

void C3DFluidVectorfieldRegularizer::on_size_changed()
{
        m_dx = get_size().x; 
        m_dxy = m_dx * get_size().y; 
        cvinfo() << "Temporary memory requirement will be " << 
                (4 + 2 + 32) * get_size().product() / (1024 * 1024 * 1024) << " GiB\n"; 
}



