/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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

#ifndef mia_2d_timestep_hh
#define mia_2d_timestep_hh

#include <mia/core/factory.hh>
#include <mia/core/typedescr.hh>
#include <mia/2d/image.hh>
#include <mia/2d/transform.hh>

NS_MIA_BEGIN

/**
   \ingroup registration
   \brief The time step class for time-marching registration algorithms

   Registration time step for  a time-marching registration algorithm like 
   those based on a variational model. 
   Generally the time marhcing model consists of two parts, a PDE describing 
   the regularization model  of the registration and the time step that 
   advances the registration transformation towards a minimum.  
  
*/
class EXPORT_2D C2DRegTimeStep : public CProductBase {
public:
	/// plugin search path data component helper
	typedef C2DImage plugin_data; 
	
	/// plugin search path plugin type component helper 
	typedef timestep_type plugin_type; 

	/**
	   Constructor 
	   \param min minimum time step 
	   \param max maximum time step
	 */

	C2DRegTimeStep(float min, float max);

	
	virtual ~C2DRegTimeStep();

	/**
	   Evaluate the pertuberation of the vector field in combination with the next 
	   transformation to be applied. What actually happens here depends on the 
	   time step model. 
	   
	   \param[in,out] io vector field resulting from the solution of the PDE, 
	   may be overwritted by its pertuberated version 
	   \param shift current transformation 
	   \returns the norm of the maxium transformation over the transformation domain 
	 */
	float calculate_pertuberation(C2DFVectorfield& io, 
				      const C2DTransformation& shift) const;

	/**
	   Depending on the time step model, a regridding may be used  - specifically this 
	   is the case for the fluid dynamic model 
	   \param b current transformation 
	   \param v velocity field in case of a fluid dynamic model 
	   \param delta time step 
	   \returns true if regridding is necessary, false if not
	 */
	bool regrid_requested(const C2DTransformation& b, const C2DFVectorfield& v, 
			      float delta) const;

	/**
	   Decrease the time step by dividing by 2.0. - if the time step falls 
	   below the appointed minimum  it will be adjusted accordingly 
	   \returns true if the time-step was decreased, and false if the time 
	   step was already at the minimum 
	 */
	bool decrease();

	/// increase thetime step by multiplying with 1.5
	void increase();

	/** evaluate the time step based on the maximum shift resulting from 
	    \a calculate_pertuberation 
	    \param maxshift maximum shift allowed for all pixles 
	    \returns the time step delta to be used  
	*/
	float get_delta(float maxshift) const;

	/** \returns true if the time step model supports regridding and false if not */
	bool has_regrid () const;

private:
	virtual float do_calculate_pertuberation(C2DFVectorfield& io, 
						 const C2DTransformation& shift) const = 0;
	virtual bool do_regrid_requested (const C2DTransformation& b, 
					  const C2DFVectorfield& v, float delta) const = 0;
	
	virtual bool do_has_regrid () const = 0;
	
	float m_min;
	float m_max;
	float m_current;
	float m_step;
};

/// pointer type for the 2D registration time step 
typedef std::shared_ptr<C2DRegTimeStep > P2DRegTimeStep;

/**
   \ingroup infrastructur 
   \brief Plug-in to create the time step evaluation 

   Plug-in to create the time step evaluation in time-marching registration 
   algorithms.  
*/
class EXPORT_2D C2DRegTimeStepPlugin : public TFactory<C2DRegTimeStep>
{
public:
	/**
	   Initialize the plug in with its given name 
	   @param name 
	 */
	C2DRegTimeStepPlugin(const char *name);
protected:
	/// @returns minium time step 
	float get_min_timestep() const;

	/// @returns maximum time setp 
	float get_max_timestep() const;
private: 
	float m_min;
	float m_max;
};

/** 
    \ingroup infrastructur 
    Time step model plugin handler 
*/
typedef THandlerSingleton<TFactoryPluginHandler<C2DRegTimeStepPlugin> > 
         C2DRegTimeStepPluginHandler;

NS_MIA_END

#endif
