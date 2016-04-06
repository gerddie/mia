/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2016 Gert Wollny
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

#ifndef __TRIANGULARMESH_HH
#define __TRIANGULARMESH_HH 1


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string>
#include <mia/core/iodata.hh>
#include <mia/core/ioplugin.hh>
#include <mia/core/iohandler.hh>
#include <mia/3d/vector.hh>

#ifdef WIN32
#  ifdef miamesh_EXPORTS
#    define EXPORT_MESH __declspec(dllexport)
#  else
#    define EXPORT_MESH __declspec(dllimport)
#  endif
#else
#  ifdef __GNUC__
/// define visibility for proper export 
#    define EXPORT_MESH __attribute__((visibility("default")))
#  else
#    define EXPORT_MESH
#  endif

#endif


NS_MIA_BEGIN

/** 
    @ingroup basic 
    \brief A class to deine a triangle mesh with some attributes 
    
    This class defines the representation of a triangle-mesh.  
*/

class EXPORT_MESH CTriangleMesh: public CIOData {

public:
	/** The type description provides information about the data type that is
	    used by the plug-in system */
        static const char *data_descr;

	typedef CTriangleMesh type; 


	typedef std::shared_ptr<CTriangleMesh> Pointer; 

	/// these are some flags to indicate, which data is actually available
	enum EData {ed_none = 0,
		    ed_vertex  = 1,
		    ed_normal  = 2,
		    ed_color   = 4,
		    ed_scale   = 8};

	/// \cond SELFEXPLAINING 
	typedef std::vector<T3DVector<unsigned int> >   CTrianglefield;
	typedef std::vector<C3DFVector>  CVertexfield;
	typedef std::vector<C3DFVector>  CNormalfield;
	typedef std::vector<C3DFVector>  CColorfield;
	typedef std::vector<float>       CScalefield;

	typedef std::shared_ptr<CTrianglefield > PTrianglefield;
	typedef std::shared_ptr<CVertexfield > PVertexfield;
	typedef std::shared_ptr<CNormalfield > PNormalfield;
	typedef std::shared_ptr<CColorfield >  PColorfield;
	typedef std::shared_ptr<CScalefield >  PScalefield;

	typedef CTrianglefield::value_type   triangle_type;
	typedef CVertexfield::value_type     vertex_type;
	typedef CNormalfield::value_type     normal_type;
	typedef CColorfield::value_type      color_type;
	typedef CScalefield::value_type      scale_type;

	typedef CVertexfield::const_iterator const_vertex_iterator;
	typedef CVertexfield::iterator       vertex_iterator;

	typedef CTrianglefield::const_iterator const_triangle_iterator;
	typedef CTrianglefield::iterator       triangle_iterator;

	typedef CNormalfield::const_iterator const_normal_iterator;
	typedef CNormalfield::iterator       normal_iterator;

	typedef CScalefield::const_iterator const_scale_iterator;
	typedef CScalefield::iterator       scale_iterator;

	typedef CColorfield::const_iterator const_color_iterator;
	typedef CColorfield::iterator       color_iterator;
	/// \endcond 

	/// Standart constructor creates an empty mesh
	CTriangleMesh();

	/** copy constructor makes a shallow copy of the mesh, i.e.
	    the storage for triangles, vertices, normals, textur-coordinates,
	    colors and the deformation-scale is shared between original and copy
	    \param orig the mesh to be copied
	*/
	CTriangleMesh(const CTriangleMesh& orig);

	/** contruct an mesh with uninitialized data
	    \param n_triangles number of triangles
	    \param n_vertices number of vertices
	*/
	CTriangleMesh(int n_triangles, int n_vertices);

	/** creates a new mesh from given input data
	    \param triangles the triangle data (required)
	    \param vertices the vertex data (required)
	    \param normals per vertex normals - can be NULL, in this case then the normals are calculated
	    \param colors per vertex color
	    \param scales
	*/
	CTriangleMesh(PTrianglefield triangles,
		      PVertexfield vertices,
		      PNormalfield normals,
		      PColorfield colors,
		      PScalefield scales);

	/** creates a new mesh from given input data
	    \param triangles the triangle data (required)
	    \param vertices the vertex data (required)
	*/
	CTriangleMesh(PTrianglefield triangles, PVertexfield vertices);


	/// \returns a copy of this mesh 
	CTriangleMesh *clone() const __attribute__((warn_unused_result));

	/** \returns a  mesh where the connectivity is copied, the vertices are allocated, but not set,
	    and without normals and scale values */
	CTriangleMesh clone_connectivity()const;

	/// Destructor - it releases all the data
	~CTriangleMesh();

	/*!
	  \returns a type sescription name usefull for some plugin handling
	*/
	static const std::string get_type_description();

	/*!
	  \returns a set of \a EData flags indicating which data is available
	*/
	int get_available_data() const;

	/*!
	  \returns a raw pointer to the vertices
	*/
	const void *get_vertex_pointer()const;

	/*!
	  \returns a raw pointer to the normals
	*/
	const void *get_normal_pointer()const;


