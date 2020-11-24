/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020 Equinor ASA
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

#include "RimPolylineFilter.h"

#include "Rim3dView.h"
#include "RimPolylineTarget.h"

#include "WellPathCommands/PointTangentManipulator/RicPolyline3dEditor.h"

#include "RimCellFilterCollection.h"
#include "WellPathCommands/RicPolylineTargetsPickEventHandler.h"

#include "RigPolyLinesData.h"

#include "RiuViewerCommands.h"

#include "cvfBoundingBox.h"

#include "cafCmdFeatureMenuBuilder.h"
#include "cafPdmUiPushButtonEditor.h"
#include "cafPdmUiTableViewEditor.h"
#include "cafPdmUiTreeOrdering.h"

namespace caf
{
template <>
void caf::AppEnum<RimPolylineFilter::PolylineFilterModeType>::setUp()
{
    addItem( RimPolylineFilter::DEPTH_Z, "DEPTH_Z", "Depth" );
    addItem( RimPolylineFilter::INDEX_K, "INDEX_K", "K index" );
    setDefault( RimPolylineFilter::DEPTH_Z );
}
} // namespace caf

CAF_PDM_SOURCE_INIT( RimPolylineFilter, "PolyLineFilter" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPolylineFilter::RimPolylineFilter()
    : m_pickTargetsEventHandler( new RicPolylineTargetsPickEventHandler( this ) )
{
    CAF_PDM_InitObject( "Polyline Filter", ":/CellFilter_Polyline.png", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_polyFilterMode, "PolylineFilterType", "Depth Type", "", "", "" );

    CAF_PDM_InitField( &m_showPolygon, "ShowPolygon", true, "Show Polygon", "", "", "" );

    CAF_PDM_InitField( &m_enablePicking, "EnablePicking", false, "", "", "", "" );
    caf::PdmUiPushButtonEditor::configureEditorForField( &m_enablePicking );
    m_enablePicking.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::LabelPosType::HIDDEN );

    CAF_PDM_InitFieldNoDefault( &m_targets, "Targets", "Targets", "", "", "" );
    m_targets.uiCapability()->setUiEditorTypeName( caf::PdmUiTableViewEditor::uiEditorTypeName() );
    // m_targets.uiCapability()->setUiTreeHidden(true);
    m_targets.uiCapability()->setUiTreeChildrenHidden( true );
    m_targets.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::TOP );
    m_targets.uiCapability()->setCustomContextMenuEnabled( true );

    this->setUi3dEditorTypeName( RicPolyline3dEditor::uiEditorTypeName() );
    this->uiCapability()->setUiTreeChildrenHidden( true );

    updateIconState();
    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPolylineFilter::~RimPolylineFilter()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolylineFilter::updateVisualization()
{
    Rim3dView* view;
    this->firstAncestorOrThisOfType( view );

    if ( view ) view->scheduleCreateDisplayModelAndRedraw();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolylineFilter::updateEditorsAndVisualization()
{
    updateConnectedEditors();
    updateVisualization();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimPolylineTarget*> RimPolylineFilter::activeTargets() const
{
    return m_targets.childObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolylineFilter::insertTarget( const RimPolylineTarget* targetToInsertBefore, RimPolylineTarget* targetToInsert )
{
    size_t index = m_targets.index( targetToInsertBefore );
    if ( index < m_targets.size() )
        m_targets.insert( index, targetToInsert );
    else
        m_targets.push_back( targetToInsert );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolylineFilter::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                               QString                    uiConfigName,
                                               caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_enablePicking )
    {
        auto* pbAttribute = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*>( attribute );
        if ( pbAttribute )
        {
            if ( !m_enablePicking )
            {
                pbAttribute->m_buttonText = "Start Picking Points";
            }
            else
            {
                pbAttribute->m_buttonText = "Stop Picking Points";
            }
        }
    }

    if ( field == &m_targets )
    {
        auto tvAttribute = dynamic_cast<caf::PdmUiTableViewEditorAttribute*>( attribute );
        if ( tvAttribute )
        {
            tvAttribute->resizePolicy = caf::PdmUiTableViewEditorAttribute::RESIZE_TO_FIT_CONTENT;

            if ( m_enablePicking )
            {
                tvAttribute->baseColor.setRgb( 255, 220, 255 );
                tvAttribute->alwaysEnforceResizePolicy = true;
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolylineFilter::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    RimCellFilter::defineUiOrdering( uiConfigName, uiOrdering );

    uiOrdering.add( &m_polyFilterMode );
    uiOrdering.add( &m_targets );
    uiOrdering.add( &m_enablePicking );
    uiOrdering.add( &m_showPolygon );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolylineFilter::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                          const QVariant&            oldValue,
                                          const QVariant&            newValue )
{
    if ( changedField == &m_enablePicking )
    {
        this->updateConnectedEditors();
    }

    updateVisualization();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolylineFilter::enablePicking( bool enable )
{
    m_enablePicking = enable;
    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimPolylineFilter::pickingEnabled() const
{
    return m_enablePicking();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PickEventHandler* RimPolylineFilter::pickEventHandler() const
{
    return m_pickTargetsEventHandler.get();
}
