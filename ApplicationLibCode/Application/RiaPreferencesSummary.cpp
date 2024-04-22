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

#include "PlotTemplateCommands/RicSummaryPlotTemplateTools.h"

#include "RiaApplication.h"
#include "RiaColorTables.h"
#include "RiaPreferences.h"

#include "PlotTemplates/RimPlotTemplateFileItem.h"

#include "cafPdmUiCheckBoxEditor.h"
#include "cafPdmUiComboBoxEditor.h"
#include "cafPdmUiListEditor.h"
#include "cafPdmUiPushButtonEditor.h"

#include <algorithm>
#include <vector>

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
    addItem( RiaPreferencesSummary::SummaryHistoryCurveStyleMode::SYMBOLS_AND_LINES, "SYMBOLS_AND_LINES", "Symbols and Lines" );
    setDefault( RiaPreferencesSummary::SummaryHistoryCurveStyleMode::SYMBOLS );
}

template <>
void RiaPreferencesSummary::SummaryReaderModeType::setUp()
{
    addItem( RiaPreferencesSummary::SummaryReaderMode::RESDATA, "RESDATA", "UNSMRY (resdata)", { "LIBECL" } );
    addItem( RiaPreferencesSummary::SummaryReaderMode::HDF5_OPM_COMMON, "HDF5_OPM_COMMON", "h5 (HDF5)" );
    addItem( RiaPreferencesSummary::SummaryReaderMode::OPM_COMMON, "OPM_COMMON", "ESMRY (opm-common)" );
    setDefault( RiaPreferencesSummary::SummaryReaderMode::HDF5_OPM_COMMON );
}

template <>
void RiaPreferencesSummary::DefaultSummaryPlotEnum::setUp()
{
    addItem( RiaPreferencesSummary::DefaultSummaryPlotType::NONE, "NONE", "No Plots" );
    addItem( RiaPreferencesSummary::DefaultSummaryPlotType::DATA_VECTORS, "DATA_VECTORS", "Use Data Vector Names" );
    addItem( RiaPreferencesSummary::DefaultSummaryPlotType::PLOT_TEMPLATES, "PLOT_TEMPLATES", "Use Plot Templates" );
    setDefault( RiaPreferencesSummary::DefaultSummaryPlotType::DATA_VECTORS );
}

} // namespace caf

