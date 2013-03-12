/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
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
#include "RimReservoirView.h"
#include "cafFixedArray.h"
#include "cvfArray.h"
#include "cafPdmObject.h"

class RimReservoirView;
class RigGridBase;
class RimCellRangeFilterCollection;
class RimCellPropertyFilterCollection;
class RigActiveCellInfo;

class RivReservoirViewPartMgr: public cvf::Object
{
public:
    RivReservoirViewPartMgr(RimReservoirView * resv);

    cvf::Transform* scaleTransform() { return m_scaleTransform.p();}
    void setScaleTransform(cvf::Mat4d scale) { m_scaleTransform->setLocalTransform(scale);}

    enum ReservoirGeometryCacheType
    {
        ACTIVE,
        ALL_WELL_CELLS,
        VISIBLE_WELL_CELLS,
        VISIBLE_WELL_FENCE_CELLS,
        INACTIVE,
        RANGE_FILTERED,
        RANGE_FILTERED_INACTIVE,
        RANGE_FILTERED_WELL_CELLS,
        VISIBLE_WELL_CELLS_OUTSIDE_RANGE_FILTER,
        VISIBLE_WELL_FENCE_CELLS_OUTSIDE_RANGE_FILTER,
        PROPERTY_FILTERED,
        PROPERTY_FILTERED_WELL_CELLS // Includes RANGE_FILTERED_WELL_CELLS and VISIBLE_WELL_CELLS_OUTSIDE_RANGE_FILTER and VISIBLE_WELL_FENCE_CELLS_OUTSIDE_RANGE_FILTER
    };

    void   clearGeometryCache();
    void   scheduleGeometryRegen(ReservoirGeometryCacheType geometryType);
   
    void   appendStaticGeometryPartsToModel (cvf::ModelBasicList* model, ReservoirGeometryCacheType geometryType, const std::vector<size_t>& gridIndices);
    void   appendDynamicGeometryPartsToModel(cvf::ModelBasicList* model, ReservoirGeometryCacheType geometryType, size_t frameIndex, const std::vector<size_t>& gridIndices);

    void   updateCellColor          (ReservoirGeometryCacheType geometryType, cvf::Color4f color);
    void   updateCellColor          (ReservoirGeometryCacheType geometryType, size_t timeStepIndex, 
                                     cvf::Color4f color);
    void   updateCellResultColor    (ReservoirGeometryCacheType geometryType, size_t timeStepIndex, 
                                     RimResultSlot* cellResultSlot);
    void   updateCellEdgeResultColor(ReservoirGeometryCacheType geometryType, size_t timeStepIndex, 
                                     RimResultSlot* cellResultSlot, RimCellEdgeResultSlot* cellEdgeResultSlot);

private:
    void createGeometry(ReservoirGeometryCacheType geometryType);
    void computeVisibility(cvf::UByteArray* cellVisibility, ReservoirGeometryCacheType geometryType, RigGridBase* grid, size_t gridIdx);

    void createPropertyFilteredGeometry(size_t frameIndex);
    void createPropertyFilteredWellGeometry(size_t frameIndex);

    void clearGeometryCache(ReservoirGeometryCacheType geomType);


    static void computeNativeVisibility  (cvf::UByteArray* cellVisibilities, const RigGridBase* grid, const RigActiveCellInfo* activeCellInfo, const cvf::UByteArray* cellIsInWellStatuses,  bool invalidCellsIsVisible, bool inactiveCellsIsVisible, bool activeCellsIsVisible, bool mainGridIsVisible);
    static void computeRangeVisibility   (cvf::UByteArray* cellVisibilities, const RigGridBase* grid, const cvf::UByteArray* nativeVisibility, const RimCellRangeFilterCollection* rangeFilterColl);
    static void computePropertyVisibility(cvf::UByteArray* cellVisibilities, const RigGridBase* grid, size_t timeStepIndex, const cvf::UByteArray* rangeFilterVisibility, RimCellPropertyFilterCollection* propFilterColl);
    static void copyByteArray(cvf::UByteArray* cellVisibilities, const cvf::UByteArray* cellIsWellStatuses );

private:

    caf::FixedArray<RivReservoirPartMgr, PROPERTY_FILTERED> m_geometries;
    caf::FixedArray<bool, PROPERTY_FILTERED>                 m_geometriesNeedsRegen;

    cvf::Collection<RivReservoirPartMgr>                    m_propFilteredGeometryFrames;
    std::vector<uchar>                                       m_propFilteredGeometryFramesNeedsRegen;

    cvf::Collection<RivReservoirPartMgr>                    m_propFilteredWellGeometryFrames;
    std::vector<uchar>                                       m_propFilteredWellGeometryFramesNeedsRegen;



    cvf::ref<cvf::Transform>              m_scaleTransform;
    caf::PdmPointer<RimReservoirView>     m_reservoirView;

};
