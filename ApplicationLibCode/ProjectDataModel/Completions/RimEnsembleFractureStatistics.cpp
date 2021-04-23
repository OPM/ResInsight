/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021 -     Equinor ASA
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

#include "RimEnsembleFractureStatistics.h"

#include "RiaDefines.h"
#include "RiaInterpolationTools.h"
#include "RiaLogging.h"
#include "RiaPreferences.h"
#include "RiaWeightedGeometricMeanCalculator.h"
#include "RiaWeightedHarmonicMeanCalculator.h"

#include "RigFractureGrid.h"
#include "RigSlice2D.h"
#include "RigStatisticsMath.h"
#include "RigStimPlanFractureDefinition.h"

#include "RifCsvDataTableFormatter.h"
#include "RifEnsembleFractureStatisticsExporter.h"
#include "RifStimPlanXmlReader.h"

#include "FractureCommands/RicNewStimPlanFractureTemplateFeature.h"

#include "cafAppEnum.h"
#include "cafPdmUiTextEditor.h"
#include "cafPdmUiToolButtonEditor.h"
#include "cafPdmUiTreeSelectionEditor.h"

#include <cmath>

#include <QDir>
#include <QFile>

namespace caf
{
template <>
void caf::AppEnum<RimEnsembleFractureStatistics::StatisticsType>::setUp()
{
    addItem( RimEnsembleFractureStatistics::StatisticsType::MEAN, "MEAN", "Mean" );
    addItem( RimEnsembleFractureStatistics::StatisticsType::MIN, "MIN", "Minimum" );
    addItem( RimEnsembleFractureStatistics::StatisticsType::MAX, "MAX", "Maximum" );
    addItem( RimEnsembleFractureStatistics::StatisticsType::P10, "P10", "P10" );
    addItem( RimEnsembleFractureStatistics::StatisticsType::P50, "P50", "P50" );
    addItem( RimEnsembleFractureStatistics::StatisticsType::P90, "P90", "P90" );
    addItem( RimEnsembleFractureStatistics::StatisticsType::OCCURRENCE, "OCCURRENCE", "Occurrence" );
    setDefault( RimEnsembleFractureStatistics::StatisticsType::MEAN );
}

template <>
void caf::AppEnum<RimEnsembleFractureStatistics::MeshType>::setUp()
{
    addItem( RimEnsembleFractureStatistics::MeshType::ADAPTIVE, "ADAPTIVE", "Adaptive" );
    addItem( RimEnsembleFractureStatistics::MeshType::UNIFORM, "UNIFORM", "Uniform" );
    addItem( RimEnsembleFractureStatistics::MeshType::NAIVE, "NAIVE", "Naive" );
    setDefault( RimEnsembleFractureStatistics::MeshType::ADAPTIVE );
}

template <>
void caf::AppEnum<RimEnsembleFractureStatistics::MeshAlignmentType>::setUp()
{
    addItem( RimEnsembleFractureStatistics::MeshAlignmentType::PERFORATION_DEPTH, "PERFORATION_DEPTH", "Perforation Depth" );
    addItem( RimEnsembleFractureStatistics::MeshAlignmentType::MESH_DEPTH, "MESH_DEPTH", "Mesh Depth" );
    setDefault( RimEnsembleFractureStatistics::MeshAlignmentType::PERFORATION_DEPTH );
}

template <>
void caf::AppEnum<RimEnsembleFractureStatistics::MeanType>::setUp()
{
    addItem( RimEnsembleFractureStatistics::MeanType::HARMONIC, "HARMONIC", "Harmonic" );
    addItem( RimEnsembleFractureStatistics::MeanType::ARITHMETIC, "ARITHMETIC", "Artihmetic" );
    addItem( RimEnsembleFractureStatistics::MeanType::GEOMETRIC, "GEOMETRIC", "Geometric" );
    addItem( RimEnsembleFractureStatistics::MeanType::MINIMUM, "MINIMUM", "Minimum" );
    setDefault( RimEnsembleFractureStatistics::MeanType::HARMONIC );
}

template <>
void caf::AppEnum<RimEnsembleFractureStatistics::AdaptiveNumLayersType>::setUp()
{
    addItem( RimEnsembleFractureStatistics::AdaptiveNumLayersType::AVERAGE, "AVERAGE", "Average" );
    addItem( RimEnsembleFractureStatistics::AdaptiveNumLayersType::MINIMUM, "MINIMUM", "Minimum" );
    addItem( RimEnsembleFractureStatistics::AdaptiveNumLayersType::MAXIMUM, "MAXIMUM", "Maximum" );
    addItem( RimEnsembleFractureStatistics::AdaptiveNumLayersType::USER_DEFINED, "USER_DEFINED", "User-Defined" );
    setDefault( RimEnsembleFractureStatistics::AdaptiveNumLayersType::AVERAGE );
}

} // namespace caf

