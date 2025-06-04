/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025 Equinor ASA
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

#include "RimHistogramCurve.h"

#include "RiaColorTables.h"
#include "RiaCurveMerger.h"
#include "RiaGuiApplication.h"
#include "RiaLogging.h"
#include "RiaPlotDefines.h"
// #include "RiaPreferencesHistogram.h"
#include "RiaQDateTimeTools.h"
#include "RiaResultNames.h"

#include "RimEclipseResultCase.h"
#include "RimHistogramPlot.h"
#// include "RimEnsembleCurveSet.h"
// #include "RimEnsembleCurveSetCollection.h"
// #
// include "RimMultipleHistogramPlotNameHelper.h"
#include "RimPlotAxisProperties.h"
#include "RimProject.h"
// #include "RimHistogramAddress.h"
// #include "RimHistogramCalculationCollection.h"
// #include "RimHistogramCase.h"
// #include "RimHistogramCurveAutoName.h"
// #include "RimHistogramCurveCollection.h"
// #include "RimHistogramEnsemble.h"
// #include "RimHistogramMultiPlot.h"
// #include "RimHistogramPlot.h"
// #include "RimHistogramTimeAxisProperties.h"
#include "RimTools.h"

#include "RiuPlotAxis.h"
#include "RiuPlotMainWindow.h"
#include "RiuQwtPlotCurve.h"
// #include "RiuHistogramVectorSelectionDialog.h"

#include "cafAssert.h"
#include "cafPdmUiComboBoxEditor.h"
#include "cafPdmUiLineEditor.h"
#include "cafPdmUiPushButtonEditor.h"
#include "cafPdmUiTreeOrdering.h"

