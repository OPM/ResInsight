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

// #include "RimEclipseCase.h"
// #include "RimEclipseCellColors.h"
// #include "RimEclipseView.h"
// #include "RimFaultInViewCollection.h"

// #include "RiuMainWindow.h"

// #include "cafPdmUiTreeOrdering.h"

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
// void RimWellTargetCandidatesGenerator::setReservoirView( RimEclipseView* ownerReservoirView )
// {
//     m_reservoirView = ownerReservoirView;
//     m_customFaultResultColors->setReservoirView( ownerReservoirView );
// }

// //--------------------------------------------------------------------------------------------------
// ///
// //--------------------------------------------------------------------------------------------------
// void RimWellTargetCandidatesGenerator::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const
// QVariant& newValue )
// {
//     updateUiIconFromToggleField();

//     if ( m_reservoirView ) m_reservoirView->scheduleCreateDisplayModelAndRedraw();
// }

// //--------------------------------------------------------------------------------------------------
// ///
// //--------------------------------------------------------------------------------------------------
// void RimWellTargetCandidatesGenerator::initAfterRead()
// {
//     m_customFaultResultColors->initAfterRead();

//     updateUiIconFromToggleField();
// }

// //--------------------------------------------------------------------------------------------------
// ///
// //--------------------------------------------------------------------------------------------------
// RimEclipseCellColors* RimWellTargetCandidatesGenerator::customFaultResult()
// {
//     return m_customFaultResultColors();
// }

// //--------------------------------------------------------------------------------------------------
// ///
// //--------------------------------------------------------------------------------------------------
// void RimWellTargetCandidatesGenerator::updateUiFieldsFromActiveResult()
// {
//     m_customFaultResultColors->updateUiFieldsFromActiveResult();
// }

// //--------------------------------------------------------------------------------------------------
// ///
// //--------------------------------------------------------------------------------------------------
// caf::PdmFieldHandle* RimWellTargetCandidatesGenerator::objectToggleField()
// {
//     return &showCustomFaultResult;
// }

// //--------------------------------------------------------------------------------------------------
// ///
// //--------------------------------------------------------------------------------------------------
// void RimWellTargetCandidatesGenerator::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
// {
//     caf::PdmUiGroup* group1 = uiOrdering.addNewGroup( "Result" );
//     m_customFaultResultColors->uiOrdering( uiConfigName, *group1 );
//     auto eclipseView = firstAncestorOfType<RimEclipseView>();
//     if ( eclipseView )
//     {
//         eclipseView->faultCollection()->uiOrderingFaults( uiConfigName, uiOrdering );
//     }
// }

// //--------------------------------------------------------------------------------------------------
// ///
// //--------------------------------------------------------------------------------------------------
// bool RimWellTargetCandidatesGenerator::hasValidCustomResult()
// {
//     if ( showCustomFaultResult() )
//     {
//         if ( m_customFaultResultColors->hasResult() || m_customFaultResultColors->isTernarySaturationSelected() )
//         {
//             return true;
//         }
//     }

//     return false;
// }

// //--------------------------------------------------------------------------------------------------
// ///
// //--------------------------------------------------------------------------------------------------
// void RimWellTargetCandidatesGenerator::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/ )
// {
//     m_customFaultResultColors()->defineUiTreeOrdering( uiTreeOrdering, uiConfigName );
// }
