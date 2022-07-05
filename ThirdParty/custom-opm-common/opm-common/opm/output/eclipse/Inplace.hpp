/*
  Copyright 2020 Equinor ASA.

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

#ifndef ORIGINAL_OIP
#define ORIGINAL_OIP

#include <string>
#include <unordered_map>
#include <vector>

namespace Opm {


class Inplace {
public:

    enum class Phase {
        WATER = 0,
        OIL = 1,
        GAS = 2,
        OilInLiquidPhase = 3,
        OilInGasPhase = 4,
        GasInLiquidPhase = 5,
        GasInGasPhase = 6,
        PoreVolume = 7,
        // The Inplace class is implemented in close relation to the
        // ecloutputblackoilmodule in opm-simulators, ane there are
        // certainly idiosyncracies here due to that coupling. For instance
        // the enum values PressurePV, HydroCarbonPV, PressureHydroCarbonPV,
        // and DynamicPoreVolume are *not* included in the return value from
        // phases().
        PressurePV = 8,
        HydroCarbonPV = 9,
        PressureHydroCarbonPV = 10,
        DynamicPoreVolume = 11,
        WaterResVolume = 12,
        OilResVolume = 13,
        GasResVolume = 14,
        SALT = 15,
    };

    /*
      The purpose of this class is to transport inplace values from the
      simulator code to the summary output code. The code is written very much
      to fit in with the current implementation in the simulator. The functions
      which don't accept region_name & region_number arguments should be called
      for totals, i.e. field properties.
    */


    void add(const std::string& region, Phase phase, std::size_t region_number, double value);
    void add(Phase phase, double value);

    double get(const std::string& region, Phase phase, std::size_t region_number) const;
    double get(Phase phase) const;

    bool has(const std::string& region, Phase phase, std::size_t region_number) const;
    bool has(Phase phase) const;

    std::size_t max_region() const;
    std::size_t max_region(const std::string& region_name) const;

    /*
      The get_vector functions return a vector length max_region() which
      contains the values added with the add() function and indexed with
      (region_number - 1). This is an incarnation of id <-> index confusion and
      should be replaced with a std::map instead.
    */
    std::vector<double> get_vector(const std::string& region, Phase phase) const;

    static const std::vector<Phase>& phases();
private:
    std::unordered_map<std::string, std::unordered_map<Phase, std::unordered_map<std::size_t, double>>> phase_values;
};


}

#endif
