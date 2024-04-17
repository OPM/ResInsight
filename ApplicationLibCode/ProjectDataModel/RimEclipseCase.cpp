/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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

#include "RimEclipseCase.h"

#include "RiaApplication.h"
#include "RiaColorTables.h"
#include "RiaDefines.h"
#include "RiaFieldHandleTools.h"
#include "RiaLogging.h"
#include "RiaPreferences.h"
#include "RiaQDateTimeTools.h"

#include "CompletionExportCommands/RicWellPathExportCompletionDataFeatureImpl.h"

#include "RicfCommandObject.h"
#include "RifInputPropertyLoader.h"
#include "RifReaderSettings.h"

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigGridManager.h"
#include "RigMainGrid.h"
#include "RigNNCData.h"
#include "RigSimWellData.h"
#include "RigVirtualPerforationTransmissibilities.h"
#include "RigWellResultFrame.h"

#include "Rim2dIntersectionViewCollection.h"
#include "RimCaseCollection.h"
#include "RimCellEdgeColors.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseContourMapView.h"
#include "RimEclipseContourMapViewCollection.h"
#include "RimEclipseInputProperty.h"
#include "RimEclipseInputPropertyCollection.h"
#include "RimEclipsePropertyFilter.h"
#include "RimEclipsePropertyFilterCollection.h"
#include "RimEclipseResultAddress.h"
#include "RimEclipseResultAddressCollection.h"
#include "RimEclipseStatisticsCase.h"
#include "RimEclipseView.h"
#include "RimEclipseViewCollection.h"
#include "RimFaultInViewCollection.h"
#include "RimFormationNames.h"
#include "RimGridCollection.h"
#include "RimIntersectionCollection.h"
#include "RimMainPlotCollection.h"
#include "RimOilField.h"
#include "RimPerforationCollection.h"
#include "RimProject.h"
#include "RimReloadCaseTools.h"
#include "RimReservoirCellResultsStorage.h"
#include "RimStimPlanColors.h"
#include "RimWellLogPlotCollection.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"

#include "cafPdmDocument.h"
#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"
#include "cafPdmUiTreeOrdering.h"
#include "cafProgressInfo.h"

#include "cafUtils.h"
#include <QFileInfo>

CAF_PDM_XML_ABSTRACT_SOURCE_INIT( RimEclipseCase, "RimReservoir" );

