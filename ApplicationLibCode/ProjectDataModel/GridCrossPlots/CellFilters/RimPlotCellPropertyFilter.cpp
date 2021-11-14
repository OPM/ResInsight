/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019- Equinor ASA
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

#include "RimPlotCellPropertyFilter.h"

#include "RiaLogging.h"

#include "RigActiveCellInfo.h"
#include "RigResultAccessor.h"
#include "RigResultAccessorFactory.h"

#include "RimEclipseCase.h"
#include "RimEclipseResultDefinition.h"
#include "RimGeoMechResultDefinition.h"

#include "cafPdmUiDoubleSliderEditor.h"

#include <algorithm>

CAF_PDM_SOURCE_INIT( RimPlotCellPropertyFilter, "RimPlotCellPropertyFilter" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPlotCellPropertyFilter::RimPlotCellPropertyFilter()
{
    CAF_PDM_InitObject( "Plot Cell Property Filter" );

    CAF_PDM_InitFieldNoDefault( &m_resultDefinition, "ResultDefinition", "Result Definition" );

    // Set to hidden to avoid this item to been displayed as a child item
    // Fields in this object are displayed using defineUiOrdering()
    m_resultDefinition.uiCapability()->setUiTreeHidden( true );
    m_resultDefinition.uiCapability()->setUiTreeChildrenHidden( true );

    CAF_PDM_InitField( &m_lowerBound, "LowerBound", 0.0, "Min" );
    m_lowerBound.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_upperBound, "UpperBound", 0.0, "Max" );
    m_upperBound.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCellPropertyFilter::setResultDefinition( caf::PdmObject* resultDefinition )
{
    m_resultDefinition = resultDefinition;

    updateName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCellPropertyFilter::setValueRange( double lowerBound, double upperBound )
{
    m_lowerBound = lowerBound;
    m_upperBound = upperBound;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCellPropertyFilter::updatePointerAfterCopy( RimPlotCellPropertyFilter* other )
{
    if ( eclipseResultDefinition() && eclipseResultDefinition()->eclipseCase() )
    {
        auto eclipseCase = eclipseResultDefinition()->eclipseCase();

        other->eclipseResultDefinition()->setEclipseCase( eclipseCase );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseResultDefinition* RimPlotCellPropertyFilter::eclipseResultDefinition()
{
    caf::PdmObject* pdmObj = m_resultDefinition;

    return dynamic_cast<RimEclipseResultDefinition*>( pdmObj );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCellPropertyFilter::findOrComputeMinMaxResultValues( double& minimumValue, double& maximumValue )
{
    RimEclipseResultDefinition* resDef = eclipseResultDefinition();
    if ( resDef )
    {
        RimEclipseCase* eclCase = resDef->eclipseCase();
        if ( !eclCase ) return;

        RigEclipseCaseData* eclipseCaseData = eclCase->eclipseCaseData();
        if ( !eclipseCaseData ) return;

        resDef->loadResult();

        RigCaseCellResultsData* cellResultsData = resDef->currentGridCellResults();
        if ( !cellResultsData ) return;

        cellResultsData->minMaxCellScalarValues( resDef->eclipseResultAddress(), minimumValue, maximumValue );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCellPropertyFilter::updateName()
{
    QString name = "Property filter - ";

    RimEclipseResultDefinition* resDef = eclipseResultDefinition();
    if ( resDef )
    {
        name += resDef->resultVariableUiName();
    }

    setName( name );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCellPropertyFilter::updateCellVisibilityFromFilter( size_t timeStepIndex, cvf::UByteArray* visibleCells )
{
    CVF_ASSERT( visibleCells );

    RimEclipseResultDefinition* resDef = eclipseResultDefinition();
    if ( resDef )
    {
        RimEclipseCase* eclCase = resDef->eclipseCase();
        if ( !eclCase ) return;

        RigEclipseCaseData* eclipseCaseData = eclCase->eclipseCaseData();
        if ( !eclipseCaseData ) return;

        resDef->loadResult();

        RigCaseCellResultsData* cellResultsData = resDef->currentGridCellResults();
        if ( !cellResultsData ) return;

        RigEclipseResultAddress rigEclipseAddress = resDef->eclipseResultAddress();

        if ( !resDef->currentGridCellResults()->hasResultEntry( rigEclipseAddress ) ) return;

        size_t clampedIndex = std::min( timeStepIndex, cellResultsData->timeStepCount( rigEclipseAddress ) - 1 );
        const std::vector<double>& cellResultValues = cellResultsData->cellScalarResults( rigEclipseAddress, clampedIndex );
        if ( cellResultValues.empty() ) return;

        const RigActiveCellInfo* actCellInfo             = cellResultsData->activeCellInfo();
        size_t                   totalReservoirCellCount = actCellInfo->reservoirCellCount();

        if ( visibleCells->size() < totalReservoirCellCount )
        {
            QString message = QString( "Size of visible Cells (%1) is less than total cell count (%2)" )
                                  .arg( visibleCells->size() )
                                  .arg( totalReservoirCellCount );

            RiaLogging::error( message );

            return;
        }

        bool           isUsingGlobalActiveIndex = cellResultsData->isUsingGlobalActiveIndex( rigEclipseAddress );
        double         lowerBound               = m_lowerBound;
        double         upperBound               = m_upperBound;
        size_t         cellResultIndex          = 0;
        double         scalarValue              = 0.0;
        FilterModeType currentFilterMode        = filterMode();

        for ( size_t reservoirCellIndex = 0; reservoirCellIndex < totalReservoirCellCount; ++reservoirCellIndex )
        {
            if ( !actCellInfo->isActive( reservoirCellIndex ) ) continue;

            cellResultIndex = reservoirCellIndex;
            if ( isUsingGlobalActiveIndex )
            {
                cellResultIndex = actCellInfo->cellResultIndex( reservoirCellIndex );
            }

            if ( cellResultIndex != cvf::UNDEFINED_SIZE_T && cellResultIndex < cellResultValues.size() )
            {
                if ( ( *visibleCells )[reservoirCellIndex] )
                {
                    scalarValue = cellResultValues[cellResultIndex];
                    if ( lowerBound <= scalarValue && scalarValue <= upperBound )
                    {
                        if ( currentFilterMode == RimPlotCellFilter::EXCLUDE )
                        {
                            ( *visibleCells )[reservoirCellIndex] = false;
                        }
                    }
                    else
                    {
                        if ( currentFilterMode == RimPlotCellFilter::INCLUDE )
                        {
                            ( *visibleCells )[reservoirCellIndex] = false;
                        }
                    }
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCellPropertyFilter::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                       QString                    uiConfigName,
                                                       caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_lowerBound || field == &m_upperBound )
    {
        caf::PdmUiDoubleSliderEditorAttribute* myAttr = dynamic_cast<caf::PdmUiDoubleSliderEditorAttribute*>( attribute );
        if ( !myAttr )
        {
            return;
        }

        double minimumValue = 0.0;
        double maximumValue = 0.0;

        findOrComputeMinMaxResultValues( minimumValue, maximumValue );

        myAttr->m_minimum = minimumValue;
        myAttr->m_maximum = maximumValue;
    }
}
