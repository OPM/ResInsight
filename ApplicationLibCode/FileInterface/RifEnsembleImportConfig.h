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

#pragma once

#include <QString>

#include <vector>

class RifEnsembleImportConfig
{
public:
    void computePatternsFromSummaryFilePaths( const QString& filePath1, const QString& filePath2 );

    std::vector<QString> restartFilesForRealization( int realizationNumber ) const;
    QString              pathToParameterFile( int realizationNumber ) const;
    bool                 useConfigValues() const;

private:
    void computeRestartPatternsFromTwoRealizations( const std::vector<QString>& restartFilesFirstCase,
                                                    const std::vector<QString>& restartFilesSecondCase );

    void computeParameterFilePathPattern( const std::vector<QString>& filePaths );

    static QString placeholderText();

private:
    bool                 m_useConfigValues = false;
    std::vector<QString> m_restartFileNamePatterns;
    QString              m_parameterFilePathPattern;
};
