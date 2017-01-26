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

#include "RimEclipseWellCollection.h"

#include "RiaApplication.h"
#include "RiaPreferences.h"

#include "RigSingleWellResultsData.h"

#include "RimEclipseView.h"
#include "RimEclipseWell.h"

#include "RiuMainWindow.h"
#include "RivReservoirViewPartMgr.h"

#include "cafPdmUiPushButtonEditor.h"


namespace caf
{
    // OBSOLETE enum
    template<>
    void RimEclipseWellCollection::WellVisibilityEnum::setUp()
    {
        addItem(RimEclipseWellCollection::PIPES_FORCE_ALL_OFF,       "FORCE_ALL_OFF",      "All Off");
        addItem(RimEclipseWellCollection::PIPES_INDIVIDUALLY,        "ALL_ON",             "Individual");
        addItem(RimEclipseWellCollection::PIPES_OPEN_IN_VISIBLE_CELLS,"OPEN_IN_VISIBLE_CELLS", "Visible cells filtered");
        addItem(RimEclipseWellCollection::PIPES_FORCE_ALL_ON,        "FORCE_ALL_ON",       "All On");
    }
}


namespace caf
{
    // OBSOLETE enum
    template<>
    void RimEclipseWellCollection::WellCellsRangeFilterEnum::setUp()
    {
        addItem(RimEclipseWellCollection::RANGE_ADD_NONE,       "FORCE_ALL_OFF",      "All Off");
        addItem(RimEclipseWellCollection::RANGE_ADD_INDIVIDUAL, "ALL_ON",             "Individually");
        addItem(RimEclipseWellCollection::RANGE_ADD_ALL,        "FORCE_ALL_ON",       "All On");
    }
}

namespace caf
{
    template<>
    void RimEclipseWellCollection::WellFenceEnum::setUp()
    {
        addItem(RimEclipseWellCollection::K_DIRECTION, "K_DIRECTION",    "K - Direction");
        addItem(RimEclipseWellCollection::J_DIRECTION, "J_DIRECTION",    "J - Direction");
        addItem(RimEclipseWellCollection::I_DIRECTION, "I_DIRECTION",    "I - Direction");
        setDefault(RimEclipseWellCollection::K_DIRECTION);
    }
}

namespace caf
{
    template<>
    void RimEclipseWellCollection::WellHeadPositionEnum::setUp()
    {
        addItem(RimEclipseWellCollection::WELLHEAD_POS_ACTIVE_CELLS_BB,    "WELLHEAD_POS_ACTIVE_CELLS_BB", "All Active Cells");
        addItem(RimEclipseWellCollection::WELLHEAD_POS_TOP_COLUMN,         "WELLHEAD_POS_TOP_COLUMN",      "Active Cell Column");
        setDefault(RimEclipseWellCollection::WELLHEAD_POS_TOP_COLUMN);
    }
}

namespace caf
{
    template<>
    void RimEclipseWellCollection::WellPipeCoordEnum::setUp()
    {
        addItem(RimEclipseWellCollection::WELLPIPE_INTERPOLATED,    "WELLPIPE_INTERPOLATED",    "Interpolated");
        addItem(RimEclipseWellCollection::WELLPIPE_CELLCENTER,      "WELLPIPE_CELLCENTER",      "Cell Centers");
        setDefault(RimEclipseWellCollection::WELLPIPE_INTERPOLATED);
    }
}

