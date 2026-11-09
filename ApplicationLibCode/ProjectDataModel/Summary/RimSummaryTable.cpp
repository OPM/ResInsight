/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023- Equinor ASA
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

#include "RimSummaryTable.h"

#include "RiaFeatureCommandContext.h"
#include "RiaHashTools.h"
#include "RiaPreferences.h"
#include "RiaQDateTimeTools.h"
#include "RiaStdStringTools.h"
#include "RiaTimeHistoryCurveResampler.h"
#include "RiaTimeTTools.h"
#include "Summary/RiaSummaryTools.h"

#include "RifSummaryReaderInterface.h"

#include "RimEclipseResultCase.h"
#include "RimEclipseView.h"
#include "RimEnsembleStatisticsCase.h"
#include "RimProject.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSummaryEnsemble.h"
#include "RimTools.h"

#include "RiuMatrixPlotWidget.h"

#include "cafCmdFeatureMenuBuilder.h"
#include "cafPdmUiComboBoxEditor.h"
#include "cafPdmUiDateEditor.h"
#include "cafPdmUiToolButtonEditor.h"
#include "cafPdmUiTreeSelectionEditor.h"
#include "cafProgressInfo.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <tuple>

#include <QContextMenuEvent>
#include <QMenu>

CAF_PDM_SOURCE_INIT( RimSummaryTable, "RimSummaryTable" );

