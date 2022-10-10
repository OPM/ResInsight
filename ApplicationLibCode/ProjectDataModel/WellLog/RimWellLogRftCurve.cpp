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
#include "RiaQDateTimeTools.h"
#include "RiaResultNames.h"
#include "RiaRftDefines.h"
#include "RiaSimWellBranchTools.h"
#include "RiaSummaryTools.h"

#include "RifEclipseRftAddress.h"
#include "RifReaderEclipseRft.h"

#include "RigEclipseCaseData.h"
#include "RigEclipseWellLogExtractor.h"
#include "RigMainGrid.h"
#include "RigWellLogCurveData.h"
#include "RigWellPath.h"
#include "RigWellPathGeometryTools.h"
#include "RigWellPathIntersectionTools.h"

#include "RimDepthTrackPlot.h"
#include "RimEclipseResultCase.h"
#include "RimMainPlotCollection.h"
#include "RimObservedFmuRftData.h"
#include "RimProject.h"
#include "RimRftTools.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseCollection.h"
#include "RimTools.h"
#include "RimWellLogPlot.h"
#include "RimWellLogPlotCollection.h"
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

    CAF_PDM_InitFieldNoDefault( &m_eclipseResultCase, "CurveEclipseResultCase", "Eclipse Result Case" );
    m_eclipseResultCase.uiCapability()->setUiTreeChildrenHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_summaryCase, "CurveSummaryCase", "Summary Case" );
    m_summaryCase.uiCapability()->setUiTreeChildrenHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_ensemble, "CurveEnsemble", "Ensemble" );
    m_ensemble.uiCapability()->setUiTreeChildrenHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_observedFmuRftData, "ObservedFmuRftData", "Observed FMU RFT Data" );
    m_observedFmuRftData.uiCapability()->setUiTreeChildrenHidden( true );

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
void RimWellLogRftCurve::setEclipseResultCase( RimEclipseResultCase* eclipseResultCase )
{
    m_eclipseResultCase = eclipseResultCase;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseResultCase* RimWellLogRftCurve::eclipseResultCase() const
{
    return m_eclipseResultCase;
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
void RimWellLogRftCurve::setEnsemble( RimSummaryCaseCollection* ensemble )
{
    m_ensemble = ensemble;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCaseCollection* RimWellLogRftCurve::ensemble() const
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

    if ( resultName.startsWith( "SEGO" ) || resultName.startsWith( "CONO" ) )
    {
        color = RiaColorTables::summaryCurveGreenPaletteColors().cycledColor3f( 0 );
    }
    else if ( resultName.startsWith( "SEGW" ) || resultName.startsWith( "CONW" ) )
    {
        color = RiaColorTables::summaryCurveBluePaletteColors().cycledColor3f( 0 );
    }
    else if ( resultName.startsWith( "SEGG" ) || resultName.startsWith( "CONG" ) )
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
std::map<QString, QString> RimWellLogRftCurve::createCurveNameKeyValueMap() const
{
    std::map<QString, QString> variableValueMap;

    if ( !wellName().isEmpty() )
    {
        variableValueMap[RiaDefines::curveNameVariableWell()] = wellName();
    }

    variableValueMap[RiaDefines::curveNameVariableResultType()] = "RFT";

    QString caseText;
    if ( m_eclipseResultCase )
    {
        caseText = m_eclipseResultCase->caseUserDescription();
    }
    else if ( m_ensemble ) // Summary RFT curves have both ensemble and summary set. Prioritize ensemble for name.
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
        variableValueMap[RiaDefines::curveNameVariableCase()] = caseText;
    }

    if ( m_rftDataType() == RftDataType::RFT_DATA )
    {
        if ( wellLogChannelUiName() != caf::AppEnum<RifEclipseRftAddress::RftWellLogChannelType>::text(
                                           RifEclipseRftAddress::RftWellLogChannelType::NONE ) )
        {
            RifEclipseRftAddress::RftWellLogChannelType channelNameEnum =
                caf::AppEnum<RifEclipseRftAddress::RftWellLogChannelType>::fromText( wellLogChannelUiName() );
            QString channelName = caf::AppEnum<RifEclipseRftAddress::RftWellLogChannelType>::uiText( channelNameEnum );

            variableValueMap[RiaDefines::curveNameVariableResultName()] = channelName;
        }
    }
    else if ( m_rftDataType() == RftDataType::RFT_SEGMENT_DATA )
    {
        variableValueMap[RiaDefines::curveNameVariableResultName()] = m_segmentResultName;

        QString branchText = QString( "Branch %1" ).arg( m_segmentBranchIndex() );

        variableValueMap[RiaDefines::curveNameVariableWellBranch()] = branchText;

        variableValueMap[RiaDefines::curveNameVariableResultType()] = m_segmentBranchType().uiText();
    }

    if ( !m_timeStep().isNull() )
    {
        variableValueMap[RiaDefines::curveNameVariableTime()] =
            m_timeStep().toString( RiaQDateTimeTools::dateFormatString() );
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
    QString curveName = templateText;

    for ( const auto& [key, value] : createCurveNameKeyValueMap() )
    {
        curveName.replace( key, value );
    }

    return curveName;
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

    this->RimPlotCurve::updateCurvePresentation( updateParentPlot );

    DerivedMDSource derivedMDSource = DerivedMDSource::NO_SOURCE;

    if ( isCurveVisible() )
    {
        RimDepthTrackPlot* wellLogPlot;
        firstAncestorOrThisOfType( wellLogPlot );
        CVF_ASSERT( wellLogPlot );

        auto* rftPlot                     = dynamic_cast<RimWellRftPlot*>( wellLogPlot );
        bool  showErrorBarsInObservedData = rftPlot ? rftPlot->showErrorBarsForObservedData() : false;
        m_showErrorBars                   = showErrorBarsInObservedData;

        std::vector<double>  measuredDepthVector = measuredDepthValues();
        std::vector<double>  tvDepthVector       = tvDepthValues();
        std::vector<double>  values              = xValues();
        std::vector<double>  errors              = errorValues();
        std::vector<QString> perPointLabels;

        if ( values.empty() || values.size() != tvDepthVector.size() )
        {
            this->detach( true );
            return;
        }

        RiaDefines::EclipseUnitSystem unitSystem = RiaDefines::EclipseUnitSystem::UNITS_METRIC;
        if ( m_eclipseResultCase )
        {
            // TODO: If no grid data, but only RFT data is loaded, we do not have any way to
            // detect unit
            if ( m_eclipseResultCase->eclipseCaseData() )
            {
                unitSystem = m_eclipseResultCase->eclipseCaseData()->unitsType();
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
        else
        {
            CVF_ASSERT( false && "Need to have either an eclipse result case, a summary case or an ensemble" );
        }

        if ( tvDepthVector.size() != measuredDepthVector.size() )
        {
            if ( deriveMeasuredDepthValuesFromWellPath( tvDepthVector, measuredDepthVector ) )
            {
                derivedMDSource = DerivedMDSource::WELL_PATH;
            }
            else if ( deriveMeasuredDepthFromObservedData( tvDepthVector, measuredDepthVector ) )
            {
                derivedMDSource = DerivedMDSource::OBSERVED_DATA;
            }
        }

        if ( tvDepthVector.size() != measuredDepthVector.size() )
        {
            derivedMDSource     = DerivedMDSource::NO_SOURCE;
            measuredDepthVector = tvDepthVector;
        }

        RimProject*  proj     = RimProject::current();
        RimWellPath* wellPath = proj->wellPathByName( m_wellName );

        double rkbDiff = 0.0;
        if ( wellPath )
        {
            rkbDiff = wellPath->wellPathGeometry()->rkbDiff();
        }

        bool useLogarithmicScale = false;
        this->setPropertyValuesWithMdAndTVD( values,
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

        if ( wellLogPlot->depthType() == RiaDefines::DepthTypeEnum::MEASURED_DEPTH )
        {
            m_plotCurve->setPerPointLabels( perPointLabels );

            auto propertyValues = this->curveData()->propertyValues();
            auto depthValues    = this->curveData()->depths( RiaDefines::DepthTypeEnum::MEASURED_DEPTH, displayUnit );

            if ( !errors.empty() )
            {
                setPropertyAndDepthsAndErrors( propertyValues, depthValues, errors );
            }
            else
            {
                setPropertyAndDepthValuesToPlotCurve( propertyValues, depthValues );
            }

            RimWellLogTrack* wellLogTrack;
            firstAncestorOrThisOfType( wellLogTrack );
            CVF_ASSERT( wellLogTrack );

            RiuQwtPlotWidget* viewer = wellLogTrack->viewer();
            if ( viewer )
            {
                QString text;

                if ( derivedMDSource != DerivedMDSource::NO_SOURCE )
                {
                    if ( derivedMDSource == DerivedMDSource::WELL_PATH )
                    {
                        text = "WELL/" + wellLogPlot->depthAxisTitle();
                    }
                    else
                    {
                        text = "OBS/" + wellLogPlot->depthAxisTitle();
                    }
                }
                else // Standard depth title set from plot
                {
                    text = wellLogPlot->depthAxisTitle();
                }

                viewer->setAxisTitleText( wellLogPlot->depthAxis(), text );
            }
        }
        else
        {
            m_plotCurve->setPerPointLabels( perPointLabels );

            auto propertyValues = this->curveData()->propertyValuesByIntervals();
            auto depthValues =
                this->curveData()->depthValuesByIntervals( RiaDefines::DepthTypeEnum::TRUE_VERTICAL_DEPTH, displayUnit );
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

        m_plotCurve->setLineSegmentStartStopIndices( this->curveData()->polylineStartStopIndices() );

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
    RimPlotCurve::updateOptionSensitivity();

    caf::PdmUiGroup* curveDataGroup = uiOrdering.addNewGroup( "Curve Data" );
    curveDataGroup->add( &m_eclipseResultCase );
    curveDataGroup->add( &m_summaryCase );
    curveDataGroup->add( &m_wellName );
    curveDataGroup->add( &m_timeStep );
    curveDataGroup->add( &m_rftDataType );

    if ( m_rftDataType() == RimWellLogRftCurve::RftDataType::RFT_DATA )
    {
        curveDataGroup->add( &m_wellLogChannelName );

        RiaSimWellBranchTools::appendSimWellBranchFieldsIfRequiredFromWellName( curveDataGroup,
                                                                                m_wellName,
                                                                                m_branchDetection,
                                                                                m_branchIndex );
    }
    else
    {
        curveDataGroup->add( &m_segmentResultName );
        curveDataGroup->add( &m_segmentBranchType );
        curveDataGroup->add( &m_segmentBranchIndex );
        curveDataGroup->add( &m_curveColorByPhase );
    }

    caf::PdmUiGroup* stackingGroup = uiOrdering.addNewGroup( "Stacking" );
    RimStackablePlotCurve::stackingUiOrdering( *stackingGroup );

    caf::PdmUiGroup* appearanceGroup = uiOrdering.addNewGroup( "Appearance" );
    RimPlotCurve::appearanceUiOrdering( *appearanceGroup );

    caf::PdmUiGroup* nameGroup = uiOrdering.addNewGroup( "Curve Name" );
    nameGroup->add( &m_showLegend );
    RimPlotCurve::curveNameUiOrdering( *nameGroup );

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
    if ( fieldNeedingOptions == &m_eclipseResultCase )
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
        options = RimRftTools::segmentBranchIndexOptions( reader, m_wellName(), m_timeStep(), m_segmentBranchType() );
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogRftCurve::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                           const QVariant&            oldValue,
                                           const QVariant&            newValue )
{
    m_idxInWellPathToIdxInRftFile.clear();

    bool loadData = false;

    RimWellLogCurve::fieldChangedByUi( changedField, oldValue, newValue );
    if ( changedField == &m_eclipseResultCase )
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
              changedField == &m_rftDataType || changedField == &m_segmentBranchType )
    {
        loadData = true;
    }

    if ( loadData ) this->loadDataAndUpdate( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> RimWellLogRftCurve::perPointLabels() const
{
    if ( m_observedFmuRftData() )
    {
        auto address = RifEclipseRftAddress::createAddress( m_wellName(),
                                                            m_timeStep,
                                                            RifEclipseRftAddress::RftWellLogChannelType::PRESSURE );
        return m_observedFmuRftData()->labels( address );
    }
    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifReaderRftInterface* RimWellLogRftCurve::rftReader() const
{
    if ( m_eclipseResultCase() )
    {
        return m_eclipseResultCase()->rftReader();
    }

    if ( m_summaryCase() ) // Summary RFT curves have both summary and ensemble set. Prioritize summary for reader.
    {
        return m_summaryCase()->rftReader();
    }

    if ( m_ensemble() )
    {
        return m_ensemble()->rftStatisticsReader();
    }

    if ( m_observedFmuRftData() )
    {
        return m_observedFmuRftData()->rftReader();
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigEclipseWellLogExtractor* RimWellLogRftCurve::extractor()
{
    RifReaderRftInterface* reader = rftReader();
    if ( !reader ) return nullptr;

    RimMainPlotCollection* mainPlotCollection;
    this->firstAncestorOrThisOfTypeAsserted( mainPlotCollection );

    RimWellLogPlotCollection* wellLogCollection = mainPlotCollection->wellLogPlotCollection();
    if ( !wellLogCollection ) return nullptr;

    RigEclipseWellLogExtractor* eclExtractor = nullptr;

    RimProject*  proj     = RimProject::current();
    RimWellPath* wellPath = proj->wellPathFromSimWellName( m_wellName() );
    eclExtractor          = wellLogCollection->findOrCreateExtractor( wellPath, m_eclipseResultCase );

    if ( !eclExtractor && m_eclipseResultCase )
    {
        QString                         simWellName = RimWellPlotTools::simWellName( m_wellName );
        std::vector<const RigWellPath*> wellPaths =
            RiaSimWellBranchTools::simulationWellBranches( simWellName, m_branchDetection );
        if ( wellPaths.empty() ) return nullptr;

        m_branchIndex = RiaSimWellBranchTools::clampBranchIndex( simWellName, m_branchIndex, m_branchDetection );

        auto wellPathBranch = wellPaths[m_branchIndex];

        eclExtractor = wellLogCollection->findOrCreateSimWellExtractor( simWellName,
                                                                        QString( "Find or create sim well extractor" ),
                                                                        wellPathBranch,
                                                                        m_eclipseResultCase->eclipseCaseData() );
    }

    return eclExtractor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellLogRftCurve::createWellPathIdxToRftFileIdxMapping()
{
    if ( !m_idxInWellPathToIdxInRftFile.empty() )
    {
        return true;
    }

    RigEclipseWellLogExtractor* eclExtractor = extractor();

    if ( !eclExtractor ) return false;

    std::vector<WellPathCellIntersectionInfo> intersections = eclExtractor->cellIntersectionInfosAlongWellPath();
    if ( intersections.empty() ) return false;

    std::map<size_t, size_t> globCellIndicesToIndexInWell;

    for ( size_t idx = 0; idx < intersections.size(); idx++ )
    {
        globCellIndicesToIndexInWell[intersections[idx].globCellIndex] = idx;
    }

    RifEclipseRftAddress depthAddress =
        RifEclipseRftAddress::createAddress( m_wellName(), m_timeStep, RifEclipseRftAddress::RftWellLogChannelType::TVD );
    std::vector<caf::VecIjk> rftIndices;
    if ( !rftReader() ) return false;

    rftReader()->cellIndices( depthAddress, &rftIndices );

    const RigMainGrid* mainGrid = eclExtractor->caseData()->mainGrid();

    for ( size_t idx = 0; idx < rftIndices.size(); idx++ )
    {
        caf::VecIjk ijkIndex        = rftIndices[idx];
        size_t      globalCellIndex = mainGrid->cellIndexFromIJK( ijkIndex.i(), ijkIndex.j(), ijkIndex.k() );

        if ( globCellIndicesToIndexInWell.find( globalCellIndex ) != globCellIndicesToIndexInWell.end() )
        {
            m_idxInWellPathToIdxInRftFile[globCellIndicesToIndexInWell[globalCellIndex]] = idx;
        }
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RimWellLogRftCurve::rftFileIndex( size_t wellPathIndex )
{
    if ( m_idxInWellPathToIdxInRftFile.empty() )
    {
        createWellPathIdxToRftFileIdxMapping();
    }

    if ( m_idxInWellPathToIdxInRftFile.find( wellPathIndex ) == m_idxInWellPathToIdxInRftFile.end() )
    {
        return cvf::UNDEFINED_SIZE_T;
    }

    return m_idxInWellPathToIdxInRftFile[wellPathIndex];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<size_t> RimWellLogRftCurve::sortedIndicesInRftFile()
{
    if ( m_idxInWellPathToIdxInRftFile.empty() )
    {
        createWellPathIdxToRftFileIdxMapping();
    }

    std::vector<size_t> indices;
    for ( auto& it : m_idxInWellPathToIdxInRftFile )
    {
        indices.push_back( it.second );
    }

    return indices;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimWellLogRftCurve::xValues()
{
    RifReaderRftInterface* reader = rftReader();
    std::vector<double>    values;

    if ( !reader ) return values;

    if ( m_rftDataType() == RftDataType::RFT_SEGMENT_DATA )
    {
        auto depthAddress = RifEclipseRftAddress::createBranchSegmentAddress( m_wellName(),
                                                                              m_timeStep,
                                                                              m_segmentResultName(),
                                                                              segmentBranchIndex(),
                                                                              m_segmentBranchType() );

        reader->values( depthAddress, &values );

        return values;
    }

    auto address = RifEclipseRftAddress::createAddress( m_wellName(), m_timeStep, m_wellLogChannelName() );

    reader->values( address, &values );

    bool wellPathExists = createWellPathIdxToRftFileIdxMapping();

    if ( wellPathExists )
    {
        std::vector<double> valuesSorted;

        for ( size_t idx : sortedIndicesInRftFile() )
        {
            if ( idx < values.size() )
            {
                valuesSorted.push_back( ( values.at( idx ) ) );
            }
        }

        return valuesSorted;
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
            RifEclipseRftAddress::createAddress( m_wellName(),
                                                 m_timeStep,
                                                 RifEclipseRftAddress::RftWellLogChannelType::PRESSURE_ERROR );
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

    if ( m_rftDataType() == RftDataType::RFT_SEGMENT_DATA )
    {
        auto depthAddress = RifEclipseRftAddress::createBranchSegmentAddress( m_wellName(),
                                                                              m_timeStep,
                                                                              RiaDefines::segmentTvdDepthResultName(),
                                                                              segmentBranchIndex(),
                                                                              m_segmentBranchType() );

        reader->values( depthAddress, &values );
        return values;
    }

    auto depthAddress =
        RifEclipseRftAddress::createAddress( m_wellName(), m_timeStep, RifEclipseRftAddress::RftWellLogChannelType::TVD );
    reader->values( depthAddress, &values );

    bool wellPathExists = createWellPathIdxToRftFileIdxMapping();

    if ( wellPathExists )
    {
        std::vector<double> valuesSorted;

        for ( size_t idx : sortedIndicesInRftFile() )
        {
            if ( idx < values.size() )
            {
                valuesSorted.push_back( ( values.at( idx ) ) );
            }
        }

        return valuesSorted;
    }

    return values;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimWellLogRftCurve::measuredDepthValues()
{
    if ( m_rftDataType() == RftDataType::RFT_SEGMENT_DATA )
    {
        std::vector<double> values;

        RifReaderRftInterface* reader = rftReader();
        if ( reader )
        {
            auto depthAddress =
                RifEclipseRftAddress::createBranchSegmentAddress( m_wellName(),
                                                                  m_timeStep,
                                                                  RiaDefines::segmentStartDepthResultName(),
                                                                  segmentBranchIndex(),
                                                                  m_segmentBranchType() );

            reader->values( depthAddress, &values );

            // Special handling of first segment
            if ( values.size() > 2 && values.front() < 0.001 )
            {
                values[0] = values[1];
            }
        }
        return values;
    }

    if ( m_observedFmuRftData && !m_ensemble && !m_summaryCase )
    {
        RifReaderRftInterface* reader = rftReader();
        std::vector<double>    values;

        if ( !reader ) return values;

        RifEclipseRftAddress depthAddress =
            RifEclipseRftAddress::createAddress( m_wellName(), m_timeStep, RifEclipseRftAddress::RftWellLogChannelType::MD );
        reader->values( depthAddress, &values );
        return values;
    }

    std::vector<double> measuredDepthForCells;

    RigEclipseWellLogExtractor* eclExtractor = extractor();

    if ( !eclExtractor ) return measuredDepthForCells;

    std::vector<double> measuredDepthForIntersections = eclExtractor->cellIntersectionMDs();

    if ( measuredDepthForIntersections.empty() )
    {
        return measuredDepthForCells;
    }

    std::vector<size_t> globCellIndices = eclExtractor->intersectedCellsGlobIdx();

    for ( size_t i = 0; i < globCellIndices.size() - 1; i = i + 2 )
    {
        double sum = measuredDepthForIntersections[i] + measuredDepthForIntersections[i + 1];

        measuredDepthForCells.push_back( sum / 2.0 );
    }

    std::vector<double> measuredDepthForCellsWhichHasRftData;

    for ( size_t i = 0; i < measuredDepthForCells.size(); i++ )
    {
        if ( rftFileIndex( i ) != cvf::UNDEFINED_SIZE_T )
        {
            measuredDepthForCellsWhichHasRftData.push_back( measuredDepthForCells[i] );
        }
    }

    return measuredDepthForCellsWhichHasRftData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellLogRftCurve::deriveMeasuredDepthValuesFromWellPath( const std::vector<double>& tvDepthValues,
                                                                std::vector<double>&       derivedMDValues )
{
    RimProject*  proj     = RimProject::current();
    RimWellPath* wellPath = proj->wellPathByName( m_wellName );

    if ( wellPath && wellPath->wellPathGeometry() )
    {
        const std::vector<double>& mdValuesOfWellPath  = wellPath->wellPathGeometry()->measuredDepths();
        const std::vector<double>& tvdValuesOfWellPath = wellPath->wellPathGeometry()->trueVerticalDepths();

        derivedMDValues =
            RigWellPathGeometryTools::interpolateMdFromTvd( mdValuesOfWellPath, tvdValuesOfWellPath, tvDepthValues );
        CVF_ASSERT( derivedMDValues.size() == tvDepthValues.size() );
        return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellLogRftCurve::deriveMeasuredDepthFromObservedData( const std::vector<double>& tvDepthValues,
                                                              std::vector<double>&       derivedMDValues )
{
    if ( m_observedFmuRftData )
    {
        RifReaderRftInterface* reader = m_observedFmuRftData->rftReader();
        if ( reader )
        {
            std::vector<double> tvdValuesOfObservedData;
            std::vector<double> mdValuesOfObservedData;

            RifEclipseRftAddress tvdAddress =
                RifEclipseRftAddress::createAddress( m_wellName(),
                                                     m_timeStep,
                                                     RifEclipseRftAddress::RftWellLogChannelType::TVD );
            RifEclipseRftAddress mdAddress =
                RifEclipseRftAddress::createAddress( m_wellName(),
                                                     m_timeStep,
                                                     RifEclipseRftAddress::RftWellLogChannelType::MD );

            reader->values( tvdAddress, &tvdValuesOfObservedData );
            reader->values( mdAddress, &mdValuesOfObservedData );

            // We are not able to estimate MD/TVD relationship for less than two samples
            if ( tvdValuesOfObservedData.size() < 2 ) return false;

            derivedMDValues = RigWellPathGeometryTools::interpolateMdFromTvd( mdValuesOfObservedData,
                                                                              tvdValuesOfObservedData,
                                                                              tvDepthValues );
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
