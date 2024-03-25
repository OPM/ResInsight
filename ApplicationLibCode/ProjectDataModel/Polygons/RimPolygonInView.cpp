/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024     Equinor ASA
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

#include "RimPolygonInView.h"

#include "RiaColorTools.h"

#include "RigPolyLinesData.h"

#include "Rim3dView.h"
#include "RimPolygon.h"
#include "RimPolygonInViewCollection.h"
#include "RimPolylineTarget.h"
#include "RimTools.h"

#include "WellPathCommands/PointTangentManipulator/RicPolyline3dEditor.h"
#include "WellPathCommands/RicPolylineTargetsPickEventHandler.h"

#include "Riu3DMainWindowTools.h"
#include "RiuGuiTheme.h"

#include "RivPolylinePartMgr.h"

#include "cafCmdFeatureMenuBuilder.h"
#include "cafDisplayCoordTransform.h"
#include "cafPdmUiPushButtonEditor.h"
#include "cafPdmUiTableViewEditor.h"
#include "cafPdmUiTreeAttributes.h"

#include "cvfModelBasicList.h"

CAF_PDM_SOURCE_INIT( RimPolygonInView, "RimPolygonInView" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPolygonInView::RimPolygonInView()
    : m_pickTargetsEventHandler( new RicPolylineTargetsPickEventHandler( this ) )
{
    CAF_PDM_InitObject( "Polygon", ":/PolylinesFromFile16x16.png" );

    CAF_PDM_InitFieldNoDefault( &m_polygon, "Polygon", "Polygon" );
    m_polygon.uiCapability()->setUiReadOnly( true );

    nameField()->uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitField( &m_enablePicking, "EnablePicking", false, "" );
    caf::PdmUiPushButtonEditor::configureEditorLabelHidden( &m_enablePicking );

    CAF_PDM_InitField( &m_selectPolygon, "SelectPolygon", false, "" );
    caf::PdmUiPushButtonEditor::configureEditorLabelHidden( &m_selectPolygon );

    CAF_PDM_InitField( &m_handleScalingFactor, "HandleScalingFactor", 1.0, "Handle Scaling Factor" );

    CAF_PDM_InitFieldNoDefault( &m_targets, "Targets", "Targets" );
    m_targets.uiCapability()->setUiEditorTypeName( caf::PdmUiTableViewEditor::uiEditorTypeName() );
    m_targets.uiCapability()->setUiTreeChildrenHidden( true );
    m_targets.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::TOP );
    m_targets.uiCapability()->setCustomContextMenuEnabled( true );
    m_targets.xmlCapability()->disableIO();

    setUi3dEditorTypeName( RicPolyline3dEditor::uiEditorTypeName() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPolygon* RimPolygonInView::polygon() const
{
    return m_polygon();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonInView::setPolygon( RimPolygon* polygon )
{
    m_polygon = polygon;

    connectSignals();

    updateTargetsFromPolygon();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonInView::appendPartsToModel( cvf::ModelBasicList*              model,
                                           const caf::DisplayCoordTransform* scaleTransform,
                                           const cvf::BoundingBox&           boundingBox )
{
    auto view = firstAncestorOfType<Rim3dView>();

    if ( m_polylinePartMgr.isNull() ) m_polylinePartMgr = new RivPolylinePartMgr( view, this, this );

    m_polylinePartMgr->appendDynamicGeometryPartsToModel( model, scaleTransform, boundingBox );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonInView::enablePicking( bool enable )
{
    m_enablePicking = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonInView::insertTarget( const RimPolylineTarget* targetToInsertBefore, RimPolylineTarget* targetToInsert )
{
    size_t index = m_targets.indexOf( targetToInsertBefore );
    if ( index < m_targets.size() )
        m_targets.insert( index, targetToInsert );
    else
        m_targets.push_back( targetToInsert );

    updatePolygonFromTargets();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonInView::deleteTarget( RimPolylineTarget* targetToDelete )
{
    m_targets.removeChild( targetToDelete );
    delete targetToDelete;

    updatePolygonFromTargets();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonInView::updateEditorsAndVisualization()
{
    updateConnectedEditors();
    updateVisualization();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonInView::updateVisualization()
{
    auto view = firstAncestorOfType<Rim3dView>();
    if ( view )
    {
        view->scheduleCreateDisplayModelAndRedraw();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimPolylineTarget*> RimPolygonInView::activeTargets() const
{
    return m_targets.childrenByType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimPolygonInView::pickingEnabled() const
{
    return m_enablePicking();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PickEventHandler* RimPolygonInView::pickEventHandler() const
{
    auto filterColl = firstAncestorOfType<RimPolygonInViewCollection>();
    if ( filterColl && !filterColl->isChecked() ) return nullptr;

    if ( !isChecked() ) return nullptr;

    if ( m_polygon() && polygon()->isReadOnly() ) return nullptr;

    return m_pickTargetsEventHandler.get();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonInView::onChildrenUpdated( caf::PdmChildArrayFieldHandle* childArray, std::vector<caf::PdmObjectHandle*>& updatedObjects )
{
    if ( childArray == &m_targets )
    {
        updatePolygonFromTargets();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<RigPolyLinesData> RimPolygonInView::polyLinesData() const
{
    if ( m_polygon )
    {
        return m_polygon->polyLinesData();
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonInView::updatePolygonFromTargets()
{
    if ( m_polygon )
    {
        std::vector<cvf::Vec3d> points;
        for ( const RimPolylineTarget* target : m_targets )
        {
            points.push_back( target->targetPointXYZ() );
        }
        m_polygon->setPointsInDomainCoords( points );

        // update other pick editors, make sure we don't update ourselves
        m_polygon->coordinatesChanged.block( this );
        m_polygon->coordinatesChanged.send();
        m_polygon->coordinatesChanged.unblock( this );

        m_polygon->objectChanged.send();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonInView::connectSignals()
{
    if ( m_polygon )
    {
        m_polygon->objectChanged.connect( this, &RimPolygonInView::onObjectChanged );
        m_polygon->coordinatesChanged.connect( this, &RimPolygonInView::onCoordinatesChanged );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonInView::updateTargetsFromPolygon()
{
    if ( m_polygon )
    {
        m_targets.deleteChildren();

        for ( const auto& p : m_polygon->pointsInDomainCoords() )
        {
            auto target = new RimPolylineTarget();
            target->setAsPointXYZ( p );

            m_targets.push_back( target );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonInView::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    updateNameField();

    bool enableEdit = true;
    if ( m_polygon() && m_polygon->isReadOnly() ) enableEdit = false;

    uiOrdering.add( m_polygon );

    if ( enableEdit )
    {
        uiOrdering.add( &m_enablePicking );
        uiOrdering.add( &m_targets );
        uiOrdering.add( &m_handleScalingFactor );
    }

    if ( m_polygon() )
    {
        uiOrdering.add( &m_selectPolygon );
    }

    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonInView::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( changedField == &m_enablePicking )
    {
        updateConnectedEditors();
    }

    if ( changedField == &m_selectPolygon && m_polygon() )
    {
        Riu3DMainWindowTools::selectAsCurrentItem( m_polygon() );
    }

    updateVisualization();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimPolygonInView::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;
    if ( fieldNeedingOptions == &m_polygon )
    {
        options.push_back( caf::PdmOptionItemInfo( "None", nullptr ) );

        RimTools::polygonOptionItems( &options );
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonInView::defineObjectEditorAttribute( QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    if ( auto attrib = dynamic_cast<RicPolyline3dEditorAttribute*>( attribute ) )
    {
        attrib->pickEventHandler = m_pickTargetsEventHandler;
        attrib->enablePicking    = m_enablePicking;
    }

    if ( m_polygon() )
    {
        if ( auto* treeItemAttribute = dynamic_cast<caf::PdmUiTreeViewItemAttribute*>( attribute ) )
        {
            auto tag = caf::PdmUiTreeViewItemAttribute::createTag( RiaColorTools::toQColor( m_polygon->color() ),
                                                                   RiuGuiTheme::getColorByVariableName( "backgroundColor1" ),
                                                                   "---" );

            tag->clicked.connect( m_polygon(), &RimPolygon::onColorTagClicked );

            treeItemAttribute->tags.push_back( std::move( tag ) );
        }

        if ( m_polygon->isReadOnly() )
        {
            caf::PdmUiTreeViewItemAttribute::appendTagToTreeViewItemAttribute( attribute, ":/padlock.svg" );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonInView::uiOrderingForLocalPolygon( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_enablePicking );
    uiOrdering.add( &m_targets );
    uiOrdering.add( &m_handleScalingFactor );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonInView::appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const
{
    RimPolygon::appendPolygonMenuItems( menuBuilder );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonInView::onObjectChanged( const caf::SignalEmitter* emitter )
{
    updateVisualization();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonInView::onCoordinatesChanged( const caf::SignalEmitter* emitter )
{
    updateTargetsFromPolygon();

    updateConnectedEditors();
    updateVisualization();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonInView::initAfterRead()
{
    connectSignals();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimPolygonInView::scalingFactorForTarget() const
{
    return m_handleScalingFactor();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonInView::defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
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

    if ( field == &m_selectPolygon )
    {
        auto* pbAttribute = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*>( attribute );
        if ( pbAttribute )
        {
            pbAttribute->m_buttonText = "Go to Polygon";
        }
    }

    if ( field == &m_targets )
    {
        if ( auto tvAttribute = dynamic_cast<caf::PdmUiTableViewEditorAttribute*>( attribute ) )
        {
            tvAttribute->resizePolicy = caf::PdmUiTableViewEditorAttribute::RESIZE_TO_FIT_CONTENT;

            if ( m_enablePicking )
            {
                tvAttribute->baseColor = RiuGuiTheme::getColorByVariableName( "externalInputColor" );
            }
            tvAttribute->alwaysEnforceResizePolicy = true;
            tvAttribute->heightHint                = 1000;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonInView::defineCustomContextMenu( const caf::PdmFieldHandle* fieldNeedingMenu, QMenu* menu, QWidget* fieldEditorWidget )
{
    if ( m_polygon() && m_polygon->isReadOnly() ) return;

    caf::CmdFeatureMenuBuilder menuBuilder;

    menuBuilder << "RicNewPolylineTargetFeature";
    menuBuilder << "Separator";
    menuBuilder << "RicDeletePolylineTargetFeature";

    menuBuilder.appendToMenu( menu );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonInView::updateNameField()
{
    QString name = "Undefined";
    if ( m_polygon() )
    {
        name = m_polygon->name();
    }

    setName( name );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonInView::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= "" */ )
{
    updateNameField();
}
