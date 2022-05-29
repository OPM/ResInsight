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

#include "RimWellPath.h"

#include "RiaColorTables.h"
#include "RiaFieldHandleTools.h"
#include "RiaSimWellBranchTools.h"
#include "RiaWellNameComparer.h"

#include "RicfCommandObject.h"

#include "RifWellPathFormationsImporter.h"
#include "RifWellPathImporter.h"

#include "RigWellPath.h"

#include "Rim3dWellLogCurve.h"
#include "Rim3dWellLogCurveCollection.h"
#include "RimFishbones.h"
#include "RimFishbonesCollection.h"
#include "RimMainPlotCollection.h"
#include "RimMswCompletionParameters.h"
#include "RimPerforationCollection.h"
#include "RimProject.h"
#include "RimStimPlanModelCollection.h"
#include "RimTools.h"
#include "RimWellIASettingsCollection.h"
#include "RimWellLogFile.h"
#include "RimWellLogFileChannel.h"
#include "RimWellLogPlotCollection.h"
#include "RimWellPathAttributeCollection.h"
#include "RimWellPathCollection.h"
#include "RimWellPathCompletionSettings.h"
#include "RimWellPathCompletions.h"
#include "RimWellPathFracture.h"
#include "RimWellPathFractureCollection.h"
#include "RimWellPathTieIn.h"

#include "RiuMainWindow.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmUiTreeAttributes.h"
#include "cafPdmUiTreeOrdering.h"
#include "cafPdmUiTreeViewEditor.h"
#include "cafUtils.h"

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QString>

#include <regex>

