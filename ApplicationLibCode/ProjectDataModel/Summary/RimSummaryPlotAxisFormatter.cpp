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
#include "RiaSummaryCurveDefinition.h"

#include "RifSummaryReaderInterface.h"

#include "RimAsciiDataCurve.h"
#include "RimPlotAxisProperties.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseCollection.h"
#include "RimSummaryCurve.h"

#include "RiuSummaryQuantityNameInfoProvider.h"
#include "RiuSummaryQwtPlot.h"

#include "qwt_date_scale_engine.h"
#include "qwt_plot_curve.h"
#include "qwt_scale_draw.h"
#include "qwt_scale_engine.h"

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
void RimSummaryPlotAxisFormatter::applyAxisPropertiesToPlot( RiuQwtPlotWidget* qwtPlot )
{
    if ( !qwtPlot ) return;

    QwtPlot::Axis qwtAxisId = RiaDefines::toQwtPlotAxis( m_axisProperties->plotAxisType() );
    {
        QString axisTitle = m_axisProperties->customTitle;
        if ( m_axisProperties->useAutoTitle() ) axisTitle = autoAxisTitle();

        Qt::AlignmentFlag titleAlignment = Qt::AlignCenter;
        if ( m_axisProperties->titlePosition() == RimPlotAxisPropertiesInterface::AXIS_TITLE_END )
        {
            titleAlignment = Qt::AlignRight;
        }
        qwtPlot->setAxisTitleText( qwtAxisId, axisTitle );
        qwtPlot->setAxisFontsAndAlignment( qwtAxisId,
                                           m_axisProperties->titleFontSize(),
                                           m_axisProperties->valuesFontSize(),
                                           true,
                                           titleAlignment );
        qwtPlot->setAxisTitleEnabled( qwtAxisId, true );
    }

    {
        if ( m_axisProperties->numberFormat == RimPlotAxisProperties::NUMBER_FORMAT_AUTO &&
             m_axisProperties->scaleFactor() == 1.0 )
        {
            // Default to Qwt's own scale draw to avoid changing too much for default values
            qwtPlot->setAxisScaleDraw( qwtAxisId, new QwtScaleDraw );
        }
        else
        {
            qwtPlot->setAxisScaleDraw( qwtAxisId,
                                       new SummaryScaleDraw( m_axisProperties->scaleFactor(),
                                                             m_axisProperties->numberOfDecimals(),
                                                             m_axisProperties->numberFormat() ) );
        }
    }

    {
        if ( m_axisProperties->isLogarithmicScaleEnabled )
        {
            QwtLogScaleEngine* currentScaleEngine =
                dynamic_cast<QwtLogScaleEngine*>( qwtPlot->axisScaleEngine( qwtAxisId ) );
            if ( !currentScaleEngine )
            {
                qwtPlot->setAxisScaleEngine( qwtAxisId, new QwtLogScaleEngine );
                qwtPlot->setAxisMaxMinor( qwtAxisId, 5 );
            }
        }
        else
        {
            QwtLinearScaleEngine* currentScaleEngine =
                dynamic_cast<QwtLinearScaleEngine*>( qwtPlot->axisScaleEngine( qwtAxisId ) );
            QwtDateScaleEngine* dateScaleEngine = dynamic_cast<QwtDateScaleEngine*>( currentScaleEngine );
            if ( !currentScaleEngine || dateScaleEngine )
            {
                qwtPlot->setAxisScaleEngine( qwtAxisId, new QwtLinearScaleEngine );
                qwtPlot->setAxisMaxMinor( qwtAxisId, 3 );
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

    // clang-format off
    auto addToUnitToQuantityMap =[&]( const std::string& unitText, 
                                      const RifEclipseSummaryAddress& sumAddress ) 
    {
        // remove any stats prefix from the quantity name
        size_t cutPos = sumAddress.quantityName().find(':');
        if (cutPos == std::string::npos) cutPos = -1;

        std::string        quantityNameForDisplay;
        const std::string& quantityName = sumAddress.quantityName().substr(cutPos+1);

        if ( sumAddress.category() == RifEclipseSummaryAddress::SUMMARY_CALCULATED )
        {
            quantityNameForDisplay = shortCalculationName( quantityName );
        }
        else
        {
            if ( m_axisProperties->showDescription() )
            {
                quantityNameForDisplay =
                    RiuSummaryQuantityNameInfoProvider::instance()->longNameFromQuantityName( quantityName );
            }

            if ( m_axisProperties->showAcronym() )
            {
                if ( !quantityNameForDisplay.empty() )
                {
                    quantityNameForDisplay += " (";
                    quantityNameForDisplay += quantityName;
                    quantityNameForDisplay += ")";
                }
            }

            if ( quantityNameForDisplay.empty() )
            {
                quantityNameForDisplay = quantityName;
            }
        }

        unitToQuantityNameMap[unitText].insert( quantityNameForDisplay );
    };

    // clang-format on

    for ( RimSummaryCurve* rimCurve : m_summaryCurves )
    {
        RifEclipseSummaryAddress sumAddress;
        std::string              unitText;

        if ( m_axisProperties->plotAxisType() == RiaDefines::PlotAxis::PLOT_AXIS_BOTTOM )
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
        RifEclipseSummaryAddress sumAddress = curveDef.summaryAddress();
        std::string              unitText;
        if ( curveDef.summaryCase() && curveDef.summaryCase()->summaryReader() )
        {
            unitText = curveDef.summaryCase()->summaryReader()->unitName( sumAddress );
        }
        else if ( curveDef.ensemble() )
        {
            std::vector<RimSummaryCase*> sumCases = curveDef.ensemble()->allSummaryCases();
            if ( sumCases.size() && sumCases[0] && sumCases[0]->summaryReader() )
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

    for ( auto unitIt : unitToQuantityNameMap )
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
