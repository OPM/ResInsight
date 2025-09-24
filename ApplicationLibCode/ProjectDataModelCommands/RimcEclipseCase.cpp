/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020- Equinor ASA
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

#include "RimcEclipseCase.h"

#include "RiaApplication.h"
#include "RiaGuiApplication.h"
#include "RiaKeyValueStoreUtil.h"

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseResultAddress.h"
#include "RigGridExportAdapter.h"
#include "RigMainGrid.h"
#include "RigResdataGridConverter.h"
#include "RigResultAccessor.h"
#include "RigResultAccessorFactory.h"

#include "RimCase.h"
#include "RimEclipseCase.h"
#include "RimProject.h"

#include "cafPdmFieldScriptingCapability.h"

#include <QDir>
#include <QFileInfo>

#include <expected>
#include <limits>

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimEclipseCase, RimcEclipseCase_importProperties, "import_properties" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcEclipseCase_importProperties::RimcEclipseCase_importProperties( caf::PdmObjectHandle* self )
    : caf::PdmVoidObjectMethod( self )
{
    CAF_PDM_InitObject( "Import Properties", "", "", "Import Properties" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_fileNames, "FileNames", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcEclipseCase_importProperties::execute()
{
    std::vector<QString> absolutePaths = m_fileNames;
    for ( auto& path : absolutePaths )
    {
        QFileInfo projectPathInfo( path );
        if ( !projectPathInfo.exists() )
        {
            QDir startDir( RiaApplication::instance()->startDir() );
            path = startDir.absoluteFilePath( path );
        }
    }

    QStringList propertyFileNames;
    std::copy( absolutePaths.begin(), absolutePaths.end(), std::back_inserter( propertyFileNames ) );

    auto eclipseCase = self<RimEclipseCase>();
    eclipseCase->importAsciiInputProperties( propertyFileNames );

    return nullptr;
}

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimEclipseCase, RimcEclipseCase_exportValuesInternal, "export_values_internal" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcEclipseCase_exportValuesInternal::RimcEclipseCase_exportValuesInternal( caf::PdmObjectHandle* self )
    : caf::PdmVoidObjectMethod( self )
{
    CAF_PDM_InitObject( "Export Values Internal", "", "", "Export Values Internal" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_coordinateX, "CoordinateX", "" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_coordinateY, "CoordinateY", "" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_coordinateZ, "CoordinateZ", "" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_resultKey, "ResultKey", "" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_propertyType, "PropertyType", "" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_propertyName, "PropertyName", "" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_timeStep, "TimeStep", "" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_porosityModel, "PorosityModel", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcEclipseCase_exportValuesInternal::execute()
{
    auto eclipseCase = self<RimEclipseCase>();

    RigEclipseCaseData* eclipseCaseData = eclipseCase->eclipseCaseData();
    if ( !eclipseCaseData )
    {
        return std::unexpected( "Unable to load eclipse case data." );
    }

    const RigMainGrid* mainGrid = eclipseCase->mainGrid();
    if ( !mainGrid )
    {
        return std::unexpected( "Unable to load main grid for eclipse case." );
    }

    caf::AppEnum<RiaDefines::ResultCatType> resultCatTypeEnum;
    if ( !resultCatTypeEnum.setFromText( m_propertyType ) )
    {
        return std::unexpected( "Invalid property type." );
    }

    caf::AppEnum<RiaDefines::PorosityModelType> porosityModel;
    if ( !porosityModel.setFromText( m_porosityModel ) )
    {
        return std::unexpected( "Invalid porosity model." );
    }

    RigEclipseResultAddress resVarAddr( resultCatTypeEnum, m_propertyName );
    eclipseCase->results( porosityModel )->ensureKnownResultLoaded( resVarAddr );

    cvf::ref<RigResultAccessor> resultAccessor =
        RigResultAccessorFactory::createFromResultAddress( eclipseCaseData, 0, porosityModel, m_timeStep, resVarAddr );
    if ( resultAccessor.isNull() )
    {
        return std::unexpected( "Result property not found." );
    }

    if ( m_timeStep < 0 || m_timeStep >= static_cast<int>( eclipseCase->timeStepDates().size() ) )
    {
        return std::unexpected( "Invalid time step." );
    }

    auto keyValueStore = RiaApplication::instance()->keyValueStore();

    std::vector<float> xs = RiaKeyValueStoreUtil::convertToFloatVector( keyValueStore->get( m_coordinateX().toStdString() ) );
    std::vector<float> ys = RiaKeyValueStoreUtil::convertToFloatVector( keyValueStore->get( m_coordinateY().toStdString() ) );
    std::vector<float> zs = RiaKeyValueStoreUtil::convertToFloatVector( keyValueStore->get( m_coordinateZ().toStdString() ) );
    if ( xs.empty() || xs.size() != ys.size() || xs.size() != zs.size() )
    {
        return std::unexpected( "Invalid positions specified." );
    }

    std::vector<cvf::Vec3d> positions;
    for ( size_t i = 0; i < xs.size(); i++ )
    {
        positions.push_back( cvf::Vec3d( xs[i], ys[i], -zs[i] ) );
    }

    std::vector<float> values;
    for ( const cvf::Vec3d& position : positions )
    {
        auto cellIdx = mainGrid->findReservoirCellIndexFromPoint( position );
        if ( cellIdx != cvf::UNDEFINED_SIZE_T )
        {
            float valueFromEclipse = static_cast<float>( resultAccessor->cellScalar( cellIdx ) );
            values.push_back( valueFromEclipse );
        }
        else
        {
            // Use inf as signal that nothing was found for the position
            values.push_back( std::numeric_limits<float>::infinity() );
        }
    }

    keyValueStore->set( m_resultKey().toStdString(), RiaKeyValueStoreUtil::convertToByteVector( values ) );

    return nullptr;
}

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimEclipseCase, RimcEclipseCase_exportCornerPointGridInternal, "export_corner_point_grid_internal" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcEclipseCase_exportCornerPointGridInternal::RimcEclipseCase_exportCornerPointGridInternal( caf::PdmObjectHandle* self )
    : caf::PdmVoidObjectMethod( self )
{
    CAF_PDM_InitObject( "Export Corner Point Grid Internal", "", "", "Export Corner Point Grid Internal" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_zcornKey, "ZcornKey", "" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_coordKey, "CoordKey", "" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_actnumKey, "ActnumKey", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcEclipseCase_exportCornerPointGridInternal::execute()
{
    auto eclipseCase = self<RimEclipseCase>();
    if ( !eclipseCase )
    {
        return std::unexpected( "Unable to get Eclipse case." );
    }

    RigEclipseCaseData* eclipseCaseData = eclipseCase->eclipseCaseData();
    if ( !eclipseCaseData )
    {
        return std::unexpected( "Unable to load eclipse case data." );
    }

    auto keyValueStore = RiaApplication::instance()->keyValueStore();

    // Use RigGridExportAdapter to handle full grid export
    // Export the full grid (no min/max bounds, no refinement, no visibility override)
    RigGridExportAdapter gridAdapter( eclipseCaseData, cvf::Vec3st::ZERO, cvf::Vec3st::UNDEFINED, cvf::Vec3st( 1, 1, 1 ), nullptr );

    // Use the existing conversion utility to get the arrays in the correct format
    std::vector<float> coordArray;
    std::vector<float> zcornArray;
    std::vector<int>   actnumArray;

    RigResdataGridConverter::convertGridToCornerPointArrays( gridAdapter, coordArray, zcornArray, actnumArray );

    // Convert actnum from int to float for key-value store
    std::vector<float> actnumFloat;
    actnumFloat.reserve( actnumArray.size() );
    for ( int value : actnumArray )
    {
        actnumFloat.push_back( static_cast<float>( value ) );
    }

    // Store arrays in key-value store
    keyValueStore->set( m_zcornKey().toStdString(), RiaKeyValueStoreUtil::convertToByteVector( zcornArray ) );
    keyValueStore->set( m_coordKey().toStdString(), RiaKeyValueStoreUtil::convertToByteVector( coordArray ) );
    keyValueStore->set( m_actnumKey().toStdString(), RiaKeyValueStoreUtil::convertToByteVector( actnumFloat ) );

    // Store grid dimensions as well
    std::vector<float> dimensions = { static_cast<float>( gridAdapter.cellCountI() ),
                                      static_cast<float>( gridAdapter.cellCountJ() ),
                                      static_cast<float>( gridAdapter.cellCountK() ) };
    keyValueStore->set( ( m_zcornKey().toStdString() + "_dimensions" ), RiaKeyValueStoreUtil::convertToByteVector( dimensions ) );

    return nullptr;
}
