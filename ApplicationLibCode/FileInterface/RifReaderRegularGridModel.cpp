/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024-     Equinor ASA
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

#include "RifReaderRegularGridModel.h"

#include "RiaDefines.h"
#include "RiaLogging.h"

#include "RifEclipseInputFileTools.h"
#include "RifEclipseInputPropertyLoader.h"
#include "RifEclipseKeywordContent.h"
#include "RifEclipseTextFileReader.h"

#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseResultAddress.h"

#include "RimEclipseCase.h"

#include <QDir>
#include <QFileInfo>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifReaderRegularGridModel::writeCache( const QString& fileName, RimEclipseCase* eclipseCase )
{
    if ( !eclipseCase ) return;

    QFileInfo storageFileInfo( fileName );
    if ( storageFileInfo.exists() )
    {
        QDir storageDir = storageFileInfo.dir();
        storageDir.remove( storageFileInfo.fileName() );
    }

    QDir::root().mkpath( storageFileInfo.absolutePath() );

    QFile cacheFile( fileName );
    if ( !cacheFile.open( QIODevice::WriteOnly ) )
    {
        RiaLogging::error( "Saving project: Can't open the cache file : " + fileName );
        return;
    }

    auto rigCellResults = eclipseCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL );
    if ( !rigCellResults ) return;

    auto resultNames = rigCellResults->resultNames( RiaDefines::ResultCatType::GENERATED );

    std::vector<QString> keywords;
    for ( const auto& resultName : resultNames )
    {
        keywords.push_back( resultName );
    }

    const bool writeEchoKeywords = false;
    if ( !RifEclipseInputFileTools::exportKeywords( fileName, eclipseCase->eclipseCaseData(), keywords, writeEchoKeywords ) )
    {
        RiaLogging::error( "Error detected when writing the cache file : " + fileName );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifReaderRegularGridModel::ensureDataIsReadFromCache( const QString& fileName, RimEclipseCase* eclipseCase )
{
    if ( !eclipseCase ) return;

    auto rigCellResults = eclipseCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL );
    if ( !rigCellResults ) return;

    // Early return if we have already read the data from the cache
    auto existingResultNames = rigCellResults->resultNames( RiaDefines::ResultCatType::GENERATED );
    if ( !existingResultNames.empty() ) return;

    auto keywordsAndData = RifEclipseTextFileReader::readKeywordAndValues( fileName.toStdString() );
    for ( const auto& content : keywordsAndData )
    {
        const auto resultName = QString::fromStdString( content.keyword );

        RigEclipseResultAddress resAddr( RiaDefines::ResultCatType::GENERATED, RiaDefines::ResultDataType::FLOAT, resultName );
        rigCellResults->createResultEntry( resAddr, false );

        auto newPropertyData = rigCellResults->modifiableCellScalarResultTimesteps( resAddr );

        std::vector<double> doubleVals;
        doubleVals.insert( doubleVals.begin(), content.values.begin(), content.values.end() );

        newPropertyData->push_back( doubleVals );
    }
}
