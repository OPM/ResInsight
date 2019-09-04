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

#include "WellPathCommands/RicCreateWellTargetsPickEventHandler.h"

#include "RiaFieldHandleTools.h"
#include "RiaJCurveCalculator.h"
#include "RiaLogging.h"
#include "RiaOffshoreSphericalCoords.h"
#include "RiaPolyArcLineSampler.h"
#include "RiaSCurveCalculator.h"

#include "RigWellPath.h"

#include "RimModeledWellPath.h"
#include "RimWellPathTarget.h"

#include "RiuViewerCommands.h"

#include "WellPathCommands/PointTangentManipulator/RicWellPathGeometry3dEditor.h"
#include "cafCmdFeatureMenuBuilder.h"
#include "cafPdmUiPushButtonEditor.h"
#include "cafPdmUiTableViewEditor.h"
#include "cafPdmUiTreeOrdering.h"
#include "cvfGeometryTools.h"

namespace caf
{
template<>
void caf::AppEnum<RimWellPathGeometryDef::WellStartType>::setUp()
{
    addItem(RimWellPathGeometryDef::START_AT_FIRST_TARGET, "START_AT_FIRST_TARGET", "Start at First Target");
    addItem(RimWellPathGeometryDef::START_AT_SURFACE, "START_AT_SURFACE", "Start at Surface");
    addItem(RimWellPathGeometryDef::START_FROM_OTHER_WELL, "START_FROM_OTHER_WELL", "Branch");
    addItem(RimWellPathGeometryDef::START_AT_AUTO_SURFACE, "START_AT_AUTO_SURFACE", "Auto Surface");

    setDefault(RimWellPathGeometryDef::START_AT_FIRST_TARGET);
}
}

