/*
  Copyright (c) 2020 Equinor ASA

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

#ifndef OPM_WRITE_RPT_HPP
#define OPM_WRITE_RPT_HPP

#include <ostream>

namespace Opm {

    class Schedule;
    class EclipseGrid;
    class UnitSystem;

    namespace RptIO {

        void write_report(
            std::ostream&,
            const std::string& report,
            unsigned value,
            const Schedule& schedule,
            const EclipseGrid& grid,
            const UnitSystem& unit_system,
            std::size_t time_step
        );

        namespace workers {

            void write_WELSPECS(std::ostream&, unsigned, const Schedule&, const EclipseGrid& grid, const UnitSystem&, std::size_t);

}   }   }
#endif // OPM_WRITE_RPT_HPP
