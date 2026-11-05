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

#include "cafPdmUiCheckBoxAndTextEditor.h"
#include "cafPdmUiComboBoxEditor.h"
#include "cafPdmUiDateEditor.h"
#include "cafPdmUiToolButtonEditor.h"
#include "cafPdmUiTreeSelectionEditor.h"

#include <cmath>
#include <limits>
#include <tuple>

CAF_PDM_SOURCE_INIT( RimSummaryTable, "RimSummaryTable" );

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
void AppEnum<RimSummaryTable::EnsembleStatistic>::setUp()
{
    addItem( RimSummaryTable::EnsembleStatistic::MIN, "MIN", "Min" );
    addItem( RimSummaryTable::EnsembleStatistic::P90, "P90", "P90" );
    addItem( RimSummaryTable::EnsembleStatistic::P50, "P50", "P50" );
    addItem( RimSummaryTable::EnsembleStatistic::MEAN, "MEAN", "Mean" );
    addItem( RimSummaryTable::EnsembleStatistic::P10, "P10", "P10" );
    addItem( RimSummaryTable::EnsembleStatistic::MAX, "MAX", "Max" );
    setDefault( RimSummaryTable::EnsembleStatistic::MEAN );
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

    CAF_PDM_InitField( &m_upperThreshold, "UpperThreshold", std::make_pair( false, 0.0 ), "Upper Threshold" );
    m_upperThreshold.uiCapability()->setUiEditorTypeName( caf::PdmUiCheckBoxAndTextEditor::uiEditorTypeName() );
    m_upperThreshold.uiCapability()->setUiToolTip(
        "Keep rows with at least one value greater than or equal to this threshold. When both thresholds are enabled, rows are kept "
        "if they have values beyond any of the thresholds. Use for example -50 and 50 to remove rows with all values between -50 "
        "and 50." );
    CAF_PDM_InitField( &m_lowerThreshold, "LowerThreshold", std::make_pair( false, 0.0 ), "Lower Threshold" );
    m_lowerThreshold.uiCapability()->setUiEditorTypeName( caf::PdmUiCheckBoxAndTextEditor::uiEditorTypeName() );
    m_lowerThreshold.uiCapability()->setUiToolTip(
        "Keep rows with at least one value less than or equal to this threshold. When both thresholds are enabled, rows are kept "
        "if they have values beyond any of the thresholds. Use for example -50 and 50 to remove rows with all values between -50 "
        "and 50." );

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

    // The threshold was always applied in older project files. A threshold of 0.0 (the old default) only excluded vectors
    // without any non-negative values, which corresponds to having no threshold enabled. Enabling an upper threshold of
    // 0.0 would also mark zero values as significant, so only non-zero thresholds are migrated.
    if ( !std::isnan( m_thresholdValue_OBSOLETE() ) )
    {
        if ( m_thresholdValue_OBSOLETE() != 0.0 )
        {
            m_upperThreshold = std::make_pair( true, m_thresholdValue_OBSOLETE() );
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
        onLoadDataAndUpdate();
    }
    else if ( changedField == &m_category )
    {
        resetVectorToFirstAvailable();
        if ( m_isAutomaticName )
        {
            m_tableName = createTableName();
        }
        onLoadDataAndUpdate();
    }
    else if ( changedField == &m_vector || changedField == &m_resamplingSelection || changedField == &m_upperThreshold ||
              changedField == &m_lowerThreshold || changedField == &m_ensembleStatistic )
    {
        if ( m_isAutomaticName )
        {
            m_tableName = createTableName();
        }
        onLoadDataAndUpdate();
    }
    else if ( changedField == &m_excludedRowsUiField )
    {
        onLoadDataAndUpdate();
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
        onLoadDataAndUpdate();
    }
    else if ( changedField == &m_mappingType && m_legendConfig )
    {
        m_legendConfig->setMappingMode( m_mappingType() );
        if ( m_rangeType == RangeType::AUTOMATIC )
        {
            m_legendConfig->updateTickCountAndUserDefinedRange();
        }
        onLoadDataAndUpdate();
    }
    else if ( changedField == &m_maxTimeLabelCount || changedField == &m_startDate || changedField == &m_endDate ||
              changedField == &m_filterTimeSteps )
    {
        onLoadDataAndUpdate();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryTable::childFieldChangedByUi( const caf::PdmFieldHandle* changedChildField )
{
    onLoadDataAndUpdate();
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
    valueFilterGroup->add( &m_upperThreshold );
    valueFilterGroup->add( &m_lowerThreshold );

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
    updateDockWindowVisibility();
    createTableData();
    setExcludedRowsUiSelectionsFromTableData();

    if ( m_matrixPlotWidget == nullptr || !hasDataSource() )
    {
        return;
    }

    // Exclude rows that are selected
    const auto excludedRows = std::set<QString>( m_excludedRowsUiField().begin(), m_excludedRowsUiField().end() );

    // Convert to strings
    std::vector<QString> timeStepStrings;
    for ( const auto& timeStep : m_tableData.timeStepsUnion )
    {
        timeStepStrings.push_back( RiaQDateTimeTools::fromTime_t( timeStep ).toString( dateFormatString() ) );
    }

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

        m_matrixPlotWidget->setRowValues( vectorData.category, vectorData.values );
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
const RimSummaryTable::EnsembleStatisticsData& RimSummaryTable::ensembleStatisticsData( const RifEclipseSummaryAddress& address )
{
    // The cache is invalid if the ensemble or the number of realizations has changed
    const size_t caseCount = m_ensemble ? m_ensemble->allSummaryCases().size() : 0;
    if ( m_ensembleStatisticsCacheEnsemble != m_ensemble() || m_ensembleStatisticsCacheCaseCount != caseCount )
    {
        clearEnsembleStatisticsCache();
        m_ensembleStatisticsCacheEnsemble  = m_ensemble();
        m_ensembleStatisticsCacheCaseCount = caseCount;
    }

    if ( auto it = m_ensembleStatisticsCache.find( address ); it != m_ensembleStatisticsCache.end() ) return it->second;

    EnsembleStatisticsData data;
    if ( m_ensemble )
    {
        m_ensembleStatisticsCase->calculate( m_ensemble->allSummaryCases(), address, false, { 0, 100 } );

        // Percentiles use the switched (oil industry) convention: percentile 0 is the maximum, percentile 100 is the minimum
        const auto statisticsAddress = [&address]( EnsembleStatistic statistic )
        {
            auto adr = address;
            switch ( statistic )
            {
                case EnsembleStatistic::MIN:
                    adr.setStatisticsType( RifEclipseSummaryAddressDefines::StatisticsType::CUSTOM );
                    adr.setPercentile( 100 );
                    break;
                case EnsembleStatistic::MAX:
                    adr.setStatisticsType( RifEclipseSummaryAddressDefines::StatisticsType::CUSTOM );
                    adr.setPercentile( 0 );
                    break;
                case EnsembleStatistic::P10:
                    adr.setStatisticsType( RifEclipseSummaryAddressDefines::StatisticsType::P10 );
                    break;
                case EnsembleStatistic::P50:
                    adr.setStatisticsType( RifEclipseSummaryAddressDefines::StatisticsType::P50 );
                    break;
                case EnsembleStatistic::P90:
                    adr.setStatisticsType( RifEclipseSummaryAddressDefines::StatisticsType::P90 );
                    break;
                case EnsembleStatistic::MEAN:
                    adr.setStatisticsType( RifEclipseSummaryAddressDefines::StatisticsType::MEAN );
                    break;
            }
            return adr;
        };

        data.timeSteps = m_ensembleStatisticsCase->timeSteps( address );

        // The unit is read from the first realization, which only knows the plain (non-statistics) address
        data.unitName = m_ensembleStatisticsCase->unitName( address );

        for ( auto statistic : { EnsembleStatistic::MIN,
                                 EnsembleStatistic::P90,
                                 EnsembleStatistic::P50,
                                 EnsembleStatistic::MEAN,
                                 EnsembleStatistic::P10,
                                 EnsembleStatistic::MAX } )
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
    m_ensembleStatisticsCacheEnsemble  = nullptr;
    m_ensembleStatisticsCacheCaseCount = 0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryTable::createTableData()
{
    // Without thresholds, all rows are kept, and values above zero are used to find the first and last time step
    m_tableData                     = TableData();
    m_tableData.upperThresholdValue = m_upperThreshold().first ? std::optional<double>( m_upperThreshold().second ) : std::nullopt;
    m_tableData.lowerThresholdValue = m_lowerThreshold().first ? std::optional<double>( m_lowerThreshold().second ) : std::nullopt;

    const auto upperThreshold     = m_tableData.upperThresholdValue;
    const auto lowerThreshold     = m_tableData.lowerThresholdValue;
    const bool useThresholdFilter = upperThreshold || lowerThreshold;

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

    // Create time step value for vectors with no values above threshold
    const time_t invalidTimeStep = 0;

    const auto summaryAddresses = getSummaryAddressesFromAddresses( availableAddresses(), m_category(), m_vector() );

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
        // statistic, the thresholds or other display settings change
        const auto& statisticsData = ensembleStatisticsData( adr );
        const auto  valuesIt       = statisticsData.values.find( m_ensembleStatistic() );
        if ( valuesIt == statisticsData.values.end() ) return {};

        return { statisticsData.timeSteps, valuesIt->second, statisticsData.unitName };
    };

    for ( const auto& adr : summaryAddresses )
    {
        const auto [timeSteps, values, unitName] = readTimeStepsAndValues( adr );
        const QString vectorName                 = QString::fromStdString( adr.vectorName() );
        const QString categoryName               = getCategoryNameFromAddress( adr );

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

        // Vectors are kept if they have values equal to or above the upper threshold, or equal to or below the lower
        // threshold. This matches RimSummaryTableTools::hasValueAboveThreshold.
        const auto maxRowValue  = *std::max_element( resampledValues.begin(), resampledValues.end() );
        const auto minRowValue  = *std::min_element( resampledValues.begin(), resampledValues.end() );
        const bool isAboveUpper = upperThreshold && maxRowValue >= *upperThreshold;
        const bool isBelowLower = lowerThreshold && minRowValue <= *lowerThreshold;
        if ( useThresholdFilter && !isAboveUpper && !isBelowLower ) continue;

        // Detect if values contain at least one value beyond the threshold(s)
        const bool hasValueAboveThreshold = RimSummaryTableTools::hasValueAboveThreshold( resampledValues, upperThreshold, lowerThreshold );

        // Find first and last time step with value above 0.0 (or beyond the thresholds when filtering) when
        // hasValueAboveThreshold flag is true (first and last should be valid/invalid simultaneously)
        const auto isSignificant = [&]( double value )
        { return RimSummaryTableTools::hasValueAboveThreshold( { value }, upperThreshold, lowerThreshold ); };
        const auto firstTimeStepItr = std::find_if( resampledValues.begin(), resampledValues.end(), isSignificant );
        const auto lastTimeStepItr  = std::find_if( resampledValues.rbegin(), resampledValues.rend(), isSignificant );
        const auto firstIdx         = static_cast<size_t>( std::distance( resampledValues.begin(), firstTimeStepItr ) );
        const auto lastIdx = resampledValues.size() - static_cast<size_t>( std::distance( resampledValues.rbegin(), lastTimeStepItr ) ) - 1;

        const auto firstTimeStep = hasValueAboveThreshold ? resampledTimeSteps[firstIdx] : invalidTimeStep;
        const auto lastTimeStep  = hasValueAboveThreshold ? resampledTimeSteps[lastIdx] : invalidTimeStep;

        // Add to collection of VectorData for table data
        VectorData vectorData{ .category      = categoryName,
                               .name          = vectorName,
                               .values        = resampledValues,
                               .firstTimeStep = firstTimeStep,
                               .lastTimeStep  = lastTimeStep };
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
