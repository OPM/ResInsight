/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RimWellLogPlotCurve.h"

#include "RimWellLogPlot.h"

#include "RiuWellLogTracePlot.h"

#include "qwt_plot_curve.h"

#include "cvfAssert.h"

CAF_PDM_SOURCE_INIT(RimWellLogPlotCurve, "WellLogPlotCurve");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogPlotCurve::RimWellLogPlotCurve()
{
    CAF_PDM_InitObject("Curve", "", "", "");

    CAF_PDM_InitField(&m_showCurve, "Show", true, "Show curve", "", "", "");
    m_showCurve.uiCapability()->setUiHidden(true);
    CAF_PDM_InitFieldNoDefault(&m_userName, "CurveDescription", "Name", "", "", "");

    m_plotCurve = new QwtPlotCurve;
    m_plotCurve->setXAxis(QwtPlot::xTop);
    m_plotCurve->setYAxis(QwtPlot::yLeft);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogPlotCurve::~RimWellLogPlotCurve()
{
    m_plotCurve->detach();    
    m_plot->replot();

    delete m_plotCurve;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlotCurve::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (changedField == &m_showCurve)
    {
        if (newValue == true)
        {
            m_plotCurve->attach(m_plot);
        }
        else
        {
            m_plotCurve->detach();
        }

        m_plot->replot();
    }

    m_plotCurve->setTitle(this->m_userName());

    this->updatePlotData();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimWellLogPlotCurve::objectToggleField()
{
    return &m_showCurve;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlotCurve::updatePlotData()
{
    CVF_ASSERT(m_plot);
    
    // Dummy Data

    std::vector<double> values;
    values.push_back(34);
    values.push_back(47);
    values.push_back(49);
    values.push_back(22);
    values.push_back(20);

    std::vector<double> depthValues;
    depthValues.push_back(200);
    depthValues.push_back(400);
    depthValues.push_back(600);
    depthValues.push_back(800);
    depthValues.push_back(1000);

    m_plotCurve->setSamples(values.data(), depthValues.data(), (int) depthValues.size());
  
    RimWellLogPlot* wellLogPlot;
    firstAnchestorOrThisOfType(wellLogPlot);

    if (wellLogPlot)
    {
        wellLogPlot->updateAvailableDepthRange();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlotCurve::setPlot(RiuWellLogTracePlot* plot)
{
    m_plot = plot;
    m_plotCurve->attach(m_plot);
    m_plot->replot();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimWellLogPlotCurve::userDescriptionField()
{
    return &m_userName;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimWellLogPlotCurve::depthRange(double* minimumDepth, double* maximumDepth)
{
    CVF_ASSERT(minimumDepth && maximumDepth);
    CVF_ASSERT(m_plotCurve);

    if (m_plotCurve->data()->size() < 1)
    {
        return false;
    }

    *minimumDepth = m_plotCurve->minYValue();
    *maximumDepth = m_plotCurve->maxYValue();

    return true;
}


#include "RimProject.h"
#include "RiaApplication.h"
#include "RimOilField.h"
#include "RimWellPathCollection.h"
#include "RimWellPath.h"
#include "RimEclipseCase.h"
#include "RimEclipseResultDefinition.h"

//==================================================================================================
///  
///  
//==================================================================================================

CAF_PDM_SOURCE_INIT(RimWellLogEclipseCurve, "WellLogEclipseCurve");
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogEclipseCurve::RimWellLogEclipseCurve()
{
    CAF_PDM_InitObject("Well Log Curve", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_wellPath, "CurveWellPath", "Well Path", "", "", "");
    m_wellPath.uiCapability()->setUiChildrenHidden(true);
    CAF_PDM_InitFieldNoDefault(&m_case, "CurveEclipseCase", "Case", "", "", "");
    m_case.uiCapability()->setUiChildrenHidden(true);
    CAF_PDM_InitFieldNoDefault(&m_resultdefinition, "CurveResult", "", "", "", "");
    m_resultdefinition.uiCapability()->setUiChildrenHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_timeStep, "CurveTimeStep", "Time Step", "", "", "");

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogEclipseCurve::~RimWellLogEclipseCurve()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogEclipseCurve::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    RimWellLogPlotCurve::fieldChangedByUi(changedField, oldValue, newValue);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogEclipseCurve::updatePlotData()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimWellLogEclipseCurve::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly)
{
   QList<caf::PdmOptionItemInfo> optionList;

    if (fieldNeedingOptions == &m_wellPath)
    {
        RimProject* proj = RiaApplication::instance()->project();
        caf::PdmChildArrayField<RimWellPath*>& wellPaths =  proj->activeOilField()->wellPathCollection()->wellPaths;

        for (size_t i = 0; i< wellPaths.size(); i++)
        {
            optionList.push_back(caf::PdmOptionItemInfo(wellPaths[i]->name(), QVariant::fromValue(caf::PdmPointer<caf::PdmObjectHandle>(wellPaths[i]))));
        }

        if (optionList.size() > 0)
        {
            optionList.push_front(caf::PdmOptionItemInfo("None", QVariant::fromValue(caf::PdmPointer<caf::PdmObjectHandle>(NULL))));
        }
    }

    
    if (fieldNeedingOptions == &m_case)
    {
        RimProject* proj = RiaApplication::instance()->project();
        std::vector<RimCase*> cases;

        proj->allCases(cases);

        for (size_t i = 0; i< cases.size(); i++)
        {
            optionList.push_back(caf::PdmOptionItemInfo(cases[i]->caseUserDescription(), QVariant::fromValue(caf::PdmPointer<caf::PdmObjectHandle>(cases[i]))));
        }

        if (optionList.size() > 0)
        {
            optionList.push_front(caf::PdmOptionItemInfo("None", QVariant::fromValue(caf::PdmPointer<caf::PdmObjectHandle>(NULL))));
        }
    }

    return optionList;
}
