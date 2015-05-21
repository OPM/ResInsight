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
#include "RivReservoirPartMgr.h"
#include "cvfTransform.h"
#include "cafFixedArray.h"
#include "cvfArray.h"
#include "cafPdmObject.h"

class RimEclipseView;
class RigGridBase;
class RimCellRangeFilterCollection;
class RimCellPropertyFilterCollection;
class RigActiveCellInfo;

class RivReservoirViewPartMgr: public cvf::Object
{
public:
    RivReservoirViewPartMgr(RimEclipseView * resv);

    cvf::Transform*             scaleTransform() { return m_scaleTransform.p();}

    enum ReservoirGeometryCacheType
    {
        ACTIVE,                                         ///< All Active cells without ALL_WELL_CELLS
        ALL_WELL_CELLS,                                 ///< All cells ever having a connection to a well (Might be inactive cells as well. Wellhead cells typically)
        VISIBLE_WELL_CELLS,                             ///< ALL_WELL_CELLS && visible well cells including Fence
        VISIBLE_WELL_FENCE_CELLS,                       ///< (! ALL_WELL_CELLS) && visible well cells including Fence
        INACTIVE,                                       ///< All inactive cells, but invalid cells might or might not be included
        RANGE_FILTERED,                                 ///< ACTIVE Filtered by the set of range filters
        RANGE_FILTERED_INACTIVE,                        ///< INACTIVE Filtered by the set of range filters
        RANGE_FILTERED_WELL_CELLS,                      ///< ALL_WELL_CELLS Filtered by the set of range filters
        VISIBLE_WELL_CELLS_OUTSIDE_RANGE_FILTER,        ///< VISIBLE_WELL_CELLS && !RANGE_FILTERED_WELL_CELLS
        VISIBLE_WELL_FENCE_CELLS_OUTSIDE_RANGE_FILTER,  ///< VISIBLE_WELL_FENCE_CELLS && !RANGE_FILTERED
        PROPERTY_FILTERED,                              ///< (RANGE_FILTERED || VISIBLE_WELL_FENCE_CELLS_OUTSIDE_RANGE_FILTER) && !ExcludedByPropFilter && IncludedByPropFilter
        PROPERTY_FILTERED_WELL_CELLS                    ///< (!(hasActiveRangeFilters || visibleWellCells) && (*ALL_WELL_CELLS)) || RANGE_FILTERED_WELL_CELLS || VISIBLE_WELL_CELLS_OUTSIDE_RANGE_FILTER
    };

    void                        clearGeometryCache();
    void                        scheduleGeometryRegen(ReservoirGeometryCacheType geometryType);
    cvf::cref<cvf::UByteArray>  cellVisibility(ReservoirGeometryCacheType geometryType, size_t gridIndex, size_t frameIndex) const;
   
    void                        appendStaticGeometryPartsToModel (cvf::ModelBasicList* model, ReservoirGeometryCacheType geometryType, const std::vector<size_t>& gridIndices);
    void                        appendDynamicGeometryPartsToModel(cvf::ModelBasicList* model, ReservoirGeometryCacheType geometryType, size_t frameIndex, const std::vector<size_t>& gridIndices);

    void                        updateCellColor          (ReservoirGeometryCacheType geometryType, cvf::Color4f color);
    void                        updateCellColor          (ReservoirGeometryCacheType geometryType, size_t timeStepIndex, 
                                                          cvf::Color4f color);
    void                        updateCellResultColor    (ReservoirGeometryCacheType geometryType, size_t timeStepIndex, 
                                                          RimResultSlot* cellResultSlot);
    void                        updateCellEdgeResultColor(ReservoirGeometryCacheType geometryType, size_t timeStepIndex, 
														  RimResultSlot* cellResultSlot,
														  RimCellEdgeResultSlot* cellEdgeResultSlot);

    // Faults
    void                        appendFaultsStaticGeometryPartsToModel(cvf::ModelBasicList* model, ReservoirGeometryCacheType geometryType);
    void                        appendFaultsDynamicGeometryPartsToModel(cvf::ModelBasicList* model, ReservoirGeometryCacheType geometryType, size_t frameIndex);
    void                        updateFaultColors(ReservoirGeometryCacheType geometryType, size_t timeStepIndex, RimResultSlot* cellResultSlot);
	void                        updateFaultCellEdgeResultColor(	ReservoirGeometryCacheType geometryType, size_t timeStepIndex,
															RimResultSlot* cellResultSlot,
															RimCellEdgeResultSlot* cellEdgeResultSlot);

    // Fault labels
    ReservoirGeometryCacheType  geometryTypeForFaultLabels(const std::vector<ReservoirGeometryCacheType>& geometryTypes) const;
    void                        appendFaultLabelsStaticGeometryPartsToModel(cvf::ModelBasicList* model, ReservoirGeometryCacheType geometryType);
    void                        appendFaultLabelsDynamicGeometryPartsToModel(cvf::ModelBasicList* model, ReservoirGeometryCacheType geometryType, size_t frameIndex);

    void                        setFaultForceVisibilityForGeometryType(ReservoirGeometryCacheType geometryType, bool forceVisibility);

private:
    void                        createGeometry(ReservoirGeometryCacheType geometryType);
    void                        computeVisibility(cvf::UByteArray* cellVisibility, ReservoirGeometryCacheType geometryType, RigGridBase* grid, size_t gridIdx);

    void                        createPropertyFilteredNoneWellCellGeometry(size_t frameIndex);
    void                        createPropertyFilteredWellGeometry(size_t frameIndex);

    void                        clearGeometryCache(ReservoirGeometryCacheType geomType);


    static void                 computeNativeVisibility  (cvf::UByteArray* cellVisibilities, const RigGridBase* grid, const RigActiveCellInfo* activeCellInfo, const cvf::UByteArray* cellIsInWellStatuses,  bool invalidCellsIsVisible, bool inactiveCellsIsVisible, bool activeCellsIsVisible, bool mainGridIsVisible);
    void                        computeRangeVisibility   (ReservoirGeometryCacheType geometryType, cvf::UByteArray* cellVisibilities, const RigGridBase* grid, const cvf::UByteArray* nativeVisibility, const RimCellRangeFilterCollection* rangeFilterColl);
    static void                 computePropertyVisibility(cvf::UByteArray* cellVisibilities, const RigGridBase* grid, size_t timeStepIndex, const cvf::UByteArray* rangeFilterVisibility, RimCellPropertyFilterCollection* propFilterColl);
    static void                 copyByteArray(cvf::UByteArray* cellVisibilities, const cvf::UByteArray* cellIsWellStatuses );

    RivReservoirPartMgr *       reservoirPartManager(ReservoirGeometryCacheType geometryType, size_t timeStepIndex );


private:
    caf::FixedArray<RivReservoirPartMgr, PROPERTY_FILTERED> m_geometries;
    caf::FixedArray<bool, PROPERTY_FILTERED>                m_geometriesNeedsRegen;

    cvf::Collection<RivReservoirPartMgr>                    m_propFilteredGeometryFrames;
    std::vector<uchar>                                      m_propFilteredGeometryFramesNeedsRegen;

    cvf::Collection<RivReservoirPartMgr>                    m_propFilteredWellGeometryFrames;
    std::vector<uchar>                                      m_propFilteredWellGeometryFramesNeedsRegen;

    cvf::ref<cvf::Transform>                                m_scaleTransform;
    caf::PdmPointer<RimEclipseView>                       m_reservoirView;

};
