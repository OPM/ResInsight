/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
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

#include "RiaDefines.h"

#include "ert/ecl/ecl_sum.hpp"

#include <QString>
#include <QStringList>

#include <string>
#include <vector>

class RifSummaryReaderInterface;

//==================================================================================================
//
//
//==================================================================================================
class RifRestartFileInfo
{
public:
    RifRestartFileInfo()
        : startDate( 0 )
        , endDate( 0 )
    {
    }
    RifRestartFileInfo( const QString& _fileName, time_t _startDate, time_t _endDate )
        : fileName( _fileName )
        , startDate( _startDate )
        , endDate( _endDate )
    {
    }
    bool valid() { return !fileName.isEmpty(); }

    QString fileName;
    time_t  startDate;
    time_t  endDate;
};

//==================================================================================================
//
//
//
//==================================================================================================
class RifEclipseSummaryTools
{
public:
    static void    findSummaryHeaderFile( const QString& inputFile, QString* headerFile, bool* isFormatted );
    static QString findGridCaseFileFromSummaryHeaderFile( const QString& summaryHeaderFile );

    static void dumpMetaData( RifSummaryReaderInterface* readerEclipseSummary );

    static std::vector<RifRestartFileInfo> getRestartFiles( const QString& headerFileName, std::vector<QString>& warnings );
    static RifRestartFileInfo              getFileInfo( const QString& headerFileName );

    static void                          closeEclSum( ecl_sum_type* ecl_sum );
    static ecl_sum_type*                 openEclSum( const QString& inHeaderFileName, bool includeRestartFiles );
    static RiaDefines::EclipseUnitSystem readUnitSystem( ecl_sum_type* ecl_sum );
    static std::vector<time_t>           getTimeSteps( ecl_sum_type* ecl_sum );

private:
    static void               findSummaryFiles( const QString& inputFile, QString* headerFile, QStringList* dataFiles );
    static RifRestartFileInfo getRestartFile( const QString& headerFileName );

    static void findSummaryHeaderFileInfo( const QString& inputFile, QString* headerFile, QString* path, QString* base, bool* isFormatted );
};
