/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-     Statoil ASA
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

#include "RicExportCompletionDataSettingsUi.h"
#include "RicExportFractureCompletionsImpl.h"

#include "RiaEclipseUnitTools.h"

#include "RimProject.h"
#include "RimTools.h"
#include "RimWellPath.h"
#include "RimWellPathCompletions.h"

namespace caf
{
template <>
void RicExportCompletionDataSettingsUi::ExportSplitType::setUp()
{
    addItem( RicExportCompletionDataSettingsUi::ExportSplit::UNIFIED_FILE, "UNIFIED_FILE", "Unified File" );
    addItem( RicExportCompletionDataSettingsUi::ExportSplit::SPLIT_ON_WELL, "SPLIT_ON_WELL", "Split on Well" );
    addItem( RicExportCompletionDataSettingsUi::ExportSplit::SPLIT_ON_WELL_AND_COMPLETION_TYPE,
             "SPLIT_ON_WELL_AND_COMPLETION_TYPE",
             "Split on Well and Completion Type" );
    setDefault( RicExportCompletionDataSettingsUi::ExportSplit::SPLIT_ON_WELL_AND_COMPLETION_TYPE );
}

template <>
void RicExportCompletionDataSettingsUi::CompdatExportType::setUp()
{
    addItem( RicExportCompletionDataSettingsUi::CompdatExport::TRANSMISSIBILITIES, "TRANSMISSIBILITIES", "Calculated Transmissibilities" );
    addItem( RicExportCompletionDataSettingsUi::CompdatExport::WPIMULT_AND_DEFAULT_CONNECTION_FACTORS,
             "WPIMULT_AND_DEFAULT_CONNECTION_FACTORS",
             "Default Connection Factors and WPIMULT (Fractures Not Supported)" );
    setDefault( RicExportCompletionDataSettingsUi::CompdatExport::TRANSMISSIBILITIES );
}

template <>
void RicExportCompletionDataSettingsUi::CombinationModeType::setUp()
{
    addItem( RicExportCompletionDataSettingsUi::CombinationMode::INDIVIDUALLY, "INDIVIDUALLY", "Individually" );
    addItem( RicExportCompletionDataSettingsUi::CombinationMode::COMBINED, "COMBINED", "Combined" );
    setDefault( RicExportCompletionDataSettingsUi::CombinationMode::INDIVIDUALLY );
}

template <>
void RicExportCompletionDataSettingsUi::TransScalingWBHPSource::setUp()
{
    addItem( RicExportFractureCompletionsImpl::WBHP_FROM_SUMMARY, "WBHP_SUMMARY", "WBHP From Summary Case" );
    addItem( RicExportFractureCompletionsImpl::WBHP_FROM_USER_DEF, "WBHP_USER_DEFINED", "Fixed User Defined WBHP" );

    setDefault( RicExportFractureCompletionsImpl::WBHP_FROM_SUMMARY );
}
} // namespace caf