CAF_PDM_SOURCE_INIT( RiaPreferencesSummary, "RiaPreferencesSummary" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaPreferencesSummary::RiaPreferencesSummary()
{
    CAF_PDM_InitFieldNoDefault( &m_summaryRestartFilesShowImportDialog, "summaryRestartFilesShowImportDialog", "Show Import Dialog" );

    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_summaryRestartFilesShowImportDialog );

    CAF_PDM_InitField( &m_summaryImportMode,
                       "summaryImportMode",
                       SummaryRestartFilesImportModeType( RiaPreferencesSummary::SummaryRestartFilesImportMode::IMPORT ),
                       "Default Summary Import Option" );
    CAF_PDM_InitField( &m_gridImportMode,
                       "gridImportMode",
                       SummaryRestartFilesImportModeType( RiaPreferencesSummary::SummaryRestartFilesImportMode::NOT_IMPORT ),
                       "Default Grid Import Option" );
    CAF_PDM_InitField( &m_summaryEnsembleImportMode,
                       "summaryEnsembleImportMode",
                       SummaryRestartFilesImportModeType( RiaPreferencesSummary::SummaryRestartFilesImportMode::IMPORT ),
                       "Default Ensemble Summary Import Option" );

    CAF_PDM_InitField( &m_defaultSummaryHistoryCurveStyle,
                       "defaultSummaryHistoryCurveStyle",
                       SummaryHistoryCurveStyleModeType( RiaPreferencesSummary::SummaryHistoryCurveStyleMode::SYMBOLS ),
                       "Default Curve Style for History Vectors" );
    CAF_PDM_InitField( &m_defaultSummaryCurvesTextFilter,
                       "defaultSummaryCurvesTextFilter",
                       QString( "FOPT" ),
                       "Default Summary Curves",
                       "",
                       "Semicolon separated list of filters used to create curves in new summary plots",
                       "" );
    CAF_PDM_InitFieldNoDefault( &m_defaultSummaryPlot, "defaultSummaryPlot", "Create Plot On Summary Data Import" );

    CAF_PDM_InitField( &m_crossPlotAddressCombinations,
                       "CrossPlotAddressCombinations",
                       QString( "FWCT FOPT;FWPR FOPT;FWIR FOPT;FGOR FOPT;FGLIR FOPR" ),
                       "Cross Plot Addresses [Y-adr X-adr]",
                       "",
                       "Semicolon separated list used to create cross plot curves. Based on selection, the names will be changed to "
                       "corresponing well or group vector names",
                       "" );

    CAF_PDM_InitField( &m_selectDefaultTemplates, "selectDefaultTemplate", false, "", "", "Select Default Templates" );
    caf::PdmUiPushButtonEditor::configureEditorLabelHidden( &m_selectDefaultTemplates );

    CAF_PDM_InitFieldNoDefault( &m_selectedDefaultTemplates, "defaultSummaryTemplates", "Select Summary Plot Templates" );
    m_selectedDefaultTemplates.uiCapability()->setUiReadOnly( true );
    m_selectedDefaultTemplates.uiCapability()->setUiEditorTypeName( caf::PdmUiListEditor::uiEditorTypeName() );
    m_selectedDefaultTemplates.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );

    CAF_PDM_InitField( &m_createEnhancedSummaryDataFile,
                       "createEnhancedSummaryDataFile_v01",
                       false,
                       "Create ESMRY Summary Files",
                       "",
                       "If not present, create summary file with extension '*.ESMRY'",
                       "" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_createEnhancedSummaryDataFile );

    CAF_PDM_InitField( &m_useEnhancedSummaryDataFile,
                       "useEnhancedSummaryDataFile",
                       true,
                       "Use ESMRY Files",
                       "",
                       "If present, import summary files with extension '*.ESMRY'",
                       "" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_useEnhancedSummaryDataFile );

    CAF_PDM_InitField( &m_createH5SummaryDataFile,
                       "createH5SummaryDataFile_v01",
                       false,
                       "Create h5 Summary Files",
                       "",
                       "If not present, create summary file with extension '*.h5'",
                       "" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_createH5SummaryDataFile );

    CAF_PDM_InitField( &m_createH5SummaryFileThreadCount, "createH5SummaryFileThreadCount", 1, "h5 Summary Export Thread Count" );

    CAF_PDM_InitFieldNoDefault( &m_summaryReader, "summaryReaderType_v01", "File Format" );

    CAF_PDM_InitField( &m_showSummaryTimeAsLongString,
                       "showSummaryTimeAsLongString",
                       false,
                       "Show resample time text as long time text (2010-11-21 23:15:00)" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_showSummaryTimeAsLongString );

    CAF_PDM_InitField( &m_useMultipleThreadsWhenLoadingSummaryCases,
                       "useMultipleThreadsWhenLoadingSummaryCases",
                       true,
                       "Use Multiple Threads for Import of Summary Files" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_useMultipleThreadsWhenLoadingSummaryCases );

    CAF_PDM_InitFieldNoDefault( &m_defaultColumnCount, "DefaultNumberOfColumns", "Columns" );
    m_defaultColumnCount = RiaDefines::ColumnCount::COLUMNS_2;
    CAF_PDM_InitFieldNoDefault( &m_defaultRowsPerPage, "DefaultRowsPerPage", "Rows per Page" );
    m_defaultRowsPerPage = RiaDefines::RowCount::ROWS_2;

    CAF_PDM_InitField( &m_curveColorByPhase, "curveColorByPhase", true, "Curve Color By Phase" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_curveColorByPhase );

    CAF_PDM_InitField( &m_appendHistoryVectors,
                       "appendHistoryVectorForDragDrop",
                       false,
                       "Append History Vectors",
                       "",
                       "When a simulated summary vector is inserted into a plot, also include the corresponding "
                       "history vector" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_appendHistoryVectors );

    CAF_PDM_InitField( &m_historyCurveContrastColor,
                       "historyCurveContrastColor",
                       RiaColorTables::historyCurveContrastColor(),
                       "History Curve Color" );
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
bool RiaPreferencesSummary::useEnhancedSummaryDataFiles() const
{
    return m_useEnhancedSummaryDataFile();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaPreferencesSummary::createEnhancedSummaryDataFiles() const
{
    return m_createEnhancedSummaryDataFile();
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
    uiOrdering.add( &m_defaultSummaryPlot );

    switch ( m_defaultSummaryPlot() )
    {
        case RiaPreferencesSummary::DefaultSummaryPlotType::DATA_VECTORS:
            uiOrdering.add( &m_defaultSummaryCurvesTextFilter );
            break;

        case RiaPreferencesSummary::DefaultSummaryPlotType::PLOT_TEMPLATES:
            uiOrdering.add( &m_selectedDefaultTemplates );
            uiOrdering.add( &m_selectDefaultTemplates );
            break;

        default:
            break;
    }

    uiOrdering.add( &m_crossPlotAddressCombinations );

    auto historyCurveGroup = uiOrdering.addNewGroup( "History Vectors" );

    historyCurveGroup->add( &m_defaultSummaryHistoryCurveStyle );
    historyCurveGroup->add( &m_historyCurveContrastColor );
    historyCurveGroup->add( &m_appendHistoryVectors );

    uiOrdering.add( &m_curveColorByPhase );
    uiOrdering.add( &m_showSummaryTimeAsLongString );

    auto multiGroup = uiOrdering.addNewGroup( "Multi Plot Defaults" );

    multiGroup->add( &m_defaultColumnCount );
    multiGroup->add( &m_defaultRowsPerPage );
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
    if ( m_defaultSummaryPlot() != DefaultSummaryPlotType::DATA_VECTORS ) return {};

    return m_defaultSummaryCurvesTextFilter;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaPreferencesSummary::colorCurvesByPhase() const
{
    return m_curveColorByPhase();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaPreferencesSummary::appendHistoryVectors() const
{
    return m_appendHistoryVectors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaPreferencesSummary::crossPlotAddressCombinations() const
{
    return m_crossPlotAddressCombinations;
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
        if ( RiaApplication::enableDevelopmentFeatures() )
        {
            uiOrdering.add( &m_useEnhancedSummaryDataFile );
        }
        uiOrdering.add( &m_createEnhancedSummaryDataFile );
    }
    else if ( m_summaryReader == SummaryReaderMode::HDF5_OPM_COMMON )
    {
        uiOrdering.add( &m_createH5SummaryDataFile );

        if ( RiaApplication::enableDevelopmentFeatures() )
        {
            uiOrdering.add( &m_createH5SummaryFileThreadCount );
        }
    }

    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaPreferencesSummary::defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_defaultRowsPerPage || field == &m_defaultColumnCount )
    {
        auto myattr = dynamic_cast<caf::PdmUiComboBoxEditorAttribute*>( attribute );
        if ( myattr )
        {
            myattr->iconSize = QSize( 24, 16 );
        }
    }
    else if ( field == &m_selectDefaultTemplates )
    {
        auto attrib = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*>( attribute );
        if ( attrib )
        {
            attrib->m_buttonText = "Select Default Templates";
        }
    }
    else if ( field == &m_selectedDefaultTemplates )
    {
        auto attrib = dynamic_cast<caf::PdmUiListEditorAttribute*>( attribute );
        if ( attrib )
        {
            attrib->heightHint = 30;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RiaPreferencesSummary::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_summaryReader )
    {
        std::vector<SummaryReaderMode> availableModes;

#ifdef USE_HDF5
        availableModes.push_back( SummaryReaderMode::HDF5_OPM_COMMON );
#endif // USE_HDF5
        availableModes.push_back( SummaryReaderMode::RESDATA );
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

        options.push_back( caf::PdmOptionItemInfo( skip.uiText(), RiaPreferencesSummary::SummaryRestartFilesImportMode::NOT_IMPORT ) );
        options.push_back( caf::PdmOptionItemInfo( separate.uiText(), RiaPreferencesSummary::SummaryRestartFilesImportMode::SEPARATE_CASES ) );
    }
    else if ( fieldNeedingOptions == &m_summaryEnsembleImportMode )
    {
        // Manual option handling in order to one only a subset of the enum values
        SummaryRestartFilesImportModeType skip( RiaPreferencesSummary::SummaryRestartFilesImportMode::NOT_IMPORT );
        SummaryRestartFilesImportModeType allowImport( RiaPreferencesSummary::SummaryRestartFilesImportMode::IMPORT );

        options.push_back( caf::PdmOptionItemInfo( skip.uiText(), RiaPreferencesSummary::SummaryRestartFilesImportMode::NOT_IMPORT ) );
        options.push_back( caf::PdmOptionItemInfo( allowImport.uiText(), RiaPreferencesSummary::SummaryRestartFilesImportMode::IMPORT ) );
    }
    else if ( fieldNeedingOptions == &m_defaultColumnCount )
    {
        for ( size_t i = 0; i < ColumnCountEnum::size(); ++i )
        {
            RiaDefines::ColumnCount enumVal = ColumnCountEnum::fromIndex( i );
            QString                 columnCountString =
                ( enumVal == RiaDefines::ColumnCount::COLUMNS_UNLIMITED ) ? "Unlimited" : QString( "%1" ).arg( static_cast<int>( enumVal ) );
            QString iconPath = QString( ":/Columns%1.png" ).arg( columnCountString );
            options.push_back(
                caf::PdmOptionItemInfo( ColumnCountEnum::uiText( enumVal ), enumVal, false, caf::IconProvider( iconPath, QSize( 24, 16 ) ) ) );
        }
    }
    else if ( fieldNeedingOptions == &m_defaultRowsPerPage )
    {
        for ( size_t i = 0; i < RowCountEnum::size(); ++i )
        {
            RiaDefines::RowCount enumVal  = RowCountEnum::fromIndex( i );
            QString              iconPath = QString( ":/Rows%1.png" ).arg( static_cast<int>( enumVal ) );
            options.push_back(
                caf::PdmOptionItemInfo( RowCountEnum::uiText( enumVal ), enumVal, false, caf::IconProvider( iconPath, QSize( 24, 16 ) ) ) );
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::ColumnCount RiaPreferencesSummary::defaultMultiPlotColumnCount() const
{
    return m_defaultColumnCount();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::RowCount RiaPreferencesSummary::defaultMultiPlotRowCount() const
{
    return m_defaultRowsPerPage();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Color3f RiaPreferencesSummary::historyCurveContrastColor() const
{
    return m_historyCurveContrastColor();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaPreferencesSummary::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( changedField == &m_selectDefaultTemplates )
    {
        m_selectDefaultTemplates = false;

        auto selection = RicSummaryPlotTemplateTools::selectDefaultPlotTemplates( m_selectedDefaultTemplates() );
        if ( selection.empty() ) return;

        m_selectedDefaultTemplates = selection;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaPreferencesSummary::DefaultSummaryPlotType RiaPreferencesSummary::defaultSummaryPlotType() const
{
    return m_defaultSummaryPlot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> RiaPreferencesSummary::defaultSummaryPlotTemplates( bool returnEnsembleTemplates ) const
{
    std::vector<QString> templatesToUse;
    for ( auto& fileName : m_selectedDefaultTemplates() )
    {
        bool singleTemplate = fileName.toLower().endsWith( ".rpt" );
        if ( singleTemplate && returnEnsembleTemplates ) continue;
        if ( !singleTemplate && !returnEnsembleTemplates ) continue;

        if ( std::count( templatesToUse.begin(), templatesToUse.end(), fileName ) == 0 ) templatesToUse.push_back( fileName );
    }

    return templatesToUse;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaPreferencesSummary::isDefaultSummaryPlotTemplate( QString filename ) const
{
    int count = std::count( m_selectedDefaultTemplates().begin(), m_selectedDefaultTemplates().end(), filename );
    return ( count > 0 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaPreferencesSummary::addToDefaultPlotTemplates( QString filename )
{
    if ( isDefaultSummaryPlotTemplate( filename ) ) return;

    std::vector<QString> newlist;
    newlist.insert( newlist.end(), m_selectedDefaultTemplates().begin(), m_selectedDefaultTemplates().end() );
    newlist.push_back( filename );
    m_selectedDefaultTemplates = newlist;

    RiaPreferences::current()->writePreferencesToApplicationStore();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaPreferencesSummary::removeFromDefaultPlotTemplates( QString filename )
{
    if ( !isDefaultSummaryPlotTemplate( filename ) ) return;

    std::vector<QString> newlist;

    for ( auto& item : m_selectedDefaultTemplates() )
    {
        if ( item != filename ) newlist.push_back( item );
    }
    m_selectedDefaultTemplates = newlist;

    RiaPreferences::current()->writePreferencesToApplicationStore();
}
