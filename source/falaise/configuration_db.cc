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

  bool configuration_db::find_direct_dependers_with_category_from(const std::string & from_urn_,
                                                                  const std::string & category_,
                                                                  std::set<std::string> & dependers_)
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

  bool configuration_db::find_direct_unique_depender_with_category_from(const std::string & from_urn_,
                                                                        const std::string & category_,
                                                                        std::string & depender_)
  {
    depender_.clear();
    std::set<std::string> dependers;
    if (!find_direct_dependers_with_category_from(from_urn_, category_, dependers)) {
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

  bool configuration_db::find_direct_dependees_with_category_from(const std::string & from_urn_,
                                                                  const std::string & category_,
                                                                  std::set<std::string> & dependees_)
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

  bool configuration_db::find_direct_unique_dependee_with_category_from(const std::string & from_urn_,
                                                                        const std::string & category_,
                                                                        std::string & dependee_)
  {
    dependee_.clear();
    std::set<std::string> dependees;
    if (!find_direct_dependees_with_category_from(from_urn_, category_, dependees)) {
      return false;
    }
    if (dependees.size() == 1) {
      dependee_ = *dependees.begin();
      return true;
    }
    return false;
  }

} // namespace falaise
