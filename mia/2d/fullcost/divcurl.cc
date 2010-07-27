

C2DDivCurlFullCost::C2DDivCurlFullCost(double div, double curl, double weight):
	C2DFullCost(weight), 
	_M_div(div), 
	_M_curl(curl)
{

}

double C2DDivCurlFullCost::do_evaluate(const C2DTransformation& t, gsl::DoubleVector& gradient) const
{
	assert(t.get_size() == get_current_size()); 
	return t.get_divcurl_cost(gradient); 
}

void C2DDivCurlFullCost::do_set_size()
{
}

/*
	add_parameter("div", new CFloatParameter(_M_div, 0.0, numeric_limits<float>::max(), 
						 false, "penalty weight on divergence"));
	add_parameter("curl", new CFloatParameter(_M_curl, false, "penalty weight on curl"));
*/
