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

#include "RimWellPathCollection.h"

#include "RiaColorTables.h"
#include "RiaGuiApplication.h"
#include "RiaLogging.h"
#include "RiaPreferences.h"
#include "RiaTextStringTools.h"
#include "RiaWellNameComparer.h"

#include "RifWellPathFormationsImporter.h"
#include "RifWellPathImporter.h"

#include "RigEclipseCaseData.h"
#include "RigMainGrid.h"
#include "RigWellPath.h"

#include "RimEclipseCase.h"
#include "RimEclipseCaseCollection.h"
#include "RimEclipseView.h"
#include "RimFileWellPath.h"
#include "RimModeledWellPath.h"
#include "RimOilField.h"
#include "RimPerforationCollection.h"
#include "RimProject.h"
#include "RimStimPlanModel.h"
#include "RimStimPlanModelCollection.h"
#include "RimWellLogFile.h"
#include "RimWellMeasurementCollection.h"
#include "RimWellPath.h"
#include "RimWellPathCompletionSettings.h"
#include "RimWellPathNode.h"
#include "RimWellPathTieIn.h"

#include "Riu3DMainWindowTools.h"

#include "cafPdmUiEditorHandle.h"
#include "cafPdmUiTreeOrdering.h"
#include "cafProgressInfo.h"

#include <QFile>
#include <QFileInfo>
#include <QString>

#include <cmath>
#include <fstream>

namespace caf
{
template <>
void RimWellPathCollection::WellVisibilityEnum::setUp()
{
    addItem( RimWellPathCollection::FORCE_ALL_OFF, "FORCE_ALL_OFF", "Off" );
    addItem( RimWellPathCollection::ALL_ON, "ALL_ON", "Individual" );
    addItem( RimWellPathCollection::FORCE_ALL_ON, "FORCE_ALL_ON", "On" );
}
} // namespace caf

CAF_PDM_SOURCE_INIT( RimWellPathCollection, "WellPaths" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPathCollection::RimWellPathCollection()
{
    CAF_PDM_InitObject( "Wells", ":/WellCollection.png", "", "" );

    CAF_PDM_InitField( &isActive, "Active", true, "Active", "", "", "" );
    isActive.uiCapability()->setUiHidden( true );

    CAF_PDM_InitField( &showWellPathLabel, "ShowWellPathLabel", true, "Show Well Path Labels", "", "", "" );

    cvf::Color3f defWellLabelColor = RiaPreferences::current()->defaultWellLabelColor();
    CAF_PDM_InitField( &wellPathLabelColor, "WellPathLabelColor", defWellLabelColor, "Well label color", "", "", "" );

    CAF_PDM_InitField( &wellPathVisibility,
                       "GlobalWellPathVisibility",
                       WellVisibilityEnum( ALL_ON ),
                       "Global Well Path Visibility",
                       "",
                       "",
                       "" );

    CAF_PDM_InitField( &wellPathRadiusScaleFactor, "WellPathRadiusScale", 0.1, "Well Path Radius Scale", "", "", "" );
    CAF_PDM_InitField( &wellPathCrossSectionVertexCount, "WellPathVertexCount", 12, "Well Path Vertex Count", "", "", "" );
    wellPathCrossSectionVertexCount.xmlCapability()->disableIO();
    wellPathCrossSectionVertexCount.uiCapability()->setUiHidden( true );

    CAF_PDM_InitField( &wellPathClip, "WellPathClip", true, "Clip Well Paths", "", "", "" );
    CAF_PDM_InitField( &wellPathClipZDistance, "WellPathClipZDistance", 100, "Well Path Clipping Depth Distance", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_wellPaths, "WellPaths", "Well Paths", "", "", "" );
    m_wellPaths.uiCapability()->setUiHidden( true );
    m_wellPaths.uiCapability()->setUiTreeHidden( true );
    m_wellPaths.uiCapability()->setUiTreeChildrenHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_wellMeasurements, "WellMeasurements", "Measurements", "", "", "" );
    m_wellMeasurements = new RimWellMeasurementCollection;
    m_wellMeasurements.uiCapability()->setUiTreeHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_wellPathNodes, "WellPathNodes", "Well Path Nodes", "", "", "" );

    m_wellPathImporter           = std::make_unique<RifWellPathImporter>();
    m_wellPathFormationsImporter = std::make_unique<RifWellPathFormationsImporter>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPathCollection::~RimWellPathCollection()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathCollection::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                              const QVariant&            oldValue,
                                              const QVariant&            newValue )
{
    scheduleRedrawAffectedViews();
}

