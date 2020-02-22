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

#include "RifReaderInterface.h"
#include "RigReservoirBuilderMock.h"

class RifReaderMockModel : public RifReaderInterface
{
public:
    RifReaderMockModel();
    ~RifReaderMockModel() override;

    void setWorldCoordinates( cvf::Vec3d minWorldCoordinate, cvf::Vec3d maxWorldCoordinate );
    void setGridPointDimensions( const cvf::Vec3st& gridPointDimensions );
    void setResultInfo( size_t resultCount, size_t timeStepCount );
    void enableWellData( bool enableWellData );

    void addLocalGridRefinement( const cvf::Vec3st& minCellPosition,
                                 const cvf::Vec3st& maxCellPosition,
                                 const cvf::Vec3st& singleCellRefinementFactors );

    bool open( const QString& fileName, RigEclipseCaseData* eclipseCase ) override;

    bool staticResult( const QString&                result,
                       RiaDefines::PorosityModelType matrixOrFracture,
                       std::vector<double>*          values ) override;
    bool dynamicResult( const QString&                result,
                        RiaDefines::PorosityModelType matrixOrFracture,
                        size_t                        stepIndex,
                        std::vector<double>*          values ) override;

private:
    void populateReservoir( RigEclipseCaseData* eclipseCase );
    bool inputProperty( const QString& propertyName, std::vector<double>* values );

    RigReservoirBuilderMock m_reservoirBuilder;
    RigEclipseCaseData*     m_reservoir;
};
