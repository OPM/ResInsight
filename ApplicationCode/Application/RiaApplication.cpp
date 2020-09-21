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

#include "RiaArgumentParser.h"
#include "RiaBaseDefs.h"
#include "RiaFilePathTools.h"
#include "RiaFontCache.h"
#include "RiaGuiApplication.h"
#include "RiaImportEclipseCaseTools.h"
#include "RiaLogging.h"
#include "RiaPreferences.h"
#include "RiaProjectModifier.h"
#include "RiaSocketServer.h"
#include "RiaTextStringTools.h"
#include "RiaVersionInfo.h"
#include "RiaViewRedrawScheduler.h"

#include "ExportCommands/RicSnapshotAllViewsToFileFeature.h"
#include "HoloLensCommands/RicHoloLensSessionManager.h"
#include "RicImportGeneralDataFeature.h"
#include "RicfCommandFileExecutor.h"
#include "RicfCommandObject.h"

#include "Rim2dIntersectionViewCollection.h"
#include "RimAnalysisPlot.h"
#include "RimAnalysisPlotCollection.h"
#include "RimAnnotationCollection.h"
#include "RimAnnotationInViewCollection.h"
#include "RimAnnotationTextAppearance.h"
#include "RimCellRangeFilterCollection.h"
#include "RimCommandObject.h"
#include "RimCorrelationPlot.h"
#include "RimCorrelationPlotCollection.h"
#include "RimCorrelationReportPlot.h"
#include "RimEclipseCaseCollection.h"
#include "RimEclipseView.h"
#include "RimFlowPlotCollection.h"
#include "RimFormationNamesCollection.h"
#include "RimFractureModel.h"
#include "RimFractureModelCollection.h"
#include "RimFractureModelPlot.h"
#include "RimFractureModelPlotCollection.h"
#include "RimFractureTemplateCollection.h"
#include "RimGeoMechCase.h"
#include "RimGeoMechCellColors.h"
#include "RimGeoMechModels.h"
#include "RimGeoMechView.h"
#include "RimGridCrossPlot.h"
#include "RimGridCrossPlotCollection.h"
#include "RimIdenticalGridCaseGroup.h"
#include "RimMainPlotCollection.h"
#include "RimMultiPlot.h"
#include "RimMultiPlotCollection.h"
#include "RimObservedDataCollection.h"
#include "RimObservedFmuRftData.h"
#include "RimObservedSummaryData.h"
#include "RimOilField.h"
#include "RimPlotWindow.h"
#include "RimPltPlotCollection.h"
#include "RimProject.h"
#include "RimRftPlotCollection.h"
#include "RimSaturationPressurePlot.h"
#include "RimSaturationPressurePlotCollection.h"
#include "RimSimWellInViewCollection.h"
#include "RimStimPlanColors.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseCollection.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSummaryCrossPlotCollection.h"
#include "RimSummaryPlot.h"
#include "RimSummaryPlotCollection.h"
#include "RimSurfaceCollection.h"
#include "RimTextAnnotation.h"
#include "RimTextAnnotationInView.h"
#include "RimViewLinker.h"
#include "RimViewLinkerCollection.h"
#include "RimWellLogFile.h"
#include "RimWellLogPlot.h"
#include "RimWellLogPlotCollection.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"
#include "RimWellPathFracture.h"
#include "RimWellPltPlot.h"
#include "RimWellRftPlot.h"

#include "Riu3DMainWindowTools.h"
#include "RiuViewer.h"
#include "RiuViewerCommands.h"

#include "cafPdmCodeGenerator.h"
#include "cafPdmDataValueField.h"
#include "cafPdmDefaultObjectFactory.h"
#include "cafPdmMarkdownBuilder.h"
#include "cafPdmMarkdownGenerator.h"
#include "cafPdmScriptIOMessages.h"
#include "cafPdmSettings.h"
#include "cafPdmUiModelChangeDetector.h"
#include "cafProgressInfo.h"
#include "cafUiProcess.h"
#include "cafUtils.h"

#include "cvfProgramOptions.h"
#include "cvfqtUtils.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>

#include <iostream>
#include <memory>

