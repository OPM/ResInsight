/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020- Equinor ASA
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

#include "RimVfpPlot.h"

#include "RimEclipseResultCase.h"
#include "RimFlowDiagSolution.h"
#include "RimProject.h"
#include "RimTools.h"
#include "RimVfpTableExtractor.h"

#include "RigEclipseCaseData.h"
#include "RigTofWellDistributionCalculator.h"

#include "RiaColorTools.h"

#include "RiuQwtPlotWidget.h"

#include "qwt_legend.h"
#include "qwt_legend_label.h"
#include "qwt_plot.h"
#include "qwt_plot_curve.h"
#include "qwt_symbol.h"

#include <QGridLayout>
#include <QTextBrowser>
#include <QWidget>

#include <array>

#include "cvfDebugTimer.h"
#include "cvfTrace.h"

#include <QFileInfo>

//==================================================================================================
//
//
//
//==================================================================================================

CAF_PDM_SOURCE_INIT( RimVfpPlot, "VfpPlot" );

namespace caf
{
template <>
void caf::AppEnum<RimVfpPlot::TableType>::setUp()
{
    addItem( RimVfpPlot::TableType::INJECTION, "INJECTION", "Injection" );
    addItem( RimVfpPlot::TableType::PRODUCTION, "PRODUCTION", "Production" );
    setDefault( RimVfpPlot::TableType::INJECTION );
}

template <>
void caf::AppEnum<RimVfpPlot::ProductionTableType>::setUp()
{
    addItem( RimVfpPlot::ProductionTableType::LIQUID_FLOW_RATE, "LIQUID_FLOW_RATE", "Liquid Flow Rate" );
    addItem( RimVfpPlot::ProductionTableType::ARTIFICIAL_LIFT_QUANTITY, "ALQ", "Artificial Lift Quantity" );
    addItem( RimVfpPlot::ProductionTableType::WATER_CUT, "WATER_CUT", "Water Cut" );
    addItem( RimVfpPlot::ProductionTableType::GAS_LIQUID_RATIO, "GAS_LIQUID_RATIO", "Gas Liquid Ratio" );
    setDefault( RimVfpPlot::ProductionTableType::LIQUID_FLOW_RATE );
}
} // namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimVfpPlot::RimVfpPlot()
{
    // TODO: add icon
    CAF_PDM_InitObject( "VFP Plot", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_case, "Case", "Case", "", "", "" );
    CAF_PDM_InitField( &m_wellName, "WellName", QString( "None" ), "Well", "", "", "" );

    caf::AppEnum<RimVfpPlot::TableType> defaultTableType = RimVfpPlot::TableType::INJECTION;
    CAF_PDM_InitField( &m_tableType, "TableType", defaultTableType, "Table Type", "", "", "" );

    caf::AppEnum<RimVfpPlot::ProductionTableType> defaultProductionTableType =
        RimVfpPlot::ProductionTableType::LIQUID_FLOW_RATE;
    CAF_PDM_InitField( &m_productionTableType,
                       "ProductionTableType",
                       defaultProductionTableType,
                       "Production Table Type",
                       "",
                       "",
                       "" );

    m_showWindow      = false;
    m_showPlotLegends = true;

    setAsPlotMdiWindow();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimVfpPlot::~RimVfpPlot()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlot::setDataSourceParameters( RimEclipseResultCase* eclipseResultCase, QString targetWellName )
{
    m_case     = eclipseResultCase;
    m_wellName = targetWellName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuQwtPlotWidget* RimVfpPlot::viewer()
{
    return m_plotWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlot::setAutoScaleXEnabled( bool /*enabled*/ )
{
    cvf::Trace::show( "RimVfpPlot::setAutoScaleXEnabled()" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlot::setAutoScaleYEnabled( bool /*enabled*/ )
{
    cvf::Trace::show( "RimVfpPlot::setAutoScaleYEnabled()" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlot::updateAxes()
{
    cvf::Trace::show( "RimVfpPlot::updateAxes()" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlot::updateLegend()
{
    if ( !m_plotWidget )
    {
        return;
    }

    // Hide the legend when in multiplot mode, as the legend is handeled by the multi plot grid layout
    bool doShowLegend = false;
    if ( isMdiWindow() )
    {
        doShowLegend = m_showPlotLegends;
    }

    if ( doShowLegend )
    {
        QwtLegend* legend = new QwtLegend( m_plotWidget );
        m_plotWidget->insertLegend( legend, QwtPlot::BottomLegend );
    }
    else
    {
        m_plotWidget->insertLegend( nullptr );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlot::updateZoomInQwt()
{
    cvf::Trace::show( "RimVfpPlot::updateZoomInQwt()" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlot::updateZoomFromQwt()
{
    cvf::Trace::show( "RimVfpPlot::updateZoomFromQwt()" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimVfpPlot::asciiDataForPlotExport() const
{
    cvf::Trace::show( "RimVfpPlot::asciiDataForPlotExport()" );
    return QString();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlot::reattachAllCurves()
{
    cvf::Trace::show( "RimVfpPlot::reattachAllCurves()" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlot::detachAllCurves()
{
    cvf::Trace::show( "RimVfpPlot::detachAllCurves()" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmObject* RimVfpPlot::findPdmObjectFromQwtCurve( const QwtPlotCurve* /*curve*/ ) const
{
    cvf::Trace::show( "RimVfpPlot::findPdmObjectFromQwtCurve()" );
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlot::onAxisSelected( int /*axis*/, bool /*toggle*/ )
{
    cvf::Trace::show( "RimVfpPlot::onAxisSelected()" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimVfpPlot::description() const
{
    return uiName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RimVfpPlot::viewWidget()
{
    cvf::Trace::show( "RimVfpPlot::viewWidget()" );
    return m_plotWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QImage RimVfpPlot::snapshotWindowContent()
{
    cvf::Trace::show( "RimVfpPlot::snapshotWindowContent()" );

    QImage image;

    if ( m_plotWidget )
    {
        QPixmap pix = m_plotWidget->grab();
        image       = pix.toImage();
    }

    return image;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlot::zoomAll()
{
    cvf::Trace::show( "RimVfpPlot::zoomAll()" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlot::doRemoveFromCollection()
{
    cvf::Trace::show( "RimVfpPlot::doRemoveFromCollection()" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuQwtPlotWidget* RimVfpPlot::doCreatePlotViewWidget( QWidget* mainWindowParent )
{
    cvf::Trace::show( "RimVfpPlot::createViewWidget()" );

    // It seems we risk being called multiple times
    if ( m_plotWidget )
    {
        return m_plotWidget;
    }

    m_plotWidget = new RiuQwtPlotWidget( this, mainWindowParent );

    //    m_plotWidget->setAutoReplot( false );

    updateLegend();
    onLoadDataAndUpdate();

    return m_plotWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlot::deleteViewWidget()
{
    cvf::Trace::show( "RimVfpPlot::deleteViewWidget()" );

    if ( m_plotWidget )
    {
        m_plotWidget->setParent( nullptr );
        delete m_plotWidget;
        m_plotWidget = nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlot::onLoadDataAndUpdate()
{
    cvf::Trace::show( "RimVfpPlot::onLoadDataAndUpdate()" );
    cvf::DebugTimer tim( "RimVfpPlot::onLoadDataAndUpdate()" );

    if ( isMdiWindow() )
    {
        updateMdiWindowVisibility();
    }
    else
    {
        updateParentLayout();
    }

    if ( !m_plotWidget )
    {
        return;
    }

    m_plotWidget->detachItems( QwtPlotItem::Rtti_PlotCurve );

    updateLegend();

    QString phaseString = "N/A";
    bool    isInjector  = m_tableType == RimVfpPlot::TableType::INJECTION;
    if ( m_case && m_case->ensureReservoirCaseIsOpen() )
    {
        if ( isInjector )
        {
            std::set<std::string> wells            = {"F2H", "C1H", "C2H", "C3H", "C4AH", "C4H", "F1H", "F3H", "F4H"};
            std::string           strippedWellName = QString( m_wellName() ).remove( "-" ).toStdString();

            if ( wells.find( strippedWellName ) != wells.end() )
            {
                QString   gridFileName = m_case->gridFileName();
                QFileInfo fi( gridFileName );

                std::string filename = fi.canonicalPath().toStdString() + "/INCLUDE/VFP/" + strippedWellName + ".Ecl";
                const std::vector<Opm::VFPInjTable> tables = RimVfpTableExtractor::extractVfpInjectionTables( filename );
                populatePlotWidgetWithCurveData( m_plotWidget, tables );
            }
        }
        else
        {
            std::set<std::string> wells = {"B1BH", "B2H", "B3H", "D1CH", "B4DH", "E1H", "D2H", "D3BH", "E3CH"};
            std::string           strippedWellName = QString( m_wellName() ).remove( "-" ).toStdString();

            if ( wells.find( strippedWellName ) != wells.end() )
            {
                QString   gridFileName = m_case->gridFileName();
                QFileInfo fi( gridFileName );

                std::string filename = fi.canonicalPath().toStdString() + "/INCLUDE/VFP/" + strippedWellName + ".Ecl";
                const std::vector<Opm::VFPProdTable> tables = RimVfpTableExtractor::extractVfpProductionTables( filename );
                populatePlotWidgetWithCurveData( m_plotWidget, tables, m_productionTableType() );
            }
        }
    }

    const QString plotTitleStr = QString( "%1 Vertical Flow Performance Plot" ).arg( m_wellName );
    m_plotWidget->setTitle( plotTitleStr );

    if ( isInjector )
    {
        m_plotWidget->setAxisTitleText( QwtPlot::xBottom, "Liquid Flow Rate [sm3/d]" );
        m_plotWidget->setAxisTitleText( QwtPlot::yLeft, "Bottom Hole Pressure [Bar]" );
    }
    else
    {
        m_plotWidget->setAxisTitleText( QwtPlot::xBottom, "x axis (todo) [x axis unit]" );
        m_plotWidget->setAxisTitleText( QwtPlot::yLeft, "y axis [y axis unit]" );
    }

    m_plotWidget->setAxisTitleEnabled( QwtPlot::xBottom, true );
    m_plotWidget->setAxisTitleEnabled( QwtPlot::yLeft, true );

    m_plotWidget->scheduleReplot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlot::populatePlotWidgetWithCurveData( RiuQwtPlotWidget* plotWidget, const std::vector<Opm::VFPInjTable>& tables )
{
    cvf::Trace::show( "RimVfpPlot::populatePlotWidgetWithCurves()" );

    plotWidget->detachItems( QwtPlotItem::Rtti_PlotCurve );
    plotWidget->setAxisScale( QwtPlot::xBottom, 0, 1 );
    plotWidget->setAxisScale( QwtPlot::yLeft, 0, 1 );
    plotWidget->setAxisAutoScale( QwtPlot::xBottom, true );
    plotWidget->setAxisAutoScale( QwtPlot::yLeft, true );

    size_t numTables = tables.size();

    for ( size_t i = 0; i < numTables; i++ )
    {
        const Opm::VFPInjTable table = tables[i];
        std::cout << "Datum depth: " << table.getDatumDepth() << std::endl;
        std::cout << "Table number: " << table.getTableNum() << std::endl;
        std::cout << "Flow type: " << static_cast<int>( table.getFloType() ) << std::endl;
        std::cout << "Flo axis: " << table.getFloAxis().size() << std::endl;
        std::cout << "THP axis: " << table.getTHPAxis().size() << std::endl;

        std::cout << "THP Axis:\n";
        for ( size_t x = 0; x < table.getTHPAxis().size(); x++ )
        {
            std::cout << " " << table.getTHPAxis()[x];
        }
        std::cout << "\n";

        for ( size_t y = 0; y < table.getFloAxis().size(); y++ )
        {
            for ( size_t x = 0; x < table.getTHPAxis().size(); x++ )
            {
                std::cout << " " << table( x, y );
            }
            std::cout << std::endl;
        }

        for ( size_t thp = 0; thp < table.getTHPAxis().size(); thp++ )
        {
            // Just create some dummy values for now
            size_t              numValues = table.getFloAxis().size();
            std::vector<double> xVals     = table.getFloAxis();
            std::vector<double> yVals( numValues, 0.0 );
            for ( size_t y = 0; y < numValues; y++ )
            {
                // Convert from Pascal to Bar
                yVals[y] = table( thp, y ) / 100000.0;
            }

            cvf::Color3f cvfClr = cvf::Color3::BLUE;
            QColor       qtClr  = RiaColorTools::toQColor( cvfClr );

            QwtPlotCurve* curve = new QwtPlotCurve;
            // Convert from Pascal to Bar
            curve->setTitle( QString( "THP: %1 Bar" ).arg( table.getTHPAxis()[thp] / 100000.0 ) );
            QwtSymbol* symbol = new QwtSymbol( QwtSymbol::Ellipse, QBrush( qtClr ), QPen( Qt::red, 2 ), QSize( 8, 8 ) );
            curve->setSymbol( symbol );

            curve->setSamples( xVals.data(), yVals.data(), numValues );
            curve->attach( plotWidget );
            curve->show();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlot::populatePlotWidgetWithCurveData( RiuQwtPlotWidget*                     plotWidget,
                                                  const std::vector<Opm::VFPProdTable>& tables,
                                                  RimVfpPlot::ProductionTableType       productionTableType )
{
    cvf::Trace::show( "RimVfpPlot::populatePlotWidgetWithCurves()" );

    plotWidget->detachItems( QwtPlotItem::Rtti_PlotCurve );
    plotWidget->setAxisScale( QwtPlot::xBottom, 0, 1 );
    plotWidget->setAxisScale( QwtPlot::yLeft, 0, 1 );
    plotWidget->setAxisAutoScale( QwtPlot::xBottom, true );
    plotWidget->setAxisAutoScale( QwtPlot::yLeft, true );

    size_t numTables = tables.size();

    for ( size_t i = 0; i < numTables; i++ )
    {
        const Opm::VFPProdTable table = tables[i];
        std::cout << "Datum depth: " << table.getDatumDepth() << std::endl;
        std::cout << "Table number: " << table.getTableNum() << std::endl;
        std::cout << "Flow type: " << static_cast<int>( table.getFloType() ) << std::endl;
        std::cout << "Flo axis: " << table.getFloAxis().size() << std::endl;
        std::cout << "THP axis: " << table.getTHPAxis().size() << std::endl;
        std::cout << "WFR axis: " << table.getWFRAxis().size() << std::endl;
        std::cout << "GFR axis: " << table.getGFRAxis().size() << std::endl;
        std::cout << "ALQ axis: " << table.getALQAxis().size() << std::endl;

        std::cout << "THP Axis:\n";
        for ( size_t x = 0; x < table.getTHPAxis().size(); x++ )
        {
            std::cout << " " << table.getTHPAxis()[x];
        }
        std::cout << "\n";

        for ( size_t thp_idx = 0; thp_idx < table.getTHPAxis().size(); thp_idx++ )
        {
            size_t wfr_idx = table.getWFRAxis().size() - 1;
            size_t gfr_idx = table.getGFRAxis().size() - 1;
            size_t alq_idx = table.getALQAxis().size() - 1;
            size_t flo_idx = table.getFloAxis().size() - 1;

            std::vector<double> xVals;
            if ( productionTableType == RimVfpPlot::ProductionTableType::WATER_CUT )
            {
                xVals = table.getWFRAxis();
            }
            else if ( productionTableType == RimVfpPlot::ProductionTableType::GAS_LIQUID_RATIO )
            {
                xVals = table.getGFRAxis();
            }
            else if ( productionTableType == RimVfpPlot::ProductionTableType::ARTIFICIAL_LIFT_QUANTITY )
            {
                xVals = table.getALQAxis();
            }
            else if ( productionTableType == RimVfpPlot::ProductionTableType::LIQUID_FLOW_RATE )
            {
                xVals = table.getFloAxis();
            }

            size_t              numValues = xVals.size();
            std::vector<double> yVals( numValues, 0.0 );

            for ( size_t y = 0; y < numValues; y++ )
            {
                if ( productionTableType == RimVfpPlot::ProductionTableType::WATER_CUT )
                    wfr_idx = y;
                else if ( productionTableType == RimVfpPlot::ProductionTableType::GAS_LIQUID_RATIO )
                    gfr_idx = y;
                else if ( productionTableType == RimVfpPlot::ProductionTableType::ARTIFICIAL_LIFT_QUANTITY )
                    alq_idx = y;
                else if ( productionTableType == RimVfpPlot::ProductionTableType::LIQUID_FLOW_RATE )
                    flo_idx = y;

                // Convert from Pascal to Bar
                yVals[y] = table( thp_idx, wfr_idx, gfr_idx, alq_idx, flo_idx ) / 100000.0;
            }

            cvf::Color3f cvfClr = cvf::Color3::BLUE;
            QColor       qtClr  = RiaColorTools::toQColor( cvfClr );

            QwtPlotCurve* curve = new QwtPlotCurve;
            // Convert from Pascal to Bar
            curve->setTitle( QString( "THP: %1 Bar" ).arg( table.getTHPAxis()[thp_idx] / 100000.0 ) );
            QwtSymbol* symbol = new QwtSymbol( QwtSymbol::Ellipse, QBrush( qtClr ), QPen( Qt::red, 2 ), QSize( 8, 8 ) );
            curve->setSymbol( symbol );

            curve->setSamples( xVals.data(), yVals.data(), numValues );
            curve->attach( plotWidget );
            curve->show();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlot::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_case );
    uiOrdering.add( &m_wellName );
    uiOrdering.add( &m_tableType );
    uiOrdering.add( &m_productionTableType );
    m_productionTableType.uiCapability()->setUiHidden( m_tableType != RimVfpPlot::TableType::PRODUCTION );

    uiOrdering.skipRemainingFields( true );
    // RimPlot::defineUiOrdering( uiConfigName, uiOrdering );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimVfpPlot::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                                 bool*                      useOptionsOnly )
{
    QList<caf::PdmOptionItemInfo> options = RimPlot::calculateValueOptions( fieldNeedingOptions, useOptionsOnly );

    if ( fieldNeedingOptions == &m_case )
    {
        RimProject* ownerProj = nullptr;
        firstAncestorOrThisOfType( ownerProj );
        if ( ownerProj )
        {
            std::vector<RimEclipseResultCase*> caseArr;
            ownerProj->descendantsIncludingThisOfType( caseArr );
            for ( RimEclipseResultCase* c : caseArr )
            {
                options.push_back( caf::PdmOptionItemInfo( c->caseUserDescription(), c, true, c->uiIconProvider() ) );
            }
        }
    }

    else if ( fieldNeedingOptions == &m_wellName )
    {
        if ( m_case && m_case->eclipseCaseData() )
        {
            caf::QIconProvider      simWellIcon( ":/Well.png" );
            const std::set<QString> sortedWellNameSet = m_case->eclipseCaseData()->findSortedWellNames();
            for ( const QString& name : sortedWellNameSet )
            {
                options.push_back( caf::PdmOptionItemInfo( name, name, true, simWellIcon ) );
            }
        }

        if ( options.size() == 0 )
        {
            options.push_back( caf::PdmOptionItemInfo( "None", QVariant() ) );
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlot::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    RimPlot::fieldChangedByUi( changedField, oldValue, newValue );

    if ( changedField == &m_case )
    {
        fixupDependentFieldsAfterCaseChange();
    }

    loadDataAndUpdate();
    updateLayout();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlot::fixupDependentFieldsAfterCaseChange()
{
    QString newWellName;

    if ( m_case )
    {
        const std::set<QString> sortedWellNameSet = m_case->eclipseCaseData()->findSortedWellNames();
        if ( sortedWellNameSet.size() > 0 )
        {
            newWellName = *sortedWellNameSet.begin();
        }
    }

    m_wellName = newWellName;
}
