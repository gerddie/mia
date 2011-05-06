#include <mia/2d/cost.hh>


NS_BEGIN(lsd_2dimage_cost)

/**
   This class implements the least square distance measure proposed in 
   Stefan Heldmann, "Multi-Modal Registration of MR Images with a Novel 
   Least-Squares Distance Measure" Medical Imaging 2010: 
   Image Processing, SPIE, February 2010
*/

class C2DLSDImageCost : public mia::C2DImageCost {
public:
	C2DLSDImageCost();
private:

	virtual double do_value(const mia::C2DImage& a, const mia::C2DImage& b) const;

	virtual double do_evaluate_force(const mia::C2DImage& a, const mia::C2DImage& b, 
					 float scale, mia::C2DFVectorfield& force) const;

	virtual void post_set_reference(const mia::C2DImage& ref); 

	std::vector<double> m_QtQinv;
	std::vector<int>    m_Q_mappping;
};

class C2DLSDImageCostPlugin: public mia::C2DImageCostPlugin {
public:
	C2DLSDImageCostPlugin();
private:
	virtual mia::C2DImageCostPlugin::ProductPtr	do_create()const;

	const std::string do_get_descr()const;
};

NS_END
