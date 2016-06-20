/*
 Copyright 2016 Statoil ASA.

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

#include <memory>

#include <opm/parser/eclipse/Deck/Deck.hpp>
#include <opm/parser/eclipse/Deck/Section.hpp>
#include <opm/parser/eclipse/EclipseState/Eclipse3DProperties.hpp>
#include <opm/parser/eclipse/EclipseState/EclipseConfig.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/EclipseGrid.hpp>
#include <opm/parser/eclipse/EclipseState/InitConfig/InitConfig.hpp>
#include <opm/parser/eclipse/EclipseState/IOConfig/IOConfig.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Schedule.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/TimeMap.hpp>
#include <opm/parser/eclipse/EclipseState/SimulationConfig/SimulationConfig.hpp>

namespace Opm {

    EclipseConfig::EclipseConfig(const Deck& deck,
                                 const Eclipse3DProperties& eclipse3DProperties,
                                 std::shared_ptr< EclipseGrid > inputGrid,
                                 const Schedule& schedule) :
            m_ioConfig(        std::make_shared<IOConfig>(deck)),
            m_initConfig(      std::make_shared<const InitConfig>(deck)),
            m_simulationConfig(std::make_shared<const SimulationConfig>(deck, eclipse3DProperties)),
            m_summaryConfig(   deck, schedule, eclipse3DProperties, inputGrid->getNXYZ())
    {
        // Hmmm - would have thought this should iterate through the SCHEDULE section as well?
        if (Section::hasSOLUTION(deck)) {
            std::shared_ptr<const SOLUTIONSection> solutionSection = std::make_shared<const SOLUTIONSection>(deck);
            m_ioConfig->handleSolutionSection(schedule.getTimeMap(), solutionSection);
        }
        m_ioConfig->initFirstOutput(schedule);
    }

    const SummaryConfig& EclipseConfig::getSummaryConfig() const {
        return m_summaryConfig;
    }

    IOConfigConstPtr EclipseConfig::getIOConfigConst() const {
        return m_ioConfig;
    }

    IOConfigPtr EclipseConfig::getIOConfig() const {
        return m_ioConfig;
    }

    InitConfigConstPtr EclipseConfig::getInitConfig() const {
        return m_initConfig;
    }

    SimulationConfigConstPtr EclipseConfig::getSimulationConfig() const {
        return m_simulationConfig;
    }
}