CAF_PDM_SOURCE_INIT( RicExportCompletionDataSettingsUi, "RicExportCompletionDataSettingsUi" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicExportCompletionDataSettingsUi::RicExportCompletionDataSettingsUi()
{
    CAF_PDM_InitObject( "RimExportCompletionDataSettings" );

    CAF_PDM_InitFieldNoDefault( &fileSplit, "FileSplit", "File Split" );

    CAF_PDM_InitFieldNoDefault( &compdatExport, "compdatExport", "Export", "", " ", "" );

    CAF_PDM_InitField( &timeStep, "TimeStepIndex", 0, "    Time Step" );

    CAF_PDM_InitField( &includeMsw, "IncludeMSW", true, "Multi Segment Well Model" );

    CAF_PDM_InitField( &useLateralNTG, "UseLateralNTG", false, "Use NTG Horizontally" );

    CAF_PDM_InitField( &includePerforations, "IncludePerforations", true, "Perforations" );
    CAF_PDM_InitField( &includeFishbones, "IncludeFishbones", true, "Fishbones" );
    CAF_PDM_InitField( &includeFractures, "IncludeFractures", true, "Fractures" );

    CAF_PDM_InitField( &performTransScaling, "TransScalingType", false, "Perform Transmissibility Scaling" );
    CAF_PDM_InitField( &transScalingTimeStep, "TransScalingTimeStep", 0, "Current Time Step" );
    CAF_PDM_InitFieldNoDefault( &transScalingWBHPSource, "TransScalingWBHPSource", "WBHP Selection" );
    CAF_PDM_InitField( &transScalingWBHP, "TransScalingWBHP", 200.0, "WBHP Before Production Start" );

    CAF_PDM_InitField( &excludeMainBoreForFishbones,
                       "ExcludeMainBoreForFishbones",
                       false,
                       "    Exclude Main Bore Transmissibility",
                       "",
                       "Main bore perforation intervals are defined by start/end MD of each active fishbone sub "
                       "definition",
                       "" );

    CAF_PDM_InitFieldNoDefault( &m_reportCompletionTypesSeparately, "ReportCompletionTypesSeparately", "Export Completion Types" );

    CAF_PDM_InitField( &m_exportDataSourceAsComment, "ExportDataSourceAsComment", true, "Comments" );

    CAF_PDM_InitField( &m_exportWelspec, "ExportWelspec", true, "WELSPEC keyword" );
    CAF_PDM_InitField( &m_completionWelspecAfterMainBore, "CompletionWelspecAfterMainBore", true, "WELSEGS per Completion Type" );

    CAF_PDM_InitField( &m_useCustomFileName, "UseCustomFileName", false, "Use Custom Filename" );
    CAF_PDM_InitField( &m_customFileName, "CustomFileName", {}, "Custom Filename" );

    m_fracturesEnabled    = true;
    m_perforationsEnabled = true;
    m_fishbonesEnabled    = true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportCompletionDataSettingsUi::enableIncludeMsw()
{
    includeMsw = true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportCompletionDataSettingsUi::setCombinationMode( CombinationMode combinationMode )
{
    m_reportCompletionTypesSeparately = combinationMode;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportCompletionDataSettingsUi::setExportDataSourceAsComment( bool enable )
{
    m_exportDataSourceAsComment = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportCompletionDataSettingsUi::showFractureInUi( bool enable )
{
    m_fracturesEnabled = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportCompletionDataSettingsUi::showPerforationsInUi( bool enable )
{
    m_perforationsEnabled = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportCompletionDataSettingsUi::showFishbonesInUi( bool enable )
{
    m_fishbonesEnabled = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicExportCompletionDataSettingsUi::reportCompletionsTypesIndividually() const
{
    return m_reportCompletionTypesSeparately() == CombinationMode::INDIVIDUALLY;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicExportCompletionDataSettingsUi::exportDataSourceAsComment() const
{
    return m_exportDataSourceAsComment;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportCompletionDataSettingsUi::setExportWelspec( bool enable )
{
    m_exportWelspec = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicExportCompletionDataSettingsUi::exportWelspec() const
{
    return m_exportWelspec;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportCompletionDataSettingsUi::setExportCompletionWelspecAfterMainBore( bool enable )
{
    m_completionWelspecAfterMainBore = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicExportCompletionDataSettingsUi::exportCompletionWelspecAfterMainBore() const
{
    return m_completionWelspecAfterMainBore();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportCompletionDataSettingsUi::setCustomFileName( const QString& fileName )
{
    m_useCustomFileName = !fileName.isEmpty();
    m_customFileName    = fileName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicExportCompletionDataSettingsUi::customFileName() const
{
    if ( m_useCustomFileName ) return m_customFileName();

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportCompletionDataSettingsUi::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                                          const QVariant&            oldValue,
                                                          const QVariant&            newValue )
{
    if ( changedField == &compdatExport )
    {
        if ( compdatExport == CompdatExport::WPIMULT_AND_DEFAULT_CONNECTION_FACTORS )
        {
            includeFractures = false;
        }
        else if ( compdatExport == CompdatExport::TRANSMISSIBILITIES || includeMsw )
        {
            includeFractures = true;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RicExportCompletionDataSettingsUi::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;
    if ( fieldNeedingOptions == &timeStep )
    {
        RimTools::timeStepsForCase( caseToApply, &options );
    }
    else if ( fieldNeedingOptions == &transScalingTimeStep )
    {
        std::map<int, std::vector<std::pair<QString, QString>>> wellProductionStartStrings = generateWellProductionStartStrings();

        QStringList timeStepNames;

        if ( caseToApply )
        {
            timeStepNames = caseToApply->timeStepStrings();
        }
        for ( int i = 0; i < timeStepNames.size(); i++ )
        {
            QString timeStepString = timeStepNames[i];
            auto    it             = wellProductionStartStrings.find( i );
            if ( it != wellProductionStartStrings.end() )
            {
                int         numberOfWells = static_cast<int>( it->second.size() );
                QStringList wellList;
                QStringList wellPressureList;
                const int   maxStringLength = 70;
                QString     startStringFormat( " [Start: %1]" );

                for ( int w = 0; w < numberOfWells; ++w )
                {
                    QString     wellString        = it->second[w].first;
                    QStringList candidateWellList = wellList;
                    candidateWellList << wellString;

                    if ( startStringFormat.arg( candidateWellList.join( ", " ) ).length() < maxStringLength )
                    {
                        wellList = candidateWellList;
                    }

                    QString     wellStringWithPressure    = QString( "%1 (%2)" ).arg( it->second[w].first ).arg( it->second[w].second );
                    QStringList candidateWellPressureList = wellPressureList;
                    candidateWellPressureList << wellStringWithPressure;
                    if ( startStringFormat.arg( candidateWellPressureList.join( ", " ) ).length() < maxStringLength )
                    {
                        wellPressureList = candidateWellPressureList;
                    }
                }

                if ( wellList.size() < numberOfWells )
                {
                    wellList += QString( "+ %1 more" ).arg( numberOfWells - wellList.size() );
                    timeStepString += startStringFormat.arg( wellList.join( ", " ) );
                }
                else if ( wellPressureList.size() < numberOfWells )
                {
                    timeStepString += startStringFormat.arg( wellList.join( ", " ) );
                }
                else
                {
                    timeStepString += startStringFormat.arg( wellPressureList.join( ", " ) );
                }
            }

            options.push_back( caf::PdmOptionItemInfo( timeStepString, i ) );
        }
    }

    else
    {
        options = RicCaseAndFileExportSettingsUi::calculateValueOptions( fieldNeedingOptions );
    }
    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportCompletionDataSettingsUi::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    {
        caf::PdmUiGroup* group = uiOrdering.addNewGroup( "Export Settings" );
        group->add( &compdatExport );
        group->add( &caseToApply );
        group->add( &useLateralNTG );
        group->add( &m_exportDataSourceAsComment );
        group->add( &m_exportWelspec );
        group->add( &includeMsw );
        group->add( &m_completionWelspecAfterMainBore );
    }

    {
        caf::PdmUiGroup* group = uiOrdering.addNewGroup( "File Settings" );
        group->add( &fileSplit );
        group->add( &m_reportCompletionTypesSeparately );
        group->add( &folder );

        if ( fileSplit() == ExportSplit::UNIFIED_FILE )
        {
            group->add( &m_useCustomFileName );

            group->add( &m_customFileName );
            m_customFileName.uiCapability()->setUiReadOnly( !m_useCustomFileName );
        }
    }

    {
        caf::PdmUiGroup* group = uiOrdering.addNewGroup( "Completions Export Selection" );
        if ( m_perforationsEnabled )
        {
            group->add( &includePerforations );
            group->add( &timeStep );
            if ( !includePerforations )
                timeStep.uiCapability()->setUiReadOnly( true );
            else
                timeStep.uiCapability()->setUiReadOnly( false );
        }

        if ( m_fracturesEnabled )
        {
            group->add( &includeFractures );

            caf::PdmUiGroup* pddGroup = group->addNewGroup( "Pressure Differential Depletion Scaling" );
            pddGroup->setUiReadOnly( !includeFractures() );

            pddGroup->add( &performTransScaling );
            pddGroup->add( &transScalingTimeStep );
            pddGroup->add( &transScalingWBHPSource );
            pddGroup->add( &transScalingWBHP );

            if ( !includeFractures() )
            {
                performTransScaling = false;
                performTransScaling.uiCapability()->setUiReadOnly( true );
            }
            else
            {
                performTransScaling.uiCapability()->setUiReadOnly( false );
            }

            if ( !performTransScaling() )
            {
                transScalingTimeStep.uiCapability()->setUiReadOnly( true );
                transScalingWBHPSource.uiCapability()->setUiReadOnly( true );
                transScalingWBHP.uiCapability()->setUiReadOnly( true );
            }
            else
            {
                transScalingTimeStep.uiCapability()->setUiReadOnly( false );
                transScalingWBHPSource.uiCapability()->setUiReadOnly( false );
                transScalingWBHP.uiCapability()->setUiReadOnly( false );
                if ( transScalingWBHPSource == RicExportFractureCompletionsImpl::WBHP_FROM_SUMMARY )
                {
                    transScalingWBHP.uiCapability()->setUiName( "WBHP Before Production Start" );
                }
                else
                {
                    transScalingWBHP.uiCapability()->setUiName( "User Defined WBHP" );
                }
            }

            // Set visibility
            includeFractures.uiCapability()->setUiHidden( compdatExport == CompdatExport::WPIMULT_AND_DEFAULT_CONNECTION_FACTORS &&
                                                          !includeMsw );
        }

        if ( m_fishbonesEnabled )
        {
            group->add( &includeFishbones );
            group->add( &excludeMainBoreForFishbones );

            // Set visibility
            if ( !includeFishbones )
                excludeMainBoreForFishbones.uiCapability()->setUiReadOnly( true );
            else
                excludeMainBoreForFishbones.uiCapability()->setUiReadOnly( false );
        }
    }

    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<int, std::vector<std::pair<QString, QString>>> RicExportCompletionDataSettingsUi::generateWellProductionStartStrings()
{
    std::map<int, std::vector<std::pair<QString, QString>>> wellProductionStartStrings;

    const RimProject* project = RimProject::current();
    if ( caseToApply )
    {
        for ( const RimWellPath* wellPath : project->allWellPaths() )
        {
            int    initialWellProductionTimeStep = -1;
            double initialWellPressure           = 0.0;
            double currentWellPressure           = 0.0;
            RicExportFractureCompletionsImpl::getWellPressuresAndInitialProductionTimeStepFromSummaryData( caseToApply,
                                                                                                           wellPath->completionSettings()
                                                                                                               ->wellNameForExport(),
                                                                                                           0,
                                                                                                           &initialWellProductionTimeStep,
                                                                                                           &initialWellPressure,
                                                                                                           &currentWellPressure );
            if ( initialWellProductionTimeStep >= 0 )
            {
                QString pressureUnits = RiaEclipseUnitTools::unitStringPressure( wellPath->unitSystem() );
                wellProductionStartStrings[initialWellProductionTimeStep].push_back(
                    std::make_pair( wellPath->name(), QString( "%1 %2" ).arg( initialWellPressure, 4, 'f', 1 ).arg( pressureUnits ) ) );
            }
        }
    }
    return wellProductionStartStrings;
}
