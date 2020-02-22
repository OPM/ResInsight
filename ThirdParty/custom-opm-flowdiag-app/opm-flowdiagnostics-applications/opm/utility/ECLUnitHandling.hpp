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

#ifndef OPM_ECLUNITHANDLING_HEADER_INCLUDED
#define OPM_ECLUNITHANDLING_HEADER_INCLUDED

#include <memory>

namespace Opm {

    namespace ECLUnits {

        struct UnitSystem
        {
            virtual std::unique_ptr<UnitSystem> clone() const = 0;

            virtual double density()             const = 0;
            virtual double depth()               const = 0;
            virtual double pressure()            const = 0;
            virtual double reservoirRate()       const = 0;
            virtual double reservoirVolume()     const = 0;
            virtual double surfaceVolumeLiquid() const = 0;
            virtual double surfaceVolumeGas()    const = 0;
            virtual double time()                const = 0;
            virtual double transmissibility()    const = 0;
            virtual double viscosity()           const = 0;

            double dissolvedGasOilRat() const; // Rs
            double vaporisedOilGasRat() const; // Rv
        };

        std::unique_ptr<const UnitSystem>
        createUnitSystem(const int usys);

    } // namespace ECLUnits
} // namespace Opm

#endif // OPM_ECLUNITHANDLING_HEADER_INCLUDED
