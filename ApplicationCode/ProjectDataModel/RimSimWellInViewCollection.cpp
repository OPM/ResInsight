/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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

#include "RimSimWellInViewCollection.h"

#include "RiaApplication.h"
#include "RiaColorTables.h"
#include "RiaFieldHandleTools.h"
#include "RiaPreferences.h"

#include "RigEclipseCaseData.h"
#include "RigSimWellData.h"

#include "RimEclipseContourMapView.h"
#include "RimEclipseCase.h"
#include "RimEclipseResultCase.h"
#include "RimEclipseView.h"
#include "RimIntersectionCollection.h"
#include "RimProject.h"
#include "RimSimWellFractureCollection.h"
#include "RimSimWellInView.h"
#include "RimWellAllocationPlot.h"

#include "RiuMainWindow.h"

#include "RivReservoirViewPartMgr.h"

#include "cafPdmUiPushButtonEditor.h"
#include "cafPdmUiCheckBoxTristateEditor.h"


namespace caf
{
    // OBSOLETE enum
    template<>
    void RimSimWellInViewCollection::WellVisibilityEnum::setUp()
    {
        addItem(RimSimWellInViewCollection::PIPES_FORCE_ALL_OFF,       "FORCE_ALL_OFF",      "All Off");
        addItem(RimSimWellInViewCollection::PIPES_INDIVIDUALLY,        "ALL_ON",             "Individual");
        addItem(RimSimWellInViewCollection::PIPES_OPEN_IN_VISIBLE_CELLS,"OPEN_IN_VISIBLE_CELLS", "Visible cells filtered");
        addItem(RimSimWellInViewCollection::PIPES_FORCE_ALL_ON,        "FORCE_ALL_ON",       "All On");
    }
}


namespace caf
{
    // OBSOLETE enum
    template<>
    void RimSimWellInViewCollection::WellCellsRangeFilterEnum::setUp()
    {
        addItem(RimSimWellInViewCollection::RANGE_ADD_NONE,       "FORCE_ALL_OFF",      "All Off");
        addItem(RimSimWellInViewCollection::RANGE_ADD_INDIVIDUAL, "ALL_ON",             "Individually");
        addItem(RimSimWellInViewCollection::RANGE_ADD_ALL,        "FORCE_ALL_ON",       "All On");
    }
}

namespace caf
{
    template<>
    void RimSimWellInViewCollection::WellFenceEnum::setUp()
    {
        addItem(RimSimWellInViewCollection::K_DIRECTION, "K_DIRECTION",    "K - Direction");
        addItem(RimSimWellInViewCollection::J_DIRECTION, "J_DIRECTION",    "J - Direction");
        addItem(RimSimWellInViewCollection::I_DIRECTION, "I_DIRECTION",    "I - Direction");
        setDefault(RimSimWellInViewCollection::K_DIRECTION);
    }
}

namespace caf
{
    template<>
    void RimSimWellInViewCollection::WellHeadPositionEnum::setUp()
    {
        addItem(RimSimWellInViewCollection::WELLHEAD_POS_ACTIVE_CELLS_BB,    "WELLHEAD_POS_ACTIVE_CELLS_BB", "Top of Active Cells");
        addItem(RimSimWellInViewCollection::WELLHEAD_POS_TOP_COLUMN,         "WELLHEAD_POS_TOP_COLUMN",      "Top of Active Cell Column");
        setDefault(RimSimWellInViewCollection::WELLHEAD_POS_TOP_COLUMN);
    }
}

namespace caf
{
    template<>
    void RimSimWellInViewCollection::WellPipeCoordEnum::setUp()
    {
        addItem(RimSimWellInViewCollection::WELLPIPE_INTERPOLATED,    "WELLPIPE_INTERPOLATED",    "Interpolated");
        addItem(RimSimWellInViewCollection::WELLPIPE_CELLCENTER,      "WELLPIPE_CELLCENTER",      "Cell Centers");
        setDefault(RimSimWellInViewCollection::WELLPIPE_INTERPOLATED);
    }
}

