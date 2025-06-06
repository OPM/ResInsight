/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RiaEclipseFileNameTools
{
public:
    enum class EclipseFileType
    {
        ECLIPSE_DATA,
        ECLIPSE_GRID,
        ECLIPSE_EGRID,
        ECLIPSE_INIT,
        ECLIPSE_UNRST,
        ECLIPSE_SMSPEC,
        ECLIPSE_UNSMRY,
        ECLIPSE_ESMRY,
        RESINSIGHT_PROJECT,
        UNKNOWN
    };

public:
    explicit RiaEclipseFileNameTools( const QString& fileName );

    // Returns both files ending with SMSPEC and ESMRY
    std::vector<QString> findSummaryFileCandidates();
    QString              findRelatedGridFile();
    QString              findRelatedDataFile();

private:
    QString relatedFilePath( EclipseFileType fileType ) const;

    static bool hasMatchingSuffix( const QString& fileName, EclipseFileType fileType );

private:
    QString m_baseName;
};