#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h> // for usleep
#endif // WIN32

#ifdef USE_UNIT_TESTS
#include "gtest/gtest.h"
#endif // USE_UNIT_TESTS

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
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaApplication::~RiaApplication()
{
    delete m_preferences;
    delete m_project;

    RiaFontCache::clear();
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
    QString environmentVar = QProcessEnvironment::systemEnvironment().value( "RESINSIGHT_DEVEL", QString( "0" ) );
    return environmentVar.toInt() == 1;
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
    RiaImportEclipseCaseTools::openEclipseInputCaseFromFileNames( QStringList( RiaDefines::mockModelBasicInputCase() ) );
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

    if ( activeView != nullptr && activeView->viewer() &&
         activeView->viewer()->viewerCommands()->isCurrentPickInComparisonView() )
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
RimProject* RiaApplication::project()
{
    return m_project;
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
    else if ( fileType == RiaDefines::ImportFileType::GEOMECH_ODB_FILE )
    {
        loadingSucceded   = openOdbCaseFromFile( fileName );
        lastUsedDialogTag = "GEOMECH_MODEL";
    }
    else if ( int( fileType ) & int( RiaDefines::ImportFileType::ANY_ECLIPSE_FILE ) )
    {
        loadingSucceded   = RicImportGeneralDataFeature::openEclipseFilesFromFileNames( QStringList{fileName}, true );
        lastUsedDialogTag = RiaDefines::defaultDirectoryLabel( fileType );
    }

    if ( loadingSucceded )
    {
        if ( !lastUsedDialogTag.isEmpty() )
        {
            RiaApplication::instance()->setLastUsedDialogDirectory( lastUsedDialogTag,
                                                                    QFileInfo( fileName ).absolutePath() );
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
    if ( m_project.isNull() ) return false;

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
        absolutePath = this->lastUsedDialogDirectory( "BINARY_GRID" );
    }

    QDir projectDir( absolutePath );

    return projectDir.absoluteFilePath( projectRelativePath );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaApplication::loadProject( const QString&      projectFileName,
                                  ProjectLoadAction   loadAction,
                                  RiaProjectModifier* projectModifier )
{
    // First Close the current project

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

    m_project->fileName = fullPathProjectFileName;
    m_project->readFile();

    // Apply any modifications to the loaded project before we go ahead and load actual data
    if ( projectModifier )
    {
        projectModifier->applyModificationsToProject( m_project );
    }

    // Propagate possible new location of project

    m_project->setProjectFileNameAndUpdateDependencies( fullPathProjectFileName );

    // On error, delete everything, and bail out.

    if ( m_project->projectFileVersionString().isEmpty() )
    {
        closeProject();

        QString errMsg = QString( "Unknown project file version detected in file \n%1\n\nCould not open project." )
                             .arg( fullPathProjectFileName );

        onProjectOpeningError( errMsg );

        // Delete all object possibly generated by readFile()
        delete m_project;
        m_project = new RimProject;

        onProjectOpened();

        return true;
    }

    ///////
    // Load the external data, and initialize stuff that needs specific ordering

    // VL check regarding specific order mentioned in comment above...

    m_preferences->lastUsedProjectFileName = fullPathProjectFileName;
    m_preferences->writePreferencesToApplicationStore();

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

    // Add well paths for each oil field
    for ( size_t oilFieldIdx = 0; oilFieldIdx < m_project->oilFields().size(); oilFieldIdx++ )
    {
        RimOilField* oilField = m_project->oilFields[oilFieldIdx];
        if ( oilField == nullptr ) continue;
        if ( oilField->wellPathCollection == nullptr )
        {
            oilField->wellPathCollection = new RimWellPathCollection();
        }

        oilField->wellPathCollection->loadDataAndUpdate();
    }

    {
        RimMainPlotCollection* mainPlotColl = m_project->mainPlotCollection();

        mainPlotColl->ensureCalculationIdsAreAssigned();
        mainPlotColl->ensureDefaultFlowPlotsAreCreated();
    }

    for ( RimOilField* oilField : m_project->oilFields )
    {
        if ( oilField == nullptr ) continue;
        // Temporary
        if ( !oilField->summaryCaseMainCollection() )
        {
            oilField->summaryCaseMainCollection = new RimSummaryCaseMainCollection();
        }
        oilField->summaryCaseMainCollection()->loadAllSummaryCaseData();

        if ( !oilField->observedDataCollection() )
        {
            oilField->observedDataCollection = new RimObservedDataCollection();
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

        oilField->fractureDefinitionCollection()->loadAndUpdateData();
        oilField->fractureDefinitionCollection()->createAndAssignTemplateCopyForNonMatchingUnit();

        {
            std::vector<RimWellPathFracture*> wellPathFractures;
            oilField->wellPathCollection->descendantsIncludingThisOfType( wellPathFractures );

            for ( auto fracture : wellPathFractures )
            {
                fracture->loadDataAndUpdate();
            }
        }

        oilField->surfaceCollection()->loadData();
    }

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

    // Now load the ReservoirViews for the cases
    // Add all "native" cases in the project
    std::vector<RimCase*> casesToLoad;
    m_project->allCases( casesToLoad );
    {
        caf::ProgressInfo caseProgress( casesToLoad.size(), "Reading Cases" );

        for ( size_t cIdx = 0; cIdx < casesToLoad.size(); ++cIdx )
        {
            RimCase* cas = casesToLoad[cIdx];
            CVF_ASSERT( cas );

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
                        std::vector<RimStimPlanColors*> stimPlanColors;
                        riv->descendantsIncludingThisOfType( stimPlanColors );
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

                    this->setActiveReservoirView( riv );

                    RimGridView* rigv = dynamic_cast<RimGridView*>( riv );
                    if ( rigv ) rigv->rangeFilterCollection()->updateIconState();

                    viewProgress.incrementProgress();
                }
            }
            caseProgress.incrementProgress();
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

    // Init summary case groups
    for ( RimOilField* oilField : m_project->oilFields )
    {
        auto sumMainCollection = oilField->summaryCaseMainCollection();
        if ( !sumMainCollection ) continue;

        for ( auto sumCaseGroup : sumMainCollection->summaryCaseCollections() )
        {
            sumCaseGroup->loadDataAndUpdate();
        }

        oilField->annotationCollection()->loadDataAndUpdate();
    }

    // Some procedures in onProjectOpened() may rely on the display model having been created
    // So we need to force the completion of the display model here.
    RiaViewRedrawScheduler::instance()->updateAndRedrawScheduledViews();

    // NB! This function must be called before executing command objects,
    // because the tree view state is restored from project file and sets
    // current active view ( see restoreTreeViewState() )
    // Default behavior for scripts is to use current active view for data read/write
    onProjectOpened();

    // Loop over command objects and execute them
    for ( size_t i = 0; i < m_project->commandObjects.size(); i++ )
    {
        m_commandQueue.push_back( m_project->commandObjects[i] );
    }

    // Lock the command queue
    m_commandQueueLock.lock();

    // Execute command objects, and release the mutex when the queue is empty
    executeCommandObjects();

    RiaLogging::info( QString( "Completed open of project file : '%1'" ).arg( projectFileName ) );

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
bool RiaApplication::saveProject( QString* errorMessage )
{
    CAF_ASSERT( errorMessage );
    CAF_ASSERT( m_project.notNull() );

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
bool RiaApplication::saveProjectAs( const QString& fileName, QString* errorMessage )
{
    // Make sure we always store path with forward slash to avoid issues when opening the project file on Linux
    m_project->fileName = RiaFilePathTools::toInternalSeparator( fileName );

    onProjectBeingSaved();

    if ( !m_project->writeProjectFile() )
    {
        CAF_ASSERT( errorMessage );
        *errorMessage = QString( "Not possible to save project file. Make sure you have sufficient access "
                                 "rights.\n\nProject file location : %1" )
                            .arg( fileName );
        return false;
    }

    m_preferences->lastUsedProjectFileName = fileName;
    m_preferences->writePreferencesToApplicationStore();

    onProjectSaved();

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaApplication::hasValidProjectFileExtension( const QString& fileName )
{
    if ( fileName.contains( ".rsp", Qt::CaseInsensitive ) || fileName.contains( ".rip", Qt::CaseInsensitive ) )
    {
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaApplication::closeProject()
{
    onProjectBeingClosed();

    terminateProcess();
    m_project->close();
    m_commandQueue.clear();

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

    RimGeoMechModels* geoMechModelCollection = m_project->activeOilField() ? m_project->activeOilField()->geoMechModels()
                                                                           : nullptr;
    // Create the geoMech model container if it is not there already
    if ( geoMechModelCollection == nullptr )
    {
        geoMechModelCollection                     = new RimGeoMechModels();
        m_project->activeOilField()->geoMechModels = geoMechModelCollection;
    }

    // Check if the file is already open, the odb reader does not support opening the same file twice very well
    for ( auto gmcase : geoMechModelCollection->cases() )
    {
        if ( gmcase->gridFileName() == fileName )
        {
            RiaLogging::warning( "File has already been opened. Cannot open the file twice! - " + fileName );
            return false;
        }
    }

    RimGeoMechCase* geoMechCase = new RimGeoMechCase();
    geoMechCase->setGridFileName( fileName );
    geoMechCase->caseUserDescription = caseName;
    geoMechCase->setApplyTimeFilter( applyTimeStepFilter );
    m_project->assignCaseIdToCase( geoMechCase );

    RimGeoMechView*   riv = geoMechCase->createAndAddReservoirView();
    caf::ProgressInfo progress( 11, "Loading Case" );
    progress.setNextProgressIncrement( 10 );

    riv->loadDataAndUpdate();

    if ( !riv->geoMechCase() )
    {
        delete geoMechCase;
        return false;
    }
    geoMechModelCollection->addCase( geoMechCase );

    progress.incrementProgress();
    progress.setProgressDescription( "Loading results information" );

    m_project->updateConnectedEditors();
    Riu3DMainWindowTools::setExpanded( riv );

    return true;
}

//--------------------------------------------------------------------------------------------------
/// Add a list of well path file paths (JSON files) to the well path collection
//--------------------------------------------------------------------------------------------------
std::vector<RimWellPath*> RiaApplication::addWellPathsToModel( QList<QString> wellPathFilePaths, QStringList* errorMessages )
{
    CAF_ASSERT( errorMessages );

    if ( m_project == nullptr || m_project->oilFields.size() < 1 ) return {};

    RimOilField* oilField = m_project->activeOilField();
    if ( oilField == nullptr ) return {};

    if ( oilField->wellPathCollection == nullptr )
    {
        // printf("Create well path collection.\n");
        oilField->wellPathCollection = new RimWellPathCollection();

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
    if ( m_project == nullptr || m_project->oilFields.size() < 1 ) return;

    RimOilField* oilField = m_project->activeOilField();
    if ( oilField == nullptr ) return;

    if ( oilField->wellPathCollection == nullptr )
    {
        oilField->wellPathCollection = new RimWellPathCollection();

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
std::vector<RimWellLogFile*> RiaApplication::addWellLogsToModel( const QList<QString>& wellLogFilePaths,
                                                                 QStringList*          errorMessages )
{
    CAF_ASSERT( errorMessages );

    if ( m_project == nullptr || m_project->oilFields.size() < 1 ) return {};

    RimOilField* oilField = m_project->activeOilField();
    if ( oilField == nullptr ) return {};

    if ( oilField->wellPathCollection == nullptr )
    {
        oilField->wellPathCollection = new RimWellPathCollection();

        m_project->updateConnectedEditors();
    }

    std::vector<RimWellLogFile*> wellLogFiles =
        oilField->wellPathCollection->addWellLogs( wellLogFilePaths, errorMessages );

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
    arguments << QApplication::applicationDirPath();

    if ( !m_preferences->octaveShowHeaderInfoWhenExecutingScripts )
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
        pathString = QApplication::applicationDirPath() + "\\octave_plugin_dependencies";
    else
        pathString = QApplication::applicationDirPath() + "\\octave_plugin_dependencies" + ";" + pathString;

    penv.insert( "PATH", pathString );
#else
    // Set the LD_LIBRARY_PATH to make the octave plugins find the embedded Qt
    QString ldPath = penv.value( "LD_LIBRARY_PATH", "" );

    if ( ldPath == "" )
        ldPath = QApplication::applicationDirPath();
    else
        ldPath = QApplication::applicationDirPath() + ":" + ldPath;

    penv.insert( "LD_LIBRARY_PATH", ldPath );
#endif

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
    QProcessEnvironment penv = QProcessEnvironment::systemEnvironment();
#ifdef ENABLE_GRPC
    penv.insert( "RESINSIGHT_GRPC_PORT", QString( "%1" ).arg( m_grpcServer->portNumber() ) );
    penv.insert( "RESINSIGHT_EXECUTABLE", QCoreApplication::applicationFilePath() );

    QStringList ripsLocations;
    QString     separator;
#ifdef WIN32
    ripsLocations << QCoreApplication::applicationDirPath() + "\\Python"
                  << QCoreApplication::applicationDirPath() + "\\..\\..\\Python";
    separator = ";";

#else
    ripsLocations << QCoreApplication::applicationDirPath() + "/Python"
                  << QCoreApplication::applicationDirPath() + "/../../Python";
    separator = ":";
#endif
    penv.insert( "PYTHONPATH",
                 QString( "%1%2%3" ).arg( penv.value( "PYTHONPATH" ) ).arg( separator ).arg( ripsLocations.join( separator ) ) );
#endif
    return penv;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaApplication::launchProcess( const QString&             program,
                                    const QStringList&         arguments,
                                    const QProcessEnvironment& processEnvironment )
{
    if ( m_workerProcess == nullptr )
    {
        // If multiple cases are present, pop the first case ID from the list and set as current case
        if ( !m_currentCaseIds.empty() )
        {
            int nextCaseId = m_currentCaseIds.front();
            m_currentCaseIds.pop_front();

            m_socketServer->setCurrentCaseId( nextCaseId );
        }
        else
        {
            // Disable current case concept
            m_socketServer->setCurrentCaseId( -1 );
        }

        m_runningWorkerProcess = true;
        m_workerProcess        = new caf::UiProcess( QCoreApplication::instance() );

        m_workerProcess->setProcessEnvironment( processEnvironment );

        QCoreApplication::instance()->connect( m_workerProcess,
                                               SIGNAL( finished( int, QProcess::ExitStatus ) ),
                                               SLOT( slotWorkerProcessFinished( int, QProcess::ExitStatus ) ) );

        startMonitoringWorkProgress( m_workerProcess );

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

            //            QMessageBox::warning(m_mainWindow, "Script execution", "Failed to start script executable
            //            located at\n" + program);

            return false;
        }

        return true;
    }
    else
    {
        // QMessageBox::warning(nullptr,
        //                   "Script execution",
        //                 "An Octave process is still running. Please stop this process before executing a new script.");
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
    m_currentCaseIds.clear();
    std::copy( caseIds.begin(), caseIds.end(), std::back_inserter( m_currentCaseIds ) );

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
        m_workerProcess->close();
    }

    m_runningWorkerProcess = false;
    m_workerProcess        = nullptr;
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
    return m_preferences;
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

    if ( this->project() )
    {
        this->project()->setScriptDirectories( m_preferences->scriptDirectories() );
        this->project()->setPlotTemplateFolders( m_preferences->plotTemplateFolders() );
        this->project()->updateConnectedEditors();
    }

    caf::ProgressInfoStatic::setEnabled( m_preferences->showProgressBar() );

    m_preferences->writePreferencesToApplicationStore();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaApplication::commandLineParameterHelp()
{
    QString helpText =
        QString( "\n%1 v. %2\n" ).arg( RI_APPLICATION_NAME ).arg( RiaApplication::getVersionStringApp( false ) );
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
void RiaApplication::addCommandObject( RimCommandObject* commandObject )
{
    m_commandQueue.push_back( commandObject );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaApplication::executeCommandObjects()
{
    {
        std::list<RimCommandObject*>::iterator it = m_commandQueue.begin();
        while ( it != m_commandQueue.end() )
        {
            RimCommandObject* toBeRemoved = *it;
            if ( !toBeRemoved->isAsyncronous() )
            {
                toBeRemoved->redo();

                ++it;
                m_commandQueue.remove( toBeRemoved );
            }
            else
            {
                ++it;
            }
        }
    }

    if ( !m_commandQueue.empty() )
    {
        std::list<RimCommandObject*>::iterator it = m_commandQueue.begin();

        RimCommandObject* first = *it;
        first->redo();

        m_commandQueue.pop_front();
    }
    else
    {
        // Unlock the command queue lock when the command queue is empty
        // Required to lock the mutex before unlocking to avoid undefined behavior
        m_commandQueueLock.tryLock();
        m_commandQueueLock.unlock();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaApplication::waitUntilCommandObjectsHasBeenProcessed()
{
    // Wait until all command objects have completed
    bool mutexLockedSuccessfully = m_commandQueueLock.tryLock();

    while ( !mutexLockedSuccessfully )
    {
        invokeProcessEvents();

        mutexLockedSuccessfully = m_commandQueueLock.tryLock();
    }
    m_commandQueueLock.unlock();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiaApplication::launchUnitTests()
{
#ifdef USE_UNIT_TESTS

    caf::ProgressInfoBlocker progressBlocker;
    cvf::Assert::setReportMode( cvf::Assert::CONSOLE );

    int                      argc      = QCoreApplication::arguments().size();
    QStringList              arguments = QCoreApplication::arguments();
    std::vector<std::string> argumentsStd;
    for ( QString qstring : arguments )
    {
        argumentsStd.push_back( qstring.toStdString() );
    }
    std::vector<char*> argVector;
    for ( std::string& string : argumentsStd )
    {
        argVector.push_back( &string.front() );
    }
    char** argv = argVector.data();

    testing::InitGoogleTest( &argc, argv );

    //
    // Use the gtest filter to execute a subset of tests
    QString filterText = RiaPreferences::current()->gtestFilter();
    if ( !filterText.isEmpty() )
    {
        ::testing::GTEST_FLAG( filter ) = filterText.toStdString();

        // Example on filter syntax
        //::testing::GTEST_FLAG( filter ) = "*RifCaseRealizationParametersReaderTest*";
    }

    // Use this macro in main() to run all tests.  It returns 0 if all
    // tests are successful, or 1 otherwise.
    //
    // RUN_ALL_TESTS() should be invoked after the command line has been
    // parsed by InitGoogleTest().

    return RUN_ALL_TESTS();

#else
    return -1;
#endif
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
bool RiaApplication::initializeGrpcServer( const cvf::ProgramOptions& progOpt )
{
#ifdef ENABLE_GRPC
    if ( !m_preferences->enableGrpcServer() ) return false;

    int  defaultPortNumber = m_preferences->defaultGrpcPortNumber();
    bool fixedPort         = false;
    if ( cvf::Option o = progOpt.option( "server" ) )
    {
        if ( o.valueCount() == 1 )
        {
            defaultPortNumber = o.value( 0 ).toInt( defaultPortNumber );
            fixedPort         = true;
        }
    }
    int portNumber = defaultPortNumber;
    if ( !fixedPort )
    {
        portNumber = RiaGrpcServer::findAvailablePortNumber( defaultPortNumber );
    }
    m_grpcServer.reset( new RiaGrpcServer( portNumber ) );
    return true;
#else
    return false;
#endif
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaApplication::initialize()
{
    m_preferences = new RiaPreferences;
    caf::PdmSettings::readFieldsFromApplicationStore( m_preferences );
    m_preferences->initAfterReadRecursively();
    applyPreferences();

    // Start with a project
    m_project = new RimProject;
    m_project->setScriptDirectories( m_preferences->scriptDirectories() );
    m_project->setPlotTemplateFolders( m_preferences->plotTemplateFolders() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiaApplication::launchUnitTestsWithConsole()
{
    return launchUnitTests();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaApplication::loadAndUpdatePlotData()
{
    RimWellLogPlotCollection*            wlpColl  = nullptr;
    RimSummaryPlotCollection*            spColl   = nullptr;
    RimSummaryCrossPlotCollection*       scpColl  = nullptr;
    RimFlowPlotCollection*               flowColl = nullptr;
    RimRftPlotCollection*                rftColl  = nullptr;
    RimPltPlotCollection*                pltColl  = nullptr;
    RimGridCrossPlotCollection*          gcpColl  = nullptr;
    RimSaturationPressurePlotCollection* sppColl  = nullptr;
    RimAnalysisPlotCollection*           alsColl  = nullptr;
    RimCorrelationPlotCollection*        corrColl = nullptr;
    RimMultiPlotCollection*              gpwColl  = nullptr;
    RimFractureModelPlotCollection*      frmColl  = nullptr;

    if ( m_project->mainPlotCollection() )
    {
        if ( m_project->mainPlotCollection()->wellLogPlotCollection() )
        {
            wlpColl = m_project->mainPlotCollection()->wellLogPlotCollection();
        }
        if ( m_project->mainPlotCollection()->summaryPlotCollection() )
        {
            spColl = m_project->mainPlotCollection()->summaryPlotCollection();
        }
        if ( m_project->mainPlotCollection()->summaryCrossPlotCollection() )
        {
            scpColl = m_project->mainPlotCollection()->summaryCrossPlotCollection();
        }
        if ( m_project->mainPlotCollection()->flowPlotCollection() )
        {
            flowColl = m_project->mainPlotCollection()->flowPlotCollection();
        }
        if ( m_project->mainPlotCollection()->rftPlotCollection() )
        {
            rftColl = m_project->mainPlotCollection()->rftPlotCollection();
        }
        if ( m_project->mainPlotCollection()->pltPlotCollection() )
        {
            pltColl = m_project->mainPlotCollection()->pltPlotCollection();
        }
        if ( m_project->mainPlotCollection()->gridCrossPlotCollection() )
        {
            gcpColl = m_project->mainPlotCollection()->gridCrossPlotCollection();
        }
        if ( m_project->mainPlotCollection()->saturationPressurePlotCollection() )
        {
            sppColl = m_project->mainPlotCollection()->saturationPressurePlotCollection();
        }
        if ( m_project->mainPlotCollection()->analysisPlotCollection() )
        {
            alsColl = m_project->mainPlotCollection()->analysisPlotCollection();
        }
        if ( m_project->mainPlotCollection->correlationPlotCollection() )
        {
            corrColl = m_project->mainPlotCollection()->correlationPlotCollection();
        }
        if ( m_project->mainPlotCollection()->multiPlotCollection() )
        {
            gpwColl = m_project->mainPlotCollection()->multiPlotCollection();
        }
        if ( m_project->mainPlotCollection()->fractureModelPlotCollection() )
        {
            frmColl = m_project->mainPlotCollection()->fractureModelPlotCollection();
        }
    }

    size_t plotCount = 0;
    plotCount += wlpColl ? wlpColl->wellLogPlots().size() : 0;
    plotCount += spColl ? spColl->plots().size() : 0;
    plotCount += scpColl ? scpColl->plots().size() : 0;
    plotCount += flowColl ? flowColl->plotCount() : 0;
    plotCount += rftColl ? rftColl->rftPlots().size() : 0;
    plotCount += pltColl ? pltColl->pltPlots().size() : 0;
    plotCount += gcpColl ? gcpColl->plotCount() : 0;
    plotCount += sppColl ? sppColl->plotCount() : 0;
    plotCount += alsColl ? alsColl->plotCount() : 0;
    plotCount += corrColl ? corrColl->plotCount() + corrColl->reports().size() : 0;
    plotCount += gpwColl ? gpwColl->multiPlots().size() : 0;
    plotCount += frmColl ? frmColl->fractureModelPlots().size() : 0;

    if ( plotCount > 0 )
    {
        caf::ProgressInfo plotProgress( plotCount, "Loading Plot Data" );
        if ( wlpColl )
        {
            for ( size_t wlpIdx = 0; wlpIdx < wlpColl->wellLogPlots().size(); ++wlpIdx )
            {
                wlpColl->wellLogPlots[wlpIdx]->loadDataAndUpdate();
                plotProgress.incrementProgress();
            }
        }

        if ( spColl )
        {
            for ( auto plot : spColl->plots() )
            {
                plot->loadDataAndUpdate();
                plotProgress.incrementProgress();
            }
        }

        if ( scpColl )
        {
            for ( auto plot : scpColl->plots() )
            {
                plot->loadDataAndUpdate();
                plotProgress.incrementProgress();
            }
        }

        if ( flowColl )
        {
            plotProgress.setNextProgressIncrement( flowColl->plotCount() );
            flowColl->loadDataAndUpdate();
            plotProgress.incrementProgress();
        }

        if ( rftColl )
        {
            for ( const auto& rftPlot : rftColl->rftPlots() )
            {
                rftPlot->loadDataAndUpdate();
                plotProgress.incrementProgress();
            }
        }

        if ( pltColl )
        {
            for ( const auto& pltPlot : pltColl->pltPlots() )
            {
                pltPlot->loadDataAndUpdate();
                plotProgress.incrementProgress();
            }
        }

        if ( gcpColl )
        {
            for ( const auto& gcpPlot : gcpColl->plots() )
            {
                gcpPlot->loadDataAndUpdate();
                plotProgress.incrementProgress();
            }
        }

        if ( sppColl )
        {
            for ( const auto& sppPlot : sppColl->plots() )
            {
                sppPlot->loadDataAndUpdate();
                plotProgress.incrementProgress();
            }
        }

        if ( alsColl )
        {
            for ( const auto& alsPlot : alsColl->plots() )
            {
                alsPlot->loadDataAndUpdate();
                plotProgress.incrementProgress();
            }
        }

        if ( corrColl )
        {
            for ( const auto& corrPlot : corrColl->plots() )
            {
                corrPlot->loadDataAndUpdate();
                plotProgress.incrementProgress();
            }
            for ( const auto& reports : corrColl->reports() )
            {
                reports->loadDataAndUpdate();
                plotProgress.incrementProgress();
            }
        }

        if ( gpwColl )
        {
            for ( const auto& multiPlot : gpwColl->multiPlots() )
            {
                multiPlot->loadDataAndUpdate();
                plotProgress.incrementProgress();
            }
        }

        if ( frmColl )
        {
            for ( const auto& fractureModelPlot : frmColl->fractureModelPlots() )
            {
                fractureModelPlot->loadDataAndUpdate();
                plotProgress.incrementProgress();
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaApplication::resetProject()
{
    if ( m_project.notNull() )
    {
        delete m_project.p();
        m_project = nullptr;
    }

    if ( m_preferences )
    {
        delete m_preferences;
        m_preferences = nullptr;
    }

    initialize();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaApplication::generateCode( const QString& fileName, QString* errMsg )
{
    CAF_ASSERT( errMsg );

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

            out << generator->generate( caf::PdmDefaultObjectFactory::instance() );
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
                out << "+++ \n";
                out << "title =  \"Command Reference (BETA)\" \n";
                out << "published = true \n";
                out << "weight = 96 \n";
                out << "+++ \n";

                out << "# Introduction\n\n";
                out << "As the Python interface is growing release by release, we are investigating how to "
                       "automate "
                       "the building of reference documentation. This document is not complete, but will improve "
                       "as "
                       "the automation "
                       "moves forward.\n";

                out << "## Currently missing features\n\n";
                out << " - Description of enums\n";
                out << " - Description of return values/classes\n";
                out << " - Description of each object\n";
            }

            std::vector<std::shared_ptr<const caf::PdmObject>> commandObjects;

            QStringList excludedClassNames{"TestCommand1", "TC2"}; // See RifCommandCore-Text.cpp

            auto allObjects = caf::PdmMarkdownBuilder::createAllObjects( caf::PdmDefaultObjectFactory::instance() );
            for ( auto classObject : allObjects )
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

        out << generator->generate( caf::PdmDefaultObjectFactory::instance() );
    }

    return true;
}
