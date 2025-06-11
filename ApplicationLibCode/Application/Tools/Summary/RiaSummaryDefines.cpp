/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022 Equinor ASA
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

#include "RiaSummaryDefines.h"

#include "cafAppEnum.h"

namespace caf
{
template <>
void caf::AppEnum<RiaDefines::HorizontalAxisType>::setUp()
{
    addItem( RiaDefines::HorizontalAxisType::TIME, "TIME", "Time" );
    addItem( RiaDefines::HorizontalAxisType::SUMMARY_VECTOR, "SUMMARY_VECTOR", "Summary Vector" );
    setDefault( RiaDefines::HorizontalAxisType::SUMMARY_VECTOR );
}

template <>
void caf::AppEnum<RiaDefines::SummaryCurveTypeMode>::setUp()
{
    addItem( RiaDefines::SummaryCurveTypeMode::AUTO, "AUTO", "Auto" );
    addItem( RiaDefines::SummaryCurveTypeMode::CUSTOM, "CUSTOM", "Custom" );
    setDefault( RiaDefines::SummaryCurveTypeMode::AUTO );
}

template <>
void caf::AppEnum<RiaDefines::EnsembleGroupingMode>::setUp()
{
    addItem( RiaDefines::EnsembleGroupingMode::FMU_FOLDER_STRUCTURE, "FMU_FOLDER_MODE", "Sub Folder" );
    addItem( RiaDefines::EnsembleGroupingMode::EVEREST_FOLDER_STRUCTURE, "EVEREST_FOLDER_MODE", "Main Folder" );
    addItem( RiaDefines::EnsembleGroupingMode::NONE, "None", "None" );
    addItem( RiaDefines::EnsembleGroupingMode::RESINSIGHT_OPMFLOW_STRUCTURE, "RESINSIGHTOPM_FOLDER_MODE", "ResInsight Opm Flow" );
    setDefault( RiaDefines::EnsembleGroupingMode::FMU_FOLDER_STRUCTURE );
}

} // namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaDefines::summaryField()
{
    return "Field";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaDefines::summaryAquifer()
{
    return "Aquifer";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaDefines::summaryNetwork()
{
    return "Network";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaDefines::summaryMisc()
{
    return "Misc";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaDefines::summaryRegion()
{
    return "Region";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaDefines::summaryRegion2Region()
{
    return "Region-Region";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaDefines::summaryWell()
{
    return "Well";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaDefines::summaryWellCompletion()
{
    return "Completion";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaDefines::summaryWellGroup()
{
    return "Group";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaDefines::summaryWellSegment()
{
    return "Segment";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaDefines::summaryWellConnection()
{
    return "Connection";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaDefines::summarySegment()
{
    return "Segment";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaDefines::summaryBlock()
{
    return "Block";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaDefines::summaryLgrConnection()
{
    return "LGR Connection";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaDefines::summaryLgrWell()
{
    return "LGR Well";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaDefines::summaryLgrBlock()
{
    return "LGR Block";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaDefines::summaryCalculated()
{
    return "Calculated";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaDefines::summaryRealizationNumber()
{
    return "RI:REALIZATION_NUM";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaDefines::key1VariableName()
{
    return "$KEY1";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaDefines::key2VariableName()
{
    return "$KEY2";
}
