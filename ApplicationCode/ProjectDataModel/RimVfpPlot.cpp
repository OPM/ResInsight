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

#include "opm/parser/eclipse/EclipseState/Schedule/VFPInjTable.hpp"

//==================================================================================================
//
//
//
//==================================================================================================

CAF_PDM_SOURCE_INIT( RimVfpPlot, "VfpPlot" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimVfpPlot::RimVfpPlot()
{
    // TODO: add icon
    CAF_PDM_InitObject( "VFP Plot", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_case, "Case", "Case", "", "", "" );
    CAF_PDM_InitField( &m_wellName, "WellName", QString( "None" ), "Well", "", "", "" );

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
    if ( m_case && m_case->ensureReservoirCaseIsOpen() )
    {
        // TODO: extract data from data file
        //

        std::set<std::string> wells            = {"F2H", "C1H", "C2H", "C3H", "C4AH", "C4H", "F1H", "F3H", "F4H"};
        std::string           strippedWellName = QString( m_wellName() ).remove( "-" ).toStdString();

        if ( wells.find( strippedWellName ) != wells.end() )
        {
            QString gridFileName = m_case->gridFileName();
            std::cout << "Grid file name: " << gridFileName.toStdString() << std::endl;
            QFileInfo fi( gridFileName );

            std::string filename = fi.canonicalPath().toStdString() + "/INCLUDE/VFP/" + strippedWellName + ".Ecl";
            const std::vector<Opm::VFPInjTable> tables = RimVfpTableExtractor::extractVfpInjectionTables( filename );

            // TODO: populate with real data
            populatePlotWidgetWithCurveData( m_plotWidget, tables );

            // TODO: Maybe display the phase?
            // if ( m_phase == RiaDefines::OIL_PHASE )
            //     phaseString = "Oil";
            // else if ( m_phase == RiaDefines::GAS_PHASE )
            //     phaseString = "Gas";
            // else if ( m_phase == RiaDefines::WATER_PHASE )
            //     phaseString = "Water";
        }
    }

    const QString plotTitleStr = QString( "%1 Vertical Flow Performance Plot" ).arg( m_wellName );
    m_plotWidget->setTitle( plotTitleStr );

    m_plotWidget->setAxisTitleText( QwtPlot::xBottom, "Liquid Flow Rate [sm3/d]" );
    m_plotWidget->setAxisTitleText( QwtPlot::yLeft, "Bottom Hole Pressure [Bar]" );
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
            // curve->setBrush( qtClr );
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

    RimPlot::defineUiOrdering( uiConfigName, uiOrdering );
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
