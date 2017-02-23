/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2017 Gert Wollny
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

#include <gts.h>
#include <mia/3d/image.hh>
#include <mia/2d/imageio.hh>
#include <mia/core/filter.hh>

using namespace mia; 
using std::vector; 
using std::string; 
using std::invalid_argument; 

class CImage2Double: public TFilter<C3DDImage> {
public:	
	template <class Data3D>
	typename CImage2Double::result_type operator () (Data3D& data)const {
		C3DDImage result(data.get_size());
		std::copy(data.begin(), data.end(), result.begin());
		return result; 
	}
};


static void iso_func(gdouble ** f, GtsCartesianGrid g, guint z, gpointer data)
{
	const C3DDImage *image = (C3DDImage *)data; 
	
	const double *p = &(*image)(0,0,z);
	cvmsg() << "extracting ..." << (100 * (z+1))/g.nz << "%\r";  
	
	
	for (size_t y = 0; y < g.ny; y++)	
		for (size_t x = 0; x < g.nx; ++x,++p)
			f[x][y] = *p;
}


struct FCopySlice: public TFilter<void> {
	FCopySlice(double **f):m_f(f){}; 
	
	template <typename T> 
	void operator()(const T2DImage<T>& image) const {
		auto p = image.begin(); 
		for (size_t y = 0; y < image.get_size().y; y++)	
			for (size_t x = 0; x < image.get_size().x; ++x,++p)
				m_f[x][y] = *p;
	}
	double **m_f; 
}; 
	
static void iso_func_from_slices(gdouble ** f, GtsCartesianGrid g, guint z, gpointer data)
{
	const vector<string> *slices = reinterpret_cast<const vector<string> *>(data); 

        auto slice_image = load_image2d((*slices)[z]); 
	
	if (slice_image->get_size().x != g.nx ||
	    slice_image->get_size().y != g.ny ) 
		throw create_exception<invalid_argument>("Input slice of wrong size, got ", slice_image->get_size(), 
							 ", expect <", g.nx, ", ", g.ny, ">"); 
	
	
	cvmsg() << "extracting ..." << (100 * (z+1))/g.nz << "%\r";  

	FCopySlice cs(f); 
	mia::filter(cs, *slice_image); 
}


static void iso_func_b(gdouble ** f, GtsCartesianGrid g, guint z, gpointer data)
{
	const C3DDImage *image = (C3DDImage *)data; 

	cvmsg() << "extracting slice ..."<< z +1 <<" of " << g.nz << "\r";
	
	if (z == 0 || z == image->get_size().z+1) {
		for (size_t y = 0; y < g.ny; ++y)
			for (size_t x = 0; x < g.nx; ++x)
				f[x][y] = 0;
		return; 
	}
		
	const double *p = &(*image)(0,0,z-1);
	size_t x,y; 

	
	for (x = 0; x < g.nx; ++x)
		f[x][0] = 0;
	for (y = 1; y < g.ny-1; y++) {
		f[0][y] = 0;
		for (x = 1; x < g.nx-1; ++x,++p)
			f[x][y] = *p;
		f[x][y] = 0;
	}
	for (x = 0; x < g.nx; ++x)
		f[x][y] = 0;
}

typedef struct {
	GtsSurface *surface;
	int max_faces; 
	int max_edges; 
	guint nmax;
	guint nold;
	GTimer* timer;
	GTimer* total_timer;
	const char *cost_name; 
} stop_info_t;


static gboolean stop_nedges (gdouble MIA_PARAM_UNUSED(cost), guint number, stop_info_t *stop_info)
{
	g_return_val_if_fail (stop_info != NULL, TRUE);
	return (number < (guint)stop_info->max_edges);
}


static gboolean stop_nfaces(gdouble MIA_PARAM_UNUSED(cost), guint MIA_PARAM_UNUSED(nedges), stop_info_t *stop_info) 
{
	g_return_val_if_fail (stop_info != NULL, TRUE);

	return ( gts_surface_face_number(stop_info->surface) < (guint)stop_info->max_faces);
}

static gboolean stop_cost(gdouble cost, guint MIA_PARAM_UNUSED(nedges), gdouble *max_cost) 
{
	g_return_val_if_fail (max_cost != NULL, TRUE);
	return (cost > *max_cost);
}

