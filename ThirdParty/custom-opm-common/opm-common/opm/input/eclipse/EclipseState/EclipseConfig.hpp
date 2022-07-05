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

#ifndef OPM_ECLIPSE_CONFIG_HPP
#define OPM_ECLIPSE_CONFIG_HPP

#include <opm/input/eclipse/EclipseState/InitConfig/InitConfig.hpp>
#include <opm/input/eclipse/EclipseState/IOConfig/IOConfig.hpp>

namespace Opm {

    class Deck;

    class EclipseConfig
    {
    public:
        EclipseConfig() = default;
        explicit EclipseConfig(const Deck& deck);
        EclipseConfig(const InitConfig& initConfig, const IOConfig& io_conf);

        static EclipseConfig serializeObject();

        InitConfig& init();
        IOConfig& io();
        const IOConfig& io() const;
        const InitConfig& init() const;

        bool operator==(const EclipseConfig& data) const;
        static bool rst_cmp(const EclipseConfig& full_config, const EclipseConfig& rst_config);
        template<class Serializer>
        void serializeOp(Serializer& serializer)
        {
            m_initConfig.serializeOp(serializer);
            io_config.serializeOp(serializer);
        }

    private:
        InitConfig m_initConfig;
        IOConfig io_config;
    };
}

#endif // OPM_ECLIPSE_CONFIG_HPP
