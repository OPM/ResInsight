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

#include "RiaApplication.h"
#include "RiaColorTables.h"
#include "RiaPreferences.h"

#include "RigMainGrid.h"

#include "RiaDefines.h"
#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimFaultInView.h"
#include "RimIntersectionCollection.h"
#include "RimNoCommonAreaNNC.h"
#include "RimNoCommonAreaNncCollection.h"

#include "RiuMainWindow.h"

#include "cafAppEnum.h"
#include "cafPdmFieldCvfColor.h"
#include "cafPdmFieldCvfMat4d.h"

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
    CAF_PDM_InitObject( "Faults", ":/draw_style_faults_24x24.png", "", "" );

    CAF_PDM_InitField( &showFaultCollection, "Active", true, "Active", "", "", "" );
    showFaultCollection.uiCapability()->setUiHidden( true );

    CAF_PDM_InitField( &showFaultFaces, "ShowFaultFaces", true, "Show Defined faces", "", "", "" );
    CAF_PDM_InitField( &showOppositeFaultFaces, "ShowOppositeFaultFaces", true, "Show Opposite Faces", "", "", "" );
    CAF_PDM_InitField( &m_showFaultsOutsideFilters, "ShowFaultsOutsideFilters", true, "Show Faults Outside Filters", "", "", "" );

    CAF_PDM_InitField( &faultResult,
                       "FaultFaceCulling",
                       caf::AppEnum<RimFaultInViewCollection::FaultFaceCullingMode>(
                           RimFaultInViewCollection::FAULT_BACK_FACE_CULLING ),
                       "Dynamic Face Selection",
                       "",
                       "",
                       "" );

    CAF_PDM_InitField( &showFaultLabel, "ShowFaultLabel", false, "Show Labels", "", "", "" );
    cvf::Color3f defWellLabelColor = RiaApplication::instance()->preferences()->defaultWellLabelColor();
    CAF_PDM_InitField( &faultLabelColor, "FaultLabelColor", defWellLabelColor, "Label Color", "", "", "" );

    CAF_PDM_InitField( &showNNCs, "ShowNNCs", true, "Show NNCs", "", "", "" );
    CAF_PDM_InitField( &hideNncsWhenNoResultIsAvailable,
                       "HideNncsWhenNoResultIsAvailable",
                       true,
                       "Hide NNC Geometry if No NNC Result is Available",
                       "",
                       "",
                       "" );

    CAF_PDM_InitFieldNoDefault( &noCommonAreaNnncCollection,
                                "NoCommonAreaNnncCollection",
                                "NNCs With No Common Area",
                                "",
                                "",
                                "" );
    noCommonAreaNnncCollection = new RimNoCommonAreaNncCollection;
    noCommonAreaNnncCollection.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &faults, "Faults", "Faults", "", "", "" );
    faults.uiCapability()->setUiHidden( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFaultInViewCollection::~RimFaultInViewCollection()
{
    faults.deleteAllChildObjects();

    delete noCommonAreaNnncCollection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultInViewCollection::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                                 const QVariant&            oldValue,
                                                 const QVariant&            newValue )
{
    this->updateUiIconFromToggleField();

    if ( &faultLabelColor == changedField )
    {
        parentView()->scheduleReservoirGridGeometryRegen();
        parentView()->intersectionCollection()->scheduleCreateDisplayModelAndRedraw2dIntersectionViews();
    }

    if ( &showFaultFaces == changedField || &showOppositeFaultFaces == changedField ||
         &showFaultCollection == changedField || &showFaultLabel == changedField ||
         &m_showFaultsOutsideFilters == changedField || &faultLabelColor == changedField ||
         &faultResult == changedField || &showNNCs == changedField || &hideNncsWhenNoResultIsAvailable == changedField )
    {
        parentView()->scheduleCreateDisplayModelAndRedraw();
        parentView()->intersectionCollection()->scheduleCreateDisplayModelAndRedraw2dIntersectionViews();
    }

    if ( &showFaultLabel == changedField )
    {
        RiuMainWindow::instance()->refreshDrawStyleActions();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimFaultInViewCollection::objectToggleField()
{
    return &showFaultCollection;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFaultInView* RimFaultInViewCollection::findFaultByName( QString name )
{
    for ( size_t i = 0; i < this->faults().size(); ++i )
    {
        if ( this->faults()[i]->name() == name )
        {
            return this->faults()[i];
        }
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// A comparing function used to sort Faults in the RimFaultCollection::syncronizeFaults() method
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
void RimFaultInViewCollection::syncronizeFaults()
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
            if ( faultName.compare( RiaDefines::undefinedGridFaultName(), Qt::CaseInsensitive ) == 0 )
            {
                undefinedFaults = sortedFaults[i];
            }

            if ( faultName.startsWith( RiaDefines::undefinedGridFaultName(), Qt::CaseInsensitive ) &&
                 faultName.contains( "Inactive" ) )
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

    // Find faults with

    std::vector<caf::PdmPointer<RimFaultInView>> newFaults;

    // Find corresponding fault from data model, or create a new
    for ( size_t fIdx = 0; fIdx < rigFaults.size(); ++fIdx )
    {
        RimFaultInView* rimFault = this->findFaultByName( rigFaults[fIdx]->name() );

        if ( !rimFault )
        {
            rimFault             = new RimFaultInView();
            rimFault->faultColor = colorTable.cycledColor3f( fIdx );
            QString faultName    = rigFaults[fIdx]->name();

            if ( faultName.startsWith( RiaDefines::undefinedGridFaultName(), Qt::CaseInsensitive ) &&
                 faultName.contains( "Inactive" ) )
            {
                rimFault->showFault = false; // Turn fault against inactive cells off by default
            }
        }

        rimFault->setFaultGeometry( rigFaults[fIdx].p() );

        newFaults.push_back( rimFault );
    }

    this->faults().clear();
    this->faults().insert( 0, newFaults );

    QString toolTip = QString( "Fault count (%1)" ).arg( newFaults.size() );
    setUiToolTip( toolTip );

    // NNCs
    this->noCommonAreaNnncCollection()->noCommonAreaNncs().deleteAllChildObjects();

    RigMainGrid*                      mainGrid       = parentView()->mainGrid();
    const std::vector<RigConnection>& nncConnections = mainGrid->nncData()->connections();
    for ( size_t connIndex = 0; connIndex < nncConnections.size(); connIndex++ )
    {
        if ( !nncConnections[connIndex].hasCommonArea() )
        {
            RimNoCommonAreaNNC* noCommonAreaNnc = new RimNoCommonAreaNNC();

            QString firstConnectionText;
            QString secondConnectionText;

            {
                size_t             gridLocalCellIndex;
                const RigGridBase* hostGrid =
                    mainGrid->gridAndGridLocalIdxFromGlobalCellIdx( nncConnections[connIndex].m_c1GlobIdx,
                                                                    &gridLocalCellIndex );

                size_t i, j, k;
                if ( hostGrid->ijkFromCellIndex( gridLocalCellIndex, &i, &j, &k ) )
                {
                    // Adjust to 1-based Eclipse indexing
                    i++;
                    j++;
                    k++;

                    if ( !hostGrid->isMainGrid() )
                    {
                        QString gridName    = QString::fromStdString( hostGrid->gridName() );
                        firstConnectionText = gridName + " ";
                    }
                    firstConnectionText += QString( "[%1 %2 %3] - " ).arg( i ).arg( j ).arg( k );
                }
            }

            {
                size_t             gridLocalCellIndex;
                const RigGridBase* hostGrid =
                    mainGrid->gridAndGridLocalIdxFromGlobalCellIdx( nncConnections[connIndex].m_c2GlobIdx,
                                                                    &gridLocalCellIndex );

                size_t i, j, k;
                if ( hostGrid->ijkFromCellIndex( gridLocalCellIndex, &i, &j, &k ) )
                {
                    // Adjust to 1-based Eclipse indexing
                    i++;
                    j++;
                    k++;

                    if ( !hostGrid->isMainGrid() )
                    {
                        QString gridName     = QString::fromStdString( hostGrid->gridName() );
                        secondConnectionText = gridName + " ";
                    }
                    secondConnectionText += QString( "[%1 %2 %3]" ).arg( i ).arg( j ).arg( k );
                }
            }

            noCommonAreaNnc->name = firstConnectionText + secondConnectionText;
            this->noCommonAreaNnncCollection()->noCommonAreaNncs().push_back( noCommonAreaNnc );
        }

        this->noCommonAreaNnncCollection()->updateName();
    }
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

    faultResult.uiCapability()->setUiReadOnly( isGridVizMode );
    showFaultFaces.uiCapability()->setUiReadOnly( isGridVizMode );
    showOppositeFaultFaces.uiCapability()->setUiReadOnly( isGridVizMode );

    caf::PdmUiGroup* ffviz = uiOrdering.addNewGroup( "Fault Face Visibility" );
    ffviz->setCollapsedByDefault( true );
    ffviz->add( &showFaultFaces );
    ffviz->add( &showOppositeFaultFaces );
    ffviz->add( &faultResult );

    caf::PdmUiGroup* nncViz = uiOrdering.addNewGroup( "NNC Visibility" );
    nncViz->setCollapsedByDefault( true );
    nncViz->add( &showNNCs );
    nncViz->add( &hideNncsWhenNoResultIsAvailable );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultInViewCollection::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    caf::PdmUiGroup* labs = uiOrdering.addNewGroup( "Fault Labels" );
    labs->add( &showFaultLabel );
    labs->add( &faultLabelColor );

    caf::PdmUiGroup* adv = uiOrdering.addNewGroup( "Fault Options" );
    adv->add( &m_showFaultsOutsideFilters );

    uiOrderingFaults( uiConfigName, uiOrdering );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseView* RimFaultInViewCollection::parentView() const
{
    RimEclipseView* view = nullptr;
    this->firstAncestorOrThisOfTypeAsserted( view );

    return view;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimFaultInViewCollection::isShowingFaultsAndFaultsOutsideFilters() const
{
    if ( !showFaultCollection ) return false;

    return m_showFaultsOutsideFilters;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultInViewCollection::setShowFaultsOutsideFilter( bool show )
{
    m_showFaultsOutsideFilters = show;
}
