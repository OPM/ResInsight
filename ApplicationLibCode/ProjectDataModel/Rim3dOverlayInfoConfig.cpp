/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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

#include "Rim3dOverlayInfoConfig.h"

#include "RiaLogging.h"
#include "RiaPreferences.h"
#include "RiaQDateTimeTools.h"

#include "RicGridStatisticsDialog.h"

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigFemPartCollection.h"
#include "RigFemPartResultsCollection.h"
#include "RigFemResultAddress.h"
#include "RigFlowDiagResults.h"
#include "RigGeoMechCaseData.h"
#include "RigMainGrid.h"

#include "Rim2dIntersectionView.h"
#include "Rim2dIntersectionViewCollection.h"
#include "Rim3dView.h"
#include "RimCase.h"
#include "RimCellEdgeColors.h"
#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseContourMapProjection.h"
#include "RimEclipseContourMapView.h"
#include "RimEclipseFaultColors.h"
#include "RimEclipsePropertyFilterCollection.h"
#include "RimEclipseStatisticsCase.h"
#include "RimEclipseView.h"
#include "RimFaultInViewCollection.h"
#include "RimGeoMechCase.h"
#include "RimGeoMechContourMapProjection.h"
#include "RimGeoMechContourMapView.h"
#include "RimGeoMechResultDefinition.h"
#include "RimGeoMechView.h"
#include "RimGridView.h"
#include "RimReservoirCellResultsStorage.h"
#include "RimSeismicData.h"
#include "RimSeismicView.h"
#include "RimSimWellInViewCollection.h"

#include "RiuViewer.h"

#include "caf.h"

#include <QLocale>

