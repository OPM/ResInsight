/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017  Statoil ASA
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

#include "RimWellLogRftCurve.h"

#include "RiaColorTables.h"
#include "RiaColorTools.h"
#include "RiaDefines.h"
#include "RiaEclipseUnitTools.h"
#include "RiaExtractionTools.h"
#include "RiaQDateTimeTools.h"
#include "RiaResultNames.h"
#include "RiaRftDefines.h"
#include "RiaSimWellBranchTools.h"
#include "RiaStatisticsTools.h"
#include "RiaSummaryTools.h"
#include "RiaTextStringTools.h"

#include "RifEclipseRftAddress.h"
#include "RifReaderEclipseRft.h"
#include "RifReaderOpmRft.h"

#include "RigEclipseCaseData.h"
#include "RigEclipseWellLogExtractor.h"
#include "RigMainGrid.h"
#include "RigWellLogCurveData.h"
#include "RigWellPath.h"
#include "RigWellPathGeometryTools.h"
#include "RigWellPathIntersectionTools.h"

#include "RimDepthTrackPlot.h"
#include "RimEclipseResultCase.h"
#include "RimFileSummaryCase.h"
#include "RimObservedFmuRftData.h"
#include "RimPressureDepthData.h"
#include "RimProject.h"
#include "RimRftTools.h"
#include "RimSummaryCase.h"
#include "RimSummaryEnsemble.h"
#include "RimTools.h"
#include "RimWellLogPlot.h"
#include "RimWellLogTrack.h"
#include "RimWellPath.h"
#include "RimWellPlotTools.h"
#include "RimWellRftPlot.h"

#include "RiuQwtPlotCurve.h"
#include "RiuQwtPlotWidget.h"

#include "cafPdmObject.h"
#include "cafVecIjk.h"
#include "cvfAssert.h"

#include <qwt_plot.h>

#include <QString>

#include <numeric>
#include <vector>

namespace caf
{
template <>
void caf::AppEnum<RifEclipseRftAddress::RftWellLogChannelType>::setUp()
{
    addItem( RifEclipseRftAddress::RftWellLogChannelType::NONE, "NONE", "None" );
    addItem( RifEclipseRftAddress::RftWellLogChannelType::TVD, "DEPTH", "Depth" );
    addItem( RifEclipseRftAddress::RftWellLogChannelType::PRESSURE, "PRESSURE", "Pressure" );
    addItem( RifEclipseRftAddress::RftWellLogChannelType::SWAT, RiaResultNames::swat(), "Water Saturation" );
    addItem( RifEclipseRftAddress::RftWellLogChannelType::SOIL, RiaResultNames::soil(), "Oil Saturation" );
    addItem( RifEclipseRftAddress::RftWellLogChannelType::SGAS, RiaResultNames::sgas(), "Gas Saturation" );
    addItem( RifEclipseRftAddress::RftWellLogChannelType::WRAT, "WRAT", "Water Flow" );
    addItem( RifEclipseRftAddress::RftWellLogChannelType::ORAT, "ORAT", "Oil Flow" );
    addItem( RifEclipseRftAddress::RftWellLogChannelType::GRAT, "GRAT", "Gas flow" );
    addItem( RifEclipseRftAddress::RftWellLogChannelType::MD, "MD", "Measured Depth" );
    addItem( RifEclipseRftAddress::RftWellLogChannelType::PRESSURE_P10, "PRESSURE_P10", "P10: Pressure" );
    addItem( RifEclipseRftAddress::RftWellLogChannelType::PRESSURE_P50, "PRESSURE_P50", "P50: Pressure" );
    addItem( RifEclipseRftAddress::RftWellLogChannelType::PRESSURE_P90, "PRESSURE_P90", "P90: Pressure" );
    addItem( RifEclipseRftAddress::RftWellLogChannelType::PRESSURE_MEAN, "PRESSURE_MEAN", "Mean: Pressure" );
    addItem( RifEclipseRftAddress::RftWellLogChannelType::PRESSURE_ERROR, "PRESSURE_ERROR", "Error: Pressure" );
    setDefault( RifEclipseRftAddress::RftWellLogChannelType::NONE );
}
} // namespace caf

namespace caf
{
template <>
void caf::AppEnum<RimWellLogRftCurve::RftDataType>::setUp()
{
    addItem( RimWellLogRftCurve::RftDataType::RFT_DATA, "RFT_DATA", "RFT" );
    addItem( RimWellLogRftCurve::RftDataType::RFT_SEGMENT_DATA, "RFT_SEGMENT_DATA", "RFT Segment" );
    setDefault( RimWellLogRftCurve::RftDataType::RFT_DATA );
}
} // namespace caf