CAF_PDM_SOURCE_INIT(RimEclipseWellCollection, "Wells");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseWellCollection::RimEclipseWellCollection()
{
    CAF_PDM_InitObject("Simulation Wells", ":/WellCollection.png", "", "");

    CAF_PDM_InitField(&isActive,              "Active",        true,   "Active", "", "", "");
    isActive.uiCapability()->setUiHidden(true);

    CAF_PDM_InitField(&showWellsIntersectingVisibleCells, "ShowWellsIntersectingVisibleCells", true, "Show Wells Intersecting Visible Cells", "", "", "");

    // Appearance
    CAF_PDM_InitField(&showWellHead,        "ShowWellHead",     true,   "Show Well Head", "", "", "");
    CAF_PDM_InitField(&showWellLabel,       "ShowWellLabel",    true,   "Show Well Label", "", "", "");
    CAF_PDM_InitField(&showWellPipe,        "ShowWellPipe",     true,   "Show Well Pipe", "", "", "");
    CAF_PDM_InitField(&showWellSpheres,     "ShowWellSpheres",  true,   "Show Well Spheres", "", "", "");

    // Scaling
    CAF_PDM_InitField(&wellHeadScaleFactor, "WellHeadScale",            1.0,    "Well Head Scale Factor", "", "", "");
    CAF_PDM_InitField(&pipeScaleFactor,     "WellPipeRadiusScale",      0.1,    "Well Pipe Scale Factor", "", "", "");
    CAF_PDM_InitField(&spheresScaleFactor,  "CellCenterSphereScale",    0.2,    "Well Sphere Scale Factor", "", "", "");

    // Color
    cvf::Color3f defWellLabelColor = RiaApplication::instance()->preferences()->defaultWellLabelColor();
    CAF_PDM_InitField(&wellLabelColor,      "WellLabelColor",   defWellLabelColor, "Well label color",  "", "", "");

    CAF_PDM_InitField(&showConnectionStatusColors, "ShowConnectionStatusColors", true, "Show Connection Status Colors Along Well", "", "", "");

    cvf::Color3f defaultApplyColor = cvf::Color3f::YELLOW;
    CAF_PDM_InitField(&m_wellColorForApply, "WellColorForApply", defaultApplyColor, "Single Well Color", "", "", "");

    CAF_PDM_InitField(&m_applySingleColorToWells, "ApplySingleColorToWells", false, "", "", "", "");
    m_applySingleColorToWells.uiCapability()->setUiEditorTypeName(caf::PdmUiPushButtonEditor::uiEditorTypeName());
    m_applySingleColorToWells.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);
    m_applySingleColorToWells.xmlCapability()->setIOReadable(false);
    m_applySingleColorToWells.xmlCapability()->setIOWritable(false);

    CAF_PDM_InitField(&m_applyIndividualColorsToWells, "ApplyIndividualColorsToWells", false, "", "", "", "");
    m_applyIndividualColorsToWells.uiCapability()->setUiEditorTypeName(caf::PdmUiPushButtonEditor::uiEditorTypeName());
    m_applyIndividualColorsToWells.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);
    m_applyIndividualColorsToWells.xmlCapability()->setIOReadable(false);
    m_applyIndividualColorsToWells.xmlCapability()->setIOWritable(false);

    CAF_PDM_InitField(&pipeCrossSectionVertexCount, "WellPipeVertexCount", 12, "Pipe vertex count", "", "", "");
    pipeCrossSectionVertexCount.uiCapability()->setUiHidden(true);
    CAF_PDM_InitField(&wellPipeCoordType,           "WellPipeCoordType", WellPipeCoordEnum(WELLPIPE_INTERPOLATED), "Well Pipe Geometry", "", "", "");

    CAF_PDM_InitField(&showWellCells,       "ShowWellCells",            false,   "Show Well Cells", "", "", "");
    CAF_PDM_InitField(&showWellCellFence,   "ShowWellFences",           false,  "Show Well Cell Fence", "", "", "");
    CAF_PDM_InitField(&wellCellFenceType,   "DefaultWellFenceDirection", WellFenceEnum(K_DIRECTION), "Well Fence Direction", "", "", "");

    CAF_PDM_InitField(&wellCellTransparencyLevel, "WellCellTransparency", 0.5, "Well Cell Transparency", "", "", "");
    CAF_PDM_InitField(&isAutoDetectingBranches, "IsAutoDetectingBranches", true, "Branch Detection", "", "Toggle wether the well pipe visualization will try to detect when a part of the well \nis really a branch, and thus is starting from wellhead", "");
    CAF_PDM_InitField(&wellHeadPosition,    "WellHeadPosition", WellHeadPositionEnum(WELLHEAD_POS_TOP_COLUMN), "Well Head Position On Top Of",  "", "", "");

    CAF_PDM_InitFieldNoDefault(&wells, "Wells", "Wells",  "", "", "");
    wells.uiCapability()->setUiHidden(true);

    CAF_PDM_InitField(&obsoleteField_wellPipeVisibility,  "GlobalWellPipeVisibility", WellVisibilityEnum(PIPES_OPEN_IN_VISIBLE_CELLS), "Global well pipe visibility",  "", "", "");
    obsoleteField_wellPipeVisibility.uiCapability()->setUiHidden(true);
    obsoleteField_wellPipeVisibility.xmlCapability()->setIOWritable(false);
    
    CAF_PDM_InitField(&obsoleteField_wellCellsToRangeFilterMode,  "GlobalWellCellVisibility", WellCellsRangeFilterEnum(RANGE_ADD_NONE),  "Add cells to range filter", "", "", "");
    obsoleteField_wellCellsToRangeFilterMode.uiCapability()->setUiHidden(true);
    obsoleteField_wellCellsToRangeFilterMode.xmlCapability()->setIOWritable(false);

    m_reservoirView = NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseWellCollection::~RimEclipseWellCollection()
{
   wells.deleteAllChildObjects();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseWell* RimEclipseWellCollection::findWell(QString name)
{
    for (size_t i = 0; i < this->wells().size(); ++i)
    {
        if (this->wells()[i]->name() == name)
        {
            return this->wells()[i];
        }
    }
    return NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimEclipseWellCollection::hasVisibleWellCells()
{
    if (!this->isActive()) return false;
    if (!this->showWellCells()) return false;
    if (this->wells().size() == 0 ) return false;

    bool hasCells = false;
    for (size_t i = 0 ; !hasCells && i < this->wells().size(); ++i)
    {
        RimEclipseWell* well = this->wells()[i];
        if ( well && well->wellResults() && ((well->showWell() && well->showWellCells())) )
        {
            for (size_t tIdx = 0; !hasCells &&  tIdx < well->wellResults()->m_wellCellsTimeSteps.size(); ++tIdx )
            {
                const RigWellResultFrame& wellResultFrame = well->wellResults()->m_wellCellsTimeSteps[tIdx];
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
bool RimEclipseWellCollection::hasVisibleWellPipes() 
{
    if (!this->isActive()) return false;
    if (this->wells().size() == 0 ) return false;

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseWellCollection::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (&isActive == changedField)
    {
        this->updateUiIconFromToggleField();
    }

    if (m_reservoirView)
    {
        if (   &isActive == changedField
            || &showWellLabel == changedField
            || &showWellCells == changedField
            || &showWellCellFence == changedField
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
                || &showWellSpheres == changedField
                || &showConnectionStatusColors == changedField)
        {
            m_reservoirView->schedulePipeGeometryRegen();
            m_reservoirView->scheduleCreateDisplayModelAndRedraw();
        }
        else if (  &pipeCrossSectionVertexCount == changedField 
                || &pipeScaleFactor == changedField 
                || &wellHeadScaleFactor == changedField 
                || &showWellHead == changedField
                || &isAutoDetectingBranches == changedField
                || &wellHeadPosition == changedField
                || &wellLabelColor == changedField
                || &showWellsIntersectingVisibleCells == changedField
                || &wellPipeCoordType == changedField
                || &showWellPipe == changedField)
        {
            m_reservoirView->schedulePipeGeometryRegen();
            m_reservoirView->scheduleCreateDisplayModelAndRedraw();
        }
    }

    if (   &showWellPipe == changedField
        || &showWellSpheres == changedField
        || &showWellHead == changedField
        || &showWellLabel == changedField)
    {
        for (RimEclipseWell* w : wells)
        {
            w->updateConnectedEditors();
        }
    }

    if (&m_applyIndividualColorsToWells == changedField)
    {
        for (size_t i = 0; i < wells.size(); i++)
        {
            cvf::Color3f col = cycledPaletteColor(i);

            wells[i]->wellPipeColor = col;
            wells[i]->updateConnectedEditors();
        }
        
        if (m_reservoirView) m_reservoirView->scheduleCreateDisplayModelAndRedraw();

        m_applyIndividualColorsToWells = false;
    }

    if (&m_applySingleColorToWells == changedField)
    {
        cvf::Color3f col = m_wellColorForApply();

        for (size_t i = 0; i < wells.size(); i++)
        {
            wells[i]->wellPipeColor = col;
            wells[i]->updateConnectedEditors();
        }

        if (m_reservoirView) m_reservoirView->scheduleCreateDisplayModelAndRedraw();

        m_applySingleColorToWells = false;
    }

    if (&showWellCells == changedField)
    {
        RiuMainWindow::instance()->refreshDrawStyleActions();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseWellCollection::setReservoirView(RimEclipseView* ownerReservoirView)
{
    m_reservoirView = ownerReservoirView;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseWellCollection::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    uiOrdering.add(&showWellsIntersectingVisibleCells);

    caf::PdmUiGroup* appearanceGroup = uiOrdering.addNewGroup("Appearance");
    appearanceGroup->add(&showWellLabel);
    appearanceGroup->add(&showWellHead);
    appearanceGroup->add(&showWellPipe);
    appearanceGroup->add(&showWellSpheres);

    caf::PdmUiGroup* sizeScalingGroup = uiOrdering.addNewGroup("Size Scaling");
    sizeScalingGroup->add(&wellHeadScaleFactor);
    sizeScalingGroup->add(&pipeScaleFactor);
    sizeScalingGroup->add(&spheresScaleFactor);

    caf::PdmUiGroup* colorGroup = uiOrdering.addNewGroup("Color");
    colorGroup->add(&wellLabelColor);
    colorGroup->add(&m_applyIndividualColorsToWells);

    colorGroup->add(&m_wellColorForApply);
    colorGroup->add(&m_applySingleColorToWells);

    colorGroup->add(&showConnectionStatusColors);

    uiOrdering.add(&wellPipeCoordType);

    caf::PdmUiGroup* advancedGroup = uiOrdering.addNewGroup("Advanced");
    advancedGroup->add(&isAutoDetectingBranches);
    advancedGroup->add(&wellCellTransparencyLevel);
    
    uiOrdering.add(&wellHeadPosition);

    caf::PdmUiGroup* filterGroup = uiOrdering.addNewGroup("Well Cells");
    filterGroup->add(&obsoleteField_wellCellsToRangeFilterMode);
    filterGroup->add(&showWellCells);
    filterGroup->add(&showWellCellFence);
    filterGroup->add(&wellCellFenceType);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimEclipseWellCollection::objectToggleField()
{
    return &isActive;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseWellCollection::initAfterRead()
{
    if (obsoleteField_wellPipeVisibility() == PIPES_OPEN_IN_VISIBLE_CELLS)
    {
        showWellsIntersectingVisibleCells = true;
    }
    else if (obsoleteField_wellPipeVisibility() == PIPES_FORCE_ALL_OFF)
    {
        showWellsIntersectingVisibleCells = false;

        for (RimEclipseWell* w : wells)
        {
            w->showWell = false;
        }
    }
    else if (obsoleteField_wellPipeVisibility() == PIPES_FORCE_ALL_ON)
    {
        showWellsIntersectingVisibleCells = false;

        for (RimEclipseWell* w : wells)
        {
            w->showWell = true;
        }
    }
    else if (obsoleteField_wellPipeVisibility() == PIPES_INDIVIDUALLY)
    {
        showWellsIntersectingVisibleCells = false;
    }

    if (obsoleteField_wellCellsToRangeFilterMode() == RANGE_ADD_NONE)
    {
        showWellCells = false;
    }
    else if (obsoleteField_wellCellsToRangeFilterMode() == RANGE_ADD_ALL)
    {
        showWellCells = true;

        for (RimEclipseWell* w : wells)
        {
            w->showWellCells = true;
        }
    }
    else if (obsoleteField_wellCellsToRangeFilterMode() == RANGE_ADD_INDIVIDUAL)
    {
        showWellCells = true;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseWellCollection::defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute)
{
    if (&m_applyIndividualColorsToWells == field)
    {
        caf::PdmUiPushButtonEditorAttribute* editorAttr = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*>(attribute);
        if (editorAttr)
        {
            editorAttr->m_buttonText = "Apply Individual Well Colors";
        }
    }

    if (&m_applySingleColorToWells == field)
    {
        caf::PdmUiPushButtonEditorAttribute* editorAttr = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*>(attribute);
        if (editorAttr)
        {
            QColor col;
            col.setRgbF(m_wellColorForApply().r(), m_wellColorForApply().g(), m_wellColorForApply().b());

            QPixmap pixmap(100, 100);
            pixmap.fill(col);

            QIcon colorIcon(pixmap);

            editorAttr->m_buttonIcon = colorIcon;
            editorAttr->m_buttonText = "Apply Single Well Color";
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<cvf::ubyte>& RimEclipseWellCollection::resultWellGeometryVisibilities(size_t frameIndex)
{
    calculateWellGeometryVisibility(frameIndex);
    return m_framesOfResultWellPipeVisibilities[frameIndex];
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseWellCollection::scheduleIsWellPipesVisibleRecalculation()
{
    m_framesOfResultWellPipeVisibilities.clear();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseWellCollection::calculateWellGeometryVisibility(size_t frameIndex)
{
    if (m_framesOfResultWellPipeVisibilities.size() > frameIndex && m_framesOfResultWellPipeVisibilities[frameIndex].size()) return;

    if (m_framesOfResultWellPipeVisibilities.size() <= frameIndex)
        m_framesOfResultWellPipeVisibilities.resize(frameIndex+1);

    if (m_framesOfResultWellPipeVisibilities[frameIndex].size() <= wells().size())
        m_framesOfResultWellPipeVisibilities[frameIndex].resize(wells().size(), false); 
    
    for (size_t i = 0; i < wells().size(); ++i)
    {
        bool wellPipeVisible = wells[i]->isWellPipeVisible(frameIndex);
        bool wellSphereVisible = wells[i]->isWellSpheresVisible(frameIndex);

        m_framesOfResultWellPipeVisibilities[frameIndex][wells[i]->resultWellIndex()] = wellPipeVisible || wellSphereVisible;
    }
}

//--------------------------------------------------------------------------------------------------
/// TODO: Consider creating a factory for colors, see also RimSummaryCurveAppearanceCalculator
//--------------------------------------------------------------------------------------------------
cvf::Color3f RimEclipseWellCollection::cycledPaletteColor(size_t colorIndex)
{
    static const size_t colorCount = 15;
    static const cvf::ubyte colorData[][3] =
    {
      {  0,  112, 136 }, // Dark Green-Blue
      { 202,   0,   0 }, // Red
      { 78,  204,   0 }, // Clear Green
      { 236, 118,   0 }, // Orange
      {  0 ,   0,   0 }, // Black
      { 56,   56, 255 }, // Vivid Blue
      { 248,   0, 170 }, // Magenta
      { 169,   2, 240 }, // Purple
      { 0,   221, 221 }, // Turquoise
      { 201, 168, 206 }, // Light Violet
      { 0,   205,  68 }, // Bluish Green
      { 236, 188,   0 }, // Mid Yellow
      { 51,  204, 255 },  // Bluer Turquoise
      { 164, 193,   0 }, // Mid Yellowish Green
      { 0,   143, 239 }, // Dark Light Blue
    };

    size_t paletteIdx = colorIndex % colorCount;

    cvf::Color3ub ubColor(colorData[paletteIdx][0], colorData[paletteIdx][1], colorData[paletteIdx][2]);
    cvf::Color3f cvfColor(ubColor);

    return cvfColor;
}

bool lessEclipseWell(const caf::PdmPointer<RimEclipseWell>& w1,  const caf::PdmPointer<RimEclipseWell>& w2)
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
void RimEclipseWellCollection::sortWellsByName()
{
   std::sort(wells.begin(), wells.end(), lessEclipseWell);
}
