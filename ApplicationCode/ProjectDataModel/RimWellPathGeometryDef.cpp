/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018 -    equinor ASA
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

#include "cafCmdFeatureMenuBuilder.h"
#include "cafPdmUiTableViewEditor.h"
#include "cafPdmUiTreeOrdering.h"

#include "RigWellPath.h"
#include "RiaPolyArcLineSampler.h"
#include "RimWellPathTarget.h"
#include "RimModeledWellPath.h"
#include "RiaSCurveCalculator.h"
#include "RiaLogging.h"

namespace caf
{
template<>
void caf::AppEnum< RimWellPathGeometryDef::WellStartType >::setUp()
{
    addItem(RimWellPathGeometryDef::START_AT_FIRST_TARGET, "START_AT_FIRST_TARGET",   "Start at First Target");
    addItem(RimWellPathGeometryDef::START_AT_SURFACE,      "START_AT_SURFACE",         "Start at Surface");
    addItem(RimWellPathGeometryDef::START_FROM_OTHER_WELL, "START_FROM_OTHER_WELL",   "Branch");
    addItem(RimWellPathGeometryDef::START_AT_AUTO_SURFACE, "START_AT_AUTO_SURFACE",   "Auto Surface");

    setDefault(RimWellPathGeometryDef::START_AT_FIRST_TARGET);
}
}

CAF_PDM_SOURCE_INIT(RimWellPathGeometryDef, "WellPathGeometryDef");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellPathGeometryDef::RimWellPathGeometryDef()
{
    CAF_PDM_InitObject("Trajectory", ":/Well.png", "", "");

    CAF_PDM_InitFieldNoDefault(&m_wellStartType, "WellStartType", "Start Type", "", "", "");
    CAF_PDM_InitField(&m_referencePoint, "ReferencePos", cvf::Vec3d(0,0,0), "UTM Reference Point", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_parentWell, "ParentWell", "Parent Well", "", "", "");
    CAF_PDM_InitField(&m_kickoffDepthOrMD, "KickoffDepthOrMD", 100.0, "Kickoff Depth", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_wellTargets, "WellPathTargets", "Well Targets", "", "", "");
    m_wellTargets.uiCapability()->setUiEditorTypeName(caf::PdmUiTableViewEditor::uiEditorTypeName());
    //m_wellTargets.uiCapability()->setUiTreeHidden(true);
    m_wellTargets.uiCapability()->setUiTreeChildrenHidden(true);
    m_wellTargets.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::TOP);
    m_wellTargets.uiCapability()->setCustomContextMenuEnabled(true);


    m_wellTargets.push_back(new RimWellPathTarget());
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
cvf::ref<RigWellPath> RimWellPathGeometryDef::createWellPathGeometry()
{
    cvf::ref<RigWellPath> wellPathGeometry = new RigWellPath; 
    
    if (activeWellTargets().size() < 2) return wellPathGeometry;

    RiaPolyArcLineSampler arcLineSampler(startTangent(),  lineArcEndpoints());

    arcLineSampler.sampledPointsAndMDs(30,
                                         false,
                                         &(wellPathGeometry->m_wellPathPoints),
                                         &(wellPathGeometry->m_measuredDepths));

    return wellPathGeometry;
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
void RimWellPathGeometryDef::insertTarget(RimWellPathTarget* targetToInsertBefore, RimWellPathTarget* targetToInsert)
{
   size_t index = m_wellTargets.index(targetToInsertBefore);
   if (index < m_wellTargets.size()) m_wellTargets.insert(index, targetToInsert);
   else m_wellTargets.push_back(targetToInsert);
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
        wellPathTarget = dynamic_cast<RimWellPathTarget*>(targets.back()->xmlCapability()->copyByXmlSerialization(caf::PdmDefaultObjectFactory::instance()));
    }
        
    if (wellPathTarget)
    {
        m_wellTargets.push_back(wellPathTarget);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimWellPathGeometryDef::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, 
                                                                             bool* useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options;

    if (fieldNeedingOptions == &m_wellStartType)
    {
        options.push_back(caf::PdmOptionItemInfo("Start at First Target",RimWellPathGeometryDef::START_AT_FIRST_TARGET  ));
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPathGeometryDef::fieldChangedByUi(const caf::PdmFieldHandle* changedField, 
                                              const QVariant& oldValue, 
                                              const QVariant& newValue)
{
    if (&m_referencePoint == changedField)
    {
        std::cout << "fieldChanged" << std::endl;
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

    uiOrdering.add(&m_referencePoint);
    uiOrdering.add(&m_wellTargets);
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

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3d> RimWellPathGeometryDef::lineArcEndpoints() const
{
    std::vector<RimWellPathTarget*> activeWellPathTargets = activeWellTargets();
    
    CVF_ASSERT(activeWellPathTargets.size() > 1);

    std::vector<cvf::Vec3d> endPoints;
    endPoints.push_back(  activeWellPathTargets[0]->targetPointXYZ() + m_referencePoint() );

    for ( size_t tIdx = 0; tIdx < activeWellPathTargets.size() - 1; ++tIdx)
    {
        RimWellPathTarget* target1 = activeWellPathTargets[tIdx];
        RimWellPathTarget* target2 = activeWellPathTargets[tIdx+1];

        if (target1->targetType() == RimWellPathTarget::POINT_AND_TANGENT
            && target2->targetType() == RimWellPathTarget::POINT_AND_TANGENT)
        {
            RiaSCurveCalculator sCurveCalc(target1->targetPointXYZ(),
                                           target1->azimuth(),
                                           target1->inclination(),
                                           115,//30.0/cvf::Math::toRadians(12.0),
                                           target2->targetPointXYZ(),
                                           target2->azimuth(),
                                           target2->inclination(),
                                           115);//30.0/cvf::Math::toRadians(12.0));

            if (!sCurveCalc.isOk())
            {
                RiaLogging::warning("SCurve Calculation failed between target " + QString::number(tIdx+1) + " and " + QString::number(tIdx+2));
                double p1p2Length = (target2->targetPointXYZ() - target1->targetPointXYZ()).length();
                sCurveCalc = RiaSCurveCalculator::fromTangentsAndLength(target1->targetPointXYZ(),
                                                                        target1->azimuth(),
                                                                        target1->inclination(),
                                                                        0.2*p1p2Length,
                                                                        target2->targetPointXYZ(),
                                                                        target2->azimuth(),
                                                                        target2->inclination(),
                                                                        0.2*p1p2Length);
            }
            endPoints.push_back( sCurveCalc.firstArcEndpoint() + m_referencePoint() );
            endPoints.push_back( sCurveCalc.secondArcStartpoint() + m_referencePoint() );
        }

        endPoints.push_back( target2->targetPointXYZ() + m_referencePoint() );
    }

    return endPoints;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RimWellPathGeometryDef::startTangent() const
{
    std::vector<RimWellPathTarget*> wellTargets = activeWellTargets();

    if (!wellTargets.empty() && wellTargets[0]->targetType() == RimWellPathTarget::POINT_AND_TANGENT)
    {
        return wellTargets[0]->tangent();
    }
    else
    {
        return { 0, 0, -1 };
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPathGeometryDef::defineCustomContextMenu(const caf::PdmFieldHandle* fieldNeedingMenu, 
                                                     QMenu* menu, 
                                                     QWidget* fieldEditorWidget)
{
    caf::CmdFeatureMenuBuilder menuBuilder;
    
    menuBuilder << "RicNewWellPathListTargetFeature";
    menuBuilder << "Separator";
    menuBuilder << "RicDeleteWellPathTargetFeature";

    menuBuilder.appendToMenu(menu);
}
