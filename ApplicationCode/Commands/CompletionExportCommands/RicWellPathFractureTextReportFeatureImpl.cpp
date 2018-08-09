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

#include "RicWellPathFractureTextReportFeatureImpl.h"

#include "RiaApplication.h"

#include "RicExportFractureCompletionsImpl.h"
#include "RicWellPathFractureReportItem.h"

#include "RifEclipseDataTableFormatter.h"

#include "RigCompletionData.h"

#include "RimEclipseCase.h"
#include "RimEllipseFractureTemplate.h"
#include "RimFileWellPath.h"
#include "RimFractureContainment.h"
#include "RimFractureTemplate.h"
#include "RimFractureTemplateCollection.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimStimPlanFractureTemplate.h"
#include "RimTools.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"
#include "RimWellPathFracture.h"
#include "RimWellPathFractureCollection.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString orientationText(RimFractureTemplate::FracOrientationEnum orientation)
{
    return caf::AppEnum<RimFractureTemplate::FracOrientationEnum>::uiText(orientation);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseOutputTableDoubleFormatting floatWithThreeDigits()
{
    return RifEclipseOutputTableDoubleFormatting(RIF_FLOAT, 3);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseOutputTableColumn floatNumberColumn(const QString& text)
{
    return RifEclipseOutputTableColumn(text, RifEclipseOutputTableDoubleFormatting(RIF_FLOAT, 3), RIGHT);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicWellPathFractureTextReportFeatureImpl::wellPathFractureReport(
    RimEclipseCase*                                   sourceCase,
    const std::vector<RimWellPath*>&                  wellPaths,
    const std::vector<RicWellPathFractureReportItem>& wellPathFractureReportItems)
{
    QString lineStart = "--";

    QString     text;
    QTextStream textStream(&text);

    textStream << lineStart
               << "========================================================================================================\n";

    textStream << lineStart << " RESINSIGHT DATA\n";

    textStream << lineStart << "\n";

    if (sourceCase)
    {
        textStream << lineStart << " Grid Model:\n";
        textStream << lineStart << " " << sourceCase->gridFileName() << "\n";
        textStream << lineStart << "\n";
    }

    {
        QString tableText = createWellFileLocationText(wellPaths);
        textStream << tableText;
        textStream << lineStart << "\n";
    }

    auto proj              = RiaApplication::instance()->project();
    auto fractureTemplates = proj->activeOilField()->fractureDefinitionCollection()->fractureTemplates();

    std::vector<RimStimPlanFractureTemplate*> stimPlanTemplates;
    std::vector<RimEllipseFractureTemplate*>  ellipseTemplates;

    for (const auto fracTemplate : fractureTemplates)
    {
        auto stimPlanTemplate = dynamic_cast<RimStimPlanFractureTemplate*>(fracTemplate);
        if (stimPlanTemplate)
        {
            stimPlanTemplates.push_back(stimPlanTemplate);
        }

        auto ellipseTemplate = dynamic_cast<RimEllipseFractureTemplate*>(fracTemplate);
        if (ellipseTemplate)
        {
            ellipseTemplates.push_back(ellipseTemplate);
        }
    }

    {
        QString tableText = createStimPlanFileLocationText(stimPlanTemplates);
        textStream << tableText;
        textStream << lineStart << "\n";
    }

    {
        QString tableText = createEllipseFractureText(ellipseTemplates);
        textStream << tableText;
        textStream << lineStart << "\n";
    }

    {
        QString tableText = createStimPlanFractureText(stimPlanTemplates);
        textStream << tableText;
        textStream << lineStart << "\n";
    }

    {
        QString tableText = createFractureText(fractureTemplates);
        textStream << tableText;
        textStream << lineStart << "\n";
    }

    {
        std::vector<RimWellPathFracture*> wellPathFractures;
        for (const auto& w : wellPaths)
        {
            for (const auto& frac : w->fractureCollection()->fractures())
            {
                wellPathFractures.push_back(frac);
            }
        }

        {
            QString tableText = createFractureInstancesText(wellPathFractures);
            textStream << tableText;
            textStream << lineStart << "\n";
        }

        {
            QString tableText = createFractureCompletionSummaryText(wellPathFractureReportItems);
            textStream << tableText;
            textStream << lineStart << "\n";
        }
    }

    return text;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWellPath*> RicWellPathFractureTextReportFeatureImpl::wellPathsWithFractures()
{
    std::vector<RimWellPath*> wellPaths;

    auto* wellPathColl = RimTools::wellPathCollection();
    if (wellPathColl)
    {
        for (const auto& wellPath : wellPathColl->wellPaths())
        {
            if (!wellPath->fractureCollection()->fractures().empty())
            {
                wellPaths.push_back(wellPath);
            }
        }
    }

    return wellPaths;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicWellPathFractureTextReportFeatureImpl::createWellFileLocationText(const std::vector<RimWellPath*>& wellPaths) const
{
    if (wellPaths.empty()) return "";

    QString tableText;

    QTextStream                  stream(&tableText);
    RifEclipseDataTableFormatter formatter(stream);
    configureFormatter(&formatter);

    std::vector<RifEclipseOutputTableColumn> header = {
        RifEclipseOutputTableColumn("Well"),
        RifEclipseOutputTableColumn("Location"),
    };

    formatter.header(header);

    formatter.addHorizontalLine('-');

    if (!wellPaths.empty())
    {
        for (const auto& wellPath : wellPaths)
        {
            auto fileWellPath = dynamic_cast<RimFileWellPath*>(wellPath);
            if (fileWellPath)
            {
                formatter.add(wellPath->name());
                formatter.add(fileWellPath->filepath());
                formatter.rowCompleted();
            }
        }
    }

    formatter.tableCompleted();

    return tableText;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicWellPathFractureTextReportFeatureImpl::createStimPlanFileLocationText(
    const std::vector<RimStimPlanFractureTemplate*>& stimPlanTemplates) const
{
    if (stimPlanTemplates.empty()) return "";

    QString tableText;

    QTextStream                  stream(&tableText);
    RifEclipseDataTableFormatter formatter(stream);
    configureFormatter(&formatter);

    std::vector<RifEclipseOutputTableColumn> header = {
        RifEclipseOutputTableColumn("StimPlan Name"),
        RifEclipseOutputTableColumn("Location"),
    };

    formatter.header(header);

    formatter.addHorizontalLine('-');

    if (!stimPlanTemplates.empty())
    {
        for (const auto& stimPlanTemplate : stimPlanTemplates)
        {
            formatter.add(stimPlanTemplate->name());
            formatter.add(stimPlanTemplate->fileName());
            formatter.rowCompleted();
        }
    }

    formatter.tableCompleted();

    return tableText;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicWellPathFractureTextReportFeatureImpl::createStimPlanFractureText(
    const std::vector<RimStimPlanFractureTemplate*>& stimPlanTemplates) const
{
    if (stimPlanTemplates.empty()) return "";

    QString tableText;

    QTextStream                  stream(&tableText);
    RifEclipseDataTableFormatter formatter(stream);
    configureFormatter(&formatter);

    std::vector<RifEclipseOutputTableColumn> header = {
        RifEclipseOutputTableColumn("StimPlan"),
        RifEclipseOutputTableColumn(" "),
        floatNumberColumn("WDiam"),
        floatNumberColumn("Skin"),
        floatNumberColumn("Dfac"),
        floatNumberColumn("LPerf"),
    };

    formatter.header(header);

    // Second header line
    {
        formatter.add("Template"); // Template
        formatter.add("Orientation"); // Orientation
        formatter.add("[m]"); // WDiam
        formatter.add("[] "); // Skin
        formatter.add("[...]"); // DFac
        formatter.add("[m]"); // LPerf
        formatter.rowCompleted();
    }

    formatter.addHorizontalLine('-');

    for (const auto& stimPlanTemplate : stimPlanTemplates)
    {
        formatter.add(stimPlanTemplate->name());
        formatter.add(orientationText(stimPlanTemplate->orientationType()));
        formatter.add(stimPlanTemplate->wellDiameter());
        formatter.add(stimPlanTemplate->skinFactor());

        if (stimPlanTemplate->isNonDarcyFlowEnabled())
        {
            formatter.add(stimPlanTemplate->dFactor());
            formatter.add(stimPlanTemplate->perforationLength());
        }
        else
        {
            formatter.add("NA");
            formatter.add("NA");
        }

        formatter.rowCompleted();
    }

    formatter.tableCompleted();

    return tableText;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicWellPathFractureTextReportFeatureImpl::createEllipseFractureText(
    const std::vector<RimEllipseFractureTemplate*>& ellipseTemplates) const
{
    if (ellipseTemplates.empty()) return "";

    QString tableText;

    QTextStream                  stream(&tableText);
    RifEclipseDataTableFormatter formatter(stream);
    configureFormatter(&formatter);

    std::vector<RifEclipseOutputTableColumn> header = {
        RifEclipseOutputTableColumn("Ellipse"),
        RifEclipseOutputTableColumn(" "),
        floatNumberColumn("Xf"),
        floatNumberColumn("Height"),
        floatNumberColumn("Kf"),
        floatNumberColumn("Wf"),
        floatNumberColumn("WDiam"),
        floatNumberColumn("Skin"),
        floatNumberColumn("Dfac"),
        floatNumberColumn("LPerf"),
    };

    formatter.header(header);

    // Second header line
    {
        formatter.add("Template"); // Template
        formatter.add("Orientation"); // Orientation
        formatter.add("[m]"); // Xf
        formatter.add("[m]"); // Height
        formatter.add("[mD]"); // Kf
        formatter.add("[m]"); // Wf
        formatter.add("[m]"); // WDiam
        formatter.add("[] "); // Skin
        formatter.add("[...]"); // DFac
        formatter.add("[m]"); // LPerf
        formatter.rowCompleted();
    }

    formatter.addHorizontalLine('-');

    for (const auto& ellipseTemplate : ellipseTemplates)
    {
        formatter.add(ellipseTemplate->name());
        formatter.add(orientationText(ellipseTemplate->orientationType()));

        formatter.add(ellipseTemplate->halfLength());
        formatter.add(ellipseTemplate->height());
        formatter.add(ellipseTemplate->conductivity());
        formatter.add(ellipseTemplate->width());

        formatter.add(ellipseTemplate->wellDiameter());
        formatter.add(ellipseTemplate->skinFactor());

        if (ellipseTemplate->isNonDarcyFlowEnabled())
        {
            formatter.add(ellipseTemplate->dFactor());
            formatter.add(ellipseTemplate->perforationLength());
        }
        else
        {
            formatter.add("NA");
            formatter.add("NA");
        }

        formatter.rowCompleted();
    }

    formatter.tableCompleted();

    return tableText;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString
    RicWellPathFractureTextReportFeatureImpl::createFractureText(const std::vector<RimFractureTemplate*>& fractureTemplates) const
{
    if (fractureTemplates.empty()) return "";

    QString tableText;

    QTextStream                  stream(&tableText);
    RifEclipseDataTableFormatter formatter(stream);
    configureFormatter(&formatter);

    std::vector<RifEclipseOutputTableColumn> header = {
        RifEclipseOutputTableColumn(" "),
        floatNumberColumn("Top"),
        floatNumberColumn("Bot"),
        floatNumberColumn("Fault"),
        floatNumberColumn("Height"),
        floatNumberColumn("Width"),
        floatNumberColumn("DFac"),
        floatNumberColumn("Conductivity"),
    };

    formatter.header(header);

    // Second header line
    {
        formatter.add("Template");
        formatter.add("Cont");
        formatter.add("Cont");
        formatter.add("Truncation");
        formatter.add("Scale");
        formatter.add("Scale");
        formatter.add("Scale");
        formatter.add("Scale");
        formatter.rowCompleted();
    }

    formatter.addHorizontalLine('-');

    for (const auto& fracTemplate : fractureTemplates)
    {
        formatter.add(fracTemplate->name());

        if (fracTemplate->fractureContainment()->isEnabled())
        {
            formatter.add(fracTemplate->fractureContainment()->topKLayer());
            formatter.add(fracTemplate->fractureContainment()->baseKLayer());
        }
        else
        {
            formatter.add("NA");
            formatter.add("NA");
        }

        if (fracTemplate->fractureContainment()->minimumFaultThrow() >= 0.0)
        {
            formatter.add(fracTemplate->fractureContainment()->minimumFaultThrow());
        }
        else
        {
            formatter.add("NA");
        }

        double heightScale, widthScale, dfactorScale, conductivityScale;
        fracTemplate->scaleFactors(&heightScale, &widthScale, &dfactorScale, &conductivityScale);
        formatter.add(heightScale);
        formatter.add(widthScale);
        formatter.add(dfactorScale);
        formatter.add(conductivityScale);

        formatter.rowCompleted();
    }

    formatter.tableCompleted();

    return tableText;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicWellPathFractureTextReportFeatureImpl::createFractureInstancesText(
    const std::vector<RimWellPathFracture*>& fractures) const
{
    if (fractures.empty()) return "";

    QString tableText;

    QTextStream                  stream(&tableText);
    RifEclipseDataTableFormatter formatter(stream);
    configureFormatter(&formatter);

    std::vector<RifEclipseOutputTableColumn> header = {
        RifEclipseOutputTableColumn("Well"),
        RifEclipseOutputTableColumn("Fracture"),
        RifEclipseOutputTableColumn("Template"),
        floatNumberColumn("MD"),
        floatNumberColumn("Dip"),
        floatNumberColumn("Tilt"),
        floatNumberColumn("LPerf"),
        floatNumberColumn("PerfEff"),
        floatNumberColumn("Wdia"),
    };

    formatter.header(header);

    formatter.addHorizontalLine('-');

    for (const auto& fracture : fractures)
    {
        QString wellName;

        RimWellPath* wellPath = nullptr;
        fracture->firstAncestorOrThisOfType(wellPath);
        if (wellPath)
        {
            wellName = wellPath->name();
        }

        formatter.add(wellName);
        formatter.add(fracture->name());

        if (fracture->fractureTemplate())
        {
            formatter.add(fracture->fractureTemplate()->name());
        }
        else
        {
            formatter.add("NA");
        }

        formatter.add(fracture->fractureMD());
        formatter.add(fracture->dip());
        formatter.add(fracture->tilt());
        formatter.add(fracture->perforationLength());
        formatter.add(fracture->perforationLength());
        formatter.add(fracture->wellRadius() * 2.0);

        formatter.rowCompleted();
    }

    formatter.tableCompleted();

    return tableText;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicWellPathFractureTextReportFeatureImpl::createFractureCompletionSummaryText(
    const std::vector<RicWellPathFractureReportItem>& wellPathFractureReportItems) const
{
    QString tableText;

    QTextStream                  stream(&tableText);
    RifEclipseDataTableFormatter formatter(stream);
    configureFormatter(&formatter);

    std::vector<RifEclipseOutputTableColumn> header = {
        RifEclipseOutputTableColumn(""),
        RifEclipseOutputTableColumn(""),
        RifEclipseOutputTableColumn(""),
        floatNumberColumn("Tr"),
        floatNumberColumn("#con"),
        floatNumberColumn("Fcd"),
        floatNumberColumn("Area"),
        floatNumberColumn("KfWf"),
        floatNumberColumn("Kf"),
        floatNumberColumn("wf"),
        floatNumberColumn("Xf"),
        floatNumberColumn("H"),
        floatNumberColumn("Km"),
    };

    formatter.header(header);

    // Second header line
    {
        formatter.add("Well");
        formatter.add("Fracture");
        formatter.add("Template");
        formatter.add("[Sm3/d/bar]"); // Tr
        formatter.add(""); // #con
        formatter.add("[]"); // Fcd
        formatter.add("[m2]"); // Area
        formatter.add("[mDm]"); // KfWf
        formatter.add("[mD]"); // Kf
        formatter.add("[m]"); // wf
        formatter.add("[m]"); // Xf
        formatter.add("[m]"); // H
        formatter.add("[mD]"); // Km
        formatter.rowCompleted();
    }

    formatter.addHorizontalLine('-');

    // Cache the fracture template area, as this is a heavy operation
    std::map<RimFractureTemplate*, double> templateAreaMap;

    for (const auto& reportItem : wellPathFractureReportItems)
    {
        QString wellPathName, fractureName, fractureTemplateName;
        reportItem.getNames(wellPathName, fractureName, fractureTemplateName);

        formatter.add(wellPathName);
        formatter.add(fractureName);
        formatter.add(fractureTemplateName);

        formatter.add(reportItem.transmissibility());
        formatter.add(reportItem.connectionCount());
        formatter.add(reportItem.fcd());
        formatter.add(reportItem.area());

        formatter.add(reportItem.kfwf()); // KfWf
        formatter.add(reportItem.kf()); // Kf
        formatter.add(reportItem.wf()); // wf

        formatter.add(reportItem.xf()); // Xf
        formatter.add(reportItem.h()); // H
        formatter.add(reportItem.km()); // Km

        formatter.rowCompleted();
    }

    formatter.tableCompleted();

    return tableText;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathFractureTextReportFeatureImpl::configureFormatter(RifEclipseDataTableFormatter* formatter) const
{
    if (!formatter) return;

    formatter->setColumnSpacing(3);
    formatter->setTableRowPrependText("-- ");
    formatter->setTableRowLineAppendText("");
}
