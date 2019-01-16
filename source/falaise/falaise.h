//! \file    falaise/falaise.h
//! \brief   Provide system init for Falaise internals
//! \details
//
// Copyright (c) 2013 by Ben Morgan <bmorgan.warwick@gmail.com>
// Copyright (c) 2013 by The University of Warwick
// Copyright (c) 2013 by Francois Mauger <mauger@lpccaen.in2p3.fr>
// Copyright (c) 2013 by Université de Caen
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

#ifndef FALAISE_FALAISE_H
#define FALAISE_FALAISE_H
// Standard Library
#include <string>

// This project

namespace falaise {
/// Initialize resources of the Falaise library
void initialize(int argc_ = 0, char* argv_[] = 0, uint32_t flags_ = 0);

/// Terminate resources of the Falaise library
void terminate();
}  // namespace falaise

#endif  // FALAISE_FALAISE_H
