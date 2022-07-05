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

#include <cassert>
#include <cmath>
#include <iostream>
#include <fmt/format.h>

#include <opm/input/eclipse/Deck/DeckItem.hpp>
#include <opm/input/eclipse/Deck/DeckKeyword.hpp>
#include <opm/input/eclipse/Deck/DeckRecord.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/V.hpp>
#include <opm/input/eclipse/Units/Dimension.hpp>
#include <opm/input/eclipse/Units/UnitSystem.hpp>

#include <opm/input/eclipse/Schedule/VFPInjTable.hpp>

//Anonymous namespace
namespace {

/**
 * Trivial helper function that throws if a zero-sized item is found.
 */
template <typename T>
inline const Opm::DeckItem& getNonEmptyItem( const Opm::DeckRecord& record) {
    const auto& retval = record.getItem<T>();
    if (!retval.hasValue(0)) {
        throw std::invalid_argument("Missing data");
    }
    return retval;
}

} //Namespace




namespace Opm {

VFPInjTable::VFPInjTable()
{
    m_table_num = -1;
    m_datum_depth = 0.0;
    m_flo_type = FLO_TYPE::FLO_OIL;
}


VFPInjTable::VFPInjTable( const DeckKeyword& table, const UnitSystem& deck_unit_system) :
    m_location(table.location())
{
    using ParserKeywords::VFPINJ;

    //Check that the table has enough records
    if (table.size() < 4) {
        throw std::invalid_argument("VFPINJ table does not appear to have enough records to be valid");
    }

    //Get record 1, the metadata for the table
    const auto& header = table.getRecord(0);

    //Get the different header items
    m_table_num   = getNonEmptyItem<VFPINJ::TABLE>(header).get< int >(0);
    m_datum_depth = getNonEmptyItem<VFPINJ::DATUM_DEPTH>(header).getSIDouble(0);

    m_flo_type = getFloType(getNonEmptyItem<VFPINJ::RATE_TYPE>(header).get< std::string >(0));

    //Not used, but check that PRESSURE_DEF is indeed THP
    std::string quantity_string = getNonEmptyItem<VFPINJ::PRESSURE_DEF>(header).get< std::string >(0);
    if (quantity_string != "THP") {
        throw std::invalid_argument("PRESSURE_DEF is required to be THP");
    }

    //Check units used for this table
    std::string units_string = "";
    if (header.getItem<VFPINJ::UNITS>().hasValue(0)) {
        units_string = header.getItem<VFPINJ::UNITS>().get< std::string >(0);
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
            table_unit_type = UnitSystem::UnitType::UNIT_TYPE_FIELD;
        }
        else if (units_string == "PVT-M") {
            throw std::invalid_argument("Unsupported UNITS string: 'PVT-M'");
        }
        else {
            throw std::invalid_argument("Invalid UNITS string");
        }

        //Sanity check
        if(table_unit_type != deck_unit_system.getType()) {
            throw std::invalid_argument("Deck units are not equal VFPINJ table units.");
        }
    }

    //Quantity in the body of the table
    std::string body_string = getNonEmptyItem<VFPINJ::BODY_DEF>(header).get< std::string >(0);
    if (body_string != "BHP") {
        throw std::invalid_argument("Invalid BODY_DEF string");
    }


    //Get actual rate / flow values
    m_flo_data = getNonEmptyItem<VFPINJ::FLOW_VALUES>(table.getRecord(1)).getData< double >();
    convertFloToSI(m_flo_type, m_flo_data, deck_unit_system);

    //Get actual tubing head pressure values
    m_thp_data = getNonEmptyItem<VFPINJ::THP_VALUES>(table.getRecord(2)).getData< double >();
    convertTHPToSI(m_thp_data, deck_unit_system);

    //Finally, read the actual table itself.
    size_t nt = m_thp_data.size();
    size_t nf = m_flo_data.size();
    m_data.resize(nt*nf);
    std::fill_n(m_data.data(), m_data.size(), std::nan("0"));

    //Check that size of table matches size of axis:
    if (table.size() != nt + 3) {
        throw std::invalid_argument("VFPINJ table does not contain enough records.");
    }

    const double table_scaling_factor = deck_unit_system.parse("Pressure").getSIScaling();
    for (size_t i=3; i<table.size(); ++i) {
        const auto& record = table.getRecord(i);
        //Get indices (subtract 1 to get 0-based index)
        int t = getNonEmptyItem<VFPINJ::THP_INDEX>(record).get< int >(0) - 1;

        //Rest of values (bottom hole pressure or tubing head temperature) have index of flo value
        const std::vector<double>& bhp_tht = getNonEmptyItem<VFPINJ::VALUES>(record).getData< double >();

        if (bhp_tht.size() != nf) {
            throw std::invalid_argument("VFPINJ table does not contain enough FLO values.");
        }

        for (unsigned int f=0; f<bhp_tht.size(); ++f) {
            const double& value = bhp_tht[f];
            if (value > 1.0e10) {
                //TODO: Replace with proper log message
                std::cerr << "Too large value encountered in VFPINJ in ["
                        << t << "," << f << "]=" << value << std::endl;
            }
            (*this)(t,f) = table_scaling_factor*value;
        }
    }

