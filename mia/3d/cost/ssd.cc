/* -*- mia-c++  -*-
 * Copyright (c) Leipzig, Madrid 2004-2010
 *
 * This program is free software; you can redistribute it and/or modify
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */


#include <mia/3d/cost.hh>

#define NS ssd_3dimage_cost
#include <mia/internal/ssd.hh>

NS_BEGIN(NS);


NS_MIA_USE;
using namespace std;
using namespace boost;

typedef CSSDCost<C3DImageCost> C3DSSDCost;
template class CSSDCost<C3DImageCost>;

class C3DSSDCostPlugin: public TSSDCostPlugin<C3DImageCostPlugin, C3DSSDCost> {
private:
	virtual const string do_get_descr()const;
	virtual bool do_test() const;
};

const string C3DSSDCostPlugin::do_get_descr()const
{
	return "3D imaga cost: sum of squared differences";
}

bool C3DSSDCostPlugin::do_test() const
{
	float src_data[27] = { 1, 2, 3,  2, 3, 4,  1, 4, 2,
			       3, 4, 6,  7, 8, 4,  2, 3, 1,
			       4, 5, 7,  3, 1, 3,  6, 7, 8 };

	float ref_data[27] = { 5, 6, 7,  2, 1, 2,  2, 3, 2,
			       6, 7, 8,  9, 0, 9,  7, 6, 5,
			       1, 2, 3,  4, 6, 8,  3, 2, 3 };

	C3DFImage *fsrc = new C3DFImage(C3DBounds(3,3,3), src_data );
	C3DFImage *fref = new C3DFImage(C3DBounds(3,3,3), ref_data );
	std::shared_ptr<C3DImage > src(fsrc);
	std::shared_ptr<C3DImage > ref(fref);

	C3DSSDCost cost;
	bool success = fabs(cost.value(*src, *ref) - 367.0 / 27.0)  < 0.001;

	C3DFVectorfield force(C3DBounds(3,3,3));

	cost.evaluate_force(*src, *ref, 0.5, force);

	cvdebug() << force(1,1,1) << " vs. " << C3DFVector(-6, -2, -4) << "\n";
	success &= (force(1,1,1) == C3DFVector(-6, -2, -4));
	return success;
 }

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C3DSSDCostPlugin();
}


NS_END


