/*
 * Copyright (c) Leipzig, Madrid 2004-2010
 * Max-Planck-Institute for Human Cognitive and Brain Science	
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

/* $Id: vistaio.h 976 2007-04-02 10:40:26Z wollny $ */

/*! \brief Main header file for vistaio

	includes all vistaio headers

\file vistaio.h
\author M. Tittgemeyer, tittge@cbs.mpg.de, 2004

*/

#ifndef __VISTAIO_H
#define __VISTAIO_H 1

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <vistaio/vista.h> 
#include <vistaio/vcplist.h>
#include <vistaio/vfield3d.h>
#include <vistaio/vfield2d.h>
#include <vistaio/vconfig.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

extern const unsigned int vistaio_major_version;
extern const unsigned int vistaio_minor_version;
extern const unsigned int vistaio_micro_version;
extern const unsigned int vistaio_interface_age;
extern const unsigned int vistaio_binary_age;

#define VISTAIO_CHECK_VERSION(major,minor,micro)    \
    (vistaio_major_version > (major) || \
    (vistaio_major_version == (major) && vistaio_minor_version > (minor)) ||   \
    (vistaio_major_version == (major) && vistaio_minor_version == (minor) && \
     vistaio_micro_version >= (micro)))

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __VISTAIO_H */
