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

#include <fmt/format.h>

#include <opm/common/OpmLog/OpmLog.hpp>

#include <opm/input/eclipse/Deck/DeckItem.hpp>
#include <opm/input/eclipse/Deck/DeckKeyword.hpp>
#include <opm/input/eclipse/Deck/DeckRecord.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/V.hpp>
#include <opm/input/eclipse/Units/Dimension.hpp>
#include <opm/input/eclipse/Units/UnitSystem.hpp>

#include <opm/input/eclipse/Schedule/VFPProdTable.hpp>


namespace Opm {

namespace {

VFPProdTable::FLO_TYPE getFloType( const DeckItem& item) {
    const std::string& flo_string = item.getTrimmedString(0);
    if (flo_string == "OIL")
        return VFPProdTable::FLO_TYPE::FLO_OIL;

    if (flo_string == "LIQ")
        return VFPProdTable::FLO_TYPE::FLO_LIQ;

    if (flo_string == "GAS")
        return VFPProdTable::FLO_TYPE::FLO_GAS;

    throw std::invalid_argument("Invalid RATE_TYPE string: " + flo_string);
}

VFPProdTable::WFR_TYPE getWFRType( const DeckItem& item) {
    const std::string& wfr_string = item.getTrimmedString(0);
    if (wfr_string == "WOR")
        return VFPProdTable::WFR_TYPE::WFR_WOR;

    if (wfr_string == "WCT")
        return VFPProdTable::WFR_TYPE::WFR_WCT;

    if (wfr_string == "WGR")
        return VFPProdTable::WFR_TYPE::WFR_WGR;

    throw std::invalid_argument("Invalid WFR string");
}

VFPProdTable::GFR_TYPE getGFRType( const DeckItem& item) {;
    const std::string& gfr_string = item.getTrimmedString(0);
    if (gfr_string == "GOR")
        return VFPProdTable::GFR_TYPE::GFR_GOR;

    if (gfr_string == "GLR")
        return VFPProdTable::GFR_TYPE::GFR_GLR;

    if (gfr_string == "OGR")
        return VFPProdTable::GFR_TYPE::GFR_OGR;

    throw std::invalid_argument("Invalid GFR string");
}

VFPProdTable::ALQ_TYPE getALQType( const DeckItem& item, bool gaslift_opt_active) {
    if (item.defaultApplied(0)) {
        if (gaslift_opt_active)
            return VFPProdTable::ALQ_TYPE::ALQ_GRAT;
        return VFPProdTable::ALQ_TYPE::ALQ_UNDEF;
    } else {
        const std::string& alq_string = item.getTrimmedString(0);

        if (alq_string == "GRAT")
            return VFPProdTable::ALQ_TYPE::ALQ_GRAT;

        if (alq_string == "IGLR")
            return VFPProdTable::ALQ_TYPE::ALQ_IGLR;

        if (alq_string == "TGLR")
            return VFPProdTable::ALQ_TYPE::ALQ_TGLR;

        if (alq_string == "PUMP")
            return VFPProdTable::ALQ_TYPE::ALQ_PUMP;

        if (alq_string == "COMP")
            return VFPProdTable::ALQ_TYPE::ALQ_COMP;

        if (alq_string == "BEAN")
            return VFPProdTable::ALQ_TYPE::ALQ_BEAN;

        if (alq_string == "") {
            if (gaslift_opt_active)
                return VFPProdTable::ALQ_TYPE::ALQ_GRAT;
            return VFPProdTable::ALQ_TYPE::ALQ_UNDEF;
        }

        throw std::invalid_argument("Invalid ALQ_DEF string: " + alq_string);
    }
}

}


VFPProdTable::VFPProdTable()
{
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
                           const std::vector<double>& data) {

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

    if (data.size() != flo_data.size() * thp_data.size() * wfr_data.size() * gfr_data.size() * alq_data.size())
        throw std::invalid_argument("Wrong data size");
    this->check();
}


VFPProdTable VFPProdTable::serializeObject()
{
    VFPProdTable result;
    result.m_table_num = 1;
    result.m_datum_depth = 2;
    result.m_flo_type = FLO_TYPE::FLO_OIL;
    result.m_wfr_type = WFR_TYPE::WFR_WOR;
    result.m_gfr_type = GFR_TYPE::GFR_GLR;
    result.m_alq_type = ALQ_TYPE::ALQ_TGLR;
    result.m_flo_data = {1.0, 2.0, 3.0, 4.0, 5.0};
    result.m_thp_data = {6.0};
    result.m_wfr_data = {7.0};
    result.m_gfr_data = {8.0};
    result.m_alq_data = {9.0, 10.0, 11.0};
    result.m_data = {12.0, 13.0, 14.0, 15.0, 16.0};
    result.m_location = KeywordLocation::serializeObject();

    return result;
}


/*
  If the gaslift_opt flag is set to true the ALQ_TYPE item will default to GRAT.
*/

VFPProdTable::VFPProdTable( const DeckKeyword& table, bool gaslift_opt_active, const UnitSystem& deck_unit_system) :
    m_location(table.location())
{
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

    m_alq_type = Opm::getALQType(header.getItem<VFPPROD::ALQ_DEF>(), gaslift_opt_active);

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
    convertAlqToSI(m_alq_type, m_alq_data, deck_unit_system);
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
    const double table_scaling_factor = deck_unit_system.getDimension(UnitSystem::measure::pressure).getSIScaling();
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
                auto msg = fmt::format("Problem with VFPPROD table {}\n"
                                       "In {} line {}\n"
                                       "Element(thp={}, wfr={}, GRF={}, ALQ={}) = {} is too large",
                                       this->m_table_num,
                                       this->m_location.filename, this->m_location.lineno,
                                       t,w,g,a,bhp_tht[f]);

                OpmLog::warning(msg);
            }
            (*this)(t,w,g,a,f) = table_scaling_factor*bhp_tht[f];
        }
    }

    check();
}




