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
#include "RiaVersionInfo.h"

#include "RicfCommandObject.h"
#include "RigEclipseCaseData.h"
#include "RigGridBase.h"

#include "PlotTemplates/RimPlotTemplateFolderItem.h"
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
#include "RimFlowPlotCollection.h"
#include "RimFormationNamesCollection.h"
#include "RimFractureModelPlotCollection.h"
#include "RimFractureTemplate.h"
#include "RimFractureTemplateCollection.h"
#include "RimGeoMechCase.h"
#include "RimGeoMechModels.h"
#include "RimGridCrossPlotCollection.h"
#include "RimGridSummaryCase.h"
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
#include "RimSummaryCalculation.h"
#include "RimSummaryCalculationCollection.h"
#include "RimSummaryCaseCollection.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSummaryCrossPlotCollection.h"
#include "RimSummaryPlotCollection.h"
#include "RimSurfaceCollection.h"
#include "RimTools.h"
#include "RimUserDefinedPolylinesAnnotation.h"
#include "RimValveTemplate.h"
#include "RimValveTemplateCollection.h"
#include "RimViewLinker.h"
#include "RimViewLinkerCollection.h"
#include "RimViewWindow.h"
#include "RimWellLogFile.h"
#include "RimWellLogPlotCollection.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"

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
RimProject::RimProject( void )
    : m_nextValidCaseId( 0 )
    , m_nextValidCaseGroupId( 0 )
    , m_nextValidViewId( 1 )
    , m_nextValidPlotId( 1 )
    , m_nextValidCalculationId( 1 )
    , m_nextValidSummaryCaseId( 1 )
    , m_nextValidEnsembleId( 1 )
{
    CAF_PDM_InitScriptableObjectWithNameAndComment( "Project", "", "", "", "Project", "The ResInsight Project" );

    CAF_PDM_InitField( &m_projectFileVersionString, "ProjectFileVersionString", QString( STRPRODUCTVER ), "", "", "", "" );
    m_projectFileVersionString.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_globalPathList, "ReferencedExternalFiles", "", "", "", "" );
    m_globalPathList.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &oilFields, "OilFields", "Oil Fields", "", "", "" );
    oilFields.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &colorLegendCollection, "ColorLegendCollection", "Color Legend Collection", "", "", "" );
    colorLegendCollection = new RimColorLegendCollection();
    colorLegendCollection->createStandardColorLegends();

    CAF_PDM_InitFieldNoDefault( &scriptCollection, "ScriptCollection", "Octave Scripts", ":/octave.png", "", "" );
    scriptCollection.uiCapability()->setUiHidden( true );
    scriptCollection.xmlCapability()->disableIO();

    CAF_PDM_InitFieldNoDefault( &wellPathImport, "WellPathImport", "WellPathImport", "", "", "" );
    wellPathImport = new RimWellPathImport();
    wellPathImport.uiCapability()->setUiHidden( true );
    wellPathImport.uiCapability()->setUiTreeChildrenHidden( true );

    CAF_PDM_InitFieldNoDefault( &mainPlotCollection, "MainPlotCollection", "Plots", "", "", "" );
    mainPlotCollection.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &viewLinkerCollection,
                                "LinkedViews",
                                "Linked Views (field in RimProject",
                                ":/LinkView16x16.png",
                                "",
                                "" );
    viewLinkerCollection.uiCapability()->setUiHidden( true );
    viewLinkerCollection = new RimViewLinkerCollection;

    CAF_PDM_InitFieldNoDefault( &calculationCollection, "CalculationCollection", "Calculation Collection", "", "", "" );
    calculationCollection = new RimSummaryCalculationCollection;

    CAF_PDM_InitFieldNoDefault( &commandObjects, "CommandObjects", "Command Objects", "", "", "" );
    // wellPathImport.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault( &multiSnapshotDefinitions, "MultiSnapshotDefinitions", "Multi Snapshot Definitions", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &mainWindowTreeViewState, "TreeViewState", "", "", "", "" );
    mainWindowTreeViewState.uiCapability()->setUiHidden( true );
    CAF_PDM_InitFieldNoDefault( &mainWindowCurrentModelIndexPath, "TreeViewCurrentModelIndexPath", "", "", "", "" );
    mainWindowCurrentModelIndexPath.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &plotWindowTreeViewState, "PlotWindowTreeViewState", "", "", "", "" );
    plotWindowTreeViewState.uiCapability()->setUiHidden( true );
    CAF_PDM_InitFieldNoDefault( &plotWindowCurrentModelIndexPath, "PlotWindowTreeViewCurrentModelIndexPath", "", "", "", "" );
    plotWindowCurrentModelIndexPath.uiCapability()->setUiHidden( true );

    CAF_PDM_InitField( &m_show3DWindow, "show3DWindow", true, "Show 3D Window", "", "", "" );
    m_show3DWindow.uiCapability()->setUiHidden( true );

    CAF_PDM_InitField( &m_showPlotWindow, "showPlotWindow", false, "Show Plot Window", "", "", "" );
    m_showPlotWindow.uiCapability()->setUiHidden( true );

    CAF_PDM_InitField( &m_subWindowsTiled3DWindow, "tiled3DWindow", false, "Tile 3D Window", "", "", "" );
    m_subWindowsTiled3DWindow.uiCapability()->setUiHidden( true );

    CAF_PDM_InitField( &m_subWindowsTiledPlotWindow, "tiledPlotWindow", false, "Tile Plot Window", "", "", "" );
    m_subWindowsTiledPlotWindow.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_dialogData, "DialogData", "DialogData", "", "", "" );
    m_dialogData = new RimDialogData();
    m_dialogData.uiCapability()->setUiHidden( true );
    m_dialogData.uiCapability()->setUiTreeChildrenHidden( true );

    // Obsolete fields. The content is moved to OilFields and friends
    CAF_PDM_InitFieldNoDefault( &casesObsolete, "Reservoirs", "", "", "", "" );
    RiaFieldhandleTools::disableWriteAndSetFieldHidden( &casesObsolete );

    CAF_PDM_InitFieldNoDefault( &caseGroupsObsolete, "CaseGroups", "", "", "", "" );
    RiaFieldhandleTools::disableWriteAndSetFieldHidden( &caseGroupsObsolete );

    // Initialization

    scriptCollection = new RimScriptCollection();
    scriptCollection->directory.uiCapability()->setUiHidden( true );
    scriptCollection->uiCapability()->setUiName( "Scripts" );
    scriptCollection->uiCapability()->setUiIconFromResourceString( ":/octave.png" );

    mainPlotCollection = new RimMainPlotCollection();

    CAF_PDM_InitFieldNoDefault( &m_plotTemplateFolderItem, "PlotTemplateCollection", "Plot Templates", "", "", "" );
    m_plotTemplateFolderItem = new RimPlotTemplateFolderItem();
    m_plotTemplateFolderItem.xmlCapability()->disableIO();

    // For now, create a default first oilfield that contains the rest of the project
    oilFields.push_back( new RimOilField );

    this->setUiHidden( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimProject::~RimProject( void )
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
    if ( mainPlotCollection() )
    {
        mainPlotCollection()->deleteAllContainedObjects();
    }

    oilFields.deleteAllChildObjects();
    oilFields.push_back( new RimOilField );

    casesObsolete.deleteAllChildObjects();
    caseGroupsObsolete.deleteAllChildObjects();

    wellPathImport->regions().deleteAllChildObjects();

    commandObjects.deleteAllChildObjects();

    multiSnapshotDefinitions.deleteAllChildObjects();

    m_dialogData->clearProjectSpecificData();

    calculationCollection->deleteAllContainedObjects();
    colorLegendCollection->deleteCustomColorLegends();

    delete viewLinkerCollection->viewLinker();
    viewLinkerCollection->viewLinker = nullptr;

    fileName = "";

    mainWindowCurrentModelIndexPath = "";
    mainWindowTreeViewState         = "";
    plotWindowCurrentModelIndexPath = "";
    plotWindowTreeViewState         = "";

    m_nextValidCaseId        = 0;
    m_nextValidCaseGroupId   = 0;
    m_nextValidViewId        = 1;
    m_nextValidPlotId        = 1;
    m_nextValidCalculationId = 1;
    m_nextValidSummaryCaseId = 1;
    m_nextValidEnsembleId    = 1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimProject::initAfterRead()
{
    this->distributePathsFromGlobalPathList();

    // Create an empty oil field in case the project did not contain one
    if ( oilFields.size() < 1 )
    {
        oilFields.push_back( new RimOilField );
    }

    // Handle old project files with obsolete structure.
    // Move caseGroupsObsolete and casesObsolete to oilFields()[idx]->analysisModels()
    RimEclipseCaseCollection* analysisModels = activeOilField() ? activeOilField()->analysisModels() : nullptr;
    bool                      movedOneRimIdenticalGridCaseGroup = false;
    for ( size_t cgIdx = 0; cgIdx < caseGroupsObsolete.size(); ++cgIdx )
    {
        RimIdenticalGridCaseGroup* sourceCaseGroup = caseGroupsObsolete[cgIdx];
        if ( analysisModels )
        {
            analysisModels->caseGroups.push_back( sourceCaseGroup );
            // printf("Moved m_project->caseGroupsObsolete[%i] to first oil fields analysis models\n", cgIdx);
            movedOneRimIdenticalGridCaseGroup = true; // moved at least one so assume the others will be moved too...
        }
    }

    if ( movedOneRimIdenticalGridCaseGroup )
    {
        caseGroupsObsolete.clear();
    }

    bool movedOneRimCase = false;
    for ( size_t cIdx = 0; cIdx < casesObsolete().size(); ++cIdx )
    {
        if ( analysisModels )
        {
            RimEclipseCase* sourceCase = casesObsolete[cIdx];
            casesObsolete.set( cIdx, nullptr );
            analysisModels->cases.push_back( sourceCase );
            // printf("Moved m_project->casesObsolete[%i] to first oil fields analysis models\n", cIdx);
            movedOneRimCase = true; // moved at least one so assume the others will be moved too...
        }
    }

    if ( movedOneRimCase )
    {
        casesObsolete.clear();
    }

    if ( casesObsolete().size() > 0 || caseGroupsObsolete.size() > 0 )
    {
        // printf("RimProject::initAfterRead: Was not able to move all cases (%i left) or caseGroups (%i left) from
        // Project to analysisModels",
        //  casesObsolete().size(), caseGroupsObsolete.size());
    }

    // Set project pointer to each well path
    for ( size_t oilFieldIdx = 0; oilFieldIdx < oilFields().size(); oilFieldIdx++ )
    {
        RimOilField* oilField = oilFields[oilFieldIdx];
        if ( oilField == nullptr || oilField->wellPathCollection == nullptr ) continue;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimProject::setupBeforeSave()
{
    RiaGuiApplication* guiApp = RiaGuiApplication::instance();

    if ( guiApp )
    {
        m_show3DWindow   = guiApp->mainWindow()->isVisible();
        m_showPlotWindow = guiApp->mainPlotWindow() && guiApp->mainPlotWindow()->isVisible();
    }

    m_projectFileVersionString = STRPRODUCTVER;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimProject::writeProjectFile()
{
    this->transferPathsToGlobalPathList();
    bool couldOpenFile = this->writeFile();
    this->distributePathsFromGlobalPathList();

    return couldOpenFile;
}

//--------------------------------------------------------------------------------------------------
/// Support list of multiple script paths divided by ';'
//--------------------------------------------------------------------------------------------------
void RimProject::setScriptDirectories( const QString& scriptDirectories )
{
    scriptCollection->calcScripts().deleteAllChildObjects();
    scriptCollection->subDirectories().deleteAllChildObjects();

    QStringList pathList = scriptDirectories.split( ';' );
    foreach ( QString path, pathList )
    {
        QDir dir( path );
        if ( !path.isEmpty() && dir.exists() && dir.isReadable() )
        {
            RimScriptCollection* sharedScriptLocation = new RimScriptCollection;
            sharedScriptLocation->directory           = path;
            sharedScriptLocation->setUiName( dir.dirName() );

            sharedScriptLocation->readContentFromDisc();

            scriptCollection->subDirectories.push_back( sharedScriptLocation );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimProject::setPlotTemplateFolders( const QStringList& plotTemplateFolders )
{
    if ( !m_plotTemplateFolderItem() )
    {
        m_plotTemplateFolderItem = new RimPlotTemplateFolderItem();
    }

    m_plotTemplateFolderItem->createRootFolderItemsFromFolderPaths( plotTemplateFolders );
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

    return !RiaProjectFileVersionTools::isCandidateVersionNewerThanOther( candidateProjectFileVersion,
                                                                          otherProjectFileVersion );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimProject::setProjectFileNameAndUpdateDependencies( const QString& projectFileName )
{
    // Extract the filename of the project file when it was saved
    QString oldProjectFileName = this->fileName;
    // Replace with the new actual filename
    this->fileName = projectFileName;

    QFileInfo fileInfo( projectFileName );
    QString   newProjectPath = fileInfo.path();

    QFileInfo fileInfoOld( oldProjectFileName );
    QString   oldProjectPath = fileInfoOld.path();

    std::vector<caf::FilePath*> filePaths;
    fieldContentsByType( this, filePaths );

    for ( caf::FilePath* filePath : filePaths )
    {
        bool                 foundFile = false;
        std::vector<QString> searchedPaths;

        QString newFilePath =
            RimTools::relocateFile( filePath->path(), newProjectPath, oldProjectPath, &foundFile, &searchedPaths );
        filePath->setPath( newFilePath );
    }

    // Loop over all cases and update file path

    std::vector<RimCase*> cases;
    allCases( cases );
    for ( size_t i = 0; i < cases.size(); i++ )
    {
        cases[i]->updateFilePathsFromProjectPath( newProjectPath, oldProjectPath );
    }

    for ( RimSummaryCase* summaryCase : allSummaryCases() )
    {
        summaryCase->updateFilePathsFromProjectPath( newProjectPath, oldProjectPath );
    }

    // Update path to well path file cache
    for ( RimOilField* oilField : oilFields )
    {
        if ( oilField == nullptr ) continue;
        if ( oilField->wellPathCollection() != nullptr )
        {
            oilField->wellPathCollection()->updateFilePathsFromProjectPath( newProjectPath, oldProjectPath );
        }
        if ( oilField->formationNamesCollection() != nullptr )
        {
            oilField->formationNamesCollection()->updateFilePathsFromProjectPath( newProjectPath, oldProjectPath );
        }
        if ( oilField->summaryCaseMainCollection() != nullptr )
        {
            oilField->summaryCaseMainCollection()->updateFilePathsFromProjectPath( newProjectPath, oldProjectPath );
        }

        CVF_ASSERT( oilField->fractureDefinitionCollection() );
        oilField->fractureDefinitionCollection()->updateFilePathsFromProjectPath( newProjectPath, oldProjectPath );
    }

    {
        std::vector<RimWellLogFile*> rimWellLogFiles;
        this->descendantsIncludingThisOfType( rimWellLogFiles );

        for ( auto rimWellLogFile : rimWellLogFiles )
        {
            rimWellLogFile->updateFilePathsFromProjectPath( newProjectPath, oldProjectPath );
        }
    }

    wellPathImport->updateFilePaths();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimProject::assignCaseIdToSummaryCase( RimSummaryCase* summaryCase )
{
    if ( summaryCase )
    {
        std::vector<RimSummaryCase*> summaryCases = allSummaryCases();
        for ( RimSummaryCase* s : summaryCases )
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
        std::vector<RimSummaryCaseCollection*> summaryGroups = RimProject::summaryGroups();
        for ( RimSummaryCaseCollection* s : summaryGroups )
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

    // TODO: Move code from allCases here
    allCases( cases );

    return cases;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimProject::assignCaseIdToCase( RimCase* reservoirCase )
{
    if ( reservoirCase )
    {
        std::vector<RimCase*> cases;
        this->descendantsIncludingThisOfType( cases );

        for ( RimCase* rimCase : cases )
        {
            m_nextValidCaseId = std::max( m_nextValidCaseId, rimCase->caseId() + 1 );
        }

        reservoirCase->caseId = m_nextValidCaseId++;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimProject::assignIdToCaseGroup( RimIdenticalGridCaseGroup* caseGroup )
{
    if ( caseGroup )
    {
        std::vector<RimIdenticalGridCaseGroup*> identicalCaseGroups;
        this->descendantsIncludingThisOfType( identicalCaseGroups );

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
        std::vector<Rim3dView*> views;
        this->descendantsIncludingThisOfType( views );

        for ( Rim3dView* existingView : views )
        {
            m_nextValidViewId = std::max( m_nextValidViewId, existingView->id() + 1 );
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
        std::vector<RimPlotWindow*> plotWindows;
        this->descendantsIncludingThisOfType( plotWindows );

        for ( RimPlotWindow* existingPlotWindow : plotWindows )
        {
            m_nextValidPlotId = std::max( m_nextValidPlotId, existingPlotWindow->id() + 1 );
        }

        plotWindow->setId( m_nextValidPlotId++ );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimProject::assignCalculationIdToCalculation( RimSummaryCalculation* calculation )
{
    if ( calculation )
    {
        for ( RimSummaryCalculation* existingCalculation : calculationCollection->calculations() )
        {
            m_nextValidCalculationId = std::max( m_nextValidCalculationId, existingCalculation->id() + 1 );
        }

        calculation->setId( m_nextValidCalculationId++ );
    }
}

//--------------------------------------------------------------------------------------------------
/// TODO: This function is deprecated, use allGridCases()
//--------------------------------------------------------------------------------------------------
void RimProject::allCases( std::vector<RimCase*>& cases ) const
{
    for ( size_t oilFieldIdx = 0; oilFieldIdx < oilFields().size(); oilFieldIdx++ )
    {
        RimOilField* oilField = oilFields[oilFieldIdx];
        if ( !oilField ) continue;

        RimEclipseCaseCollection* analysisModels = oilField->analysisModels();
        if ( analysisModels )
        {
            for ( size_t caseIdx = 0; caseIdx < analysisModels->cases.size(); caseIdx++ )
            {
                cases.push_back( analysisModels->cases[caseIdx] );
            }
            for ( size_t cgIdx = 0; cgIdx < analysisModels->caseGroups.size(); cgIdx++ )
            {
                // Load the Main case of each IdenticalGridCaseGroup
                RimIdenticalGridCaseGroup* cg = analysisModels->caseGroups[cgIdx];
                if ( cg == nullptr ) continue;

                if ( cg->statisticsCaseCollection() )
                {
                    for ( size_t caseIdx = 0; caseIdx < cg->statisticsCaseCollection()->reservoirs.size(); caseIdx++ )
                    {
                        cases.push_back( cg->statisticsCaseCollection()->reservoirs[caseIdx] );
                    }
                }
                if ( cg->caseCollection() )
                {
                    for ( size_t caseIdx = 0; caseIdx < cg->caseCollection()->reservoirs.size(); caseIdx++ )
                    {
                        cases.push_back( cg->caseCollection()->reservoirs[caseIdx] );
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
            sumCases.insert( sumCases.end(), allSummaryCases.begin(), allSummaryCases.end() );
        }

        auto observedDataColl = oilField->observedDataCollection();
        if ( observedDataColl != nullptr && observedDataColl->allObservedSummaryData().size() > 0 )
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
void RimProject::allNotLinkedViews( std::vector<RimGridView*>& views )
{
    std::vector<RimCase*> cases;
    allCases( cases );

    std::vector<RimGridView*> alreadyLinkedViews;
    if ( viewLinkerCollection->viewLinker() )
    {
        viewLinkerCollection->viewLinker()->allViews( alreadyLinkedViews );
    }

    for ( size_t caseIdx = 0; caseIdx < cases.size(); caseIdx++ )
    {
        RimCase* rimCase = cases[caseIdx];
        if ( !rimCase ) continue;

        std::vector<Rim3dView*> caseViews = rimCase->views();
        for ( size_t viewIdx = 0; viewIdx < caseViews.size(); viewIdx++ )
        {
            RimGridView* gridView = dynamic_cast<RimGridView*>( caseViews[viewIdx] );

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
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimProject::allViews( std::vector<Rim3dView*>& views ) const
{
    std::vector<RimCase*> cases;
    allCases( cases );

    for ( size_t caseIdx = 0; caseIdx < cases.size(); caseIdx++ )
    {
        RimCase* rimCase = cases[caseIdx];
        if ( !rimCase ) continue;

        std::vector<Rim3dView*> caseViews = rimCase->views();
        for ( size_t viewIdx = 0; viewIdx < caseViews.size(); viewIdx++ )
        {
            if ( caseViews[viewIdx] )
            {
                views.push_back( caseViews[viewIdx] );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimProject::allVisibleViews( std::vector<Rim3dView*>& views ) const
{
    std::vector<RimCase*> cases;
    allCases( cases );

    for ( size_t caseIdx = 0; caseIdx < cases.size(); caseIdx++ )
    {
        RimCase* rimCase = cases[caseIdx];
        if ( !rimCase ) continue;

        std::vector<Rim3dView*> caseViews = rimCase->views();
        for ( size_t viewIdx = 0; viewIdx < caseViews.size(); viewIdx++ )
        {
            if ( caseViews[viewIdx] && caseViews[viewIdx]->viewer() )
            {
                views.push_back( caseViews[viewIdx] );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimProject::allVisibleGridViews( std::vector<RimGridView*>& views ) const
{
    std::vector<Rim3dView*> visibleViews;
    this->allVisibleViews( visibleViews );
    for ( Rim3dView* view : visibleViews )
    {
        RimGridView* gridView = dynamic_cast<RimGridView*>( view );
        if ( gridView ) views.push_back( gridView );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimProject::scheduleCreateDisplayModelAndRedrawAllViews()
{
    std::vector<RimCase*> cases;
    allCases( cases );
    for ( size_t caseIdx = 0; caseIdx < cases.size(); caseIdx++ )
    {
        RimCase* rimCase = cases[caseIdx];
        if ( rimCase == nullptr ) continue;
        std::vector<Rim3dView*> views = rimCase->views();

        for ( size_t viewIdx = 0; viewIdx < views.size(); viewIdx++ )
        {
            views[viewIdx]->scheduleCreateDisplayModelAndRedraw();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimProject::allOilFields( std::vector<RimOilField*>& allOilFields ) const
{
    allOilFields.clear();
    for ( const auto& oilField : this->oilFields )
    {
        allOilFields.push_back( oilField );
    }
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
    std::vector<RimCase*> cases;
    allCases( cases );

    cvf::BoundingBox projectBB;

    for ( size_t i = 0; i < cases.size(); i++ )
    {
        RimEclipseCase* rimCase = dynamic_cast<RimEclipseCase*>( cases[i] );

        if ( rimCase && rimCase->eclipseCaseData() )
        {
            for ( size_t gridIdx = 0; gridIdx < rimCase->eclipseCaseData()->gridCount(); gridIdx++ )
            {
                RigGridBase* rigGrid = rimCase->eclipseCaseData()->grid( gridIdx );

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
bool RimProject::subWindowsTiled3DWindow() const
{
    return m_subWindowsTiled3DWindow;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimProject::subWindowsTiledPlotWindow() const
{
    return m_subWindowsTiledPlotWindow;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimProject::setSubWindowsTiledIn3DWindow( bool tiled )
{
    m_subWindowsTiled3DWindow = tiled;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimProject::setSubWindowsTiledInPlotWindow( bool tiled )
{
    m_subWindowsTiledPlotWindow = tiled;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimProject::reloadCompletionTypeResultsInAllViews()
{
    scheduleCreateDisplayModelAndRedrawAllViews();
    RiaCompletionTypeCalculationScheduler::instance()->scheduleRecalculateCompletionTypeAndRedrawAllViews();

    this->mainPlotCollection()->updatePlotsWithCompletions();
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
    std::vector<RimEclipseCase*> allCases;
    for ( const auto& oilField : oilFields )
    {
        const auto& cases = oilField->analysisModels->cases;
        allCases.insert( allCases.end(), cases.begin(), cases.end() );
    }
    return allCases;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseCase* RimProject::eclipseCaseFromGridFileName( const QString& gridFileName ) const
{
    for ( RimEclipseCase* eclCase : eclipseCases() )
    {
        if ( RiaFilePathTools::toInternalSeparator( eclCase->gridFileName() ) ==
             RiaFilePathTools::toInternalSeparator( gridFileName ) )
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
    std::vector<RimWellPath*> paths;
    for ( const auto& oilField : oilFields() )
    {
        auto wellPathColl = oilField->wellPathCollection();
        for ( const auto& path : wellPathColl->wellPaths )
        {
            paths.push_back( path );
        }
    }
    return paths;
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
    std::vector<RimGridView*> visibleViews;
    allVisibleGridViews( visibleViews );
    for ( const auto& view : visibleViews )
    {
        std::vector<RimAnnotationInViewCollection*> annotationColls;
        view->descendantsIncludingThisOfType( annotationColls );

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

    for ( size_t oilFieldIdx = 0; oilFieldIdx < oilFields().size(); oilFieldIdx++ )
    {
        RimOilField* oilField = oilFields[oilFieldIdx];
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
    std::vector<RimOilField*>                   rimOilFields;

    allOilFields( rimOilFields );
    for ( RimOilField* oilField : rimOilFields )
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
    std::vector<RimOilField*>                rimOilFields;

    allOilFields( rimOilFields );
    for ( RimOilField* oilField : rimOilFields )
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
RiaEclipseUnitTools::UnitSystemType RimProject::commonUnitSystemForAllCases() const
{
    std::vector<RimCase*> rimCases;
    allCases( rimCases );

    RiaEclipseUnitTools::UnitSystem commonUnitSystem = RiaEclipseUnitTools::UnitSystem::UNITS_UNKNOWN;

    for ( const auto& c : rimCases )
    {
        auto eclipseCase = dynamic_cast<RimEclipseCase*>( c );
        if ( eclipseCase && eclipseCase->eclipseCaseData() )
        {
            if ( commonUnitSystem == RiaEclipseUnitTools::UnitSystem::UNITS_UNKNOWN )
            {
                commonUnitSystem = eclipseCase->eclipseCaseData()->unitsType();
            }
            else if ( commonUnitSystem != eclipseCase->eclipseCaseData()->unitsType() )
            {
                commonUnitSystem = RiaEclipseUnitTools::UnitSystem::UNITS_UNKNOWN;
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
RimPlotTemplateFolderItem* RimProject::rootPlotTemlateItem() const
{
    return m_plotTemplateFolderItem;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimProject::reloadCompletionTypeResultsForEclipseCase( RimEclipseCase* eclipseCase )
{
    std::vector<Rim3dView*> views = eclipseCase->views();

    for ( size_t viewIdx = 0; viewIdx < views.size(); viewIdx++ )
    {
        views[viewIdx]->scheduleCreateDisplayModelAndRedraw();
    }

    RiaCompletionTypeCalculationScheduler::instance()->scheduleRecalculateCompletionTypeAndRedrawAllViews( eclipseCase );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimProject::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/ )
{
    if ( uiConfigName == "PlotWindow" )
    {
        {
            auto itemCollection = uiTreeOrdering.add( "Cases and Data", ":/Folder.png" );

            RimOilField* oilField = activeOilField();
            if ( oilField )
            {
                if ( oilField->summaryCaseMainCollection() )
                {
                    itemCollection->add( oilField->summaryCaseMainCollection() );
                }
                if ( oilField->observedDataCollection() )
                {
                    itemCollection->add( oilField->observedDataCollection() );
                }
            }
        }

        if ( mainPlotCollection )
        {
            auto itemCollection = uiTreeOrdering.add( "Plots", ":/Folder.png" );
            if ( mainPlotCollection->summaryPlotCollection() )
            {
                itemCollection->add( mainPlotCollection->summaryPlotCollection() );
            }

            if ( mainPlotCollection->analysisPlotCollection() )
            {
                itemCollection->add( mainPlotCollection->analysisPlotCollection() );
            }

            if ( mainPlotCollection->correlationPlotCollection() )
            {
                itemCollection->add( mainPlotCollection->correlationPlotCollection() );
            }

            if ( mainPlotCollection->summaryCrossPlotCollection() )
            {
                itemCollection->add( mainPlotCollection->summaryCrossPlotCollection() );
            }

            if ( mainPlotCollection->wellLogPlotCollection() )
            {
                itemCollection->add( mainPlotCollection->wellLogPlotCollection() );
            }

            if ( mainPlotCollection->rftPlotCollection() )
            {
                itemCollection->add( mainPlotCollection->rftPlotCollection() );
            }

            if ( mainPlotCollection->pltPlotCollection() )
            {
                itemCollection->add( mainPlotCollection->pltPlotCollection() );
            }

            if ( mainPlotCollection->flowPlotCollection() )
            {
                itemCollection->add( mainPlotCollection->flowPlotCollection() );
            }

            if ( mainPlotCollection->gridCrossPlotCollection() )
            {
                itemCollection->add( mainPlotCollection->gridCrossPlotCollection() );
            }

            if ( mainPlotCollection->saturationPressurePlotCollection() )
            {
                itemCollection->add( mainPlotCollection->saturationPressurePlotCollection() );
            }

            if ( mainPlotCollection->multiPlotCollection() &&
                 !mainPlotCollection->multiPlotCollection()->multiPlots().empty() )
            {
                itemCollection->add( mainPlotCollection->multiPlotCollection() );
            }

            if ( mainPlotCollection->fractureModelPlotCollection() )
            {
                itemCollection->add( mainPlotCollection->fractureModelPlotCollection() );
            }
        }

        uiTreeOrdering.add( scriptCollection() );
    }
    else
    {
        if ( viewLinkerCollection()->viewLinker() )
        {
            // Use object instead of field to avoid duplicate entries in the tree view
            uiTreeOrdering.add( viewLinkerCollection() );
        }

        RimOilField* oilField = activeOilField();
        if ( oilField )
        {
            if ( oilField->analysisModels() ) uiTreeOrdering.add( oilField->analysisModels() );
            if ( oilField->geoMechModels() ) uiTreeOrdering.add( oilField->geoMechModels() );
            if ( oilField->wellPathCollection() ) uiTreeOrdering.add( oilField->wellPathCollection() );
            if ( oilField->surfaceCollection() ) uiTreeOrdering.add( oilField->surfaceCollection() );
            if ( oilField->formationNamesCollection() ) uiTreeOrdering.add( oilField->formationNamesCollection() );
            if ( oilField->completionTemplateCollection() )
                uiTreeOrdering.add( oilField->completionTemplateCollection() );
            if ( oilField->annotationCollection() ) uiTreeOrdering.add( oilField->annotationCollection() );
        }

        uiTreeOrdering.add( colorLegendCollection() );
        uiTreeOrdering.add( scriptCollection() );
    }

    uiTreeOrdering.skipRemainingChildren( true );
}

#define PATHIDCHAR "$"

class GlobalPathListMapper
{
    const QString pathIdBaseString = "PathId_";

public:
    GlobalPathListMapper( const QString& globalPathListTable )
    {
        m_maxUsedIdNumber     = 0;
        QStringList pathPairs = globalPathListTable.split( ";", QString::SkipEmptyParts );

        for ( const QString& pathIdPathPair : pathPairs )
        {
            QStringList pathIdPathComponents = pathIdPathPair.trimmed().split( PATHIDCHAR );

            if ( pathIdPathComponents.size() == 3 && pathIdPathComponents[0].size() == 0 )
            {
                QString pathIdCore = pathIdPathComponents[1];
                QString pathId     = PATHIDCHAR + pathIdCore + PATHIDCHAR;
                QString path       = pathIdPathComponents[2].trimmed();

                // Check if we have a standard id, and store the max number

                if ( pathIdCore.startsWith( pathIdBaseString ) )
                {
                    bool    isOk       = false;
                    QString numberText = pathIdCore.right( pathIdCore.size() - pathIdBaseString.size() );
                    size_t  idNumber   = numberText.toUInt( &isOk );

                    if ( isOk )
                    {
                        m_maxUsedIdNumber = std::max( m_maxUsedIdNumber, idNumber );
                    }
                }

                // Check for unique pathId
                {
                    auto pathIdPathPairIt = m_oldPathIdToPathMap.find( pathId );

                    if ( pathIdPathPairIt != m_oldPathIdToPathMap.end() )
                    {
                        // Error: pathID is already used
                    }
                }

                // Check for multiple identical paths
                {
                    auto pathPathIdPairIt = m_oldPathToPathIdMap.find( path );

                    if ( pathPathIdPairIt != m_oldPathToPathIdMap.end() )
                    {
                        // Warning: path has already been assigned a pathId
                    }
                }

                m_oldPathIdToPathMap[pathId] = path;
                m_oldPathToPathIdMap[path]   = pathId;
            }
            else
            {
                // Error: The text is ill formatted
            }
        }
    }

    QString addPathAndGetId( const QString& path )
    {
        // Want to re-use ids from last save to avoid unnecessary changes and make the behavior predictable
        QString pathId;
        QString trimmedPath = path.trimmed();

        auto pathToIdIt = m_oldPathToPathIdMap.find( trimmedPath );
        if ( pathToIdIt != m_oldPathToPathIdMap.end() )
        {
            pathId = pathToIdIt->second;
        }
        else
        {
            auto pathPathIdPairIt = m_newPathToPathIdMap.find( trimmedPath );
            if ( pathPathIdPairIt != m_newPathToPathIdMap.end() )
            {
                pathId = pathPathIdPairIt->second;
            }
            else
            {
                pathId = createUnusedId();
            }
        }

        m_newPathIdToPathMap[pathId]      = trimmedPath;
        m_newPathToPathIdMap[trimmedPath] = pathId;

        return pathId;
    };

    QString newGlobalPathListTable() const
    {
        QString pathList;
        pathList += "\n";
        for ( const auto& pathIdPathPairIt : m_newPathIdToPathMap )
        {
            pathList += "        " + pathIdPathPairIt.first + " " + pathIdPathPairIt.second + ";\n";
        }

        pathList += "    ";

        return pathList;
    }

    QString pathFromPathId( const QString& pathId, bool* isFound ) const
    {
        auto it = m_oldPathIdToPathMap.find( pathId );
        if ( it != m_oldPathIdToPathMap.end() )
        {
            ( *isFound ) = true;
            return it->second;
        }

        ( *isFound ) = false;
        return "";
    }

private:
    QString createUnusedId()
    {
        m_maxUsedIdNumber++;

        QString numberString   = QString( "%1" ).arg( (uint)m_maxUsedIdNumber, 3, 10, QChar( '0' ) );
        QString pathIdentifier = PATHIDCHAR + pathIdBaseString + numberString + PATHIDCHAR;

        return pathIdentifier;
    }

    size_t m_maxUsedIdNumber; // Set when parsing the globalPathListTable. Increment while creating new id's

    std::map<QString, QString> m_newPathIdToPathMap;
    std::map<QString, QString> m_newPathToPathIdMap;

    std::map<QString, QString> m_oldPathIdToPathMap;
    std::map<QString, QString> m_oldPathToPathIdMap;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimProject::transferPathsToGlobalPathList()
{
    GlobalPathListMapper pathListMapper( m_globalPathList() );

    std::vector<caf::FilePath*> filePaths;
    fieldContentsByType( this, filePaths );

    for ( caf::FilePath* filePath : filePaths )
    {
        QString path = filePath->path();
        if ( !path.isEmpty() )
        {
            QString pathId = pathListMapper.addPathAndGetId( path );
            filePath->setPath( pathId );
        }
    }

    m_globalPathList = pathListMapper.newGlobalPathListTable();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimProject::distributePathsFromGlobalPathList()
{
    GlobalPathListMapper pathListMapper( m_globalPathList() );

    std::vector<caf::FilePath*> filePaths;
    fieldContentsByType( this, filePaths );

    for ( caf::FilePath* filePath : filePaths )
    {
        QString     pathIdCandidate  = filePath->path().trimmed();
        QStringList pathIdComponents = pathIdCandidate.split( PATHIDCHAR );

        if ( pathIdComponents.size() == 3 && pathIdComponents[0].size() == 0 && pathIdComponents[1].size() > 0 &&
             pathIdComponents[2].size() == 0 )
        {
            bool    isFound = false;
            QString path    = pathListMapper.pathFromPathId( pathIdCandidate, &isFound );
            if ( isFound )
            {
                filePath->setPath( path );
            }
            else
            {
                // The pathId can not be found in the path list
            }
        }
        else
        {
            // The pathIdCandidate is probably a real path. Leave alone.
        }
    }
}
