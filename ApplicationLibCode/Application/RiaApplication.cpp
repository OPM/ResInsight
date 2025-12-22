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
#include "RiaApplication.h"

#include "Cloud/RiaConnectorTools.h"
#include "Cloud/RiaOsduConnector.h"
#include "Cloud/RiaSumoConnector.h"
#include "Cloud/RiaSumoDefines.h"
#include "KeyValueStore/RiaKeyValueStore.h"
#include "RiaDefines.h"
#include "RiaOsduDefines.h"

#include "RiaBaseDefs.h"
#include "RiaFilePathTools.h"
#include "RiaFontCache.h"
#include "RiaImportEclipseCaseTools.h"
#include "RiaLogging.h"
#include "RiaOpenMPTools.h"
#include "RiaOpenTelemetryManager.h"
#include "RiaPlotWindowRedrawScheduler.h"
#include "RiaPreferences.h"
#include "RiaPreferencesOsdu.h"
#include "RiaPreferencesSumo.h"
#include "RiaPreferencesSystem.h"
#include "RiaProjectModifier.h"
#include "RiaRegressionTestRunner.h"
#include "RiaSocketServer.h"
#include "RiaTextStringTools.h"
#include "RiaToCafLogging.h"
#include "RiaVersionInfo.h"
#include "RiaViewRedrawScheduler.h"
#include "RiaWellNameComparer.h"

#include "RicImportGeneralDataFeature.h"
#include "RicfCommandFileExecutor.h"
#include "RicfCommandObject.h"

#include "Formations/RimFormationNamesCollection.h"
#include "PlotTemplates/RimPlotTemplateFolderItem.h"
#include "Polygons/RimPolygonCollection.h"
#include "QuickAccess/RimQuickAccessCollection.h"
#include "Rim2dIntersectionViewCollection.h"
#include "RimCellFilterCollection.h"
#include "RimCommandRouter.h"
#include "RimCompletionTemplateCollection.h"
#include "RimEclipseCaseCollection.h"
#include "RimEclipseCaseEnsemble.h"
#include "RimEclipseView.h"
#include "RimEclipseViewCollection.h"
#include "RimEnsembleWellLogsCollection.h"
#include "RimFaultReactivationModelCollection.h"
#include "RimFileWellPath.h"
#include "RimFileWellPathDataLoader.h"
#include "RimFractureTemplateCollection.h"
#include "RimGeoMechCase.h"
#include "RimGeoMechModels.h"
#include "RimGeoMechView.h"
#include "RimGridCalculationCollection.h"
#include "RimGridSummaryCase.h"
#include "RimIdenticalGridCaseGroup.h"
#include "RimMainPlotCollection.h"
#include "RimModeledWellPath.h"
#include "RimModeledWellPathDataLoader.h"
#include "RimObservedDataCollection.h"
#include "RimObservedFmuRftData.h"
#include "RimObservedSummaryData.h"
#include "RimOilField.h"
#include "RimOsduWellLog.h"
#include "RimOsduWellLogDataLoader.h"
#include "RimOsduWellPath.h"
#include "RimOsduWellPathDataLoader.h"
#include "RimPlotWindow.h"
#include "RimProject.h"
#include "RimScriptCollection.h"
#include "RimSeismicData.h"
#include "RimSeismicDataCollection.h"
#include "RimSeismicView.h"
#include "RimSeismicViewCollection.h"
#include "RimSimWellInViewCollection.h"
#include "RimStimPlanColors.h"
#include "RimStimPlanModel.h"
#include "RimStimPlanModelCollection.h"
#include "RimSummaryCalculationCollection.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSummaryEnsemble.h"
#include "RimSurfaceCollection.h"
#include "RimViewLinker.h"
#include "RimViewLinkerCollection.h"
#include "RimWellLogFile.h"
#include "RimWellLogFileDataLoader.h"
#include "RimWellLogLasFile.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"
#include "RimWellPathFracture.h"
#include "VerticalFlowPerformance/RimVfpDataCollection.h"
#include "VerticalFlowPerformance/RimVfpPlotCollection.h"

#include "Riu3DMainWindowTools.h"
#include "RiuMainWindow.h"
#include "RiuViewer.h"
#include "RiuViewerCommands.h"

#include "cafDataLoadController.h"
#include "cafPdmCodeGenerator.h"
#include "cafPdmDataValueField.h"
#include "cafPdmDefaultObjectFactory.h"
#include "cafPdmDeprecation.h"
#include "cafPdmMarkdownBuilder.h"
#include "cafPdmMarkdownGenerator.h"
#include "cafPdmScriptIOMessages.h"
#include "cafPdmSettings.h"
#include "cafPdmUiModelChangeDetector.h"
#include "cafProgressInfo.h"
#include "cafSelectionManager.h"
#include "cafUiProcess.h"
#include "cafUtils.h"

#include <QCoreApplication>

#include <memory>

#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h> // for usleep
#endif // WIN32

// Required to ignore warning of usused variable when defining caf::PdmMarkdownGenerator
#if defined( __clang__ )
#pragma clang diagnostic ignored "-Wunused-variable"
#endif

