//===========================================================================
//
// File: ParameterStrings.hpp
//
// Created: Tue Jun  2 19:04:15 2009
//
// Author(s): Bård Skaflestad     <bard.skaflestad@sintef.no>
//            Atgeirr F Rasmussen <atgeirr@sintef.no>
//
// $Date$
//
// $Revision$
//
//===========================================================================

/*
  Copyright 2009, 2010 SINTEF ICT, Applied Mathematics.
  Copyright 2009, 2010 Statoil ASA.

  This file is part of the Open Porous Media project (OPM).

  OPM is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  OPM is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with OPM.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef OPM_PARAMETERSTRINGS_HEADER
#define OPM_PARAMETERSTRINGS_HEADER

#include <string>

namespace Opm {
	const std::string ID_true                   = "true";
	const std::string ID_false                  = "false";

	const std::string ID_xmltag__param_grp      = "ParameterGroup";
	const std::string ID_xmltag__param          = "Parameter";

	const std::string ID_param_type__bool       = "bool";
	const std::string ID_param_type__int        = "int";
	const std::string ID_param_type__float      = "double";
	const std::string ID_param_type__string     = "string";
	const std::string ID_param_type__file       = "file";
	const std::string ID_param_type__cmdline    = "cmdline";

	//

	const std::string ID_path_root            = "";
	const std::string ID_delimiter_path       = "/";
	const std::string ID_comment              = "//";
	const std::string ID_delimiter_assignment = "=";
} // namespace Opm

#endif // OPM_PARAMETERSTRINGS_HEADER
