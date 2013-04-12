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

#include "cvfBase.h"
#include "cvfObject.h"
#include "cvfArray.h"
#include "cvfVector3.h"
#include "RigCell.h"
#include "RigCaseData.h"

class LocalGridRefinement
{
public:
    LocalGridRefinement(const cvf::Vec3st& mainGridMin, const cvf::Vec3st& mainGridMax, const cvf::Vec3st& singleCellRefinementFactors)
    {
        m_mainGridMinCellPosition = mainGridMin;
        m_mainGridMaxCellPosition = mainGridMax;
        m_singleCellRefinementFactors = singleCellRefinementFactors;
    }

    cvf::Vec3st m_mainGridMinCellPosition;
    cvf::Vec3st m_mainGridMaxCellPosition;
    cvf::Vec3st m_singleCellRefinementFactors;
};

class RigReservoirBuilderMock
{
public:
    RigReservoirBuilderMock();

    void setWorldCoordinates(cvf::Vec3d minWorldCoordinate, cvf::Vec3d maxWorldCoordinate);
    void setGridPointDimensions(const cvf::Vec3st& gridPointDimensions);
    void setResultInfo(size_t resultCount, size_t timeStepCount);

    size_t resultCount() const { return m_resultCount; }
    size_t timeStepCount() const { return m_timeStepCount; }
    cvf::Vec3st gridPointDimensions() const { return m_gridPointDimensions; }

    void addLocalGridRefinement(const cvf::Vec3st& minCellPosition, const cvf::Vec3st& maxCellPosition, const cvf::Vec3st& singleCellRefinementFactors);

    void populateReservoir(RigCaseData* eclipseCase);

    bool inputProperty(RigCaseData* eclipseCase, const QString& propertyName, std::vector<double>* values );
    bool staticResult(RigCaseData* eclipseCase, const QString& result, std::vector<double>* values );
    bool dynamicResult(RigCaseData* eclipseCase, const QString& result, size_t stepIndex, std::vector<double>* values );

private:
    void        addWellData(RigCaseData* eclipseCase, RigGridBase* grid);
    static void appendCells(size_t nodeStartIndex, size_t cellCount, RigGridBase* hostGrid, std::vector<RigCell>& cells);
	
    static void appendNodes(const cvf::Vec3d& min, const cvf::Vec3d& max, const cvf::Vec3st& cubeDimension, std::vector<cvf::Vec3d>& nodes);
    static void appendCubeNodes(const cvf::Vec3d& min, const cvf::Vec3d& max, std::vector<cvf::Vec3d>& nodes);

    size_t cellIndexFromIJK(size_t i, size_t j, size_t k) const
    {
        CVF_TIGHT_ASSERT(i < (m_gridPointDimensions.x() - 1));
        CVF_TIGHT_ASSERT(j < (m_gridPointDimensions.y() - 1));
        CVF_TIGHT_ASSERT(k < (m_gridPointDimensions.z() - 1));

        size_t ci = i + j*(m_gridPointDimensions.x() - 1) + k*((m_gridPointDimensions.x() - 1)*(m_gridPointDimensions.y() - 1));
        return ci;
    }

    cvf::Vec3st cellDimension()
    {
        return cvf::Vec3st(m_gridPointDimensions.x() - 1, m_gridPointDimensions.y() - 1, m_gridPointDimensions.z() - 1);
    }


private:
    cvf::Vec3d m_minWorldCoordinate;
    cvf::Vec3d m_maxWorldCoordinate;
    cvf::Vec3st m_gridPointDimensions;
    size_t      m_resultCount;
    size_t      m_timeStepCount;

    std::vector<LocalGridRefinement> m_localGridRefinements;
};


