/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RimCrossSection.h"

#include "RiaApplication.h"

#include "RicCommandFeature.h"

#include "RigSimulationWellCenterLineCalculator.h"

#include "RimCase.h"
#include "RimEclipseView.h"
#include "RimEclipseWell.h"
#include "RimEclipseWellCollection.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimView.h"
#include "RimWellPath.h"

#include "RiuViewer.h"
#include "RivCrossSectionPartMgr.h"

#include "cafCmdFeature.h"
#include "cafCmdFeatureManager.h"
#include "cafPdmUiPushButtonEditor.h"


namespace caf {

template<>
void caf::AppEnum< RimCrossSection::CrossSectionEnum >::setUp()
{
    addItem(RimCrossSection::CS_WELL_PATH,       "CS_WELL_PATH",       "Well Path");
    addItem(RimCrossSection::CS_SIMULATION_WELL, "CS_SIMULATION_WELL", "Simulation Well");
    addItem(RimCrossSection::CS_POLYLINE,        "CS_POLYLINE",        "Polyline");
    setDefault(RimCrossSection::CS_WELL_PATH);
}

template<>
void caf::AppEnum< RimCrossSection::CrossSectionDirEnum >::setUp()
{
    addItem(RimCrossSection::CS_VERTICAL,   "CS_VERTICAL",      "Vertical");
    addItem(RimCrossSection::CS_HORIZONTAL, "CS_HORIZONTAL",    "Horizontal");
    setDefault(RimCrossSection::CS_VERTICAL);
}

} 


