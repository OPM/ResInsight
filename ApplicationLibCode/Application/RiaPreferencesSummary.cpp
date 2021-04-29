/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021     Equinor ASA
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

#include "RiaPreferencesSummary.h"

#include "RiaApplication.h"
#include "RiaPreferences.h"

#include "cafPdmUiCheckBoxEditor.h"

#include <algorithm>

namespace caf
{
template <>
void RiaPreferencesSummary::SummaryRestartFilesImportModeType::setUp()
{
    addItem( RiaPreferencesSummary::SummaryRestartFilesImportMode::IMPORT, "IMPORT", "Unified" );
    addItem( RiaPreferencesSummary::SummaryRestartFilesImportMode::SEPARATE_CASES, "SEPARATE_CASES", "Separate Cases" );
    addItem( RiaPreferencesSummary::SummaryRestartFilesImportMode::NOT_IMPORT, "NOT_IMPORT", "Skip" );
    setDefault( RiaPreferencesSummary::SummaryRestartFilesImportMode::IMPORT );
}

template <>
void RiaPreferencesSummary::SummaryHistoryCurveStyleModeType::setUp()
{
    addItem( RiaPreferencesSummary::SummaryHistoryCurveStyleMode::SYMBOLS, "SYMBOLS", "Symbols" );
    addItem( RiaPreferencesSummary::SummaryHistoryCurveStyleMode::LINES, "LINES", "Lines" );
    addItem( RiaPreferencesSummary::SummaryHistoryCurveStyleMode::SYMBOLS_AND_LINES,
             "SYMBOLS_AND_LINES",
             "Symbols and Lines" );
    setDefault( RiaPreferencesSummary::SummaryHistoryCurveStyleMode::SYMBOLS );
}

template <>
void RiaPreferencesSummary::SummaryReaderModeType::setUp()
{
    addItem( RiaPreferencesSummary::SummaryReaderMode::LIBECL, "LIBECL", "UNSMRY (libecl)" );
    addItem( RiaPreferencesSummary::SummaryReaderMode::HDF5_OPM_COMMON, "HDF5_OPM_COMMON", "h5 (HDF5)" );
    addItem( RiaPreferencesSummary::SummaryReaderMode::OPM_COMMON, "OPM_COMMON", "LODSMRY (opm-common)" );
    setDefault( RiaPreferencesSummary::SummaryReaderMode::HDF5_OPM_COMMON );
}
} // namespace caf