	/*!
	  \returns a raw pointer to the triangles
	*/
	const void *get_triangle_pointer()const;

	/*!
	  \returns a raw pointer to the colors
	*/
	const void *get_color_pointer()const;


	/*!
	  \returns the number of triangles, the mesh consists of
	*/
	unsigned int triangle_size()const;

	/*!
	  \returns the number of vertices the mesh consists of
	*/
	unsigned int vertices_size()const;

	/*!
	  \returns a cost reference to the triangle at index i
	*/
	const triangle_type&    triangle_at(unsigned int i)const;

	/*!
	  \returns a const reference to the i-th vertex
		*/
	const vertex_type&      vertex_at(unsigned int i)const;

	/*!
	  \returns a const reference to the normal of the i-th vertex
	*/
	const normal_type&      normal_at(unsigned int i)const;

	/*!
	  \returns a const reference to the scale at the i-th vertex
	*/
	const scale_type&       scale_at(unsigned int i)const;

	/**
	   \returns the color at the given index i 
	   \param i 
	 */
	const color_type&       color_at(unsigned int i)const;

	/// \cond SELFEXPLAINING 
	const_triangle_iterator triangles_begin() const;
	triangle_iterator       triangles_begin();
	const_triangle_iterator triangles_end() const;
	triangle_iterator       triangles_end();

	const_vertex_iterator vertices_begin()const;
	const_vertex_iterator vertices_end()const;
	vertex_iterator       vertices_begin();
	vertex_iterator       vertices_end();

	const_normal_iterator normals_begin()const;
	const_normal_iterator normals_end()const;
	normal_iterator       normals_begin();
	normal_iterator       normals_end();

	const_scale_iterator scale_begin()const;
	const_scale_iterator scale_end()const;
	scale_iterator       scale_begin();
	scale_iterator       scale_end();

	const_color_iterator color_begin()const;
	const_color_iterator color_end()const;
	color_iterator       color_begin();
	color_iterator       color_end();
	/// \endcond 
	
	/** \returns the format the mesh was given in the beginning */
	const std::string& get_sourceformat()const {
			return m_sourceformat;
	}

	/** \param fmt set the save format of the mesh */
	void set_sourceformat(const std::string& fmt){
		m_sourceformat = fmt;
	}

	/** Force the re-evaluation of the normals of the mesh. The normal at a vertex
	    is evaluared as the area-weighted average of the normals of the triangles,
	    the vertex is part of.
	*/

	void evaluate_normals();

private:
	std::string m_sourceformat;
	struct CTriangleMeshData * data;
};

/// Pointer type of the CTriangle mesh class
typedef CTriangleMesh::Pointer PTriangleMesh;


PTriangleMesh EXPORT_MESH get_sub_mesh(const CTriangleMesh& mesh, const std::vector<unsigned>& triangle_indices); 
	

/** This function is used to generate  the deformation scale of a mesh as the
    scalar product of the surface normal and a deformation field.
    if neccessary the normals are evaluated
    \param mesh the mesh whose scale values will be set
    \param deform the deformation to be used to set the scale values
*/

template <class Deformation>
void colorize_mesh(CTriangleMesh  *mesh, const Deformation& deform)
{
	if (! (mesh->get_available_data() & CTriangleMesh::ed_normal))
		mesh->evaluate_normals();

	// make sure we use the const versions of some iterators to keep the sharing of
	// vertex (and evl. normal) data
	const CTriangleMesh& cmesh = *mesh;


	CTriangleMesh::scale_iterator  sb = mesh->scale_begin();
	CTriangleMesh::scale_iterator  se = mesh->scale_end();

	CTriangleMesh::const_vertex_iterator vb = cmesh.vertices_begin();
	CTriangleMesh::const_normal_iterator nb = cmesh.normals_begin();

	while (sb != se)
		*sb++ = dot(*nb++, deform.apply(*vb++));

}


/** This function is used to generate  the deformation scale of a mesh as the
    scalar product of the surface normal and a deformation field.
    \param mesh the input mesh
    \param deform the deformation to be used to set the scale values
    \returns a copy of the input mesh with additionally set scale values and possibly added normals
*/


template <class Deformation>
CTriangleMesh colorize_mesh(const CTriangleMesh& mesh, const Deformation& deform)
{
	CTriangleMesh result(mesh);
	colorize_mesh(&result,deform);
	return result;
}


/* evaluate the distance between two meshes, and set the scale accordingly
   \param mesh this mesh will get the per vertex distance values in its scale
   \param reference the mesh we want to know the distance from
   void distance_transform(CTriangleMesh *mesh, const CTriangleMesh& reference);
*/


/// IO plugin for triangular meshes
typedef TIOPlugin<CTriangleMesh> CMeshIOPlugin;

/// Plug-in handler for triangulat mesh IO 
typedef THandlerSingleton<TIOPluginHandler<CMeshIOPlugin> > CMeshIOPluginHandler;


NS_MIA_END

#endif // __TRIANGULARMESH_HH
