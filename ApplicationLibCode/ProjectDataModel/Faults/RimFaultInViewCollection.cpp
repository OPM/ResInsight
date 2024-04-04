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

#include "RimFaultInViewCollection.h"

#include "RiaColorTables.h"
#include "RiaDefines.h"
#include "RiaPreferences.h"
#include "RiaResultNames.h"

#include "RigMainGrid.h"

#include "RimEclipseCase.h"
#include "RimEclipseFaultColors.h"
#include "RimEclipseInputCase.h"
#include "RimEclipseView.h"
#include "RimFaultInView.h"
#include "RimIntersectionCollection.h"
#include "RimProject.h"

#include "RiuMainWindow.h"

#include "cafAppEnum.h"
#include "cafPdmFieldCvfColor.h"
#include "cafPdmFieldCvfMat4d.h"
#include "cafPdmUiCheckBoxEditor.h"
#include "cafPdmUiTreeOrdering.h"

namespace caf
{
template <>
void AppEnum<RimFaultInViewCollection::FaultFaceCullingMode>::setUp()
{
    addItem( RimFaultInViewCollection::FAULT_BACK_FACE_CULLING, "FAULT_BACK_FACE_CULLING", "Cell behind fault" );
    addItem( RimFaultInViewCollection::FAULT_FRONT_FACE_CULLING, "FAULT_FRONT_FACE_CULLING", "Cell in front of fault" );
    addItem( RimFaultInViewCollection::FAULT_NO_FACE_CULLING, "FAULT_NO_FACE_CULLING", "Show both" );
    setDefault( RimFaultInViewCollection::FAULT_NO_FACE_CULLING );
}
} // namespace caf

