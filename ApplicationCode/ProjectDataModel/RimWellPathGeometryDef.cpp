/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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

#include "RimWellPathGeometryDef.h"

#include "WellPathCommands/PointTangentManipulator/RicWellPathGeometry3dEditor.h"
#include "WellPathCommands/RicCreateWellTargetsPickEventHandler.h"

#include "RiaApplication.h"
#include "RiaFieldHandleTools.h"
#include "RiaJCurveCalculator.h"
#include "RiaLogging.h"
#include "RiaOffshoreSphericalCoords.h"
#include "RiaPolyArcLineSampler.h"
#include "RiaSCurveCalculator.h"

#include "RigWellPath.h"

#include "RimModeledWellPath.h"
#include "RimProject.h"
#include "RimWellPathTarget.h"

#include "RiuViewerCommands.h"

#include "cafCmdFeatureMenuBuilder.h"
#include "cafPdmUiDoubleValueEditor.h"
#include "cafPdmUiLineEditor.h"
#include "cafPdmUiPushButtonEditor.h"
#include "cafPdmUiTableViewEditor.h"
#include "cafPdmUiTreeOrdering.h"
#include "cvfGeometryTools.h"

namespace caf
{
template <>
void caf::AppEnum<RimWellPathGeometryDef::WellStartType>::setUp()
{
    addItem( RimWellPathGeometryDef::START_AT_FIRST_TARGET, "START_AT_FIRST_TARGET", "Start at First Target" );
    addItem( RimWellPathGeometryDef::START_AT_SURFACE, "START_AT_SURFACE", "Start at Surface" );
    addItem( RimWellPathGeometryDef::START_FROM_OTHER_WELL, "START_FROM_OTHER_WELL", "Branch" );
    addItem( RimWellPathGeometryDef::START_AT_AUTO_SURFACE, "START_AT_AUTO_SURFACE", "Auto Surface" );

    setDefault( RimWellPathGeometryDef::START_AT_SURFACE );
}
} // namespace caf

