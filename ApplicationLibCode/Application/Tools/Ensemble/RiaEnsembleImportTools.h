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

#include "Summary/RiaSummaryDefines.h"

#include <QStringList>

class RimSummaryCase;

namespace RiaEnsembleImportTools
{
struct CreateConfig
{
    RiaDefines::FileType fileType;
    bool                 ensembleOrGroup;
    bool                 allowDialogs;
    bool                 buildSummaryAddresses = true;
};
std::vector<RimSummaryCase*> createSummaryCasesFromFiles( const QStringList& fileNames, CreateConfig createConfig );

std::pair<QString, QString> findPathPattern( const QStringList& filePaths, const QString& placeholderString );

QStringList createPathsFromPattern( const QString& basePath, const QString& numberRange, const QString& placeholderString );
QStringList createPathsBySearchingFileSystem( const QString& pathPattern, const QString& placeholderString, const QString& enumerationString );
QStringList getMatchingFiles( const QString& basePath, const QString& regexPattern );

} // namespace RiaEnsembleImportTools
