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

#include "WellPathCommands/RicImportWellPaths.h"

#include "CompletionData/RimCompletionData.h"
#include "RimEclipseCase.h"
#include "RimEclipseResultCase.h"
#include "RimModeledWellPath.h"
#include "RimPointBasedWellPath.h"
#include "RimProject.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"

#include "cafPdmFieldScriptingCapability.h"

#include "cvfVector3.h"

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

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimWellPathCollection, RimcWellPathCollection_wellCompletions, "WellCompletions" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcWellPathCollection_wellCompletions::RimcWellPathCollection_wellCompletions( caf::PdmObjectHandle* self )
    : caf::PdmObjectMethod( self, PdmObjectMethod::NullPointerType::NULL_IS_INVALID, PdmObjectMethod::ResultType::PERSISTENT_FALSE )
{
    CAF_PDM_InitObject( "Well Completions" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_wellName, "WellName", "Name of Well" );
    CAF_PDM_InitScriptableField( &m_caseId, "eclipseCaseId", -1, "Eclipse Case to extract data from" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcWellPathCollection_wellCompletions::execute()
{
    auto wellPathCollection = self<RimWellPathCollection>();
    if ( !wellPathCollection )
    {
        return std::unexpected( QString( "Well path collection is missing. Cannot get completion data." ) );
    }

    if ( m_wellName().isEmpty() )
    {
        return std::unexpected( QString( "Well name is empty. Nothing to do." ) );
    }

    if ( m_caseId() < 0 )
    {
        return std::unexpected( QString( "Eclipse case is not set. Cannot get completion data." ) );
    }

    RimEclipseCase* eCase = RimProject::current()->eclipseCaseFromCaseId( m_caseId() );
    if ( eCase == nullptr )
    {
        return std::unexpected( QString( "Eclipse case is not set. Cannot get completion data." ) );
    }

    if ( auto wellPath = wellPathCollection->wellPathByName( m_wellName ) )
    {
        if ( auto modWellPath = dynamic_cast<RimModeledWellPath*>( wellPath ) )
        {
            if ( auto completionData = modWellPath->completionData( eCase ) )
            {
                return completionData;
            }
            else
            {
                return std::unexpected( QString( "Well path '%1' does not have any completion data." ).arg( m_wellName ) );
            }
        }
    }
    return std::unexpected( QString( "Modeled Well path with name '%1' does not exist. Cannot get completion data." ).arg( m_wellName ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimcWellPathCollection_wellCompletions::classKeywordReturnedType() const
{
    return RimCompletionData::classKeywordStatic();
}