CAF_PDM_SOURCE_INIT( RimFaultInViewCollection, "Faults" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFaultInViewCollection::RimFaultInViewCollection()
{
    CAF_PDM_InitObject( "Faults", ":/draw_style_faults_24x24.png" );

    CAF_PDM_InitField( &m_showFaultCollection, "Active", true, "Active" );
    m_showFaultCollection.uiCapability()->setUiHidden( true );

    CAF_PDM_InitField( &m_showFaultFaces, "ShowFaultFaces", true, "Show Defined faces" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_showFaultFaces );

    CAF_PDM_InitField( &m_showOppositeFaultFaces, "ShowOppositeFaultFaces", true, "Show Opposite Faces" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_showOppositeFaultFaces );

    CAF_PDM_InitField( &m_applyCellFilters, "ApplyCellFilters", true, "Use Cell Filters for Faults" );

    CAF_PDM_InitField( &m_onlyShowWithNeighbor, "OnlyShowWithDefNeighbor", false, "Show Only Faces with Juxtaposition" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_onlyShowWithNeighbor );

    CAF_PDM_InitField( &m_faultResult,
                       "FaultFaceCulling",
                       caf::AppEnum<RimFaultInViewCollection::FaultFaceCullingMode>( RimFaultInViewCollection::FAULT_BACK_FACE_CULLING ),
                       "Dynamic Face Selection" );

    CAF_PDM_InitField( &m_showFaultLabel, "ShowFaultLabel", false, "Show Labels" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_showFaultLabel );

    cvf::Color3f defWellLabelColor = RiaPreferences::current()->defaultWellLabelColor();
    CAF_PDM_InitField( &m_faultLabelColor, "FaultLabelColor", defWellLabelColor, "Label Color" );

    CAF_PDM_InitField( &m_showNNCs, "ShowNNCs", true, "Show NNCs" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_showNNCs );

    CAF_PDM_InitField( &m_hideNNCsWhenNoResultIsAvailable,
                       "HideNncsWhenNoResultIsAvailable",
                       true,
                       "Hide NNC Geometry if No NNC Result is Available" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_hideNNCsWhenNoResultIsAvailable );

    CAF_PDM_InitFieldNoDefault( &m_faults, "Faults", "Faults" );

    CAF_PDM_InitField( &m_showFaultsOutsideFilters_obsolete, "ShowFaultsOutsideFilters", true, "Show Faults Outside Filters" );
    m_showFaultsOutsideFilters_obsolete.xmlCapability()->setIOWritable( false );
    m_showFaultsOutsideFilters_obsolete.uiCapability()->setUiHidden( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFaultInViewCollection::~RimFaultInViewCollection()
{
    m_faults.deleteChildren();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimFaultInViewCollection::isActive() const
{
    return m_showFaultCollection;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultInViewCollection::setActive( bool bActive )
{
    m_showFaultCollection = bActive;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Color3f RimFaultInViewCollection::faultLabelColor() const
{
    return m_faultLabelColor();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::AppEnum<RimFaultInViewCollection::FaultFaceCullingMode> RimFaultInViewCollection::faultResult() const
{
    return m_faultResult();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimFaultInViewCollection::showFaultFaces() const
{
    return m_showFaultFaces();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimFaultInViewCollection::showFaultLabel() const
{
    return m_showFaultLabel();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimFaultInViewCollection::showOppositeFaultFaces() const
{
    return m_showOppositeFaultFaces();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimFaultInViewCollection::showNNCs() const
{
    return m_showOppositeFaultFaces();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimFaultInViewCollection::hideNNCsWhenNoResultIsAvailable() const
{
    return m_hideNNCsWhenNoResultIsAvailable();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultInViewCollection::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    updateUiIconFromToggleField();

    if ( &m_faultLabelColor == changedField )
    {
        parentView()->scheduleReservoirGridGeometryRegen();
        parentView()->intersectionCollection()->scheduleCreateDisplayModelAndRedraw2dIntersectionViews();
    }

    if ( changedField == &m_onlyShowWithNeighbor )
    {
        parentView()->scheduleReservoirGridGeometryRegen();
    }

    if ( &m_showFaultFaces == changedField || &m_showOppositeFaultFaces == changedField || &m_showFaultCollection == changedField ||
         &m_showFaultLabel == changedField || &m_applyCellFilters == changedField || &m_faultLabelColor == changedField ||
         &m_onlyShowWithNeighbor == changedField || &m_faultResult == changedField || &m_showNNCs == changedField ||
         &m_hideNNCsWhenNoResultIsAvailable == changedField )
    {
        parentView()->scheduleCreateDisplayModelAndRedraw();
        parentView()->intersectionCollection()->scheduleCreateDisplayModelAndRedraw2dIntersectionViews();
    }

    if ( &m_showFaultLabel == changedField )
    {
        RiuMainWindow::instance()->refreshDrawStyleActions();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimFaultInViewCollection::objectToggleField()
{
    return &m_showFaultCollection;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFaultInView* RimFaultInViewCollection::findFaultByName( QString name )
{
    for ( auto& fault : m_faults )
    {
        if ( fault->name() == name ) return fault;
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// A comparing function used to sort Faults in the RimFaultCollection::synchronizeFaults() method
//--------------------------------------------------------------------------------------------------
bool faultComparator( const cvf::ref<RigFault>& a, const cvf::ref<RigFault>& b )
{
    CVF_TIGHT_ASSERT( a.notNull() && b.notNull() );

    int compareValue = a->name().compare( b->name(), Qt::CaseInsensitive );

    return ( compareValue < 0 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultInViewCollection::synchronizeFaults()
{
    if ( !( parentView() && parentView()->mainGrid() ) ) return;

    const caf::ColorTable& colorTable = RiaColorTables::faultsPaletteColors();

    const cvf::Collection<RigFault> constRigFaults = parentView()->mainGrid()->faults();

    cvf::Collection<RigFault> rigFaults;
    {
        cvf::Collection<RigFault> sortedFaults( constRigFaults );

        std::sort( sortedFaults.begin(), sortedFaults.end(), faultComparator );

        cvf::ref<RigFault> undefinedFaults;
        cvf::ref<RigFault> undefinedFaultsWInactive;

        for ( size_t i = 0; i < sortedFaults.size(); i++ )
        {
            QString faultName = sortedFaults[i]->name();
            if ( faultName.compare( RiaResultNames::undefinedGridFaultName(), Qt::CaseInsensitive ) == 0 )
            {
                undefinedFaults = sortedFaults[i];
            }

            if ( faultName.startsWith( RiaResultNames::undefinedGridFaultName(), Qt::CaseInsensitive ) && faultName.contains( "Inactive" ) )
            {
                undefinedFaultsWInactive = sortedFaults[i];
            }
        }

        if ( undefinedFaults.notNull() )
        {
            sortedFaults.erase( undefinedFaults.p() );
            rigFaults.push_back( undefinedFaults.p() );
        }

        if ( undefinedFaultsWInactive.notNull() )
        {
            sortedFaults.erase( undefinedFaultsWInactive.p() );
            rigFaults.push_back( undefinedFaultsWInactive.p() );
        }

        for ( size_t i = 0; i < sortedFaults.size(); i++ )
        {
            rigFaults.push_back( sortedFaults[i].p() );
        }
    }

    std::vector<caf::PdmPointer<RimFaultInView>> newFaults;

    // Find corresponding fault from data model, or create a new
    for ( size_t fIdx = 0; fIdx < rigFaults.size(); ++fIdx )
    {
        RimFaultInView* rimFault = findFaultByName( rigFaults[fIdx]->name() );

        if ( !rimFault )
        {
            rimFault             = new RimFaultInView();
            rimFault->faultColor = colorTable.cycledColor3f( fIdx );
            QString faultName    = rigFaults[fIdx]->name();

            if ( faultName.startsWith( RiaResultNames::undefinedGridFaultName(), Qt::CaseInsensitive ) && faultName.contains( "Inactive" ) )
            {
                rimFault->showFault = false; // Turn fault against inactive cells off by default
            }
        }

        rimFault->setFaultGeometry( rigFaults[fIdx].p() );

        newFaults.push_back( rimFault );
    }

    m_faults().clearWithoutDelete();
    m_faults().insert( 0, newFaults );

    QString toolTip = QString( "Fault count (%1)" ).arg( newFaults.size() );
    setUiToolTip( toolTip );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimFaultInViewCollection::isGridVisualizationMode() const
{
    return parentView()->isGridVisualizationMode();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultInViewCollection::uiOrderingFaults( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    bool isGridVizMode = isGridVisualizationMode();

    m_faultResult.uiCapability()->setUiReadOnly( isGridVizMode );
    m_showFaultFaces.uiCapability()->setUiReadOnly( isGridVizMode );
    m_showOppositeFaultFaces.uiCapability()->setUiReadOnly( isGridVizMode );

    caf::PdmUiGroup* ffviz = uiOrdering.addNewGroup( "Fault Face Visibility" );
    ffviz->setCollapsedByDefault();
    ffviz->add( &m_showFaultFaces );
    ffviz->add( &m_showOppositeFaultFaces );
    ffviz->add( &m_faultResult );
    ffviz->add( &m_onlyShowWithNeighbor );

    caf::PdmUiGroup* nncViz = uiOrdering.addNewGroup( "NNC Visibility" );
    nncViz->setCollapsedByDefault();
    nncViz->add( &m_showNNCs );
    nncViz->add( &m_hideNNCsWhenNoResultIsAvailable );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultInViewCollection::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    caf::PdmUiGroup* general = uiOrdering.addNewGroup( "General" );
    general->add( &m_applyCellFilters );

    caf::PdmUiGroup* labs = uiOrdering.addNewGroup( "Fault Labels" );
    labs->add( &m_showFaultLabel );
    labs->add( &m_faultLabelColor );

    uiOrderingFaults( uiConfigName, uiOrdering );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultInViewCollection::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName )
{
    auto eclipseView = firstAncestorOfType<RimEclipseView>();
    if ( eclipseView )
    {
        auto uiTree = eclipseView->faultResultSettings()->uiTreeOrdering();
        uiTreeOrdering.appendChild( uiTree );
    }

    for ( const auto& fault : m_faults )
    {
        uiTreeOrdering.add( fault );
    }

    uiTreeOrdering.skipRemainingChildren( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseView* RimFaultInViewCollection::parentView() const
{
    return firstAncestorOrThisOfTypeAsserted<RimEclipseView>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultInViewCollection::initAfterRead()
{
    if ( RimProject::current()->isProjectFileVersionEqualOrOlderThan( "2023.03.0" ) )
    {
        m_applyCellFilters = !m_showFaultsOutsideFilters_obsolete();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimFaultInViewCollection::shouldApplyCellFiltersToFaults() const
{
    if ( !m_showFaultCollection() ) return false;

    return m_applyCellFilters;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimFaultInViewCollection::onlyShowFacesWithDefinedNeighbor() const
{
    return m_onlyShowWithNeighbor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimFaultInView*> RimFaultInViewCollection::faults() const
{
    return m_faults.childrenByType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultInViewCollection::setFaultResult( caf::AppEnum<FaultFaceCullingMode> resultType )
{
    m_faultResult = resultType;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultInViewCollection::setShouldApplyCellFiltersToFaults( bool bEnabled )
{
    m_applyCellFilters = bEnabled;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultInViewCollection::setShowOppositeFaultFaces( bool bEnabled )
{
    m_showOppositeFaultFaces = bEnabled;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultInViewCollection::setShowFaultLabelWithFieldChanged( bool bEnabled )
{
    m_showFaultLabel.setValueWithFieldChanged( bEnabled );
}