CAF_PDM_SOURCE_INIT( RimWellPathGeometryDef, "WellPathGeometryDef" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPathGeometryDef::RimWellPathGeometryDef()
    : m_pickTargetsEventHandler( new RicCreateWellTargetsPickEventHandler( this ) )
{
    CAF_PDM_InitObject( "Well Targets", ":/WellTargets.png", "", "" );

    this->setUi3dEditorTypeName( RicWellPathGeometry3dEditor::uiEditorTypeName() );
    CAF_PDM_InitField( &m_referencePointUtmXyd, "ReferencePosUtmXyd", cvf::Vec3d( 0, 0, 0 ), "UTM Reference Point", "", "", "" );

    CAF_PDM_InitField( &m_airGap, "AirGap", 0.0, "Air Gap", "", "", "" );
    m_airGap.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleValueEditor::uiEditorTypeName() );
    CAF_PDM_InitField( &m_mdAtFirstTarget, "MdAtFirstTarget", 0.0, "MD at First Target", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_wellTargets, "WellPathTargets", "Well Targets", "", "", "" );
    m_wellTargets.uiCapability()->setUiEditorTypeName( caf::PdmUiTableViewEditor::uiEditorTypeName() );
    m_wellTargets.uiCapability()->setUiTreeChildrenHidden( true );
    m_wellTargets.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::TOP );
    m_wellTargets.uiCapability()->setCustomContextMenuEnabled( true );

    CAF_PDM_InitField( &m_pickPointsEnabled, "m_pickPointsEnabled", false, "", "", "", "" );
    caf::PdmUiPushButtonEditor::configureEditorForField( &m_pickPointsEnabled );

    CAF_PDM_InitFieldNoDefault( &m_wellStartType, "WellStartType", "Start Type", "", "", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPathGeometryDef::~RimWellPathGeometryDef()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RimWellPathGeometryDef::referencePointXyz() const
{
    cvf::Vec3d xyz( m_referencePointUtmXyd() );
    xyz.z() = -xyz.z();
    return xyz;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathGeometryDef::setReferencePointXyz( const cvf::Vec3d& refPointXyz )
{
    cvf::Vec3d xyd( refPointXyz );
    xyd.z()                = -xyd.z();
    m_referencePointUtmXyd = xyd;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellPathGeometryDef::airGap() const
{
    return m_airGap;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathGeometryDef::setAirGap( double airGap )
{
    m_airGap = airGap;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellPathGeometryDef::mdrkbAtFirstTarget() const
{
    return m_mdAtFirstTarget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathGeometryDef::setMdrkbAtFirstTarget( double mdrkb )
{
    m_mdAtFirstTarget = mdrkb;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<RigWellPath> RimWellPathGeometryDef::createWellPathGeometry()
{
    cvf::ref<RigWellPath> wellPathGeometry = new RigWellPath;

    RiaLineArcWellPathCalculator wellPathCalculator = lineArcWellPathCalculator();

    if ( wellPathCalculator.lineArcEndpoints().size() < 2 ) return wellPathGeometry;

    RiaPolyArcLineSampler arcLineSampler( wellPathCalculator.startTangent(), wellPathCalculator.lineArcEndpoints() );

    arcLineSampler.sampledPointsAndMDs( 30,
                                        false,
                                        &( wellPathGeometry->m_wellPathPoints ),
                                        &( wellPathGeometry->m_measuredDepths ) );

    if ( m_airGap != 0.0 )
    {
        wellPathGeometry->setDatumElevation( m_airGap );
    }

    if ( m_wellStartType == START_AT_SURFACE && !wellPathGeometry->m_wellPathPoints.empty() )
    {
        cvf::Vec3d mslPoint = wellPathGeometry->m_wellPathPoints.front();
        mslPoint.z()        = 0.0;
        double depthDiff    = mslPoint.z() - wellPathGeometry->m_wellPathPoints.front().z();
        if ( std::abs( depthDiff ) > 1.0e-8 )
        {
            CAF_ASSERT( wellPathGeometry->m_wellPathPoints.size() == wellPathGeometry->m_measuredDepths.size() );

            std::vector<cvf::Vec3d> newPoints;
            newPoints.reserve( wellPathGeometry->m_wellPathPoints.size() + 1 );
            std::vector<double> newMds;
            newMds.reserve( wellPathGeometry->m_measuredDepths.size() + 1 );

            newPoints.push_back( mslPoint );
            newMds.push_back( 0.0 );
            for ( size_t i = 0; i < wellPathGeometry->m_wellPathPoints.size(); ++i )
            {
                newPoints.push_back( wellPathGeometry->m_wellPathPoints[i] );
                newMds.push_back( wellPathGeometry->m_measuredDepths[i] + depthDiff );
            }
            wellPathGeometry->m_wellPathPoints.swap( newPoints );
            wellPathGeometry->m_measuredDepths.swap( newMds );
        }
    }

    return wellPathGeometry;
}
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RiaWellPlanCalculator::WellPlanSegment> RimWellPathGeometryDef::wellPlan() const
{
    RiaLineArcWellPathCalculator wellPathCalculator = lineArcWellPathCalculator();

    RiaWellPlanCalculator wpCalc( wellPathCalculator.startTangent(), wellPathCalculator.lineArcEndpoints() );

    return wpCalc.wellPlan();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathGeometryDef::updateWellPathVisualization()
{
    RimModeledWellPath* modWellPath;
    this->firstAncestorOrThisOfTypeAsserted( modWellPath );
    modWellPath->updateWellPathVisualization();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<RimWellPathTarget*, RimWellPathTarget*>
    RimWellPathGeometryDef::findActiveTargetsAroundInsertionPoint( const RimWellPathTarget* targetToInsertBefore )
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

    return {before, after};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathGeometryDef::insertTarget( const RimWellPathTarget* targetToInsertBefore, RimWellPathTarget* targetToInsert )
{
    size_t index = m_wellTargets.index( targetToInsertBefore );
    if ( index < m_wellTargets.size() )
        m_wellTargets.insert( index, targetToInsert );
    else
        m_wellTargets.push_back( targetToInsert );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathGeometryDef::deleteTarget( RimWellPathTarget* targetTodelete )
{
    m_wellTargets.removeChildObject( targetTodelete );
    delete targetTodelete;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathGeometryDef::appendTarget()
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
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RimWellPathTarget* RimWellPathGeometryDef::firstActiveTarget() const
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
const RimWellPathTarget* RimWellPathGeometryDef::lastActiveTarget() const
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
void RimWellPathGeometryDef::enableTargetPointPicking( bool isEnabling )
{
    m_pickPointsEnabled = isEnabling;
    this->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo>
    RimWellPathGeometryDef::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_wellStartType )
    {
        options.push_back( caf::PdmOptionItemInfo( "Start at Surface", RimWellPathGeometryDef::START_AT_SURFACE ) );

        options.push_back(
            caf::PdmOptionItemInfo( "Start at First Target", RimWellPathGeometryDef::START_AT_FIRST_TARGET ) );
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathGeometryDef::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                               const QVariant&            oldValue,
                                               const QVariant&            newValue )
{
    if ( &m_referencePointUtmXyd == changedField )
    {
        std::cout << "fieldChanged" << std::endl;
    }
    else if ( changedField == &m_pickPointsEnabled )
    {
        this->updateConnectedEditors();
    }

    updateWellPathVisualization();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathGeometryDef::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_wellStartType );
    uiOrdering.add( &m_referencePointUtmXyd );
    uiOrdering.add( &m_airGap );
    if ( m_wellStartType == START_AT_FIRST_TARGET )
    {
        uiOrdering.add( &m_mdAtFirstTarget );
    }
    uiOrdering.add( &m_wellTargets );
    uiOrdering.add( &m_pickPointsEnabled );
    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathGeometryDef::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName )
{
    uiTreeOrdering.skipRemainingChildren( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWellPathTarget*> RimWellPathGeometryDef::activeWellTargets() const
{
    std::vector<RimWellPathTarget*> active;
    for ( const auto& wt : m_wellTargets )
    {
        if ( wt->isEnabled() )
        {
            active.push_back( wt );
        }
    }

    return active;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaLineArcWellPathCalculator RimWellPathGeometryDef::lineArcWellPathCalculator() const
{
    std::vector<RimWellPathTarget*> wellTargets = activeWellTargets();

    std::vector<RiaLineArcWellPathCalculator::WellTarget> targetDatas;

    for ( auto wellTarget : wellTargets )
    {
        targetDatas.push_back( wellTarget->wellTargetData() );
    }

    RiaLineArcWellPathCalculator wellPathCalculator( referencePointXyz(), targetDatas );
    const std::vector<RiaLineArcWellPathCalculator::WellTargetStatus>& targetStatuses =
        wellPathCalculator.targetStatuses();

    for ( size_t tIdx = 0; tIdx < wellTargets.size(); ++tIdx )
    {
        wellTargets[tIdx]->flagRadius1AsIncorrect( targetStatuses[tIdx].isRadius1Editable, false, 0 );
        wellTargets[tIdx]->flagRadius2AsIncorrect( targetStatuses[tIdx].isRadius2Editable, false, 0 );

        if ( targetStatuses[tIdx].hasDerivedTangent )
        {
            wellTargets[tIdx]->setDerivedTangent( targetStatuses[tIdx].resultAzimuth,
                                                  targetStatuses[tIdx].resultInclination );
        }

        if ( targetStatuses[tIdx].hasOverriddenRadius1 )
        {
            wellTargets[tIdx]->flagRadius1AsIncorrect( targetStatuses[tIdx].isRadius1Editable,
                                                       true,
                                                       targetStatuses[tIdx].resultRadius1 );
        }

        if ( targetStatuses[tIdx].hasOverriddenRadius2 )
        {
            wellTargets[tIdx]->flagRadius2AsIncorrect( targetStatuses[tIdx].isRadius2Editable,
                                                       true,
                                                       targetStatuses[tIdx].resultRadius2 );
        }
    }

    return wellPathCalculator;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathGeometryDef::defineCustomContextMenu( const caf::PdmFieldHandle* fieldNeedingMenu,
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
void RimWellPathGeometryDef::defineEditorAttribute( const caf::PdmFieldHandle* field,
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

    if ( field == &m_wellTargets )
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

    if ( field == &m_referencePointUtmXyd )
    {
        auto uiDisplayStringAttr = dynamic_cast<caf::PdmUiLineEditorAttributeUiDisplayString*>( attribute );

        if ( uiDisplayStringAttr )
        {
            uiDisplayStringAttr->m_displayString = QString::number( m_referencePointUtmXyd()[0], 'f', 2 ) + " " +
                                                   QString::number( m_referencePointUtmXyd()[1], 'f', 2 ) + " " +
                                                   QString::number( m_referencePointUtmXyd()[2], 'f', 2 );
        }
    }

    if ( field == &m_airGap )
    {
        auto uiDoubleValueEditorAttr = dynamic_cast<caf::PdmUiDoubleValueEditorAttribute*>( attribute );
        if ( uiDoubleValueEditorAttr )
        {
            uiDoubleValueEditorAttr->m_decimals  = 2;
            uiDoubleValueEditorAttr->m_validator = new QDoubleValidator( 0.0, std::numeric_limits<double>::max(), 2 );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathGeometryDef::defineObjectEditorAttribute( QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    RicWellPathGeometry3dEditorAttribute* attrib = dynamic_cast<RicWellPathGeometry3dEditorAttribute*>( attribute );
    if ( attrib )
    {
        attrib->pickEventHandler = m_pickTargetsEventHandler;
        attrib->enablePicking    = m_pickPointsEnabled;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathGeometryDef::initAfterRead()
{
    if ( RiaApplication::instance()->project()->isProjectFileVersionEqualOrOlderThan( "2019.12.1" ) )
    {
        m_wellStartType = START_AT_FIRST_TARGET;
    }
}
