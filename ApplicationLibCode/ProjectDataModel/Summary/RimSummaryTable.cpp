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
#include "RimSummaryCase.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimTools.h"

#include "RiuMatrixPlotWidget.h"

#include "cafPdmUiComboBoxEditor.h"
#include "cafPdmUiDateEditor.h"
#include "cafPdmUiPushButtonEditor.h"
#include "cafPdmUiToolButtonEditor.h"
#include "cafPdmUiTreeSelectionEditor.h"

#include "cvfScalarMapper.h"

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

    CAF_PDM_InitFieldNoDefault( &m_case, "SummaryCase", "Case" );
    m_case.uiCapability()->setUiTreeChildrenHidden( true );
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

    CAF_PDM_InitField( &m_thresholdValue, "ThresholdValue", 0.0, "Threshold" );

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

    setLegendsVisible( true );
    setAsPlotMdiWindow();
    setShowWindow( true );
    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryTable::~RimSummaryTable()
{
    if ( isMdiWindow() ) removeMdiWindowFromMdiArea();

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

    const auto summaryReader = summaryReaderWithAddresses();
    if ( !summaryReader ) return;

    const auto categoryVectors = getCategoryVectorFromSummaryReader( summaryReader, m_category() );
    if ( !categoryVectors.empty() )
    {
        m_vector = *categoryVectors.begin();
    }
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
void RimSummaryTable::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    RimPlotWindow::fieldChangedByUi( changedField, oldValue, newValue );

    if ( changedField == &m_case )
    {
        if ( m_case )
        {
            const auto summaryReader   = summaryReaderWithAddresses();
            const auto categoryVectors = getCategoryVectorFromSummaryReader( summaryReader, m_category() );
            if ( summaryReader && !categoryVectors.empty() )
            {
                m_vector = *categoryVectors.begin();
            }
            else
            {
                m_vector = QString( "" );
            }
        }
        if ( m_isAutomaticName )
        {
            m_tableName = createTableName();
        }
        onLoadDataAndUpdate();
    }
    else if ( changedField == &m_category )
    {
        if ( m_case )
        {
            const auto summaryReader   = summaryReaderWithAddresses();
            const auto categoryVectors = getCategoryVectorFromSummaryReader( summaryReader, m_category() );
            if ( summaryReader && !categoryVectors.empty() )
            {
                m_vector = *categoryVectors.begin();
            }
            else
            {
                m_vector = QString( "" );
            }
        }
        else
        {
            m_vector = QString( " " );
        }
        if ( m_isAutomaticName )
        {
            m_tableName = createTableName();
        }
        onLoadDataAndUpdate();
    }
    else if ( changedField == &m_vector || changedField == &m_resamplingSelection || changedField == &m_thresholdValue )
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
    dataGroup.add( &m_case );
    dataGroup.add( &m_category );
    dataGroup.add( &m_vector );
    dataGroup.add( &m_resamplingSelection );
    dataGroup.add( &m_thresholdValue );
    dataGroup.add( &m_excludedRowsUiField );

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
void RimSummaryTable::defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_startDate || field == &m_endDate )
    {
        if ( auto myAttr = dynamic_cast<caf::PdmUiDateEditorAttribute*>( attribute ) )
        {
            QString dateFormat = "yyyy-MM-dd";
            if ( m_resamplingSelection() == RiaDefines::DateTimePeriod::DECADE || m_resamplingSelection() == RiaDefines::DateTimePeriod::YEAR )
            {
                dateFormat = "yyyy";
            }
            else if ( m_resamplingSelection() == RiaDefines::DateTimePeriod::MONTH ||
                      m_resamplingSelection() == RiaDefines::DateTimePeriod::QUARTER ||
                      m_resamplingSelection() == RiaDefines::DateTimePeriod::HALFYEAR )
            {
                dateFormat = "yyyy-MM";
            }

            myAttr->dateFormat = dateFormat;
        }
    }
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
    else if ( fieldNeedingOptions == &m_vector && m_case )
    {
        const auto summaryReader = summaryReaderWithAddresses();
        if ( summaryReader )
        {
            const auto categoryVectorsUnion = getCategoryVectorFromSummaryReader( summaryReader, m_category() );
            for ( const auto& vectorName : categoryVectorsUnion )
            {
                options.push_back( caf::PdmOptionItemInfo( vectorName, vectorName ) );
            }
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
    updateMdiWindowVisibility();
    createTableData();
    setExcludedRowsUiSelectionsFromTableData();

    if ( m_matrixPlotWidget == nullptr || m_case == nullptr )
    {
        return;
    }

    const auto summaryReader = summaryReaderWithAddresses();
    if ( !summaryReader )
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
    const QString title = QString( "Summary Table - %1 [%2]<br>Date Resampling: %3</br>" )
                              .arg( m_vector() )
                              .arg( m_tableData.unitName )
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
QImage RimSummaryTable::snapshotWindowContent()
{
    QImage image;

    if ( m_matrixPlotWidget )
    {
        QPixmap pix = m_matrixPlotWidget->grab();
        image       = pix.toImage();
    }

    return image;
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
std::set<RifEclipseSummaryAddress> RimSummaryTable::getSummaryAddressesFromReader( const RifSummaryReaderInterface* summaryReader,
                                                                                   RifEclipseSummaryAddressDefines::SummaryCategory category,
                                                                                   const QString& vector ) const
{
    if ( !summaryReader ) return {};

    std::set<RifEclipseSummaryAddress>       categoryAddresses;
    const std::set<RifEclipseSummaryAddress> allResultAddresses = summaryReader->allResultAddresses();
    const std::string                        selectedVector     = vector.toStdString();
    for ( const auto& resAdr : allResultAddresses )
    {
        if ( resAdr.category() != category || resAdr.vectorName() != selectedVector ) continue;
        categoryAddresses.emplace( resAdr );
    }
    return categoryAddresses;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<QString> RimSummaryTable::getCategoryVectorFromSummaryReader( const RifSummaryReaderInterface*                 summaryReader,
                                                                       RifEclipseSummaryAddressDefines::SummaryCategory category ) const
{
    if ( !summaryReader ) return {};
    if ( category != RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_WELL &&
         category != RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_GROUP &&
         category != RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_REGION )
    {
        return {};
    }

    std::set<QString>                        categoryVectors;
    const std::set<RifEclipseSummaryAddress> allResultAddresses = summaryReader->allResultAddresses();
    for ( const auto& resAdr : allResultAddresses )
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
    auto summaryReader = summaryReaderWithAddresses();
    if ( !summaryReader ) return;

    const std::set<RifEclipseSummaryAddress> allResultAddresses = summaryReader->allResultAddresses();
    if ( allResultAddresses.empty() ) return;

    auto timeSteps = summaryReader->timeSteps( *allResultAddresses.begin() );
    if ( timeSteps.empty() ) return;

    m_startDate = RiaQDateTimeTools::fromTime_t( timeSteps.front() );
    m_endDate   = RiaQDateTimeTools::fromTime_t( timeSteps.back() );
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
void RimSummaryTable::createTableData()
{
    m_tableData                = TableData();
    m_tableData.thresholdValue = m_thresholdValue();

    const auto summaryReader = summaryReaderWithAddresses();
    if ( !summaryReader ) return;

    // Create time step value for vectors with no values above threshold
    const time_t invalidTimeStep = 0;

    // Get all summary addresses for selected category (group, region, well)
    const auto summaryAddresses = getSummaryAddressesFromReader( summaryReader, m_category(), m_vector() );
    QString    unitName;
    for ( const auto& adr : summaryAddresses )
    {
        auto [isOk, values]                    = summaryReader->values( adr );
        const std::vector<time_t> timeSteps    = summaryReader->timeSteps( adr );
        const QString             vectorName   = QString::fromStdString( adr.vectorName() );
        const QString             categoryName = getCategoryNameFromAddress( adr );

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

        // Exclude vectors with values BELOW threshold - to include visualization of values equal to threshold!
        const auto maxRowValue = *std::max_element( resampledValues.begin(), resampledValues.end() );
        const auto minRowValue = *std::min_element( resampledValues.begin(), resampledValues.end() );
        if ( maxRowValue < m_tableData.thresholdValue ) continue;

        // Detect if values contain at least one value ABOVE threshold
        const bool hasValueAboveThreshold = RimSummaryTableTools::hasValueAboveThreshold( resampledValues, m_tableData.thresholdValue );

        // Find first and last time step with value above 0.0 when hasValueAboveThreshold flag is true (first and last should be
        // valid/invalid simultaneously)
        const auto firstTimeStepItr =
            std::find_if( resampledValues.begin(), resampledValues.end(), [&]( double value ) { return value > 0.0; } );
        const auto lastTimeStepItr =
            std::find_if( resampledValues.rbegin(), resampledValues.rend(), [&]( double value ) { return value > 0.0; } );
        const auto firstIdx = static_cast<size_t>( std::distance( resampledValues.begin(), firstTimeStepItr ) );
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
            m_tableData.unitName = QString::fromStdString( summaryReader->unitName( adr ) );
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
