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
#include "RiaLogging.h"

#include "CompletionExportCommands/RicWellPathExportCompletionDataFeatureImpl.h"
#include "CompletionExportCommands/RicWellPathExportMswCompletionsImpl.h"
#include "ExportCommands/RicEclipseCellResultToFileImpl.h"
#include "ExportCommands/RicExportLgrFeature.h"
#include "RicCreateTemporaryLgrFeature.h"
#include "RicDeleteTemporaryLgrsFeature.h"

#include "RifInputPropertyLoader.h"

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseResultAddress.h"
#include "RigGridBase.h"
#include "RigGridExportAdapter.h"
#include "RigMainGrid.h"
#include "RigNoRefinement.h"
#include "RigResdataGridConverter.h"
#include "RigReservoirGridTools.h"
#include "RigResultAccessor.h"
#include "RigResultAccessorFactory.h"

#include "RimCase.h"
#include "RimCellFilter.h"
#include "RimEclipseCase.h"
#include "RimEclipseResultCase.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimRoffCase.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"

#include "RimcDataContainerString.h"

#include "cafCmdFeatureManager.h"
#include "cafPdmFieldScriptingCapability.h"

#include "cvfArray.h"

#include <QDir>
#include <QFileInfo>

#include <algorithm>
#include <expected>
#include <limits>
#include <vector>

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimEclipseCase, RimcEclipseCase_importProperties, "import_properties" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcEclipseCase_importProperties::RimcEclipseCase_importProperties( caf::PdmObjectHandle* self )
    : caf::PdmObjectMethod( self, PdmObjectMethod::NullPointerType::NULL_IS_INVALID, PdmObjectMethod::ResultType::PERSISTENT_FALSE )
{
    CAF_PDM_InitObject( "Import Properties", "", "", "Import Properties" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_fileNames, "FileNames", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcEclipseCase_importProperties::execute()
{
    std::vector<QString> absolutePaths;
    for ( auto path : m_fileNames() )
    {
        QFileInfo projectPathInfo( path );
        if ( !projectPathInfo.exists() )
        {
            QDir startDir( RiaApplication::instance()->startDir() );
            path = startDir.absoluteFilePath( path );
        }
        absolutePaths.push_back( path );
    }

    std::vector<QString> propertyNames;

    if ( auto eclipseCase = self<RimEclipseCase>() )
    {
        propertyNames = RifInputPropertyLoader::loadAndSynchronizeInputProperties( eclipseCase->inputPropertyCollection(),
                                                                                   eclipseCase->eclipseCaseData(),
                                                                                   absolutePaths,
                                                                                   false /* no faults */ );
    }

    auto dataObject            = new RimcDataContainerString();
    dataObject->m_stringValues = propertyNames;

    return dataObject;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimcEclipseCase_importProperties::classKeywordReturnedType() const
{
    return RimcDataContainerString::classKeywordStatic();
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

    // Num timesteps can be zero for roff cases.
    int numTimesteps = static_cast<int>( eclipseCase->timeStepDates().size() );
    if ( m_timeStep < 0 || ( m_timeStep >= numTimesteps && numTimesteps != 0 ) )
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
    const auto*          mainGrid = eclipseCaseData->mainGrid();
    cvf::Vec3st          fullSize( mainGrid->cellCountI(), mainGrid->cellCountJ(), mainGrid->cellCountK() );
    cvf::Vec3st          fullMax( mainGrid->cellCountI() - 1, mainGrid->cellCountJ() - 1, mainGrid->cellCountK() - 1 );
    RigGridExportAdapter gridAdapter( eclipseCaseData, cvf::Vec3st::ZERO, fullMax, RigNoRefinement( fullSize ), nullptr );

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

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimEclipseCase, RimcEclipseCase_addResultAlias, "add_result_alias" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcEclipseCase_addResultAlias::RimcEclipseCase_addResultAlias( caf::PdmObjectHandle* self )
    : caf::PdmVoidObjectMethod( self )
{
    CAF_PDM_InitObject( "Add Result Alias", "", "", "Add Result Alias" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_resultName, "ResultName", "" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_aliasName, "AliasName", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcEclipseCase_addResultAlias::execute()
{
    auto eclipseCase = self<RimEclipseCase>();
    if ( !eclipseCase )
    {
        return std::unexpected( "Unable to get Eclipse case." );
    }
    eclipseCase->addResultAlias( m_resultName(), m_aliasName() );
    return nullptr;
}

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimEclipseCase, RimcEclipseCase_clearResultAliases, "clear_result_aliases" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcEclipseCase_clearResultAliases::RimcEclipseCase_clearResultAliases( caf::PdmObjectHandle* self )
    : caf::PdmVoidObjectMethod( self )
{
    CAF_PDM_InitObject( "Clear Result Aliases", "", "", "Clear Result Aliases" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcEclipseCase_clearResultAliases::execute()
{
    auto eclipseCase = self<RimEclipseCase>();
    if ( !eclipseCase )
    {
        return std::unexpected( "Unable to get Eclipse case." );
    }
    eclipseCase->clearResultAliases();
    return nullptr;
}

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimEclipseCase, RimcEclipseCase_filteredCellsInternal, "filtered_cells_internal" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcEclipseCase_filteredCellsInternal::RimcEclipseCase_filteredCellsInternal( caf::PdmObjectHandle* self )
    : caf::PdmVoidObjectMethod( self )
{
    CAF_PDM_InitObject( "Filtered Cells Internal",
                        "",
                        "",
                        "Apply a cell filter to this case for a grid + time step and write a per-cell 0/1 mask "
                        "to the key-value store. The vector length and ordering match grid_property(...) for the "
                        "same grid index." );

    CAF_PDM_InitScriptableFieldNoDefault( &m_filter, "Filter", "", "", "", "Cell Filter" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_maskKey, "MaskKey", "" );
    CAF_PDM_InitScriptableField( &m_timeStep, "TimeStep", 0, "" );
    CAF_PDM_InitScriptableField( &m_gridIndex, "GridIndex", 0, "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcEclipseCase_filteredCellsInternal::execute()
{
    auto* eclipseCase = self<RimEclipseCase>();
    if ( !eclipseCase || !eclipseCase->eclipseCaseData() )
    {
        return std::unexpected( QString( "Self is not an Eclipse case with case data." ) );
    }
    if ( m_maskKey().isEmpty() )
    {
        return std::unexpected( QString( "Mask key is empty." ) );
    }

    RimCellFilter* filter = m_filter();
    if ( !filter )
    {
        return std::unexpected( QString( "Filter argument is null." ) );
    }

    RigEclipseCaseData* caseData = eclipseCase->eclipseCaseData();
    const size_t        gridIdx  = static_cast<size_t>( std::max( 0, m_gridIndex() ) );
    if ( gridIdx >= caseData->gridCount() )
    {
        return std::unexpected( QString( "Grid index out of range." ) );
    }

    const RigGridBase* grid = caseData->grid( gridIdx );
    const size_t       n    = grid->cellCount();

    cvf::UByteArray cellVis( n );
    cellVis.setAll( 1 );
    filter->applyToCellVisibility( &cellVis, grid, static_cast<size_t>( std::max( 0, m_timeStep() ) ) );

    std::vector<float> values;
    values.reserve( n );
    for ( size_t i = 0; i < n; ++i )
    {
        values.push_back( cellVis[i] ? 1.0f : 0.0f );
    }

    auto keyValueStore = RiaApplication::instance()->keyValueStore();
    keyValueStore->set( m_maskKey().toStdString(), RiaKeyValueStoreUtil::convertToByteVector( values ) );

    return nullptr;
}

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimEclipseCase, RimcEclipseCase_propertyDataType, "property_data_type" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcEclipseCase_propertyDataType::RimcEclipseCase_propertyDataType( caf::PdmObjectHandle* self )
    : caf::PdmEnumObjectMethod<RiaDefines::ResultDataType>( self )
{
    CAF_PDM_InitObject( "Property Data Type", "", "", "Get the data type (FLOAT or INTEGER) of a grid property" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_propertyType, "PropertyType", "" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_propertyName, "PropertyName", "" );
    CAF_PDM_InitScriptableField( &m_porosityModel,
                                 "PorosityModel",
                                 caf::AppEnum<RiaDefines::PorosityModelType>( RiaDefines::PorosityModelType::MATRIX_MODEL ),
                                 "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<RiaDefines::ResultDataType, QString> RimcEclipseCase_propertyDataType::executeEnum()
{
    auto eclipseCase = self<RimEclipseCase>();
    if ( !eclipseCase ) return std::unexpected( "No eclipse case" );

    auto resultsData = eclipseCase->results( m_porosityModel() );
    if ( !resultsData )
    {
        return std::unexpected( "Eclipse case has no result data." );
    }

    for ( const auto& address : resultsData->existingResults() )
    {
        if ( address.resultCatType() == m_propertyType() && address.resultName() == m_propertyName() )
        {
            return address.dataType();
        }
    }

    return std::unexpected( QString( "Property not found: %1" ).arg( m_propertyName() ) );
}

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimEclipseCase, RimEclipseCase_exportProperty, "exportProperty" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseCase_exportProperty::RimEclipseCase_exportProperty( caf::PdmObjectHandle* self )
    : caf::PdmVoidObjectMethod( self )
{
    CAF_PDM_InitObject( "Export Property", "", "", "Export a cell property of the case to a GRDECL style text file" );

    CAF_PDM_InitScriptableField( &m_timeStep, "TimeStep", -1, "Time Step", "", "", "Zero-based time step index. Ignored for static properties." );
    CAF_PDM_InitScriptableField( &m_propertyName, "PropertyName", QString(), "Property Name", "", "", "Name of the property to export" );
    CAF_PDM_InitScriptableField( &m_eclipseKeyword,
                                 "EclipseKeyword",
                                 QString(),
                                 "Eclipse Keyword",
                                 "",
                                 "",
                                 "Keyword written to the file header. Defaults to the property name." );
    CAF_PDM_InitScriptableField( &m_undefinedValue, "UndefinedValue", 0.0, "Undefined Value", "", "", "Value written for undefined cells" );
    CAF_PDM_InitScriptableField( &m_exportFile, "ExportFile", QString(), "Export File", "", "", "Full path of the file to write" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCase_exportProperty::setTimeStep( int timeStep )
{
    m_timeStep = timeStep;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCase_exportProperty::setPropertyName( const QString& propertyName )
{
    m_propertyName = propertyName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCase_exportProperty::setEclipseKeyword( const QString& eclipseKeyword )
{
    m_eclipseKeyword = eclipseKeyword;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCase_exportProperty::setUndefinedValue( double undefinedValue )
{
    m_undefinedValue = undefinedValue;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCase_exportProperty::setExportFile( const QString& exportFile )
{
    m_exportFile = exportFile;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimEclipseCase_exportProperty::execute()
{
    auto* eclipseCase = self<RimEclipseCase>();
    if ( !eclipseCase ) return std::unexpected( "No case is available." );

    if ( m_propertyName().isEmpty() ) return std::unexpected( "No property name specified." );
    if ( m_exportFile().isEmpty() ) return std::unexpected( "No export file specified." );

    if ( !eclipseCase->eclipseCaseData() && !eclipseCase->openReservoirCase() )
    {
        return std::unexpected( QString( "Could not open case '%1'" ).arg( eclipseCase->caseUserDescription() ) );
    }

    RigEclipseCaseData*     eclipseCaseData = eclipseCase->eclipseCaseData();
    RigCaseCellResultsData* cellResultsData = eclipseCaseData->results( RiaDefines::PorosityModelType::MATRIX_MODEL );

    if ( !cellResultsData->ensureKnownResultLoaded( RigEclipseResultAddress( m_propertyName() ) ) )
    {
        return std::unexpected( QString( "Could not find result property '%1'" ).arg( m_propertyName() ) );
    }

    QString eclipseKeyword = m_eclipseKeyword();
    if ( eclipseKeyword.isEmpty() ) eclipseKeyword = m_propertyName();

    const bool writeEchoKeywords = false;
    QString    errorMessage;
    if ( !RicEclipseCellResultToFileImpl::writePropertyToTextFile( m_exportFile(),
                                                                   eclipseCaseData,
                                                                   m_timeStep(),
                                                                   m_propertyName(),
                                                                   eclipseKeyword,
                                                                   m_undefinedValue(),
                                                                   writeEchoKeywords,
                                                                   &errorMessage ) )
    {
        return std::unexpected( errorMessage );
    }

    return nullptr;
}

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimEclipseCase, RimEclipseCase_exportCompletions, "exportCompletions" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseCase_exportCompletions::RimEclipseCase_exportCompletions( caf::PdmObjectHandle* self )
    : caf::PdmVoidObjectMethod( self )
{
    CAF_PDM_InitObject( "Export Completions", "", "", "Export completion data (COMPDAT, WELSPECS, MSW keywords etc.) for well paths in this case" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_wellPaths,
                                          "WellPaths",
                                          "Well Paths",
                                          "",
                                          "",
                                          "Well paths to export. Empty list exports all visible well paths." );
    CAF_PDM_InitScriptableField( &m_timeStep, "TimeStep", 0, "Time Step", "", "", "Zero-based time step index" );
    CAF_PDM_InitScriptableField( &m_exportFolder, "ExportFolder", QString(), "Export Folder", "", "", "Folder to write the export files to" );
    CAF_PDM_InitScriptableField( &m_customFileName,
                                 "CustomFileName",
                                 QString(),
                                 "Custom File Name",
                                 "",
                                 "",
                                 "Optional file name (without folder) used when FileSplit is UNIFIED_FILE" );

    CAF_PDM_InitScriptableField( &m_fileSplit,
                                 "FileSplit",
                                 RicExportCompletionDataSettingsUi::ExportSplitType(),
                                 "File Split",
                                 "",
                                 "",
                                 "Controls how export data is split into files" );
    CAF_PDM_InitScriptableField( &m_compdatExport,
                                 "CompdatExport",
                                 RicExportCompletionDataSettingsUi::CompdatExportType(),
                                 "Compdat Export",
                                 "",
                                 "",
                                 "Compdat export type" );

    CAF_PDM_InitScriptableField( &m_includeMsw, "IncludeMsw", true, "Include MSW", "", "", "Export Multi Segment Well model" );
    CAF_PDM_InitScriptableField( &m_useNtgHorizontally, "UseNtgHorizontally", false, "Use NTG Horizontally" );
    CAF_PDM_InitScriptableField( &m_includePerforations, "IncludePerforations", true, "Include Perforations" );
    CAF_PDM_InitScriptableField( &m_includeFishbones, "IncludeFishbones", true, "Include Fishbones" );
    CAF_PDM_InitScriptableField( &m_includeFractures, "IncludeFractures", true, "Include Fractures" );
    CAF_PDM_InitScriptableField( &m_excludeMainBoreForFishbones, "ExcludeMainBoreForFishbones", false, "Exclude Main Bore for Fishbones" );

    CAF_PDM_InitScriptableField( &m_performTransScaling, "PerformTransScaling", false, "Perform Transmissibility Scaling" );
    CAF_PDM_InitScriptableField( &m_transScalingTimeStep, "TransScalingTimeStep", 0, "Transmissibility Scaling Pressure Time Step" );
    CAF_PDM_InitScriptableField( &m_transScalingWbhpSource,
                                 "TransScalingWbhpSource",
                                 RicExportCompletionDataSettingsUi::TransScalingWBHPSource(),
                                 "Transmissibility Scaling WBHP Source" );
    CAF_PDM_InitScriptableField( &m_transScalingWbhp, "TransScalingWbhp", 200.0, "Transmissibility Scaling Constant WBHP Value" );

    CAF_PDM_InitScriptableField( &m_exportComments, "ExportComments", true, "Export Comments", "", "", "Export data source as comments" );
    CAF_PDM_InitScriptableField( &m_exportWelspec, "ExportWelspec", true, "Export WELSPEC", "", "", "Export WELSPEC keyword" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCase_exportCompletions::setWellPaths( const std::vector<RimWellPath*>& wellPaths )
{
    m_wellPaths.setValue( wellPaths );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCase_exportCompletions::setTimeStep( int timeStep )
{
    m_timeStep = timeStep;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCase_exportCompletions::setExportFolder( const QString& exportFolder )
{
    m_exportFolder = exportFolder;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCase_exportCompletions::setCustomFileName( const QString& customFileName )
{
    m_customFileName = customFileName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCase_exportCompletions::setFileSplit( RicExportCompletionDataSettingsUi::ExportSplit fileSplit )
{
    m_fileSplit = fileSplit;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCase_exportCompletions::setCompdatExport( RicExportCompletionDataSettingsUi::CompdatExport compdatExport )
{
    m_compdatExport = compdatExport;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCase_exportCompletions::setIncludeMsw( bool enable )
{
    m_includeMsw = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCase_exportCompletions::setUseNtgHorizontally( bool enable )
{
    m_useNtgHorizontally = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCase_exportCompletions::setIncludePerforations( bool enable )
{
    m_includePerforations = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCase_exportCompletions::setIncludeFishbones( bool enable )
{
    m_includeFishbones = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCase_exportCompletions::setIncludeFractures( bool enable )
{
    m_includeFractures = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCase_exportCompletions::setExcludeMainBoreForFishbones( bool enable )
{
    m_excludeMainBoreForFishbones = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCase_exportCompletions::setPerformTransScaling( bool enable )
{
    m_performTransScaling = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCase_exportCompletions::setTransScalingTimeStep( int timeStep )
{
    m_transScalingTimeStep = timeStep;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCase_exportCompletions::setTransScalingWbhpSource( RicExportFractureCompletionsImpl::PressureDepletionWBHPSource source )
{
    m_transScalingWbhpSource = source;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCase_exportCompletions::setTransScalingWbhp( double wbhp )
{
    m_transScalingWbhp = wbhp;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCase_exportCompletions::setExportComments( bool enable )
{
    m_exportComments = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCase_exportCompletions::setExportWelspec( bool enable )
{
    m_exportWelspec = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimEclipseCase_exportCompletions::execute()
{
    auto* eclipseCase = self<RimEclipseCase>();
    if ( !eclipseCase ) return std::unexpected( "No case is available." );

    if ( m_exportFolder().isEmpty() ) return std::unexpected( "No export folder specified." );

    QDir folder( m_exportFolder() );
    if ( !folder.exists() ) return std::unexpected( QString( "The export folder '%1' does not exist." ).arg( m_exportFolder() ) );

    eclipseCase->ensureReservoirCaseIsOpen();
    if ( !eclipseCase->eclipseCaseData() )
    {
        return std::unexpected( QString( "No data available for case '%1'" ).arg( eclipseCase->caseUserDescription() ) );
    }

    std::vector<RimWellPath*> wellPaths = m_wellPaths.ptrReferencedObjectsByType();
    if ( wellPaths.empty() )
    {
        RimProject* project = RimProject::current();
        for ( RimWellPath* wellPath : project->activeOilField()->wellPathCollection->allWellPaths() )
        {
            if ( wellPath->showWellPath() ) wellPaths.push_back( wellPath );
        }
    }

    if ( wellPaths.empty() ) return std::unexpected( "No well paths to export." );

    RicExportCompletionDataSettingsUi exportSettings;
    exportSettings.caseToApply = eclipseCase;
    exportSettings.folder      = m_exportFolder();
    exportSettings.timeStep    = std::max( 0, m_timeStep() );

    exportSettings.fileSplit     = m_fileSplit();
    exportSettings.compdatExport = m_compdatExport();

    exportSettings.performTransScaling    = m_performTransScaling();
    exportSettings.transScalingTimeStep   = m_transScalingTimeStep();
    exportSettings.transScalingWBHPSource = m_transScalingWbhpSource();
    exportSettings.transScalingWBHP       = m_transScalingWbhp();

    exportSettings.includeMsw                  = m_includeMsw();
    exportSettings.useLateralNTG               = m_useNtgHorizontally();
    exportSettings.includePerforations         = m_includePerforations();
    exportSettings.includeFishbones            = m_includeFishbones();
    exportSettings.excludeMainBoreForFishbones = m_excludeMainBoreForFishbones();
    exportSettings.includeFractures            = m_includeFractures();

    exportSettings.setExportDataSourceAsComment( m_exportComments() );
    exportSettings.setExportWelspec( m_exportWelspec() );

    if ( !m_customFileName().isEmpty() ) exportSettings.setCustomFileName( m_customFileName() );

    RicWellPathExportCompletionDataFeatureImpl::exportCompletions( wellPaths, exportSettings );

    return nullptr;
}

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimEclipseCase, RimEclipseCase_exportMswCompletions, "exportMswCompletions" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseCase_exportMswCompletions::RimEclipseCase_exportMswCompletions( caf::PdmObjectHandle* self )
    : caf::PdmVoidObjectMethod( self )
{
    CAF_PDM_InitObject( "Export MSW Completions", "", "", "Export the Multi Segment Well model keywords for well paths in this case" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_wellPaths, "WellPaths", "Well Paths", "", "", "Well paths to export" );
    CAF_PDM_InitScriptableField( &m_exportFolder, "ExportFolder", QString(), "Export Folder", "", "", "Folder to write the export files to" );
    CAF_PDM_InitScriptableField( &m_fileSplit,
                                 "FileSplit",
                                 RicExportCompletionDataSettingsUi::ExportSplitType(),
                                 "File Split",
                                 "",
                                 "",
                                 "Controls how export data is split into files" );
    CAF_PDM_InitScriptableField( &m_includePerforations, "IncludePerforations", true, "Include Perforations" );
    CAF_PDM_InitScriptableField( &m_includeFishbones, "IncludeFishbones", true, "Include Fishbones" );
    CAF_PDM_InitScriptableField( &m_includeFractures, "IncludeFractures", true, "Include Fractures" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCase_exportMswCompletions::setWellPaths( const std::vector<RimWellPath*>& wellPaths )
{
    m_wellPaths.setValue( wellPaths );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCase_exportMswCompletions::setExportFolder( const QString& exportFolder )
{
    m_exportFolder = exportFolder;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCase_exportMswCompletions::setFileSplit( RicExportCompletionDataSettingsUi::ExportSplit fileSplit )
{
    m_fileSplit = fileSplit;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCase_exportMswCompletions::setIncludePerforations( bool enable )
{
    m_includePerforations = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCase_exportMswCompletions::setIncludeFishbones( bool enable )
{
    m_includeFishbones = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCase_exportMswCompletions::setIncludeFractures( bool enable )
{
    m_includeFractures = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimEclipseCase_exportMswCompletions::execute()
{
    auto* eclipseCase = self<RimEclipseCase>();
    if ( !eclipseCase ) return std::unexpected( "No case is available." );

    if ( m_exportFolder().isEmpty() ) return std::unexpected( "No export folder specified." );

    QDir folder( m_exportFolder() );
    if ( !folder.exists() ) return std::unexpected( QString( "The export folder '%1' does not exist." ).arg( m_exportFolder() ) );

    std::vector<RimWellPath*> wellPaths = m_wellPaths.ptrReferencedObjectsByType();
    if ( wellPaths.empty() ) return std::unexpected( "No well paths specified." );

    eclipseCase->ensureReservoirCaseIsOpen();
    if ( !eclipseCase->eclipseCaseData() )
    {
        return std::unexpected( QString( "No data available for case '%1'" ).arg( eclipseCase->caseUserDescription() ) );
    }

    RicExportCompletionDataSettingsUi exportSettings;
    exportSettings.caseToApply         = eclipseCase;
    exportSettings.folder              = m_exportFolder();
    exportSettings.fileSplit           = m_fileSplit();
    exportSettings.includePerforations = m_includePerforations();
    exportSettings.includeFishbones    = m_includeFishbones();
    exportSettings.includeFractures    = m_includeFractures();

    RicWellPathExportMswCompletionsImpl::exportWellSegmentsForAllCompletions( exportSettings, wellPaths );

    return nullptr;
}

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimEclipseCase, RimEclipseCase_createLgrForCompletions, "createLgrForCompletions" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseCase_createLgrForCompletions::RimEclipseCase_createLgrForCompletions( caf::PdmObjectHandle* self )
    : caf::PdmVoidObjectMethod( self )
{
    CAF_PDM_InitObject( "Create LGR for Completions",
                        "",
                        "",
                        "Create temporary local grid refinements around the completions of the given well paths" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_wellPaths, "WellPaths", "Well Paths", "", "", "Well paths to create LGRs for" );
    CAF_PDM_InitScriptableField( &m_timeStep, "TimeStep", 0, "Time Step", "", "", "Zero-based time step index" );
    CAF_PDM_InitScriptableField( &m_refinementI, "RefinementI", 1, "Refinement I", "", "", "Number of refined cells in I direction" );
    CAF_PDM_InitScriptableField( &m_refinementJ, "RefinementJ", 1, "Refinement J", "", "", "Number of refined cells in J direction" );
    CAF_PDM_InitScriptableField( &m_refinementK, "RefinementK", 1, "Refinement K", "", "", "Number of refined cells in K direction" );
    CAF_PDM_InitScriptableField( &m_splitType, "SplitType", Lgr::SplitTypeEnum(), "Split Type", "", "", "Defines how to split the LGRs" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCase_createLgrForCompletions::setWellPaths( const std::vector<RimWellPath*>& wellPaths )
{
    m_wellPaths.setValue( wellPaths );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCase_createLgrForCompletions::setTimeStep( int timeStep )
{
    m_timeStep = timeStep;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCase_createLgrForCompletions::setRefinement( int refinementI, int refinementJ, int refinementK )
{
    m_refinementI = refinementI;
    m_refinementJ = refinementJ;
    m_refinementK = refinementK;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCase_createLgrForCompletions::setSplitType( Lgr::SplitType splitType )
{
    m_splitType = splitType;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RimEclipseCase_createLgrForCompletions::wellsIntersectingOtherLgrs() const
{
    return m_wellsIntersectingOtherLgrs;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimEclipseCase_createLgrForCompletions::execute()
{
    auto* eclipseCase = self<RimEclipseCase>();
    if ( !eclipseCase ) return std::unexpected( "No case is available." );

    std::vector<RimWellPath*> wellPaths = m_wellPaths.ptrReferencedObjectsByType();
    if ( wellPaths.empty() ) return std::unexpected( "No well paths specified." );

    if ( m_refinementI() < 1 || m_refinementJ() < 1 || m_refinementK() < 1 )
    {
        return std::unexpected( "Refinement must be at least 1 in all directions." );
    }

    auto* feature = dynamic_cast<RicCreateTemporaryLgrFeature*>(
        caf::CmdFeatureManager::instance()->getCommandFeature( "RicCreateTemporaryLgrFeature" ) );
    if ( !feature ) return std::unexpected( "The create LGR feature is not available." );

    RicDeleteTemporaryLgrsFeature::deleteAllTemporaryLgrs( eclipseCase );

    cvf::Vec3st lgrCellCounts( m_refinementI(), m_refinementJ(), m_refinementK() );
    m_wellsIntersectingOtherLgrs.clear();

    feature->createLgrsForWellPaths( wellPaths,
                                     eclipseCase,
                                     m_timeStep(),
                                     lgrCellCounts,
                                     m_splitType(),
                                     { RigCompletionData::CompletionType::PERFORATION,
                                       RigCompletionData::CompletionType::FRACTURE,
                                       RigCompletionData::CompletionType::FISHBONES },
                                     &m_wellsIntersectingOtherLgrs );

    RigReservoirGridTools::refreshEclipseCaseDataAndViews( eclipseCase );

    if ( !m_wellsIntersectingOtherLgrs.empty() )
    {
        RiaLogging::warning( QString( "No LGRs created for some wells due to existing intersecting LGR(s). Affected wells: %1" )
                                 .arg( m_wellsIntersectingOtherLgrs.join( ", " ) )
                                 .toStdString() );
    }

    return nullptr;
}

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimEclipseCase, RimEclipseCase_exportLgrForCompletions, "exportLgrForCompletions" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseCase_exportLgrForCompletions::RimEclipseCase_exportLgrForCompletions( caf::PdmObjectHandle* self )
    : caf::PdmVoidObjectMethod( self )
{
    CAF_PDM_InitObject( "Export LGR for Completions",
                        "",
                        "",
                        "Export local grid refinements around the completions of the given well paths to CARFIN files" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_wellPaths, "WellPaths", "Well Paths", "", "", "Well paths to export LGRs for" );
    CAF_PDM_InitScriptableField( &m_timeStep, "TimeStep", 0, "Time Step", "", "", "Zero-based time step index" );
    CAF_PDM_InitScriptableField( &m_exportFolder, "ExportFolder", QString(), "Export Folder", "", "", "Folder to write the export files to" );
    CAF_PDM_InitScriptableField( &m_refinementI, "RefinementI", 1, "Refinement I", "", "", "Number of refined cells in I direction" );
    CAF_PDM_InitScriptableField( &m_refinementJ, "RefinementJ", 1, "Refinement J", "", "", "Number of refined cells in J direction" );
    CAF_PDM_InitScriptableField( &m_refinementK, "RefinementK", 1, "Refinement K", "", "", "Number of refined cells in K direction" );
    CAF_PDM_InitScriptableField( &m_splitType, "SplitType", Lgr::SplitTypeEnum(), "Split Type", "", "", "Defines how to split the LGRs" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCase_exportLgrForCompletions::setWellPaths( const std::vector<RimWellPath*>& wellPaths )
{
    m_wellPaths.setValue( wellPaths );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCase_exportLgrForCompletions::setTimeStep( int timeStep )
{
    m_timeStep = timeStep;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCase_exportLgrForCompletions::setExportFolder( const QString& exportFolder )
{
    m_exportFolder = exportFolder;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCase_exportLgrForCompletions::setRefinement( int refinementI, int refinementJ, int refinementK )
{
    m_refinementI = refinementI;
    m_refinementJ = refinementJ;
    m_refinementK = refinementK;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCase_exportLgrForCompletions::setSplitType( Lgr::SplitType splitType )
{
    m_splitType = splitType;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RimEclipseCase_exportLgrForCompletions::wellsIntersectingOtherLgrs() const
{
    return m_wellsIntersectingOtherLgrs;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimEclipseCase_exportLgrForCompletions::execute()
{
    auto* eclipseCase = self<RimEclipseCase>();
    if ( !eclipseCase ) return std::unexpected( "No case is available." );

    std::vector<RimWellPath*> wellPaths = m_wellPaths.ptrReferencedObjectsByType();
    if ( wellPaths.empty() ) return std::unexpected( "No well paths specified." );

    if ( m_exportFolder().isEmpty() ) return std::unexpected( "No export folder specified." );

    if ( m_refinementI() < 1 || m_refinementJ() < 1 || m_refinementK() < 1 )
    {
        return std::unexpected( "Refinement must be at least 1 in all directions." );
    }

    cvf::Vec3st lgrCellCounts( m_refinementI(), m_refinementJ(), m_refinementK() );
    m_wellsIntersectingOtherLgrs.clear();

    RicExportLgrFeature::exportLgrsForWellPaths( m_exportFolder(),
                                                 wellPaths,
                                                 eclipseCase,
                                                 m_timeStep(),
                                                 lgrCellCounts,
                                                 m_splitType(),
                                                 { RigCompletionData::CompletionType::PERFORATION,
                                                   RigCompletionData::CompletionType::FRACTURE,
                                                   RigCompletionData::CompletionType::FISHBONES },
                                                 &m_wellsIntersectingOtherLgrs );

    if ( !m_wellsIntersectingOtherLgrs.empty() )
    {
        RiaLogging::warning( QString( "No export for some wells due to existing intersecting LGR(s). Affected wells: %1" )
                                 .arg( m_wellsIntersectingOtherLgrs.join( ", " ) )
                                 .toStdString() );
    }

    return nullptr;
}
