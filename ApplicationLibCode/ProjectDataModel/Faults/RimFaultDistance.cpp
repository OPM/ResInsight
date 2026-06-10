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

#include "RimFaultDistance.h"

#include "RiaDefines.h"

#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseResultAddress.h"
#include "RigFault.h"
#include "RigSelectedFaultDistanceResultCalculator.h"

#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseView.h"
#include "RimFaultInView.h"
#include "RimFaultInViewCollection.h"

#include "cafCmdFeatureMenuBuilder.h"
#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"
#include "cafPdmUiPushButtonEditor.h"
#include "cafPdmUiTreeSelectionEditor.h"

CAF_PDM_SOURCE_INIT( RimFaultDistance, "RimFaultDistance" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFaultDistance::RimFaultDistance()
{
    CAF_PDM_InitScriptableObjectWithNameAndComment( "Fault Distance",
                                                    ":/draw_style_faults_24x24.png",
                                                    "",
                                                    "",
                                                    "FaultDistance",
                                                    "Per-cell distance to a selected subset of faults" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_resultName, "ResultName", "Name" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_faults, "SelectedFaults", "Faults" );
    m_faults.uiCapability()->setUiEditorTypeName( caf::PdmUiTreeSelectionEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_generateButton, "Generate", "" );
    caf::PdmUiPushButtonEditor::configureEditorLabelLeft( &m_generateButton );

    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
/// Remove the generated result from the case when the object is deleted. The object is still
/// attached to the project tree while this destructor body runs, so the owner case is reachable.
//--------------------------------------------------------------------------------------------------
RimFaultDistance::~RimFaultDistance()
{
    removeGeneratedResult( m_resultName() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimFaultDistance::resultName() const
{
    return m_resultName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultDistance::setResultName( const QString& name )
{
    m_resultName = name;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultDistance::setSelectedFaults( const std::vector<RimFaultInView*>& faults )
{
    m_faults.setValue( faults );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<const RigFault*> RimFaultDistance::selectedRigFaults() const
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
void RimFaultDistance::compute()
{
    if ( m_resultName().isEmpty() ) return;

    RimEclipseCase* eclipseCase = firstAncestorOrThisOfType<RimEclipseCase>();
    if ( !eclipseCase ) return;

    RigEclipseCaseData* caseData = eclipseCase->eclipseCaseData();
    if ( !caseData ) return;

    RigSelectedFaultDistanceResultCalculator::compute( caseData, m_resultName(), selectedRigFaults() );

    const auto views = eclipseCase->reservoirViews();
    if ( !views.empty() && views.front() )
    {
        RimEclipseView* firstView = views.front();

        // Select the generated result in the first view so the computation result is visible.
        if ( RimEclipseCellColors* cellResult = firstView->cellResult() )
        {
            cellResult->setResultType( RiaDefines::ResultCatType::GENERATED );
            cellResult->setResultVariable( m_resultName() );
            cellResult->loadResult();
            cellResult->updateConnectedEditors();
        }

        firstView->scheduleCreateDisplayModelAndRedraw();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultDistance::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( changedField == &m_resultName )
    {
        const QString previousName = oldValue.toString();
        if ( !previousName.isEmpty() && previousName != m_resultName() )
        {
            removeGeneratedResult( previousName );
        }
    }
    else if ( changedField == &m_generateButton )
    {
        m_generateButton = false;
        compute();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimFaultDistance::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_faults )
    {
        RimEclipseCase* eclipseCase     = firstAncestorOrThisOfType<RimEclipseCase>();
        const auto      views           = eclipseCase ? eclipseCase->reservoirViews() : std::vector<RimEclipseView*>();
        auto            faultCollection = ( !views.empty() && views.front() ) ? views.front()->faultCollection() : nullptr;
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
caf::PdmFieldHandle* RimFaultDistance::userDescriptionField()
{
    return &m_resultName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultDistance::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_resultName );
    uiOrdering.add( &m_faults );
    uiOrdering.add( &m_generateButton );
    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultDistance::defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_generateButton )
    {
        if ( auto* attrib = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*>( attribute ) )
        {
            attrib->m_buttonText = "Generate";
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultDistance::removeGeneratedResult( const QString& name )
{
    if ( name.isEmpty() ) return;

    RimEclipseCase* eclipseCase = firstAncestorOrThisOfType<RimEclipseCase>();
    if ( !eclipseCase ) return;

    RigEclipseCaseData* caseData = eclipseCase->eclipseCaseData();
    if ( !caseData ) return;

    RigCaseCellResultsData* resultsData = caseData->results( RiaDefines::PorosityModelType::MATRIX_MODEL );
    if ( !resultsData ) return;

    RigEclipseResultAddress resultAddress( RiaDefines::ResultCatType::GENERATED, name );

    // Select "None" result in any view currently displaying the result being removed.
    for ( RimEclipseView* view : eclipseCase->reservoirViews() )
    {
        if ( !view ) continue;

        RimEclipseCellColors* cellResult = view->cellResult();
        if ( cellResult && cellResult->resultType() == RiaDefines::ResultCatType::GENERATED && cellResult->resultVariable() == name )
        {
            cellResult->setResultVariable( "None" );
            cellResult->updateConnectedEditors();
            view->scheduleCreateDisplayModelAndRedraw();
        }
    }

    resultsData->clearScalarResult( resultAddress );
    resultsData->setRemovedTagOnGeneratedResult( resultAddress );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultDistance::appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const
{
    menuBuilder << "RicNewFaultDistanceResultFeature";
}