    check();
}


VFPInjTable VFPInjTable::serializeObject()
{
    VFPInjTable result;
    result.m_table_num = 1;
    result.m_datum_depth = 2.0;
    result.m_flo_type = FLO_TYPE::FLO_WAT;
    result.m_flo_data = {3.0, 4.0};
    result.m_thp_data = {5.0, 6.0};
    result.m_data = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0};
    result.m_location = KeywordLocation::serializeObject();

    return result;
}




namespace {

void check_axis(const std::vector<double>& axis) {
    if (axis.size() == 0)
        throw std::invalid_argument("Empty axis");

    if (!std::is_sorted(axis.begin(), axis.end()))
        throw std::invalid_argument("Axis is not sorted");
}

}




void VFPInjTable::check() {
    if (this->m_table_num <= 0)
        throw std::invalid_argument(fmt::format("Invalid table number: {}", this->m_table_num));

    //Data axis size
    check_axis(this->m_flo_data);
    check_axis(this->m_thp_data);

    //Check data size matches axes
    if (this->m_data.size() != this->m_thp_data.size() * this->m_flo_data.size())
        throw std::invalid_argument("Wrong data size");

    //Finally, check that all data is within reasonable ranges, defined to be up-to 1.0e10...
    for (std::size_t t = 0; t < m_thp_data.size(); ++t) {
        for (std::size_t f = 0; f < m_flo_data.size(); ++f) {
            if (std::isnan((*this)(t,f))) {
                const auto& location = this->m_location;
                auto msg = fmt::format("VFPINJ table {}\n"
                                       "In {} line {}\n"
                                       "Element THP={}  FLO={} not initialized",
                                       this->m_table_num,
                                       location.filename, location.lineno,
                                       t,f);

                throw std::invalid_argument(msg);
            }
        }
    }
}









VFPInjTable::FLO_TYPE VFPInjTable::getFloType(std::string flo_string) {
    if (flo_string == "OIL")
        return FLO_TYPE::FLO_OIL;

    if (flo_string == "WAT")
        return FLO_TYPE::FLO_WAT;

    if (flo_string == "GAS")
        return FLO_TYPE::FLO_GAS;

    throw std::invalid_argument("Invalid RATE_TYPE string");
}








void VFPInjTable::scaleValues(std::vector<double>& values,
                               const double& scaling_factor) {
    if (scaling_factor == 1.0) {
        return;
    }
    else {
        for (unsigned int i=0; i<values.size(); ++i) {
            values[i] *= scaling_factor;
        }
    }
}







void VFPInjTable::convertFloToSI(const FLO_TYPE& type,
                                  std::vector<double>& values,
                                  const UnitSystem& unit_system) {
    double scaling_factor = 1.0;
    switch (type) {
    case FLO_TYPE::FLO_OIL:
    case FLO_TYPE::FLO_WAT:
        scaling_factor = unit_system.getDimension(UnitSystem::measure::liquid_surface_rate).getSIScaling();
        break;
    case FLO_TYPE::FLO_GAS:
        scaling_factor = unit_system.getDimension(UnitSystem::measure::gas_surface_rate).getSIScaling();
        break;
    default:
        throw std::logic_error("Invalid FLO type");
    }
    scaleValues(values, scaling_factor);
}







void VFPInjTable::convertTHPToSI(std::vector<double>& values,
                                 const UnitSystem& unit_system) {
    double scaling_factor = unit_system.parse("Pressure").getSIScaling();
    scaleValues(values, scaling_factor);
}


bool VFPInjTable::operator==(const VFPInjTable& data) const {
    return this->getTableNum() == data.getTableNum() &&
           this->getDatumDepth () == data.getDatumDepth() &&
           this->getFloType() == data.getFloType() &&
           this->getFloAxis() == data.getFloAxis() &&
           this->getTHPAxis() == data.getTHPAxis() &&
           this->getTable() == data.getTable() &&
           this->location() == data.location();
}


double VFPInjTable::operator()(size_t thp_idx, size_t flo_idx) const {
    return m_data[thp_idx*m_flo_data.size() + flo_idx];
}


double& VFPInjTable::operator()(size_t thp_idx, size_t flo_idx) {
    return m_data[thp_idx*m_flo_data.size() + flo_idx];
}


std::array<size_t,2> VFPInjTable::shape() const {
    return {m_thp_data.size(), m_flo_data.size()};
}


} //Namespace
