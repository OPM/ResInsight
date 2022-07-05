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
#include <opm/input/eclipse/Schedule/OilVaporizationProperties.hpp>

#include <stdexcept>

namespace Opm {

    OilVaporizationProperties::OilVaporizationProperties()
    {
        m_type = OilVaporization::UNDEF;
        m_vap1 = m_vap2 = -1.0;
    }

    OilVaporizationProperties::OilVaporizationProperties(const size_t numPvtRegionIdx):
         m_vap1(-1.0),
         m_vap2(-1.0),
         m_maxDRSDT(numPvtRegionIdx, -1.0),
         m_maxDRSDT_allCells(numPvtRegionIdx),
         m_maxDRVDT(numPvtRegionIdx, -1.0)
    {  }

    OilVaporizationProperties OilVaporizationProperties::serializeObject()
    {
        OilVaporizationProperties result;
        result.m_type = OilVaporization::VAPPARS;
        result.m_vap1 = 1.0;
        result.m_vap2 = 2.0;
        result.m_maxDRSDT = {3.0};
        result.m_maxDRSDT_allCells = {true};
        result.m_maxDRVDT = {5.0};

        return result;
    }

    double OilVaporizationProperties::getMaxDRVDT(const size_t pvtRegionIdx) const{
        if (drvdtActive()){
            return m_maxDRVDT[pvtRegionIdx];
        }else{
            throw std::logic_error("Only valid if DRVDT is active");
        }
    }

    double OilVaporizationProperties::getMaxDRSDT(const size_t pvtRegionIdx) const{
        if (drsdtActive()){
            return m_maxDRSDT[pvtRegionIdx];
        }else{
            throw std::logic_error("Only valid if DRSDT is active");
        }
    }

    bool OilVaporizationProperties::getOption(const size_t pvtRegionIdx) const{
        if (drsdtActive()){
            return m_maxDRSDT_allCells[pvtRegionIdx];
        }else{
            throw std::logic_error("Only valid if DRSDT is active");
        }
    }

    OilVaporizationProperties::OilVaporization OilVaporizationProperties::getType() const{
        return m_type;
    }

    void OilVaporizationProperties::updateDRSDT(OilVaporizationProperties& ovp, const std::vector<double>& maximums, const std::vector<std::string>& options){
        ovp.m_type = OilVaporization::DRDT;
        ovp.m_maxDRSDT = maximums;
        for (size_t pvtRegionIdx = 0; pvtRegionIdx < options.size(); ++pvtRegionIdx) {
            if (options[pvtRegionIdx] == "ALL"){
                ovp.m_maxDRSDT_allCells[pvtRegionIdx] = true;
            } else if (options[pvtRegionIdx] == "FREE") {
                ovp.m_maxDRSDT_allCells[pvtRegionIdx] = false;
            } else {
                throw std::invalid_argument("Only ALL or FREE is allowed as option string");
            }
        }
    }

    void OilVaporizationProperties::updateDRSDTCON(OilVaporizationProperties& ovp, const std::vector<double>& maximums, const std::vector<std::string>& options){
        ovp.m_type = OilVaporization::DRSDTCON;
        ovp.m_maxDRSDT = maximums;
        for (size_t pvtRegionIdx = 0; pvtRegionIdx < options.size(); ++pvtRegionIdx) {
            if (options[pvtRegionIdx] == "ALL"){
                ovp.m_maxDRSDT_allCells[pvtRegionIdx] = true;
            } else if (options[pvtRegionIdx] == "FREE") {
                ovp.m_maxDRSDT_allCells[pvtRegionIdx] = false;
            } else {
                throw std::invalid_argument("Only ALL or FREE is allowed as option string");
            }
        }
    }

    void OilVaporizationProperties::updateDRVDT(OilVaporizationProperties& ovp, const std::vector<double>& maximums){
        ovp.m_type = OilVaporization::DRDT;
        ovp.m_maxDRVDT = maximums;
    }

    void OilVaporizationProperties::updateVAPPARS(OilVaporizationProperties& ovp, double vap1, double vap2){
        ovp.m_type = OilVaporization::VAPPARS;
        ovp.m_vap1 = vap1;
        ovp.m_vap2 = vap2;
    }

    bool OilVaporizationProperties::defined() const {
        return this->m_type != OilVaporization::UNDEF;
    }

    bool OilVaporizationProperties::drsdtActive() const {
        return (m_maxDRSDT[0] >= 0 && (m_type == OilVaporization::DRDT || m_type == OilVaporization::DRSDTCON));
    }

    bool OilVaporizationProperties::drsdtConvective() const {
        return this->m_type == OilVaporization::DRSDTCON;
    }

    bool OilVaporizationProperties::drvdtActive() const {
        return (m_maxDRVDT[0] >= 0 && m_type == OilVaporization::DRDT);
    }

    bool OilVaporizationProperties::operator==( const OilVaporizationProperties& rhs ) const {
        return m_type == rhs.m_type &&
               m_vap1 == rhs.m_vap1 &&
               m_vap2 == rhs.m_vap2 &&
               m_maxDRSDT == rhs.m_maxDRSDT &&
               m_maxDRSDT_allCells == rhs.m_maxDRSDT_allCells &&
               m_maxDRVDT == rhs.m_maxDRVDT;
    }

    bool OilVaporizationProperties::operator!=( const OilVaporizationProperties& rhs ) const {
        return !(*this == rhs);
    }

    double OilVaporizationProperties::vap1() const {
        return m_vap1;
    }

    double OilVaporizationProperties::vap2() const {
        return m_vap2;
    }
}