CAF_PDM_SOURCE_INIT( Rim3dOverlayInfoConfig, "View3dOverlayInfoConfig" );
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Rim3dOverlayInfoConfig::Rim3dOverlayInfoConfig()
{
    CAF_PDM_InitObject( "Info Box", ":/InfoBox16x16.png" );

    CAF_PDM_InitField( &m_active, "Active", true, "Active" );
    m_active.uiCapability()->setUiHidden( true );
    m_active = RiaPreferences::current()->showInfoBox();

    CAF_PDM_InitField( &m_showAnimProgress, "ShowAnimProgress", true, "Animation progress" );
    CAF_PDM_InitField( &m_showCaseInfo, "ShowInfoText", true, "Case Info" );
    CAF_PDM_InitField( &m_showResultInfo, "ShowResultInfo", true, "Result Info" );
    CAF_PDM_InitField( &m_showHistogram, "ShowHistogram", true, "Histogram" );
    CAF_PDM_InitField( &m_showVolumeWeightedMean, "ShowVolumeWeightedMean", true, "Mobile Volume Weighted Mean" );
    CAF_PDM_InitField( &m_showVersionInfo, "ShowVersionInfo", true, "Version Info" );

    caf::AppEnum<RimHistogramCalculator::StatisticsTimeRangeType> defaultTimeRange =
        RimHistogramCalculator::StatisticsTimeRangeType::CURRENT_TIMESTEP;
    CAF_PDM_InitField( &m_statisticsTimeRange, "StatisticsTimeRange", defaultTimeRange, "Statistics Time Range" );

    caf::AppEnum<RimHistogramCalculator::StatisticsCellRangeType> defaultCellRange =
        RimHistogramCalculator::StatisticsCellRangeType::VISIBLE_CELLS;
    CAF_PDM_InitField( &m_statisticsCellRange, "StatisticsCellRange", defaultCellRange, "Statistics Cell Range" );

    m_histogramCalculator.reset( new RimHistogramCalculator );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Rim3dOverlayInfoConfig::~Rim3dOverlayInfoConfig()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dOverlayInfoConfig::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( hasInvalidStatisticsCombination() )
    {
        displayPropertyFilteredStatisticsMessage( false );
        if ( changedField == &m_statisticsTimeRange )
            m_statisticsTimeRange = RimHistogramCalculator::StatisticsTimeRangeType::CURRENT_TIMESTEP;
        if ( changedField == &m_statisticsCellRange ) m_statisticsCellRange = RimHistogramCalculator::StatisticsCellRangeType::ALL_CELLS;
    }

    if ( changedField == &m_showResultInfo )
    {
        if ( !m_showResultInfo() )
        {
            m_showVolumeWeightedMean = false;
            m_showVolumeWeightedMean.uiCapability()->setUiReadOnly( true );
        }
        else
        {
            m_showVolumeWeightedMean = true;
            m_showVolumeWeightedMean.uiCapability()->setUiReadOnly( false );
        }
    }

    update3DInfo();

    if ( m_viewDef && m_viewDef->viewer() )
    {
        m_viewDef->viewer()->update();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dOverlayInfoConfig::setPosition( cvf::Vec2ui position )
{
    m_position = position;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigHistogramData Rim3dOverlayInfoConfig::histogramData()
{
    auto eclipseView       = dynamic_cast<RimEclipseView*>( m_viewDef.p() );
    auto geoMechView       = dynamic_cast<RimGeoMechView*>( m_viewDef.p() );
    auto eclipseContourMap = dynamic_cast<RimEclipseContourMapView*>( eclipseView );
    auto geoMechContourMap = dynamic_cast<RimGeoMechContourMapView*>( geoMechView );
    auto seismicView       = dynamic_cast<RimSeismicView*>( m_viewDef.p() );

    if ( eclipseContourMap )
        return m_histogramCalculator->histogramData( eclipseContourMap );
    else if ( geoMechContourMap )
        return m_histogramCalculator->histogramData( geoMechContourMap );
    else if ( eclipseView )
        return m_histogramCalculator->histogramData( eclipseView, m_statisticsCellRange(), m_statisticsTimeRange() );
    else if ( geoMechView )
        return m_histogramCalculator->histogramData( geoMechView, m_statisticsCellRange(), m_statisticsTimeRange() );
    else if ( seismicView )
    {
        return seismicView->histogramData();
    }
    return RigHistogramData();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString Rim3dOverlayInfoConfig::timeStepText()
{
    RimEclipseView* eclipseView = dynamic_cast<RimEclipseView*>( m_viewDef.p() );
    RimGeoMechView* geoMechView = dynamic_cast<RimGeoMechView*>( m_viewDef.p() );

    if ( eclipseView ) return timeStepText( eclipseView );
    if ( geoMechView ) return timeStepText( geoMechView );
    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString Rim3dOverlayInfoConfig::caseInfoText()
{
    auto eclipseView = dynamic_cast<RimEclipseView*>( m_viewDef.p() );
    if ( eclipseView ) return caseInfoText( eclipseView );

    auto geoMechView = dynamic_cast<RimGeoMechView*>( m_viewDef.p() );
    if ( geoMechView ) return caseInfoText( geoMechView );

    auto seisView = dynamic_cast<RimSeismicView*>( m_viewDef.p() );
    if ( seisView ) return caseInfoText( seisView );

    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString Rim3dOverlayInfoConfig::resultInfoText( const RigHistogramData& histData )
{
    auto eclipseView = dynamic_cast<RimEclipseView*>( m_viewDef.p() );
    auto geoMechView = dynamic_cast<RimGeoMechView*>( m_viewDef.p() );

    if ( eclipseView ) return resultInfoText( histData, eclipseView, m_showVolumeWeightedMean() );
    if ( geoMechView ) return resultInfoText( histData, geoMechView );
    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString Rim3dOverlayInfoConfig::sampleCountText( const std::vector<size_t>& histogram )
{
    size_t sampleCount = std::accumulate( histogram.begin(), histogram.end(), size_t( 0 ) );

    QLocale localeWithSpaceAsGroupSeparator( caf::norwegianLocale() );
    QString text = localeWithSpaceAsGroupSeparator.toString( (qulonglong)sampleCount );

    return QString( "<br><b>Sample Count:</b> %1" ).arg( text );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicGridStatisticsDialog* Rim3dOverlayInfoConfig::getOrCreateGridStatisticsDialog()
{
    if ( !m_gridStatisticsDialog )
    {
        m_gridStatisticsDialog.reset( new RicGridStatisticsDialog( nullptr ) );
    }
    CVF_ASSERT( m_gridStatisticsDialog );
    return m_gridStatisticsDialog.get();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QImage Rim3dOverlayInfoConfig::statisticsDialogScreenShotImage()
{
    if ( getOrCreateGridStatisticsDialog()->isVisible() )
    {
        return getOrCreateGridStatisticsDialog()->screenShotImage();
    }
    return QImage();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool Rim3dOverlayInfoConfig::showAnimProgress() const
{
    return m_showAnimProgress;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool Rim3dOverlayInfoConfig::showCaseInfo() const
{
    return m_showCaseInfo;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool Rim3dOverlayInfoConfig::showResultInfo() const
{
    return m_showResultInfo;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool Rim3dOverlayInfoConfig::isActive() const
{
    return m_active;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool Rim3dOverlayInfoConfig::showVersionInfo() const
{
    return m_showVersionInfo();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString Rim3dOverlayInfoConfig::caseInfoText( RimEclipseView* eclipseView )
{
    QString infoText;

    if ( eclipseView && eclipseView->eclipseCase() )
    {
        QString caseName = eclipseView->eclipseCase()->caseUserDescription();

        QLocale localeWithSpaceAsGroupSeparator( caf::norwegianLocale() );

        RimEclipseContourMapView* contourMap = dynamic_cast<RimEclipseContourMapView*>( eclipseView );
        if ( contourMap && contourMap->contourMapProjection() )
        {
            QString   totCellCount        = localeWithSpaceAsGroupSeparator.toString( contourMap->contourMapProjection()->numberOfCells() );
            cvf::uint validCellCount      = contourMap->contourMapProjection()->numberOfValidCells();
            QString   activeCellCountText = localeWithSpaceAsGroupSeparator.toString( validCellCount );
            QString   aggregationType     = contourMap->contourMapProjection()->resultAggregationText();
            QString   weightingParameterString;
            if ( contourMap->contourMapProjection()->weightingParameter() != "None" )
            {
                weightingParameterString += QString( " (Weight: %1)" ).arg( contourMap->contourMapProjection()->weightingParameter() );
            }

            infoText += QString( "<p><b>-- Contour Map: %1 --</b><p>  "
                                 "<b>Sample Count. Total:</b> %2 <b>Valid Results:</b> %3 <br>"
                                 "<b>Projection Type:</b> %4%5<br>" )
                            .arg( caseName, totCellCount, activeCellCountText, aggregationType, weightingParameterString );
        }
        else if ( eclipseView->mainGrid() )
        {
            QString totCellCount =
                localeWithSpaceAsGroupSeparator.toString( static_cast<int>( eclipseView->mainGrid()->globalCellArray().size() ) );

            size_t mxActCellCount =
                eclipseView->eclipseCase()->eclipseCaseData()->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL )->reservoirActiveCellCount();
            size_t frActCellCount = eclipseView->eclipseCase()
                                        ->eclipseCaseData()
                                        ->activeCellInfo( RiaDefines::PorosityModelType::FRACTURE_MODEL )
                                        ->reservoirActiveCellCount();

            QString activeCellCountText;
            if ( frActCellCount > 0 ) activeCellCountText += "Matrix : ";
            activeCellCountText += localeWithSpaceAsGroupSeparator.toString( static_cast<int>( mxActCellCount ) );
            if ( frActCellCount > 0 )
                activeCellCountText += " Fracture : " + localeWithSpaceAsGroupSeparator.toString( static_cast<int>( frActCellCount ) );

            QString iSize = QString::number( eclipseView->mainGrid()->cellCountI() );
            QString jSize = QString::number( eclipseView->mainGrid()->cellCountJ() );
            QString kSize = QString::number( eclipseView->mainGrid()->cellCountK() );

            QString zScale = QString::number( eclipseView->scaleZ() );
            infoText += QString( "<p><b>-- %1 --</b><p>  "
                                 "<b>Cell count. Total:</b> %2 <b>Active:</b> %3 <br>"
                                 "<b>Main Grid I,J,K:</b> %4, %5, %6 <b>Z-Scale:</b> %7<br>" )
                            .arg( caseName, totCellCount, activeCellCountText, iSize, jSize, kSize, zScale );
        }
    }

    return infoText;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString Rim3dOverlayInfoConfig::caseInfoText( RimGeoMechView* geoMechView )
{
    QString infoText;

    if ( geoMechView )
    {
        RimGeoMechCase*       geoMechCase = geoMechView->geoMechCase();
        RigGeoMechCaseData*   caseData    = geoMechCase ? geoMechCase->geoMechData() : nullptr;
        RigFemPartCollection* femParts    = caseData ? caseData->femParts() : nullptr;

        if ( femParts )
        {
            QString                   caseName   = geoMechCase->caseUserDescription();
            RimGeoMechContourMapView* contourMap = dynamic_cast<RimGeoMechContourMapView*>( geoMechView );

            if ( contourMap && contourMap->contourMapProjection() )
            {
                QString   totCellCount        = QString::number( contourMap->contourMapProjection()->numberOfCells() );
                cvf::uint validCellCount      = contourMap->contourMapProjection()->numberOfValidCells();
                QString   activeCellCountText = QString::number( validCellCount );
                QString   aggregationType     = contourMap->contourMapProjection()->resultAggregationText();

                infoText += QString( "<p><b>-- Contour Map: %1 --</b><p>  "
                                     "<b>Sample Count. Total:</b> %2 <b>Valid Results:</b> %3 <br>"
                                     "<b>Projection Type:</b> %4<br>" )
                                .arg( caseName, totCellCount, activeCellCountText, aggregationType );
            }
            else
            {
                QString cellCount = QString( "%1" ).arg( femParts->totalElementCount() );
                QString zScale    = QString::number( geoMechView->scaleZ() );

                infoText = QString( "<p><b>-- %1 --</b><p>"
                                    "<b>Cell count:</b> %2 <b>Z-Scale:</b> %3<br>" )
                               .arg( caseName, cellCount, zScale );
            }
        }
    }
    return infoText;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString Rim3dOverlayInfoConfig::caseInfoText( RimSeismicView* seisView )
{
    QString infoText;

    if ( seisView )
    {
        auto seisData = seisView->seismicData();

        if ( seisData )
        {
            QString depthRange = QString( "%1 to %2" ).arg( seisData->zMin() ).arg( seisData->zMax() );
            QString zScale     = QString::number( seisView->scaleZ() );
            QString ilineRange = QString( "%1 to %2" ).arg( seisData->inlineMin() ).arg( seisData->inlineMax() );
            QString xlineRange = QString( "%1 to %2" ).arg( seisData->xlineMin() ).arg( seisData->xlineMax() );
            QString seisName   = QString::fromStdString( seisData->userDescription() );

            infoText = QString( "<p><b>-- %1 --</b><p>"
                                "<b>Depth Range:</b> %2 <b>Z-Scale:</b> %3<br>"
                                "<b>Inline Range:</b> %4 <br>"
                                "<b>Xline Range:</b> %5 <br>" )
                           .arg( seisName, depthRange, zScale, ilineRange, xlineRange );
        }
    }

    return infoText;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString Rim3dOverlayInfoConfig::resultInfoText( const RigHistogramData& histData, RimEclipseView* eclipseView, bool showVolumeWeightedMean )
{
    QString infoText;

    RimEclipseContourMapView* contourMap = dynamic_cast<RimEclipseContourMapView*>( eclipseView );

    if ( contourMap )
    {
        bool isResultsInfoRelevant = contourMap->contourMapProjection()->numberOfValidCells() > 0u;
        if ( isResultsInfoRelevant )
        {
            QString propName      = eclipseView->cellResult()->resultVariableUiShortName();
            QString diffResString = eclipseView->cellResult()->additionalResultText();
            if ( !contourMap->contourMapProjection()->isColumnResult() )
            {
                infoText += QString( "<b>Cell Property:</b> %1<br>" ).arg( propName );
            }
            if ( !diffResString.isEmpty() )
            {
                infoText += QString( "%1<br>" ).arg( diffResString );
            }
            if ( histData.isMinMaxValid() )
            {
                infoText += QString( "<br><b>Statistics:</b> Current Time Step and Visible Cells" );

                infoText += sampleCountText( histData.histogram );

                infoText += QString( "<table border=0 cellspacing=5 >"
                                     "<tr> <td>Min</td> <td>Mean</td> <td>Max</td> </tr>"
                                     "<tr> <td>%1</td>  <td> %2</td> <td>  %3</td> </tr>"
                                     "</table>" )
                                .arg( histData.min )
                                .arg( histData.mean )
                                .arg( histData.max );
            }
        }
    }
    else if ( eclipseView )
    {
        bool isResultsInfoRelevant = eclipseView->cellResult()->hasResult();

        if ( eclipseView->cellResult()->isTernarySaturationSelected() )
        {
            QString propName = eclipseView->cellResult()->resultVariableUiShortName();
            infoText += QString( "<b>Cell Property:</b> %1 " ).arg( propName );
        }

        if ( isResultsInfoRelevant )
        {
            QString propName      = eclipseView->cellResult()->resultVariableUiShortName();
            QString diffResString = eclipseView->cellResult()->additionalResultText();
            QString timeRangeText = m_statisticsTimeRange().uiText();
            if ( eclipseView->cellResult()->isFlowDiagOrInjectionFlooding() )
            {
                timeRangeText = caf::AppEnum<RimHistogramCalculator::StatisticsTimeRangeType>::uiText(
                    RimHistogramCalculator::StatisticsTimeRangeType::CURRENT_TIMESTEP );
            }

            infoText += QString( "<b>Cell Property:</b> %1<br>" ).arg( propName );
            if ( !diffResString.isEmpty() )
            {
                infoText += QString( "%1<br>" ).arg( diffResString );
            }

            const RimSimWellInViewCollection* wellCollection = eclipseView->wellCollection();
            if ( wellCollection && wellCollection->isActive() && wellCollection->isWellDisksVisible() )
            {
                infoText += QString( "<b>Well Disk Property:</b> %1<br>" ).arg( wellCollection->wellDiskPropertyUiText() );
            }

            if ( eclipseView->cellResult()->hasDualPorFractureResult() )
            {
                QString porosityModelText = caf::AppEnum<RiaDefines::PorosityModelType>::uiText( eclipseView->cellResult()->porosityModel() );

                infoText += QString( "<b>Dual Porosity Type:</b> %1<br>" ).arg( porosityModelText );
            }

            if ( histData.isMinMaxValid() )
            {
                infoText += QString( "<br><b>Statistics:</b> " ) + timeRangeText + " and " + m_statisticsCellRange().uiText();

                infoText += sampleCountText( histData.histogram );

                infoText += QString( "<table border=0 cellspacing=5 >"
                                     "<tr> <td>Min</td> <td>P90</td> <td>Mean</td> <td>P10</td> <td>Max</td> <td>Sum</td> </tr>"
                                     "<tr> <td>%1</td>  <td> %2</td> <td>  %3</td> <td> %4</td> <td> %5</td> <td> %6</td> </tr>"
                                     "</table>" )
                                .arg( histData.min )
                                .arg( histData.p90 )
                                .arg( histData.mean )
                                .arg( histData.p10 )
                                .arg( histData.max )
                                .arg( histData.sum );
            }
            if ( eclipseView->faultResultSettings()->hasValidCustomResult() )
            {
                QString faultMapping;
                bool    isShowingGrid = eclipseView->faultCollection()->isGridVisualizationMode();
                if ( !isShowingGrid )
                {
                    if ( eclipseView->faultCollection()->faultResult() == RimFaultInViewCollection::FAULT_BACK_FACE_CULLING )
                    {
                        faultMapping = "Cells behind fault";
                    }
                    else if ( eclipseView->faultCollection()->faultResult() == RimFaultInViewCollection::FAULT_FRONT_FACE_CULLING )
                    {
                        faultMapping = "Cells in front of fault";
                    }
                    else
                    {
                        faultMapping = "Cells in front and behind fault";
                    }
                }
                else
                {
                    faultMapping = "Cells in front and behind fault";
                }

                infoText += QString( "<b>Fault results: </b> %1<br>" ).arg( faultMapping );
                infoText += QString( "<b>Fault Property:</b> %1 <br>" )
                                .arg( eclipseView->faultResultSettings()->customFaultResult()->resultVariableUiShortName() );
            }
        }

        if ( eclipseView->cellEdgeResult()->hasResult() )
        {
            double  min, max;
            QString cellEdgeName = eclipseView->cellEdgeResult()->resultVariableUiShortName();
            eclipseView->cellEdgeResult()->minMaxCellEdgeValues( min, max );
            infoText += QString( "<b>Cell Edge Property:</b> %1 " ).arg( cellEdgeName );
            infoText += QString( "<table border=0 cellspacing=5 >"
                                 "<tr> <td>Min</td> <td></td> <td></td> <td></td> <td>Max</td> </tr>"
                                 "<tr> <td>%1</td>  <td></td> <td></td> <td></td> <td> %2</td></tr>"
                                 "</table>" )
                            .arg( min )
                            .arg( max );
        }

        if ( showVolumeWeightedMean && histData.weightedMean != HUGE_VAL )
        {
            infoText += QString( "<b>Mobile Volume Weighted Mean:</b> %1" ).arg( histData.weightedMean );
        }
    }
    return infoText;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString Rim3dOverlayInfoConfig::resultInfoText( const RigHistogramData& histData, RimGeoMechView* geoMechView )
{
    QString infoText;

    if ( geoMechView )
    {
        RimGeoMechCase*     geoMechCase           = geoMechView->geoMechCase();
        RigGeoMechCaseData* caseData              = geoMechCase ? geoMechCase->geoMechData() : nullptr;
        bool                isResultsInfoRelevant = caseData && geoMechView->cellResultResultDefinition()->hasResult();

        if ( isResultsInfoRelevant )
        {
            QString resultPos;
            QString fieldName     = geoMechView->cellResultResultDefinition()->resultFieldUiName();
            QString compName      = geoMechView->cellResultResultDefinition()->resultComponentUiName();
            QString diffResString = geoMechView->cellResultResultDefinition()->diffResultUiName();
            switch ( geoMechView->cellResultResultDefinition()->resultPositionType() )
            {
                case RIG_NODAL:
                    resultPos = "Nodal";
                    break;

                case RIG_ELEMENT_NODAL:
                    resultPos = "Element nodal";
                    break;

                case RIG_INTEGRATION_POINT:
                    resultPos = "Integration point";
                    break;

                case RIG_ELEMENT:
                    resultPos = "Element";
                    break;

                case RIG_DIFFERENTIALS:
                    resultPos = "Differentials";
                    break;
                default:
                    break;
            }
            if ( compName == "" )
            {
                infoText += QString( "<b>Cell result:</b> %1, %2<br>" ).arg( resultPos ).arg( fieldName );
            }
            else
            {
                infoText += QString( "<b>Cell result:</b> %1, %2, %3<br>" ).arg( resultPos ).arg( fieldName ).arg( compName );
            }

            if ( geoMechView->cellResultResultDefinition()->isBiotCoefficientDependent() )
            {
                if ( geoMechCase->biotCoefficientType() == RimGeoMechCase::BiotCoefficientType::BIOT_NONE )
                {
                    infoText += QString( "<b>Biot Coefficient</b>: 1.0 (None)<br>" );
                }
                else if ( geoMechCase->biotCoefficientType() == RimGeoMechCase::BiotCoefficientType::BIOT_FIXED )
                {
                    infoText += QString( "<b>Biot Coefficient</b>: %1 (Fixed)<br>" ).arg( geoMechCase->biotFixedCoefficient() );
                }
                else if ( geoMechCase->biotCoefficientType() == RimGeoMechCase::BiotCoefficientType::BIOT_PER_ELEMENT )
                {
                    infoText += QString( "<b>Biot Coefficient</b>: %1 (From element property)<br>" ).arg( geoMechCase->biotResultAddress() );
                }
            }

            const RimGeoMechContourMapView* contourMapView = dynamic_cast<const RimGeoMechContourMapView*>( geoMechView );
            if ( contourMapView )
            {
                if ( !diffResString.isEmpty() )
                {
                    infoText += QString( "%1<br>" ).arg( diffResString );
                }

                if ( histData.isMinMaxValid() )
                {
                    infoText += QString( "<br><b>Statistics:</b> " ) + m_statisticsTimeRange().uiText() + " and " +
                                m_statisticsCellRange().uiText();
                    infoText += QString( "<table border=0 cellspacing=5 >"
                                         "<tr> <td>Min</td> <td>Mean</td> <td>Max</td> </tr>"
                                         "<tr> <td>%1</td>  <td> %2</td> <td> %3</td> </tr>"
                                         "</table>" )
                                    .arg( histData.min )
                                    .arg( histData.mean )
                                    .arg( histData.max );
                }
            }
            else
            {
                if ( !diffResString.isEmpty() )
                {
                    infoText += QString( "%1<br>" ).arg( diffResString );
                }

                if ( histData.isMinMaxValid() )
                {
                    infoText += QString( "<br><b>Statistics:</b> " ) + m_statisticsTimeRange().uiText() + " and " +
                                m_statisticsCellRange().uiText();
                    infoText += QString( "<table border=0 cellspacing=5 >"
                                         "<tr> <td>Min</td> <td>P90</td> <td>Mean</td> <td>P10</td> <td>Max</td> <td>Sum</td> </tr>"
                                         "<tr> <td>%1</td>  <td> %2</td> <td> %3</td>  <td> %4</td> <td> %5</td> <td> %6</td> </tr>"
                                         "</table>" )
                                    .arg( histData.min )
                                    .arg( histData.p90 )
                                    .arg( histData.mean )
                                    .arg( histData.p10 )
                                    .arg( histData.max )
                                    .arg( histData.sum );
                }
            }
        }
        else
        {
            infoText += QString( "<b>No valid result selected</b>" );
        }
    }
    return infoText;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dOverlayInfoConfig::showStatisticsInfoDialog( bool raise )
{
    auto gridView = dynamic_cast<RimGridView*>( m_viewDef.p() );
    if ( gridView )
    {
        RicGridStatisticsDialog* dialog = getOrCreateGridStatisticsDialog();
        // Show dialog before setting data due to text edit auto height setting
        dialog->resize( 600, 800 );
        dialog->show();

        dialog->setLabel( "Grid statistics" );
        dialog->updateFromRimView( gridView );

        if ( raise )
        {
            dialog->raise();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dOverlayInfoConfig::update3DInfo()
{
    updateUiIconFromToggleField();

    if ( !m_viewDef ) return;
    if ( !m_viewDef->viewer() ) return;

    if ( !m_active() )
    {
        m_viewDef->viewer()->showInfoText( false );
        m_viewDef->viewer()->showHistogram( false );
        m_viewDef->viewer()->showAnimationProgress( false );
        m_viewDef->viewer()->showVersionInfo( false );

        update3DInfoIn2dViews();
        return;
    }

    m_viewDef->viewer()->showInfoText( m_showCaseInfo() || ( m_showResultInfo() && !m_viewDef->activeComparisonView() ) );
    m_viewDef->viewer()->showHistogram( false );
    m_viewDef->viewer()->showAnimationProgress( m_showAnimProgress() );
    m_viewDef->viewer()->showVersionInfo( m_showVersionInfo() );

    m_histogramCalculator->invalidateVisibleCellsCache();

    if ( hasInvalidStatisticsCombination() )
    {
        displayPropertyFilteredStatisticsMessage( true );
        m_statisticsTimeRange = RimHistogramCalculator::StatisticsTimeRangeType::CURRENT_TIMESTEP;
    }

    RimEclipseView* reservoirView = dynamic_cast<RimEclipseView*>( m_viewDef.p() );
    if ( reservoirView )
    {
        const RimEclipseStatisticsCase* eclipseStat = dynamic_cast<const RimEclipseStatisticsCase*>( reservoirView->eclipseCase() );
        if ( eclipseStat )
        {
            m_showVolumeWeightedMean = false;
        }
        updateEclipse3DInfo( reservoirView );

        // Update statistics dialog
        getOrCreateGridStatisticsDialog()->updateFromRimView( reservoirView );
    }

    RimGeoMechView* geoMechView = dynamic_cast<RimGeoMechView*>( m_viewDef.p() );
    if ( geoMechView )
    {
        m_showVolumeWeightedMean = false;

        updateGeoMech3DInfo( geoMechView );

        // Update statistics dialog
        getOrCreateGridStatisticsDialog()->updateFromRimView( geoMechView );
    }

    RimSeismicView* seisView = dynamic_cast<RimSeismicView*>( m_viewDef.p() );
    if ( seisView )
    {
        m_showVolumeWeightedMean = false;
        m_showAnimProgress       = false;

        updateSeismicInfo( seisView );
    }

    update3DInfoIn2dViews();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* Rim3dOverlayInfoConfig::objectToggleField()
{
    return &m_active;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dOverlayInfoConfig::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    caf::PdmUiGroup* visGroup = uiOrdering.addNewGroup( "Visibility" );

    RimSeismicView* seisView = dynamic_cast<RimSeismicView*>( m_viewDef.p() );
    if ( seisView )
    {
        visGroup->add( &m_showCaseInfo );
        visGroup->add( &m_showHistogram );
        visGroup->add( &m_showVersionInfo );

        uiOrdering.skipRemainingFields( true );
        return;
    }

    RimEclipseView*           eclipseView = dynamic_cast<RimEclipseView*>( m_viewDef.p() );
    RimEclipseContourMapView* contourMap  = dynamic_cast<RimEclipseContourMapView*>( eclipseView );
    RimGeoMechView*           geoMechView = dynamic_cast<RimGeoMechView*>( m_viewDef.p() );

    bool isEclipseStatsCase = false;
    if ( eclipseView )
    {
        isEclipseStatsCase = dynamic_cast<RimEclipseStatisticsCase*>( eclipseView->eclipseCase() ) != nullptr;
    }

    visGroup->add( &m_showAnimProgress );
    visGroup->add( &m_showCaseInfo );
    visGroup->add( &m_showResultInfo );
    if ( !geoMechView && !contourMap && !isEclipseStatsCase )
    {
        visGroup->add( &m_showVolumeWeightedMean );
    }

    if ( !contourMap )
    {
        visGroup->add( &m_showHistogram );
    }

    visGroup->add( &m_showVersionInfo );

    if ( contourMap )
    {
        m_statisticsTimeRange = RimHistogramCalculator::StatisticsTimeRangeType::CURRENT_TIMESTEP;
        m_statisticsCellRange = RimHistogramCalculator::StatisticsCellRangeType::VISIBLE_CELLS;
    }
    else
    {
        caf::PdmUiGroup* statGroup = uiOrdering.addNewGroup( "Statistics Options" );

        if ( !eclipseView || !eclipseView->cellResult()->isFlowDiagOrInjectionFlooding() )
        {
            statGroup->add( &m_statisticsTimeRange );
        }
        statGroup->add( &m_statisticsCellRange );
    }

    bool isUsingComparisonView = m_viewDef->activeComparisonView();
    m_showResultInfo.uiCapability()->setUiReadOnly( isUsingComparisonView );
    m_showVolumeWeightedMean.uiCapability()->setUiReadOnly( isUsingComparisonView );
    m_showHistogram.uiCapability()->setUiReadOnly( isUsingComparisonView );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dOverlayInfoConfig::setReservoirView( Rim3dView* ownerReservoirView )
{
    m_viewDef = ownerReservoirView;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dOverlayInfoConfig::updateEclipse3DInfo( RimEclipseView* eclipseView )
{
    RigHistogramData histData;

    if ( m_showHistogram() || m_showResultInfo() )
    {
        histData = histogramData();
    }

    QString infoText;

    if ( m_showCaseInfo() )
    {
        infoText = caseInfoText();
    }

    if ( m_showResultInfo() )
    {
        infoText += resultInfoText( histData );
    }

    if ( !infoText.isEmpty() )
    {
        eclipseView->viewer()->setInfoText( infoText );
    }

    if ( m_showHistogram() )
    {
        bool isResultsInfoRelevant = eclipseView->cellResult()->hasResult();

        if ( isResultsInfoRelevant && histData.isHistogramVectorValid() )
        {
            eclipseView->viewer()->showHistogram( true );
            eclipseView->viewer()->setHistogram( histData.min, histData.max, histData.histogram );
            eclipseView->viewer()->setHistogramPercentiles( histData.p90, histData.p10, histData.mean );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dOverlayInfoConfig::updateGeoMech3DInfo( RimGeoMechView* geoMechView )
{
    RigHistogramData histData;

    if ( m_showResultInfo() || m_showHistogram() )
    {
        histData = m_histogramCalculator->histogramData( geoMechView, m_statisticsCellRange(), m_statisticsTimeRange() );
    }

    // Compose text

    QString infoText;

    if ( m_showCaseInfo() )
    {
        infoText = caseInfoText( geoMechView );
    }

    if ( m_showResultInfo() )
    {
        infoText += resultInfoText( histData, geoMechView );
    }

    if ( !infoText.isEmpty() )
    {
        geoMechView->viewer()->setInfoText( infoText );
    }

    // Populate histogram

    if ( m_showHistogram() )
    {
        RimGeoMechCase*     geoMechCase           = geoMechView->geoMechCase();
        RigGeoMechCaseData* caseData              = geoMechCase ? geoMechCase->geoMechData() : nullptr;
        bool                isResultsInfoRelevant = caseData && geoMechView->cellResultResultDefinition()->hasResult();

        if ( isResultsInfoRelevant && histData.isHistogramVectorValid() )
        {
            geoMechView->viewer()->showHistogram( true );
            geoMechView->viewer()->setHistogram( histData.min, histData.max, histData.histogram );
            geoMechView->viewer()->setHistogramPercentiles( histData.p90, histData.p10, histData.mean );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dOverlayInfoConfig::updateSeismicInfo( RimSeismicView* seisView )
{
    RigHistogramData histData;

    if ( m_showResultInfo() || m_showHistogram() )
    {
        histData = seisView->histogramData();
    }

    // Compose text

    QString infoText;

    if ( m_showCaseInfo() )
    {
        infoText = caseInfoText( seisView );
    }

    seisView->viewer()->setInfoText( infoText );

    // Populate histogram

    if ( m_showHistogram() )
    {
        if ( histData.isHistogramVectorValid() )
        {
            seisView->viewer()->showHistogram( true );
            seisView->viewer()->setHistogram( histData.min, histData.max, histData.histogram );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dOverlayInfoConfig::update3DInfoIn2dViews() const
{
    if ( auto rimView = firstAncestorOrThisOfType<Rim3dView>() )
    {
        if ( RimCase* rimCase = rimView->ownerCase() )
        {
            for ( Rim2dIntersectionView* view : rimCase->intersectionViewCollection()->views() )
            {
                view->update3dInfo();
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString Rim3dOverlayInfoConfig::timeStepText( RimEclipseView* eclipseView )
{
    QString dateTimeString;

    if ( eclipseView && eclipseView->currentGridCellResults() )
    {
        int                    currTimeStepIndex = eclipseView->currentTimeStep();
        std::vector<QDateTime> timeSteps         = eclipseView->currentGridCellResults()->allTimeStepDatesFromEclipseReader();

        if ( currTimeStepIndex >= 0 && currTimeStepIndex < (int)timeSteps.size() )
        {
            QString dateFormat = RiaQDateTimeTools::createTimeFormatStringFromDates( timeSteps );

            QString dateString = RiaQDateTimeTools::toStringUsingApplicationLocale( timeSteps[currTimeStepIndex], dateFormat );

            dateTimeString =
                QString( "Time Step: %1/%2  %3" ).arg( QString::number( currTimeStepIndex ), QString::number( timeSteps.size() - 1 ), dateString );
        }
    }

    return QString( "<p><b><center>-- %1 --</center></b>" ).arg( dateTimeString ) +
           QString( "<center>------------------------------------------------</center>" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString Rim3dOverlayInfoConfig::timeStepText( RimGeoMechView* geoMechView )
{
    int currTimeStepIndex = geoMechView->currentTimeStep();

    QStringList timeSteps;
    if ( geoMechView->geoMechCase() ) timeSteps = geoMechView->geoMechCase()->timeStepStrings();

    QString dateTimeString;
    if ( currTimeStepIndex >= 0 && currTimeStepIndex < timeSteps.size() )
    {
        dateTimeString =
            QString( "Time Step: %1/%2  %3" )
                .arg( QString::number( currTimeStepIndex ), QString::number( timeSteps.size() - 1 ), timeSteps[currTimeStepIndex] );
    }

    return QString( "<p><b><center>-- %1 --</center></b>" ).arg( dateTimeString ) +
           QString( "<center>------------------------------------------------</center>" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dOverlayInfoConfig::displayPropertyFilteredStatisticsMessage( bool showSwitchToCurrentTimestep )
{
    static bool isShowing = false;

    QString switchString;
    if ( showSwitchToCurrentTimestep )
    {
        switchString = QString( "<br>"
                                "Switching to statistics for <b>Current Time Step</b>" );
    }

    if ( !isShowing )
    {
        isShowing = true;
        RiaLogging::errorInMessageBox( m_viewDef->viewer()->layoutWidget(),
                                       QString( "ResInsight" ),
                                       QString( "Statistics not available<br>"
                                                "<br>"
                                                "Statistics calculations of <b>Visible Cells</b> for <b>All Time Steps</b> "
                                                "is not supported<br>"
                                                "when you have an active Property filter on a time varying result.<br>" ) +
                                           switchString );
        isShowing = false;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool Rim3dOverlayInfoConfig::hasInvalidStatisticsCombination()
{
    auto gridView = dynamic_cast<RimGridView*>( m_viewDef.p() );

    if ( gridView && gridView->propertyFilterCollection() && gridView->propertyFilterCollection()->hasActiveDynamicFilters() &&
         m_statisticsCellRange() == RimHistogramCalculator::StatisticsCellRangeType::VISIBLE_CELLS &&
         m_statisticsTimeRange() == RimHistogramCalculator::StatisticsTimeRangeType::ALL_TIMESTEPS )
    {
        RimEclipseView* eclipseView = dynamic_cast<RimEclipseView*>( gridView );
        if ( !( eclipseView && eclipseView->cellResult()->isFlowDiagOrInjectionFlooding() ) ) // If
                                                                                              // isFlowDiagOrInjFlooding
                                                                                              // then skip this check as
                                                                                              // ALL_TIMESTEPS is
                                                                                              // overridden to CURRENT
                                                                                              // behind the scenes
        {
            return true;
        }
    }

    return false;
}
