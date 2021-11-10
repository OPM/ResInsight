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

#include "RiaApplication.h"
#include "RiaDefines.h"
#include "RiaInterpolationTools.h"
#include "RiaLogging.h"
#include "RiaNumberFormat.h"
#include "RiaPreferences.h"
#include "RiaWeightedGeometricMeanCalculator.h"
#include "RiaWeightedHarmonicMeanCalculator.h"

#include "RigEnsembleFractureStatisticsCalculator.h"
#include "RigFractureGrid.h"
#include "RigHistogramData.h"
#include "RigSlice2D.h"
#include "RigStatisticsMath.h"
#include "RigStimPlanFractureDefinition.h"

#ifdef USE_QTCHARTS
#include "RimEnsembleFractureStatisticsPlot.h"
#endif
#include "RimFractureTemplateCollection.h"
#include "RimHistogramCalculator.h"
#include "RimProject.h"
#include "RimStimPlanFractureTemplate.h"

#include "RifCsvDataTableFormatter.h"
#include "RifEnsembleFractureStatisticsExporter.h"
#include "RifStimPlanXmlReader.h"

#include "FractureCommands/RicNewStimPlanFractureTemplateFeature.h"

#include "cafAppEnum.h"
#include "cafPdmUiLineEditor.h"
#include "cafPdmUiTextEditor.h"
#include "cafPdmUiToolButtonEditor.h"
#include "cafPdmUiTreeSelectionEditor.h"

#include <cmath>

