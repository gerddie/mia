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

#include <mia/core/export_handler.hh>
#include <mia/core/ioplugin.hh>

NS_MIA_BEGIN

const char * io_plugin_type::type_descr = "io";
const char * const io_plugin_property_multi_record = "multi-record";
const char * const io_plugin_property_history_split = "history-split";
const char * const io_plugin_property_has_attributes = "has_attributes";
const char * const io_plugin_property_can_pipe = "can-pipe";

NS_MIA_END
