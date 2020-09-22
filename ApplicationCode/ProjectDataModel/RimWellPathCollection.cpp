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

#include "RiaApplication.h"
#include "RiaColorTables.h"
#include "RiaGuiApplication.h"
#include "RiaLogging.h"
#include "RiaPreferences.h"
#include "RiaWellNameComparer.h"

#include "RigEclipseCaseData.h"
#include "RigMainGrid.h"
#include "RigWellPath.h"

#include "RimEclipseCase.h"
#include "RimEclipseCaseCollection.h"
#include "RimEclipseView.h"
#include "RimFractureModel.h"
#include "RimFractureModelCollection.h"
#include "RimOilField.h"
#include "RimPerforationCollection.h"
#include "RimProject.h"
#include "RimWellLogFile.h"
#include "RimWellMeasurementCollection.h"
#include "RimWellPath.h"
#include "Riu3DMainWindowTools.h"

#include "RifWellPathFormationsImporter.h"
#include "RifWellPathImporter.h"

#include "cafPdmUiEditorHandle.h"
#include "cafPdmUiTreeOrdering.h"
#include "cafProgressInfo.h"

#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QString>

#include "RimFileWellPath.h"
#include "RimModeledWellPath.h"
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

    cvf::Color3f defWellLabelColor = RiaApplication::instance()->preferences()->defaultWellLabelColor();
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

    CAF_PDM_InitFieldNoDefault( &wellPaths, "WellPaths", "Well Paths", "", "", "" );
    wellPaths.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_wellMeasurements, "WellMeasurements", "Measurements", "", "", "" );
    m_wellMeasurements = new RimWellMeasurementCollection;
    m_wellMeasurements.uiCapability()->setUiTreeHidden( true );

    m_wellPathImporter            = new RifWellPathImporter;
    m_wellPathFormationsImporter  = new RifWellPathFormationsImporter;
    m_mostRecentlyUpdatedWellPath = nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPathCollection::~RimWellPathCollection()
{
    wellPaths.deleteAllChildObjects();
    delete m_wellPathImporter;
    delete m_wellPathFormationsImporter;
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
    caf::ProgressInfo progress( wellPaths.size(), "Reading well paths from file" );

    readWellPathFormationFiles();

    for ( RimWellPath* wellPath : wellPaths() )
    {
        progress.setProgressDescription( QString( "Reading file %1" ).arg( wellPath->name() ) );

        RimFileWellPath*    fWPath = dynamic_cast<RimFileWellPath*>( wellPath );
        RimModeledWellPath* mWPath = dynamic_cast<RimModeledWellPath*>( wellPath );
        if ( fWPath )
        {
            if ( !fWPath->filePath().isEmpty() )
            {
                QString errorMessage;
                if ( !fWPath->readWellPathFile( &errorMessage, m_wellPathImporter, false ) )
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

            RimFractureModelCollection* fractureModelCollection = wellPath->fractureModelCollection();
            if ( fractureModelCollection )
            {
                for ( RimFractureModel* fractureModel : fractureModelCollection->allFractureModels() )
                {
                    fractureModel->loadDataAndUpdate();
                }
            }
        }
        progress.incrementProgress();
    }

    this->sortWellsByName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWellPath*> RimWellPathCollection::addWellPaths( QStringList filePaths, QStringList* errorMessages )
{
    CAF_ASSERT( errorMessages );

    std::vector<RimFileWellPath*> wellPathArray;

    for ( QString filePath : filePaths )
    {
        // Check if this file is already open
        bool alreadyOpen = false;
        for ( size_t wpIdx = 0; wpIdx < wellPaths.size(); wpIdx++ )
        {
            RimFileWellPath* fWPath = dynamic_cast<RimFileWellPath*>( wellPaths[wpIdx] );
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

    readAndAddWellPaths( wellPathArray );
    CAF_ASSERT( wellPathArray.empty() );

    scheduleRedrawAffectedViews();
    updateAllRequiredEditors();

    return wellPaths.childObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathCollection::readAndAddWellPaths( std::vector<RimFileWellPath*>& wellPathArray )
{
    caf::ProgressInfo progress( wellPathArray.size(), "Reading well paths from file" );

    for ( size_t wpIdx = 0; wpIdx < wellPathArray.size(); wpIdx++ )
    {
        RimFileWellPath* wellPath = wellPathArray[wpIdx];
        wellPath->readWellPathFile( nullptr, m_wellPathImporter, true );

        progress.setProgressDescription( QString( "Reading file %1" ).arg( wellPath->name() ) );

        // If a well path with this name exists already, make it read the well path file
        RimFileWellPath* existingWellPath = dynamic_cast<RimFileWellPath*>( tryFindMatchingWellPath( wellPath->name() ) );
        if ( existingWellPath )
        {
            existingWellPath->setFilepath( wellPath->filePath() );
            existingWellPath->setWellPathIndexInFile( wellPath->wellPathIndexInFile() );
            existingWellPath->readWellPathFile( nullptr, m_wellPathImporter, true );

            // Let name from well path file override name from well log file
            existingWellPath->setName( wellPath->name() );

            m_mostRecentlyUpdatedWellPath = existingWellPath;
            delete wellPath;
        }
        else
        {
            wellPath->setWellPathColor( RiaColorTables::wellPathsPaletteColors().cycledColor3f( wellPaths.size() ) );
            wellPath->setUnitSystem( findUnitSystemForWellPath( wellPath ) );
            m_mostRecentlyUpdatedWellPath = wellPath;
            wellPaths.push_back( wellPath );
        }

        progress.incrementProgress();
    }
    wellPathArray.clear(); // This should not be used again. We may have deleted items
    this->sortWellsByName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathCollection::addWellPaths( const std::vector<RimWellPath*> incomingWellPaths )
{
    for ( const auto& wellPath : incomingWellPaths )
    {
        this->wellPaths.push_back( wellPath );
    }
    this->sortWellsByName();

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
                wellPaths.push_back( wellPath );
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
                wellPaths.push_back( wellPath );
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
        if ( RiaGuiApplication::isRunning() )
        {
            QMessageBox::information( Riu3DMainWindowTools::mainWindowWidget(), "Well Picks Import", outputMessage );
        }
        RiaLogging::info( outputMessage );
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

    if ( !wellPaths.empty() )
    {
        uiTreeOrdering.add( &wellPaths );
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
    for ( size_t wellPathIdx = 0; wellPathIdx < wellPaths.size(); wellPathIdx++ )
    {
        wellPaths[wellPathIdx]->updateFilePathsFromProjectPath( newProjectPath, oldProjectPath );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellPathCollection::anyWellsContainingPerforationIntervals() const
{
    for ( const auto& wellPath : wellPaths )
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
    for ( size_t wellPathIdx = 0; wellPathIdx < wellPaths.size(); wellPathIdx++ )
    {
        if ( dynamic_cast<RimModeledWellPath*>( wellPaths[wellPathIdx] ) )
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
    for ( size_t wellPathIdx = 0; wellPathIdx < wellPaths.size(); wellPathIdx++ )
    {
        if ( wellPaths[wellPathIdx]->name() == wellPathName )
        {
            return wellPaths[wellPathIdx];
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
    wellPaths.deleteAllChildObjects();

    m_wellPathImporter->clear();
    updateAllRequiredEditors();
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
    caf::ProgressInfo progress( wellPaths.size(), "Reading well picks from file" );

    for ( size_t wpIdx = 0; wpIdx < wellPaths.size(); wpIdx++ )
    {
        QString errorMessage;
        if ( !wellPaths[wpIdx]->readWellPathFormationsFile( &errorMessage, m_wellPathFormationsImporter ) )
        {
            if ( RiaGuiApplication::isRunning() )
            {
                QMessageBox::warning( Riu3DMainWindowTools::mainWindowWidget(), "File open error", errorMessage );
            }
            RiaLogging::warning( errorMessage );
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
    caf::ProgressInfo progress( wellPaths.size(), "Reloading well picks from file" );

    for ( size_t wpIdx = 0; wpIdx < wellPaths.size(); wpIdx++ )
    {
        QString errorMessage;
        if ( !wellPaths[wpIdx]->reloadWellPathFormationsFile( &errorMessage, m_wellPathFormationsImporter ) )
        {
            if ( RiaGuiApplication::isRunning() )
            {
                QMessageBox::warning( Riu3DMainWindowTools::mainWindowWidget(), "File open error", errorMessage );
            }
            RiaLogging::warning( errorMessage );
        }

        progress.setProgressDescription( QString( "Reloading formation file %1" ).arg( wpIdx ) );
        progress.incrementProgress();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathCollection::removeWellPath( RimWellPath* wellPath )
{
    wellPaths.removeChildObject( wellPath );

    RimFileWellPath* fileWellPath = dynamic_cast<RimFileWellPath*>( wellPath );
    if ( fileWellPath )
    {
        bool isFilePathUsed = false;
        for ( size_t i = 0; i < wellPaths.size(); i++ )
        {
            RimFileWellPath* fWPath = dynamic_cast<RimFileWellPath*>( wellPaths[i] );
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
    std::sort( wellPaths.begin(), wellPaths.end(), lessWellPath );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaEclipseUnitTools::UnitSystemType RimWellPathCollection::findUnitSystemForWellPath( const RimWellPath* wellPath )
{
    RimProject* project;
    firstAncestorOrThisOfTypeAsserted( project );
    if ( project->activeOilField()->analysisModels->cases.empty() )
    {
        return RiaEclipseUnitTools::UnitSystem::UNITS_UNKNOWN;
    }

    const RigEclipseCaseData* eclipseCaseData = project->activeOilField()->analysisModels->cases()[0]->eclipseCaseData();
    cvf::BoundingBox          caseBoundingBox = eclipseCaseData->mainGrid()->boundingBox();
    cvf::BoundingBox          wellPathBoundingBox;
    for ( auto& wellPathPoint : wellPath->wellPathGeometry()->m_wellPathPoints )
    {
        wellPathBoundingBox.add( wellPathPoint );
    }

    if ( caseBoundingBox.intersects( wellPathBoundingBox ) )
    {
        return eclipseCaseData->unitsType();
    }
    return RiaEclipseUnitTools::UnitSystem::UNITS_UNKNOWN;
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
    scheduleRedrawAffectedViews();
    uiCapability()->updateConnectedEditors();
}