CAF_PDM_SOURCE_INIT(RimCrossSection, "CrossSection");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimCrossSection::RimCrossSection()
{
    CAF_PDM_InitObject("Intersection", ":/CrossSection16x16.png", "", "");

    CAF_PDM_InitField(&name,        "UserDescription",  QString("Intersection Name"), "Name", "", "", "");
    CAF_PDM_InitField(&isActive,    "Active",           true, "Active", "", "", "");
    isActive.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&type,           "Type",                "Type", "", "", "");
    CAF_PDM_InitFieldNoDefault(&direction,      "Direction",           "Direction", "", "", "");
    CAF_PDM_InitFieldNoDefault(&wellPath,       "WellPath",            "Well Path        ", "", "", "");
    CAF_PDM_InitFieldNoDefault(&simulationWell, "SimulationWell",      "Simulation Well", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_userPolyline, "Points",              "Points", "", "", "");
    CAF_PDM_InitField         (&m_branchIndex,  "Branch",          -1, "Branch", "", "", "");
    CAF_PDM_InitField         (&m_extentLength, "ExtentLength", 200.0, "Extent length", "", "", "");
    CAF_PDM_InitField         (&showInactiveCells, "ShowInactiveCells", false, "Inactive Cells", "", "", "");

    CAF_PDM_InitFieldNoDefault(&inputFromViewerEnabled, "m_activateUiAppendPointsCommand", "", "", "", "");
    inputFromViewerEnabled.xmlCapability()->setIOWritable(false);
    inputFromViewerEnabled.xmlCapability()->setIOReadable(false);
    inputFromViewerEnabled.uiCapability()->setUiEditorTypeName(caf::PdmUiPushButtonEditor::uiEditorTypeName());
    inputFromViewerEnabled.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);

    inputFromViewerEnabled = false;

    uiCapability()->setUiChildrenHidden(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCrossSection::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (changedField == &isActive ||
        changedField == &type ||
        changedField == &direction ||
        changedField == &wellPath ||
        changedField == &simulationWell ||
        changedField == &m_branchIndex ||
        changedField == &m_extentLength ||
        changedField == &showInactiveCells)
    {
        rebuildGeometryAndScheduleCreateDisplayModel();
    }

    if (changedField == &simulationWell 
        || changedField == &isActive 
        || changedField == &type)
    {
        m_wellBranchCenterlines.clear();
        updateWellCenterline();
        m_branchIndex = -1;
    }


    if (changedField == &simulationWell 
        || changedField == &wellPath
        || changedField == &m_branchIndex)
    {
        updateName();
    }

    if (changedField == &inputFromViewerEnabled
        || changedField == &m_userPolyline)
    {
        rebuildGeometryAndScheduleCreateDisplayModel();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCrossSection::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    uiOrdering.add(&name);
    caf::PdmUiGroup* geometryGroup = uiOrdering.addNewGroup("Intersecting Geometry");
    geometryGroup->add(&type);

    if (type == CS_WELL_PATH)
    {
        geometryGroup->add(&wellPath);
    }
    else if (type == CS_SIMULATION_WELL)
    {
        geometryGroup->add(&simulationWell);
        updateWellCenterline();
        if (simulationWell() && m_wellBranchCenterlines.size() > 1)
        {
            geometryGroup->add(&m_branchIndex);
        }
    }
    else if (type == CS_POLYLINE)
    {
        geometryGroup->add(&m_userPolyline);
        geometryGroup->add(&inputFromViewerEnabled);
    }

    caf::PdmUiGroup* optionsGroup = uiOrdering.addNewGroup("Options");

    optionsGroup->add(&direction);
    optionsGroup->add(&m_extentLength);
    optionsGroup->add(&showInactiveCells);

    if (type == CS_POLYLINE)
    {
        m_extentLength.uiCapability()->setUiReadOnly(true);
    }
    else
    {
        m_extentLength.uiCapability()->setUiReadOnly(false);
    }

    updateWellExtentDefaultValue();

    uiOrdering.setForgetRemainingFields(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimCrossSection::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options;

    if (fieldNeedingOptions == &wellPath)
    {
        RimProject* proj = RiaApplication::instance()->project();
        if (proj->activeOilField()->wellPathCollection())
        {
            caf::PdmChildArrayField<RimWellPath*>& wellPaths = proj->activeOilField()->wellPathCollection()->wellPaths;
            
            QIcon wellIcon(":/Well.png");
            for (size_t i = 0; i < wellPaths.size(); i++)
            {
                options.push_back(caf::PdmOptionItemInfo(wellPaths[i]->name(), QVariant::fromValue(caf::PdmPointer<caf::PdmObjectHandle>(wellPaths[i])), false, wellIcon));
            }
        }

        if (options.size() == 0)
        {
            options.push_front(caf::PdmOptionItemInfo("None", QVariant::fromValue(caf::PdmPointer<caf::PdmObjectHandle>(NULL))));
        }
    }
    else if (fieldNeedingOptions == &simulationWell)
    {
        RimEclipseWellCollection* coll = simulationWellCollection();
        if (coll)
        {
            caf::PdmChildArrayField<RimEclipseWell*>& eclWells = coll->wells;

            QIcon simWellIcon(":/Well.png");
            for (size_t i = 0; i < eclWells.size(); i++)
            {
                options.push_back(caf::PdmOptionItemInfo(eclWells[i]->name(), QVariant::fromValue(caf::PdmPointer<caf::PdmObjectHandle>(eclWells[i])), false, simWellIcon));
            }
        }

        if (options.size() == 0)
        {
            options.push_front(caf::PdmOptionItemInfo("None", QVariant::fromValue(caf::PdmPointer<caf::PdmObjectHandle>(NULL))));
        }
    }
    else if (fieldNeedingOptions == &m_branchIndex)
    {
        updateWellCenterline();

        size_t branchCount = m_wellBranchCenterlines.size();
        
        options.push_back(caf::PdmOptionItemInfo("All", -1));

        for (int bIdx = 0; bIdx < branchCount; ++bIdx)
        {
            options.push_back(caf::PdmOptionItemInfo(QString::number(bIdx + 1), QVariant::fromValue(bIdx)));
        }
    }
    return options;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimCrossSection::userDescriptionField()
{
    return &name;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimCrossSection::objectToggleField()
{
    return &isActive;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseWellCollection* RimCrossSection::simulationWellCollection()
{
    RimEclipseView* eclipseView = NULL;
    firstAnchestorOrThisOfType(eclipseView);

    if (eclipseView)
    {
        return eclipseView->wellCollection;
    }

    return NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector< std::vector <cvf::Vec3d> > RimCrossSection::polyLines() const
{
    std::vector< std::vector <cvf::Vec3d> > lines;
    if (type == CS_WELL_PATH)
    {
        if (wellPath())
        {
            lines.push_back(wellPath->wellPathGeometry()->m_wellPathPoints);
            clipToReservoir(lines[0]);
        }
    }
    else if (type == CS_SIMULATION_WELL)
    {
        if (simulationWell())
        {
            updateWellCenterline();

            if (0 <= m_branchIndex && m_branchIndex < m_wellBranchCenterlines.size())
            {
                lines.push_back(m_wellBranchCenterlines[m_branchIndex]);
            }

            if (m_branchIndex == -1)
            {
                lines = m_wellBranchCenterlines;
            }
        }
    }
    else if (type == CS_POLYLINE)
    {
        lines.push_back(m_userPolyline);
    }

    if (type == CS_WELL_PATH || type == CS_SIMULATION_WELL)
    {
        for (int lIdx = 0; lIdx < lines.size(); ++lIdx)
        {
            std::vector<cvf::Vec3d>& polyLine = lines[lIdx];
            addExtents(polyLine);
        }
    }

    return lines;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivCrossSectionPartMgr* RimCrossSection::crossSectionPartMgr()
{
    if (m_crossSectionPartMgr.isNull()) m_crossSectionPartMgr = new RivCrossSectionPartMgr(this);

    return m_crossSectionPartMgr.p();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCrossSection::updateWellCenterline() const
{
    if (isActive() && type == CS_SIMULATION_WELL && simulationWell())
    {
        if (m_wellBranchCenterlines.size() == 0)
        {
            std::vector< std::vector <RigWellResultPoint> > pipeBranchesCellIds;

            RigSimulationWellCenterLineCalculator::calculateWellPipeCenterline(simulationWell(), m_wellBranchCenterlines, pipeBranchesCellIds);
        }
    }
    else
    {
        m_wellBranchCenterlines.clear();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCrossSection::addExtents(std::vector<cvf::Vec3d> &polyLine) const
{
    size_t lineVxCount = polyLine.size();
    
    if (lineVxCount == 0) return;

    // Add extent at end of well
    {
        size_t endIdxOffset = lineVxCount > 3 ? 3: lineVxCount;
        cvf::Vec3d endDirection = (polyLine[lineVxCount-1] - polyLine[lineVxCount-endIdxOffset]);
        endDirection[2] = 0; // Remove z. make extent lenght be horizontally
        if (endDirection.length() < 1e-2)
        {
            endDirection = polyLine.back() - polyLine.front();
            endDirection[2] = 0;

            if (endDirection.length() < 1e-2)
            {
                endDirection = cvf::Vec3d::X_AXIS;
            }
        }

        endDirection.normalize();

        cvf::Vec3d newEnd = polyLine.back() + endDirection * m_extentLength();

        polyLine.push_back(newEnd);
    }

    // Add extent at start
    {
        size_t endIdxOffset = lineVxCount > 3 ? 3: lineVxCount-1;
        cvf::Vec3d startDirection = (polyLine[0] - polyLine[endIdxOffset]);
        startDirection[2] = 0; // Remove z. make extent lenght be horizontally
        if (startDirection.length() < 1e-2)
        {
            startDirection = polyLine.front() - polyLine.back();
            startDirection[2] = 0;

            if (startDirection.length() < 1e-2)
            {
                startDirection = -cvf::Vec3d::X_AXIS;
            }
        }

        startDirection.normalize();

        cvf::Vec3d newStart = polyLine.front() + startDirection * m_extentLength();

        polyLine.insert(polyLine.begin(), newStart);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCrossSection::updateWellExtentDefaultValue()
{
    RimCase* ownerCase = NULL;
    firstAnchestorOrThisOfType(ownerCase);

    if (ownerCase)
    {
        cvf::BoundingBox caseBB = ownerCase->activeCellsBoundingBox();
        if (m_extentLength == m_extentLength.defaultValue() && caseBB.radius() < 1000)
        {
            m_extentLength = caseBB.radius() * 0.1;
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCrossSection::updateName()
{
    if (type == CS_SIMULATION_WELL && simulationWell())
    {
        name = simulationWell()->name();
        if (m_branchIndex() != -1)
        { 
            name = name() + " Branch " + QString::number(m_branchIndex() + 1);
        }
    }
    else if (type() == CS_WELL_PATH && wellPath())
    {
        name = wellPath()->name();
    }

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCrossSection::clipToReservoir(std::vector<cvf::Vec3d> &polyLine) const
{
    RimCase* ownerCase = NULL;
    firstAnchestorOrThisOfType(ownerCase);
    
    std::vector<cvf::Vec3d> clippedPolyLine;

    if (ownerCase)
    {
        cvf::BoundingBox caseBB = ownerCase->activeCellsBoundingBox();
        bool hasEnteredReservoirBB = false;
        for (size_t vxIdx = 0 ; vxIdx < polyLine.size(); ++vxIdx)
        {
            if (!caseBB.contains(polyLine[vxIdx]))
            { 
                continue;
            }

            if (!hasEnteredReservoirBB)
            {
                if (vxIdx > 0)
                {
                    // clip line, and add vx to start
                    cvf::Plane topPlane;
                    topPlane.setFromPointAndNormal(caseBB.max(), cvf::Vec3d::Z_AXIS);
                    cvf::Vec3d intersection;
                
                    if (topPlane.intersect(polyLine[vxIdx-1], polyLine[vxIdx], &intersection))
                    {
                        clippedPolyLine.push_back(intersection);
                    }
                }

                hasEnteredReservoirBB = true;
            }

            clippedPolyLine.push_back(polyLine[vxIdx]);
        }
    }
 
    polyLine.swap(clippedPolyLine);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCrossSection::defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute)
{
    if (field == &inputFromViewerEnabled)
    {
        caf::PdmUiPushButtonEditorAttribute* attrib = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*> (attribute);

        if (inputFromViewerEnabled)
        {
            attrib->m_buttonText = "Stop picking points";
        }
        else
        {
            attrib->m_buttonText = "Start picking points";
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCrossSection::appendPointToPolyLine(const cvf::Vec3d& point)
{
    m_userPolyline.v().push_back(point);

    m_userPolyline.uiCapability()->updateConnectedEditors();

    rebuildGeometryAndScheduleCreateDisplayModel();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCrossSection::rebuildGeometryAndScheduleCreateDisplayModel()
{
    m_crossSectionPartMgr = NULL;

    RimView* rimView = NULL;
    this->firstAnchestorOrThisOfType(rimView);
    if (rimView)
    {
        rimView->scheduleCreateDisplayModelAndRedraw();
    }
}