CAF_PDM_SOURCE_INIT( RimHistogramCurve, "HistogramCurve" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimHistogramCurve::RimHistogramCurve()
{
    CAF_PDM_InitObject( "Histogram Curve", ":/SummaryCurve16x16.png" );

    // Y Values

    // X Values

    CAF_PDM_InitFieldNoDefault( &m_yPlotAxisProperties, "Axis", "Axis" );
    CAF_PDM_InitFieldNoDefault( &m_xPlotAxisProperties, "XAxis", "Axis" );

    setSymbolSkipDistance( 10.0f );
    setLineThickness( 2 );

    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimHistogramCurve::~RimHistogramCurve()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
// void RimHistogramCurve::setTopOrBottomAxisX( RiuPlotAxis plotAxis )
// {
//     CAF_ASSERT( plotAxis.isHorizontal() );

//     RimHistogramPlot* plot = firstAncestorOrThisOfTypeAsserted<RimHistogramPlot>();
//     m_xPlotAxisProperties  = plot->axisPropertiesForPlotAxis( plotAxis );
// }

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotAxis RimHistogramCurve::axisX() const
{
    if ( m_xPlotAxisProperties )
        return m_xPlotAxisProperties->plotAxis();
    else
        return RiuPlotAxis::defaultBottom();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RimHistogramCurve::unitNameY() const
{
    // RifHistogramReaderInterface* reader = valuesHistogramReaderY();
    // if ( reader ) return reader->unitName( histogramAddressY() );

    return "unit y";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RimHistogramCurve::unitNameX() const
{
    // RifHistogramReaderInterface* reader = valuesHistogramReaderX();
    // if ( reader ) return reader->unitName( histogramAddressX() );

    return "unit x";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimHistogramCurve::valuesY() const
{
    // RifHistogramReaderInterface* reader = valuesHistogramReaderY();

    // if ( !reader ) return {};

    // RifEclipseHistogramAddress addr = m_yValuesHistogramAddress()->address();

    // auto [isOk, values] = reader->values( addr );
    // if ( values.empty() ) return values;

    // RimHistogramPlot* plot         = firstAncestorOrThisOfTypeAsserted<RimHistogramPlot>();
    // bool              isNormalized = plot->isNormalizationEnabled();
    // if ( isNormalized )
    // {
    //     auto   minMaxPair = std::minmax_element( values.begin(), values.end() );
    //     double min        = *minMaxPair.first;
    //     double max        = *minMaxPair.second;
    //     double range      = max - min;

    //     for ( double& v : values )
    //     {
    //         v = ( v - min ) / range;
    //     }
    // }

    std::vector<double> values = { 1.2, 3.4, 5.6, 7.4, 9.9 };

    return values;
}

// //--------------------------------------------------------------------------------------------------
// ///
// //--------------------------------------------------------------------------------------------------
// std::vector<double> RimHistogramCurve::errorValuesY() const
// {
//     RifHistogramReaderInterface* reader = valuesHistogramReaderY();
//     if ( !reader ) return {};

//     RifEclipseHistogramAddress addr = errorHistogramAddressY();
//     if ( !reader->hasAddress( addr ) ) return {};

//     auto [isOk, values] = reader->values( addr );
//     return values;
// }

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimHistogramCurve::valuesX() const
{
    std::vector<double> values = { 1.2, 3.4, 5.6, 7.4, 9.9 };
    return values;
}

// //--------------------------------------------------------------------------------------------------
// ///
// //--------------------------------------------------------------------------------------------------
// RifEclipseHistogramAddressDefines::CurveType RimHistogramCurve::curveType() const
// {
//     return m_yCurveType();
// }

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
// void RimHistogramCurve::setAxisTypeX( RiaDefines::HorizontalAxisType axisType )
// {
//     m_xAxisType = axisType;
// }

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
// RiaDefines::HorizontalAxisType RimHistogramCurve::axisTypeX() const
// {
//     return m_xAxisType();
// }

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramCurve::setLeftOrRightAxisY( RiuPlotAxis plotAxis )
{
    CAF_ASSERT( plotAxis.isVertical() );

    RimHistogramPlot* plot = firstAncestorOrThisOfTypeAsserted<RimHistogramPlot>();
    m_yPlotAxisProperties  = plot->axisPropertiesForPlotAxis( plotAxis );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotAxis RimHistogramCurve::axisY() const
{
    if ( m_yPlotAxisProperties )
        return m_yPlotAxisProperties->plotAxis();
    else
        return RiuPlotAxis::defaultLeft();
}

// //--------------------------------------------------------------------------------------------------
// ///
// //--------------------------------------------------------------------------------------------------
// bool RimHistogramCurve::isEnsembleCurve() const
// {
//     auto curveSet = firstAncestorOrThisOfType<RimEnsembleCurveSet>();
//     return curveSet != nullptr;
// }

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimHistogramCurve::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options = RimPlotCurve::calculateValueOptions( fieldNeedingOptions );
    if ( !options.isEmpty() ) return options;

    // auto createOptionsForHistogramCase = []( RimHistogramCase* histogramCase, QList<caf::PdmOptionItemInfo>& options )
    // {
    //     RimProject*                    proj  = RimProject::current();
    //     std::vector<RimHistogramCase*> cases = proj->allHistogramCases();

    //     options = RiaHistogramTools::optionsForHistogramCases( cases );

    //     if ( !options.empty() )
    //     {
    //         options.push_front( caf::PdmOptionItemInfo( "None", nullptr ) );
    //     }
    // };

    else if ( fieldNeedingOptions == &m_yPlotAxisProperties )
    {
        auto plot = firstAncestorOrThisOfTypeAsserted<RimHistogramPlot>();

        for ( auto axis : plot->plotAxes( RimPlotAxisProperties::Orientation::VERTICAL ) )
        {
            options.push_back( caf::PdmOptionItemInfo( axis->objectName(), axis ) );
        }
    }
    else if ( fieldNeedingOptions == &m_xPlotAxisProperties )
    {
        auto plot = firstAncestorOrThisOfTypeAsserted<RimHistogramPlot>();

        for ( auto axis : plot->plotAxes( RimPlotAxisProperties::Orientation::HORIZONTAL ) )
        {
            options.push_back( caf::PdmOptionItemInfo( axis->objectName(), axis ) );
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimHistogramCurve::createCurveAutoName()
{
    // const RimHistogramNameHelper*              currentPlotNameHelper = nullptr;
    // std::vector<const RimHistogramNameHelper*> plotNameHelpers;
    // {
    //     auto plot = firstAncestorOrThisOfType<RimHistogramPlot>();
    //     if ( plot )
    //     {
    //         currentPlotNameHelper = plot->plotTitleHelper();
    //         if ( currentPlotNameHelper ) plotNameHelpers.push_back( currentPlotNameHelper );
    //     }
    // }
    // {
    //     RimHistogramMultiPlot* histogramMultiPlot = firstAncestorOrThisOfType<RimHistogramMultiPlot>();
    //     if ( histogramMultiPlot )
    //     {
    //         auto nameHelper = histogramMultiPlot->nameHelper();
    //         if ( nameHelper ) plotNameHelpers.push_back( nameHelper );
    //     }
    // }

    // RimMultiHistogramPlotNameHelper multiNameHelper( plotNameHelpers );

    QString curveName = "auto name"; // m_curveNameConfig->curveName( curveAddress(), currentPlotNameHelper, &multiNameHelper );
    if ( curveName.isEmpty() )
    {
        curveName = "Curve Name Placeholder";
    }

    return curveName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramCurve::updateZoomInParentPlot()
{
    auto plot = firstAncestorOrThisOfTypeAsserted<RimHistogramPlot>();

    plot->updatePlotWidgetFromAxisRanges();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramCurve::onLoadDataAndUpdate( bool updateParentPlot )
{
    RimPlotCurve::updateCurvePresentation( updateParentPlot );

    updateConnectedEditors();

    // setZIndexFromCurveInfo();

    if ( isChecked() )
    {
        std::vector<double> curveValuesX = valuesX();
        std::vector<double> curveValuesY = valuesY();

        auto plot                = firstAncestorOrThisOfTypeAsserted<RimHistogramPlot>();
        bool useLogarithmicScale = plot->isLogarithmicScaleEnabled( axisY() );

        setSamplesFromXYValues( curveValuesX, curveValuesY, useLogarithmicScale );

        // bool shouldPopulateViewWithEmptyData = false;

        // if ( m_xAxisType == RiaDefines::HorizontalAxisType::HISTOGRAM_VECTOR )
        // {
        //     if ( m_xValuesHistogramAddress()->address().isStatistics() )
        //     {
        //         // Read x and y values from ensemble statistics (not time steps are read)
        //         if ( RifHistogramReaderInterface* reader = valuesHistogramReaderX() )
        //         {
        //             auto [isOkX, curveValuesX] = reader->values( m_xValuesHistogramAddress->address() );
        //             auto [isOkY, curveValuesY] = reader->values( m_yValuesHistogramAddress->address() );
        //             setSamplesFromXYValues( curveValuesX, curveValuesY, useLogarithmicScale );
        //         }
        //     }
        //     else
        //     {
        //         auto curveTimeStepsX = timeStepsX();
        //         auto curveTimeStepsY = timeStepsY();

        //         if ( curveValuesY.empty() || curveValuesX.empty() )
        //         {
        //             shouldPopulateViewWithEmptyData = true;
        //         }
        //         else
        //         {
        //             RiaTimeHistoryCurveMerger curveMerger;
        //             curveMerger.addCurveData( curveTimeStepsX, curveValuesX );
        //             curveMerger.addCurveData( curveTimeStepsY, curveValuesY );

        //             bool includeValuesFromPartialCurves = true;
        //             curveMerger.computeInterpolatedValues( includeValuesFromPartialCurves );

        //             if ( !curveMerger.allXValues().empty() )
        //             {
        //                 setSamplesFromXYValues( curveMerger.interpolatedYValuesForAllXValues( 0 ),
        //                                         curveMerger.interpolatedYValuesForAllXValues( 1 ),
        //                                         useLogarithmicScale );
        //             }
        //             else
        //             {
        //                 shouldPopulateViewWithEmptyData = true;
        //             }
        //         }
        //     }
        // }
        // else
        // {
        //     std::vector<time_t> curveTimeStepsY = timeStepsY();
        //     if ( plot->timeAxisProperties() && !curveTimeStepsY.empty() && curveTimeStepsY.size() == curveValuesY.size() )
        //     {
        //         if ( plot->timeAxisProperties()->timeMode() == RimHistogramTimeAxisProperties::DATE )
        //         {
        //             RifEclipseHistogramAddress errAddress;
        //             std::vector<double>        errValues;

        //             if ( auto reader = valuesHistogramReaderY() )
        //             {
        //                 errAddress = reader->errorAddress( histogramAddressY() );
        //                 errValues  = reader->values( errAddress ).second;
        //             }

        //             if ( errAddress.isValid() )
        //             {
        //                 auto timeSteps = RiuQwtPlotCurve::fromTime_t( curveTimeStepsY );

        //                 if ( !errValues.empty() )
        //                 {
        //                     setSamplesFromXYErrorValues( timeSteps, curveValuesY, errValues, useLogarithmicScale );
        //                 }
        //                 else
        //                 {
        //                     setSamplesFromXYValues( timeSteps, curveValuesY, useLogarithmicScale );
        //                 }
        //             }
        //             else
        //             {
        //                 if ( m_yValuesResampling() != RiaDefines::DateTimePeriod::NONE )
        //                 {
        //                     auto [resampledTimeSteps, resampledValues] =
        //                         RiaHistogramTools::resampledValuesForPeriod( m_yValuesHistogramAddress->address(),
        //                                                                      curveTimeStepsY,
        //                                                                      curveValuesY,
        //                                                                      m_yValuesResampling() );

        //                     if ( !resampledValues.empty() && !resampledTimeSteps.empty() )
        //                     {
        //                         // When values are resampled, each time step value is reported at the end of each
        //                         // resampling period. Insert a duplicate of the first value at the start of the time
        //                         // series to make curve start at the very first reported time step.

        //                         resampledTimeSteps.insert( resampledTimeSteps.begin(), curveTimeStepsY.front() );
        //                         resampledValues.insert( resampledValues.begin(), resampledValues.front() );

        //                         setSamplesFromTimeTAndYValues( resampledTimeSteps, resampledValues, useLogarithmicScale );
        //                     }
        //                 }
        //                 else
        //                 {
        //                     setSamplesFromTimeTAndYValues( curveTimeStepsY, curveValuesY, useLogarithmicScale );
        //                 }
        //             }
        //         }
        //         else
        //         {
        //             double timeScale = plot->timeAxisProperties()->fromTimeTToDisplayUnitScale();

        //             std::vector<double> timeFromSimulationStart;
        //             if ( !curveTimeStepsY.empty() )
        //             {
        //                 time_t startDate = curveTimeStepsY[0];
        //                 for ( const auto& date : curveTimeStepsY )
        //                 {
        //                     timeFromSimulationStart.push_back( timeScale * ( date - startDate ) );
        //                 }
        //             }

        //             setSamplesFromXYValues( timeFromSimulationStart, curveValuesY, useLogarithmicScale );
        //         }
        //     }
        //     else
        //     {
        //         shouldPopulateViewWithEmptyData = true;
        //     }

        // updateTimeAnnotations();
    }

    // if ( shouldPopulateViewWithEmptyData )
    // {
    //     setSamplesFromXYValues( std::vector<double>(), std::vector<double>(), useLogarithmicScale );
    // }

    if ( updateParentPlot && hasParentPlot() )
    {
        updateZoomInParentPlot();
        replotParentPlot();
    }

    if ( updateParentPlot ) updatePlotAxis();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramCurve::updateLegendsInPlot()
{
    auto plot = firstAncestorOrThisOfTypeAsserted<RimHistogramPlot>();
    plot->updateLegend();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramCurve::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/ )
{
    RimPlotCurve::defineUiTreeOrdering( uiTreeOrdering, uiConfigName );

    caf::IconProvider iconProvider = uiIconProvider();
    if ( !iconProvider.valid() ) return;

    // RimHistogramCurveCollection* coll = firstAncestorOrThisOfType<RimHistogramCurveCollection>();
    // if ( coll && coll->curveForSourceStepping() == this )
    // {
    //     iconProvider.setOverlayResourceString( ":/StepUpDownCorner16x16.png" );
    // }
    // else
    // {
    //     iconProvider.setOverlayResourceString( "" );
    // }

    setUiIcon( iconProvider );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimHistogramCurve::computeCurveZValue()
{
    // auto sumAddr = histogramAddressY();
    // auto sumCase = histogramCaseY();

    double zOrder = 0.0;

    // if ( sumCase && sumAddr.isValid() )
    // {
    //     if ( sumCase->isObservedData() )
    //     {
    //         zOrder = RiuQwtPlotCurveDefines::zDepthForIndex( RiuQwtPlotCurveDefines::ZIndex::Z_SINGLE_CURVE_OBSERVED );
    //     }
    //     else if ( sumAddr.isStatistics() )
    //     {
    //         zOrder = RiuQwtPlotCurveDefines::zDepthForIndex( RiuQwtPlotCurveDefines::ZIndex::Z_ENSEMBLE_STAT_CURVE );
    //     }
    //     else if ( firstAncestorOrThisOfType<RimEnsembleCurveSetCollection>() )
    //     {
    //         zOrder = RiuQwtPlotCurveDefines::zDepthForIndex( RiuQwtPlotCurveDefines::ZIndex::Z_ENSEMBLE_CURVE );
    //     }
    //     else
    //     {
    //         zOrder = RiuQwtPlotCurveDefines::zDepthForIndex( RiuQwtPlotCurveDefines::ZIndex::Z_SINGLE_CURVE_NON_OBSERVED );
    //     }
    // }

    // if ( m_isTopZWithinCategory )
    // {
    //     zOrder += 1.0;
    // }

    return zOrder;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramCurve::defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    // if ( &m_yPushButtonSelectHistogramAddress == field || &m_xPushButtonSelectHistogramAddress == field )
    // {
    //     caf::PdmUiPushButtonEditorAttribute* attrib = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*>( attribute );
    //     if ( attrib )
    //     {
    //         attrib->m_buttonText = "...";
    //     }
    // }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramCurve::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    RimPlotCurve::updateFieldUiState();

    {
        QString          curveDataGroupName = "Histogram Vector";
        caf::PdmUiGroup* curveDataGroup     = uiOrdering.addNewGroupWithKeyword( curveDataGroupName, "curveDataGroupName" );
        curveDataGroup->add( &m_yPlotAxisProperties, { .newRow = true, .totalColumnSpan = 3, .leftLabelColumnSpan = 1 } );

        // caf::PdmUiGroup* detailGroup = curveDataGroup->addNewGroup( "Advanced Properties" );
        // detailGroup->setCollapsedByDefault();
        // // detailGroup->add( &m_yCurveTypeMode );
        // // detailGroup->add( &m_yCurveType );
        // m_yCurveType.uiCapability()->setUiReadOnly( m_yCurveTypeMode() == RiaDefines::HistogramCurveTypeMode::AUTO );
    }

    // if ( m_showXAxisGroup )
    // {
    //     caf::PdmUiGroup* curveDataGroup = uiOrdering.addNewGroup( "Histogram Vector X Axis" );
    //     curveDataGroup->add( &m_xAxisType, { .newRow = true, .totalColumnSpan = 3, .leftLabelColumnSpan = 1 } );
    //     curveDataGroup->add( &m_xValuesHistogramCase, { .newRow = true, .totalColumnSpan = 3, .leftLabelColumnSpan = 1 } );
    //     curveDataGroup->add( &m_xValuesHistogramAddressUiField, { .newRow = true, .totalColumnSpan = 2, .leftLabelColumnSpan = 1 } );
    //     curveDataGroup->add( &m_xPushButtonSelectHistogramAddress, { .newRow = false, .totalColumnSpan = 1, .leftLabelColumnSpan = 0 } );
    //     curveDataGroup->add( &m_xPlotAxisProperties, { .newRow = true, .totalColumnSpan = 3, .leftLabelColumnSpan = 1 } );
    // }

    caf::PdmUiGroup* stackingGroup = uiOrdering.addNewGroup( "Stacking" );
    RimStackablePlotCurve::stackingUiOrdering( *stackingGroup );

    caf::PdmUiGroup* appearanceGroup = uiOrdering.addNewGroup( "Appearance" );
    RimPlotCurve::appearanceUiOrdering( *appearanceGroup );

    caf::PdmUiGroup* nameGroup = uiOrdering.addNewGroup( "Curve Name" );
    nameGroup->setCollapsedByDefault();
    nameGroup->add( &m_showLegend );
    RimPlotCurve::curveNameUiOrdering( *nameGroup );

    // if ( m_namingMethod == RiaDefines::ObjectNamingMethod::AUTO )
    // {
    //     m_curveNameConfig->uiOrdering( uiConfigName, *nameGroup );
    // }

    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
// void RimHistogramCurve::appendOptionItemsForHistogramAddresses( QList<caf::PdmOptionItemInfo>* options, RimHistogramCase* histogramCase )
// {
//     if ( histogramCase )
//     {
//         RifHistogramReaderInterface* reader = histogramCase->histogramReader();
//         if ( reader )
//         {
//             const std::set<RifEclipseHistogramAddress> allAddresses = reader->allResultAddresses();

//             for ( auto& address : allAddresses )
//             {
//                 if ( address.isErrorResult() ) continue;

//                 std::string name = address.uiText();
//                 QString     s    = QString::fromStdString( name );
//                 options->push_back( caf::PdmOptionItemInfo( s, QVariant::fromValue( address ) ) );
//             }
//         }

//         options->push_front(
//             caf::PdmOptionItemInfo( RiaResultNames::undefinedResultName(), QVariant::fromValue( RifEclipseHistogramAddress() ) ) );
//     }
// }

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
// void RimHistogramCurve::setZIndexFromCurveInfo()
// {
//     auto zValue = computeCurveZValue();

//     setZOrder( zValue );
// }

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramCurve::updatePlotAxis()
{
    updateYAxisInPlot( axisY() );
    updateXAxisInPlot( axisX() );
}

// //--------------------------------------------------------------------------------------------------
// ///
// //--------------------------------------------------------------------------------------------------
// QString RimHistogramCurve::curveExportDescription( const RifEclipseHistogramAddress& address ) const
// {
//     auto addr = address.isValid() ? address : m_yValuesHistogramAddress->address();

//     RimEnsembleCurveSetCollection* coll = firstAncestorOrThisOfType<RimEnsembleCurveSetCollection>();

//     auto curveSet = coll ? coll->findCurveSetFromPlotCurve( m_plotCurve ) : nullptr;
//     auto group    = curveSet ? curveSet->histogramEnsemble() : nullptr;

//     auto addressUiText = addr.uiText();
//     if ( !m_yValuesHistogramCase() )
//     {
//         return QString::fromStdString( addressUiText );
//     }

//     if ( group && group->isEnsemble() )
//     {
//         return QString( "%1.%2.%3" )
//             .arg( QString::fromStdString( addressUiText ) )
//             .arg( m_yValuesHistogramCase->nativeCaseName() )
//             .arg( group->name() );
//     }

//     return QString( "%1.%2" ).arg( QString::fromStdString( addressUiText ) ).arg( m_yValuesHistogramCase->nativeCaseName() );
// }

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
// void RimHistogramCurve::setCurveAppearanceFromCaseType()
// {
//     if ( m_yValuesHistogramCase )
//     {
//         if ( m_yValuesHistogramCase->isObservedData() )
//         {
//             setLineStyle( RiuQwtPlotCurveDefines::LineStyleEnum::STYLE_NONE );
//             setSymbol( RiuPlotCurveSymbol::SYMBOL_XCROSS );

//             return;
//         }
//     }

//     if ( m_yValuesHistogramAddress && m_yValuesHistogramAddress->address().isHistoryVector() )
//     {
//         RiaPreferencesHistogram* prefs = RiaPreferencesHistogram::current();

//         if ( prefs->defaultHistogramHistoryCurveStyle() == RiaPreferencesHistogram::HistogramHistoryCurveStyleMode::SYMBOLS )
//         {
//             setSymbolEdgeColor( m_curveAppearance->color() );

//             setSymbol( RiuPlotCurveSymbol::SYMBOL_XCROSS );
//             setLineStyle( RiuQwtPlotCurveDefines::LineStyleEnum::STYLE_NONE );
//         }
//         else if ( prefs->defaultHistogramHistoryCurveStyle() ==
//         RiaPreferencesHistogram::HistogramHistoryCurveStyleMode::SYMBOLS_AND_LINES )
//         {
//             setSymbolEdgeColor( m_curveAppearance->color() );

//             setSymbol( RiuPlotCurveSymbol::SYMBOL_XCROSS );
//             setLineStyle( RiuQwtPlotCurveDefines::LineStyleEnum::STYLE_SOLID );
//         }
//         else if ( prefs->defaultHistogramHistoryCurveStyle() == RiaPreferencesHistogram::HistogramHistoryCurveStyleMode::LINES )
//         {
//             setSymbol( RiuPlotCurveSymbol::SYMBOL_NONE );
//             setLineStyle( RiuQwtPlotCurveDefines::LineStyleEnum::STYLE_SOLID );
//         }

//         return;
//     }
// }

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
// void RimHistogramCurve::setDefaultCurveAppearance()
// {
//     auto plot = firstAncestorOrThisOfType<RimHistogramPlot>();
//     if ( plot ) plot->applyDefaultCurveAppearances( { this } );
// }

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
// void RimHistogramCurve::setAsTopZWithinCategory( bool enable )
// {
//     m_isTopZWithinCategory = enable;
// }

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramCurve::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    RimStackablePlotCurve::fieldChangedByUi( changedField, oldValue, newValue );

    auto plot = firstAncestorOrThisOfTypeAsserted<RimHistogramPlot>();

    bool loadAndUpdate = false;
    // bool crossPlotTestForMatchingTimeSteps = false;

    //  if ( &m_xAxisType == changedField )
    // {
    //     if ( m_xAxisType() == RiaDefines::HorizontalAxisType::TIME )
    //     {
    //         m_xPlotAxisProperties = plot->timeAxisProperties();
    //     }
    //     else if ( m_xAxisType() == RiaDefines::HorizontalAxisType::HISTOGRAM_VECTOR )
    //     {
    //         if ( !m_xValuesHistogramCase() )
    //         {
    //             m_xValuesHistogramCase = m_yValuesHistogramCase();
    //             m_xValuesHistogramAddress->setAddress( m_yValuesHistogramAddress()->address() );
    //         }

    //         plot->findOrAssignPlotAxisX( this );
    //     }
    //     plot->updateAxes();
    //     plot->updatePlotTitle();
    //     plot->zoomAll();
    //     loadAndUpdate = true;
    // }
    if ( &m_showCurve == changedField )
    {
        plot->updateAxes();
        plot->updatePlotTitle();
    }
    else if ( changedField == &m_yPlotAxisProperties )
    {
        updateYAxisInPlot( axisY() );
        plot->updateAxes();
        dataChanged.send();
    }
    else if ( changedField == &m_xPlotAxisProperties )
    {
        updateXAxisInPlot( axisX() );
        plot->updateAxes();
        dataChanged.send();
    }

    if ( loadAndUpdate )
    {
        loadAndUpdateDataAndPlot();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramCurve::loadAndUpdateDataAndPlot()
{
    loadDataAndUpdate( true );

    auto plot = firstAncestorOrThisOfTypeAsserted<RimHistogramPlot>();

    plot->updateAxes();
    plot->updatePlotTitle();
    plot->updateConnectedEditors();

    RiuPlotMainWindow* mainPlotWindow = RiaGuiApplication::instance()->mainPlotWindow();
    mainPlotWindow->updateMultiPlotToolBar();

    dataChanged.send();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramCurve::updateLegendEntryVisibilityNoPlotUpdate()
{
    if ( !m_plotCurve ) return;
    // if ( firstAncestorOrThisOfType<RimEnsembleCurveSet>() ) return;

    bool showLegendInPlot = m_showLegend();
    // if ( auto histogramPlot = firstAncestorOrThisOfType<RimHistogramPlot>() )
    // {
    //     bool anyCalculated = false;
    //     for ( const auto c : histogramPlot->histogramCurves() )
    //     {
    //         if ( c->histogramAddressY().isCalculated() )
    //         {
    //             // Never hide the legend for calculated curves, as the curve legend is used to
    //             // show some essential auto generated data
    //             anyCalculated = true;
    //         }
    //     }

    //     auto isMultiPlot = ( firstAncestorOrThisOfType<RimMultiPlot>() != nullptr );

    //     if ( !anyCalculated && isMultiPlot && histogramPlot->ensembleCurveSetCollection()->curveSets().empty() &&
    //          histogramPlot->curveCount() == 1 )
    //     {
    //         // Disable display of legend if the histogram plot has only one single curve
    //         showLegendInPlot = false;
    //     }
    // }

    m_plotCurve->setVisibleInLegend( showLegendInPlot );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimHistogramCurve::canCurveBeAttached() const
{
    if ( !RimPlotCurve::canCurveBeAttached() ) return false;

    bool isVisibleInPossibleParent = true;

    // auto histogramCurveCollection = firstAncestorOrThisOfType<RimHistogramCurveCollection>();
    // if ( histogramCurveCollection ) isVisibleInPossibleParent = histogramCurveCollection->isCurvesVisible();

    // auto ensembleCurveSet = firstAncestorOrThisOfType<RimEnsembleCurveSet>();
    // if ( ensembleCurveSet ) isVisibleInPossibleParent = ensembleCurveSet->isCurvesVisible();

    return isVisibleInPossibleParent;
}