CAF_PDM_SOURCE_INIT( RimWellLogRftCurve, "WellLogRftCurve" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::PhaseType RimWellLogRftCurve::phaseType() const
{
    if ( m_rftDataType() == RimWellLogRftCurve::RftDataType::RFT_DATA )
    {
        if ( m_wellLogChannelName() == RifEclipseRftAddress::RftWellLogChannelType::SWAT ||
             m_wellLogChannelName() == RifEclipseRftAddress::RftWellLogChannelType::WRAT )
        {
            return RiaDefines::PhaseType::WATER_PHASE;
        }

        if ( m_wellLogChannelName() == RifEclipseRftAddress::RftWellLogChannelType::SGAS ||
             m_wellLogChannelName() == RifEclipseRftAddress::RftWellLogChannelType::GRAT )
        {
            return RiaDefines::PhaseType::GAS_PHASE;
        }

        if ( m_wellLogChannelName() == RifEclipseRftAddress::RftWellLogChannelType::SOIL ||
             m_wellLogChannelName() == RifEclipseRftAddress::RftWellLogChannelType::ORAT )
        {
            return RiaDefines::PhaseType::OIL_PHASE;
        }
    }
    else if ( m_rftDataType() == RimWellLogRftCurve::RftDataType::RFT_SEGMENT_DATA )
    {
        if ( m_segmentResultName().startsWith( "SEGO" ) ) return RiaDefines::PhaseType::OIL_PHASE;
        if ( m_segmentResultName().startsWith( "SEGW" ) ) return RiaDefines::PhaseType::WATER_PHASE;
        if ( m_segmentResultName().startsWith( "SEGG" ) ) return RiaDefines::PhaseType::GAS_PHASE;
    }

    return RiaDefines::PhaseType::PHASE_NOT_APPLICABLE;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellLogRftCurve::RimWellLogRftCurve()
{
    CAF_PDM_InitObject( "Well Log RFT Curve", RimWellLogCurve::wellLogCurveIconName() );

    CAF_PDM_InitFieldNoDefault( &m_eclipseCase, "CurveEclipseResultCase", "Eclipse Result Case" );
    m_eclipseCase.uiCapability()->setUiTreeChildrenHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_summaryCase, "CurveSummaryCase", "Summary Case" );
    m_summaryCase.uiCapability()->setUiTreeChildrenHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_ensemble, "CurveEnsemble", "Ensemble" );
    m_ensemble.uiCapability()->setUiTreeChildrenHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_observedFmuRftData, "ObservedFmuRftData", "Observed FMU RFT Data" );
    m_observedFmuRftData.uiCapability()->setUiTreeChildrenHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_pressureDepthData, "PressureDepthData", "Pressure Depth Data" );
    m_pressureDepthData.uiCapability()->setUiTreeChildrenHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_timeStep, "TimeStep", "Time Step" );

    CAF_PDM_InitFieldNoDefault( &m_wellName, "WellName", "Well Name" );
    CAF_PDM_InitField( &m_branchIndex, "BranchIndex", 0, "Branch Index" );
    CAF_PDM_InitField( &m_branchDetection,
                       "BranchDetection",
                       true,
                       "Branch Detection",
                       "",
                       "Compute branches based on how simulation well cells are organized",
                       "" );

    CAF_PDM_InitFieldNoDefault( &m_wellLogChannelName, "WellLogChannelName", "Well Property" );
    CAF_PDM_InitFieldNoDefault( &m_rftDataType, "RftDataType", "Data Type" );

    CAF_PDM_InitField( &m_segmentResultName, "SegmentResultName", RiaResultNames::undefinedResultName(), "Result Name" );
    CAF_PDM_InitField( &m_segmentBranchIndex, "SegmentBranchIndex", -1, "Branch" );
    CAF_PDM_InitFieldNoDefault( &m_segmentBranchType, "SegmentBranchType", "Completion" );

    CAF_PDM_InitField( &m_scaleFactor, "ScaleFactor", 1.0, "Scale Factor" );

    CAF_PDM_InitField( &m_curveColorByPhase, "CurveColorByPhase", false, "Color by Phase" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellLogRftCurve::~RimWellLogRftCurve()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogRftCurve::setWellName( const QString& wellName )
{
    m_wellName = wellName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellLogRftCurve::wellName() const
{
    return m_wellName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellLogRftCurve::wellLogChannelUiName() const
{
    return m_wellLogChannelName().text();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellLogRftCurve::wellLogChannelUnits() const
{
    return RiaWellLogUnitTools<double>::noUnitString();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogRftCurve::setTimeStep( const QDateTime& dateTime )
{
    m_timeStep = dateTime;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QDateTime RimWellLogRftCurve::timeStep() const
{
    return m_timeStep();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogRftCurve::setSegmentBranchIndex( int branchIndex )
{
    m_segmentBranchIndex = branchIndex;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogRftCurve::setSegmentBranchType( RiaDefines::RftBranchType branchType )
{
    m_segmentBranchType = branchType;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogRftCurve::setEclipseCase( RimEclipseCase* eclipseCase )
{
    m_eclipseCase = eclipseCase;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseCase* RimWellLogRftCurve::eclipseCase() const
{
    return m_eclipseCase;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogRftCurve::setSummaryCase( RimSummaryCase* summaryCase )
{
    m_summaryCase = summaryCase;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCase* RimWellLogRftCurve::summaryCase() const
{
    return m_summaryCase;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogRftCurve::setEnsemble( RimSummaryEnsemble* ensemble )
{
    m_ensemble = ensemble;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryEnsemble* RimWellLogRftCurve::ensemble() const
{
    return m_ensemble;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogRftCurve::setObservedFmuRftData( RimObservedFmuRftData* observedFmuRftData )
{
    m_observedFmuRftData = observedFmuRftData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimObservedFmuRftData* RimWellLogRftCurve::observedFmuRftData() const
{
    return m_observedFmuRftData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogRftCurve::setPressureDepthData( RimPressureDepthData* observedFmuRftData )
{
    m_pressureDepthData = observedFmuRftData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPressureDepthData* RimWellLogRftCurve::pressureDepthData() const
{
    return m_pressureDepthData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogRftCurve::setRftAddress( RifEclipseRftAddress address )
{
    m_timeStep           = address.timeStep();
    m_wellName           = address.wellName();
    m_wellLogChannelName = address.wellLogChannel();

    if ( address.wellLogChannel() == RifEclipseRftAddress::RftWellLogChannelType::SEGMENT_VALUES )
    {
        m_rftDataType        = RftDataType::RFT_SEGMENT_DATA;
        m_segmentResultName  = address.segmentResultName();
        m_segmentBranchIndex = address.segmentBranchIndex();
        m_segmentBranchType  = address.segmentBranchType();
    }
    else
    {
        m_rftDataType = RftDataType::RFT_DATA;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseRftAddress RimWellLogRftCurve::rftAddress() const
{
    if ( m_rftDataType == RftDataType::RFT_SEGMENT_DATA )
    {
        return RifEclipseRftAddress::createBranchSegmentAddress( m_wellName,
                                                                 m_timeStep,
                                                                 m_segmentResultName(),
                                                                 m_segmentBranchIndex(),
                                                                 m_segmentBranchType() );
    }

    return RifEclipseRftAddress::createAddress( m_wellName, m_timeStep, m_wellLogChannelName() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogRftCurve::setDefaultAddress( QString wellName )
{
    RifReaderRftInterface* reader = rftReader();
    if ( !reader ) return;

    bool              wellNameHasRftData = false;
    std::set<QString> wellNames          = reader->wellNames();
    for ( const QString& wellNameWithRft : wellNames )
    {
        if ( wellName == wellNameWithRft )
        {
            wellNameHasRftData = true;
            m_wellName         = wellName;
            break;
        }
    }

    if ( !wellNameHasRftData )
    {
        m_wellLogChannelName = RifEclipseRftAddress::RftWellLogChannelType::NONE;
        m_timeStep           = QDateTime();
        return;
    }

    m_wellLogChannelName = RifEclipseRftAddress::RftWellLogChannelType::PRESSURE;

    std::set<QDateTime> timeSteps = reader->availableTimeSteps( m_wellName, m_wellLogChannelName() );
    if ( !timeSteps.empty() )
    {
        m_timeStep = *( timeSteps.begin() );
    }
    else
    {
        m_timeStep = QDateTime();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogRftCurve::updateWellChannelNameAndTimeStep()
{
    if ( !m_timeStep().isValid() || m_wellLogChannelName() == RifEclipseRftAddress::RftWellLogChannelType::NONE )
    {
        setDefaultAddress( m_wellName );
        return;
    }

    RifReaderRftInterface* reader = rftReader();
    if ( !reader ) return;

    std::set<RifEclipseRftAddress::RftWellLogChannelType> channelNames = reader->availableWellLogChannels( m_wellName );

    if ( channelNames.empty() )
    {
        m_wellLogChannelName = RifEclipseRftAddress::RftWellLogChannelType::NONE;
    }
    else if ( !channelNames.count( m_wellLogChannelName() ) )
    {
        m_wellLogChannelName = RifEclipseRftAddress::RftWellLogChannelType::PRESSURE;
    }

    std::set<QDateTime> timeSteps = reader->availableTimeSteps( m_wellName, m_wellLogChannelName() );

    if ( timeSteps.empty() )
    {
        m_timeStep = QDateTime();
    }
    else if ( !timeSteps.count( m_timeStep() ) )
    {
        m_timeStep = *( timeSteps.begin() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogRftCurve::setSimWellBranchData( bool branchDetection, int branchIndex )
{
    m_branchDetection = branchDetection;
    m_branchIndex     = branchIndex;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogRftCurve::enableColorFromResultName( bool enable )
{
    m_curveColorByPhase = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogRftCurve::assignColorFromResultName( const QString& resultName )
{
    cvf::Color3f color = cvf::Color3f::BLACK;

    if ( resultName.startsWith( "SEGO" ) || resultName.startsWith( "CONO" ) || resultName.startsWith( "SOIL" ) )
    {
        color = RiaColorTables::summaryCurveGreenPaletteColors().cycledColor3f( 0 );
    }
    else if ( resultName.startsWith( "SEGW" ) || resultName.startsWith( "CONW" ) || resultName.startsWith( "SWAT" ) )
    {
        color = RiaColorTables::summaryCurveBluePaletteColors().cycledColor3f( 0 );
    }
    else if ( resultName.startsWith( "SEGG" ) || resultName.startsWith( "CONG" ) || resultName.startsWith( "SGAS" ) )
    {
        color = RiaColorTables::summaryCurveRedPaletteColors().cycledColor3f( 0 );
    }

    // Do nothing if not phase is identified
    if ( color == cvf::Color3f::BLACK ) return;

    float scalingFactor = 0.5;
    auto  fillColor     = RiaColorTools::makeLighter( color, scalingFactor );
    setColor( color );
    setFillColor( fillColor );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogRftCurve::setScaleFactor( double factor )
{
    m_scaleFactor = factor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<QString, QString> RimWellLogRftCurve::createCurveNameKeyValueMap() const
{
    std::map<QString, QString> variableValueMap;

    if ( !wellName().isEmpty() )
    {
        variableValueMap[RiaDefines::namingVariableWell()] = wellName();
    }

    variableValueMap[RiaDefines::namingVariableResultType()] = "RFT";

    QString caseText;
    if ( m_eclipseCase )
    {
        caseText = m_eclipseCase->caseUserDescription();
    }
    else if ( m_summaryCase && m_ensemble ) // Summary RFT curves have both ensemble and summary set
    {
        caseText = QString( "%1, %2" ).arg( m_ensemble->name() ).arg( m_summaryCase->displayCaseName() );
    }
    else if ( m_ensemble )
    {
        caseText = m_ensemble->name();
    }
    else if ( m_summaryCase )
    {
        caseText = m_summaryCase->displayCaseName();
    }
    else if ( m_observedFmuRftData )
    {
        caseText = m_observedFmuRftData->name();
    }

    if ( !caseText.isEmpty() )
    {
        variableValueMap[RiaDefines::namingVariableCase()] = caseText;
    }

    if ( m_rftDataType() == RftDataType::RFT_DATA )
    {
        if ( wellLogChannelUiName() !=
             caf::AppEnum<RifEclipseRftAddress::RftWellLogChannelType>::text( RifEclipseRftAddress::RftWellLogChannelType::NONE ) )
        {
            RifEclipseRftAddress::RftWellLogChannelType channelNameEnum =
                caf::AppEnum<RifEclipseRftAddress::RftWellLogChannelType>::fromText( wellLogChannelUiName() );
            QString channelName = caf::AppEnum<RifEclipseRftAddress::RftWellLogChannelType>::uiText( channelNameEnum );

            variableValueMap[RiaDefines::namingVariableResultName()] = channelName;
        }
    }
    else if ( m_rftDataType() == RftDataType::RFT_SEGMENT_DATA )
    {
        variableValueMap[RiaDefines::namingVariableResultName()] = m_segmentResultName;

        QString branchText = QString( "Branch %1" ).arg( m_segmentBranchIndex() );

        variableValueMap[RiaDefines::namingVariableWellBranch()] = branchText;

        if ( RiaDefines::isSegmentResult( m_segmentResultName() ) )
        {
            variableValueMap[RiaDefines::namingVariableResultType()] = m_segmentBranchType().uiText();
        }
        else
        {
            variableValueMap[RiaDefines::namingVariableResultType()] = "Reservoir";
        }
    }

    if ( !m_timeStep().isNull() )
    {
        variableValueMap[RiaDefines::namingVariableTime()] = m_timeStep().toString( RiaQDateTimeTools::dateFormatString() );
    }

    return variableValueMap;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellLogRftCurve::createCurveAutoName()
{
    QStringList curveNameSubStrings;

    for ( const auto& [key, value] : createCurveNameKeyValueMap() )
    {
        curveNameSubStrings.push_back( value );
    }

    return curveNameSubStrings.join( ", " );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellLogRftCurve::createCurveNameFromTemplate( const QString& templateText )
{
    auto name = RiaTextStringTools::replaceTemplateTextWithValues( templateText, createCurveNameKeyValueMap() );

    if ( m_scaleFactor() != 1.0 )
    {
        int  exponent = std::log10( m_scaleFactor() );
        auto text     = QString( "x1e%1" ).arg( QString::number( exponent ) );

        name += QString( " [%1]" ).arg( text );
    }

    return name;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RimWellLogRftCurve::supportedCurveNameVariables() const
{
    return { RiaDefines::namingVariableWell(),
             RiaDefines::namingVariableResultName(),
             RiaDefines::namingVariableResultType(),
             RiaDefines::namingVariableCase(),
             RiaDefines::namingVariableWellBranch(),
             RiaDefines::namingVariableTime() };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogRftCurve::onLoadDataAndUpdate( bool updateParentPlot )
{
    if ( m_curveColorByPhase && m_rftDataType() == RimWellLogRftCurve::RftDataType::RFT_SEGMENT_DATA )
    {
        assignColorFromResultName( m_segmentResultName );
    }

    RimPlotCurve::updateCurvePresentation( updateParentPlot );

    QString axisPrefixText;

    if ( m_autoCheckStateBasedOnCurveData() || isChecked() )
    {
        auto wellLogPlot = firstAncestorOrThisOfTypeAsserted<RimDepthTrackPlot>();

        auto* rftPlot                     = dynamic_cast<RimWellRftPlot*>( wellLogPlot );
        bool  showErrorBarsInObservedData = rftPlot ? rftPlot->showErrorBarsForObservedData() : false;
        m_showErrorBars                   = showErrorBarsInObservedData;

        std::vector<double>  measuredDepthVector = measuredDepthValues( axisPrefixText );
        std::vector<double>  tvDepthVector       = tvDepthValues();
        std::vector<double>  values              = xValues();
        std::vector<double>  errors              = errorValues();
        std::vector<QString> perPointLabels;

        auto anyValidValuesPresent = []( const std::vector<double>& values ) -> bool
        {
            for ( const auto& v : values )
            {
                if ( RiaStatisticsTools::isValidNumber<double>( v ) ) return true;
            }
            return false;
        };

        if ( !anyValidValuesPresent( values ) || ( values.size() != tvDepthVector.size() ) )
        {
            clearCurveData();
            detach( true );
            return;
        }

        RiaDefines::EclipseUnitSystem unitSystem = RiaDefines::EclipseUnitSystem::UNITS_METRIC;
        if ( m_eclipseCase )
        {
            // TODO: If no grid data, but only RFT data is loaded, we do not have any way to
            // detect unit
            if ( m_eclipseCase->eclipseCaseData() )
            {
                unitSystem = m_eclipseCase->eclipseCaseData()->unitsType();
            }
        }
        else if ( m_summaryCase )
        {
            unitSystem = m_summaryCase->unitsSystem();
        }
        else if ( m_ensemble )
        {
            unitSystem = m_ensemble->unitSystem();
        }
        else if ( m_observedFmuRftData )
        {
            // TODO: Read unit system somewhere for FMU RFT Data
            unitSystem     = RiaDefines::EclipseUnitSystem::UNITS_METRIC;
            perPointLabels = this->perPointLabels();
        }
        else if ( m_pressureDepthData )
        {
            // TODO: Read unit system for pressure data
            unitSystem = RiaDefines::EclipseUnitSystem::UNITS_METRIC;
            // perPointLabels = this->perPointLabels();
        }
        else
        {
            CVF_ASSERT( false && "Need to have either an eclipse result case, a summary case or an ensemble" );
        }

        if ( tvDepthVector.size() != measuredDepthVector.size() )
        {
            if ( deriveMeasuredDepthFromObservedData( tvDepthVector, measuredDepthVector ) )
            {
                axisPrefixText = "OBS/";
            }
        }

        if ( tvDepthVector.size() != measuredDepthVector.size() )
        {
            measuredDepthVector = tvDepthVector;
        }

        RimProject*  proj     = RimProject::current();
        RimWellPath* wellPath = proj->wellPathByName( m_wellName );

        double rkbDiff = 0.0;
        if ( wellPath && wellPath->wellPathGeometry() )
        {
            rkbDiff = wellPath->wellPathGeometry()->rkbDiff();
        }

        bool useLogarithmicScale = false;
        setPropertyValuesWithMdAndTVD( values,
                                       measuredDepthVector,
                                       tvDepthVector,
                                       rkbDiff,
                                       RiaDefines::fromEclipseUnit( unitSystem ),
                                       false,
                                       useLogarithmicScale );

        RiaDefines::DepthUnitType displayUnit = RiaDefines::DepthUnitType::UNIT_METER;
        if ( wellLogPlot )
        {
            displayUnit = wellLogPlot->depthUnit();
        }

        if ( m_plotCurve )
        {
            if ( wellLogPlot->depthType() == RiaDefines::DepthTypeEnum::MEASURED_DEPTH )
            {
                m_plotCurve->setPerPointLabels( perPointLabels );

                auto propertyValues = curveData()->propertyValuesByIntervals();
                auto depthValues    = curveData()->depthValuesByIntervals( RiaDefines::DepthTypeEnum::MEASURED_DEPTH, displayUnit );

                if ( !errors.empty() )
                {
                    setPropertyAndDepthsAndErrors( propertyValues, depthValues, errors );
                }
                else
                {
                    setPropertyAndDepthValuesToPlotCurve( propertyValues, depthValues );
                }

                m_plotCurve->setLineSegmentStartStopIndices( curveData()->polylineStartStopIndices() );

                auto wellLogTrack = firstAncestorOrThisOfTypeAsserted<RimWellLogTrack>();

                RiuQwtPlotWidget* viewer = wellLogTrack->viewer();
                if ( viewer )
                {
                    QString text = axisPrefixText + wellLogPlot->depthAxisTitle();

                    viewer->setAxisTitleText( wellLogPlot->depthAxis(), text );
                }
            }
            else
            {
                m_plotCurve->setPerPointLabels( perPointLabels );

                auto propertyValues = curveData()->propertyValuesByIntervals();
                auto depthValues    = curveData()->depthValuesByIntervals( RiaDefines::DepthTypeEnum::TRUE_VERTICAL_DEPTH, displayUnit );
                bool useLogarithmicScale = false;

                if ( !errors.empty() )
                {
                    setPropertyAndDepthsAndErrors( propertyValues, depthValues, errors );
                }
                else
                {
                    if ( isVerticalCurve() )
                    {
                        m_plotCurve->setSamplesFromXValuesAndYValues( propertyValues, depthValues, useLogarithmicScale );
                    }
                    else
                    {
                        m_plotCurve->setSamplesFromXValuesAndYValues( depthValues, propertyValues, useLogarithmicScale );
                    }
                }
            }

            m_plotCurve->setLineSegmentStartStopIndices( curveData()->polylineStartStopIndices() );
        }

        if ( updateParentPlot )
        {
            updateZoomInParentPlot();
        }

        if ( m_parentPlot )
        {
            m_parentPlot->replot();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogRftCurve::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    RimPlotCurve::updateFieldUiState();

    caf::PdmUiGroup* curveDataGroup = uiOrdering.addNewGroup( "Curve Data" );
    curveDataGroup->add( &m_eclipseCase );
    curveDataGroup->add( &m_summaryCase );
    curveDataGroup->add( &m_wellName );
    curveDataGroup->add( &m_timeStep );
    curveDataGroup->add( &m_rftDataType );
    curveDataGroup->add( &m_scaleFactor );

    caf::PdmUiGroup* automationGroup = uiOrdering.addNewGroup( "Automation" );
    automationGroup->setCollapsedByDefault();
    automationGroup->add( &m_autoCheckStateBasedOnCurveData );

    if ( m_rftDataType() == RimWellLogRftCurve::RftDataType::RFT_DATA )
    {
        curveDataGroup->add( &m_wellLogChannelName );

        RiaSimWellBranchTools::appendSimWellBranchFieldsIfRequiredFromWellName( curveDataGroup, m_wellName, m_branchDetection, m_branchIndex );
    }
    else
    {
        curveDataGroup->add( &m_segmentResultName );
        curveDataGroup->add( &m_segmentBranchType );
        curveDataGroup->add( &m_segmentBranchIndex );
        curveDataGroup->add( &m_curveColorByPhase );
    }

    RimStackablePlotCurve::defaultUiOrdering( uiOrdering );

    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimWellLogRftCurve::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    options = RimWellLogCurve::calculateValueOptions( fieldNeedingOptions );

    if ( !options.empty() ) return options;

    RifReaderRftInterface* reader = rftReader();
    if ( fieldNeedingOptions == &m_eclipseCase )
    {
        RimTools::caseOptionItems( &options );

        options.push_front( caf::PdmOptionItemInfo( "None", nullptr ) );
    }
    else if ( fieldNeedingOptions == &m_summaryCase )
    {
        options = RiaSummaryTools::optionsForSummaryCases( RimProject::current()->allSummaryCases() );
        options.push_front( caf::PdmOptionItemInfo( "None", nullptr ) );
    }
    else if ( fieldNeedingOptions == &m_wellName )
    {
        options = RimRftTools::wellNameOptions( reader );
    }
    else if ( fieldNeedingOptions == &m_wellLogChannelName )
    {
        options = RimRftTools::wellLogChannelsOptions( reader, m_wellName() );
    }
    else if ( fieldNeedingOptions == &m_timeStep )
    {
        if ( m_rftDataType == RimWellLogRftCurve::RftDataType::RFT_SEGMENT_DATA )
            options = RimRftTools::segmentTimeStepOptions( reader, m_wellName );
        else
            options = RimRftTools::timeStepOptions( reader, m_wellName, m_wellLogChannelName() );
    }
    else if ( fieldNeedingOptions == &m_branchIndex )
    {
        auto simulationWellBranches =
            RiaSimWellBranchTools::simulationWellBranches( RimWellPlotTools::simWellName( m_wellName ), m_branchDetection );

        options = RiaSimWellBranchTools::valueOptionsForBranchIndexField( simulationWellBranches );
    }
    else if ( fieldNeedingOptions == &m_segmentResultName )
    {
        options = RimRftTools::segmentResultNameOptions( reader, m_wellName(), m_timeStep() );
    }
    else if ( fieldNeedingOptions == &m_segmentBranchIndex )
    {
        options =
            RimRftTools::segmentBranchIndexOptions( dynamic_cast<RifReaderOpmRft*>( reader ), m_wellName(), m_timeStep(), m_segmentBranchType() );
    }
    else if ( fieldNeedingOptions == &m_scaleFactor )
    {
        for ( int exp = -12; exp <= 12; exp += 3 )
        {
            QString uiText = exp == 0 ? "1" : QString( "10 ^ %1" ).arg( exp );
            double  value  = std::pow( 10, exp );

            options.push_back( caf::PdmOptionItemInfo( uiText, value ) );
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogRftCurve::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    bool loadData = false;

    RimWellLogCurve::fieldChangedByUi( changedField, oldValue, newValue );
    if ( changedField == &m_eclipseCase )
    {
        m_timeStep           = QDateTime();
        m_wellName           = "";
        m_wellLogChannelName = RifEclipseRftAddress::RftWellLogChannelType::NONE;

        loadData = true;
    }
    else if ( changedField == &m_wellName )
    {
        m_branchIndex = 0;

        updateWellChannelNameAndTimeStep();
        loadData = true;
    }
    else if ( changedField == &m_branchDetection || changedField == &m_branchIndex )
    {
        QString simWellName = RimWellPlotTools::simWellName( m_wellName );

        m_branchIndex = RiaSimWellBranchTools::clampBranchIndex( simWellName, m_branchIndex, m_branchDetection );

        updateWellChannelNameAndTimeStep();
        loadData = true;
    }
    else if ( changedField == &m_wellLogChannelName )
    {
        if ( m_wellLogChannelName == RifEclipseRftAddress::RftWellLogChannelType::NONE )
        {
            m_timeStep = QDateTime();
        }
        loadData = true;
    }
    else if ( changedField == &m_timeStep || changedField == &m_segmentResultName || changedField == &m_segmentBranchIndex ||
              changedField == &m_rftDataType || changedField == &m_segmentBranchType || m_scaleFactor )
    {
        loadData = true;
    }

    if ( changedField == &m_rftDataType )
    {
        if ( m_rftDataType() == RftDataType::RFT_SEGMENT_DATA )
        {
            auto fileSummaryCase = dynamic_cast<RimFileSummaryCase*>( m_summaryCase() );
            if ( fileSummaryCase ) fileSummaryCase->searchForWseglinkAndRecreateRftReader();
        }
    }

    if ( loadData ) loadDataAndUpdate( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> RimWellLogRftCurve::perPointLabels() const
{
    if ( m_observedFmuRftData() )
    {
        auto address = RifEclipseRftAddress::createAddress( m_wellName(), m_timeStep, RifEclipseRftAddress::RftWellLogChannelType::PRESSURE );
        return m_observedFmuRftData()->labels( address );
    }
    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifReaderRftInterface* RimWellLogRftCurve::rftReader() const
{
    if ( m_summaryCase() ) // Summary RFT curves have both summary and ensemble set. Prioritize summary for reader.
    {
        return m_summaryCase()->rftReader();
    }

    if ( m_ensemble() )
    {
        auto wellLogPlot = firstAncestorOrThisOfType<RimWellRftPlot>();
        auto curveSet    = wellLogPlot->findEnsembleCurveSet( m_ensemble );
        if ( curveSet )
        {
            return curveSet->statisticsEclipseRftReader();
        }

        return nullptr;
    }

    if ( m_observedFmuRftData() )
    {
        return m_observedFmuRftData()->rftReader();
    }

    if ( m_pressureDepthData() )
    {
        return m_pressureDepthData()->rftReader();
    }

    if ( auto resultCase = dynamic_cast<RimEclipseResultCase*>( m_eclipseCase() ) )
    {
        return resultCase->rftReader();
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigEclipseWellLogExtractor* RimWellLogRftCurve::extractor()
{
    RimProject*  proj     = RimProject::current();
    RimWellPath* wellPath = proj->wellPathFromSimWellName( m_wellName() );

    // The well path extractor has the best geometrical representation, so use this if found
    if ( auto wellPathExtractor = RiaExtractionTools::findOrCreateWellLogExtractor( wellPath, m_eclipseCase ) )
    {
        return wellPathExtractor;
    }

    // Use sim well extractor as fallback
    QString simWellName = RimWellPlotTools::simWellName( m_wellName );
    return RiaExtractionTools::findOrCreateSimWellExtractor( m_eclipseCase(), simWellName, m_branchDetection(), m_branchIndex() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimWellLogRftCurve::xValues()
{
    RifReaderRftInterface* reader = rftReader();
    if ( !reader ) return {};

    std::vector<double> values;

    if ( m_rftDataType() == RftDataType::RFT_SEGMENT_DATA )
    {
        auto depthAddress = RifEclipseRftAddress::createBranchSegmentAddress( m_wellName(),
                                                                              m_timeStep,
                                                                              m_segmentResultName(),
                                                                              segmentBranchIndex(),
                                                                              m_segmentBranchType() );

        reader->values( depthAddress, &values );
    }
    else
    {
        auto address = RifEclipseRftAddress::createAddress( m_wellName(), m_timeStep, m_wellLogChannelName() );

        reader->values( address, &values );
    }

    if ( m_scaleFactor() != 1.0 )
    {
        for ( auto& val : values )
        {
            val *= m_scaleFactor();
        }
    }

    return values;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimWellLogRftCurve::errorValues()
{
    RifReaderRftInterface* reader = rftReader();
    std::vector<double>    errorValues;

    if ( reader && m_rftDataType() == RftDataType::RFT_DATA )
    {
        RifEclipseRftAddress errorAddress =
            RifEclipseRftAddress::createAddress( m_wellName(), m_timeStep, RifEclipseRftAddress::RftWellLogChannelType::PRESSURE_ERROR );
        reader->values( errorAddress, &errorValues );
    }

    return errorValues;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimWellLogRftCurve::tvDepthValues()
{
    RifReaderRftInterface* reader = rftReader();
    std::vector<double>    values;

    if ( !reader ) return values;

    RifEclipseRftAddress depthAddress =
        RifEclipseRftAddress::createAddress( m_wellName(), m_timeStep, RifEclipseRftAddress::RftWellLogChannelType::TVD );

    if ( m_rftDataType() == RftDataType::RFT_SEGMENT_DATA )
    {
        if ( RiaDefines::isSegmentConnectionResult( m_segmentResultName ) )

        {
            depthAddress = RifEclipseRftAddress::createBranchSegmentAddress( m_wellName(),
                                                                             m_timeStep,
                                                                             RiaDefines::segmentConnectionTvdDepthResultName(),
                                                                             segmentBranchIndex(),
                                                                             m_segmentBranchType() );
        }
        else
        {
            depthAddress = RifEclipseRftAddress::createBranchSegmentAddress( m_wellName(),
                                                                             m_timeStep,
                                                                             RiaDefines::segmentTvdDepthResultName(),
                                                                             segmentBranchIndex(),
                                                                             m_segmentBranchType() );
        }
    }

    reader->values( depthAddress, &values );

    return values;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimWellLogRftCurve::measuredDepthValues( QString& prefixText )
{
    if ( m_rftDataType() == RftDataType::RFT_SEGMENT_DATA )
    {
        if ( auto opmRftReader = dynamic_cast<RifReaderOpmRft*>( rftReader() ) )
        {
            prefixText = "SEGMENT/";

            if ( RiaDefines::isSegmentConnectionResult( m_segmentResultName() ) )
            {
                return RimRftTools::segmentConnectionMdValues( opmRftReader, m_wellName(), m_timeStep, segmentBranchIndex(), m_segmentBranchType() );
            }

            // Always use segment end MD values for segment data, as the curve is plotted as step left
            return RimRftTools::segmentEndMdValues( opmRftReader, m_wellName(), m_timeStep, segmentBranchIndex(), m_segmentBranchType() );
        }

        return {};
    }

    if ( m_observedFmuRftData && !m_ensemble && !m_summaryCase )
    {
        RifReaderRftInterface* reader = rftReader();
        std::vector<double>    values;

        if ( !reader ) return values;

        RifEclipseRftAddress depthAddress =
            RifEclipseRftAddress::createAddress( m_wellName(), m_timeStep, RifEclipseRftAddress::RftWellLogChannelType::MD );
        reader->values( depthAddress, &values );

        prefixText = "OBS/";
        return values;
    }

    if ( m_pressureDepthData && !m_ensemble && !m_summaryCase )
    {
        // Pressure depth data does not have MD
        return {};
    }

    RigEclipseWellLogExtractor* eclExtractor = extractor();
    if ( !eclExtractor ) return {};

    if ( auto reader = rftReader() )
    {
        prefixText = "WELL/";

        RifEclipseRftAddress depthAddress =
            RifEclipseRftAddress::createAddress( m_wellName(), m_timeStep, RifEclipseRftAddress::RftWellLogChannelType::MD );

        std::vector<double> values;
        reader->values( depthAddress, &values );
        if ( !values.empty() ) return values;

        return rftReader()->computeMeasuredDepth( m_wellName(), m_timeStep(), eclExtractor );
    };

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellLogRftCurve::deriveMeasuredDepthFromObservedData( const std::vector<double>& tvDepthValues, std::vector<double>& derivedMDValues )
{
    if ( m_observedFmuRftData )
    {
        RifReaderRftInterface* reader = m_observedFmuRftData->rftReader();
        if ( reader )
        {
            std::vector<double> tvdValuesOfObservedData;
            std::vector<double> mdValuesOfObservedData;

            RifEclipseRftAddress tvdAddress =
                RifEclipseRftAddress::createAddress( m_wellName(), m_timeStep, RifEclipseRftAddress::RftWellLogChannelType::TVD );
            RifEclipseRftAddress mdAddress =
                RifEclipseRftAddress::createAddress( m_wellName(), m_timeStep, RifEclipseRftAddress::RftWellLogChannelType::MD );

            reader->values( tvdAddress, &tvdValuesOfObservedData );
            reader->values( mdAddress, &mdValuesOfObservedData );

            // We are not able to estimate MD/TVD relationship for less than two samples
            if ( tvdValuesOfObservedData.size() < 2 ) return false;

            derivedMDValues = RigWellPathGeometryTools::interpolateMdFromTvd( mdValuesOfObservedData, tvdValuesOfObservedData, tvDepthValues );
            CVF_ASSERT( derivedMDValues.size() == tvDepthValues.size() );
            return true;
        }
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimWellLogRftCurve::segmentBranchIndex() const
{
    return m_segmentBranchIndex();
}
