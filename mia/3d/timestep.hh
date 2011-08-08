/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
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

#ifndef mia_3d_timestep_hh
#define mia_3d_timestep_hh

#include <mia/core/factory.hh>
#include <mia/core/typedescr.hh>
#include <mia/3d/3DImage.hh>

NS_MIA_BEGIN

/**
   \brief The time-step part of time-marching registration algorithms. 

   

*/

class EXPORT_3D C3DRegTimeStep :public CProductBase {
public:
	/// plugin hanlder helper  
	typedef C3DImage plugin_data; 
	/// plugin hanlder helper 
	typedef timestep_type plugin_type; 

	/**
	   Contructor to initialize the time setep with the range of possible 
	   time step length 
	   \param min
	   \param max
	 */
	C3DRegTimeStep(float min, float max);

	virtual ~C3DRegTimeStep();

	/**
	   Apply the time step based on an input vector field to an output transformation 
	   \param infield input vector field to add
	   \param[in,out] outfield at input: vector field that is to be changed, at output: updated field 
	   \param scale a scaling to apply to the input field prior to applying it 
	 */
	void apply(const C3DFVectorfield& infield, C3DFVectorfield& outfield, float scale);

	/**
	   Evaluate the pertuberation related to the time step type and transformation 
	   \param[in,out] io at input: the field to pertoperate, output: pertuperated field 
	   \param shift related current transformation to base the pertuberation on 
	 */
	float calculate_pertuberation(C3DFVectorfield& io, const C3DFVectorfield& shift) const;

	/**
	   Decrease the current time step 
	   \returns true if decreasing time-step was sucessfull, otherwise false 
	   
	 */
	bool decrease();

	/**
	   Increase time step (at most to its maximum)
	 */
	void increase();

	/**
	   Evaluate the delta for the application of the time step based on the maxium allowed 
	   transformation 
	   \param maxshift maximum hypothetical transfornmation to be applied 
	   \returns the delta value corresponding to the current time step 
	 */
	float get_delta(float maxshift) const;

	/**
	   For a fluid-dynamic like registration evaluate whether a regridding is needed. 
	   For other time-steps it always should return false 
	   \param b current force field
	   \param v velocity field 
	   \param delta time step value to be applied 
	   \returns true if regridding is to be applied 
	 */
	bool regrid_requested (const C3DFVectorfield& b, const C3DFVectorfield& v, float delta) const;

	/**
	   \returns true if the time-step requires regridding 
	 */
	bool has_regrid () const;

private:
	virtual float do_calculate_pertuberation(C3DFVectorfield& io, const C3DFVectorfield& shift) const = 0;
	virtual bool do_regrid_requested (const C3DFVectorfield& b, const C3DFVectorfield& v, float delta) const = 0;
	virtual bool do_has_regrid () const = 0;


	float m_min;
	float m_max;
	float m_current;
	float m_step;
};


/**
   \brief The plug-in class for the time-step class of time-marching registration algorithms. 

 

*/
class EXPORT_3D C3DRegTimeStepPlugin : public TFactory<C3DRegTimeStep>
{
public:
	/// Constructor to create the plug-in with its name 
	C3DRegTimeStepPlugin(const char *name);
protected:
	/// \returns the minimal allowed time step 
	float get_min_timestep() const;
	/// \returns the maximum allowed time step 
	float get_max_timestep() const;
private: 
	float m_min;
	float m_max;
};

/// Pointer type for time steps in 3D time-marching image registration \sa C3DRegTimeStep
typedef std::shared_ptr<C3DRegTimeStep > P3DRegTimeStep;

/// plugin handler for 3D time marching time step operator plug-ins 
typedef THandlerSingleton<TFactoryPluginHandler<C3DRegTimeStepPlugin> > C3DRegTimeStepPluginHandler;

NS_MIA_END

#endif
