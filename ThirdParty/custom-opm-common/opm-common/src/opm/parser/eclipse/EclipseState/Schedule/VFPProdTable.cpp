/*
  Copyright 2015 SINTEF ICT, Applied Mathematics.

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

#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>

#include <opm/common/OpmLog/OpmLog.hpp>

#include <opm/parser/eclipse/Deck/DeckItem.hpp>
#include <opm/parser/eclipse/Deck/DeckKeyword.hpp>
#include <opm/parser/eclipse/Deck/DeckRecord.hpp>
#include <opm/parser/eclipse/Parser/ParserKeywords/V.hpp>
#include <opm/parser/eclipse/Units/Dimension.hpp>
#include <opm/parser/eclipse/Units/UnitSystem.hpp>

#include <opm/parser/eclipse/EclipseState/Schedule/VFPProdTable.hpp>


namespace Opm {

namespace {

VFPProdTable::FLO_TYPE getFloType( const DeckItem& item) {
    const std::string& flo_string = item.getTrimmedString(0);
    if (flo_string == "OIL") {
        return VFPProdTable::FLO_OIL;
    }
    else if (flo_string == "LIQ") {
        return VFPProdTable::FLO_LIQ;
    }
    else if (flo_string == "GAS") {
        return VFPProdTable::FLO_GAS;
    }
    else {
        throw std::invalid_argument("Invalid RATE_TYPE string: " + flo_string);
    }
    return VFPProdTable::FLO_INVALID;
}

VFPProdTable::WFR_TYPE getWFRType( const DeckItem& item) {
    const std::string& wfr_string = item.getTrimmedString(0);
    if (wfr_string == "WOR") {
        return VFPProdTable::WFR_WOR;
    }
    else if (wfr_string == "WCT") {
        return VFPProdTable::WFR_WCT;
    }
    else if (wfr_string == "WGR") {
        return VFPProdTable::WFR_WGR;
    }
    else {
        throw std::invalid_argument("Invalid WFR string");
    }
    return VFPProdTable::WFR_INVALID;
}

VFPProdTable::GFR_TYPE getGFRType( const DeckItem& item) {;
    const std::string& gfr_string = item.getTrimmedString(0);
    if (gfr_string == "GOR") {
        return VFPProdTable::GFR_GOR;
    }
    else if (gfr_string == "GLR") {
        return VFPProdTable::GFR_GLR;
    }
    else if (gfr_string == "OGR") {
        return VFPProdTable::GFR_OGR;
    }
    else {
        throw std::invalid_argument("Invalid GFR string");
    }
    return VFPProdTable::GFR_INVALID;
}

VFPProdTable::ALQ_TYPE getALQType( const DeckItem& item) {
    if (item.defaultApplied(0)) {
        return VFPProdTable::ALQ_UNDEF;
    } else {
        const std::string& alq_string = item.getTrimmedString(0);

        if (alq_string == "GRAT") {
            return VFPProdTable::ALQ_GRAT;
        }
        else if (alq_string == "IGLR") {
            return VFPProdTable::ALQ_IGLR;
        }
        else if (alq_string == "TGLR") {
            return VFPProdTable::ALQ_TGLR;
        }
        else if (alq_string == "PUMP") {
            return VFPProdTable::ALQ_PUMP;
        }
        else if (alq_string == "COMP") {
            return VFPProdTable::ALQ_COMP;
        }
        else if (alq_string == "BEAN") {
            return VFPProdTable::ALQ_BEAN;
        }
        else if (alq_string == "") {
            return VFPProdTable::ALQ_UNDEF;
        }
        else {
            throw std::invalid_argument("Invalid ALQ_DEF string: " + alq_string);
        }

        return VFPProdTable::ALQ_INVALID;
    }
}

}


VFPProdTable::VFPProdTable()
{
    m_table_num = -1;
    m_datum_depth = 0.0;
    m_flo_type = FLO_INVALID;
    m_wfr_type = WFR_INVALID;
    m_gfr_type = GFR_INVALID;
    m_alq_type = ALQ_INVALID;

}


VFPProdTable::VFPProdTable(int table_num,
                           double datum_depth,
                           FLO_TYPE flo_type,
                           WFR_TYPE wfr_type,
                           GFR_TYPE gfr_type,
                           ALQ_TYPE alq_type,
                           const std::vector<double>& flo_data,
                           const std::vector<double>& thp_data,
                           const std::vector<double>& wfr_data,
                           const std::vector<double>& gfr_data,
                           const std::vector<double>& alq_data,
                           const array_type& data) {

    m_table_num = table_num;
    m_datum_depth = datum_depth;
    m_flo_type = flo_type;
    m_wfr_type = wfr_type;
    m_gfr_type = gfr_type;
    m_alq_type = alq_type;
    m_flo_data = flo_data;
    m_thp_data = thp_data;
    m_wfr_data = wfr_data;
    m_gfr_data = gfr_data;
    m_alq_data = alq_data;
    m_data = data;

    //check();
}


VFPProdTable VFPProdTable::serializeObject()
{
    VFPProdTable result;
    result.m_table_num = 1;
    result.m_datum_depth = 2;
    result.m_flo_type = FLO_OIL;
    result.m_wfr_type = WFR_WOR;
    result.m_gfr_type = GFR_GLR;
    result.m_alq_type = ALQ_TGLR;
    result.m_flo_data = {1.0, 2.0, 3.0, 4.0, 5.0};
    result.m_thp_data = {6.0};
    result.m_wfr_data = {7.0};
    result.m_gfr_data = {8.0};
    result.m_alq_data = {9.0, 10.0, 11.0};
    result.m_data = {12.0, 13.0, 14.0, 15.0, 16.0};

    return result;
}


VFPProdTable::VFPProdTable( const DeckKeyword& table, const UnitSystem& deck_unit_system) {
    using ParserKeywords::VFPPROD;

    //Check that the table has enough records
    if (table.size() < 7) {
        throw std::invalid_argument("VFPPROD table does not appear to have enough records to be valid");
    }

    //Get record 1, the metadata for the table
    const auto& header = table.getRecord(0);

    //Get the different header items
    m_table_num   = header.getItem<VFPPROD::TABLE>().get< int >(0);
    m_datum_depth = header.getItem<VFPPROD::DATUM_DEPTH>().getSIDouble(0);

    m_flo_type = Opm::getFloType(header.getItem<VFPPROD::RATE_TYPE>());
    m_wfr_type = Opm::getWFRType(header.getItem<VFPPROD::WFR>());
    m_gfr_type = Opm::getGFRType(header.getItem<VFPPROD::GFR>());

    //Not used, but check that PRESSURE_DEF is indeed THP
    std::string quantity_string = header.getItem<VFPPROD::PRESSURE_DEF>().get< std::string >(0);
    if (quantity_string != "THP") {
        throw std::invalid_argument("PRESSURE_DEF is required to be THP");
    }

    m_alq_type = Opm::getALQType(header.getItem<VFPPROD::ALQ_DEF>());

    //Check units used for this table
    std::string units_string = "";
    if (header.getItem<VFPPROD::UNITS>().hasValue(0)) {
        units_string = header.getItem<VFPPROD::UNITS>().get< std::string >(0);
    }
    else {
        //If units does not exist in record, the default value is the
        //unit system of the deck itself: do nothing...
    }

    if (units_string != "") {
        UnitSystem::UnitType table_unit_type;

        //FIXME: Only metric and field supported at the moment.
        //Need to change all of the convertToSI functions to support LAB/PVT-M

        if (units_string == "METRIC") {
            table_unit_type = UnitSystem::UnitType::UNIT_TYPE_METRIC;
        }
        else if (units_string == "FIELD") {
            table_unit_type = UnitSystem::UnitType::UNIT_TYPE_FIELD;
        }
        else if (units_string == "LAB") {
            table_unit_type = UnitSystem::UnitType::UNIT_TYPE_LAB;
        }
        else if (units_string == "PVT-M") {
            throw std::invalid_argument("Unsupported UNITS string: 'PVT-M'");
        }
        else {
            throw std::invalid_argument("Invalid UNITS string");
        }

        //Sanity check
        if(table_unit_type != deck_unit_system.getType()) {
            throw std::invalid_argument("Deck units are not equal VFPPROD table units.");
        }
    }

    //Quantity in the body of the table
    std::string body_string = header.getItem<VFPPROD::BODY_DEF>().get< std::string >(0);
    if (body_string == "TEMP") {
        throw std::invalid_argument("Invalid BODY_DEF string: TEMP not supported");
    }
    else if (body_string == "BHP") {

    }
    else {
        throw std::invalid_argument("Invalid BODY_DEF string");
    }


    //Get actual rate / flow values
    m_flo_data = table.getRecord(1).getItem<VFPPROD::FLOW_VALUES>().getData< double >();
    convertFloToSI(m_flo_type, m_flo_data, deck_unit_system);

    //Get actual tubing head pressure values
    m_thp_data = table.getRecord(2).getItem<VFPPROD::THP_VALUES>().getData< double >();
    convertTHPToSI(m_thp_data, deck_unit_system);

    //Get actual water fraction values
    m_wfr_data = table.getRecord(3).getItem<VFPPROD::WFR_VALUES>().getData< double >();
    convertWFRToSI(m_wfr_type, m_wfr_data, deck_unit_system);

    //Get actual gas fraction values
    m_gfr_data = table.getRecord(4).getItem<VFPPROD::GFR_VALUES>().getData< double >();
    convertGFRToSI(m_gfr_type, m_gfr_data, deck_unit_system);

    //Get actual gas fraction values
    m_alq_data = table.getRecord(5).getItem<VFPPROD::ALQ_VALUES>().getData< double >();

    //Finally, read the actual table itself.
    size_t nt = m_thp_data.size();
    size_t nw = m_wfr_data.size();
    size_t ng = m_gfr_data.size();
    size_t na = m_alq_data.size();
    size_t nf = m_flo_data.size();
    m_data.resize(nt*nw*ng*na*nf);
    std::fill_n(m_data.data(), m_data.size(), std::nan("0"));

    //Check that size of table matches size of axis:
    if (table.size() != nt*nw*ng*na + 6) {
        throw std::invalid_argument("VFPPROD table does not contain enough records.");
    }

    //FIXME: Unit for TEMP=Tubing head temperature is not Pressure, see BODY_DEF
    const double table_scaling_factor = deck_unit_system.parse("Pressure").getSIScaling();
    for (size_t i=6; i<table.size(); ++i) {
        const auto& record = table.getRecord(i);
        //Get indices (subtract 1 to get 0-based index)
        int t = record.getItem<VFPPROD::THP_INDEX>().get< int >(0) - 1;
        int w = record.getItem<VFPPROD::WFR_INDEX>().get< int >(0) - 1;
        int g = record.getItem<VFPPROD::GFR_INDEX>().get< int >(0) - 1;
        int a = record.getItem<VFPPROD::ALQ_INDEX>().get< int >(0) - 1;

        //Rest of values (bottom hole pressure or tubing head temperature) have index of flo value
        const std::vector<double>& bhp_tht = record.getItem<VFPPROD::VALUES>().getData< double >();

        if (bhp_tht.size() != nf) {
            throw std::invalid_argument("VFPPROD table does not contain enough FLO values.");
        }

        for (size_t f=0; f<bhp_tht.size(); ++f) {
            //Check that all data is within reasonable ranges, defined to be up-to 1.0e10...
            if (bhp_tht[f] > 1.0e10) {
                //TODO: Replace with proper log message
                std::cerr << "VFPPROD element ["
                        << t << "," << w << "," << g << "," << a << "," << f
                        << "]=" << bhp_tht[f] << " too large" << std::endl;
            }
            (*this)(t,w,g,a,f) = table_scaling_factor*bhp_tht[f];
        }
    }

    check(table, table_scaling_factor);
}










void VFPProdTable::check(const DeckKeyword& keyword, const double table_scaling_factor) {
    //Table number
    assert(m_table_num > 0);

    //Misc types
    assert(m_flo_type >= FLO_OIL && m_flo_type < FLO_INVALID);
    assert(m_wfr_type >= WFR_WOR && m_wfr_type < WFR_INVALID);
    assert(m_gfr_type >= GFR_GOR && m_gfr_type < GFR_INVALID);
    assert(m_alq_type >= ALQ_GRAT && m_alq_type < ALQ_INVALID);

    //Data axis size
    assert(m_flo_data.size() > 0);
    assert(m_thp_data.size() > 0);
    assert(m_wfr_data.size() > 0);
    assert(m_gfr_data.size() > 0);
    assert(m_alq_data.size() > 0);

    //Data axis sorted?
    assert(std::is_sorted(m_flo_data.begin(), m_flo_data.end()));
    assert(std::is_sorted(m_thp_data.begin(), m_thp_data.end()));
    assert(std::is_sorted(m_wfr_data.begin(), m_wfr_data.end()));
    assert(std::is_sorted(m_gfr_data.begin(), m_gfr_data.end()));
    assert(std::is_sorted(m_alq_data.begin(), m_alq_data.end()));

    size_t nt = m_thp_data.size();
    size_t nw = m_wfr_data.size();
    size_t ng = m_gfr_data.size();
    size_t na = m_alq_data.size();
    size_t nf = m_flo_data.size();

    //Check data size matches axes
    assert(m_data.size() == nt*nw*ng*na*nf);

    //Check that bhp(thp) is a monotonic increasing function.
    //If this is not the case, we might not be able to determine
    //the thp from the bhp easily
    typedef array_type::size_type size_type;
    std::string points;
    for (size_type w = 0; w < nw; ++w) {
        for (size_type g = 0; g < ng; ++g) {
            for (size_type a = 0; a < na; ++a) {
                for (size_type f = 0; f < nf; ++f) {
                    double bhp_last = (*this)(0,w,g,a,f);
                    for (size_type t = 0; t < nt; ++t) {
                        //Check that all elements have been set
                        if (std::isnan((*this)(t,w,g,a,f))) {
                            //TODO: Replace with proper log message
                            std::cerr << "VFPPROD element ["
                                    << t << "," << w << "," << g << "," << a << "," << f
                                    << "] not set!" << std::endl;
                            throw std::invalid_argument("Missing VFPPROD value");
                        }
                        if ((*this)(t,w,g,a,f) < bhp_last) {
                            points += "At point (FLOW, THP, WFR, GFR, ALQ) = "
                                    + std::to_string(f) + " " + std::to_string(t) + " "
                                    + std::to_string(w) + " " + std::to_string(g) + " "
                                    + std::to_string(a) + " at BHP = "
                                    + std::to_string((*this)(t,w,g,a,f) / table_scaling_factor) + "\n";
                        }
                        bhp_last = (*this)(t,w,g,a,f);
                    }
                }
            }
        }
    }

    if (!points.empty()) {
        const auto& location = keyword.location();
        OpmLog::warning("VFP table for production wells has BHP versus THP not "
                           + std::string("monotonically increasing.\nThis may cause convergence ")
                           + "issues due to switching between BHP and THP control mode."
                           + std::string("\nIn keyword VFPPROD table number ")
                           + std::to_string(m_table_num)
                           + ", file " + location.filename
                           + ", line " + std::to_string(location.lineno)
                           + "\n");
        OpmLog::note(points);
    }
}








void VFPProdTable::scaleValues(std::vector<double>& values,
                               const double& scaling_factor) {
    if (scaling_factor == 1.0) {
        return;
    }
    else {
        for (size_t i=0; i<values.size(); ++i) {
            values[i] *= scaling_factor;
        }
    }
}







void VFPProdTable::convertFloToSI(const FLO_TYPE& type,
                                  std::vector<double>& values,
                                  const UnitSystem& unit_system) {
    double scaling_factor = 1.0;
    switch (type) {
        case FLO_OIL:
        case FLO_LIQ:
            scaling_factor = unit_system.parse("LiquidSurfaceVolume/Time").getSIScaling();
            break;
        case FLO_GAS:
            scaling_factor = unit_system.parse("GasSurfaceVolume/Time").getSIScaling();
            break;
        default:
            throw std::logic_error("Invalid FLO type");
    }
    scaleValues(values, scaling_factor);
}







void VFPProdTable::convertTHPToSI(std::vector<double>& values,
                                  const UnitSystem& unit_system) {
    double scaling_factor = unit_system.parse("Pressure").getSIScaling();
    scaleValues(values, scaling_factor);
}







void VFPProdTable::convertWFRToSI(const WFR_TYPE& type,
                                  std::vector<double>& values,
                                  const UnitSystem& unit_system) {
    double scaling_factor = 1.0;
    switch (type) {
        case WFR_WOR:
        case WFR_WCT:
            scaling_factor = unit_system.parse("LiquidSurfaceVolume/LiquidSurfaceVolume").getSIScaling();
            break;
        case WFR_WGR:
            scaling_factor = unit_system.parse("LiquidSurfaceVolume/GasSurfaceVolume").getSIScaling();
            break;
        default:
            throw std::logic_error("Invalid FLO type");
    }
    scaleValues(values, scaling_factor);
}







void VFPProdTable::convertGFRToSI(const GFR_TYPE& type,
                                  std::vector<double>& values,
                                  const UnitSystem& unit_system) {
    double scaling_factor = 1.0;
    switch (type) {
        case GFR_GOR:
        case GFR_GLR:
            scaling_factor = unit_system.parse("GasSurfaceVolume/LiquidSurfaceVolume").getSIScaling();
            break;
        case GFR_OGR:
            scaling_factor = unit_system.parse("LiquidSurfaceVolume/GasSurfaceVolume").getSIScaling();
            break;
        default:
            throw std::logic_error("Invalid FLO type");
    }
    scaleValues(values, scaling_factor);
}


bool VFPProdTable::operator==(const VFPProdTable& data) const {
    return this->getTableNum() == data.getTableNum() &&
           this->getDatumDepth () == data.getDatumDepth() &&
           this->getFloType() == data.getFloType() &&
           this->getWFRType() == data.getWFRType() &&
           this->getGFRType() == data.getGFRType() &&
           this->getALQType() == data.getALQType() &&
           this->getFloAxis() == data.getFloAxis() &&
           this->getTHPAxis() == data.getTHPAxis() &&
           this->getWFRAxis() == data.getWFRAxis() &&
           this->getGFRAxis() == data.getGFRAxis() &&
           this->getALQAxis() == data.getALQAxis() &&
           this->getTable() == data.getTable();
}


double VFPProdTable::operator()(size_t thp_idx, size_t wfr_idx, size_t gfr_idx, size_t alq_idx, size_t flo_idx) const {
    size_t nw = m_wfr_data.size();
    size_t ng = m_gfr_data.size();
    size_t na = m_alq_data.size();
    size_t nf = m_flo_data.size();

    return m_data[thp_idx*nw*ng*na*nf + wfr_idx*ng*na*nf + gfr_idx*na*nf + alq_idx*nf + flo_idx];
}


double& VFPProdTable::operator()(size_t thp_idx, size_t wfr_idx, size_t gfr_idx, size_t alq_idx, size_t flo_idx) {
    size_t nw = m_wfr_data.size();
    size_t ng = m_gfr_data.size();
    size_t na = m_alq_data.size();
    size_t nf = m_flo_data.size();

    return m_data[thp_idx*nw*ng*na*nf + wfr_idx*ng*na*nf + gfr_idx*na*nf + alq_idx*nf + flo_idx];
}


std::array<size_t,5> VFPProdTable::shape() const {
    size_t nt = m_thp_data.size();
    size_t nw = m_wfr_data.size();
    size_t ng = m_gfr_data.size();
    size_t na = m_alq_data.size();
    size_t nf = m_flo_data.size();

    return {nt, nw, ng, na, nf};
}

} //Namespace opm