namespace
{
//--------------------------------------------------------------------------------------------------
/// Shows the context menu for a summary table widget. The table data is passed through the command context, as clicking
/// the matrix plot does not select the table in the project tree.
//--------------------------------------------------------------------------------------------------
class SummaryTableContextMenuLauncher : public QObject
{
public:
    SummaryTableContextMenuLauncher( QWidget* widget, RimSummaryTable* table )
        : QObject( widget )
        , m_table( table )
    {
        widget->installEventFilter( this );
    }

protected:
    bool eventFilter( QObject* watched, QEvent* event ) override
    {
        if ( event->type() != QEvent::ContextMenu ) return QObject::eventFilter( watched, event );

        if ( m_table )
        {
            const QString                      title = m_table->description();
            RiaFeatureCommandContextTextHelper helper( title, title + "\n\n" + m_table->asciiDataForPlotExport() );

            caf::CmdFeatureMenuBuilder menuBuilder;
            menuBuilder << "RicShowPlotDataFeature";

            QMenu menu;
            menuBuilder.appendToMenu( &menu );
            if ( !menu.actions().empty() ) menu.exec( static_cast<QContextMenuEvent*>( event )->globalPos() );
        }
        return true;
    }

private:
    caf::PdmPointer<RimSummaryTable> m_table;
};
} // namespace

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
namespace caf
{
template <>
void AppEnum<RimSummaryTable::RangeType>::setUp()
{
    addItem( RimSummaryTable::RangeType::AUTOMATIC, "AUTOMATIC", "Min and Max in Table" );
    addItem( RimSummaryTable::RangeType::USER_DEFINED, "USER_DEFINED_MAX_MIN", "User Defined Range" );
    setDefault( RimSummaryTable::RangeType::AUTOMATIC );
}

template <>
void AppEnum<RimSummaryTable::DataSource>::setUp()
{
    addItem( RimSummaryTable::DataSource::SINGLE_CASE, "SINGLE_CASE", "Single Case" );
    addItem( RimSummaryTable::DataSource::ENSEMBLE_STATISTICS, "ENSEMBLE_STATISTICS", "Ensemble Statistics" );
    setDefault( RimSummaryTable::DataSource::SINGLE_CASE );
}

template <>
void AppEnum<RimSummaryTableTools::ValueFilterType>::setUp()
{
    addItem( RimSummaryTableTools::ValueFilterType::NONE, "NONE", "Disabled" );
    addItem( RimSummaryTableTools::ValueFilterType::ABOVE, "ABOVE", "Above" );
    addItem( RimSummaryTableTools::ValueFilterType::BELOW, "BELOW", "Below" );
    addItem( RimSummaryTableTools::ValueFilterType::BETWEEN, "BETWEEN", "Between" );
    addItem( RimSummaryTableTools::ValueFilterType::OUTSIDE, "OUTSIDE", "Outside" );
    setDefault( RimSummaryTableTools::ValueFilterType::NONE );
}
} // namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryTable::RimSummaryTable()
{
    CAF_PDM_InitObject( "Summary Table", ":/CorrelationMatrixPlot16x16.png" );
    uiCapability()->setUiTreeChildrenHidden( true );

    CAF_PDM_InitField( &m_tableName, "TableName", QString( "Summary Table" ), "Name" );
    CAF_PDM_InitField( &m_isAutomaticName, "AutomaticTableName", true, "Automatic Name" );

    CAF_PDM_InitFieldNoDefault( &m_dataSource, "DataSource", "Data Source" );
    CAF_PDM_InitFieldNoDefault( &m_case, "SummaryCase", "Case" );
    m_case.uiCapability()->setUiTreeChildrenHidden( true );
    CAF_PDM_InitFieldNoDefault( &m_ensemble, "SummaryEnsemble", "Ensemble" );
    m_ensemble.uiCapability()->setUiTreeChildrenHidden( true );
    CAF_PDM_InitFieldNoDefault( &m_ensembleStatistic, "EnsembleStatistic", "Statistic" );
    CAF_PDM_InitFieldNoDefault( &m_vector, "Vector", "Vector" );
    m_vector.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_category, "Categories", RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_WELL, "Category" );
    m_category.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );
    caf::AppEnum<RifEclipseSummaryAddressDefines::SummaryCategory>::setEnumSubset( &m_category,
                                                                                   { RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_WELL,
                                                                                     RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_GROUP,
                                                                                     RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_REGION } );

    CAF_PDM_InitFieldNoDefault( &m_resamplingSelection, "ResamplingSelection", "Date Resampling" );
    m_resamplingSelection.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );
    m_resamplingSelection = RiaDefines::DateTimePeriod::YEAR;

    CAF_PDM_InitFieldNoDefault( &m_valueFilterType, "ValueFilterType", "Value Filter" );
    m_valueFilterType.uiCapability()->setUiToolTip( "Keep rows with at least one value matching the filter. Use Outside to remove rows "
                                                    "where all values are in a range, for example between -50 and 50." );
    CAF_PDM_InitField( &m_upperThreshold, "UpperThreshold", 0.0, "Upper Threshold" );
    CAF_PDM_InitField( &m_lowerThreshold, "LowerThreshold", 0.0, "Lower Threshold" );

    // The default is NaN to detect if the keyword was present in the project file
    CAF_PDM_InitField( &m_thresholdValue_OBSOLETE, "ThresholdValue", std::numeric_limits<double>::quiet_NaN(), "Threshold" );
    m_thresholdValue_OBSOLETE.uiCapability()->setUiHidden( true );
    m_thresholdValue_OBSOLETE.xmlCapability()->setIOWritable( false );

    CAF_PDM_InitFieldNoDefault( &m_excludedRowsUiField, "ExcludedTableRows", "Exclude Rows" );
    m_excludedRowsUiField.uiCapability()->setUiEditorTypeName( caf::PdmUiTreeSelectionEditor::uiEditorTypeName() );

    // Table settings
    CAF_PDM_InitField( &m_showValueLabels, "ShowValueLabels", false, "Show Value Labels" );
    CAF_PDM_InitField( &m_maxTimeLabelCount, "MaxTimeLabelCount", 20, "Maximum Time Label Count" );

    // Font control
    CAF_PDM_InitFieldNoDefault( &m_axisTitleFontSize, "AxisTitleFontSize", "Axis Title Font Size" );
    CAF_PDM_InitFieldNoDefault( &m_axisLabelFontSize, "AxisLabelFontSize", "Axis Label Font Size" );
    CAF_PDM_InitFieldNoDefault( &m_valueLabelFontSize, "ValueLabelFontSize", "Value Label Font Size" );
    m_axisTitleFontSize = caf::FontTools::RelativeSize::Large;
    m_axisLabelFontSize = caf::FontTools::RelativeSize::Medium;

    CAF_PDM_InitFieldNoDefault( &m_legendConfig, "LegendConfig", "" );
    m_legendConfig = new RimRegularLegendConfig();
    m_legendConfig->setShowLegend( true );
    m_legendConfig->setAutomaticRanges( 0.0, 100.0, 0.0, 100.0 );
    m_legendConfig->setColorLegend( RimRegularLegendConfig::mapToColorLegend( RimRegularLegendConfig::ColorRangesType::HEAT_MAP ) );

    CAF_PDM_InitField( &m_mappingType, "MappingType", RimRegularLegendConfig::MappingType::LINEAR_CONTINUOUS, "Mapping Type" );
    caf::AppEnum<RimRegularLegendConfig::MappingType>::setEnumSubset( &m_mappingType,
                                                                      { RimRegularLegendConfig::MappingType::LINEAR_DISCRETE,
                                                                        RimRegularLegendConfig::MappingType::LINEAR_CONTINUOUS,
                                                                        RimRegularLegendConfig::MappingType::LOG10_CONTINUOUS,
                                                                        RimRegularLegendConfig::MappingType::LOG10_DISCRETE } );

    CAF_PDM_InitFieldNoDefault( &m_rangeType, "RangeType", "Range Type" );

    CAF_PDM_InitField( &m_filterTimeSteps, "FilterTimeSteps", false, "Filter Time Steps" );
    CAF_PDM_InitField( &m_startDate, "StartDate", QDateTime::currentDateTime(), "Start Date" );
    CAF_PDM_InitField( &m_endDate, "EndDate", QDateTime::currentDateTime(), "End Date" );

    const auto dateFormatCallback = [this]() -> QString
    {
        if ( m_resamplingSelection() == RiaDefines::DateTimePeriod::DECADE || m_resamplingSelection() == RiaDefines::DateTimePeriod::YEAR )
        {
            return "yyyy";
        }
        if ( m_resamplingSelection() == RiaDefines::DateTimePeriod::MONTH || m_resamplingSelection() == RiaDefines::DateTimePeriod::QUARTER ||
             m_resamplingSelection() == RiaDefines::DateTimePeriod::HALFYEAR )
        {
            return "yyyy-MM";
        }
        return "yyyy-MM-dd";
    };
    m_startDate.uiCapability()->setAttribute<std::function<QString()>>( caf::PdmUiDateEditor::Keys::DATE_FORMAT, dateFormatCallback );
    m_endDate.uiCapability()->setAttribute<std::function<QString()>>( caf::PdmUiDateEditor::Keys::DATE_FORMAT, dateFormatCallback );

    setLegendsVisible( true );
    dockAsPlotWindow();
    setShowWindow( true );
    setDeletable( true );

    m_ensembleStatisticsCase = std::make_unique<RimEnsembleStatisticsCase>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryTable::~RimSummaryTable()
{
    if ( isMainDockedWindow() ) removeWindowFromDock();

    cleanupBeforeClose();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryTable::setDefaultCaseAndCategoryAndVectorName()
{
    const auto summaryCases = getToplevelSummaryCases();
    m_case                  = nullptr;
    m_category              = RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_WELL;
    m_vector                = "";

    m_tableName = createTableName();
    if ( summaryCases.empty() ) return;

    m_case = summaryCases.front();

    resetVectorToFirstAvailable();
    m_tableName = createTableName();

    initializeDateRange();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryTable::setFromCaseAndCategoryAndVectorName( RimSummaryCase*                                  summaryCase,
                                                           RifEclipseSummaryAddressDefines::SummaryCategory category,
                                                           const QString&                                   vectorName )
{
    m_case      = summaryCase;
    m_category  = category;
    m_vector    = vectorName;
    m_tableName = createTableName();

    initializeDateRange();

    onLoadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryTable::setDescription( const QString& description )
{
    m_tableName = description;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryTable::cleanupBeforeClose()
{
    if ( m_matrixPlotWidget )
    {
        m_matrixPlotWidget->qwtPlot()->detachItems();
        m_matrixPlotWidget->setParent( nullptr );
        delete m_matrixPlotWidget;
        m_matrixPlotWidget = nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryTable::initAfterRead()
{
    RimPlotWindow::initAfterRead();

    // The threshold was always applied in older project files, and kept rows with at least one value equal to or above the
    // threshold. A threshold of 0.0 (the old default) only excluded rows where all values are negative. Such rows are rare for
    // rates, so a threshold of 0.0 is not migrated, and these rows are now kept.
    if ( !std::isnan( m_thresholdValue_OBSOLETE() ) )
    {
        if ( m_thresholdValue_OBSOLETE() != 0.0 )
        {
            const auto valueFilter = RimSummaryTableTools::valueFilterFromLegacyThreshold( m_thresholdValue_OBSOLETE() );
            m_valueFilterType      = valueFilter.type;
            m_lowerThreshold       = valueFilter.lowerThreshold;
        }
        m_thresholdValue_OBSOLETE = std::numeric_limits<double>::quiet_NaN();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryTable::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    RimPlotWindow::fieldChangedByUi( changedField, oldValue, newValue );

    if ( changedField == &m_case || changedField == &m_ensemble || changedField == &m_dataSource )
    {
        clearEnsembleStatisticsCache();
        resetVectorToFirstAvailable();
        if ( changedField == &m_dataSource && m_dataSource() == DataSource::ENSEMBLE_STATISTICS && m_ensemble() == nullptr )
        {
            for ( auto ensemble : RimProject::current()->summaryEnsembles() )
            {
                if ( ensemble->isEnsemble() )
                {
                    m_ensemble = ensemble;
                    resetVectorToFirstAvailable();
                    break;
                }
            }
        }
        if ( m_isAutomaticName )
        {
            m_tableName = createTableName();
        }
        if ( changedField != &m_case ) initializeDateRange();
        updateTableAndPlot();
    }
    else if ( changedField == &m_category )
    {
        resetVectorToFirstAvailable();
        if ( m_isAutomaticName )
        {
            m_tableName = createTableName();
        }
        updateTableAndPlot();
    }
    else if ( changedField == &m_vector || changedField == &m_resamplingSelection || changedField == &m_valueFilterType ||
              changedField == &m_upperThreshold || changedField == &m_lowerThreshold || changedField == &m_ensembleStatistic )
    {
        if ( m_isAutomaticName )
        {
            m_tableName = createTableName();
        }
        updateTableAndPlot();
    }
    else if ( changedField == &m_excludedRowsUiField )
    {
        updateTableAndPlot();
    }
    else if ( changedField == &m_isAutomaticName && m_isAutomaticName )
    {
        m_tableName = createTableName();
    }
    else if ( m_matrixPlotWidget && changedField == &m_showValueLabels )
    {
        m_matrixPlotWidget->setShowValueLabel( m_showValueLabels );
    }
    else if ( m_matrixPlotWidget && ( changedField == &m_titleFontSize || changedField == &m_legendFontSize ||
                                      changedField == &m_axisTitleFontSize || changedField == &m_axisLabelFontSize ) )
    {
        m_matrixPlotWidget->setPlotTitleFontSize( titleFontSize() );
        m_matrixPlotWidget->setLegendFontSize( legendFontSize() );
        m_matrixPlotWidget->setAxisTitleFontSize( axisTitleFontSize() );
        m_matrixPlotWidget->setAxisLabelFontSize( axisLabelFontSize() );
    }
    else if ( changedField == &m_valueLabelFontSize && m_matrixPlotWidget )
    {
        m_matrixPlotWidget->setValueFontSize( valueLabelFontSize() );
    }
    else if ( changedField == &m_rangeType && m_legendConfig )
    {
        auto rangeMode = m_rangeType == RangeType::AUTOMATIC ? RimLegendConfig::RangeModeType::AUTOMATIC_ALLTIMESTEPS
                                                             : RimLegendConfig::RangeModeType::USER_DEFINED;
        m_legendConfig->setRangeMode( rangeMode );
        m_legendConfig->updateTickCountAndUserDefinedRange();
        updateTableAndPlot();
    }
    else if ( changedField == &m_mappingType && m_legendConfig )
    {
        m_legendConfig->setMappingMode( m_mappingType() );
        if ( m_rangeType == RangeType::AUTOMATIC )
        {
            m_legendConfig->updateTickCountAndUserDefinedRange();
        }
        updateTableAndPlot();
    }
    else if ( changedField == &m_maxTimeLabelCount || changedField == &m_startDate || changedField == &m_endDate ||
              changedField == &m_filterTimeSteps )
    {
        updateTableAndPlot();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryTable::childFieldChangedByUi( const caf::PdmFieldHandle* changedChildField )
{
    updateTableAndPlot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryTable::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    RimViewWindow::defineUiOrdering( uiConfigName, uiOrdering );

    caf::PdmUiGroup& dataGroup = *uiOrdering.addNewGroup( "Plot Data" );
    dataGroup.add( &m_tableName );
    dataGroup.add( &m_isAutomaticName );
    dataGroup.add( &m_dataSource );
    if ( isEnsembleStatistics() )
    {
        dataGroup.add( &m_ensemble );
        dataGroup.add( &m_ensembleStatistic );
    }
    else
    {
        dataGroup.add( &m_case );
    }
    dataGroup.add( &m_category );
    dataGroup.add( &m_vector );
    dataGroup.add( &m_resamplingSelection );
    dataGroup.add( &m_excludedRowsUiField );

    caf::PdmUiGroup* valueFilterGroup = dataGroup.addNewGroup( "Value Filter" );
    valueFilterGroup->add( &m_valueFilterType );
    switch ( m_valueFilterType() )
    {
        case RimSummaryTableTools::ValueFilterType::ABOVE:
            m_lowerThreshold.uiCapability()->setUiName( "Threshold" );
            valueFilterGroup->add( &m_lowerThreshold );
            break;

        case RimSummaryTableTools::ValueFilterType::BELOW:
            m_upperThreshold.uiCapability()->setUiName( "Threshold" );
            valueFilterGroup->add( &m_upperThreshold );
            break;

        case RimSummaryTableTools::ValueFilterType::BETWEEN:
        case RimSummaryTableTools::ValueFilterType::OUTSIDE:
            m_upperThreshold.uiCapability()->setUiName( "Upper Threshold" );
            valueFilterGroup->add( &m_upperThreshold );
            m_lowerThreshold.uiCapability()->setUiName( "Lower Threshold" );
            valueFilterGroup->add( &m_lowerThreshold );
            break;

        case RimSummaryTableTools::ValueFilterType::NONE:
        default:
            break;
    }

    if ( m_valueFilterType() != RimSummaryTableTools::ValueFilterType::NONE )
    {
        valueFilterGroup->addNewLabel( "Value range: " + valueRangeText() );
    }

    caf::PdmUiGroup* timeFilterGroup = dataGroup.addNewGroup( "Time Filter" );
    timeFilterGroup->add( &m_filterTimeSteps );
    timeFilterGroup->add( &m_startDate );
    timeFilterGroup->add( &m_endDate );

    m_startDate.uiCapability()->setUiReadOnly( !m_filterTimeSteps() );
    m_endDate.uiCapability()->setUiReadOnly( !m_filterTimeSteps() );

    caf::PdmUiGroup* tableSettingsGroup = uiOrdering.addNewGroup( "Table Settings" );
    tableSettingsGroup->add( &m_showValueLabels );
    m_legendConfig->uiOrdering( "FlagAndColorsOnly", *tableSettingsGroup );
    tableSettingsGroup->add( &m_mappingType );
    tableSettingsGroup->add( &m_rangeType );
    m_legendConfig->uiOrdering( "UserDefinedMinMaxOnly", *tableSettingsGroup );

    caf::PdmUiGroup* fontGroup = uiOrdering.addNewGroup( "Fonts" );
    fontGroup->setCollapsedByDefault();
    RimPlotWindow::uiOrderingForFonts( uiConfigName, *fontGroup );
    fontGroup->add( &m_axisTitleFontSize );
    fontGroup->add( &m_axisLabelFontSize );
    fontGroup->add( &m_valueLabelFontSize );
    fontGroup->add( &m_maxTimeLabelCount );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimSummaryTable::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options = RimPlotWindow::calculateValueOptions( fieldNeedingOptions );
    if ( !options.empty() )
    {
        return options;
    }
    else if ( fieldNeedingOptions == &m_case )
    {
        options = RiaSummaryTools::optionsForAllSummaryCases();
    }
    else if ( fieldNeedingOptions == &m_ensemble )
    {
        for ( auto ensemble : RimProject::current()->summaryEnsembles() )
        {
            if ( ensemble->isEnsemble() )
            {
                options.push_back( caf::PdmOptionItemInfo( ensemble->name(), ensemble, false, ensemble->uiIconProvider() ) );
            }
        }
    }
    else if ( fieldNeedingOptions == &m_vector )
    {
        const auto categoryVectorsUnion = getCategoryVectorFromAddresses( availableAddresses(), m_category() );
        for ( const auto& vectorName : categoryVectorsUnion )
        {
            options.push_back( caf::PdmOptionItemInfo( vectorName, vectorName ) );
        }
    }
    else if ( fieldNeedingOptions == &m_excludedRowsUiField )
    {
        const auto vectorNames = RimSummaryTableTools::categoryNames( m_tableData.vectorDataCollection );
        for ( const auto& vectorName : vectorNames )
        {
            options.push_back( caf::PdmOptionItemInfo( vectorName, vectorName ) );
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryTable::onLoadDataAndUpdate()
{
    // Called when data is loaded or reloaded from outside the property editor, so the source data may have changed
    clearEnsembleStatisticsCache();
    updateTableAndPlot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryTable::updateTableAndPlot()
{
    updateDockWindowVisibility();
    createTableData();
    setExcludedRowsUiSelectionsFromTableData();

    if ( m_matrixPlotWidget == nullptr || !hasDataSource() )
    {
        return;
    }

    // Exclude rows that are selected
    const auto excludedRows = this->excludedRows();

    // Convert to strings
    const auto timeStepStrings = this->timeStepStrings();

    // Clear matrix plot
    m_matrixPlotWidget->clearPlotData();
    m_matrixPlotWidget->setColumnHeaders( timeStepStrings );

    double posClosestToZeroValue = std::numeric_limits<double>::max();
    double negClosestToZeroValue = std::numeric_limits<double>::lowest();
    for ( const auto& vectorData : m_tableData.vectorDataCollection )
    {
        if ( excludedRows.contains( vectorData.category ) ) continue;

        // Find positive and negative value closest to zero
        for ( const auto& value : vectorData.values )
        {
            if ( value > 0.0 && value < posClosestToZeroValue ) posClosestToZeroValue = value;
            if ( value < 0.0 && value > negClosestToZeroValue ) negClosestToZeroValue = value;
        }

        m_matrixPlotWidget->setRowValues( vectorData.category,
                                          RimSummaryTableTools::valuesForTimeSteps( vectorData, m_tableData.timeStepsUnion ) );
    }

    if ( m_legendConfig )
    {
        m_legendConfig->setAutomaticRanges( m_tableData.minValue, m_tableData.maxValue, 0.0, 0.0 );
        m_legendConfig->setClosestToZeroValues( posClosestToZeroValue, negClosestToZeroValue, posClosestToZeroValue, negClosestToZeroValue );
    }

    // Set titles and font sizes
    const QString title = QString( "Summary Table - %1 [%2]<br>%3Date Resampling: %4</br>" )
                              .arg( m_vector() )
                              .arg( m_tableData.unitName )
                              .arg( isEnsembleStatistics() ? dataSourceDescription() + "<br>" : QString() )
                              .arg( m_resamplingSelection().uiText() );
    m_matrixPlotWidget->setPlotTitle( title );
    m_matrixPlotWidget->setRowTitle( QString( "%1s" ).arg( m_category().uiText() ) );
    m_matrixPlotWidget->setColumnTitle( "Time steps" );
    m_matrixPlotWidget->setPlotTitleFontSize( titleFontSize() );
    m_matrixPlotWidget->setLegendFontSize( legendFontSize() );
    m_matrixPlotWidget->setAxisTitleFontSize( axisTitleFontSize() );
    m_matrixPlotWidget->setAxisLabelFontSize( axisLabelFontSize() );
    m_matrixPlotWidget->setMaxColumnLabelCount( m_maxTimeLabelCount() );

    const auto windowTitle = RiaStdStringTools::removeHtmlTags( title.toStdString() );
    m_matrixPlotWidget->setWindowTitle( QString::fromStdString( windowTitle ) );

    m_matrixPlotWidget->createPlot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RimSummaryTable::viewWidget()
{
    return m_matrixPlotWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryTable::zoomAll()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RimSummaryTable::createViewWidget( QWidget* mainWindowParent )
{
    m_matrixPlotWidget = new RiuMatrixPlotWidget( this, m_legendConfig, mainWindowParent );
    m_matrixPlotWidget->setShowValueLabel( m_showValueLabels );
    m_matrixPlotWidget->setUseInvalidValueColor( true );

    new SummaryTableContextMenuLauncher( m_matrixPlotWidget, this );
    new SummaryTableContextMenuLauncher( m_matrixPlotWidget->qwtPlot(), this );
    return m_matrixPlotWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryTable::deleteViewWidget()
{
    cleanupBeforeClose();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummaryTable::description() const
{
    return m_tableName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryTable::doRenderWindowContent( QPaintDevice* paintDevice )
{
    return;
}

caf::PdmFieldHandle* RimSummaryTable::userDescriptionField()
{
    return &m_tableName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimSummaryTable::axisTitleFontSize() const
{
    return caf::FontTools::absolutePointSize( RiaPreferences::current()->defaultPlotFontSize(), m_axisTitleFontSize() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimSummaryTable::axisLabelFontSize() const
{
    return caf::FontTools::absolutePointSize( RiaPreferences::current()->defaultPlotFontSize(), m_axisLabelFontSize() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimSummaryTable::valueLabelFontSize() const
{
    return caf::FontTools::absolutePointSize( RiaPreferences::current()->defaultPlotFontSize(), m_valueLabelFontSize() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummaryTable::valueRangeText() const
{
    const auto& minMax = m_tableData.unfilteredMinMax;
    if ( !minMax ) return "No data";

    return QString( "[%1..%2]" ).arg( m_legendConfig->valueToText( minMax->first ) ).arg( m_legendConfig->valueToText( minMax->second ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummaryTable::createTableName() const
{
    if ( isEnsembleStatistics() )
    {
        return QString( "Summary Table - %1 (%2)" ).arg( m_vector() ).arg( m_ensembleStatistic().uiText() );
    }
    return QString( "Summary Table - %1" ).arg( m_vector() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<double, double> RimSummaryTable::createLegendMinMaxValues( const double maxTableValue ) const
{
    return std::make_pair( 0.0, maxTableValue );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummaryTable::dateFormatString() const
{
    return RiaQDateTimeTools::dateFormatString( RiaPreferences::current()->dateFormat(),
                                                RiaDefines::DateFormatComponents::DATE_FORMAT_YEAR_MONTH_DAY );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RifEclipseSummaryAddress> RimSummaryTable::getSummaryAddressesFromAddresses( const std::set<RifEclipseSummaryAddress>& allAddresses,
                                                                                      RifEclipseSummaryAddressDefines::SummaryCategory category,
                                                                                      const QString& vector ) const
{
    std::set<RifEclipseSummaryAddress> categoryAddresses;
    const std::string                  selectedVector = vector.toStdString();
    for ( const auto& resAdr : allAddresses )
    {
        if ( resAdr.category() != category || resAdr.vectorName() != selectedVector ) continue;
        categoryAddresses.emplace( resAdr );
    }
    return categoryAddresses;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<QString> RimSummaryTable::getCategoryVectorFromAddresses( const std::set<RifEclipseSummaryAddress>&        allAddresses,
                                                                   RifEclipseSummaryAddressDefines::SummaryCategory category ) const
{
    if ( category != RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_WELL &&
         category != RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_GROUP &&
         category != RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_REGION )
    {
        return {};
    }

    std::set<QString> categoryVectors;
    for ( const auto& resAdr : allAddresses )
    {
        if ( resAdr.category() != category ) continue;
        const QString vectorName = QString::fromStdString( resAdr.vectorName() );
        categoryVectors.emplace( vectorName );
    }
    return categoryVectors;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummaryTable::getCategoryNameFromAddress( const RifEclipseSummaryAddress& address ) const
{
    if ( address.category() == RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_WELL )
    {
        return QString::fromStdString( address.wellName() );
    }
    else if ( address.category() == RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_GROUP )
    {
        return QString::fromStdString( address.groupName() );
    }
    else if ( address.category() == RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_REGION )
    {
        return QString( "Region %1" ).arg( address.regionNumber() );
    }
    return QString();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryCase*> RimSummaryTable::getToplevelSummaryCases() const
{
    RimSummaryCaseMainCollection* summaryCaseMainCollection = RiaSummaryTools::summaryCaseMainCollection();
    if ( !summaryCaseMainCollection ) return {};

    return summaryCaseMainCollection->topLevelSummaryCases();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryTable::initializeDateRange()
{
    std::vector<time_t> timeSteps;
    if ( isEnsembleStatistics() )
    {
        if ( !m_ensemble ) return;
        const auto ensembleTimeSteps = m_ensemble->ensembleTimeSteps();
        timeSteps                    = std::vector<time_t>( ensembleTimeSteps.begin(), ensembleTimeSteps.end() );
    }
    else
    {
        auto summaryReader = summaryReaderWithAddresses();
        if ( !summaryReader ) return;

        const std::set<RifEclipseSummaryAddress> allResultAddresses = summaryReader->allResultAddresses();
        if ( allResultAddresses.empty() ) return;

        timeSteps = summaryReader->timeSteps( *allResultAddresses.begin() );
    }
    if ( timeSteps.empty() ) return;

    m_startDate = RiaQDateTimeTools::fromTime_t( timeSteps.front() );
    m_endDate   = RiaQDateTimeTools::fromTime_t( timeSteps.back() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RifEclipseSummaryAddress> RimSummaryTable::availableAddresses()
{
    if ( isEnsembleStatistics() )
    {
        if ( !m_ensemble ) return {};
        return m_ensemble->ensembleSummaryAddresses();
    }

    if ( auto summaryReader = summaryReaderWithAddresses() )
    {
        return summaryReader->allResultAddresses();
    }
    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryTable::isEnsembleStatistics() const
{
    return m_dataSource() == DataSource::ENSEMBLE_STATISTICS;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryTable::hasDataSource() const
{
    return isEnsembleStatistics() ? m_ensemble() != nullptr : m_case() != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryTable::resetVectorToFirstAvailable()
{
    const auto categoryVectors = getCategoryVectorFromAddresses( availableAddresses(), m_category() );
    m_vector                   = categoryVectors.empty() ? QString( "" ) : *categoryVectors.begin();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummaryTable::dataSourceDescription() const
{
    if ( !isEnsembleStatistics() ) return m_case ? m_case->displayCaseName() : QString();

    const QString ensembleName = m_ensemble ? m_ensemble->name() : QString();
    return QString( "Ensemble %1: %2" ).arg( ensembleName ).arg( m_ensembleStatistic().uiText() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> RimSummaryTable::timeStepStrings() const
{
    std::vector<QString> strings;
    for ( const auto& timeStep : m_tableData.timeStepsUnion )
    {
        strings.push_back( RiaQDateTimeTools::fromTime_t( timeStep ).toString( dateFormatString() ) );
    }
    return strings;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<QString> RimSummaryTable::excludedRows() const
{
    return std::set<QString>( m_excludedRowsUiField().begin(), m_excludedRowsUiField().end() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummaryTable::asciiDataForPlotExport() const
{
    const auto excludedRows = this->excludedRows();

    // Build all cells first, so that each column can be padded to a common width. The padded cells are still
    // separated by tabs, to keep the text both readable and easy to paste into a spreadsheet.
    std::vector<std::vector<QString>> rows;

    std::vector<QString> header = { m_category().uiText() };
    for ( const auto& timeStepString : timeStepStrings() )
    {
        header.push_back( timeStepString );
    }
    rows.push_back( header );

    for ( const auto& vectorData : m_tableData.vectorDataCollection )
    {
        if ( excludedRows.contains( vectorData.category ) ) continue;

        // Values are aligned to the time steps of the header, and time steps without a value are left empty
        std::vector<QString> row = { vectorData.category };
        for ( const double value : RimSummaryTableTools::valuesForTimeSteps( vectorData, m_tableData.timeStepsUnion ) )
        {
            row.push_back( std::isnan( value ) ? QString() : QString::number( value, 'g', 6 ) );
        }
        rows.push_back( row );
    }

    std::vector<qsizetype> columnWidths;
    for ( const auto& row : rows )
    {
        if ( columnWidths.size() < row.size() ) columnWidths.resize( row.size(), 0 );
        for ( size_t col = 0; col < row.size(); ++col )
        {
            columnWidths[col] = std::max( columnWidths[col], row[col].size() );
        }
    }

    QString text;
    for ( const auto& row : rows )
    {
        QStringList cells;
        for ( size_t col = 0; col < row.size(); ++col )
        {
            // Row names are left aligned, values and dates are right aligned
            cells.push_back( col == 0 ? row[col].leftJustified( columnWidths[col] ) : row[col].rightJustified( columnWidths[col] ) );
        }
        text += cells.join( "\t" ) + "\n";
    }

    return text;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifSummaryReaderInterface* RimSummaryTable::summaryReaderWithAddresses()
{
    if ( !m_case ) return nullptr;

    if ( auto summaryReader = m_case->summaryReader() )
    {
        // This is required for ensemble cases, addresses are created on demand
        // Consider moving this to the summary reader interface, and make the call as part of the pure virtual method
        // virtual RifSummaryReaderInterface* summaryReader() = 0; in RimSummaryCase
        summaryReader->createAddressesIfRequired();
        return summaryReader;
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryTable::updateEnsembleStatisticsCacheKey()
{
    // Only the statistics for the current vector and category are kept. The realizations are part of the key, so that the
    // cache is cleared if realizations are added, removed or replaced.
    std::vector<RimSummaryCase*> summaryCases;
    if ( m_ensemble ) summaryCases = m_ensemble->allSummaryCases();

    const auto cacheKey = RiaHashTools::hash( m_ensemble(), summaryCases, static_cast<int>( m_category().value() ), m_vector().toStdString() );
    if ( cacheKey != m_ensembleStatisticsCacheKey )
    {
        clearEnsembleStatisticsCache();
        m_ensembleStatisticsCacheKey = cacheKey;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RimSummaryTable::EnsembleStatisticsData& RimSummaryTable::ensembleStatisticsData( const RifEclipseSummaryAddress& address )
{
    if ( auto it = m_ensembleStatisticsCache.find( address ); it != m_ensembleStatisticsCache.end() ) return it->second;

    EnsembleStatisticsData data;
    if ( m_ensemble )
    {
        // The realization tooltips and read-only state are not updated, as they would only reflect the last calculated address
        const bool includeIncompleteCurves  = false;
        const bool includeZeroValues        = true;
        const bool updateRealizationUiState = false;
        m_ensembleStatisticsCase->calculate( m_ensemble->allSummaryCases(),
                                             address,
                                             includeIncompleteCurves,
                                             { 0, 100 },
                                             includeZeroValues,
                                             updateRealizationUiState );

        // Percentiles use the switched (oil industry) convention: percentile 0 is the maximum, percentile 100 is the minimum
        const auto statisticsAddress = [&address]( RiaDefines::StatisticsType statistic )
        {
            auto adr = address;
            switch ( statistic )
            {
                case RiaDefines::StatisticsType::MIN:
                    adr.setStatisticsType( RifEclipseSummaryAddressDefines::StatisticsType::CUSTOM );
                    adr.setPercentile( 100 );
                    break;
                case RiaDefines::StatisticsType::MAX:
                    adr.setStatisticsType( RifEclipseSummaryAddressDefines::StatisticsType::CUSTOM );
                    adr.setPercentile( 0 );
                    break;
                case RiaDefines::StatisticsType::P10:
                    adr.setStatisticsType( RifEclipseSummaryAddressDefines::StatisticsType::P10 );
                    break;
                case RiaDefines::StatisticsType::P50:
                    adr.setStatisticsType( RifEclipseSummaryAddressDefines::StatisticsType::P50 );
                    break;
                case RiaDefines::StatisticsType::P90:
                    adr.setStatisticsType( RifEclipseSummaryAddressDefines::StatisticsType::P90 );
                    break;
                case RiaDefines::StatisticsType::MEAN:
                    adr.setStatisticsType( RifEclipseSummaryAddressDefines::StatisticsType::MEAN );
                    break;
            }
            return adr;
        };

        data.timeSteps = m_ensembleStatisticsCase->timeSteps( address );

        // The unit is read from the first realization, which only knows the plain (non-statistics) address
        data.unitName = m_ensembleStatisticsCase->unitName( address );

        for ( auto statistic : { RiaDefines::StatisticsType::MIN,
                                 RiaDefines::StatisticsType::P90,
                                 RiaDefines::StatisticsType::P50,
                                 RiaDefines::StatisticsType::MEAN,
                                 RiaDefines::StatisticsType::P10,
                                 RiaDefines::StatisticsType::MAX } )
        {
            auto [isOk, values]    = m_ensembleStatisticsCase->values( statisticsAddress( statistic ) );
            data.values[statistic] = std::move( values );
        }
    }

    return m_ensembleStatisticsCache.emplace( address, std::move( data ) ).first->second;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryTable::clearEnsembleStatisticsCache()
{
    m_ensembleStatisticsCache.clear();
    m_ensembleStatisticsCacheKey = 0;

    // The statistics case skips the calculation if the input is unchanged, which would return stale values after a reload
    m_ensembleStatisticsCase = std::make_unique<RimEnsembleStatisticsCase>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryTable::createTableData()
{
    // Without a value filter, all rows are kept, and values above zero are used to find the first and last time step
    m_tableData             = TableData();
    m_tableData.valueFilter = { .type = m_valueFilterType(), .lowerThreshold = m_lowerThreshold(), .upperThreshold = m_upperThreshold() };

    const auto valueFilter        = m_tableData.valueFilter;
    const bool useThresholdFilter = valueFilter.type != RimSummaryTableTools::ValueFilterType::NONE;

    // Get all summary addresses for selected category (group, region, well)
    RifSummaryReaderInterface* summaryReader = nullptr;
    if ( !isEnsembleStatistics() )
    {
        summaryReader = summaryReaderWithAddresses();
        if ( !summaryReader ) return;
    }
    else if ( !m_ensemble )
    {
        return;
    }

    // Create time step value for vectors with no values matching the value filter
    const time_t invalidTimeStep = 0;

    const auto summaryAddresses = getSummaryAddressesFromAddresses( availableAddresses(), m_category(), m_vector() );

    // Ensemble statistics are calculated on the GUI thread, so show progress when statistics for any address must be calculated
    std::unique_ptr<caf::ProgressInfo> progressInfo;
    if ( isEnsembleStatistics() )
    {
        updateEnsembleStatisticsCacheKey();

        const bool isCalculationRequired = std::any_of( summaryAddresses.begin(),
                                                        summaryAddresses.end(),
                                                        [this]( const RifEclipseSummaryAddress& adr )
                                                        { return !m_ensembleStatisticsCache.contains( adr ); } );
        if ( isCalculationRequired )
        {
            progressInfo = std::make_unique<caf::ProgressInfo>( summaryAddresses.size(), "Calculating Ensemble Statistics" );
        }
    }

    // Time steps, values and unit name for one address. Ensemble statistics are calculated from all realizations.
    const auto readTimeStepsAndValues =
        [&]( const RifEclipseSummaryAddress& adr ) -> std::tuple<std::vector<time_t>, std::vector<double>, std::string>
    {
        if ( summaryReader )
        {
            auto [isOk, values] = summaryReader->values( adr );
            return { summaryReader->timeSteps( adr ), values, summaryReader->unitName( adr ) };
        }

        // Ensemble statistics for all statistic types are calculated once per address, and reused when only the selected
        // statistic, the value filter or other display settings change
        const auto& statisticsData = ensembleStatisticsData( adr );
        const auto  valuesIt       = statisticsData.values.find( m_ensembleStatistic() );
        if ( valuesIt == statisticsData.values.end() ) return {};

        return { statisticsData.timeSteps, valuesIt->second, statisticsData.unitName };
    };

    for ( const auto& adr : summaryAddresses )
    {
        const auto [timeSteps, values, unitName] = readTimeStepsAndValues( adr );
        if ( progressInfo ) progressInfo->incrementProgress();

        const QString vectorName   = QString::fromStdString( adr.vectorName() );
        const QString categoryName = getCategoryNameFromAddress( adr );

        if ( timeSteps.size() != values.size() ) continue;

        // Get resampled time steps and values
        auto [resampledTimeSteps, resampledValues] =
            RiaSummaryTools::resampledValuesForPeriod( adr, timeSteps, values, m_resamplingSelection() );

        if ( resampledValues.empty() ) continue;

        if ( m_filterTimeSteps() )
        {
            auto filterTimeStepsAndValues = []( auto& resampledTimeSteps, auto& resampledValues, time_t startTime, time_t endTime )
            {
                auto lowerBound = std::lower_bound( resampledTimeSteps.begin(), resampledTimeSteps.end(), startTime );
                if ( lowerBound != resampledTimeSteps.end() )
                {
                    const auto baseIndex = lowerBound - resampledTimeSteps.begin();

                    // Use one time step less to make sure the start date is included
                    const auto startIndex = ( baseIndex > 1 ) ? baseIndex - 1 : 0;

                    resampledTimeSteps.erase( resampledTimeSteps.begin(), resampledTimeSteps.begin() + startIndex );
                    resampledValues.erase( resampledValues.begin(), resampledValues.begin() + startIndex );
                }

                const auto upperBound = std::upper_bound( resampledTimeSteps.begin(), resampledTimeSteps.end(), endTime );
                if ( upperBound != resampledTimeSteps.end() )
                {
                    const auto endIndex = ( upperBound - resampledTimeSteps.begin() ) + 1;

                    if ( endIndex < (int)resampledTimeSteps.size() )
                    {
                        resampledTimeSteps.erase( resampledTimeSteps.begin() + endIndex, resampledTimeSteps.end() );
                        resampledValues.erase( resampledValues.begin() + endIndex, resampledValues.end() );
                    }
                }
            };

            auto startTime = RiaTimeTTools::fromQDateTime( m_startDate );
            auto endTime   = std::max( startTime, RiaTimeTTools::fromQDateTime( m_endDate ) );

            filterTimeStepsAndValues( resampledTimeSteps, resampledValues, startTime, endTime );

            if ( resampledValues.empty() ) continue;
        }

        // The value range of all rows is shown in the property editor when a value filter is active
        const auto maxRowValue = *std::max_element( resampledValues.begin(), resampledValues.end() );
        const auto minRowValue = *std::min_element( resampledValues.begin(), resampledValues.end() );
        if ( m_tableData.unfilteredMinMax )
        {
            auto& [unfilteredMin, unfilteredMax] = *m_tableData.unfilteredMinMax;
            unfilteredMin                        = std::min( unfilteredMin, minRowValue );
            unfilteredMax                        = std::max( unfilteredMax, maxRowValue );
        }
        else
        {
            m_tableData.unfilteredMinMax = std::make_pair( minRowValue, maxRowValue );
        }

        // Vectors are kept if they have values matching the value filter
        const bool hasValueMatchingFilter = RimSummaryTableTools::hasValueMatchingFilter( resampledValues, valueFilter );
        if ( useThresholdFilter && !hasValueMatchingFilter ) continue;

        // Find first and last time step with value above 0.0 (or matching the value filter when filtering) when
        // hasValueMatchingFilter flag is true (first and last should be valid/invalid simultaneously)
        const auto isSignificant    = [&]( double value ) { return RimSummaryTableTools::isValueMatchingFilter( value, valueFilter ); };
        const auto firstTimeStepItr = std::find_if( resampledValues.begin(), resampledValues.end(), isSignificant );
        const auto lastTimeStepItr  = std::find_if( resampledValues.rbegin(), resampledValues.rend(), isSignificant );
        const auto firstIdx         = static_cast<size_t>( std::distance( resampledValues.begin(), firstTimeStepItr ) );
        const auto lastIdx = resampledValues.size() - static_cast<size_t>( std::distance( resampledValues.rbegin(), lastTimeStepItr ) ) - 1;

        const auto firstTimeStep = hasValueMatchingFilter ? resampledTimeSteps[firstIdx] : invalidTimeStep;
        const auto lastTimeStep  = hasValueMatchingFilter ? resampledTimeSteps[lastIdx] : invalidTimeStep;

        // Add to collection of VectorData for table data
        VectorData vectorData{ .category      = categoryName,
                               .name          = vectorName,
                               .values        = resampledValues,
                               .firstTimeStep = firstTimeStep,
                               .lastTimeStep  = lastTimeStep,
                               .timeSteps     = resampledTimeSteps };
        m_tableData.vectorDataCollection.push_back( vectorData );

        // Update min/max values
        m_tableData.maxValue = std::max( m_tableData.maxValue, maxRowValue );
        m_tableData.minValue = std::min( m_tableData.minValue, minRowValue );

        // Build union of resampled time steps
        m_tableData.timeStepsUnion.insert( resampledTimeSteps.begin(), resampledTimeSteps.end() );

        // Set unit name
        if ( m_tableData.unitName.isEmpty() )
        {
            m_tableData.unitName = QString::fromStdString( unitName );
        }
    }

    // Sort vector data on date
    RimSummaryTableTools::sortVectorDataOnDate( m_tableData );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryTable::setExcludedRowsUiSelectionsFromTableData()
{
    const auto initialSelections = std::set<QString>( m_excludedRowsUiField().begin(), m_excludedRowsUiField().end() );

    std::vector<QString> newSelections;
    const auto           categoryNames = RimSummaryTableTools::categoryNames( m_tableData.vectorDataCollection );
    for ( const auto& categoryName : categoryNames )
    {
        if ( initialSelections.contains( categoryName ) )
        {
            newSelections.push_back( categoryName );
        }
    }
    m_excludedRowsUiField.setValueWithFieldChanged( newSelections );
}