//------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseCase::RimEclipseCase()
{
    CAF_PDM_InitScriptableObjectWithNameAndComment( "EclipseCase", ":/Case48x48.png", "", "", "Reservoir", "Abstract base class for Eclipse Cases" );

    CAF_PDM_InitFieldNoDefault( &m_reservoirViews_OBSOLETE, "ReservoirViews", "Views", "", "", "", "All Eclipse Views in the case" );

    CAF_PDM_InitFieldNoDefault( &m_matrixModelResults, "MatrixModelResults", "" );
    CAF_PDM_InitFieldNoDefault( &m_fractureModelResults, "FractureModelResults", "" );

    CAF_PDM_InitField( &m_flipXAxis, "FlipXAxis", false, "Flip X Axis" );
    CAF_PDM_InitField( &m_flipYAxis, "FlipYAxis", false, "Flip Y Axis" );

    CAF_PDM_InitFieldNoDefault( &m_filesContainingFaults, "CachedFileNamesContainingFaults", "" );
    m_filesContainingFaults.uiCapability()->setUiHidden( true );
    // Caching of file names causes issues when using the project file as template, do not save to disk
    // https://github.com/OPM/ResInsight/issues/7308
    m_filesContainingFaults.xmlCapability()->disableIO();

    CAF_PDM_InitFieldNoDefault( &m_contourMapCollection_OBSOLETE, "ContourMaps", "2d Contour Maps" );
    m_contourMapCollection_OBSOLETE = new RimEclipseContourMapViewCollection;

    CAF_PDM_InitFieldNoDefault( &m_inputPropertyCollection, "InputPropertyCollection", "" );
    m_inputPropertyCollection = new RimEclipseInputPropertyCollection;
    m_inputPropertyCollection->parentField()->uiCapability()->setUiTreeHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_resultAddressCollections, "ResultAddressCollections", "Result Addresses" );
    m_resultAddressCollections.uiCapability()->setUiHidden( true );
    m_resultAddressCollections.xmlCapability()->disableIO();

    CAF_PDM_InitFieldNoDefault( &m_viewCollection, "ViewCollection", "Views" );
    m_viewCollection = new RimEclipseViewCollection;

    // Init

    m_matrixModelResults = new RimReservoirCellResultsStorage;
    m_matrixModelResults.uiCapability()->setUiTreeChildrenHidden( true );

    m_fractureModelResults = new RimReservoirCellResultsStorage;
    m_fractureModelResults.uiCapability()->setUiTreeChildrenHidden( true );

    setReservoirData( nullptr );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseCase::~RimEclipseCase()
{
    delete m_matrixModelResults();
    delete m_fractureModelResults();
    delete m_inputPropertyCollection;
    delete m_viewCollection;

    RimProject* project = RimProject::current();
    if ( project )
    {
        if ( project->mainPlotCollection() )
        {
            RimWellLogPlotCollection* plotCollection = project->mainPlotCollection()->wellLogPlotCollection();
            if ( plotCollection )
            {
                plotCollection->removeExtractors( eclipseCaseData() );
            }
        }
    }

    if ( eclipseCaseData() )
    {
        // At this point, we assume that memory should be released
        CVF_ASSERT( eclipseCaseData()->refCount() == 1 );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigEclipseCaseData* RimEclipseCase::eclipseCaseData()
{
    return m_rigEclipseCase.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigEclipseCaseData* RimEclipseCase::eclipseCaseData() const
{
    return m_rigEclipseCase.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCase::ensureDeckIsParsedForEquilData()
{
    if ( m_rigEclipseCase.notNull() )
    {
        QString includeFileAbsolutePathPrefix;
        {
            RiaPreferences* prefs = RiaPreferences::current();
            if ( prefs->readerSettings() )
            {
                includeFileAbsolutePathPrefix = prefs->readerSettings()->includeFileAbsolutePathPrefix();
            }
        }

        QString dataDeckFile;
        {
            QFileInfo fi( gridFileName() );

            dataDeckFile = caf::Utils::constructFullFileName( fi.absolutePath(), fi.baseName(), ".DATA" );
        }

        m_rigEclipseCase->ensureDeckIsParsedForEquilData( dataDeckFile, includeFileAbsolutePathPrefix );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Color3f RimEclipseCase::defaultWellColor( const QString& wellName )
{
    if ( m_wellToColorMap.empty() )
    {
        const caf::ColorTable& colorTable             = RiaColorTables::wellsPaletteColors();
        cvf::Color3ubArray     wellColors             = colorTable.color3ubArray();
        cvf::Color3ubArray     interpolatedWellColors = wellColors;

        const cvf::Collection<RigSimWellData>& simWellData = eclipseCaseData()->wellResults();
        if ( simWellData.size() > 1 )
        {
            interpolatedWellColors = caf::ColorTable::interpolateColorArray( wellColors, simWellData.size() );
        }

        for ( size_t wIdx = 0; wIdx < simWellData.size(); ++wIdx )
        {
            m_wellToColorMap[simWellData[wIdx]->m_wellName] = cvf::Color3f::BLACK;
        }

        size_t wIdx = 0;
        for ( auto& wNameColorPair : m_wellToColorMap )
        {
            wNameColorPair.second = cvf::Color3f( interpolatedWellColors[wIdx] );

            ++wIdx;
        }
    }

    auto nmColor = m_wellToColorMap.find( wellName );
    if ( nmColor != m_wellToColorMap.end() )
    {
        return nmColor->second;
    }
    else
    {
        return cvf::Color3f::LIGHT_GRAY;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigMainGrid* RimEclipseCase::mainGrid() const
{
    if ( eclipseCaseData() )
    {
        return eclipseCaseData()->mainGrid();
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEclipseCase::isGridSizeEqualTo( const RimEclipseCase* otherCase ) const
{
    if ( !mainGrid() || !otherCase || !otherCase->mainGrid() )
    {
        return false;
    }

    return RigGridManager::isMainGridDimensionsEqual( mainGrid(), otherCase->mainGrid() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCase::initAfterRead()
{
    RimCase::initAfterRead();

    if ( RimProject::current()->isProjectFileVersionEqualOrOlderThan( "2024.03.0" ) )
    {
        // Move views to view collection.
        RimEclipseViewCollection* viewColl = viewCollection();
        for ( RimEclipseView* riv : m_reservoirViews_OBSOLETE.childrenByType() )
        {
            CVF_ASSERT( riv );
            riv->setEclipseCase( this );
            m_reservoirViews_OBSOLETE.removeChild( riv );
            viewColl->addView( riv );
        }

        m_reservoirViews_OBSOLETE.clearWithoutDelete();

        // Move contour maps
        auto mapViewColl = contourMapCollection();
        for ( RimEclipseContourMapView* contourMap : m_contourMapCollection_OBSOLETE->views() )
        {
            contourMap->setEclipseCase( this );
            m_contourMapCollection_OBSOLETE->removeChild( contourMap );
            mapViewColl->addView( contourMap );
        }

        m_contourMapCollection_OBSOLETE->clearWithoutDelete();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseView* RimEclipseCase::createAndAddReservoirView( bool useGlobalViewCollection )
{
    RimEclipseViewCollection* viewColl = useGlobalViewCollection ? globalViewCollection() : viewCollection();
    return createAndAddReservoirView( viewColl );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseView* RimEclipseCase::createAndAddReservoirView( RimEclipseViewCollection* viewColl )
{
    if ( !viewColl ) return nullptr;

    return viewColl->addView( this );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseView* RimEclipseCase::createCopyAndAddView( const RimEclipseView* sourceView )
{
    CVF_ASSERT( sourceView );

    RimEclipseView* rimEclipseView =
        dynamic_cast<RimEclipseView*>( sourceView->xmlCapability()->copyByXmlSerialization( caf::PdmDefaultObjectFactory::instance() ) );
    CVF_ASSERT( rimEclipseView );
    rimEclipseView->setEclipseCase( this );

    caf::PdmDocument::updateUiIconStateRecursively( rimEclipseView );

    RimEclipseViewCollection* viewColl = viewCollection();
    if ( !viewColl ) return nullptr;

    viewColl->addView( rimEclipseView );

    // Resolve references after reservoir view has been inserted into Rim structures
    rimEclipseView->resolveReferencesRecursively();
    rimEclipseView->initAfterReadRecursively();

    return rimEclipseView;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigVirtualPerforationTransmissibilities* RimEclipseCase::computeAndGetVirtualPerforationTransmissibilities()
{
    RigEclipseCaseData* rigEclipseCase = eclipseCaseData();
    if ( !rigEclipseCase ) return nullptr;

    if ( rigEclipseCase->virtualPerforationTransmissibilities() == nullptr )
    {
        cvf::ref<RigVirtualPerforationTransmissibilities> perfTrans = new RigVirtualPerforationTransmissibilities;

        std::vector<RimWellPath*> visibleWellPaths;
        bool                      anyPerforationsPresent = false;
        {
            RimProject*               proj      = RimProject::current();
            std::vector<RimWellPath*> wellPaths = proj->allWellPaths();
            for ( auto w : wellPaths )
            {
                if ( w->showWellPath() )
                {
                    visibleWellPaths.push_back( w );

                    if ( !w->perforationIntervalCollection()->perforations().empty() )
                    {
                        anyPerforationsPresent = true;
                    }
                }
            }
        }

        for ( auto w : visibleWellPaths )
        {
            std::vector<RigCompletionData> staticCompletionData =
                RicWellPathExportCompletionDataFeatureImpl::computeStaticCompletionsForWellPath( w, this );

            if ( anyPerforationsPresent )
            {
                std::vector<std::vector<RigCompletionData>> allCompletionData;

                size_t maxIndexCount = 1;
                if ( !timeStepDates().empty() )
                {
                    maxIndexCount = timeStepDates().size();
                }
                for ( size_t i = 0; i < maxIndexCount; i++ )
                {
                    std::vector<RigCompletionData> dynamicCompletionDataOneTimeStep =
                        RicWellPathExportCompletionDataFeatureImpl::computeDynamicCompletionsForWellPath( w, this, i );

                    std::copy( staticCompletionData.begin(), staticCompletionData.end(), std::back_inserter( dynamicCompletionDataOneTimeStep ) );

                    allCompletionData.push_back( dynamicCompletionDataOneTimeStep );
                }

                perfTrans->setCompletionDataForWellPath( w, allCompletionData );
            }
            else
            {
                std::vector<std::vector<RigCompletionData>> allCompletionData;
                allCompletionData.push_back( staticCompletionData );

                perfTrans->setCompletionDataForWellPath( w, allCompletionData );
            }
        }

        for ( const auto& wellRes : rigEclipseCase->wellResults() )
        {
            std::vector<std::vector<RigCompletionData>> completionsPerTimeStep;
            for ( size_t i = 0; i < timeStepDates().size(); i++ )
            {
                std::vector<RigCompletionData> completionData;

                if ( wellRes->hasWellResult( i ) )
                {
                    for ( const auto& wellResultBranch : wellRes->wellResultFrame( i )->wellResultBranches() )
                    {
                        for ( const auto& r : wellResultBranch.branchResultPoints() )
                        {
                            if ( r.isCell() )
                            {
                                RigCompletionData compData( wellRes->m_wellName,
                                                            RigCompletionDataGridCell( r.cellIndex(), rigEclipseCase->mainGrid() ),
                                                            0 );
                                compData.setTransmissibility( r.connectionFactor() );

                                completionData.push_back( compData );
                            }
                        }
                    }
                }

                completionsPerTimeStep.push_back( completionData );

                perfTrans->setCompletionDataForSimWell( wellRes.p(), completionsPerTimeStep );
            }
        }

        rigEclipseCase->setVirtualPerforationTransmissibilities( perfTrans.p() );
    }

    return rigEclipseCase->virtualPerforationTransmissibilities();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCase::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    RimCase::fieldChangedByUi( changedField, oldValue, newValue );
    if ( changedField == &m_releaseResultMemory )
    {
        RimReloadCaseTools::reloadEclipseGrid( this );

        m_releaseResultMemory = oldValue.toBool();
    }
    else if ( changedField == &m_flipXAxis || changedField == &m_flipYAxis )
    {
        RigEclipseCaseData* rigEclipseCase = eclipseCaseData();
        if ( rigEclipseCase )
        {
            rigEclipseCase->mainGrid()->setFlipAxis( m_flipXAxis, m_flipYAxis );

            computeCachedData();

            for ( RimEclipseView* reservoirView : reservoirViews() )
            {
                reservoirView->scheduleReservoirGridGeometryRegen();
                reservoirView->scheduleSimWellGeometryRegen();
                reservoirView->createDisplayModelAndRedraw();
            }
        }
    }
    else if ( changedField == &m_activeFormationNames )
    {
        updateFormationNamesData();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCase::updateFormationNamesData()
{
    RigEclipseCaseData* rigEclipseCase = eclipseCaseData();
    if ( rigEclipseCase )
    {
        if ( activeFormationNames() )
        {
            rigEclipseCase->setActiveFormationNames( activeFormationNames()->formationNamesData() );
        }
        else
        {
            rigEclipseCase->setActiveFormationNames( nullptr );
        }

        // Update plots based on formations
        RimMainPlotCollection::current()->updatePlotsWithFormations();

        std::vector<Rim3dView*> views = this->views();
        for ( Rim3dView* view : views )
        {
            RimEclipseView* eclView = dynamic_cast<RimEclipseView*>( view );

            if ( eclView && eclView->isUsingFormationNames() )
            {
                if ( !activeFormationNames() )
                {
                    if ( eclView->cellResult()->resultType() == RiaDefines::ResultCatType::FORMATION_NAMES )
                    {
                        eclView->cellResult()->setResultVariable( RiaResultNames::undefinedResultName() );
                        eclView->cellResult()->updateConnectedEditors();
                    }

                    RimEclipsePropertyFilterCollection* eclFilColl = eclView->eclipsePropertyFilterCollection();
                    for ( RimEclipsePropertyFilter* propFilter : eclFilColl->propertyFilters() )
                    {
                        if ( propFilter->resultDefinition()->resultType() == RiaDefines::ResultCatType::FORMATION_NAMES )
                        {
                            propFilter->resultDefinition()->setResultVariable( RiaResultNames::undefinedResultName() );
                        }
                    }
                }

                RimEclipsePropertyFilterCollection* eclFilColl = eclView->eclipsePropertyFilterCollection();
                for ( RimEclipsePropertyFilter* propFilter : eclFilColl->propertyFilters() )
                {
                    if ( propFilter->resultDefinition()->resultType() == RiaDefines::ResultCatType::FORMATION_NAMES )
                    {
                        propFilter->setToDefaultValues();
                        propFilter->updateConnectedEditors();
                    }
                }

                view->scheduleGeometryRegen( PROPERTY_FILTERED );
                view->scheduleCreateDisplayModelAndRedraw();
                eclView->intersectionCollection()->scheduleCreateDisplayModelAndRedraw2dIntersectionViews();
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCase::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName )
{
    if ( uiConfigName == "MainWindow.ProjectTree" )
    {
        for ( auto view : m_viewCollection->views() )
        {
            uiTreeOrdering.add( view );
        }

        if ( !m_2dIntersectionViewCollection->views().empty() )
        {
            uiTreeOrdering.add( &m_2dIntersectionViewCollection );
        }
    }
    else if ( uiConfigName == "MainWindow.DataSources" )
    {
        if ( m_resultAddressCollections.empty() ) buildResultChildNodes();
        uiTreeOrdering.add( &m_resultAddressCollections );
    }

    uiTreeOrdering.skipRemainingChildren( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCase::buildResultChildNodes()
{
    m_resultAddressCollections.deleteChildren();

    auto cellResultData = results( RiaDefines::PorosityModelType::MATRIX_MODEL );
    if ( !cellResultData ) return;

    std::vector<RiaDefines::ResultCatType> resultTypes = { RiaDefines::ResultCatType::STATIC_NATIVE,
                                                           RiaDefines::ResultCatType::DYNAMIC_NATIVE,
                                                           RiaDefines::ResultCatType::INPUT_PROPERTY,
                                                           RiaDefines::ResultCatType::GENERATED };
    for ( auto resultType : resultTypes )
    {
        auto resultAddressCollection = new RimEclipseResultAddressCollection;
        resultAddressCollection->setResultType( resultType );
        QString name = caf::AppEnum<RiaDefines::ResultCatType>::uiText( resultType );
        resultAddressCollection->setName( name );

        QStringList resultNames = cellResultData->resultNames( resultType );
        for ( auto resultName : resultNames )
        {
            resultAddressCollection->addAddress( resultName, resultType, this );
        }

        m_resultAddressCollections.push_back( resultAddressCollection );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCase::computeCachedData()
{
    RigEclipseCaseData* rigEclipseCase = eclipseCaseData();
    if ( rigEclipseCase )
    {
        caf::ProgressInfo pInf( 30, "" );

        {
            // NB! Call computeCachedData() first, as this function also computes the bounding box used in other functions, specifically
            // computeActiveCellsBoundingBox()

            auto task = pInf.task( "Calculating Cell Search Tree", 10 );

            std::string aabbTreeInfo;
            rigEclipseCase->mainGrid()->computeCachedData( &aabbTreeInfo );

            // Debug output of the content of the AABB tree
            // RiaLogging::debug( QString::fromStdString( aabbTreeInfo ) );
        }

        {
            auto task = pInf.task( "", 1 );
            computeActiveCellsBoundingBox();
        }

        {
            auto task = pInf.task( "Calculating faults", 17 );

            ensureFaultDataIsComputed();
        }

        {
            auto task = pInf.task( "Calculating Formation Names Result", 2 );
            if ( activeFormationNames() )
            {
                rigEclipseCase->setActiveFormationNames( activeFormationNames()->formationNamesData() );
            }
            else
            {
                rigEclipseCase->setActiveFormationNames( nullptr );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCaseCollection* RimEclipseCase::parentCaseCollection()
{
    return dynamic_cast<RimCaseCollection*>( parentField()->ownerObject() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseContourMapViewCollection* RimEclipseCase::contourMapCollection() const
{
    RimProject* project = RimProject::current();
    if ( !project ) return nullptr;

    RimOilField* oilField = project->activeOilField();
    if ( !oilField ) return nullptr;

    return oilField->eclipseContourMapCollection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseInputPropertyCollection* RimEclipseCase::inputPropertyCollection() const
{
    return m_inputPropertyCollection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> RimEclipseCase::additionalFiles() const
{
    std::vector<QString> additionalFiles;
    for ( const RimEclipseInputProperty* inputProperty : m_inputPropertyCollection()->inputProperties() )
    {
        if ( inputProperty->fileName == gridFileName() ) continue;

        additionalFiles.push_back( inputProperty->fileName().path() );
    }

    return additionalFiles;
}

//--------------------------------------------------------------------------------------------------
/// Loads input property data from the gridFile and additional files
/// Creates new InputProperties if necessary, and flags the unused ones as obsolete
//--------------------------------------------------------------------------------------------------
void RimEclipseCase::loadAndSynchronizeInputProperties( bool importGridOrFaultData )
{
    // Make sure we actually have reservoir data

    CVF_ASSERT( eclipseCaseData() );
    CVF_ASSERT( eclipseCaseData()->mainGrid()->cellCount() != 0 );

    // Then read the properties from all the files referenced by the InputReservoir

    std::vector<QString> filenames;
    for ( const QString& fileName : additionalFiles() )
    {
        filenames.push_back( fileName );
    }

    RifInputPropertyLoader::loadAndSynchronizeInputProperties( inputPropertyCollection(), eclipseCaseData(), filenames, importGridOrFaultData );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCase::ensureFaultDataIsComputed()
{
    RigEclipseCaseData* rigEclipseCase = eclipseCaseData();
    if ( rigEclipseCase )
    {
        bool computeFaults = ( m_readerSettings && m_readerSettings->importFaults() ) ||
                             ( !m_readerSettings && RiaPreferences::current()->readerSettings()->importFaults() );
        if ( computeFaults )
        {
            RigActiveCellInfo* actCellInfo = rigEclipseCase->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL );
            rigEclipseCase->mainGrid()->calculateFaults( actCellInfo );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEclipseCase::ensureNncDataIsComputed()
{
    bool                computedData   = false;
    RigEclipseCaseData* rigEclipseCase = eclipseCaseData();
    if ( rigEclipseCase && rigEclipseCase->mainGrid() )
    {
        computedData = rigEclipseCase->mainGrid()->nncData()->ensureAllConnectionDataIsProcessed();
    }

    return computedData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCase::createDisplayModelAndUpdateAllViews()
{
    for ( const auto& v : views() )
    {
        RimEclipseView* eclipseView = dynamic_cast<RimEclipseView*>( v );
        if ( eclipseView )
        {
            eclipseView->scheduleReservoirGridGeometryRegen();
        }

        v->loadDataAndUpdate();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCase::computeActiveCellsBoundingBox()
{
    if ( !eclipseCaseData() ) return;

    bool useOptimizedVersion = true;

    if ( auto proj = RimProject::current() )
    {
        if ( proj->isProjectFileVersionEqualOrOlderThan( "2023.12.0" ) )
        {
            useOptimizedVersion = false;
        }
    }

    eclipseCaseData()->computeActiveCellBoundingBoxes( useOptimizedVersion );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCase::setReservoirData( RigEclipseCaseData* eclipseCase )
{
    m_rigEclipseCase = eclipseCase;
    if ( eclipseCaseData() )
    {
        m_fractureModelResults()->setCellResults( eclipseCaseData()->results( RiaDefines::PorosityModelType::FRACTURE_MODEL ) );
        m_matrixModelResults()->setCellResults( eclipseCaseData()->results( RiaDefines::PorosityModelType::MATRIX_MODEL ) );
    }
    else
    {
        m_fractureModelResults()->setCellResults( nullptr );
        m_matrixModelResults()->setCellResults( nullptr );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCase::createTimeStepFormatString()
{
    std::vector<QDateTime> timeStepDates = this->timeStepDates();

    m_timeStepFormatString = RiaQDateTimeTools::createTimeFormatStringFromDates( timeStepDates );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::BoundingBox RimEclipseCase::reservoirBoundingBox()
{
    return activeCellsBoundingBox();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::BoundingBox RimEclipseCase::activeCellsBoundingBox() const
{
    if ( m_rigEclipseCase.notNull() && m_rigEclipseCase->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL ) )
    {
        return m_rigEclipseCase->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL )->geometryBoundingBox();
    }
    else
    {
        return cvf::BoundingBox();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::BoundingBox RimEclipseCase::allCellsBoundingBox() const
{
    if ( m_rigEclipseCase.notNull() && m_rigEclipseCase->mainGrid() )
    {
        return m_rigEclipseCase->mainGrid()->boundingBox();
    }
    else
    {
        return cvf::BoundingBox();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RimEclipseCase::displayModelOffset() const
{
    if ( m_rigEclipseCase.notNull() && m_rigEclipseCase->mainGrid() )
    {
        return m_rigEclipseCase->mainGrid()->displayModelOffset();
    }
    else
    {
        return cvf::Vec3d::ZERO;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigCaseCellResultsData* RimEclipseCase::results( RiaDefines::PorosityModelType porosityModel )
{
    if ( m_rigEclipseCase.notNull() )
    {
        return m_rigEclipseCase->results( porosityModel );
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigCaseCellResultsData* RimEclipseCase::results( RiaDefines::PorosityModelType porosityModel ) const
{
    if ( m_rigEclipseCase.notNull() )
    {
        return m_rigEclipseCase->results( porosityModel );
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimReservoirCellResultsStorage* RimEclipseCase::resultsStorage( RiaDefines::PorosityModelType porosityModel )
{
    if ( porosityModel == RiaDefines::PorosityModelType::MATRIX_MODEL )
    {
        return m_matrixModelResults();
    }

    return m_fractureModelResults();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RimReservoirCellResultsStorage* RimEclipseCase::resultsStorage( RiaDefines::PorosityModelType porosityModel ) const
{
    if ( porosityModel == RiaDefines::PorosityModelType::MATRIX_MODEL )
    {
        return m_matrixModelResults();
    }

    return m_fractureModelResults();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> RimEclipseCase::filesContainingFaults() const
{
    std::vector<QString> stdPathList;

    for ( auto& filePath : m_filesContainingFaults() )
    {
        stdPathList.push_back( filePath.path() );
    }

    return stdPathList;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCase::setFilesContainingFaults( const std::vector<QString>& pathStrings )
{
    std::vector<caf::FilePath> filePaths;
    for ( const auto& pathString : pathStrings )
    {
        filePaths.push_back( pathString );
    }

    m_filesContainingFaults = filePaths;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEclipseCase::ensureReservoirCaseIsOpen()
{
    // Call openReserviorCase, as this is a cheap method to call multiple times
    // Add extra testing here if performance issues are seen

    return openReserviorCase();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEclipseCase::openReserviorCase()
{
    if ( !openEclipseGridFile() )
    {
        return false;
    }

    if ( eclipseCaseData() && eclipseCaseData()->mainGrid() && !eclipseCaseData()->mainGrid()->hasValidCharacteristicCellSizes() )
    {
        RigMainGrid* mainGrid = eclipseCaseData()->mainGrid();

        auto activeCellInfo = eclipseCaseData()->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL );
        if ( activeCellInfo )
        {
            std::vector<size_t> reservoirCellIndices;
            for ( size_t i = 0; i < mainGrid->cellCount(); i++ )
            {
                if ( activeCellInfo->isActive( i ) )
                {
                    reservoirCellIndices.push_back( i );
                }
            }
            mainGrid->computeCharacteristicCellSize( reservoirCellIndices );
            mainGrid->computeFaceNormalsDirection( reservoirCellIndices );
        }
    }

    bool createPlaceholderEntries = true;
    if ( dynamic_cast<RimEclipseStatisticsCase*>( this ) )
    {
        // Never create placeholder entries for statistical cases. This does not make sense, and breaks the
        // logic for testing if data is present in RimEclipseStatisticsCase::hasComputedStatistics()
        createPlaceholderEntries = false;
    }

    if ( createPlaceholderEntries )
    {
        if ( RigCaseCellResultsData* results = this->results( RiaDefines::PorosityModelType::MATRIX_MODEL ) )
        {
            results->createPlaceholderResultEntries();
            // After the placeholder result for combined transmissibility is created,
            // make sure the nnc transmissibilities can be addressed by this scalarResultIndex as well

            RigEclipseResultAddress combinedTransmissibilityResAddr( RiaDefines::ResultCatType::STATIC_NATIVE,
                                                                     RiaResultNames::combinedTransmissibilityResultName() );
            if ( results->hasResultEntry( combinedTransmissibilityResAddr ) )
            {
                eclipseCaseData()->mainGrid()->nncData()->setEclResultAddress( RiaDefines::propertyNameCombTrans(),
                                                                               combinedTransmissibilityResAddr );
            }

            RigEclipseResultAddress combinedWaterFluxResAddr( RiaDefines::ResultCatType::DYNAMIC_NATIVE,
                                                              RiaResultNames::combinedWaterFluxResultName() );
            if ( results->hasResultEntry( combinedWaterFluxResAddr ) )
            {
                eclipseCaseData()->mainGrid()->nncData()->setEclResultAddress( RiaDefines::propertyNameFluxWat(), combinedWaterFluxResAddr );
            }

            RigEclipseResultAddress combinedOilFluxResAddr( RiaDefines::ResultCatType::DYNAMIC_NATIVE,
                                                            RiaResultNames::combinedOilFluxResultName() );
            if ( results->hasResultEntry( combinedOilFluxResAddr ) )
            {
                eclipseCaseData()->mainGrid()->nncData()->setEclResultAddress( RiaDefines::propertyNameFluxOil(), combinedOilFluxResAddr );
            }
            RigEclipseResultAddress combinedGasFluxResAddr( RiaDefines::ResultCatType::DYNAMIC_NATIVE,
                                                            RiaResultNames::combinedGasFluxResultName() );

            if ( results->hasResultEntry( combinedGasFluxResAddr ) )
            {
                eclipseCaseData()->mainGrid()->nncData()->setEclResultAddress( RiaDefines::propertyNameFluxGas(), combinedGasFluxResAddr );
            }
        }

        if ( RigCaseCellResultsData* results = this->results( RiaDefines::PorosityModelType::FRACTURE_MODEL ) )
        {
            results->createPlaceholderResultEntries();
        }
    }

    // Build result child nodes after the placeholder entries have been created
    if ( m_resultAddressCollections.empty() ) buildResultChildNodes();

    createTimeStepFormatString();

    // Associate existing well paths with simulation wells
    RimProject* proj = RimProject::current();
    for ( const auto& oilField : proj->oilFields() )
    {
        for ( const auto& wellPath : oilField->wellPathCollection()->allWellPaths() )
        {
            if ( !wellPath->isAssociatedWithSimulationWell() )
            {
                wellPath->tryAssociateWithSimulationWell();
            }
        }
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<Rim3dView*> RimEclipseCase::allSpecialViews() const
{
    std::vector<Rim3dView*> views;
    for ( RimEclipseView* view : reservoirViews() )
    {
        views.push_back( view );
    }

    for ( RimEclipseContourMapView* view : contourMapViews() )
    {
        views.push_back( view );
    }

    return views;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RimEclipseCase::timeStepStrings() const
{
    QStringList stringList;

    const RigCaseCellResultsData* cellResultData = results( RiaDefines::PorosityModelType::MATRIX_MODEL );
    if ( cellResultData )
    {
        int timeStepCount = static_cast<int>( cellResultData->maxTimeStepCount() );
        for ( int i = 0; i < timeStepCount; i++ )
        {
            stringList += timeStepName( i );
        }
    }

    return stringList;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimEclipseCase::timeStepName( int frameIdx ) const
{
    std::vector<QDateTime> timeStepDates = this->timeStepDates();
    if ( frameIdx < static_cast<int>( timeStepDates.size() ) )
    {
        QDateTime date = timeStepDates.at( frameIdx );

        return RiaQDateTimeTools::toStringUsingApplicationLocale( date, m_timeStepFormatString );
    }

    return QString( "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimEclipseCase::characteristicCellSize() const
{
    const RigEclipseCaseData* rigEclipseCase = eclipseCaseData();
    if ( rigEclipseCase && rigEclipseCase->mainGrid() )
    {
        double maxSize = 200.0;
        return std::min( rigEclipseCase->mainGrid()->characteristicIJCellSize(), maxSize );
    }

    return 10.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<QString> RimEclipseCase::sortedSimWellNames() const
{
    std::set<QString> sortedWellNames;
    if ( eclipseCaseData() )
    {
        const cvf::Collection<RigSimWellData>& simWellData = eclipseCaseData()->wellResults();

        for ( size_t wIdx = 0; wIdx < simWellData.size(); ++wIdx )
        {
            sortedWellNames.insert( simWellData[wIdx]->m_wellName );
        }
    }
    return sortedWellNames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QDateTime> RimEclipseCase::timeStepDates() const
{
    if ( results( RiaDefines::PorosityModelType::MATRIX_MODEL ) )
    {
        return results( RiaDefines::PorosityModelType::MATRIX_MODEL )->timeStepDates();
    }
    return std::vector<QDateTime>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEclipseCase::importAsciiInputProperties( const QStringList& fileNames )
{
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCase::setReaderSettings( std::shared_ptr<RifReaderSettings> readerSettings )
{
    m_readerSettings = readerSettings;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCase::updateResultAddressCollection()
{
    m_resultAddressCollections.deleteChildren();
    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseViewCollection* RimEclipseCase::viewCollection() const
{
    return m_viewCollection;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseViewCollection* RimEclipseCase::globalViewCollection() const
{
    RimProject* project = RimProject::current();
    if ( !project ) return nullptr;

    RimOilField* oilField = project->activeOilField();
    if ( !oilField ) return nullptr;

    return oilField->eclipseViewCollection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimEclipseView*> RimEclipseCase::reservoirViews() const
{
    std::vector<RimEclipseView*> views;

    addViewsFromViewCollection( views, viewCollection() );
    addViewsFromViewCollection( views, globalViewCollection() );
    return views;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCase::addViewsFromViewCollection( std::vector<RimEclipseView*>& views, const RimEclipseViewCollection* viewColl ) const
{
    if ( viewColl )
    {
        for ( auto view : viewColl->views() )
        {
            if ( view && view->eclipseCase() && view->eclipseCase() == this )
            {
                views.push_back( view );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimEclipseContourMapView*> RimEclipseCase::contourMapViews() const
{
    std::vector<RimEclipseContourMapView*> views;
    if ( RimEclipseContourMapViewCollection* viewColl = contourMapCollection() )
    {
        for ( auto view : viewColl->views() )
        {
            if ( view && view->eclipseCase() && view->eclipseCase() == this )
            {
                views.push_back( view );
            }
        }
    }

    return views;
}
