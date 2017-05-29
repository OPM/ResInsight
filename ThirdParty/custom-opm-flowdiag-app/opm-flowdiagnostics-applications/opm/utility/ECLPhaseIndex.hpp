/*
  Copyright 2017 Statoil ASA.

  This file is part of the Open Porous Media Project (OPM).

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

#ifndef OPM_ECLPHASEINDEX_HEADER_INCLUDED
#define OPM_ECLPHASEINDEX_HEADER_INCLUDED

namespace Opm {

    /// Enum for indicating the phase--or set of phases--on which to apply a
    /// phase-dependent operation (e.g., extracting flux data from a result
    /// set or computing relative permeabilities from tabulated functions).
    enum class ECLPhaseIndex { Aqua = 0, Liquid = 1, Vapour = 2 };

} // namespace Opm

#endif // OPM_ECLPHASEINDEX_HEADER_INCLUDED
