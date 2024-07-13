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

#include "Cloud/RiaOsduConnector.h"
#include "RiaColorTables.h"
#include "RiaDateStringParser.h"
#include "RiaGuiApplication.h"
#include "RiaLogging.h"
#include "RiaPreferences.h"
#include "RiaTextStringTools.h"
#include "RiaWellNameComparer.h"

#include "RifOsduWellLogReader.h"
#include "RifOsduWellPathReader.h"
#include "RifWellPathFormationsImporter.h"
#include "RifWellPathImporter.h"

#include "RigEclipseCaseData.h"
#include "RigMainGrid.h"
#include "RigOsduWellLogData.h"
#include "RigWellPath.h"

#include "RimEclipseCase.h"
#include "RimEclipseCaseCollection.h"
#include "RimEclipseView.h"
#include "RimFileWellPath.h"
#include "RimModeledWellPath.h"
#include "RimOilField.h"
#include "RimOsduWellLog.h"
#include "RimOsduWellPath.h"
#include "RimPerforationCollection.h"
#include "RimProject.h"
#include "RimStimPlanModel.h"
#include "RimStimPlanModelCollection.h"
#include "RimWellLogLasFile.h"
#include "RimWellMeasurementCollection.h"
#include "RimWellPath.h"
#include "RimWellPathCompletionSettings.h"
#include "RimWellPathTieIn.h"

#include "RiuMainWindow.h"

#include "cafTreeNode.h" // TODO: Move to caf

#include "Riu3DMainWindowTools.h"

#include "cafDataLoadController.h"
#include "cafDataLoader.h"
#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"
#include "cafPdmUiEditorHandle.h"
#include "cafPdmUiTreeOrdering.h"
#include "cafProgressInfo.h"

#include <QCollator>
#include <QFile>
#include <QFileInfo>
#include <QString>

#include <cmath>
#include <fstream>
#include <memory>

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
    CAF_PDM_InitScriptableObjectWithNameAndComment( "Wells", ":/WellCollection.png", "", "", "WellPathCollection", "Collection of Well Paths" );

    CAF_PDM_InitField( &isActive, "Active", true, "Active" );
    isActive.uiCapability()->setUiHidden( true );

    CAF_PDM_InitField( &showWellPathLabel, "ShowWellPathLabel", true, "Show Well Path Labels" );

    cvf::Color3f defWellLabelColor = RiaPreferences::current()->defaultWellLabelColor();
    CAF_PDM_InitField( &wellPathLabelColor, "WellPathLabelColor", defWellLabelColor, "Well label color" );

    CAF_PDM_InitField( &wellPathVisibility, "GlobalWellPathVisibility", WellVisibilityEnum( ALL_ON ), "Global Well Path Visibility" );

    CAF_PDM_InitField( &wellPathRadiusScaleFactor, "WellPathRadiusScale", 0.1, "Well Path Radius Scale" );
    CAF_PDM_InitField( &wellPathCrossSectionVertexCount, "WellPathVertexCount", 12, "Well Path Vertex Count" );
    wellPathCrossSectionVertexCount.xmlCapability()->disableIO();
    wellPathCrossSectionVertexCount.uiCapability()->setUiHidden( true );

    CAF_PDM_InitField( &wellPathClip, "WellPathClip", true, "Clip Well Paths" );
    CAF_PDM_InitField( &wellPathClipZDistance, "WellPathClipZDistance", 100, "Well Path Clipping Depth Distance" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_wellPaths, "WellPaths", "Well Paths" );
    m_wellPaths.uiCapability()->setUiTreeChildrenHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_wellMeasurements, "WellMeasurements", "Measurements" );
    m_wellMeasurements = new RimWellMeasurementCollection;

    CAF_PDM_InitFieldNoDefault( &m_wellPathNodes, "WellPathNodes", "Well Path Nodes" );
    m_wellPathNodes.xmlCapability()->disableIO();

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
void RimWellPathCollection::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    scheduleRedrawAffectedViews();
}

