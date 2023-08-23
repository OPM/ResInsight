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

#include "RiaApplication.h"
#include "RiaPreferencesGeoMech.h"

#include "RigBasicPlane.h"
#include "RigFaultReactivationModel.h"
#include "RigPolyLinesData.h"

#include "WellPathCommands/PointTangentManipulator/RicPolyline3dEditor.h"
#include "WellPathCommands/RicPolylineTargetsPickEventHandler.h"

#include "RiuViewer.h"

#include "RivFaultReactivationModelPartMgr.h"

#include "Rim3dView.h"
#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimFaultInView.h"
#include "RimFaultInViewCollection.h"
#include "RimPolylineTarget.h"
#include "RimTools.h"

#include "cafPdmUiDoubleSliderEditor.h"
#include "cafPdmUiTableViewEditor.h"

#include "cvfPlane.h"
#include "cvfTextureImage.h"

CAF_PDM_SOURCE_INIT( RimFaultReactivationModel, "FaultReactivationModel" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFaultReactivationModel::RimFaultReactivationModel()
    : m_pickTargetsEventHandler( new RicPolylineTargetsPickEventHandler( this ) )
{
    CAF_PDM_InitObject( "Fault Reactivation Model", ":/fault_react_24x24.png" );

    CAF_PDM_InitField( &m_userDescription, "UserDescription", QString( "Model" ), "Name" );

    CAF_PDM_InitFieldNoDefault( &m_baseDir, "BaseDirectory", "Working folder" );

    CAF_PDM_InitField( &m_modelThickness, "ModelThickness", 100.0, "Model Cell Thickness" );

    CAF_PDM_InitField( &m_extentHorizontal, "HorizontalExtent", 1000.0, "Horizontal Extent" );
    CAF_PDM_InitField( &m_extentVerticalAbove, "VerticalExtentAbove", 200.0, "Vertical Extent Above Anchor" );
    m_extentVerticalAbove.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );
    m_extentVerticalAbove.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::LabelPosType::TOP );

    CAF_PDM_InitField( &m_extentVerticalBelow, "VerticalExtentBelow", 200.0, "Vertical Extent Below Anchor" );
    m_extentVerticalBelow.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );
    m_extentVerticalBelow.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::LabelPosType::TOP );

    CAF_PDM_InitField( &m_modelExtentFromAnchor, "ModelExtentFromAnchor", 1000.0, "Horz. Extent from Anchor" );
    CAF_PDM_InitField( &m_modelMinZ, "ModelMinZ", 0.0, "Start Depth" );
    CAF_PDM_InitField( &m_modelBelowSize, "ModelBelowSize", 500.0, "Depth Below Fault" );

    CAF_PDM_InitField( &m_showFaultPlane, "ShowFaultPlane", true, "Show Fault Plane" );
    CAF_PDM_InitField( &m_showModelPlane, "ShowModelPlane", false, "Show 2D Model" );

    CAF_PDM_InitFieldNoDefault( &m_fault, "Fault", "Fault" );
    m_fault.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitField( &m_faultPlaneColor, "FaultPlaneColor", cvf::Color3f( cvf::Color3f::GRAY ), "Plane Color" );
    CAF_PDM_InitField( &m_modelPart1Color, "ModelPart1Color", cvf::Color3f( cvf::Color3f::GREEN ), "Part 1 Color" );
    CAF_PDM_InitField( &m_modelPart2Color, "ModelPart2Color", cvf::Color3f( cvf::Color3f::BLUE ), "Part 2 Color" );

    CAF_PDM_InitField( &m_numberOfCellsHorzPart1, "NumberOfCellsHorzPart1", 20, "Horizontal Number of Cells, Part 1" );
    CAF_PDM_InitField( &m_numberOfCellsHorzPart2, "NumberOfCellsHorzPart2", 20, "Horizontal Number of Cells, Part 2" );
    CAF_PDM_InitField( &m_numberOfCellsVertUp, "NumberOfCellsVertUp", 20, "Vertical Number of Cells, Upper Part" );
    CAF_PDM_InitField( &m_numberOfCellsVertMid, "NumberOfCellsVertMid", 20, "Vertical Number of Cells, Middle Part" );
    CAF_PDM_InitField( &m_numberOfCellsVertLow, "NumberOfCellsVertLow", 20, "Vertical Number of Cells, Lower Part" );

    CAF_PDM_InitFieldNoDefault( &m_targets, "Targets", "Targets" );
    m_targets.uiCapability()->setUiEditorTypeName( caf::PdmUiTableViewEditor::uiEditorTypeName() );
    m_targets.uiCapability()->setUiTreeChildrenHidden( true );
    m_targets.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::TOP );
    m_targets.uiCapability()->setCustomContextMenuEnabled( false );

    this->setUi3dEditorTypeName( RicPolyline3dEditor::uiEditorTypeName() );
    this->uiCapability()->setUiTreeChildrenHidden( true );

    setDeletable( true );

    m_faultPlane = new RigBasicPlane();
    m_modelPlane = new RigFaultReactivationModel();
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
void RimFaultReactivationModel::initAfterRead()
{
    updateVisualization();
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
    // do nothing, we should always have 2 predefined targets
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
    auto view = firstAncestorOrThisOfType<Rim3dView>();
    if ( !view ) return;

    auto normal = m_targets[1]->targetPointXYZ() - m_targets[0]->targetPointXYZ();
    normal.z()  = normal.z() * view->scaleZ() * view->scaleZ();
    normal.normalize();

    m_faultPlane->setPlane( m_targets[0]->targetPointXYZ(), normal );
    m_faultPlane->setMaxExtentFromAnchor( m_extentHorizontal, m_extentVerticalAbove, m_extentVerticalBelow );
    m_faultPlane->setColor( m_faultPlaneColor );
    m_faultPlane->updateRect();

    double maxZ              = m_faultPlane->maxDepth();
    auto [topInt, bottomInt] = m_faultPlane->intersectTopBottomLine();

    cvf::Vec3d zdir( 0, 0, 1 );
    auto       modelNormal = normal ^ zdir;
    modelNormal.normalize();

    m_modelPlane->setPlane( m_targets[0]->targetPointXYZ(), modelNormal );
    m_modelPlane->setFaultPlaneIntersect( topInt, bottomInt );
    m_modelPlane->setMaxExtentFromAnchor( m_modelExtentFromAnchor, m_modelMinZ, maxZ + m_modelBelowSize );
    m_modelPlane->setPartColors( m_modelPart1Color, m_modelPart2Color );
    m_modelPlane->setCellCounts( m_numberOfCellsHorzPart1,
                                 m_numberOfCellsHorzPart2,
                                 m_numberOfCellsVertUp,
                                 m_numberOfCellsVertMid,
                                 m_numberOfCellsVertLow );
    m_modelPlane->setThickness( m_modelThickness );

    m_modelPlane->updateRects();

    view->scheduleCreateDisplayModelAndRedraw();
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
cvf::ref<RigFaultReactivationModel> RimFaultReactivationModel::model() const
{
    return m_modelPlane;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimFaultReactivationModel::showFaultPlane() const
{
    return m_showFaultPlane;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimFaultReactivationModel::showModel() const
{
    return m_showModelPlane;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultReactivationModel::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    auto genGrp = uiOrdering.addNewGroup( "General" );
    genGrp->add( &m_userDescription );
    genGrp->add( &m_fault );
    genGrp->add( &m_baseDir );

    auto faultGrp = uiOrdering.addNewGroup( "Fault Plane" );

    faultGrp->add( &m_showFaultPlane );
    faultGrp->add( &m_faultPlaneColor );
    faultGrp->add( &m_extentHorizontal );
    faultGrp->add( &m_extentVerticalAbove );
    faultGrp->add( &m_extentVerticalBelow );

    auto modelGrp = uiOrdering.addNewGroup( "2D Model" );
    modelGrp->add( &m_showModelPlane );

    auto sizeModelGrp = modelGrp->addNewGroup( "Size" );
    sizeModelGrp->add( &m_modelExtentFromAnchor );
    sizeModelGrp->add( &m_modelMinZ );
    sizeModelGrp->add( &m_modelBelowSize );

    auto gridModelGrp = modelGrp->addNewGroup( "Grid" );

    gridModelGrp->add( &m_modelThickness );
    gridModelGrp->add( &m_numberOfCellsHorzPart1 );
    gridModelGrp->add( &m_numberOfCellsHorzPart2 );
    gridModelGrp->add( &m_numberOfCellsVertUp );
    gridModelGrp->add( &m_numberOfCellsVertMid );
    gridModelGrp->add( &m_numberOfCellsVertLow );

    auto appModelGrp = modelGrp->addNewGroup( "Appearance" );
    appModelGrp->add( &m_modelPart1Color );
    appModelGrp->add( &m_modelPart2Color );

    auto trgGroup = uiOrdering.addNewGroup( "Debug" );
    trgGroup->setCollapsedByDefault();
    trgGroup->add( &m_targets );

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
    }
    else
    {
        updateVisualization();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultReactivationModel::defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_targets )
    {
        auto tvAttribute = dynamic_cast<caf::PdmUiTableViewEditorAttribute*>( attribute );
        if ( tvAttribute )
        {
            tvAttribute->resizePolicy = caf::PdmUiTableViewEditorAttribute::RESIZE_TO_FIT_CONTENT;
        }
    }
    else if ( ( field == &m_extentVerticalAbove ) || ( field == &m_extentVerticalBelow ) )
    {
        auto* attr = dynamic_cast<caf::PdmUiDoubleSliderEditorAttribute*>( attribute );

        if ( attr )
        {
            auto eclCase = eclipseCase();
            if ( eclCase )
            {
                auto   bb       = eclCase->allCellsBoundingBox();
                double diff     = bb.max().z() - bb.min().z();
                attr->m_minimum = 0;
                attr->m_maximum = diff;
            }
            else
            {
                attr->m_minimum = 0;
                attr->m_maximum = 1000;
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseCase* RimFaultReactivationModel::eclipseCase()
{
    auto activeView = dynamic_cast<RimEclipseView*>( RiaApplication::instance()->activeGridView() );
    if ( activeView )
    {
        return activeView->eclipseCase();
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultReactivationModel::setBaseDir( QString path )
{
    m_baseDir = path;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimFaultReactivationModel::baseDir() const
{
    return m_baseDir().path();
}
