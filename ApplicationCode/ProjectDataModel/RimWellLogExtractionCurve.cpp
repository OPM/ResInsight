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

#include "RimWellLogExtractionCurve.h"
#include "RimProject.h"
#include "RiaApplication.h"
#include "RimOilField.h"
#include "RimWellPathCollection.h"
#include "RimWellPath.h"
#include "RimEclipseCase.h"
#include "RimEclipseResultDefinition.h"
#include "RimGeoMechResultDefinition.h"
#include "RimGeoMechCase.h"
#include "RigEclipseWellLogExtractor.h"
#include "RigResultAccessorFactory.h"
#include "RigCaseCellResultsData.h"
#include "RigCaseData.h"
#include "RimWellLogPlotCurve.h"
#include "RimWellLogPlot.h"
#include "RimWellLogPlotTrack.h"

#include "RiuWellLogTrackPlot.h"
#include "RiuWellLogPlotCurve.h"

#include "RimWellLogPlotCollection.h"
#include "cafPdmUiTreeOrdering.h"
#include "RigGeoMechWellLogExtractor.h"

//==================================================================================================
///  
///  
//==================================================================================================

CAF_PDM_SOURCE_INIT(RimWellLogExtractionCurve, "WellLogEclipseCurve");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogExtractionCurve::RimWellLogExtractionCurve()
{
    CAF_PDM_InitObject("Well Log Curve", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_wellPath, "CurveWellPath", "Well Path", "", "", "");
    m_wellPath.uiCapability()->setUiChildrenHidden(true);
    //m_wellPath.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_case, "CurveCase", "Case", "", "", "");
    m_case.uiCapability()->setUiChildrenHidden(true);
    //m_case.uiCapability()->setUiHidden(true);
    CAF_PDM_InitFieldNoDefault(&m_eclipseResultDefinition, "CurveEclipseResult", "", "", "", "");
    m_eclipseResultDefinition.uiCapability()->setUiHidden(true);
    m_eclipseResultDefinition.uiCapability()->setUiChildrenHidden(true);
    m_eclipseResultDefinition = new RimEclipseResultDefinition;

    CAF_PDM_InitFieldNoDefault(&m_geomResultDefinition, "CurveGeomechResult", "", "", "", "");
    m_geomResultDefinition.uiCapability()->setUiHidden(true);
    m_geomResultDefinition.uiCapability()->setUiChildrenHidden(true);
    m_geomResultDefinition = new RimGeoMechResultDefinition;

    CAF_PDM_InitField(&m_timeStep, "CurveTimeStep", 0,"Time Step", "", "", "");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogExtractionCurve::~RimWellLogExtractionCurve()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogExtractionCurve::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    RimWellLogPlotCurve::fieldChangedByUi(changedField, oldValue, newValue);

    if (changedField == &m_case)
    {
        this->updatePlotData();
    }    
    
    if (changedField == &m_wellPath)
    {
        this->updatePlotData();
    }

    if (changedField == &m_timeStep)
    {
        this->updatePlotData();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogExtractionCurve::updatePlotData()
{
    RimWellLogPlotCurve::updatePlotConfiguration();

    if (isCurveVisibile())
    {
        // Make sure we have set correct case data into the result definitions.

        RimGeoMechCase* geomCase = dynamic_cast<RimGeoMechCase*>(m_case.value());
        RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>(m_case.value());
        m_eclipseResultDefinition->setEclipseCase(eclipseCase);
        m_geomResultDefinition->setGeoMechCase(geomCase);

        RimWellLogPlotCollection* wellLogCollection = NULL;
        this->firstAnchestorOrThisOfType(wellLogCollection);

        CVF_ASSERT(wellLogCollection);

        cvf::ref<RigEclipseWellLogExtractor> eclExtractor = wellLogCollection->findOrCreateExtractor(m_wellPath, eclipseCase);
        cvf::ref<RigGeoMechWellLogExtractor> geomExtractor = wellLogCollection->findOrCreateExtractor(m_wellPath, geomCase);

        std::vector<double> filteredValues;
        std::vector<double> filteredDepths;
        std::vector< std::pair<size_t, size_t> > plotIntervals;

        if (eclExtractor.notNull())
        {
            RimWellLogPlot* wellLogPlot;
            firstAnchestorOrThisOfType(wellLogPlot);
            CVF_ASSERT(wellLogPlot);

            const std::vector<double>& depthValues = wellLogPlot->depthType() == RimWellLogPlot::MEASURED_DEPTH ? eclExtractor->measuredDepth() : eclExtractor->trueVerticalDepth();

            RifReaderInterface::PorosityModelResultType porosityModel = RigCaseCellResultsData::convertFromProjectModelPorosityModel(m_eclipseResultDefinition->porosityModel());
            m_eclipseResultDefinition->loadResult();

            cvf::ref<RigResultAccessor> resAcc = RigResultAccessorFactory::createResultAccessor(
                eclipseCase->reservoirData(), 0,
                porosityModel,
                m_timeStep,
                m_eclipseResultDefinition->resultVariable());

            std::vector<double> values;

            if (resAcc.notNull())
            {
                eclExtractor->curveData(resAcc.p(), &values);
            }

            if (values.size() > 0 && values.size() == depthValues.size())
            {
                validCurvePointIntervals(depthValues, values, plotIntervals);
                addValuesFromIntervals(depthValues, plotIntervals, &filteredDepths);
                addValuesFromIntervals(values, plotIntervals, &filteredValues);
            }
        }
        else if (geomExtractor.notNull()) // geomExtractor
        {
            const std::vector<double>& depthValues = geomExtractor->measuredDepth();
            m_geomResultDefinition->loadResult();
            std::vector<double> values;
            geomExtractor->curveData(m_geomResultDefinition->resultAddress(), m_timeStep, &values);
            
            if (values.size() > 0 && values.size() == depthValues.size())
            {
                validCurvePointIntervals(depthValues, values, plotIntervals);
                addValuesFromIntervals(depthValues, plotIntervals, &filteredDepths);
                addValuesFromIntervals(values, plotIntervals, &filteredValues);
            }
        }

        m_plotCurve->setSamples(filteredValues.data(), filteredDepths.data(), (int)filteredValues.size());
        
        std::vector< std::pair<size_t, size_t> > fltrIntervals;
        filteredIntervals(plotIntervals, &fltrIntervals);

        m_plotCurve->setPlotIntervals(fltrIntervals);

        if (filteredValues.size())
        {
            RimWellLogPlotTrack* plotTrack;
            firstAnchestorOrThisOfType(plotTrack);

            if (plotTrack)
            {
                plotTrack->updateXAxisRangeFromCurves();
            }

            RimWellLogPlot* wellLogPlot;
            firstAnchestorOrThisOfType(wellLogPlot);

            if (wellLogPlot && plotTrack)
            {
                bool setDepthRange = !wellLogPlot->hasAvailableDepthRange();
                wellLogPlot->updateAvailableDepthRange();

                if (setDepthRange)
                {
                    wellLogPlot->setVisibleDepthRangeFromContents();
                }
                else if (plotTrack->curveCount() == 1)
                {
                    plotTrack->updateAxisRangesAndReplot();
                }
            }
        }

        m_plot->replot();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimWellLogExtractionCurve::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly)
{
   QList<caf::PdmOptionItemInfo> optionList;

    if (fieldNeedingOptions == &m_wellPath)
    {
        RimProject* proj = RiaApplication::instance()->project();
        if (proj->activeOilField()->wellPathCollection())
        {
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


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogExtractionCurve::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    RimWellLogPlotCurve::defineUiOrdering(uiConfigName, uiOrdering);

    RimGeoMechCase* geomCase = dynamic_cast<RimGeoMechCase*>(m_case.value());
    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>(m_case.value());

    uiOrdering.add(&m_wellPath);

    caf::PdmUiGroup* group1 = uiOrdering.addNewGroup("Result");
    group1->add(&m_case);
    if (eclipseCase)
    {
        group1->add(&(m_eclipseResultDefinition->m_resultTypeUiField));
        group1->add(&(m_eclipseResultDefinition->m_porosityModelUiField));
        group1->add(&(m_eclipseResultDefinition->m_resultVariableUiField));
    }
    if (geomCase)
    {
        group1->add(&(m_geomResultDefinition->m_resultPositionTypeUiField));
        group1->add(&(m_geomResultDefinition->m_resultVariableUiField));
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogExtractionCurve::initAfterRead()
{
    RimGeoMechCase* geomCase = dynamic_cast<RimGeoMechCase*>(m_case.value());
    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>(m_case.value());

    m_eclipseResultDefinition->setEclipseCase(eclipseCase);
    m_geomResultDefinition->setGeoMechCase(geomCase);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogExtractionCurve::defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/)
{
    uiTreeOrdering.setForgetRemainingFields(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogExtractionCurve::validCurvePointIntervals(const std::vector<double>& depthValues, const std::vector<double>& values, std::vector< std::pair<size_t, size_t> >& intervals)
{
    const size_t valuesCount = values.size();
    CVF_ASSERT(valuesCount == depthValues.size());

    // !! TODO: Find a reasonable tolerance
    const double depthDiffTolerance = 0.1;

    // Find intervals containing valid depth values
    std::vector< std::pair<size_t, size_t> > validDepthIntervals;
    size_t validDepthStartIdx = 0;    
    for (size_t vIdx = 1; vIdx < valuesCount - 1; vIdx += 2)
    {
        if (abs(depthValues[vIdx + 1] - depthValues[vIdx]) > depthDiffTolerance)
        {
            validDepthIntervals.push_back(std::make_pair(validDepthStartIdx, vIdx));
            validDepthStartIdx = vIdx + 1;
        }
    }

    if (validDepthStartIdx >= 0 && validDepthStartIdx < valuesCount)
    {
        validDepthIntervals.push_back(std::make_pair(validDepthStartIdx, valuesCount - 1));
    }

    // Find intervals containing valid values within intervals of valid depth values
    for (size_t intIdx = 0; intIdx < validDepthIntervals.size(); intIdx++)
    {
        size_t intervalIdx1 = validDepthIntervals[intIdx].first;
        bool prevValueValid = false;
        for (size_t vIdx = validDepthIntervals[intIdx].first; vIdx <= validDepthIntervals[intIdx].second; vIdx++)
        {
            double value = values[vIdx];
            if (value == HUGE_VAL || value == -HUGE_VAL || value != value)
            {
                if (prevValueValid && vIdx > intervalIdx1)
                {
                    intervals.push_back(std::make_pair(intervalIdx1, vIdx - 1));
                }
                else intervalIdx1 = vIdx + 1;

                prevValueValid = false;
            }
            else
            {
                prevValueValid = true;
            }
        }
        
        if (intIdx == validDepthIntervals.size() - 1)
        {
            if (intervalIdx1 >= validDepthIntervals[intIdx].first && intervalIdx1 <= validDepthIntervals[intIdx].second)
            {
                intervals.push_back(std::make_pair(intervalIdx1, validDepthIntervals[intIdx].second));
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogExtractionCurve::addValuesFromIntervals(const std::vector<double>& values, std::vector< std::pair<size_t, size_t> >& intervals, std::vector<double>* filteredValues)
{
    CVF_ASSERT(filteredValues);

    for (size_t intIdx = 0; intIdx < intervals.size(); intIdx++)
    {
        for (size_t vIdx = intervals[intIdx].first; vIdx <= intervals[intIdx].second; vIdx++)
        {
            filteredValues->push_back(values[vIdx]);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogExtractionCurve::filteredIntervals(const std::vector< std::pair<size_t, size_t> >& intervals, std::vector< std::pair<size_t, size_t> >* fltrIntervals)
{
    CVF_ASSERT(fltrIntervals);

    const size_t intervalCount = intervals.size();
    if (intervalCount < 1) return;

    size_t index = 0;
    for (size_t intIdx = 0; intIdx < intervalCount; intIdx++)
    {
        size_t intervalSize = intervals[intIdx].second - intervals[intIdx].first + 1;
        fltrIntervals->push_back(std::make_pair(index, index + intervalSize - 1));

        index += intervalSize;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimWellLogExtractionCurve::createCurveName()
{
    RimGeoMechCase* geomCase = dynamic_cast<RimGeoMechCase*>(m_case.value());
    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>(m_case.value());
    QString resVar;
    if (eclipseCase)
    {
        resVar = m_eclipseResultDefinition->resultVariable();
    }

    if (geomCase)
    {
        QString resCompName = m_geomResultDefinition->resultComponentUiName();
        if (resCompName.isEmpty())
            resVar = m_geomResultDefinition->resultFieldUiName();
        else
            resVar = m_geomResultDefinition->resultFieldUiName() + "." + resCompName;
    }

    return resVar;
}

