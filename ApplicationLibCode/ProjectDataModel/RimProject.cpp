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

#include "RimProject.h"

#include "RiaCompletionTypeCalculationScheduler.h"
#include "RiaFieldHandleTools.h"
#include "RiaFilePathTools.h"
#include "RiaGuiApplication.h"
#include "RiaProjectFileVersionTools.h"
#include "RiaTextStringTools.h"
#include "RiaVersionInfo.h"

#include "RicfCommandObject.h"
#include "RigEclipseCaseData.h"
#include "RigGridBase.h"

#include "PlotTemplates/RimPlotTemplateFolderItem.h"
#include "Polygons/RimPolygonCollection.h"
#include "RimAdvancedSnapshotExportDefinition.h"
#include "RimAnalysisPlotCollection.h"
#include "RimAnnotationCollection.h"
#include "RimAnnotationInViewCollection.h"
#include "RimCalcScript.h"
#include "RimCase.h"
#include "RimCaseCollection.h"
#include "RimColorLegendCollection.h"
#include "RimCommandObject.h"
#include "RimCompletionTemplateCollection.h"
#include "RimContextCommandBuilder.h"
#include "RimCorrelationPlotCollection.h"
#include "RimDialogData.h"
#include "RimEclipseCase.h"
#include "RimEclipseCaseCollection.h"
#include "RimEclipseContourMapViewCollection.h"
#include "RimEclipseViewCollection.h"
#include "RimEnsembleWellLogsCollection.h"
#include "RimFileWellPath.h"
#include "RimFlowPlotCollection.h"
#include "RimFormationNamesCollection.h"
#include "RimFractureTemplate.h"
#include "RimFractureTemplateCollection.h"
#include "RimGeoMechCase.h"
#include "RimGeoMechModels.h"
#include "RimGridCalculationCollection.h"
#include "RimGridCrossPlotCollection.h"
#include "RimGridView.h"
#include "RimIdenticalGridCaseGroup.h"
#include "RimMainPlotCollection.h"
#include "RimMeasurement.h"
#include "RimMultiPlotCollection.h"
#include "RimObservedDataCollection.h"
#include "RimObservedSummaryData.h"
#include "RimOilField.h"
#include "RimPlotWindow.h"
#include "RimPltPlotCollection.h"
#include "RimPolylinesFromFileAnnotation.h"
#include "RimRftPlotCollection.h"
#include "RimSaturationPressurePlotCollection.h"
#include "RimScriptCollection.h"
#include "RimSeismicDataCollection.h"
#include "RimSeismicView.h"
#include "RimSeismicViewCollection.h"
#include "RimStimPlanModelPlotCollection.h"
#include "RimSummaryCalculation.h"
#include "RimSummaryCalculationCollection.h"
#include "RimSummaryCaseCollection.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSummaryMultiPlotCollection.h"
#include "RimSummaryTableCollection.h"
#include "RimSurfaceCollection.h"
#include "RimTools.h"
#include "RimUserDefinedPolylinesAnnotation.h"
#include "RimValveTemplate.h"
#include "RimValveTemplateCollection.h"
#include "RimVfpPlotCollection.h"
#include "RimViewLinker.h"
#include "RimViewLinkerCollection.h"
#include "RimViewWindow.h"
#include "RimWellLogLasFile.h"
#include "RimWellLogPlotCollection.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"

#include "Tools/RiaVariableMapper.h"

#ifdef USE_QTCHARTS
#include "RimEnsembleFractureStatisticsPlot.h"
#include "RimEnsembleFractureStatisticsPlotCollection.h"
#include "RimGridStatisticsPlot.h"
#include "RimGridStatisticsPlotCollection.h"
#endif

#include "SsiHubImportCommands/RimWellPathImport.h"

#include "RiuMainWindow.h"
#include "RiuPlotMainWindow.h"

#include "OctaveScriptCommands/RicExecuteScriptForCasesFeature.h"

#include "cafCmdFeature.h"
#include "cafCmdFeatureManager.h"
#include "cafCmdFeatureMenuBuilder.h"
#include "cafPdmUiTreeOrdering.h"
#include "cvfBoundingBox.h"

#include <QDebug>
#include <QDir>
#include <QMenu>

#include <algorithm>

