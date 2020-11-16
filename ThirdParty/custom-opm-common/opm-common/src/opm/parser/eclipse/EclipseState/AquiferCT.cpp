/*
  Copyright (C) 2017 TNO

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

#include <opm/parser/eclipse/EclipseState/AquiferCT.hpp>
#include <opm/parser/eclipse/EclipseState/EclipseState.hpp>

namespace Opm {

namespace {

// The default Pd v/s Td tables (constant terminal rate case for an infinite
// aquifer) as described in Van Everdingen, A. & Hurst, W., December, 1949.The
// Application of the Laplace Transformation to Flow Problems in Reservoirs.
// Petroleum Transactions, AIME.

static const std::vector<double> default_pressure = { 0.112, 0.229, 0.315, 0.376, 0.424, 0.469, 0.503, 0.564,
                                                      0.616, 0.659, 0.702, 0.735, 0.772, 0.802, 0.927, 1.020,
                                                      1.101, 1.169, 1.275, 1.362, 1.436, 1.500, 1.556, 1.604,
                                                      1.651, 1.829, 1.960, 2.067, 2.147, 2.282, 2.388, 2.476,
                                                      2.550, 2.615, 2.672, 2.723, 2.921, 3.064, 3.173, 3.263,
                                                      3.406, 3.516, 3.608, 3.684, 3.750, 3.809, 3.86 };

static const std::vector<double> default_time =     { 0.010, 0.050, 0.100, 0.150, 0.200, 0.250, 0.300, 0.400,
                                                      0.500, 0.600, 0.700, 0.800, 0.900, 1.000, 1.500, 2.000,
                                                      2.500, 3.000, 4.000, 5.000, 6.000, 7.000, 8.000, 9.000,
                                                      10.00, 15.00, 20.00, 25.00, 30.00, 40.00, 50.00, 60.00,
                                                      70.00, 80.00, 90.00, 100.0, 150.0, 200.0, 250.0, 300.0,
                                                      400.0, 500.0, 600.0, 700.0, 800.0, 900.0, 1000 };

}


AquiferCT::AQUCT_data::AQUCT_data(const DeckRecord& record, const TableManager& tables) :
    aquiferID(record.getItem("AQUIFER_ID").get<int>(0)),
    inftableID(record.getItem("TABLE_NUM_INFLUENCE_FN").get<int>(0)),
    pvttableID(record.getItem("TABLE_NUM_WATER_PRESS").get<int>(0)),
    phi_aq(record.getItem("PORO_AQ").getSIDouble(0)),
    d0(record.getItem("DAT_DEPTH").getSIDouble(0)),
    C_t(record.getItem("C_T").getSIDouble(0)),
    r_o(record.getItem("RAD").getSIDouble(0)),
    k_a(record.getItem("PERM_AQ").getSIDouble(0)),
    c1(1.0),
    h(record.getItem("THICKNESS_AQ").getSIDouble(0)),
    theta( record.getItem("INFLUENCE_ANGLE").getSIDouble(0)/360.0),
    c2(6.283),         // Value of C2 used by E100 (for METRIC, PVT-M and LAB unit systems)
    p0(std::make_pair(false, 0))
{
    if (record.getItem("P_INI").hasValue(0))
        this->p0 = std::make_pair(true, record.getItem("P_INI").getSIDouble(0));

    // Get the correct influence table values
    if (this->inftableID > 1) {
        const auto& aqutabTable = tables.getAqutabTables().getTable(this->inftableID - 2);
        const auto& aqutab_tdColumn = aqutabTable.getColumn(0);
        const auto& aqutab_piColumn = aqutabTable.getColumn(1);
        this->td = aqutab_tdColumn.vectorCopy();
        this->pi = aqutab_piColumn.vectorCopy();
    } else {
        this->td = default_time;
        this->pi = default_pressure;
    }
}


bool AquiferCT::AQUCT_data::operator==(const AquiferCT::AQUCT_data& other) const {
    return this->aquiferID == other.aquiferID &&
           this->inftableID == other.inftableID &&
           this->pvttableID == other.pvttableID &&
           this->phi_aq == other.phi_aq &&
           this->d0 == other.d0 &&
           this->C_t == other.C_t &&
           this->r_o == other.r_o &&
           this->k_a == other.k_a &&
           this->c1  == other.c1  &&
           this->h   == other.h   &&
           this->theta == other.theta &&
           this->c2 == other.c2 &&
           this->p0 == other.p0 &&
           this->td == other.td &&
           this->pi == other.pi &&
           this->cell_id == other.cell_id;
}

AquiferCT::AQUCT_data::AQUCT_data(int aqID,
                                  int infID,
                                  int pvtID,
                                  double phi_aq_,
                                  double d0_,
                                  double C_t_,
                                  double r_o_,
                                  double k_a_,
                                  double c1_,
                                  double h_,
                                  double theta_,
                                  double c2_,
                                  const std::pair<bool, double>& p0_,
                                  const std::vector<double>& td_,
                                  const std::vector<double>& pi_,
                                  const std::vector<int>& cell_id_) :
    aquiferID(aqID),
    inftableID(infID),
    pvttableID(pvtID),
    phi_aq(phi_aq_),
    d0(d0_),
    C_t(C_t_),
    r_o(r_o_),
    k_a(k_a_),
    c1(c1_),
    h(h_),
    theta(theta_),
    c2(c2_),
    p0(p0_),
    td(td_),
    pi(pi_),
    cell_id(cell_id_)
{}

AquiferCT::AquiferCT(const TableManager& tables, const Deck& deck)
{
    using AQUCT = ParserKeywords::AQUCT;
    if (!deck.hasKeyword<AQUCT>())
        return;

    const auto& aquctKeyword = deck.getKeyword<AQUCT>();
    for (auto& record : aquctKeyword)
        this->m_aquct.emplace_back(record, tables);
}

AquiferCT::AquiferCT(const std::vector<AquiferCT::AQUCT_data>& data) :
    m_aquct(data)
{}

AquiferCT AquiferCT::serializeObject()
{
    AquiferCT result;
    result.m_aquct = {{1, 2, 3, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0,
                       11.0, 12.0, {true, 13.0}, {14.0}, {15.0}, {16}}};

    return result;
}

std::size_t AquiferCT::size() const {
    return this->m_aquct.size();
}

std::vector<AquiferCT::AQUCT_data>::const_iterator AquiferCT::begin() const {
    return this->m_aquct.begin();
}

std::vector<AquiferCT::AQUCT_data>::const_iterator AquiferCT::end() const {
    return this->m_aquct.end();
}

bool AquiferCT::operator==(const AquiferCT& other) const {
    return this->m_aquct == other.m_aquct;
}

const std::vector<AquiferCT::AQUCT_data>& AquiferCT::data() const {
    return this->m_aquct;
}

}
