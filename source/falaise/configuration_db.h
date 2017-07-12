//! \file    falaise/configuration_db.h
//! \brief   Utilities for accessing falaise resource configuration files
//! \details The falaise library makes use of a tag system which allows
//!          to identify and locate configuration items, typically files and/or
//!          assemblies of files. The configuration DB class implements some
//!          high-level query operations to search configuration items through their
//!          tags, locate then as files published by the Falaise library, find associated
//!          dependencies.
//
// Copyright (c) 2017 by François Mauger <mauger@lpccaen.in2p3.fr>
// Copyright (c) 2017 by Université de Caen Normandie
//
// This file is part of falaise.
//
// falaise is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// falaise is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with falaise.  If not, see <http://www.gnu.org/licenses/>.

#ifndef FALAISE_CONFIGURATION_DB_H
#define FALAISE_CONFIGURATION_DB_H

// Standard library:
#include <memory>
#include <set>
#include <map>
#include <vector>

// Third Party:
// - Bayeux:
#include "bayeux/datatools/logger.h"

namespace falaise {

  /// \brief Utility to access tagged configuration resources through the Bayeux system URN query service
  class configuration_db
  {
  public:

    /// \brief Labels associated to supported categories of tagged items
    struct category {
      /// Label of an experiment
      static const std::string & experiment_label();
      /// Label for a geometry setup
      static const std::string & geometry_setup_label();
      /// Label for a devices manager setup
      static const std::string & devices_setup_label();
      /// Label for an experimental setup
      static const std::string & experimental_setup_label();
      /// Label for a processing setup
      static const std::string & processing_setup_label();
      /// Label for a processing pipeline
      static const std::string & processing_pipeline_label();
      /// Label for a processing module
      static const std::string & processing_module_label();
      /// Label for a simulation setup
      static const std::string & simulation_setup_label();
      /// Label for a digitization setup
      static const std::string & digitization_setup_label();
      /// Label for a reconstruction setup
      static const std::string & reconstruction_setup_label();
      /// Label for a variants service setup
      static const std::string & variants_service_label();
      /// Label for a variants registry setup
      static const std::string & variants_registry_label();
      /// Label for a variants profile
      static const std::string & variants_profile_label();
      /// Label for a services setup
      static const std::string & services_manager_label();
      /// Label for a generic configuration
      static const std::string & configuration_label();
      /// Label for a generic dataset
      static const std::string & data_label();
      /// Label suffix for default tags (format: urn:xxx[:yyy[:zzz]]]:default)
      static const std::string & default_urn_suffix();
    };

    /// \brief Simple record representing a registered URN
    struct urn_entry {
      std::string urn;         //!< URN identifier/tag of the resource
      std::string description; //!< Description of the resource
      std::string category;    //!< Category of the resource
      std::map<std::string, std::string> meta; //!< Dictionary of metadata as key/value pairs
    };

    /// Constructor
    configuration_db();

    /// Destructor
    ~configuration_db();

    /// Check if an URN is registered
    bool check(const std::string & urn_) const;

    /// Check if an URN is registered with a given category
    bool check_with_category(const std::string & urn_, const std::string & category_) const;

    /// Find the list of URN entries which optionally match some DB pattern, some URN pattern, some category pattern
    bool find(std::vector<urn_entry> & entries_,
              const std::string & urn_regex_ = "",
              const std::string & category_regex_ = "",
              const std::string & db_regex_ = "") const;

    /// Find an unique URN entry which optionally matches some DB pattern, some URN pattern, some category pattern
    bool find_unique(urn_entry & entry_,
                     const std::string & urn_regex_ = "",
                     const std::string & category_regex_ = "",
                     const std::string & db_regex_ = "") const;

    /// Check if an URN can be resolved as a path
    bool path_can_be_resolved_from(const std::string & urn_) const;

    /// Resolve the category, MIME type and path associated to an URN
    bool resolve(const std::string & urn_,
                 std::string & category_,
                 std::string & mime_,
                 std::string & path_) const;

    /// Find direct dependee URNs of given category from a given URN
    /// \code
    ///                                [from_urn]
    ///                                    |
    ///           +------------------------+----------------------------+ "*"
    ///           |                        |                            |
    ///           v                        v                            v
    /// [dependee-0 as "category"] [dependee-1 as "category"] ... [dependee-N as "category"]
    /// \end code
    bool find_direct_dependees_with_category_from(std::set<std::string> & dependees_,
                                                  const std::string & from_urn_,
                                                  const std::string & category_);

    /// Find an unique direct dependee URN of given category from a given URN
    /// \code
    ///    [from-urn]
    ///        |
    ///        | "*"
    ///        v
    ///    [dependee as "category"]
    /// \end code
    bool find_direct_unique_dependee_with_category_from(std::string & dependee_,
                                                        const std::string & from_urn_,
                                                        const std::string & category_);

    /// Find direct depender URNs of given category from a given URN
    /// \code
    /// [depender-0 as "category"] [depender-1 as "category"] ... [depender-N as "category"]
    ///           |                        |                            |
    ///           +------------------------+----------------------------+ "*"
    ///                                    |
    ///                                    v
    ///                                [from-urn]
    /// \end code
    bool find_direct_dependers_with_category_from(std::set<std::string> & dependers_,
                                                  const std::string & from_urn_,
                                                  const std::string & category_);

    /// Find an unique depender URNs of given category from a given URN
    /// \code
    ///    [depender as "category"]
    ///        |
    ///        | "*"
    ///        v
    ///    [from-urn]
    /// \end code
    bool find_direct_unique_depender_with_category_from(std::string & depender_,
                                                        const std::string & from_urn_,
                                                        const std::string & category_);

    /// Check if a given URN directly depends on another URN
    /// \code
    ///      [urn]
    ///        |
    ///        | "*"
    ///        v
    ///    [dependee-urn]
    /// \end code
    /// \input urn_ The URN to be tested as an depender of the second input argument
    /// \input dependee_urn_ The URN on which the tested URN may depend
    /// \return true if the first URN depends on the second one
    bool direct_depends_on(const std::string & urn_, const std::string & dependee_urn_) const;

    /// Check if a given URN is an alias of another URN
    /// \code
    ///      [urn]
    ///        |
    ///        | "alias"
    ///        v
    ///  [aliased-urn]
    /// \end code
    /// \input urn_ The URN to be tested as an alias of the second input argument
    /// \input aliased_urn_ The URN of which the tested URN maybe an alias
    /// \return true if the first URN is an alias of the second one
    bool is_alias_of(const std::string & urn_, const std::string & aliased_urn_) const;

  private:

    struct pimpl_type;
    std::unique_ptr<pimpl_type> _pimpl_; ///< Private implementation

  };

} // namespace falaise

#endif // FALAISE_CONFIGURATION_DB_H

// Local Variables: --
// mode: c++ --
// c-file-style: "gnu" --
// tab-width: 2 --
// End: --