CAF_PDM_SOURCE_INIT( RiaPreferencesSummary, "RiaPreferencesSummary" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaPreferencesSummary::RiaPreferencesSummary()
{
    CAF_PDM_InitFieldNoDefault( &m_summaryRestartFilesShowImportDialog,
                                "summaryRestartFilesShowImportDialog",
                                "Show Import Dialog",
                                "",
                                "",
                                "" );

    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_summaryRestartFilesShowImportDialog );

    CAF_PDM_InitField( &m_summaryImportMode,
                       "summaryImportMode",
                       SummaryRestartFilesImportModeType( RiaPreferencesSummary::SummaryRestartFilesImportMode::IMPORT ),
                       "Default Summary Import Option",
                       "",
                       "",
                       "" );
    CAF_PDM_InitField( &m_gridImportMode,
                       "gridImportMode",
                       SummaryRestartFilesImportModeType( RiaPreferencesSummary::SummaryRestartFilesImportMode::NOT_IMPORT ),
                       "Default Grid Import Option",
                       "",
                       "",
                       "" );
    CAF_PDM_InitField( &m_summaryEnsembleImportMode,
                       "summaryEnsembleImportMode",
                       SummaryRestartFilesImportModeType( RiaPreferencesSummary::SummaryRestartFilesImportMode::IMPORT ),
                       "Default Ensemble Summary Import Option",
                       "",
                       "",
                       "" );

    CAF_PDM_InitField( &m_defaultSummaryHistoryCurveStyle,
                       "defaultSummaryHistoryCurveStyle",
                       SummaryHistoryCurveStyleModeType( RiaPreferencesSummary::SummaryHistoryCurveStyleMode::SYMBOLS ),
                       "Default Curve Style for History Vectors",
                       "",
                       "",
                       "" );
    CAF_PDM_InitField( &m_defaultSummaryCurvesTextFilter,
                       "defaultSummaryCurvesTextFilter",
                       QString( "FOPT" ),
                       "Default Summary Curves",
                       "",
                       "Semicolon separated list of filters used to create curves in new summary plots",
                       "" );

    CAF_PDM_InitField( &m_createOptimizedSummaryDataFile,
                       "createOptimizedSummaryDataFile",
                       false,
                       "Create LODSMRY Summary Files",
                       "",
                       "If not present, create summary file with extension '*.LODSMRY'",
                       "" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_createOptimizedSummaryDataFile );

    CAF_PDM_InitField( &m_useOptimizedSummaryDataFile,
                       "useOptimizedSummaryDataFile",
                       true,
                       "Use LODSMRY Summary Files",
                       "",
                       "If present, import summary files with extension '*.LODSMRY'",
                       "" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_useOptimizedSummaryDataFile );

    CAF_PDM_InitField( &m_createH5SummaryDataFile,
                       "createH5SummaryDataFile",
                       false,
                       "Create h5 Summary Files",
                       "",
                       "If not present, create summary file with extension '*.h5'",
                       "" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_createH5SummaryDataFile );

    CAF_PDM_InitField( &m_checkH5FileTimeStamp,
                       "checkH5FileTimeStamp",
                       false,
                       "Check File Timestamp",
                       "",
                       "Compare timestamp of h5 and SMSPEC, and recreate h5 when required",
                       "" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_checkH5FileTimeStamp );

    CAF_PDM_InitField( &m_createH5SummaryFileThreadCount,
                       "createH5SummaryFileThreadCount",
                       1,
                       "h5 Summary Export Thread Count",
                       "",
                       "",
                       "" );

    CAF_PDM_InitFieldNoDefault( &m_summaryReader, "summaryReaderType", "File Format", "", "", "" );

    CAF_PDM_InitField( &m_showSummaryTimeAsLongString,
                       "showSummaryTimeAsLongString",
                       false,
                       "Show resample time text as long time text (2010-11-21 23:15:00)",
                       "",
                       "",
                       "" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_showSummaryTimeAsLongString );

    CAF_PDM_InitField( &m_useMultipleThreadsWhenLoadingSummaryCases,
                       "useMultipleThreadsWhenLoadingSummaryCases",
                       true,
                       "Use Multiple Threads for Import of Summary Files",
                       "",
                       "",
                       "" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_useMultipleThreadsWhenLoadingSummaryCases );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaPreferencesSummary* RiaPreferencesSummary::current()
{
    return RiaApplication::instance()->preferences()->summaryPreferences();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaPreferencesSummary::SummaryReaderMode RiaPreferencesSummary::summaryDataReader() const
{
    return m_summaryReader();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaPreferencesSummary::useOptimizedSummaryDataFiles() const
{
    return m_useOptimizedSummaryDataFile();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaPreferencesSummary::createOptimizedSummaryDataFiles() const
{
    return m_createOptimizedSummaryDataFile();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaPreferencesSummary::createH5SummaryDataFiles() const
{
    return m_createH5SummaryDataFile();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaPreferencesSummary::checkH5SummaryDataTimeStamp() const
{
    return m_checkH5FileTimeStamp;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiaPreferencesSummary::createH5SummaryDataThreadCount() const
{
    const int minimumThreadCount = 1;

    return std::max( minimumThreadCount, m_createH5SummaryFileThreadCount() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaPreferencesSummary::appendRestartFileGroup( caf::PdmUiOrdering& uiOrdering ) const
{
    caf::PdmUiGroup* restartBehaviourGroup = uiOrdering.addNewGroup( "Origin Files" );
    restartBehaviourGroup->add( &m_summaryRestartFilesShowImportDialog );

    {
        caf::PdmUiGroup* group = restartBehaviourGroup->addNewGroup( "Origin Summary Files" );
        group->add( &m_summaryImportMode );
    }

    {
        caf::PdmUiGroup* group = restartBehaviourGroup->addNewGroup( "Origin Grid Files" );
        group->add( &m_gridImportMode );
    }

    {
        caf::PdmUiGroup* group = restartBehaviourGroup->addNewGroup( "Origin Ensemble Summary Files" );
        group->add( &m_summaryEnsembleImportMode );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaPreferencesSummary::appendItemsToPlottingGroup( caf::PdmUiOrdering& uiOrdering ) const
{
    uiOrdering.add( &m_defaultSummaryCurvesTextFilter );
    uiOrdering.add( &m_defaultSummaryHistoryCurveStyle );
    uiOrdering.add( &m_showSummaryTimeAsLongString );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaPreferencesSummary::showSummaryTimeAsLongString() const
{
    return m_showSummaryTimeAsLongString;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaPreferencesSummary::useMultipleThreadsWhenLoadingSummaryData() const
{
    return m_useMultipleThreadsWhenLoadingSummaryCases;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaPreferencesSummary::summaryRestartFilesShowImportDialog() const
{
    return m_summaryRestartFilesShowImportDialog;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaPreferencesSummary::SummaryRestartFilesImportMode RiaPreferencesSummary::summaryImportMode() const
{
    return m_summaryImportMode();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaPreferencesSummary::SummaryRestartFilesImportMode RiaPreferencesSummary::gridImportMode() const
{
    return m_gridImportMode();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaPreferencesSummary::SummaryRestartFilesImportMode RiaPreferencesSummary::summaryEnsembleImportMode() const
{
    return m_summaryEnsembleImportMode();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaPreferencesSummary::defaultSummaryCurvesTextFilter() const
{
    return m_defaultSummaryCurvesTextFilter;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaPreferencesSummary::SummaryHistoryCurveStyleMode RiaPreferencesSummary::defaultSummaryHistoryCurveStyle() const
{
    return m_defaultSummaryHistoryCurveStyle();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaPreferencesSummary::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_useMultipleThreadsWhenLoadingSummaryCases );
    uiOrdering.add( &m_summaryReader );

    if ( m_summaryReader == SummaryReaderMode::OPM_COMMON )
    {
        uiOrdering.add( &m_useOptimizedSummaryDataFile );
        uiOrdering.add( &m_createOptimizedSummaryDataFile );
    }
    else if ( m_summaryReader == SummaryReaderMode::HDF5_OPM_COMMON )
    {
        uiOrdering.add( &m_createH5SummaryDataFile );
        uiOrdering.add( &m_checkH5FileTimeStamp );

        if ( RiaApplication::instance()->enableDevelopmentFeatures() )
        {
            uiOrdering.add( &m_createH5SummaryFileThreadCount );
        }
    }

    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo>
    RiaPreferencesSummary::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly )
{
    QList<caf::PdmOptionItemInfo> options;
    *useOptionsOnly = true;

    if ( fieldNeedingOptions == &m_summaryReader )
    {
        std::vector<SummaryReaderMode> availableModes;

#ifdef USE_HDF5
        availableModes.push_back( SummaryReaderMode::HDF5_OPM_COMMON );
#endif // USE_HDF5
        availableModes.push_back( SummaryReaderMode::LIBECL );
        availableModes.push_back( SummaryReaderMode::OPM_COMMON );

        for ( auto enumValue : availableModes )
        {
            options.push_back( caf::PdmOptionItemInfo( SummaryReaderModeType::uiText( enumValue ), enumValue ) );
        }
    }
    else if ( fieldNeedingOptions == &m_gridImportMode )
    {
        // Manual option handling in order to one only a subset of the enum values
        SummaryRestartFilesImportModeType skip( RiaPreferencesSummary::SummaryRestartFilesImportMode::NOT_IMPORT );
        SummaryRestartFilesImportModeType separate( RiaPreferencesSummary::SummaryRestartFilesImportMode::SEPARATE_CASES );

        options.push_back(
            caf::PdmOptionItemInfo( skip.uiText(), RiaPreferencesSummary::SummaryRestartFilesImportMode::NOT_IMPORT ) );
        options.push_back( caf::PdmOptionItemInfo( separate.uiText(),
                                                   RiaPreferencesSummary::SummaryRestartFilesImportMode::SEPARATE_CASES ) );
    }
    else if ( fieldNeedingOptions == &m_summaryEnsembleImportMode )
    {
        // Manual option handling in order to one only a subset of the enum values
        SummaryRestartFilesImportModeType skip( RiaPreferencesSummary::SummaryRestartFilesImportMode::NOT_IMPORT );
        SummaryRestartFilesImportModeType allowImport( RiaPreferencesSummary::SummaryRestartFilesImportMode::IMPORT );

        options.push_back(
            caf::PdmOptionItemInfo( skip.uiText(), RiaPreferencesSummary::SummaryRestartFilesImportMode::NOT_IMPORT ) );
        options.push_back( caf::PdmOptionItemInfo( allowImport.uiText(),
                                                   RiaPreferencesSummary::SummaryRestartFilesImportMode::IMPORT ) );
    }

    return options;
}