namespace {

void check_axis(const std::vector<double>& axis) {
    if (axis.size() == 0)
        throw std::invalid_argument("Empty axis");

    if (!std::is_sorted(axis.begin(), axis.end()))
        throw std::invalid_argument("Axis is not sorted");
}

}


void VFPProdTable::check() {
    if (this->m_table_num <= 0)
        throw std::invalid_argument(fmt::format("Invalid table number: {}", this->m_table_num));

    check_axis(this->m_flo_data);
    check_axis(this->m_thp_data);
    check_axis(this->m_wfr_data);
    check_axis(this->m_gfr_data);
    check_axis(this->m_alq_data);

    size_t nt = this->m_thp_data.size();
    size_t nw = this->m_wfr_data.size();
    size_t ng = this->m_gfr_data.size();
    size_t na = this->m_alq_data.size();
    size_t nf = this->m_flo_data.size();

    //Check that bhp(thp) is a monotonic increasing function.
    //If this is not the case, we might not be able to determine
    //the thp from the bhp easily
    std::size_t error_count = 0;
    for (std::size_t w = 0; w < nw; ++w) {
        for (std::size_t g = 0; g < ng; ++g) {
            for (std::size_t a = 0; a < na; ++a) {
                for (std::size_t f = 0; f < nf; ++f) {
                    double bhp_last = (*this)(0,w,g,a,f);
                    for (std::size_t t = 0; t < nt; ++t) {
                        //Check that all elements have been set
                        if (std::isnan((*this)(t,w,g,a,f))) {
                            const auto& location = this->m_location;
                            auto msg = fmt::format("VFPPROD table {}\n"
                                                   "In {} line {}\n"
                                                   "Element THP={}  WFR={} GFR={} ALQ={} FLO={} not initialized",
                                                   this->m_table_num,
                                                   location.filename, location.lineno,
                                                   t,w,g,a,f);

                            throw std::invalid_argument(msg);
                        }
                        if ((*this)(t,w,g,a,f) < bhp_last)
                            error_count += 1;

                        bhp_last = (*this)(t,w,g,a,f);
                    }
                }
            }
        }
    }

    if (error_count > 0) {
        const auto& location = this->m_location;
        OpmLog::warning(fmt::format("VFPPROD table {0} has {1} nonmonotonic points of BHP(THP)\n"
                                    "In {2} line {3}\n"
                                    "This may cause convergence issues due to switching between BHP and THP control.\n",
                                    error_count,
                                    m_table_num,
                                    location.filename,
                                    location.lineno));
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
    const auto liquid_surface_volume = unit_system.getDimension(UnitSystem::measure::liquid_surface_volume).getSIScaling();
    const auto gas_surface_volume    = unit_system.getDimension(UnitSystem::measure::gas_surface_volume).getSIScaling();
    const auto time                  = unit_system.getDimension(UnitSystem::measure::time).getSIScaling();

    switch (type) {
    case FLO_TYPE::FLO_OIL:
    case FLO_TYPE::FLO_LIQ:
        scaling_factor = liquid_surface_volume / time;
        break;
    case FLO_TYPE::FLO_GAS:
        scaling_factor = gas_surface_volume / time;
        break;
    default:
        throw std::logic_error("Invalid FLO type");
    }
    scaleValues(values, scaling_factor);
}







void VFPProdTable::convertTHPToSI(std::vector<double>& values,
                                  const UnitSystem& unit_system) {
    const auto scaling_factor = unit_system.getDimension(UnitSystem::measure::pressure).getSIScaling();
    scaleValues(values, scaling_factor);
}







void VFPProdTable::convertWFRToSI(const WFR_TYPE& type,
                                  std::vector<double>& values,
                                  const UnitSystem& unit_system) {
    double scaling_factor = 1.0;
    const auto liquid_surface_volume = unit_system.getDimension(UnitSystem::measure::liquid_surface_volume).getSIScaling();
    const auto gas_surface_volume    = unit_system.getDimension(UnitSystem::measure::gas_surface_volume).getSIScaling();
    switch (type) {
    case WFR_TYPE::WFR_WOR:
    case WFR_TYPE::WFR_WCT:
        break;
    case WFR_TYPE::WFR_WGR:
        scaling_factor = liquid_surface_volume / gas_surface_volume;
        break;
    default:
        throw std::logic_error("Invalid WFR type");
    }
    scaleValues(values, scaling_factor);
}







void VFPProdTable::convertGFRToSI(const GFR_TYPE& type,
                                  std::vector<double>& values,
                                  const UnitSystem& unit_system) {
    double scaling_factor = 1.0;
    const auto liquid_surface_volume = unit_system.getDimension(UnitSystem::measure::liquid_surface_volume).getSIScaling();
    const auto gas_surface_volume    = unit_system.getDimension(UnitSystem::measure::gas_surface_volume).getSIScaling();
    switch (type) {
    case GFR_TYPE::GFR_GOR:
    case GFR_TYPE::GFR_GLR:
        scaling_factor = gas_surface_volume / liquid_surface_volume;
        break;
    case GFR_TYPE::GFR_OGR:
        scaling_factor = liquid_surface_volume / gas_surface_volume;
        break;
    default:
        throw std::logic_error("Invalid GFR type");
    }
    scaleValues(values, scaling_factor);
}


Dimension VFPProdTable::ALQDimension(const ALQ_TYPE& alq_type, const UnitSystem& unit_system) {
    double scaling_factor = 1.0;
    const auto liquid_surface_volume = unit_system.getDimension(UnitSystem::measure::liquid_surface_volume).getSIScaling();
    const auto gas_surface_volume    = unit_system.getDimension(UnitSystem::measure::gas_surface_volume).getSIScaling();
    const auto time                  = unit_system.getDimension(UnitSystem::measure::time).getSIScaling();

    switch (alq_type) {
    case ALQ_TYPE::ALQ_IGLR:
        scaling_factor = gas_surface_volume / liquid_surface_volume;
        break;
    case ALQ_TYPE::ALQ_TGLR:
        scaling_factor = gas_surface_volume / liquid_surface_volume;
        break;
    case ALQ_TYPE::ALQ_GRAT:
        scaling_factor = gas_surface_volume / time;
        break;
    case ALQ_TYPE::ALQ_UNDEF:
        break;
    case ALQ_TYPE::ALQ_PUMP:
    case ALQ_TYPE::ALQ_COMP:
    case ALQ_TYPE::ALQ_BEAN:
        std::logic_error("scaling of the given ALQ type, not implemented ");
        break;
    default:
        throw std::logic_error("Invalid ALQ type");
    }

    return Dimension(scaling_factor);
}


void VFPProdTable::convertAlqToSI(const ALQ_TYPE& type,
                                  std::vector<double>& values,
                                  const UnitSystem& unit_system) {
    const auto& dim = VFPProdTable::ALQDimension(type, unit_system);
    scaleValues(values, dim.getSIScaling());
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
           this->getTable() == data.getTable() &&
           this->location() == data.location();
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
