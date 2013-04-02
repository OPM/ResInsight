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

#include "RifReaderInterface.h"
#include "RigReservoirBuilderMock.h"

class RifReaderMockModel : public RifReaderInterface
{
public:
    RifReaderMockModel();
    virtual ~RifReaderMockModel();

    void                    setWorldCoordinates(cvf::Vec3d minWorldCoordinate, cvf::Vec3d maxWorldCoordinate);
    void                    setGridPointDimensions(const cvf::Vec3st& gridPointDimensions);
    void                    setResultInfo(size_t resultCount, size_t timeStepCount);

    void                    addLocalGridRefinement(const cvf::Vec3st& minCellPosition, const cvf::Vec3st& maxCellPosition, const cvf::Vec3st& singleCellRefinementFactors);

    virtual bool            open( const QString& fileName, RigCaseData* eclipseCase );
    virtual void            close();
                            
    virtual bool            staticResult( const QString& result, RifReaderInterface::PorosityModelResultType matrixOrFracture, std::vector<double>* values );
    virtual bool            dynamicResult( const QString& result, RifReaderInterface::PorosityModelResultType matrixOrFracture, size_t stepIndex, std::vector<double>* values );

private:
    void                    populateReservoir(RigCaseData* eclipseCase);
    bool                    inputProperty( const QString& propertyName, std::vector<double>* values );

    RigReservoirBuilderMock m_reservoirBuilder;
    RigCaseData*         m_reservoir;
};