CAF_PDM_SOURCE_INIT( RimProject, "ResInsightProject" );
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimProject::RimProject()
    : m_nextValidCaseId( 0 )
    , m_nextValidCaseGroupId( 0 )
    , m_nextValidViewId( -1 )
    , m_nextValidPlotId( -1 )
    , m_nextValidSummaryCaseId( 1 )
    , m_nextValidEnsembleId( 1 )
{
    CAF_PDM_InitScriptableObjectWithNameAndComment( "Project", "", "", "", "Project", "The ResInsight Project" );

    CAF_PDM_InitField( &m_projectFileVersionString, "ProjectFileVersionString", QString( STRPRODUCTVER ), "" );
    m_projectFileVersionString.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_globalPathList, "ReferencedExternalFiles", "" );
    m_globalPathList.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &oilFields, "OilFields", "Oil Fields" );

    CAF_PDM_InitFieldNoDefault( &colorLegendCollection, "ColorLegendCollection", "Color Legend Collection" );
    colorLegendCollection = new RimColorLegendCollection();
    colorLegendCollection->createStandardColorLegends();

    CAF_PDM_InitFieldNoDefault( &scriptCollection, "ScriptCollection", "Octave Scripts", ":/octave.png" );
    scriptCollection.xmlCapability()->disableIO();

    CAF_PDM_InitFieldNoDefault( &wellPathImport, "WellPathImport", "WellPathImport" );
    wellPathImport = new RimWellPathImport();
    wellPathImport.uiCapability()->setUiTreeChildrenHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_mainPlotCollection, "MainPlotCollection", "Plots" );

    CAF_PDM_InitFieldNoDefault( &viewLinkerCollection, "LinkedViews", "Linked Views", ":/LinkView.svg" );
    viewLinkerCollection = new RimViewLinkerCollection;

    CAF_PDM_InitFieldNoDefault( &calculationCollection, "CalculationCollection", "Calculation Collection" );
    calculationCollection = new RimSummaryCalculationCollection;

    CAF_PDM_InitFieldNoDefault( &gridCalculationCollection, "GridCalculationCollection", "Grid Calculation Collection" );
    gridCalculationCollection = new RimGridCalculationCollection;

    CAF_PDM_InitFieldNoDefault( &commandObjects, "CommandObjects", "Command Objects" );

    CAF_PDM_InitFieldNoDefault( &multiSnapshotDefinitions, "MultiSnapshotDefinitions", "Multi Snapshot Definitions" );

    CAF_PDM_InitFieldNoDefault( &mainWindowTreeViewStates, "TreeViewStates", "" );
    mainWindowTreeViewStates.uiCapability()->setUiHidden( true );
    CAF_PDM_InitFieldNoDefault( &mainWindowCurrentModelIndexPaths, "TreeViewCurrentModelIndexPaths", "" );
    mainWindowCurrentModelIndexPaths.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &plotWindowTreeViewStates, "PlotWindowTreeViewStates", "" );
    plotWindowTreeViewStates.uiCapability()->setUiHidden( true );
    CAF_PDM_InitFieldNoDefault( &plotWindowCurrentModelIndexPaths, "PlotWindowTreeViewCurrentModelIndexPaths", "" );
    plotWindowCurrentModelIndexPaths.uiCapability()->setUiHidden( true );

    CAF_PDM_InitField( &m_show3DWindow, "show3DWindow", true, "Show 3D Window" );
    m_show3DWindow.uiCapability()->setUiHidden( true );

    CAF_PDM_InitField( &m_showPlotWindow, "showPlotWindow", false, "Show Plot Window" );
    m_showPlotWindow.uiCapability()->setUiHidden( true );

    CAF_PDM_InitField( &m_showPlotWindowOnTopOf3DWindow, "showPlotWindowOnTopOf3DWindow", false, "Show Plot On Top" );
    m_showPlotWindowOnTopOf3DWindow.uiCapability()->setUiHidden( true );

    CAF_PDM_InitField( &m_subWindowsTiled3DWindow_OBSOLETE, "tiled3DWindow", false, "Tile 3D Window" );
    m_subWindowsTiled3DWindow_OBSOLETE.uiCapability()->setUiHidden( true );

    CAF_PDM_InitField( &m_subWindowsTiledPlotWindow_OBSOLETE, "tiledPlotWindow", false, "Tile Plot Window" );
    m_subWindowsTiledPlotWindow_OBSOLETE.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_dialogData, "DialogData", "DialogData" );
    m_dialogData = new RimDialogData();
    m_dialogData.uiCapability()->setUiTreeChildrenHidden( true );

    // Obsolete fields. The content is moved to OilFields and friends
    CAF_PDM_InitFieldNoDefault( &casesObsolete, "Reservoirs", "" );
    RiaFieldHandleTools::disableWriteAndSetFieldHidden( &casesObsolete );

    CAF_PDM_InitFieldNoDefault( &caseGroupsObsolete, "CaseGroups", "" );
    RiaFieldHandleTools::disableWriteAndSetFieldHidden( &caseGroupsObsolete );

    CAF_PDM_InitFieldNoDefault( &m_subWindowsTileMode3DWindow, "TileMode3DWindow", "TileMode3DWindow" );
    m_subWindowsTileMode3DWindow.uiCapability()->setUiHidden( true );
    CAF_PDM_InitFieldNoDefault( &m_subWindowsTileModePlotWindow, "TileModePlotWindow", "TileModePlotWindow" );
    m_subWindowsTileModePlotWindow.uiCapability()->setUiHidden( true );

    // Initialization

    scriptCollection = new RimScriptCollection();
    scriptCollection->directory.uiCapability()->setUiHidden( true );
    scriptCollection->uiCapability()->setUiName( "Scripts" );
    scriptCollection->uiCapability()->setUiIconFromResourceString( ":/octave.png" );

    m_mainPlotCollection = new RimMainPlotCollection();

    CAF_PDM_InitFieldNoDefault( &m_plotTemplateTopFolder, "PlotTemplateCollection", "Plot Templates" );
    m_plotTemplateTopFolder = new RimPlotTemplateFolderItem();
    m_plotTemplateTopFolder.xmlCapability()->disableIO();

    // For now, create a default first oilfield that contains the rest of the project
    oilFields.push_back( new RimOilField );

    setUiHidden( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimProject::~RimProject()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimProject* RimProject::current()
{
    return RiaApplication::instance()->project();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimProject::close()
{
    if ( m_mainPlotCollection() )
    {
        m_mainPlotCollection()->deleteAllContainedObjects();
    }

    oilFields.deleteChildren();
    oilFields.push_back( new RimOilField );

    casesObsolete.deleteChildren();
    caseGroupsObsolete.deleteChildren();

    wellPathImport->regions().deleteChildren();

    commandObjects.deleteChildren();

    multiSnapshotDefinitions.deleteChildren();

    m_dialogData->clearProjectSpecificData();

    calculationCollection->deleteAllContainedObjects();
    gridCalculationCollection->deleteAllContainedObjects();
    colorLegendCollection->deleteCustomColorLegends();

    delete viewLinkerCollection->viewLinker();
    viewLinkerCollection->viewLinker = nullptr;

    fileName = "";

    m_globalPathList = "";

    mainWindowCurrentModelIndexPaths = "";
    mainWindowTreeViewStates         = "";
    plotWindowCurrentModelIndexPaths = "";
    plotWindowTreeViewStates         = "";

    m_nextValidCaseId        = 0;
    m_nextValidCaseGroupId   = 0;
    m_nextValidViewId        = -1;
    m_nextValidPlotId        = -1;
    m_nextValidSummaryCaseId = 1;
    m_nextValidEnsembleId    = 1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimProject::beforeInitAfterRead()
{
    distributePathsFromGlobalPathList();

    // Create an empty oil field in case the project did not contain one
    if ( oilFields.empty() )
    {
        oilFields.push_back( new RimOilField );
    }

    // Handle old project files with obsolete structure.
    // Move caseGroupsObsolete and casesObsolete to oilFields()[idx]->analysisModels()
    RimEclipseCaseCollection* analysisModels                    = activeOilField() ? activeOilField()->analysisModels() : nullptr;
    bool                      movedOneRimIdenticalGridCaseGroup = false;
    for ( size_t cgIdx = 0; cgIdx < caseGroupsObsolete.size(); ++cgIdx )
    {
        RimIdenticalGridCaseGroup* sourceCaseGroup = caseGroupsObsolete[cgIdx];
        if ( analysisModels )
        {
            analysisModels->caseGroups.push_back( sourceCaseGroup );
            movedOneRimIdenticalGridCaseGroup = true; // moved at least one so assume the others will be moved too...
        }
    }

    if ( movedOneRimIdenticalGridCaseGroup )
    {
        caseGroupsObsolete.clearWithoutDelete();
    }

    bool movedOneRimCase = false;
    for ( size_t cIdx = 0; cIdx < casesObsolete().size(); ++cIdx )
    {
        if ( analysisModels )
        {
            RimEclipseCase* sourceCase = casesObsolete[cIdx];
            casesObsolete.set( cIdx, nullptr );
            analysisModels->cases.push_back( sourceCase );
            movedOneRimCase = true; // moved at least one so assume the others will be moved too...
        }
    }

    if ( movedOneRimCase )
    {
        casesObsolete.clearWithoutDelete();
    }

    // Set project pointer to each well path
    for ( size_t oilFieldIdx = 0; oilFieldIdx < oilFields().size(); oilFieldIdx++ )
    {
        RimOilField* oilField = oilFields[oilFieldIdx];
        if ( oilField == nullptr || oilField->wellPathCollection == nullptr ) continue;
    }

    if ( m_subWindowsTiled3DWindow_OBSOLETE ) m_subWindowsTileMode3DWindow = RiaDefines::WindowTileMode::DEFAULT;
    if ( m_subWindowsTiledPlotWindow_OBSOLETE ) m_subWindowsTileModePlotWindow = RiaDefines::WindowTileMode::DEFAULT;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimProject::initAfterRead()
{
    // Function moved to beforeInitAfterRead() to make sure that file path objects are replaced before other initAfterRead() is called
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimProject::setupBeforeSave()
{
    if ( RiaGuiApplication::isRunning() )
    {
        RiaGuiApplication* guiApp = RiaGuiApplication::instance();

        if ( guiApp )
        {
            m_show3DWindow   = guiApp->isMain3dWindowVisible();
            m_showPlotWindow = guiApp->isMainPlotWindowVisible();

            if ( m_showPlotWindow )
            {
                auto plotWindow                 = RiuPlotMainWindow::instance();
                m_showPlotWindowOnTopOf3DWindow = plotWindow->isTopLevel();
            }
        }
    }

    m_projectFileVersionString = STRPRODUCTVER;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimMainPlotCollection* RimProject::mainPlotCollection() const
{
    return m_mainPlotCollection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimProject::writeProjectFile()
{
    transferPathsToGlobalPathList();
    bool couldOpenFile = writeFile();
    distributePathsFromGlobalPathList();

    return couldOpenFile;
}

//--------------------------------------------------------------------------------------------------
/// Support list of multiple script paths divided by ';'
//--------------------------------------------------------------------------------------------------
void RimProject::setScriptDirectories( const QString& scriptDirectories, int maxFolderDepth )
{
    scriptCollection->calcScripts().deleteChildren();
    scriptCollection->subDirectories().deleteChildren();

    QStringList pathList = scriptDirectories.split( ';' );
    for ( const QString& path : pathList )
    {
        QDir dir( path );
        if ( !path.isEmpty() && dir.exists() && dir.isReadable() )
        {
            RimScriptCollection* sharedScriptLocation = new RimScriptCollection;
            sharedScriptLocation->directory           = path;
            sharedScriptLocation->setUiName( dir.dirName() );

            sharedScriptLocation->readContentFromDisc( maxFolderDepth );

            scriptCollection->subDirectories.push_back( sharedScriptLocation );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimProject::setPlotTemplateFolders( const QStringList& plotTemplateFolders )
{
    if ( !m_plotTemplateTopFolder() )
    {
        m_plotTemplateTopFolder = new RimPlotTemplateFolderItem();
    }

    m_plotTemplateTopFolder->createRootFolderItemsFromFolderPaths( plotTemplateFolders );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimProject::projectFileVersionString() const
{
    return m_projectFileVersionString;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimProject::isProjectFileVersionEqualOrOlderThan( const QString& otherProjectFileVersion ) const
{
    QString candidateProjectFileVersion = projectFileVersionString();

    return !RiaProjectFileVersionTools::isCandidateVersionNewerThanOther( candidateProjectFileVersion, otherProjectFileVersion );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimProject::setProjectFileNameAndUpdateDependencies( const QString& projectFileName )
{
    // Extract the filename of the project file when it was saved
    QString oldProjectFileName = fileName;
    // Replace with the new actual filename
    fileName = projectFileName;

    QFileInfo fileInfo( projectFileName );
    QString   newProjectPath = fileInfo.path();

    QFileInfo fileInfoOld( oldProjectFileName );
    QString   oldProjectPath = fileInfoOld.path();

    std::vector<caf::FilePath*> filePaths = allFilePaths();
    for ( caf::FilePath* filePath : filePaths )
    {
        bool                 foundFile = false;
        std::vector<QString> searchedPaths;

        QString filePathCandidate = filePath->path();

        QString newFilePath = RimTools::relocateFile( filePathCandidate, newProjectPath, oldProjectPath, &foundFile, &searchedPaths );
        filePath->setPath( newFilePath );
    }

    wellPathImport->updateFilePaths();
    auto* wellPathColl = RimTools::wellPathCollection();
    if ( wellPathColl )
    {
        for ( auto wellPath : wellPathColl->allWellPaths() )
        {
            if ( auto fileWellPath = dynamic_cast<RimFileWellPath*>( wellPath ) )
            {
                fileWellPath->updateFilePathsFromProjectPath( oldProjectPath, newProjectPath );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimProject::assignCaseIdToSummaryCase( RimSummaryCase* summaryCase )
{
    if ( summaryCase )
    {
        for ( RimSummaryCase* s : allSummaryCases() )
        {
            m_nextValidSummaryCaseId = std::max( m_nextValidSummaryCaseId, s->caseId() + 1 );
        }

        summaryCase->setCaseId( m_nextValidSummaryCaseId++ );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimProject::assignIdToEnsemble( RimSummaryCaseCollection* summaryCaseCollection )
{
    if ( summaryCaseCollection )
    {
        for ( RimSummaryCaseCollection* s : RimProject::summaryGroups() )
        {
            m_nextValidEnsembleId = std::max( m_nextValidEnsembleId, s->ensembleId() + 1 );
        }

        summaryCaseCollection->setEnsembleId( m_nextValidEnsembleId );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimCase*> RimProject::allGridCases() const
{
    std::vector<RimCase*> cases;

    for ( RimOilField* oilField : oilFields() )
    {
        if ( !oilField ) continue;

        RimEclipseCaseCollection* analysisModels = oilField->analysisModels();
        if ( analysisModels )
        {
            for ( RimEclipseCase* eclipseCase : analysisModels->cases )
            {
                cases.push_back( eclipseCase );
            }
            for ( RimIdenticalGridCaseGroup* cg : analysisModels->caseGroups )
            {
                // Load the Main case of each IdenticalGridCaseGroup
                if ( cg == nullptr ) continue;

                if ( cg->statisticsCaseCollection() )
                {
                    for ( RimEclipseCase* eclipseCase : cg->statisticsCaseCollection()->reservoirs )
                    {
                        cases.push_back( eclipseCase );
                    }
                }
                if ( cg->caseCollection() )
                {
                    for ( RimEclipseCase* eclipseCase : cg->caseCollection()->reservoirs )
                    {
                        cases.push_back( eclipseCase );
                    }
                }
            }
        }

        RimGeoMechModels* geomModels = oilField->geoMechModels();
        if ( geomModels )
        {
            for ( auto acase : geomModels->cases() )
            {
                cases.push_back( acase );
            }
        }
    }

    return cases;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimProject::assignCaseIdToCase( RimCase* reservoirCase )
{
    if ( reservoirCase )
    {
        std::vector<RimCase*> cases = descendantsIncludingThisOfType<RimCase>();
        for ( RimCase* rimCase : cases )
        {
            m_nextValidCaseId = std::max( m_nextValidCaseId, rimCase->caseId() + 1 );
        }

        reservoirCase->setCaseId( m_nextValidCaseId++ );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimProject::assignIdToCaseGroup( RimIdenticalGridCaseGroup* caseGroup )
{
    if ( caseGroup )
    {
        std::vector<RimIdenticalGridCaseGroup*> identicalCaseGroups = descendantsIncludingThisOfType<RimIdenticalGridCaseGroup>();

        for ( RimIdenticalGridCaseGroup* existingCaseGroup : identicalCaseGroups )
        {
            m_nextValidCaseGroupId = std::max( m_nextValidCaseGroupId, existingCaseGroup->groupId() + 1 );
        }

        caseGroup->groupId = m_nextValidCaseGroupId++;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimProject::assignViewIdToView( Rim3dView* view )
{
    if ( view )
    {
        if ( m_nextValidViewId < 0 )
        {
            std::vector<Rim3dView*> views = descendantsIncludingThisOfType<Rim3dView>();

            for ( Rim3dView* existingView : views )
            {
                m_nextValidViewId = std::max( m_nextValidViewId, existingView->id() + 1 );
            }
        }

        view->setId( m_nextValidViewId++ );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimProject::assignPlotIdToPlotWindow( RimPlotWindow* plotWindow )
{
    if ( plotWindow )
    {
        if ( m_nextValidPlotId < 0 )
        {
            std::vector<RimPlotWindow*> plotWindows = descendantsIncludingThisOfType<RimPlotWindow>();

            for ( RimPlotWindow* existingPlotWindow : plotWindows )
            {
                m_nextValidPlotId = std::max( m_nextValidPlotId, existingPlotWindow->id() + 1 );
            }
        }

        plotWindow->setId( m_nextValidPlotId++ );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryCase*> RimProject::allSummaryCases() const
{
    std::vector<RimSummaryCase*> sumCases;

    for ( RimOilField* oilField : oilFields )
    {
        if ( !oilField ) continue;
        RimSummaryCaseMainCollection* sumCaseMainColl = oilField->summaryCaseMainCollection();
        if ( sumCaseMainColl )
        {
            std::vector<RimSummaryCase*> allSummaryCases = sumCaseMainColl->allSummaryCases();
            if ( !allSummaryCases.empty() )
            {
                sumCases.insert( sumCases.end(), allSummaryCases.begin(), allSummaryCases.end() );
            }
        }

        auto& observedDataColl = oilField->observedDataCollection();
        if ( observedDataColl != nullptr && !observedDataColl->allObservedSummaryData().empty() )
        {
            auto observedData = observedDataColl->allObservedSummaryData();
            sumCases.insert( sumCases.end(), observedData.begin(), observedData.end() );
        }
    }

    return sumCases;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryCaseCollection*> RimProject::summaryGroups() const
{
    std::vector<RimSummaryCaseCollection*> groups;

    for ( RimOilField* oilField : oilFields )
    {
        if ( !oilField ) continue;
        RimSummaryCaseMainCollection* sumCaseMainColl = oilField->summaryCaseMainCollection();
        if ( sumCaseMainColl )
        {
            std::vector<RimSummaryCaseCollection*> g = sumCaseMainColl->summaryCaseCollections();
            groups.insert( groups.end(), g.begin(), g.end() );
        }
    }

    return groups;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCaseMainCollection* RimProject::firstSummaryCaseMainCollection() const
{
    if ( oilFields.empty() ) return nullptr;
    return oilFields[0]->summaryCaseMainCollection;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<Rim3dView*> RimProject::allNotLinkedViews() const
{
    std::vector<Rim3dView*> alreadyLinkedViews;
    if ( viewLinkerCollection->viewLinker() )
    {
        alreadyLinkedViews = viewLinkerCollection->viewLinker()->allViews();
    }

    std::vector<Rim3dView*> views;

    for ( RimCase* rimCase : allGridCases() )
    {
        if ( !rimCase ) continue;

        for ( Rim3dView* caseView : rimCase->views() )
        {
            RimGridView* gridView = dynamic_cast<RimGridView*>( caseView );

            if ( !gridView ) continue;

            bool isLinked = false;
            for ( size_t lnIdx = 0; lnIdx < alreadyLinkedViews.size(); lnIdx++ )
            {
                if ( gridView == alreadyLinkedViews[lnIdx] )
                {
                    isLinked = true;
                }
            }
            if ( !isLinked )
            {
                views.push_back( gridView );
            }
        }
    }

    return views;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<Rim3dView*> RimProject::allViews() const
{
    std::vector<Rim3dView*> views;

    for ( RimCase* rimCase : allGridCases() )
    {
        if ( !rimCase ) continue;

        for ( Rim3dView* view : rimCase->views() )
        {
            if ( view )
            {
                views.push_back( view );
            }
        }
    }

    for ( RimOilField* oilField : oilFields() )
    {
        if ( !oilField ) continue;
        if ( !oilField->seismicViewCollection() ) continue;

        for ( auto seisview : oilField->seismicViewCollection()->views() )
        {
            views.push_back( seisview );
        }
    }

    return views;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<Rim3dView*> RimProject::allVisibleViews() const
{
    std::vector<Rim3dView*> views;
    for ( RimCase* rimCase : allGridCases() )
    {
        if ( !rimCase ) continue;

        for ( Rim3dView* view : rimCase->views() )
        {
            if ( view && view->viewer() )
            {
                views.push_back( view );
            }
        }
    }

    return views;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimGridView*> RimProject::allVisibleGridViews() const
{
    std::vector<RimGridView*> views;
    for ( Rim3dView* view : allVisibleViews() )
    {
        RimGridView* gridView = dynamic_cast<RimGridView*>( view );
        if ( gridView ) views.push_back( gridView );
    }

    return views;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimProject::scheduleCreateDisplayModelAndRedrawAllViews()
{
    for ( RimCase* rimCase : allGridCases() )
    {
        if ( !rimCase ) continue;
        for ( Rim3dView* view : rimCase->views() )
        {
            view->scheduleCreateDisplayModelAndRedraw();
        }
    }

    auto seismicViewCollection = activeOilField()->seismicViewCollection();
    if ( seismicViewCollection )
    {
        for ( auto seisview : seismicViewCollection->views() )
        {
            seisview->scheduleCreateDisplayModelAndRedraw();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimOilField*> RimProject::allOilFields() const
{
    return oilFields.childrenByType();
}

//--------------------------------------------------------------------------------------------------
/// Currently there will be only one oil field in Resinsight, so return hardcoded first oil field
/// from the RimOilField collection.
//--------------------------------------------------------------------------------------------------
RimOilField* RimProject::activeOilField()
{
    CVF_ASSERT( oilFields.size() == 1 );

    return oilFields[0];
}

//--------------------------------------------------------------------------------------------------
/// Currently there will be only one oil field in Resinsight, so return hardcoded first oil field
/// from the RimOilField collection.
//--------------------------------------------------------------------------------------------------
const RimOilField* RimProject::activeOilField() const
{
    CVF_ASSERT( oilFields.size() == 1 );

    return oilFields[0];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimProject::computeUtmAreaOfInterest()
{
    cvf::BoundingBox projectBB;
    for ( RimCase* rimCase : allGridCases() )
    {
        RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>( rimCase );
        if ( eclipseCase && eclipseCase->eclipseCaseData() )
        {
            for ( size_t gridIdx = 0; gridIdx < eclipseCase->eclipseCaseData()->gridCount(); gridIdx++ )
            {
                RigGridBase* rigGrid = eclipseCase->eclipseCaseData()->grid( gridIdx );
                projectBB.add( rigGrid->boundingBox() );
            }
        }
        else
        {
            // Todo : calculate BBox of GeoMechCase
        }
    }

    if ( projectBB.isValid() )
    {
        double north, south, east, west;

        north = projectBB.max().y();
        south = projectBB.min().y();

        west = projectBB.min().x();
        east = projectBB.max().x();

        wellPathImport->north = north;
        wellPathImport->south = south;
        wellPathImport->east  = east;
        wellPathImport->west  = west;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimProject::actionsBasedOnSelection( QMenu& contextMenu )
{
    caf::CmdFeatureMenuBuilder menuBuilder = RimContextCommandBuilder::commandsFromSelection();

    menuBuilder.appendToMenu( &contextMenu );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimProject::show3DWindow() const
{
    return m_show3DWindow;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimProject::showPlotWindow() const
{
    return m_showPlotWindow;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimProject::showPlotWindowOnTop() const
{
    return m_showPlotWindowOnTopOf3DWindow();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::WindowTileMode RimProject::subWindowsTileMode3DWindow() const
{
    return m_subWindowsTileMode3DWindow();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::WindowTileMode RimProject::subWindowsTileModePlotWindow() const
{
    return m_subWindowsTileModePlotWindow();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimProject::setSubWindowsTileMode3DWindow( RiaDefines::WindowTileMode tileMode )
{
    m_subWindowsTileMode3DWindow = tileMode;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimProject::setSubWindowsTileModePlotWindow( RiaDefines::WindowTileMode tileMode )
{
    m_subWindowsTileModePlotWindow = tileMode;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimProject::reloadCompletionTypeResultsInAllViews()
{
    RiaCompletionTypeCalculationScheduler::instance()->clearCompletionTypeResultsInAllCases();
    scheduleCreateDisplayModelAndRedrawAllViews();

    m_mainPlotCollection()->scheduleUpdatePlotsWithCompletions();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimDialogData* RimProject::dialogData() const
{
    return m_dialogData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimEclipseCase*> RimProject::eclipseCases() const
{
    std::vector<RimEclipseCase*> allGridCases;
    for ( const auto& oilField : oilFields )
    {
        const auto& cases = oilField->analysisModels->cases;
        allGridCases.insert( allGridCases.end(), cases.begin(), cases.end() );
    }
    return allGridCases;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseCase* RimProject::eclipseCaseFromGridFileName( const QString& gridFileName ) const
{
    for ( RimEclipseCase* eclCase : eclipseCases() )
    {
        if ( RiaFilePathTools::toInternalSeparator( eclCase->gridFileName() ) == RiaFilePathTools::toInternalSeparator( gridFileName ) )
        {
            return eclCase;
        }
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseCase* RimProject::eclipseCaseFromCaseId( const int caseId ) const
{
    for ( RimEclipseCase* eclCase : eclipseCases() )
    {
        if ( eclCase->caseId() == caseId )
        {
            return eclCase;
        }
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> RimProject::simulationWellNames() const
{
    std::set<QString> wellNames;

    for ( RimOilField* oilField : oilFields )
    {
        auto analysisCaseColl = oilField->analysisModels();
        for ( RimEclipseCase* eclCase : analysisCaseColl->cases() )
        {
            const auto& eclData = eclCase->eclipseCaseData();
            if ( eclData == nullptr ) continue;

            const auto names = eclData->simulationWellNames();
            wellNames.insert( names.begin(), names.end() );
        }
    }
    return std::vector<QString>( wellNames.begin(), wellNames.end() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPath* RimProject::wellPathFromSimWellName( const QString& simWellName, int branchIndex )
{
    for ( RimWellPath* const path : allWellPaths() )
    {
        if ( QString::compare( path->associatedSimulationWellName(), simWellName ) == 0 &&
             ( branchIndex < 0 || path->associatedSimulationWellBranch() == branchIndex ) )
        {
            return path;
        }
    }

    for ( RimWellPath* const path : allWellPaths() )
    {
        if ( QString::compare( path->name(), simWellName ) == 0 )
        {
            return path;
        }
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPath* RimProject::wellPathByName( const QString& wellPathName ) const
{
    for ( RimWellPath* const path : allWellPaths() )
    {
        if ( path->name() == wellPathName ) return path;
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWellPath*> RimProject::allWellPaths() const
{
    std::vector<RimWellPath*> wellPaths;
    for ( const auto& oilField : oilFields() )
    {
        auto wellPathColl = oilField->wellPathCollection();
        for ( auto wellPath : wellPathColl->allWellPaths() )
        {
            wellPaths.push_back( wellPath );
        }
    }
    return wellPaths;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimTextAnnotation*> RimProject::textAnnotations() const
{
    std::vector<RimTextAnnotation*> annotations;

    // 'Global' text annotations
    for ( const auto& oilField : oilFields() )
    {
        auto annotationColl = oilField->annotationCollection();
        for ( const auto& annotation : annotationColl->textAnnotations() )
        {
            annotations.push_back( annotation );
        }
    }

    // 'Local' text annotations
    for ( const auto& view : allVisibleGridViews() )
    {
        std::vector<RimAnnotationInViewCollection*> annotationColls = view->descendantsIncludingThisOfType<RimAnnotationInViewCollection>();

        if ( annotationColls.size() == 1 )
        {
            for ( const auto& annotation : annotationColls.front()->textAnnotations() )
            {
                annotations.push_back( annotation );
            }
        }
    }

    return annotations;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimReachCircleAnnotation*> RimProject::reachCircleAnnotations() const
{
    std::vector<RimReachCircleAnnotation*> annotations;
    for ( const auto& oilField : oilFields() )
    {
        auto annotationColl = oilField->annotationCollection();
        for ( const auto& annotation : annotationColl->reachCircleAnnotations() )
        {
            annotations.push_back( annotation );
        }
    }
    return annotations;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimPolylinesAnnotation*> RimProject::polylineAnnotations() const
{
    std::vector<RimPolylinesAnnotation*> annotations;
    for ( const auto& oilField : oilFields() )
    {
        auto annotationColl = oilField->annotationCollection();
        for ( const auto& annotation : annotationColl->userDefinedPolylineAnnotations() )
        {
            annotations.push_back( annotation );
        }

        for ( const auto& annotation : annotationColl->polylinesFromFileAnnotations() )
        {
            annotations.push_back( annotation );
        }
    }
    return annotations;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimGeoMechCase*> RimProject::geoMechCases() const
{
    std::vector<RimGeoMechCase*> cases;
    for ( RimOilField* oilField : oilFields() )
    {
        if ( !oilField ) continue;

        RimGeoMechModels* geomModels = oilField->geoMechModels();
        if ( geomModels )
        {
            for ( auto acase : geomModels->cases() )
            {
                cases.push_back( acase );
            }
        }
    }
    return cases;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimFractureTemplateCollection*> RimProject::allFractureTemplateCollections() const
{
    std::vector<RimFractureTemplateCollection*> templColls;
    for ( RimOilField* oilField : allOilFields() )
    {
        templColls.push_back( oilField->fractureDefinitionCollection() );
    }
    return templColls;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimFractureTemplate*> RimProject::allFractureTemplates() const
{
    std::vector<RimFractureTemplate*> templates;
    for ( RimFractureTemplateCollection* templColl : allFractureTemplateCollections() )
    {
        for ( RimFractureTemplate* templ : templColl->fractureTemplates() )
        {
            templates.push_back( templ );
        }
    }
    return templates;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimValveTemplateCollection*> RimProject::allValveTemplateCollections() const
{
    std::vector<RimValveTemplateCollection*> templColls;
    for ( RimOilField* oilField : allOilFields() )
    {
        templColls.push_back( oilField->valveTemplateCollection() );
    }
    return templColls;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimValveTemplate*> RimProject::allValveTemplates() const
{
    std::vector<RimValveTemplate*> templates;
    for ( RimValveTemplateCollection* templColl : allValveTemplateCollections() )
    {
        for ( RimValveTemplate* templ : templColl->valveTemplates() )
        {
            templates.push_back( templ );
        }
    }
    return templates;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::AppEnum<RiaDefines::EclipseUnitSystem> RimProject::commonUnitSystemForAllCases() const
{
    RiaDefines::EclipseUnitSystem commonUnitSystem = RiaDefines::EclipseUnitSystem::UNITS_UNKNOWN;

    for ( const auto& c : allGridCases() )
    {
        auto eclipseCase = dynamic_cast<RimEclipseCase*>( c );
        if ( eclipseCase && eclipseCase->eclipseCaseData() )
        {
            if ( commonUnitSystem == RiaDefines::EclipseUnitSystem::UNITS_UNKNOWN )
            {
                commonUnitSystem = eclipseCase->eclipseCaseData()->unitsType();
            }
            else if ( commonUnitSystem != eclipseCase->eclipseCaseData()->unitsType() )
            {
                commonUnitSystem = RiaDefines::EclipseUnitSystem::UNITS_UNKNOWN;
                break;
            }
        }
    }

    return commonUnitSystem;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimMeasurement* RimProject::measurement() const
{
    return activeOilField()->measurement;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPlotTemplateFolderItem* RimProject::rootPlotTemplateItem() const
{
    return m_plotTemplateTopFolder;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<caf::FilePath*> RimProject::allFilePaths() const
{
    std::vector<caf::FilePath*> filePaths;
    fieldContentsByType( this, filePaths );

    return filePaths;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimProject::reloadCompletionTypeResultsForEclipseCase( RimEclipseCase* eclipseCase )
{
    for ( Rim3dView* view : eclipseCase->views() )
    {
        view->scheduleCreateDisplayModelAndRedraw();
    }

    RiaCompletionTypeCalculationScheduler::instance()->scheduleRecalculateCompletionTypeAndRedrawAllViews( { eclipseCase } );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimProject::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/ )
{
    if ( uiConfigName == "PlotWindow.Plots" )
    {
        if ( m_mainPlotCollection )
        {
            if ( m_mainPlotCollection->summaryMultiPlotCollection() )
            {
                uiTreeOrdering.add( m_mainPlotCollection->summaryMultiPlotCollection() );
            }

            if ( m_mainPlotCollection->analysisPlotCollection() )
            {
                uiTreeOrdering.add( m_mainPlotCollection->analysisPlotCollection() );
            }

            if ( m_mainPlotCollection->correlationPlotCollection() )
            {
                uiTreeOrdering.add( m_mainPlotCollection->correlationPlotCollection() );
            }

            if ( m_mainPlotCollection->summaryTableCollection() )
            {
                uiTreeOrdering.add( m_mainPlotCollection->summaryTableCollection() );
            }

            if ( m_mainPlotCollection->wellLogPlotCollection() )
            {
                uiTreeOrdering.add( m_mainPlotCollection->wellLogPlotCollection() );
            }

            if ( m_mainPlotCollection->rftPlotCollection() )
            {
                uiTreeOrdering.add( m_mainPlotCollection->rftPlotCollection() );
            }

            if ( m_mainPlotCollection->pltPlotCollection() )
            {
                uiTreeOrdering.add( m_mainPlotCollection->pltPlotCollection() );
            }

            if ( m_mainPlotCollection->flowPlotCollection() )
            {
                uiTreeOrdering.add( m_mainPlotCollection->flowPlotCollection() );
            }

            if ( m_mainPlotCollection->gridCrossPlotCollection() )
            {
                uiTreeOrdering.add( m_mainPlotCollection->gridCrossPlotCollection() );
            }

            if ( m_mainPlotCollection->saturationPressurePlotCollection() )
            {
                uiTreeOrdering.add( m_mainPlotCollection->saturationPressurePlotCollection() );
            }

            if ( m_mainPlotCollection->multiPlotCollection() )
            {
                uiTreeOrdering.add( m_mainPlotCollection->multiPlotCollection() );
            }

            if ( m_mainPlotCollection->stimPlanModelPlotCollection() )
            {
                uiTreeOrdering.add( m_mainPlotCollection->stimPlanModelPlotCollection() );
            }

            if ( m_mainPlotCollection->vfpPlotCollection() )
            {
                uiTreeOrdering.add( m_mainPlotCollection->vfpPlotCollection() );
            }
#ifdef USE_QTCHARTS
            if ( m_mainPlotCollection->gridStatisticsPlotCollection() || m_mainPlotCollection->ensembleFractureStatisticsPlotCollection() )
            {
                auto statisticsItemCollection = uiTreeOrdering.add( "Statistics Plots", ":/Folder.png" );
                if ( m_mainPlotCollection->gridStatisticsPlotCollection() )
                    statisticsItemCollection->add( m_mainPlotCollection->gridStatisticsPlotCollection() );

                if ( m_mainPlotCollection->ensembleFractureStatisticsPlotCollection() )
                    statisticsItemCollection->add( m_mainPlotCollection->ensembleFractureStatisticsPlotCollection() );
            }
#endif
        }
    }
    else if ( uiConfigName == "PlotWindow.DataSources" )
    {
        RimOilField* oilField = activeOilField();
        if ( oilField )
        {
            if ( oilField->summaryCaseMainCollection() )
            {
                uiTreeOrdering.add( oilField->summaryCaseMainCollection() );
            }
            if ( oilField->observedDataCollection() )
            {
                uiTreeOrdering.add( oilField->observedDataCollection() );
            }
            if ( oilField->ensembleWellLogsCollection() )
            {
                uiTreeOrdering.add( oilField->ensembleWellLogsCollection() );
            }
        }
    }
    else if ( uiConfigName == "PlotWindow.Scripts" || uiConfigName == "MainWindow.Scripts" )
    {
        uiTreeOrdering.add( scriptCollection() );
    }
    else if ( uiConfigName == "PlotWindow.Templates" )
    {
        uiTreeOrdering.add( m_plotTemplateTopFolder );
    }
    else if ( uiConfigName == "MainWindow.DataSources" )
    {
        RimOilField* oilField = activeOilField();
        if ( oilField )
        {
            if ( oilField->analysisModels() ) uiTreeOrdering.add( oilField->analysisModels() );
        }
    }
    else
    {
        // Use object instead of field to avoid duplicate entries in the tree view
        uiTreeOrdering.add( viewLinkerCollection() );

        RimOilField* oilField = activeOilField();
        if ( oilField )
        {
            if ( oilField->analysisModels() ) uiTreeOrdering.add( oilField->analysisModels() );
            if ( oilField->eclipseViewCollection() ) uiTreeOrdering.add( oilField->eclipseViewCollection() );
            if ( oilField->geoMechModels() ) uiTreeOrdering.add( oilField->geoMechModels() );
            if ( oilField->wellPathCollection() ) uiTreeOrdering.add( oilField->wellPathCollection() );
            if ( oilField->polygonCollection() ) uiTreeOrdering.add( oilField->polygonCollection() );
            if ( oilField->surfaceCollection() ) uiTreeOrdering.add( oilField->surfaceCollection() );
            if ( oilField->seismicDataCollection() )
            {
                auto child = uiTreeOrdering.add( "Seismic", ":/Seismic16x16.png" );
                child->add( oilField->seismicDataCollection() );
                child->add( oilField->seismicViewCollection() );
            }
            if ( oilField->formationNamesCollection() ) uiTreeOrdering.add( oilField->formationNamesCollection() );
            if ( oilField->completionTemplateCollection() ) uiTreeOrdering.add( oilField->completionTemplateCollection() );
            if ( oilField->annotationCollection() ) uiTreeOrdering.add( oilField->annotationCollection() );
            if ( oilField->eclipseContourMapCollection() ) uiTreeOrdering.add( oilField->eclipseContourMapCollection() );
        }

        uiTreeOrdering.add( colorLegendCollection() );
    }

    uiTreeOrdering.skipRemainingChildren( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimProject::transferPathsToGlobalPathList()
{
    RiaVariableMapper variableMapper( m_globalPathList() );

    for ( caf::FilePath* filePath : allFilePaths() )
    {
        QString path = filePath->path();
        if ( !path.isEmpty() )
        {
            QString pathId = variableMapper.addPathAndGetId( path );
            filePath->setPath( pathId );
        }
    }

    for ( auto summaryCase : allSummaryCases() )
    {
        if ( summaryCase->displayNameType() == RimCaseDisplayNameTools::DisplayName::CUSTOM )
        {
            // At this point, after the replace of variables into caf::FilePath objects, the variable name is
            // stored in the summary case object. Read out the variable name and append "_name" for custom
            // summary variables.

            QString variableName = summaryCase->summaryHeaderFilename();
            variableName         = variableName.remove( RiaVariableMapper::variableToken() );

            variableName = RiaVariableMapper::variableToken() + variableName + RiaVariableMapper::postfixName() +
                           RiaVariableMapper::variableToken();

            QString variableValue = summaryCase->displayCaseName();
            variableMapper.addVariable( variableName, variableValue );

            summaryCase->setCustomCaseName( variableName );
        }
    }

    for ( auto gridCase : allGridCases() )
    {
        if ( gridCase->displayNameType() == RimCaseDisplayNameTools::DisplayName::CUSTOM )
        {
            // At this point, after the replace of variables into caf::FilePath objects, the variable name is
            // stored in the summary case object. Read out the variable name and append "_name" for custom
            // summary variables.

            QString variableName = gridCase->gridFileName();
            variableName         = variableName.remove( RiaVariableMapper::variableToken() );

            variableName = RiaVariableMapper::variableToken() + variableName + RiaVariableMapper::postfixName() +
                           RiaVariableMapper::variableToken();

            QString variableValue = gridCase->caseUserDescription();
            variableMapper.addVariable( variableName, variableValue );

            gridCase->setCustomCaseName( variableName );
        }
    }

    variableMapper.replaceVariablesInValues();

    m_globalPathList = variableMapper.variableTableAsText();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimProject::distributePathsFromGlobalPathList()
{
    RiaVariableMapper pathListMapper( m_globalPathList() );

    for ( caf::FilePath* filePath : allFilePaths() )
    {
        QString     pathIdCandidate  = filePath->path().trimmed();
        QStringList pathIdComponents = pathIdCandidate.split( RiaVariableMapper::variableToken() );

        if ( pathIdComponents.size() == 3 && pathIdComponents[0].size() == 0 && pathIdComponents[1].size() > 0 &&
             pathIdComponents[2].size() == 0 )
        {
            bool    isFound = false;
            QString path    = pathListMapper.valueForVariable( pathIdCandidate, &isFound );
            if ( isFound )
            {
                filePath->setPath( path );
            }
        }
    }

    for ( auto summaryCase : allSummaryCases() )
    {
        if ( summaryCase->displayNameType() == RimCaseDisplayNameTools::DisplayName::CUSTOM )
        {
            auto variableName = summaryCase->displayCaseName();

            bool    isFound       = false;
            QString variableValue = pathListMapper.valueForVariable( variableName, &isFound );
            if ( isFound )
            {
                summaryCase->setCustomCaseName( variableValue );
            }
            else if ( variableName.contains( RiaVariableMapper::postfixName() + RiaVariableMapper::variableToken() ) )
            {
                // The variable name is not found in the variable list, but the name indicates a variable. Reset
                // to full case name.
                summaryCase->setDisplayNameOption( RimCaseDisplayNameTools::DisplayName::FULL_CASE_NAME );
            }
        }
    }

    for ( auto gridCase : allGridCases() )
    {
        if ( gridCase->displayNameType() == RimCaseDisplayNameTools::DisplayName::CUSTOM )
        {
            auto variableName = gridCase->caseUserDescription();

            bool    isFound       = false;
            QString variableValue = pathListMapper.valueForVariable( variableName, &isFound );
            if ( isFound )
            {
                gridCase->setCustomCaseName( variableValue );
            }
            else if ( variableName.contains( RiaVariableMapper::postfixName() + RiaVariableMapper::variableToken() ) )
            {
                // The variable name is not found in the variable list, but the name indicates a variable. Reset
                // to full case name.
                gridCase->setDisplayNameType( RimCaseDisplayNameTools::DisplayName::FULL_CASE_NAME );
            }
        }
    }
}
