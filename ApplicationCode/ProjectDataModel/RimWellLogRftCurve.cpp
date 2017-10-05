/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017  Statoil ASA
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


#include "RimWellLogRftCurve.h"

#include "RimEclipseResultCase.h"
#include "RimTools.h"

#include "RifReaderEclipseRft.h"

#include "cafPdmObject.h"
#include "cvfAssert.h"


CAF_PDM_SOURCE_INIT(RimWellLogRftCurve, "WellLogRftCurve");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogRftCurve::RimWellLogRftCurve()
{
    CAF_PDM_InitObject("Well Log RFT Curve", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_eclipseResultCase, "CurveEclipseResultCase", "Eclipse Result Case", "", "", "");
    m_eclipseResultCase.uiCapability()->setUiTreeChildrenHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_timeStep, "TimeStep", "Time Step", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_wellName, "WellName", "Well Name", "", "", "");
    
    CAF_PDM_InitFieldNoDefault(&m_wellLogChannelName, "WellLogChannelName", "Well Property", "", "", "");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogRftCurve::~RimWellLogRftCurve()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimWellLogRftCurve::wellName() const
{
    //return QString(m_eclipseRftAddress->wellName().c_str());
    return m_wellName;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimWellLogRftCurve::wellLogChannelName() const
{
    //return QString(m_eclipseRftAddress->wellLogChannelName().c_str());
    return m_wellLogChannelName;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<double> RimWellLogRftCurve::yValues()
{
    RifReaderEclipseRft* reader = rftReader();
    std::vector<double> values;

    if (!reader) return values;

    RifEclipseRftAddress address(m_wellName().toStdString(), m_timeStep, m_wellLogChannelName().toStdString());

    reader->values(address, &values);

    return values;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<double> RimWellLogRftCurve::xValues()
{
    RifReaderEclipseRft* reader = rftReader();
    std::vector<double> values;

    if (!reader) return values;

    RifEclipseRftAddress address(m_wellName().toStdString(), m_timeStep, "DEPTH");

    reader->values(address, &values);

    return values;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int RimWellLogRftCurve::currentTimeStep() const
{
    return m_timeStep;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogRftCurve::setEclipseResultCase(RimEclipseResultCase* eclipseResultCase)
{
    m_eclipseResultCase = eclipseResultCase;
    //clearGeneratedSimWellPaths();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseResultCase* RimWellLogRftCurve::eclipseResultCase() const
{
    return m_eclipseResultCase;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimWellLogRftCurve::createCurveAutoName()
{
    QString name = wellName() + ": " + wellLogChannelName();
    
    return name;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogRftCurve::onLoadDataAndUpdate(bool updateParentPlot)
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimWellLogRftCurve::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options;

    options = RimWellLogCurve::calculateValueOptions(fieldNeedingOptions, useOptionsOnly);

    if (options.size() > 0) return options;

    if (fieldNeedingOptions == &m_eclipseResultCase)
    {
        RimTools::caseOptionItems(&options);

        options.push_front(caf::PdmOptionItemInfo("None", nullptr));
    }

    return options;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifReaderEclipseRft* RimWellLogRftCurve::rftReader() const 
{
    if (!m_eclipseResultCase()) return nullptr;

    return m_eclipseResultCase()->rftReader();
}