CAF_PDM_SOURCE_INIT( RimWellPath, "WellPathBase" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const char RimWellPath::SIM_WELL_NONE_UI_TEXT[] = "None";

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellPath::isDeletable() const
{
    // Avoid framework functions, as delete is implemented in RicDeleteWellPathFeature
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPath::RimWellPath()
    : nameChanged( this )
{
    CAF_PDM_InitScriptableObjectWithNameAndComment( "WellPath", ":/Well.svg", "", "", "WellPath", "A ResInsight Well Path" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_name, "Name", "Name" );
    m_name.registerKeywordAlias( "WellPathName" );
    m_name.uiCapability()->setUiReadOnly( true );
    m_name.uiCapability()->setUiHidden( true );
    m_name.xmlCapability()->disableIO();

    CAF_PDM_InitFieldNoDefault( &m_airGap, "AirGap", "Air Gap" );
    m_airGap.registerGetMethod( this, &RimWellPath::airGap );
    m_airGap.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_datumElevation, "DatumElevation", "Datum Elevation" );
    m_datumElevation.registerGetMethod( this, &RimWellPath::datumElevation );
    m_datumElevation.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_unitSystem, "UnitSystem", "Unit System" );
    m_unitSystem.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitField( &m_simWellName, "SimWellName", QString( "" ), "Well" );
    CAF_PDM_InitField( &m_branchIndex, "SimBranchIndex", 0, "Branch" );

    CAF_PDM_InitField( &m_showWellPathLabel, "ShowWellPathLabel", true, "Show Well Path Label" );

    CAF_PDM_InitField( &m_showWellPath, "ShowWellPath", true, "Show Well Path" );
    m_showWellPath.uiCapability()->setUiHidden( true );

    CAF_PDM_InitField( &m_wellPathRadiusScaleFactor, "WellPathRadiusScale", 1.0, "Well Path Radius Scale" );
    CAF_PDM_InitField( &m_wellPathColor, "WellPathColor", cvf::Color3f( 0.999f, 0.333f, 0.999f ), "Well Path Color" );

    CAF_PDM_InitFieldNoDefault( &m_completions, "Completions", "Completions" );
    m_completions = new RimWellPathCompletions;
    m_completions.uiCapability()->setUiTreeHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_completionSettings, "CompletionSettings", "Completion Settings" );
    m_completionSettings = new RimWellPathCompletionSettings;

    CAF_PDM_InitFieldNoDefault( &m_wellLogFiles, "WellLogFiles", "Well Log Files" );
    m_wellLogFiles.uiCapability()->setUiTreeHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_3dWellLogCurves, "CollectionOf3dWellLogCurves", "3D Track" );
    m_3dWellLogCurves = new Rim3dWellLogCurveCollection;
    m_3dWellLogCurves.uiCapability()->setUiTreeHidden( true );

    CAF_PDM_InitField( &m_formationKeyInFile, "WellPathFormationKeyInFile", QString( "" ), "Key in File" );
    m_formationKeyInFile.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_wellPathFormationFilePath, "WellPathFormationFilePath", "File Path" );
    m_wellPathFormationFilePath.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_wellPathAttributes, "WellPathAttributes", "Casing Design Rubbish" );
    m_wellPathAttributes = new RimWellPathAttributeCollection;
    m_wellPathAttributes->uiCapability()->setUiTreeHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_wellPathTieIn, "WellPathTieIn", "well Path Tie-In" );
    m_wellPathTieIn = new RimWellPathTieIn;
    m_wellPathTieIn->connectWellPaths( nullptr, this, 0.0 );

    CAF_PDM_InitFieldNoDefault( &m_wellIASettingsCollection, "WellIASettings", "Integrity Analysis Settings" );
    m_wellIASettingsCollection = new RimWellIASettingsCollection();

    this->setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPath::~RimWellPath()
{
    for ( const auto& file : m_wellLogFiles() )
    {
        delete file;
    }

    RimProject* project;
    firstAncestorOrThisOfType( project );
    if ( project )
    {
        if ( project->mainPlotCollection() )
        {
            RimWellLogPlotCollection* plotCollection = project->mainPlotCollection()->wellLogPlotCollection();
            if ( plotCollection )
            {
                plotCollection->removeExtractors( m_wellPathGeometry.p() );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimWellPath::userDescriptionField()
{
    return &m_name;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellPath::wellPathRadius( double characteristicCellSize ) const
{
    double radius = characteristicCellSize * m_wellPathRadiusScaleFactor();

    RimWellPathCollection* coll = nullptr;
    this->firstAncestorOrThisOfType( coll );
    if ( coll )
    {
        radius *= coll->wellPathRadiusScaleFactor();
    }

    return radius;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellPath::wellPathRadiusScaleFactor() const
{
    return m_wellPathRadiusScaleFactor();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellPath::isEnabled() const
{
    return m_showWellPath;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::WellPathComponentType RimWellPath::componentType() const
{
    return RiaDefines::WellPathComponentType::WELL_PATH;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellPath::componentLabel() const
{
    return name();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellPath::componentTypeLabel() const
{
    return "Well Path";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Color3f RimWellPath::defaultComponentColor() const
{
    return RiaColorTables::wellPathComponentColors()[componentType()];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPath::applyOffset( double offsetMD )
{
    // Nothing to do here, as the offset is intended for well path completions
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFishbonesCollection* RimWellPath::fishbonesCollection()
{
    CVF_ASSERT( m_completions );

    return m_completions->fishbonesCollection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RimFishbonesCollection* RimWellPath::fishbonesCollection() const
{
    CVF_ASSERT( m_completions );

    return m_completions->fishbonesCollection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPerforationCollection* RimWellPath::perforationIntervalCollection()
{
    CVF_ASSERT( m_completions );

    return m_completions->perforationCollection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RimPerforationCollection* RimWellPath::perforationIntervalCollection() const
{
    CVF_ASSERT( m_completions );

    return m_completions->perforationCollection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RimWellPathCompletions* RimWellPath::completions() const
{
    return m_completions();
}

//--------------------------------------------------------------------------------------------------
/// Returns top-level completion settings
//--------------------------------------------------------------------------------------------------
const RimWellPathCompletionSettings* RimWellPath::completionSettings() const
{
    if ( isTopLevelWellPath() ) return m_completionSettings();

    return topLevelWellPath()->completionSettings();
}

//--------------------------------------------------------------------------------------------------
/// Returns top-level completion settings
//--------------------------------------------------------------------------------------------------
RimWellPathCompletionSettings* RimWellPath::completionSettings()
{
    if ( isTopLevelWellPath() ) return m_completionSettings();

    return topLevelWellPath()->completionSettings();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimMswCompletionParameters* RimWellPath::mswCompletionParameters()
{
    auto params = m_completionSettings->mswCompletionParameters();
    if ( !isTopLevelWellPath() )
    {
        auto topMsw = topLevelWellPath()->mswCompletionParameters();

        // Propagate most settings from top level well into lateral parameters
        params->updateFromTopLevelWell( topMsw );
    }

    return params;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RimMswCompletionParameters* RimWellPath::mswCompletionParameters() const
{
    auto params = m_completionSettings->mswCompletionParameters();
    if ( !isTopLevelWellPath() )
    {
        auto topMsw = topLevelWellPath()->mswCompletionParameters();

        // Propagate most settings from top level well into lateral parameters
        params->updateFromTopLevelWell( topMsw );
    }

    return params;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPathFractureCollection* RimWellPath::fractureCollection()
{
    CVF_ASSERT( m_completions );

    return m_completions->fractureCollection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RimWellPathFractureCollection* RimWellPath::fractureCollection() const
{
    CVF_ASSERT( m_completions );

    return m_completions->fractureCollection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimStimPlanModelCollection* RimWellPath::stimPlanModelCollection()
{
    CVF_ASSERT( m_completions );

    return m_completions->stimPlanModelCollection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RimStimPlanModelCollection* RimWellPath::stimPlanModelCollection() const
{
    CVF_ASSERT( m_completions );

    return m_completions->stimPlanModelCollection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigWellPath* RimWellPath::wellPathGeometry()
{
    return m_wellPathGeometry.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigWellPath* RimWellPath::wellPathGeometry() const
{
    return m_wellPathGeometry.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellPath::startMD() const
{
    if ( wellPathGeometry() )
    {
        return wellPathGeometry()->measuredDepths().front();
    }
    return std::numeric_limits<double>::infinity();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellPath::endMD() const
{
    if ( wellPathGeometry() )
    {
        return wellPathGeometry()->measuredDepths().back();
    }
    return std::numeric_limits<double>::infinity();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellPath::uniqueStartMD() const
{
    if ( wellPathGeometry() )
    {
        auto uniqueMDs = wellPathGeometry()->uniqueMeasuredDepths();
        if ( !uniqueMDs.empty() ) return uniqueMDs.front();
    }
    return std::numeric_limits<double>::infinity();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellPath::uniqueEndMD() const
{
    if ( wellPathGeometry() )
    {
        auto uniqueMDs = wellPathGeometry()->uniqueMeasuredDepths();
        if ( !uniqueMDs.empty() ) return uniqueMDs.back();
    }
    return std::numeric_limits<double>::infinity();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPath::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    RimProject* proj;
    this->firstAncestorOrThisOfTypeAsserted( proj );
    if ( changedField == &m_showWellPath )
    {
        proj->reloadCompletionTypeResultsInAllViews();
    }
    else if ( changedField == &m_name )
    {
        QString previousName = oldValue.toString();
        QString newName      = newValue.toString();
        m_completionSettings->updateWellPathNameHasChanged( newName, previousName );
    }
    else
    {
        proj->scheduleCreateDisplayModelAndRedrawAllViews();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimWellPath::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_simWellName )
    {
        RimProject* proj = RimProject::current();

        // Find simulation wells already assigned to a well path
        std::set<QString> associatedSimWells;
        for ( const auto& wellPath : proj->allWellPaths() )
        {
            if ( wellPath->isAssociatedWithSimulationWell() && wellPath != this )
            {
                associatedSimWells.insert( wellPath->associatedSimulationWellName() );
            }
        }

        options.push_back( caf::PdmOptionItemInfo( SIM_WELL_NONE_UI_TEXT, "" ) );
        for ( const auto& wellName : proj->simulationWellNames() )
        {
            if ( associatedSimWells.count( wellName ) > 0 ) continue;

            options.push_back( caf::PdmOptionItemInfo( wellName, wellName ) );
        }
    }
    else if ( fieldNeedingOptions == &m_branchIndex )
    {
        size_t branchCount = RimWellPath::simulationWellBranchCount( m_simWellName );

        if ( branchCount == 0 ) branchCount = 1;

        size_t index = 0;
        while ( index < branchCount )
        {
            QString uiText = QString( "Branch %1" ).arg( QString::number( index + 1 ) );
            options.push_back( caf::PdmOptionItemInfo( uiText, QVariant::fromValue( index ) ) );
            index++;
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPath::initAfterRead()
{
    if ( RimProject::current()->isProjectFileVersionEqualOrOlderThan( "2020.10.1" ) )
    {
        if ( m_completionSettings->wellNameForExport().isEmpty() )
        {
            m_completionSettings->setWellNameForExport( name() );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------- -----------------------------
QString RimWellPath::name() const
{
    return m_name();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------- -----------------------------
void RimWellPath::setName( const QString& name )
{
    setNameNoUpdateOfExportName( name );

    m_completionSettings->setWellNameForExport( name );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPath::setNameNoUpdateOfExportName( const QString& name )
{
    m_name = name;
    tryAssociateWithSimulationWell();
    nameChanged.send();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWellLogFile*> RimWellPath::wellLogFiles() const
{
    return std::vector<RimWellLogFile*>( m_wellLogFiles.begin(), m_wellLogFiles.end() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellLogFile* RimWellPath::firstWellLogFileMatchingChannelName( const QString& channelName ) const
{
    std::vector<RimWellLogFile*> allWellLogFiles = wellLogFiles();
    for ( RimWellLogFile* logFile : allWellLogFiles )
    {
        std::vector<RimWellLogFileChannel*> channels = logFile->wellLogChannels();
        for ( RimWellLogFileChannel* channel : channels )
        {
            if ( channel->name() == channelName )
            {
                return logFile;
            }
        }
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPathAttributeCollection* RimWellPath::attributeCollection()
{
    return m_wellPathAttributes;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RimWellPathAttributeCollection* RimWellPath::attributeCollection() const
{
    return m_wellPathAttributes;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellIASettingsCollection* RimWellPath::wellIASettingsCollection()
{
    return m_wellIASettingsCollection;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellPath::showWellPathLabel() const
{
    return m_showWellPathLabel();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellPath::showWellPath() const
{
    return m_showWellPath();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Color3f RimWellPath::wellPathColor() const
{
    return m_wellPathColor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPath::setWellPathColor( const cvf::Color3f& color )
{
    m_wellPathColor = color;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimWellPath::objectToggleField()
{
    return &m_showWellPath;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPath::setWellPathGeometry( RigWellPath* wellPathModel )
{
    m_wellPathGeometry = wellPathModel;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPath::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    if ( m_wellPathTieIn() )
    {
        m_wellPathTieIn->uiOrdering( uiConfigName, uiOrdering );
    }

    if ( m_simWellName().isEmpty() )
    {
        // Try to set default simulation well name
        tryAssociateWithSimulationWell();
    }

    caf::PdmUiGroup* appGroup = uiOrdering.addNewGroup( "Appearance" );
    appGroup->add( &m_showWellPathLabel );
    appGroup->add( &m_wellPathColor );
    appGroup->add( &m_wellPathRadiusScaleFactor );

    caf::PdmUiGroup* simWellGroup = uiOrdering.addNewGroup( "Simulation Well" );
    simWellGroup->add( &m_simWellName );

    if ( simulationWellBranchCount( m_simWellName ) > 1 )
    {
        simWellGroup->add( &m_branchIndex );
    }

    caf::PdmUiGroup* ssihubGroup = uiOrdering.addNewGroup( "Well Info" );

    if ( m_wellPathGeometry.notNull() && m_wellPathGeometry->rkbDiff() > 0.0 )
    {
        ssihubGroup->add( &m_airGap );
    }

    if ( m_wellPathGeometry.notNull() && m_wellPathGeometry->hasDatumElevation() )
    {
        ssihubGroup->add( &m_datumElevation );
    }
    ssihubGroup->add( &m_unitSystem );

    caf::PdmUiGroup* formationFileInfoGroup = uiOrdering.addNewGroup( "Well Picks" );
    formationFileInfoGroup->add( &m_wellPathFormationFilePath );
    formationFileInfoGroup->add( &m_formationKeyInFile );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPath::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName )
{
    uiTreeOrdering.add( &m_wellLogFiles );

    if ( m_wellIASettingsCollection()->isEnabled() && m_wellIASettingsCollection()->hasSettings() )
    {
        uiTreeOrdering.add( m_wellIASettingsCollection() );
    }

    if ( m_completionSettings() && !allCompletionsRecursively().empty() )
    {
        uiTreeOrdering.add( m_completionSettings() );
    }

    if ( m_completions->fishbonesCollection()->hasFishbones() )
    {
        uiTreeOrdering.add( m_completions->fishbonesCollection() );
    }
    if ( m_completions->fractureCollection()->hasFractures() )
    {
        uiTreeOrdering.add( m_completions->fractureCollection() );
    }
    if ( m_completions->perforationCollection()->hasPerforations() )
    {
        uiTreeOrdering.add( m_completions->perforationCollection() );
    }
    if ( m_completions->stimPlanModelCollection()->hasStimPlanModels() )
    {
        uiTreeOrdering.add( m_completions->stimPlanModelCollection() );
    }

    if ( m_3dWellLogCurves->has3dWellLogCurves() )
    {
        uiTreeOrdering.add( m_3dWellLogCurves() );
    }

    if ( !m_wellPathAttributes->attributes().empty() )
    {
        uiTreeOrdering.add( m_wellPathAttributes() );
    }

    uiTreeOrdering.skipRemainingChildren( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPath::copyCompletionSettings( RimWellPath* from, RimWellPath* to )
{
    if ( !from->m_completionSettings ) return;

    if ( !to->m_completionSettings )
    {
        to->m_completionSettings = new RimWellPathCompletionSettings( *from->m_completionSettings() );
    }
    else
    {
        *( to->m_completionSettings() ) = *( from->m_completionSettings() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPath::defineObjectEditorAttribute( QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    auto myAttr = dynamic_cast<caf::PdmUiTreeViewEditorAttribute*>( attribute );
    if ( myAttr )
    {
        // The nodes for well paths are created by the well path collection object. When a well path object is asked to
        // be updated in the project tree, always rebuild the tree from the well path collection object.

        myAttr->objectForUpdateOfUiTree = RimTools::wellPathCollection();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RimWellPath::simulationWellBranchCount( const QString& simWellName )
{
    bool detectBranches = true;

    auto branches = RiaSimWellBranchTools::simulationWellBranches( simWellName, detectBranches );

    return branches.size();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPath::updateFilePathsFromProjectPath( const QString& newProjectPath, const QString& oldProjectPath )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellPath::combinedScaleFactor() const
{
    RimWellPathCollection* wellPathColl = nullptr;
    this->firstAncestorOrThisOfTypeAsserted( wellPathColl );

    return this->m_wellPathRadiusScaleFactor() * wellPathColl->wellPathRadiusScaleFactor();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPath::setUnitSystem( RiaDefines::EclipseUnitSystem unitSystem )
{
    m_unitSystem = unitSystem;

    m_completions->setUnitSystemSpecificDefaults();

    std::vector<RimMswCompletionParameters*> mswParameters;
    descendantsOfType( mswParameters );
    for ( auto mswParams : mswParameters )
    {
        mswParams->setUnitSystemSpecificDefaults();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::EclipseUnitSystem RimWellPath::unitSystem() const
{
    return m_unitSystem();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellPath::airGap() const
{
    if ( m_wellPathGeometry.notNull() && m_wellPathGeometry->rkbDiff() > 0.0 )
    {
        return m_wellPathGeometry->rkbDiff();
    }
    return 0.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellPath::datumElevation() const
{
    if ( m_wellPathGeometry.notNull() && m_wellPathGeometry->hasDatumElevation() )
    {
        return m_wellPathGeometry->datumElevation();
    }
    return 0.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPath::addWellLogFile( RimWellLogFile* logFileInfo )
{
    // Prevent the same file from being loaded more than once
    auto itr = std::find_if( m_wellLogFiles.begin(), m_wellLogFiles.end(), [&]( const RimWellLogFile* file ) {
        return QString::compare( file->fileName(), logFileInfo->fileName(), Qt::CaseInsensitive ) == 0;
    } );

    // Todo: Verify well name to ensure all well log files having the same well name

    if ( itr == m_wellLogFiles.end() )
    {
        m_wellLogFiles.push_back( logFileInfo );

        if ( m_wellLogFiles.size() == 1 && name().isEmpty() )
        {
            setName( m_wellLogFiles[0]->wellName() );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPath::deleteWellLogFile( RimWellLogFile* logFileInfo )
{
    detachWellLogFile( logFileInfo );
    delete logFileInfo;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPath::detachWellLogFile( RimWellLogFile* logFileInfo )
{
    auto pdmObject = dynamic_cast<caf::PdmObjectHandle*>( logFileInfo );
    for ( size_t i = 0; i < m_wellLogFiles.size(); i++ )
    {
        if ( m_wellLogFiles[i] == pdmObject )
        {
            m_wellLogFiles.removeChildObject( pdmObject );
            break;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPath::setFormationsGeometry( cvf::ref<RigWellPathFormations> wellPathFormations )
{
    m_wellPathFormations        = wellPathFormations;
    m_wellPathFormationFilePath = wellPathFormations->filePath();
    m_formationKeyInFile        = wellPathFormations->keyInFile();

    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellPath::readWellPathFormationsFile( QString*                       errorMessage,
                                              RifWellPathFormationsImporter* wellPathFormationsImporter )
{
    if ( m_wellPathFormationFilePath().path().isEmpty() )
    {
        return true;
    }

    if ( caf::Utils::fileExists( m_wellPathFormationFilePath().path() ) )
    {
        m_wellPathFormations = wellPathFormationsImporter->readWellPathFormations( m_wellPathFormationFilePath().path(),
                                                                                   m_formationKeyInFile() );
        if ( m_name().isEmpty() )
        {
            setName( m_formationKeyInFile() );
        }
        return true;
    }
    else
    {
        if ( errorMessage )
            ( *errorMessage ) = "Could not find the well pick file: " + m_wellPathFormationFilePath().path();
        return false;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellPath::reloadWellPathFormationsFile( QString*                       errorMessage,
                                                RifWellPathFormationsImporter* wellPathFormationsImporter )
{
    if ( m_wellPathFormationFilePath().path().isEmpty() )
    {
        return true;
    }

    if ( caf::Utils::fileExists( m_wellPathFormationFilePath().path() ) )
    {
        m_wellPathFormations = wellPathFormationsImporter->reloadWellPathFormations( m_wellPathFormationFilePath().path(),
                                                                                     m_formationKeyInFile() );
        return true;
    }
    else
    {
        if ( errorMessage )
            ( *errorMessage ) = "Could not find the well pick file: " + m_wellPathFormationFilePath().path();
        return false;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellPath::hasFormations() const
{
    if ( m_wellPathFormations.isNull() )
    {
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigWellPathFormations* RimWellPath::formationsGeometry() const
{
    return m_wellPathFormations.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPath::add3dWellLogCurve( Rim3dWellLogCurve* rim3dWellLogCurve )
{
    m_3dWellLogCurves->add3dWellLogCurve( rim3dWellLogCurve );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Rim3dWellLogCurveCollection* RimWellPath::rim3dWellLogCurveCollection() const
{
    return m_3dWellLogCurves();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<const RimWellPathComponentInterface*> RimWellPath::allCompletionsRecursively() const
{
    std::vector<const RimWellPathComponentInterface*> allCompletions;

    auto laterals = allWellPathLaterals();
    for ( auto w : laterals )
    {
        auto completions = w->completions()->allCompletions();
        allCompletions.insert( allCompletions.end(), completions.begin(), completions.end() );
    }

    return allCompletions;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QString RimWellPath::associatedSimulationWellName() const
{
    return m_simWellName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimWellPath::associatedSimulationWellBranch() const
{
    return m_branchIndex;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellPath::tryAssociateWithSimulationWell()
{
    if ( !m_simWellName().isEmpty() ) return false;

    QString matchedSimWell = RiaWellNameComparer::tryFindMatchingSimWellName( m_name );

    if ( !matchedSimWell.isEmpty() )
    {
        m_simWellName = matchedSimWell;
        return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellPath::isAssociatedWithSimulationWell() const
{
    return !m_simWellName().isEmpty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPath::onChildDeleted( caf::PdmChildArrayFieldHandle*      childArray,
                                  std::vector<caf::PdmObjectHandle*>& referringObjects )
{
    RimProject::current()->reloadCompletionTypeResultsInAllViews();

    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellPath::isTopLevelWellPath() const
{
    return this == topLevelWellPath();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellPath::isMultiLateralWellPath() const
{
    auto top = topLevelWellPath();

    std::vector<RimWellPath*> wells;
    top->descendantsIncludingThisOfType( wells );

    return wells.size() > 1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPath* RimWellPath::topLevelWellPath()
{
    if ( m_wellPathTieIn() && m_wellPathTieIn->parentWell() )
    {
        if ( m_wellPathTieIn->parentWell() == this ) return this;

        return m_wellPathTieIn()->parentWell()->topLevelWellPath();
    }

    return this;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RimWellPath* RimWellPath::topLevelWellPath() const
{
    if ( m_wellPathTieIn() && m_wellPathTieIn->parentWell() )
    {
        if ( m_wellPathTieIn->parentWell() == this ) return this;

        return m_wellPathTieIn()->parentWell()->topLevelWellPath();
    }

    return this;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPath::wellPathLateralsRecursively( std::vector<RimWellPath*>& wellPathLaterals ) const
{
    wellPathLaterals.push_back( const_cast<RimWellPath*>( this ) );

    std::vector<caf::PdmObjectHandle*> referringObjects;
    this->objectsWithReferringPtrFields( referringObjects );
    for ( auto obj : referringObjects )
    {
        if ( auto tieIn = dynamic_cast<RimWellPathTieIn*>( obj ) )
        {
            auto tieInWellPath = tieIn->childWell();
            if ( tieInWellPath )
            {
                if ( std::find( wellPathLaterals.begin(), wellPathLaterals.end(), tieInWellPath ) == wellPathLaterals.end() )
                {
                    tieInWellPath->wellPathLateralsRecursively( wellPathLaterals );
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWellPath*> RimWellPath::allWellPathLaterals() const
{
    std::vector<RimWellPath*> laterals;

    this->wellPathLateralsRecursively( laterals );

    return laterals;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWellPath*> RimWellPath::wellPathLaterals() const
{
    std::vector<RimWellPath*> laterals;

    std::vector<caf::PdmObjectHandle*> referringObjects;
    this->objectsWithReferringPtrFields( referringObjects );
    for ( auto obj : referringObjects )
    {
        if ( auto tieIn = dynamic_cast<RimWellPathTieIn*>( obj ) )
        {
            auto tieInWellPath = tieIn->childWell();
            if ( tieInWellPath == this ) continue;
            if ( tieInWellPath )
            {
                laterals.push_back( tieInWellPath );
            }
        }
    }

    return laterals;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPathTieIn* RimWellPath::wellPathTieIn() const
{
    return m_wellPathTieIn();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPath::connectWellPaths( RimWellPath* parentWell, double parentTieInMeasuredDepth )
{
    CVF_ASSERT( parentWell != this );

    if ( parentWell != this )
    {
        if ( !m_wellPathTieIn() ) m_wellPathTieIn = new RimWellPathTieIn;

        m_wellPathTieIn->connectWellPaths( parentWell, this, parentTieInMeasuredDepth );
        m_wellPathTieIn->updateFirstTargetFromParentWell();
    }
}
