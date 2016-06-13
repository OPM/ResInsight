/*
  Copyright 2015 Statoil ASA.

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

#ifndef OPM_SIMULATION_CONFIG_HPP
#define OPM_SIMULATION_CONFIG_HPP

#include <opm/parser/eclipse/Deck/Deck.hpp>
#include <opm/parser/eclipse/EclipseState/Eclipse3DProperties.hpp>

namespace Opm {

    class ThresholdPressure;

    class SimulationConfig {

    public:

        SimulationConfig(const Deck& deck,
                         const Eclipse3DProperties& gridProperties);

        std::shared_ptr<const ThresholdPressure> getThresholdPressure() const;
        bool hasThresholdPressure() const;
        bool useCPR() const;
        bool hasDISGAS() const;
        bool hasVAPOIL() const;

    private:

        void initThresholdPressure(const Deck& deck,
                                   const Eclipse3DProperties& gridProperties);

        std::shared_ptr< const ThresholdPressure > m_ThresholdPressure;
        bool m_useCPR;
        bool m_DISGAS;
        bool m_VAPOIL;
    };



    typedef std::shared_ptr<SimulationConfig> SimulationConfigPtr;
    typedef std::shared_ptr<const SimulationConfig> SimulationConfigConstPtr;

} //namespace Opm



#endif
