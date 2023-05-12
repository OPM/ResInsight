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

CAF_PDM_SOURCE_INIT( RimFaultReactivationModel, "FaultReactivationModel" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFaultReactivationModel::RimFaultReactivationModel()
{
    CAF_PDM_InitObject( "Fault Reactivation Model", ":/fault_react_24x24.png" );

    CAF_PDM_InitField( &m_userDescription, "UserDescription", QString( "Model" ), "Name" );

    CAF_PDM_InitFieldNoDefault( &m_fault, "Fault", "Fault" );
    m_fault.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_targets, "Targets", "Targets" );
    m_targets.uiCapability()->setUiEditorTypeName( caf::PdmUiTableViewEditor::uiEditorTypeName() );
    m_targets.uiCapability()->setUiTreeChildrenHidden( true );
    m_targets.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::TOP );
    m_targets.uiCapability()->setCustomContextMenuEnabled( false );
    m_targets.uiCapability()->setUiReadOnly( true );
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
    return m_targets.children();
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
        RimFaultInViewCollection* coll = nullptr;
        if ( m_fault() != nullptr ) m_fault->firstAncestorOrThisOfType( coll );

        if ( coll != nullptr ) RimTools::faultOptionItems( &options, coll );
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
