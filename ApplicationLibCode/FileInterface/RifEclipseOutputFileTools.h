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

#include "RifEclipseReportKeywords.h"

#include "RiaDefines.h"
#include "RiaPorosityModel.h"

#include "ert/ecl/ecl_file_view.h"
#include "ert/ecl/ecl_grid.h"
#include "ert/ecl/ecl_util.h"

#include "cvfObject.h"

#include <QDateTime>
#include <QString>
#include <QStringList>

#include <optional>
#include <vector>

using ecl_file_type = struct ecl_file_struct;

class RifEclipseRestartDataAccess;
class RigEclipseTimeStepInfo;
class RigActiveCellInfo;
class RigEclipseCaseData;

class QByteArray;

//==================================================================================================
//
//
//
//==================================================================================================
class RifEclipseOutputFileTools
{
public:
    RifEclipseOutputFileTools();
    virtual ~RifEclipseOutputFileTools();

    static std::vector<RifEclipseKeywordValueCount> keywordValueCounts( const std::vector<ecl_file_type*>& ecl_files );

    static void createResultEntries( const std::vector<RifEclipseKeywordValueCount>& fileKeywordInfo,
                                     const std::vector<RigEclipseTimeStepInfo>&      timeStepInfo,
                                     RiaDefines::ResultCatType                       resultCategory,
                                     RigEclipseCaseData*                             eclipseCaseData,
                                     size_t                                          totalTimeSteps );

    static bool keywordData( const ecl_file_type* ecl_file, const QString& keyword, size_t fileKeywordOccurrence, std::vector<double>* values );
    static bool keywordData( const ecl_file_type* ecl_file, const QString& keyword, size_t fileKeywordOccurrence, std::vector<int>* values );

    static void timeSteps( const ecl_file_type*    ecl_file,
                           std::vector<QDateTime>* timeSteps,
                           std::vector<double>*    daysSinceSimulationStart,
                           size_t*                 perTimeStepHeaderKeywordCount );

    static bool       isValidEclipseFileName( const QString& fileName );
    static QByteArray md5sum( const QString& fileName );
    static bool       findSiblingFilesWithSameBaseName( const QString& fileName, QStringList* fileSet );

    static QString     firstFileNameOfType( const QStringList& fileSet, ecl_file_enum fileType );
    static QStringList filterFileNamesOfType( const QStringList& fileSet, ecl_file_enum fileType );

    static void readGridDimensions( const QString& gridFileName, std::vector<std::vector<int>>& gridDimensions );

    static int readUnitsType( const ecl_file_type* ecl_file );

    static cvf::ref<RifEclipseRestartDataAccess> createDynamicResultAccess( const QString& fileName );

    static QString createIndexFileName( const QString& resultFileName );

    static std::set<RiaDefines::PhaseType> findAvailablePhases( const ecl_file_type* ecl_file );

    static void transferNncFluxData( const ecl_grid_type*      grid,
                                     const ecl_file_view_type* summaryView,
                                     std::vector<double>*      waterFlux,
                                     std::vector<double>*      oilFlux,
                                     std::vector<double>*      gasFlux );

    static bool isExportedFromIntersect( const ecl_file_type* ecl_file );

    static FILE* fopen( const QString& filePath, const QString& mode );

    static bool assignActiveCellData( std::vector<std::vector<int>>& actnumValuesPerGrid, RigEclipseCaseData* eclipseCaseData );

    static std::vector<RifEclipseKeywordValueCount>
        validKeywordsForPorosityModel( const std::vector<RifEclipseKeywordValueCount>& keywordItemCounts,
                                       const RigActiveCellInfo*                        activeCellInfo,
                                       const RigActiveCellInfo*                        fractureActiveCellInfo,
                                       RiaDefines::PorosityModelType                   matrixOrFracture,
                                       size_t                                          timeStepCount );

    static void extractResultValuesBasedOnPorosityModel( RigEclipseCaseData*           eclipseCaseData,
                                                         RiaDefines::PorosityModelType matrixOrFracture,
                                                         std::vector<double>*          values,
                                                         const std::vector<double>&    fileValues );

    // Unit system handling utilities
    static std::optional<RiaDefines::EclipseUnitSystem> unitValueToEnum( int unitValue );
    static RiaDefines::EclipseUnitSystem                determineUnitSystem( const std::optional<RiaDefines::EclipseUnitSystem>& egridUnit,
                                                                             const std::optional<RiaDefines::EclipseUnitSystem>& initUnit,
                                                                             const std::optional<RiaDefines::EclipseUnitSystem>& unrstUnit );

private:
    static void                     getDayMonthYear( const ecl_kw_type* intehead_kw, int* day, int* month, int* year );
    static RifEclipseReportKeywords createReportStepsMetaData( const std::vector<ecl_file_type*>& ecl_files );
};