RiaApplication* RiaApplication::s_riaApplication = nullptr;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaApplication* RiaApplication::instance()
{
    return s_riaApplication;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaApplication::RiaApplication()
    : m_socketServer( nullptr )
    , m_workerProcess( nullptr )
    , m_preferences( nullptr )
    , m_runningWorkerProcess( false )
    , m_currentScriptCaseId( -1 )
{
    CAF_ASSERT( s_riaApplication == nullptr );
    s_riaApplication = this;

    // USed to get registry settings in the right place
    QCoreApplication::setOrganizationName( RI_COMPANY_NAME );
    QCoreApplication::setApplicationName( RI_APPLICATION_NAME );

#ifdef WIN32
    m_startupDefaultDirectory = QDir::homePath();
#else
    m_startupDefaultDirectory = QDir::currentPath();
#endif

    setLastUsedDialogDirectory( "MULTICASEIMPORT", "/" );

    m_commandRouter = std::make_unique<RimCommandRouter>();
    m_osduConnector = nullptr;
    m_sumoConnector = nullptr;

    m_keyValueStore = std::make_unique<RiaKeyValueStore<char>>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaApplication::~RiaApplication()
{
    RiaFontCache::clear();

    // Shutdown CAF logging bridge
    RiaCafLoggingManager::shutdownCafLogging();

    caf::SelectionManager::instance()->setPdmRootObject( nullptr );

    m_project.reset();

    delete m_osduConnector.data();
    m_osduConnector.clear();
    m_osduConnector = nullptr;

    delete m_sumoConnector.data();
    m_sumoConnector.clear();
    m_sumoConnector = nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const char* RiaApplication::getVersionStringApp( bool includeCrtInfo )
{
    // Use static buf so we can return ptr
    static char szBuf[1024];

    cvf::String crtInfo;
    if ( includeCrtInfo )
    {
#ifdef _MT
#ifdef _DLL
        crtInfo = " (DLL CRT)";
#else
        crtInfo = " (static CRT)";
#endif
#endif //_MT
    }

    cvf::System::sprintf( szBuf, 1024, "%s%s", STRPRODUCTVER, crtInfo.toAscii().ptr() );

    return szBuf;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaApplication::enableDevelopmentFeatures()
{
    static int envValue = -999;
    if ( envValue == -999 )
    {
        QString environmentVar = QProcessEnvironment::systemEnvironment().value( "RESINSIGHT_DEVEL", QString( "0" ) );
        envValue               = environmentVar.toInt();
    }

    return envValue == 1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaApplication::createMockModel()
{
    RiaImportEclipseCaseTools::openMockModel( RiaDefines::mockModelBasic() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaApplication::createResultsMockModel()
{
    RiaImportEclipseCaseTools::openMockModel( RiaDefines::mockModelBasicWithResults() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaApplication::createLargeResultsMockModel()
{
    RiaImportEclipseCaseTools::openMockModel( RiaDefines::mockModelLargeWithResults() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaApplication::createMockModelCustomized()
{
    RiaImportEclipseCaseTools::openMockModel( RiaDefines::mockModelCustomized() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaApplication::createInputMockModel()
{
    bool createView = true;
    RiaImportEclipseCaseTools::openEclipseInputCaseAndPropertiesFromFileNames( QStringList( RiaDefines::mockModelBasicInputCase() ),
                                                                               createView );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaApplication::setActiveReservoirView( Rim3dView* rv )
{
    m_activeReservoirView = rv;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const Rim3dView* RiaApplication::activeReservoirView() const
{
    return m_activeReservoirView;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Rim3dView* RiaApplication::activeReservoirView()
{
    return m_activeReservoirView;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGridView* RiaApplication::activeGridView()
{
    return dynamic_cast<RimGridView*>( m_activeReservoirView.p() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGridView* RiaApplication::activeMainOrComparisonGridView()
{
    RimGridView* activeView           = RiaApplication::instance()->activeGridView();
    RimGridView* viewOrComparisonView = activeView;

    if ( activeView != nullptr && activeView->viewer() && activeView->viewer()->viewerCommands()->isCurrentPickInComparisonView() )
    {
        if ( RimGridView* compView = dynamic_cast<RimGridView*>( activeView->activeComparisonView() ) )
        {
            viewOrComparisonView = compView;
        }
    }

    return viewOrComparisonView;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiaApplication::currentScriptCaseId() const
{
    return m_currentScriptCaseId;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimProject* RiaApplication::project()
{
    return m_project.get();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCommandRouter* RiaApplication::commandRouter()
{
    return m_commandRouter.get();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaApplication::setThreadCount() const
{
    std::optional<int> threadCount = std::nullopt;
    QString            source;

    if ( m_threadCountFromCommandLine.has_value() )
    {
        threadCount = m_threadCountFromCommandLine.value();
        source      = "Command Line";
    }
    else
    {
        auto threadCountPrefs = RiaPreferencesSystem::current()->threadCount();
        if ( threadCountPrefs.has_value() )
        {
            threadCount = threadCountPrefs.value();
            source      = "Preferences";
        }
    }

    if ( threadCount.has_value() )
    {
        RiaLogging::info( QString( "Setting number of threads to %1 from %2" ).arg( threadCount.value() ).arg( source ) );

        RiaOpenMPTools::setMaxThreads( threadCount.value() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaApplication::openFile( const QString& fileName )
{
    if ( !caf::Utils::fileExists( fileName ) ) return false;

    bool loadingSucceded = false;

    QString lastUsedDialogTag;

    RiaDefines::ImportFileType fileType = RiaDefines::obtainFileTypeFromFileName( fileName );

    if ( fileType == RiaDefines::ImportFileType::RESINSIGHT_PROJECT_FILE )
    {
        loadingSucceded = loadProject( fileName );
    }
    else if ( RiaDefines::isGeoMechFileType( fileType ) )
    {
        loadingSucceded   = openOdbCaseFromFile( fileName );
        lastUsedDialogTag = "GEOMECH_MODEL";
    }
    else if ( RiaDefines::isEclipseFileType( fileType ) )
    {
        bool createView   = true;
        bool createPlot   = true;
        loadingSucceded   = RicImportGeneralDataFeature::openEclipseFilesFromFileNames( QStringList{ fileName }, createPlot, createView );
        lastUsedDialogTag = RiaDefines::defaultDirectoryLabel( fileType );
    }

    if ( loadingSucceded )
    {
        if ( !lastUsedDialogTag.isEmpty() )
        {
            RiaApplication::instance()->setLastUsedDialogDirectory( lastUsedDialogTag, QFileInfo( fileName ).absolutePath() );
        }

        onFileSuccessfullyLoaded( fileName, fileType );
    }

    return loadingSucceded;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaApplication::isProjectSavedToDisc() const
{
    if ( !m_project ) return false;

    return caf::Utils::fileExists( m_project->fileName() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaApplication::currentProjectPath() const
{
    QString projectFolder;
    if ( m_project )
    {
        QString projectFileName = m_project->fileName();

        if ( !projectFileName.isEmpty() )
        {
            QFileInfo fi( projectFileName );
            projectFolder = fi.absolutePath();
        }
    }

    return projectFolder;
}

//--------------------------------------------------------------------------------------------------
/// Create an absolute path from a path that is specified relative to the project directory
///
/// If the path specified in \a projectRelativePath is already absolute, no changes will be made
//--------------------------------------------------------------------------------------------------
QString RiaApplication::createAbsolutePathFromProjectRelativePath( QString projectRelativePath )
{
    // Check if path is already absolute
    if ( QDir::isAbsolutePath( projectRelativePath ) )
    {
        return projectRelativePath;
    }

    QString absolutePath;
    if ( m_project && !m_project->fileName().isEmpty() )
    {
        absolutePath = QFileInfo( m_project->fileName() ).absolutePath();
    }
    else
    {
        absolutePath = lastUsedDialogDirectory( "BINARY_GRID" );
    }

    QDir projectDir( absolutePath );

    return projectDir.absoluteFilePath( projectRelativePath );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaApplication::loadProject( const QString& projectFileName, ProjectLoadAction loadAction, RiaProjectModifier* projectModifier )
{
    auto startTime = RiaLogging::currentTime();

    // First Close the current project

    caf::ProgressInfo progress( 100, "Loading Project File" );

    bool logTiming = RiaPreferencesSystem::current()->isLoggingActivatedForKeyword( "RiaApplication" );

    {
        QString taskName = QString( "Reading Project Structure from File" );
        auto    task     = progress.task( taskName, 10 );

        closeProject();

        onProjectBeingOpened();

        RiaLogging::info( QString( "Starting to open project file : '%1'" ).arg( projectFileName ) );

        // Create a absolute path file name, as this is required for update of file references in the project modifier object
        QString fullPathProjectFileName = caf::Utils::absoluteFileName( projectFileName );
        if ( !caf::Utils::fileExists( fullPathProjectFileName ) )
        {
            RiaLogging::info( QString( "File does not exist : '%1'" ).arg( fullPathProjectFileName ) );
            return false;
        }

        m_project->setFileName( fullPathProjectFileName );
        std::vector<QString> deprecationMessages = m_project->readFile( defaultDeprecations() );
        for ( const QString& deprecationMessage : deprecationMessages )
        {
            RiaLogging::info( deprecationMessage );
        }

        m_project->updatesAfterProjectFileIsRead();

        // Apply any modifications to the loaded project before we go ahead and load actual data
        if ( projectModifier )
        {
            projectModifier->applyModificationsToProject( m_project.get() );
        }

        // Propagate possible new location of project

        m_project->setProjectFileNameAndUpdateDependencies( fullPathProjectFileName );

        // On error, delete everything, and bail out.

        if ( m_project->projectFileVersionString().isEmpty() )
        {
            closeProject();

            QString errMsg =
                QString( "Unknown project file version detected in file \n%1\n\nCould not open project." ).arg( fullPathProjectFileName );

            onProjectOpeningError( errMsg );

            // Delete all object possibly generated by readFile()
            m_project = std::make_unique<RimProject>();

            onProjectOpened();

            return true;
        }

        // At this point, all the file paths variables are replaced and all file paths updated to the new location. This will enable use of
        // file paths in initAfterRead().
        // If Rim-objects are created in initAfterRead(), they will be resolved in the second pass
        // Example: Realization objects in ensemble file set
        m_project->resolveReferencesRecursively();
        m_project->initAfterReadRecursively();

        if ( RimProject::current()->isProjectFileVersionEqualOrOlderThan( "2024.09.2" ) )
        {
            // Traverse objects recursively and add quick access fields for old projects
            RimQuickAccessCollection::instance()->addQuickAccessFieldsRecursively( m_project.get() );
        }

        // Migrate all RimGridCases to RimFileSummaryCase
        RimGridSummaryCase_obsolete::convertGridCasesToSummaryFileCases( m_project.get() );

        ///////
        // Load the external data, and initialize stuff that needs specific ordering

        // VL check regarding specific order mentioned in comment above...

        m_preferences->lastUsedProjectFileName = fullPathProjectFileName;
        if ( m_preferencesFileName.isEmpty() )
        {
            m_preferences->writePreferencesToApplicationStore();
        }

        if ( logTiming ) RiaLogging::logElapsedTime( taskName, startTime );
    }

    {
        QString taskName = QString( "Loading Grid Data" );
        auto    task     = progress.task( taskName, 10 );

        for ( size_t oilFieldIdx = 0; oilFieldIdx < m_project->oilFields().size(); oilFieldIdx++ )
        {
            RimOilField*              oilField       = m_project->oilFields[oilFieldIdx];
            RimEclipseCaseCollection* analysisModels = oilField ? oilField->analysisModels() : nullptr;
            if ( analysisModels == nullptr ) continue;

            for ( size_t cgIdx = 0; cgIdx < analysisModels->caseGroups.size(); ++cgIdx )
            {
                // Load the Main case of each IdenticalGridCaseGroup
                RimIdenticalGridCaseGroup* igcg = analysisModels->caseGroups[cgIdx];
                igcg->loadMainCaseAndActiveCellInfo(); // VL is this supposed to be done for each RimOilField?
            }
        }

        // Load the formation names

        for ( RimOilField* oilField : m_project->oilFields )
        {
            if ( oilField == nullptr ) continue;
            if ( oilField->formationNamesCollection() != nullptr )
            {
                oilField->formationNamesCollection()->readAllFormationNames();
            }
        }

        for ( size_t oilFieldIdx = 0; oilFieldIdx < m_project->oilFields().size(); oilFieldIdx++ )
        {
            RimOilField* oilField = m_project->oilFields[oilFieldIdx];
            if ( oilField == nullptr ) continue;
            if ( oilField->wellPathCollection == nullptr )
            {
                oilField->wellPathCollection = std::make_unique<RimWellPathCollection>();
            }

            // Initialize well paths
            if ( !oilField->wellPathCollection->loadDataAndUpdate() )
            {
                // Opening well path was cancelled or failed: close project.
                closeProject();
                m_project = std::make_unique<RimProject>();
                onProjectOpened();
                return true;
            }

            oilField->ensembleWellLogsCollection->loadDataAndUpdate();
            oilField->vfpDataCollection->loadDataAndUpdate();

            // Initialize seismic data
            auto& seisDataColl = oilField->seismicDataCollection();
            for ( auto seismicData : seisDataColl->seismicData() )
            {
                seismicData->ensureFileReaderIsInitialized();
            }

            oilField->polygonCollection()->loadData();
        }

        if ( logTiming ) RiaLogging::logElapsedTime( taskName, startTime );
    }

    {
        QString taskName = QString( "Loading Data for 2D Plots" );
        auto    task     = progress.task( taskName, 10 );

        {
            RimMainPlotCollection* mainPlotColl = RimMainPlotCollection::current();
            mainPlotColl->ensureDefaultFlowPlotsAreCreated();

            // RimVfpTable are not persisted in the project file, and are created in vfpDataCollection->loadDataAndUpdate(). Existing VFP
            // plots will have references to RimVfpTables. Call resolveReferencesRecursively() to update the references to RimVfpTable objects.
            mainPlotColl->vfpPlotCollection()->resolveReferencesRecursively();
        }

        for ( RimOilField* oilField : m_project->oilFields )
        {
            if ( oilField == nullptr ) continue;
            // Temporary
            if ( !oilField->summaryCaseMainCollection() )
            {
                oilField->summaryCaseMainCollection = std::make_unique<RimSummaryCaseMainCollection>();
            }
            oilField->summaryCaseMainCollection()->loadAllSummaryCaseData();

            m_project->calculationCollection()->rebuildCaseMetaData();

            if ( !oilField->observedDataCollection() )
            {
                oilField->observedDataCollection = std::make_unique<RimObservedDataCollection>();
            }
            for ( RimObservedSummaryData* observedData : oilField->observedDataCollection()->allObservedSummaryData() )
            {
                observedData->createSummaryReaderInterface();
                observedData->createRftReaderInterface();
                observedData->updateMetaData();
            }
            for ( RimObservedFmuRftData* observedFmuData : oilField->observedDataCollection()->allObservedFmuRftData() )
            {
                observedFmuData->createRftReaderInterface();
            }

            oilField->completionTemplateCollection()->loadAndUpdateData();
            oilField->fractureDefinitionCollection()->createAndAssignTemplateCopyForNonMatchingUnit();

            {
                std::vector<RimWellPathFracture*> wellPathFractures =
                    oilField->wellPathCollection->descendantsIncludingThisOfType<RimWellPathFracture>();

                for ( auto fracture : wellPathFractures )
                {
                    fracture->loadDataAndUpdate();
                }
            }

            oilField->surfaceCollection()->loadData();
        }

        if ( logTiming ) RiaLogging::logElapsedTime( taskName, startTime );
    }

    {
        QString taskName = QString( "Calculation Statistics for Grid Case Groups" );
        auto    task     = progress.task( taskName, 10 );

        // If load action is specified to recalculate statistics, do it now.
        // Apparently this needs to be done before the views are loaded, lest the number of time steps for statistics will
        // be clamped
        if ( loadAction == ProjectLoadAction::PLA_CALCULATE_STATISTICS )
        {
            for ( size_t oilFieldIdx = 0; oilFieldIdx < m_project->oilFields().size(); oilFieldIdx++ )
            {
                RimOilField*              oilField       = m_project->oilFields[oilFieldIdx];
                RimEclipseCaseCollection* analysisModels = oilField ? oilField->analysisModels() : nullptr;
                if ( analysisModels )
                {
                    analysisModels->recomputeStatisticsForAllCaseGroups();
                }
            }
        }

        if ( logTiming ) RiaLogging::logElapsedTime( taskName, startTime );
    }

    {
        QString taskName = QString( "Loading 3D Views" );
        auto    task     = progress.task( taskName, 10 );

        // Now load the ReservoirViews for the cases
        // Add all "native" cases in the project
        std::vector<RimCase*> casesToLoad = m_project->allGridCases();
        {
            caf::ProgressInfo caseProgress( casesToLoad.size(), "Reading Cases" );

            for ( size_t cIdx = 0; cIdx < casesToLoad.size(); ++cIdx )
            {
                RimCase* cas = casesToLoad[cIdx];
                CVF_ASSERT( cas );

                // Make sure case name is updated. It can be out-of-sync if the
                // user has updated the project file manually.
                cas->updateAutoShortName();

                caseProgress.setProgressDescription( cas->caseUserDescription() );
                std::vector<Rim3dView*> views = cas->views();
                { // To delete the view progress before incrementing the caseProgress
                    caf::ProgressInfo viewProgress( views.size(), "Creating Views" );

                    size_t j;
                    for ( j = 0; j < views.size(); j++ )
                    {
                        Rim3dView* riv = views[j];
                        CVF_ASSERT( riv );

                        viewProgress.setProgressDescription( riv->name() );

                        if ( m_project->isProjectFileVersionEqualOrOlderThan( "2018.1.0.103" ) )
                        {
                            std::vector<RimStimPlanColors*> stimPlanColors = riv->descendantsIncludingThisOfType<RimStimPlanColors>();
                            if ( stimPlanColors.size() == 1 )
                            {
                                stimPlanColors[0]->updateConductivityResultName();
                            }
                        }

                        riv->loadDataAndUpdate();

                        if ( m_project->isProjectFileVersionEqualOrOlderThan( "2018.1.1.110" ) )
                        {
                            auto* geoView = dynamic_cast<RimGeoMechView*>( riv );
                            if ( geoView )
                            {
                                geoView->convertCameraPositionFromOldProjectFiles();
                            }
                        }

                        if ( riv->showWindow() )
                        {
                            setActiveReservoirView( riv );
                        }

                        RimGridView* rigv = dynamic_cast<RimGridView*>( riv );
                        if ( rigv ) rigv->cellFilterCollection()->updateIconState();

                        viewProgress.incrementProgress();
                    }
                }
                caseProgress.incrementProgress();
            }
        }

        // Load all grid ensemble views
        {
            auto gridCaseEnsembles = m_project->activeOilField()->analysisModels()->caseEnsembles.childrenByType();

            for ( auto gridCaseEnsemble : gridCaseEnsembles )
            {
                auto views = gridCaseEnsemble->viewCollection()->views();
                for ( auto view : views )
                {
                    view->loadDataAndUpdate();
                }
            }
        }

        if ( m_project->viewLinkerCollection() && m_project->viewLinkerCollection()->viewLinker() )
        {
            m_project->viewLinkerCollection()->viewLinker()->updateOverrides();
        }

        // Intersection Views: Sync from intersections in the case.

        for ( RimCase* cas : casesToLoad )
        {
            cas->intersectionViewCollection()->syncFromExistingIntersections( false );
        }

        {
            for ( auto view : m_project->allViews() )
            {
                if ( auto eclipseView = dynamic_cast<RimEclipseView*>( view ) )
                {
                    eclipseView->faultReactivationModelCollection()->loadDataAndUpdate();
                }
            }
        }

        for ( RimOilField* oilField : m_project->oilFields )
        {
            for ( auto seisView : oilField->seismicViewCollection()->views() )
            {
                seisView->loadDataAndUpdate();
            }
        }

        if ( logTiming ) RiaLogging::logElapsedTime( taskName, startTime );
    }

    {
        const int taskSteps = 30;

        // Init summary case groups
        for ( RimOilField* oilField : m_project->oilFields )
        {
            auto sumMainCollection = oilField->summaryCaseMainCollection();
            if ( !sumMainCollection ) continue;

            sumMainCollection->updateAutoShortName();

            int stepSize = 1;
            if ( !sumMainCollection->summaryEnsembles().empty() )
            {
                stepSize = std::max( 1, int( taskSteps / sumMainCollection->summaryEnsembles().size() ) );
            }

            for ( auto ensemble : sumMainCollection->summaryEnsembles() )
            {
                auto ensembleTask = progress.task( QString( "Loading Summary Ensemble '%1'" ).arg( ensemble->name() ), stepSize );
                ensemble->loadDataAndUpdate();
                RiaLogging::info( QString( "Loaded ensemble : '%1'" ).arg( ensemble->name() ) );
            }
            sumMainCollection->updateEnsembleNames();

            for ( auto well : oilField->wellPathCollection()->allWellPaths() )
            {
                for ( auto stimPlan : well->stimPlanModelCollection()->allStimPlanModels() )
                    stimPlan->resetAnchorPositionAndThicknessDirection();
            }
        }

        // Second pass: a full recursive resolution to robustly handle both previously failing references and any newly created ones from
        // initAfterRead.
        m_project->resolveReferencesRecursively();

        // Some procedures in onProjectOpened() may rely on the display model having been created
        // So we need to force the completion of the display model here.
        RiaViewRedrawScheduler::instance()->updateAndRedrawScheduledViews();

        // NB! This function must be called before executing command objects,
        // because the tree view state is restored from project file and sets
        // current active view ( see restoreTreeViewState() )
        // Default behavior for scripts is to use current active view for data read/write
        onProjectOpened();

        if ( logTiming ) RiaLogging::logElapsedTime( "Load Summary Data", startTime );
    }

    {
        QString taskName = QString( "Grid Cells Calculations" );
        auto    task     = progress.task( taskName, 10 );

        // Recalculate the results from grid property calculations.
        // Has to be done late since the results are filtered by view cell visibility
        for ( auto gridCalculation : m_project->gridCalculationCollection()->sortedGridCalculations() )
        {
            gridCalculation->calculate();
            gridCalculation->updateDependentObjects();
        }

        RiaPlotWindowRedrawScheduler::instance()->performScheduledUpdates();

        if ( logTiming ) RiaLogging::logElapsedTime( taskName, startTime );
    }

    auto logText = QString( "Project file '%1' loaded successfully." ).arg( projectFileName );
    if ( logTiming )
    {
        RiaLogging::logElapsedTime( logText, startTime );
    }
    else
    {
        RiaLogging::info( logText );
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaApplication::loadProject( const QString& projectFileName )
{
    return loadProject( projectFileName, ProjectLoadAction::PLA_NONE, nullptr );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaApplication::saveProject( gsl::not_null<QString*> errorMessage )
{
    CAF_ASSERT( m_project );

    if ( !isProjectSavedToDisc() )
    {
        *errorMessage = "Project hasn't already been saved and no file name has been provided";
        return false;
    }
    else
    {
        return saveProjectAs( m_project->fileName(), errorMessage );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaApplication::saveProjectAs( const QString& fileName, gsl::not_null<QString*> errorMessage )
{
    CAF_ASSERT( m_project );
    // Make sure we always store path with forward slash to avoid issues when opening the project file on Linux
    m_project->setFileName( RiaFilePathTools::toInternalSeparator( fileName ) );

    onProjectBeingSaved();

    if ( !m_project->writeProjectFile() )
    {
        *errorMessage = QString( "Not possible to save project file. Make sure you have sufficient access "
                                 "rights.\n\nProject file location : %1" )
                            .arg( fileName );
        return false;
    }

    m_preferences->lastUsedProjectFileName = fileName;
    if ( m_preferencesFileName.isEmpty() )
    {
        m_preferences->writePreferencesToApplicationStore();
    }

    onProjectSaved();

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaApplication::hasValidProjectFileExtension( const QString& fileName )
{
    return fileName.contains( ".rsp", Qt::CaseInsensitive ) || fileName.contains( ".rip", Qt::CaseInsensitive );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaApplication::closeProject()
{
    onProjectBeingClosed();

    m_project->close();

    RiaWellNameComparer::clearCache();

    onProjectClosed();

    caf::PdmUiModelChangeDetector::instance()->reset();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaApplication::lastUsedDialogDirectory( const QString& dialogName )
{
    QString lastUsedDirectory = m_startupDefaultDirectory;

    auto it = m_fileDialogDefaultDirectories.find( dialogName );
    if ( it != m_fileDialogDefaultDirectories.end() )
    {
        lastUsedDirectory = it->second;
    }

    return lastUsedDirectory;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaApplication::lastUsedDialogDirectoryWithFallback( const QString& dialogName, const QString& fallbackDirectory )
{
    QString lastUsedDirectory = m_startupDefaultDirectory;
    if ( !fallbackDirectory.isEmpty() )
    {
        lastUsedDirectory = fallbackDirectory;
    }

    auto it = m_fileDialogDefaultDirectories.find( dialogName );
    if ( it != m_fileDialogDefaultDirectories.end() )
    {
        lastUsedDirectory = it->second;
    }

    return lastUsedDirectory;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaApplication::lastUsedDialogDirectoryWithFallbackToProjectFolder( const QString& dialogName )
{
    return lastUsedDialogDirectoryWithFallback( dialogName, currentProjectPath() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaApplication::setLastUsedDialogDirectory( const QString& dialogName, const QString& directory )
{
    m_fileDialogDefaultDirectories[dialogName] = directory;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaApplication::openOdbCaseFromFile( const QString& fileName, bool applyTimeStepFilter )
{
    if ( !caf::Utils::fileExists( fileName ) ) return false;

    QFileInfo gridFileName( fileName );
    QString   caseName = gridFileName.completeBaseName();

    if ( !m_project->activeOilField() ) return false;

    // Create the geoMech model container if it is not there already
    if ( !m_project->activeOilField()->geoMechModels )
    {
        m_project->activeOilField()->geoMechModels = std::make_unique<RimGeoMechModels>();
    }

    gsl::not_null<RimGeoMechModels*> geoMechModelCollection = m_project->activeOilField()->geoMechModels();

    // Check if the file is already open, the odb reader does not support opening the same file twice very well
    for ( auto gmcase : geoMechModelCollection->cases() )
    {
        if ( gmcase->gridFileName() == fileName )
        {
            RiaLogging::warning( "File has already been opened. Cannot open the file twice! - " + fileName );
            return false;
        }
    }

    auto geoMechCase = std::make_unique<RimGeoMechCase>();
    geoMechCase->setGridFileName( fileName );
    geoMechCase->setCaseUserDescription( caseName );
    geoMechCase->setApplyTimeFilter( applyTimeStepFilter );
    m_project->assignCaseIdToCase( geoMechCase.get() );

    RimGeoMechView*   riv = geoMechCase->createAndAddReservoirView();
    caf::ProgressInfo progress( 11, "Loading Case" );
    progress.setNextProgressIncrement( 10 );

    riv->loadDataAndUpdate();

    if ( !riv->geoMechCase() )
    {
        return false;
    }
    geoMechModelCollection->addCase( geoMechCase.release() );

    progress.incrementProgress();
    progress.setProgressDescription( "Loading results information" );

    m_project->updateConnectedEditors();
    Riu3DMainWindowTools::setExpanded( riv );
    Riu3DMainWindowTools::selectAsCurrentItem( riv );

    return true;
}

//--------------------------------------------------------------------------------------------------
/// Add a list of well path file paths (JSON files) to the well path collection
//--------------------------------------------------------------------------------------------------
std::vector<RimWellPath*> RiaApplication::addWellPathsToModel( QList<QString> wellPathFilePaths, gsl::not_null<QStringList*> errorMessages )
{
    if ( m_project == nullptr || m_project->oilFields.empty() ) return {};

    RimOilField* oilField = m_project->activeOilField();
    if ( oilField == nullptr ) return {};

    if ( oilField->wellPathCollection == nullptr )
    {
        // printf("Create well path collection.\n");
        oilField->wellPathCollection = std::make_unique<RimWellPathCollection>();

        m_project->updateConnectedEditors();
    }

    std::vector<RimWellPath*> wellPaths;
    if ( oilField->wellPathCollection )
    {
        wellPaths = oilField->wellPathCollection->addWellPaths( wellPathFilePaths, errorMessages );
    }

    oilField->wellPathCollection->updateConnectedEditors();

    return wellPaths;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaApplication::addWellPathFormationsToModel( QList<QString> wellPathFormationsFilePaths )
{
    if ( m_project == nullptr || m_project->oilFields.empty() ) return;

    RimOilField* oilField = m_project->activeOilField();
    if ( oilField == nullptr ) return;

    if ( oilField->wellPathCollection == nullptr )
    {
        oilField->wellPathCollection = std::make_unique<RimWellPathCollection>();

        m_project->updateConnectedEditors();
    }

    if ( oilField->wellPathCollection )
    {
        oilField->wellPathCollection->addWellPathFormations( wellPathFormationsFilePaths );
    }

    oilField->wellPathCollection->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
/// Add a list of well log file paths (LAS files) to the well path collection
//--------------------------------------------------------------------------------------------------
std::vector<RimWellLogLasFile*> RiaApplication::addWellLogsToModel( const QList<QString>&       wellLogFilePaths,
                                                                    gsl::not_null<QStringList*> errorMessages )
{
    if ( m_project == nullptr || m_project->oilFields.empty() ) return {};

    RimOilField* oilField = m_project->activeOilField();
    if ( oilField == nullptr ) return {};

    if ( !oilField->wellPathCollection )
    {
        oilField->wellPathCollection = std::make_unique<RimWellPathCollection>();

        m_project->updateConnectedEditors();
    }

    std::vector<RimWellLogLasFile*> wellLogFiles = oilField->wellPathCollection->addWellLogs( wellLogFilePaths, errorMessages );

    oilField->wellPathCollection->updateConnectedEditors();

    return wellLogFiles;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaApplication::scriptDirectories() const
{
    return m_preferences->scriptDirectories();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaApplication::scriptEditorPath() const
{
    return m_preferences->scriptEditorExecutable();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaApplication::octavePath() const
{
    return m_preferences->octaveExecutable();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RiaApplication::octaveArguments() const
{
    // http://www.gnu.org/software/octave/doc/interpreter/Command-Line-Options.html#Command-Line-Options

    // -p path
    // Add path to the head of the search path for function files. The value of path specified on the command line
    // will override any value of OCTAVE_PATH found in the environment, but not any commands in the system or
    // user startup files that set the internal load path through one of the path functions.

    QStringList arguments;
    arguments.append( "--path" );
    arguments << QCoreApplication::applicationDirPath();

    if ( !m_preferences->octaveShowHeaderInfoWhenExecutingScripts() )
    {
        // -q
        // Don't print the usual greeting and version message at startup.

        arguments.append( "-q" );
    }

    return arguments;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QProcessEnvironment RiaApplication::octaveProcessEnvironment() const
{
    QProcessEnvironment penv = QProcessEnvironment::systemEnvironment();

#ifdef WIN32
    // Octave plugins compiled by ResInsight are dependent on Qt (currently Qt 32-bit only)
    // Some Octave installations for Windows have included Qt, and some don't. To make sure these plugins always can be
    // executed, the path to octave_plugin_dependencies is added to global path

    QString pathString = penv.value( "PATH", "" );

    if ( pathString == "" )
        pathString = QCoreApplication::applicationDirPath() + "\\octave_plugin_dependencies";
    else
        pathString = QCoreApplication::applicationDirPath() + "\\octave_plugin_dependencies" + ";" + pathString;

    penv.insert( "PATH", pathString );
#else
    // Set the LD_LIBRARY_PATH to make the octave plugins find the embedded Qt
    QString ldPath = penv.value( "LD_LIBRARY_PATH", "" );

    if ( ldPath == "" )
        ldPath = QCoreApplication::applicationDirPath();
    else
        ldPath = QCoreApplication::applicationDirPath() + ":" + ldPath;

    penv.insert( "LD_LIBRARY_PATH", ldPath );
#endif

    // Set the environment variable for the port number used by Octave plugins
    // The plugins can access the port number using riOctavePlugin::portNumber()
    // If the port number is not set in preferences, the plugins will use the default port number
    const QString key = QString::fromStdString( riOctavePlugin::portNumberKey() );
    if ( !m_preferences->octavePortNumber().isEmpty() )
    {
        penv.insert( key, m_preferences->octavePortNumber() );
    }
    else
    {
        penv.remove( key );
    }

    return penv;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaApplication::pythonPath() const
{
    return m_preferences->pythonExecutable();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QProcessEnvironment RiaApplication::pythonProcessEnvironment() const
{
    return QProcessEnvironment::systemEnvironment();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaApplication::launchProcess( const QString& program, const QStringList& arguments, const QProcessEnvironment& processEnvironment )
{
    if ( m_workerProcess == nullptr )
    {
        // If multiple cases are present, pop the first case ID from the list and set as current case
        if ( !m_scriptCaseIds.empty() )
        {
            int nextCaseId = m_scriptCaseIds.front();
            m_scriptCaseIds.pop_front();

            m_currentScriptCaseId = nextCaseId;
        }
        else
        {
            // Disable current case concept
            m_currentScriptCaseId = -1;
        }

        m_runningWorkerProcess = true;
        m_workerProcess        = std::make_unique<caf::UiProcess>( QCoreApplication::instance() );

        m_workerProcess->setProcessEnvironment( processEnvironment );

        QCoreApplication::instance()->connect( m_workerProcess.get(),
                                               SIGNAL( finished( int, QProcess::ExitStatus ) ),
                                               SLOT( slotWorkerProcessFinished( int, QProcess::ExitStatus ) ) );

        startMonitoringWorkProgress( m_workerProcess.get() );

        m_workerProcess->start( program, arguments );

        // The wait time is a compromise between large wait time when processing many octave runs after each other and
        // short wait time when starting octave processes interactively
        int waitTimeMilliseconds = 7 * 1000;
        if ( !m_workerProcess->waitForStarted( waitTimeMilliseconds ) )
        {
            m_workerProcess->close();
            m_workerProcess        = nullptr;
            m_runningWorkerProcess = false;

            stopMonitoringWorkProgress();

            return false;
        }

        return true;
    }
    else
    {
        return false;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaApplication::launchProcessForMultipleCases( const QString&             program,
                                                    const QStringList&         arguments,
                                                    const std::vector<int>&    caseIds,
                                                    const QProcessEnvironment& processEnvironment )
{
    m_scriptCaseIds.clear();
    std::copy( caseIds.begin(), caseIds.end(), std::back_inserter( m_scriptCaseIds ) );

    m_currentProgram   = program;
    m_currentArguments = arguments;

    return launchProcess( m_currentProgram, m_currentArguments, processEnvironment );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaApplication::terminateProcess()
{
    if ( m_workerProcess )
    {
        m_workerProcess->kill();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaApplication::waitForProcess() const
{
    while ( m_runningWorkerProcess )
    {
#ifdef WIN32
        Sleep( 100 );
#else
        usleep( 100000 );
#endif
        QCoreApplication::processEvents();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaPreferences* RiaApplication::preferences()
{
    return m_preferences.get();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaApplication::applyPreferences()
{
    // The creation of a font is time consuming, so make sure you really need your own font
    // instead of using the application font
    std::map<RiaDefines::FontSettingType, RiaFontCache::FontSize> fontSizes = m_preferences->defaultFontSizes();

    m_defaultSceneFont      = RiaFontCache::getFont( fontSizes[RiaDefines::FontSettingType::SCENE_FONT] );
    m_defaultAnnotationFont = RiaFontCache::getFont( fontSizes[RiaDefines::FontSettingType::ANNOTATION_FONT] );
    m_defaultWellLabelFont  = RiaFontCache::getFont( fontSizes[RiaDefines::FontSettingType::WELL_LABEL_FONT] );

    if ( project() )
    {
        project()->setScriptDirectories( m_preferences->scriptDirectories(), m_preferences->maxScriptFoldersDepth() );
        project()->setPlotTemplateFolders( m_preferences->plotTemplateFolders() );

        project()->scriptCollection()->updateConnectedEditors();
        project()->rootPlotTemplateItem()->updateConnectedEditors();
    }

    caf::ProgressInfoStatic::setEnabled( RiaPreferencesSystem::current()->showProgressBar() );

    // Reinitialize OpenTelemetry based on current preferences
    RiaOpenTelemetryManager::instance().reinitialize();

    if ( m_preferencesFileName.isEmpty() )
    {
        m_preferences->writePreferencesToApplicationStore();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaApplication::commandLineParameterHelp()
{
    QString helpText = QString( "\n%1 v. %2\n" ).arg( RI_APPLICATION_NAME ).arg( RiaApplication::getVersionStringApp( false ) );
    helpText += "Copyright Equinor ASA, Ceetron Solution AS, Ceetron AS\n\n";
    helpText += m_commandLineHelpText;

    return helpText;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaApplication::setCacheDataObject( const QString& key, const QVariant& dataObject )
{
    m_sessionCache[key] = dataObject;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QVariant RiaApplication::cacheDataObject( const QString& key ) const
{
    QMap<QString, QVariant>::const_iterator it = m_sessionCache.find( key );

    if ( it != m_sessionCache.end() )
    {
        return it.value();
    }

    return QVariant();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaApplication::executeCommandFile( const QString& commandFile )
{
    QFile                    file( commandFile );
    caf::PdmScriptIOMessages messages;
    if ( !file.open( QIODevice::ReadOnly | QIODevice::Text ) )
    {
        // TODO : Error logging?
        return;
    }

    QTextStream in( &file );
    RicfCommandFileExecutor::instance()->executeCommands( in );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QString RiaApplication::startDir() const
{
    return m_startupDefaultDirectory;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaApplication::setStartDir( const QString& startDir )
{
    m_startupDefaultDirectory = startDir;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaApplication::setCommandLineHelpText( const QString& commandLineHelpText )
{
    m_commandLineHelpText = commandLineHelpText;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> RiaApplication::readFileListFromTextFile( QString listFileName )
{
    std::vector<QString> fileList;

    QFile file( listFileName );
    if ( !file.open( QIODevice::ReadOnly | QIODevice::Text ) )
    {
        return fileList;
    }

    QTextStream in( &file );
    QString     line = in.readLine();
    while ( !line.isNull() )
    {
        line = line.trimmed();
        if ( !line.isEmpty() )
        {
            fileList.push_back( line );
        }

        line = in.readLine();
    }

    return fileList;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Font* RiaApplication::defaultSceneFont()
{
    CVF_ASSERT( m_defaultSceneFont.notNull() );

    // The creation of a font is time consuming, so make sure you really need your own font
    // instead of using the application font

    return m_defaultSceneFont.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Font* RiaApplication::sceneFont( int fontSize )
{
    if ( fontSize != caf::FontTools::absolutePointSize( m_preferences->defaultSceneFontSize() ) )
    {
        auto font = RiaFontCache::getFont( fontSize );
        return font.p();
    }
    return defaultSceneFont();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Font* RiaApplication::defaultAnnotationFont()
{
    CVF_ASSERT( m_defaultAnnotationFont.notNull() );

    return m_defaultAnnotationFont.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Font* RiaApplication::defaultWellLabelFont()
{
    CVF_ASSERT( m_defaultWellLabelFont.notNull() );

    return m_defaultWellLabelFont.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaApplication::initialize()
{
    m_preferences = std::make_unique<RiaPreferences>();

    if ( !m_preferencesFileName.isEmpty() )
    {
        // The text file with preference values has priority above reading from user settings
        m_preferences->importPreferenceValuesFromFile( m_preferencesFileName );
    }
    else
    {
        caf::PdmSettings::readFieldsFromApplicationStore( m_preferences.get() );
    }

    m_preferences->initAfterReadRecursively();
    applyPreferences();

    // Start with a project
    m_project = std::make_unique<RimProject>();
    m_project->setScriptDirectories( m_preferences->scriptDirectories(), m_preferences->maxScriptFoldersDepth() );
    m_project->setPlotTemplateFolders( m_preferences->plotTemplateFolders() );

    caf::SelectionManager::instance()->setPdmRootObject( project() );

    // Initialize CAF logging bridge to forward CAF framework messages to ResInsight logging
    RiaCafLoggingManager::initializeCafLogging();

    initializeDataLoadController();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaApplication::loadAndUpdatePlotData()
{
    RimMainPlotCollection::current()->loadDataAndUpdateAllPlots();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaApplication::resetProject()
{
    m_project.reset();
    m_preferences.reset();

    // Call RiaApplication::initialize() to recreate project and preferences. Do not call virtual method initialize(),
    // as RiaGuiApplication::initialize() creates a new logger causing console text to disappear.
    RiaApplication::initialize();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaApplication::generateCode( const QString& fileName, gsl::not_null<QString*> errMsg )
{
    std::string fileExt = QFileInfo( fileName ).suffix().toStdString();

    {
        // TODO: Manually instantiate the markdown generator until cmake issues are fixed
        // This will make sure the markdown generator is registered in the factory in the cafPdmScripting library
        caf::PdmMarkdownGenerator testObj;
    }

    std::unique_ptr<caf::PdmCodeGenerator> generator( caf::PdmCodeGeneratorFactory::instance()->create( fileExt ) );
    if ( !generator )
    {
        *errMsg = QString( "No code generator matches the provided file extension" );
        return false;
    }

    auto markdownGenerator = dynamic_cast<caf::PdmMarkdownGenerator*>( generator.get() );
    if ( markdownGenerator )
    {
        QFileInfo fi( fileName );
        QDir      dir( fi.absoluteDir() );

        QString baseName = fi.baseName();

        {
            QString outputFileName = dir.absoluteFilePath( baseName + "_class.md" );

            QFile outputFile( outputFileName );
            if ( !outputFile.open( QIODevice::WriteOnly | QIODevice::Text ) )
            {
                *errMsg = QString( "Could not open file %1 for writing" ).arg( outputFileName );
                return false;
            }
            QTextStream out( &outputFile );

            {
                out << "+++ \n";
                out << "title =  \"Python Classes (BETA)\" \n";
                out << "published = true \n";
                out << "weight = 95 \n";
                out << "+++ \n";

                out << "# Introduction\n\n";
                out << "As the Python interface is growing release by release, we are investigating how to automate "
                       "the building of documentation. This document shows the inheritance relationship between "
                       "objects derived from **PdmObject**. The **PdmObject** is the base object for all "
                       "objects automatically created based on the data model in ResInsight.";
            }

            std::vector<QString> logMessages;

            out << generator->generate( caf::PdmDefaultObjectFactory::instance(), logMessages );
        }

        {
            QString outputFileName = dir.absoluteFilePath( baseName + "_commands.md" );

            QFile outputFile( outputFileName );
            if ( !outputFile.open( QIODevice::WriteOnly | QIODevice::Text ) )
            {
                *errMsg = QString( "Could not open file %1 for writing" ).arg( outputFileName );
                return false;
            }
            QTextStream out( &outputFile );

            {
                out << "As the Python interface is growing release by release, we are investigating how to automate "
                       "the building of reference documentation. This document is not complete, "
                       "but will improve as the automation moves forward.\n\n";

                out << "More details on the command file operations, see "
                       "https://resinsight.org/scripting/commandfile/\n\n";

                // TODO
                //                 out << "## Currently missing features\n\n";
                //                 out << " - Description of enums\n";
                //                 out << " - Description of return values/classes\n";
                //                 out << " - Description of each object\n\n";
            }

            std::vector<std::shared_ptr<const caf::PdmObject>> commandObjects;

            QStringList excludedClassNames{ "TestCommand1", "TC2" }; // See RifCommandCore-Text.cpp

            auto allObjects = caf::PdmMarkdownBuilder::createAllObjects( caf::PdmDefaultObjectFactory::instance() );
            for ( const auto& classObject : allObjects )
            {
                if ( dynamic_cast<const RicfCommandObject*>( classObject.get() ) )
                {
                    if ( !excludedClassNames.contains( classObject->classKeyword(), Qt::CaseInsensitive ) )
                    {
                        commandObjects.push_back( classObject );
                    }
                }
            }

            out << caf::PdmMarkdownBuilder::generateDocCommandObjects( commandObjects );
        }
    }
    else
    {
        QFile outputFile( fileName );
        if ( !outputFile.open( QIODevice::WriteOnly | QIODevice::Text ) )
        {
            *errMsg = QString( "Could not open file %1 for writing" ).arg( fileName );
            return false;
        }
        QTextStream out( &outputFile );

        std::vector<QString> logMessages;

        out << generator->generate( caf::PdmDefaultObjectFactory::instance(), logMessages );

        QString errorText;
        for ( const auto& msg : logMessages )
        {
            errorText += msg;
            errorText += "\n";
        }

        if ( !errorText.isEmpty() )
        {
            *errMsg = errorText;
            return false;
        }
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaOsduConnector* RiaApplication::makeOsduConnector()
{
    if ( m_osduConnector ) return m_osduConnector;

    if ( !QSslSocket::supportsSsl() )
    {
        QString errMsg = "SSL support is not available. ";
#ifdef Q_OS_WIN
        errMsg +=
            "Make sure that the SSL libraries are available (on Windows platform, they are called 'libcrypto*.dll' and 'libssl*.dll').";
#endif
        RiaLogging::errorInMessageBox( nullptr, "OSDU Service Connection", errMsg );

        return nullptr;
    }

    RiaPreferencesOsdu* osduPreferences = preferences()->osduPreferences();
    const QString       server          = osduPreferences->server();
    const QString       dataPartitionId = osduPreferences->dataPartitionId();
    const QString       authority       = osduPreferences->authority();
    const QString       scopes          = osduPreferences->scopes();
    const QString       clientId        = osduPreferences->clientId();
    const unsigned int  port            = 35327;

    m_osduConnector = new RiaOsduConnector( RiuMainWindow::instance(), server, dataPartitionId, authority, scopes, clientId, port );
    m_osduConnector->setTokenDataFilePath( RiaOsduDefines::tokenPath() );
    m_osduConnector->importTokenFromFile();

    return m_osduConnector;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaSumoConnector* RiaApplication::makeSumoConnector()
{
    if ( RiaRegressionTestRunner::instance()->isRunningRegressionTests() )
    {
        return nullptr;
    }

    if ( !m_sumoConnector )
    {
        auto               sumoPrefs = preferences()->sumoPreferences();
        const QString      server    = sumoPrefs->server();
        const QString      authority = sumoPrefs->authority();
        const QString      scopes    = sumoPrefs->scopes();
        const QString      clientId  = sumoPrefs->clientId();
        const unsigned int port      = 53527;

        m_sumoConnector = new RiaSumoConnector( RiuMainWindow::instance(), server, authority, scopes, clientId, port );
        m_sumoConnector->setTokenDataFilePath( RiaSumoDefines::tokenPath() );
        m_sumoConnector->importTokenFromFile();
    }

    return m_sumoConnector;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaApplication::initializeDataLoadController()
{
    caf::DataLoadController* dataLoadController = caf::DataLoadController::instance();

    const QString wellPathGeometryKeyword = "WELL_PATH_GEOMETRY";
    dataLoadController->registerDataLoader( RimFileWellPath::classKeywordStatic(),
                                            wellPathGeometryKeyword,
                                            std::make_unique<RimFileWellPathDataLoader>() );
    dataLoadController->registerDataLoader( RimOsduWellPath::classKeywordStatic(),
                                            wellPathGeometryKeyword,
                                            std::make_unique<RimOsduWellPathDataLoader>() );
    dataLoadController->registerDataLoader( RimModeledWellPath::classKeywordStatic(),
                                            wellPathGeometryKeyword,
                                            std::make_unique<RimModeledWellPathDataLoader>() );

    const QString wellLogKeyword = "WELL_LOG";
    dataLoadController->registerDataLoader( RimWellLogLasFile::classKeywordStatic(),
                                            wellLogKeyword,
                                            std::make_unique<RimWellLogFileDataLoader>() );
    dataLoadController->registerDataLoader( RimWellLogFile::classKeywordStatic(), wellLogKeyword, std::make_unique<RimWellLogFileDataLoader>() );
    dataLoadController->registerDataLoader( RimOsduWellLog::classKeywordStatic(), wellLogKeyword, std::make_unique<RimOsduWellLogDataLoader>() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaKeyValueStore<char>* RiaApplication::keyValueStore() const
{
    return m_keyValueStore.get();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<caf::PdmDeprecation> RiaApplication::defaultDeprecations()
{
    // Deprecations can be used for adding info messages when pdm fields have been removed.
    // To extend the concept to pdm objects more work is needed.
    return {};
}
