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

#include "RimIntersection.h"

#include "RiaApplication.h"

#include "RigEclipseCaseData.h"
#include "RigWellPath.h"

#include "RimCase.h"
#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimSimWellInView.h"
#include "RimSimWellInViewCollection.h"
#include "RimTools.h"
#include "Rim3dView.h"
#include "RimWellPath.h"

#include "RiuViewer.h"
#include "RivIntersectionPartMgr.h"

#include "cafCmdFeature.h"
#include "cafCmdFeatureManager.h"
#include "cafPdmUiDoubleSliderEditor.h"
#include "cafPdmUiListEditor.h"
#include "cafPdmUiPushButtonEditor.h"

#include "cvfBoundingBox.h"
#include "cvfGeometryTools.h"
#include "cvfPlane.h"
#include "Rim2dIntersectionView.h"


namespace caf {

template<>
void caf::AppEnum< RimIntersection::CrossSectionEnum >::setUp()
{
    addItem(RimIntersection::CS_WELL_PATH,       "CS_WELL_PATH",       "Well Path");
    addItem(RimIntersection::CS_SIMULATION_WELL, "CS_SIMULATION_WELL", "Simulation Well");
    addItem(RimIntersection::CS_POLYLINE,        "CS_POLYLINE",        "Polyline");
    addItem(RimIntersection::CS_AZIMUTHLINE,     "CS_AZIMUTHLINE",     "Azimuth and Dip");
    setDefault(RimIntersection::CS_WELL_PATH);
}

template<>
void caf::AppEnum< RimIntersection::CrossSectionDirEnum >::setUp()
{
    addItem(RimIntersection::CS_VERTICAL,   "CS_VERTICAL",   "Vertical");
    addItem(RimIntersection::CS_HORIZONTAL, "CS_HORIZONTAL", "Horizontal");
    addItem(RimIntersection::CS_TWO_POINTS, "CS_TWO_POINTS", "Defined by Two Points");
    setDefault(RimIntersection::CS_VERTICAL);
}

} 