CAF_PDM_SOURCE_INIT( RimEnsembleFractureStatistics, "EnsembleFractureStatistics" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEnsembleFractureStatistics::RimEnsembleFractureStatistics()
{
    CAF_PDM_InitObject( "Ensemble Fracture Statistics", ":/FractureTemplate16x16.png", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_filePaths, "FilePaths", "", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_filePathsTable, "FilePathsTable", "File Paths Table", "", "", "" );
    m_filePathsTable.uiCapability()->setUiEditorTypeName( caf::PdmUiTextEditor::uiEditorTypeName() );
    m_filePathsTable.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );
    m_filePathsTable.uiCapability()->setUiReadOnly( true );
    m_filePathsTable.xmlCapability()->disableIO();

    CAF_PDM_InitFieldNoDefault( &m_meshAlignmentType, "MeshAlignmentType", "Mesh Alignment", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_meshType, "MeshType", "Mesh Type", "", "", "" );

    // Uniform sampling
    CAF_PDM_InitField( &m_numSamplesX, "NumberOfSamplesX", 100, "X", "", "", "" );
    CAF_PDM_InitField( &m_numSamplesY, "NumberOfSamplesY", 200, "Y", "", "", "" );

    // Adaptive sampling
    CAF_PDM_InitFieldNoDefault( &m_adaptiveMeanType, "AdaptiveMeanType", "Mean Type", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_adaptiveNumLayersType, "AdaptiveNumLayersType", "Number of Layers", "", "", "" );
    CAF_PDM_InitField( &m_adaptiveNumLayers, "AdaptiveNumLayers", 30, "Number of Layers Y", "", "", "" );

    std::vector<caf::AppEnum<RimEnsembleFractureStatistics::StatisticsType>> defaultStatisticsTypes = {
        caf::AppEnum<RimEnsembleFractureStatistics::StatisticsType>( RimEnsembleFractureStatistics::StatisticsType::MEAN ) };

    CAF_PDM_InitField( &m_selectedStatisticsType, "SelectedStatisticsType", defaultStatisticsTypes, "Statistics Type", "", "", "" );
    m_selectedStatisticsType.uiCapability()->setUiEditorTypeName( caf::PdmUiTreeSelectionEditor::uiEditorTypeName() );
    m_selectedStatisticsType.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::TOP );

    CAF_PDM_InitFieldNoDefault( &m_computeStatistics, "ComputeStatistics", "Compute Templates", "", "", "" );
    m_computeStatistics.uiCapability()->setUiEditorTypeName( caf::PdmUiToolButtonEditor::uiEditorTypeName() );
    m_computeStatistics.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );

    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEnsembleFractureStatistics::~RimEnsembleFractureStatistics()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleFractureStatistics::addFilePath( const QString& filePath )
{
    m_filePaths.v().push_back( filePath );
    m_filePathsTable = generateFilePathsTable();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimEnsembleFractureStatistics::generateFilePathsTable()
{
    QString body;
    for ( auto prop : m_filePaths.v() )
    {
        body.append( prop.path() + "<br>" );
    }

    return body;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo>
    RimEnsembleFractureStatistics::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                          bool*                      useOptionsOnly )
{
    QList<caf::PdmOptionItemInfo> options;
    if ( fieldNeedingOptions == &m_selectedStatisticsType )
    {
        for ( size_t i = 0; i < caf::AppEnum<RimEnsembleFractureStatistics::StatisticsType>::size(); ++i )
        {
            caf::AppEnum<RimEnsembleFractureStatistics::StatisticsType> t =
                caf::AppEnum<RimEnsembleFractureStatistics::StatisticsType>::fromIndex( i );
            t.uiText();

            options.push_back( caf::PdmOptionItemInfo( t.uiText(), t.value() ) );
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleFractureStatistics::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                           QString                    uiConfigName,
                                                           caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_filePathsTable )
    {
        auto myAttr = dynamic_cast<caf::PdmUiTextEditorAttribute*>( attribute );
        if ( myAttr )
        {
            myAttr->wrapMode = caf::PdmUiTextEditorAttribute::NoWrap;
            myAttr->textMode = caf::PdmUiTextEditorAttribute::HTML;
        }
    }
    else if ( field == &m_selectedStatisticsType )
    {
        caf::PdmUiTreeSelectionEditorAttribute* attrib = dynamic_cast<caf::PdmUiTreeSelectionEditorAttribute*>( attribute );
        if ( attrib )
        {
            attrib->showTextFilter        = false;
            attrib->showToggleAllCheckbox = false;
            attrib->singleSelectionMode   = false;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleFractureStatistics::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                                      const QVariant&            oldValue,
                                                      const QVariant&            newValue )
{
    if ( changedField == &m_computeStatistics )
    {
        m_computeStatistics            = false;
        std::vector<QString> filePaths = computeStatistics();
        RicNewStimPlanFractureTemplateFeature::createNewTemplatesFromFiles( filePaths );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleFractureStatistics::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( nameField() );
    uiOrdering.add( &m_filePathsTable );
    uiOrdering.add( &m_meshAlignmentType );
    uiOrdering.add( &m_meshType );
    uiOrdering.add( &m_numSamplesX );
    uiOrdering.add( &m_numSamplesY );
    bool isUniformMesh = m_meshType() == MeshType::UNIFORM;
    m_numSamplesX.uiCapability()->setUiHidden( !isUniformMesh );
    m_numSamplesY.uiCapability()->setUiHidden( !isUniformMesh );

    uiOrdering.add( &m_adaptiveMeanType );
    uiOrdering.add( &m_adaptiveNumLayersType );
    uiOrdering.add( &m_adaptiveNumLayers );

    bool isAdaptiveMesh = m_meshType() == MeshType::ADAPTIVE;
    m_adaptiveMeanType.uiCapability()->setUiHidden( !isAdaptiveMesh );
    m_adaptiveNumLayersType.uiCapability()->setUiHidden( !isAdaptiveMesh );

    bool adaptiveSamplesUserDefined = m_adaptiveNumLayersType() == AdaptiveNumLayersType::USER_DEFINED;
    m_adaptiveNumLayers.uiCapability()->setUiHidden( !isAdaptiveMesh || !adaptiveSamplesUserDefined );

    uiOrdering.add( &m_selectedStatisticsType );
    uiOrdering.add( &m_computeStatistics );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleFractureStatistics::loadAndUpdateData()
{
    m_filePathsTable = generateFilePathsTable();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> RimEnsembleFractureStatistics::computeStatistics()
{
    auto unitSystem = RiaDefines::EclipseUnitSystem::UNITS_METRIC;

    std::vector<cvf::ref<RigStimPlanFractureDefinition>> stimPlanFractureDefinitions =
        readFractureDefinitions( m_filePaths.v(), unitSystem );

    std::set<std::pair<QString, QString>> availableResults = findAllResultNames( stimPlanFractureDefinitions );

    std::map<std::pair<RimEnsembleFractureStatistics::StatisticsType, QString>, std::shared_ptr<RigSlice2D>> statisticsGridsAll;

    auto selectedStatistics = m_selectedStatisticsType.value();

    // TODO: take from an incoming xml?
    double timeStep = 1.0;

    double referenceDepth = 0.0;
    if ( m_meshAlignmentType() == MeshAlignmentType::PERFORATION_DEPTH )
    {
        for ( auto definition : stimPlanFractureDefinitions )
        {
            referenceDepth += computeDepthOfWellPathAtFracture( definition );
        }
        referenceDepth /= stimPlanFractureDefinitions.size();
    }

    std::vector<double> gridXs;
    std::vector<double> gridYs;
    auto [minX, maxX, minY, maxY] = findSamplingIntervals( stimPlanFractureDefinitions, gridXs, gridYs );
    RiaLogging::info(
        QString( "Ensemble Fracture Size: X = [%1, %2] Y = [%3, %4]" ).arg( minX ).arg( maxX ).arg( minY ).arg( maxY ) );

    for ( auto result : availableResults )
    {
        RiaLogging::info( QString( "Creating statistics for result: %1" ).arg( result.first ) );

        std::vector<cvf::cref<RigFractureGrid>> fractureGrids =
            createFractureGrids( stimPlanFractureDefinitions, unitSystem, result.first, m_meshAlignmentType() );

        std::vector<std::vector<double>> samples( gridXs.size() * gridYs.size() );
        sampleAllGrids( fractureGrids, gridXs, gridYs, samples );

        std::map<RimEnsembleFractureStatistics::StatisticsType, std::shared_ptr<RigSlice2D>> statisticsGrids;
        generateStatisticsGrids( samples, gridXs.size(), gridYs.size(), fractureGrids.size(), statisticsGrids, selectedStatistics );

        for ( auto [statType, slice] : statisticsGrids )
        {
            auto key                = std::make_pair( statType, result.first );
            statisticsGridsAll[key] = slice;
        }
    }

    std::vector<QString> xmlFilePaths;

    // Save images in snapshot catalog relative to project directory
    RiaApplication* app                 = RiaApplication::instance();
    QString         outputDirectoryPath = app->createAbsolutePathFromProjectRelativePath( "fracturestats" );
    QDir            outputDirectory( outputDirectoryPath );
    if ( !outputDirectory.exists() )
    {
        outputDirectory.mkpath( outputDirectoryPath );
    }

    for ( auto t : selectedStatistics )
    {
        QString text = t.text();

        // Get the all the properties for this statistics type
        std::vector<std::shared_ptr<RigSlice2D>> statisticsSlices;
        std::vector<std::pair<QString, QString>> properties;
        for ( auto result : availableResults )
        {
            properties.push_back( result );
            std::shared_ptr<RigSlice2D> slice = statisticsGridsAll[std::make_pair( t.value(), result.first )];
            statisticsSlices.push_back( slice );

            QString csvFilePath = outputDirectoryPath + "/" + text + "-" + result.first + ".csv";
            writeStatisticsToCsv( csvFilePath, *slice );
        }

        QString xmlFilePath = outputDirectoryPath + "/" + name() + "-" + text + ".xml";

        // TODO: add offset for grid ys
        std::vector<double> gridYsWithOffset;
        for ( double depth : gridYs )
            gridYsWithOffset.push_back( referenceDepth - depth );

        RiaLogging::info( QString( "Writing fracture group statistics to: %1" ).arg( xmlFilePath ) );
        RifEnsembleFractureStatisticsExporter::writeAsStimPlanXml( statisticsSlices,
                                                                   properties,
                                                                   xmlFilePath,
                                                                   gridXs,
                                                                   gridYsWithOffset,
                                                                   timeStep,
                                                                   unitSystem );

        xmlFilePaths.push_back( xmlFilePath );
    }

    return xmlFilePaths;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<cvf::ref<RigStimPlanFractureDefinition>>
    RimEnsembleFractureStatistics::readFractureDefinitions( const std::vector<caf::FilePath>& filePaths,
                                                            RiaDefines::EclipseUnitSystem     unitSystem ) const
{
    double conductivityScaleFactor = 1.0;

    std::vector<cvf::ref<RigStimPlanFractureDefinition>> results;
    for ( auto filePath : m_filePaths.v() )
    {
        RiaLogging::info( QString( "Loading file: %1" ).arg( filePath.path() ) );
        QString                                 errorMessage;
        cvf::ref<RigStimPlanFractureDefinition> stimPlanFractureDefinitionData =
            RifStimPlanXmlReader::readStimPlanXMLFile( filePath.path(),
                                                       conductivityScaleFactor,
                                                       RifStimPlanXmlReader::MirrorMode::MIRROR_AUTO,
                                                       unitSystem,
                                                       &errorMessage );
        if ( !errorMessage.isEmpty() )
        {
            RiaLogging::error( QString( "Error when reading file: '%1'" ).arg( errorMessage ) );
        }

        if ( stimPlanFractureDefinitionData.notNull() )
        {
            results.push_back( stimPlanFractureDefinitionData );
        }
    }

    return results;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<std::pair<QString, QString>> RimEnsembleFractureStatistics::findAllResultNames(
    const std::vector<cvf::ref<RigStimPlanFractureDefinition>>& stimPlanFractureDefinitions )
{
    std::set<std::pair<QString, QString>> resultNames;
    for ( auto stimPlanFractureDefinitionData : stimPlanFractureDefinitions )
    {
        for ( auto propertyNameWithUnit : stimPlanFractureDefinitionData->getStimPlanPropertyNamesUnits() )
        {
            resultNames.insert( propertyNameWithUnit );
        }
    }

    return resultNames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<cvf::cref<RigFractureGrid>> RimEnsembleFractureStatistics::createFractureGrids(
    const std::vector<cvf::ref<RigStimPlanFractureDefinition>>& stimPlanFractureDefinitions,
    RiaDefines::EclipseUnitSystem                               unitSystem,
    const QString&                                              resultNameOnFile,
    MeshAlignmentType                                           meshAlignmentType )
{
    // Defaults to avoid scaling
    double halfLengthScaleFactor = 1.0;
    double heightScaleFactor     = 1.0;

    std::vector<cvf::cref<RigFractureGrid>> fractureGrids;
    for ( auto stimPlanFractureDefinitionData : stimPlanFractureDefinitions )
    {
        double wellPathDepthAtFracture = 0.0;
        if ( meshAlignmentType == MeshAlignmentType::PERFORATION_DEPTH )
            wellPathDepthAtFracture = computeDepthOfWellPathAtFracture( stimPlanFractureDefinitionData );

        // Always use last time steps
        std::vector<double> timeSteps           = stimPlanFractureDefinitionData->timeSteps();
        int                 activeTimeStepIndex = timeSteps.size() - 1;

        cvf::cref<RigFractureGrid> fractureGrid =
            stimPlanFractureDefinitionData->createFractureGrid( resultNameOnFile,
                                                                activeTimeStepIndex,
                                                                halfLengthScaleFactor,
                                                                heightScaleFactor,
                                                                wellPathDepthAtFracture,
                                                                unitSystem );

        if ( fractureGrid.notNull() )
        {
            fractureGrids.push_back( fractureGrid );
        }
    }

    return fractureGrids;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::tuple<double, double, double, double> RimEnsembleFractureStatistics::findSamplingIntervals(
    const std::vector<cvf::ref<RigStimPlanFractureDefinition>>& stimPlanFractureDefinitions,
    std::vector<double>&                                        gridXs,
    std::vector<double>&                                        gridYs ) const
{
    // Find min and max extent of all the grids
    auto [minX, maxX, minY, maxY] = findMaxGridExtents( stimPlanFractureDefinitions, m_meshAlignmentType() );

    if ( m_meshType() == MeshType::UNIFORM )
    {
        generateUniformMesh( minX, maxX, minY, maxY, gridXs, gridYs );
    }
    else if ( m_meshType() == MeshType::NAIVE )
    {
        generateNaiveMesh( minX, maxX, minY, maxY, stimPlanFractureDefinitions, gridXs, gridYs );
    }
    else if ( m_meshType() == MeshType::ADAPTIVE )
    {
        generateAdaptiveMesh( minX, maxX, minY, maxY, stimPlanFractureDefinitions, gridXs, gridYs );
    }

    return std::make_tuple( minX, maxX, minY, maxY );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::tuple<double, double, double, double> RimEnsembleFractureStatistics::findMaxGridExtents(
    const std::vector<cvf::ref<RigStimPlanFractureDefinition>>& stimPlanFractureDefinitions,
    MeshAlignmentType                                           meshAlignmentType )
{
    double minX = std::numeric_limits<double>::max();
    double maxX = -std::numeric_limits<double>::max();
    double minY = std::numeric_limits<double>::max();
    double maxY = -std::numeric_limits<double>::max();

    for ( auto def : stimPlanFractureDefinitions )
    {
        minX = std::min( minX, def->xs().front() );
        maxX = std::max( maxX, def->xs().back() );

        double offset = 0.0;
        if ( meshAlignmentType == MeshAlignmentType::PERFORATION_DEPTH )
            offset = computeDepthOfWellPathAtFracture( def );

        minY = std::min( minY, offset + def->ys().back() );
        maxY = std::max( maxY, offset + def->ys().front() );
    }

    return std::make_tuple( minX, maxX, minY, maxY );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleFractureStatistics::generateUniformMesh( double               minX,
                                                         double               maxX,
                                                         double               minY,
                                                         double               maxY,
                                                         std::vector<double>& gridXs,
                                                         std::vector<double>& gridYs ) const
{
    int    numSamplesX     = m_numSamplesX();
    double sampleDistanceX = linearSampling( minX, maxX, numSamplesX, gridXs );

    int    numSamplesY     = m_numSamplesY();
    double sampleDistanceY = linearSampling( minY, maxY, numSamplesY, gridYs );

    RiaLogging::info( QString( "Uniform Mesh. Output size: %1x%2. Sampling Distance X = %3 Sampling Distance Y = %4" )
                          .arg( numSamplesX )
                          .arg( numSamplesY )
                          .arg( sampleDistanceX )
                          .arg( sampleDistanceY ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleFractureStatistics::generateNaiveMesh(
    double                                                      minX,
    double                                                      maxX,
    double                                                      minY,
    double                                                      maxY,
    const std::vector<cvf::ref<RigStimPlanFractureDefinition>>& stimPlanFractureDefinitions,
    std::vector<double>&                                        gridXs,
    std::vector<double>&                                        gridYs ) const
{
    // Find max number of cells in x direction
    int maxNx = 0;
    for ( auto def : stimPlanFractureDefinitions )
    {
        maxNx = std::max( maxNx, static_cast<int>( def->xs().size() ) );
    }

    // Do linear sampling in x drection
    linearSampling( minX, maxX, maxNx, gridXs );

    std::vector<double> depths;
    for ( auto def : stimPlanFractureDefinitions )
    {
        double offset = 0.0;
        if ( m_meshAlignmentType() == MeshAlignmentType::PERFORATION_DEPTH )
            offset = computeDepthOfWellPathAtFracture( def );

        for ( double y : def->ys() )
        {
            depths.push_back( offset + y );
        }
    }

    std::sort( depths.begin(), depths.end() );
    gridYs = depths;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleFractureStatistics::generateAdaptiveMesh(
    double                                                      minX,
    double                                                      maxX,
    double                                                      minY,
    double                                                      maxY,
    const std::vector<cvf::ref<RigStimPlanFractureDefinition>>& stimPlanFractureDefinitions,
    std::vector<double>&                                        gridXs,
    std::vector<double>&                                        gridYs ) const
{
    // Find max number of cells in x direction
    int maxNx = 0;
    for ( auto def : stimPlanFractureDefinitions )
    {
        maxNx = std::max( maxNx, static_cast<int>( def->xs().size() ) );
    }

    // Do linear sampling in x drection
    linearSampling( minX, maxX, maxNx, gridXs );

    std::vector<Layer> layers;
    generateAllLayers( stimPlanFractureDefinitions, layers, m_meshAlignmentType() );

    const int targetNumLayers = getTargetNumberOfLayers( stimPlanFractureDefinitions );

    // Group the layers into linearly spaced bins
    double totalDepth = maxY - minY;
    double binSize    = totalDepth / targetNumLayers;

    RiaLogging::info( QString( "Adaptive mesh. Total depth: %1. Number of layers: %2. Layer thickness: %3" )
                          .arg( totalDepth )
                          .arg( targetNumLayers )
                          .arg( binSize ) );

    std::vector<double> baseDepth;
    std::vector<double> means;
    computeMeanThicknessPerLayer( layers, targetNumLayers, minY, binSize, means, baseDepth );

    // Scale mean by bin size
    std::vector<double> scaledMeans;
    double              sumScaledMean = 0.0;
    for ( double mean : means )
    {
        double scaledMean = 0.0;
        if ( mean != 0.0 ) scaledMean = binSize / mean;
        scaledMeans.push_back( scaledMean );
        sumScaledMean += scaledMean;
    }

    // Determine the relative thickness given a fixed number of layers
    std::vector<double> relativeThickness;
    relativeThickness.push_back( 0.0 );
    double sumRelativeThickness = 0.0;
    for ( int layerNo = 0; layerNo < targetNumLayers; layerNo++ )
    {
        double thickness = scaledMeans[layerNo] * targetNumLayers / sumScaledMean;
        sumRelativeThickness += thickness;
        relativeThickness.push_back( sumRelativeThickness );
    }

    // Find the index of the last item where value is smaller
    auto findSmallerIndex = []( double value, const std::vector<double>& vec ) {
        for ( size_t i = 0; i < vec.size(); i++ )
            if ( vec[i] > value ) return i - 1;
        return vec.size();
    };

    // Linear interpolation for each layer
    for ( int layerNo = 0; layerNo < targetNumLayers; layerNo++ )
    {
        int binLayerNo = findSmallerIndex( static_cast<double>( layerNo ), relativeThickness );
        CAF_ASSERT( binLayerNo >= 0 );
        CAF_ASSERT( binLayerNo < static_cast<int>( relativeThickness.size() ) );
        double x1         = relativeThickness[binLayerNo];
        double x2         = relativeThickness[binLayerNo + 1];
        double deltaLayer = x2 - x1;
        double y1         = baseDepth[binLayerNo];
        double y2         = baseDepth[binLayerNo + 1];
        double deltaDepth = y2 - y1;
        double offset     = ( static_cast<double>( layerNo ) - x1 ) * deltaDepth / deltaLayer;
        double baseDepth  = y1 + offset;
        gridYs.push_back( baseDepth );
    }
    gridYs.push_back( maxY );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleFractureStatistics::computeMeanThicknessPerLayer( const std::vector<Layer>& layers,
                                                                  int                       targetNumLayers,
                                                                  double                    minY,
                                                                  double                    binSize,
                                                                  std::vector<double>&      means,
                                                                  std::vector<double>&      baseDepth ) const
{
    baseDepth.push_back( minY );

    // Bin the layers into fixed size bins and compute the layer thickess mean per bin
    for ( int layerNo = 0; layerNo < targetNumLayers; layerNo++ )
    {
        double binTopDepth    = minY + layerNo * binSize;
        double binBottomDepth = minY + ( layerNo + 1 ) * binSize;

        baseDepth.push_back( binBottomDepth );

        RiaWeightedHarmonicMeanCalculator  harmonicMeanCalculator;
        RiaWeightedGeometricMeanCalculator geometricMeanCalculator;

        double sum          = 0.0;
        double nMatches     = 0;
        double minThickness = std::numeric_limits<double>::max();
        for ( Layer layer : layers )
        {
            // Layer y direction is negative up
            if ( layer.centerDepth() > binTopDepth && layer.centerDepth() <= binBottomDepth )
            {
                harmonicMeanCalculator.addValueAndWeight( layer.thickness(), 1.0 );
                geometricMeanCalculator.addValueAndWeight( layer.thickness(), 1.0 );

                minThickness = std::min( minThickness, layer.thickness() );

                sum += layer.thickness();
                nMatches++;
            }
        }

        double arithmeticMean = 0.0;
        if ( nMatches > 0 ) arithmeticMean = sum / nMatches;

        double harmonicMean  = harmonicMeanCalculator.weightedMean();
        double geometricMean = geometricMeanCalculator.weightedMean();

        RiaLogging::info( QString( "Binning layer #%1: [%2 - %3] n=%4 means: A=%5 H=%6 G=%7" )
                              .arg( layerNo )
                              .arg( binTopDepth )
                              .arg( binBottomDepth )
                              .arg( nMatches )
                              .arg( arithmeticMean )
                              .arg( harmonicMean )
                              .arg( geometricMean ) );

        double mean = std::numeric_limits<double>::infinity();
        if ( m_adaptiveMeanType() == MeanType::HARMONIC )
        {
            mean = harmonicMean;
        }
        else if ( m_adaptiveMeanType() == MeanType::ARITHMETIC )
        {
            mean = arithmeticMean;
        }
        else if ( m_adaptiveMeanType() == MeanType::GEOMETRIC )
        {
            mean = geometricMean;
        }
        else
        {
            CAF_ASSERT( m_adaptiveMeanType() == MeanType::MINIMUM );
            mean = minThickness;
        }

        means.push_back( mean );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleFractureStatistics::generateAllLayers(
    const std::vector<cvf::ref<RigStimPlanFractureDefinition>>& stimPlanFractureDefinitions,
    std::vector<Layer>&                                         layers,
    MeshAlignmentType                                           meshAlignmentType )
{
    for ( auto def : stimPlanFractureDefinitions )
    {
        double offset = 0.0;
        if ( meshAlignmentType == MeshAlignmentType::PERFORATION_DEPTH )
            offset = computeDepthOfWellPathAtFracture( def );

        bool   isFirst  = true;
        double topDepth = 0.0;
        for ( double y : def->ys() )
        {
            double depth = offset + y;
            if ( !isFirst )
            {
                double bottomDepth = depth;
                Layer  layer( topDepth, bottomDepth );
                layers.push_back( layer );
            }

            isFirst  = false;
            topDepth = depth;
        }
    }

    std::sort( layers.begin(), layers.end(), []( const Layer& a, const Layer& b ) {
        return a.centerDepth() > b.centerDepth();
    } );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimEnsembleFractureStatistics::getTargetNumberOfLayers(
    const std::vector<cvf::ref<RigStimPlanFractureDefinition>>& stimPlanFractureDefinitions ) const
{
    if ( m_adaptiveNumLayersType() == AdaptiveNumLayersType::USER_DEFINED ) return m_adaptiveNumLayers();

    int maxNy = 0;
    int minNy = std::numeric_limits<int>::max();
    int sum   = 0;
    for ( auto def : stimPlanFractureDefinitions )
    {
        int ny = static_cast<int>( def->ys().size() );
        maxNy  = std::max( maxNy, ny );
        minNy  = std::min( minNy, ny );
        sum += ny;
    }

    if ( m_adaptiveNumLayersType() == AdaptiveNumLayersType::MAXIMUM )
        return maxNy;
    else if ( m_adaptiveNumLayersType() == AdaptiveNumLayersType::MINIMUM )
        return minNy;
    else
    {
        CAF_ASSERT( m_adaptiveNumLayersType() == AdaptiveNumLayersType::AVERAGE );
        return static_cast<int>( std::ceil( static_cast<double>( sum ) / stimPlanFractureDefinitions.size() ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimEnsembleFractureStatistics::linearSampling( double               minValue,
                                                      double               maxValue,
                                                      int                  numSamples,
                                                      std::vector<double>& samples )
{
    double sampleDistance = ( maxValue - minValue ) / numSamples;

    for ( int s = 0; s < numSamples; s++ )
    {
        double pos = minValue + s * sampleDistance + sampleDistance * 0.5;
        samples.push_back( pos );
    }

    return sampleDistance;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEnsembleFractureStatistics::isCoordinateInsideFractureCell( double x, double y, const RigFractureCell& cell )
{
    const cvf::Vec3d& minPoint = cell.getPolygon()[0];
    const cvf::Vec3d& maxPoint = cell.getPolygon()[2];
    // TODO: Investigate strange ordering for y coords.
    return ( x > minPoint.x() && x <= maxPoint.x() && y <= minPoint.y() && y > maxPoint.y() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimEnsembleFractureStatistics::computeDepthOfWellPathAtFracture(
    cvf::ref<RigStimPlanFractureDefinition> stimPlanFractureDefinitionData )
{
    double firstTvd = stimPlanFractureDefinitionData->topPerfTvd();
    double lastTvd  = stimPlanFractureDefinitionData->bottomPerfTvd();

    if ( firstTvd != HUGE_VAL && lastTvd != HUGE_VAL )
    {
        return ( firstTvd + lastTvd ) / 2;
    }
    else
    {
        firstTvd = stimPlanFractureDefinitionData->minDepth();
        lastTvd  = stimPlanFractureDefinitionData->maxDepth();
        return ( firstTvd + lastTvd ) / 2;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleFractureStatistics::sampleAllGrids( const std::vector<cvf::cref<RigFractureGrid>>& fractureGrids,
                                                    const std::vector<double>&                     samplesX,
                                                    const std::vector<double>&                     samplesY,
                                                    std::vector<std::vector<double>>&              samples )
{
    for ( size_t y = 0; y < samplesY.size(); y++ )
    {
        for ( size_t x = 0; x < samplesX.size(); x++ )
        {
            double posX = samplesX[x];
            double posY = samplesY[y];

            for ( auto fractureGrid : fractureGrids )
            {
                for ( auto fractureCell : fractureGrid->fractureCells() )
                {
                    if ( isCoordinateInsideFractureCell( posX, posY, fractureCell ) )
                    {
                        size_t idx   = y * samplesX.size() + x;
                        double value = fractureCell.getConductivityValue();
                        if ( !std::isinf( value ) ) samples[idx].push_back( value );
                        break;
                    }
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEnsembleFractureStatistics::writeStatisticsToCsv( const QString& filePath, const RigSlice2D& samples )
{
    QFile data( filePath );
    if ( !data.open( QFile::WriteOnly | QFile::Truncate ) )
    {
        return false;
    }

    QTextStream              stream( &data );
    QString                  fieldSeparator = RiaPreferences::current()->csvTextExportFieldSeparator;
    RifCsvDataTableFormatter formatter( stream, fieldSeparator );

    std::vector<RifTextDataTableColumn> header;
    for ( size_t y = 0; y < samples.ny(); y++ )
        header.push_back( RifTextDataTableColumn( "", RifTextDataTableDoubleFormat::RIF_FLOAT ) );
    formatter.header( header );

    for ( size_t y = 0; y < samples.ny(); y++ )
    {
        for ( size_t x = 0; x < samples.nx(); x++ )
        {
            formatter.add( samples.getValue( x, y ) );
        }
        formatter.rowCompleted();
    }
    formatter.tableCompleted();
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleFractureStatistics::generateStatisticsGrids(
    const std::vector<std::vector<double>>&                                               samples,
    size_t                                                                                numSamplesX,
    size_t                                                                                numSamplesY,
    size_t                                                                                numGrids,
    std::map<RimEnsembleFractureStatistics::StatisticsType, std::shared_ptr<RigSlice2D>>& statisticsGrids,
    const std::vector<caf::AppEnum<RimEnsembleFractureStatistics::StatisticsType>>&       statisticsTypes )
{
    for ( auto t : statisticsTypes )
    {
        std::shared_ptr<RigSlice2D> grid = std::make_shared<RigSlice2D>( numSamplesX, numSamplesY );
        statisticsGrids[t.value()]       = grid;
    }

    auto isCalculationEnabled = []( StatisticsType t, auto statisticsTypes ) {
        return std::find( statisticsTypes.begin(), statisticsTypes.end(), t ) != statisticsTypes.end();
    };

    bool calculateMin        = isCalculationEnabled( StatisticsType::MIN, statisticsTypes );
    bool calculateMax        = isCalculationEnabled( StatisticsType::MAX, statisticsTypes );
    bool calculateMean       = isCalculationEnabled( StatisticsType::MEAN, statisticsTypes );
    bool calculateP10        = isCalculationEnabled( StatisticsType::P10, statisticsTypes );
    bool calculateP50        = isCalculationEnabled( StatisticsType::P50, statisticsTypes );
    bool calculateP90        = isCalculationEnabled( StatisticsType::P90, statisticsTypes );
    bool calculateOccurrence = isCalculationEnabled( StatisticsType::OCCURRENCE, statisticsTypes );

    auto setValueNoInf = []( std::shared_ptr<RigSlice2D>& grid, size_t x, size_t y, double value ) {
        // Guard against inf (happens in the regions not covered by any mesh)
        if ( std::isinf( value ) ) value = 0.0;
        grid->setValue( x, y, value );
    };

    for ( size_t y = 0; y < numSamplesY; y++ )
    {
        for ( size_t x = 0; x < numSamplesX; x++ )
        {
            size_t idx = y * numSamplesX + x;

            if ( calculateMin || calculateMax || calculateMean )
            {
                double min;
                double max;
                double sum;
                double range;
                double mean;
                double dev;
                RigStatisticsMath::calculateBasicStatistics( samples[idx], &min, &max, &sum, &range, &mean, &dev );

                // Only include cells which have values in half of more of the grids
                bool hasEnoughDataPoints = samples[idx].size() >= static_cast<size_t>( std::floor( numGrids / 2.0 ) );
                if ( calculateMean && hasEnoughDataPoints )
                {
                    setValueNoInf( statisticsGrids[RimEnsembleFractureStatistics::StatisticsType::MEAN], x, y, mean );
                }

                if ( calculateMin )
                    setValueNoInf( statisticsGrids[RimEnsembleFractureStatistics::StatisticsType::MIN], x, y, min );

                if ( calculateMax )
                    setValueNoInf( statisticsGrids[RimEnsembleFractureStatistics::StatisticsType::MAX], x, y, max );
            }

            if ( calculateP10 || calculateP50 || calculateP90 )
            {
                double p10;
                double p50;
                double p90;
                double mean;
                RigStatisticsMath::calculateStatisticsCurves( samples[idx], &p10, &p50, &p90, &mean );

                if ( calculateP10 )
                    setValueNoInf( statisticsGrids[RimEnsembleFractureStatistics::StatisticsType::P10], x, y, p10 );

                if ( calculateP50 )
                    setValueNoInf( statisticsGrids[RimEnsembleFractureStatistics::StatisticsType::P50], x, y, p50 );

                if ( calculateP90 )
                    setValueNoInf( statisticsGrids[RimEnsembleFractureStatistics::StatisticsType::P90], x, y, p90 );
            }

            if ( calculateOccurrence )
            {
                statisticsGrids[RimEnsembleFractureStatistics::StatisticsType::OCCURRENCE]->setValue( x,
                                                                                                      y,
                                                                                                      samples[idx].size() );
            }
        }
    }
}
