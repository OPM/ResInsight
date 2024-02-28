/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023- Equinor ASA
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

#include <memory>

namespace Opm::EclIO
{
class EInit;
class ERst;
} // namespace Opm::EclIO

class RigMainGrid;

//==================================================================================================
//
//
//==================================================================================================
class RifReaderOpmCommon : public RifReaderInterface
{
public:
    RifReaderOpmCommon();
    ~RifReaderOpmCommon() override;

    bool open( const QString& fileName, RigEclipseCaseData* caseData ) override;

    bool staticResult( const QString& result, RiaDefines::PorosityModelType matrixOrFracture, std::vector<double>* values ) override;
    bool dynamicResult( const QString& result, RiaDefines::PorosityModelType matrixOrFracture, size_t stepIndex, std::vector<double>* values ) override;

private:
    void buildMetaData( RigEclipseCaseData* caseData );
    bool importGrid( RigMainGrid* mainGrid, RigEclipseCaseData* caseData );

    struct TimeDataFile
    {
        int    sequenceNumber;
        int    year;
        int    month;
        int    day;
        double simulationTimeFromStart;
    };

    static std::vector<TimeDataFile> readTimeSteps( std::shared_ptr<Opm::EclIO::ERst> restartFile );
    static void                      readWellCells( std::shared_ptr<Opm::EclIO::ERst> restartFile,
                                                    RigEclipseCaseData*               eclipseCase,
                                                    const std::vector<QDateTime>&     timeSteps );

private:
    std::string m_gridFileName;

    std::shared_ptr<Opm::EclIO::ERst>  m_restartFile;
    std::shared_ptr<Opm::EclIO::EInit> m_initFile;

    std::vector<QDateTime> m_timeSteps;
};