CAF_PDM_SOURCE_INIT(RimIntersection, "CrossSection");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimIntersection::RimIntersection()
{
    CAF_PDM_InitObject("Intersection", ":/CrossSection16x16.png", "", "");

    CAF_PDM_InitField(&name,        "UserDescription",  QString("Intersection Name"), "Name", "", "", "");
    CAF_PDM_InitField(&isActive,    "Active",           true, "Active", "", "", "");
    isActive.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&type,           "Type",                "Type", "", "", "");
    CAF_PDM_InitFieldNoDefault(&direction,      "Direction",           "Direction", "", "", "");
    CAF_PDM_InitFieldNoDefault(&wellPath,       "WellPath",            "Well Path        ", "", "", "");
    CAF_PDM_InitFieldNoDefault(&simulationWell, "SimulationWell",      "Simulation Well", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_userPolyline, "Points",              "Points", "", "Use Ctrl-C for copy and Ctrl-V for paste", "");

    CAF_PDM_InitField(&m_azimuthAngle, "AzimuthAngle", 0.0, "Azimuth", "", "", "");
    m_azimuthAngle.uiCapability()->setUiEditorTypeName(caf::PdmUiDoubleSliderEditor::uiEditorTypeName());

    CAF_PDM_InitField(&m_dipAngle, "DipAngle", 90.0, "Dip", "", "", "");
    m_dipAngle.uiCapability()->setUiEditorTypeName(caf::PdmUiDoubleSliderEditor::uiEditorTypeName());

    CAF_PDM_InitFieldNoDefault(&m_customExtrusionPoints, "CustomExtrusionPoints", "", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_twoAzimuthPoints, "TwoAzimuthPoints", "Points", "", "Use Ctrl-C for copy and Ctrl-V for paste", "");

    CAF_PDM_InitField         (&m_branchIndex,     "Branch",            -1,     "Branch", "", "", "");
    CAF_PDM_InitField         (&m_extentLength,    "ExtentLength",      200.0,  "Extent Length", "", "", "");
    CAF_PDM_InitField         (&m_lengthUp,        "lengthUp",          1000.0, "Length Up", "", "", "");
    CAF_PDM_InitField         (&m_lengthDown,      "lengthDown",        1000.0, "Length Down", "", "", "");
    
    CAF_PDM_InitField         (&showInactiveCells, "ShowInactiveCells", false, "Show Inactive Cells", "", "", "");

    CAF_PDM_InitFieldNoDefault(&inputPolyLineFromViewerEnabled, "m_activateUiAppendPointsCommand", "", "", "", "");
    caf::PdmUiPushButtonEditor::configureEditorForField(&inputPolyLineFromViewerEnabled);
    inputPolyLineFromViewerEnabled = false;

    CAF_PDM_InitFieldNoDefault(&inputExtrusionPointsFromViewerEnabled, "inputExtrusionPointsFromViewerEnabled", "", "", "", "");
    caf::PdmUiPushButtonEditor::configureEditorForField(&inputExtrusionPointsFromViewerEnabled);
    inputExtrusionPointsFromViewerEnabled = false;

    CAF_PDM_InitFieldNoDefault(&inputTwoAzimuthPointsFromViewerEnabled, "inputTwoAzimuthPointsFromViewerEnabled", "", "", "", "");
    caf::PdmUiPushButtonEditor::configureEditorForField(&inputTwoAzimuthPointsFromViewerEnabled);
    inputTwoAzimuthPointsFromViewerEnabled = false;

    uiCapability()->setUiTreeChildrenHidden(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimIntersection::~RimIntersection()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimIntersection::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (changedField == &isActive ||
        changedField == &type ||
        changedField == &direction ||
        changedField == &wellPath ||
        changedField == &simulationWell ||
        changedField == &m_branchIndex ||
        changedField == &m_extentLength ||
        changedField == &m_lengthUp ||
        changedField == &m_lengthDown ||
        changedField == &showInactiveCells)
    {
        rebuildGeometryAndScheduleCreateDisplayModel();
    }

    if (changedField == &simulationWell 
        || changedField == &isActive 
        || changedField == &type)
    {
        recomputeSimulationWellBranchData();
    }

    if (changedField == &simulationWell 
        || changedField == &wellPath
        || changedField == &m_branchIndex)
    {
        updateName();
    }

    if (changedField == &inputPolyLineFromViewerEnabled
        || changedField == &m_userPolyline)
    {
        if (inputPolyLineFromViewerEnabled)
        {
            inputExtrusionPointsFromViewerEnabled = false;
            inputTwoAzimuthPointsFromViewerEnabled = false;
        }

        rebuildGeometryAndScheduleCreateDisplayModel();
    }

    if (changedField == &inputExtrusionPointsFromViewerEnabled
        || changedField == &m_customExtrusionPoints)
    {
        if (inputExtrusionPointsFromViewerEnabled)
        {
            inputPolyLineFromViewerEnabled = false;
            inputTwoAzimuthPointsFromViewerEnabled = false;
        }

        rebuildGeometryAndScheduleCreateDisplayModel();
    }

    if (changedField == &inputTwoAzimuthPointsFromViewerEnabled
        || changedField == &m_twoAzimuthPoints)
    {
        if (inputTwoAzimuthPointsFromViewerEnabled)
        {
            inputPolyLineFromViewerEnabled = false;
            inputExtrusionPointsFromViewerEnabled = false;
        }

        rebuildGeometryAndScheduleCreateDisplayModel();
    }

    if (changedField == &m_azimuthAngle)
    {
        updateAzimuthLine();
        rebuildGeometryAndScheduleCreateDisplayModel();
    }

    if(changedField == &m_dipAngle)
    {
        rebuildGeometryAndScheduleCreateDisplayModel();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimIntersection::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
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
        updateSimulationWellCenterline();
        if (simulationWell() && m_simulationWellBranchCenterlines.size() > 1)
        {
            geometryGroup->add(&m_branchIndex);
        }
    }
    else if (type == CS_POLYLINE)
    {
        geometryGroup->add(&m_userPolyline);
        geometryGroup->add(&inputPolyLineFromViewerEnabled);
    }
    else if (type == CS_AZIMUTHLINE)
    {
        geometryGroup->add(&m_twoAzimuthPoints);
        geometryGroup->add(&inputTwoAzimuthPointsFromViewerEnabled);
        geometryGroup->add(&m_azimuthAngle);
        geometryGroup->add(&m_dipAngle);
    }

    caf::PdmUiGroup* optionsGroup = uiOrdering.addNewGroup("Options");

    if (type == CS_AZIMUTHLINE)
    {
        optionsGroup->add(&m_lengthUp);
        optionsGroup->add(&m_lengthDown);
    }
    else
    {
        optionsGroup->add(&direction);
        optionsGroup->add(&m_extentLength);
    }

    if (direction == CS_TWO_POINTS)
    {
        optionsGroup->add(&m_customExtrusionPoints);
        optionsGroup->add(&inputExtrusionPointsFromViewerEnabled);
    }

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

    uiOrdering.skipRemainingFields(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimIntersection::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options;

    if (fieldNeedingOptions == &wellPath)
    {
        RimTools::wellPathOptionItems(&options);

        if (options.size() > 0)
        {
            options.push_front(caf::PdmOptionItemInfo("None", nullptr));
        }
    }
    else if (fieldNeedingOptions == &simulationWell)
    {
        RimSimWellInViewCollection* coll = simulationWellCollection();
        if (coll)
        {
            caf::PdmChildArrayField<RimSimWellInView*>& simWells = coll->wells;

            QIcon simWellIcon(":/Well.png");
            for (RimSimWellInView* eclWell : simWells)
            {
                options.push_back(caf::PdmOptionItemInfo(eclWell->name(), eclWell, false, simWellIcon));
            }
        }

        if (options.size() == 0)
        {
            options.push_front(caf::PdmOptionItemInfo("None", nullptr));
        }
    }
    else if (fieldNeedingOptions == &m_branchIndex)
    {
        updateSimulationWellCenterline();

        size_t branchCount = m_simulationWellBranchCenterlines.size();
        
        options.push_back(caf::PdmOptionItemInfo("All", -1));

        for (size_t bIdx = 0; bIdx < branchCount; ++bIdx)
        {
            options.push_back(caf::PdmOptionItemInfo(QString::number(bIdx + 1), QVariant::fromValue(bIdx)));
        }
    }
    return options;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimIntersection::userDescriptionField()
{
    return &name;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimIntersection::objectToggleField()
{
    return &isActive;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSimWellInViewCollection* RimIntersection::simulationWellCollection()
{
    RimEclipseView* eclipseView = nullptr;
    firstAncestorOrThisOfType(eclipseView);

    if (eclipseView)
    {
        return eclipseView->wellCollection;
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimIntersection::updateAzimuthLine()
{
    if (m_twoAzimuthPoints().size() == 2)
    {
        double currentAzimuth = azimuthInRadians(m_twoAzimuthPoints()[1] - m_twoAzimuthPoints()[0]);
        double newAzimuth = cvf::Math::toRadians(m_azimuthAngle);
        double rotAngle = newAzimuth - currentAzimuth;

        cvf::Mat4d rotMat = cvf::Mat4d::fromRotation(-cvf::Vec3d::Z_AXIS, rotAngle);
        cvf::Mat4d transFromOriginMat = cvf::Mat4d::fromTranslation(m_twoAzimuthPoints()[0]);
        cvf::Mat4d transToOriginMat = cvf::Mat4d::fromTranslation(-m_twoAzimuthPoints()[0]);

        m_twoAzimuthPoints.v()[1] = m_twoAzimuthPoints()[1].getTransformedPoint(transFromOriginMat*rotMat*transToOriginMat);

        m_twoAzimuthPoints.uiCapability()->updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector< std::vector <cvf::Vec3d> > RimIntersection::polyLines(double * horizontalLengthAlongWellToPolylineStart) const
{
    CVF_ASSERT(horizontalLengthAlongWellToPolylineStart != nullptr);

    std::vector< std::vector <cvf::Vec3d> > lines;
    double horizontalProjectedLengthAlongWellPathToClipPoint = 0.0;
    if (horizontalLengthAlongWellToPolylineStart)  *horizontalLengthAlongWellToPolylineStart = 0.0;
    if (type == CS_WELL_PATH)
    {
        if (wellPath() && wellPath->wellPathGeometry() )
        {
            lines.push_back(wellPath->wellPathGeometry()->m_wellPathPoints);
            clipToReservoir(lines[0], &horizontalProjectedLengthAlongWellPathToClipPoint);
        }
    }
    else if (type == CS_SIMULATION_WELL)
    {
        if (simulationWell())
        {
            updateSimulationWellCenterline();

            if (0 <= m_branchIndex && m_branchIndex < static_cast<int>(m_simulationWellBranchCenterlines.size()))
            {
                lines.push_back(m_simulationWellBranchCenterlines[m_branchIndex]);
            }

            if (m_branchIndex == -1)
            {
                lines = m_simulationWellBranchCenterlines;
            }
        }
    }
    else if (type == CS_POLYLINE)
    {
        lines.push_back(m_userPolyline);
    }
    else if (type == CS_AZIMUTHLINE)
    {
        lines.push_back(m_twoAzimuthPoints);
    }

    if (type == CS_WELL_PATH || type == CS_SIMULATION_WELL)
    {
        if (type == CS_SIMULATION_WELL && simulationWell())
        {
            cvf::Vec3d top, bottom;

            simulationWell->wellHeadTopBottomPosition(-1, &top, &bottom);

            for ( size_t lIdx = 0; lIdx < lines.size(); ++lIdx )
            {
                std::vector<cvf::Vec3d>& polyLine = lines[lIdx];
                polyLine.insert(polyLine.begin(), top);
            }
        }

        for (size_t lIdx = 0; lIdx < lines.size(); ++lIdx)
        {
            std::vector<cvf::Vec3d>& polyLine = lines[lIdx];
            addExtents(polyLine);
        }

        if (horizontalLengthAlongWellToPolylineStart) 
        {
            *horizontalLengthAlongWellToPolylineStart = horizontalProjectedLengthAlongWellPathToClipPoint - m_extentLength;
        }
    }

    return lines;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivIntersectionPartMgr* RimIntersection::intersectionPartMgr()
{
    if (m_crossSectionPartMgr.isNull()) m_crossSectionPartMgr = new RivIntersectionPartMgr(this);

    return m_crossSectionPartMgr.p();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector< std::vector <cvf::Vec3d> > RimIntersection::polyLinesForExtrusionDirection() const
{
    std::vector< std::vector <cvf::Vec3d> > lines;

    lines.push_back(m_customExtrusionPoints);

    return lines;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimIntersection::updateSimulationWellCenterline() const
{
    if (isActive() && type == CS_SIMULATION_WELL && simulationWell())
    {
        if (m_simulationWellBranchCenterlines.empty())
        {
            auto branches = simulationWell->wellPipeBranches();
            for (const auto& branch : branches)
            {
                m_simulationWellBranchCenterlines.push_back(branch->m_wellPathPoints);
            }
        }
    }
    else
    {
        m_simulationWellBranchCenterlines.clear();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimIntersection::addExtents(std::vector<cvf::Vec3d> &polyLine) const
{
    size_t lineVxCount = polyLine.size();
    
    if (lineVxCount == 0) return;

    // Add extent at end of well
    {
        size_t endIdxOffset = lineVxCount > 3 ? 3: lineVxCount;
        cvf::Vec3d endDirection = (polyLine[lineVxCount-1] - polyLine[lineVxCount-endIdxOffset]);
        endDirection[2] = 0; // Remove z. make extent length be horizontally
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
        startDirection[2] = 0; // Remove z. make extent length be horizontally
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
void RimIntersection::updateWellExtentDefaultValue()
{
    RimCase* ownerCase = nullptr;
    firstAncestorOrThisOfType(ownerCase);

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
void RimIntersection::updateName()
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
void RimIntersection::clipToReservoir(std::vector<cvf::Vec3d> &polyLine, double * horizontalLengthAlongWellToClipPoint) const
{
    CVF_ASSERT(horizontalLengthAlongWellToClipPoint != nullptr);

    *horizontalLengthAlongWellToClipPoint = 0.0;
    RimCase* ownerCase = nullptr;
    firstAncestorOrThisOfType(ownerCase);
    
    std::vector<cvf::Vec3d> clippedPolyLine;

    if (ownerCase)
    {
        cvf::BoundingBox caseBB = ownerCase->activeCellsBoundingBox();
        bool hasEnteredReservoirBB = false;
        for (size_t vxIdx = 0 ; vxIdx < polyLine.size(); ++vxIdx)
        {
            if (!caseBB.contains(polyLine[vxIdx]))
            { 
                if (vxIdx > 0)
                {
                    cvf::Vec3d segment = polyLine[vxIdx] - polyLine[vxIdx-1];
                    segment[2] = 0.0;
                    *horizontalLengthAlongWellToClipPoint += segment.length();
                }
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
                        cvf::Vec3d segment = intersection - polyLine[vxIdx-1];
                        segment[2] = 0.0;
                        *horizontalLengthAlongWellToClipPoint += segment.length();

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
void RimIntersection::setPushButtonText(bool buttonEnable, caf::PdmUiPushButtonEditorAttribute* attribute)
{
    if (attribute)
    {
        if (buttonEnable)
        {
            attribute->m_buttonText = "Stop picking points";
        }
        else
        {
            attribute->m_buttonText = "Start picking points";
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimIntersection::setBaseColor(bool enable, caf::PdmUiListEditorAttribute* attribute)
{
    if (attribute && enable)
    {
        attribute->m_baseColor.setRgb(255, 220, 255);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimIntersection::defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute)
{

    caf::PdmUiDoubleSliderEditorAttribute* doubleSliderAttrib = dynamic_cast<caf::PdmUiDoubleSliderEditorAttribute*>(attribute);
    if (doubleSliderAttrib)
    {
        if (field == &m_azimuthAngle)
        {
            doubleSliderAttrib->m_minimum = 0;
            doubleSliderAttrib->m_maximum = 360;
            doubleSliderAttrib->m_sliderTickCount = 360;
        }
        else if (field == &m_dipAngle)
        {
            doubleSliderAttrib->m_minimum = 0;
            doubleSliderAttrib->m_maximum = 180;
            doubleSliderAttrib->m_sliderTickCount = 180;
        }
    }
    else if (field == &inputPolyLineFromViewerEnabled)
    {
        setPushButtonText(inputPolyLineFromViewerEnabled, dynamic_cast<caf::PdmUiPushButtonEditorAttribute*> (attribute));
    }
    else if (field == &m_userPolyline)
    {
        setBaseColor(inputPolyLineFromViewerEnabled, dynamic_cast<caf::PdmUiListEditorAttribute*>(attribute));
    }
    else if (field == &inputTwoAzimuthPointsFromViewerEnabled)
    {
        setPushButtonText(inputTwoAzimuthPointsFromViewerEnabled, dynamic_cast<caf::PdmUiPushButtonEditorAttribute*> (attribute));
    }
    else if (field == &m_twoAzimuthPoints)
    {
        setBaseColor(inputTwoAzimuthPointsFromViewerEnabled, dynamic_cast<caf::PdmUiListEditorAttribute*>(attribute));
    }
    else if (field == &inputExtrusionPointsFromViewerEnabled)
    {
        setPushButtonText(inputExtrusionPointsFromViewerEnabled, dynamic_cast<caf::PdmUiPushButtonEditorAttribute*> (attribute));
    }
    else if (field == &m_customExtrusionPoints)
    {
        setBaseColor(inputExtrusionPointsFromViewerEnabled, dynamic_cast<caf::PdmUiListEditorAttribute*>(attribute));
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimIntersection::appendPointToPolyLine(const cvf::Vec3d& point)
{
    m_userPolyline.v().push_back(point);

    m_userPolyline.uiCapability()->updateConnectedEditors();

    rebuildGeometryAndScheduleCreateDisplayModel();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Rim2dIntersectionView* RimIntersection::correspondingIntersectionView()
{
    std::vector<caf::PdmObjectHandle*> objects;

    this->objectsWithReferringPtrFields(objects);
    Rim2dIntersectionView* isectView = nullptr;
    for (auto obj : objects)
    {
        isectView = dynamic_cast<Rim2dIntersectionView*>(obj);
        if (isectView) break;
    }
    return isectView;

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimIntersection::appendPointToExtrusionDirection(const cvf::Vec3d& point)
{
    if (m_customExtrusionPoints().size() > 1) m_customExtrusionPoints.v().clear();

    m_customExtrusionPoints.v().push_back(point);

    m_customExtrusionPoints.uiCapability()->updateConnectedEditors();

    rebuildGeometryAndScheduleCreateDisplayModel();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimIntersection::appendPointToAzimuthLine(const cvf::Vec3d& point)
{
    if (m_twoAzimuthPoints().empty())
    {
        m_twoAzimuthPoints.v().push_back(point);
    }
    else if (m_twoAzimuthPoints().size() == 1)
    {
        cvf::Vec3d projectedPoint = cvf::Vec3d(point[0], point[1], m_twoAzimuthPoints.v()[0][2]);
        m_twoAzimuthPoints.v().push_back(projectedPoint);

        m_azimuthAngle = cvf::Math::toDegrees(azimuthInRadians(m_twoAzimuthPoints()[1] - m_twoAzimuthPoints()[0]));
        m_azimuthAngle.uiCapability()->updateConnectedEditors();
    }
    else if (m_twoAzimuthPoints().size() > 1)
    {
        m_twoAzimuthPoints.v().clear();
        m_twoAzimuthPoints.v().push_back(point);
    }

    m_twoAzimuthPoints.uiCapability()->updateConnectedEditors();

    rebuildGeometryAndScheduleCreateDisplayModel();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RimIntersection::extrusionDirection() const
{
    cvf::Vec3d dir = cvf::Vec3d::Z_AXIS;

    if (direction() == RimIntersection::CS_HORIZONTAL)
    {
        std::vector< std::vector <cvf::Vec3d> > lines = this->polyLines();
        if (lines.size() > 0 && lines[0].size() > 1)
        {
            std::vector <cvf::Vec3d> firstLine = lines[0];

            // Use first and last point of polyline to approximate orientation of polyline
            // Then cross with Z axis to find extrusion direction

            cvf::Vec3d polyLineDir = firstLine[firstLine.size() - 1] - firstLine[0];
            cvf::Vec3d up = cvf::Vec3d::Z_AXIS;
            dir = polyLineDir ^ up;
        }
    }
    else if (direction() == RimIntersection::CS_TWO_POINTS && m_customExtrusionPoints().size() > 1)
    {
        dir = m_customExtrusionPoints()[m_customExtrusionPoints().size() - 1] - m_customExtrusionPoints()[0];
    }
    else if (m_twoAzimuthPoints().size() == 2)
    {
        double dipInRad = cvf::Math::toRadians(m_dipAngle);
        
        cvf::Vec3d azimutDirection = m_twoAzimuthPoints()[1] - m_twoAzimuthPoints()[0];
        
        cvf::Mat3d rotMat = cvf::Mat3d::fromRotation(azimutDirection, dipInRad);
        cvf::Vec3d vecPerpToRotVecInHorizontalPlane = azimutDirection ^ cvf::Vec3d::Z_AXIS;

        dir = vecPerpToRotVecInHorizontalPlane.getTransformedVector(rotMat);
    }

    return dir;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RimIntersection::lengthUp() const
{
    return m_lengthUp;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RimIntersection::lengthDown() const
{
    return m_lengthDown;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimIntersection::setLengthDown(double lengthDown)
{
    m_lengthDown = lengthDown;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimIntersection::recomputeSimulationWellBranchData()
{
    if (type() == CS_SIMULATION_WELL)
    {
        m_simulationWellBranchCenterlines.clear();
        updateSimulationWellCenterline();
        m_branchIndex = -1;

        m_crossSectionPartMgr = nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimIntersection::setLengthUp(double lengthUp)
{
    m_lengthUp = lengthUp;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimIntersection::rebuildGeometryAndScheduleCreateDisplayModel()
{
    m_crossSectionPartMgr = nullptr;

    Rim3dView* rimView = nullptr;
    this->firstAncestorOrThisOfType(rimView);
    if (rimView)
    {
        rimView->scheduleCreateDisplayModelAndRedraw();
    }

    Rim2dIntersectionView * iview = correspondingIntersectionView();
    if (iview)
    {
        iview->scheduleGeometryRegen(RivCellSetEnum::ALL_CELLS);
        iview->scheduleCreateDisplayModelAndRedraw();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RimIntersection::azimuthInRadians(cvf::Vec3d vec)
{
    return cvf::GeometryTools::getAngle(-cvf::Vec3d::Z_AXIS, cvf::Vec3d::Y_AXIS, vec);
}

