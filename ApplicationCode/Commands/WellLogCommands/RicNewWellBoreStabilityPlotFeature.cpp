/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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

#include "RicNewWellBoreStabilityPlotFeature.h"

#include "RicNewWellLogPlotFeatureImpl.h"
#include "RicNewWellLogFileCurveFeature.h"
#include "RicNewWellLogCurveExtractionFeature.h"
#include "RicWellLogTools.h"

#include "RigFemResultAddress.h"
#include "RigFemPartResultsCollection.h"
#include "RigGeoMechCaseData.h"
#include "RigWellPath.h"
#include "RimGeoMechCase.h"
#include "RimGeoMechView.h"
#include "RimProject.h"
#include "RimWellLogPlot.h"
#include "RimWellLogTrack.h"
#include "RimWellLogExtractionCurve.h"
#include "RimWellLogFile.h"
#include "RimWellLogFileCurve.h"
#include "RimWellLogFileChannel.h"
#include "RimWellPath.h"

#include "RicWellLogTools.h"
#include "RiuPlotMainWindowTools.h"

#include "RiaApplication.h"

#include "cafProgressInfo.h"
#include "cvfAssert.h"
#include "cvfMath.h"

#include <QAction>
#include <QDateTime>
#include <QString>

#include <algorithm>

CAF_CMD_SOURCE_INIT(RicNewWellBoreStabilityPlotFeature, "RicNewWellBoreStabilityPlotFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicNewWellBoreStabilityPlotFeature::isCommandEnabled()
{
    RimWellPath* selectedWellPath = RicWellLogTools::findWellPathFromSelection();
    if (!selectedWellPath)
    {
        return false;
    }

    Rim3dView* view = RiaApplication::instance()->activeReservoirView();
    if (!view) return false;
    RimGeoMechView* geoMechView = dynamic_cast<RimGeoMechView*>(view);
    return geoMechView != nullptr;    
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewWellBoreStabilityPlotFeature::onActionTriggered(bool isChecked)
{
    RimWellPath* selectedWellPath = RicWellLogTools::findWellPathFromSelection();
    if (!selectedWellPath) return;

    Rim3dView* view = RiaApplication::instance()->activeReservoirView();
    if (!view) return;

    RimGeoMechView* geoMechView = dynamic_cast<RimGeoMechView*>(view);
    if (!geoMechView) return;

    caf::ProgressInfo progInfo(100, "Creating Well Bore Stability Plot");
    progInfo.setProgressDescription("Creating plot and formation track");
    progInfo.setNextProgressIncrement(2);
    RimGeoMechCase* geoMechCase = geoMechView->geoMechCase();

    QString         plotName("Well Bore Stability");
    RimWellLogPlot* plot = RicNewWellLogPlotFeatureImpl::createWellLogPlot(false, plotName);
    createFormationTrack(plot, selectedWellPath, geoMechCase);
    progInfo.incrementProgressAndUpdateNextStep(3, "Creating casing shoe size track");
    createCasingShoeTrack(plot, selectedWellPath, geoMechCase);
    progInfo.incrementProgressAndUpdateNextStep(75, "Creating stability curves track");
    createStabilityCurvesTrack(plot, selectedWellPath, geoMechView);
    progInfo.incrementProgressAndUpdateNextStep(15, "Creating angles track");
    createAnglesTrack(plot, selectedWellPath, geoMechView);
    progInfo.incrementProgressAndUpdateNextStep(5, "Updating all tracks");
    plot->enableAllAutoNameTags(true);
    plot->setPlotTitleVisible(true);
    plot->setTrackLegendsVisible(true);
    plot->setTrackLegendsHorizontal(true);
    plot->setDepthType(RimWellLogPlot::TRUE_VERTICAL_DEPTH);
    plot->setDepthAutoZoom(true);

    RicNewWellLogPlotFeatureImpl::updateAfterCreation(plot);
    progInfo.incrementProgress();

    RiuPlotMainWindowTools::selectAsCurrentItem(plot);

    // Make sure the summary plot window is visible
    RiuPlotMainWindowTools::showPlotMainWindow();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewWellBoreStabilityPlotFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("New Well Bore Stability Plot");
    actionToSetup->setIcon(QIcon(":/WellLogPlot16x16.png"));
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewWellBoreStabilityPlotFeature::createFormationTrack(RimWellLogPlot* plot, RimWellPath* wellPath, RimGeoMechCase* geoMechCase)
{
    RimWellLogTrack* formationTrack = RicNewWellLogPlotFeatureImpl::createWellLogPlotTrack(false, "Formations", plot);
    formationTrack->setFormationWellPath(wellPath);
    formationTrack->setFormationCase(geoMechCase);
    formationTrack->setShowFormations(true);
    formationTrack->setVisibleXRange(0.0, 0.0);
    formationTrack->setWidthScaleFactor(RimWellLogTrack::NARROW_TRACK);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewWellBoreStabilityPlotFeature::createCasingShoeTrack(RimWellLogPlot* plot, RimWellPath* wellPath, RimGeoMechCase* geoMechCase)
{
    RimWellLogTrack* casingShoeTrack = RicNewWellLogPlotFeatureImpl::createWellLogPlotTrack(false, "Casing Shoe", plot);
    casingShoeTrack->setWidthScaleFactor(RimWellLogTrack::NARROW_TRACK);
    casingShoeTrack->setFormationWellPath(wellPath);
    casingShoeTrack->setFormationCase(geoMechCase);
    casingShoeTrack->setShowFormations(true);
    casingShoeTrack->setShowFormationLabels(false);
    casingShoeTrack->setVisibleXRange(0.0, 0.0);
    std::vector<RimWellLogFile*> wellLogFiles = wellPath->wellLogFiles();
    for (RimWellLogFile* logFile : wellLogFiles)
    {
        std::vector<RimWellLogFileChannel*> channels = logFile->wellLogChannels();
        for (RimWellLogFileChannel* channel : channels)
        {
            if (channel->name() == "CASING_SIZE")
            {
                RimWellLogFileCurve* fileCurve = RicWellLogTools::addFileCurve(casingShoeTrack, false);
                fileCurve->setWellLogFile(logFile);
                fileCurve->setWellPath(wellPath);
                fileCurve->setWellLogChannelName(channel->name());
                fileCurve->setCustomName(QString("Casing size [in]"));
                fileCurve->setLineThickness(2);
                fileCurve->loadDataAndUpdate(false);
                casingShoeTrack->setAutoScaleXEnabled(true);
                casingShoeTrack->calculateXZoomRangeAndUpdateQwt();
                break;
            }
        }
    }    
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewWellBoreStabilityPlotFeature::createStabilityCurvesTrack(RimWellLogPlot* plot, RimWellPath* wellPath, RimGeoMechView* geoMechView)
{
    RimWellLogTrack* stabilityCurvesTrack = RicNewWellLogPlotFeatureImpl::createWellLogPlotTrack(false, "Stability Curves", plot);
    stabilityCurvesTrack->setWidthScaleFactor(RimWellLogTrack::EXTRA_WIDE_TRACK);
    stabilityCurvesTrack->setAutoScaleXEnabled(true);
    stabilityCurvesTrack->setTickIntervals(0.5, 0.05);
    stabilityCurvesTrack->setXAxisGridVisibility(RimWellLogPlot::AXIS_GRID_MAJOR_AND_MINOR);
    stabilityCurvesTrack->setFormationWellPath(wellPath);
    stabilityCurvesTrack->setFormationCase(geoMechView->geoMechCase());
    stabilityCurvesTrack->setShowFormations(true);
    stabilityCurvesTrack->setShowFormationLabels(false);

    std::vector<QString> resultNames = RiaDefines::wellPathStabilityResultNames();

    std::vector<cvf::Color3f> colors = { cvf::Color3f::RED, cvf::Color3f::PURPLE, cvf::Color3f::GREEN, cvf::Color3f::BLUE, cvf::Color3f::ORANGE };
    std::vector<RimPlotCurve::LineStyleEnum> lineStyles = { RimPlotCurve::STYLE_SOLID, RimPlotCurve::STYLE_DASH, RimPlotCurve::STYLE_DASH_DOT, RimPlotCurve::STYLE_SOLID, RimPlotCurve::STYLE_DASH};
    
    for (size_t i = 0; i < resultNames.size(); ++i)
    {
        const QString& resultName = resultNames[i];
        RigFemResultAddress resAddr(RIG_WELLPATH_DERIVED, resultName.toStdString(), "");
        RimWellLogExtractionCurve* curve = RicWellLogTools::addExtractionCurve(stabilityCurvesTrack, geoMechView, wellPath, nullptr, 0, false, false);
        curve->setGeoMechResultAddress(resAddr);
        curve->setCurrentTimeStep(geoMechView->currentTimeStep());
        curve->setCustomName(resultName);
        curve->setColor(colors[i % colors.size()]);
        curve->setLineStyle(lineStyles[i % lineStyles.size()]);
        curve->setLineThickness(2);
        curve->loadDataAndUpdate(false);
    }
    stabilityCurvesTrack->calculateXZoomRangeAndUpdateQwt();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewWellBoreStabilityPlotFeature::createAnglesTrack(RimWellLogPlot* plot, RimWellPath* wellPath, RimGeoMechView* geoMechView)
{
    RimWellLogTrack* wellPathAnglesTrack = RicNewWellLogPlotFeatureImpl::createWellLogPlotTrack(false, "Well Path Angles", plot);
    double minValue = 360.0, maxValue = 0.0;
    const double epsilon = 1.0e-3;
    const double angleIncrement = 90.0;
    std::vector<QString> resultNames = RiaDefines::wellPathAngleResultNames();
    
    std::vector<cvf::Color3f> colors = { cvf::Color3f::DARK_RED, cvf::Color3f::BLUE };

    std::vector<RimPlotCurve::LineStyleEnum> lineStyles = { RimPlotCurve::STYLE_SOLID, RimPlotCurve::STYLE_DASH };

    for (size_t i = 0; i < resultNames.size(); ++i)
    {
        const QString& resultName = resultNames[i];
        RigFemResultAddress resAddr(RIG_WELLPATH_DERIVED, resultName.toStdString(), "");
        RimWellLogExtractionCurve* curve = RicWellLogTools::addExtractionCurve(wellPathAnglesTrack, geoMechView, wellPath, nullptr, 0, false, false);
        curve->setGeoMechResultAddress(resAddr);
        curve->setCurrentTimeStep(geoMechView->currentTimeStep());
        curve->setCustomName(resultName);

        curve->setColor(colors[i % colors.size()]);
        curve->setLineStyle(lineStyles[i % lineStyles.size()]);
        curve->setLineThickness(2);

        curve->loadDataAndUpdate(false);
        
        double actualMinValue = minValue, actualMaxValue = maxValue;
        curve->valueRange(&actualMinValue, &actualMaxValue);
        while (maxValue < actualMaxValue)
        {
            maxValue += angleIncrement;
        }
        while (minValue > actualMinValue)
        {
            minValue -= angleIncrement;
        }
        maxValue = cvf::Math::clamp(maxValue, angleIncrement, 360.0);
        minValue = cvf::Math::clamp(minValue, 0.0, maxValue - 90.0);
    }
    wellPathAnglesTrack->setWidthScaleFactor(RimWellLogTrack::NORMAL_TRACK);
    wellPathAnglesTrack->setVisibleXRange(minValue, maxValue);
    wellPathAnglesTrack->setTickIntervals(90.0, 30.0);
    wellPathAnglesTrack->setXAxisGridVisibility(RimWellLogPlot::AXIS_GRID_MAJOR_AND_MINOR);
    wellPathAnglesTrack->setFormationWellPath(wellPath);
    wellPathAnglesTrack->setFormationCase(geoMechView->geoMechCase());
    wellPathAnglesTrack->setShowFormations(true);
    wellPathAnglesTrack->setShowFormationLabels(false);
}