static gboolean stop_nedges_verbose (gdouble cost, guint number, stop_info_t *stop_info)
{
	g_return_val_if_fail (stop_info != NULL, TRUE);
	
	if (stop_info->timer == NULL) {
		stop_info->nmax = stop_info->nold = number;
		stop_info->timer = g_timer_new ();
		stop_info->total_timer = g_timer_new ();
		g_timer_start (stop_info->total_timer);
	}
	
	if (number != stop_info->nold && number % 121 == 0 &&
	    number < stop_info->nmax && 
	    (gint)stop_info->nmax > stop_info->max_edges) {
		gdouble total_elapsed = g_timer_elapsed (stop_info->total_timer, NULL);
		gdouble remaining;
		gdouble hours, mins, secs;
		gdouble hours1, mins1, secs1;
		
		g_timer_stop (stop_info->timer);
		
		hours = floor (total_elapsed/3600.);
		mins = floor ((total_elapsed - 3600.*hours)/60.);
		secs = floor (total_elapsed - 3600.*hours - 60.*mins);
		
		remaining = total_elapsed*((stop_info->nmax - stop_info->max_edges)/(gdouble) (stop_info->nmax - number) - 1.);
		hours1 = floor (remaining/3600.);
		mins1 = floor ((remaining - 3600.*hours1)/60.);
		secs1 = floor (remaining - 3600.*hours1 - 60. * mins1);

		fprintf (stderr, 
			 "\rCost (%s): %10.5f "
			 "Edges: %10u %3.0f%% %6.0f edges/s "
			 "Elapsed: %02.0f:%02.0f:%02.0f "
			 "Remaining: %02.0f:%02.0f:%02.0f",
			 stop_info->cost_name, 
			 cost,
			 number, 
			 100.*(stop_info->nmax - number)/(stop_info->nmax - stop_info->max_edges),
			 (stop_info->nold - number)/g_timer_elapsed (stop_info->timer, NULL),
			 hours, mins, secs,
			 hours1, mins1, secs1);
		fflush (stderr);

		stop_info->nold = number;
		g_timer_start (stop_info->timer);
	}
	if ((gint)number < stop_info->max_edges) {
		g_timer_destroy (stop_info->timer);
		g_timer_destroy (stop_info->total_timer);
		return TRUE;
	}
	return FALSE;
}


static int stop_nfaces_verbose(gdouble cost, guint MIA_PARAM_UNUSED(nedges), stop_info_t *stop_info) 
{
	size_t number;
	
	g_return_val_if_fail (stop_info != NULL, TRUE);

	number = gts_surface_face_number(stop_info->surface); 

	if (stop_info->timer == NULL) {
		stop_info->nmax = stop_info->nold = number;
		stop_info->timer = g_timer_new ();
		stop_info->total_timer = g_timer_new ();
		g_timer_start (stop_info->total_timer);
	}
	
	if (number != stop_info->nold && number % 121 == 0 &&
	    number < stop_info->nmax && 
	    (gint)stop_info->nmax > stop_info->max_faces) {
		gdouble total_elapsed = g_timer_elapsed (stop_info->total_timer, NULL);
		gdouble remaining;
		gdouble hours, mins, secs;
		gdouble hours1, mins1, secs1;
		
		g_timer_stop (stop_info->timer);
		
		hours = floor (total_elapsed/3600.);
		mins = floor ((total_elapsed - 3600.*hours)/60.);
		secs = floor (total_elapsed - 3600.*hours - 60. *mins);
		
		remaining = total_elapsed*((stop_info->nmax - stop_info->max_faces)/(gdouble) (stop_info->nmax - number) - 1.);
		hours1 = floor (remaining/3600.);
		mins1 = floor ((remaining - 3600.*hours1)/60.);
		secs1 = floor (remaining - 3600.*hours1 - 60.*mins1);
		
		fprintf (stderr, 
			 "\rCost(%s): %10.5f "
			 "Faces: %10u %3.0f%% %6.0f faces/s "
			 "Elapsed: %02.0f:%02.0f:%02.0f "
			 "Remaining: %02.0f:%02.0f:%02.0f",
			 stop_info->cost_name, 
			 cost,
			 static_cast<unsigned int>(number), 
			 100.*(stop_info->nmax - number)/(stop_info->nmax - stop_info->max_faces),
			 (stop_info->nold - number)/g_timer_elapsed (stop_info->timer, NULL),
			 hours, mins, secs,
			 hours1, mins1, secs1);
		fflush (stderr);

		
		stop_info->nold = number;
		g_timer_start (stop_info->timer);
	}
	if ((gint)number < stop_info->max_faces) {
		g_timer_destroy (stop_info->timer);
		g_timer_destroy (stop_info->total_timer);
		return TRUE;
	}
	return FALSE;
}

