/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020 Equinor ASA
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

#include "RimCorrelationMatrixPlot.h"

#include "RiaColorTools.h"
#include "RiaPreferences.h"
#include "RiaQDateTimeTools.h"
#include "RiaStatisticsTools.h"
#include "RiaSummaryCurveDefinition.h"
#include "RiuPlotMainWindowTools.h"
#include "RiuQwtLinearScaleEngine.h"
#include "RiuQwtPlotTools.h"
#include "RiuQwtPlotWidget.h"

#include "RifSummaryReaderInterface.h"

#include "RigEnsembleParameter.h"

#include "RimDerivedSummaryCase.h"
#include "RimEnsembleCurveSet.h"
#include "RimPlotAxisProperties.h"
#include "RimPlotAxisPropertiesInterface.h"
#include "RimPlotDataFilterCollection.h"
#include "RimRegularLegendConfig.h"
#include "RimSummaryAddress.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseCollection.h"
#include "RimSummaryPlotAxisFormatter.h"

#include "cafPdmUiComboBoxEditor.h"
#include "cafPdmUiTreeOrdering.h"
#include "cafPdmUiTreeSelectionEditor.h"

#include "cvfScalarMapper.h"

#include "qwt_plot_marker.h"
#include "qwt_plot_shapeitem.h"
#include "qwt_scale_draw.h"
#include "qwt_text.h"

#include <algorithm>
#include <limits>
#include <map>
#include <set>

namespace caf
{
template <>
void caf::AppEnum<RimCorrelationMatrixPlot::Sorting>::setUp()
{
    addItem( RimCorrelationMatrixPlot::Sorting::NO_SORTING, "NO_SORTING", "No Sorting" );
    addItem( RimCorrelationMatrixPlot::Sorting::ROWS, "SORT_ROWS", "Sort Rows" );
    addItem( RimCorrelationMatrixPlot::Sorting::COLUMNS, "SORT_COLUMNS", "Sort Columns" );
    addItem( RimCorrelationMatrixPlot::Sorting::BOTH, "SORT_BOTH", "Sort Both Rows/Columns" );
    setDefault( RimCorrelationMatrixPlot::Sorting::BOTH );
}
} // namespace caf

CAF_PDM_SOURCE_INIT( RimCorrelationMatrixPlot, "CorrelationMatrixPlot" );

class CorrelationMatrixShapeItem : public QwtPlotShapeItem
{
public:
    CorrelationMatrixShapeItem( const QString& title = QString() )
        : QwtPlotShapeItem( title )
    {
    }

public:
    QString                   parameter;
    RiaSummaryCurveDefinition curveDef;
};

class TextScaleDraw : public QwtScaleDraw
{
public:
    TextScaleDraw( const std::map<size_t, QString>& tickLabels )
        : m_tickLabels( tickLabels )
    {
    }

    QwtText label( double value ) const override
    {
        size_t intValue = static_cast<size_t>( value + 0.25 );
        auto   it       = m_tickLabels.find( intValue );
        return it != m_tickLabels.end() ? it->second : "";
    }

private:
    std::map<size_t, QString> m_tickLabels;
};

template <typename KeyType, typename ValueType>
class CorrelationMatrixRowOrColumn
{
public:
    CorrelationMatrixRowOrColumn( const KeyType&                key,
                                  const std::vector<double>&    correlations,
                                  const std::vector<ValueType>& values )
        : m_key( key )
        , m_correlations( correlations )
        , m_values( values )
        , m_correlationSum( 0.0 )
        , m_correlationAbsSum( 0.0 )
    {
        bool anyValid = false;
        for ( auto value : correlations )
        {
            if ( RiaCurveDataTools::isValidValue( value, false ) )
            {
                m_correlationSum += value;
                m_correlationAbsSum += std::abs( value );
                anyValid = true;
            }
        }
        if ( !anyValid )
        {
            m_correlationSum    = std::numeric_limits<double>::infinity();
            m_correlationAbsSum = std::numeric_limits<double>::infinity();
        }
    }

    KeyType                m_key;
    std::vector<double>    m_correlations;
    std::vector<ValueType> m_values;
    double                 m_correlationSum;
    double                 m_correlationAbsSum;
};

