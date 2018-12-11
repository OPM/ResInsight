/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     equinor ASA
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

#include "RimUserDefinedPolylinesAnnotation.h"

#include "WellPathCommands/PointTangentManipulator/RicPolyline3dEditor.h"
#include "WellPathCommands/RicPolylineTargetsPickEventHandler.h"

#include "RimAnnotationCollection.h"
#include "RimAnnotationLineAppearance.h"
#include "RimPolylineTarget.h"

#include "RigPolyLinesData.h"

#include "RiuViewerCommands.h"

#include "cvfBoundingBox.h"

#include "cafPdmUiTableViewEditor.h"
#include "cafCmdFeatureMenuBuilder.h"
#include "cafPdmUiPushButtonEditor.h"
#include "cafPdmUiTreeOrdering.h"

//--------------------------------------------------------------------------------------------------
/// Internal function
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3d> xydToXyzVector(const std::vector<cvf::Vec3d>& xyds)
{
    std::vector<cvf::Vec3d> xyzs;
    for (const auto& xyd : xyds)
    {
        auto xyz = xyd;
        xyz.z() = -xyd.z();
        xyzs.push_back(xyz);
    }
    return xyzs;
}


CAF_PDM_SOURCE_INIT(RimUserDefinedPolylinesAnnotation, "UserDefinedPolylinesAnnotation");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimUserDefinedPolylinesAnnotation::RimUserDefinedPolylinesAnnotation()
    : m_pickTargetsEventHandler(new RicPolylineTargetsPickEventHandler(this))
{
    CAF_PDM_InitObject("PolyLines Annotation", ":/PolylinesFromFile16x16.png", "", "");

    CAF_PDM_InitField(&m_enablePicking, "EnablePicking", false, "", "", "", "");
    caf::PdmUiPushButtonEditor::configureEditorForField(&m_enablePicking);
    m_enablePicking.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::LabelPosType::HIDDEN);

    CAF_PDM_InitFieldNoDefault(&m_targets, "Targets", "Targets", "", "", "");
    m_targets.uiCapability()->setUiEditorTypeName(caf::PdmUiTableViewEditor::uiEditorTypeName());
    //m_targets.uiCapability()->setUiTreeHidden(true);
    m_targets.uiCapability()->setUiTreeChildrenHidden(true);
    m_targets.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::TOP);
    m_targets.uiCapability()->setCustomContextMenuEnabled(true);

    this->setUi3dEditorTypeName(RicPolyline3dEditor::uiEditorTypeName());

}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimUserDefinedPolylinesAnnotation::~RimUserDefinedPolylinesAnnotation()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<RigPolyLinesData> RimUserDefinedPolylinesAnnotation::polyLinesData()
{
    cvf::ref<RigPolyLinesData> pld = new RigPolyLinesData;
    std::vector<cvf::Vec3d> line;
    std::vector<std::vector<cvf::Vec3d> > lines;
    for (const RimPolylineTarget* target : m_targets)
    {
        line.push_back(target->targetPointXYZ());
    }
    pld->setPolyLines({ line });

    return pld;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimPolylineTarget*> RimUserDefinedPolylinesAnnotation::activeTargets() const
{
    return m_targets.childObjects();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimUserDefinedPolylinesAnnotation::isEmpty()
{
    return m_targets.empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimUserDefinedPolylinesAnnotation::appendTarget(const cvf::Vec3d& defaultPos)
{
    RimPolylineTarget* target = nullptr;

    auto targets = m_targets.childObjects();
    if (targets.empty())
    {
        target = new RimPolylineTarget();
        target->setAsPointXYZ(defaultPos);
    }
    else
    {
        target = dynamic_cast<RimPolylineTarget*>(
            targets.back()->xmlCapability()->copyByXmlSerialization(caf::PdmDefaultObjectFactory::instance()));
    }

    if (target)
    {
        m_targets.push_back(target);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimUserDefinedPolylinesAnnotation::insertTarget(const RimPolylineTarget* targetToInsertBefore,
                                                     RimPolylineTarget*       targetToInsert)
{
    size_t index = m_targets.index(targetToInsertBefore);
    if (index < m_targets.size())
        m_targets.insert(index, targetToInsert);
    else
        m_targets.push_back(targetToInsert);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimUserDefinedPolylinesAnnotation::deleteTarget(RimPolylineTarget* targetTodelete)
{
    m_targets.removeChildObject(targetTodelete);
    delete targetTodelete;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<RimPolylineTarget*, RimPolylineTarget*>
    RimUserDefinedPolylinesAnnotation::findActiveTargetsAroundInsertionPoint(const RimPolylineTarget* targetToInsertBefore)
{
    RimPolylineTarget* before = nullptr;
    RimPolylineTarget* after  = nullptr;

    bool foundTarget = false;
    for (const auto& wt : m_targets)
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
void RimUserDefinedPolylinesAnnotation::updateVisualization()
{
    RimAnnotationCollection* annColl = nullptr;
    this->firstAncestorOrThisOfTypeAsserted(annColl);

    annColl->scheduleRedrawOfRelevantViews();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimUserDefinedPolylinesAnnotation::enablePicking(bool enable)
{
    m_enablePicking = enable;
    if (enable)
    {
        RiuViewerCommands::setPickEventHandler(m_pickTargetsEventHandler);
    }
    else
    {
        RiuViewerCommands::removePickEventHandlerIfActive(m_pickTargetsEventHandler);
    }
    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimUserDefinedPolylinesAnnotation::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    uiOrdering.add(&m_targets);
    uiOrdering.add(&m_enablePicking);

    auto appearanceGroup = uiOrdering.addNewGroup("Line Appearance");
    appearance()->uiOrdering(uiConfigName, *appearanceGroup);

    uiOrdering.skipRemainingFields(true);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimUserDefinedPolylinesAnnotation::defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName)
{
    uiTreeOrdering.skipRemainingChildren(true);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimUserDefinedPolylinesAnnotation::fieldChangedByUi(const caf::PdmFieldHandle* changedField,
                                                         const QVariant&            oldValue,
                                                         const QVariant&            newValue)
{
    if (changedField == &m_enablePicking)
    {
        enablePicking(m_enablePicking);
    }

    updateVisualization();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimUserDefinedPolylinesAnnotation::defineCustomContextMenu(const caf::PdmFieldHandle* fieldNeedingMenu,
                                                                QMenu*                     menu,
                                                                QWidget*                   fieldEditorWidget)
{
    caf::CmdFeatureMenuBuilder menuBuilder;

    menuBuilder << "RicNewPolylineTargetFeature";
    menuBuilder << "Separator";
    menuBuilder << "RicDeletePolylineTargetFeature";

    menuBuilder.appendToMenu(menu);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimUserDefinedPolylinesAnnotation::defineEditorAttribute(const caf::PdmFieldHandle* field,
                                                              QString                    uiConfigName,
                                                              caf::PdmUiEditorAttribute* attribute)
{
    if (field == &m_enablePicking)
    {
        auto* pbAttribute = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*>(attribute);
        if (pbAttribute)
        {
            if (!m_enablePicking)
            {
                pbAttribute->m_buttonText = "Start Picking Points";
            }
            else
            {
                pbAttribute->m_buttonText = "Stop Picking Points";
            }
        }
    }

    if (field == &m_targets)
    {
        auto tvAttribute = dynamic_cast<caf::PdmUiTableViewEditorAttribute*>(attribute);
        if (tvAttribute)
        {
            tvAttribute->resizePolicy = caf::PdmUiTableViewEditorAttribute::RESIZE_TO_FIT_CONTENT;

            if (m_enablePicking)
            {
                tvAttribute->baseColor.setRgb(255, 220, 255);
                tvAttribute->alwaysEnforceResizePolicy = true;
            }
        }
    }
}
