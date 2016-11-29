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
#include <opm/parser/eclipse/EclipseState/Schedule/OilVaporizationProperties.hpp>

namespace Opm {

    double OilVaporizationProperties::getMaxDRVDT() const{
        if (m_type == Opm::OilVaporizationEnum::DRVDT){
            return m_maxDRVDT;
        }else{
            throw std::logic_error("Only valid if type is DRVDT");
        }
    }

    double OilVaporizationProperties::getMaxDRSDT() const{
        if (m_type == Opm::OilVaporizationEnum::DRSDT){
            return m_maxDRSDT;
        }else{
            throw std::logic_error("Only valid if type is DRSDT");
        }
    }

    bool OilVaporizationProperties::getOption() const{
        if (m_type == Opm::OilVaporizationEnum::DRSDT){
            return m_maxDRSDT_allCells;
        }else{
            throw std::logic_error("Only valid if type is DRSDT");
        }
    }

    Opm::OilVaporizationEnum OilVaporizationProperties::getType() const{
        return m_type;
    }

    double OilVaporizationProperties::getVap1() const{
        if (m_type == Opm::OilVaporizationEnum::VAPPARS){
            return m_vap1;
        }else{
            throw std::logic_error("Only valid if type is VAPPARS");
        }
    }

    double OilVaporizationProperties::getVap2() const{
        if (m_type == Opm::OilVaporizationEnum::VAPPARS){
            return m_vap2;
        }else{
            throw std::logic_error("Only valid if type is VAPPARS");
        }
    }

    OilVaporizationProperties OilVaporizationProperties::createDRSDT(double maximum, std::string option){
        OilVaporizationProperties ovp;
        ovp.m_type = Opm::OilVaporizationEnum::DRSDT;
        ovp.m_maxDRSDT = maximum;
        if (option == "ALL"){
            ovp.m_maxDRSDT_allCells = true;
        }else if (option == "FREE") {
            ovp.m_maxDRSDT_allCells = false;
        }else{
            throw std::invalid_argument("Only ALL or FREE is allowed as option string");
        }
        return ovp;
    }

    OilVaporizationProperties OilVaporizationProperties::createDRVDT(double maximum){
        OilVaporizationProperties ovp;
        ovp.m_type = Opm::OilVaporizationEnum::DRVDT;
        ovp.m_maxDRVDT = maximum;
        return ovp;
    }

    OilVaporizationProperties OilVaporizationProperties::createVAPPARS(double vap1, double vap2){
        OilVaporizationProperties ovp;
        ovp.m_type = Opm::OilVaporizationEnum::VAPPARS;
        ovp.m_vap1 = vap1;
        ovp.m_vap2 = vap2;
        return ovp;
    }

    bool OilVaporizationProperties::operator==( const OilVaporizationProperties& rhs ) const {
        if( this->m_type == OilVaporizationEnum::UNDEF
         || rhs.m_type   == OilVaporizationEnum::UNDEF
         || this->m_type != rhs.m_type ) return false;

        switch( this->m_type ) {
            case OilVaporizationEnum::DRSDT:
                return this->m_maxDRSDT == rhs.m_maxDRSDT
                    && this->m_maxDRSDT_allCells == rhs.m_maxDRSDT_allCells;

            case OilVaporizationEnum::DRVDT:
                return this->m_maxDRVDT == rhs.m_maxDRVDT;

            case OilVaporizationEnum::VAPPARS:
                return this->m_vap1 == rhs.m_vap1
                    && this->m_vap2 == rhs.m_vap2;

            default:
                throw std::logic_error( "UNDEF Oil vaporization property; this should never happen" );
        }
    }

    bool OilVaporizationProperties::operator!=( const OilVaporizationProperties& rhs ) const {
        return this->m_type == OilVaporizationEnum::UNDEF
            || rhs.m_type == OilVaporizationEnum::UNDEF
            || this->m_type != rhs.m_type;
    }
}
