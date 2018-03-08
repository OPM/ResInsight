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

#include "Rim3dWellLogCurve.h"

#include "Rim3dView.h"
#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseResultDefinition.h"
#include "RimEclipseView.h"
#include "RimGeoMechCase.h"
#include "RimGeoMechResultDefinition.h"
#include "RimGeoMechView.h"
#include "RimTools.h"

//==================================================================================================
///  
///  
//==================================================================================================

CAF_PDM_SOURCE_INIT(Rim3dWellLogCurve, "Rim3dWellLogCurve");

namespace caf
{
    template<>
    void AppEnum< Rim3dWellLogCurve::DrawPlane >::setUp()
    {
        addItem(Rim3dWellLogCurve::HORIZONTAL_LEFT, "HORIZONTAL_LEFT", "Horizontal - Left");
        addItem(Rim3dWellLogCurve::HORIZONTAL_RIGHT, "HORIZONTAL_RIGHT", "Horizontal - Right");
        addItem(Rim3dWellLogCurve::VERTICAL_ABOVE, "VERTICAL_ABOVE", "Vertical - Above");
        addItem(Rim3dWellLogCurve::VERTICAL_BELOW, "VERTICAL_BELOW", "Vertical - Below");
        addItem(Rim3dWellLogCurve::CAMERA_ALIGNED_SIDE1, "CAMERA_ALIGNED_SIDE_1", "Camera Aligned - Side 1");
        addItem(Rim3dWellLogCurve::CAMERA_ALIGNED_SIDE2, "CAMERA_ALIGNED_SIDE_2", "Camera Aligned - Side 2");
        setDefault(Rim3dWellLogCurve::HORIZONTAL_LEFT);
    }

    template<>
    void AppEnum< Rim3dWellLogCurve::DrawStyle >::setUp()
    {
        addItem(Rim3dWellLogCurve::LINE, "LINE", "Line");
        addItem(Rim3dWellLogCurve::FILLED, "FILLED", "Filled");
        setDefault(Rim3dWellLogCurve::LINE);
    }

    template<>
    void AppEnum< Rim3dWellLogCurve::ColoringStyle >::setUp()
    {
        addItem(Rim3dWellLogCurve::SINGLE_COLOR, "SINGLE_COLOR", "Single Color");
        addItem(Rim3dWellLogCurve::CURVE_VALUE, "CURVE_VALUE", "Curve Value");
        addItem(Rim3dWellLogCurve::OTHER_RESULT, "OTHER_RESULT", "Other Result");
        setDefault(Rim3dWellLogCurve::SINGLE_COLOR);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Rim3dWellLogCurve::Rim3dWellLogCurve()
{
    CAF_PDM_InitObject("3d Well Log Curve", ":/WellLogCurve16x16.png", "", "");

    CAF_PDM_InitField(&m_showCurve, "Show3dWellLogCurve", true, "Show 3d Well Log Curve", "", "", "");
    m_showCurve.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_drawPlane, "DrawPlane", "Draw Plane", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_drawStyle, "DrawStyle", "Draw Style", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_coloringStyle, "ColoringStyle", "Coloring Style", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_case, "CurveCase", "Case", "", "", "");
    m_case.uiCapability()->setUiTreeChildrenHidden(true);
    m_case = nullptr;

    CAF_PDM_InitFieldNoDefault(&m_eclipseResultDefinition, "CurveEclipseResult", "", "", "", "");
    m_eclipseResultDefinition.uiCapability()->setUiHidden(true);
    m_eclipseResultDefinition.uiCapability()->setUiTreeChildrenHidden(true);
    m_eclipseResultDefinition = new RimEclipseResultDefinition;
    m_eclipseResultDefinition->findField("MResultType")->uiCapability()->setUiName("Result Type");

    CAF_PDM_InitFieldNoDefault(&m_geomResultDefinition, "CurveGeomechResult", "", "", "", "");
    m_geomResultDefinition.uiCapability()->setUiHidden(true);
    m_geomResultDefinition.uiCapability()->setUiTreeChildrenHidden(true);
    m_geomResultDefinition = new RimGeoMechResultDefinition;

    CAF_PDM_InitField(&m_timeStep, "CurveTimeStep", 0, "Time Step", "", "", "");

    CAF_PDM_InitField(&m_name, "Name", QString("3D Well Log Curve"), "3d Well Log Curve", "", "", "");
    m_name.uiCapability()->setUiHidden(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Rim3dWellLogCurve::~Rim3dWellLogCurve()
{
    delete m_geomResultDefinition;
    delete m_eclipseResultDefinition;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Rim3dWellLogCurve::setPropertiesFromView(Rim3dView* view)
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
caf::PdmFieldHandle* Rim3dWellLogCurve::objectToggleField()
{
    return &m_showCurve;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Rim3dWellLogCurve::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> Rim3dWellLogCurve::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options;

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
caf::PdmFieldHandle* Rim3dWellLogCurve::userDescriptionField()
{
    return &m_name;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Rim3dWellLogCurve::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
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

    caf::PdmUiGroup* appearanceGroup = uiOrdering.addNewGroup("Appearance");
    appearanceGroup->add(&m_drawPlane);
    appearanceGroup->add(&m_drawStyle);
    appearanceGroup->add(&m_coloringStyle);

    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Rim3dWellLogCurve::initAfterRead()
{
    RimGeoMechCase* geomCase = dynamic_cast<RimGeoMechCase*>(m_case.value());
    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>(m_case.value());

    m_eclipseResultDefinition->setEclipseCase(eclipseCase);
    m_geomResultDefinition->setGeoMechCase(geomCase);
}

