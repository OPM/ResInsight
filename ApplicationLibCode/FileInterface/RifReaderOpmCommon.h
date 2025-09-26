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

#include <QDateTime>

namespace Opm::EclIO
{
class EInit;
class ERst;
class EGrid;
} // namespace Opm::EclIO

class RigMainGrid;
class RigActiveCellGrid;
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

    bool isRadialGrid() const override;

protected:
    virtual bool importGrid( RigMainGrid* mainGrid, RigEclipseCaseData* caseData );

    void transferActiveCells( Opm::EclIO::EGrid&  opmGrid,
                              size_t              cellStartIndex,
                              RigEclipseCaseData* eclipseCaseData,
                              size_t              matrixActiveStartIndex,
                              size_t              fractureActiveStartIndex );
    void transferStaticNNCData( Opm::EclIO::EGrid& opmMainGrid, std::vector<Opm::EclIO::EGrid>& lgrGrids, RigMainGrid* mainGrid );

    bool verifyActiveCellInfo( int activeSizeMat, int activeSizeFrac );
    void updateActiveCellInfo( RigEclipseCaseData*             eclipseCaseData,
                               Opm::EclIO::EGrid&              opmGrid,
                               std::vector<Opm::EclIO::EGrid>& lgrGrids,
                               RigMainGrid*                    mainGrid );

private:
    void buildMetaData( RigEclipseCaseData* caseData, caf::ProgressInfo& progress );

    std::vector<RigEclipseTimeStepInfo> createFilteredTimeStepInfos();
    std::vector<std::vector<int>>       readActiveCellInfoFromPorv( RigEclipseCaseData* eclipseCaseData, bool isDualPorosity );

    void transferGeometry( Opm::EclIO::EGrid&  opmMainGrid,
                           Opm::EclIO::EGrid&  opmGrid,
                           RigMainGrid*        riMainGrid,
                           RigGridBase*        riGrid,
                           RigEclipseCaseData* caseData );
    void transferDynamicNNCData( RigMainGrid* mainGrid );

    void locateInitAndRestartFiles( QString gridFileName );
    void setupInitAndRestartAccess();

    struct TimeDataFile
    {
        int    sequenceNumber;
        int    year;
        int    month;
        int    day;
        double simulationTimeFromStart;
    };

    std::vector<TimeDataFile> readTimeSteps();
    QDateTime dateTimeFromTimeStepOnFile( RifReaderOpmCommon::TimeDataFile timeOnFile, QDate startDate, double startDayOffset );

protected:
    enum class ActiveType
    {
        ACTIVE_MATRIX_VALUE   = 1,
        ACTIVE_FRACTURE_VALUE = 2
    };

    std::string              m_gridFileName;
    int                      m_gridUnit;
    std::vector<std::string> m_gridNames;

    RigEclipseCaseData* m_eclipseCaseData;

private:
    std::string                        m_initFileName;
    std::string                        m_restartFileName;
    std::unique_ptr<Opm::EclIO::ERst>  m_restartFile;
    std::unique_ptr<Opm::EclIO::EInit> m_initFile;
    bool                               m_radialGridDetected = false;
};
