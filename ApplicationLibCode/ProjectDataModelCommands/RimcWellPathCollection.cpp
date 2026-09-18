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

#include "RimcWellPathCollection.h"

#include "RiaApplication.h"
#include "RiaKeyValueStoreUtil.h"

#include "WellLogCommands/RicWellLogsImportFileFeature.h"
#include "WellPathCommands/RicImportWellPaths.h"

#include "RimEclipseCase.h"
#include "RimModeledWellPath.h"
#include "RimPointBasedWellPath.h"
#include "RimProject.h"
#include "RimWellLogLasFile.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"
#include "RimcDataContainerString.h"

#include "cafPdmFieldScriptingCapability.h"

#include "cvfVector3.h"

#include <QDir>
#include <QFileInfo>

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimWellPathCollection, RimcWellPathCollection_importWellPath, "ImportWellPath" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcWellPathCollection_importWellPath::RimcWellPathCollection_importWellPath( caf::PdmObjectHandle* self )
    : caf::PdmObjectCreationMethod( self )
{
    CAF_PDM_InitObject( "Import Well Path" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_fileName, "FileName", "File Name" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcWellPathCollection_importWellPath::execute()
{
    auto wellPathCollection = self<RimWellPathCollection>();
    if ( !wellPathCollection )
    {
        return std::unexpected( QString( "Well path collection is null. Cannot add well path." ) );
    }

    if ( m_fileName().isEmpty() )
    {
        return std::unexpected( QString( "File name is empty. Cannot add well path." ) );
    }

    QStringList               errorMessages;
    std::vector<RimWellPath*> importedWellPaths = wellPathCollection->addWellPaths( { m_fileName() }, &errorMessages );
    if ( importedWellPaths.empty() )
    {
        if ( !errorMessages.empty() )
        {
            return std::unexpected( errorMessages.join( "\n" ) );
        }
        else
        {
            return std::unexpected( QString( "No well paths were imported from file '%1'." ).arg( m_fileName() ) );
        }
    }

    return importedWellPaths.front();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimcWellPathCollection_importWellPath::classKeywordReturnedType() const
{
    return RimWellPath::classKeywordStatic();
}

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimWellPathCollection,
                                   RimcWellPathCollection_importWellPathFromPointsInternal,
                                   "ImportWellPathFromPointsInternal" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcWellPathCollection_importWellPathFromPointsInternal::RimcWellPathCollection_importWellPathFromPointsInternal( caf::PdmObjectHandle* self )
    : caf::PdmObjectCreationMethod( self )
{
    CAF_PDM_InitObject( "Import Well Path From Points Internal" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_name, "Name", "Name" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_coordinateXKey, "CoordinateXKey", "Coordinate X Key" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_coordinateYKey, "CoordinateYKey", "Coordinate Y Key" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_coordinateZKey, "CoordinateZKey", "Coordinate Z Key" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcWellPathCollection_importWellPathFromPointsInternal::execute()
{
    auto wellPathCollection = self<RimWellPathCollection>();
    if ( !wellPathCollection )
    {
        return std::unexpected( QString( "Well path collection is null. Cannot add well path." ) );
    }

    if ( m_name().isEmpty() )
    {
        return std::unexpected( QString( "Name is empty. Cannot add well path." ) );
    }

    if ( m_coordinateXKey().isEmpty() || m_coordinateYKey().isEmpty() || m_coordinateZKey().isEmpty() )
    {
        return std::unexpected( QString( "Coordinate keys are empty. Cannot add well path." ) );
    }

    // Retrieve coordinates from key-value store
    auto keyValueStore = RiaApplication::instance()->keyValueStore();

    auto xData = keyValueStore->get( m_coordinateXKey().toStdString() );
    auto yData = keyValueStore->get( m_coordinateYKey().toStdString() );
    auto zData = keyValueStore->get( m_coordinateZKey().toStdString() );

    if ( !xData || !yData || !zData )
    {
        return std::unexpected( QString( "Failed to retrieve coordinate data from key-value store." ) );
    }

    std::vector<float> xCoords = RiaKeyValueStoreUtil::convertToFloatVector( xData );
    std::vector<float> yCoords = RiaKeyValueStoreUtil::convertToFloatVector( yData );
    std::vector<float> zCoords = RiaKeyValueStoreUtil::convertToFloatVector( zData );

    if ( xCoords.empty() || yCoords.empty() || zCoords.empty() )
    {
        return std::unexpected( QString( "Failed to convert coordinate data from key-value store." ) );
    }

    if ( xCoords.size() != yCoords.size() || yCoords.size() != zCoords.size() )
    {
        return std::unexpected(
            QString( "Coordinate arrays have different sizes: X=%1, Y=%2, Z=%3" ).arg( xCoords.size() ).arg( yCoords.size() ).arg( zCoords.size() ) );
    }

    // Convert to cvf::Vec3d
    std::vector<cvf::Vec3d> trajectoryPoints;
    for ( size_t i = 0; i < xCoords.size(); ++i )
    {
        trajectoryPoints.push_back(
            cvf::Vec3d( static_cast<double>( xCoords[i] ), static_cast<double>( yCoords[i] ), static_cast<double>( zCoords[i] ) ) );
    }

    // Create the fixed trajectory well path
    auto fixedTrajectoryWellPath = std::make_unique<RimPointBasedWellPath>();
    fixedTrajectoryWellPath->setName( m_name() );
    fixedTrajectoryWellPath->setTrajectoryPoints( trajectoryPoints );

    auto* wellPath = fixedTrajectoryWellPath.release();
    wellPathCollection->addWellPath( wellPath );

    wellPathCollection->updateConnectedEditors();
    wellPathCollection->scheduleRedrawAffectedViews();

    return wellPath;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimcWellPathCollection_importWellPathFromPointsInternal::classKeywordReturnedType() const
{
    return RimPointBasedWellPath::classKeywordStatic();
}

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimWellPathCollection, RimcWellPathCollection_setMswNameGrouping, "setMswNameGrouping" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcWellPathCollection_setMswNameGrouping::RimcWellPathCollection_setMswNameGrouping( caf::PdmObjectHandle* self )
    : caf::PdmVoidObjectMethod( self )
{
    CAF_PDM_InitObject( "Set MSW Name Grouping" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_mswNameGrouping, "MswNameGrouping", "MSW Name Grouping" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcWellPathCollection_setMswNameGrouping::execute()
{
    auto wellPathCollection = self<RimWellPathCollection>();
    if ( !wellPathCollection )
    {
        return std::unexpected( QString( "Well path collection is null. Cannot add well path." ) );
    }

    wellPathCollection->setMswWellPattern( m_mswNameGrouping() );

    return nullptr;
}

namespace
{
//--------------------------------------------------------------------------------------------------
/// Resolve explicit file paths (absolute, or relative to the folder / start dir) and all files in the folder matching
/// the name filters. Missing files are reported as errors.
//--------------------------------------------------------------------------------------------------
std::expected<QStringList, QString> resolveImportFiles( const std::vector<QString>& files, const QString& folder, const QStringList& nameFilters )
{
    QStringList errorMessages;
    QStringList resolvedFiles;

    QDir baseDir = folder.isEmpty() ? QDir( RiaApplication::instance()->startDir() ) : QDir( folder );

    if ( !folder.isEmpty() )
    {
        if ( baseDir.exists() )
        {
            for ( const QString& relativePath : baseDir.entryList( nameFilters, QDir::Files | QDir::NoDotAndDotDot ) )
            {
                resolvedFiles.push_back( baseDir.absoluteFilePath( relativePath ) );
            }
        }
        else
        {
            errorMessages << ( baseDir.absolutePath() + " does not exist" );
        }
    }

    for ( const QString& file : files )
    {
        if ( QFileInfo::exists( file ) )
        {
            resolvedFiles.push_back( file );
        }
        else if ( QFileInfo::exists( baseDir.absoluteFilePath( file ) ) )
        {
            resolvedFiles.push_back( baseDir.absoluteFilePath( file ) );
        }
        else
        {
            errorMessages << ( file + " does not exist" );
        }
    }

    if ( !errorMessages.empty() ) return std::unexpected( errorMessages.join( "\n" ) );

    return resolvedFiles;
}
} // namespace

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimWellPathCollection, RimWellPathCollection_importWellPaths, "importWellPaths" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPathCollection_importWellPaths::RimWellPathCollection_importWellPaths( caf::PdmObjectHandle* self )
    : caf::PdmObjectMethod( self, PdmObjectMethod::NullPointerType::NULL_IS_INVALID, PdmObjectMethod::ResultType::PERSISTENT_FALSE )
{
    CAF_PDM_InitObject( "Import Well Paths", "", "", "Import well paths from files and/or all well path files in a folder" );

    CAF_PDM_InitScriptableField( &m_wellPathFiles, "WellPathFiles", std::vector<QString>(), "Well Path Files", "", "", "Well path files to import" );
    CAF_PDM_InitScriptableField( &m_wellPathFolder,
                                 "WellPathFolder",
                                 QString(),
                                 "Well Path Folder",
                                 "",
                                 "",
                                 "Folder to import all well path files from. Also used to resolve relative file paths." );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathCollection_importWellPaths::setWellPathFiles( const std::vector<QString>& wellPathFiles )
{
    m_wellPathFiles = wellPathFiles;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathCollection_importWellPaths::setWellPathFolder( const QString& wellPathFolder )
{
    m_wellPathFolder = wellPathFolder;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RimWellPathCollection_importWellPaths::warnings() const
{
    return m_warnings;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimWellPathCollection_importWellPaths::execute()
{
    m_warnings.clear();

    auto wellPathCollection = self<RimWellPathCollection>();
    if ( !wellPathCollection ) return std::unexpected( "No well path collection is available." );

    auto files = resolveImportFiles( m_wellPathFiles(), m_wellPathFolder(), RicImportWellPaths::wellPathNameFilters() );
    if ( !files ) return std::unexpected( files.error() );
    if ( files->empty() ) return std::unexpected( "No well path files found" );

    std::vector<RimWellPath*> importedWellPaths = RicImportWellPaths::importWellPaths( files.value(), &m_warnings );

    auto* result = new RimcDataContainerString();
    for ( RimWellPath* wellPath : importedWellPaths )
    {
        result->m_stringValues.v().push_back( wellPath->name() );
    }
    return result;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellPathCollection_importWellPaths::classKeywordReturnedType() const
{
    return RimcDataContainerString::classKeywordStatic();
}

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimWellPathCollection, RimWellPathCollection_importWellLogFiles, "importWellLogFiles" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPathCollection_importWellLogFiles::RimWellPathCollection_importWellLogFiles( caf::PdmObjectHandle* self )
    : caf::PdmObjectMethod( self, PdmObjectMethod::NullPointerType::NULL_IS_INVALID, PdmObjectMethod::ResultType::PERSISTENT_FALSE )
{
    CAF_PDM_InitObject( "Import Well Log Files", "", "", "Import LAS files and attach them to the well paths with matching names" );

    CAF_PDM_InitScriptableField( &m_wellLogFiles, "WellLogFiles", std::vector<QString>(), "Well Log Files", "", "", "Well log files to import" );
    CAF_PDM_InitScriptableField( &m_wellLogFolder,
                                 "WellLogFolder",
                                 QString(),
                                 "Well Log Folder",
                                 "",
                                 "",
                                 "Folder to import all well log files from. Also used to resolve relative file paths." );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathCollection_importWellLogFiles::setWellLogFiles( const std::vector<QString>& wellLogFiles )
{
    m_wellLogFiles = wellLogFiles;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathCollection_importWellLogFiles::setWellLogFolder( const QString& wellLogFolder )
{
    m_wellLogFolder = wellLogFolder;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RimWellPathCollection_importWellLogFiles::warnings() const
{
    return m_warnings;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimWellPathCollection_importWellLogFiles::execute()
{
    m_warnings.clear();

    auto wellPathCollection = self<RimWellPathCollection>();
    if ( !wellPathCollection ) return std::unexpected( "No well path collection is available." );

    auto files = resolveImportFiles( m_wellLogFiles(), m_wellLogFolder(), RicWellLogsImportFileFeature::wellLogFileNameFilters() );
    if ( !files ) return std::unexpected( files.error() );
    if ( files->empty() ) return std::unexpected( "No well log files found" );

    std::vector<RimWellLogLasFile*> importedFiles = RicWellLogsImportFileFeature::importWellLogFiles( files.value(), &m_warnings );

    auto* result = new RimcDataContainerString();
    for ( RimWellLogLasFile* wellLogFile : importedFiles )
    {
        result->m_stringValues.v().push_back( wellLogFile->wellName() );
    }
    return result;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellPathCollection_importWellLogFiles::classKeywordReturnedType() const
{
    return RimcDataContainerString::classKeywordStatic();
}
