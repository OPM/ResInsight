/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023     Equinor ASA
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

#include "RimFaultReactivationModel.h"

#include "RiaPreferencesGeoMech.h"

#include "RigBasicPlane.h"
#include "RigPolyLinesData.h"

#include "WellPathCommands/PointTangentManipulator/RicPolyline3dEditor.h"
#include "WellPathCommands/RicPolylineTargetsPickEventHandler.h"

#include "RiuViewer.h"

#include "RivFaultReactivationModelPartMgr.h"

#include "Rim3dView.h"
#include "RimFaultInView.h"
#include "RimFaultInViewCollection.h"
#include "RimPolylineTarget.h"
#include "RimTools.h"

#include "cafPdmUiTableViewEditor.h"

#include "cvfPlane.h"

CAF_PDM_SOURCE_INIT( RimFaultReactivationModel, "FaultReactivationModel" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFaultReactivationModel::RimFaultReactivationModel()
    : m_pickTargetsEventHandler( new RicPolylineTargetsPickEventHandler( this ) )
{
    CAF_PDM_InitObject( "Fault Reactivation Model", ":/fault_react_24x24.png" );

    CAF_PDM_InitField( &m_userDescription, "UserDescription", QString( "Model" ), "Name" );

    CAF_PDM_InitField( &m_extentHorizontal, "HorizontalExtent", 1000.0, "Horizontal Extent" );
    CAF_PDM_InitField( &m_extentVertical, "VerticalExtent", 500.0, "Vertical Extent" );

    CAF_PDM_InitFieldNoDefault( &m_fault, "Fault", "Fault" );
    m_fault.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitField( &m_faultPlaneColor, "FaultPlaneColor", cvf::Color3f( cvf::Color3f::GRAY ), "Plane Color" );

    CAF_PDM_InitFieldNoDefault( &m_targets, "Targets", "Targets" );
    m_targets.uiCapability()->setUiEditorTypeName( caf::PdmUiTableViewEditor::uiEditorTypeName() );
    m_targets.uiCapability()->setUiTreeChildrenHidden( true );
    m_targets.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::TOP );
    m_targets.uiCapability()->setCustomContextMenuEnabled( false );

    this->setUi3dEditorTypeName( RicPolyline3dEditor::uiEditorTypeName() );
    this->uiCapability()->setUiTreeChildrenHidden( true );

    setDeletable( true );

    m_faultPlane = new RigBasicPlane();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFaultReactivationModel::~RimFaultReactivationModel()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimFaultReactivationModel::userDescription()
{
    return m_userDescription;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultReactivationModel::setUserDescription( QString description )
{
    m_userDescription = description;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimFaultReactivationModel::userDescriptionField()
{
    return &m_userDescription;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultReactivationModel::setFault( RimFaultInView* fault )
{
    m_fault = fault;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFaultInView* RimFaultReactivationModel::fault() const
{
    return m_fault();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultReactivationModel::setTargets( cvf::Vec3d target1, cvf::Vec3d target2 )
{
    m_targets.deleteChildren();

    RimPolylineTarget* planeCenter = new RimPolylineTarget();
    planeCenter->setAsPointXYZ( target1 );

    m_targets.push_back( planeCenter );

    RimPolylineTarget* steeringTarget = new RimPolylineTarget();
    steeringTarget->setAsPointXYZ( target2 );

    m_targets.push_back( steeringTarget );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimPolylineTarget*> RimFaultReactivationModel::activeTargets() const
{
    return m_targets.childrenByType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultReactivationModel::insertTarget( const RimPolylineTarget* targetToInsertBefore, RimPolylineTarget* targetToInsert )
{
    // do nothing, we should only have 2 predefined targets
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultReactivationModel::deleteTarget( RimPolylineTarget* targetToDelete )
{
    // do nothing, we should only have 2 predefined targets
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimFaultReactivationModel::pickingEnabled() const
{
    // never pick, we only have our 2 predefined targets
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PickEventHandler* RimFaultReactivationModel::pickEventHandler() const
{
    return m_pickTargetsEventHandler.get();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultReactivationModel::updateVisualization()
{
    auto normal = m_targets[1]->targetPointXYZ() - m_targets[0]->targetPointXYZ();
    normal.normalize();

    m_faultPlane->setPlane( m_targets[0]->targetPointXYZ(), normal );
    m_faultPlane->setMaxExtentFromAnchor( m_extentHorizontal, m_extentVertical );
    m_faultPlane->setColor( m_faultPlaneColor );

    auto view = firstAncestorOrThisOfType<Rim3dView>();
    if ( view ) view->scheduleCreateDisplayModelAndRedraw();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultReactivationModel::updateEditorsAndVisualization()
{
    updateConnectedEditors();
    updateVisualization();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<RigPolyLinesData> RimFaultReactivationModel::polyLinesData() const
{
    cvf::ref<RigPolyLinesData> pld = new RigPolyLinesData;

    std::vector<cvf::Vec3d> line;
    for ( const RimPolylineTarget* target : m_targets )
    {
        line.push_back( target->targetPointXYZ() );
    }
    pld->setPolyLine( line );

    return pld;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimFaultReactivationModel::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_fault )
    {
        if ( m_fault() != nullptr )
        {
            auto coll = m_fault->firstAncestorOrThisOfType<RimFaultInViewCollection>();
            if ( coll != nullptr ) RimTools::faultOptionItems( &options, coll );
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivFaultReactivationModelPartMgr* RimFaultReactivationModel::partMgr()
{
    if ( m_partMgr.isNull() ) m_partMgr = new RivFaultReactivationModelPartMgr( this );

    return m_partMgr.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<RigBasicPlane> RimFaultReactivationModel::faultPlane() const
{
    return m_faultPlane;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::Plane> RimFaultReactivationModel::modelPlane() const
{
    cvf::ref<cvf::Plane> plane = new cvf::Plane();

    cvf::Vec3d p3 = m_targets[0]->targetPointXYZ();
    p3.z()        = p3.z() + 1000.0;

    plane->setFromPoints( m_targets[0]->targetPointXYZ(), m_targets[1]->targetPointXYZ(), p3 );
    return plane;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultReactivationModel::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    auto genGrp = uiOrdering.addNewGroup( "General" );
    genGrp->add( &m_userDescription );

    auto faultGrp = uiOrdering.addNewGroup( "Fault Plane" );

    faultGrp->add( &m_fault );
    faultGrp->add( &m_faultPlaneColor );
    faultGrp->add( &m_extentHorizontal );
    faultGrp->add( &m_extentVertical );

    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultReactivationModel::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( changedField == &m_userDescription )
    {
        updateConnectedEditors();
        return;
    }

    if ( ( changedField == &m_extentHorizontal ) || ( changedField == &m_extentVertical ) || ( changedField == &m_faultPlaneColor ) ||
         ( changedField == &m_targets ) )
    {
        updateVisualization();
    }
}
