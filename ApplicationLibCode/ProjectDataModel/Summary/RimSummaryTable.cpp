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
#include "RiaSummaryTools.h"
#include "RiaTimeHistoryCurveResampler.h"

#include "RifSummaryReaderInterface.h"

#include "RimEclipseResultCase.h"
#include "RimEclipseView.h"
#include "RimRegularLegendConfig.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimTools.h"

#include "RiuMatrixPlotWidget.h"

#include "cafPdmUiComboBoxEditor.h"
#include "cafPdmUiPushButtonEditor.h"
#include "cafPdmUiToolButtonEditor.h"
#include "cafPdmUiTreeSelectionEditor.h"

#include "cvfScalarMapper.h"

CAF_PDM_SOURCE_INIT( RimSummaryTable, "RimSummaryTable" );

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
    CAF_PDM_InitFieldNoDefault( &m_vector, "Vectors", "Vector" );
    m_vector.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );
    CAF_PDM_InitFieldNoDefault( &m_categories, "Categories", "Category" );
    m_categories.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );
    m_categories = RifEclipseSummaryAddress::SummaryVarCategory::SUMMARY_WELL;

    CAF_PDM_InitFieldNoDefault( &m_resamplingSelection, "ResamplingSelection", "Date Resampling" );
    m_resamplingSelection.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );
    m_resamplingSelection = RiaDefines::DateTimePeriod::YEAR;

    CAF_PDM_InitField( &m_thresholdValue, "ThresholdValue", 0.0, "Threshold" );

    // Table settings
    CAF_PDM_InitField( &m_showValueLabels, "ShowValueLabels", false, "Show Value Labels" );

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
    m_categories            = RifEclipseSummaryAddress::SUMMARY_WELL;
    m_vector                = "";
    m_tableName             = createTableName();
    if ( summaryCases.empty() ) return;

    m_case = summaryCases.front();

    const auto summaryReader = m_case->summaryReader();
    if ( !summaryReader ) return;

    const auto categoryVectors = getCategoryVectorsFromSummaryReader( summaryReader, m_categories() );
    if ( !categoryVectors.empty() )
    {
        m_vector = *categoryVectors.begin();
    }
    m_tableName = createTableName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryTable::setFromCaseAndCategoryAndVectorName( RimSummaryCase*                              summaryCase,
                                                           RifEclipseSummaryAddress::SummaryVarCategory category,
                                                           const QString&                               vectorName )
{
    m_case       = summaryCase;
    m_categories = category;
    m_vector     = vectorName;
    m_tableName  = createTableName();
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
            auto*      summaryReader   = m_case->summaryReader();
            const auto categoryVectors = getCategoryVectorsFromSummaryReader( summaryReader, m_categories() );
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
    else if ( changedField == &m_categories )
    {
        if ( m_case )
        {
            auto*      summaryReader   = m_case->summaryReader();
            const auto categoryVectors = getCategoryVectorsFromSummaryReader( summaryReader, m_categories() );
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
    dataGroup.add( &m_categories );
    dataGroup.add( &m_vector );
    dataGroup.add( &m_resamplingSelection );
    dataGroup.add( &m_thresholdValue );

    caf::PdmUiGroup* tableSettingsGroup = uiOrdering.addNewGroup( "Table Settings" );
    tableSettingsGroup->add( &m_showValueLabels );
    m_legendConfig->uiOrdering( "FlagColorsAndMappingModeOnly", *tableSettingsGroup );

    caf::PdmUiGroup* fontGroup = uiOrdering.addNewGroup( "Fonts" );
    fontGroup->setCollapsedByDefault();
    RimPlotWindow::uiOrderingForFonts( uiConfigName, *fontGroup );
    fontGroup->add( &m_axisTitleFontSize );
    fontGroup->add( &m_axisLabelFontSize );
    fontGroup->add( &m_valueLabelFontSize );

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
        std::vector<RimSummaryCase*> summaryCases = getToplevelSummaryCases();
        for ( auto* summaryCase : summaryCases )
        {
            options.push_back( caf::PdmOptionItemInfo( summaryCase->displayCaseName(), summaryCase, false, summaryCase->uiIconProvider() ) );
        }
    }
    else if ( fieldNeedingOptions == &m_categories )
    {
        options.push_back( caf::PdmOptionItemInfo( caf::AppEnum<RifEclipseSummaryAddress::SummaryVarCategory>::uiText(
                                                       RifEclipseSummaryAddress::SummaryVarCategory::SUMMARY_WELL ),
                                                   RifEclipseSummaryAddress::SummaryVarCategory::SUMMARY_WELL ) );
        options.push_back( caf::PdmOptionItemInfo( caf::AppEnum<RifEclipseSummaryAddress::SummaryVarCategory>::uiText(
                                                       RifEclipseSummaryAddress::SummaryVarCategory::SUMMARY_GROUP ),
                                                   RifEclipseSummaryAddress::SummaryVarCategory::SUMMARY_GROUP ) );
        options.push_back( caf::PdmOptionItemInfo( caf::AppEnum<RifEclipseSummaryAddress::SummaryVarCategory>::uiText(
                                                       RifEclipseSummaryAddress::SummaryVarCategory::SUMMARY_REGION ),
                                                   RifEclipseSummaryAddress::SummaryVarCategory::SUMMARY_REGION ) );
    }
    else if ( fieldNeedingOptions == &m_vector && m_case )
    {
        auto* summaryReader = m_case->summaryReader();
        if ( summaryReader )
        {
            const auto categoryVectorsUnion = getCategoryVectorsFromSummaryReader( summaryReader, m_categories() );
            for ( const auto& vectorName : categoryVectorsUnion )
            {
                options.push_back( caf::PdmOptionItemInfo( vectorName, vectorName ) );
            }
        }
    }
    else if ( fieldNeedingOptions == &m_axisTitleFontSize || fieldNeedingOptions == &m_axisLabelFontSize ||
              fieldNeedingOptions == &m_valueLabelFontSize )
    {
        options = caf::FontTools::relativeSizeValueOptions( RiaPreferences::current()->defaultPlotFontSize() );
    }
    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryTable::onLoadDataAndUpdate()
{
    updateMdiWindowVisibility();

    if ( m_matrixPlotWidget == nullptr || m_case == nullptr )
    {
        return;
    }

    const auto summaryReader = m_case->summaryReader();
    if ( !summaryReader )
    {
        return;
    }

    // Struct for storing vector data
    struct VectorData
    {
        QString             category;
        QString             name;
        std::vector<double> values;
        time_t              firstTimeStep;
        time_t              lastTimeStep;
        bool                hasValueAboveThreshold;
    };

    // Create time step value for vectors with no values above threshold
    const time_t invalidTimeStep = 0;

    // Get all summary addresses for selected category (group, region, well)
    std::vector<VectorData> vectorDataCollection;
    std::set<time_t>        timeStepsUnion;
    const auto              summaryAddresses = getSummaryAddressesFromReader( summaryReader, m_categories(), m_vector() );
    QString                 unitName;
    for ( const auto& adr : summaryAddresses )
    {
        std::vector<double> values;
        summaryReader->values( adr, &values );
        const std::vector<time_t> timeSteps    = summaryReader->timeSteps( adr );
        const QString             vectorName   = QString::fromStdString( adr.vectorName() );
        const QString             categoryName = getCategoryNameFromAddress( adr );
        unitName                               = QString::fromStdString( summaryReader->unitName( adr ) );

        // Get re-sampled time steps and values
        const auto [resampledTimeSteps, resampledValues] =
            RiaSummaryTools::resampledValuesForPeriod( adr, timeSteps, values, m_resamplingSelection() );

        // Detect if values contain at least one value above threshold
        const auto valueAboveThresholdItr =
            std::find_if( resampledValues.begin(), resampledValues.end(), [&]( double value ) { return value > m_thresholdValue; } );
        const bool hasValueAboveThreshold = valueAboveThresholdItr != resampledValues.end();

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

        // Add to vector of VectorData
        VectorData vectorData{ .category               = categoryName,
                               .name                   = vectorName,
                               .values                 = resampledValues,
                               .firstTimeStep          = firstTimeStep,
                               .lastTimeStep           = lastTimeStep,
                               .hasValueAboveThreshold = hasValueAboveThreshold };
        vectorDataCollection.push_back( vectorData );

        // Build union of resampled time steps
        timeStepsUnion.insert( resampledTimeSteps.begin(), resampledTimeSteps.end() );
    }

    // Sort vector data on date (vectors with no values above threshold placed last):
    std::sort( vectorDataCollection.begin(),
               vectorDataCollection.end(),
               []( const VectorData& v1, const VectorData& v2 )
               {
                   if ( !v1.hasValueAboveThreshold ) return false;
                   if ( v1.hasValueAboveThreshold && !v2.hasValueAboveThreshold ) return true;
                   if ( v1.firstTimeStep < v2.firstTimeStep ) return true;
                   if ( v1.firstTimeStep == v2.firstTimeStep && v1.lastTimeStep < v2.lastTimeStep ) return true;
                   return false;
               } );

    // Convert to strings
    std::vector<QString> timeStepStrings;
    for ( const auto& timeStep : timeStepsUnion )
    {
        timeStepStrings.push_back( RiaQDateTimeTools::fromTime_t( timeStep ).toString( dateFormatString() ) );
    }

    // Clear matrix plot
    m_matrixPlotWidget->clearPlotData();
    m_matrixPlotWidget->setColumnHeaders( timeStepStrings );
    double maxValue = 0.0;
    double minValue = 0.0;
    for ( const auto& vectorData : vectorDataCollection )
    {
        const auto maxRowValue = *std::max_element( vectorData.values.begin(), vectorData.values.end() );
        const auto minRowValue = *std::min_element( vectorData.values.begin(), vectorData.values.end() );
        if ( maxRowValue < m_thresholdValue() ) continue;
        maxValue = std::max( maxValue, maxRowValue );
        minValue = std::min( minValue, minRowValue );
        m_matrixPlotWidget->setRowValues( vectorData.category, vectorData.values );
    }

    if ( m_legendConfig )
    {
        m_legendConfig->setAutomaticRanges( minValue, maxValue, 0.0, 0.0 );
    }

    // Set titles and font sizes
    const QString title =
        QString( "Summary Table - %1 [%2]<br>Date Resampling: %3</br>" ).arg( m_vector() ).arg( unitName ).arg( m_resamplingSelection().uiText() );
    m_matrixPlotWidget->setPlotTitle( title );
    m_matrixPlotWidget->setRowTitle( QString( "%1s" ).arg( m_categories().uiText() ) );
    m_matrixPlotWidget->setColumnTitle( "Time steps" );
    m_matrixPlotWidget->setPlotTitleFontSize( titleFontSize() );
    m_matrixPlotWidget->setLegendFontSize( legendFontSize() );
    m_matrixPlotWidget->setAxisTitleFontSize( axisTitleFontSize() );
    m_matrixPlotWidget->setAxisLabelFontSize( axisLabelFontSize() );

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
                                                                                   RifEclipseSummaryAddress::SummaryVarCategory category,
                                                                                   const QString& vector ) const
{
    if ( !summaryReader ) return std::set<RifEclipseSummaryAddress>();

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
std::set<QString> RimSummaryTable::getCategoryVectorsFromSummaryReader( const RifSummaryReaderInterface*             summaryReader,
                                                                        RifEclipseSummaryAddress::SummaryVarCategory category ) const
{
    if ( !summaryReader ) return std::set<QString>();
    if ( category != RifEclipseSummaryAddress::SummaryVarCategory::SUMMARY_WELL &&
         category != RifEclipseSummaryAddress::SummaryVarCategory::SUMMARY_GROUP &&
         category != RifEclipseSummaryAddress::SummaryVarCategory::SUMMARY_REGION )
    {
        return std::set<QString>();
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
    if ( address.category() == RifEclipseSummaryAddress::SummaryVarCategory::SUMMARY_WELL )
    {
        return QString::fromStdString( address.wellName() );
    }
    else if ( address.category() == RifEclipseSummaryAddress::SummaryVarCategory::SUMMARY_GROUP )
    {
        return QString::fromStdString( address.groupName() );
    }
    else if ( address.category() == RifEclipseSummaryAddress::SummaryVarCategory::SUMMARY_REGION )
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
