/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor ASA
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

#include "RimWellPathLateralGeometryDef.h"

#include "WellPathCommands/PointTangentManipulator/RicWellPathGeometry3dEditor.h"
#include "WellPathCommands/RicCreateWellTargetsPickEventHandler.h"

#include "RiaFieldHandleTools.h"
#include "RiaJCurveCalculator.h"
#include "RiaLogging.h"
#include "RiaOffshoreSphericalCoords.h"
#include "RiaPolyArcLineSampler.h"
#include "RiaSCurveCalculator.h"

#include "RigWellPath.h"

#include "RimModeledWellPath.h"
#include "RimProject.h"
#include "RimWellPathGroup.h"
#include "RimWellPathTarget.h"

#include "RiuViewerCommands.h"

#include "cafCmdFeatureMenuBuilder.h"
#include "cafPdmFieldScriptingCapabilityCvfVec3d.h"
#include "cafPdmObjectScriptingCapability.h"
#include "cafPdmUiDoubleSliderEditor.h"
#include "cafPdmUiDoubleValueEditor.h"
#include "cafPdmUiLineEditor.h"
#include "cafPdmUiPushButtonEditor.h"
#include "cafPdmUiTableViewEditor.h"
#include "cafPdmUiTreeOrdering.h"
#include "cvfGeometryTools.h"