CAF_PDM_SOURCE_INIT(RimWellPathGeometryDef, "WellPathGeometryDef");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPathGeometryDef::RimWellPathGeometryDef()
    : m_pickTargetsEventHandler(new RicCreateWellTargetsPickEventHandler(this))
{
    CAF_PDM_InitObject("Well Targets", ":/WellTargets.png", "", "");

    this->setUi3dEditorTypeName(RicWellPathGeometry3dEditor::uiEditorTypeName());
    CAF_PDM_InitField(&m_referencePointUtmXyd, "ReferencePosUtmXyd", cvf::Vec3d(0, 0, 0), "UTM Reference Point", "", "", "");

    CAF_PDM_InitField(&m_mdrkbAtFirstTarget, "MdrkbAtFirstTarget", 0.0, "MDRKB at First Target", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_wellTargets, "WellPathTargets", "Well Targets", "", "", "");
    m_wellTargets.uiCapability()->setUiEditorTypeName(caf::PdmUiTableViewEditor::uiEditorTypeName());
    // m_wellTargets.uiCapability()->setUiTreeHidden(true);
    m_wellTargets.uiCapability()->setUiTreeChildrenHidden(true);
    m_wellTargets.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::TOP);
    m_wellTargets.uiCapability()->setCustomContextMenuEnabled(true);

    CAF_PDM_InitField(&m_pickPointsEnabled, "m_pickPointsEnabled", false, "", "", "", "");
    caf::PdmUiPushButtonEditor::configureEditorForField(&m_pickPointsEnabled);

    // Temp conversion field.
    CAF_PDM_InitField(&m_referencePointXyz_OBSOLETE, "ReferencePos", cvf::Vec3d(0, 0, 0), "UTM Reference Point", "", "", "");
    RiaFieldhandleTools::disableWriteAndSetFieldHidden(&m_referencePointXyz_OBSOLETE);

    /// To be removed ?

    CAF_PDM_InitFieldNoDefault(&m_wellStartType, "WellStartType", "Start Type", "", "", "");
    m_wellStartType.xmlCapability()->disableIO();
    CAF_PDM_InitFieldNoDefault(&m_parentWell, "ParentWell", "Parent Well", "", "", "");
    m_parentWell.xmlCapability()->disableIO();
    CAF_PDM_InitField(&m_kickoffDepthOrMD, "KickoffDepthOrMD", 100.0, "Kickoff Depth", "", "", "");
    m_kickoffDepthOrMD.xmlCapability()->disableIO();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPathGeometryDef::~RimWellPathGeometryDef() {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RimWellPathGeometryDef::referencePointXyz() const
{
    cvf::Vec3d xyz(m_referencePointUtmXyd());
    xyz.z() = -xyz.z();
    return xyz;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathGeometryDef::setReferencePointXyz(const cvf::Vec3d& refPointXyz)
{
    cvf::Vec3d xyd(refPointXyz);
    xyd.z()                = -xyd.z();
    m_referencePointUtmXyd = xyd;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellPathGeometryDef::mdrkbAtFirstTarget() const
{
    return m_mdrkbAtFirstTarget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathGeometryDef::setMdrkbAtFirstTarget(double mdrkb)
{
    m_mdrkbAtFirstTarget = mdrkb;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<RigWellPath> RimWellPathGeometryDef::createWellPathGeometry()
{
    cvf::ref<RigWellPath> wellPathGeometry = new RigWellPath;

    RiaLineArcWellPathCalculator wellPathCalculator = lineArcWellPathCalculator();

    if (wellPathCalculator.lineArcEndpoints().size() < 2) return wellPathGeometry;

    RiaPolyArcLineSampler arcLineSampler(wellPathCalculator.startTangent(), wellPathCalculator.lineArcEndpoints());

    arcLineSampler.sampledPointsAndMDs(30, false, &(wellPathGeometry->m_wellPathPoints), &(wellPathGeometry->m_measuredDepths));

    return wellPathGeometry;
}
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RiaWellPlanCalculator::WellPlanSegment> RimWellPathGeometryDef::wellPlan() const
{
    RiaLineArcWellPathCalculator wellPathCalculator = lineArcWellPathCalculator();

    RiaWellPlanCalculator wpCalc(wellPathCalculator.startTangent(), wellPathCalculator.lineArcEndpoints());

    return wpCalc.wellPlan();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathGeometryDef::updateWellPathVisualization()
{
    RimModeledWellPath* modWellPath;
    this->firstAncestorOrThisOfTypeAsserted(modWellPath);
    modWellPath->updateWellPathVisualization();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<RimWellPathTarget*, RimWellPathTarget*>
    RimWellPathGeometryDef::findActiveTargetsAroundInsertionPoint(const RimWellPathTarget* targetToInsertBefore)
{
    RimWellPathTarget* before = nullptr;
    RimWellPathTarget* after  = nullptr;

    bool foundTarget = false;
    for (const auto& wt : m_wellTargets)
    {
        if (wt == targetToInsertBefore)
        {
            foundTarget = true;
        }

        if (wt->isEnabled() && !after && foundTarget) after = wt;

        if (wt->isEnabled() && !foundTarget) before = wt;
    }

    return {before, after};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathGeometryDef::insertTarget(const RimWellPathTarget* targetToInsertBefore, RimWellPathTarget* targetToInsert)
{
    size_t index = m_wellTargets.index(targetToInsertBefore);
    if (index < m_wellTargets.size())
        m_wellTargets.insert(index, targetToInsert);
    else
        m_wellTargets.push_back(targetToInsert);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathGeometryDef::deleteTarget(RimWellPathTarget* targetTodelete)
{
    m_wellTargets.removeChildObject(targetTodelete);
    delete targetTodelete;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathGeometryDef::appendTarget()
{
    RimWellPathTarget* wellPathTarget = nullptr;

    auto targets = m_wellTargets.childObjects();
    if (targets.empty())
    {
        wellPathTarget = new RimWellPathTarget;
    }
    else
    {
        wellPathTarget = dynamic_cast<RimWellPathTarget*>(
            targets.back()->xmlCapability()->copyByXmlSerialization(caf::PdmDefaultObjectFactory::instance()));
    }

    if (wellPathTarget)
    {
        m_wellTargets.push_back(wellPathTarget);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RimWellPathTarget* RimWellPathGeometryDef::firstActiveTarget() const
{
    for (const RimWellPathTarget* target : m_wellTargets)
    {
        if (target->isEnabled())
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
    if (!m_wellTargets.size()) return nullptr;

    for (int tIdx = static_cast<int>(m_wellTargets.size() - 1); tIdx >= 0; --tIdx)
    {
        if (m_wellTargets[tIdx]->isEnabled())
        {
            return m_wellTargets[tIdx];
        }
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathGeometryDef::enableTargetPointPicking(bool isEnabling)
{
    m_pickPointsEnabled = isEnabling;
    this->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimWellPathGeometryDef::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions,
                                                                            bool*                      useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options;

    if (fieldNeedingOptions == &m_wellStartType)
    {
        options.push_back(caf::PdmOptionItemInfo("Start at First Target", RimWellPathGeometryDef::START_AT_FIRST_TARGET));
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathGeometryDef::fieldChangedByUi(const caf::PdmFieldHandle* changedField,
                                              const QVariant&            oldValue,
                                              const QVariant&            newValue)
{
    if (&m_referencePointUtmXyd == changedField)
    {
        std::cout << "fieldChanged" << std::endl;
    }
    else if (changedField == &m_pickPointsEnabled)
    {
        this->updateConnectedEditors();
    }

    updateWellPathVisualization();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathGeometryDef::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    uiOrdering.add(&m_wellStartType);
    if (m_wellStartType == START_FROM_OTHER_WELL)
    {
        uiOrdering.add(&m_parentWell);
        m_kickoffDepthOrMD.uiCapability()->setUiName("Measured Depth");
        uiOrdering.add(&m_kickoffDepthOrMD);
    }

    if (m_wellStartType == START_AT_SURFACE)
    {
        m_kickoffDepthOrMD.uiCapability()->setUiName("Kick-Off Depth");
        uiOrdering.add(&m_kickoffDepthOrMD);
    }

    uiOrdering.add(&m_referencePointUtmXyd);
    uiOrdering.add(&m_mdrkbAtFirstTarget);
    uiOrdering.add(&m_wellTargets);
    uiOrdering.add(&m_pickPointsEnabled);
    uiOrdering.skipRemainingFields(true);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathGeometryDef::defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName)
{
    uiTreeOrdering.skipRemainingChildren(true);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWellPathTarget*> RimWellPathGeometryDef::activeWellTargets() const
{
    std::vector<RimWellPathTarget*> active;
    for (const auto& wt : m_wellTargets)
    {
        if (wt->isEnabled())
        {
            active.push_back(wt);
        }
    }

    return active;
}

#if 0 // Kept for reference a bit longer
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3d> RimWellPathGeometryDef::lineArcEndpoints() const
{
    double prevSegmentEndAzi = 0;
    double prevSegmentEndInc = 0;

    std::vector<RimWellPathTarget*> activeWellPathTargets = activeWellTargets();
    
    CVF_ASSERT(activeWellPathTargets.size() > 1);

    std::vector<cvf::Vec3d> endPoints;
    endPoints.push_back(  activeWellPathTargets[0]->targetPointXYZ() + referencePointXyz() );

    for ( size_t tIdx = 0; tIdx < activeWellPathTargets.size() - 1; ++tIdx)
    {
        RimWellPathTarget* target1 = activeWellPathTargets[tIdx];
        RimWellPathTarget* target2 = activeWellPathTargets[tIdx+1];

        // Ignore targets in the same place
        if ((target1->targetPointXYZ() - target2->targetPointXYZ()).length() < 1e-6) continue;

        target1->flagRadius2AsIncorrect(false, 0);
        target2->flagRadius1AsIncorrect(false, 0);

        if (   target1->targetType() == RimWellPathTarget::POINT_AND_TANGENT
            && target2->targetType() == RimWellPathTarget::POINT_AND_TANGENT)
        {
            RiaSCurveCalculator sCurveCalc(target1->targetPointXYZ(),
                                           target1->azimuth(),
                                           target1->inclination(),
                                           target1->radius2(),
                                           target2->targetPointXYZ(),
                                           target2->azimuth(),
                                           target2->inclination(),
                                           target2->radius1());

            if ( sCurveCalc.solveStatus() != RiaSCurveCalculator::CONVERGED )
            {
                double p1p2Length = (target2->targetPointXYZ() - target1->targetPointXYZ()).length();
                sCurveCalc = RiaSCurveCalculator::fromTangentsAndLength(target1->targetPointXYZ(),
                                                                        target1->azimuth(),
                                                                        target1->inclination(),
                                                                        0.2*p1p2Length,
                                                                        target2->targetPointXYZ(),
                                                                        target2->azimuth(),
                                                                        target2->inclination(),
                                                                        0.2*p1p2Length);

                RiaLogging::warning("Using fall-back calculation of well path geometry between active target number: " + QString::number(tIdx+1) + " and " + QString::number(tIdx+2));

                target1->flagRadius2AsIncorrect(true, sCurveCalc.firstRadius());
                target2->flagRadius1AsIncorrect(true, sCurveCalc.secondRadius());
            }

            endPoints.push_back( sCurveCalc.firstArcEndpoint() + referencePointXyz() );
            endPoints.push_back( sCurveCalc.secondArcStartpoint() + referencePointXyz() );
            endPoints.push_back( target2->targetPointXYZ() + referencePointXyz() );

        }
        else if (   target1->targetType() == RimWellPathTarget::POINT
                 && target2->targetType() == RimWellPathTarget::POINT_AND_TANGENT)
        {
            RiaSCurveCalculator sCurveCalc(target1->targetPointXYZ(),
                                           prevSegmentEndAzi,
                                           prevSegmentEndInc,
                                           target1->radius2(),
                                           target2->targetPointXYZ(),
                                           target2->azimuth(),
                                           target2->inclination(),
                                           target2->radius1());

            if ( sCurveCalc.solveStatus() != RiaSCurveCalculator::CONVERGED )
            {
                double p1p2Length = (target2->targetPointXYZ() - target1->targetPointXYZ()).length();
                sCurveCalc = RiaSCurveCalculator::fromTangentsAndLength(target1->targetPointXYZ(),
                                                                        prevSegmentEndAzi,
                                                                        prevSegmentEndInc,
                                                                        0.2*p1p2Length,
                                                                        target2->targetPointXYZ(),
                                                                        target2->azimuth(),
                                                                        target2->inclination(),
                                                                        0.2*p1p2Length);

                RiaLogging::warning("Using fall-back calculation of well path geometry between active target number: " + QString::number(tIdx+1) + " and " + QString::number(tIdx+2));

                target1->flagRadius2AsIncorrect(true, sCurveCalc.firstRadius());
                target2->flagRadius1AsIncorrect(true, sCurveCalc.secondRadius());
            }

            endPoints.push_back( sCurveCalc.firstArcEndpoint() + referencePointXyz() );
            endPoints.push_back( sCurveCalc.secondArcStartpoint() + referencePointXyz() );
            endPoints.push_back( target2->targetPointXYZ() + referencePointXyz() );
        }
        else if (   target1->targetType() == RimWellPathTarget::POINT_AND_TANGENT
                 && target2->targetType() == RimWellPathTarget::POINT)
        {
            RiaJCurveCalculator jCurve(target1->targetPointXYZ(),
                                       target1->azimuth(),
                                       target1->inclination(),
                                       target1->radius2(),
                                       target2->targetPointXYZ());

            if ( jCurve.curveStatus() == RiaJCurveCalculator::OK )
            {
                endPoints.push_back(jCurve.firstArcEndpoint() + referencePointXyz());
            }
            else if ( jCurve.curveStatus() == RiaJCurveCalculator::FAILED_RADIUS_TOO_LARGE )
            {
                target1->flagRadius2AsIncorrect(true, jCurve.radius());
            }
            
            endPoints.push_back( target2->targetPointXYZ() + referencePointXyz() );
            prevSegmentEndAzi = jCurve.endAzimuth();
            prevSegmentEndInc = jCurve.endInclination();

            target2->setDerivedTangent(prevSegmentEndAzi, prevSegmentEndInc);
        }

        else if (   target1->targetType() == RimWellPathTarget::POINT
                 && target2->targetType() == RimWellPathTarget::POINT)
        {
            RiaJCurveCalculator jCurve(target1->targetPointXYZ(),
                                       prevSegmentEndAzi,
                                       prevSegmentEndInc,
                                       target1->radius2(),
                                       target2->targetPointXYZ());

            if ( jCurve.curveStatus() == RiaJCurveCalculator::OK )
            {
                endPoints.push_back(jCurve.firstArcEndpoint() + referencePointXyz());
            }
            else if ( jCurve.curveStatus() == RiaJCurveCalculator::FAILED_RADIUS_TOO_LARGE )
            {
                target1->flagRadius2AsIncorrect(true, jCurve.radius());
            }

            endPoints.push_back( target2->targetPointXYZ() + referencePointXyz() );
            prevSegmentEndAzi = jCurve.endAzimuth();
            prevSegmentEndInc = jCurve.endInclination();
            
            target2->setDerivedTangent(prevSegmentEndAzi, prevSegmentEndInc);
        }
        else
        {
            CVF_ASSERT(false);
        }
    }

    return endPoints;
}

#endif

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaLineArcWellPathCalculator RimWellPathGeometryDef::lineArcWellPathCalculator() const
{
    std::vector<RimWellPathTarget*> wellTargets = activeWellTargets();

    std::vector<RiaLineArcWellPathCalculator::WellTarget> targetDatas;

    for (auto wellTarget : wellTargets)
    {
        targetDatas.push_back(wellTarget->wellTargetData());
    }

    RiaLineArcWellPathCalculator                                       wellPathCalculator(referencePointXyz(), targetDatas);
    const std::vector<RiaLineArcWellPathCalculator::WellTargetStatus>& targetStatuses = wellPathCalculator.targetStatuses();

    for (size_t tIdx = 0; tIdx < wellTargets.size(); ++tIdx)
    {
        wellTargets[tIdx]->flagRadius1AsIncorrect(false, 0);
        wellTargets[tIdx]->flagRadius2AsIncorrect(false, 0);

        if (targetStatuses[tIdx].hasDerivedTangent)
        {
            wellTargets[tIdx]->setDerivedTangent(targetStatuses[tIdx].resultAzimuth, targetStatuses[tIdx].resultInclination);
        }

        if (targetStatuses[tIdx].hasOverriddenRadius1)
        {
            wellTargets[tIdx]->flagRadius1AsIncorrect(true, targetStatuses[tIdx].resultRadius1);
        }

        if (targetStatuses[tIdx].hasOverriddenRadius2)
        {
            wellTargets[tIdx]->flagRadius2AsIncorrect(true, targetStatuses[tIdx].resultRadius2);
        }
    }

    return wellPathCalculator;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathGeometryDef::defineCustomContextMenu(const caf::PdmFieldHandle* fieldNeedingMenu,
                                                     QMenu*                     menu,
                                                     QWidget*                   fieldEditorWidget)
{
    caf::CmdFeatureMenuBuilder menuBuilder;

    menuBuilder << "RicNewWellPathListTargetFeature";
    menuBuilder << "Separator";
    menuBuilder << "RicDeleteWellPathTargetFeature";

    menuBuilder.appendToMenu(menu);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathGeometryDef::defineEditorAttribute(const caf::PdmFieldHandle* field,
                                                   QString                    uiConfigName,
                                                   caf::PdmUiEditorAttribute* attribute)
{
    if (field == &m_pickPointsEnabled)
    {
        caf::PdmUiPushButtonEditorAttribute* pbAttribute = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*>(attribute);
        if (pbAttribute)
        {
            if (!m_pickPointsEnabled)
            {
                pbAttribute->m_buttonText = "Start Picking Targets";
            }
            else
            {
                pbAttribute->m_buttonText = "Stop Picking Targets";
            }
        }
    }

    if (field == &m_wellTargets)
    {
        auto tvAttribute = dynamic_cast<caf::PdmUiTableViewEditorAttribute*>(attribute);
        if (tvAttribute)
        {
            tvAttribute->resizePolicy = caf::PdmUiTableViewEditorAttribute::RESIZE_TO_FIT_CONTENT;

            if (m_pickPointsEnabled)
            {
                tvAttribute->baseColor.setRgb(255, 220, 255);
                tvAttribute->alwaysEnforceResizePolicy = true;
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathGeometryDef::defineObjectEditorAttribute(QString uiConfigName, caf::PdmUiEditorAttribute* attribute)
{
    RicWellPathGeometry3dEditorAttribute* attrib = dynamic_cast<RicWellPathGeometry3dEditorAttribute*>(attribute);
    if (attrib)
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
    // To be removed before release 2018.11

    if (m_referencePointXyz_OBSOLETE != cvf::Vec3d::ZERO && m_referencePointUtmXyd == cvf::Vec3d::ZERO)
    {
        m_referencePointUtmXyd = cvf::Vec3d(
            m_referencePointXyz_OBSOLETE().x(), m_referencePointXyz_OBSOLETE().y(), -m_referencePointXyz_OBSOLETE().z());
    }
}
