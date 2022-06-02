/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016 Statoil ASA
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

#include "RimSummaryPlotAxisFormatter.h"

#include "RiaDefines.h"
#include "RiaNumberFormat.h"
#include "RiaSummaryCurveDefinition.h"

#include "RifSummaryReaderInterface.h"

#include "RimAsciiDataCurve.h"
#include "RimPlotAxisProperties.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseCollection.h"
#include "RimSummaryCurve.h"

#include "RiuQtChartsPlotWidget.h"
#include "RiuQwtPlotTools.h"
#include "RiuSummaryQuantityNameInfoProvider.h"
#include "RiuSummaryQwtPlot.h"

#include "qwt_date_scale_engine.h"
#include "qwt_plot.h"
#include "qwt_plot_curve.h"
#include "qwt_scale_draw.h"
#include "qwt_text.h"

#include <cmath>
#include <set>
#include <string>

//--------------------------------------------------------------------------------------------------
// e    format as [-]9.9e[+|-]999
// E    format as[-]9.9E[+| -]999
// f    format as[-]9.9
// g    use e or f format, whichever is the most concise
// G    use E or f format, whichever is the most concise

//--------------------------------------------------------------------------------------------------
class SummaryScaleDraw : public QwtScaleDraw
{
public:
    SummaryScaleDraw( double                                  scaleFactor,
                      int                                     numberOfDecimals,
                      RimPlotAxisProperties::NumberFormatType numberFormat = RimPlotAxisProperties::NUMBER_FORMAT_AUTO )
    {
        m_scaleFactor      = scaleFactor;
        m_numberOfDecimals = numberOfDecimals;
        m_numberFormat     = numberFormat;
    }

    QwtText label( double value ) const override
    {
        if ( qFuzzyCompare( scaledValue( value ) + 1.0, 1.0 ) ) value = 0.0;

        return QString::number( scaledValue( value ), numberFormat(), m_numberOfDecimals );
    }

private:
    char numberFormat() const
    {
        switch ( m_numberFormat )
        {
            case RimPlotAxisProperties::NUMBER_FORMAT_AUTO:
                return 'g';
            case RimPlotAxisProperties::NUMBER_FORMAT_DECIMAL:
                return 'f';
            case RimPlotAxisProperties::NUMBER_FORMAT_SCIENTIFIC:
                return 'e';
            default:
                return 'g';
        }
    }

