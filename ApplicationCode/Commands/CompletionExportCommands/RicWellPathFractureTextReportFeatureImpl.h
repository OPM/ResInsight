/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018 Statoil ASA
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

#pragma once

#include "cafCmdFeature.h"

class RimWellPath;
class RimWellPathFracture;
class RimEclipseCase;
class RimFractureTemplate;
class RimEllipseFractureTemplate;
class RimStimPlanFractureTemplate;
class RifEclipseDataTableFormatter;
class RicWellPathFractureReportItem;

//==================================================================================================
///
//==================================================================================================
class RicWellPathFractureTextReportFeatureImpl
{
public:
    QString wellPathFractureReport(RimEclipseCase*                                   sourceCase,
                                   const std::vector<RimWellPath*>&                  wellPaths,
                                   const std::vector<RicWellPathFractureReportItem>& wellPathFractureReportItems) const;

    static std::vector<RimWellPath*> wellPathsWithActiveFractures();

private:
    QString createWellFileLocationText(const std::vector<RimWellPath*>& wellPaths) const;
    QString createStimPlanFileLocationText(const std::vector<RimStimPlanFractureTemplate*>& stimPlanTemplates) const;
    QString createStimPlanFractureText(const std::vector<RimStimPlanFractureTemplate*>& stimPlanTemplates) const;
    QString createEllipseFractureText(const std::vector<RimEllipseFractureTemplate*>& ellipseTemplates) const;
    QString createFractureText(const std::vector<RimFractureTemplate*>& fractureTemplates) const;
    QString createFractureInstancesText(const std::vector<RimWellPathFracture*>& fractureTemplates) const;

    QString
        createFractureCompletionSummaryText(const std::vector<RicWellPathFractureReportItem>& wellPathFractureReportItems) const;
    QString createConnectionsPerWellText(const std::vector<RicWellPathFractureReportItem>& wellPathFractureReportItems) const;

    void configureFormatter(RifEclipseDataTableFormatter* formatter) const;
};
