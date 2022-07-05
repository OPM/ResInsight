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

#include <opm/input/eclipse/EclipseState/SimulationConfig/RockConfig.hpp>
#include <opm/input/eclipse/EclipseState/SimulationConfig/ThresholdPressure.hpp>
#include <opm/input/eclipse/EclipseState/SimulationConfig/BCConfig.hpp>

namespace Opm {

    class Deck;
    class FieldPropsManager;

    class SimulationConfig {

    public:

        SimulationConfig();
        SimulationConfig(bool restart,
                         const Deck& deck,
                         const FieldPropsManager& fp);

        static SimulationConfig serializeObject();

        const RockConfig& rock_config() const;
        const ThresholdPressure& getThresholdPressure() const;
        const BCConfig& bcconfig() const;
        bool useThresholdPressure() const;
        bool useCPR() const;
        bool hasDISGAS() const;
        bool hasVAPOIL() const;
        bool hasVAPWAT() const;
        bool isThermal() const;
        bool isDiffusive() const;
        bool hasPRECSALT() const;

        bool operator==(const SimulationConfig& data) const;
        static bool rst_cmp(const SimulationConfig& full_config, const SimulationConfig& rst_config);

        template<class Serializer>
        void serializeOp(Serializer& serializer)
        {
            m_ThresholdPressure.serializeOp(serializer);
            m_bcconfig.serializeOp(serializer);
            m_rock_config.serializeOp(serializer);
            serializer(m_useCPR);
            serializer(m_DISGAS);
            serializer(m_VAPOIL);
            serializer(m_VAPWAT);
            serializer(m_isThermal);
            serializer(m_diffuse);
            serializer(m_PRECSALT);
        }

    private:
        ThresholdPressure m_ThresholdPressure;
        BCConfig m_bcconfig;
        RockConfig m_rock_config;
        bool m_useCPR;
        bool m_DISGAS;
        bool m_VAPOIL;
        bool m_VAPWAT;
        bool m_isThermal;
        bool m_diffuse;
        bool m_PRECSALT;
    };

} //namespace Opm



#endif
