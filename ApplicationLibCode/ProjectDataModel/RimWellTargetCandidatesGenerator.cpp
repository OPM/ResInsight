/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024-  Equinor ASA
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

#include "RimWellTargetCandidatesGenerator.h"

#include "RiaLogging.h"

#include "RiaPorosityModel.h"
#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseResultAddress.h"
#include "RigMainGrid.h"

#include "RimEclipseCase.h"
#include "RimEclipseCaseEnsemble.h"

#include "cafPdmUiPushButtonEditor.h"
#include "cafVecIjk.h"
#include <limits>

CAF_PDM_SOURCE_INIT( RimWellTargetCandidatesGenerator, "RimWellTargetCandidatesGenerator" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellTargetCandidatesGenerator::RimWellTargetCandidatesGenerator()
{
    CAF_PDM_InitObject( "Well Target Candidates Generator" );

    CAF_PDM_InitField( &m_volume, "Volume", 0.0, "Volume" );
    CAF_PDM_InitField( &m_pressure, "Pressure", 0.0, "Pressure" );
    CAF_PDM_InitField( &m_permeability, "Permeability", 0.0, "Permeability" );
    CAF_PDM_InitField( &m_transmissibility, "Transmissibility", 0.0, "Transmissibility" );

    CAF_PDM_InitField( &m_generateButton, "Generate", false, "Generate" );
    m_generateButton.uiCapability()->setUiEditorTypeName( caf::PdmUiPushButtonEditor::uiEditorTypeName() );
    m_generateButton.xmlCapability()->disableIO();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellTargetCandidatesGenerator::~RimWellTargetCandidatesGenerator()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellTargetCandidatesGenerator::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( changedField == &m_generateButton )
    {
        m_generateButton = false;

        generateCandidates();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellTargetCandidatesGenerator::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                              QString                    uiConfigName,
                                                              caf::PdmUiEditorAttribute* attribute )

{
    if ( field == &m_generateButton )
    {
        auto myAttr          = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*>( attribute );
        myAttr->m_buttonText = "Generate";
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellTargetCandidatesGenerator::generateCandidates()
{
    auto ensemble = firstAncestorOrThisOfType<RimEclipseCaseEnsemble>();
    if ( !ensemble ) return;

    if ( ensemble->cases().empty() ) return;

    RimEclipseCase* eclipseCase = ensemble->cases().front();

    std::optional<caf::VecIjk> startCell = findStartCell( eclipseCase );

    if ( startCell.has_value() )
    {
        RiaLogging::info(
            QString( "Found start cell: [%1 %2 %3]" ).arg( startCell->i() + 1 ).arg( startCell->j() + 1 ).arg( startCell->k() + 1 ) );
    }
    else
    {
        RiaLogging::error( "No suitable starting cell found" );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::optional<caf::VecIjk> RimWellTargetCandidatesGenerator::findStartCell( RimEclipseCase* eclipseCase )
{
    // TODO: let user choose timestep
    size_t timeStepIdx = 0;

    auto resultsData = eclipseCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL );
    if ( !resultsData )
    {
        RiaLogging::error( "No results data found for eclipse case" );
        return {};
    }

    RigEclipseResultAddress soilAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, RiaResultNames::riPorvSoil() );

    resultsData->ensureKnownResultLoaded( soilAddress );
    const std::vector<double>& porvSoil = resultsData->cellScalarResults( soilAddress, timeStepIdx );

    size_t startCell         = std::numeric_limits<size_t>::max();
    double maxValue          = -std::numeric_limits<double>::max();
    size_t numReservoirCells = resultsData->activeCellInfo()->reservoirCellCount();
    for ( size_t reservoirCellIdx = 0; reservoirCellIdx < numReservoirCells; reservoirCellIdx++ )
    {
        size_t resultIndex = resultsData->activeCellInfo()->cellResultIndex( reservoirCellIdx );
        if ( resultIndex != cvf::UNDEFINED_SIZE_T )
        {
            double value = porvSoil[resultIndex];
            if ( value > maxValue )
            {
                maxValue  = value;
                startCell = reservoirCellIdx;
            }
        }
    }

    return eclipseCase->mainGrid()->ijkFromCellIndex( startCell );
}
