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

#include "RigNncConnection.h"
#include "RigReservoirBuilder.h"

#include "cvfArray.h"
#include "cvfObject.h"
#include "cvfVector3.h"

class RigEclipseCaseData;
class RigMainGrid;
class RigGridBase;
class RigCell;

class QString;

class RigReservoirBuilderMock
{
public:
    RigReservoirBuilderMock();

    void setWorldCoordinates( cvf::Vec3d minWorldCoordinate, cvf::Vec3d maxWorldCoordinate );
    void setCellCounts( const cvf::Vec3st& cellCounts );
    void setResultInfo( size_t resultCount, size_t timeStepCount );
    void enableWellData( bool enableWellData );

    size_t resultCount() const { return m_resultCount; }
    size_t timeStepCount() const { return m_timeStepCount; }

    void addLocalGridRefinement( const cvf::Vec3st& minCellPosition,
                                 const cvf::Vec3st& maxCellPosition,
                                 const cvf::Vec3st& singleCellRefinementFactors );

    void populateReservoir( RigEclipseCaseData* eclipseCase );

    bool inputProperty( RigEclipseCaseData* eclipseCase, const QString& propertyName, std::vector<double>* values );
    bool staticResult( RigEclipseCaseData* eclipseCase, const QString& result, std::vector<double>* values );
    bool dynamicResult( RigEclipseCaseData* eclipseCase, const QString& result, size_t stepIndex, std::vector<double>* values );

private:
    void addFaults( RigEclipseCaseData* eclipseCase );

    static void
        addNnc( RigMainGrid* grid, size_t i1, size_t j1, size_t k1, size_t i2, size_t j2, size_t k2, RigConnectionContainer& nncConnections );
    void addWellData( RigEclipseCaseData* eclipseCase, RigGridBase* grid );

private:
    size_t m_resultCount;
    size_t m_timeStepCount;
    bool   m_enableWellData;

    RigReservoirBuilder m_reservoirBuilder;
};