static int stop_cost_verbose(gdouble cost, guint nedges, gdouble *max_cost) 
{
	static int hide = 200; 
	
	g_return_val_if_fail (max_cost != NULL, TRUE);
	
	if (!hide--) {
		cvmsg()  << "\rCost(volume change): "<< std::setw(10) 
			 << cost << " Edges: "<< std::setw(10) << nedges;
		hide = 200; 
	}
	
	if (cost > *max_cost){
		return TRUE;
	}
	return FALSE; 	
}

void do_revert(GtsTriangle *t, gpointer *MIA_PARAM_UNUSED(data)) 
{
	gts_triangle_revert(t);
}

static GtsSurface *extract_surface(const vector<string>& slices, gdouble iso_value, bool MIA_PARAM_UNUSED(use_border))
{
	GtsCartesianGrid g;	
	GtsSurface *surface; 

	auto prototype = load_image2d(slices[0]); 
	
	
	C2DFVector vox = prototype->get_pixel_size(); 
	g.dx  = vox.x; 
	g.dy  = vox.y; 
	g.dz  = 1; 
	cvmsg() << "voxel = " << g.dx <<","<< g.dy <<","<< g.dz << '\n';

	g.x = 0.0; g.nx = prototype->get_size().x; 
	g.y = 0.0; g.ny = prototype->get_size().y; 
	g.z = 0.0; g.nz = slices.size();

	surface = gts_surface_new (gts_surface_class (), gts_face_class (), gts_edge_class (),
				   gts_vertex_class ());
	
	if (surface)
		gts_isosurface_tetra(surface, g, iso_func_from_slices, (gpointer)&slices, iso_value);
	gts_surface_foreach_face(surface,(GtsFunc)do_revert,0);
	
	return surface; 
}

static GtsSurface *extract_surface(const C3DImage& src, gdouble iso_value, bool use_border)
{
	GtsCartesianGrid g;	
	GtsSurface *surface; 
	
	C3DFVector vox = src.get_voxel_size();
	g.dx  = vox.x; 
	g.dy  = vox.y; 
	g.dz  = vox.z; 
	cvmsg() << "voxel = " << g.dx <<","<< g.dy <<","<< g.dz << '\n';

	g.x = 0.0; g.nx = src.get_size().x; 
	g.y = 0.0; g.ny = src.get_size().y; 
	g.z = 0.0; g.nz = src.get_size().z;

	surface = gts_surface_new (gts_surface_class (), gts_face_class (), gts_edge_class (),
				   gts_vertex_class ());
	
	CImage2Double img2double;
	
	C3DDImage scr_double = mia::filter(img2double, src); 

	if (surface) {
		if (use_border) {
			g.x = -g.dx; g.nx += 2; 
			g.y = -g.dy; g.ny += 2; 
			g.z = -g.dz; g.nz += 2;
			gts_isosurface_tetra(surface, g, iso_func_b, 
						     (gpointer)&scr_double, iso_value);
		}else
			gts_isosurface_tetra(surface, g, iso_func, 
					     (gpointer)&scr_double, iso_value);
	}
	gts_surface_foreach_face(surface,(GtsFunc)do_revert,0);
	
	return surface; 
}

