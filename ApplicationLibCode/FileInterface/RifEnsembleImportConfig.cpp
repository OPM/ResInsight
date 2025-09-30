/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025     Equinor ASA
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

#include "RifEnsembleImportConfig.h"

#include "Ensemble/RiaEnsembleImportTools.h"
#include "RiaLogging.h"

#include "RifCaseRealizationParametersReader.h"
#include "RifEclipseSummaryTools.h"
#include "RifOpmSummaryTools.h"

#include <QStringList>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifEnsembleImportConfig::useConfigValues() const
{
    return m_useConfigValues;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> RifEnsembleImportConfig::restartFilesForRealization( int realizationNumber ) const
{
    QString numberString = QString::number( realizationNumber );

    std::vector<QString> filePaths;
    for ( auto pattern : m_restartFileNamePatterns )
    {
        filePaths.push_back( pattern.replace( placeholderText(), numberString ) );
    }

    return filePaths;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifEnsembleImportConfig::computePatternsFromSummaryFilePaths( const QString& filePath1, const QString& filePath2 )
{
    std::vector<QString> warnings;

    auto restartFileNames1 = RifEclipseSummaryTools::getRestartFileNamesOpm( filePath1, warnings );
    auto paramFilePath1    = RifCaseRealizationParametersFileLocator::locate( filePath1 );

    auto restartFileNames2 = RifEclipseSummaryTools::getRestartFileNamesOpm( filePath2, warnings );
    auto paramFilePath2    = RifCaseRealizationParametersFileLocator::locate( filePath2 );

    computeRestartPatternsFromTwoRealizations( restartFileNames1, restartFileNames2 );
    computeParameterFilePathPattern( { paramFilePath1, paramFilePath2 } );

    m_useConfigValues = true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifEnsembleImportConfig::computeRestartPatternsFromTwoRealizations( const std::vector<QString>& restartFilesFirstCase,
                                                                         const std::vector<QString>& restartFilesSecondCase )
{
    m_restartFileNamePatterns.clear();

    if ( restartFilesFirstCase.empty() ) return;

    if ( restartFilesFirstCase.size() != restartFilesSecondCase.size() )
    {
        RiaLogging::error( "RifEnsembleImportConfig::computeRestartPatternsFromTwoRealizations: "
                           "The number of restart files for the two realizations do not match." );
        return;
    }

    std::vector<QString> restartPatterns;

    for ( size_t i = 0; i < restartFilesFirstCase.size(); i++ )
    {
        QStringList filePaths;
        filePaths.push_back( restartFilesFirstCase[i] );
        filePaths.push_back( restartFilesSecondCase[i] );

        const auto [pattern, range] = RiaEnsembleImportTools::findPathPattern( filePaths, RifEnsembleImportConfig::placeholderText() );
        restartPatterns.push_back( pattern );
    }

    m_restartFileNamePatterns = restartPatterns;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RifEnsembleImportConfig::pathToParameterFile( int realizationNumber ) const
{
    QString numberString = QString::number( realizationNumber );

    auto parameterFilePath = m_parameterFilePathPattern;
    parameterFilePath.replace( placeholderText(), numberString );
    return parameterFilePath;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifEnsembleImportConfig::computeParameterFilePathPattern( const std::vector<QString>& filePaths )
{
    m_parameterFilePathPattern.clear();
    if ( filePaths.size() < 2 )
    {
        RiaLogging::error( "RifEnsembleImportConfig::computeParameterFilePathPattern: "
                           "At least two parameter files are required to compute a pattern." );
        return;
    }

    QStringList filePathsList;
    for ( const auto& filePath : filePaths )
    {
        filePathsList.push_back( filePath );
    }

    const auto [pattern, range] = RiaEnsembleImportTools::findPathPattern( filePathsList, RifEnsembleImportConfig::placeholderText() );

    m_parameterFilePathPattern = pattern;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RifEnsembleImportConfig::placeholderText()
{
    return "$INDEX";
}
