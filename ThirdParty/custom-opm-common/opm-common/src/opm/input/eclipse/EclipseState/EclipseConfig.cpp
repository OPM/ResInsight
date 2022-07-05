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

#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/input/eclipse/Deck/DeckSection.hpp>
#include <opm/input/eclipse/EclipseState/EclipseConfig.hpp>
#include <opm/input/eclipse/EclipseState/InitConfig/InitConfig.hpp>
#include <opm/input/eclipse/EclipseState/IOConfig/IOConfig.hpp>


namespace Opm {

    EclipseConfig::EclipseConfig(const Deck& deck) :
        m_initConfig(deck),
        io_config(deck)
    {
    }


    EclipseConfig::EclipseConfig(const InitConfig& initConfig, const IOConfig& io_conf):
        m_initConfig(initConfig),
        io_config(io_conf)
    {
    }


    EclipseConfig EclipseConfig::serializeObject()
    {
        EclipseConfig result;
        result.m_initConfig = InitConfig::serializeObject();
        result.io_config = IOConfig::serializeObject();

        return result;
    }

    InitConfig& EclipseConfig::init() {
        return const_cast<InitConfig &>(this->m_initConfig);
    }

    const InitConfig& EclipseConfig::init() const{
        return m_initConfig;
    }

    bool EclipseConfig::operator==(const EclipseConfig& data) const {
        return this->init() == data.init() &&
               this->io() == data.io();
    }

    IOConfig& EclipseConfig::io() {
        return const_cast<IOConfig &>(this->io_config);
    }

    const IOConfig& EclipseConfig::io() const {
        return this->io_config;
    }


    bool EclipseConfig::rst_cmp(const EclipseConfig& full_config, const EclipseConfig& rst_config) {
        return IOConfig::rst_cmp(full_config.io(), rst_config.io()) &&
               InitConfig::rst_cmp(full_config.init(), rst_config.init());
    }
}
