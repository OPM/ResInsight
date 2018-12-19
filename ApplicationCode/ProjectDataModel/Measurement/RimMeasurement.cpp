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

#include "RimMeasurement.h"

#include "RiaApplication.h"

#include "Rim3dView.h"

#include "MeasurementCommands/RicMeasurementPickEventHandler.h"

#include "RiuViewerCommands.h"


CAF_PDM_SOURCE_INIT(RimMeasurement, "RimMeasurement");


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimMeasurement::RimMeasurement()
    : m_isInMeasurementMode(false)
{
    CAF_PDM_InitObject("Measurement", ":/TextAnnotation16x16.png", "", "");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimMeasurement::~RimMeasurement()
{

}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMeasurement::setMeasurementMode(bool measurementMode)
{
    m_isInMeasurementMode = measurementMode;

    if (m_isInMeasurementMode)
        RiuViewerCommands::setPickEventHandler(RicMeasurementPickEventHandler::instance());
    else
    {
        RiuViewerCommands::removePickEventHandlerIfActive(RicMeasurementPickEventHandler::instance());

        m_pointsInDomain.clear();
        updateView();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimMeasurement::isInMeasurementMode() const
{
    return m_isInMeasurementMode;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMeasurement::addPointInDomain(const Vec3d& pointInDomain)
{
    m_pointsInDomain.push_back(pointInDomain);
    updateView();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3d> RimMeasurement::pointsInDomain() const
{
    return m_pointsInDomain;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMeasurement::updateView() const
{
    auto view = RiaApplication::instance()->activeReservoirView();
    if (view) view->scheduleCreateDisplayModelAndRedraw();
}
