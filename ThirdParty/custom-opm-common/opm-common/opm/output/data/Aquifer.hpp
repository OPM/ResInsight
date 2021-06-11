/*
  Copyright 2019 Statoil ASA.

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

#ifndef OPM_OUTPUT_AQUIFER_HPP
#define OPM_OUTPUT_AQUIFER_HPP

#include <map>
#include <memory>
#include <vector>

#include <opm/parser/eclipse/Units/UnitSystem.hpp>

namespace Opm { namespace data {

    /**
     * Small struct that keeps track of data for output to restart/summary
     * files.
     */
    enum class AquiferType
    {
        Fetkovich, CarterTracey,
    };

    struct FetkovichData {
        double initVolume;
        double prodIndex;
        double timeConstant;
    };

    struct AquiferData {
        int aquiferID;    //< One-based ID, range 1..NANAQ
        double pressure;  //< Aquifer pressure
        double volume;    //< Produced liquid volume
        double initPressure;    //< Aquifer's initial pressure
        double datumDepth;      //< Aquifer's pressure reference depth

        AquiferType type;
        std::shared_ptr<FetkovichData> aquFet;
    };

}} // Opm::data

#endif // OPM_OUTPUT_AQUIFER_HPP
