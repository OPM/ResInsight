/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021     Equinor ASA
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

#include <QString>

#include <map>
#include <vector>

class RimSummaryCase;

//==================================================================================================
//
//==================================================================================================
class RiaEnsembleNameTools
{
public:
    enum class EnsembleGroupingMode
    {
        FMU_FOLDER_STRUCTURE,
        EVEREST_FOLDER_STRUCTURE,
        NONE
    };

public:
    static QString findSuitableEnsembleName( const QStringList& fileNames, EnsembleGroupingMode groupingMode );
    static QString findCommonBaseName( const QStringList& fileNames );

    static QString
        uniqueShortName( const QString& sourceFileName, const QStringList& allFileNames, const QString& ensembleCaseName = QString() );

    static QString uniqueShortNameFromComponents( const QString&                        sourceFileName,
                                                  const std::map<QString, QStringList>& keyFileComponentsForAllFiles,
                                                  const QString&                        ensembleCaseName );

    static std::vector<QStringList>       groupFilesByEnsemble( const QStringList& fileNames, EnsembleGroupingMode groupingMode );
    static std::map<QString, QStringList> groupFilesByStimPlanEnsemble( const QStringList& fileNames );

    static std::map<QString, std::pair<QString, QString>>
        findUniqueStimPlanEnsembleNames( const QStringList& fileNames, const std::vector<QStringList>& fileNameComponents );

    static QString uniqueShortNameForEnsembleCase( RimSummaryCase* summaryCase );
    static QString uniqueShortNameForSummaryCase( RimSummaryCase* summaryCase );

private:
    static QStringList findUniqueEnsembleNames( const QStringList&              fileNames,
                                                const std::vector<QStringList>& fileNameComponents,
                                                EnsembleGroupingMode            groupingMode );
};
