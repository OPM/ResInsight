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
#include <opm/parser/eclipse/Parser/ParseContext.hpp>
#include <opm/parser/eclipse/Deck/Section.hpp>
#include <opm/parser/eclipse/EclipseState/Eclipse3DProperties.hpp>
#include <opm/parser/eclipse/EclipseState/EclipseConfig.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/GridDims.hpp>
#include <opm/parser/eclipse/EclipseState/InitConfig/InitConfig.hpp>
#include <opm/parser/eclipse/EclipseState/IOConfig/IOConfig.hpp>
#include <opm/parser/eclipse/EclipseState/IOConfig/RestartConfig.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Schedule.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/TimeMap.hpp>
#include <opm/parser/eclipse/EclipseState/SimulationConfig/SimulationConfig.hpp>

namespace Opm {

    EclipseConfig::EclipseConfig(const Deck& deck,
                                 const Eclipse3DProperties& eclipse3DProperties,
                                 const GridDims& inputGrid,
                                 const Schedule& schedule,
                                 const ParseContext& parseContext) :
            m_ioConfig(        std::make_shared<IOConfig>(deck)),
            m_initConfig(      deck),
            m_simulationConfig(deck, eclipse3DProperties),
            m_summaryConfig(   deck, schedule, eclipse3DProperties, parseContext , inputGrid.getNXYZ()),
            m_restartConfig(   deck )
    {
        m_ioConfig->initFirstRFTOutput(schedule);
    }


    const InitConfig& EclipseConfig::init() const {
        return m_initConfig;
    }

    const IOConfig& EclipseConfig::io() const {
        return *m_ioConfig;
    }

    IOConfig& EclipseConfig::io() {
        return *m_ioConfig;
    }

    const SimulationConfig& EclipseConfig::simulation() const {
        return m_simulationConfig;
    }

    const SummaryConfig& EclipseConfig::summary() const {
        return m_summaryConfig;
    }

    const RestartConfig& EclipseConfig::restart() const {
        return this->m_restartConfig;
    }

    // [[deprecated]] --- use summary()
    const SummaryConfig& EclipseConfig::getSummaryConfig() const {
        return summary();
    }

    // [[deprecated]] --- use restart()
    const RestartConfig& EclipseConfig::getRestartConfig() const {
        return this->restart();
    }

    IOConfigConstPtr EclipseConfig::getIOConfigConst() const {
        return m_ioConfig;
    }

    IOConfigPtr EclipseConfig::getIOConfig() const {
        return m_ioConfig;
    }

    // [[deprecated]] --- use init()
    const InitConfig& EclipseConfig::getInitConfig() const {
        return init();
    }

    // [[deprecated]] --- use simulation()
    const SimulationConfig& EclipseConfig::getSimulationConfig() const {
        return simulation();
    }
}
