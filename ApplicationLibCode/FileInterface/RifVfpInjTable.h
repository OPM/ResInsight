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
//  RifVfpInjTable - VFP Injection Table Reader
//
//  This class is based on the OPM VFPInjTable implementation found at:
//  ThirdParty/custom-opm-common/opm-common/opm/input/eclipse/Schedule/VFPInjTable.hpp
//
//  RATIONALE:
//  This ResInsight-specific implementation is required because the OPM VFPInjTable class
//  automatically converts all data values to SI units during import. For ResInsight's use cases,
//  we need access to the original keyword values as they appear in the Eclipse deck file
//  without any unit conversions applied.
//
//  KEY DIFFERENCES FROM OPM VERSION:
//  - Removed all unit conversion functions (convertFloToSI, convertTHPToSI)
//  - Datum depth stored as raw value (not converted to SI)
//  - All axis data (FLO, THP) preserved in original units
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

class RifVfpInjTable
{
public:
    enum class FLO_TYPE
    {
        FLO_OIL = 1,
        FLO_WAT,
        FLO_GAS,
    };

    RifVfpInjTable();
    RifVfpInjTable( const Opm::DeckKeyword& table );

    int                tableNum() const;
    double             datumDepth() const;
    FLO_TYPE           floType() const;
    const std::string& units() const;
    const std::string& bodyDef() const;

    const std::vector<double>& floAxis() const;
    const std::vector<double>& thpAxis() const;
    const std::vector<double>& table() const;

    std::array<size_t, 2> shape() const;
    double                operator()( size_t thpIdx, size_t floIdx ) const;

    static void checkAxis( const std::vector<double>& axis );

private:
    int         m_tableNum;
    double      m_datumDepth;
    FLO_TYPE    m_floType;
    std::string m_units;
    std::string m_bodyDef;

    std::vector<double> m_floData;
    std::vector<double> m_thpData;
    std::vector<double> m_data;

    void    check();
    double& operator()( size_t thpIdx, size_t floIdx );
};