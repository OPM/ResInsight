/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
//
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT
//  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
//
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////
#include "RiaGridCrossPlotCurveNameHelper.h"

#include "RimGridCrossPlotCurveSet.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGridCrossPlotCurveNameHelper::addCurveSet(RimGridCrossPlotCurveSet* curveSet)
{
    m_caseNameSet.insert(curveSet->caseNameString());
    m_axisVariablesSet.insert(curveSet->axisVariableString());
    m_timeStepSet.insert(curveSet->timeStepString());
    for (auto category : curveSet->groupStrings())
    {
        m_categoriesSet.insert(category);
    }

    m_curveSets.push_back(curveSet);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGridCrossPlotCurveNameHelper::applyCurveNames()
{
    for (auto curveSet : m_curveSets)
    {
        curveSet->updateCurveNames(m_caseNameSet.size() > 1u, m_axisVariablesSet.size() > 1u, m_timeStepSet.size() > 1u, m_categoriesSet.size() > 1u);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGridCrossPlotCurveNameHelper::reset()
{
    m_caseNameSet.clear();
    m_axisVariablesSet.clear();
    m_timeStepSet.clear();
    m_categoriesSet.clear();
    m_curveSets.clear();
}
