//! \file    falaise/configuration_db.h
//! \brief   Utilities for accessing falaise resource configuration files
//! \details The falaise library makes use of several resource files
//!          containing isotopic and radioactive decays data.
//!          Falaise also implements a plugin architecture, with
//!          several supplied as core functionality.
//!
//!          As these files are not compiled into the falaise library,
//!          a simple API is provided to return the root paths to core
//!          resource and plugin directories.
//!
//!          These paths are calculated based on the location of the
//!          Falaise library, allowing relocation of the Falaise
//!          package after installation.
//!
//!          If your application uses resources from Falaise,
//!          you must call the falaise::init_resources function
//!          before trying to access resources.
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

// Third Party:
// - Bayeux:
#include "bayeux/datatools/logger.h"

namespace falaise {

  /// \brief Utility to access tagged configuration resources through the system URN query service
  class configuration_db
  {
  public:

    /// Constructor
    configuration_db();

    /// Destructor
    ~configuration_db();

    /// Check if an URN is registered
    bool check(const std::string & urn_) const;

    /// Check if an URN is registered with a given category
    bool check_with_category(const std::string & urn_, const std::string & category_) const;

    /// Check if an URN can be resolved as a path
    bool path_can_be_resolved_from(const std::string & urn_) const;

    /// Resolve the category, MIME type and path associated to an URN
    bool resolve(const std::string & urn_,
                 std::string & category_,
                 std::string & mime_,
                 std::string & path_) const;

    /// Find direct dependee URNs of given category from a given URN
    /// \code
    ///                                    |
    ///                                    v
    ///                                [from_urn]
    ///                                    |
    ///           +------------------------+----------------------------+
    ///           |                        |                            |
    /// [dependee-0 as "category"] [dependee-1 as "category"] ... [dependee-N as "category"]
    /// \end code
    bool find_direct_dependees_with_category_from(const std::string & from_urn_,
                                                  const std::string & category_,
                                                  std::set<std::string> & dependees_);

    /// Find an unique direct dependee URN of given category from a given URN
    /// \code
    ///    [from-urn]
    ///        |
    ///        | "*"
    ///        v
    ///    [dependee as "category"]
    /// \end code
    bool find_direct_unique_dependee_with_category_from(const std::string & from_urn_,
                                                        const std::string & category_,
                                                        std::string & dependers_);

    /// Find direct depender URNs of given category from a given URN
    /// \code
    /// [depender-0 as "category"] [depender-1 as "category"] ... [depender-N as "category"]
    ///           |                        |                            |
    ///           +------------------------+----------------------------+
    ///                                    |
    ///                                    v
    ///                                [from-urn]
    /// \end code
    bool find_direct_dependers_with_category_from(const std::string & from_urn_,
                                                  const std::string & category_,
                                                  std::set<std::string> & dependers_);

    /// Find an unique depender URNs of given category from a given URN
    /// \code
    ///    [depender as "category"]
    ///        |
    ///        | "*"
    ///        v
    ///    [from-urn]
    /// \end code
    bool find_direct_unique_depender_with_category_from(const std::string & from_urn_,
                                                        const std::string & category_,
                                                        std::string & depender_);

    /// Check if a given URN directly depends on another URN
    /// \code
    ///      [urn]
    ///        |
    ///        | "alias"
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
