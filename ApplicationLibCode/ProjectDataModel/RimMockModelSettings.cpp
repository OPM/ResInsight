/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
//  Copyright (C) Ceetron Solutions AS
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

#include "RimMockModelSettings.h"

CAF_PDM_SOURCE_INIT( RimMockModelSettings, "MockModelSettings" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimMockModelSettings::RimMockModelSettings()
{
    CAF_PDM_InitObject( "Mock Model Settings" );

    CAF_PDM_InitField( &cellCountX, "CellCountX", quint64( 100 ), "Cell Count X" );
    CAF_PDM_InitField( &cellCountY, "CellCountY", quint64( 100 ), "Cell Count Y" );
    CAF_PDM_InitField( &cellCountZ, "CellCountZ", quint64( 10 ), "Cell Count Z" );

    CAF_PDM_InitFieldNoDefault( &totalCellCount, "TotalCellCount", "Total Cell Count" );
    totalCellCount.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitField( &extentX, "ExtentX", 6000.0, "Extent X" );
    extentX.setMinValue( 0.0 );
    CAF_PDM_InitField( &extentY, "ExtentY", 12000.0, "Extent Y" );
    extentY.setMinValue( 0.0 );
    CAF_PDM_InitField( &extentZ, "ExtentZ", 500.0, "Extent Z" );
    extentZ.setMinValue( 0.0 );

    CAF_PDM_InitField( &offsetX, "OffsetX", 400000.0, "Offset X" );
    CAF_PDM_InitField( &offsetY, "OffsetY", 6000000.0, "Offset Y" );

    CAF_PDM_InitField( &resultCount, "ResultCount", quint64( 3 ), "Result Count" );
    CAF_PDM_InitField( &timeStepCount, "TimeStepCount", quint64( 10 ), "Time Step Count" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimMockModelSettings::~RimMockModelSettings()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMockModelSettings::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    totalCellCount = cellCountX * cellCountY * cellCountZ;

    caf::PdmUiFieldHandle* uiFieldHandle = totalCellCount.uiCapability();
    if ( uiFieldHandle )
    {
        uiFieldHandle->updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMockModelSettings::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    caf::PdmUiGroup* gridSizeGroup = uiOrdering.addNewGroup( "Grid size" );
    gridSizeGroup->add( &cellCountX );
    gridSizeGroup->add( &cellCountY );
    gridSizeGroup->add( &cellCountZ );
    gridSizeGroup->add( &totalCellCount );

    caf::PdmUiGroup* extentGroup = uiOrdering.addNewGroup( "Extent" );
    extentGroup->add( &extentX );
    extentGroup->add( &extentY );
    extentGroup->add( &extentZ );

    caf::PdmUiGroup* offsetGroup = uiOrdering.addNewGroup( "Offset" );
    offsetGroup->add( &offsetX );
    offsetGroup->add( &offsetY );

    caf::PdmUiGroup* resultGroup = uiOrdering.addNewGroup( "Results" );
    resultGroup->add( &resultCount );
    resultGroup->add( &timeStepCount );
}
