// configuration_db.cc - Implementation of utilities for accessing falaise resource configuration files
//
// Copyright (c) 2017 by François Mauger <mauger@lpccaen.in2p3.fr>
// Copyright (c) 2017 by Université de Caen Normandie
//
// This file is part of Falaise.
//
// Falaise is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Falaise is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Falaise.  If not, see <http://www.gnu.org/licenses/>.

// Ourselves:
#include "falaise/configuration_db.h"

// Third Party:
// - Bayeux:
#include "bayeux/datatools/urn.h"
#include "bayeux/datatools/kernel.h"
#include <bayeux/datatools/urn_query_service.h>
#include <bayeux/datatools/urn_info.h>

// This Project:
#include "falaise/falaise.h"
#include "falaise/version.h"

namespace falaise {

  // static
  const std::string & configuration_db::category::experiment_label()
  {
    static const std::string _label("experiment");
    return _label;
  }

  // static
  const std::string & configuration_db::category::geometry_setup_label()
  {
    static const std::string _label("geometry");
    return _label;
  }

  // static
  const std::string & configuration_db::category::devices_setup_label()
  {
    static const std::string _label("devices");
    return _label;
  }

  // static
  const std::string & configuration_db::category::experimental_setup_label()
  {
    static const std::string _label("expsetup");
    return _label;
  }

  // static
  const std::string & configuration_db::category::processing_setup_label()
  {
    static const std::string _label("processing");
    return _label;
  }

  // static
  const std::string & configuration_db::category::processing_pipeline_label()
  {
    static const std::string _label("pipeline");
    return _label;
  }

  // static
  const std::string & configuration_db::category::processing_module_label()
  {
    static const std::string _label("procmodule");
    return _label;
  }

  // static
  const std::string & configuration_db::category::simulation_setup_label()
  {
    static const std::string _label("simsetup");
    return _label;
  }

  // static
  const std::string & configuration_db::category::reconstruction_setup_label()
  {
    static const std::string _label("recsetup");
    return _label;
  }

  // static
  const std::string & configuration_db::category::variants_service_label()
  {
    static const std::string _label("variants");
    return _label;
  }

  // static
  const std::string & configuration_db::category::variants_registry_label()
  {
    static const std::string _label("varregistry");
    return _label;
  }

  // static
  const std::string & configuration_db::category::variants_profile_label()
  {
    static const std::string _label("varprofile");
    return _label;
  }

  // static
  const std::string & configuration_db::category::services_manager_label()
  {
    static const std::string _label("services");
    return _label;
  }

  // static
  const std::string & configuration_db::category::configuration_label()
  {
    static const std::string _label("configuration");
    return _label;
  }

  // static
  const std::string & configuration_db::category::data_label()
  {
    static const std::string _label("data");
    return _label;
  }

  // static
  const std::string & configuration_db::category::default_urn_suffix()
  {
    static const std::string _label(":default");
    return _label;
  }

  struct configuration_db::pimpl_type
  {
    pimpl_type()
      : kuqs(datatools::kernel::instance().get_urn_query())
    {
      return;
    }
    const datatools::urn_query_service & kuqs;
  };

  configuration_db::configuration_db()
  {
    _pimpl_.reset(new pimpl_type);
    return;
  }

  configuration_db::~configuration_db()
  {
    _pimpl_.reset();
    return;
  }

  bool configuration_db::check(const std::string & urn_) const
  {
    return _pimpl_->kuqs.check_urn_info(urn_);
  }

  bool configuration_db::check_with_category(const std::string & urn_,
                                             const std::string & category_) const
  {
    return _pimpl_->kuqs.check_urn_info(urn_, category_);
  }

  bool configuration_db::find(std::vector<urn_entry> & entries_,
                              const std::string & urn_regex_,
                              const std::string & category_regex_,
                              const std::string & db_regex_) const
  {
    std::vector<std::string> urn_infos;
    if (_pimpl_->kuqs.find_urn_info(urn_infos,
                                    db_regex_,
                                    urn_regex_,
                                    category_regex_)) {
      for (std::size_t i = 0; i < urn_infos.size(); i++) {
        urn_entry ue;
        ue.urn = urn_infos[i];
        const datatools::urn_info & uinfo = _pimpl_->kuqs.get_urn_info(ue.urn);
        ue.description = uinfo.get_description();
        ue.category = uinfo.get_category();
        entries_.push_back(ue);
      }
    }
    return entries_.size() > 0;
  }

  bool configuration_db::find_unique(urn_entry & entry_,
                                     const std::string & urn_regex_,
                                     const std::string & category_regex_,
                                     const std::string & db_regex_) const
  {
    std::vector<urn_entry> entries;
    if (find(entries, urn_regex_, category_regex_, db_regex_)) {
      if (entries.size() != 1) return false;
      entry_ = entries.front();
      return true;
    }
    return false;
  }

  bool configuration_db::path_can_be_resolved_from(const std::string & urn_) const
  {
    return _pimpl_->kuqs.check_urn_to_path(urn_);
  }

