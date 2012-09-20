/*
** Copyright (C) 1999 Max-Planck-Institute of Cognitive Neurosience
**                    Gert Wollny <wollnyAtcbs.mpg.de>
**  
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU Lesser Public License as published by
** the Free Software Foundation; either version 2.1 of the License, or
** (at your option) any later version.
** 
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU Lesser Public License for more details.
** 
** You should have received a copy of the GNU Lesser Public License
** along with this program; if not, write to the Free Software 
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/
/* $Id: iso_backend.cc 954 2006-07-13 12:47:09Z write1 $ */

#include <gts.h>
#include <mia/3d/3DImage.hh>
#include <mia/core/filter.hh>

using namespace mia; 

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

static void iso_func_b(gdouble ** f, GtsCartesianGrid g, guint z, gpointer data)
{
	const C3DDImage *image = (C3DDImage *)data; 

	cvmsg() << "extracting slice ..."<< z +1 <<" of " << g.nz << "\r";
	
	if (z == 0 || z == image->get_size().z + 1) {
		for (size_t y = 0; y < g.ny; ++y)
			for (size_t x = 0; x < g.nx; ++x)
				f[x][y] = 0;
		return; 
	}
		
	const double *p = &(*image)(0,0,z);
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
} stop_info_t;


static gboolean stop_nedges (gdouble cost, guint number, stop_info_t *stop_info)
{
	g_return_val_if_fail (stop_info != NULL, TRUE);
	return (number < (guint)stop_info->max_edges);
}


static gboolean stop_nfaces(gdouble cost, guint nedges, stop_info_t *stop_info) 
{
	g_return_val_if_fail (stop_info != NULL, TRUE);

	return ( gts_surface_face_number(stop_info->surface) < (guint)stop_info->max_faces);
}

static gboolean stop_cost(gdouble cost, guint nedges, gdouble *max_cost) 
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
			 "\rCost: %10.5f "
			 "Edges: %10u %3.0f%% %6.0f edges/s "
			 "Elapsed: %02.0f:%02.0f:%02.0f "
			 "Remaining: %02.0f:%02.0f:%02.0f",
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


static int stop_nfaces_verbose(gdouble cost, guint nedges, stop_info_t *stop_info) 
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
			 "\rCost: %10.5f "
			 "Faces: %10u %3.0f%% %6.0f faces/s "
			 "Elapsed: %02.0f:%02.0f:%02.0f "
			 "Remaining: %02.0f:%02.0f:%02.0f",
			 cost,
			 number, 
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
		cvmsg()  << "\rCost: "<< cost << " Edges: "<< nedges;
		hide = 200; 
	}
	
	if (cost > *max_cost){
		return TRUE;
	}
	return FALSE; 	
}

void do_revert(GtsTriangle *t, gpointer *data) 
{
	gts_triangle_revert(t);
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




GtsSurface *iso_surface(const C3DImage& src, gfloat iso_value, gint max_edges, gint max_faces, 
			gdouble max_cost, gboolean bordered, 
			gfloat coarsen_method_factor)
{
	bool verbose = cverb.get_level() <= vstream::ml_message;

	
	cvmsg()  << "verbose = " << verbose << " " << cverb.get_level() << '\n'; 
     	stop_info_t stop_info ={ NULL, -1, -1,0,0, NULL, NULL};
	gdouble fold = M_PI/180.;
	GtsVolumeOptimizedParams params = { 0.5, 0.5, 0. };
	
	stop_info.max_faces = max_faces; 
	stop_info.max_edges = max_edges; 

	
	stop_info.surface = extract_surface(src, iso_value, bordered);

	
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
		stop_info_t stop_info_first ={NULL, -1, -1, 0, 0, NULL, NULL};
		
		stop_info_first.surface = stop_info.surface; 
		stop_info_first.max_edges = int(coarsen_method_factor * stop_info.max_edges);

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
		
		
		stop_info_t stop_info_first ={NULL, -1, -1,0,0,NULL,NULL};

		stop_info_first.surface = stop_info.surface; 
		stop_info_first.max_faces  = int(coarsen_method_factor * stop_info.max_faces);
		
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


/*
  $Log$
  Revision 1.5  2005/07/04 08:28:20  tittge
  adjusted to libmona-0.7

  Revision 1.2  2005/06/02 13:38:23  gerddie
  adapt code to new plugin handling

  Revision 1.1.1.1  2005/03/17 13:46:56  gerddie
  initial checkin

  Revision 1.4  2005/01/25 14:13:40  wollny
  3dvector attribute support

  Revision 1.3  2005/01/19 15:22:07  wollny
  cvs logging + license

*/