namespace caf
{
    template<>
    void RimSimWellInViewCollection::WellPipeColorsEnum::setUp()
    {
        addItem(RimSimWellInViewCollection::WELLPIPE_COLOR_UNIQUE,    "WELLPIPE_COLOR_INDIDUALLY",    "Unique Colors");
        addItem(RimSimWellInViewCollection::WELLPIPE_COLOR_UNIFORM,      "WELLPIPE_COLOR_UNIFORM",      "Uniform Default Color");
        setDefault(RimSimWellInViewCollection::WELLPIPE_COLOR_UNIQUE);
    }
}

CAF_PDM_SOURCE_INIT(RimSimWellInViewCollection, "Wells");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSimWellInViewCollection::RimSimWellInViewCollection()
{
    CAF_PDM_InitObject("Simulation Wells", ":/WellCollection.png", "", "");

    CAF_PDM_InitField(&isActive,              "Active",        true,   "Active", "", "", "");
    isActive.uiCapability()->setUiHidden(true);

    //CAF_PDM_InitField(&showWellsIntersectingVisibleCells, "ShowWellsIntersectingVisibleCells", false, "Hide Wells Not Intersecting Filtered Cells", "", "", "");
    CAF_PDM_InitField(&showWellsIntersectingVisibleCells, "ShowWellsIntersectingVisibleCells", false, "Wells Through Visible Cells Only", "", "", "");
    //CAF_PDM_InitField(&showWellsIntersectingVisibleCells, "ShowWellsIntersectingVisibleCells", false, "Hide Wells Missing Visible Cells", "", "", "");

    // Appearance
    CAF_PDM_InitFieldNoDefault(&m_showWellHead,        "ShowWellHeadTristate",      "Well Head", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_showWellLabel,       "ShowWellLabelTristate",     "Label", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_showWellPipe,        "ShowWellPipe",              "Pipe", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_showWellSpheres,     "ShowWellSpheres",           "Spheres", "", "", "");

    m_showWellHead.uiCapability()->setUiEditorTypeName(caf::PdmUiCheckBoxTristateEditor::uiEditorTypeName());
    m_showWellHead.xmlCapability()->disableIO();

    m_showWellLabel.uiCapability()->setUiEditorTypeName(caf::PdmUiCheckBoxTristateEditor::uiEditorTypeName());
    m_showWellLabel.xmlCapability()->disableIO();

    m_showWellPipe.uiCapability()->setUiEditorTypeName(caf::PdmUiCheckBoxTristateEditor::uiEditorTypeName());
    m_showWellPipe.xmlCapability()->disableIO();

    m_showWellSpheres.uiCapability()->setUiEditorTypeName(caf::PdmUiCheckBoxTristateEditor::uiEditorTypeName());
    m_showWellSpheres.xmlCapability()->disableIO();

    // Scaling
    CAF_PDM_InitField(&wellHeadScaleFactor, "WellHeadScale",            1.0,    "Well Head Scale", "", "", "");
    CAF_PDM_InitField(&pipeScaleFactor,     "WellPipeRadiusScale",      0.1,    "Pipe Radius Scale ", "", "", "");
    CAF_PDM_InitField(&spheresScaleFactor,  "CellCenterSphereScale",    0.2,    "Sphere Radius Scale", "", "", "");

    // Color
    cvf::Color3f defWellLabelColor = RiaApplication::instance()->preferences()->defaultWellLabelColor();
    CAF_PDM_InitField(&wellLabelColor,      "WellLabelColor",   defWellLabelColor, "Label Color",  "", "", "");

    CAF_PDM_InitField(&showConnectionStatusColors, "ShowConnectionStatusColors", true, "Color Pipe Connections", "", "", "");

    cvf::Color3f defaultApplyColor = cvf::Color3f::YELLOW;
    CAF_PDM_InitField(&m_defaultWellPipeColor, "WellColorForApply", defaultApplyColor, "Uniform Well Color", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_wellPipeColors, "WellPipeColors", "Individual Pipe Colors", "", "", "");

    CAF_PDM_InitField(&pipeCrossSectionVertexCount, "WellPipeVertexCount", 12, "Pipe Vertex Count", "", "", "");
    pipeCrossSectionVertexCount.uiCapability()->setUiHidden(true);
    CAF_PDM_InitField(&wellPipeCoordType,           "WellPipeCoordType", WellPipeCoordEnum(WELLPIPE_INTERPOLATED), "Type", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_showWellCells,       "ShowWellCellsTristate", "Show Well Cells", "", "", "");
    m_showWellCells.uiCapability()->setUiEditorTypeName(caf::PdmUiCheckBoxTristateEditor::uiEditorTypeName());
    m_showWellCells.xmlCapability()->disableIO();

    CAF_PDM_InitField(&wellCellFenceType,   "DefaultWellFenceDirection", WellFenceEnum(K_DIRECTION), "Well Fence Direction", "", "", "");

    CAF_PDM_InitField(&wellCellTransparencyLevel, "WellCellTransparency", 0.5, "Well Cell Transparency", "", "", "");
    CAF_PDM_InitField(&isAutoDetectingBranches, "IsAutoDetectingBranches", true, "Branch Detection", "", "Toggle whether the well pipe visualization will try to detect when a part of the well \nis really a branch, and thus is starting from wellhead", "");
    CAF_PDM_InitField(&wellHeadPosition,    "WellHeadPosition", WellHeadPositionEnum(WELLHEAD_POS_TOP_COLUMN), "Well Head Position",  "", "", "");

    CAF_PDM_InitFieldNoDefault(&wells, "Wells", "Wells",  "", "", "");
    wells.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_showWellCellFence, "ShowWellCellFenceTristate", "Show Well Cell Fence", "", "", "");
    m_showWellCellFence.uiCapability()->setUiEditorTypeName(caf::PdmUiCheckBoxTristateEditor::uiEditorTypeName());
    m_showWellCellFence.xmlCapability()->disableIO();

    CAF_PDM_InitField(&obsoleteField_wellPipeVisibility,  "GlobalWellPipeVisibility", WellVisibilityEnum(PIPES_INDIVIDUALLY), "Global well pipe visibility",  "", "", "");
    RiaFieldhandleTools::disableWriteAndSetFieldHidden(&obsoleteField_wellPipeVisibility);
    
    CAF_PDM_InitField(&obsoleteField_wellCellsToRangeFilterMode,  "GlobalWellCellVisibility", WellCellsRangeFilterEnum(RANGE_ADD_INDIVIDUAL),  "Add cells to range filter", "", "", "");
    RiaFieldhandleTools::disableWriteAndSetFieldHidden(&obsoleteField_wellCellsToRangeFilterMode);

    CAF_PDM_InitField(&obsoleteField_showWellHead,              "ShowWellHead",     true, "Show Well Head", "", "", "");
    CAF_PDM_InitField(&obsoleteField_showWellLabel,             "ShowWellLabel",    true, "Show Well Label", "", "", "");
    CAF_PDM_InitField(&obsoleteField_showWellCellFence,         "ShowWellFences",   false,  "Show Well Cell Fence", "", "", "");
    RiaFieldhandleTools::disableWriteAndSetFieldHidden(&obsoleteField_showWellHead);
    RiaFieldhandleTools::disableWriteAndSetFieldHidden(&obsoleteField_showWellLabel);
    RiaFieldhandleTools::disableWriteAndSetFieldHidden(&obsoleteField_showWellCellFence);

    CAF_PDM_InitField(&m_showWellCommunicationLines,         "ShowWellCommunicationLines",   false,  "Communication Lines", "", "", "");

    m_reservoirView = nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSimWellInViewCollection::~RimSimWellInViewCollection()
{
   wells.deleteAllChildObjects();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSimWellInViewCollection::setShowWellCellsState(bool enable)
{
    for (RimSimWellInView* w : wells)
    {
        w->showWellCells = enable;
    }

    updateConnectedEditors();

    if (m_reservoirView)
    {
        m_reservoirView->scheduleGeometryRegen(VISIBLE_WELL_CELLS);
        m_reservoirView->scheduleCreateDisplayModelAndRedraw();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimSimWellInViewCollection::showWellCells()
{
    if (m_showWellCells().isFalse())
    {
        return false;
    }
    else
    {
        return true;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSimWellInView* RimSimWellInViewCollection::findWell(QString name)
{
    for (size_t i = 0; i < this->wells().size(); ++i)
    {
        if (this->wells()[i]->name() == name)
        {
            return this->wells()[i];
        }
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimSimWellInViewCollection::hasVisibleWellCells()
{
    if (!this->isActive()) return false;
    if (this->wells().size() == 0 ) return false;

    bool hasCells = false;
    for (size_t i = 0 ; !hasCells && i < this->wells().size(); ++i)
    {
        RimSimWellInView* well = this->wells()[i];
        if ( well && well->simWellData() && ((well->showWell() && well->showWellCells())) )
        {
            for (size_t tIdx = 0; !hasCells &&  tIdx < well->simWellData()->m_wellCellsTimeSteps.size(); ++tIdx )
            {
                const RigWellResultFrame& wellResultFrame = well->simWellData()->m_wellCellsTimeSteps[tIdx];
                for (size_t wsIdx = 0; !hasCells &&  wsIdx < wellResultFrame.m_wellResultBranches.size(); ++wsIdx)
                {
                    if (wellResultFrame.m_wellResultBranches[wsIdx].m_branchResultPoints.size() > 0  ) hasCells = true; 
                }
            }
        }
    }

    if (!hasCells) return false;

    // Todo: Handle range filter intersection

    return true;
}

//--------------------------------------------------------------------------------------------------
/// Used to know if we need animation of time steps due to the wells
//--------------------------------------------------------------------------------------------------
bool RimSimWellInViewCollection::hasVisibleWellPipes() 
{
    if (!this->isActive()) return false;
    if (this->wells().size() == 0 ) return false;

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSimWellInViewCollection::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (&isActive == changedField)
    {
        this->updateUiIconFromToggleField();

        Rim3dView* view;
        firstAncestorOrThisOfType(view);
        if (view)
        {
            view->hasUserRequestedAnimation = true;
        }
    }

    if (&m_showWellLabel == changedField)
    {
        for (RimSimWellInView* w : wells)
        {
            w->showWellLabel = !(m_showWellLabel().isFalse());
            w->updateConnectedEditors();
        }
    }

    if (&m_showWellHead == changedField)
    {
        for (RimSimWellInView* w : wells)
        {
            w->showWellHead = !(m_showWellHead().isFalse());
            w->updateConnectedEditors();
        }
    }

    if (&m_showWellPipe == changedField)
    {
        for (RimSimWellInView* w : wells)
        {
            w->showWellPipe = !(m_showWellPipe().isFalse());
            w->updateConnectedEditors();
        }
    }

    if (&m_showWellSpheres == changedField)
    {
        for (RimSimWellInView* w : wells)
        {
            w->showWellSpheres = !(m_showWellSpheres().isFalse());
            w->updateConnectedEditors();
        }
    }

    if (&m_showWellCells == changedField)
    {
        for (RimSimWellInView* w : wells)
        {
            w->showWellCells = !(m_showWellCells().isFalse());
            w->updateConnectedEditors();
        }
    }

    if (&m_showWellCellFence == changedField)
    {
        for (RimSimWellInView* w : wells)
        {
            w->showWellCellFence = !(m_showWellCellFence().isFalse());
            w->updateConnectedEditors();
        }
    }

    if (m_reservoirView)
    {
        if (   &isActive == changedField
            || &m_showWellLabel == changedField
            || &m_showWellCells == changedField
            || &m_showWellCellFence == changedField
            || &wellCellFenceType == changedField)
        {
            m_reservoirView->scheduleGeometryRegen(VISIBLE_WELL_CELLS);
            m_reservoirView->scheduleCreateDisplayModelAndRedraw();
        }
        else if (&wellCellTransparencyLevel == changedField)
        {
            m_reservoirView->scheduleCreateDisplayModelAndRedraw();
        }
        else if (  &spheresScaleFactor == changedField
                || &m_showWellSpheres == changedField
                || &showConnectionStatusColors == changedField)
        {
            m_reservoirView->scheduleSimWellGeometryRegen();
            m_reservoirView->scheduleCreateDisplayModelAndRedraw();
        }
        else if (  &pipeCrossSectionVertexCount == changedField 
                || &pipeScaleFactor == changedField 
                || &wellHeadScaleFactor == changedField 
                || &m_showWellHead == changedField
                || &isAutoDetectingBranches == changedField
                || &wellHeadPosition == changedField
                || &wellLabelColor == changedField
                || &wellPipeCoordType == changedField
                || &m_showWellPipe == changedField)
        {
            m_reservoirView->scheduleSimWellGeometryRegen();
            m_reservoirView->scheduleCreateDisplayModelAndRedraw();

            for (RimSimWellInView* w : wells) w->schedule2dIntersectionViewUpdate();
        }
        else if (&showWellsIntersectingVisibleCells == changedField)
        {
            m_reservoirView->scheduleGeometryRegen(VISIBLE_WELL_CELLS);
            m_reservoirView->scheduleSimWellGeometryRegen();
            m_reservoirView->scheduleCreateDisplayModelAndRedraw();
        }
    }

    if (&m_wellPipeColors == changedField || &m_defaultWellPipeColor == changedField)
    {
        if (m_wellPipeColors == WELLPIPE_COLOR_UNIQUE)
        {
            assignDefaultWellColors();
        }
        else
        {
            cvf::Color3f col = m_defaultWellPipeColor();

            for (size_t i = 0; i < wells.size(); i++)
            {
                wells[i]->wellPipeColor = col;
                wells[i]->updateConnectedEditors();
            }
            RimSimWellInViewCollection::updateWellAllocationPlots();
        }
        if (m_reservoirView) m_reservoirView->scheduleCreateDisplayModelAndRedraw();
    }

    if (&m_showWellCells == changedField)
    {
        RiuMainWindow::instance()->refreshDrawStyleActions();
    }

    if (&m_showWellCommunicationLines == changedField)
    {
        if (m_reservoirView) m_reservoirView->scheduleCreateDisplayModelAndRedraw();
    }

    if (&wellPipeCoordType == changedField || &isAutoDetectingBranches == changedField)
    {
        if (m_reservoirView)
        {
            m_reservoirView->crossSectionCollection()->recomputeSimWellBranchData();
        }

        for (RimSimWellInView* w : wells)
        {
            w->simwellFractureCollection()->recomputeSimWellCenterlines();

        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSimWellInViewCollection::assignDefaultWellColors()
{
    RimEclipseCase* ownerCase; 
    firstAncestorOrThisOfTypeAsserted(ownerCase);
    
    for (size_t wIdx = 0; wIdx < wells.size(); ++wIdx)
    {
        RimSimWellInView* well = wells[wIdx];
        if (well && well->simWellData() )
        {
            well->wellPipeColor = ownerCase->defaultWellColor(well->simWellData()->m_wellName);
        }
    }

    RimSimWellInViewCollection::updateWellAllocationPlots();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSimWellInViewCollection::updateWellAllocationPlots()
{
    RimProject* proj = RiaApplication::instance()->project();

    std::vector<RimWellAllocationPlot*> wellAllocationPlots;
    proj->descendantsIncludingThisOfType(wellAllocationPlots);

    for (auto wap : wellAllocationPlots)
    {
        wap->loadDataAndUpdate();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSimWellInViewCollection::setReservoirView(RimEclipseView* ownerReservoirView)
{
    m_reservoirView = ownerReservoirView;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSimWellInViewCollection::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    updateStateForVisibilityCheckboxes();

    bool isContourMap = dynamic_cast<const RimEclipseContourMapView*>(m_reservoirView) != nullptr;

    caf::PdmUiGroup* appearanceGroup = uiOrdering.addNewGroup("Visibility");
    if (!isContourMap)
    {
        appearanceGroup->add(&showWellsIntersectingVisibleCells);
    }
    appearanceGroup->add(&m_showWellLabel);
    appearanceGroup->add(&m_showWellHead);
    appearanceGroup->add(&m_showWellPipe);
    appearanceGroup->add(&m_showWellSpheres);
    appearanceGroup->add(&m_showWellCommunicationLines);
    
    if (!isContourMap)
    {
        caf::PdmUiGroup* filterGroup = uiOrdering.addNewGroup("Well Cells and Fence");
        filterGroup->add(&m_showWellCells);
        filterGroup->add(&m_showWellCellFence);
        filterGroup->add(&wellCellFenceType);
    }

    caf::PdmUiGroup* sizeScalingGroup = uiOrdering.addNewGroup("Size Scaling");
    sizeScalingGroup->add(&wellHeadScaleFactor);
    sizeScalingGroup->add(&pipeScaleFactor);
    sizeScalingGroup->add(&spheresScaleFactor);

    caf::PdmUiGroup* colorGroup = uiOrdering.addNewGroup("Colors");
    colorGroup->setCollapsedByDefault(true);
    colorGroup->add(&showConnectionStatusColors);
    colorGroup->add(&wellLabelColor);
    colorGroup->add(&m_wellPipeColors);
    if (m_wellPipeColors == WELLPIPE_COLOR_UNIFORM)
    {
        colorGroup->add(&m_defaultWellPipeColor);
    }

    caf::PdmUiGroup* wellPipeGroup = uiOrdering.addNewGroup("Well Pipe Geometry" );
    wellPipeGroup->add(&wellPipeCoordType);
    wellPipeGroup->add(&isAutoDetectingBranches);

    if (!isContourMap)
    {
        caf::PdmUiGroup* advancedGroup = uiOrdering.addNewGroup("Advanced");
        advancedGroup->setCollapsedByDefault(true);
        advancedGroup->add(&wellCellTransparencyLevel);
        advancedGroup->add(&wellHeadPosition);
    }

    RimEclipseResultCase* ownerCase = nullptr; 
    firstAncestorOrThisOfType(ownerCase);
    if (ownerCase)
    {
        m_showWellCommunicationLines.uiCapability()->setUiHidden(!ownerCase->flowDiagSolverInterface());
    }

    m_showWellCellFence.uiCapability()->setUiReadOnly(!showWellCells());
    wellCellFenceType.uiCapability()->setUiReadOnly(!showWellCells());

    uiOrdering.skipRemainingFields(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSimWellInViewCollection::updateStateForVisibilityCheckboxes()
{
    size_t showLabelCount = 0;
    size_t showWellHeadCount = 0;
    size_t showPipeCount = 0;
    size_t showSphereCount = 0;
    size_t showWellCellsCount = 0;
    size_t showWellCellFenceCount = 0;

    for (RimSimWellInView* w : wells)
    {
        if (w->showWellLabel())     showLabelCount++;
        if (w->showWellHead())      showWellHeadCount++;
        if (w->showWellPipe())      showPipeCount++;
        if (w->showWellSpheres())   showSphereCount++;
        if (w->showWellCells())     showWellCellsCount++;
        if (w->showWellCellFence()) showWellCellFenceCount++;
    }

    updateStateFromEnabledChildCount(showLabelCount, &m_showWellLabel);
    updateStateFromEnabledChildCount(showWellHeadCount, &m_showWellHead);
    updateStateFromEnabledChildCount(showPipeCount, &m_showWellPipe);
    updateStateFromEnabledChildCount(showSphereCount, &m_showWellSpheres);
    updateStateFromEnabledChildCount(showWellCellsCount, &m_showWellCells);
    updateStateFromEnabledChildCount(showWellCellFenceCount, &m_showWellCellFence);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSimWellInViewCollection::updateStateFromEnabledChildCount(size_t enabledChildCount, caf::PdmField<caf::Tristate>* fieldToUpdate)
{
    caf::Tristate tristate;

    if (enabledChildCount == 0)
    {
        tristate = caf::Tristate::State::False;
    }
    else if (enabledChildCount == wells.size())
    {
        tristate = caf::Tristate::State::True;
    }
    else
    {
        tristate = caf::Tristate::State::PartiallyTrue;
    }

    fieldToUpdate->setValue(tristate);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimSimWellInViewCollection::objectToggleField()
{
    return &isActive;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSimWellInViewCollection::initAfterRead()
{
    if (obsoleteField_wellPipeVisibility() == PIPES_OPEN_IN_VISIBLE_CELLS)
    {
        showWellsIntersectingVisibleCells = true;
    }
    else if (obsoleteField_wellPipeVisibility() == PIPES_FORCE_ALL_OFF)
    {
        showWellsIntersectingVisibleCells = false;

        for (RimSimWellInView* w : wells)
        {
            w->showWellPipe = false;
        }
    }
    else if (obsoleteField_wellPipeVisibility() == PIPES_FORCE_ALL_ON)
    {
        showWellsIntersectingVisibleCells = false;

        for (RimSimWellInView* w : wells)
        {
            w->showWellPipe = true;
        }
    }

    if (obsoleteField_wellCellsToRangeFilterMode() == RANGE_ADD_NONE)
    {
        for (RimSimWellInView* w : wells)
        {
            w->showWellCells = false;
        }
    }
    else if (obsoleteField_wellCellsToRangeFilterMode() == RANGE_ADD_ALL)
    {
        for (RimSimWellInView* w : wells)
        {
            w->showWellCells = true;
        }
    }

    if (!obsoleteField_showWellLabel())
    {
        for (RimSimWellInView* w : wells)
        {
            w->showWellLabel = false;
        }
    }

    if (!obsoleteField_showWellHead())
    {
        for (RimSimWellInView* w : wells)
        {
            w->showWellHead = false;
        }
    }

    if (obsoleteField_showWellCellFence())
    {
        for (RimSimWellInView* w : wells)
        {
            w->showWellCellFence = true;
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<cvf::ubyte>& RimSimWellInViewCollection::resultWellGeometryVisibilities(size_t frameIndex)
{
    calculateWellGeometryVisibility(frameIndex);
    return m_framesOfResultWellPipeVisibilities[frameIndex];
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSimWellInViewCollection::scheduleIsWellPipesVisibleRecalculation()
{
    m_framesOfResultWellPipeVisibilities.clear();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSimWellInViewCollection::calculateWellGeometryVisibility(size_t frameIndex)
{
    if (m_framesOfResultWellPipeVisibilities.size() > frameIndex && m_framesOfResultWellPipeVisibilities[frameIndex].size()) return;

    if (m_framesOfResultWellPipeVisibilities.size() <= frameIndex)
        m_framesOfResultWellPipeVisibilities.resize(frameIndex+1);

    if (m_framesOfResultWellPipeVisibilities[frameIndex].size() <= wells().size())
        m_framesOfResultWellPipeVisibilities[frameIndex].resize(wells().size(), false); 
    
    for (const RimSimWellInView* well : wells())
    {
        bool wellPipeVisible = well->isWellPipeVisible(frameIndex);
        bool wellSphereVisible = well->isWellSpheresVisible(frameIndex);

        m_framesOfResultWellPipeVisibilities[frameIndex][well->resultWellIndex()] = wellPipeVisible || wellSphereVisible;
    }
}

bool lessEclipseWell(const caf::PdmPointer<RimSimWellInView>& w1,  const caf::PdmPointer<RimSimWellInView>& w2)
{
    if (w1.notNull() && w2.notNull())
        return (w1->name() < w2->name());
    else if (w1.notNull())
        return true;
    else
        return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSimWellInViewCollection::sortWellsByName()
{
   std::sort(wells.begin(), wells.end(), lessEclipseWell);
}