static GtsSurface *iso_surface_optimize(GtsSurface *mesh, gint max_edges, gint max_faces, 
			gdouble max_cost, gfloat coarsen_method_factor)
{
	bool verbose = cverb.get_level() <= vstream::ml_message;
     	stop_info_t stop_info ={ NULL, -1, -1,0,0, NULL, NULL, NULL};
	gdouble fold = M_PI/180.;
	GtsVolumeOptimizedParams params = { 0.5, 0.5, 0. };
	
	stop_info.max_faces = max_faces; 
	stop_info.max_edges = max_edges; 
	stop_info.cost_name = "volume change"; 
	stop_info.surface = mesh; 

	if (!stop_info.surface) {
		cverr()<<"Unable to generate surface" << '\n'; 
		return NULL; 
	}
	if (verbose) {
		gts_surface_print_stats(stop_info.surface, stderr);		
	}
	cvmsg() << "Optimizing for " << stop_info.max_edges << " edges, " 
		<< stop_info.max_faces << " faces, or cost " << max_cost; 
	
	
	if (stop_info.max_edges > 0) {
		stop_info_t stop_info_first ={NULL, -1, -1, 0, 0, NULL, NULL, NULL};
		
		stop_info_first.surface = stop_info.surface; 
		stop_info_first.max_edges = int(coarsen_method_factor * stop_info.max_edges);
		stop_info_first.cost_name = "edge length"; 
	
		gts_surface_coarsen(stop_info.surface,NULL,
				    (gpointer)&params,
				    NULL,
				    (gpointer)&params,
				    verbose ? (GtsStopFunc)stop_nedges_verbose: (GtsStopFunc)stop_nedges,
				    &stop_info_first,
				    fold);
		cvmsg() << "\n Stopped first stage... \r"; 
		gts_surface_coarsen(stop_info.surface,
				    (GtsKeyFunc)gts_volume_optimized_cost,
				    (gpointer)&params,
				    (GtsCoarsenFunc)gts_volume_optimized_vertex,
				    (gpointer)&params,
				    verbose ? (GtsStopFunc)stop_nedges_verbose: (GtsStopFunc)stop_nedges,
				    &stop_info,
				    fold);
	}
	else if (stop_info.max_faces >0 ) {
		
		
		stop_info_t stop_info_first ={NULL, -1, -1,0,0,NULL,NULL, NULL};

		stop_info_first.surface = stop_info.surface; 
		stop_info_first.max_faces  = int(coarsen_method_factor * stop_info.max_faces);
		stop_info_first.cost_name = "edge length"; 
				
		gts_surface_coarsen(stop_info.surface,NULL,(gpointer)&params,
				    NULL,(gpointer)&params,
				    verbose ? (GtsStopFunc)stop_nfaces_verbose: (GtsStopFunc)stop_nfaces,
				    &stop_info_first,fold);
		cvmsg() << "\n Stopped first stage... \r"; 
		gts_surface_coarsen(stop_info.surface,
				    (GtsKeyFunc)gts_volume_optimized_cost,
				    (gpointer)&params,
				    (GtsCoarsenFunc)gts_volume_optimized_vertex,
				    (gpointer)&params,
				    verbose ? (GtsStopFunc)stop_nfaces_verbose : (GtsStopFunc)stop_nfaces,
				    (gpointer)&stop_info,
				    fold);
	}else {
		cvmsg() << "cost" << '\n';  
		gts_surface_coarsen(stop_info.surface,
				    (GtsKeyFunc)gts_volume_optimized_cost,
				    (gpointer)&params,
				    (GtsCoarsenFunc)gts_volume_optimized_vertex,
				    (gpointer)&params,
				    verbose ? (GtsStopFunc)stop_cost_verbose : (GtsStopFunc)stop_cost,
				    &max_cost,
				    fold);
	}
	
	if (verbose)
		gts_surface_print_stats(stop_info.surface, stderr);
	
	return stop_info.surface;	
		
}

/** This function creates a GTS triangle mesh as the iso-surface of a given input image.
    The mesh is afterwards optimized by first collapsing small edges, and then by smallest volume change
    \param src the input image
    \param iso_value the iso value for surface extraction 
    \param max_edges mesh optimization stops if number of edges is less the this value; -1 = unlimited, overrides all other stop criteria
    \param max_faces mesh optimization stops if number of faces is less the this value; -1 = unlimited, overrides cost criterium
    \param max_cost mesh optimization stops if the cost of an edge collaps is above, no method switch is applied; -1 unlimited 
    \param bordered create a bounding box around the image to ensure a closed surface
    \param coarsen_method_factor switch from the first edge collapsing function to the latter at this factor of max edges/faces from targeted one
    \param verbose give some output
    \returns an GtsSurface
*/
EXPORT GtsSurface *iso_surface(const C3DImage& src, gfloat iso_value, gint max_edges, gint max_faces, 
			gdouble max_cost, gboolean bordered, 
			gfloat coarsen_method_factor)
{

	GtsSurface *mesh = extract_surface(src, iso_value, bordered);
	
	return iso_surface_optimize(mesh, max_edges, max_faces, max_cost, coarsen_method_factor); 
}

EXPORT GtsSurface  *iso_surface(const vector<string>& slices, gfloat iso_value, gint max_edges, gint max_faces, 
			gdouble max_cost, gboolean bordered, 
			gfloat coarsen_method_factor)
{

	GtsSurface *mesh = extract_surface(slices, iso_value, bordered);
	
	return iso_surface_optimize(mesh, max_edges, max_faces, max_cost, coarsen_method_factor); 
}





