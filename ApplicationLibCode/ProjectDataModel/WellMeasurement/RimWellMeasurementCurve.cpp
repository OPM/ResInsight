/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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

#include "RimWellMeasurementCurve.h"

#include "RigWellLogCurveData.h"
#include "RigWellPath.h"

#include "RimDepthTrackPlot.h"
#include "RimProject.h"
#include "RimTools.h"
#include "RimWellLogFile.h"
#include "RimWellLogTrack.h"
#include "RimWellMeasurement.h"
#include "RimWellMeasurementCollection.h"
#include "RimWellMeasurementFilter.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"
#include "RimWellPlotTools.h"
#include "RimWellRftPlot.h"

#include "RiuQwtPlotCurve.h"
#include "RiuQwtPlotWidget.h"

#include "RiaPreferences.h"

#include "cafPdmUiTreeOrdering.h"

#include <QFileInfo>

CAF_PDM_SOURCE_INIT( RimWellMeasurementCurve, "WellMeasurementCurve" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellMeasurementCurve::RimWellMeasurementCurve()
{
    CAF_PDM_InitObject( "Well Measurement Curve", RimWellLogCurve::wellLogCurveIconName() );

    CAF_PDM_InitFieldNoDefault( &m_wellPath, "CurveWellPath", "Well Path" );
    m_wellPath.uiCapability()->setUiTreeChildrenHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_measurementKind, "CurveMeasurementKind", "Measurement Kind" );
    m_measurementKind.uiCapability()->setUiTreeChildrenHidden( true );

    m_wellPath = nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellMeasurementCurve::~RimWellMeasurementCurve()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellMeasurementCurve::onLoadDataAndUpdate( bool updateParentPlot )
{
    RimDepthTrackPlot* wellLogPlot;
    firstAncestorOrThisOfType( wellLogPlot );
    CVF_ASSERT( wellLogPlot );

    RimWellPathCollection* wellPathCollection = RimTools::wellPathCollection();
    if ( m_wellPath && !m_measurementKind().isEmpty() && wellPathCollection )
    {
        const RimWellMeasurementCollection* measurementCollection = wellPathCollection->measurementCollection();

        std::vector<QString> measurementKinds;
        measurementKinds.push_back( measurementKind() );

        std::vector<RimWellMeasurement*> measurements =
            RimWellMeasurementFilter::filterMeasurements( measurementCollection->measurements(),
                                                          *wellPathCollection,
                                                          *m_wellPath,
                                                          measurementKinds );

        // Extract the values for this measurement kind
        std::vector<double> values;
        std::vector<double> measuredDepthValues;
        for ( auto& measurement : measurements )
        {
            if ( measurement->kind() == measurementKind() )
            {
                values.push_back( measurement->value() );
                measuredDepthValues.push_back( measurement->MD() );
            }
        }

        if ( values.size() == measuredDepthValues.size() )
        {
            RigWellPath* rigWellPath = m_wellPath->wellPathGeometry();
            if ( rigWellPath )
            {
                std::vector<double> trueVerticalDepthValues;

                for ( double measuredDepthValue : measuredDepthValues )
                {
                    trueVerticalDepthValues.push_back(
                        -rigWellPath->interpolatedPointAlongWellPath( measuredDepthValue ).z() );
                }

                bool useLogarithmicScale = false;
                this->setPropertyValuesWithMdAndTVD( values,
                                                     measuredDepthValues,
                                                     trueVerticalDepthValues,
                                                     m_wellPath->wellPathGeometry()->rkbDiff(),
                                                     RiaDefines::DepthUnitType::UNIT_METER,
                                                     false,
                                                     useLogarithmicScale );
            }
            else
            {
                bool useLogarithmicScale = false;
                this->setPropertyValuesAndDepths( values,
                                                  measuredDepthValues,
                                                  RiaDefines::DepthTypeEnum::MEASURED_DEPTH,
                                                  0.0,
                                                  RiaDefines::DepthUnitType::UNIT_METER,
                                                  false,
                                                  useLogarithmicScale );
            }
        }

        if ( m_isUsingAutoName )
        {
            m_plotCurve->setTitle( createCurveAutoName() );
        }

        setSymbol( getSymbolForMeasurementKind( m_measurementKind() ) );
        setColor( getColorForMeasurementKind( measurementKind() ) );
        setSymbolEdgeColor( getColorForMeasurementKind( measurementKind() ) );
        setLineStyle( RiuQwtPlotCurveDefines::LineStyleEnum::STYLE_NONE );

        RiaDefines::DepthUnitType displayUnit = RiaDefines::DepthUnitType::UNIT_METER;
        if ( wellLogPlot )
        {
            displayUnit = wellLogPlot->depthUnit();
        }

        RiaDefines::DepthTypeEnum depthType = RiaDefines::DepthTypeEnum::MEASURED_DEPTH;
        if ( wellLogPlot && this->curveData()->availableDepthTypes().count( wellLogPlot->depthType() ) )
        {
            depthType = wellLogPlot->depthType();
        }

        bool useLogarithmicScale = false;
        m_plotCurve->setSamplesFromXValuesAndYValues( this->curveData()->propertyValuesByIntervals(),
                                                      this->curveData()->depthValuesByIntervals( depthType, displayUnit ),
                                                      useLogarithmicScale );
        m_plotCurve->setLineSegmentStartStopIndices( this->curveData()->polylineStartStopIndices() );
    }

    this->RimPlotCurve::updateCurvePresentation( updateParentPlot );

    if ( updateParentPlot )
    {
        updateZoomInParentPlot();
    }

    if ( m_parentPlot )
    {
        m_parentPlot->replot();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellMeasurementCurve::setWellPath( RimWellPath* wellPath )
{
    m_wellPath = wellPath;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPath* RimWellMeasurementCurve::wellPath() const
{
    return m_wellPath;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellMeasurementCurve::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                                const QVariant&            oldValue,
                                                const QVariant&            newValue )
{
    RimWellLogCurve::fieldChangedByUi( changedField, oldValue, newValue );

    if ( changedField == &m_wellPath || changedField == &m_measurementKind )
    {
        this->loadDataAndUpdate( true );
    }

    if ( m_parentPlot ) m_parentPlot->replot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellMeasurementCurve::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    RimPlotCurve::updateOptionSensitivity();

    caf::PdmUiGroup* curveDataGroup = uiOrdering.addNewGroup( "Curve Data" );
    curveDataGroup->add( &m_wellPath );
    curveDataGroup->add( &m_measurementKind );

    caf::PdmUiGroup* appearanceGroup = uiOrdering.addNewGroup( "Appearance" );
    RimPlotCurve::appearanceUiOrdering( *appearanceGroup );

    caf::PdmUiGroup* nameGroup = uiOrdering.addNewGroup( "Curve Name" );
    nameGroup->add( &m_showLegend );
    RimPlotCurve::curveNameUiOrdering( *nameGroup );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellMeasurementCurve::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/ )
{
    uiTreeOrdering.skipRemainingChildren( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimWellMeasurementCurve::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    options = RimWellLogCurve::calculateValueOptions( fieldNeedingOptions );
    if ( options.size() > 0 ) return options;

    if ( fieldNeedingOptions == &m_wellPath )
    {
        RimTools::wellPathOptionItems( &options );
    }
    else if ( fieldNeedingOptions == &m_measurementKind )
    {
        RimWellPathCollection* wellPathCollection = nullptr;
        if ( m_wellPath )
        {
            m_wellPath->firstAncestorOrThisOfTypeAsserted( wellPathCollection );
        }

        std::set<QString> kindNames;

        if ( wellPathCollection )
        {
            const RimWellMeasurementCollection* measurementCollection = wellPathCollection->measurementCollection();
            for ( RimWellMeasurement* measurement : measurementCollection->measurements() )
            {
                if ( measurement->wellName() == m_wellPath->name() )
                {
                    kindNames.insert( measurement->kind() );
                }
            }
        }

        options.push_back( caf::PdmOptionItemInfo( "None", "None" ) );

        for ( const auto& kind : kindNames )
        {
            options.push_back( caf::PdmOptionItemInfo( kind, kind ) );
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellMeasurementCurve::createCurveAutoName()
{
    if ( m_wellPath )
    {
        return measurementKind();
    }

    return "Empty curve";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellMeasurementCurve::wellLogChannelUiName() const
{
    // Does not really make sense for measurement curve.
    return QString( "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellMeasurementCurve::wellLogChannelUnits() const
{
    return RiaWellLogUnitTools<double>::noUnitString();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellMeasurementCurve::wellName() const
{
    if ( m_wellPath )
    {
        return m_wellPath->name();
    }

    return QString( "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellMeasurementCurve::measurementKind() const
{
    if ( !m_measurementKind().isEmpty() )
    {
        return m_measurementKind();
    }

    return QString( "None" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellMeasurementCurve::setMeasurementKind( const QString& measurementKind )
{
    m_measurementKind = measurementKind;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotCurveSymbol::PointSymbolEnum RimWellMeasurementCurve::getSymbolForMeasurementKind( const QString& measurementKind )
{
    std::map<QString, RiuPlotCurveSymbol::PointSymbolEnum> symbolTable;
    symbolTable["XLOT"] = RiuPlotCurveSymbol::SYMBOL_RECT;
    symbolTable["LOT"]  = RiuPlotCurveSymbol::SYMBOL_TRIANGLE;
    symbolTable["FIT"]  = RiuPlotCurveSymbol::SYMBOL_DIAMOND;
    symbolTable["MCF"]  = RiuPlotCurveSymbol::SYMBOL_ELLIPSE;
    symbolTable["MNF"]  = RiuPlotCurveSymbol::SYMBOL_ELLIPSE;
    symbolTable["TH"]   = RiuPlotCurveSymbol::SYMBOL_STAR1;
    symbolTable["LE"]   = RiuPlotCurveSymbol::SYMBOL_STAR2;
    symbolTable["BA"]   = RiuPlotCurveSymbol::SYMBOL_STAR1;
    symbolTable["CORE"] = RiuPlotCurveSymbol::SYMBOL_RECT;
    symbolTable["PPG"]  = RiuPlotCurveSymbol::SYMBOL_RECT;

    auto it = symbolTable.find( measurementKind );
    if ( it != symbolTable.end() )
        return it->second;
    else
        return RiuPlotCurveSymbol::SYMBOL_CROSS;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Color3f RimWellMeasurementCurve::getColorForMeasurementKind( const QString& measurementKind )
{
    if ( measurementKind == "TH" ) return cvf::Color3f::RED;
    if ( measurementKind == "LE" ) return cvf::Color3f::BLUE;
    if ( measurementKind == "BA" ) return cvf::Color3f::GREEN;
    if ( measurementKind == "CORE" ) return cvf::Color3f::BLACK;

    return cvf::Color3f::CRIMSON;
}