CAF_PDM_SOURCE_INIT( RimWellPathLateralGeometryDef, "WellPathLateralGeometryDef", "WellPathLateralGeometry" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RiaLineArcWellPathCalculator::WellTarget> createTargetsFromPoints( const std::vector<cvf::Vec3d>& points )
{
    CAF_ASSERT( points.size() >= 2u );

    std::vector<RiaLineArcWellPathCalculator::WellTarget> targets;

    for ( size_t i = 0; i < points.size(); ++i )
    {
        cvf::Vec3d tangent;
        if ( i < points.size() - 1u )
        {
            tangent = points[i + 1] - points[i];
        }
        else if ( i > 0u )
        {
            tangent = points[i] - points[i - 1];
        }
        RiaOffshoreSphericalCoords sphericalCoords( tangent );

        RiaLineArcWellPathCalculator::WellTarget target =
            { points[i], true, sphericalCoords.azi(), sphericalCoords.inc(), 0.0, 0.0 };
        targets.push_back( target );
    }
    return targets;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPathLateralGeometryDef::RimWellPathLateralGeometryDef()
    : changed( this )
    , m_pickTargetsEventHandler( new RicCreateWellTargetsPickEventHandler( this ) )
{
    CAF_PDM_InitScriptableObjectWithNameAndComment( "Well Targets",
                                                    ":/WellTargets.png",
                                                    "",
                                                    "",
                                                    "WellPathLateralGeometry",
                                                    "Class containing the geometry of a modeled Well Path Lateral" );

    this->setUi3dEditorTypeName( RicWellPathGeometry3dEditor::uiEditorTypeName() );

    CAF_PDM_InitScriptableField( &m_connectionMdOnParentWellPath, "MdAtConnection", 0.0, "MD at Well Path Connection", "", "", "" );
    m_connectionMdOnParentWellPath.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );

    // Temporarily disable changing of MD. It doesn't work right without also altering angles.
    m_connectionMdOnParentWellPath.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitScriptableFieldNoDefault( &m_wellTargets, "WellPathTargets", "Well Targets", "", "", "" );
    m_wellTargets.uiCapability()->setUiEditorTypeName( caf::PdmUiTableViewEditor::uiEditorTypeName() );
    m_wellTargets.uiCapability()->setUiTreeChildrenHidden( true );
    m_wellTargets.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::TOP );
    m_wellTargets.uiCapability()->setCustomContextMenuEnabled( true );

    CAF_PDM_InitField( &m_pickPointsEnabled, "m_pickPointsEnabled", false, "", "", "", "" );
    caf::PdmUiPushButtonEditor::configureEditorForField( &m_pickPointsEnabled );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPathLateralGeometryDef::~RimWellPathLateralGeometryDef()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellPathLateralGeometryDef::mdAtConnection() const
{
    return m_connectionMdOnParentWellPath;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathLateralGeometryDef::setMdAtConnection( double md )
{
    m_connectionMdOnParentWellPath = md;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RimWellPathLateralGeometryDef::anchorPointXyz() const
{
    CAF_ASSERT( m_parentGeometry.notNull() );
    return m_parentGeometry->interpolatedPointAlongWellPath( m_connectionMdOnParentWellPath );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathLateralGeometryDef::createTargetAtConnectionPoint( const cvf::Vec3d& tangent )
{
    auto target = appendTarget();
    target->setAsPointXYZAndTangentTarget( cvf::Vec3d::ZERO, tangent );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathLateralGeometryDef::setParentGeometry( const RigWellPath* parentGeometry )
{
    m_parentGeometry = parentGeometry;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<RigWellPath> RimWellPathLateralGeometryDef::createWellPathGeometry()
{
    CAF_ASSERT( m_parentGeometry.notNull() );

    cvf::ref<RigWellPath> wellPathLateralGeometry = new RigWellPath;

    RiaLineArcWellPathCalculator wellPathCalculator = lineArcWellPathCalculator();

    auto [allWellPathPoints, allMeasuredDepths] =
        m_parentGeometry->clippedPointSubset( m_parentGeometry->measuredDepths().front(), m_connectionMdOnParentWellPath );
    auto originalSize = allWellPathPoints.size();

    if ( wellPathCalculator.lineArcEndpoints().size() >= 2 )
    {
        RiaPolyArcLineSampler arcLineSampler( wellPathCalculator.startTangent(), wellPathCalculator.lineArcEndpoints() );
        auto [wellPathPoints, measuredDepths] = arcLineSampler.sampledPointsAndMDs( 30, false );
        allWellPathPoints.insert( allWellPathPoints.end(), wellPathPoints.begin(), wellPathPoints.end() );
        std::transform( measuredDepths.begin(),
                        measuredDepths.end(),
                        std::back_inserter( allMeasuredDepths ),
                        [this]( double md ) { return md + this->m_connectionMdOnParentWellPath; } );
    }
    wellPathLateralGeometry->setWellPathPoints( allWellPathPoints );
    wellPathLateralGeometry->setMeasuredDepths( allMeasuredDepths );
    wellPathLateralGeometry->setUniqueStartIndex( originalSize );

    return wellPathLateralGeometry;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RiaWellPlanCalculator::WellPlanSegment> RimWellPathLateralGeometryDef::wellPlan() const
{
    RiaLineArcWellPathCalculator wellPathCalculator = lineArcWellPathCalculator();

    RiaWellPlanCalculator wpCalc( wellPathCalculator.startTangent(), wellPathCalculator.lineArcEndpoints() );

    return wpCalc.wellPlan();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathLateralGeometryDef::updateWellPathVisualization( bool fullUpdate )
{
    changed.send( fullUpdate );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<RimWellPathTarget*, RimWellPathTarget*>
    RimWellPathLateralGeometryDef::findActiveTargetsAroundInsertionPoint( const RimWellPathTarget* targetToInsertBefore )
{
    RimWellPathTarget* before = nullptr;
    RimWellPathTarget* after  = nullptr;

    bool foundTarget = false;
    for ( const auto& wt : m_wellTargets )
    {
        if ( wt == targetToInsertBefore )
        {
            foundTarget = true;
        }

        if ( wt->isEnabled() && !after && foundTarget ) after = wt;

        if ( wt->isEnabled() && !foundTarget ) before = wt;
    }

    return { before, after };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathLateralGeometryDef::insertTarget( const RimWellPathTarget* targetToInsertBefore,
                                                  RimWellPathTarget*       targetToInsert )
{
    size_t index = m_wellTargets.index( targetToInsertBefore );
    if ( index < m_wellTargets.size() )
        m_wellTargets.insert( index, targetToInsert );
    else
        m_wellTargets.push_back( targetToInsert );

    targetToInsert->moved.connect( this, &RimWellPathLateralGeometryDef::onTargetMoved );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathLateralGeometryDef::deleteTarget( RimWellPathTarget* targetTodelete )
{
    m_wellTargets.removeChildObject( targetTodelete );
    delete targetTodelete;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathLateralGeometryDef::deleteAllTargets()
{
    m_wellTargets.deleteAllChildObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPathTarget* RimWellPathLateralGeometryDef::appendTarget()
{
    RimWellPathTarget* wellPathTarget = nullptr;

    auto targets = m_wellTargets.childObjects();
    if ( targets.empty() )
    {
        wellPathTarget = new RimWellPathTarget;
    }
    else
    {
        wellPathTarget = dynamic_cast<RimWellPathTarget*>(
            targets.back()->xmlCapability()->copyByXmlSerialization( caf::PdmDefaultObjectFactory::instance() ) );
    }

    if ( wellPathTarget )
    {
        m_wellTargets.push_back( wellPathTarget );
    }
    wellPathTarget->moved.connect( this, &RimWellPathLateralGeometryDef::onTargetMoved );
    return wellPathTarget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RimWellPathTarget* RimWellPathLateralGeometryDef::firstActiveTarget() const
{
    for ( const RimWellPathTarget* target : m_wellTargets )
    {
        if ( target->isEnabled() )
        {
            return target;
        }
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RimWellPathTarget* RimWellPathLateralGeometryDef::lastActiveTarget() const
{
    if ( !m_wellTargets.size() ) return nullptr;

    for ( int tIdx = static_cast<int>( m_wellTargets.size() - 1 ); tIdx >= 0; --tIdx )
    {
        if ( m_wellTargets[tIdx]->isEnabled() )
        {
            return m_wellTargets[tIdx];
        }
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathLateralGeometryDef::enableTargetPointPicking( bool isEnabling )
{
    m_pickPointsEnabled = isEnabling;
    this->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathLateralGeometryDef::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                                      const QVariant&            oldValue,
                                                      const QVariant&            newValue )
{
    if ( changedField == &m_pickPointsEnabled )
    {
        this->updateConnectedEditors();
    }

    updateWellPathVisualization( false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathLateralGeometryDef::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_connectionMdOnParentWellPath );
    uiOrdering.add( &m_wellTargets );
    uiOrdering.add( &m_pickPointsEnabled );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathLateralGeometryDef::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName )
{
    uiTreeOrdering.skipRemainingChildren( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathLateralGeometryDef::initAfterRead()
{
    RimWellPathGroup* group = nullptr;
    this->firstAncestorOrThisOfTypeAsserted( group );
    this->setParentGeometry( group->wellPathGeometry() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWellPathTarget*> RimWellPathLateralGeometryDef::activeWellTargets() const
{
    std::vector<RimWellPathTarget*> active;

    for ( const auto& wt : m_wellTargets )
    {
        if ( wt->targetType() != RimWellPathTarget::LATERAL_ANCHOR_POINT_MD && wt->isEnabled() )
        {
            active.push_back( wt );
        }
    }

    return active;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaLineArcWellPathCalculator RimWellPathLateralGeometryDef::lineArcWellPathCalculator() const
{
    std::vector<RiaLineArcWellPathCalculator::WellTarget> targetDatas;

    auto [pointVector, measuredDepths] = m_parentGeometry->clippedPointSubset( 0.0, m_connectionMdOnParentWellPath );
    cvf::Vec3d connectionPoint         = anchorPointXyz();

    auto N = pointVector.size();
    if ( N >= 2u )
    {
        targetDatas =
            createTargetsFromPoints( { pointVector[N - 2] - connectionPoint, pointVector[N - 1] - connectionPoint } );
    }

    std::vector<RimWellPathTarget*> activeTargets = activeWellTargets();

    for ( auto wellTarget : activeTargets )
    {
        targetDatas.push_back( wellTarget->wellTargetData() );
    }

    RiaLineArcWellPathCalculator wellPathCalculator( connectionPoint, targetDatas );

    const std::vector<RiaLineArcWellPathCalculator::WellTargetStatus>& targetStatuses =
        wellPathCalculator.targetStatuses();

    for ( size_t tIdx = 0; tIdx < activeTargets.size(); ++tIdx )
    {
        activeTargets[tIdx]->flagRadius1AsIncorrect( targetStatuses[tIdx].isRadius1Editable, false, 0 );
        activeTargets[tIdx]->flagRadius2AsIncorrect( targetStatuses[tIdx].isRadius2Editable, false, 0 );

        if ( targetStatuses[tIdx].hasDerivedTangent )
        {
            activeTargets[tIdx]->setDerivedTangent( targetStatuses[tIdx].resultAzimuth,
                                                    targetStatuses[tIdx].resultInclination );
        }

        if ( targetStatuses[tIdx].hasOverriddenRadius1 )
        {
            activeTargets[tIdx]->flagRadius1AsIncorrect( targetStatuses[tIdx].isRadius1Editable,
                                                         true,
                                                         targetStatuses[tIdx].resultRadius1 );
        }

        if ( targetStatuses[tIdx].hasOverriddenRadius2 )
        {
            activeTargets[tIdx]->flagRadius2AsIncorrect( targetStatuses[tIdx].isRadius2Editable,
                                                         true,
                                                         targetStatuses[tIdx].resultRadius2 );
        }
    }

    return wellPathCalculator;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathLateralGeometryDef::onTargetMoved( const caf::SignalEmitter* moved, bool fullUpdate )
{
    changed.send( fullUpdate );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathLateralGeometryDef::defineCustomContextMenu( const caf::PdmFieldHandle* fieldNeedingMenu,
                                                             QMenu*                     menu,
                                                             QWidget*                   fieldEditorWidget )
{
    caf::CmdFeatureMenuBuilder menuBuilder;

    menuBuilder << "RicNewWellPathListTargetFeature";
    menuBuilder << "Separator";
    menuBuilder << "RicDeleteWellPathTargetFeature";

    menuBuilder.appendToMenu( menu );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathLateralGeometryDef::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                           QString                    uiConfigName,
                                                           caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_pickPointsEnabled )
    {
        caf::PdmUiPushButtonEditorAttribute* pbAttribute = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*>( attribute );
        if ( pbAttribute )
        {
            if ( !m_pickPointsEnabled )
            {
                pbAttribute->m_buttonText = "Start Picking Targets";
            }
            else
            {
                pbAttribute->m_buttonText = "Stop Picking Targets";
            }
        }
    }
    else if ( field = &m_connectionMdOnParentWellPath )
    {
        auto myAttr = dynamic_cast<caf::PdmUiDoubleSliderEditorAttribute*>( attribute );
        if ( myAttr )
        {
            myAttr->m_minimum = m_parentGeometry->uniqueMeasuredDepths().front();
            myAttr->m_maximum = m_parentGeometry->uniqueMeasuredDepths().back();
        }
    }
    else if ( field == &m_wellTargets )
    {
        auto tvAttribute = dynamic_cast<caf::PdmUiTableViewEditorAttribute*>( attribute );
        if ( tvAttribute )
        {
            tvAttribute->resizePolicy = caf::PdmUiTableViewEditorAttribute::RESIZE_TO_FIT_CONTENT;

            if ( m_pickPointsEnabled )
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
void RimWellPathLateralGeometryDef::defineObjectEditorAttribute( QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    RicWellPathGeometry3dEditorAttribute* attrib = dynamic_cast<RicWellPathGeometry3dEditorAttribute*>( attribute );
    if ( attrib )
    {
        attrib->pickEventHandler = m_pickTargetsEventHandler;
        attrib->enablePicking    = m_pickPointsEnabled;
    }
}
