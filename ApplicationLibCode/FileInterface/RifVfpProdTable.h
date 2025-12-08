/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025-     Equinor ASA
//
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
//
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

//==================================================================================================
//
//  RifVfpProdTable - VFP Production Table Reader
//
//  This class is based on the OPM VFPProdTable implementation found at:
//  ThirdParty/custom-opm-common/opm-common/opm/input/eclipse/Schedule/VFPProdTable.hpp
//
//  RATIONALE:
//  This ResInsight-specific implementation is required because the OPM VFPProdTable class
//  automatically converts all data values to SI units during import. For ResInsight's use cases,
//  we need access to the original keyword values as they appear in the Eclipse deck file
//  without any unit conversions applied.
//
//  KEY DIFFERENCES FROM OPM VERSION:
//  - Removed all unit conversion functions (convertFloToSI, convertTHPToSI, etc.)
//  - Datum depth stored as raw value (not converted to SI)
//  - All axis data (FLO, THP, WFR, GFR, ALQ) preserved in original units
//  - Table data (BHP values) stored without pressure scaling
//  - Added storage for original UNITS and BODY_DEF strings for reference
//
//==================================================================================================

#pragma once

#include <array>
#include <string>
#include <vector>

namespace Opm
{
class DeckKeyword;
}

class RifVfpProdTable
{
public:
    enum class FLO_TYPE
    {
        FLO_OIL = 1,
        FLO_LIQ,
        FLO_GAS
    };

    enum class WFR_TYPE
    {
        WFR_WOR = 11,
        WFR_WCT,
        WFR_WGR
    };

    enum class GFR_TYPE
    {
        GFR_GOR = 21,
        GFR_GLR,
        GFR_OGR
    };

    enum class ALQ_TYPE
    {
        ALQ_GRAT = 31,
        ALQ_IGLR,
        ALQ_TGLR,
        ALQ_PUMP,
        ALQ_COMP,
        ALQ_BEAN,
        ALQ_UNDEF
    };

    RifVfpProdTable();
    RifVfpProdTable( const Opm::DeckKeyword& table, bool gasliftOptActive );

    int                tableNum() const;
    double             datumDepth() const;
    FLO_TYPE           floType() const;
    WFR_TYPE           wfrType() const;
    GFR_TYPE           gfrType() const;
    ALQ_TYPE           alqType() const;
    const std::string& units() const;
    const std::string& bodyDef() const;

    const std::vector<double>& floAxis() const;
    const std::vector<double>& thpAxis() const;
    const std::vector<double>& wfrAxis() const;
    const std::vector<double>& gfrAxis() const;
    const std::vector<double>& alqAxis() const;
    const std::vector<double>& table() const;

    std::array<size_t, 5> shape() const;
    double                operator()( size_t thpIdx, size_t wfrIdx, size_t gfrIdx, size_t alqIdx, size_t floIdx ) const;

    static void checkAxis( const std::vector<double>& axis );

private:
    int         m_tableNum;
    double      m_datumDepth;
    FLO_TYPE    m_floType;
    WFR_TYPE    m_wfrType;
    GFR_TYPE    m_gfrType;
    ALQ_TYPE    m_alqType;
    std::string m_units;
    std::string m_bodyDef;

    std::vector<double> m_floData;
    std::vector<double> m_thpData;
    std::vector<double> m_wfrData;
    std::vector<double> m_gfrData;
    std::vector<double> m_alqData;
    std::vector<double> m_data;

    void    check();
    double& operator()( size_t thpIdx, size_t wfrIdx, size_t gfrIdx, size_t alqIdx, size_t floIdx );
};