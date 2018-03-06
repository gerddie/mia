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

#ifndef mia_core_plugin_base_hh
#define mia_core_plugin_base_hh

#include <map>
#include <set>
#include <string>
#include <ostream>

#include <mia/core/paramtranslator.hh>
#include <mia/core/module.hh>
#include <mia/core/property_flags.hh>

#include <mia/core/import_handler.hh>

NS_MIA_BEGIN


/// standard string to print out help in the factory plug-in handler
EXPORT_CORE extern const std::string plugin_help;


/**
   This class sets the plug-in search path for
   to the build tree based location. It should only instanciated
   for running unit-like tests.
   Calling the destructor will reset the plug-in search path estimation
   to its normal behaviour.
*/

struct EXPORT_CORE PrepareTestPluginPath {
       PrepareTestPluginPath();
       ~PrepareTestPluginPath();

};

/**
    \ingroup plugin
   \brief The base class for all plug-ins.

   The base class for all plug-ins. It supports handling parameters
   and a provides a help interface.
*/

class EXPORT_CORE CPluginBase: public CParamTranslator
{
public:
       /**
          The constructor  initialises the plug-in with its name and sets
          its link-up to nil ant initialises its parameter map.
          \param name
       */
       CPluginBase(const char *name);

       /** this destructur exists for the sole purpouse to ensure a virtual destructor
           for all plug-ins.
       */
       virtual ~CPluginBase();


       /**
          Set the plug-in priority, if two plug-ins of the same type and
          name are loaded, then the one with the higher priority is kept.
          \param p priority of this plugin
        */
       void set_priority(unsigned p);

       /**
          \returns the priority of the plug-in
        */
       unsigned get_priority() const;


       /**
          link up another plug-in in order to be able to put several
          plug-ins of the same type into the same dynamic library
       */
       void append_interface(CPluginBase *plugin);

       /// \returns a pointer to the next plug-in, or NULL, if it doesn't exist
       CPluginBase *next_interface();

       /**
          \param property a named property
          \returns true if the property is supported, false otherwise
       */
       bool has_property(const char *property) const;


       /** set the shared modules containing the code of this plugin
          This enshures that the modules is not unloaded while the plug-in
          still exists and needs to call its destructor whos code resides
          in the module.
          @param module
        */
       void set_module(const PPluginModule& module);

       /**
         \returns the module
        */
       PPluginModule get_module() const;

       /**
          Add all plugin handlers that may be called by this plugin
          \param [in,out] handler_map
        */
       void add_dependend_handlers(HandlerHelpMap& handler_map);

       /**
          Add a porperty to the map of supported properties
          \param property a named property
       */
       void add_property(const char *property);


private:


       /* pointer to the next interface in a plugin chain
          NULL indicates end of chain
       */
       CPluginBase *m_next_interface;

       /*
         Specific properties of this plug in
        */
       CPropertyFlagHolder m_properties;

       unsigned m_priority;

       /*
         The dynamically loadable module that holds the code of this plugin.
        */
       PPluginModule  m_module;

};


/**
    \ingroup plugin

   \brief The generic base for all plug-ins

   Templated plugin base class. The classes \a D and \a T must define a
   static const char *data_descr and static const char *type_descr respectively.
   The combination D::data_descr and T::type_descr is will be part of the plugin search path.
   \tparam D data type descriptior
   \tparam T plugin type descriptor

*/
template <typename D, typename T>
class EXPORT_HANDLER TPlugin: public  CPluginBase
{
public:
       /// Typedef for the data type descriptor handled by this plug-in
       typedef D PlugData;

       /// Typedef for the plugin type descriptor handled by this plug-in
       typedef T PlugType;

       /**
          The constructor  initialises the plug-in with its name and sets
          its link-up to nil ant initialises its parameter map.
          \param name
       */
       TPlugin(const char *name);

       /**
          Write a help regarding the plugin to the output stream
          \param os
        */
       virtual void get_help(std::ostream& os) const;


       static const char *get_data_path_part();

       static const char *get_type_path_part();


       /// @returns the long name of the plug in consisting of its name, its type, and data descriptors
       const std::string get_long_name() const;

};

/**
   Some plug-ins can be chained directly at creation time.
   To make the factory aware of this interface, this trait is used.
*/

template <typename Plugin>
struct plugin_can_chain {
       static constexpr bool value = false;
       typedef void Chained;
};

NS_MIA_END

#endif