using CorrelationMatrixColumn = CorrelationMatrixRowOrColumn<QString, RiaSummaryCurveDefinition>;
using CorrelationMatrixRow    = CorrelationMatrixRowOrColumn<RiaSummaryCurveDefinition, QString>;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCorrelationMatrixPlot::RimCorrelationMatrixPlot()
    : RimAbstractCorrelationPlot()
    , matrixCellSelected( this )
{
    CAF_PDM_InitObject( "Correlation Plot", ":/CorrelationMatrixPlot16x16.png", "", "" );

    CAF_PDM_InitField( &m_showAbsoluteValues, "CorrelationAbsValues", false, "Show Absolute Values", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_sortByValues, "CorrelationSorting", "Sort Matrix by Values", "", "", "" );
    CAF_PDM_InitField( &m_sortByAbsoluteValues, "CorrelationAbsSorting", true, "Sort by Absolute Values", "", "", "" );
    CAF_PDM_InitField( &m_excludeParametersWithoutVariation,
                       "ExcludeParamsWithoutVariation",
                       true,
                       "Exclude Parameters Without Variation",
                       "",
                       "",
                       "" );
    CAF_PDM_InitField( &m_showOnlyTopNCorrelations, "ShowOnlyTopNCorrelations", true, "Show Only Top Correlations", "", "", "" );
    CAF_PDM_InitField( &m_topNFilterCount, "TopNFilterCount", 20, "Number rows/columns", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_legendConfig, "LegendConfig", "", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_selectedParametersList, "SelectedParameters", "Select Parameters", "", "", "" );
    m_selectedParametersList.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::TOP );
    m_selectedParametersList.uiCapability()->setUiEditorTypeName( caf::PdmUiTreeSelectionEditor::uiEditorTypeName() );

    m_legendConfig = new RimRegularLegendConfig();
    m_legendConfig->setAutomaticRanges( -1.0, 1.0, -1.0, 1.0 );
    m_legendConfig->setColorLegend(
        RimRegularLegendConfig::mapToColorLegend( RimRegularLegendConfig::ColorRangesType::BLUE_WHITE_RED ) );

    setLegendsVisible( false );

    this->uiCapability()->setUiTreeChildrenHidden( true );
    m_selectMultipleVectors = true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCorrelationMatrixPlot::~RimCorrelationMatrixPlot()
{
    if ( isMdiWindow() ) removeMdiWindowFromMdiArea();

    cleanupBeforeClose();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimCorrelationMatrixPlot::showAbsoluteValues() const
{
    return m_showAbsoluteValues;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimCorrelationMatrixPlot::sortByAbsoluteValues() const
{
    return m_sortByAbsoluteValues;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimRegularLegendConfig* RimCorrelationMatrixPlot::legendConfig()
{
    return m_legendConfig();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCorrelationMatrixPlot::selectAllParameters()
{
    m_selectedParametersList.v().clear();
    std::set<RigEnsembleParameter> params = variationSortedEnsembleParameters();
    for ( auto param : params )
    {
        if ( !m_excludeParametersWithoutVariation() || param.variationBin > RigEnsembleParameter::NO_VARIATION )
        {
            m_selectedParametersList.v().push_back( param.name );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimCorrelationMatrixPlot::showTopNCorrelations() const
{
    return m_showOnlyTopNCorrelations();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimCorrelationMatrixPlot::topNFilterCount() const
{
    return m_topNFilterCount();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCorrelationMatrixPlot::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                                 const QVariant&            oldValue,
                                                 const QVariant&            newValue )
{
    RimAbstractCorrelationPlot::fieldChangedByUi( changedField, oldValue, newValue );

    bool sendSelectedSignal = false;

    if ( changedField == &m_showAbsoluteValues || changedField == &m_sortByValues ||
         changedField == &m_sortByAbsoluteValues || changedField == &m_showOnlyTopNCorrelations ||
         changedField == &m_topNFilterCount || changedField == &m_excludeParametersWithoutVariation ||
         changedField == &m_selectedParametersList )
    {
        if ( changedField == &m_excludeParametersWithoutVariation )
        {
            selectAllParameters();
        }
        updateLegend();
        loadDataAndUpdate();
        updateConnectedEditors();
        sendSelectedSignal = true;
    }

    if ( changedField == &m_pushButtonSelectSummaryAddress )
    {
        sendSelectedSignal = true;
    }

    if ( sendSelectedSignal )
    {
        auto curves     = curveDefinitions();
        auto parameters = m_selectedParametersList();
        if ( !curves.empty() && !parameters.empty() )
        {
            auto firstCurve     = curves.front();
            auto firstParameter = parameters.front();

            matrixCellSelected.send( { firstParameter, firstCurve } );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCorrelationMatrixPlot::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    caf::PdmUiGroup* correlationGroup = uiOrdering.addNewGroup( "Correlation Settings" );
    correlationGroup->add( &m_excludeParametersWithoutVariation );
    correlationGroup->add( &m_selectedParametersList );
    correlationGroup->add( &m_showAbsoluteValues );
    correlationGroup->add( &m_sortByValues );
    if ( !m_showAbsoluteValues() && m_sortByValues() != Sorting::NO_SORTING )
    {
        correlationGroup->add( &m_sortByAbsoluteValues );
    }
    if ( m_sortByValues() != Sorting::NO_SORTING )
    {
        correlationGroup->add( &m_showOnlyTopNCorrelations );
        if ( m_showOnlyTopNCorrelations() )
        {
            correlationGroup->add( &m_topNFilterCount );
        }
    }

    appendDataSourceFields( uiConfigName, uiOrdering );

    if ( uiConfigName != "report" )
    {
        caf::PdmUiGroup* plotGroup = uiOrdering.addNewGroup( "Plot Settings" );
        plotGroup->add( &m_showPlotTitle );
        plotGroup->add( &m_useAutoPlotTitle );
        plotGroup->add( &m_description );
        m_description.uiCapability()->setUiReadOnly( m_useAutoPlotTitle() );
        RimPlot::defineUiOrdering( uiConfigName, *plotGroup );

        plotGroup->add( &m_titleFontSize );
        plotGroup->add( &m_labelFontSize );
        plotGroup->add( &m_axisTitleFontSize );
        plotGroup->add( &m_axisValueFontSize );
        m_legendConfig->uiOrdering( "ColorsOnly", *plotGroup );
    }

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo>
    RimCorrelationMatrixPlot::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly )
{
    QList<caf::PdmOptionItemInfo> options =
        RimAbstractCorrelationPlot::calculateValueOptions( fieldNeedingOptions, useOptionsOnly );

    if ( fieldNeedingOptions == &m_selectedParametersList )
    {
        std::set<RigEnsembleParameter> params = variationSortedEnsembleParameters();
        for ( auto param : params )
        {
            if ( !m_excludeParametersWithoutVariation() || param.variationBin > RigEnsembleParameter::NO_VARIATION )
            {
                options.push_back( caf::PdmOptionItemInfo( param.uiName(), param.name ) );
            }
        }
    }
    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCorrelationMatrixPlot::onLoadDataAndUpdate()
{
    updateMdiWindowVisibility();

    m_selectedVarsUiField = selectedQuantitiesText();

    if ( m_plotWidget )
    {
        m_plotWidget->detachItems( QwtPlotItem::Rtti_PlotBarChart );
        m_plotWidget->detachItems( QwtPlotItem::Rtti_PlotScale );
        m_plotWidget->detachItems( QwtPlotItem::Rtti_PlotItem );

        updateLegend();
        createMatrix();

        m_plotWidget->insertLegend( nullptr );

        this->updateAxes();
        this->updatePlotTitle();
        m_plotWidget->scheduleReplot();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCorrelationMatrixPlot::childFieldChangedByUi( const caf::PdmFieldHandle* changedChildField )
{
    this->loadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCorrelationMatrixPlot::updateAxes()
{
    if ( !m_plotWidget ) return;

    m_plotWidget->setAxisScaleDraw( QwtPlot::yLeft, new TextScaleDraw( m_resultLabels ) );
    m_plotWidget->setAxisScaleEngine( QwtPlot::yLeft, new RiuQwtLinearScaleEngine );
    m_plotWidget->setAxisTitleText( QwtPlot::yLeft, "Result Vector" );
    m_plotWidget->setAxisTitleEnabled( QwtPlot::yLeft, true );
    m_plotWidget->setAxisFontsAndAlignment( QwtPlot::yLeft, axisTitleFontSize(), axisValueFontSize(), false, Qt::AlignCenter );
    m_plotWidget->setAxisLabelsAndTicksEnabled( QwtPlot::yLeft, true, false );
    m_plotWidget->setAxisRange( QwtPlot::yLeft, 0.0, (double)m_resultLabels.size() + 1 );
    m_plotWidget->setMajorAndMinorTickIntervalsAndRange( QwtPlot::yLeft,
                                                         1.0,
                                                         0.0,
                                                         0.5,
                                                         (double)m_resultLabels.size() - 0.5,
                                                         0.0,
                                                         m_resultLabels.size() );

    auto scaleDraw = new TextScaleDraw( m_paramLabels );
    scaleDraw->setLabelRotation( 30.0 );
    m_plotWidget->setAxisScaleDraw( QwtPlot::xBottom, scaleDraw );

    m_plotWidget->setAxisScaleEngine( QwtPlot::xBottom, new RiuQwtLinearScaleEngine );
    m_plotWidget->setAxisTitleText( QwtPlot::xBottom, "Ensemble Parameter" );
    m_plotWidget->setAxisTitleEnabled( QwtPlot::xBottom, true );
    m_plotWidget->setAxisFontsAndAlignment( QwtPlot::xBottom,
                                            axisTitleFontSize(),
                                            axisValueFontSize(),
                                            false,
                                            Qt::AlignCenter | Qt::AlignTop );
    m_plotWidget->setAxisLabelsAndTicksEnabled( QwtPlot::xBottom, true, false );
    m_plotWidget->setAxisRange( QwtPlot::xBottom, 0.0, (double)m_paramLabels.size() + 1 );
    m_plotWidget->setMajorAndMinorTickIntervalsAndRange( QwtPlot::xBottom,
                                                         1.0,
                                                         0.0,
                                                         0.5,
                                                         (double)m_paramLabels.size() - 0.5,
                                                         0.0,
                                                         (double)m_paramLabels.size() );

    m_plotWidget->setAxisLabelAlignment( QwtPlot::xBottom, Qt::AlignRight );
}

template <typename KeyType, typename ValueType>
void eraseInvalidEntries( std::vector<CorrelationMatrixRowOrColumn<KeyType, ValueType>>& matrix )
{
    matrix.erase( std::remove_if( matrix.begin(),
                                  matrix.end(),
                                  [=]( const CorrelationMatrixRowOrColumn<KeyType, ValueType>& entry ) {
                                      bool isValid = RiaCurveDataTools::isValidValue( entry.m_correlationSum, false );
                                      return !isValid;
                                  } ),
                  matrix.end() );
}

template <typename KeyType, typename ValueType>
void sortEntries( std::vector<CorrelationMatrixRowOrColumn<KeyType, ValueType>>& matrix, bool sortByAbsoluteValues )
{
    std::sort( matrix.begin(),
               matrix.end(),
               [&sortByAbsoluteValues]( const CorrelationMatrixRowOrColumn<KeyType, ValueType>& lhs,
                                        const CorrelationMatrixRowOrColumn<KeyType, ValueType>& rhs ) -> bool {
                   if ( sortByAbsoluteValues )
                       return lhs.m_correlationAbsSum > rhs.m_correlationAbsSum;
                   else
                       return lhs.m_correlationSum > rhs.m_correlationSum;
               } );
}

template <typename KeyType, typename ValueType>
std::vector<CorrelationMatrixRowOrColumn<ValueType, KeyType>>
    transpose( const std::vector<CorrelationMatrixRowOrColumn<KeyType, ValueType>>& matrix )
{
    std::vector<CorrelationMatrixRowOrColumn<ValueType, KeyType>> transposedMatrix;

    for ( size_t rowIdx = 0u; !matrix.empty() && rowIdx < matrix[0].m_correlations.size(); ++rowIdx )
    {
        ValueType            key = matrix[0].m_values[rowIdx];
        std::vector<double>  correlations;
        std::vector<KeyType> values;
        for ( size_t colIdx = 0u; colIdx < matrix.size(); ++colIdx )
        {
            correlations.push_back( matrix[colIdx].m_correlations[rowIdx] );
            values.push_back( matrix[colIdx].m_key );
        }
        transposedMatrix.push_back( CorrelationMatrixRowOrColumn<ValueType, KeyType>( key, correlations, values ) );
    }
    return transposedMatrix;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCorrelationMatrixPlot::createMatrix()
{
    time_t selectedTimestep = m_timeStep().toSecsSinceEpoch();

    m_paramLabels.clear();
    m_resultLabels.clear();

    auto curveDefs = curveDefinitions();
    if ( curveDefs.empty() ) return;

    QStringList ensembleNames;

    std::vector<CorrelationMatrixColumn> correlationMatrixColumns;

    for ( QString paramName : m_selectedParametersList() )
    {
        bool                                   anyValidResults = false;
        std::vector<double>                    correlations;
        std::vector<RiaSummaryCurveDefinition> selectedCurveDefs;

        for ( auto curveDef : curveDefs )
        {
            auto ensemble = curveDef.ensemble();
            auto address  = curveDef.summaryAddress();

            if ( ensemble )
            {
                std::set<RimSummaryCase*> activeCases = filterEnsembleCases( ensemble );

                std::vector<double> caseValuesAtTimestep;
                std::vector<double> parameterValues;

                RigEnsembleParameter parameter = ensemble->ensembleParameter( paramName );

                if ( parameter.isValid() )
                {
                    double correlation = std::numeric_limits<double>::infinity();

                    for ( size_t caseIdx = 0u; caseIdx < ensemble->allSummaryCases().size(); ++caseIdx )
                    {
                        auto summaryCase = ensemble->allSummaryCases()[caseIdx];
                        if ( activeCases.count( summaryCase ) == 0 ) continue;

                        RifSummaryReaderInterface* reader = summaryCase->summaryReader();
                        if ( reader )
                        {
                            std::vector<double> values;
                            double              closestValue    = std::numeric_limits<double>::infinity();
                            time_t              closestTimeStep = 0;
                            if ( reader->values( address, &values ) )
                            {
                                const std::vector<time_t>& timeSteps = reader->timeSteps( address );
                                for ( size_t i = 0; i < timeSteps.size(); ++i )
                                {
                                    if ( timeDiff( timeSteps[i], selectedTimestep ) <
                                         timeDiff( selectedTimestep, closestTimeStep ) )
                                    {
                                        closestValue    = values[i];
                                        closestTimeStep = timeSteps[i];
                                    }
                                }
                            }
                            if ( closestValue != std::numeric_limits<double>::infinity() )
                            {
                                caseValuesAtTimestep.push_back( closestValue );
                                double paramValue = parameter.values[caseIdx].toDouble();
                                parameterValues.push_back( paramValue );
                            }
                        }
                    }

                    if ( parameterValues.empty() ) continue;

                    correlation = RiaStatisticsTools::pearsonCorrelation( parameterValues, caseValuesAtTimestep );

                    bool validResult = RiaCurveDataTools::isValidValue( correlation, false );
                    if ( validResult )
                    {
                        if ( m_showAbsoluteValues() ) correlation = std::abs( correlation );
                        anyValidResults = true;
                    }
                    correlations.push_back( correlation );
                    selectedCurveDefs.push_back( curveDef );
                    ensembleNames.push_back( ensemble->name() );
                }
            }
        }
        if ( anyValidResults )
        {
            correlationMatrixColumns.push_back( CorrelationMatrixColumn( paramName, correlations, selectedCurveDefs ) );
        }
    }

    eraseInvalidEntries( correlationMatrixColumns );
    if ( m_sortByValues() == Sorting::COLUMNS || m_sortByValues() == Sorting::BOTH )
    {
        sortEntries( correlationMatrixColumns, m_sortByAbsoluteValues() || m_showAbsoluteValues() );

        if ( m_showOnlyTopNCorrelations && (size_t)m_topNFilterCount < correlationMatrixColumns.size() )
        {
            correlationMatrixColumns.erase( correlationMatrixColumns.begin() + m_topNFilterCount(),
                                            correlationMatrixColumns.end() );
        }
    }

    auto correlationMatrixRows = transpose( correlationMatrixColumns );

    eraseInvalidEntries( correlationMatrixRows );
    if ( m_sortByValues() == Sorting::ROWS || m_sortByValues() == Sorting::BOTH )
    {
        sortEntries( correlationMatrixRows, m_sortByAbsoluteValues() || m_showAbsoluteValues() );

        if ( m_showOnlyTopNCorrelations && (size_t)m_topNFilterCount < correlationMatrixRows.size() )
        {
            correlationMatrixRows.erase( correlationMatrixRows.begin() + m_topNFilterCount(), correlationMatrixRows.end() );
        }
    }

    ensembleNames.removeDuplicates();
    QString combinedEnsembleNames = ensembleNames.join( ";; " );
    for ( size_t rowIdx = 0u; rowIdx < correlationMatrixRows.size(); ++rowIdx )
    {
        for ( size_t colIdx = 0u; colIdx < correlationMatrixRows[rowIdx].m_correlations.size(); ++colIdx )
        {
            double correlation = correlationMatrixRows[rowIdx].m_correlations[colIdx];
            auto   label       = QString( "%1" ).arg( correlation, 0, 'f', 2 );

            cvf::Color3ub color = m_legendConfig->scalarMapper()->mapToColor( correlation );
            QColor        qColor( color.r(), color.g(), color.b() );
            auto          rectangle = RiuQwtPlotTools::createBoxShapeT<CorrelationMatrixShapeItem>( label,
                                                                                           (double)colIdx,
                                                                                           (double)colIdx + 1.0,
                                                                                           (double)rowIdx,
                                                                                           (double)rowIdx + 1,
                                                                                           qColor );
            rectangle->curveDef     = correlationMatrixRows[rowIdx].m_key;
            rectangle->parameter    = correlationMatrixRows[rowIdx].m_values[colIdx];
            QwtText      textLabel( label );
            cvf::Color3f contrastColor = RiaColorTools::contrastColor( cvf::Color3f( color ) );
            textLabel.setColor( RiaColorTools::toQColor( contrastColor ) );
            QFont font = textLabel.font();
            font.setPixelSize( caf::FontTools::pointSizeToPixelSize( labelFontSize() ) );
            textLabel.setFont( font );
            QwtPlotMarker* marker = new QwtPlotMarker();
            marker->setLabel( textLabel );
            marker->setXValue( colIdx + 0.5 );
            marker->setYValue( rowIdx + 0.5 );
            rectangle->attach( m_plotWidget );
            marker->attach( m_plotWidget );

            m_paramLabels[colIdx] = correlationMatrixRows[rowIdx].m_values[colIdx];
        }
        // Remove ensemble name from label if we only have one ensemble
        // If we have multiple ensembles, no labels contain the combined ensemble names.
        QString resultLabel = correlationMatrixRows[rowIdx].m_key.curveDefinitionText();
        resultLabel.remove( combinedEnsembleNames + ", " );

        m_resultLabels[rowIdx] = resultLabel;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCorrelationMatrixPlot::updatePlotTitle()
{
    if ( m_useAutoPlotTitle )
    {
        m_description = QString( "Correlation Matrix for Parameters vs Result Vectors at %2" ).arg( timeStepString() );
    }

    if ( m_plotWidget )
    {
        m_plotWidget->setPlotTitle( m_description );
        m_plotWidget->setPlotTitleEnabled( m_showPlotTitle && !isSubPlot() );
        if ( isMdiWindow() )
        {
            m_plotWidget->setPlotTitleFontSize( titleFontSize() );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCorrelationMatrixPlot::updateLegend()
{
    if ( m_legendConfig ) m_legendConfig->recreateLegend();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCorrelationMatrixPlot::onPlotItemSelected( QwtPlotItem* plotItem, bool toggle, int sampleIndex )
{
    CorrelationMatrixShapeItem* matrixItem = dynamic_cast<CorrelationMatrixShapeItem*>( plotItem );
    if ( matrixItem )
    {
        matrixCellSelected.send( std::make_pair( matrixItem->parameter, matrixItem->curveDef ) );
    }
}