  bool configuration_db::resolve(const std::string & urn_,
                                 std::string & category_,
                                 std::string & mime_,
                                 std::string & path_) const
  {
    DT_THROW_IF(!path_can_be_resolved_from(urn_),
                std::logic_error,
                "URN '" << urn_ << "' cannot be resolved as a path!");
    return _pimpl_->kuqs.resolve_urn_to_path(urn_, category_, mime_, path_);
  }

  bool configuration_db::find_direct_dependers_with_category_from(std::set<std::string> & dependers_,
                                                                  const std::string & from_urn_,
                                                                  const std::string & category_)
  {
    DT_THROW_IF(!check(from_urn_), std::logic_error, "No URN '" << from_urn_ << "'!");
    dependers_.clear();
    // Find URNS with the given category:
    std::vector<std::string> urns;
    DT_THROW_IF(!_pimpl_->kuqs.find_urn_info(urns, "", "", category_),
                std::logic_error, "No URNs with category '" << category_ << "'!");
    for (std::size_t iurn = 0; iurn < urns.size(); iurn++) {
      const std::string & urn = urns[iurn];
      if (direct_depends_on(urn, from_urn_)) {
        dependers_.insert(urn);
      }
    }
    return dependers_.size() > 0;
  }

  bool configuration_db::find_direct_unique_depender_with_category_from(std::string & depender_,
                                                                        const std::string & from_urn_,
                                                                        const std::string & category_)
  {
    depender_.clear();
    std::set<std::string> dependers;
    if (!find_direct_dependers_with_category_from(dependers, from_urn_, category_)) {
      return false;
    }
    if (dependers.size() == 1) {
      depender_ = *dependers.begin();
      return true;
    }
    return false;
  }

  bool configuration_db::direct_depends_on(const std::string & urn_, const std::string & dependee_urn_) const
  {
    DT_THROW_IF(!check(urn_), std::logic_error, "No URN '" << urn_ << "'!");
    DT_THROW_IF(!check(dependee_urn_), std::logic_error, "No URN '" << dependee_urn_ << "'!");
    const datatools::urn_info & uinfo = _pimpl_->kuqs.get_urn_info(urn_);
    std::vector<std::string> topics;
    uinfo.build_topics(topics);
    for (std::size_t itopic = 0; itopic < topics.size(); itopic++) {
      const std::vector<std::string> & comps = uinfo.get_components_by_topic(topics[itopic]);
      for (std::size_t icomp = 0; icomp < comps.size(); icomp++) {
        std::string comp_urn = comps[icomp];
        if (comp_urn == dependee_urn_) {
          return true;
        }
      }
    }
    return false;
  }

  bool configuration_db::is_alias_of(const std::string & urn_, const std::string & aliased_urn_) const
  {
    DT_THROW_IF(!check(urn_), std::logic_error, "No URN '" << urn_ << "'!");
    DT_THROW_IF(!check(aliased_urn_), std::logic_error, "No URN '" << aliased_urn_ << "'!");
    if (!direct_depends_on(urn_, aliased_urn_)) return false;
    const datatools::urn_info & uinfo = _pimpl_->kuqs.get_urn_info(urn_);
    std::vector<std::string> topics;
    uinfo.build_topics(topics);
    for (std::size_t itopic = 0; itopic < topics.size(); itopic++) {
      if (topics[itopic] == "alias") {
        const std::vector<std::string> & comps = uinfo.get_components_by_topic(topics[itopic]);
        if (comps.size() == 1 && comps[0] == aliased_urn_) {
          return true;
        }
      }
    }
    return false;
  }

  bool configuration_db::find_direct_dependees_with_category_from(std::set<std::string> & dependees_,
                                                                  const std::string & from_urn_,
                                                                  const std::string & category_)
  {
    DT_THROW_IF(!check(from_urn_), std::logic_error, "No URN '" << from_urn_ << "'!");
    const datatools::urn_info & uinfo = _pimpl_->kuqs.get_urn_info(from_urn_);
    // Extract the list of topics from the URN info record:
    std::vector<std::string> topics;
    uinfo.build_topics(topics);
    std::set<std::string> dependees;
    dependees_.clear();
    for (std::size_t itopic = 0; itopic < topics.size(); itopic++) {
      const std::vector<std::string> & comps = uinfo.get_components_by_topic(topics[itopic]);
      for (std::size_t icomp = 0; icomp < comps.size(); icomp++) {
        std::string comp_urn = comps[icomp];
        const datatools::urn_info & comp_uinfo = _pimpl_->kuqs.get_urn_info(comp_urn);
        if (comp_uinfo.has_category() && comp_uinfo.get_category() == category_) {
          dependees_.insert(comp_urn);
        }
      }
    }
    return dependees_.size() > 0;
  }

  bool configuration_db::find_direct_unique_dependee_with_category_from(std::string & dependee_,
                                                                        const std::string & from_urn_,
                                                                        const std::string & category_)
  {
    dependee_.clear();
    std::set<std::string> dependees;
    if (!find_direct_dependees_with_category_from(dependees, from_urn_, category_)) {
      return false;
    }
    if (dependees.size() == 1) {
      dependee_ = *dependees.begin();
      return true;
    }
    return false;
  }

} // namespace falaise
