/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA, Ceetron Solutions AS
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

#include "cvfBase.h"
#include "cvfObject.h"
#include "cvfVector3.h"
#include "cvfBoundingBox.h"

#include <vector>

#include <QString>
#include "cvfStructGrid.h"
#include "cvfCellRange.h"
#include "cafFixedArray.h"

class RigMainGrid;

class RigFaultsPrCellAccumulator : public cvf::Object
{
public:
    enum { NO_FAULT = -1, UNKNOWN_FAULT = -2 };

public:
    RigFaultsPrCellAccumulator(size_t globalCellCount) 
    { 
        const int  initVals[6] = { NO_FAULT, NO_FAULT, NO_FAULT, NO_FAULT, NO_FAULT, NO_FAULT}; 
        caf::IntArray6 initVal;
        initVal = initVals; 
        m_faultIdxForCellFace.resize(globalCellCount, initVal);
    }

    inline int faultIdx(size_t globalCellIdx, cvf::StructGridInterface::FaceType face) 
    {
        return m_faultIdxForCellFace[globalCellIdx][face];
    }

    inline void setFaultIdx(size_t globalCellIdx, cvf::StructGridInterface::FaceType face, int faultIdx)
    {
        m_faultIdxForCellFace[globalCellIdx][face] = faultIdx;
    }

private:
    std::vector< caf::IntArray6 > m_faultIdxForCellFace; 
};

class RigFault : public cvf::Object
{
public:
   
    struct FaultFace
    {
        FaultFace(size_t nativeGlobalCellIndex, cvf::StructGridInterface::FaceType nativeFace, size_t oppositeGlobalCellIndex) :
            m_nativeGlobalCellIndex(nativeGlobalCellIndex),
            m_nativeFace(nativeFace),
            m_oppositeGlobalCellIndex(oppositeGlobalCellIndex)
            { }

        size_t                              m_nativeGlobalCellIndex;
        cvf::StructGridInterface::FaceType  m_nativeFace;
        size_t                              m_oppositeGlobalCellIndex;
    };

public:
    RigFault();

    void setName(const QString& name);
    QString name() const;

    void addCellRangeForFace(cvf::StructGridInterface::FaceType face, const cvf::CellRange& cellRange);
    void computeFaultFacesFromCellRanges(const RigMainGrid* grid);

    void accumulateFaultsPrCell(RigFaultsPrCellAccumulator* faultsPrCellAcc, int faultIdx);

    std::vector<FaultFace>&         faultFaces();
    const std::vector<FaultFace>&   faultFaces() const;

    std::vector<size_t>&         connectionIndices()       { return m_connectionIndices; }
    const std::vector<size_t>&   connectionIndices() const { return m_connectionIndices; }

    static RigFaultsPrCellAccumulator* faultsPrCellAccumulator()    { CVF_ASSERT(m_faultsPrCellAcc.notNull()); return m_faultsPrCellAcc.p();}
    static void initFaultsPrCellAccumulator(size_t globalCellCount) { m_faultsPrCellAcc = new RigFaultsPrCellAccumulator(globalCellCount); }

private:
    QString m_name;

    caf::FixedArray<std::vector<cvf::CellRange>, 6> m_cellRangesForFaces;
    
    std::vector<FaultFace> m_faultFaces;
    std::vector<size_t> m_connectionIndices;

    static cvf::ref<RigFaultsPrCellAccumulator> m_faultsPrCellAcc;
};
