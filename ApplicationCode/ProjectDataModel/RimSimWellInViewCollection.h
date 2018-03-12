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

#pragma once

#include "cafAppEnum.h"
#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmFieldCvfColor.h" // Include to make Pdm work for cvf::Color
#include "cafPdmObject.h"
#include "cafPdmPointer.h"
#include "cafTristate.h"

class RimEclipseView;
class RimSimWellInView;

//==================================================================================================
///  
///  
//==================================================================================================
class RimSimWellInViewCollection : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;
public:

    RimSimWellInViewCollection();
    virtual ~RimSimWellInViewCollection();
    
    void                                setReservoirView(RimEclipseView* ownerReservoirView);

    enum WellVisibilityType
    {
        PIPES_FORCE_ALL_OFF,
        PIPES_INDIVIDUALLY,
        PIPES_OPEN_IN_VISIBLE_CELLS,
        PIPES_FORCE_ALL_ON
    };
    typedef caf::AppEnum<RimSimWellInViewCollection::WellVisibilityType> WellVisibilityEnum;

    enum WellCellsRangeFilterType
    {
        RANGE_ADD_ALL,
        RANGE_ADD_INDIVIDUAL,
        RANGE_ADD_NONE
    };
    typedef caf::AppEnum<RimSimWellInViewCollection::WellCellsRangeFilterType> WellCellsRangeFilterEnum;

    enum WellFenceType
    {
        K_DIRECTION, 
        J_DIRECTION,
        I_DIRECTION
    };
    typedef caf::AppEnum<RimSimWellInViewCollection::WellFenceType> WellFenceEnum;

    enum WellHeadPositionType
    {
        WELLHEAD_POS_ACTIVE_CELLS_BB, 
        WELLHEAD_POS_TOP_COLUMN
    };
    typedef caf::AppEnum<RimSimWellInViewCollection::WellHeadPositionType> WellHeadPositionEnum;

    enum WellPipeCoordType
    {
        WELLPIPE_CELLCENTER,
        WELLPIPE_INTERPOLATED
    };
    typedef caf::AppEnum<RimSimWellInViewCollection::WellPipeCoordType> WellPipeCoordEnum;


    caf::PdmField<bool>                 isActive;
    caf::PdmField<bool>                 showWellsIntersectingVisibleCells;
    
    caf::PdmField<double>               wellHeadScaleFactor;
    caf::PdmField<double>               pipeScaleFactor;
    caf::PdmField<double>               spheresScaleFactor;

    caf::PdmField<cvf::Color3f>         wellLabelColor;
    caf::PdmField<bool>                 showConnectionStatusColors;

    void                                setShowWellCellsState(bool enable);
    bool                                showWellCells();

    bool                                showWellCommunicationLines() { return m_showWellCommunicationLines();}

    caf::PdmField<WellFenceEnum>        wellCellFenceType;
    caf::PdmField<double>               wellCellTransparencyLevel;

    caf::PdmField<int>                  pipeCrossSectionVertexCount;
    caf::PdmField<WellPipeCoordEnum>    wellPipeCoordType;

    caf::PdmField<WellHeadPositionEnum> wellHeadPosition;

    caf::PdmField<bool>                 isAutoDetectingBranches;
    
    caf::PdmChildArrayField<RimSimWellInView*>     wells;

    RimSimWellInView*                   findWell(QString name);
    bool                                hasVisibleWellCells();
    bool                                hasVisibleWellPipes();
    void                                sortWellsByName();

    const std::vector<cvf::ubyte>&      resultWellGeometryVisibilities(size_t frameIndex);       
    void                                scheduleIsWellPipesVisibleRecalculation();
    void                                updateStateForVisibilityCheckboxes();

    
    void                                assignDefaultWellColors();

    static void                         updateWellAllocationPlots();

protected:
    virtual void                        fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;


    virtual void                        defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;


    virtual caf::PdmFieldHandle*        objectToggleField() override;
    virtual void                        initAfterRead() override;
    virtual void                        defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute) override;

private:
    void                                calculateWellGeometryVisibility(size_t frameIndex);
    void                                updateStateFromEnabledChildCount(size_t showLabelCount, caf::PdmField<caf::Tristate>* fieldToUpdate);

private:
    RimEclipseView*                     m_reservoirView;
    std::vector< std::vector< cvf::ubyte > > m_framesOfResultWellPipeVisibilities;  
    
    // Fields
    caf::PdmField<cvf::Color3f>         m_wellColorForApply;
    caf::PdmField<bool>                 m_applySingleColorToWells;
    caf::PdmField<bool>                 m_applyIndividualColorsToWells;

    caf::PdmField<caf::Tristate>        m_showWellLabel;
    caf::PdmField<caf::Tristate>        m_showWellHead;
    caf::PdmField<caf::Tristate>        m_showWellPipe;
    caf::PdmField<caf::Tristate>        m_showWellSpheres;
    caf::PdmField<caf::Tristate>        m_showWellCells;
    caf::PdmField<caf::Tristate>        m_showWellCellFence;

    caf::PdmField<bool>                 m_showWellCommunicationLines; 

    // Obsolete fields
    caf::PdmField<WellVisibilityEnum>       obsoleteField_wellPipeVisibility;
    caf::PdmField<WellCellsRangeFilterEnum> obsoleteField_wellCellsToRangeFilterMode;

    caf::PdmField<bool>                     obsoleteField_showWellLabel;
    caf::PdmField<bool>                     obsoleteField_showWellHead;
    caf::PdmField<bool>                     obsoleteField_showWellCellFence;
};
