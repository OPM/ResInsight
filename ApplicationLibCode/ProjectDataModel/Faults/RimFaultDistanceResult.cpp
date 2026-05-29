/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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

#include "RimFaultDistanceResult.h"

#include "RiaDefines.h"

#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseResultAddress.h"
#include "RigFault.h"
#include "RigSelectedFaultDistanceResultCalculator.h"

#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimFaultInView.h"
#include "RimFaultInViewCollection.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"
#include "cafPdmUiTreeSelectionEditor.h"

CAF_PDM_SOURCE_INIT( RimFaultDistanceResult, "RimFaultDistanceResult" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFaultDistanceResult::RimFaultDistanceResult()
{
    CAF_PDM_InitScriptableObjectWithNameAndComment( "Fault Distance Result",
                                                    ":/draw_style_faults_24x24.png",
                                                    "",
                                                    "",
                                                    "FaultDistanceResult",
                                                    "Per-cell distance to a selected subset of faults" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_resultName, "ResultName", "Name" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_faults, "SelectedFaults", "Faults" );
    m_faults.uiCapability()->setUiEditorTypeName( caf::PdmUiTreeSelectionEditor::uiEditorTypeName() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimFaultDistanceResult::resultName() const
{
    return m_resultName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultDistanceResult::setResultName( const QString& name )
{
    m_resultName = name;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultDistanceResult::setSelectedFaults( const std::vector<RimFaultInView*>& faults )
{
    m_faults.setValue( faults );
    compute();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<const RigFault*> RimFaultDistanceResult::selectedRigFaults() const
{
    std::vector<const RigFault*> rigFaults;
    for ( RimFaultInView* fault : m_faults )
    {
        if ( fault && fault->faultGeometry() ) rigFaults.push_back( fault->faultGeometry() );
    }
    return rigFaults;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultDistanceResult::compute()
{
    if ( m_resultName().isEmpty() ) return;

    auto eclipseView = firstAncestorOrThisOfType<RimEclipseView>();
    if ( !eclipseView ) return;

    RimEclipseCase* eclipseCase = eclipseView->eclipseCase();
    if ( !eclipseCase ) return;

    RigEclipseCaseData* caseData = eclipseCase->eclipseCaseData();
    if ( !caseData ) return;

    RigSelectedFaultDistanceResultCalculator::compute( caseData, m_resultName(), selectedRigFaults() );

    eclipseView->scheduleCreateDisplayModelAndRedraw();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultDistanceResult::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( changedField == &m_resultName )
    {
        const QString previousName = oldValue.toString();
        if ( !previousName.isEmpty() && previousName != m_resultName() )
        {
            removeGeneratedResult( previousName );
        }
        compute();
    }
    else if ( changedField == &m_faults )
    {
        compute();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimFaultDistanceResult::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_faults )
    {
        auto eclipseView     = firstAncestorOrThisOfType<RimEclipseView>();
        auto faultCollection = eclipseView ? eclipseView->faultCollection() : nullptr;
        if ( faultCollection )
        {
            for ( RimFaultInView* fault : faultCollection->faults() )
            {
                if ( fault ) options.push_back( caf::PdmOptionItemInfo( fault->name(), fault ) );
            }
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimFaultDistanceResult::userDescriptionField()
{
    return &m_resultName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultDistanceResult::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_resultName );
    uiOrdering.add( &m_faults );
    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultDistanceResult::removeGeneratedResult( const QString& name )
{
    if ( name.isEmpty() ) return;

    auto eclipseView = firstAncestorOrThisOfType<RimEclipseView>();
    if ( !eclipseView ) return;

    RimEclipseCase* eclipseCase = eclipseView->eclipseCase();
    if ( !eclipseCase ) return;

    RigEclipseCaseData* caseData = eclipseCase->eclipseCaseData();
    if ( !caseData ) return;

    RigCaseCellResultsData* resultsData = caseData->results( RiaDefines::PorosityModelType::MATRIX_MODEL );
    if ( !resultsData ) return;

    resultsData->clearScalarResult( RiaDefines::ResultCatType::GENERATED, name );
}
