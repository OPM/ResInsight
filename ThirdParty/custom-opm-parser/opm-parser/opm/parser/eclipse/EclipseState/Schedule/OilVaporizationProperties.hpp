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
#ifndef DRSDT_HPP
#define DRSDT_HPP

#include <string>
#include <memory>

#include <opm/parser/eclipse/EclipseState/Schedule/ScheduleEnums.hpp>

namespace Opm
{
     /*
     * The OilVaporizationProperties class
     * This classe is used to store the values from {VAPPARS, DRSDT, DRVDT} the behavior of the keywords are mutal exclusive.
     * Any one of the three keywords {VAPPARS, DRSDT, DRVDT} will cancel previous settings of the other keywords.
     * Ask for type first and the ask for the correct values for this type, asking for values not valid for the current type will throw a logic exception.
     */
    class OilVaporizationProperties {
    public:
        static OilVaporizationProperties createDRSDT(double maxDRSDT, std::string option);
        static OilVaporizationProperties createDRVDT(double maxDRVDT);
        static OilVaporizationProperties createVAPPARS(double vap1, double vap2);

        Opm::OilVaporizationEnum getType() const;
        double getVap1() const;
        double getVap2() const;
        double getMaxDRSDT() const;
        double getMaxDRVDT() const;
        bool getOption() const;

        /*
         * if either argument was default constructed == will always be false
         * and != will always be true
         */
        bool operator==( const OilVaporizationProperties& ) const;
        bool operator!=( const OilVaporizationProperties& ) const;

    private:
        Opm::OilVaporizationEnum m_type = OilVaporizationEnum::UNDEF;
        double m_vap1;
        double m_vap2;
        double m_maxDRSDT;
        double m_maxDRVDT;
        bool m_maxDRSDT_allCells;
    };
}
#endif // DRSDT_H
