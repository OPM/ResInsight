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
#include <vector>

namespace Opm
{
     /*
     * The OilVaporizationProperties class
     * This classe is used to store the values from {VAPPARS, DRSDT, DRVDT, DRSDTCON}
     * The VAPPARS and {DRSDT, DRVDT} are mutal exclusive and will cancel previous settings of the other keywords.
     * The DRSDTCON implements a dissolution rate based on convective mixing.
     * Ask for type first and the ask for the correct values for this type, asking for values not valid for the current type will throw a logic exception.
     */
    class OilVaporizationProperties {
    public:
        enum class OilVaporization {
            UNDEF = 0,
            VAPPARS = 1,
            DRDT = 2, // DRSDT or DRVDT
            DRSDTCON = 3 // DRSDTCON
        };


        OilVaporizationProperties();
        explicit OilVaporizationProperties(const size_t numPvtReginIdx);

        static OilVaporizationProperties serializeObject();

        static void updateDRSDT(Opm::OilVaporizationProperties& ovp, const std::vector<double>& maxDRSDT, const std::vector<std::string>& option);
        static void updateDRSDTCON(Opm::OilVaporizationProperties& ovp, const std::vector<double>& maxDRSDT, const std::vector<std::string>& option);
        static void updateDRVDT(Opm::OilVaporizationProperties& ovp, const std::vector<double>& maxDRVDT);
        static void updateVAPPARS(Opm::OilVaporizationProperties& ovp, double vap1, double vap2);

        OilVaporization getType() const;
        double getMaxDRSDT(const size_t pvtRegionIdx) const;
        double getMaxDRVDT(const size_t pvtRegionIdx) const;
        bool getOption(const size_t pvtRegionIdx) const;
        bool drsdtActive() const;
        bool drvdtActive() const;
        bool drsdtConvective() const;
        bool defined() const;
        size_t numPvtRegions() const {return m_maxDRSDT.size();}

        double vap1() const;
        double vap2() const;

        /*
         * if either argument was default constructed == will always be false
         * and != will always be true
         */
        bool operator==( const OilVaporizationProperties& ) const;
        bool operator!=( const OilVaporizationProperties& ) const;

        template<class Serializer>
        void serializeOp(Serializer& serializer)
        {
            serializer(m_type);
            serializer(m_vap1);
            serializer(m_vap2);
            serializer(m_maxDRSDT);
            serializer(m_maxDRSDT_allCells);
            serializer(m_maxDRVDT);
        }

    private:
        OilVaporization m_type = OilVaporization::UNDEF;
        double m_vap1;
        double m_vap2;
        std::vector<double> m_maxDRSDT;
        std::vector<bool> m_maxDRSDT_allCells;
        std::vector<double> m_maxDRVDT;
    };
}
#endif // DRSDT_H
