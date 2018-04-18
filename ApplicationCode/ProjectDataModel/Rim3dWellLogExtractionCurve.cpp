/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Statoil ASA
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

#include "Rim3dWellLogExtractionCurve.h"

#include "RigWellLogFile.h"

#include "RiaExtractionTools.h"
#include "RigEclipseWellLogExtractor.h"
#include "RigGeoMechWellLogExtractor.h"
#include "RigResultAccessorFactory.h"
#include "Rim3dView.h"
#include "RimCase.h"
#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseResultDefinition.h"
#include "RimEclipseView.h"
#include "RimGeoMechCase.h"
#include "RimGeoMechResultDefinition.h"
#include "RimGeoMechView.h"
#include "RimTools.h"
#include "RimWellLogFile.h"
#include "RimWellLogFileChannel.h"
#include "RimWellPath.h"

#include <QFileInfo>

//==================================================================================================
///
///
//==================================================================================================

CAF_PDM_SOURCE_INIT(Rim3dWellLogExtractionCurve, "Rim3dWellLogExtractionCurve");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Rim3dWellLogExtractionCurve::Rim3dWellLogExtractionCurve()
{
    CAF_PDM_InitObject("3d Well Log Extraction Curve", ":/WellLogCurve16x16.png", "", "");

    CAF_PDM_InitFieldNoDefault(&m_case, "CurveCase", "Case", "", "", "");
    m_case.uiCapability()->setUiTreeChildrenHidden(true);
    m_case = nullptr;

    CAF_PDM_InitField(&m_timeStep, "CurveTimeStep", 0, "Time Step", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_eclipseResultDefinition, "CurveEclipseResult", "", "", "", "");
    m_eclipseResultDefinition.uiCapability()->setUiHidden(true);
    m_eclipseResultDefinition.uiCapability()->setUiTreeChildrenHidden(true);
    m_eclipseResultDefinition = new RimEclipseResultDefinition;
    m_eclipseResultDefinition->findField("MResultType")->uiCapability()->setUiName("Result Type");

    CAF_PDM_InitFieldNoDefault(&m_geomResultDefinition, "CurveGeomechResult", "", "", "", "");
    m_geomResultDefinition.uiCapability()->setUiHidden(true);
    m_geomResultDefinition.uiCapability()->setUiTreeChildrenHidden(true);
    m_geomResultDefinition = new RimGeoMechResultDefinition;

    m_name = "3D Well Log Curve";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Rim3dWellLogExtractionCurve::~Rim3dWellLogExtractionCurve() 
{
    delete m_geomResultDefinition;
    delete m_eclipseResultDefinition;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Rim3dWellLogExtractionCurve::setPropertiesFromView(Rim3dView* view)
{
    if (!view) return;

    m_case = view->ownerCase();

    RimGeoMechCase* geomCase = dynamic_cast<RimGeoMechCase*>(m_case.value());
    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>(m_case.value());
    m_eclipseResultDefinition->setEclipseCase(eclipseCase);
    m_geomResultDefinition->setGeoMechCase(geomCase);

    RimEclipseView* eclipseView = dynamic_cast<RimEclipseView*>(view);
    if (eclipseView)
    {
        m_eclipseResultDefinition->simpleCopy(eclipseView->cellResult());
        m_timeStep = eclipseView->currentTimeStep();
    }

    RimGeoMechView* geoMechView = dynamic_cast<RimGeoMechView*>(view);
    if (geoMechView)
    {
        m_geomResultDefinition->setResultAddress(geoMechView->cellResultResultDefinition()->resultAddress());
        m_timeStep = geoMechView->currentTimeStep();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dWellLogExtractionCurve::curveValuesAndMds(std::vector<double>* values, std::vector<double>* measuredDepthValues) const
{
    CAF_ASSERT(values != nullptr);
    CAF_ASSERT(measuredDepthValues != nullptr);

    cvf::ref<RigEclipseWellLogExtractor> eclExtractor;
    cvf::ref<RigGeoMechWellLogExtractor> geomExtractor;

    RimWellPath* wellPath;
    firstAncestorOrThisOfType(wellPath);

    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>(m_case());

    if (eclipseCase)
    {
        eclExtractor = RiaExtractionTools::wellLogExtractorEclipseCase(wellPath, eclipseCase);
    }
    else
    {
        RimGeoMechCase* geomCase = dynamic_cast<RimGeoMechCase*>(m_case());
        if (geomCase)
        {
            geomExtractor = RiaExtractionTools::wellLogExtractorGeoMechCase(wellPath, geomCase);
        }
    }

    if (eclExtractor.notNull() && eclipseCase)
    {
        *measuredDepthValues = eclExtractor->measuredDepth();

        m_eclipseResultDefinition->loadResult();

        cvf::ref<RigResultAccessor> resAcc = RigResultAccessorFactory::createFromResultDefinition(eclipseCase->eclipseCaseData(),
                                                                                                  0,
                                                                                                  m_timeStep,
                                                                                                  m_eclipseResultDefinition);
        if (resAcc.notNull())
        {
            eclExtractor->curveData(resAcc.p(), values);
        }
    }
    else if (geomExtractor.notNull())
    {
        *measuredDepthValues = geomExtractor->measuredDepth();

        m_geomResultDefinition->loadResult();

        geomExtractor->curveData(m_geomResultDefinition->resultAddress(), m_timeStep, values);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> Rim3dWellLogExtractionCurve::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions,
                                                                                 bool*                      useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options;

    options = Rim3dWellLogCurve::calculateValueOptions(fieldNeedingOptions, useOptionsOnly);

    if (fieldNeedingOptions == &m_case)
    {
        RimTools::caseOptionItems(&options);

        options.push_front(caf::PdmOptionItemInfo("None", nullptr));
    }
    else if (fieldNeedingOptions == &m_timeStep)
    {
        QStringList timeStepNames;

        if (m_case)
        {
            timeStepNames = m_case->timeStepStrings();
        }

        for (int i = 0; i < timeStepNames.size(); i++)
        {
            options.push_back(caf::PdmOptionItemInfo(timeStepNames[i], i));
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dWellLogExtractionCurve::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    caf::PdmUiGroup* curveDataGroup = uiOrdering.addNewGroup("Curve Data");

    curveDataGroup->add(&m_case);

    RimGeoMechCase* geomCase = dynamic_cast<RimGeoMechCase*>(m_case.value());
    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>(m_case.value());

    if (eclipseCase)
    {
        m_eclipseResultDefinition->uiOrdering(uiConfigName, *curveDataGroup);
    }
    else if (geomCase)
    {
        m_geomResultDefinition->uiOrdering(uiConfigName, *curveDataGroup);
    }

    if ((eclipseCase && m_eclipseResultDefinition->hasDynamicResult())
        || geomCase)
    {
        curveDataGroup->add(&m_timeStep);
    }

    Rim3dWellLogCurve::configurationUiOrdering(uiOrdering);

    uiOrdering.skipRemainingFields(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Rim3dWellLogExtractionCurve::initAfterRead()
{
    RimGeoMechCase* geomCase = dynamic_cast<RimGeoMechCase*>(m_case.value());
    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>(m_case.value());

    m_eclipseResultDefinition->setEclipseCase(eclipseCase);
    m_geomResultDefinition->setGeoMechCase(geomCase);
}