    double scaledValue( double value ) const { return value / m_scaleFactor; }

private:
    double                                  m_scaleFactor;
    int                                     m_numberOfDecimals;
    RimPlotAxisProperties::NumberFormatType m_numberFormat;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryPlotAxisFormatter::RimSummaryPlotAxisFormatter( RimPlotAxisProperties*                        axisProperties,
                                                          const std::vector<RimSummaryCurve*>&          summaryCurves,
                                                          const std::vector<RiaSummaryCurveDefinition>& curveDefinitions,
                                                          const std::vector<RimAsciiDataCurve*>&        asciiCurves,
                                                          const std::set<QString>& timeHistoryCurveQuantities )
    : m_axisProperties( axisProperties )
    , m_summaryCurves( summaryCurves )
    , m_curveDefinitions( curveDefinitions )
    , m_asciiDataCurves( asciiCurves )
    , m_timeHistoryCurveQuantities( timeHistoryCurveQuantities )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotAxisFormatter::applyAxisPropertiesToPlot( RiuPlotWidget* plotWidget )
{
    if ( !plotWidget ) return;

    RiuPlotAxis axis = m_axisProperties->plotAxisType();
    {
        QString axisTitle = m_axisProperties->customTitle();
        if ( m_axisProperties->useAutoTitle() ) axisTitle = autoAxisTitle();

        Qt::AlignmentFlag titleAlignment = Qt::AlignCenter;
        if ( m_axisProperties->titlePosition() == RimPlotAxisPropertiesInterface::AXIS_TITLE_END )
        {
            titleAlignment = Qt::AlignRight;
        }

        QString objectName = createAxisObjectName();
        m_axisProperties->setNameAndAxis( objectName, axisTitle, axis.axis(), axis.index() );
        plotWidget->setAxisTitleText( axis, axisTitle );

        bool titleBold = false;
        plotWidget->setAxisFontsAndAlignment( axis,
                                              m_axisProperties->titleFontSize(),
                                              m_axisProperties->valuesFontSize(),
                                              titleBold,
                                              titleAlignment );
        plotWidget->setAxisTitleEnabled( axis, true );
    }

    auto qwtPlotWidget = dynamic_cast<RiuQwtPlotWidget*>( plotWidget );
    if ( qwtPlotWidget )
    {
        auto qwtAxisId = qwtPlotWidget->toQwtPlotAxis( axis );

        if ( m_axisProperties->numberFormat() == RimPlotAxisProperties::NUMBER_FORMAT_AUTO &&
             m_axisProperties->scaleFactor() == 1.0 )
        {
            // Default to Qwt's own scale draw to avoid changing too much for default values
            qwtPlotWidget->qwtPlot()->setAxisScaleDraw( qwtAxisId, new QwtScaleDraw );
        }
        else
        {
            qwtPlotWidget->qwtPlot()->setAxisScaleDraw( qwtAxisId,
                                                        new SummaryScaleDraw( m_axisProperties->scaleFactor(),
                                                                              m_axisProperties->decimalCount(),
                                                                              m_axisProperties->numberFormat() ) );
        }
    }

#ifdef USE_QTCHARTS
    auto qtChartsPlotWidget = dynamic_cast<RiuQtChartsPlotWidget*>( plotWidget );
    if ( qtChartsPlotWidget )
    {
        auto mapToRiaNumberFormatType = []( RimPlotAxisProperties::NumberFormatType formatType ) {
            if ( formatType == RimPlotAxisProperties::NumberFormatType::NUMBER_FORMAT_DECIMAL )
                return RiaNumberFormat::NumberFormatType::FIXED;

            if ( formatType == RimPlotAxisProperties::NumberFormatType::NUMBER_FORMAT_SCIENTIFIC )
                return RiaNumberFormat::NumberFormatType::SCIENTIFIC;

            return RiaNumberFormat::NumberFormatType::AUTO;
        };

        auto    formatType = mapToRiaNumberFormatType( m_axisProperties->numberFormat() );
        QString format     = RiaNumberFormat::sprintfFormat( formatType, m_axisProperties->decimalCount() );
        qtChartsPlotWidget->setAxisFormat( axis, format );
    }
#endif

    {
        if ( m_axisProperties->isLogarithmicScaleEnabled() )
        {
            bool isLogScale = plotWidget->axisScaleType( axis ) == RiuQwtPlotWidget::AxisScaleType::LOGARITHMIC;
            if ( !isLogScale )
            {
                plotWidget->setAxisScaleType( axis, RiuQwtPlotWidget::AxisScaleType::LOGARITHMIC );
                plotWidget->setAxisMaxMinor( axis, 5 );
            }
        }
        else
        {
            bool isLinearScale = plotWidget->axisScaleType( axis ) == RiuQwtPlotWidget::AxisScaleType::LINEAR;
            if ( !isLinearScale )
            {
                plotWidget->setAxisScaleType( axis, RiuQwtPlotWidget::AxisScaleType::LINEAR );
                plotWidget->setAxisMaxMinor( axis, 3 );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummaryPlotAxisFormatter::autoAxisTitle() const
{
    std::map<std::string, std::set<std::string>> unitToQuantityNameMap;

    auto addToUnitToQuantityMap = [&]( const std::string& unitText, const RifEclipseSummaryAddress& sumAddress ) {
        size_t cutPos = sumAddress.vectorName().find( ':' );
        if ( cutPos == std::string::npos ) cutPos = -1;

        std::string        titleText;
        const std::string& quantityName = sumAddress.vectorName().substr( cutPos + 1 );

        if ( sumAddress.category() == RifEclipseSummaryAddress::SUMMARY_CALCULATED )
        {
            titleText = shortCalculationName( quantityName );
        }
        else
        {
            if ( m_axisProperties->showDescription() )
            {
                auto candidateName = quantityName;

                if ( sumAddress.isHistoryVector() ) candidateName = quantityName.substr( 0, quantityName.size() - 1 );

                titleText = RiuSummaryQuantityNameInfoProvider::instance()->longNameFromVectorName( candidateName );
            }

            if ( m_axisProperties->showAcronym() )
            {
                if ( !titleText.empty() )
                {
                    titleText += " (";
                    titleText += quantityName;
                    titleText += ")";
                }
                else
                {
                    titleText += quantityName;
                }
            }
        }

        unitToQuantityNameMap[unitText].insert( titleText );
    };

    for ( RimSummaryCurve* rimCurve : m_summaryCurves )
    {
        RifEclipseSummaryAddress sumAddress;
        std::string              unitText;

        if ( m_axisProperties->plotAxisType().axis() == RiaDefines::PlotAxis::PLOT_AXIS_BOTTOM )
        {
            sumAddress = rimCurve->summaryAddressX();
            unitText   = rimCurve->unitNameX();
        }
        else if ( rimCurve->axisY() == this->m_axisProperties->plotAxisType() )
        {
            sumAddress = rimCurve->summaryAddressY();
            unitText   = rimCurve->unitNameY();
        }
        else
        {
            continue;
        }

        addToUnitToQuantityMap( unitText, sumAddress );
    }

    for ( const RiaSummaryCurveDefinition& curveDef : m_curveDefinitions )
    {
        const RifEclipseSummaryAddress& sumAddress = curveDef.summaryAddress();
        std::string                     unitText;
        if ( curveDef.summaryCase() && curveDef.summaryCase()->summaryReader() )
        {
            unitText = curveDef.summaryCase()->summaryReader()->unitName( sumAddress );
        }
        else if ( curveDef.ensemble() )
        {
            std::vector<RimSummaryCase*> sumCases = curveDef.ensemble()->allSummaryCases();
            if ( !sumCases.empty() && sumCases[0] && sumCases[0]->summaryReader() )
            {
                unitText = sumCases[0]->summaryReader()->unitName( sumAddress );
            }
        }

        addToUnitToQuantityMap( unitText, sumAddress );
    }

    QString assembledYAxisText;
    QString scaleFactorText = "";

    if ( m_axisProperties->scaleFactor() != 1.0 )
    {
        int exponent    = std::log10( m_axisProperties->scaleFactor() );
        scaleFactorText = QString( " x 10<sup>%1</sup> " ).arg( QString::number( exponent ) );
    }

    for ( const auto& unitIt : unitToQuantityNameMap )
    {
        for ( const auto& quantIt : unitIt.second )
        {
            assembledYAxisText += QString::fromStdString( quantIt ) + " ";
        }

        if ( m_axisProperties->showUnitText() )
        {
            QString unitAndScaleText = QString::fromStdString( unitIt.first ) + scaleFactorText;

            if ( !unitAndScaleText.isEmpty() )
            {
                assembledYAxisText += "[" + unitAndScaleText + "] ";
            }
        }
    }

    if ( !m_timeHistoryCurveQuantities.empty() )
    {
        if ( !assembledYAxisText.isEmpty() )
        {
            assembledYAxisText += " : ";
        }

        for ( const auto& timeQuantity : m_timeHistoryCurveQuantities )
        {
            assembledYAxisText += timeQuantity + " ";
        }
    }

    return assembledYAxisText;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummaryPlotAxisFormatter::createAxisObjectName() const
{
    std::set<std::string> vectorNames;

    auto addVectorNames = [&]( const RifEclipseSummaryAddress& sumAddress ) {
        size_t cutPos = sumAddress.vectorName().find( ':' );
        if ( cutPos == std::string::npos ) cutPos = -1;

        std::string        name;
        const std::string& quantityName = sumAddress.vectorName().substr( cutPos + 1 );

        if ( sumAddress.category() == RifEclipseSummaryAddress::SUMMARY_CALCULATED )
        {
            name = shortCalculationName( quantityName );
        }
        else
        {
            name = quantityName;
        }
        vectorNames.insert( name );
    };

    for ( RimSummaryCurve* rimCurve : m_summaryCurves )
    {
        RifEclipseSummaryAddress sumAddress;

        if ( m_axisProperties->plotAxisType().axis() == RiaDefines::PlotAxis::PLOT_AXIS_BOTTOM )
        {
            sumAddress = rimCurve->summaryAddressX();
        }
        else if ( rimCurve->axisY() == this->m_axisProperties->plotAxisType() )
        {
            sumAddress = rimCurve->summaryAddressY();
        }
        else
        {
            continue;
        }

        addVectorNames( sumAddress );
    }

    for ( const RiaSummaryCurveDefinition& curveDef : m_curveDefinitions )
    {
        const RifEclipseSummaryAddress& sumAddress = curveDef.summaryAddress();

        addVectorNames( sumAddress );
    }

    QString assembledAxisObjectName;

    for ( const auto& vectorName : vectorNames )
    {
        assembledAxisObjectName += QString::fromStdString( vectorName ) + " ";
    }

    if ( !m_timeHistoryCurveQuantities.empty() )
    {
        if ( !assembledAxisObjectName.isEmpty() )
        {
            assembledAxisObjectName += " : ";
        }

        for ( const auto& timeQuantity : m_timeHistoryCurveQuantities )
        {
            assembledAxisObjectName += timeQuantity + " ";
        }
    }

    const int    maxChars = 100;
    QFont        font;
    QFontMetrics fm( font );
    assembledAxisObjectName = fm.elidedText( assembledAxisObjectName, Qt::ElideRight, maxChars );

    return assembledAxisObjectName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RimSummaryPlotAxisFormatter::shortCalculationName( const std::string& calculationName )
{
    QString calculationShortName = QString::fromStdString( calculationName );

    int indexOfFirstSpace = calculationShortName.indexOf( ' ' );
    if ( indexOfFirstSpace > -1 && indexOfFirstSpace < calculationShortName.size() )
    {
        calculationShortName = calculationShortName.left( indexOfFirstSpace );
    }

    return calculationShortName.toStdString();
}
