/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "RiuPvtPlotPanel.h"

#include "RiuPvtPlotUpdater.h"
#include "RiuPvtPlotWidget.h"

#include "RigFlowDiagSolverInterface.h"

#include "cvfMath.h"

#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

//==================================================================================================
///
/// \class RiuPvtPlotPanel
///
///
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPvtPlotPanel::RiuPvtPlotPanel( QWidget* parent )
    : QWidget( parent )
    , m_unitSystem( RiaDefines::EclipseUnitSystem::UNITS_UNKNOWN )
    , m_plotUpdater( new RiuPvtPlotUpdater( this ) )
{
    m_phaseComboBox = new QComboBox( this );
    m_phaseComboBox->setEditable( false );
    m_phaseComboBox->addItem( "Oil", QVariant( RigFlowDiagDefines::PvtCurve::OIL ) );
    m_phaseComboBox->addItem( "Gas", QVariant( RigFlowDiagDefines::PvtCurve::GAS ) );

    m_titleLabel = new QLabel( "", this );
    m_titleLabel->setAlignment( Qt::AlignHCenter );
    QFont font = m_titleLabel->font();
    font.setPointSize( 10 );
    font.setBold( true );
    m_titleLabel->setFont( font );

    QHBoxLayout* topLayout = new QHBoxLayout();
    topLayout->addWidget( new QLabel( "Phase:" ) );
    topLayout->addWidget( m_phaseComboBox );
    topLayout->addWidget( m_titleLabel, 1 );
    topLayout->setContentsMargins( 5, 5, 0, 0 );

    m_fvfPlot       = new RiuPvtPlotWidget( this );
    m_viscosityPlot = new RiuPvtPlotWidget( this );

    QHBoxLayout* plotLayout = new QHBoxLayout();
    plotLayout->addWidget( m_fvfPlot );
    plotLayout->addWidget( m_viscosityPlot );
    plotLayout->setSpacing( 0 );
    plotLayout->setContentsMargins( 0, 0, 0, 0 );

    QVBoxLayout* mainLayout = new QVBoxLayout();
    mainLayout->addLayout( topLayout );
    mainLayout->addLayout( plotLayout );
    mainLayout->setContentsMargins( 0, 0, 0, 0 );

    setLayout( mainLayout );

    connect( m_phaseComboBox, SIGNAL( currentIndexChanged( int ) ), SLOT( slotPhaseComboCurrentIndexChanged( int ) ) );

    plotUiSelectedCurves();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPvtPlotPanel::~RiuPvtPlotPanel()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPvtPlotPanel::setPlotData( RiaDefines::EclipseUnitSystem                    unitSystem,
                                   const std::vector<RigFlowDiagDefines::PvtCurve>& fvfCurveArr,
                                   const std::vector<RigFlowDiagDefines::PvtCurve>& viscosityCurveArr,
                                   const FvfDynProps&                               fvfDynProps,
                                   const ViscosityDynProps&                         viscosityDynProps,
                                   const CellValues&                                cellValues,
                                   const QString&                                   cellReferenceText )
{
    // cvf::Trace::show("RiuPvtPlotPanel::setPlotData()");

    m_unitSystem            = unitSystem;
    m_allFvfCurvesArr       = fvfCurveArr;
    m_allViscosityCurvesArr = viscosityCurveArr;
    m_fvfDynProps           = fvfDynProps;
    m_viscosityDynProps     = viscosityDynProps;
    m_cellValues            = cellValues;
    m_cellReferenceText     = cellReferenceText;

    plotUiSelectedCurves();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPvtPlotPanel::clearPlot()
{
    // cvf::Trace::show("RiuPvtPlotPanel::clearPlot()");

    if ( m_allFvfCurvesArr.empty() && m_allViscosityCurvesArr.empty() && m_cellReferenceText.isEmpty() )
    {
        return;
    }

    m_unitSystem = RiaDefines::EclipseUnitSystem::UNITS_UNKNOWN;
    m_allFvfCurvesArr.clear();
    m_allViscosityCurvesArr.clear();
    m_fvfDynProps       = FvfDynProps();
    m_viscosityDynProps = ViscosityDynProps();
    m_cellValues        = CellValues();
    m_cellReferenceText.clear();

    plotUiSelectedCurves();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPvtPlotUpdater* RiuPvtPlotPanel::plotUpdater()
{
    return m_plotUpdater.get();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPvtPlotPanel::applyFontSizes( bool replot )
{
    if ( m_fvfPlot ) m_fvfPlot->applyFontSizes( replot );
    if ( m_viscosityPlot ) m_viscosityPlot->applyFontSizes( replot );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPvtPlotPanel::plotUiSelectedCurves()
{
    // Determine which curves (phase) to actually plot based on selection in GUI
    const int                                 currComboIdx = m_phaseComboBox->currentIndex();
    const RigFlowDiagDefines::PvtCurve::Phase phaseToPlot =
        static_cast<RigFlowDiagDefines::PvtCurve::Phase>( m_phaseComboBox->itemData( currComboIdx ).toInt() );

    QString phaseString = "";
    if ( phaseToPlot == RigFlowDiagDefines::PvtCurve::GAS )
    {
        phaseString = "Gas ";
    }
    else if ( phaseToPlot == RigFlowDiagDefines::PvtCurve::OIL )
    {
        phaseString = "Oil ";
    }

    // FVF plot
    {
        RigFlowDiagDefines::PvtCurve::Ident curveIdentToPlot    = RigFlowDiagDefines::PvtCurve::Unknown;
        double                              pointMarkerFvfValue = HUGE_VAL;
        QString                             pointMarkerLabel    = "";

        if ( phaseToPlot == RigFlowDiagDefines::PvtCurve::GAS )
        {
            curveIdentToPlot    = RigFlowDiagDefines::PvtCurve::Bg;
            pointMarkerFvfValue = m_fvfDynProps.bg;
            pointMarkerLabel    = QString( "%1 (%2)" ).arg( pointMarkerFvfValue ).arg( m_cellValues.pressure );
            if ( m_cellValues.rv != HUGE_VAL )
            {
                pointMarkerLabel += QString( "\nRv = %1" ).arg( m_cellValues.rv );
            }
        }
        else if ( phaseToPlot == RigFlowDiagDefines::PvtCurve::OIL )
        {
            curveIdentToPlot    = RigFlowDiagDefines::PvtCurve::Bo;
            pointMarkerFvfValue = m_fvfDynProps.bo;
            pointMarkerLabel    = QString( "%1 (%2)" ).arg( pointMarkerFvfValue ).arg( m_cellValues.pressure );
            if ( m_cellValues.rs != HUGE_VAL )
            {
                pointMarkerLabel += QString( "\nRs = %1" ).arg( m_cellValues.rs );
            }
        }

        std::vector<RigFlowDiagDefines::PvtCurve> selectedFvfCurves;
        for ( RigFlowDiagDefines::PvtCurve curve : m_allFvfCurvesArr )
        {
            if ( curve.ident == curveIdentToPlot )
            {
                selectedFvfCurves.push_back( curve );
            }
        }

        const QString plotTitle = QString( "%1 Formation Volume Factor" ).arg( phaseString );
        const QString yAxisTitle =
            QString( "%1 Formation Volume Factor [%2]" ).arg( phaseString ).arg( unitLabelFromCurveIdent( m_unitSystem, curveIdentToPlot ) );
        m_fvfPlot->plotCurves( m_unitSystem, selectedFvfCurves, m_cellValues.pressure, pointMarkerFvfValue, pointMarkerLabel, plotTitle, yAxisTitle );
    }

    // Viscosity plot
    {
        RigFlowDiagDefines::PvtCurve::Ident curveIdentToPlot          = RigFlowDiagDefines::PvtCurve::Unknown;
        double                              pointMarkerViscosityValue = HUGE_VAL;
        QString                             pointMarkerLabel          = "";

        if ( phaseToPlot == RigFlowDiagDefines::PvtCurve::GAS )
        {
            curveIdentToPlot          = RigFlowDiagDefines::PvtCurve::Visc_g;
            pointMarkerViscosityValue = m_viscosityDynProps.mu_g;
            pointMarkerLabel          = QString( "%1 (%2)" ).arg( pointMarkerViscosityValue ).arg( m_cellValues.pressure );
            if ( m_cellValues.rv != HUGE_VAL )
            {
                pointMarkerLabel += QString( "\nRv = %1" ).arg( m_cellValues.rv );
            }
        }
        else if ( phaseToPlot == RigFlowDiagDefines::PvtCurve::OIL )
        {
            curveIdentToPlot          = RigFlowDiagDefines::PvtCurve::Visc_o;
            pointMarkerViscosityValue = m_viscosityDynProps.mu_o;
            pointMarkerLabel          = QString( "%1 (%2)" ).arg( pointMarkerViscosityValue ).arg( m_cellValues.pressure );
            if ( m_cellValues.rs != HUGE_VAL )
            {
                pointMarkerLabel += QString( "\nRs = %1" ).arg( m_cellValues.rs );
            }
        }

        std::vector<RigFlowDiagDefines::PvtCurve> selectedViscosityCurves;
        for ( RigFlowDiagDefines::PvtCurve curve : m_allViscosityCurvesArr )
        {
            if ( curve.ident == curveIdentToPlot )
            {
                selectedViscosityCurves.push_back( curve );
            }
        }

        const QString plotTitle = QString( "%1 Viscosity" ).arg( phaseString );
        const QString yAxisTitle =
            QString( "%1 Viscosity [%2]" ).arg( phaseString ).arg( unitLabelFromCurveIdent( m_unitSystem, curveIdentToPlot ) );
        m_viscosityPlot->plotCurves( m_unitSystem,
                                     selectedViscosityCurves,
                                     m_cellValues.pressure,
                                     pointMarkerViscosityValue,
                                     pointMarkerLabel,
                                     plotTitle,
                                     yAxisTitle );
    }

    // Update the label on top in our panel
    QString titleStr = "PVT";
    if ( !m_cellReferenceText.isEmpty() )
    {
        titleStr += ", " + m_cellReferenceText;
    }

    m_titleLabel->setText( titleStr );
}

//--------------------------------------------------------------------------------------------------
/// Static helper to get unit labels
//--------------------------------------------------------------------------------------------------
QString RiuPvtPlotPanel::unitLabelFromCurveIdent( RiaDefines::EclipseUnitSystem unitSystem, RigFlowDiagDefines::PvtCurve::Ident curveIdent )
{
    if ( curveIdent == RigFlowDiagDefines::PvtCurve::Bo )
    {
        switch ( unitSystem )
        {
            case RiaDefines::EclipseUnitSystem::UNITS_METRIC:
                return "rm3/sm3";
            case RiaDefines::EclipseUnitSystem::UNITS_FIELD:
                return "rb/stb";
            case RiaDefines::EclipseUnitSystem::UNITS_LAB:
                return "rcc/scc";
            default:
                return "";
        }
    }
    else if ( curveIdent == RigFlowDiagDefines::PvtCurve::Bg )
    {
        switch ( unitSystem )
        {
            case RiaDefines::EclipseUnitSystem::UNITS_METRIC:
                return "rm3/sm3";
            case RiaDefines::EclipseUnitSystem::UNITS_FIELD:
                return "rb/Mscf";
            case RiaDefines::EclipseUnitSystem::UNITS_LAB:
                return "rcc/scc";
            default:
                return "";
        }
    }
    else if ( curveIdent == RigFlowDiagDefines::PvtCurve::Visc_o || curveIdent == RigFlowDiagDefines::PvtCurve::Visc_g )
    {
        switch ( unitSystem )
        {
            case RiaDefines::EclipseUnitSystem::UNITS_METRIC:
                return "cP";
            case RiaDefines::EclipseUnitSystem::UNITS_FIELD:
                return "cP";
            case RiaDefines::EclipseUnitSystem::UNITS_LAB:
                return "cP";
            default:
                return "";
        }
    }

    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPvtPlotPanel::slotPhaseComboCurrentIndexChanged( int )
{
    plotUiSelectedCurves();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPvtPlotPanel::showEvent( QShowEvent* event )
{
    if ( m_plotUpdater != nullptr ) m_plotUpdater->doDelayedUpdate();
    QWidget::showEvent( event );
}
