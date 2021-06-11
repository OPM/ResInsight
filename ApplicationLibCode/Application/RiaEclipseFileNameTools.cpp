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

#include "RiaEclipseFileNameTools.h"

#include "QFileInfo"

namespace caf
{
template <>
void caf::AppEnum<RiaEclipseFileNameTools::EclipseFileType>::setUp()
{
    addItem( RiaEclipseFileNameTools::EclipseFileType::ECLIPSE_DATA, "DATA", "Data Deck" );
    addItem( RiaEclipseFileNameTools::EclipseFileType::ECLIPSE_GRID, "GRID", "Grid" );
    addItem( RiaEclipseFileNameTools::EclipseFileType::ECLIPSE_EGRID, "EGRID", "Grid" );
    addItem( RiaEclipseFileNameTools::EclipseFileType::ECLIPSE_UNRST, "UNRST", "Unified Restart" );
    addItem( RiaEclipseFileNameTools::EclipseFileType::ECLIPSE_SMSPEC, "SMSPEC", "Summary Specification" );
    addItem( RiaEclipseFileNameTools::EclipseFileType::ECLIPSE_UNSMRY, "UNSMR", "Summary Vectors" );

    addItem( RiaEclipseFileNameTools::EclipseFileType::RESINSIGHT_PROJECT, "rsp", "ResInsight Project" );
}

} // End namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaEclipseFileNameTools::RiaEclipseFileNameTools( const QString& inputFilePath )
{
    QFileInfo fi( inputFilePath );

    m_baseName = fi.absolutePath() + "/" + fi.baseName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaEclipseFileNameTools::findRelatedSummarySpecFile()
{
    return relatedFilePath( EclipseFileType::ECLIPSE_SMSPEC );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaEclipseFileNameTools::findRelatedGridFile()
{
    QString candidate = relatedFilePath( EclipseFileType::ECLIPSE_EGRID );
    if ( !candidate.isEmpty() )
    {
        return candidate;
    }

    return relatedFilePath( EclipseFileType::ECLIPSE_GRID );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaEclipseFileNameTools::findRelatedDataFile()
{
    return relatedFilePath( EclipseFileType::ECLIPSE_DATA );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaEclipseFileNameTools::isProjectFile( const QString& fileName )
{
    return hasMatchingSuffix( fileName, EclipseFileType::RESINSIGHT_PROJECT );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaEclipseFileNameTools::isGridFile( const QString& fileName )
{
    if ( hasMatchingSuffix( fileName, EclipseFileType::ECLIPSE_EGRID ) )
    {
        return true;
    }

    return hasMatchingSuffix( fileName, EclipseFileType::ECLIPSE_GRID );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaEclipseFileNameTools::isSummarySpecFile( const QString& fileName )
{
    return hasMatchingSuffix( fileName, EclipseFileType::ECLIPSE_SMSPEC );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaEclipseFileNameTools::isSummaryDataFilePresent( const QString& smspecFileName )
{
    QFileInfo fi( smspecFileName );
    {
        QString candidateFileName = fi.absolutePath() + '/' + fi.baseName() + ".UNSMRY";
        if ( QFile::exists( candidateFileName ) ) return true;
    }

    {
        QString candidateFileName = fi.absolutePath() + '/' + fi.baseName() + ".FUNSMRY";
        if ( QFile::exists( candidateFileName ) ) return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaEclipseFileNameTools::relatedFilePath( EclipseFileType fileType ) const
{
    const QString extension        = caf::AppEnum<EclipseFileType>::text( fileType );
    const QString completeFilePath = m_baseName + "." + extension;

    QFileInfo fi( completeFilePath );
    if ( fi.exists() )
    {
        return fi.absoluteFilePath();
    }

    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaEclipseFileNameTools::hasMatchingSuffix( const QString& fileName, EclipseFileType fileType )
{
    QFileInfo fi( fileName );

    QString suffix = fi.completeSuffix();

    if ( suffix.compare( caf::AppEnum<EclipseFileType>::text( fileType ), Qt::CaseInsensitive ) == 0 )
    {
        return true;
    }

    return false;
}
