/*
  Copyright 2019 SINTEF Digital, Mathematics and Cybernetics.

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

#ifndef OPM_FOAMCONFIG_HPP
#define OPM_FOAMCONFIG_HPP

#include <opm/input/eclipse/EclipseState/Runspec.hpp>

#include <cstddef>
#include <vector>

namespace Opm
{

class Deck;
class DeckRecord;

/// Foam behaviour data for a single SATNUM region.
class FoamData
{
public:
    FoamData();
    FoamData(const DeckRecord& FOAMFSC_record, const DeckRecord& FOAMROCK_record);
    explicit FoamData(const DeckRecord& FOAMROCK_record);

    static FoamData serializeObject();

    double referenceSurfactantConcentration() const;
    double exponent() const;
    double minimumSurfactantConcentration() const;

    bool allowDesorption() const;
    double rockDensity() const;

    bool operator==(const FoamData& data) const;

    template<class Serializer>
    void serializeOp(Serializer& serializer)
    {
        serializer(reference_surfactant_concentration_);
        serializer(exponent_);
        serializer(minimum_surfactant_concentration_);
        serializer(allow_desorption_);
        serializer(rock_density_);
    }

private:
    double reference_surfactant_concentration_;
    double exponent_;
    double minimum_surfactant_concentration_;
    bool allow_desorption_;
    double rock_density_;
};

/// Foam behaviour data for all SATNUM regions.
class FoamConfig
{
public:
    enum class MobilityModel {
      TAB,
      FUNC
    };

    FoamConfig() = default;
    explicit FoamConfig(const Deck&);

    static FoamConfig serializeObject();

    const FoamData& getRecord(std::size_t index) const;

    Opm::Phase getTransportPhase() const;
    MobilityModel getMobilityModel() const;

    std::size_t size() const;
    bool empty() const;

    using const_iterator = std::vector<FoamData>::const_iterator;
    const_iterator begin() const;
    const_iterator end() const;

    bool operator==(const FoamConfig& data) const;

    template<class Serializer>
    void serializeOp(Serializer& serializer)
    {
        serializer.vector(data_);
        serializer(transport_phase_);
        serializer(mobility_model_);
    }

private:
    std::vector<FoamData> data_;
    Phase transport_phase_ = Phase::GAS;
    MobilityModel mobility_model_ = MobilityModel::TAB;
};

} // end namespace Opm

#endif // OPM_FOAMCONFIG_HPP
