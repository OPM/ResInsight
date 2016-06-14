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

#ifndef OPM_INIT_CONFIG_HPP
#define OPM_INIT_CONFIG_HPP

#include <opm/parser/eclipse/EclipseState/InitConfig/Equil.hpp>

namespace Opm {

    class Deck;

    class InitConfig {

    public:
        InitConfig(std::shared_ptr< const Deck > deck);
        InitConfig(const Deck& deck);

        void setRestart( const std::string& root, int step);
        bool restartRequested() const;
        int getRestartStep() const;
        const std::string& getRestartRootName() const;

        bool hasEquil() const;
        const Equil& getEquil() const;

    private:
        void initRestartKW(const Deck& deck);

        bool m_restartRequested = false;
        int m_restartStep = 0;
        std::string m_restartRootName;

        Equil equil;
    };

    typedef std::shared_ptr<InitConfig> InitConfigPtr;
    typedef std::shared_ptr<const InitConfig> InitConfigConstPtr;

} //namespace Opm

#endif