//--------------------------------------------------------------------------------------------------
/// Read files containing well path data, or create geometry based on the targets
//--------------------------------------------------------------------------------------------------
void RimWellPathCollection::loadDataAndUpdate()
{
    caf::ProgressInfo progress( 3, "Reading well paths from file" );

    readWellPathFormationFiles();

    auto hasOsduData = []( const std::vector<RimWellPath*>& wellPaths ) -> bool
    {
        for ( RimWellPath* wellPath : wellPaths )
        {
            if ( dynamic_cast<RimOsduWellPath*>( wellPath ) )
            {
                return true;
            }
        }

        return false;
    };

    if ( hasOsduData( allWellPaths() ) )
    {
        auto osduConnector = RiaApplication::instance()->makeOsduConnector();
        osduConnector->requestTokenBlocking();
    }

    caf::DataLoadController* dataLoadController = caf::DataLoadController::instance();

    const QString wellPathGeometryKeyword = "WELL_PATH_GEOMETRY";
    const QString wellLogKeyword          = "WELL_LOG";

    progress.setProgressDescription( QString( "Reading well path geometry." ) );
    for ( RimWellPath* wellPath : allWellPaths() )
    {
        dataLoadController->loadData( *wellPath, wellPathGeometryKeyword, progress );
    }
    dataLoadController->blockUntilDone( wellPathGeometryKeyword );
    progress.incrementProgress();

    progress.setProgressDescription( QString( "Reading well logs." ) );
    for ( RimWellPath* wellPath : allWellPaths() )
    {
        for ( RimWellLog* wellLog : wellPath->wellLogs() )
        {
            dataLoadController->loadData( *wellLog, wellLogKeyword, progress );
        }
    }
    dataLoadController->blockUntilDone( wellLogKeyword );
    progress.incrementProgress();

    progress.setProgressDescription( QString( "Reading additional data." ) );
    for ( RimWellPath* wellPath : allWellPaths() )
    {
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

    rebuildWellPathNodes();

    sortWellsByName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWellPath*> RimWellPathCollection::addWellPaths( QStringList filePaths, QStringList* errorMessages )
{
    CAF_ASSERT( errorMessages );

    std::vector<RimFileWellPath*> wellPathArray;

    for ( const QString& filePath : filePaths )
    {
        // Check if this file is already open
        bool alreadyOpen = false;
        for ( const auto& wellPath : m_wellPaths )
        {
            auto* fWPath = dynamic_cast<RimFileWellPath*>( wellPath.p() );
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
                auto* wellPath = new RimFileWellPath();
                wellPath->setFilepath( filePath );
                wellPathArray.push_back( wellPath );
            }
            else
            {
                // Create Well path objects for all the paths in the assumed ascii file
                size_t wellPathCount = m_wellPathImporter->wellDataCount( filePath );
                for ( size_t i = 0; i < wellPathCount; ++i )
                {
                    auto* wellPath = new RimFileWellPath();
                    wellPath->setFilepath( filePath );
                    wellPath->setWellPathIndexInFile( static_cast<int>( i ) );
                    wellPathArray.push_back( wellPath );
                }
            }
        }
    }

    readAndAddWellPaths( wellPathArray );
    CAF_ASSERT( wellPathArray.empty() );

    scheduleRedrawAffectedViews();
    updateAllRequiredEditors();

    return allWellPaths();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathCollection::addWellPath( gsl::not_null<RimWellPath*> wellPath )
{
    m_wellPaths.push_back( wellPath );

    rebuildWellPathNodes();

    m_mostRecentlyUpdatedWellPath = wellPath;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWellPath*> RimWellPathCollection::allWellPaths() const
{
    return m_wellPaths.childrenByType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathCollection::readAndAddWellPaths( std::vector<RimFileWellPath*>& wellPathArray )
{
    caf::ProgressInfo progress( wellPathArray.size(), "Reading well paths from file" );

    for ( RimFileWellPath* wellPath : wellPathArray )
    {
        wellPath->readWellPathFile( nullptr, m_wellPathImporter.get(), true );

        progress.setProgressDescription( QString( "Reading file %1" ).arg( wellPath->name() ) );

        // If a well path with this name exists already, make it read the well path file. This is useful if a well log
        // file has been imported before a well path file containing the full geometry for the well path.
        // NB! Do not use tryFindMatchingWellPath(), as this function will remove the prefix and will return an false
        // match in many cases.
        auto* existingWellPath = dynamic_cast<RimFileWellPath*>( wellPathByName( wellPath->name() ) );
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
            addWellPath( wellPath );
        }

        progress.incrementProgress();
    }
    wellPathArray.clear(); // This should not be used again. We may have deleted items

    groupWellPaths( allWellPaths() );
    sortWellsByName();
    rebuildWellPathNodes();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathCollection::addWellPaths( const std::vector<RimWellPath*> incomingWellPaths )
{
    for ( const auto& wellPath : incomingWellPaths )
    {
        addWellPath( wellPath );
    }

    groupWellPaths( allWellPaths() );
    sortWellsByName();
    rebuildWellPathNodes();

    updateAllRequiredEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWellLogLasFile*> RimWellPathCollection::addWellLogs( const QStringList& filePaths, QStringList* errorMessages )
{
    CAF_ASSERT( errorMessages );

    std::vector<RimWellLogLasFile*> logFileInfos;

    foreach ( QString filePath, filePaths )
    {
        QString            errorMessage;
        RimWellLogLasFile* logFileInfo = RimWellLogLasFile::readWellLogFile( filePath, &errorMessage );
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
                addWellPath( wellPath );
            }

            wellPath->addWellLog( logFileInfo );
            logFileInfos.push_back( logFileInfo );
        }
    }

    sortWellsByName();
    updateAllRequiredEditors();

    return logFileInfos;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathCollection::addWellLog( RimWellLog* wellLog, RimWellPath* wellPath )
{
    wellPath->addWellLog( wellLog );
    sortWellsByName();
    updateAllRequiredEditors();
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

    for ( const QString& filePath : filePaths )
    {
        std::map<QString, cvf::ref<RigWellPathFormations>> newFormations =
            m_wellPathFormationsImporter->readWellPathFormationsFromPath( filePath );

        for ( const auto& newFormation : newFormations )
        {
            fileReadSuccess = true;

            RimWellPath* wellPath = tryFindMatchingWellPath( newFormation.first );
            if ( !wellPath )
            {
                wellPath = new RimWellPath();
                wellPath->setName( newFormation.first );
                addWellPath( wellPath );
                RiaLogging::info( QString( "Created new well: %1" ).arg( wellPath->name() ) );
            }
            wellPath->setFormationsGeometry( newFormation.second );

            QString wellFormationsCount = QString( "%1" ).arg( newFormation.second->formationNamesCount() );

            m_mostRecentlyUpdatedWellPath = wellPath;

            outputMessage += newFormation.first + "\t\t";
            outputMessage += wellPath->name() + " \t\t\t";
            outputMessage += wellFormationsCount + "\n";
        }
    }
    outputMessage += "-----------------------------------------------";

    if ( fileReadSuccess )
    {
        RiaLogging::errorInMessageBox( Riu3DMainWindowTools::mainWindowWidget(), "Well Picks Import", outputMessage );
    }

    sortWellsByName();
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
    RimProject* proj = RimProject::current();
    if ( proj ) proj->reloadCompletionTypeResultsInAllViews();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellPathCollection::anyWellsContainingPerforationIntervals() const
{
    for ( const auto& wellPath : m_wellPaths )
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
    for ( const auto& wellPath : m_wellPaths )
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
    for ( const auto& wellPath : m_wellPaths )
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
    m_wellPaths.deleteChildren();
    m_wellPathNodes.deleteChildren();

    m_wellPathImporter->clear();
    updateAllRequiredEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathCollection::deleteWell( RimWellPath* wellPath )
{
    removeWellPath( wellPath );

    m_wellPaths.removeChild( wellPath );
    delete wellPath;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathCollection::groupWellPaths( const std::vector<RimWellPath*>& wellPaths )
{
    const auto& rootWells = wellPathsForWellNameStem( wellPaths );

    for ( const auto& [groupName, wellPathsInGroup] : rootWells )
    {
        if ( groupName == unGroupedText() ) continue;

        for ( const auto& wellPathToConnect : wellPathsInGroup )
        {
            // Assign the group names as well name for export
            wellPathToConnect->completionSettings()->setWellNameForExport( groupName );

            const auto wellPathGeometry = wellPathToConnect->wellPathGeometry();
            if ( wellPathGeometry )
            {
                std::map<RimWellPath*, double> sharedWellPathLengths;

                for ( const auto& otherWellPath : wellPathsInGroup )
                {
                    if ( otherWellPath == wellPathToConnect ) continue;

                    if ( otherWellPath && otherWellPath->wellPathGeometry() )
                    {
                        const double sharedWellPathLength = otherWellPath->wellPathGeometry()->identicalTubeLength( *wellPathGeometry );
                        const double eps                  = 1.0e-2;
                        if ( sharedWellPathLength > eps )
                        {
                            sharedWellPathLengths[otherWellPath] = sharedWellPathLength;
                        }
                    }
                }

                RimWellPath* longestSharedWellPath       = nullptr;
                double       longestSharedWellPathLength = 0.0;
                for ( const auto& [wellPathCandidate, sharedWellPathLength] : sharedWellPathLengths )
                {
                    if ( wellPathCandidate )
                    {
                        const double distanceDifference = fabs( sharedWellPathLength - longestSharedWellPathLength );

                        const double differenceThreshold = 1.0;
                        if ( longestSharedWellPath && ( distanceDifference < differenceThreshold ) )
                        {
                            // If the main well is named WELL_A, each side steps of a MSW can be given the following names
                            // WELL_A_Y1
                            // WELL_A_Y2
                            // WELL_A_Y3
                            //
                            // If Y3 has equal shared geometry with both Y2 and Y1, make sure that Y3 is connected to Y1.
                            if ( wellPathCandidate->name() < longestSharedWellPath->name() )
                            {
                                longestSharedWellPath       = wellPathCandidate;
                                longestSharedWellPathLength = sharedWellPathLength;
                            }
                        }
                        else if ( sharedWellPathLength > longestSharedWellPathLength )
                        {
                            longestSharedWellPath       = wellPathCandidate;
                            longestSharedWellPathLength = sharedWellPathLength;
                        }
                    }
                }

                if ( longestSharedWellPath )
                {
                    if ( wellPathToConnect->name() > longestSharedWellPath->name() )
                    {
                        wellPathToConnect->connectWellPaths( longestSharedWellPath, longestSharedWellPathLength );
                    }
                    else
                    {
                        longestSharedWellPath->connectWellPaths( wellPathToConnect, longestSharedWellPathLength );
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

    for ( const auto& wellPath : m_wellPaths )
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
void RimWellPathCollection::removeWellPath( gsl::not_null<RimWellPath*> wellToRemove )
{
    auto* fileWellToRemove = dynamic_cast<RimFileWellPath*>( wellToRemove.get() );
    if ( fileWellToRemove )
    {
        bool isFilePathUsed = false;
        for ( const auto& well : m_wellPaths )
        {
            auto fileWell = dynamic_cast<RimFileWellPath*>( well.p() );
            if ( fileWell == fileWellToRemove ) continue;

            if ( fileWell && fileWell->filePath() == fileWellToRemove->filePath() )
            {
                isFilePathUsed = true;
                break;
            }
        }

        if ( !isFilePathUsed )
        {
            // One file can have multiple well paths
            // If no other well paths are referencing the filepath, remove cached data from the file reader
            m_wellPathImporter->removeFilePath( fileWellToRemove->filePath() );
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
    {
        // Use QCollator to sort integer values in addition to text

        QCollator collator;
        collator.setNumericMode( true );
        return collator.compare( w1->name(), w2->name() ) < 0;
    }

    if ( w1.notNull() ) return true;

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
    RimProject* project = RimProject::current();
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

    if ( wellPathBoundingBox.isValid() && caseBoundingBox.isValid() && caseBoundingBox.intersects( wellPathBoundingBox ) )
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
    m_wellPathNodes.deleteChildren();

    std::map<QString, std::vector<RimWellPath*>> rootWells = wellPathsForWellNameStem( m_wellPaths.childrenByType() );
    for ( auto [groupName, wellPathGroup] : rootWells )
    {
        if ( groupName == unGroupedText() )
        {
            // For single wells, create well paths directly in the well collection folder

            for ( auto wellPath : wellPathGroup )
            {
                if ( !wellPath ) continue;
                if ( !wellPath->isTopLevelWellPath() ) continue;

                auto node = addWellToWellNode( nullptr, wellPath );
                m_wellPathNodes.push_back( node );
            }
        }
        else
        {
            // Create a group node with group name and put related wells into this group

            auto rootNode = new cafNamedTreeNode;
            rootNode->setName( groupName );
            rootNode->setIcon( ":/WellPathGroup.svg" );
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
cafTreeNode* RimWellPathCollection::addWellToWellNode( cafTreeNode* parent, RimWellPath* wellPath )
{
    if ( wellPath == nullptr ) return nullptr;

    std::vector<RimWellPath*> wellPaths = connectedWellPathLaterals( wellPath );

    auto node = new cafObjectReferenceTreeNode;
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
                continue;
            }
        }

        rootWells[RimWellPathCollection::unGroupedText()].push_back( wellPath );
    }

    for ( auto [groupName, wellPaths] : rootWells )
    {
        std::sort( wellPaths.begin(), wellPaths.end(), lessWellPath );
    }

    return rootWells;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathCollection::buildUiTreeOrdering( cafTreeNode* treeNode, caf::PdmUiTreeOrdering* parentUiTreeNode, const QString& uiConfigName )
{
    CAF_ASSERT( treeNode );
    CAF_ASSERT( parentUiTreeNode );

    if ( auto obj = treeNode->referencedObject() )
    {
        // Create the standard uiTreeOrdering with no relationship to parent using the helper function in
        // PdmUiObjectHandle. This will ensure that any child objects will be created correctly including any
        // recursive children. If a PdmUiTreeOrdering is created directly based on the PdmObject, andy child objects
        // must be added manually.

        auto uiTreeNode = obj->uiCapability()->uiTreeOrdering( uiConfigName );
        parentUiTreeNode->appendChild( uiTreeNode );

        // Build additional child nodes recursively
        for ( auto childNode : treeNode->childNodes() )
        {
            buildUiTreeOrdering( childNode, uiTreeNode, uiConfigName );
        }
    }
    else
    {
        // If no referenced object is attached, fallback to the node. Not sure if this is the best solution
        auto uiTreeNode = parentUiTreeNode->add( treeNode );

        // Build additional child nodes recursively
        for ( auto childNode : treeNode->childNodes() )
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
void RimWellPathCollection::onChildDeleted( caf::PdmChildArrayFieldHandle* childArray, std::vector<caf::PdmObjectHandle*>& referringObjects )
{
    rebuildWellPathNodes();

    scheduleRedrawAffectedViews();
    uiCapability()->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathCollection::onChildAdded( caf::PdmFieldHandle* containerForNewObject )
{
    rebuildWellPathNodes();

    scheduleRedrawAffectedViews();
    uiCapability()->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<cvf::ref<RigWellPath>, QString> RimWellPathCollection::loadWellPathGeometryFromOsdu( RiaOsduConnector* osduConnector,
                                                                                               const QString&    wellboreTrajectoryId,
                                                                                               double            datumElevation )
{
    auto [fileContents, errorMessage] = osduConnector->requestWellboreTrajectoryParquetDataByIdBlocking( wellboreTrajectoryId );
    if ( !errorMessage.isEmpty() )
    {
        return { nullptr, errorMessage };
    }

    return RifOsduWellPathReader::readWellPathData( fileContents, datumElevation );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<cvf::ref<RigOsduWellLogData>, QString> RimWellPathCollection::loadWellLogFromOsdu( RiaOsduConnector* osduConnector,
                                                                                             const QString&    wellLogId )
{
    auto [fileContents, errorMessage] = osduConnector->requestWellLogParquetDataByIdBlocking( wellLogId );
    if ( !errorMessage.isEmpty() )
    {
        return { nullptr, errorMessage };
    }

    return RifOsduWellLogReader::readWellLogData( fileContents );
}