#include <QDir>
#include <QFile>
#include <QIntValidator>

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

    CAF_PDM_InitField( &m_excludeZeroWidthFractures,
                       "ExcludeZeroWidthFractures",
                       true,
                       "Exclude Zero Width Fractures",
                       "",
                       "",
                       "" );

    CAF_PDM_InitFieldNoDefault( &m_statisticsTable, "StatisticsTable", "Statistics Table", "", "", "" );
    m_statisticsTable.uiCapability()->setUiEditorTypeName( caf::PdmUiTextEditor::uiEditorTypeName() );
    m_statisticsTable.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );
    m_statisticsTable.uiCapability()->setUiReadOnly( true );
    m_statisticsTable.xmlCapability()->disableIO();

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
    else if ( field == &m_statisticsTable )
    {
        auto myAttr = dynamic_cast<caf::PdmUiTextEditorAttribute*>( attribute );
        if ( myAttr )
        {
            myAttr->wrapMode = caf::PdmUiTextEditorAttribute::NoWrap;
            myAttr->textMode = caf::PdmUiTextEditorAttribute::HTML;
        }
    }
    else if ( field == &m_adaptiveNumLayers || field == &m_numSamplesX || field == &m_numSamplesY )
    {
        caf::PdmUiLineEditorAttribute* lineEditorAttr = dynamic_cast<caf::PdmUiLineEditorAttribute*>( attribute );
        if ( lineEditorAttr )
        {
            // Positive integer
            QIntValidator* validator  = new QIntValidator( 1, std::numeric_limits<int>::max(), nullptr );
            lineEditorAttr->validator = validator;
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

        // Create (or reuse matching) templates for the statistics
        auto updatedTemplates = RicNewStimPlanFractureTemplateFeature::createNewTemplatesFromFiles( filePaths, true );

        // Update views
        if ( !updatedTemplates.empty() )
        {
            RimFractureTemplateCollection* templateCollection = nullptr;
            updatedTemplates.front()->firstAncestorOrThisOfTypeAsserted( templateCollection );
            templateCollection->updateConnectedEditors();

            RimProject::current()->scheduleCreateDisplayModelAndRedrawAllViews();
        }
    }
    else if ( changedField == &m_excludeZeroWidthFractures )
    {
        loadAndUpdateData();

#ifdef USE_QTCHARTS
        // Update referring plots
        std::vector<caf::PdmObjectHandle*> referringObjects;
        this->objectsWithReferringPtrFields( referringObjects );
        for ( caf::PdmObjectHandle* obj : referringObjects )
        {
            auto plot = dynamic_cast<RimEnsembleFractureStatisticsPlot*>( obj );
            if ( plot ) plot->loadDataAndUpdate();
        }
#endif
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleFractureStatistics::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    caf::PdmUiOrdering* settingsGroup = uiOrdering.addNewGroup( "Settings" );
    settingsGroup->add( nameField() );
    settingsGroup->add( &m_excludeZeroWidthFractures );
    settingsGroup->add( &m_meshAlignmentType );
    settingsGroup->add( &m_meshType );
    settingsGroup->add( &m_numSamplesX );
    settingsGroup->add( &m_numSamplesY );
    bool isUniformMesh = m_meshType() == MeshType::UNIFORM;
    m_numSamplesX.uiCapability()->setUiHidden( !isUniformMesh );
    m_numSamplesY.uiCapability()->setUiHidden( !isUniformMesh );

    settingsGroup->add( &m_adaptiveMeanType );
    settingsGroup->add( &m_adaptiveNumLayersType );
    settingsGroup->add( &m_adaptiveNumLayers );

    bool isAdaptiveMesh = m_meshType() == MeshType::ADAPTIVE;
    m_adaptiveMeanType.uiCapability()->setUiHidden( !isAdaptiveMesh );
    m_adaptiveNumLayersType.uiCapability()->setUiHidden( !isAdaptiveMesh );

    bool adaptiveSamplesUserDefined = m_adaptiveNumLayersType() == AdaptiveNumLayersType::USER_DEFINED;
    m_adaptiveNumLayers.uiCapability()->setUiHidden( !isAdaptiveMesh || !adaptiveSamplesUserDefined );

    settingsGroup->add( &m_selectedStatisticsType );
    settingsGroup->add( &m_computeStatistics );

    caf::PdmUiOrdering* statisticsGroup = uiOrdering.addNewGroup( "Statistics" );
    statisticsGroup->add( &m_statisticsTable );

    uiOrdering.add( &m_filePathsTable );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleFractureStatistics::loadAndUpdateData()
{
    m_filePathsTable = generateFilePathsTable();

    auto unitSystem = RiaDefines::EclipseUnitSystem::UNITS_METRIC;

    std::vector<cvf::ref<RigStimPlanFractureDefinition>> stimPlanFractureDefinitions =
        readFractureDefinitions( m_filePaths.v(), unitSystem );

    // Log area and conductivty for each fracture for debugging
    std::vector<double> area =
        RigEnsembleFractureStatisticsCalculator::calculateProperty( stimPlanFractureDefinitions,
                                                                    RigEnsembleFractureStatisticsCalculator::PropertyType::AREA );
    std::vector<double> conductivity =
        RigEnsembleFractureStatisticsCalculator::calculateProperty( stimPlanFractureDefinitions,
                                                                    RigEnsembleFractureStatisticsCalculator::PropertyType::KFWF );

    CAF_ASSERT( m_filePaths.v().size() == area.size() );
    CAF_ASSERT( area.size() == conductivity.size() );
    for ( size_t i = 0; i < m_filePaths.v().size(); i++ )
    {
        RiaLogging::info(
            QString( "%1 Area: %2 Conductivity: %3" ).arg( m_filePaths.v()[i].path() ).arg( area[i] ).arg( conductivity[i] ) );
    }

    if ( m_excludeZeroWidthFractures() )
    {
        size_t numBeforeFiltering = stimPlanFractureDefinitions.size();
        stimPlanFractureDefinitions =
            RigEnsembleFractureStatisticsCalculator::removeZeroWidthDefinitions( stimPlanFractureDefinitions );
        size_t numRemoved = numBeforeFiltering - stimPlanFractureDefinitions.size();
        RiaLogging::info( QString( "Excluded %1 zero width fractures." ).arg( numRemoved ) );
    }

    m_statisticsTable = generateStatisticsTable( stimPlanFractureDefinitions );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> RimEnsembleFractureStatistics::computeStatistics()
{
    auto unitSystem = RiaDefines::EclipseUnitSystem::UNITS_METRIC;

    std::vector<cvf::ref<RigStimPlanFractureDefinition>> stimPlanFractureDefinitions =
        readFractureDefinitions( m_filePaths.v(), unitSystem );

    if ( m_excludeZeroWidthFractures() )
    {
        stimPlanFractureDefinitions =
            RigEnsembleFractureStatisticsCalculator::removeZeroWidthDefinitions( stimPlanFractureDefinitions );
    }

    std::set<std::pair<QString, QString>> availableResults = findAllResultNames( stimPlanFractureDefinitions );

    std::map<std::pair<RimEnsembleFractureStatistics::StatisticsType, QString>, std::shared_ptr<RigSlice2D>> statisticsGridsAll;

    auto selectedStatistics = m_selectedStatisticsType.value();

    // TODO: take from an incoming xml?
    double timeStep = 1.0;

    double referenceDepth = 0.0;

    RigStimPlanFractureDefinition::Orientation orientation = RigStimPlanFractureDefinition::Orientation::UNDEFINED;
    if ( m_meshAlignmentType() == MeshAlignmentType::PERFORATION_DEPTH )
    {
        for ( auto definition : stimPlanFractureDefinitions )
        {
            referenceDepth += computeDepthOfWellPathAtFracture( definition );
            // Take the first orientation which is defined (all the fractures
            // should have the same orientation).
            if ( orientation == RigStimPlanFractureDefinition::Orientation::UNDEFINED )
                orientation = definition->orientation();
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

        double           numBins = 50;
        RigHistogramData areaHistogramData =
            RigEnsembleFractureStatisticsCalculator::createStatisticsData( stimPlanFractureDefinitions,
                                                                           RigEnsembleFractureStatisticsCalculator::PropertyType::AREA,
                                                                           numBins );

        std::vector<std::vector<double>> samples( gridXs.size() * gridYs.size() );
        std::shared_ptr<RigSlice2D>      areaGrid     = std::make_shared<RigSlice2D>( gridXs.size(), gridYs.size() );
        std::shared_ptr<RigSlice2D>      distanceGrid = std::make_shared<RigSlice2D>( gridXs.size(), gridYs.size() );
        sampleAllGrids( fractureGrids, gridXs, gridYs, samples, areaGrid, distanceGrid );

        std::map<RimEnsembleFractureStatistics::StatisticsType, std::shared_ptr<RigSlice2D>> statisticsGrids;
        generateStatisticsGrids( samples,
                                 gridXs.size(),
                                 gridYs.size(),
                                 fractureGrids.size(),
                                 statisticsGrids,
                                 selectedStatistics,
                                 areaHistogramData,
                                 areaGrid,
                                 distanceGrid );

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
                                                                   unitSystem,
                                                                   orientation );

        xmlFilePaths.push_back( xmlFilePath );
    }

    return xmlFilePaths;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<cvf::ref<RigStimPlanFractureDefinition>> RimEnsembleFractureStatistics::readFractureDefinitions() const
{
    return readFractureDefinitions( m_filePaths, RiaDefines::EclipseUnitSystem::UNITS_METRIC );
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
bool RimEnsembleFractureStatistics::excludeZeroWidthFractures() const
{
    return m_excludeZeroWidthFractures;
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
        int                 activeTimeStepIndex = static_cast<int>( timeSteps.size() - 1 );

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
    CAF_ASSERT( m_numSamplesX > 0 );
    CAF_ASSERT( m_numSamplesY > 0 );

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
        int binLayerNo = static_cast<int>( findSmallerIndex( static_cast<double>( layerNo ), relativeThickness ) );
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
    if ( m_adaptiveNumLayersType() == AdaptiveNumLayersType::USER_DEFINED )
    {
        CAF_ASSERT( m_adaptiveNumLayers() > 0 );
        return m_adaptiveNumLayers();
    }

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
    CAF_ASSERT( numSamples > 0 );
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
                                                    std::vector<std::vector<double>>&              samples,
                                                    std::shared_ptr<RigSlice2D>                    areaGrid,
                                                    std::shared_ptr<RigSlice2D>                    distanceGrid )
{
    auto computeCellSideLength = []( const std::vector<double>& values, size_t idx ) {
        if ( idx < values.size() - 1 )
            return values[idx + 1] - values[idx];
        else
            return values[1] - values[0];
    };

    const int ny = static_cast<int>( samplesY.size() );
#pragma omp parallel for
    for ( int y = 0; y < ny; y++ )
    {
        for ( size_t x = 0; x < samplesX.size(); x++ )
        {
            // Position of cell center
            const cvf::Vec3d pos( samplesX[x], samplesY[y], 0.0 );
            const size_t     idx = y * samplesX.size() + x;

            double sumDistance = 0.0;
            for ( auto fractureGrid : fractureGrids )
            {
                // Sample each fracture grid at a given position
                const RigFractureCell* fractureCell = fractureGrid->getCellFromPosition( pos );
                if ( fractureCell )
                {
                    double value = fractureCell->getConductivityValue();
                    if ( !std::isinf( value ) ) samples[idx].push_back( value );
                }

                // Find distance to the fracture grid well path intersection for the given
                // cell center position (X, Y).
                std::pair<size_t, size_t> centerIJ = fractureGrid->fractureCellAtWellCenter();
                size_t centerIdx = fractureGrid->getGlobalIndexFromIJ( centerIJ.first, centerIJ.second );
                const RigFractureCell& centerCell = fractureGrid->cellFromIndex( centerIdx );
                const cvf::Vec3d&      centerPos  = centerCell.centerPosition();

                sumDistance += pos.pointDistance( centerPos );
            }

            // Average distance for all the fractures
            distanceGrid->setValue( x, y, sumDistance / fractureGrids.size() );

            // Compute the area of the cell
            double area = 0.0;
            if ( !samples.empty() )
            {
                double sum = 0.0;
                for ( auto s : samples[idx] )
                    sum += s;

                // Only cells with non-zero conductivity have defined area
                if ( sum > 0.0 )
                {
                    double diffX = computeCellSideLength( samplesX, x );
                    double diffY = computeCellSideLength( samplesY, y );
                    area         = std::fabs( diffX ) * std::fabs( diffY );
                }
            }
            areaGrid->setValue( x, y, area );
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
    const std::vector<caf::AppEnum<RimEnsembleFractureStatistics::StatisticsType>>&       statisticsTypes,
    const RigHistogramData&                                                               areaHistogram,
    std::shared_ptr<RigSlice2D>                                                           areaGrid,
    std::shared_ptr<RigSlice2D>                                                           distanceGrid )
{
    auto setValueNoInf = []( RigSlice2D& grid, size_t x, size_t y, double value ) {
        // Guard against inf (happens in the regions not covered by any mesh)
        if ( std::isinf( value ) ) value = 0.0;
        grid.setValue( x, y, value );
    };

    auto removeNonPositiveValues = []( const std::vector<double>& values ) {
        std::vector<double> nonZeroValues;
        for ( double value : values )
            if ( value > 0.0 ) nonZeroValues.push_back( value );
        return nonZeroValues;
    };

    RigSlice2D occurrenceGrid( numSamplesX, numSamplesY );
    RigSlice2D meanGrid( numSamplesX, numSamplesY );

    const int ny = static_cast<int>( numSamplesY );
#pragma omp parallel for
    for ( int y = 0; y < ny; y++ )
    {
        for ( size_t x = 0; x < numSamplesX; x++ )
        {
            size_t idx = y * numSamplesX + x;

            // Remove samples without positive values (no conductivity).
            std::vector<double> values = removeNonPositiveValues( samples[idx] );

            double min;
            double max;
            double sum;
            double range;
            double mean;
            double dev;
            RigStatisticsMath::calculateBasicStatistics( values, &min, &max, &sum, &range, &mean, &dev );
            setValueNoInf( meanGrid, x, y, mean );

            // Internal occurrence grid for the area correction is always calculated
            occurrenceGrid.setValue( x, y, values.size() );
        }
    }

    std::map<RimEnsembleFractureStatistics::StatisticsType, double> areaMapping;
    areaMapping[RimEnsembleFractureStatistics::StatisticsType::MIN]        = areaHistogram.min;
    areaMapping[RimEnsembleFractureStatistics::StatisticsType::MAX]        = areaHistogram.max;
    areaMapping[RimEnsembleFractureStatistics::StatisticsType::MEAN]       = areaHistogram.mean;
    areaMapping[RimEnsembleFractureStatistics::StatisticsType::P50]        = areaHistogram.mean;
    areaMapping[RimEnsembleFractureStatistics::StatisticsType::P10]        = areaHistogram.p10;
    areaMapping[RimEnsembleFractureStatistics::StatisticsType::P90]        = areaHistogram.p90;
    areaMapping[RimEnsembleFractureStatistics::StatisticsType::OCCURRENCE] = areaHistogram.max;

    // Post-process the resulting grids improve area representation
    for ( auto statisticsType : statisticsTypes )
    {
        if ( statisticsType != RimEnsembleFractureStatistics::StatisticsType::OCCURRENCE )
            statisticsGrids[statisticsType] =
                setCellsToFillTargetArea( meanGrid, occurrenceGrid, *areaGrid, *distanceGrid, areaMapping[statisticsType] );
        else
            statisticsGrids[statisticsType] = setCellsToFillTargetArea( occurrenceGrid,
                                                                        occurrenceGrid,
                                                                        *areaGrid,
                                                                        *distanceGrid,
                                                                        areaMapping[statisticsType] );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::shared_ptr<RigSlice2D> RimEnsembleFractureStatistics::setCellsToFillTargetArea( const RigSlice2D& grid,
                                                                                     const RigSlice2D& occurrenceGrid,
                                                                                     const RigSlice2D& areaGrid,
                                                                                     const RigSlice2D& distanceGrid,
                                                                                     double            targetArea )
{
    std::shared_ptr<RigSlice2D> outputGrid = std::make_shared<RigSlice2D>( grid.nx(), grid.ny() );

    // Invalid target area: can happen for P10/P90 grids.
    if ( std::isinf( targetArea ) ) return outputGrid;

    // Internal cell data class for ordering cells.
    class CellData
    {
    public:
        size_t x;
        size_t y;
        double occurrence;
        double area;
        double distance;

        // The cells are ordered by:
        // 1. Occurrence: how often the cell had conductivity in the individual fractures.
        // 2. Distance: distance from the well path intersection point (average for all fractures).
        //              Short distance is preferred.
        // 3. Area: The area of the cell. Smaller is preferred.
        // 4. X and Y cell index: Not important for sorting, but used to avoid rejecting cells
        //       when everything else is equal.
        bool operator<( CellData const& p ) const
        {
            if ( occurrence > p.occurrence )
                return true;
            else if ( occurrence == p.occurrence && distance < p.distance )
                return true;
            else if ( occurrence == p.occurrence && distance == p.distance && area < p.area )
                return true;
            else if ( occurrence == p.occurrence && distance == p.distance && area == p.area && x < p.x )
                return true;
            else if ( occurrence == p.occurrence && distance == p.distance && area == p.area && x == p.x && y < p.y )
                return true;

            return false;
        }
    };

    // Create ordered list of the cells
    std::set<CellData> cells;
    for ( size_t y = 0; y < occurrenceGrid.ny(); y++ )
    {
        for ( size_t x = 0; x < occurrenceGrid.nx(); x++ )
        {
            CellData cell;
            cell.x          = x;
            cell.y          = y;
            cell.occurrence = occurrenceGrid.getValue( x, y );
            cell.area       = areaGrid.getValue( x, y );
            cell.distance   = distanceGrid.getValue( x, y );
            cells.insert( cell );
        }
    }

    // Fill cells in the output grid until the target area is reached.
    // This ensures that the statistics fracture grids have representantive sizes.
    double area = 0.0;
    for ( const CellData& cellData : cells )
    {
        if ( area < targetArea )
        {
            double value = grid.getValue( cellData.x, cellData.y );
            if ( !std::isinf( value ) && cellData.area > 0.0 )
            {
                outputGrid->setValue( cellData.x, cellData.y, value );
                area += cellData.area;
            }
        }
        else
        {
            // Target area is reached.
            break;
        }
    }

    RiaLogging::info( QString( "Statistics fracture area: %1 target area: %2" ).arg( area ).arg( targetArea ) );

    return outputGrid;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimEnsembleFractureStatistics::generateStatisticsTable(
    const std::vector<cvf::ref<RigStimPlanFractureDefinition>>& stimPlanFractureDefinitions ) const
{
    std::vector<RigEnsembleFractureStatisticsCalculator::PropertyType> propertyTypes =
        RigEnsembleFractureStatisticsCalculator::propertyTypes();

    QString text;
    text += "<table border=1><thead><tr bgcolor=lightblue>";
    std::vector<QString> statisticsTypes = { "Name", "Minimum", "P90", "Mean", "P10", "Maximum" };
    for ( auto statType : statisticsTypes )
    {
        text += QString( "<th>%1</th>" ).arg( statType );
    }

    text += "</thead>";
    text += "<tbody>";

    auto emptyTextOnInf = []( double value, RiaNumberFormat::NumberFormatType numberFormat, int precision ) {
        if ( std::isinf( value ) )
            return QString( "" );
        else
            return RiaNumberFormat::valueToText( value, numberFormat, precision );
    };

    for ( auto propertyType : propertyTypes )
    {
        QString name    = caf::AppEnum<RigEnsembleFractureStatisticsCalculator::PropertyType>::uiText( propertyType );
        int     numBins = 50;
        RigHistogramData histogramData =
            RigEnsembleFractureStatisticsCalculator::createStatisticsData( stimPlanFractureDefinitions,
                                                                           propertyType,
                                                                           numBins );

        auto [numberFormat, precision] = RigEnsembleFractureStatisticsCalculator::numberFormatForProperty( propertyType );
        text += QString( "<tr>"
                         "<td>%1</td>"
                         "<td align=right>%2</td>"
                         "<td align=right>%3</td>"
                         "<td align=right>%4</td>"
                         "<td align=right>%5</td>"
                         "<td align=right>%6</td>"
                         "</tr>" )
                    .arg( name )
                    .arg( emptyTextOnInf( histogramData.min, numberFormat, precision ) )
                    .arg( emptyTextOnInf( histogramData.p90, numberFormat, precision ) )
                    .arg( emptyTextOnInf( histogramData.mean, numberFormat, precision ) )
                    .arg( emptyTextOnInf( histogramData.p10, numberFormat, precision ) )
                    .arg( emptyTextOnInf( histogramData.max, numberFormat, precision ) );
    }

    text += "</tbody>";
    text += "</table>";
    return text;
}
