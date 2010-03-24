/*
 * Copyright (c) 2004 
 * Max-Planck-Institute for Human Cognitive and Brain Science	
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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

/* $Id: version.c 65 2004-03-05 14:42:23Z tittge $ */

/*! \brief Just an export of some global version variables

\file version.c
\author M. Tittgemeyer, tittge@cbs.mpg.de, 2004

*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "vconfig.h"

const unsigned int vistaio_major_version = VISTAIO_MAJOR_VERSION;
const unsigned int vistaio_minor_version = VISTAIO_MINOR_VERSION;
const unsigned int vistaio_micro_version = VISTAIO_MICRO_VERSION;

