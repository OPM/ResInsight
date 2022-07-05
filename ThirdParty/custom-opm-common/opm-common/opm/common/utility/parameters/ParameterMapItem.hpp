//===========================================================================
//
// File: ParameterMapItem.hpp
//
// Created: Tue Jun  2 19:05:54 2009
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

#ifndef OPM_PARAMETERMAPITEM_HEADER
#define OPM_PARAMETERMAPITEM_HEADER

#include <string>

namespace Opm {
	/// The parameter handlig system is structured as a tree,
	/// where each node inhertis from ParameterMapItem.
	///
	/// The abstract virtual function getTag() is  used to determine
	/// which derived class the node actually is.
	struct ParameterMapItem {
	    /// Default constructor
	    ParameterMapItem() : used_(false) {}

	    /// Destructor
	    virtual ~ParameterMapItem() {}

	    /// \brief This function returns a string describing
	    ///        the ParameterMapItem.
	    virtual std::string getTag() const = 0;
	    void setUsed() const { used_ = true; }
	    bool used() const { return used_; }
	private:
	    mutable bool used_;
	};

	template<typename T>
	struct ParameterMapItemTrait {
	    static T convert(const ParameterMapItem&,
                             std::string& conversion_error);
	    static std::string type();
	};
} // namespace Opm

#endif // OPM_PARAMETERMAPITEM_HEADER
