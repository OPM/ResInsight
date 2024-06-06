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
#include <string>
#include <vector>

namespace Opm::EclIO
{
class EInit;
class ERst;
class EGrid;
} // namespace Opm::EclIO

class RigMainGrid;
class RigGridBase;
class RigEclipseCaseData;
class RigEclipseTimeStepInfo;

namespace caf
{
class ProgressInfo;
}

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

    std::vector<QDateTime> timeStepsOnFile( QString gridFileName );

private:
    void buildMetaData( RigEclipseCaseData* caseData, caf::ProgressInfo& progress );
    bool importGrid( RigMainGrid* mainGrid, RigEclipseCaseData* caseData );
    void transferGeometry( Opm::EclIO::EGrid&  opmMainGrid,
                           Opm::EclIO::EGrid&  opmGrid,
                           RigMainGrid*        riMainGrid,
                           RigGridBase*        riGrid,
                           RigEclipseCaseData* caseData,
                           size_t              matrixActiveStartIndex,
                           size_t              fractureActiveStartIndex );

    void transferStaticNNCData( Opm::EclIO::EGrid& opmMainGrid, std::vector<Opm::EclIO::EGrid>& lgrGrids, RigMainGrid* mainGrid );
    void transferDynamicNNCData( RigMainGrid* mainGrid );

    void locateInitAndRestartFiles( QString gridFileName );
    void setupInitAndRestartAccess();

    std::vector<RigEclipseTimeStepInfo> createFilteredTimeStepInfos();

    struct TimeDataFile
    {
        int    sequenceNumber;
        int    year;
        int    month;
        int    day;
        double simulationTimeFromStart;
    };

    std::vector<TimeDataFile> readTimeSteps();

private:
    std::string m_gridFileName;
    std::string m_initFileName;
    std::string m_restartFileName;
    int         m_gridUnit;

    RigEclipseCaseData* m_eclipseCaseData;

    std::unique_ptr<Opm::EclIO::ERst>  m_restartFile;
    std::unique_ptr<Opm::EclIO::EInit> m_initFile;

    std::vector<std::string> m_gridNames;
};
