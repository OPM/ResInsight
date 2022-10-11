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
#include "RiaSummaryTools.h"

#include "RifReaderOpmRft.h"

#include "RigWellLogCurveData.h"

#include "RimDepthTrackPlot.h"
#include "RimProject.h"
#include "RimRftTools.h"
#include "RimSummaryCase.h"
#include "RimWellLogPlot.h"

#include "RiuPlotCurve.h"

CAF_PDM_SOURCE_INIT( RimRftTopologyCurve, "RimRftTopologyCurve" );

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
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRftTopologyCurve::setDataSource( RimSummaryCase*           summaryCase,
                                         const QDateTime&          timeStep,
                                         const QString&            wellName,
                                         int                       segmentBranchIndex,
                                         RiaDefines::RftBranchType branchType )
{
    setDataSource( summaryCase, timeStep, wellName, segmentBranchIndex );

    m_segmentBranchType = branchType;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRftTopologyCurve::setDataSource( RimSummaryCase*  summaryCase,
                                         const QDateTime& timeStep,
                                         const QString&   wellName,
                                         int              segmentBranchIndex )
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
    return "Topology curve";
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
QString RimRftTopologyCurve::createCurveAutoName()
{
    QString text;

    if ( m_segmentBranchType() == RiaDefines::RftBranchType::RFT_ANNULUS ) text += "Annulus";
    if ( m_segmentBranchType() == RiaDefines::RftBranchType::RFT_DEVICE ) text += "Device";
    if ( m_segmentBranchType() == RiaDefines::RftBranchType::RFT_TUBING ) text += "Tubing";

    text += QString( " (%1)" ).arg( m_segmentBranchIndex() );

    return text;
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
    curveDataGroup->add( &m_segmentBranchIndex );
    curveDataGroup->add( &m_segmentBranchType );

    caf::PdmUiGroup* stackingGroup = uiOrdering.addNewGroup( "Stacking" );
    RimStackablePlotCurve::stackingUiOrdering( *stackingGroup );

    caf::PdmUiGroup* appearanceGroup = uiOrdering.addNewGroup( "Appearance" );
    RimPlotCurve::appearanceUiOrdering( *appearanceGroup );

    caf::PdmUiGroup* nameGroup = uiOrdering.addNewGroup( "Curve Name" );
    nameGroup->setCollapsedByDefault();
    nameGroup->add( &m_showLegend );
    RimPlotCurve::curveNameUiOrdering( *nameGroup );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimRftTopologyCurve::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    if ( !m_summaryCase ) return {};

    QList<caf::PdmOptionItemInfo> options;

    auto reader = m_summaryCase->rftReader();

    if ( fieldNeedingOptions == &m_summaryCase )
    {
        options = RiaSummaryTools::optionsForSummaryCases( RimProject::current()->allSummaryCases() );
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
        options = RimRftTools::segmentBranchIndexOptions( reader,
                                                          m_wellName(),
                                                          m_timeStep(),
                                                          RiaDefines::RftBranchType::RFT_UNKNOWN );
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRftTopologyCurve::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                            const QVariant&            oldValue,
                                            const QVariant&            newValue )
{
    RimWellLogCurve::fieldChangedByUi( changedField, oldValue, newValue );

    this->loadDataAndUpdate( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRftTopologyCurve::onLoadDataAndUpdate( bool updateParentPlot )
{
    this->RimPlotCurve::updateCurvePresentation( updateParentPlot );

    if ( m_summaryCase )
    {
        auto rftReader = dynamic_cast<RifReaderOpmRft*>( m_summaryCase->rftReader() );

        // Update well path attributes, packers and casing based on RFT data
        if ( rftReader )
        {
            std::vector<double> seglenstValues;
            std::vector<double> seglenenValues;

            auto resultNameSeglenst = RifEclipseRftAddress::createSegmentAddress( m_wellName, m_timeStep, "SEGLENST" );
            rftReader->values( resultNameSeglenst, &seglenstValues );

            auto resultNameSeglenen = RifEclipseRftAddress::createSegmentAddress( m_wellName, m_timeStep, "SEGLENEN" );
            rftReader->values( resultNameSeglenen, &seglenenValues );

            auto segment        = rftReader->segmentForWell( m_wellName, m_timeStep );
            auto segmentIndices = segment.segmentIndicesForBranchIndex( m_segmentBranchIndex(), m_segmentBranchType() );
            if ( !segmentIndices.empty() )
            {
                std::vector<double> depths;
                std::vector<double> propertyValues;

                // Assign a static property value to each type of curve to make sure they all are separated and easily
                // visible
                double curveValue = 1.0;
                if ( m_segmentBranchType() == RiaDefines::RftBranchType::RFT_TUBING ) curveValue = 2.0;
                if ( m_segmentBranchType() == RiaDefines::RftBranchType::RFT_DEVICE ) curveValue = 3.0;
                if ( m_segmentBranchType() == RiaDefines::RftBranchType::RFT_ANNULUS ) curveValue = 4.0;

                // Adjust the location of each branch if multiple branches are visible at the same time
                curveValue += m_segmentBranchIndex() * 0.2;

                for ( auto segmentIndex : segmentIndices )
                {
                    depths.push_back( seglenstValues[segmentIndex] );
                    depths.push_back( seglenenValues[segmentIndex] );

                    propertyValues.push_back( curveValue );
                    propertyValues.push_back( curveValue );
                }

                RimDepthTrackPlot* wellLogPlot;
                firstAncestorOrThisOfTypeAsserted( wellLogPlot );

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
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRftTopologyCurve::applyDefaultAppearance()
{
    cvf::Color3f color = cvf::Color3f::BLUE;
    if ( m_segmentBranchType() == RiaDefines::RftBranchType::RFT_TUBING )
    {
        color = RiaColorTables::wellLogPlotPaletteColors().cycledColor3f( 0 );
    }
    else if ( m_segmentBranchType() == RiaDefines::RftBranchType::RFT_DEVICE )
    {
        color = RiaColorTables::wellLogPlotPaletteColors().cycledColor3f( 1 );
        setLineStyle( RiuQwtPlotCurveDefines::LineStyleEnum::STYLE_NONE );
    }
    else if ( m_segmentBranchType() == RiaDefines::RftBranchType::RFT_ANNULUS )
    {
        color = RiaColorTables::wellLogPlotPaletteColors().cycledColor3f( 2 );
        setLineStyle( RiuQwtPlotCurveDefines::LineStyleEnum::STYLE_NONE );
    }

    setColor( color );
    setLineThickness( 5.0 );
    setSymbol( RiuPlotCurveSymbol::PointSymbolEnum::SYMBOL_ELLIPSE );
}
