/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022     Equinor ASA
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

#include "RimRftTopologyCurve.h"

#include "RiaColorTables.h"
#include "RiaColorTools.h"
#include "Summary/RiaSummaryTools.h"

#include "RifReaderOpmRft.h"

#include "Well/RigWellLogCurveData.h"

#include "RimDepthTrackPlot.h"
#include "RimProject.h"
#include "RimRftTools.h"
#include "RimSummaryCase.h"
#include "RimWellLogPlot.h"

#include "RiuPlotCurve.h"

CAF_PDM_SOURCE_INIT( RimRftTopologyCurve, "RimRftTopologyCurve" );

namespace caf
{
template <>
void caf::AppEnum<RimRftTopologyCurve::CurveType>::setUp()
{
    addItem( RimRftTopologyCurve::CurveType::PACKER, "PACKER", "Packer" );
    addItem( RimRftTopologyCurve::CurveType::TUBING, "TUBING", "Tubing" );
    addItem( RimRftTopologyCurve::CurveType::ANNULUS, "ANNULUS", "Annulus" );
    addItem( RimRftTopologyCurve::CurveType::DEVICE, "DEVICE", "Device" );

    setDefault( RimRftTopologyCurve::CurveType::TUBING );
}
template <>
void caf::AppEnum<RimRftTopologyCurve::SymbolLocationType>::setUp()
{
    addItem( RimRftTopologyCurve::SymbolLocationType::START, "START", "Start" );
    addItem( RimRftTopologyCurve::SymbolLocationType::MID, "MID", "Midpoint" );
    addItem( RimRftTopologyCurve::SymbolLocationType::END, "END", "End" );

    setDefault( RimRftTopologyCurve::SymbolLocationType::END );
}

} // End namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimRftTopologyCurve::RimRftTopologyCurve()
{
    CAF_PDM_InitObject( "RFT Topology Curve" );

    CAF_PDM_InitFieldNoDefault( &m_summaryCase, "SummaryCase", "Summary Case" );
    m_summaryCase.uiCapability()->setUiTreeChildrenHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_timeStep, "TimeStep", "Time Step" );
    CAF_PDM_InitFieldNoDefault( &m_wellName, "WellName", "Well Name" );

    CAF_PDM_InitField( &m_segmentBranchIndex, "SegmentBranchIndex", -1, "Branch" );
    CAF_PDM_InitFieldNoDefault( &m_segmentBranchType, "SegmentBranchType", "Completion" );

    CAF_PDM_InitFieldNoDefault( &m_curveType, "CurveType", "Curve Type" );
    CAF_PDM_InitFieldNoDefault( &m_symbolLocation, "SymbolLocation", "Symbol Location on Segment" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimRftTopologyCurve* RimRftTopologyCurve::createPackerCurve( RimSummaryCase*  summaryCase,
                                                             const QDateTime& timeStep,
                                                             const QString&   wellName,
                                                             int              segmentBranchIndex )
{
    auto* curve = new RimRftTopologyCurve();
    curve->setDataSource( summaryCase, timeStep, wellName, segmentBranchIndex );
    curve->m_curveType = CurveType::PACKER;

    return curve;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimRftTopologyCurve* RimRftTopologyCurve::createTopologyCurve( RimSummaryCase*           summaryCase,
                                                               const QDateTime&          timeStep,
                                                               const QString&            wellName,
                                                               int                       segmentBranchIndex,
                                                               RiaDefines::RftBranchType branchType )
{
    auto* curve = new RimRftTopologyCurve();
    curve->setDataSource( summaryCase, timeStep, wellName, segmentBranchIndex );

    switch ( branchType )
    {
        case RiaDefines::RftBranchType::RFT_TUBING:
            curve->m_curveType = CurveType::TUBING;
            break;
        case RiaDefines::RftBranchType::RFT_DEVICE:
            curve->m_curveType = CurveType::DEVICE;
            break;
        case RiaDefines::RftBranchType::RFT_ANNULUS:
            curve->m_curveType = CurveType::ANNULUS;
            break;
        case RiaDefines::RftBranchType::RFT_UNKNOWN:
            break;
        default:
            break;
    }
    curve->m_segmentBranchType = branchType;

    return curve;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRftTopologyCurve::setDataSource( RimSummaryCase* summaryCase, const QDateTime& timeStep, const QString& wellName, int segmentBranchIndex )
{
    m_summaryCase        = summaryCase;
    m_timeStep           = timeStep;
    m_wellName           = wellName;
    m_segmentBranchIndex = segmentBranchIndex;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimRftTopologyCurve::wellName() const
{
    return m_wellName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimRftTopologyCurve::wellLogChannelUiName() const
{
    return "Topology curve channel";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimRftTopologyCurve::wellLogChannelUnits() const
{
    return "Topology curve units";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Color3f RimRftTopologyCurve::colorForBranchType( CurveType curveType )
{
    switch ( curveType )
    {
        case RimRftTopologyCurve::CurveType::PACKER:
            return RiaColorTools::fromQColorTo3f( QColor( "DarkGoldenRod" ) );
            break;
        case RimRftTopologyCurve::CurveType::TUBING:
            return colorForRftBranchType( RiaDefines::RftBranchType::RFT_TUBING );
            break;
        case RimRftTopologyCurve::CurveType::DEVICE:
            return colorForRftBranchType( RiaDefines::RftBranchType::RFT_DEVICE );
            break;
        case RimRftTopologyCurve::CurveType::ANNULUS:
            return colorForRftBranchType( RiaDefines::RftBranchType::RFT_ANNULUS );
            break;
        default:
            break;
    }

    return RiaColorTools::fromQColorTo3f( QColor( "LightBrown" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Color3f RimRftTopologyCurve::colorForRftBranchType( RiaDefines::RftBranchType branchType )
{
    switch ( branchType )
    {
        case RiaDefines::RftBranchType::RFT_TUBING:
            return RiaColorTools::fromQColorTo3f( QColor( "ForestGreen" ) );
        case RiaDefines::RftBranchType::RFT_DEVICE:
            return RiaColorTools::fromQColorTo3f( QColor( "IndianRed" ) );
        case RiaDefines::RftBranchType::RFT_ANNULUS:
            return RiaColorTools::fromQColorTo3f( QColor( "DeepSkyBlue" ) );
        case RiaDefines::RftBranchType::RFT_UNKNOWN:
            break;
        default:
            break;
    }

    return RiaColorTools::fromQColorTo3f( QColor( "LightBrown" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimRftTopologyCurve::createCurveAutoName()
{
    return m_curveType().uiText();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRftTopologyCurve::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    RimPlotCurve::updateFieldUiState();

    caf::PdmUiGroup* curveDataGroup = uiOrdering.addNewGroup( "Data Source" );
    curveDataGroup->add( &m_summaryCase );
    curveDataGroup->add( &m_wellName );
    curveDataGroup->add( &m_timeStep );
    curveDataGroup->add( &m_curveType );
    curveDataGroup->add( &m_symbolLocation );

    curveDataGroup->add( &m_segmentBranchIndex );
    curveDataGroup->add( &m_segmentBranchType );

    RimStackablePlotCurve::defaultUiOrdering( uiOrdering );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimRftTopologyCurve::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    if ( !m_summaryCase ) return {};

    QList<caf::PdmOptionItemInfo> options = RimWellLogCurve::calculateValueOptions( fieldNeedingOptions );

    auto reader = m_summaryCase->rftReader();

    if ( fieldNeedingOptions == &m_summaryCase )
    {
        bool includeEnsembleName = true;
        options                  = RiaSummaryTools::optionsForSummaryCases( RimProject::current()->allSummaryCases(), includeEnsembleName );
        options.push_front( caf::PdmOptionItemInfo( "None", nullptr ) );
    }
    else if ( fieldNeedingOptions == &m_wellName )
    {
        options = RimRftTools::wellNameOptions( reader );
    }
    else if ( fieldNeedingOptions == &m_timeStep )
    {
        options = RimRftTools::segmentTimeStepOptions( reader, m_wellName );
    }
    else if ( fieldNeedingOptions == &m_segmentBranchIndex )
    {
        options = RimRftTools::segmentBranchIndexOptions( dynamic_cast<RifReaderOpmRft*>( reader ),
                                                          m_wellName(),
                                                          m_timeStep(),
                                                          RiaDefines::RftBranchType::RFT_UNKNOWN );
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRftTopologyCurve::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    RimWellLogCurve::fieldChangedByUi( changedField, oldValue, newValue );

    loadDataAndUpdate( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRftTopologyCurve::onLoadDataAndUpdate( bool updateParentPlot )
{
    RimPlotCurve::updateCurvePresentation( updateParentPlot );

    if ( m_summaryCase )
    {
        auto rftReader = dynamic_cast<RifReaderOpmRft*>( m_summaryCase->rftReader() );

        // Update well path attributes, packers and casing based on RFT data
        if ( rftReader )
        {
            std::vector<double> depths;
            std::vector<double> propertyValues;

            std::vector<double> symbolLocationDepths;
            if ( m_symbolLocation() == SymbolLocationType::START )
            {
                symbolLocationDepths =
                    RimRftTools::segmentStartMdValues( rftReader, m_wellName, m_timeStep, -1, RiaDefines::RftBranchType::RFT_UNKNOWN );
            }
            else if ( m_symbolLocation() == SymbolLocationType::MID )
            {
                symbolLocationDepths =
                    RimRftTools::segmentStartMdValues( rftReader, m_wellName, m_timeStep, -1, RiaDefines::RftBranchType::RFT_UNKNOWN );
                auto endDepths =
                    RimRftTools::segmentEndMdValues( rftReader, m_wellName, m_timeStep, -1, RiaDefines::RftBranchType::RFT_UNKNOWN );

                if ( symbolLocationDepths.size() == endDepths.size() )
                {
                    for ( size_t i = 0; i < symbolLocationDepths.size(); ++i )
                    {
                        symbolLocationDepths[i] = ( symbolLocationDepths[i] + endDepths[i] ) / 2.0;
                    }
                }
            }
            else if ( m_symbolLocation() == SymbolLocationType::END )
            {
                symbolLocationDepths =
                    RimRftTools::segmentEndMdValues( rftReader, m_wellName, m_timeStep, -1, RiaDefines::RftBranchType::RFT_UNKNOWN );
            }

            auto segment        = rftReader->segmentForWell( m_wellName, m_timeStep );
            auto segmentIndices = segment.segmentIndicesForBranchIndex( m_segmentBranchIndex(), m_segmentBranchType() );
            if ( !segmentIndices.empty() )
            {
                // Assign a static property value to each type of curve to make sure they all are separated and
                // easily visible
                double curveValue = 1.0;
                if ( m_segmentBranchType() == RiaDefines::RftBranchType::RFT_TUBING ) curveValue = 2.0;
                if ( m_segmentBranchType() == RiaDefines::RftBranchType::RFT_DEVICE ) curveValue = 3.0;
                if ( m_segmentBranchType() == RiaDefines::RftBranchType::RFT_ANNULUS ) curveValue = 4.0;

                if ( m_curveType() == CurveType::PACKER )
                {
                    curveValue = 4.0;
                }

                // Adjust the location of each branch if multiple branches are visible at the same time
                curveValue += m_segmentBranchIndex() * 0.2;

                if ( m_curveType() == RimRftTopologyCurve::CurveType::PACKER )
                {
                    auto packerSegmentIndices = segment.packerSegmentIndicesOnAnnulus( m_segmentBranchIndex() );

                    for ( auto segmentIndex : packerSegmentIndices )
                    {
                        depths.push_back( symbolLocationDepths[segmentIndex] );

                        propertyValues.push_back( curveValue );
                    }
                }
                else
                {
                    for ( auto segmentIndex : segmentIndices )
                    {
                        depths.push_back( symbolLocationDepths[segmentIndex] );

                        propertyValues.push_back( curveValue );
                    }
                }
            }

            auto wellLogPlot = firstAncestorOrThisOfTypeAsserted<RimDepthTrackPlot>();

            RimWellLogPlot::DepthTypeEnum depthType           = wellLogPlot->depthType();
            RiaDefines::DepthUnitType     displayUnit         = wellLogPlot->depthUnit();
            bool                          isExtractionCurve   = false;
            bool                          useLogarithmicScale = false;
            setPropertyValuesAndDepths( propertyValues, depths, depthType, 0.0, displayUnit, isExtractionCurve, useLogarithmicScale );

            // Assign curve values based on horizontal or vertical plot
            setPropertyAndDepthValuesToPlotCurve( propertyValues, depths );

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
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRftTopologyCurve::setAdditionalDataSources( const std::vector<RimPlotCurve*>& additionalDataSources )
{
    m_additionalDataSources.setValue( additionalDataSources );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRftTopologyCurve::applyDefaultAppearance()
{
    auto color = colorForBranchType( m_curveType() );
    setColor( color );

    if ( m_curveType() == RimRftTopologyCurve::CurveType::PACKER )
    {
        int adjustedSymbolSize = symbolSize() * 2;

        setSymbolSize( adjustedSymbolSize );
        setLineStyle( RiuQwtPlotCurveDefines::LineStyleEnum::STYLE_NONE );
        setSymbol( RiuPlotCurveSymbol::PointSymbolEnum::SYMBOL_RECT );
    }
    else
    {
        if ( m_curveType() == RimRftTopologyCurve::CurveType::TUBING )
        {
            setLineThickness( 5.0 );
        }
        else if ( m_curveType() == RimRftTopologyCurve::CurveType::DEVICE )
        {
            setSymbolEdgeColor( color );
            setLineStyle( RiuQwtPlotCurveDefines::LineStyleEnum::STYLE_NONE );
        }
        else if ( m_curveType() == RimRftTopologyCurve::CurveType::ANNULUS )
        {
            setSymbolEdgeColor( color );
            setLineStyle( RiuQwtPlotCurveDefines::LineStyleEnum::STYLE_NONE );
        }

        int adjustedSymbolSize = symbolSize() * 1.2;
        setSymbolSize( adjustedSymbolSize );
        setSymbol( RiuPlotCurveSymbol::PointSymbolEnum::SYMBOL_ELLIPSE );
    }
}