//--------------------------------------------------------------------------------------------------
/// Read files containing well path data, or create geometry based on the targets
//--------------------------------------------------------------------------------------------------
void RimWellPathCollection::loadDataAndUpdate()
{
    caf::ProgressInfo progress( m_wellPaths.size(), "Reading well paths from file" );

    readWellPathFormationFiles();

    for ( RimWellPath* wellPath : allWellPaths() )
    {
        progress.setProgressDescription( QString( "Reading file %1" ).arg( wellPath->name() ) );

        RimFileWellPath*    fWPath = dynamic_cast<RimFileWellPath*>( wellPath );
        RimModeledWellPath* mWPath = dynamic_cast<RimModeledWellPath*>( wellPath );
        if ( fWPath )
        {
            if ( !fWPath->filePath().isEmpty() )
            {
                QString errorMessage;
                if ( !fWPath->readWellPathFile( &errorMessage, m_wellPathImporter.get(), false ) )
                {
                    RiaLogging::warning( errorMessage );
                }
            }
        }
        else if ( mWPath )
        {
            mWPath->createWellPathGeometry();
        }

        if ( wellPath )
        {
            for ( RimWellLogFile* const wellLogFile : wellPath->wellLogFiles() )
            {
                if ( wellLogFile )
                {
                    QString errorMessage;
                    if ( !wellLogFile->readFile( &errorMessage ) )
                    {
                        RiaLogging::warning( errorMessage );
                    }
                }
            }

            RimStimPlanModelCollection* stimPlanModelCollection = wellPath->stimPlanModelCollection();
            if ( stimPlanModelCollection )
            {
                for ( RimStimPlanModel* stimPlanModel : stimPlanModelCollection->allStimPlanModels() )
                {
                    stimPlanModel->loadDataAndUpdate();
                }
            }
        }
        progress.incrementProgress();
    }

    rebuildWellPathNodes();

    this->sortWellsByName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWellPath*>
    RimWellPathCollection::addWellPaths( QStringList filePaths, bool importGrouped, QStringList* errorMessages )
{
    CAF_ASSERT( errorMessages );

    std::vector<RimFileWellPath*> wellPathArray;

    for ( QString filePath : filePaths )
    {
        // Check if this file is already open
        bool alreadyOpen = false;
        for ( auto wellPath : m_wellPaths )
        {
            RimFileWellPath* fWPath = dynamic_cast<RimFileWellPath*>( wellPath.p() );
            if ( !fWPath ) continue;

            QFile f1;
            f1.setFileName( filePath );
            QString s1 = f1.fileName();
            QFile   f2;
            f2.setFileName( fWPath->filePath() );
            QString s2 = f2.fileName();
            if ( s1 == s2 )
            {
                // printf("Attempting to open well path JSON file that is already open:\n  %s\n", (const char*)
                // filePath.toLocal8Bit());
                alreadyOpen = true;
                errorMessages->push_back( QString( "%1 is already loaded" ).arg( filePath ) );
                break;
            }
        }

        if ( !alreadyOpen )
        {
            QFileInfo fi( filePath );

            if ( fi.suffix().compare( "json" ) == 0 )
            {
                RimFileWellPath* wellPath = new RimFileWellPath();
                wellPath->setFilepath( filePath );
                wellPathArray.push_back( wellPath );
            }
            else
            {
                // Create Well path objects for all the paths in the assumed ascii file
                size_t wellPathCount = m_wellPathImporter->wellDataCount( filePath );
                for ( size_t i = 0; i < wellPathCount; ++i )
                {
                    RimFileWellPath* wellPath = new RimFileWellPath();
                    wellPath->setFilepath( filePath );
                    wellPath->setWellPathIndexInFile( static_cast<int>( i ) );
                    wellPathArray.push_back( wellPath );
                }
            }
        }
    }

    readAndAddWellPaths( wellPathArray, importGrouped );
    CAF_ASSERT( wellPathArray.empty() );

    scheduleRedrawAffectedViews();
    updateAllRequiredEditors();

    return topLevelWellPaths();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathCollection::addWellPath( gsl::not_null<RimWellPath*> wellPath, bool importGrouped )
{
    m_wellPaths.push_back( wellPath );

    rebuildWellPathNodes();

    m_mostRecentlyUpdatedWellPath = wellPath;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWellPath*> RimWellPathCollection::topLevelWellPaths() const
{
    return m_wellPaths.childObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWellPath*> RimWellPathCollection::allWellPaths() const
{
    std::vector<RimWellPath*> wellPaths;
    descendantsOfType( wellPaths );
    return wellPaths;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathCollection::readAndAddWellPaths( std::vector<RimFileWellPath*>& wellPathArray, bool importGrouped )
{
    caf::ProgressInfo progress( wellPathArray.size(), "Reading well paths from file" );

    std::vector<RimWellPath*> wellPathsToGroup;
    for ( size_t wpIdx = 0; wpIdx < wellPathArray.size(); wpIdx++ )
    {
        RimFileWellPath* wellPath = wellPathArray[wpIdx];
        wellPath->readWellPathFile( nullptr, m_wellPathImporter.get(), true );

        progress.setProgressDescription( QString( "Reading file %1" ).arg( wellPath->name() ) );

        // If a well path with this name exists already, make it read the well path file
        RimFileWellPath* existingWellPath = dynamic_cast<RimFileWellPath*>( tryFindMatchingWellPath( wellPath->name() ) );

        if ( existingWellPath )
        {
            existingWellPath->setFilepath( wellPath->filePath() );
            existingWellPath->setWellPathIndexInFile( wellPath->wellPathIndexInFile() );
            existingWellPath->readWellPathFile( nullptr, m_wellPathImporter.get(), true );

            // Let name from well path file override name from well log file
            existingWellPath->setName( wellPath->name() );

            m_mostRecentlyUpdatedWellPath = existingWellPath;
            delete wellPath;
        }
        else
        {
            wellPath->setWellPathColor( RiaColorTables::wellPathsPaletteColors().cycledColor3f( m_wellPaths.size() ) );
            wellPath->setUnitSystem( findUnitSystemForWellPath( wellPath ) );
            addWellPath( wellPath, false );

            wellPathsToGroup.push_back( wellPath );
        }

        progress.incrementProgress();
    }
    wellPathArray.clear(); // This should not be used again. We may have deleted items

    groupWellPaths( wellPathsToGroup );
    sortWellsByName();
    rebuildWellPathNodes();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathCollection::addWellPaths( const std::vector<RimWellPath*> incomingWellPaths, bool importGrouped )
{
    for ( const auto& wellPath : incomingWellPaths )
    {
        addWellPath( wellPath, importGrouped );
    }

    groupWellPaths( incomingWellPaths );
    sortWellsByName();
    rebuildWellPathNodes();

    updateAllRequiredEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWellLogFile*> RimWellPathCollection::addWellLogs( const QStringList& filePaths, QStringList* errorMessages )
{
    CAF_ASSERT( errorMessages );

    std::vector<RimWellLogFile*> logFileInfos;

    foreach ( QString filePath, filePaths )
    {
        QString         errorMessage;
        RimWellLogFile* logFileInfo = RimWellLogFile::readWellLogFile( filePath, &errorMessage );
        if ( !errorMessage.isEmpty() )
        {
            errorMessages->push_back( errorMessage );
        }
        if ( logFileInfo )
        {
            RimWellPath* wellPath = tryFindMatchingWellPath( logFileInfo->wellName() );
            if ( !wellPath )
            {
                wellPath = new RimWellPath();
                addWellPath( wellPath, false );
            }

            wellPath->addWellLogFile( logFileInfo );
            logFileInfos.push_back( logFileInfo );
        }
    }

    this->sortWellsByName();
    updateAllRequiredEditors();

    return logFileInfos;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathCollection::addWellPathFormations( const QStringList& filePaths )
{
    QString outputMessage = "Well Picks Import\n";
    outputMessage += "-----------------------------------------------\n";
    outputMessage += "Well Name \tDetected Well Path \tCount\n";

    bool fileReadSuccess = false;

    for ( QString filePath : filePaths )
    {
        std::map<QString, cvf::ref<RigWellPathFormations>> newFormations =
            m_wellPathFormationsImporter->readWellPathFormationsFromPath( filePath );

        for ( auto it = newFormations.begin(); it != newFormations.end(); it++ )
        {
            fileReadSuccess = true;

            RimWellPath* wellPath = tryFindMatchingWellPath( it->first );
            if ( !wellPath )
            {
                wellPath = new RimWellPath();
                wellPath->setName( it->first );
                addWellPath( wellPath, false );
                RiaLogging::info( QString( "Created new well: %1" ).arg( wellPath->name() ) );
            }
            wellPath->setFormationsGeometry( it->second );

            QString wellFormationsCount = QString( "%1" ).arg( it->second->formationNamesCount() );

            m_mostRecentlyUpdatedWellPath = wellPath;

            outputMessage += it->first + "\t\t";
            outputMessage += wellPath->name() + " \t\t\t";
            outputMessage += wellFormationsCount + "\n";
        }
    }
    outputMessage += "-----------------------------------------------";

    if ( fileReadSuccess )
    {
        RiaLogging::errorInMessageBox( Riu3DMainWindowTools::mainWindowWidget(), "Well Picks Import", outputMessage );
    }

    this->sortWellsByName();
    updateAllRequiredEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathCollection::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    caf::PdmUiGroup* wellHeadGroup = uiOrdering.addNewGroup( "Well labels" );
    wellHeadGroup->add( &showWellPathLabel );
    wellHeadGroup->add( &wellPathLabelColor );

    caf::PdmUiGroup* wellPipe = uiOrdering.addNewGroup( "Well pipe" );
    wellPipe->add( &wellPathVisibility );
    wellPipe->add( &wellPathRadiusScaleFactor );

    caf::PdmUiGroup* advancedGroup = uiOrdering.addNewGroup( "Clipping" );
    advancedGroup->add( &wellPathClip );
    advancedGroup->add( &wellPathClipZDistance );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathCollection::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/ )
{
    if ( !m_wellMeasurements->isEmpty() )
    {
        uiTreeOrdering.add( &m_wellMeasurements );
    }

    for ( const auto& wellPathNode : m_wellPathNodes() )
    {
        RimWellPathCollection::buildUiTreeOrdering( wellPathNode, &uiTreeOrdering, uiConfigName );
    }

    uiTreeOrdering.skipRemainingChildren( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimWellPathCollection::objectToggleField()
{
    return &isActive;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathCollection::scheduleRedrawAffectedViews()
{
    RimProject* proj;
    this->firstAncestorOrThisOfType( proj );
    if ( proj ) proj->reloadCompletionTypeResultsInAllViews();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathCollection::updateFilePathsFromProjectPath( const QString& newProjectPath, const QString& oldProjectPath )
{
    for ( auto wellPath : m_wellPaths )
    {
        wellPath->updateFilePathsFromProjectPath( newProjectPath, oldProjectPath );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellPathCollection::anyWellsContainingPerforationIntervals() const
{
    for ( auto wellPath : m_wellPaths )
    {
        if ( !wellPath->perforationIntervalCollection()->perforations().empty() ) return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RimWellPathCollection::modelledWellPathCount() const
{
    size_t count = 0;
    for ( auto wellPath : m_wellPaths )
    {
        if ( dynamic_cast<const RimModeledWellPath*>( wellPath.p() ) )
        {
            count++;
        }
    }
    return count;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPath* RimWellPathCollection::wellPathByName( const QString& wellPathName ) const
{
    for ( auto wellPath : m_wellPaths )
    {
        if ( wellPath->name() == wellPathName )
        {
            return wellPath;
        }
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPath* RimWellPathCollection::tryFindMatchingWellPath( const QString& wellName ) const
{
    QString matchedWellPath = RiaWellNameComparer::tryFindMatchingWellPath( wellName );

    return !matchedWellPath.isEmpty() ? wellPathByName( matchedWellPath ) : nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathCollection::deleteAllWellPaths()
{
    m_wellPaths.deleteAllChildObjects();
    m_wellPathNodes.deleteAllChildObjects();

    m_wellPathImporter->clear();
    updateAllRequiredEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathCollection::groupWellPaths( const std::vector<RimWellPath*>& wellPaths )
{
    auto rootWells = wellPathsForWellNameStem( wellPaths );

    for ( auto [name, wellPathCommonName] : rootWells )
    {
        for ( auto wellPath : wellPathCommonName )
        {
            auto wellPathGeometry = wellPath->wellPathGeometry();
            if ( wellPathGeometry )
            {
                const double                   eps = 1.0e-2;
                std::map<RimWellPath*, double> wellPathsWithCommonGeometry;

                for ( auto existingWellPath : wellPathCommonName )
                {
                    if ( existingWellPath == wellPath ) continue;

                    if ( wellPath->name() < existingWellPath->name() ) continue;

                    double identicalTubeLength =
                        existingWellPath->wellPathGeometry()->identicalTubeLength( *wellPathGeometry );
                    if ( identicalTubeLength > eps )
                    {
                        wellPathsWithCommonGeometry[existingWellPath] = identicalTubeLength;
                    }
                }

                RimWellPath* mostSimilarWellPath        = nullptr;
                double       longestIdenticalTubeLength = 0.0;
                for ( auto [existingWellPath, identicalTubeLength] : wellPathsWithCommonGeometry )
                {
                    if ( existingWellPath && ( existingWellPath != wellPath ) &&
                         identicalTubeLength > longestIdenticalTubeLength )
                    {
                        mostSimilarWellPath        = existingWellPath;
                        longestIdenticalTubeLength = identicalTubeLength;
                    }
                }

                if ( mostSimilarWellPath )
                {
                    if ( wellPath->name() > mostSimilarWellPath->name() )
                    {
                        wellPath->connectWellPaths( mostSimilarWellPath, longestIdenticalTubeLength );
                    }
                    else
                    {
                        mostSimilarWellPath->connectWellPaths( wellPath, longestIdenticalTubeLength );
                    }
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPath* RimWellPathCollection::mostRecentlyUpdatedWellPath()
{
    return m_mostRecentlyUpdatedWellPath;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathCollection::readWellPathFormationFiles()
{
    caf::ProgressInfo progress( m_wellPaths.size(), "Reading well picks from file" );

    for ( size_t wpIdx = 0; wpIdx < m_wellPaths.size(); wpIdx++ )
    {
        QString errorMessage;
        if ( !m_wellPaths[wpIdx]->readWellPathFormationsFile( &errorMessage, m_wellPathFormationsImporter.get() ) )
        {
            RiaLogging::errorInMessageBox( Riu3DMainWindowTools::mainWindowWidget(), "File open error", errorMessage );
        }

        progress.setProgressDescription( QString( "Reading formation file %1" ).arg( wpIdx ) );
        progress.incrementProgress();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathCollection::reloadAllWellPathFormations()
{
    caf::ProgressInfo progress( m_wellPaths.size(), "Reloading well picks from file" );

    for ( auto wellPath : m_wellPaths )
    {
        QString errorMessage;
        if ( !wellPath->reloadWellPathFormationsFile( &errorMessage, m_wellPathFormationsImporter.get() ) )
        {
            RiaLogging::errorInMessageBox( Riu3DMainWindowTools::mainWindowWidget(), "File open error", errorMessage );
        }

        progress.setProgressDescription( QString( "Reloading formation file for %1" ).arg( wellPath->name() ) );
        progress.incrementProgress();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathCollection::removeWellPath( gsl::not_null<RimWellPath*> wellPath )
{
    RimFileWellPath* fileWellPath = dynamic_cast<RimFileWellPath*>( wellPath.get() );
    if ( fileWellPath )
    {
        bool isFilePathUsed = false;
        for ( auto wellPath : m_wellPaths )
        {
            RimFileWellPath* fWPath = dynamic_cast<RimFileWellPath*>( wellPath.p() );
            if ( fWPath && fWPath->filePath() == fileWellPath->filePath() )
            {
                isFilePathUsed = true;
                break;
            }
        }

        if ( !isFilePathUsed )
        {
            // One file can have multiple well paths
            // If no other well paths are referencing the filepath, remove cached data from the file reader
            m_wellPathImporter->removeFilePath( fileWellPath->filePath() );
        }
    }
    updateAllRequiredEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool lessWellPath( const caf::PdmPointer<RimWellPath>& w1, const caf::PdmPointer<RimWellPath>& w2 )
{
    if ( w1.notNull() && w2.notNull() )
        return ( w1->name() < w2->name() );
    else if ( w1.notNull() )
        return true;
    else
        return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathCollection::sortWellsByName()
{
    std::sort( m_wellPaths.begin(), m_wellPaths.end(), lessWellPath );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::AppEnum<RiaDefines::EclipseUnitSystem> RimWellPathCollection::findUnitSystemForWellPath( const RimWellPath* wellPath )
{
    RimProject* project;
    firstAncestorOrThisOfTypeAsserted( project );
    if ( project->activeOilField()->analysisModels->cases.empty() )
    {
        return RiaDefines::EclipseUnitSystem::UNITS_UNKNOWN;
    }

    const RigEclipseCaseData* eclipseCaseData = project->activeOilField()->analysisModels->cases()[0]->eclipseCaseData();
    cvf::BoundingBox          caseBoundingBox = eclipseCaseData->mainGrid()->boundingBox();
    cvf::BoundingBox          wellPathBoundingBox;
    for ( const auto& wellPathPoint : wellPath->wellPathGeometry()->wellPathPoints() )
    {
        wellPathBoundingBox.add( wellPathPoint );
    }

    if ( caseBoundingBox.intersects( wellPathBoundingBox ) )
    {
        return eclipseCaseData->unitsType();
    }
    return RiaDefines::EclipseUnitSystem::UNITS_UNKNOWN;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathCollection::rebuildWellPathNodes()
{
    m_wellPathNodes.deleteAllChildObjects();

    std::map<QString, std::vector<RimWellPath*>> rootWells = wellPathsForWellNameStem( m_wellPaths.childObjects() );
    for ( auto [groupName, wellPathGroup] : rootWells )
    {
        if ( groupName == unGroupedText() )
        {
            // For single wells, create well paths directly in the well collection folder

            for ( auto wellPath : wellPathGroup )
            {
                if ( !wellPath ) continue;

                auto node = addWellToWellNode( nullptr, wellPath );
                m_wellPathNodes.push_back( node );
            }
        }
        else
        {
            // Create a group node with group name and put related wells into this group

            auto rootNode = new RimWellPathNode;
            rootNode->setUiName( groupName );
            rootNode->setUiIconFromResourceString( ":/WellPathGroup.svg" );
            m_wellPathNodes.push_back( rootNode );

            for ( auto wellPath : wellPathsWithNoParent( wellPathGroup ) )
            {
                if ( !wellPath ) continue;

                addWellToWellNode( rootNode, wellPath );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPathNode* RimWellPathCollection::addWellToWellNode( RimWellPathNode* parent, RimWellPath* wellPath )
{
    if ( wellPath == nullptr ) return nullptr;

    std::vector<RimWellPath*> wellPaths = connectedWellPathLaterals( wellPath );

    auto node = new RimWellPathNode;
    node->setReferencedObject( wellPath );
    if ( parent ) parent->addChild( node );

    for ( auto w : wellPaths )
    {
        addWellToWellNode( node, w );
    }

    return node;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWellPath*> RimWellPathCollection::wellPathsWithNoParent( const std::vector<RimWellPath*>& sourceWellPaths ) const
{
    std::vector<RimWellPath*> wellPaths;

    for ( const auto& w : sourceWellPaths )
    {
        if ( !w ) continue;

        if ( w->isTopLevelWellPath() )
        {
            wellPaths.push_back( w );
        }
    }

    return wellPaths;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWellPath*> RimWellPathCollection::connectedWellPathLaterals( const RimWellPath* parentWellPath ) const
{
    if ( !parentWellPath ) return {};

    std::vector<RimWellPath*> wellPaths;

    for ( const auto& w : m_wellPaths )
    {
        if ( !w ) continue;

        if ( w->wellPathTieIn() && ( w->wellPathTieIn()->parentWell() == parentWellPath ) )
        {
            wellPaths.push_back( w );
        }
    }

    return wellPaths;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<QString, std::vector<RimWellPath*>>
    RimWellPathCollection::wellPathsForWellNameStem( const std::vector<RimWellPath*>& sourceWellPaths ) const
{
    std::map<QString, std::vector<RimWellPath*>> rootWells;

    QString multiLateralWellPathPattern = RiaPreferences::current()->multiLateralWellNamePattern();
    QRegExp re( multiLateralWellPathPattern, Qt::CaseInsensitive, QRegExp::Wildcard );

    for ( auto wellPath : sourceWellPaths )
    {
        QString name = wellPath->name();
        if ( re.exactMatch( name ) )
        {
            int indexOfLateralStart = name.indexOf( 'Y' );
            if ( indexOfLateralStart > 0 )
            {
                QString rootWellName = wellPath->name().left( indexOfLateralStart );

                rootWells[rootWellName].push_back( wellPath );
            }
        }
        else
        {
            rootWells[RimWellPathCollection::unGroupedText()].push_back( wellPath );
        }
    }

    return rootWells;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathCollection::buildUiTreeOrdering( RimWellPathNode*        wellPathNode,
                                                 caf::PdmUiTreeOrdering* parentUiTreeNode,
                                                 const QString&          uiConfigName )
{
    CAF_ASSERT( wellPathNode );
    CAF_ASSERT( parentUiTreeNode );

    if ( auto obj = wellPathNode->referencedObject() )
    {
        // Create the standard uiTreeOrdering with no relationship to parent using the helper function in
        // PdmUiObjectHandle. This will ensure that any child objects will be created correctly including any
        // recursive children. If a PdmUiTreeOrdering is created directly based on the PdmObject, andy child objects
        // must be added manually.

        auto uiTreeNode = obj->uiCapability()->uiTreeOrdering( uiConfigName );
        parentUiTreeNode->appendChild( uiTreeNode );

        // Build additional child nodes recursively
        for ( auto childNode : wellPathNode->childNodes() )
        {
            buildUiTreeOrdering( childNode, uiTreeNode, uiConfigName );
        }
    }
    else
    {
        // If no referenced object is attached, fallback to the node. Not sure if this is the best solution
        auto uiTreeNode = parentUiTreeNode->add( wellPathNode );

        // Build additional child nodes recursively
        for ( auto childNode : wellPathNode->childNodes() )
        {
            buildUiTreeOrdering( childNode, uiTreeNode, uiConfigName );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellPathCollection::unGroupedText()
{
    return "UnGrouped";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellMeasurementCollection* RimWellPathCollection::measurementCollection()
{
    return m_wellMeasurements;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RimWellMeasurementCollection* RimWellPathCollection::measurementCollection() const
{
    return m_wellMeasurements;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathCollection::onChildDeleted( caf::PdmChildArrayFieldHandle*      childArray,
                                            std::vector<caf::PdmObjectHandle*>& referringObjects )
{
    rebuildWellPathNodes();

    scheduleRedrawAffectedViews();
    uiCapability()->updateConnectedEditors();
}
